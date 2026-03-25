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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <hal_dma.h>
#include <hal_cmd.h>
#include <hal_timer.h>
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif

#include <sound_v2/sunxi_sound_io.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include "sunxi_sound_common.h"


#include "sunxi-pcm.h"
#include "sunxi-owa.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-plat-owa"
#define DRV_NAME			"sunxi-snd-plat-owa-dai"

struct sunxi_owa_info {
	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;

	struct sunxi_owa_clk clk;
};

static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_OWA_CTL),
	REG_LABEL(SUNXI_OWA_TXCFG),
	REG_LABEL(SUNXI_OWA_RXCFG),
	REG_LABEL(SUNXI_OWA_INT_STA),
	/* REG_LABEL(SUNXI_OWA_RXFIFO), */
	REG_LABEL(SUNXI_OWA_FIFO_CTL),
	REG_LABEL(SUNXI_OWA_FIFO_STA),
	REG_LABEL(SUNXI_OWA_INT),
	/* REG_LABEL(SUNXI_OWA_TXFIFO), */
	REG_LABEL(SUNXI_OWA_TXCNT),
	REG_LABEL(SUNXI_OWA_RXCNT),
	REG_LABEL(SUNXI_OWA_TXCH_STA0),
	REG_LABEL(SUNXI_OWA_TXCH_STA1),
	REG_LABEL(SUNXI_OWA_RXCH_STA0),
	REG_LABEL(SUNXI_OWA_RXCH_STA1),
#ifdef CONFIG_SND_SUNXI_OWA_RX_IEC61937
	REG_LABEL(SUNXI_OWA_EXP_CTL),
	REG_LABEL(SUNXI_OWA_EXP_ISTA),
	REG_LABEL(SUNXI_OWA_EXP_INFO0),
	REG_LABEL(SUNXI_OWA_EXP_INFO1),
	REG_LABEL(SUNXI_OWA_EXP_DBG0),
	REG_LABEL(SUNXI_OWA_EXP_DBG1),
	REG_LABEL(SUNXI_OWA_EXP_VER),
#endif
	REG_LABEL_END,
};

static const struct owa_rate sample_rate_orig[] = {
	{22050,  0xB},
	{24000,  0x9},
	{32000,  0xC},
	{44100,  0xF},
	{48000,  0xD},
	{88200,  0x7},
	{96000,  0x5},
	{176400, 0x3},
	{192000, 0x1},
};

static const struct owa_rate sample_rate_freq[] = {
	{22050,  0x4},
	{24000,  0x6},
	{32000,  0x3},
	{44100,  0x0},
	{48000,  0x2},
	{88200,  0x8},
	{96000,  0xA},
	{176400, 0xC},
	{192000, 0xE},
};

static int sunxi_owa_get_audio_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	unsigned int reg_val;
	unsigned long value = 0;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	reg_val = sunxi_sound_component_read(component, SUNXI_OWA_TXCFG);
	value = ((reg_val >> TXCFG_DATA_TYPE) & 0x1);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_owa_set_audio_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}


	sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
		(0x1 << TXCFG_DATA_TYPE), (info->value << TXCFG_DATA_TYPE));
	sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA0,
		(0x1 << TXCHSTA0_AUDIO), (info->value << TXCHSTA0_AUDIO));
	sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA0,
		(0x1 << RXCHSTA0_AUDIO), (info->value << RXCHSTA0_AUDIO));

	snd_info("mask:0x%x, items:%d, value:0x%lx\n",
			adf_control->mask, adf_control->items, info->value);

	return 0;
}

#ifdef CONFIG_SND_SUNXI_OWA_RX_IEC61937
static int sunxi_owa_get_rx_data_type(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	unsigned int reg_val;
	unsigned long value = 0;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	reg_val = sunxi_sound_component_read(component, SUNXI_OWA_EXP_CTL);
	value = ((reg_val >> RX_MODE_MAN) & 0x1);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_owa_set_rx_data_type(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}

	sunxi_sound_component_update_bits(component, SUNXI_OWA_EXP_CTL,
		(0x1 << RX_MODE), (0x0 << RX_MODE));
	sunxi_sound_component_update_bits(component, SUNXI_OWA_EXP_CTL,
		(0x1 << RX_MODE_MAN), (info->value << RX_MODE_MAN));

	snd_info("mask:0x%x, items:%d, value:0x%lx\n",
			adf_control->mask, adf_control->items, info->value);

	return 0;
}
#endif

