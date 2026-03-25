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
#include <hal_dma.h>
#include <hal_clk.h>
#include <hal_gpio.h>
#include <hal_timer.h>
#include <hal_reset.h>
#include <sunxi_hal_timer.h>
#include <sunxi_hal_efuse.h>
#include <sound_v2/sunxi_sound_io.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif

#include "sun8iw20-codec.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-codec"
#define DRV_NAME			"sunxi-snd-codec-dai"

enum {
	PB_AUDIO_ROUTE_LINEOUT_SPK = 0,
	PB_AUDIO_ROUTE_HEADPHONE_SPK,
	PB_AUDIO_ROUTE_LINEOUT,
	PB_AUDIO_ROUTE_HEADPHONE,
	PB_AUDIO_ROUTE_LO_HP,
	PB_AUDIO_ROUTE_LO_HP_SPK,
	PB_AUDIO_ROUTE_MAX,
};

enum {
	ADC_AUDIO_ROUTE_ON = 0x1,
	ADC_AUDIO_ROUTE_MIC = 0x2,
	ADC_AUDIO_ROUTE_LINEIN = 0x4,
	ADC_AUDIO_ROUTE_FMIN = 0x8,
};

#ifdef CONFIG_COMPONENTS_PM
static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_VOL_CTL),
	REG_LABEL(SUNXI_DAC_FIFOC),
	REG_LABEL(SUNXI_DAC_FIFOS),
	REG_LABEL(SUNXI_DAC_TXDATA),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG),
	REG_LABEL(SUNXI_ADC_FIFOC),
	REG_LABEL(SUNXI_ADC_VOL_CTL1),
	REG_LABEL(SUNXI_ADC_FIFOS),
	REG_LABEL(SUNXI_ADC_RXDATA),
	REG_LABEL(SUNXI_ADC_CNT),
	REG_LABEL(SUNXI_ADC_DG),
	REG_LABEL(SUNXI_ADC_DIG_CTL),
	REG_LABEL(SUNXI_DAC_DAP_CTL),
	REG_LABEL(SUNXI_ADC_DAP_CTL),
	REG_LABEL(SUNXI_ADC1_ANA_CTL),
	REG_LABEL(SUNXI_ADC2_ANA_CTL),
	REG_LABEL(SUNXI_ADC3_ANA_CTL),
	REG_LABEL(SUNXI_DAC_ANA_CTL),
	REG_LABEL(SUNXI_MICBIAS_ANA_CTL),
	REG_LABEL(SUNXI_RAMP_ANA_CTL),
	REG_LABEL(SUNXI_BIAS_ANA_CTL),
	REG_LABEL(SUNXI_HMIC_CTRL),
	REG_LABEL(SUNXI_HMIC_STS),
	REG_LABEL(SUNXI_HP_ANA_CTL),
	REG_LABEL(SUNXI_POWER_ANA_CTL),
	REG_LABEL(SUNXI_ADC_CUR_ANA_CTL),

	REG_LABEL_END,
};
#endif

static struct sunxi_codec_param default_param = {
	.dac_vol	= 0x0,
	.hpout_vol	= 0x3,
	.lineout_vol	= 0x1a,
	.mic1_gain		= 0x1f,
	.mic2_gain		= 0x1f,
	.mic3_gain		= 0x1f,
	.mic1_en		= true,
	.mic2_en		= true,
	.mic3_en		= false,
	.linein_gain	= 0x0,
	.adcdrc_cfg     	= 0,
	.adchpf_cfg     	= 1,
	.dacdrc_cfg     	= 0,
	.dachpf_cfg     	= 0,
	.pb_audio_route 	= PB_AUDIO_ROUTE_HEADPHONE,
	.rx_sync_en     	= false,
	.rx_sync_ctl		= false,
	.hp_detect_used		= false,

};

static struct sunxi_pa_config default_pa_cfg = {
	.gpio 		= GPIOB(3),
	.drv_level	= GPIO_DRIVING_LEVEL1,
	.mul_sel	= GPIO_MUXSEL_OUT,
	.data		= GPIO_DATA_HIGH,
	.pa_msleep_time	= 160,
};

struct sample_rate {
	unsigned int samplerate;
	unsigned int rate_bit;
};

static const struct sample_rate sample_rate_conv[] = {
	{44100, 0},
	{48000, 0},
	{8000, 5},
	{32000, 1},
	{22050, 2},
	{24000, 2},
	{16000, 3},
	{11025, 4},
	{12000, 4},
	{192000, 6},
	{96000, 7},
	{88200, 7}, /* audio spec do not supply 88.2k */
};

static int snd_sunxi_clk_init(struct sunxi_codec_clk *clk);
static void snd_sunxi_clk_exit(struct sunxi_codec_clk *clk);
static int snd_sunxi_clk_enable(struct sunxi_codec_clk *clk);
static void snd_sunxi_clk_disable(struct sunxi_codec_clk *clk);

static int sunxi_get_adc_ch(struct sunxi_sound_adf_component *component)
{
	uint32_t reg_val;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *codec_param = &sunxi_codec->param;

	codec_param->adc1_en = 0;
	reg_val = sunxi_sound_component_read(component, SUNXI_ADC1_ANA_CTL);

	if (reg_val & (1 << MIC_PGA_EN) || codec_param->mic1_en) {
		codec_param->adc1_en = ADC_AUDIO_ROUTE_MIC;
	} else if (reg_val & (1 << FMINL_EN)) {
		codec_param->adc1_en = ADC_AUDIO_ROUTE_FMIN;
	} else if (reg_val & (1 << LINEINL_EN)) {
		codec_param->adc1_en = ADC_AUDIO_ROUTE_FMIN;
	} else if (codec_param->adc1_en != 0) {
		codec_param->adc1_en |= ADC_AUDIO_ROUTE_ON;
	}

	codec_param->adc2_en = 0;
	reg_val = sunxi_sound_component_read(component, SUNXI_ADC2_ANA_CTL);

	if (reg_val & (1 << MIC_PGA_EN) || codec_param->mic2_en) {
		codec_param->adc2_en = ADC_AUDIO_ROUTE_MIC;
	} else if (reg_val & (1 << FMINR_EN)) {
		codec_param->adc2_en = ADC_AUDIO_ROUTE_FMIN;
	} else if (reg_val & (1 << LINEINR_EN)) {
		codec_param->adc2_en = ADC_AUDIO_ROUTE_FMIN;
	} else if (codec_param->adc2_en != 0) {
		codec_param->adc2_en |= ADC_AUDIO_ROUTE_ON;
	}

	codec_param->adc3_en = 0;
	reg_val = sunxi_sound_component_read(component, SUNXI_ADC3_ANA_CTL);

	if (reg_val & (1 << MIC_PGA_EN) || codec_param->mic3_en) {
		codec_param->adc3_en = ADC_AUDIO_ROUTE_MIC;
	} else if (codec_param->mic3_en != 0) {
		codec_param->adc3_en |= ADC_AUDIO_ROUTE_ON;
	}

	if (codec_param->adc1_en || codec_param->adc2_en ||
		codec_param->adc3_en)
		return 0;

	return -1;
}

