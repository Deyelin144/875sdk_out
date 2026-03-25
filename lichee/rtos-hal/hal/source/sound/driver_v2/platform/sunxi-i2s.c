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
#include <hal_timer.h>
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif
#include <sound_v2/sunxi_sound_io.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include "sunxi_sound_common.h"
#include "sunxi_sound_rxsync.h"

#ifdef CONFIG_DRIVER_SYSCONFIG
#include <hal_cfg.h>
#include <script.h>
#endif

#include "sunxi-pcm.h"
#include "sunxi-i2s.h"

#define COMPONENT_DRV0_NAME	"sunxi-snd-plat-i2s0"
#define COMPONENT_DRV1_NAME	"sunxi-snd-plat-i2s1"
#define COMPONENT_DRV2_NAME	"sunxi-snd-plat-i2s2"
#define COMPONENT_DRV3_NAME	"sunxi-snd-plat-i2s3"

#define DRV_NAME			"sunxi-snd-plat-i2s-dai"

struct sunxi_i2s_info {
	struct sunxi_i2s_clk clk;
	struct i2s_pinctrl *pinctrl;
	uint8_t pinctrl_num;
	struct pa_config *pa_cfg;
	uint8_t pa_cfg_num;

	struct sunxi_i2s_param param;
	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;
	struct sunxi_i2s_dai_fmt i2s_dai_fmt;

	int asrc_en;
};

#if defined(SUNXI_I2S_DEBUG_REG) || defined(CONFIG_COMPONENTS_PM)
static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_I2S_CTL),
	REG_LABEL(SUNXI_I2S_FMT0),
	REG_LABEL(SUNXI_I2S_FMT1),
	REG_LABEL(SUNXI_I2S_INTSTA),
	/* REG_LABEL(SUNXI_I2S_RXFIFO), */
	REG_LABEL(SUNXI_I2S_FIFOCTL),
	REG_LABEL(SUNXI_I2S_FIFOSTA),
	REG_LABEL(SUNXI_I2S_INTCTL),
	/* REG_LABEL(SUNXI_I2S_TXFIFO), */
	REG_LABEL(SUNXI_I2S_CLKDIV),
	REG_LABEL(SUNXI_I2S_TXCNT),
	REG_LABEL(SUNXI_I2S_RXCNT),
	REG_LABEL(SUNXI_I2S_CHCFG),
	REG_LABEL(SUNXI_I2S_TX0CHSEL),
	REG_LABEL(SUNXI_I2S_TX1CHSEL),
	REG_LABEL(SUNXI_I2S_TX2CHSEL),
	REG_LABEL(SUNXI_I2S_TX3CHSEL),
	REG_LABEL(SUNXI_I2S_TX0CHMAP0),
	REG_LABEL(SUNXI_I2S_TX0CHMAP1),
	REG_LABEL(SUNXI_I2S_TX1CHMAP0),
	REG_LABEL(SUNXI_I2S_TX1CHMAP1),
	REG_LABEL(SUNXI_I2S_TX2CHMAP0),
	REG_LABEL(SUNXI_I2S_TX2CHMAP1),
	REG_LABEL(SUNXI_I2S_TX3CHMAP0),
	REG_LABEL(SUNXI_I2S_TX3CHMAP1),
	REG_LABEL(SUNXI_I2S_RXCHSEL),
	REG_LABEL(SUNXI_I2S_RXCHMAP0),
	REG_LABEL(SUNXI_I2S_RXCHMAP1),
	REG_LABEL(SUNXI_I2S_RXCHMAP2),
	REG_LABEL(SUNXI_I2S_RXCHMAP3),
	REG_LABEL(SUNXI_I2S_DEBUG),
	REG_LABEL_END,
};
#endif
static int snd_sunxi_pa_pin_init(struct sunxi_sound_adf_component *component);
/*static void snd_sunxi_pa_pin_exit(struct snd_platform *platform);*/
static int snd_sunxi_pa_pin_enable(struct sunxi_sound_adf_component *component);
static void snd_sunxi_pa_pin_disable(struct sunxi_sound_adf_component *component);

static void sunxi_sdout_enable(struct sunxi_sound_adf_component *component, bool *tx_pin)
{
	/* tx_pin[x] -- x < 4 */
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 0),
			   tx_pin[0] << (SDO0_EN + 0));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 1),
			   tx_pin[1] << (SDO0_EN + 1));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 2),
			   tx_pin[2] << (SDO0_EN + 2));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 3),
			   tx_pin[3] << (SDO0_EN + 3));
}

static void sunxi_sdout_disable(struct sunxi_sound_adf_component *component)
{
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 0), 0 << (SDO0_EN + 0));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 1), 0 << (SDO0_EN + 0));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 2), 0 << (SDO0_EN + 0));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << (SDO0_EN + 3), 0 << (SDO0_EN + 0));
}


static void sunxi_i2s_dai_tx_route(struct sunxi_sound_adf_component *component, bool enable)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_param *param = &sunxi_i2s->param;

	snd_debug("\n");
	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_INTCTL,
					(1 << TXDRQEN), (1 << TXDRQEN));
		sunxi_sdout_enable(component, param->tx_pin);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					1 << CTL_TXEN, 1 << CTL_TXEN);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_INTCTL,
					(1 << TXDRQEN), (0 << TXDRQEN));
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					(1 << CTL_TXEN), (0 << CTL_TXEN));
		sunxi_sdout_disable(component);
	}
}

static void sunxi_i2s_dai_rx_route(struct sunxi_sound_adf_component *component, bool enable)
{
	snd_debug("\n");
	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
				(1 << CTL_RXEN), (1 << CTL_RXEN));
		sunxi_sound_component_update_bits(component, SUNXI_I2S_INTCTL,
				(1 << RXDRQEN), (1 << RXDRQEN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_INTCTL,
				(1 << RXDRQEN), (0 << RXDRQEN));
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
				(1 << CTL_RXEN), (0 << CTL_RXEN));
	}
}

static int sunxi_get_i2s_dai_fmt(struct sunxi_i2s_dai_fmt *i2s_dai_fmt,
				 enum SUNXI_I2S_DAI_FMT_SEL dai_fmt_sel,
				 unsigned int *val)
{
	switch (dai_fmt_sel) {
	case SUNXI_I2S_DAI_PLL:
		*val = i2s_dai_fmt->pllclk_freq;
		break;
	case SUNXI_I2S_DAI_MCLK:
		*val = i2s_dai_fmt->moduleclk_freq;
		break;
	case SUNXI_I2S_DAI_FMT:
		*val = i2s_dai_fmt->fmt & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	case SUNXI_I2S_DAI_MASTER:
		*val = i2s_dai_fmt->fmt & SND_SOC_DAIFMT_MASTER_MASK;
		break;
	case SUNXI_I2S_DAI_INVERT:
		*val = i2s_dai_fmt->fmt & SND_SOC_DAIFMT_INV_MASK;
		break;
	case SUNXI_I2S_DAI_SLOT_NUM:
		*val = i2s_dai_fmt->slots;
		break;
	case SUNXI_I2S_DAI_SLOT_WIDTH:
		*val = i2s_dai_fmt->slot_width;
		break;
	default:
		snd_err("unsupport dai fmt sel %d\n", dai_fmt_sel);
		return -EINVAL;
	}

	return 0;
}

