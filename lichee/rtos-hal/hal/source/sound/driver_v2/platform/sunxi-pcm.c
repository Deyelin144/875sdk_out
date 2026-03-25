/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>

#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_dma_wrap.h>
#include <sound_v2/sunxi_sound_pcm_dma.h>
#include <sound_v2/sunxi_sound_pcm_common.h>

#include "sunxi-pcm.h"

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
#include "sunxi-mad.h"
#endif

#ifndef sunxi_slave_id
#define sunxi_slave_id(d, s) (((d)<<16) | (s))
#endif


/* playback: period_size=2048*(16*2/8)=8K buffer_size=8K*8=64K */
/* capture:  period_size=2048*(16*4/8)=16K buffer_size=16K*8=128K */
static struct sunxi_sound_pcm_hardware sunxi_pcm_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED
				| SNDRV_PCM_INFO_BLOCK_TRANSFER
				| SNDRV_PCM_INFO_MMAP
				| SNDRV_PCM_INFO_MMAP_VALID
				| SNDRV_PCM_INFO_PAUSE
				| SNDRV_PCM_INFO_RESUME,
	.buffer_bytes_max	= 1024 * 128,
	.period_bytes_min	= 256,
	.period_bytes_max	= 1024 * 64,
	.periods_min		= 2,
	.periods_max		= 16,
};

static int sunxi_pcm_preallocate_dma_buffer(struct sunxi_sound_pcm *pcm,
						int stream,
					    size_t buffer_bytes_max)
{
	struct sunxi_sound_dma_buffer *buf = NULL;
	struct sunxi_sound_pcm_data *pcm_data = &pcm->data[stream];;
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;

	snd_debug("prealloc dma buffer\n");

	dataflow = pcm_data->dataflow;
	if (dataflow == NULL) {
		snd_err("stream=%d dataflow is null!\n", stream);
		return -EFAULT;
	}

	for (dataflow = pcm_data->dataflow; dataflow != NULL; dataflow = dataflow->next) {

		if (!dataflow) {
			snd_info("stream=%d streams is null!\n", stream);
			return -EFAULT;
		}

		buf = &dataflow->dma_buffer;

		if (buffer_bytes_max > SUNXI_SOUND_CMA_MAX_BYTES) {
			buffer_bytes_max = SUNXI_SOUND_CMA_MAX_BYTES;
			snd_info("buffer_bytes_max too max, set %zu\n", buffer_bytes_max);
		}
		if (buffer_bytes_max < SUNXI_SOUND_CMA_MIN_BYTES) {
			buffer_bytes_max = SUNXI_SOUND_CMA_MIN_BYTES;
			snd_info("buffer_bytes_max too min, set %zu\n", buffer_bytes_max);
		}

		buf->addr = hal_malloc_coherent(buffer_bytes_max);
		if (!buf->addr)
			return -ENOMEM;

		buf->bytes = buffer_bytes_max;

		snd_info("dma buffer addr %p size %u\n", buf->addr, buf->bytes);
	}
	return 0;
}

void sunxi_pcm_free_dma_buffer(struct sunxi_sound_pcm *pcm, int stream)
{
	struct sunxi_sound_pcm_data *pcm_data = &pcm->data[stream];
	struct sunxi_sound_pcm_dataflow *dataflow = NULL;
	struct sunxi_sound_dma_buffer *buf;
	snd_debug("\n");

	dataflow = pcm_data->dataflow;
	if (dataflow == NULL) {
		snd_err("stream=%d dataflow is null!\n", stream);
		return;
	}

	for (dataflow = pcm_data->dataflow; dataflow != NULL; dataflow = dataflow->next) {

		if (!dataflow) {
			snd_info("stream=%d streams is null!\n", stream);
			return;
		}

		buf = &dataflow->dma_buffer;
		if (!buf->addr) {
			snd_info("stream=%d buf->addr is null!\n", stream);
			return;
		}

		hal_free_coherent(buf->addr);
		buf->addr = NULL;
	}
	return ;
}

int sunxi_pcm_construct(struct sunxi_sound_adf_component *component,
	struct sunxi_sound_adf_pcm_running_param *rtp)
{
	int ret;
	struct sunxi_sound_pcm *pcm = rtp->pcm;
	struct sunxi_sound_adf_dai_bind *dai_bind = rtp->dai_bind;

	struct sunxi_sound_adf_dai *cpu_dai = adf_rtp_to_cpu(rtp, 0);
	struct sunxi_dma_params *capture_dma_data  = cpu_dai->capture_dma_data;
	struct sunxi_dma_params *playback_dma_data = cpu_dai->playback_dma_data;
	size_t capture_cma_bytes  = SUNXI_SOUND_CMA_BLOCK_BYTES;
	size_t playback_cma_bytes = SUNXI_SOUND_CMA_BLOCK_BYTES;

