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
#include <aw_common.h>
#include <sound_v2/sunxi_sound_io.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif

#include "sun8iw19-codec.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-codec"
#define DRV_NAME			"sunxi-snd-codec-dai"

#ifdef CONFIG_COMPONENTS_PM
static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_FIFOC),
	REG_LABEL(SUNXI_DAC_FIFOS),
	REG_LABEL(SUNXI_DAC_TXDATA),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG),
	REG_LABEL(SUNXI_ADC_FIFOC),
	REG_LABEL(SUNXI_ADC_FIFOS),
	REG_LABEL(SUNXI_ADC_RXDATA),
	REG_LABEL(SUNXI_ADC_CNT),
	REG_LABEL(SUNXI_ADC_DG),
	REG_LABEL(SUNXI_DAC_DAP_CTL),
	REG_LABEL(SUNXI_ADC_DAP_CTL),
	REG_LABEL(SUNXI_ADCL_ANA_CTL),
	REG_LABEL(SUNXI_DAC_ANA_CTL),
	REG_LABEL(SUNXI_MICBIAS_ANA_CTL),
	REG_LABEL(SUNXI_BIAS_ANA_CTL),
	REG_LABEL_END,
};
#endif

static struct sunxi_codec_param default_param = {
	.dac_vol	= 0x0,
	.lineout_vol	= 0x1f,
	.mic1_gain	= 0x1f,
	.mic1_en		= true,
	.linein_gain	= 0x0,
	.adcdrc_cfg     = 0,
	.adchpf_cfg     = 1,
	.dacdrc_cfg     = 0,
	.dachpf_cfg     = 0,
};

static struct sunxi_pa_config default_pa_cfg = {
	.gpio 		= GPIOH(4),
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
};

#ifdef SUNXI_ADC_DAUDIO_SYNC
struct sunxi_sound_adf_component *adc_daudio_sync_codec;
static int substream_mode;
int adc_sync_flag;

int sunxi_codec_get_pcm_trigger_substream_mode(void)
{
	return substream_mode;
}

void sunxi_codec_set_pcm_trigger_substream_mode(int value)
{
	if (!((adc_sync_flag >> ADC_I2S_RUNNING) & 0x1)) {
		substream_mode = value;
	} else {
		snd_err("set the adc sync mode should be stop the record.\n");
	}
}

void sunxi_codec_set_pcm_adc_sync_flag(int value)
{
       adc_sync_flag = value;
}

int sunxi_codec_get_pcm_adc_sync_flag(void)
{
	return adc_sync_flag;
}

/* for adc and i2s rx sync */
void sunxi_cpudai_adc_drq_enable(bool enable)
{
	if (enable) {
		sunxi_sound_component_update_bits(adc_daudio_sync_codec, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (1 << ADC_DRQ_EN));
	} else {
		sunxi_sound_component_update_bits(adc_daudio_sync_codec, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (0 << ADC_DRQ_EN));
	}
}
#endif

static int sunxi_get_adc_ch(struct sunxi_sound_adf_component *component)
{
	uint32_t reg_val;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *codec_param = &sunxi_codec->param;

	codec_param->adc1_en = 0;
	reg_val = sunxi_sound_component_read(component, SUNXI_ADC_FIFOC);

	if (reg_val & (1<<ADC_CHAN_SEL) || codec_param->mic1_en) {
		codec_param->adc1_en = 1;
	}

	if (codec_param->adc1_en)
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
			(0x1 << ADC_DRC0_EN), (0x1 << ADC_DRC0_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN), (0x0 << ADC_DRC0_EN));
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
			(0x1 << ADC_HPF0_EN), (0x1 << ADC_HPF0_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN), (0x0 << ADC_HPF0_EN));
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

/* for adc and i2s rx sync */
#ifdef SUNXI_ADC_DAUDIO_SYNC
static int sunxi_codec_get_substream_mode(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	unsigned int val = 0;
	uint32_t __cpsr;

	__cpsr = hal_spin_lock_irqsave();

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED)
		return -EINVAL;

	val = sunxi_codec_get_pcm_trigger_substream_mode();

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, val);

	hal_spin_unlock_irqrestore(__cpsr);

	return 0;

}