static int sunxi_set_i2s_dai_fmt(struct sunxi_i2s_dai_fmt *i2s_dai_fmt,
				 enum SUNXI_I2S_DAI_FMT_SEL dai_fmt_sel,
				 unsigned int val)
{
	switch (dai_fmt_sel) {
	case SUNXI_I2S_DAI_PLL:
		i2s_dai_fmt->pllclk_freq = val;
		break;
	case SUNXI_I2S_DAI_MCLK:
		i2s_dai_fmt->moduleclk_freq = val;
		break;
	case SUNXI_I2S_DAI_FMT:
		i2s_dai_fmt->fmt &= ~SND_SOC_DAIFMT_FORMAT_MASK;
		i2s_dai_fmt->fmt |= SND_SOC_DAIFMT_FORMAT_MASK & val;
		break;
	case SUNXI_I2S_DAI_MASTER:
		i2s_dai_fmt->fmt &= ~SND_SOC_DAIFMT_MASTER_MASK;
		i2s_dai_fmt->fmt |= SND_SOC_DAIFMT_MASTER_MASK & val;
		break;
	case SUNXI_I2S_DAI_INVERT:
		i2s_dai_fmt->fmt &= ~SND_SOC_DAIFMT_INV_MASK;
		i2s_dai_fmt->fmt |= SND_SOC_DAIFMT_INV_MASK & val;
		break;
	case SUNXI_I2S_DAI_SLOT_NUM:
		i2s_dai_fmt->slots = val;
		break;
	case SUNXI_I2S_DAI_SLOT_WIDTH:
		i2s_dai_fmt->slot_width = val;
		break;
	default:
		snd_err("unsupport dai fmt sel %d\n", dai_fmt_sel);
		return -EINVAL;
	}

	return 0;
}

static void sunxi_rx_sync_enable(void *data, bool enable)
{
	struct sunxi_sound_adf_component *component = data;

	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					 1 << RX_SYNC_EN_STA, 1 << RX_SYNC_EN_STA);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					 1 << RX_SYNC_EN_STA, 0 << RX_SYNC_EN_STA);
	}

}

static int sunxi_i2s_dai_startup(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &sunxi_i2s->playback_dma_param);
	} else {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &sunxi_i2s->capture_dma_param);
	}

	snd_sunxi_pa_pin_enable(component);

	return 0;
}

static void sunxi_i2s_dai_shutdown(struct sunxi_sound_pcm_dataflow *dataflow,
				struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");
	snd_sunxi_pa_pin_disable(component);
}

static int sunxi_i2s_dai_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				 struct sunxi_sound_pcm_hw_params *params,
				 struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");

	switch (params_format(params)) {
	case SND_PCM_FORMAT_S16_LE:
		sunxi_sound_component_update_bits(component,
			SUNXI_I2S_FMT0,
			(SUNXI_I2S_SR_MASK<<I2S_SAMPLE_RESOLUTION),
			(SUNXI_I2S_SR_16BIT<<I2S_SAMPLE_RESOLUTION));
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sunxi_sound_component_update_bits(component,
				SUNXI_I2S_FIFOCTL,
				(SUNXI_I2S_TXIM_MASK<<TXIM),
				(SUNXI_I2S_TXIM_VALID_LSB<<TXIM));
		else
			sunxi_sound_component_update_bits(component,
				SUNXI_I2S_FIFOCTL,
				(SUNXI_I2S_RXOM_MASK<<RXOM),
				(SUNXI_I2S_RXOM_EXPH<<RXOM));
		break;
	case SND_PCM_FORMAT_S24_LE:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
				(SUNXI_I2S_SR_MASK<<I2S_SAMPLE_RESOLUTION),
				(SUNXI_I2S_SR_24BIT<<I2S_SAMPLE_RESOLUTION));
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sunxi_sound_component_update_bits(component,
					SUNXI_I2S_FIFOCTL,
					(SUNXI_I2S_TXIM_MASK<<TXIM),
					(SUNXI_I2S_TXIM_VALID_LSB<<TXIM));
		else
			sunxi_sound_component_update_bits(component,
					SUNXI_I2S_FIFOCTL,
					(SUNXI_I2S_RXOM_MASK<<RXOM),
					(SUNXI_I2S_RXOM_EXP0<<RXOM));
		break;
	case SND_PCM_FORMAT_S32_LE:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
				(SUNXI_I2S_SR_MASK<<I2S_SAMPLE_RESOLUTION),
				(SUNXI_I2S_SR_32BIT<<I2S_SAMPLE_RESOLUTION));
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sunxi_sound_component_update_bits(component,
					SUNXI_I2S_FIFOCTL,
					(SUNXI_I2S_TXIM_MASK<<TXIM),
					(SUNXI_I2S_TXIM_VALID_LSB<<TXIM));
		else
			sunxi_sound_component_update_bits(component,
					SUNXI_I2S_FIFOCTL,
					(SUNXI_I2S_RXOM_MASK<<RXOM),
					(SUNXI_I2S_RXOM_EXPH<<RXOM));
		break;
	default:
		snd_err("unrecognized format\n");
		return -EINVAL;
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CHCFG,
				(SUNXI_I2S_TX_SLOT_MASK<<TX_SLOT_NUM),
				((params_channels(params)-1)<<TX_SLOT_NUM));
		sunxi_sound_component_write(component,
			SUNXI_I2S_TX0CHMAP0, SUNXI_DEFAULT_CHMAP0);
		sunxi_sound_component_write(component,
			SUNXI_I2S_TX0CHMAP1, SUNXI_DEFAULT_CHMAP1);
		sunxi_sound_component_update_bits(component,
			SUNXI_I2S_TX0CHSEL,
			(SUNXI_I2S_TX_CHSEL_MASK<<TX_CHSEL),
			((params_channels(params)-1)<<TX_CHSEL));
		sunxi_sound_component_update_bits(component,
			SUNXI_I2S_TX0CHSEL,
			(SUNXI_I2S_TX_CHEN_MASK<<TX_CHEN),
			((1<<params_channels(params))-1)<<TX_CHEN);
	} else {
		unsigned int SUNXI_I2S_RXCHMAPX = 0;
		int index = 0;

		for (index = 0; index < 16; index++) {
			if (index >= 12)
				SUNXI_I2S_RXCHMAPX = SUNXI_I2S_RXCHMAP0;
			else if (index >= 8)
				SUNXI_I2S_RXCHMAPX = SUNXI_I2S_RXCHMAP1;
			else if (index >= 4)
				SUNXI_I2S_RXCHMAPX = SUNXI_I2S_RXCHMAP2;
			else
				SUNXI_I2S_RXCHMAPX = SUNXI_I2S_RXCHMAP3;
			sunxi_sound_component_update_bits(component,
				SUNXI_I2S_RXCHMAPX,
				I2S_RXCHMAP(index),
				I2S_RXCH_DEF_MAP(index));
		}
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CHCFG,
				(SUNXI_I2S_RX_SLOT_MASK<<RX_SLOT_NUM),
				((params_channels(params)-1)<<RX_SLOT_NUM));
		sunxi_sound_component_update_bits(component, SUNXI_I2S_RXCHSEL,
				(SUNXI_I2S_RX_CHSEL_MASK<<RX_CHSEL),
				((params_channels(params)-1)<<RX_CHSEL));
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (sunxi_i2s->asrc_en) {
			snd_debug("sunxi i2s asrc enabled\n");
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_MCLKCFG,
						(0x1 << I2S_ASRC_MCLK_GATE),
						(0x1 << I2S_ASRC_MCLK_GATE));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_MCLKCFG,
						(0xF << I2S_ASRC_MCLK_RATIO),
						(0x1 << I2S_ASRC_MCLK_RATIO));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSOUTCFG,
						(0x1 << I2S_ASRC_FSOUT_GATE),
						(0x1 << I2S_ASRC_FSOUT_GATE));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSOUTCFG,
						(0xF << I2S_ASRC_FSOUT_CLKSRC),
						(0x0 << I2S_ASRC_FSOUT_CLKSRC));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSOUTCFG,
						(0xF << I2S_ASRC_FSOUT_CLKDIV1),
						(0xF << I2S_ASRC_FSOUT_CLKDIV1));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSOUTCFG,
						(0xF << I2S_ASRC_FSOUT_CLKDIV2),
						(0xA << I2S_ASRC_FSOUT_CLKDIV2));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSIN_EXTCFG,
						(0x1 << I2S_ASRC_FSIN_EXTEN),
						(0x1 << I2S_ASRC_FSIN_EXTEN));
			sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_FSIN_EXTCFG,
						(0xFF << I2S_ASRC_FSIN_EXTCYCLE),
						(0xA << I2S_ASRC_FSIN_EXTCYCLE));
			/* regmap_write(platform, SUNXI_I2S_ASRC_MANCFG, 0); */
		}
	}

	return 0;
}