static int sunxi_owa_get_audio_hub_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	unsigned int reg_val;
	unsigned long value = 0;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	reg_val = sunxi_sound_component_read(component, SUNXI_OWA_FIFO_CTL);
	value = ((reg_val >> FIFO_CTL_HUBEN) & 0x1);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_owa_set_audio_hub_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}

	sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
		(0x1 << FIFO_CTL_HUBEN), (info->value << FIFO_CTL_HUBEN));
	sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
		(0x1 << TXCFG_TXEN), (info->value << TXCFG_TXEN));

	snd_info("mask:0x%x, items:%d, value:0x%x\n",
			adf_control->mask, adf_control->items, info->value);

	return 0;
}

static const char * const sunxi_owa_audio_format_function[] = {"PCM", "DTS"};

#ifdef CONFIG_SND_SUNXI_OWA_RX_IEC61937
static const char * const sunxi_owa_rx_data_type[] = {"IEC-60958", "IEC-61937"};
#endif

static const char * const sunxi_owa_audio_hub_mode[] = {"Disabled", "Enabled"};

static struct sunxi_sound_adf_control_new sunxi_owa_controls[] = {
	SOUND_CTRL_ENUM_EXT("owa audio format function",
					ARRAY_SIZE(sunxi_owa_audio_format_function),
					sunxi_owa_audio_format_function,
					SUNXI_SOUND_CTRL_PART_AUTO_MASK,
					sunxi_owa_get_audio_mode,
					sunxi_owa_set_audio_mode),
#ifdef CONFIG_SND_SUNXI_OWA_RX_IEC61937
	SOUND_CTRL_ENUM_EXT("owa rx data type",
					ARRAY_SIZE(sunxi_owa_rx_data_type),
					sunxi_owa_rx_data_type,
					SUNXI_SOUND_CTRL_PART_AUTO_MASK,
					sunxi_owa_get_rx_data_type,
					sunxi_owa_set_rx_data_type),
#endif
	SOUND_CTRL_ENUM_EXT("owa audio hub mode",
					ARRAY_SIZE(sunxi_owa_audio_hub_mode),
					sunxi_owa_audio_hub_mode,
					SUNXI_SOUND_CTRL_PART_AUTO_MASK,
					sunxi_owa_get_audio_hub_mode,
					sunxi_owa_set_audio_hub_mode),
	SOUND_CTRL_ADF_CONTROL("sunxi owa loopback debug", SUNXI_OWA_CTL, CTL_LOOP_EN, 0x1),
};

/*
 * Configure DMA , Chan enable & Global enable
 */
static void sunxi_owa_txctrl_enable(struct sunxi_sound_adf_component *component, bool enable)
{
	snd_debug("\n");
	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
				(0x1 << TXCFG_TXEN), (0x1 << TXCFG_TXEN));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_INT,
				(0x1 << INT_TXDRQEN), (0x1 << INT_TXDRQEN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
				(0x1 << TXCFG_TXEN), (0x0 << TXCFG_TXEN));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_INT,
				(0x1 << INT_TXDRQEN), (0x0 << INT_TXDRQEN));
	}
}

static void sunxi_owa_rxctrl_enable(struct sunxi_sound_adf_component *component, bool enable)
{
	snd_debug("\n");
	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCFG,
				(0x1 << RXCFG_CHSR_CP), (0x1 << RXCFG_CHSR_CP));

		sunxi_sound_component_update_bits(component, SUNXI_OWA_INT,
				(0x1 << INT_RXDRQEN), (0x1 << INT_RXDRQEN));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCFG,
				(0x1 << RXCFG_RXEN), (0x1 << RXCFG_RXEN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCFG,
				(0x1 << RXCFG_RXEN), (0x0 << RXCFG_RXEN));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_INT,
				(0x1 << INT_RXDRQEN), (0x0 << INT_RXDRQEN));
	}
}