static int sunxi_codec_set_substream_mode(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	uint32_t __cpsr;

	__cpsr = hal_spin_lock_irqsave();

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED)
		return -EINVAL;

	if (info->value >= adf_control->items)
		return -EINVAL;

	sunxi_codec_set_pcm_trigger_substream_mode(info->value);

	hal_spin_unlock_irqrestore(__cpsr);

	snd_info("mask:0x%x, items:%d, value:0x%lx\n",
			adf_control->mask, adf_control->items, info->value);

	return 0;
}

static const char * const sunxi_codec_substream_mode_function[] = {"ADC_ASYNC",
			"ADC_I2S_SYNC"};
#endif

static const char * const codec_format_function[] = {
			"hub_disable", "hub_enable"};

static const char * const codec_output_mode_select[] = {
			"DACL_SINGLE", "DACL_DIFFER"};

static const char * const codec_linein_switch[] = {
			"Off", "On"};


static struct sunxi_sound_adf_control_new sunxi_codec_controls[] = {
	SOUND_CTRL_ENUM("codec hub mode",
		ARRAY_SIZE(codec_format_function), codec_format_function,
					SUNXI_DAC_DPC, DAC_HUB_EN),
	SOUND_CTRL_ENUM("Left LINEOUT Mux",
		ARRAY_SIZE(codec_output_mode_select), codec_output_mode_select,
					SUNXI_DAC_ANA_CTL, LINEOUTLDIFFEN),
	SOUND_CTRL_ENUM("Left Input Mixer LINEINL Switch",
		ARRAY_SIZE(codec_linein_switch), codec_linein_switch,
					SUNXI_ADCL_ANA_CTL, LINEINLEN),
#ifdef SUNXI_ADC_DAUDIO_SYNC
	SOUND_CTRL_ENUM_EXT("codec trigger substream mode",
				ARRAY_SIZE(sunxi_codec_substream_mode_function),
				sunxi_codec_substream_mode_function,
				SND_CTL_ENUM_AUTO_MASK,
				sunxi_codec_get_substream_mode,
				sunxi_codec_set_substream_mode),
#endif
	SOUND_CTRL_ADF_CONTROL("digital volume", SUNXI_DAC_DPC, DVOL, 0x3F),
	SOUND_CTRL_ADF_CONTROL("LINEIN gain volume", SUNXI_ADCL_ANA_CTL, LINEINLG, 0x1),
	SOUND_CTRL_ADF_CONTROL("MIC1 gain volume", SUNXI_ADCL_ANA_CTL, PGA_GAIN_CTRL, 0x1F),
	SOUND_CTRL_ADF_CONTROL("LINEOUT volume", SUNXI_DAC_ANA_CTL, LINEOUT_VOL, 0x1F),
};

static void sunxi_codec_init(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	/* Enable ADCFDT to overcome niose at the beginning */
	sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
			(0x7 << ADCDFEN), (0x7 << ADCDFEN));

	/* init the mic pga and vol params */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
			0x1F << LINEOUT_VOL,
			param->lineout_vol << LINEOUT_VOL);

	sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
			0x3F << DVOL,
			param->dac_vol << DVOL);

	sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
			0x1F << PGA_GAIN_CTRL,
			param->mic1_gain << PGA_GAIN_CTRL);

	sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
			0x1 << LINEINLG,
			param->linein_gain << LINEINLG);

	sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
			0x3 << IOPLINE, 0x1 << IOPLINE);

#ifdef SUNXI_CODEC_DAP_ENABLE
	if (param->dacdrc_cfg || param->dachpf_cfg) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
				(0x1 << DDAP_EN), (0x1 << DDAP_EN));
	}

	if (param->adcdrc_cfg || param->adchpf_cfg) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
				(0x1 << ADC_DAP0_EN), (0x1 << ADC_DAP0_EN));
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

	/* pll */
	ret = hal_clock_enable(clk->clk_pll_audio);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_pll_audio enable failed.\n");
		goto err_enable_clk_pll_audio;
	}


	ret = hal_clock_enable(clk->clk_audio);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_audio enable failed.\n");
		goto err_enable_clk_audio;
	}

	return HAL_CLK_STATUS_OK;