static int sunxi_i2s_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
				 unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_dai_fmt *i2s_dai_fmt = &sunxi_i2s->i2s_dai_fmt;
	struct sunxi_i2s_clk *clk = &sunxi_i2s->clk;
	int ret;

	snd_debug("\n");

	if (snd_sunxi_i2s_clk_set_rate(clk, 0, freq_in, freq_out)) {
		snd_err("clk set rate failed\n");
		return -EINVAL;
	}

	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_PLL, freq_in);
	if (ret < 0)
		return -EINVAL;
	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_MCLK, freq_out);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static int sunxi_i2s_dai_set_sysclk(struct sunxi_sound_adf_dai *dai,
				int clk_id, unsigned int freq, int dir)
{
	int ret;
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_dai_fmt *i2s_dai_fmt = &sunxi_i2s->i2s_dai_fmt;
	unsigned int pllclk_freq, mclk_ratio, mclk_ratio_map;

	snd_debug("\n");

	if (freq == 0) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CLKDIV, 1 << MCLKOUT_EN, 0 << MCLKOUT_EN);
		return 0;
	}

	ret = sunxi_get_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_PLL, &pllclk_freq);
	if (ret < 0)
		return -EINVAL;

	if (pllclk_freq == 0) {
		snd_err("pllclk freq is invalid\n");
		return -ENOMEM;
	}
	mclk_ratio = pllclk_freq / freq;

	switch (mclk_ratio) {
	case 1:
		mclk_ratio_map = 1;
		break;
	case 2:
		mclk_ratio_map = 2;
		break;
	case 4:
		mclk_ratio_map = 3;
		break;
	case 6:
		mclk_ratio_map = 4;
		break;
	case 8:
		mclk_ratio_map = 5;
		break;
	case 12:
		mclk_ratio_map = 6;
		break;
	case 16:
		mclk_ratio_map = 7;
		break;
	case 24:
		mclk_ratio_map = 8;
		break;
	case 32:
		mclk_ratio_map = 9;
		break;
	case 48:
		mclk_ratio_map = 10;
		break;
	case 64:
		mclk_ratio_map = 11;
		break;
	case 96:
		mclk_ratio_map = 12;
		break;
	case 128:
		mclk_ratio_map = 13;
		break;
	case 176:
		mclk_ratio_map = 14;
		break;
	case 192:
		mclk_ratio_map = 15;
		break;
	default:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CLKDIV, 1 << MCLKOUT_EN, 0 << MCLKOUT_EN);
		snd_err("mclk freq div unsupport\n");
		return -EINVAL;
	}

	sunxi_sound_component_update_bits(component, SUNXI_I2S_CLKDIV,
							0xf << MCLK_DIV, mclk_ratio_map << MCLK_DIV);
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CLKDIV, 1 << MCLKOUT_EN, 1 << MCLKOUT_EN);

	return 0;
}

static int sunxi_i2s_dai_set_bclk_ratio(struct sunxi_sound_adf_dai *dai, unsigned int ratio)
{
	struct sunxi_sound_adf_component *component = dai->component;
	unsigned int bclk_ratio;

	snd_debug("\n");

	switch (ratio) {
	case	1:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_1;
		break;
	case	2:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_2;
		break;
	case	4:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_3;
		break;
	case	6:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_4;
		break;
	case	8:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_5;
		break;
	case	12:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_6;
		break;
	case	16:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_7;
		break;
	case	24:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_8;
		break;
	case	32:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_9;
		break;
	case	48:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_10;
		break;
	case	64:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_11;
		break;
	case	96:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_12;
		break;
	case	128:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_13;
		break;
	case	176:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_14;
		break;
	case	192:
		bclk_ratio = SUNXI_I2S_BCLK_DIV_15;
		break;
	default:
		snd_err("unsupport clk_div\n");
		return -EINVAL;
	}
	/* setting bclk to driver external codec bit clk */
	sunxi_sound_component_update_bits(component, SUNXI_I2S_CLKDIV,
			(SUNXI_I2S_BCLK_DIV_MASK<<BCLK_DIV),
			(bclk_ratio<<BCLK_DIV));

	return 0;
}