static void sunxi_owa_init(struct sunxi_sound_adf_component *component)
{
	snd_debug("\n");
	/* FIFO CTL register default setting */
	sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
			(CTL_TXTL_MASK << FIFO_CTL_TXTL),
			(CTL_TXTL_DEFAULT << FIFO_CTL_TXTL));
	sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
			(CTL_RXTL_MASK << FIFO_CTL_RXTL),
			(CTL_RXTL_DEFAULT << FIFO_CTL_RXTL));

	sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
			1 << TXCFG_CHAN_STA_EN, 1 << TXCFG_CHAN_STA_EN);

	sunxi_sound_component_write(component, SUNXI_OWA_TXCH_STA0, 0x2 << TXCHSTA0_CHNUM);
	sunxi_sound_component_write(component, SUNXI_OWA_RXCH_STA0, 0x2 << RXCHSTA0_CHNUM);

	sunxi_sound_component_update_bits(component, SUNXI_OWA_CTL, 1 << CTL_GEN_EN, 0 << CTL_GEN_EN);
}

static int sunxi_owa_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				 struct sunxi_sound_pcm_hw_params *params,
				 struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	unsigned int tx_input_mode = 0;
	unsigned int rx_output_mode = 0;
	unsigned int origin_freq_bit = 0, sample_freq_bit = 0;
	unsigned int reg_temp;
	unsigned int i;

	snd_debug("\n");
	switch (params_format(params)) {
	case	SNDRV_PCM_FORMAT_S16_LE:
		reg_temp = 0;
		tx_input_mode = 1;
		rx_output_mode = 3;
		break;
/*
	case	SNDRV_PCM_FORMAT_S20_3LE:
		reg_temp = 1;
		tx_input_mode = 0;
		rx_output_mode = 0;
		break;
*/
	case	SNDRV_PCM_FORMAT_S24_LE:
	/* only for the compatible of tinyalsa */
	case	SNDRV_PCM_FORMAT_S32_LE:
		reg_temp = 2;
		tx_input_mode = 0;
		rx_output_mode = 0;
		break;
	default:
		snd_err("sunxi owa params_format[%d] error!\n", params_format(params));
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_orig); i++) {
		if (params_rate(params) == sample_rate_orig[i].samplerate) {
			origin_freq_bit = sample_rate_orig[i].rate_bit;
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_freq); i++) {
		if (params_rate(params) == sample_rate_freq[i].samplerate) {
			sample_freq_bit = sample_rate_freq[i].rate_bit;
//			sunxi_owa->rate = sample_rate_freq[i].samplerate;
			break;
		}
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
				(0x3 << TXCFG_SAMPLE_BIT),
				(reg_temp<<TXCFG_SAMPLE_BIT));

		sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
				(0x1 << FIFO_CTL_TXIM),
				(tx_input_mode << FIFO_CTL_TXIM));

		if (params_channels(params) == 1) {
			sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
					(0x1 << TXCFG_SINGLE_MOD),
					(0x1 << TXCFG_SINGLE_MOD));
		} else {
			sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
					(0x1 << TXCFG_SINGLE_MOD),
					(0x0 << TXCFG_SINGLE_MOD));
		}

		/* samplerate conversion */
		sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA0,
				(0xF << TXCHSTA0_SAMFREQ),
				(sample_freq_bit << TXCHSTA0_SAMFREQ));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA1,
				(0xF << TXCHSTA1_ORISAMFREQ),
				(origin_freq_bit << TXCHSTA1_ORISAMFREQ));
		switch (reg_temp) {
		case	0:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA1,
					(0xF << TXCHSTA1_MAXWORDLEN),
					(0x2 << TXCHSTA1_MAXWORDLEN));
			break;
		case	1:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA1,
					(0xF << TXCHSTA1_MAXWORDLEN),
					(0xC << TXCHSTA1_MAXWORDLEN));
			break;
		case	2:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCH_STA1,
					(0xF << TXCHSTA1_MAXWORDLEN),
					(0xB << TXCHSTA1_MAXWORDLEN));
			break;
		default:
			snd_err("sunxi owa unexpection error\n");
			return -EINVAL;
		}
	} else {
		/*
		 * FIXME, not sync as spec says, just test 16bit & 24bit,
		 * using 3 working ok
		 */
		sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
				(0x3 << FIFO_CTL_RXOM),
				(rx_output_mode << FIFO_CTL_RXOM));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA0,
				(0xF<<RXCHSTA0_SAMFREQ),
				(sample_freq_bit << RXCHSTA0_SAMFREQ));
		sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA1,
				(0xF<<RXCHSTA1_ORISAMFREQ),
				(origin_freq_bit << RXCHSTA1_ORISAMFREQ));

		switch (reg_temp) {
		case	0:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA1,
					(0xF << RXCHSTA1_MAXWORDLEN),
					(0x2 << RXCHSTA1_MAXWORDLEN));
			break;
		case	1:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA1,
					(0xF << RXCHSTA1_MAXWORDLEN),
					(0xC << RXCHSTA1_MAXWORDLEN));
			break;
		case	2:
			sunxi_sound_component_update_bits(component, SUNXI_OWA_RXCH_STA1,
					(0xF << RXCHSTA1_MAXWORDLEN),
					(0xB << RXCHSTA1_MAXWORDLEN));
			break;
		default:
			snd_err("sunxi owa unexpection error\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int sunxi_owa_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
		unsigned int freq_in, unsigned int freq_out)
{
	int ret;
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_owa_info *sunxi_owa = component->private_data;

	snd_debug("\n");

	ret = snd_sunxi_owa_clk_set_rate(&sunxi_owa->clk, 0, freq_in, freq_out);
	if (ret < 0) {
		snd_err("snd_sunxi_owa_clk_set_rate failed\n");
		return -1;
	}

	return 0;
}

