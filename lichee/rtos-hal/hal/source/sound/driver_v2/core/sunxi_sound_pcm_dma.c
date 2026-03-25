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
#include <stdlib.h>
#include <string.h>
#include <hal_osal.h>
#include <hal_cache.h>
#include <aw_common.h>

#include <sound_v2/sunxi_sound_dma_wrap.h>
#include <sound_v2/sunxi_sound_pcm_dma.h>
#include <sound_v2/sunxi_sound_pcm_common.h>

struct sunxi_sound_dmaengine_pcm_running_data {
	struct dma_chan *dma_chan;
	/* used for no residue, noneed now */
	uint32_t pos;
};

static inline struct sunxi_sound_dmaengine_pcm_running_data *dataflow_to_prtd(
	const struct sunxi_sound_pcm_dataflow *dataflow)
{
	return dataflow->pcm_running->private_data;
}

struct dma_chan *sunxi_sound_dmaengine_pcm_get_chan(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_dmaengine_pcm_running_data *prtd = dataflow_to_prtd(dataflow);

	return prtd->dma_chan;
}

int sunxi_sound_hwparams_to_dma_slave_config(const struct sunxi_sound_pcm_dataflow *dataflow,
				const struct sunxi_sound_pcm_hw_params *params,
				struct dma_slave_config *slave_config)
{
	enum dma_slave_buswidth buswidth;
	switch (params_format(params)) {
		case SND_PCM_FORMAT_S8:
			buswidth = DMA_SLAVE_BUSWIDTH_1_BYTE;
			break;
		case SND_PCM_FORMAT_S16_LE:
			buswidth = DMA_SLAVE_BUSWIDTH_2_BYTES;
			break;
		case SND_PCM_FORMAT_S24_LE:
		case SND_PCM_FORMAT_S32_LE:
			buswidth = DMA_SLAVE_BUSWIDTH_4_BYTES;
			break;
		default:
		return -EINVAL;
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		slave_config->direction = DMA_MEM_TO_DEV;
		slave_config->dst_addr_width = buswidth;
	} else {
		slave_config->direction = DMA_DEV_TO_MEM;
		slave_config->src_addr_width = buswidth;
	}
	return 0;
}

int sunxi_sound_dmaengine_pcm_open_request_chan(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct dma_chan *chan;
	struct sunxi_sound_dmaengine_pcm_running_data *prtd;

	chan = sunxi_dma_request_channel();
	if (!chan)
		return -ENXIO;
	prtd = sound_malloc(sizeof(struct sunxi_sound_dmaengine_pcm_running_data));
	if (!prtd)
		return -ENOMEM;
	prtd->dma_chan = chan;
	dataflow->pcm_running->private_data = prtd;

	return 0;
}

int sunxi_sound_dmaengine_pcm_close_release_chan(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_dmaengine_pcm_running_data *prtd = dataflow_to_prtd(dataflow);

	sunxi_dma_release_channel(prtd->dma_chan);
	sound_free(prtd);
	dataflow->pcm_running->private_data = NULL;
	return 0;
}


snd_pcm_uframes_t sunxi_sound_dmaengine_pcm_pointer(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_dmaengine_pcm_running_data *prtd = dataflow_to_prtd(dataflow);
	uint32_t residue = 0;
	enum dma_status status;
	uint32_t pos = 0;
	uint32_t buf_size;

	status = sunxi_dmaengine_tx_status(prtd->dma_chan, &residue);
	snd_debug("dma status:%u, residue:%u(0x%x) bytes\n", status, residue, residue);
	if (status == DMA_IN_PROGRESS || status == DMA_PAUSED) {
		buf_size = sunxi_sound_pcm_lib_buffer_bytes(dataflow);
		if (residue > 0 && residue <= buf_size)
			pos = buf_size - residue;
	}
	snd_debug("----pos:0x%x(%u) bytes, pos frames offset:0x%lx\n",
			pos, pos, bytes_to_frames(dataflow->pcm_running, pos));
	return bytes_to_frames(dataflow->pcm_running, pos);
}

static inline enum dma_transfer_direction
 sunxi_sound_pcm_substream_to_dma_direction(const struct sunxi_sound_pcm_dataflow *dataflow)
{
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return DMA_MEM_TO_DEV;
	else
		return DMA_DEV_TO_MEM;
}

static void sunxi_sound_dmaengine_pcm_dma_complete(void *arg)
{
	struct sunxi_sound_pcm_dataflow *dataflow = arg;
	struct sunxi_sound_dmaengine_pcm_running_data *prtd;

	snd_debug("=========dma callback start============\n");
	if (!dataflow->pcm_running)
		return;
	prtd = dataflow_to_prtd(dataflow);
	prtd->pos += sunxi_sound_pcm_lib_period_bytes(dataflow);
	if (prtd->pos >= sunxi_sound_pcm_lib_buffer_bytes(dataflow))
		prtd->pos = 0;
	sunxi_sound_pcm_period_elapsed(dataflow);
	snd_debug("==========dma callback finish===========\n");
}

static int sunxi_sound_dmaengine_pcm_prepare_and_submit(struct sunxi_sound_pcm_dataflow *dataflow)
{
	struct sunxi_sound_dmaengine_pcm_running_data *prtd = dataflow_to_prtd(dataflow);
	struct dma_chan *chan = prtd->dma_chan;
	enum dma_transfer_direction direction;
	int ret = 0;
	dma_callback callback;

	direction =  sunxi_sound_pcm_substream_to_dma_direction(dataflow);

	prtd->pos = 0;

	/* flush ringbuffer */
	hal_dcache_clean_invalidate((unsigned long)(dataflow->pcm_running->dma_addr),
				sunxi_sound_pcm_lib_buffer_bytes(dataflow));

	ret = sunxi_dmaengine_prep_dma_cyclic(chan,
		dataflow->pcm_running->dma_addr,
		sunxi_sound_pcm_lib_buffer_bytes(dataflow),
		sunxi_sound_pcm_lib_period_bytes(dataflow), direction);
	if (ret != 0) {
		snd_err("[%s] dma cyclic failed!!!\n", __func__);
		return ret;
	}
	callback = sunxi_sound_dmaengine_pcm_dma_complete;

	sunxi_dmaengine_submit(chan, callback, (void *)dataflow);

	return 0;
}

int sunxi_sound_dmaengine_pcm_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd)
{
	struct sunxi_sound_dmaengine_pcm_running_data *prtd = dataflow_to_prtd(dataflow);
	int ret;

	snd_debug("\n");
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ret = sunxi_sound_dmaengine_pcm_prepare_and_submit(dataflow);
		if (ret < 0)
			return ret;
		sunxi_dma_async_issue_pending(prtd->dma_chan);
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		sunxi_dmaengine_resume(prtd->dma_chan);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
#if 0
		if (runtime->info & SNDRV_PCM_INFO_PAUSE)
			dmaengine_pause(prtd->dma_chan);
		else
#endif
		sunxi_dmaengine_terminate_async(prtd->dma_chan);
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		sunxi_dmaengine_pause(prtd->dma_chan);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		sunxi_dmaengine_terminate_async(prtd->dma_chan);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


