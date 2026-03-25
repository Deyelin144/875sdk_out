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
#include <hal_dma.h>
#include <sound_v2/sunxi_adf_core.h>
#include "sunxi-aaudio.h"
#include "sunxi-pcm.h"
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
#include "sunxi-mad.h"
#endif

#define COMPONENT_DRV_NAME	"sunxi-snd-plat-aaudio"
#define COMPONENT_DAC_DRV_NAME	"sunxi-snd-plat-dac-aaudio"
#define COMPONENT_ADC_DRV_NAME	"sunxi-snd-plat-adc-aaudio"

#define DRV_NAME			"sunxi-snd-plat-aaudio-dai"

struct sunxi_cpudai_info {
	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;
};

#ifdef CONFIG_DRIVER_SYSCONFIG
static int sunxi_sound_parse_dma_params(const char* prefix, const char* type_name,
			struct sunxi_cpudai_info *info)
{
	char key_name[32];
	char cma_name[32];
	int ret;
	int32_t tmp_val;

	snprintf(key_name, sizeof(key_name), "%s_plat", prefix);
	if (type_name)
		snprintf(cma_name, sizeof(cma_name), "%s-playback-cma", type_name);
	else
		snprintf(cma_name, sizeof(cma_name), "%s", "playback-cma");
	ret = hal_cfg_get_keyvalue(key_name, cma_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("cpudai:%s miss.\n", cma_name);
		info->playback_dma_param.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES;
	} else {
		if (tmp_val > SUNXI_SOUND_CMA_MAX_KBYTES)
			tmp_val = SUNXI_SOUND_CMA_MAX_KBYTES;
		else if (tmp_val < SUNXI_SOUND_CMA_MIN_KBYTES)
			tmp_val	= SUNXI_SOUND_CMA_MIN_KBYTES;
		info->playback_dma_param.cma_kbytes = tmp_val;
	}

	if (type_name)
		snprintf(cma_name, sizeof(cma_name), "%s-capture-cma", type_name);
	else
		snprintf(cma_name, sizeof(cma_name), "%s", "capture-cma");
	ret = hal_cfg_get_keyvalue(key_name, cma_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("cpudai:%s miss.\n", cma_name);
		info->capture_dma_param.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES;
	} else {
		if (tmp_val > SUNXI_SOUND_CMA_MAX_KBYTES)
			tmp_val = SUNXI_SOUND_CMA_MAX_KBYTES;
		else if (tmp_val < SUNXI_SOUND_CMA_MIN_KBYTES)
			tmp_val	= SUNXI_SOUND_CMA_MIN_KBYTES;
		info->capture_dma_param.cma_kbytes = tmp_val;
	}
	return 0;
}
#else
static struct sunxi_cpudai_info default_aaudio_param = {
	.playback_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
	.capture_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
};
#endif

static int sunxi_cpudai_platform_probe(struct sunxi_sound_adf_component *component)
{
	struct sunxi_cpudai_info *info = NULL;

	snd_debug("\n");

	info = sound_malloc(sizeof(struct sunxi_cpudai_info));
	if (!info) {
		snd_err("no memory\n");
		return -ENOMEM;
	}
	component->private_data = (void *)info;

#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_dma_params("audiocodec", NULL ,info);
#else
	*info = default_aaudio_param;
#endif

	/* dma para */
	info->playback_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_DAC_TXDATA);
	info->playback_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	info->playback_dma_param.dst_maxburst = 4;
	info->playback_dma_param.src_maxburst = 4;

	info->capture_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_ADC_RXDATA);
	info->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	info->capture_dma_param.dst_maxburst = 4;
	info->capture_dma_param.src_maxburst = 4;

	return 0;
}

#if defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)
static int sunxi_cpudai_platform_probe_dac(struct sunxi_sound_adf_component *component)
{
	struct sunxi_cpudai_info *info = NULL;

	snd_debug("\n");

	info = sound_malloc(sizeof(struct sunxi_cpudai_info));
	if (!info) {
		snd_err("no memory\n");
		return -ENOMEM;
	}
	component->private_data = (void *)info;

#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_dma_params("audiocodec", "dac", info);
#else
	*info = default_aaudio_param;
#endif

	/* dma para */
	info->playback_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_DAC_TXDATA);
	info->playback_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	info->playback_dma_param.dst_maxburst = 4;
	info->playback_dma_param.src_maxburst = 4;

	info->capture_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + AC_DAC_LBFIFO);
	info->capture_dma_param.dma_drq_type_num = DRQSRC_CODEC_DAC_RX;
	info->capture_dma_param.dst_maxburst = 4;
	info->capture_dma_param.src_maxburst = 4;

	return 0;
}
#else
static int sunxi_cpudai_platform_probe_dac(struct sunxi_sound_adf_component *component)
{
	(void)component;
	snd_err("api is disable\n");
	return 0;
}
#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)
static int sunxi_cpudai_platform_probe_adc(struct sunxi_sound_adf_component *component)
{
	struct sunxi_cpudai_info *info = NULL;

	snd_debug("\n");

	info = sound_malloc(sizeof(struct sunxi_cpudai_info));
	if (!info) {
		snd_err("no memory\n");
		return -ENOMEM;
	}
	component->private_data = (void *)info;

#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_dma_params("audiocodec", "adc", info);
#else
	*info = default_aaudio_param;
#endif

	/* dma para */
	info->capture_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_ADC_RXDATA);
	info->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	info->capture_dma_param.dst_maxburst = 4;
	info->capture_dma_param.src_maxburst = 4;


	return 0;
}
#else
static int sunxi_cpudai_platform_probe_adc(struct sunxi_sound_adf_component *component)
{
	(void)component;
	snd_err("api is disable\n");
	return 0;
}
#endif