static int sunxi_owa_set_clkdiv(struct sunxi_sound_adf_dai *dai, int clk_id, int clk_div)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");
	/* fs = PLL/[(div+1)*64*2] */
	clk_div = clk_div >> 7;

	sunxi_sound_component_update_bits(component, SUNXI_OWA_TXCFG,
			(0x1F << TXCFG_CLK_DIV_RATIO),
			((clk_div - 1) << TXCFG_CLK_DIV_RATIO));

	return 0;
}

static int sunxi_owa_startup(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_owa_info *sunxi_owa = component->private_data;

	snd_debug("\n");
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &sunxi_owa->playback_dma_param);
	} else {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &sunxi_owa->capture_dma_param);
	}

	return 0;
}

static void sunxi_owa_shutdown(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");
}

static int sunxi_owa_trigger(struct sunxi_sound_pcm_dataflow *dataflow,
		int cmd, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	int ret = 0;

	snd_debug("stream:%u, cmd:%u\n", dataflow->stream, cmd);
	switch (cmd) {
	case	SNDRV_PCM_TRIGGER_START:
	case	SNDRV_PCM_TRIGGER_RESUME:
	case	SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_owa_txctrl_enable(component, true);
		} else {
			sunxi_owa_rxctrl_enable(component, true);
		}
		break;
	case	SNDRV_PCM_TRIGGER_STOP:
	case	SNDRV_PCM_TRIGGER_SUSPEND:
	case	SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_owa_txctrl_enable(component, false);
		} else {
			sunxi_owa_rxctrl_enable(component, false);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*
 * Reset & Flush FIFO
 */
static int sunxi_owa_prepare(struct sunxi_sound_pcm_dataflow *dataflow,
			struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	unsigned int reg_val;

	snd_debug("\n");
	sunxi_sound_component_update_bits(component, SUNXI_OWA_CTL,
			(0x1 << CTL_GEN_EN), (0x0 << CTL_GEN_EN));

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
				(0x1 << FIFO_CTL_FTX), (0x1 << FIFO_CTL_FTX));
		sunxi_sound_component_write(component, SUNXI_OWA_TXCNT, 0x0);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_OWA_FIFO_CTL,
				(0x1 << FIFO_CTL_FRX), (0x1 << FIFO_CTL_FRX));
		sunxi_sound_component_write(component, SUNXI_OWA_RXCNT, 0x0);
	}

	/* clear all interrupt status */
	reg_val = sunxi_sound_component_read(component, SUNXI_OWA_INT_STA);
	sunxi_sound_component_write(component, SUNXI_OWA_INT_STA, reg_val);

	/* need reset */
	sunxi_sound_component_update_bits(component, SUNXI_OWA_CTL,
			(0x1 << CTL_RESET) | (0x1 << CTL_GEN_EN),
			(0x1 << CTL_RESET) | (0x1 << CTL_GEN_EN));
	return 0;
}

/* owa module init status */
static int sunxi_owa_dai_probe(struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_owa_info *sunxi_owa = component->private_data;

	snd_debug("\n");

	/* pcm_new will using the dma_param about the cma and fifo params. */
	sunxi_sound_adf_dai_init_dma_data(dai,
				  &sunxi_owa->playback_dma_param,
				  &sunxi_owa->capture_dma_param);

	sunxi_owa_init(component);

	return 0;
}