err_enable_clk_audio:
	hal_clock_disable(clk->clk_pll_audio);
err_enable_clk_pll_audio:
	return HAL_CLK_STATUS_ERROR;
}

static void snd_sunxi_clk_disable(struct sunxi_codec_clk *clk)
{
	snd_debug("\n");

	hal_clock_disable(clk->clk_audio);
	hal_clock_disable(clk->clk_pll_audio);
	return;
}

static int snd_sunxi_clk_init(struct sunxi_codec_clk *clk)
{
	int ret;

	snd_debug("\n");


	/* pll clk -> 24.576M */
	clk->clk_pll_audio = hal_clock_get(HAL_SUNXI_CCU, HAL_CLK_PLL_AUDIO);
	if (!clk->clk_pll_audio) {
		snd_err("codec clk_pll_audio hal_clock_get failed\n");
		goto err_clk_pll_audio;
	}

	/* module audiocodec clk */
	clk->clk_audio = hal_clock_get(HAL_SUNXI_CCU, HAL_CLK_PERIPH_AUDIOCODEC_1X);
	if (!clk->clk_audio) {
		snd_err("codec clk_audio hal_clock_get failed\n");
		goto err_get_clk_audio;
	}

	ret = hal_clk_set_parent(clk->clk_audio, clk->clk_pll_audio);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_pll_audio4x -> clk_audio clk_set_parent failed.\n");
		goto err_set_parent_clk;
	}

	/* note: Enable and then set the freq to avoid clock lock errors */
	ret = snd_sunxi_clk_enable(clk);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec snd_sunxi_clk_enable failed.\n");
		goto err_clk_enable;
	}

	return HAL_CLK_STATUS_OK;

err_clk_enable:
err_set_parent_clk:
	hal_clock_put(clk->clk_audio);
err_get_clk_audio:
	hal_clock_put(clk->clk_pll_audio);
err_clk_pll_audio:
	return HAL_CLK_STATUS_ERROR;
}

static void snd_sunxi_clk_exit(struct sunxi_codec_clk *clk)
{
	snd_debug("\n");

	snd_sunxi_clk_disable(clk);

	hal_clock_put(clk->clk_audio);
	hal_clock_put(clk->clk_pll_audio);

	return;
}