static void sunxi_cpudai_platform_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_cpudai_info *info = NULL;

	snd_debug("\n");
	info = component->private_data;
	sound_free(info);
	component->private_data = NULL;
	return;
}

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
static int sunxi_cpudai_mad_startup(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_cpudai_info *info = component->private_data;
	int ret = 0;
	bool mad_bind = false;

	(void)dataflow;

	snd_debug("mad\n");

	ret = sunxi_sound_mad_bind_get(MAD_PATH_CODECADC, &mad_bind);
	if (ret) {
		mad_bind = false;
		snd_err("get mad_bind failed, path: %d\n", MAD_PATH_CODECADC);
	}

	if (mad_bind) {
		snd_debug("mad bind\n");
		sunxi_sound_sram_dma_config(&info->capture_dma_param);
		return 0;
	}

	snd_debug("mad unbind\n");

	info->capture_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_ADC_RXDATA);
	info->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;

	return 0;
}
#endif

static int sunxi_aaudio_dai_startup(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_cpudai_info *info = component->private_data;

	snd_debug("\n");

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &info->playback_dma_param);
	} else {
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
		snd_debug("mad\n");
		sunxi_cpudai_mad_startup(dataflow, dai);
#endif
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &info->capture_dma_param);
	}

	return 0;
}

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
static void sunxi_cpudai_mad_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_cpudai_info *info = component->private_data;

	(void)dataflow;

	snd_debug("mad\n");

	/*if not use mad again*/
	info->capture_dma_param.dma_addr = (dma_addr_t)(SUNXI_CODEC_BASE_ADDR + SUNXI_ADC_RXDATA);
	info->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
}
#endif

static void sunxi_aaudio_dai_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_MAD
	snd_debug("mad\n");

	if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE)
		sunxi_cpudai_mad_shutdown(dataflow, dai);
#endif
}


static struct sunxi_sound_adf_dai_ops sunxi_cpudai_dai_ops = {
	.startup = sunxi_aaudio_dai_startup,
	.shutdown = sunxi_aaudio_dai_shutdown,
};

static int sunxi_aaudio_dai_probe(struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_cpudai_info *info = component->private_data;

	/* pcm_new will using the dma_param about the cma and fifo params. */
	sunxi_sound_adf_dai_init_dma_data(dai,
				  &info->playback_dma_param,
				  &info->capture_dma_param);
	return 0;
}

static struct sunxi_sound_adf_dai_driver sunxi_aaudio_dai = {
	.name		= DRV_NAME,
	.probe	= 	sunxi_aaudio_dai_probe,
	.playback	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 192000,
	},
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 4,
		.rates		= SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 48000,
	},
	.ops		= &sunxi_cpudai_dai_ops,
};


static struct sunxi_sound_adf_component_driver sunxi_aaudio_component_dev = {
	.remove		= sunxi_cpudai_platform_remove,
};

int sunxi_aaudio_component_probe(enum snd_platform_type plat_type)
{
	int ret;
	char name[48];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			sunxi_aaudio_component_dev.name = COMPONENT_DRV_NAME;
			sunxi_aaudio_component_dev.probe = sunxi_cpudai_platform_probe;
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			sunxi_aaudio_component_dev.name = COMPONENT_DAC_DRV_NAME;
			sunxi_aaudio_component_dev.probe = sunxi_cpudai_platform_probe_dac;
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			sunxi_aaudio_component_dev.name = COMPONENT_ADC_DRV_NAME;
			sunxi_aaudio_component_dev.probe = sunxi_cpudai_platform_probe_adc;
			break;
		default:
			ret = -EINVAL;
			goto err;
	}

	ret = sunxi_sound_adf_register_component(&sunxi_aaudio_component_dev, &sunxi_aaudio_dai, 1);
	if (ret != 0) {
		snd_err("sunxi_sound_adf_register_component failed");
		return ret;
	}

	snprintf(name, sizeof(name), "%s-dma", sunxi_aaudio_component_dev.name);
	ret = sunxi_pcm_dma_platform_register(name);
	if (ret != 0) {
		snd_err("sunxi_pcm_dma_platform_register failed");
		return ret;
	}

err:
	return ret;
}

void sunxi_aaudio_component_remove(enum snd_platform_type plat_type)
{
	char name[48];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			sunxi_aaudio_component_dev.name = COMPONENT_DRV_NAME;
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			sunxi_aaudio_component_dev.name = COMPONENT_DAC_DRV_NAME;
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			sunxi_aaudio_component_dev.name = COMPONENT_ADC_DRV_NAME;
			break;
		default:
			snd_err("type %d is invalid", plat_type);
			return;
	}
	snprintf(name, sizeof(name), "%s-dma", sunxi_aaudio_component_dev.name);

	sunxi_sound_adf_unregister_component(&sunxi_aaudio_component_dev);
	sunxi_pcm_dma_platform_unregister(name);

	return;
}