static int sunxi_owa_dai_remove(struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");
	return 0;
}

static struct sunxi_sound_adf_dai_ops sunxi_owa_dai_ops = {
	/* call by machine */
	.set_pll = sunxi_owa_dai_set_pll,
	.set_clkdiv = sunxi_owa_set_clkdiv,
	/* call by asoc */
	.hw_params = sunxi_owa_hw_params,
	.startup = sunxi_owa_startup,
	.shutdown = sunxi_owa_shutdown,
	.trigger = sunxi_owa_trigger,
	.prepare = sunxi_owa_prepare,
};

static struct sunxi_sound_adf_dai_driver sunxi_owa_dai = {
	.id		= 1,
	.name		= DRV_NAME,
	.playback = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SUNXI_OWA_RATES,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 192000,
	},
	.capture = {
		.stream_name	= "Capture",
		.channels_min	= 2,
		.channels_max	= 2,
		.rates		= SUNXI_OWA_RATES,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 192000,
	},
	.probe = sunxi_owa_dai_probe,
	.remove = sunxi_owa_dai_remove,
	.ops = &sunxi_owa_dai_ops,
};

#ifdef CONFIG_DRIVER_SYSCONFIG
static int sunxi_sound_parse_owa_dma_params(const char* prefix, const char* type_name,
			struct sunxi_owa_info *info)
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
static struct sunxi_owa_info default_owa_param = {
	.playback_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
	.capture_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
};
#endif


static void sunxi_owa_params_init(struct sunxi_owa_info *info)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_owa_dma_params("owa", NULL ,info);
#else
	*info = default_owa_param;
#endif
}

static void sunxi_owa_gpio_init(bool enable)
{
	snd_debug("\n");
	if (enable) {
		/* CLK */
//		hal_gpio_pinmux_set_function(g_owa_gpio.clk.gpio,
//					g_owa_gpio.clk.mux);
		/* OUT */
		hal_gpio_pinmux_set_function(g_owa_gpio.out.gpio,
					g_owa_gpio.out.mux);
		/* IN */
		hal_gpio_pinmux_set_function(g_owa_gpio.in.gpio,
					g_owa_gpio.in.mux);
	} else {
		/* CLK */
//		hal_gpio_pinmux_set_function(g_owa_gpio.clk.gpio,
//					GPIO_MUXSEL_DISABLED);
		/* OUT */
		hal_gpio_pinmux_set_function(g_owa_gpio.out.gpio,
					GPIO_MUXSEL_DISABLED);
		/* IN */
		hal_gpio_pinmux_set_function(g_owa_gpio.in.gpio,
					GPIO_MUXSEL_DISABLED);
	}
}

/* suspend and resume */
#ifdef CONFIG_COMPONENTS_PM
static unsigned int snd_read_func(void *data, unsigned int reg)
{
	struct sunxi_sound_adf_component *component;

	if (!data) {
		snd_err("data is invailed\n");
		return 0;
	}

	component = data;
	return sunxi_sound_component_read(component, reg);
}

static void snd_write_func(void *data, unsigned int reg, unsigned int val)
{
	struct sunxi_sound_adf_component *component;

	if (!data) {
		snd_err("data is invailed\n");
		return;
	}

	component = data;
	sunxi_sound_component_write(component, reg, val);
}

static int sunxi_owa_suspend(struct sunxi_sound_adf_component *component)
{
	struct sunxi_owa_info *sunxi_owa = component->private_data;

	snd_debug("\n");

	sunxi_sound_save_reg(sunxi_reg_labels, (void *)component, snd_read_func);
	snd_sunxi_owa_clk_disable(&sunxi_owa->clk);

	return 0;
}

static int sunxi_owa_resume(struct sunxi_sound_adf_component *component)
{
	struct sunxi_owa_info *sunxi_owa = component->private_data;

	snd_debug("\n");

	snd_sunxi_owa_clk_enable(&sunxi_owa->clk);
	sunxi_owa_init(component);
	sunxi_sound_echo_reg(sunxi_reg_labels, (void *)component, snd_write_func);

	return 0;
}

#else

static int sunxi_owa_suspend(struct sunxi_sound_adf_component *component)
{
	return 0;
}