static int sunxi_i2s_dai_init_fmt(struct sunxi_sound_adf_component *component, unsigned int fmt)
{
	unsigned int offset, mode;
	unsigned int lrck_polarity, brck_polarity;
	unsigned int dai_mode;

	snd_debug("\n");
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
				(SUNXI_I2S_LRCK_OUT_MASK<<LRCK_OUT),
				(SUNXI_I2S_LRCK_OUT_DISABLE<<LRCK_OUT));
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << BCLK_OUT, 1 << BCLK_OUT);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << LRCK_OUT, 0 << LRCK_OUT);
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << BCLK_OUT, 0 << BCLK_OUT);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 << LRCK_OUT, 1 << LRCK_OUT);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
				(SUNXI_I2S_LRCK_OUT_MASK<<LRCK_OUT),
				(SUNXI_I2S_LRCK_OUT_ENABLE<<LRCK_OUT));
		break;
	default:
		snd_err("unknown maser/slave format\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case	SND_SOC_DAIFMT_I2S:
		offset = SUNXI_I2S_TX_OFFSET_1;
		mode = SUNXI_I2S_MODE_CTL_I2S;
		break;
	case	SND_SOC_DAIFMT_RIGHT_J:
		offset = SUNXI_I2S_TX_OFFSET_0;
		mode = SUNXI_I2S_MODE_CTL_RIGHT;
		break;
	case	SND_SOC_DAIFMT_LEFT_J:
		offset = SUNXI_I2S_TX_OFFSET_0;
		mode = SUNXI_I2S_MODE_CTL_LEFT;
		break;
	case	SND_SOC_DAIFMT_DSP_A:
		offset = SUNXI_I2S_TX_OFFSET_1;
		mode = SUNXI_I2S_MODE_CTL_PCM;
		/* L data MSB after FRM LRC (short frame) */
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0, 1 << LRCK_WIDTH, 0 << LRCK_WIDTH);
		break;
	case	SND_SOC_DAIFMT_DSP_B:
		offset = SUNXI_I2S_TX_OFFSET_0;
		mode = SUNXI_I2S_MODE_CTL_PCM;
		/* L data MSB after FRM LRC (long frame) */
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0, 1 << LRCK_WIDTH, 1 << LRCK_WIDTH);
		break;
	default:
		snd_err("format setting failed\n");
		return -EINVAL;
	}

	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
			(SUNXI_I2S_MODE_CTL_MASK<<MODE_SEL),
			(mode<<MODE_SEL));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_TX0CHSEL,
			(SUNXI_I2S_TX_OFFSET_MASK<<TX_OFFSET),
			(offset<<TX_OFFSET));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_TX1CHSEL,
			(SUNXI_I2S_TX_OFFSET_MASK<<TX_OFFSET),
			(offset<<TX_OFFSET));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_TX2CHSEL,
			(SUNXI_I2S_TX_OFFSET_MASK<<TX_OFFSET),
			(offset<<TX_OFFSET));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_TX3CHSEL,
			(SUNXI_I2S_TX_OFFSET_MASK<<TX_OFFSET),
			(offset<<TX_OFFSET));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_RXCHSEL,
			(SUNXI_I2S_RX_OFFSET_MASK<<RX_OFFSET),
			(offset<<RX_OFFSET));


	/* get dai mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		dai_mode = 0;
		break;
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		dai_mode = 1;
		break;
	default:
		snd_err("dai_mode setting failed\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		lrck_polarity = SUNXI_I2S_LRCK_POLARITY_NOR;
		brck_polarity = SUNXI_I2S_BCLK_POLARITY_NOR;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		lrck_polarity = SUNXI_I2S_LRCK_POLARITY_INV;
		brck_polarity = SUNXI_I2S_BCLK_POLARITY_NOR;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		lrck_polarity = SUNXI_I2S_LRCK_POLARITY_NOR;
		brck_polarity = SUNXI_I2S_BCLK_POLARITY_INV;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		lrck_polarity = SUNXI_I2S_LRCK_POLARITY_INV;
		brck_polarity = SUNXI_I2S_BCLK_POLARITY_INV;
		break;
	default:
		snd_err("invert clk setting failed\n");
		return -EINVAL;
	}

	/* lrck polarity of i2s format
	 * LRCK_POLARITY	-> 0
	 * Left channel when LRCK is low(I2S/RIGHT_J/LEFT_J);
	 * PCM LRCK asserted at the negative edge(DSP_A/DSP_B);
	 * LRCK_POLARITY	-> 1
	 * Left channel when LRCK is high(I2S/RIGHT_J/LEFT_J);
	 * PCM LRCK asserted at the positive edge(DSP_A/DSP_B);
	 */
	lrck_polarity ^= dai_mode;

	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
			(1<<LRCK_POLARITY), (lrck_polarity<<LRCK_POLARITY));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
			(1<<BRCK_POLARITY), (brck_polarity<<BRCK_POLARITY));

	return 0;
}

static int sunxi_i2s_dai_set_fmt(struct sunxi_sound_adf_dai *dai, unsigned int fmt)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_dai_fmt *i2s_dai_fmt = &sunxi_i2s->i2s_dai_fmt;
	int ret;

	snd_debug("\n");

	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_FMT, fmt);
	if (ret < 0)
		return -EINVAL;
	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_MASTER, fmt);
	if (ret < 0)
		return -EINVAL;
	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_INVERT, fmt);
	if (ret < 0)
		return -EINVAL;

	ret = sunxi_i2s_dai_init_fmt(component, fmt);
	if (ret < 0)
		return ret;

	return 0;
}

static int sunxi_i2s_dai_set_tdm_slot(struct sunxi_sound_adf_dai *dai,
				      unsigned int tx_mask, unsigned int rx_mask,
				      int slots, int slot_width)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_dai_fmt *i2s_dai_fmt = &sunxi_i2s->i2s_dai_fmt;
	unsigned int slot_width_map, lrck_width_map;
	unsigned int dai_fmt_get;
	int ret;

	snd_debug("\n");

	switch (slot_width) {
	case 8:
		slot_width_map = 1;
		break;
	case 12:
		slot_width_map = 2;
		break;
	case 16:
		slot_width_map = 3;
		break;
	case 20:
		slot_width_map = 4;
		break;
	case 24:
		slot_width_map = 5;
		break;
	case 28:
		slot_width_map = 6;
		break;
	case 32:
		slot_width_map = 7;
		break;
	default:
		snd_err("unknown slot width\n");
		return -EINVAL;
	}
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
			   7 << SLOT_WIDTH, slot_width_map << SLOT_WIDTH);

	/* bclk num of per channel
	 * I2S/RIGHT_J/LEFT_J	-> lrck long total is lrck_width_map * 2
	 * DSP_A/DSP_B		-> lrck long total is lrck_width_map * 1
	 */
	ret = sunxi_get_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_FMT, &dai_fmt_get);
	if (ret < 0)
		return -EINVAL;
	switch (dai_fmt_get) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		lrck_width_map = (slots / 2) * slot_width - 1;
		break;
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		lrck_width_map = slots * slot_width - 1;
		break;
	default:
		snd_err("unsupoort format\n");
		return -EINVAL;
	}
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT0,
			   0x3ff << LRCK_PERIOD, lrck_width_map << LRCK_PERIOD);

	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_SLOT_NUM, slots);
	if (ret < 0)
		return -EINVAL;
	ret = sunxi_set_i2s_dai_fmt(i2s_dai_fmt, SUNXI_I2S_DAI_SLOT_WIDTH, slot_width);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static int sunxi_i2s_dai_prepare(struct sunxi_sound_pcm_dataflow *dataflow,
			struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	unsigned int i;

	snd_debug("\n");
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		for (i = 0 ; i < SUNXI_I2S_FTX_TIMES ; i++) {
			sunxi_sound_component_update_bits(component,
				SUNXI_I2S_FIFOCTL,
				(1 << FIFO_CTL_FTX), (1 << FIFO_CTL_FTX));
			hal_usleep(1000);
		}
		sunxi_sound_component_write(component, SUNXI_I2S_TXCNT, 0);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FIFOCTL,
				(1<<FIFO_CTL_FRX), (1<<FIFO_CTL_FRX));
		sunxi_sound_component_write(component, SUNXI_I2S_RXCNT, 0);
	}

	return 0;
}