	snd_debug("\n");

	if (capture_dma_data)
		capture_cma_bytes *= capture_dma_data->cma_kbytes;
	if (playback_dma_data)
		playback_cma_bytes *= playback_dma_data->cma_kbytes;

	if (dai_bind->capture_only) {
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_CAPTURE, capture_cma_bytes);
		if (ret) {
			snd_err("pcm new capture failed, err=%d\n", ret);
			return ret;
		}
	} else if (dai_bind->playback_only) {
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_PLAYBACK, playback_cma_bytes);
		if (ret) {
			snd_err("pcm new playback failed, err=%d\n", ret);
			return ret;
		}
	} else {
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_CAPTURE, capture_cma_bytes);
		if (ret) {
			snd_err("pcm new capture failed, err=%d\n", ret);
			goto err_pcm_prealloc_capture_buffer;
		}
		ret = sunxi_pcm_preallocate_dma_buffer(pcm,
				SNDRV_PCM_STREAM_PLAYBACK, playback_cma_bytes);
		if (ret) {
			snd_err("pcm new playback failed, err=%d\n", ret);
			goto err_pcm_prealloc_playback_buffer;
		}
	}

	return 0;

err_pcm_prealloc_playback_buffer:
	sunxi_pcm_free_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
err_pcm_prealloc_capture_buffer:

	return ret;
}

static void sunxi_pcm_destruct(struct sunxi_sound_adf_component *component, struct sunxi_sound_pcm *pcm)
{
	int stream;

	snd_debug("\n");

	for (stream = 0; stream < SNDRV_PCM_STREAM_LAST; stream++) {
		sunxi_pcm_free_dma_buffer(pcm, stream);
	}
}

static int sunxi_pcm_open(struct sunxi_sound_adf_component *component,
	struct sunxi_sound_pcm_dataflow *dataflow)

{
	int ret;
	struct sunxi_sound_adf_pcm_running_param *rtp = dataflow->priv_data;
	struct sunxi_dma_params *dma_params = NULL;

	dma_params = sunxi_sound_adf_dai_get_dma_data(adf_rtp_to_cpu(rtp, 0), dataflow);
	sunxi_pcm_hardware.buffer_bytes_max = dma_params->cma_kbytes * SUNXI_SOUND_CMA_BLOCK_BYTES;
	sunxi_pcm_hardware.period_bytes_max = sunxi_pcm_hardware.buffer_bytes_max / 2;

	sunxi_adf_sound_set_runtime_hwparams(dataflow, &sunxi_pcm_hardware);
	snd_info("request dma channel\n");
	/* request dma channel */
	ret = sunxi_sound_dmaengine_pcm_open_request_chan(dataflow);
	if (ret != 0)
		snd_err("dmaengine pcm open failed with err %d\n", ret);
	return ret;
}

static int sunxi_pcm_close(struct sunxi_sound_adf_component *component,
	struct sunxi_sound_pcm_dataflow *dataflow)
{
	snd_debug("\n");
	return sunxi_sound_dmaengine_pcm_close_release_chan(dataflow);
}

static int sunxi_pcm_ioctl(struct sunxi_sound_adf_component *component,
			   struct sunxi_sound_pcm_dataflow *dataflow,
			   unsigned int cmd, void *arg)
{
	snd_debug("cmd -> %u\n", cmd);

	return sunxi_sound_pcm_lib_ioctl(dataflow, cmd, arg);
}

static int sunxi_pcm_hw_params(struct sunxi_sound_adf_component *component,
		struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_pcm_hw_params *params)
{
	int ret;
	struct dma_chan *chan;
	struct dma_slave_config slave_config = {0};
	struct sunxi_sound_adf_pcm_running_param *rtp = dataflow->priv_data;
	struct sunxi_dma_params *dma_params;
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
	bool mad_bind = false;
#endif

	chan = sunxi_sound_dmaengine_pcm_get_chan(dataflow);
	if (chan == NULL) {
		snd_err("dma pcm get chan failed! chan is NULL\n");
		return -EINVAL;
	}

	dma_params = sunxi_sound_adf_dai_get_dma_data(adf_rtp_to_cpu(rtp, 0), dataflow);

	ret = sunxi_sound_hwparams_to_dma_slave_config(dataflow, params, &slave_config);
	if (ret != 0) {
		snd_err("hw params config failed with err %d\n", ret);
		return ret;
	}
	slave_config.dst_maxburst = dma_params->dst_maxburst;
	slave_config.src_maxburst = dma_params->src_maxburst;

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
	struct sunxi_sound_card *card = dataflow->pcm->card;
	if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (!strncmp(card->name, "audiocodecadc", 13)) {
			ret = sunxi_sound_mad_bind_get(MAD_PATH_CODECADC, &mad_bind);
			if (ret) {
				mad_bind = false;
				snd_err("get mad_bind failed, path: %d\n", MAD_PATH_CODECADC);
			}
		} else if (!strncmp(card->name, "snddmic", 7)) {
			ret = sunxi_sound_mad_bind_get(MAD_PATH_DMIC, &mad_bind);
			if (ret) {
				mad_bind = false;
				snd_err("get mad_bind failed, path: %d\n", MAD_PATH_DMIC);
			}
		} else if (!strncmp(card->name, "snddaudio0", 10)) {
			ret = sunxi_sound_mad_bind_get(MAD_PATH_I2S0, &mad_bind);
			if (ret) {
				mad_bind = false;
				snd_err("get mad_bind failed, path: %d\n", MAD_PATH_I2S0);
			}
		}
		snd_info("mad_bind[%s]: %s\n", card->name, mad_bind ? "On":"Off");
		if (mad_bind) {
			slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		}
	}