static int sunxi_owa_resume(struct sunxi_sound_adf_component *component)
{
	return 0;
}

#endif


/* owa probe */
static int sunxi_owa_probe(struct sunxi_sound_adf_component *component)
{
	int ret;
	struct sunxi_owa_info *sunxi_owa;

	snd_debug("\n");
	sunxi_owa = sound_malloc(sizeof(struct sunxi_owa_info));
	if (!sunxi_owa) {
		snd_err("no memory\n");
		return -ENOMEM;
	}
	component->private_data = (void *)sunxi_owa;

	/* mem base */
	component->addr_base = (void *)SUNXI_OWA_MEMBASE;

	/* clk */
	ret = snd_sunxi_owa_clk_init(&sunxi_owa->clk);
	if (ret != 0) {
		snd_err("snd_sunxi_owa_clk_init failed\n");
		goto err_owa_set_clock;
	}

	/* pinctrl */
	sunxi_owa_gpio_init(true);

	sunxi_owa_params_init(sunxi_owa);

	/* dma config */
	sunxi_owa->playback_dma_param.dst_maxburst = 8;
	sunxi_owa->playback_dma_param.src_maxburst = 8;
	sunxi_owa->playback_dma_param.dma_addr =
			(dma_addr_t)component->addr_base + SUNXI_OWA_TXFIFO;
	sunxi_owa->playback_dma_param.dma_drq_type_num = DRQDST_SPDIF;

	sunxi_owa->capture_dma_param.src_maxburst = 8;
	sunxi_owa->capture_dma_param.dst_maxburst = 8;
	sunxi_owa->capture_dma_param.dma_addr =
			(dma_addr_t)component->addr_base + SUNXI_OWA_RXFIFO;
	sunxi_owa->capture_dma_param.dma_drq_type_num = DRQSRC_SPDIF;

	return 0;

err_owa_set_clock:
	snd_sunxi_owa_clk_exit(&sunxi_owa->clk);

	return -1;
}

static void sunxi_owa_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_owa_info *sunxi_owa;

	snd_debug("\n");
	sunxi_owa = component->private_data;
	if (!sunxi_owa)
		return;

	snd_sunxi_owa_clk_exit(&sunxi_owa->clk);

	sound_free(sunxi_owa);
	component->private_data = NULL;
	return;
}

static struct sunxi_sound_adf_component_driver sunxi_owa_component_dev = {
	.name		= COMPONENT_DRV_NAME,
	.probe		= sunxi_owa_probe,
	.remove		= sunxi_owa_remove,
	.suspend 	= sunxi_owa_suspend,
	.resume 	= sunxi_owa_resume,
	.controls       = sunxi_owa_controls,
	.num_controls   = ARRAY_SIZE(sunxi_owa_controls),
};

int sunxi_owa_component_probe()
{
	int ret;
	char name[48];
	snd_debug("\n");

	ret= sunxi_sound_adf_register_component(&sunxi_owa_component_dev, &sunxi_owa_dai, 1);
	if (ret != 0) {
		snd_err("sunxi_sound_adf_register_component failed");
		return ret;
	}

	snprintf(name, sizeof(name), "%s-dma", sunxi_owa_component_dev.name);
	ret = sunxi_pcm_dma_platform_register(name);
	if (ret != 0) {
		snd_err("sunxi_pcm_dma_platform_register failed");
		return ret;
	}
	return 0;
}

void sunxi_owa_component_remove()
{
	char name[48];

	snd_debug("\n");
	sunxi_sound_adf_unregister_component(&sunxi_owa_component_dev);
	snprintf(name, sizeof(name), "%s-dma", sunxi_owa_component_dev.name);
	sunxi_pcm_dma_platform_unregister(name);
}


#ifdef SUNXI_OWA_DEBUG_REG
/* for debug */
#include <console.h>
int cmd_owa_dump(void)
{
	int owa_num = 0;
	void *membase;
	int i = 0;

	membase = (void *)SUNXI_OWA_MEMBASE;

	while (sunxi_reg_labels[i].name != NULL) {
		printf("%-20s[0x%03x]: 0x%-10x\n",
			sunxi_reg_labels[i].name,
			sunxi_reg_labels[i].address,
			snd_readl(membase + sunxi_reg_labels[i].address));
		i++;
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_owa_dump, owa, owa dump reg);
#endif /* SUNXI_OWA_DEBUG_REG */