#ifdef SUNXI_CODEC_DAP_ENABLE
static void adcdrc_config(struct sunxi_sound_adf_component *component)
{
	/* Left peak filter attack time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFLAT, 0x000B77BF & 0xFFFF);
	/* Left peak filter release time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LPFLAT, 0x00012BAF & 0xFFFF);
	/* Right RMS filter attack time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_RPFLAT, 0x00012BAF & 0xFFFF);

	/* smooth filter attack time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_SFHAT, (0x00017665 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_SFLAT, 0x00017665 & 0xFFFF);
	/* gain smooth filter release time */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HOPL, (0xFBD8FBA7 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LOPL, 0xFBD8FBA7 & 0xFFFF);
	/* OPC */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HOPC, (0xF95B2C3F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LOPC, 0xF95B2C3F & 0xFFFF);
	/* OPE */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HOPE, (0xF45F8D6E >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LOPE, 0xF45F8D6E & 0xFFFF);
	/* LT */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HLT, (0x01A934F0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LLT, 0x01A934F0 & 0xFFFF);
	/* CT */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HCT, (0x06A4D3C0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LCT, 0x06A4D3C0 & 0xFFFF);
	/* ET */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HET, (0x0BA07291 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LET, 0x0BA07291 & 0xFFFF);
	/* Ki */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HKI, (0x00051EB8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LKI, 0x00051EB8 & 0xFFFF);
	/* Kc */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HKC, (0x00800000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LKC, 0x00800000 & 0xFFFF);
	/* Kn */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HKN, (0x01000000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LKN, 0x01000000 & 0xFFFF);
	/* Ke */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HKE, (0x0000F45F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LKE, 0x0000F45F & 0xFFFF);
}

static void adcdrc_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN),
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN),
			(0x0 << ADC_DRC0_EN | 0x0 << ADC_DRC1_EN));
	}
}

static void adchpf_config(struct sunxi_sound_adf_component *component)
{
	/* HPF */
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_ADC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void adchpf_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN),
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN),
			(0x0 << ADC_HPF0_EN | 0x0 << ADC_HPF1_EN));
	}
}

static void dacdrc_config(struct sunxi_sound_adf_component *component)
{
	/* Left peak filter attack time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RPFHAT, (0x000B77F0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RPFLAT, 0x000B77F0 & 0xFFFF);

	/* Left peak filter release time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LRMSHAT, (0x00012BB0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LRMSLAT, 0x00012BB0 & 0xFFFF);
	/* Right RMS filter attack time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RRMSHAT, (0x00012BB0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_RRMSLAT, 0x00012BB0 & 0xFFFF);

	/* smooth filter attack time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_SFHAT, (0x00017665 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_SFLAT, 0x00017665 & 0xFFFF);
	/* gain smooth filter release time */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HOPL, (0xFF641741 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LOPL, 0xFF641741 & 0xFFFF);
	/* OPC */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HOPC, (0xF9E8E88C >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LOPC, 0xF9E8E88C & 0xFFFF);
	/* OPE */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HOPE, (0xF5DE3D14 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LOPE, 0xF5DE3D14 & 0xFFFF);
	/* LT */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HLT, (0x0336110B >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LLT, 0x0336110B & 0xFFFF);
	/* CT */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HCT, (0x08BF6C28 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LCT, 0x08BF6C28 & 0xFFFF);
	/* ET */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HET, (0x0C9F9255 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LET, 0x0C9F9255 & 0xFFFF);
	/* Ki */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HKI, (0x001A7B96 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LKI, 0x001A7B96 & 0xFFFF);
	/* Kc */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HKC, (0x00FD70A5 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LKC, 0x00FD70A5 & 0xFFFF);
	/* Kn */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HKN, (0x010AF8B0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LKN, 0x010AF8B0 & 0xFFFF);
	/* Ke */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HKE, (0x06286BA0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LKE, 0x06286BA0 & 0xFFFF);
	/* MXG */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_MXGHS, (0x035269E0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_MXGLS, 0x035269E0 & 0xFFFF);
	/* MNG */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_MNGHS, (0xF95B2C3F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_MNGLS, 0xF95B2C3F & 0xFFFF);
	/* EPS */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_EPSHC, (0x00025600 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_EPSLC, 0x00025600 & 0xFFFF);
}

static void dacdrc_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		/* detect noise when ET enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_NOISE_DET_EN),
			(0x1 << DAC_DRC_NOISE_DET_EN));

		/* 0x0:RMS filter; 0x1:Peak filter */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_SIGNAL_SEL),
			(0x1 << DAC_DRC_SIGNAL_SEL));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_GAIN_MAX_EN),
			(0x1 << DAC_DRC_GAIN_MAX_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_GAIN_MIN_EN),
			(0x1 << DAC_DRC_GAIN_MIN_EN));

		/* delay function enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_DELAY_BUF_EN),
			(0x1 << DAC_DRC_DELAY_BUF_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_LT_EN),
			(0x1 << DAC_DRC_LT_EN));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_ET_EN),
			(0x1 << DAC_DRC_ET_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x1 << DDAP_DRC_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x0 << DDAP_DRC_EN));

		/* detect noise when ET enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_NOISE_DET_EN),
			(0x0 << DAC_DRC_NOISE_DET_EN));

		/* 0x0:RMS filter; 0x1:Peak filter */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_SIGNAL_SEL),
			(0x0 << DAC_DRC_SIGNAL_SEL));

		/* delay function enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_DELAY_BUF_EN),
			(0x0 << DAC_DRC_DELAY_BUF_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_GAIN_MAX_EN),
			(0x0 << DAC_DRC_GAIN_MAX_EN));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_GAIN_MIN_EN),
			(0x0 << DAC_DRC_GAIN_MIN_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_LT_EN),
			(0x0 << DAC_DRC_LT_EN));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DRC_CTRL,
			(0x1 << DAC_DRC_ET_EN),
			(0x0 << DAC_DRC_ET_EN));
	}
}

static void dachpf_config(struct sunxi_sound_adf_component *component)
{
	/* HPF */
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, SUNXI_DAC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void dachpf_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x1 << DDAP_HPF_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x0 << DDAP_HPF_EN));
	}
}
#endif

static const char * const lineout_mux[] = {
			"DAC_SINGLE", "DAC_DIFFER"};

static const char * const micin_select[] = {
			"MIC_DIFFER", "MIC_SINGLE"};

static const char *sunxi_switch_text[] = {"Off", "On"};

static const char * const pb_audio_route_text[] = {
			"lo+spk", "hp+spk", "lineout", "headphone",
			"lo+hp", "lo+hp+spk"};

static int sunxi_pb_audio_route_get_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, param->pb_audio_route);
	return 0;
}

static int sunxi_pb_audio_route_set_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	if (info->value >= PB_AUDIO_ROUTE_MAX)
		return -1;

	param->pb_audio_route = info->value;
	return 0;
}

static inline int sunxi_get_data_invert(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	int value = 0;
	value = sunxi_sound_component_read(component, adf_control->reg);
	value = (adf_control->max & value >> adf_control->shift);

	info->value = adf_control->max - value;
	info->id = adf_control->id;
	info->type = adf_control->type;
	info->name = adf_control->name;
	info->min = adf_control->min;
	info->max = adf_control->max;
	info->count = adf_control->count;
	info->items = adf_control->items;
	info->texts = adf_control->texts;

	return 0;
}

static inline int sunxi_set_data_invert(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;

	if (info->value > adf_control->max || info->value < adf_control->min)
		return -1;

	info->value = adf_control->max - info->value;
	sunxi_sound_component_update_bits(component, adf_control->reg,
			(adf_control->max << adf_control->shift), (info->value << adf_control->shift));
	return 0;
}

static void sunxi_rx_sync_enable(void *data, bool enable)
{
	struct sunxi_sound_adf_component *component = data;

	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				      1 << RX_SYNC_EN_STA, 1 << RX_SYNC_EN_STA);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				      1 << RX_SYNC_EN_STA, 0 << RX_SYNC_EN_STA);
	}

}

static int sunxi_get_rx_sync_mode(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	unsigned long value = 0;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid adf_control type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	value = sunxi_codec->param.rx_sync_ctl;

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, value);

	return 0;
}