static int sunxi_i2s_dai_trigger(struct sunxi_sound_pcm_dataflow *dataflow,
		int cmd, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_param *param = &sunxi_i2s->param;

	snd_debug("\n");
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_i2s_dai_tx_route(component, true);
		} else {
			sunxi_i2s_dai_rx_route(component, true);
			if (param->rx_sync_en && param->rx_sync_ctl) {
				sunxi_sound_rx_sync_control(sunxi_i2s->param.rx_sync_domain,
						      sunxi_i2s->param.rx_sync_id, true);
			}
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_i2s_dai_tx_route(component, false);
		} else {
			sunxi_i2s_dai_rx_route(component, false);
			if (param->rx_sync_en && param->rx_sync_ctl) {
				sunxi_sound_rx_sync_control(sunxi_i2s->param.rx_sync_domain,
						      sunxi_i2s->param.rx_sync_id, false);
			}
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct sunxi_sound_adf_dai_ops sunxi_i2s_dai_ops = {
	/* call by machine */
	.set_pll	= sunxi_i2s_dai_set_pll,		/* set pllclk */
	.set_sysclk = sunxi_i2s_dai_set_sysclk,		/* set mclk */
	.set_bclk_ratio = sunxi_i2s_dai_set_bclk_ratio,		/* set bclk freq */
	.set_fmt = sunxi_i2s_dai_set_fmt,		/* set tdm fmt */
	.set_tdm_slot	= sunxi_i2s_dai_set_tdm_slot,	/* set slot num and width */
	/* call by asoc */
	.startup = sunxi_i2s_dai_startup,
	.hw_params = sunxi_i2s_dai_hw_params,
	.prepare = sunxi_i2s_dai_prepare,
	.trigger = sunxi_i2s_dai_trigger,
	.shutdown = sunxi_i2s_dai_shutdown,
};

static int sunxi_i2s_init(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_param *param = &sunxi_i2s->param;

	snd_debug("\n");

	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT1,
				(0x1 << TX_MLS),
				(param->msb_lsb_first << TX_MLS));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT1,
				(0x1 << RX_MLS),
				(param->msb_lsb_first << RX_MLS));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT1,
				(0x3 << SEXT),
				(param->sign_extend << SEXT));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT1,
				(0x3 << TX_PDM),
				(param->tx_data_mode << TX_PDM));
	sunxi_sound_component_update_bits(component, SUNXI_I2S_FMT1,
				(0x3 << RX_PDM),
				(param->rx_data_mode << RX_PDM));

	if (sunxi_i2s->param.rx_sync_en)
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					 1 <<  RX_SYNC_EN, 0 << RX_SYNC_EN);

	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					(1 << GLOBAL_EN), (1 << GLOBAL_EN));

	return 0;

}

static int sunxi_i2s_dai_probe(struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");
	/* pcm_new will using the dma_param about the cma and fifo params. */
	sunxi_sound_adf_dai_init_dma_data(dai,
				  &sunxi_i2s->playback_dma_param,
				  &sunxi_i2s->capture_dma_param);

	sunxi_i2s_init(component);

	return 0;
}

static int sunxi_i2s_dai_remove(struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");

	sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					(1 << GLOBAL_EN), (0 << GLOBAL_EN));

	return 0;
}

static struct sunxi_sound_adf_dai_driver sunxi_i2s_dai = {
	.name		= DRV_NAME,
	.playback	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 8,
		.rates		= SUNXI_I2S_RATES,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 192000,
	},
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 8,
		.rates		= SUNXI_I2S_RATES,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 192000,
	},
	.probe		= sunxi_i2s_dai_probe,
	.remove		= sunxi_i2s_dai_remove,
	.ops		= &sunxi_i2s_dai_ops,
};

static int sunxi_get_tx_hub_mode(struct sunxi_sound_adf_control *adf_control,
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

	reg_val = sunxi_sound_component_read(component, SUNXI_I2S_FIFOCTL);
	value = ((reg_val & (1 << HUB_EN)) ? 1 : 0);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_set_tx_hub_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_param *param = &sunxi_i2s->param;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}

	if (info->value) {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					 1 << CTL_TXEN, 1 << CTL_TXEN);
		sunxi_sdout_disable(component);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FIFOCTL,
					 1 << HUB_EN, 1 << HUB_EN);
	}
	else {
		sunxi_sound_component_update_bits(component, SUNXI_I2S_FIFOCTL,
					 1 << HUB_EN, 0 << HUB_EN);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL,
					 1 << CTL_TXEN, 0 << CTL_TXEN);
		sunxi_sdout_enable(component, param->tx_pin);
	}

	snd_info("mask:0x%x, items:%d, value:0x%lx\n", adf_control->mask, adf_control->items, info->value);

	return 0;
}

static int sunxi_get_rx_sync_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	unsigned long value = 0;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	value = sunxi_i2s->param.rx_sync_ctl;

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_set_rx_sync_mode(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct sunxi_i2s_param *param = &sunxi_i2s->param;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}

	if (info->value) {
		if (param->rx_sync_ctl) {
			return 0;
		}
		param->rx_sync_ctl = true;
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 <<  RX_SYNC_EN, 1 << RX_SYNC_EN);
		sunxi_sound_rx_sync_startup(param->rx_sync_domain, param->rx_sync_id, (void *)component,
				      sunxi_rx_sync_enable);
	} else {
		if (!param->rx_sync_ctl) {
			return 0;
		}
		sunxi_sound_rx_sync_shutdown(param->rx_sync_domain, param->rx_sync_id);
		sunxi_sound_component_update_bits(component, SUNXI_I2S_CTL, 1 <<  RX_SYNC_EN, 0 << RX_SYNC_EN);
		sunxi_i2s->param.rx_sync_ctl = false;
	}

	snd_info("mask:0x%x, items:%d, value:0x%lx\n", adf_control->mask, adf_control->items, info->value);

	return 0;
}