static int sunxi_codec_dapm_control(struct sunxi_sound_adf_component *component,
			struct sunxi_sound_pcm_dataflow *dataflow, int onoff)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg;
	int ret;

	if (dataflow->dapm_state == onoff)
		return 0;

	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/*
		 * Playback:
		 * Playback --> DACL --> Left LINEOUT Mux --> LINEOUTL --> External Speaker
		 *
		 */
		if (onoff) {
			/* Playback on */
			/* analog DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
					(0x1<<DACLEN), (0x1<<DACLEN));
			/* digital DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x1<<EN_DAC));
			hal_msleep(10);
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLMUTE), (0x1<<DACLMUTE));
			/* LINEOUT */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<LINEOUTL_EN), (0x1<<LINEOUTL_EN));

			if (pa_cfg->gpio > 0) {
				ret = sunxi_sound_pa_enable(pa_cfg);
				if (ret) {
					snd_err("pa pin enable failed!\n");
				}
			}
		} else {
			/* Playback off */
			if (pa_cfg->gpio > 0) {
				sunxi_sound_pa_disable(pa_cfg);
			}
			/* LINEOUT */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<LINEOUTL_EN), (0x0<<LINEOUTL_EN));

			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLMUTE), (0x0<<DACLMUTE));

			/* digital DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x0<<EN_DAC));
			/* analog DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
					(0x1<<DACLEN), (0x0<<DACLEN));
		}
	} else {
		/*
		 * Capture:
		 * Capture <-- ADCL <-- Left Input Mixer <-- MIC1 PGA <-- MIC1 <-- MainMic Bias
		 *
		 */
		unsigned int channels = 0;
		channels = dataflow->pcm_running->channels;

		snd_debug("channels = %u\n", channels);
		if (onoff) {
			/* Capture on */
			/* digital ADC enable */
			hal_msleep(100);
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
					(0x1<<EN_AD), (0x1<<EN_AD));
			switch (channels) {
			case 1:
				/* analog ADCL enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
						(0x1<<ADCLEN), (0x1<<ADCLEN));

				sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
					(0x1<<MIC1AMPEN), (0x1<<MIC1AMPEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
			}
			/* MainMic Bias */
			sunxi_sound_component_update_bits(component, SUNXI_MICBIAS_ANA_CTL,
					(0x1<<MMICBIASEN), (0x1<<MMICBIASEN));
		} else {
			/* Capture off */
			/* MainMic Bias */
			sunxi_sound_component_update_bits(component, SUNXI_MICBIAS_ANA_CTL,
					(0x1<<MMICBIASEN), (0x0<<MMICBIASEN));
			switch (channels) {
			case 1:
				/* MIC1 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
					(0x1<<MIC1AMPEN), (0x0<<MIC1AMPEN));

				/* analog ADCL enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
						(0x1<<ADCLEN), (0x0<<ADCLEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
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
			snd_err("capture only support 1 channel\n");
			return -EINVAL;
		}

		if (codec_param->adc1_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
											0x1<<ADC_CHAN_SEL,
											0x1<<ADC_CHAN_SEL);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
											0x1<<ADC_CHAN_SEL,
											0x0<<ADC_CHAN_SEL);

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
	if (hal_clk_set_rate(clk->clk_pll_audio, freq_out)) {
		snd_err("set clk_pll_audio rate %u failed\n", freq_out);
		return -EINVAL;
	}

	return 0;
}

static void sunxi_codec_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg;

	snd_debug("\n");
	/*
	 * Playback:
	 * Playback --> DACL --> DACL_SINGLE --> LINEOUTL --> External Speaker
	 * Playback --> DACL --> DACL_DIFFER --> LINEOUTL --> External Speaker
	 *
	 * Capture:
	 * Capture <-- ADCL <-- Left Input Mixer <-- MIC1 PGA <-- MIC1 <-- MainMic Bias
	 * Capture <-- ADCL <-- Left Input Mixer <-- LINEINL PGA <-- LINEINL <-- MainMic Bias
	 *
	 */
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (pa_cfg->gpio > 0)
			sunxi_sound_pa_disable(pa_cfg);

		/* LINEOUT */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<LINEOUTL_EN), (0x0<<LINEOUTL_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLMUTE), (0x0<<DACLMUTE));

		/* digital DAC enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
				(0x1<<EN_DAC), (0x0<<EN_DAC));

		/* analog DAC enable */
		sunxi_sound_component_update_bits(component, SUNXI_DAC_ANA_CTL,
				(0x1<<DACLEN), (0x0<<DACLEN));
	} else {
		/* MainMic Bias */
		sunxi_sound_component_update_bits(component, SUNXI_MICBIAS_ANA_CTL,
				(0x1<<MMICBIASEN), (0x0<MMICBIASEN));

		/* MIC PGA */
		sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
				(0x1<<MIC1AMPEN), (0x0<<MIC1AMPEN));

		/* digital ADC enable */
		sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
				(0x1<<EN_AD), (0x0<<EN_AD));

		/* analog ADCL enable */
		sunxi_sound_component_update_bits(component, SUNXI_ADCL_ANA_CTL,
				(0x1<<ADCLEN), (0x0<<ADCLEN));
	}

	return;
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
#ifdef SUNXI_ADC_DAUDIO_SYNC
	unsigned int sync_mode = 0;
	int adc_sync_flag = 0;
	uint32_t __cpsr;