static int sunxi_set_rx_sync_mode(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	snd_debug("\n");
	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid adf_control type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid adf_control items = %ld.\n", info->value);
		return -EINVAL;
	}

	if (info->value) {
		if (param->rx_sync_ctl) {
			return 0;
		}
		param->rx_sync_ctl = true;
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC, 1 <<  RX_SYNC_EN, 1 << RX_SYNC_EN);
		sunxi_sound_rx_sync_startup(param->rx_sync_domain, param->rx_sync_id, (void *)component,
				      sunxi_rx_sync_enable);
	} else {
		if (!param->rx_sync_ctl) {
			return 0;
		}
		sunxi_sound_rx_sync_shutdown(param->rx_sync_domain, param->rx_sync_id);
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC, 1 <<  RX_SYNC_EN, 0 << RX_SYNC_EN);
		param->rx_sync_ctl = false;
	}

	snd_info("mask:0x%x, items:%d, value:0x%lx\n", adf_control->mask, adf_control->items, info->value);

	return 0;
}

static struct sunxi_sound_adf_control_new sunxi_codec_controls[] = {
	SOUND_CTRL_ENUM((unsigned char*)"tx hub mode",
		     ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
		     SUNXI_DAC_DPC, DAC_HUB_EN),
	SOUND_CTRL_ENUM_EXT("rx sync mode",
			 ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			 SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			 sunxi_get_rx_sync_mode,
			 sunxi_set_rx_sync_mode),

	/* global digital volume */
	SOUND_CTRL_ADF_CONTROL_EXT_REG((unsigned char*)"digital volume",
			SUNXI_DAC_DPC, DVOL, 0x3F,
			sunxi_get_data_invert, sunxi_set_data_invert),

	SOUND_CTRL_ENUM((unsigned char*)"LINEOUTL Output Select",
			ARRAY_SIZE(lineout_mux), lineout_mux,
			SUNXI_DAC_ANA_CTL, LINEOUTLDIFFEN),
	SOUND_CTRL_ENUM((unsigned char*)"LINEOUTR Output Select",
			ARRAY_SIZE(lineout_mux), lineout_mux,
			SUNXI_DAC_ANA_CTL, LINEOUTRDIFFEN),
	/* DAC digital volume */
	SOUND_CTRL_ENUM((unsigned char*)"DAC digital volume switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_DAC_VOL_CTL, DAC_VOL_SEL),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"DACL digital volume",
			SUNXI_DAC_VOL_CTL, DAC_VOL_L, 0xFF),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"DACR digital volume",
			SUNXI_DAC_VOL_CTL, DAC_VOL_R, 0xFF),

	/* analog volume */
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"MIC1 gain volume",
			SUNXI_ADC1_ANA_CTL, ADC_PGA_GAIN_CTL, 0x1F),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"MIC2 gain volume",
			SUNXI_ADC2_ANA_CTL, ADC_PGA_GAIN_CTL, 0x1F),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"MIC3 gain volume",
			SUNXI_ADC3_ANA_CTL, ADC_PGA_GAIN_CTL, 0x1F),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"FM Left gain volume",
			SUNXI_ADC1_ANA_CTL, FMINL_GAIN, 0x1),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"FM Right gain volume",
			SUNXI_ADC2_ANA_CTL, FMINR_GAIN, 0x1),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"Line Right gain volume",
			SUNXI_ADC2_ANA_CTL, LINEINL_GAIN, 0x1),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"Line Left gain volume",
			SUNXI_ADC2_ANA_CTL, LINEINR_GAIN, 0x1),
	SOUND_CTRL_ADF_CONTROL_EXT_REG((unsigned char*)"HPOUT gain volume",
			SUNXI_HP_ANA_CTL, HP_GAIN, 0x7,
			sunxi_get_data_invert, sunxi_set_data_invert),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"LINEOUT volume",
			SUNXI_DAC_ANA_CTL, LINEOUT_VOL, 0x1f),

	/* ADC digital volume */
	SOUND_CTRL_ENUM((unsigned char*)"ADC1_2 digital volume switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC_DIG_CTL, ADC1_2_VOL_EN),
	SOUND_CTRL_ENUM((unsigned char*)"ADC3 digital volume switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC_DIG_CTL, ADC3_VOL_EN),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"ADC1 digital volume",
			SUNXI_ADC_VOL_CTL1, ADC1_VOL, 0xFF),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"ADC2 digital volume",
			SUNXI_ADC_VOL_CTL1, ADC2_VOL, 0xFF),
	SOUND_CTRL_ADF_CONTROL((unsigned char*)"ADC3 digital volume",
			SUNXI_ADC_VOL_CTL1, ADC3_VOL, 0xFF),

	/* mic input select */
	SOUND_CTRL_ENUM((unsigned char*)"MIC1 input select",
			ARRAY_SIZE(micin_select), micin_select,
			SUNXI_ADC1_ANA_CTL, MIC_SIN_EN),
	SOUND_CTRL_ENUM((unsigned char*)"MIC2 input select",
			ARRAY_SIZE(micin_select), micin_select,
			SUNXI_ADC2_ANA_CTL, MIC_SIN_EN),
	SOUND_CTRL_ENUM((unsigned char*)"MIC3 input select",
			ARRAY_SIZE(micin_select), micin_select,
			SUNXI_ADC3_ANA_CTL, MIC_SIN_EN),
	/* ADC1 */
	SOUND_CTRL_ENUM((unsigned char*)"MIC1 input switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC1_ANA_CTL, MIC_PGA_EN),
	SOUND_CTRL_ENUM((unsigned char*)"FM input L switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC1_ANA_CTL, FMINL_EN),
	SOUND_CTRL_ENUM((unsigned char*)"LINE input L switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC1_ANA_CTL, LINEINL_EN),
	/* ADC2 */
	SOUND_CTRL_ENUM((unsigned char*)"MIC2 input switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC2_ANA_CTL, MIC_PGA_EN),
	SOUND_CTRL_ENUM((unsigned char*)"FM input R switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC2_ANA_CTL, FMINR_EN),
	SOUND_CTRL_ENUM((unsigned char*)"LINE input R switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC2_ANA_CTL, LINEINR_EN),
	/* ADC3 */
	SOUND_CTRL_ENUM((unsigned char*)"MIC3 input switch",
			ARRAY_SIZE(sunxi_switch_text), sunxi_switch_text,
			SUNXI_ADC3_ANA_CTL, MIC_PGA_EN),

	SOUND_CTRL_ENUM_EXT((unsigned char *)"Playback audio route",
			ARRAY_SIZE(pb_audio_route_text), pb_audio_route_text,
			SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			sunxi_pb_audio_route_get_data, sunxi_pb_audio_route_set_data),
};

static void sunxi_codec_init(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	/* get chp version */
	sunxi_codec->chip_ver = hal_efuse_get_chip_ver();

	/* In order to ensure that the ADC sampling is normal,
	 * the A chip SOC needs to always open HPLDO and RMC_EN
	 */
	if (sunxi_codec->chip_ver == CHIP_VER_A) {
		sunxi_sound_component_update_bits(component, SUNXI_POWER_ANA_CTL,
			(0x1<<HPLDO_EN), (0x1<<HPLDO_EN));
		sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
			(0x1<<RMCEN), (0x1<<RMCEN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_POWER_ANA_CTL,
			(0x1<<HPLDO_EN), (0x0<<HPLDO_EN));
		sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
			(0x1<<RMCEN), (0x0<<RMCEN));
	}

	/* Disable HPF(high passed filter) */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
			(1 << HPF_EN), (0x0 << HPF_EN));

	if (param->rx_sync_en) {
		/* disabled ADCFDT */
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(0x1 << ADCDFEN), (0x0 << ADCDFEN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(0x3 << ADCFDT), (0x2 << ADCFDT));
		/* RX_SYNC_EN */
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC, 1 << RX_SYNC_EN, 0 << RX_SYNC_EN);
	} else {
		/* Enable ADCFDT to overcome niose at the beginning */
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(0x1 << ADCDFEN), (0x1 << ADCDFEN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(0x3 << ADCFDT), (0x2 << ADCFDT));
	}

	/* init the mic pga and vol params */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
			0x1F << LINEOUT_VOL,
			param->lineout_vol << LINEOUT_VOL);
	sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
			0x7 << HP_GAIN,
			param->hpout_vol << HP_GAIN);

	/* DAC_VOL_SEL enable */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_VOL_CTL,
			(0x1 << DAC_VOL_SEL), (0x1 << DAC_VOL_SEL));
	sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
			0x3F << DVOL,
			param->dac_vol << DVOL);

	sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
			(0x1 << LINEOUTLDIFFEN) | (0x1 << LINEOUTRDIFFEN),
			(0x1 << LINEOUTLDIFFEN) | (0x1 << LINEOUTRDIFFEN));

	sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
			0x1F << ADC_PGA_GAIN_CTL,
			param->mic1_gain << ADC_PGA_GAIN_CTL);

	sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
			0x1F << ADC_PGA_GAIN_CTL,
			param->mic2_gain << ADC_PGA_GAIN_CTL);

	sunxi_sound_component_update_bits(component, SUNXI_ADC3_ANA_CTL,
			0x1F << ADC_PGA_GAIN_CTL,
			param->mic3_gain << ADC_PGA_GAIN_CTL);