static int sunxi_i2s_get_asrc_function(struct sunxi_sound_adf_control *adf_control,
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


	reg_val = sunxi_sound_component_read(component, SUNXI_I2S_ASRC_ASRCEN);
	value = ((reg_val & (1 << I2S_ASRC_ASRCEN)) ? 1 : 0);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_i2s_set_asrc_function(struct sunxi_sound_adf_control *adf_control,
		struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid kcontrol items = %ld.\n", info->value);
		return -EINVAL;
	}


	sunxi_sound_component_update_bits(component, SUNXI_I2S_ASRC_ASRCEN,
			(1 << I2S_ASRC_ASRCEN), (info->value << I2S_ASRC_ASRCEN));
	sunxi_i2s->asrc_en = info->value;

	snd_info("mask:0x%x, items:%d, value:0x%lx\n",
			adf_control->mask, adf_control->items, info->value);

	return 0;
}

static const char *sunxi_switch_text[] = {"Off", "On"};

/* pcm Audio Mode Select */
static struct sunxi_sound_adf_control_new sunxi_i2s_controls[] = {
	SOUND_CTRL_ENUM_EXT("tx hub mode",
			 ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			 SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			 sunxi_get_tx_hub_mode,
			 sunxi_set_tx_hub_mode),
	SOUND_CTRL_ENUM_EXT("rx sync mode",
			 ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			 SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			 sunxi_get_rx_sync_mode,
			 sunxi_set_rx_sync_mode),
	SOUND_CTRL_ENUM("loopback debug",
		     ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
		     SUNXI_I2S_CTL, LOOP_EN),
	SOUND_CTRL_ENUM_EXT("sunxi I2S asrc function",
			 ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			 SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			 sunxi_i2s_get_asrc_function,
			 sunxi_i2s_set_asrc_function),
};

#ifdef CONFIG_DRIVER_SYSCONFIG
static int sunxi_sound_parse_i2s_dma_params(const char* prefix, const char* type_name,
			struct sunxi_i2s_info *info)
{
	char key_name[32];
	char cma_name[32];
	int ret;
	int32_t tmp_val;

	snprintf(key_name, sizeof(key_name), "%s%u_plat", prefix, info->param.tdm_num);

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
static struct sunxi_i2s_info default_i2s_param = {
	.playback_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
	.capture_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
};
#endif


static void sunxi_i2s_params_init(struct sunxi_i2s_info *info)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_i2s_dma_params("i2s", NULL ,info);
#else
	*info = default_i2s_param;
#endif
}

static int sunxi_i2s_gpio_init(struct sunxi_sound_adf_component *component, bool enable)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
#ifdef CONFIG_DRIVER_SYSCONFIG
	user_gpio_set_t gpio_cfg[6];
	int count, i, ret = 0;
	char i2s_name[16];
	gpio_pin_t i2s_pin;

	memset(gpio_cfg, 0, sizeof(gpio_cfg));

	sprintf(i2s_name, "i2s%d_plat", sunxi_i2s->param.tdm_num);
	count = hal_cfg_get_gpiosec_keycount(i2s_name);
	if (!count) {
		snd_err("[i2s%d] sys_config has no GPIO\n", sunxi_i2s->param.tdm_num);
	}
	hal_cfg_get_gpiosec_data(i2s_name, gpio_cfg, count);

	for (i = 0; i < count; i++) {
		i2s_pin = (gpio_cfg[i].port - 1) * 32 + gpio_cfg[i].port_num;
		snd_info("i2s_pin %d mul_sel %d\n", i2s_pin, gpio_cfg[i].mul_sel);
		if (enable) {
			ret = hal_gpio_pinmux_set_function(i2s_pin, gpio_cfg[i].mul_sel);
			if (ret != 0)
				snd_err("i2s%d pinmux[%d] set failed.\n",
					sunxi_i2s->param.tdm_num, i2s_pin);
			ret = hal_gpio_set_driving_level(i2s_pin, gpio_cfg[i].drv_level);
			if (ret != 0)
				snd_err("i2s%d driv_level = %d set failed.\n",
					sunxi_i2s->param.tdm_num, i2s_pin);
		} else {
			ret = hal_gpio_pinmux_set_function(i2s_pin, GPIO_MUXSEL_DISABLED);
			if (ret != 0)
				snd_err("i2s%d pinmux[%d] set failed.\n",
					sunxi_i2s->param.tdm_num, i2s_pin);
		}
	}

	return ret;
#else
	int i, ret;

	switch (sunxi_i2s->param.tdm_num) {
	default:
	case 0:
		sunxi_i2s->pinctrl = i2s0_pinctrl;
		sunxi_i2s->pinctrl_num = ARRAY_SIZE(i2s0_pinctrl);
		break;
#if I2S_NUM_MAX > 1
	case 1:
		sunxi_i2s->pinctrl = i2s1_pinctrl;
		sunxi_i2s->pinctrl_num = ARRAY_SIZE(i2s1_pinctrl);
		break;
#endif
#if I2S_NUM_MAX > 2
	case 2:
		sunxi_i2s->pinctrl = i2s2_pinctrl;
		sunxi_i2s->pinctrl_num = ARRAY_SIZE(i2s2_pinctrl);
		break;
#endif
#if I2S_NUM_MAX > 3
	case 3:
		sunxi_i2s->pinctrl = i2s3_pinctrl;
		sunxi_i2s->pinctrl_num = ARRAY_SIZE(i2s3_pinctrl);
		break;
#endif
	}
	snd_debug("i2s%d pinctrl_num = %d.\n", sunxi_i2s->param.tdm_num, sunxi_i2s->pinctrl_num);

	if (enable) {
		for (i = 0; i < sunxi_i2s->pinctrl_num; i++) {
			ret = hal_gpio_pinmux_set_function(sunxi_i2s->pinctrl[i].gpio_pin,
						sunxi_i2s->pinctrl[i].mux);
			if (ret != 0) {
				snd_err("i2s%d pinmux[%d] set failed.\n",
						sunxi_i2s->param.tdm_num,
						sunxi_i2s->pinctrl[i].gpio_pin);
			}
			ret = hal_gpio_set_driving_level(sunxi_i2s->pinctrl[i].gpio_pin,
						sunxi_i2s->pinctrl[i].driv_level);
			if (ret != 0) {
				snd_err("i2s%d driv_level = %d set failed.\n",
						sunxi_i2s->param.tdm_num,
						sunxi_i2s->pinctrl[i].driv_level);
			}
		}
	} else {
		for (i = 0; i < sunxi_i2s->pinctrl_num; i++) {
			ret = hal_gpio_pinmux_set_function(sunxi_i2s->pinctrl[i].gpio_pin,
						GPIO_MUXSEL_DISABLED);
			if (ret != 0) {
				snd_err("i2s%d pinmux[%d] set failed.\n",
						sunxi_i2s->param.tdm_num,
						sunxi_i2s->pinctrl[i].gpio_pin);
			}
		}
	}

	return 0;