#endif

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		slave_config.dst_addr = (unsigned long)dma_params->dma_addr;
		slave_config.src_addr_width = slave_config.dst_addr_width;
		slave_config.slave_id = sunxi_slave_id(dma_params->dma_drq_type_num, 0);
	} else {
		slave_config.src_addr = (unsigned long)dma_params->dma_addr;
		slave_config.dst_addr_width = slave_config.src_addr_width;
		slave_config.slave_id = sunxi_slave_id(0, dma_params->dma_drq_type_num);
	}
	snd_debug("src_addr:%p, dst_addr:%p, id:%u, d:%u, aw:%u, dw:%u, sm:%u, dm:%u, drq_type:%d\n",
		(void *)slave_config.src_addr, (void *)slave_config.dst_addr, slave_config.slave_id,
		slave_config.direction, slave_config.src_addr_width, slave_config.dst_addr_width,
		slave_config.src_maxburst, slave_config.dst_maxburst, dma_params->dma_drq_type_num);

	ret = sunxi_dmaengine_slave_config(chan, &slave_config);
	if (ret != 0) {
		snd_err("dmaengine_slave_config failed with err %d\n", ret);
		return ret;
	}

	sunxi_sound_pcm_set_runtime_buffer(dataflow, &dataflow->dma_buffer);

	return 0;
}

static int sunxi_pcm_hw_free(struct sunxi_sound_adf_component *component,
		struct sunxi_sound_pcm_dataflow *dataflow)
{
	snd_debug("\n");
	sunxi_sound_pcm_set_runtime_buffer(dataflow, NULL);
	return 0;
}

static int sunxi_pcm_trigger(struct sunxi_sound_adf_component *component,
	struct sunxi_sound_pcm_dataflow *dataflow, int cmd)
{
	snd_info("stream:%u, cmd:%u\n", dataflow->stream, cmd);
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			sunxi_sound_dmaengine_pcm_trigger(dataflow,
					SNDRV_PCM_TRIGGER_START);
			break;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			sunxi_sound_dmaengine_pcm_trigger(dataflow,
					SNDRV_PCM_TRIGGER_STOP);
			break;
		}
	} else {
		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			sunxi_sound_dmaengine_pcm_trigger(dataflow,
					SNDRV_PCM_TRIGGER_START);
			break;
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			sunxi_sound_dmaengine_pcm_trigger(dataflow,
					SNDRV_PCM_TRIGGER_STOP);
			break;
		}
	}
	return 0;
}

static snd_pcm_uframes_t sunxi_pcm_pointer(struct sunxi_sound_adf_component *component,
			struct sunxi_sound_pcm_dataflow *dataflow)
{
	return sunxi_sound_dmaengine_pcm_pointer(dataflow);
}

static struct sunxi_sound_adf_component_driver sunxi_adf_platform = {
	.pcm_construct	= sunxi_pcm_construct,
	.pcm_destruct	= sunxi_pcm_destruct,
	.open		= sunxi_pcm_open,
	.close		= sunxi_pcm_close,
	.ioctl		= sunxi_pcm_ioctl,
	.hw_params	= sunxi_pcm_hw_params,
	.hw_free	= sunxi_pcm_hw_free,
	.trigger	= sunxi_pcm_trigger,
	.pointer	= sunxi_pcm_pointer,
};

int sunxi_pcm_dma_platform_register(const char *name)
{
	snd_debug("\n");

	sunxi_adf_platform.name = name;

	return sunxi_sound_adf_register_component(&sunxi_adf_platform, NULL, 0);
}

void sunxi_pcm_dma_platform_unregister(const char *name)
{
	snd_debug("\n");

	sunxi_adf_platform.name = name;

	sunxi_sound_adf_unregister_component(&sunxi_adf_platform);
}