#ifdef SUNXI_CODEC_DAP_ENABLE
	if (param->dacdrc_cfg || param->dachpf_cfg) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN), (0x1 << DDAP_EN));
	}

	if (param->adcdrc_cfg || param->adchpf_cfg) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN),
				(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN));
	}

	if (param->adcdrc_cfg) {
		adcdrc_config(component);
		adcdrc_enable(component, 1);
	}
	if (param->adchpf_cfg) {
		adchpf_config(component);
		adchpf_enable(component, 1);
	}
	if (param->dacdrc_cfg) {
		dacdrc_config(component);
		dacdrc_enable(component, 1);
	}
	if (param->dachpf_cfg) {
		dachpf_config(component);
		dachpf_enable(component, 1);
	}
#endif
}

static int snd_sunxi_clk_enable(struct sunxi_codec_clk *clk)
{
	int ret;

	snd_debug("\n");

	/* rst & bus */
	ret = hal_reset_control_deassert(clk->clk_rst);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_rst clk_deassert failed.\n");
		goto err_deassert_clk_rst;
	}
	ret = hal_clock_enable(clk->clk_bus);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_bus enable failed.\n");
		goto err_enable_clk_bus;
	}

	/* pll */
	ret = hal_clock_enable(clk->clk_pll_audio0);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_pll_audio0 enable failed.\n");
		goto err_enable_clk_pll_audio0;
	}

	ret = hal_clock_enable(clk->clk_dac);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_dac enable failed.\n");
		goto err_enable_clk_dac;
	}

	ret = hal_clock_enable(clk->clk_adc);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_adc enable failed.\n");
		goto err_enable_clk_adc;
	}

	return HAL_CLK_STATUS_OK;

err_enable_clk_adc:
	hal_clock_disable(clk->clk_dac);
err_enable_clk_dac:
	hal_clock_disable(clk->clk_pll_audio0);
err_enable_clk_pll_audio0:
	hal_clock_disable(clk->clk_bus);
err_enable_clk_bus:
	hal_reset_control_assert(clk->clk_rst);
err_deassert_clk_rst:
	return HAL_CLK_STATUS_ERROR;
}

static void snd_sunxi_clk_disable(struct sunxi_codec_clk *clk)
{
	snd_debug("\n");

	hal_clock_disable(clk->clk_adc);
	hal_clock_disable(clk->clk_dac);
	hal_clock_disable(clk->clk_bus);
	hal_reset_control_assert(clk->clk_rst);
	hal_clock_disable(clk->clk_pll_audio0);

	return;
}

static int snd_sunxi_clk_init(struct sunxi_codec_clk *clk)
{
	int ret;

	snd_debug("\n");

	/* rst & bus */
	clk->clk_rst = hal_reset_control_get(HAL_SUNXI_RESET, RST_BUS_AUDIO_CODEC);
	if (!clk->clk_rst) {
		snd_err("codec clk_rst hal_reset_control_get failed\n");
		goto err_get_clk_rst;
	}

	clk->clk_bus = hal_clock_get(HAL_SUNXI_CCU, CLK_BUS_AUDIO_CODEC);
	if (!clk->clk_bus) {
		snd_err("codec clk_bus hal_clock_get failed\n");
		goto err_get_clk_bus;
	}

	/* pll clk -> 24.576M */
	clk->clk_pll_audio0 = hal_clock_get(HAL_SUNXI_CCU, CLK_PLL_AUDIO0);
	if (!clk->clk_pll_audio0) {
		snd_err("codec clk_pll_audio0 hal_clock_get failed\n");
		goto err_get_clk_pll_audio;
	}

	/* module dac clk */
	clk->clk_dac = hal_clock_get(HAL_SUNXI_CCU, CLK_AUDIO_DAC);
	if (!clk->clk_dac) {
		snd_err("codec clk_dac hal_clock_get failed\n");
		goto err_get_clk_dac;
	}

	/* module adc clk */
	clk->clk_adc = hal_clock_get(HAL_SUNXI_CCU, CLK_AUDIO_ADC);
	if (!clk->clk_adc) {
		snd_err("codec clk_adc hal_clock_get failed\n");
		goto err_get_clk_adc;
	}

	ret = hal_clk_set_parent(clk->clk_dac, clk->clk_pll_audio0);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_pll_audio1x -> clk_audpll_hosc_sel clk_set_parent failed.\n");
		goto err_set_parent_clk;
	}

	ret = hal_clk_set_parent(clk->clk_adc, clk->clk_pll_audio0);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_audpll_hosc_sel -> clk_dac clk_set_parent failed.\n");
		goto err_set_parent_clk;
	}

	/* note: Enable and then set the freq to avoid clock lock errors */
	ret = snd_sunxi_clk_enable(clk);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec snd_sunxi_clk_enable failed.\n");
		goto err_clk_enable;
	}

	/* pll div limit */
	ret = hal_clk_set_rate(clk->clk_pll_audio0, 22579200);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("set clk_pll_audio rate failed\n");
		goto err_set_rate_clk;
	}

	return HAL_CLK_STATUS_OK;

err_clk_enable:
err_set_rate_clk:
err_set_parent_clk:
	hal_clock_put(clk->clk_adc);
err_get_clk_adc:
	hal_clock_put(clk->clk_dac);
err_get_clk_dac:
	hal_clock_put(clk->clk_pll_audio0);
err_get_clk_pll_audio:
	hal_clock_put(clk->clk_bus);
err_get_clk_bus:
	hal_reset_control_put(clk->clk_rst);
err_get_clk_rst:
	return HAL_CLK_STATUS_ERROR;
}

static void snd_sunxi_clk_exit(struct sunxi_codec_clk *clk)
{
	snd_debug("\n");

	snd_sunxi_clk_disable(clk);

	hal_clock_put(clk->clk_adc);

	hal_clock_put(clk->clk_dac);
	hal_clock_put(clk->clk_pll_audio0);

	hal_clock_put(clk->clk_bus);
	hal_reset_control_put(clk->clk_rst);

	return;
}