#endif
}

static int snd_sunxi_pa_pin_init(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	sunxi_i2s->pa_cfg = i2s_pa_cfg;
	sunxi_i2s->pa_cfg_num = ARRAY_SIZE(i2s_pa_cfg);

	snd_sunxi_pa_pin_disable(component);

	return 0;
}

static int snd_sunxi_pa_pin_enable(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct pa_config *pa_cfg = sunxi_i2s->pa_cfg;
	int i;

	if (!pa_cfg)
		return 0;

	for (i = 0; i < sunxi_i2s->pa_cfg_num; i++) {
		if (!pa_cfg[i].used)
			continue;
		hal_gpio_set_direction(pa_cfg[i].pin, GPIO_MUXSEL_OUT);
		hal_gpio_set_data(pa_cfg[i].pin, pa_cfg[i].level);
		hal_msleep(pa_cfg[i].msleep);
	}

	return 0;
}

static void snd_sunxi_pa_pin_disable(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;
	struct pa_config *pa_cfg = sunxi_i2s->pa_cfg;
	int i;

	if (!pa_cfg)
		return;

	for (i = 0; i < sunxi_i2s->pa_cfg_num; i++) {
		if (!pa_cfg[i].used)
			continue;
		hal_gpio_set_direction(pa_cfg->pin, GPIO_MUXSEL_OUT);
		hal_gpio_set_data(pa_cfg->pin, !pa_cfg->level);
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

static int sunxi_i2s_suspend(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");

	sunxi_sound_save_reg(sunxi_reg_labels, (void *)component, snd_read_func);
	snd_sunxi_i2s_clk_disable(&sunxi_i2s->clk);

	return 0;
}

static int sunxi_i2s_resume(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = component->private_data;

	snd_debug("\n");

	snd_sunxi_i2s_clk_enable(&sunxi_i2s->clk, sunxi_i2s->param.tdm_num);
	sunxi_i2s_init(component);
	sunxi_sound_echo_reg(sunxi_reg_labels, (void *)component, snd_write_func);

	return 0;
}

#else

static int sunxi_i2s_suspend(struct sunxi_sound_adf_component *component)
{
	return 0;
}

static int sunxi_i2s_resume(struct sunxi_sound_adf_component *component)
{
	return 0;
}

#endif


static int sunxi_i2s_find_i2s_num(struct sunxi_i2s_param *param)
{
	int i, ret;
	int32_t tmp_val, used;
	char key_name[16];
	char i2s_name[16];

	for (i = 0; i < I2S_NUM_MAX; i++)
	{
		snprintf(key_name, sizeof(key_name) ,"i2s%d", i);
		snprintf(i2s_name, sizeof(i2s_name) ,"i2s%d_used", i);
		snd_err("key_name %s i2s_name %s.\n", key_name,i2s_name);
		ret = hal_cfg_get_keyvalue(key_name, i2s_name, (int32_t *)&tmp_val, 1);
		if (ret) {
			snd_debug("i2s:tdm_num miss.\n");
			used = 0;
		} else {
			used = tmp_val;
		}
		if (used)
			break;
	}

	if (used == 0) {
		snd_err("don't find used i2s:%u.\n", i);
		return -EFAULT;
	}

	param->i2s_index = i;

	return 0;
}


static int sunxi_i2s_param_set(struct sunxi_i2s_param *param)
{
	char i2s_name[16];
	char pin_name[16];
	int ret, i;
#ifdef CONFIG_DRIVER_SYSCONFIG
	int32_t tmp_val;
#endif

	snd_debug("\n");

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_i2s_find_i2s_num(param);
	if (ret != 0) {
		snd_err("don't find invalid i2s number.\n");
		return -EFAULT;
	}

	snprintf(i2s_name, 8,"i2s%d", param->i2s_index);
	ret = hal_cfg_get_keyvalue(i2s_name, "tdm_num", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:tdm_num miss.\n");
		param->tdm_num = i2s_param[param->i2s_index].tdm_num;
	} else {
		param->tdm_num = tmp_val;
	}
	if (param->tdm_num > I2S_NUM_MAX) {
		snd_err("tdm_num:%u overflow.\n", param->tdm_num);
		ret = -EFAULT;
	}

	for (i = 0 ;i < 4; i++) {
		snprintf(pin_name, 16,"tx-pin-%u", i);
		ret = hal_cfg_get_keyvalue(i2s_name, pin_name, (int32_t *)&tmp_val, 1);
		if (ret) {
			snd_debug("i2s:pcm_lrck_period miss.\n");
			param->tx_pin[i] = false;
		} else {
			param->tx_pin[i] = tmp_val;
		}

		snprintf(pin_name, 16,"rx-pin-%u", i);
		ret = hal_cfg_get_keyvalue(i2s_name, pin_name, (int32_t *)&tmp_val, 1);
		if (ret) {
			snd_debug("i2s:pcm_lrck_period miss.\n");
			param->rx_pin[i] = false;
		} else {
			param->rx_pin[i] = tmp_val;
		}
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "msb_lsb_first", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:msb_lsb_first miss.\n");
		param->msb_lsb_first = i2s_param[param->i2s_index].msb_lsb_first;
	} else {
		param->msb_lsb_first = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "tx_data_mode", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:tx_data_mode miss.\n");
		param->tx_data_mode = i2s_param[param->i2s_index].tx_data_mode;
	} else {
		param->tx_data_mode = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "rx_data_mode", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:rx_data_mode miss.\n");
		param->rx_data_mode = i2s_param[param->i2s_index].rx_data_mode;
	} else {
		param->rx_data_mode = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "rx_sync_en", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:rx_sync_en miss.\n");
		param->rx_sync_en = i2s_param[param->i2s_index].rx_sync_en;
	} else {
		param->rx_sync_en = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "rx_sync_ctl", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_debug("i2s:rx_sync_ctl miss.\n");
		param->rx_sync_ctl = i2s_param[param->i2s_index].rx_sync_ctl;
	} else {
		param->rx_sync_ctl = tmp_val;
	}

#else
	*param = i2s_param[param->i2s_index];
#endif

	return 0;
}

/* i2s probe */
static int sunxi_i2s_probe(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s = NULL;
	int ret = 0;

	snd_debug("\n");
	sunxi_i2s = sound_malloc(sizeof(struct sunxi_i2s_info));
	if (!sunxi_i2s) {
		snd_err("no memory\n");
		return -ENOMEM;
	}

	component->private_data = (void *)sunxi_i2s;

	ret = sunxi_i2s_param_set(&sunxi_i2s->param);
	if (ret) {
		ret = -EFAULT;
		goto err_i2s_get_param;
	}

	if (sunxi_i2s->param.rx_sync_en) {
		sunxi_i2s->param.rx_sync_domain = RX_SYNC_SYS_DOMAIN;
		sunxi_i2s->param.rx_sync_id =
			sunxi_sound_rx_sync_probe(sunxi_i2s->param.rx_sync_domain);
		if (sunxi_i2s->param.rx_sync_id < 0) {
			snd_err("sunxi_rx_sync_probe failed.\n");
			return -EINVAL;
		}
		snd_info("sunxi_rx_sync_probe successful. domain=%d, id=%d\n",
			 sunxi_i2s->param.rx_sync_domain, sunxi_i2s->param.rx_sync_id);
	}

	/* mem base */
	component->addr_base = (void *)SUNXI_I2S_BASE + (0x1000 * sunxi_i2s->param.tdm_num);

	/* clk */
	ret = snd_sunxi_i2s_clk_init(&sunxi_i2s->clk, sunxi_i2s->param.tdm_num);
	if (ret != 0) {
		snd_err("snd_sunxi_i2s_clk_init failed\n");
		goto err_i2s_set_clock;
	}

	/* pinctrl */
	sunxi_i2s_gpio_init(component, true);

	sunxi_i2s_params_init(sunxi_i2s);

	/* dma config */
	sunxi_i2s->playback_dma_param.src_maxburst = 4;
	sunxi_i2s->playback_dma_param.dst_maxburst = 4;
	sunxi_i2s->playback_dma_param.dma_addr =
			component->addr_base + SUNXI_I2S_TXFIFO;
	sunxi_i2s->capture_dma_param.src_maxburst = 4;
	sunxi_i2s->capture_dma_param.dst_maxburst = 4;
	sunxi_i2s->capture_dma_param.dma_addr =
			component->addr_base + SUNXI_I2S_RXFIFO;
	switch (sunxi_i2s->param.tdm_num) {
	case 0:
		SUNXI_I2S_DRQDST(sunxi_i2s, 0);
		SUNXI_I2S_DRQSRC(sunxi_i2s, 0);
		break;
#if I2S_NUM_MAX > 1
	case 1:
		SUNXI_I2S_DRQDST(sunxi_i2s, 1);
		SUNXI_I2S_DRQSRC(sunxi_i2s, 1);
		break;
#endif
#if I2S_NUM_MAX > 2
	case 2:
		SUNXI_I2S_DRQDST(sunxi_i2s, 2);
		SUNXI_I2S_DRQSRC(sunxi_i2s, 2);
		break;
#endif
#if I2S_NUM_MAX > 3
	case 3:
		SUNXI_I2S_DRQDST(sunxi_i2s, 3);
		SUNXI_I2S_DRQSRC(sunxi_i2s, 3);
		break;
#endif
	default:
		snd_err("tdm_num:%u overflow\n", sunxi_i2s->param.tdm_num);
		ret = -EFAULT;
		goto err_i2s_tdm_num_over;
	}

	snd_sunxi_pa_pin_init(component);

	return 0;

err_i2s_tdm_num_over:
err_i2s_set_clock:
	snd_sunxi_i2s_clk_exit(&sunxi_i2s->clk);
err_i2s_get_param:
	return ret;
}

static void sunxi_i2s_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_i2s_info *sunxi_i2s;

	snd_debug("\n");
	sunxi_i2s = component->private_data;
	if (!sunxi_i2s)
		return;

	if (sunxi_i2s->param.rx_sync_en)
		sunxi_sound_rx_sync_remove(sunxi_i2s->param.rx_sync_domain);

	snd_sunxi_i2s_clk_exit(&sunxi_i2s->clk);
	sunxi_i2s_gpio_init(component, false);
	sound_free(sunxi_i2s);
	component->private_data = NULL;

	return;
}

static struct sunxi_sound_adf_component_driver sunxi_i2s_component_dev = {
	.probe		= sunxi_i2s_probe,
	.remove		= sunxi_i2s_remove,
	.suspend 	= sunxi_i2s_suspend,
	.resume 	= sunxi_i2s_resume,
	.controls       = sunxi_i2s_controls,
	.num_controls   = ARRAY_SIZE(sunxi_i2s_controls),
};

int sunxi_i2s_component_probe(enum snd_platform_type plat_type)
{
	int ret;
	char name[48];
	snd_debug("\n");

	switch (plat_type) {
		case SND_PLATFORM_TYPE_I2S0:
			sunxi_i2s_component_dev.name = COMPONENT_DRV0_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S1:
			sunxi_i2s_component_dev.name = COMPONENT_DRV1_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S2:
			sunxi_i2s_component_dev.name = COMPONENT_DRV2_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S3:
			sunxi_i2s_component_dev.name = COMPONENT_DRV3_NAME;
			break;
		default:
			ret = -EINVAL;
			goto err;
	}

	ret= sunxi_sound_adf_register_component(&sunxi_i2s_component_dev, &sunxi_i2s_dai, 1);
	if (ret != 0) {
		snd_err("sunxi_sound_adf_register_component failed");
		return ret;
	}

	snprintf(name, sizeof(name), "%s-dma", sunxi_i2s_component_dev.name);
	ret = sunxi_pcm_dma_platform_register(name);
	if (ret != 0) {
		snd_err("sunxi_pcm_dma_platform_register failed");
		return ret;
	}
	return 0;
err:
	return ret;
}

void sunxi_i2s_component_remove(enum snd_platform_type plat_type)
{
	char name[48];
	snd_debug("\n");

	switch (plat_type) {
		case SND_PLATFORM_TYPE_I2S0:
			sunxi_i2s_component_dev.name = COMPONENT_DRV0_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S1:
			sunxi_i2s_component_dev.name = COMPONENT_DRV1_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S2:
			sunxi_i2s_component_dev.name = COMPONENT_DRV2_NAME;
			break;
		case SND_PLATFORM_TYPE_I2S3:
			sunxi_i2s_component_dev.name = COMPONENT_DRV3_NAME;
			break;
		default:
			snd_err("type %d is invalid", plat_type);
			return;
	}


	sunxi_sound_adf_unregister_component(&sunxi_i2s_component_dev);

	snprintf(name, sizeof(name), "%s-dma", sunxi_i2s_component_dev.name);
	sunxi_pcm_dma_platform_unregister(name);
}



/* #define SUNXI_I2S_DEBUG_REG */

#ifdef SUNXI_I2S_DEBUG_REG
/* for debug */
int cmd_i2s_dump(int argc, char *argv[])
{
	int i2s_num = 0;
	void *membase;
	int i = 0;

	if (argc == 2) {
		i2s_num = atoi(argv[1]);
	}
	membase = (void *)SUNXI_I2S_BASE + (0x1000 * i2s_num);

	while (sunxi_reg_labels[i].name != NULL) {
		printf("%-20s[0x%03x]: 0x%-10x\n",
			sunxi_reg_labels[i].name,
			sunxi_reg_labels[i].address,
			snd_readl(membase + sunxi_reg_labels[i].address));
		i++;
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_i2s_dump, i2s, i2s dump reg);
#endif /* SUNXI_I2S_DEBUG_REG */