#endif

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
#ifndef SUNXI_ADC_DAUDIO_SYNC
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (1 << ADC_DRQ_EN));
#else
			__cpsr = hal_spin_lock_irqsave();
			sync_mode = sunxi_codec_get_pcm_trigger_substream_mode();
			if (sync_mode) {
				adc_sync_flag = sunxi_codec_get_pcm_adc_sync_flag();
				adc_sync_flag |= (0x1 << ADC_CODEC_SYNC);
				if (adc_sync_flag & (0x1 << ADC_I2S_RUNNING)) {
					sunxi_cpudai_adc_drq_enable(true);
				} else if ((adc_sync_flag & (0x1 << ADC_CODEC_SYNC)) &&
						(adc_sync_flag & (0x1 << ADC_I2S_SYNC))) {
					adc_sync_flag |= (0x1 << ADC_I2S_RUNNING);
					sunxi_cpudai_adc_drq_enable(true);
					sunxi_daudio_rx_drq_enable(true);
				}
				sunxi_codec_set_pcm_adc_sync_flag(adc_sync_flag);
			} else
				sunxi_cpudai_adc_drq_enable(true);
			hal_spin_unlock_irqrestore(__cpsr);
#endif
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
#ifndef SUNXI_ADC_DAUDIO_SYNC
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFOC,
				(1 << ADC_DRQ_EN), (0 << ADC_DRQ_EN));
#else
			__cpsr = hal_spin_lock_irqsave();
			adc_sync_flag = sunxi_codec_get_pcm_adc_sync_flag();
			adc_sync_flag &= ~(0x1 << ADC_CODEC_SYNC);
			if (!((adc_sync_flag >> ADC_CODEC_SYNC) & 0x1) &&
					(!((adc_sync_flag >> ADC_I2S_SYNC) & 0x1))) {
				adc_sync_flag &= ~(0x1 << ADC_I2S_RUNNING);
			}
			sunxi_codec_set_pcm_adc_sync_flag(adc_sync_flag);
			sunxi_cpudai_adc_drq_enable(false);
			hal_spin_unlock_irqrestore(__cpsr);
#endif
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct sunxi_sound_adf_dai_ops sun8iw19_codec_dai_ops = {
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
			| SNDRV_PCM_FMTBIT_S24_LE,
		.rate_min       = 8000,
		.rate_max       = 192000,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000_48000
			| SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE
			| SNDRV_PCM_FMTBIT_S24_LE,
		.rate_min       = 8000,
		.rate_max       = 48000,
	},
	.ops = &sun8iw19_codec_dai_ops,
};

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

	ret = hal_cfg_get_keyvalue(CODEC, "mic1_en", &val, 1);
	if (ret) {
		snd_debug("%s: mic1_en miss.\n", CODEC);
		params->mic1_en = default_param.mic1_en;
	} else
		params->mic1_en = val;

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

static int sun8iw19_codec_probe(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = NULL;
	int ret;

	sunxi_codec = sound_malloc(sizeof(struct sunxi_codec_info));
	if (!sunxi_codec) {
		snd_err("no memory\n");
		return -ENOMEM;
	}

	component->private_data = (void *)sunxi_codec;

	snd_debug("codec para init\n");
	/* get codec para from board config? */
	sunxi_codec->param = default_param;

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

#ifdef SUNXI_ADC_DAUDIO_SYNC
	adc_daudio_sync_codec = component;
#endif

	return 0;

err_codec_set_clock:
	snd_sunxi_clk_exit(&sunxi_codec->clk);
	return -1;
}

static void sun8iw19_codec_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	if (param->adcdrc_cfg)
		adcdrc_enable(component, 0);
	if (param->adchpf_cfg)
		adchpf_enable(component, 0);
	if (param->dacdrc_cfg)
		dacdrc_enable(component, 0);
	if (param->dachpf_cfg)
		dachpf_enable(component, 0);

	snd_sunxi_clk_exit(&sunxi_codec->clk);

	sound_free(sunxi_codec);
	component->private_data = NULL;

	return;
}

static struct sunxi_sound_adf_component_driver sunxi_codec_component_dev = {
	.name		= COMPONENT_DRV_NAME,
	.probe		= sun8iw19_codec_probe,
	.remove		= sun8iw19_codec_remove,
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