static void sunxi_codec_playback_hp_route(struct sunxi_sound_adf_component *component, int spk, int onoff)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg;
	int ret;

	if (onoff) {
		if (sunxi_codec->chip_ver == CHIP_VER_A) {
			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<HPFB_BUF_EN) | (0x1<<HPFB_IN_EN),
					(0x1<<HPFB_BUF_EN) | (0x1<<HPFB_IN_EN));
			/* ramp out enable */
			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<RAMP_OUT_EN) | (0x1<<RSWITCH),
					(0x1<<RAMP_OUT_EN) | (0x1<<RSWITCH));
			/* digital DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x1<<EN_DAC));
			hal_msleep(5);
			/* analog DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
					(0x1<<DACLEN) | (0x1<<DACREN),
					(0x1<<DACLEN) | (0x1<<DACREN));
			/* HEADPONEOUT */
			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
				(0x1<<HP_DRVEN) | (0x1<<HP_DRVOUTEN),
				(0x1<<HP_DRVEN) | (0x1<<HP_DRVOUTEN));
			/* Playback on */
			sunxi_sound_component_update_bits(component, SUNXI_POWER_ANA_CTL,
				(0x1<<HPLDO_EN), (0x1<<HPLDO_EN));
		} else {
			/* digital DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x1<<EN_DAC));
			sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
					(0x1<<RDEN), (0x1<<RDEN));
		}
		if (spk) {
			ret = sunxi_sound_pa_enable(pa_cfg);
			if (ret) {
				snd_err("pa pin enable failed!\n");
			}
		}
	} else {
		if (spk) {
			sunxi_sound_pa_disable(pa_cfg);
		}
		/* Playback off */
		if (sunxi_codec->chip_ver == CHIP_VER_A) {
			/* HEADPONE_EN */
			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<HP_DRVEN), (0x0<<HP_DRVEN));
			/* power off */
			/* note: HPLDO is always open to avoid KEY_ADC sampling fail */
			/* sunxi_sound_component_update_bits(component, SUNXI_POWER_ANA_CTL, */
			/* 		(0x1<<HPLDO_EN), (0x0<<HPLDO_EN)); */
			hal_msleep(30);
			/* HEADPONE_OUT */
			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<HP_DRVOUTEN), (0x0<<HP_DRVOUTEN));
			/* analog DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
					(0x1<<DACLEN) | (0x1<<DACREN),
					(0x0<<DACLEN) | (0x0<<DACREN));
			/* digital DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x0<<EN_DAC));

			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<RAMP_OUT_EN) | (0x1<<RSWITCH),
					(0x0<<RAMP_OUT_EN) | (0x0<<RSWITCH));

			sunxi_sound_component_update_bits(component, SUNXI_HP_ANA_CTL,
					(0x1<<HPFB_BUF_EN) | (0x1<<HPFB_IN_EN),
					(0x0<<HPFB_BUF_EN) | (0x0<<HPFB_IN_EN));
		} else {
			/* analog DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
					(0x1<<DACLEN) | (0x1<<DACREN),
					(0x0<<DACLEN) | (0x0<<DACREN));
			/* digital DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x0<<EN_DAC));
			sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
					(0x1<<RDEN), (0x0<<RDEN));
		}
	}
}

static void sunxi_codec_playback_lineout_route(struct sunxi_sound_adf_component *component, int spk, int onoff)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg;
	int ret;

	if (onoff) {
		/* digital DAC enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
				(0x1<<EN_DAC), (0x1<<EN_DAC));
		hal_msleep(5);
		/* analog DAC enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLEN) | (0x1<<DACREN),
				(0x1<<DACLEN) | (0x1<<DACREN));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLMUTE) | (0x1<<DACRMUTE),
				(0x1<<DACLMUTE) | (0x1<<DACRMUTE));
		if (sunxi_codec->chip_ver != CHIP_VER_A) {
			uint32_t reg_val;
			reg_val = sunxi_sound_component_read(component, SUNXI_RAMP_ANA_CTL);
			if(!(reg_val & (0x1 << RDEN)))
				sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
						0x1<<RDEN, 0x1<<RDEN);
		}
		sunxi_sound_component_update_bits(component, SUNXI_DAC_REG,
				(0x1<<LINEOUTL_EN) | (0x1<<LINEOUTR_EN),
				(0x1<<LINEOUTL_EN) | (0x1<<LINEOUTR_EN));
		if (spk) {
			ret = sunxi_sound_pa_enable(pa_cfg);
			if (ret) {
				snd_err("pa pin enable failed!\n");
			}
		}
	} else {
		if (spk) {
			sunxi_sound_pa_disable(pa_cfg);
		}
		/* analog DAC */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLEN) | (0x1<<DACREN),
				(0x0<<DACLEN) | (0x0<<DACREN));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLMUTE) | (0x1<<DACRMUTE),
				(0x0<<DACLMUTE) | (0x0<<DACRMUTE));
		/* digital DAC enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
				(0x1<<EN_DAC), (0x0<<EN_DAC));
		sunxi_sound_component_update_bits(component, SUNXI_DAC_REG,
				(0x1<<LINEOUTL_EN) | (0x1<<LINEOUTR_EN),
				(0x0<<LINEOUTL_EN) | (0x0<<LINEOUTR_EN));
		if (sunxi_codec->chip_ver != CHIP_VER_A) {
			uint32_t reg_val;
			reg_val = sunxi_sound_component_read(component, SUNXI_RAMP_ANA_CTL);
			if(!(reg_val & (0x1 << RDEN)))
				sunxi_sound_component_update_bits(component, SUNXI_RAMP_ANA_CTL,
						0x1<<RDEN, 0x0<<RDEN);
		}
	}

}

static int sunxi_codec_dapm_control(struct sunxi_sound_adf_component *component,
			struct sunxi_sound_pcm_dataflow *dataflow, int onoff)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	if (dataflow->dapm_state == onoff)
		return 0;
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/*
		 * Playback:
		 * Playback --> DAC --> DAC_DIFFER --> LINEOUT/HPOUT + (External Speaker)
		 */

		switch (param->pb_audio_route) {
		case PB_AUDIO_ROUTE_LINEOUT_SPK:
			sunxi_codec_playback_lineout_route(component, 1, onoff);
			break;
		case PB_AUDIO_ROUTE_HEADPHONE_SPK:
			sunxi_codec_playback_hp_route(component, 1, onoff);
			break;
		case PB_AUDIO_ROUTE_LINEOUT:
			sunxi_codec_playback_lineout_route(component, 0, onoff);
			break;
		case PB_AUDIO_ROUTE_HEADPHONE:
			sunxi_codec_playback_hp_route(component, 0, onoff);
			break;
		case PB_AUDIO_ROUTE_LO_HP:
			sunxi_codec_playback_lineout_route(component, 0, onoff);
			sunxi_codec_playback_hp_route(component, 0, onoff);
			break;
		case PB_AUDIO_ROUTE_LO_HP_SPK:
			sunxi_codec_playback_lineout_route(component, onoff ? 0 : 1, onoff);
			sunxi_codec_playback_hp_route(component, onoff ? 1 : 0, onoff);
			break;
		}
	} else {
		/*
		 * Capture:
		 * Capture1 <-- ADC1 <-- Input Mixer <-- LINEINL PGA <-- LINEINL
		 * Capture1 <-- ADC1 <-- Input Mixer <-- FMINL PGA <-- FMINL
		 *
		 * Capture2 <-- ADC2 <-- Input Mixer <-- LINEINR PGA <-- LINEINR
		 * Capture2 <-- ADC2 <-- Input Mixer <-- FMINR PGA <-- FMINR
		 *
		 * Capture3 <-- ADC3 <-- Input Mixer <-- MIC3 PGA <-- MIC3
		 */
		unsigned int channels = 0;
		channels = dataflow->pcm_running->channels;

		snd_debug("channels = %u\n", channels);
		snd_debug("adc flag:%d,%d,%d\n",
			param->adc1_en, param->adc2_en, param->adc3_en);
		if (onoff) {
			if ((param->adc1_en & ADC_AUDIO_ROUTE_MIC) ||
				(param->adc2_en & ADC_AUDIO_ROUTE_MIC) ||
				(param->adc3_en & ADC_AUDIO_ROUTE_MIC)) {
				sunxi_sound_component_update_bits(component, SUNXI_MICBIAS_REG,
						      0x1<<MMICBIASEN,
						      0x1<<MMICBIASEN);
			}
			/* Capture on */
			hal_msleep(100);
			/* digital ADC enable */
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
					(0x1<<EN_AD), (0x1<<EN_AD));

			if (channels > 3 || channels < 1) {
				snd_err("unknown channels:%u\n", channels);
				return -EINVAL;
			}

			if (param->adc1_en == ADC_AUDIO_ROUTE_MIC) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<ADC_EN,
						      0x1<<ADC_EN);
			} else if (param->adc1_en == ADC_AUDIO_ROUTE_LINEIN ||
					param->adc1_en == ADC_AUDIO_ROUTE_FMIN) {

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1F<<ADC_PGA_GAIN_CTL,
						      0x00<<ADC_PGA_GAIN_CTL);

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<MIC_SIN_EN,
						      0x1<<MIC_SIN_EN);

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<ADC_EN,
						      0x1<<ADC_EN);
			}

			if (param->adc2_en == ADC_AUDIO_ROUTE_MIC) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<ADC_EN,
						      0x1<<ADC_EN);
			} else if (param->adc2_en == ADC_AUDIO_ROUTE_LINEIN ||
					param->adc2_en == ADC_AUDIO_ROUTE_FMIN) {

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1F<<ADC_PGA_GAIN_CTL,
						      0x00<<ADC_PGA_GAIN_CTL);

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<MIC_SIN_EN,
						      0x1<<MIC_SIN_EN);

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<ADC_EN,
						      0x1<<ADC_EN);
			}

			if (param->adc3_en) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC3_ANA_CTL,
						      0x1<<ADC_EN,
						      0x1<<ADC_EN);
			}
		} else {
			/* Capture off */
			if (channels > 3 || channels < 1) {
				snd_err("unknown channels:%u\n", channels);
				return -EINVAL;
			}

			if ((param->adc1_en & ADC_AUDIO_ROUTE_MIC) ||
				(param->adc2_en & ADC_AUDIO_ROUTE_MIC) ||
				(param->adc3_en & ADC_AUDIO_ROUTE_MIC)) {
				sunxi_sound_component_update_bits(component, SUNXI_MICBIAS_REG,
						      0x1<<MMICBIASEN,
						      0x0<<MMICBIASEN);
			}

			if (param->adc1_en == ADC_AUDIO_ROUTE_MIC) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<ADC_EN,
						      0x0<<ADC_EN);
			} else if (param->adc1_en == ADC_AUDIO_ROUTE_LINEIN ||
					param->adc1_en == ADC_AUDIO_ROUTE_FMIN) {

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1F<<ADC_PGA_GAIN_CTL,
						      param->mic1_gain<<ADC_PGA_GAIN_CTL);

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<MIC_SIN_EN,
						      0x0<<MIC_SIN_EN);

				sunxi_sound_component_update_bits(component, SUNXI_ADC1_ANA_CTL,
						      0x1<<ADC_EN,
						      0x0<<ADC_EN);
			}

			if (param->adc2_en == ADC_AUDIO_ROUTE_MIC) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<ADC_EN,
						      0x0<<ADC_EN);
			} else if (param->adc2_en == ADC_AUDIO_ROUTE_LINEIN ||
					param->adc2_en == ADC_AUDIO_ROUTE_FMIN) {

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1F<<ADC_PGA_GAIN_CTL,
						      param->mic2_gain<<ADC_PGA_GAIN_CTL);

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<MIC_SIN_EN,
						      0x0<<MIC_SIN_EN);

				sunxi_sound_component_update_bits(component, SUNXI_ADC2_ANA_CTL,
						      0x1<<ADC_EN,
						      0x0<<ADC_EN);
			}

			if (param->adc3_en) {
				sunxi_sound_component_update_bits(component, SUNXI_ADC3_ANA_CTL,
						      0x1<<ADC_EN,
						      0x0<<ADC_EN);
			}
			/* digital ADC enable */
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
					(0x1<<EN_AD), (0x0<<EN_AD));
		}
	}
	dataflow->dapm_state = onoff;
	return 0;
}

static int sunxi_codec_startup(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");

	return 0;
}


static int sunxi_codec_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				 struct sunxi_sound_pcm_hw_params *params,
				 struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *codec_param = &sunxi_codec->param;
	int i = 0;

	snd_debug("\n");
	switch (params_format(params)) {
	case	SND_PCM_FORMAT_S16_LE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(3 << FIFO_MODE), (3 << FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(1 << TX_SAMPLE_BITS), (0 << TX_SAMPLE_BITS));
		} else {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << RX_FIFO_MODE), (1 << RX_FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << RX_SAMPLE_BITS), (0 << RX_SAMPLE_BITS));
		}
		break;
	case	SND_PCM_FORMAT_S24_LE:
	case	SND_PCM_FORMAT_S32_LE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(3 << FIFO_MODE), (0 << FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(1 << TX_SAMPLE_BITS), (1 << TX_SAMPLE_BITS));
		} else {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << RX_FIFO_MODE), (0 << RX_FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << RX_SAMPLE_BITS), (1 << RX_SAMPLE_BITS));
		}
		break;
	default:
		break;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_conv); i++) {
		if (sample_rate_conv[i].samplerate == params_rate(params)) {
			if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
					(0x7 << DAC_FS),
					(sample_rate_conv[i].rate_bit << DAC_FS));
			} else {
				if (sample_rate_conv[i].samplerate > 48000)
					return -EINVAL;
				sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
					(0x7 << ADC_FS),
					(sample_rate_conv[i].rate_bit<<ADC_FS));
			}
		}
	}

	/* reset the adchpf func setting for different sampling */
	if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (codec_param->adchpf_cfg) {
			if (params_rate(params) == 16000) {

				sunxi_sound_component_write(component, SUNXI_ADC_DRC_HHPFC,
						(0x00F623A5 >> 16) & 0xFFFF);

				sunxi_sound_component_write(component, SUNXI_ADC_DRC_LHPFC,
							0x00F623A5 & 0xFFFF);

			} else if (params_rate(params) == 44100) {

				sunxi_sound_component_write(component, SUNXI_ADC_DRC_HHPFC,
						(0x00FC60DB >> 16) & 0xFFFF);

				sunxi_sound_component_write(component, SUNXI_ADC_DRC_LHPFC,
							0x00FC60DB & 0xFFFF);
			} else {
				sunxi_sound_component_write(component, SUNXI_ADC_DRC_HHPFC,
						(0x00FCABB3 >> 16) & 0xFFFF);

				sunxi_sound_component_write(component, SUNXI_ADC_DRC_LHPFC,
							0x00FCABB3 & 0xFFFF);
			}
		}
	}

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (params_channels(params)) {
		case 1:
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
					(1<<DAC_MONO_EN), 1<<DAC_MONO_EN);
			break;
		case 2:
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
					(1<<DAC_MONO_EN), (0<<DAC_MONO_EN));
			break;
		default:
			snd_err("cannot support the channels:%u.\n",
				params_channels(params));
			return -EINVAL;
		}
	} else {
		if (sunxi_get_adc_ch(component) < 0) {
			snd_err("capture only support 1~3 channel\n");
			return -EINVAL;
		}
		if (codec_param->adc1_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC1_CHANNEL_EN,
					      0x1<<ADC1_CHANNEL_EN);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC1_CHANNEL_EN,
					      0x0<<ADC1_CHANNEL_EN);
		if (codec_param->adc2_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC2_CHANNEL_EN,
					      0x1<<ADC2_CHANNEL_EN);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC2_CHANNEL_EN,
					      0x0<<ADC2_CHANNEL_EN);
		if (codec_param->adc3_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC3_CHANNEL_EN,
					      0x1<<ADC3_CHANNEL_EN);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_DIG_CTL,
					      0x1<<ADC3_CHANNEL_EN,
					      0x0<<ADC3_CHANNEL_EN);
	}

	return 0;
}

static int sunxi_codec_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
				 unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_clk *clk = &sunxi_codec->clk;

	snd_debug("\n");
	if (hal_clk_set_rate(clk->clk_pll_audio0, freq_out)) {
		snd_err("set pllclk rate %u failed\n", freq_out);
		return -EINVAL;
	}
#if 0
	if (dir == SNDRV_PCM_STREAM_PLAYBACK) {
		hal_clock_disable(sunxi_codec->moduleclk);
		if (freq == 24576000) {
			hal_clk_set_parent(sunxi_codec->moduleclk, sunxi_codec->pllclk1);
		} else {
			hal_clk_set_parent(sunxi_codec->moduleclk, sunxi_codec->pllclk);
		}
		hal_clk_set_rate(sunxi_codec->moduleclk, freq);
		hal_msleep(50);
		hal_clock_enable(sunxi_codec->moduleclk);
	}

	if (dir == SNDRV_PCM_STREAM_CAPTURE) {
		if (freq == 24576000) {
			hal_clk_set_parent(sunxi_codec->moduleclk1, sunxi_codec->pllclk1);
		} else {
			hal_clk_set_parent(sunxi_codec->moduleclk1, sunxi_codec->pllclk);
		}
		hal_clk_set_rate(sunxi_codec->moduleclk1, freq);
	}
#endif
	return 0;
}

static void sunxi_codec_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	return ;
}

static int sunxi_codec_prepare(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
			(1 << FIFO_FLUSH), (1 << FIFO_FLUSH));
		sunxi_sound_component_write(component, SUNXI_DAC_FIFOS,
			(1 << DAC_TXE_INT | 1 << DAC_TXU_INT | 1 << DAC_TXO_INT));
		sunxi_sound_component_write(component, SUNXI_DAC_CNT, 0);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << ADC_FIFO_FLUSH), (1 << ADC_FIFO_FLUSH));
		sunxi_sound_component_write(component, SUNXI_ADC_FIFOS,
				(1 << ADC_RXA_INT | 1 << ADC_RXO_INT));
		sunxi_sound_component_write(component, SUNXI_ADC_CNT, 0);
	}

	return 0;
}

static int sunxi_codec_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	snd_debug("\n");
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(1 << DAC_DRQ_EN), (1 << DAC_DRQ_EN));
		}
		else if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (1 << ADC_DRQ_EN));
			if (sunxi_codec->param.rx_sync_en && sunxi_codec->param.rx_sync_ctl)
				sunxi_sound_rx_sync_control(sunxi_codec->param.rx_sync_domain,
						      sunxi_codec->param.rx_sync_id, 1);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFOC,
				(1 << DAC_DRQ_EN), (0 << DAC_DRQ_EN));
		}
		else if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (0 << ADC_DRQ_EN));
			if (sunxi_codec->param.rx_sync_en && sunxi_codec->param.rx_sync_ctl)
				sunxi_sound_rx_sync_control(sunxi_codec->param.rx_sync_domain,
						      sunxi_codec->param.rx_sync_id, 0);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct sunxi_sound_adf_dai_ops sunxi_codec_dai_ops = {
	.startup	= sunxi_codec_startup,
	.hw_params	= sunxi_codec_hw_params,
	.shutdown	= sunxi_codec_shutdown,
	.set_pll	= sunxi_codec_dai_set_pll,		/* set pllclk */
	.trigger	= sunxi_codec_trigger,
	.prepare	= sunxi_codec_prepare,
};

static struct sunxi_sound_adf_dai_driver sunxi_codec_dai = {
	.name = DRV_NAME,
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates	= SNDRV_PCM_RATE_8000_192000
			| SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE
			| SNDRV_PCM_FMTBIT_S24_LE
			| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min       = 8000,
		.rate_max       = 192000,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 3,
		.rates = SNDRV_PCM_RATE_8000_48000
			| SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE
			| SNDRV_PCM_FMTBIT_S24_LE
			| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min       = 8000,
		.rate_max       = 48000,
	},
	.ops = &sunxi_codec_dai_ops,
};

static void sunxi_jack_detect(void *param)
{
	struct sunxi_codec_info *sunxi_codec = param;
	struct sunxi_codec_param *codec_param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg;
	gpio_data_t gpio_status;

	if (pa_cfg->gpio > 0) {
		hal_gpio_set_direction(pa_cfg->gpio, GPIO_MUXSEL_OUT);
		hal_gpio_get_data(SUNXI_CODEC_JACK_DET, &gpio_status);
		if (gpio_status) {
			/* jack plug out */
			hal_gpio_set_data(pa_cfg->gpio, pa_cfg->data);
		} else {
			/* jack plug in */
			hal_gpio_set_data(pa_cfg->gpio, !pa_cfg->data);
		}
	}
}

static hal_irqreturn_t sunxi_jack_irq_handler(void *ptr)
{
	struct sunxi_codec_info *sunxi_codec = ptr;

	hal_timer_set_oneshot(SUNXI_TMR0, 300000, sunxi_jack_detect, sunxi_codec);

	return 0;
}

static void snd_sunxi_params_init(struct sunxi_codec_param *params)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	int ret;
	int32_t val;

	ret = hal_cfg_get_keyvalue(CODEC, "dac_vol", &val, 1);
	if (ret) {
		snd_err("%s: dac_vol miss.\n", CODEC);
		params->dac_vol = default_param.dac_vol;
	} else
		params->dac_vol = val;

	ret = hal_cfg_get_keyvalue(CODEC, "lineout_vol", &val, 1);
	if (ret) {
		snd_err("%s: lineout_vol miss.\n", CODEC);
		params->lineout_vol = default_param.lineout_vol;
	} else
		params->lineout_vol = val;

	ret = hal_cfg_get_keyvalue(CODEC, "hpout_vol", &val, 1);
	if (ret) {
		snd_err("%s: lineoutl_en miss.\n", CODEC);
		params->hpout_vol = default_param.hpout_vol;
	} else
		params->hpout_vol = val;

	ret = hal_cfg_get_keyvalue(CODEC, "adc_gain", &val, 1);
	if (ret) {
		snd_debug("%s: adc_gain miss.\n", CODEC);
		params->adc_gain = default_param.adc_gain;
	} else
		params->adc_gain = val;

	ret = hal_cfg_get_keyvalue(CODEC, "linein_gain", &val, 1);
	if (ret) {
		snd_debug("%s: linein_gain miss.\n", CODEC);
		params->linein_gain = default_param.linein_gain;
	} else
		params->linein_gain = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic1_gain", &val, 1);
	if (ret) {
		snd_debug("%s: mic1_gain miss.\n", CODEC);
		params->mic1_gain = default_param.mic1_gain;
	} else
		params->mic1_gain = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic2_gain", &val, 1);
	if (ret) {
		snd_debug("%s: mic2_gain miss.\n", CODEC);
		params->mic2_gain = default_param.mic2_gain;
	} else
		params->mic2_gain = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic3_gain", &val, 1);
	if (ret) {
		snd_debug("%s: mic3_gain miss.\n", CODEC);
		params->mic3_gain = default_param.mic3_gain;
	} else
		params->mic3_gain = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic1_en", &val, 1);
	if (ret) {
		snd_debug("%s: mic1_en miss.\n", CODEC);
		params->mic1_en = default_param.mic1_en;
	} else
		params->mic1_en = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic2_en", &val, 1);
	if (ret) {
		snd_debug("%s: mic2_en miss.\n", CODEC);
		params->mic2_en = default_param.mic2_en;
	} else
		params->mic2_en = val;

	ret = hal_cfg_get_keyvalue(CODEC, "mic3_en", &val, 1);
	if (ret) {
		snd_debug("%s: mic3_en miss.\n", CODEC);
		params->mic3_en = default_param.mic3_en;
	} else
		params->mic3_en = val;

	ret = hal_cfg_get_keyvalue(CODEC, "adcdrc_cfg", &val, 1);
	if (ret) {
		snd_debug("%s: adcdrc_cfg miss.\n", CODEC);
		params->adcdrc_cfg = default_param.adcdrc_cfg;
	} else
		params->adcdrc_cfg = val;

	ret = hal_cfg_get_keyvalue(CODEC, "adchpf_cfg", &val, 1);
	if (ret) {
		snd_debug("%s: adchpf_cfg miss.\n", CODEC);
		params->adchpf_cfg = default_param.adchpf_cfg;
	} else
		params->adchpf_cfg = val;

	ret = hal_cfg_get_keyvalue(CODEC, "dacdrc_cfg", &val, 1);
	if (ret) {
		snd_debug("%s: dacdrc_cfg miss.\n", CODEC);
		params->dacdrc_cfg = default_param.dacdrc_cfg;
	} else
		params->dacdrc_cfg = val;

	ret = hal_cfg_get_keyvalue(CODEC, "dachpf_cfg", &val, 1);
	if (ret) {
		snd_debug("%s: dachpf_cfg miss.\n", CODEC);
		params->dachpf_cfg = default_param.dachpf_cfg;
	} else
		params->dachpf_cfg = val;

	ret = hal_cfg_get_keyvalue(CODEC, "rx_sync_en", (int32_t *)&val, 1);
	if (ret) {
		snd_debug("%s:rx_sync_en miss.\n", CODEC);
		params->rx_sync_en = default_param.rx_sync_en;
	} else {
		params->rx_sync_en = val;
	}

	ret = hal_cfg_get_keyvalue(CODEC, "rx_sync_ctl", (int32_t *)&val, 1);
	if (ret) {
		snd_debug("%s:rx_sync_ctl miss.\n", CODEC);
		params->rx_sync_ctl = default_param.rx_sync_ctl;
	} else {
		params->rx_sync_ctl = val;
	}

	ret = hal_cfg_get_keyvalue(CODEC, "hp_detect_used", (int32_t *)&val, 1);
	if (ret) {
		snd_debug("%s:hp_detect_used miss.\n", CODEC);
		params->hp_detect_used = default_param.hp_detect_used;
	} else {
		params->hp_detect_used = val;
	}
#else
	*params = default_param;
#endif
}

/* suspend and resume */
#ifdef CONFIG_COMPONENTS_PM
static unsigned int snd_read_func(void *data, unsigned int reg)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_codec_info *sunxi_codec;

	if (!data) {
		snd_err("data is invailed\n");
		return 0;
	}

	component = data;
	sunxi_codec = component->private_data;

	return sunxi_sound_component_read(component, reg);
}

static void snd_write_func(void *data, unsigned int reg, unsigned int val)
{
	struct sunxi_sound_adf_component *component;
	struct sunxi_codec_info *sunxi_codec;
	if (!data) {
		snd_err("data is invailed\n");
		return;
	}

	component = data;
	sunxi_codec = component->private_data;
	sunxi_sound_component_write(component, reg, val);
}

static int sunxi_codec_suspend(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	snd_err("\n");

	sunxi_sound_save_reg(sunxi_reg_labels, (void *)component, snd_read_func);
	sunxi_sound_pa_disable(&sunxi_codec->pa_cfg);
	snd_sunxi_clk_disable(&sunxi_codec->clk);

	return 0;
}

static int sunxi_codec_resume(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	snd_err("\n");

	sunxi_sound_pa_disable(&sunxi_codec->pa_cfg);
	snd_sunxi_clk_enable(&sunxi_codec->clk);
	sunxi_codec_init(component);
	sunxi_sound_echo_reg(sunxi_reg_labels, (void *)component, snd_write_func);

	return 0;
}

#else

static int sunxi_codec_suspend(struct sunxi_sound_adf_component *component)
{
	return 0;
}

static int sunxi_codec_resume(struct sunxi_sound_adf_component *component)
{
	return 0;
}

#endif

static int sun8iw20_codec_probe(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = NULL;
	struct reset_control *reset;
	hal_clk_status_t ret;

	sunxi_codec = sound_malloc(sizeof(struct sunxi_codec_info));
	if (!sunxi_codec) {
		snd_err("no memory\n");
		return -ENOMEM;
	}

	component->private_data = (void *)sunxi_codec;

	snd_debug("codec para init\n");
	/* get codec para from board config? */
	sunxi_codec->param = default_param;

	if (sunxi_codec->param.rx_sync_en) {
		sunxi_codec->param.rx_sync_domain = RX_SYNC_SYS_DOMAIN;
		sunxi_codec->param.rx_sync_id =
			sunxi_sound_rx_sync_probe(sunxi_codec->param.rx_sync_domain);
		if (sunxi_codec->param.rx_sync_id < 0) {
			snd_err("sunxi_rx_sync_probe failed");
			return -EINVAL;
		}
		snd_info("sunxi_rx_sync_probe successful. domain=%d, id=%d",
			 sunxi_codec->param.rx_sync_domain,
			 sunxi_codec->param.rx_sync_id);
	}

	snd_sunxi_params_init(&sunxi_codec->param);
	sunxi_sound_pa_init(&sunxi_codec->pa_cfg, &default_pa_cfg, CODEC);
	sunxi_sound_pa_disable(&sunxi_codec->pa_cfg);

	component->addr_base = (void *)SUNXI_CODEC_BASE_ADDR;

	ret = snd_sunxi_clk_init(&sunxi_codec->clk);
	if (ret != 0) {
		snd_err("snd_sunxi_clk_init failed\n");
		goto err_codec_set_clock;
	}

	sunxi_codec_init(component);

	hal_timer_init(SUNXI_TMR0);

	if (sunxi_codec->param.hp_detect_used) {
		if (hal_gpio_to_irq(SUNXI_CODEC_JACK_DET, &sunxi_codec->irq) < 0)
			snd_err("sunxi jack gpio to irq err.\n");
		if (hal_gpio_irq_request(sunxi_codec->irq, sunxi_jack_irq_handler, IRQ_TYPE_EDGE_FALLING, sunxi_codec) < 0)
			snd_err("sunxi jack irq request err.\n");
		sunxi_jack_detect(sunxi_codec);
		if (hal_gpio_irq_enable(sunxi_codec->irq) < 0)
			snd_err("sunxi jack irq enable err.\n");
	}

	return 0;

err_codec_set_clock:
	snd_sunxi_clk_exit(&sunxi_codec->clk);

	return -1;
}

static void sun8iw20_codec_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	if (sunxi_codec->param.rx_sync_en) {
		sunxi_sound_rx_sync_remove(sunxi_codec->param.rx_sync_domain);
	}

	if (param->adcdrc_cfg)
		adcdrc_enable(component, 0);
	if (param->adchpf_cfg)
		adchpf_enable(component, 0);
	if (param->dacdrc_cfg)
		dacdrc_enable(component, 0);
	if (param->dachpf_cfg)
		dachpf_enable(component, 0);

	snd_sunxi_clk_exit(&sunxi_codec->clk);

	hal_gpio_irq_disable(sunxi_codec->irq);
	hal_gpio_irq_free(sunxi_codec->irq);
	hal_timer_uninit(SUNXI_TMR0);
	sound_free(sunxi_codec);
	component->private_data = NULL;

	return;
}

static struct sunxi_sound_adf_component_driver sunxi_codec_component_dev = {
	.name		= COMPONENT_DRV_NAME,
	.probe		= sun8iw20_codec_probe,
	.remove		= sun8iw20_codec_remove,
	.suspend 	= sunxi_codec_suspend,
	.resume 	= sunxi_codec_resume,
	.controls       = sunxi_codec_controls,
	.num_controls   = ARRAY_SIZE(sunxi_codec_controls),
	.dapm_control	= sunxi_codec_dapm_control,
};

int sunxi_codec_component_probe()
{
	snd_debug("\n");
	return sunxi_sound_adf_register_component(&sunxi_codec_component_dev, &sunxi_codec_dai, 1);
}

void sunxi_codec_component_remove()
{
	snd_debug("\n");
	sunxi_sound_adf_unregister_component(&sunxi_codec_component_dev);
}


