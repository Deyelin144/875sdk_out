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

#include "sun8iw18-codec.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-codec"
#define DRV_NAME			"sunxi-snd-codec-dai"

#define SUNXI_AUDIOCODEC_REG_DEBUG


#if defined(CONFIG_COMPONENTS_PM) || defined(SUNXI_AUDIOCODEC_REG_DEBUG)

static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_DAC_DPC),
	REG_LABEL(SUNXI_DAC_FIFO_CTL),
	REG_LABEL(SUNXI_DAC_FIFO_STA),
	REG_LABEL(SUNXI_DAC_CNT),
	REG_LABEL(SUNXI_DAC_DG),
	REG_LABEL(SUNXI_ADC_FIFO_CTL),
	REG_LABEL(SUNXI_ADC_FIFO_STA),
	REG_LABEL(SUNXI_ADC_CNT),
	REG_LABEL(SUNXI_ADC_DG),
	REG_LABEL(SUNXI_DAC_DAP_CTL),
	REG_LABEL(SUNXI_ADC_DAP_CTL),

	REG_LABEL(SUNXI_HP_CTL),
	REG_LABEL(SUNXI_MIX_DAC_CTL),
	REG_LABEL(SUNXI_LINEOUT_CTL0),
	REG_LABEL(SUNXI_LINEOUT_CTL1),
	REG_LABEL(SUNXI_MIC1_CTL),
	REG_LABEL(SUNXI_MIC2_MIC3_CTL),

	REG_LABEL(SUNXI_LADCMIX_SRC),
	REG_LABEL(SUNXI_RADCMIX_SRC),
	REG_LABEL(SUNXI_XADCMIX_SRC),
	REG_LABEL(SUNXI_ADC_CTL),
	REG_LABEL(SUNXI_MBIAS_CTL),
	REG_LABEL(SUNXI_APT_REG),

	REG_LABEL(SUNXI_OP_BIAS_CTL0),
	REG_LABEL(SUNXI_OP_BIAS_CTL1),
	REG_LABEL(SUNXI_ZC_VOL_CTL),
	REG_LABEL(SUNXI_BIAS_CAL_CTRL),
	REG_LABEL_END,
};
#endif

#ifdef SUNXI_AUDIOCODEC_REG_DEBUG

void sunxi_audiocodec_reg_dump(struct sunxi_sound_adf_component *component)
{
	int i = 0;

	while (sunxi_reg_labels[i].name != NULL) {
		printf("%-20s[0x%03x]: 0x%-10x\n",
			sunxi_reg_labels[i].name,
			sunxi_reg_labels[i].address,
			sunxi_sound_component_read(component, sunxi_reg_labels[i].address));
		i++;
	}

	return;
}
#else
void sunxi_audiocodec_reg_dump(struct sunxi_sound_adf_component *component)
{
	return ;
}
#endif

struct sunxi_codec_param default_param = {
#if AW87579_ANALOG_PA
	.twi_dev		= AW87579_CHIP_CFG,
#endif
	.dac_vol	= 0x0,
	.lineout_vol	= 0x1a,
	.mic1_gain		= 0x1f,
	.mic2_gain		= 0x1f,
	.mic3_gain		= 0x1f,
	.mic1_en		= true,
	.mic2_en		= true,
	.mic3_en		= false,
	.linein_gain	= 0x0,
	.adc_gain		= 0x3,
	.adcdrc_cfg 	= 0,
	.adchpf_cfg 	= 1,
	.dacdrc_cfg 	= 0,
	.dachpf_cfg 	= 0,
};

static struct sunxi_pa_config default_pa_cfg = {
	.gpio 		= GPIOH(9),
	.drv_level	= GPIO_DRIVING_LEVEL1,
	.mul_sel	= GPIO_MUXSEL_OUT,
	.data		= GPIO_DATA_HIGH,
	.pa_msleep_time	= 50,
};

static struct sunxi_pa_config default_pa_power_cfg = {
	.gpio 		= GPIOH(2),
	.drv_level	= GPIO_DRIVING_LEVEL1,
	.mul_sel	= GPIO_MUXSEL_OUT,
	.data		= GPIO_DATA_HIGH,
	.pa_msleep_time	= 50,
};


static uint32_t read_prcm_wvalue(uint32_t addr, void *ADDA_PR_CFG_REG)
{
	uint32_t reg = 0;

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 28);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1 << 24);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= ~(0x3f << 16);
	reg |= (addr << 16);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= (0xff << 0);

	return reg;
}

static void write_prcm_wvalue(uint32_t addr, uint32_t val, void *ADDA_PR_CFG_REG)
{
	uint32_t reg;

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 28);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= ~(0x3f << 16);
	reg |= (addr << 16);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= ~(0xff << 8);
	reg |= (val << 8);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 24);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);

	reg = sunxi_sound_readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1 << 24);
	sunxi_sound_writel(reg, ADDA_PR_CFG_REG);
}

static unsigned int sunxi_codec_read(struct sunxi_sound_adf_component *component,
			     unsigned int reg)
{
	unsigned int reg_val;

	if (reg >= SND_SUNXI_ACPRCFG_ADDR) {
		reg = reg - SND_SUNXI_ACPRCFG_ADDR;
		return read_prcm_wvalue(reg, component->addr_base + SND_SUNXI_ACPRCFG_ADDR);
	} else {
		reg_val = sunxi_sound_component_read(component, reg);
	}
	return reg_val;
}

static int sunxi_codec_write(struct sunxi_sound_adf_component *component,
		     unsigned int reg, unsigned int val)
{
	if (reg >= SND_SUNXI_ACPRCFG_ADDR) {
		reg = reg - SND_SUNXI_ACPRCFG_ADDR;
		write_prcm_wvalue(reg, val, component->addr_base + SND_SUNXI_ACPRCFG_ADDR);
	} else {
		sunxi_sound_component_write(component, reg, val);
	}
	return 0;
}

#if AW87579_ANALOG_PA
static twi_status_t aw87579_init_i2c_device(twi_port_t port)
{
	twi_status_t ret = 0;

	ret = hal_twi_init(port);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err ret=%d.\n", ret);
		return ret;
	}

	return TWI_STATUS_OK;
}

static twi_status_t aw87579_deinit_i2c_device(twi_port_t port)
{
	twi_status_t ret = 0;

	ret = hal_twi_uninit(port);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err ret=%d.\n", ret);
		return ret;
	}

	return TWI_STATUS_OK;
}

static twi_status_t aw87579_read(struct twi_device *twi_dev,
	unsigned char reg, unsigned char *rt_value)
{
	twi_status_t ret;

	ret = hal_twi_read(twi_dev->bus, I2C_SLAVE, twi_dev->addr, reg, rt_value, 1);
	if (ret != TWI_STATUS_OK) {
		snd_err("error = %d [REG-0x%02x]\n", ret, reg);
		return ret;
	}

	return TWI_STATUS_OK;
}

static int aw87579_write(struct twi_device *twi_dev,
	unsigned char reg, unsigned char value)
{
	twi_status_t ret;
	twi_msg_t msg;
	unsigned char buf[2] = {reg, value};

	msg.flags = 0;
	msg.addr =  twi_dev->addr;
	msg.len = 2;
	msg.buf = buf;

	ret = hal_twi_write(twi_dev->bus, &msg, 1);
	if (ret != TWI_STATUS_OK) {
		snd_err("error = %d [REG-0x%02x]\n", ret, reg);
		return ret;
	}

	return TWI_STATUS_OK;
}

static int aw87579_analog_pa_get_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	twi_status_t ret;
	unsigned char value = 0;

	ret = aw87579_read(&sunxi_codec->param.twi_dev, AW87579_REG_SYSCTRL, &value);
	if (ret != TWI_STATUS_OK) {
		snd_err("aw87579 analog pa get data failed!");
		return -1;
	}
	if (value == 0x78) {
		info->value = 1;
	} else {
		info->value = 0;
	}
	snd_debug("get analog pa value:%u\n", info->value);
	info->id = adf_control->id;
	info->name = adf_control->name;
	info->min = adf_control->min;
	info->max = adf_control->max;
	return 0;
}

static int aw87579_analog_pa_set_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;

	if (info->value != AW87579_ANALOG_PA_ON && info->value != AW87579_ANALOG_PA_OFF)
		return -1;

	unsigned char value = 0;
	if (info->value == AW87579_ANALOG_PA_ON) {
		value = 0x78;
	} else {
		value = 0x01;
	}
	aw87579_write(&sunxi_codec->param.twi_dev, AW87579_REG_SYSCTRL, value);
	snd_debug("set analog pa value:%lu\n", info->value);
	return 0;
}

#else

static int sunxi_spk_gpio_get_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg[0];

	if (pa_cfg->gpio > 0) {
		hal_gpio_get_data(pa_cfg->gpio,
					(gpio_data_t *)&info->value);
		snd_debug("get spk value:%u\n", info->value);
		info->id = adf_control->id;
		info->name = adf_control->name;
		info->min = adf_control->min;
		info->max = adf_control->max;
		return 0;
	}

	return -1;
}

static int sunxi_spk_gpio_set_data(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg[0];

	if (info->value != GPIO_DATA_LOW && info->value != GPIO_DATA_HIGH)
		return -1;
	if (pa_cfg->gpio > 0) {
		hal_gpio_set_direction(pa_cfg->gpio, GPIO_DIRECTION_OUTPUT);
		hal_gpio_set_data(pa_cfg->gpio, (gpio_data_t)info->value);
		snd_debug("set spk value:%u\n", info->value);
		return 0;
	}

	return -1;
}
#endif

static const char * const codec_format_function[] = {
			"hub_disable", "hub_enable"};

static struct sunxi_sound_adf_control_new sunxi_codec_controls[] = {
	SOUND_CTRL_ADF_CONTROL("digital volume", SUNXI_DAC_DPC, DVOL, 0x3F),
	SOUND_CTRL_ADF_CONTROL("MIC1 gain volume", SUNXI_MIC1_CTL, MIC1BOOST, 0x7),
	SOUND_CTRL_ADF_CONTROL("MIC2 gain volume", SUNXI_MIC2_MIC3_CTL, MIC2BOOST, 0x7),
	SOUND_CTRL_ADF_CONTROL("MIC3 gain volume", SUNXI_MIC2_MIC3_CTL, MIC3BOOST, 0x7),
	SOUND_CTRL_ADF_CONTROL("ADC gain volume", SUNXI_ADC_CTL, ADCG, 0x7),
	SOUND_CTRL_ADF_CONTROL("LINEOUT volume", SUNXI_LINEOUT_CTL1, LINEOUT_VOL, 0x1f),
	#if AW87579_ANALOG_PA
	SOUND_CTRL_ADF_CONTROL_EXT("Spk PA Switch", 1, 0,
					aw87579_analog_pa_get_data,
					aw87579_analog_pa_set_data),
	#else
	SOUND_CTRL_ADF_CONTROL_EXT("Spk PA Switch", 1, 0,
					sunxi_spk_gpio_get_data,
					sunxi_spk_gpio_set_data),
	#endif

	SOUND_CTRL_ADF_CONTROL("Left Input Mixer DACL Switch", SUNXI_LADCMIX_SRC, LADC_DACL, 1),
	SOUND_CTRL_ADF_CONTROL("Left Input Mixer MIC1 Boost Switch", SUNXI_LADCMIX_SRC, LADC_MIC1_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Left Input Mixer MIC2 Boost Switch", SUNXI_LADCMIX_SRC, LADC_MIC2_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Left Input Mixer MIC3 Boost Switch", SUNXI_LADCMIX_SRC, LADC_MIC3_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Right Input Mixer DACL Switch", SUNXI_RADCMIX_SRC, RADC_DACL, 1),
	SOUND_CTRL_ADF_CONTROL("Right Input Mixer MIC1 Boost Switch", SUNXI_RADCMIX_SRC, RADC_MIC1_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Right Input Mixer MIC2 Boost Switch", SUNXI_RADCMIX_SRC, RADC_MIC2_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Right Input Mixer MIC3 Boost Switch", SUNXI_RADCMIX_SRC, RADC_MIC3_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Xadc Input Mixer DACL Switch", SUNXI_XADCMIX_SRC, XADC_DACL, 1),
	SOUND_CTRL_ADF_CONTROL("Xadc Input Mixer MIC1 Boost Switch", SUNXI_XADCMIX_SRC, XADC_MIC1_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Xadc Input Mixer MIC2 Boost Switch", SUNXI_XADCMIX_SRC, XADC_MIC2_STAGE, 1),
	SOUND_CTRL_ADF_CONTROL("Xadc Input Mixer MIC3 Boost Switch", SUNXI_XADCMIX_SRC, XADC_MIC3_STAGE, 1),

	SOUND_CTRL_ADF_CONTROL("Left LINEOUT Mux", SUNXI_LINEOUT_CTL0, LINEOUTL_SRC, 1),
	SOUND_CTRL_ADF_CONTROL("Right LINEOUT Mux", SUNXI_LINEOUT_CTL0, LINEOUTR_SRC, 1),
	SOUND_CTRL_ENUM("sunxi codec audio hub mode",
					ARRAY_SIZE(codec_format_function), codec_format_function,
					SUNXI_DAC_DPC, DAC_HUB_EN),
	SOUND_CTRL_ADF_CONTROL_USER("Soft Volume Master", 255, 0, 255),
};

static void adchpf_config(struct sunxi_sound_adf_component *component)
{
	sunxi_sound_component_write(component, AC_ADC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LHPFC, 0xFFE644 & 0xFFFF);
}

static void adchpf_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN),
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN),
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_HPF0_EN | 0x1 << ADC_HPF1_EN),
			(0x0 << ADC_HPF0_EN | 0x0 << ADC_HPF1_EN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN),
			(0x0 << ADC_DAP0_EN | 0x0 << ADC_DAP1_EN));
	}
}

static void adcdrc_config(struct sunxi_sound_adf_component *component)
{
	/* Left peak filter attack time */
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFLAT, 0x000B77BF & 0xFFFF);
	/* Left peak filter release time */
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LPFLAT, 0x00012BAF & 0xFFFF);
	/* Right RMS filter attack time */
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_RPFLAT, 0x00012BAF & 0xFFFF);

	/* smooth filter attack time */
	sunxi_sound_component_write(component, AC_ADC_DRC_SFHAT, (0x00025600 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_SFLAT, 0x00025600 & 0xFFFF);
	/* gain smooth filter release time */
	sunxi_sound_component_write(component, AC_ADC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	sunxi_sound_component_write(component, AC_ADC_DRC_HOPL, (0xFBD8FBA7 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LOPL, 0xFBD8FBA7 & 0xFFFF);
	/* OPC */
	sunxi_sound_component_write(component, AC_ADC_DRC_HOPC, (0xF95B2C3F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LOPC, 0xF95B2C3F & 0xFFFF);
	/* OPE */
	sunxi_sound_component_write(component, AC_ADC_DRC_HOPE, (0xF45F8D6E >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LOPE, 0xF45F8D6E & 0xFFFF);
	/* LT */
	sunxi_sound_component_write(component, AC_ADC_DRC_HLT, (0x01A934F0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LLT, 0x01A934F0 & 0xFFFF);
	/* CT */
	sunxi_sound_component_write(component, AC_ADC_DRC_HCT, (0x06A4D3C0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LCT, 0x06A4D3C0 & 0xFFFF);
	/* ET */
	sunxi_sound_component_write(component, AC_ADC_DRC_HET, (0x0BA07291 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LET, 0x0BA07291 & 0xFFFF);
	/* Ki */
	sunxi_sound_component_write(component, AC_ADC_DRC_HKI, (0x00051EB8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LKI, 0x00051EB8 & 0xFFFF);
	/* Kc */
	sunxi_sound_component_write(component, AC_ADC_DRC_HKC, (0x00800000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LKC, 0x00800000 & 0xFFFF);
	/* Kn */
	sunxi_sound_component_write(component, AC_ADC_DRC_HKN, (0x01000000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LKN, 0x01000000 & 0xFFFF);
	/* Ke */
	sunxi_sound_component_write(component, AC_ADC_DRC_HKE, (0x0000F45F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_ADC_DRC_LKE, 0x0000F45F & 0xFFFF);
}

static void adcdrc_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN),
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN),
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DAP0_EN | 0x1 << ADC_DAP1_EN),
			(0x0 << ADC_DAP0_EN | 0x0 << ADC_DAP1_EN));
		sunxi_sound_component_update_bits(component, SUNXI_ADC_DAP_CTL,
			(0x1 << ADC_DRC0_EN | 0x1 << ADC_DRC1_EN),
			(0x0 << ADC_DRC0_EN | 0x0 << ADC_DRC1_EN));
	}
}

static void dachpf_config(struct sunxi_sound_adf_component *component)
{
	/* HPF */
	sunxi_sound_component_write(component, AC_DAC_DRC_HHPFC, (0xFFFAC1 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LHPFC, 0xFFFAC1 & 0xFFFF);
}

static void dachpf_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x1 << DDAP_HPF_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_EN),
			(0x1 << DDAP_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_EN),
			(0x0 << DDAP_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_HPF_EN),
			(0x0 << DDAP_HPF_EN));
	}
}

static void dacdrc_config(struct sunxi_sound_adf_component *component)
{
	/* Left peak filter attack time */
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFLAT, 0x000B77BF & 0xFFFF);
	/* Right peak filter attack time */
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFHAT, (0x000B77BF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFLAT, 0x000B77BF & 0xFFFF);
	/* Left peak filter release time */
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFLRT, 0x00FFE1F8 & 0xFFFF);
	/* Right peak filter release time */
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFHRT, (0x00FFE1F8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFLRT, 0x00FFE1F8 & 0xFFFF);

	/* Left RMS filter attack time */
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LPFLAT, 0x00012BAF & 0xFFFF);
	/* Right RMS filter attack time */
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFHAT, (0x00012BAF >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_RPFLAT, 0x00012BAF & 0xFFFF);

	/* smooth filter attack time */
	sunxi_sound_component_write(component, AC_DAC_DRC_SFHAT, (0x00025600 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_SFLAT, 0x00025600 & 0xFFFF);
	/* gain smooth filter release time */
	sunxi_sound_component_write(component, AC_DAC_DRC_SFHRT, (0x00000F04 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_SFLRT, 0x00000F04 & 0xFFFF);

	/* OPL */
	sunxi_sound_component_write(component, AC_DAC_DRC_HOPL, (0xFBD8FBA7 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LOPL, 0xFBD8FBA7 & 0xFFFF);
	/* OPC */
	sunxi_sound_component_write(component, AC_DAC_DRC_HOPC, (0xF95B2C3F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LOPC, 0xF95B2C3F & 0xFFFF);
	/* OPE */
	sunxi_sound_component_write(component, AC_DAC_DRC_HOPE, (0xF45F8D6E >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LOPE, 0xF45F8D6E & 0xFFFF);
	/* LT */
	sunxi_sound_component_write(component, AC_DAC_DRC_HLT, (0x01A934F0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LLT, 0x01A934F0 & 0xFFFF);
	/* CT */
	sunxi_sound_component_write(component, AC_DAC_DRC_HCT, (0x06A4D3C0 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LCT, 0x06A4D3C0 & 0xFFFF);
	/* ET */
	sunxi_sound_component_write(component, AC_DAC_DRC_HET, (0x0BA07291 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LET, 0x0BA07291 & 0xFFFF);
	/* Ki */
	sunxi_sound_component_write(component, AC_DAC_DRC_HKI, (0x00051EB8 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LKI, 0x00051EB8 & 0xFFFF);
	/* Kc */
	sunxi_sound_component_write(component, AC_DAC_DRC_HKC, (0x00800000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LKC, 0x00800000 & 0xFFFF);
	/* Kn */
	sunxi_sound_component_write(component, AC_DAC_DRC_HKN, (0x01000000 >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LKN, 0x01000000 & 0xFFFF);
	/* Ke */
	sunxi_sound_component_write(component, AC_DAC_DRC_HKE, (0x0000F45F >> 16) & 0xFFFF);
	sunxi_sound_component_write(component, AC_DAC_DRC_LKE, 0x0000F45F & 0xFFFF);
}

static void dacdrc_enable(struct sunxi_sound_adf_component *component, bool on)
{
	if (on) {
		/* detect noise when ET enable */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_CONTROL_DRC_EN),
			(0x1 << DAC_DRC_CTL_CONTROL_DRC_EN));
		/* 0x0:RMS filter; 0x1:Peak filter */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_SIGNAL_FUN_SEL),
			(0x1 << DAC_DRC_CTL_SIGNAL_FUN_SEL));
		/* delay function enable */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DEL_FUN_EN),
			(0x0 << DAC_DRC_CTL_DEL_FUN_EN));

		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DRC_LT_EN),
			(0x1 << DAC_DRC_CTL_DRC_LT_EN));
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DRC_ET_EN),
			(0x1 << DAC_DRC_CTL_DRC_ET_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x1 << DDAP_DRC_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_EN),
			(0x1 << DDAP_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_EN),
			(0x0 << DDAP_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DAC_DAP_CTL,
			(0x1 << DDAP_DRC_EN),
			(0x0 << DDAP_DRC_EN));

		/* detect noise when ET enable */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_CONTROL_DRC_EN),
			(0x0 << DAC_DRC_CTL_CONTROL_DRC_EN));
		/* 0x0:RMS filter; 0x1:Peak filter */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_SIGNAL_FUN_SEL),
			(0x1 << DAC_DRC_CTL_SIGNAL_FUN_SEL));
		/* delay function enable */
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DEL_FUN_EN),
			(0x0 << DAC_DRC_CTL_DEL_FUN_EN));

		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DRC_LT_EN),
			(0x0 << DAC_DRC_CTL_DRC_LT_EN));
		sunxi_sound_component_update_bits(component, AC_DAC_DRC_CTL,
			(0x1 << DAC_DRC_CTL_DRC_ET_EN),
			(0x0 << DAC_DRC_CTL_DRC_ET_EN));
	}
}

static int sunxi_codec_init(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

	/* Disable DRC function for playback */
	sunxi_sound_component_write(component, SUNXI_DAC_DAP_CTL, 0);
	/* Disable HPF(high passed filter) */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
                        (1 << HPF_EN), (0x0 << HPF_EN));
	/* Enable ADCFDT to overcome niose at the beginning */
	sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
                        (7 << ADCDFEN), (7 << ADCDFEN));
	/* set digital volume */
	sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
			(0x3f << DVOL), (param->dac_vol << DVOL));
	/* set LINEOUT volume */
	sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL1,
			(0x1f << LINEOUT_VOL),
			(param->lineout_vol << LINEOUT_VOL));
	/* set MIC2,3 Boost AMP disable */
	sunxi_sound_component_write(component, SUNXI_MIC2_MIC3_CTL, 0x44);
	/* set LADC Mixer mute */
	sunxi_sound_component_write(component, SUNXI_LADCMIX_SRC, 0x0);
	/* MIC1 AMP gain */
	sunxi_sound_component_update_bits(component, SUNXI_MIC1_CTL,
                        0x7 << MIC1BOOST,
                        param->mic1_gain << MIC1BOOST);
	/* MIC2,MIC3 AMP gain */
	sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
                        0x7 << MIC2BOOST,
                        param->mic2_gain << MIC2BOOST);
	sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
                        0x7 << MIC3BOOST,
                        param->mic3_gain << MIC3BOOST);
	/* adc gain */
	sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
                        0x1f << ADCG,
                        param->adc_gain << ADCG);

	/* LINEOUT Mux */
	sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
			(0x1<<LINEOUTL_SRC), (0x0<<LINEOUTL_SRC));
	sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
			(0x1<<LINEOUTR_SRC), (0x1<<LINEOUTR_SRC));

	/* Left Input Mixer */
	sunxi_sound_component_update_bits(component, SUNXI_LADCMIX_SRC,
			(0x1<<LADC_MIC1_STAGE), (0x1<<LADC_MIC1_STAGE));
	/* Right Input Mixer */
	sunxi_sound_component_update_bits(component, SUNXI_RADCMIX_SRC,
			(0x1<<RADC_MIC2_STAGE), (0x1<<RADC_MIC2_STAGE));
	/* X Input Mixer */
	sunxi_sound_component_update_bits(component, SUNXI_XADCMIX_SRC,
			(0x1<<XADC_MIC3_STAGE), (0x1<<XADC_MIC3_STAGE));

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

	return 0;
}

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
	reg_val = sunxi_sound_component_read(component, SUNXI_ADC_FIFO_CTL);

	if (reg_val & (1<<ADC_CHAN_SEL) || codec_param->mic1_en) {
		codec_param->adc1_en = 1;
	}

	codec_param->adc2_en = 0;
	if (reg_val & (2 << ADC_CHAN_SEL) || codec_param->mic2_en) {
		codec_param->adc2_en = 1;
	}

	codec_param->adc3_en = 0;
	if (reg_val & (4 << ADC_CHAN_SEL) || codec_param->mic3_en) {
		codec_param->adc3_en = 1;
	}

	if (codec_param->adc1_en || codec_param->adc2_en ||
		codec_param->adc3_en)
		return 0;

	return -1;
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

	ret = hal_clock_enable(clk->clk_pll_audio4x);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_pll_audio4x enable failed.\n");
		goto err_enable_clk_pll_audio4x;
	}

	ret = hal_clock_enable(clk->clk_audio);
	if (ret != HAL_CLK_STATUS_OK) {
		snd_err("codec clk_audio enable failed.\n");
		goto err_enable_clk_audio;
	}

	return HAL_CLK_STATUS_OK;

err_enable_clk_audio:
	hal_clock_disable(clk->clk_pll_audio4x);
err_enable_clk_pll_audio4x:
	hal_clock_disable(clk->clk_pll_audio);
err_enable_clk_pll_audio:
	return HAL_CLK_STATUS_ERROR;
}

static void snd_sunxi_clk_disable(struct sunxi_codec_clk *clk)
{
	snd_debug("\n");

	hal_clock_disable(clk->clk_audio);
	hal_clock_disable(clk->clk_pll_audio);
	hal_clock_disable(clk->clk_pll_audio4x);

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

	/* module dac clk */
	clk->clk_pll_audio4x = hal_clock_get(HAL_SUNXI_CCU, HAL_CLK_PLL_AUDIOX4);
	if (!clk->clk_pll_audio4x) {
		snd_err("codec clk_pll_audio4x hal_clock_get failed\n");
		goto err_get_clk_pll_audio4x;
	}

	/* module audiocodec clk */
	clk->clk_audio = hal_clock_get(HAL_SUNXI_CCU, HAL_CLK_PERIPH_AUDIOCODEC_1X);
	if (!clk->clk_audio) {
		snd_err("codec clk_audio hal_clock_get failed\n");
		goto err_get_clk_audio;
	}

	ret = hal_clk_set_parent(clk->clk_audio, clk->clk_pll_audio4x);
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
err_set_rate_clk:
err_set_parent_clk:
	hal_clock_put(clk->clk_audio);
err_get_clk_audio:
	hal_clock_put(clk->clk_pll_audio4x);
err_get_clk_pll_audio4x:
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
	hal_clock_put(clk->clk_pll_audio4x);


	return;
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
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
				(3<<FIFO_MODE), (3<<FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
				(1<<TX_SAMPLE_BITS), (0<<TX_SAMPLE_BITS));
		} else {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
				(1<<RX_FIFO_MODE), (1<<RX_FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
				(1<<RX_SAMPLE_BITS), (0<<RX_SAMPLE_BITS));
		}
		break;
	case	SND_PCM_FORMAT_S32_LE:
		/* only for the compatible of tinyalsa */
	case	SND_PCM_FORMAT_S24_LE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
				(3<<FIFO_MODE), (0<<FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
				(1<<TX_SAMPLE_BITS), (1<<TX_SAMPLE_BITS));
		} else {
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
				(1<<RX_FIFO_MODE), (0<<RX_FIFO_MODE));
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
				(1<<RX_SAMPLE_BITS), (1<<RX_SAMPLE_BITS));
		}
		break;
	default:
		snd_err("params_format[%d] error!\n", params_format(params));
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(sample_rate_conv); i++) {
		if (sample_rate_conv[i].samplerate == params_rate(params)) {
			if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
				sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
					(0x7<<DAC_FS),
					(sample_rate_conv[i].rate_bit<<DAC_FS));
			} else {
				if (sample_rate_conv[i].samplerate > 48000)
					return -EINVAL;
				sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
					(0x7<<ADC_FS),
					(sample_rate_conv[i].rate_bit<<ADC_FS));
			}
		}
	}
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (params_channels(params)) {
		case 1:
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
					(1<<DAC_MONO_EN), 1<<DAC_MONO_EN);
			break;
		case 2:
			sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
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
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x1<<ADC_CHAN_SEL,
											0x1<<ADC_CHAN_SEL);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x1<<ADC_CHAN_SEL,
											0x0<<ADC_CHAN_SEL);

		if (codec_param->adc2_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x2<<ADC_CHAN_SEL,
											0x2<<ADC_CHAN_SEL);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x2<<ADC_CHAN_SEL,
											0x0<<ADC_CHAN_SEL);

		if (codec_param->adc3_en)
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x4<<ADC_CHAN_SEL,
											0x4<<ADC_CHAN_SEL);
		else
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
											0x4<<ADC_CHAN_SEL,
											0x0<<ADC_CHAN_SEL);

	}
	return 0;
}

static void sunxi_codec_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{

	return;
}

static int sunxi_codec_dapm_control(struct sunxi_sound_adf_component *component,
			struct sunxi_sound_pcm_dataflow *dataflow, int onoff)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;
	struct sunxi_pa_config *pa_cfg = &sunxi_codec->pa_cfg[0];
	struct sunxi_pa_config *pa_power_cfg = &sunxi_codec->pa_cfg[1];
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
			sunxi_sound_component_update_bits(component, SUNXI_MIX_DAC_CTL,
					(0x1<<DACALEN), (0x1<<DACALEN));
			/* digital DAC enable */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x1<<EN_DAC));
			/* delay 10ms to avoid digitabl DAC square wave */
			hal_msleep(10);
			/* LINEOUT */
			sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
				(0x1<<LINEOUTL_EN), (0x1<<LINEOUTL_EN));
			sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
				(0x1<<LINEOUTR_EN), (0x1<<LINEOUTR_EN));
			#if AW87579_ANALOG_PA
			aw87579_write(&sunxi_codec->param.twi_dev, AW87579_REG_SYSCTRL, 0x78);
			#else
			if (pa_cfg->gpio > 0) {
				ret = sunxi_sound_pa_enable(pa_cfg);
				if (ret) {
					snd_err("pa pin enable failed!\n");
				}
			}
			if (pa_power_cfg->gpio > 0) {
				ret = sunxi_sound_pa_enable(pa_power_cfg);
				if (ret) {
					snd_err("pa pin enable failed!\n");
				}
			}
			#endif
			/* delay to wait PA stable */
			//hal_msleep(param->pa_msleep_time);
		} else {
			/* Playback off */
			#if AW87579_ANALOG_PA
			aw87579_write(&sunxi_codec->param.twi_dev, AW87579_REG_SYSCTRL, 0x01);
			#else
			if (pa_cfg->gpio > 0) {
				sunxi_sound_pa_disable(pa_cfg);
			}
			if (pa_power_cfg->gpio > 0) {
				sunxi_sound_pa_disable(pa_power_cfg);
			}
			#endif
			//hal_msleep(param->pa_msleep_time);
			/* LINEOUT */
			sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
				(0x1<<LINEOUTL_EN), (0x0<<LINEOUTL_EN));
			sunxi_sound_component_update_bits(component, SUNXI_LINEOUT_CTL0,
				(0x1<<LINEOUTR_EN), (0x0<<LINEOUTR_EN));
			/* digital DAC */
			sunxi_sound_component_update_bits(component, SUNXI_DAC_DPC,
					(0x1<<EN_DAC), (0x0<<EN_DAC));
			/* analog DAC */
			sunxi_sound_component_update_bits(component, SUNXI_MIX_DAC_CTL,
					(0x1<<DACALEN), (0x0<<DACALEN));
		}
	} else {
		/*
		 * Capture:
		 * Capture <-- ADCL <-- Left Input Mixer <-- MIC1 PGA <-- MIC1 <-- MainMic Bias
		 * Capture <-- ADCR <-- Right Input Mixer <-- MIC2 PGA <-- MIC2 <-- MainMic Bias
		 * Capture <-- ADCX <-- Xadc Input Mixer <-- MIC3 PGA <-- MIC3 <-- MainMic Bias
		 *
		 */
		unsigned int channels = 0;
		channels = dataflow->pcm_running->channels;

		snd_debug("channels = %u\n", channels);
		if (onoff) {
			/* Capture on */
			/* digital ADC enable */
#ifndef CONFIG_SND_MULTI_SOUNDCARD
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
					(0x1<<EN_AD), (0x1<<EN_AD));
#endif
			switch (channels) {
			case 4:
			case 3:
				/* analog ADCX enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCXEN), (0x1<<ADCXEN));
			case 2:
				/* analog ADCR enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCREN), (0x1<<ADCREN));
			case 1:
				/* analog ADCL enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCLEN), (0x1<<ADCLEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
			}
			switch (channels) {
			case 4:
			case 3:
				/* MIC3 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
					(0x1<<MIC3AMPEN), (0x1<<MIC3AMPEN));
			case 2:
				/* MIC2 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
					(0x1<<MIC2AMPEN), (0x1<<MIC2AMPEN));
			case 1:
				/* MIC1 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC1_CTL,
					(0x1<<MIC1AMPEN), (0x1<<MIC1AMPEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
			}
			/* MainMic Bias */
			sunxi_sound_component_update_bits(component, SUNXI_MBIAS_CTL,
					(0x1<<MMICBIASEN), (0x1<<MMICBIASEN));
		} else {
			/* Capture off */
			/* MainMic Bias */
			sunxi_sound_component_update_bits(component, SUNXI_MBIAS_CTL,
					(0x1<<MMICBIASEN), (0x0<<MMICBIASEN));
			switch (channels) {
			case 4:
			case 3:
				/* MIC3 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
					(0x1<<MIC3AMPEN), (0x0<<MIC3AMPEN));
			case 2:
				/* MIC2 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC2_MIC3_CTL,
					(0x1<<MIC2AMPEN), (0x0<<MIC2AMPEN));
			case 1:
				/* MIC1 PGA */
				sunxi_sound_component_update_bits(component, SUNXI_MIC1_CTL,
					(0x1<<MIC1AMPEN), (0x0<<MIC1AMPEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
			}
			switch (channels) {
			case 4:
			case 3:
				/* analog ADCX enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCXEN), (0x0<<ADCXEN));
			case 2:
				/* analog ADCR enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCREN), (0x0<<ADCREN));
			case 1:
				/* analog ADCL enable */
				sunxi_sound_component_update_bits(component, SUNXI_ADC_CTL,
						(0x1<<ADCLEN), (0x0<<ADCLEN));
				break;
			default:
				snd_err("unknown channels:%u\n", channels);
				return -1;
			}
#ifndef CONFIG_SND_MULTI_SOUNDCARD
			/* digital ADC enable */
			sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
					(0x1<<EN_AD), (0x0<<EN_AD));
#endif
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

static int sunxi_codec_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("cmd=%d\n", cmd);
	switch (cmd) {
	case	SNDRV_PCM_TRIGGER_START:
	case	SNDRV_PCM_TRIGGER_RESUME:
	case	SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sunxi_sound_component_update_bits(component,
					SUNXI_DAC_FIFO_CTL,
					(1<<DAC_DRQ_EN), (1<<DAC_DRQ_EN));
		else {
#ifndef CONFIG_SND_MULTI_SOUNDCARD
			sunxi_sound_component_update_bits(component,
					SUNXI_ADC_FIFO_CTL,
					(1 << ADC_DRQ_EN), (1 << ADC_DRQ_EN));
#endif
		}
		break;
	case	SNDRV_PCM_TRIGGER_STOP:
	case	SNDRV_PCM_TRIGGER_SUSPEND:
	case	SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK)
			sunxi_sound_component_update_bits(component,
					SUNXI_DAC_FIFO_CTL,
					(1 << DAC_DRQ_EN), (0 << DAC_DRQ_EN));
		else {
#ifndef CONFIG_SND_MULTI_SOUNDCARD
			sunxi_sound_component_update_bits(component,
					SUNXI_ADC_FIFO_CTL,
					(1 << ADC_DRQ_EN), (0 << ADC_DRQ_EN));
#endif
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int sunxi_codec_prepare(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_sound_component_update_bits(component, SUNXI_DAC_FIFO_CTL,
				(1<<FIFO_FLUSH), (1<<FIFO_FLUSH));
		sunxi_sound_component_write(component, SUNXI_DAC_FIFO_STA,
			(1 << DAC_TXE_INT | 1 << DAC_TXU_INT | 1 << DAC_TXO_INT));
		sunxi_sound_component_write(component, SUNXI_DAC_CNT, 0);
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_ADC_FIFO_CTL,
			(1<<ADC_FIFO_FLUSH), (1<<ADC_FIFO_FLUSH));
		sunxi_sound_component_write(component, SUNXI_ADC_FIFO_STA,
			(1 << ADC_RXA_INT | 1 << ADC_RXO_INT));
		sunxi_sound_component_write(component, SUNXI_ADC_CNT, 0);
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

static struct sunxi_sound_adf_dai_ops sun8iw18_codec_dai_ops = {
	.startup	= sunxi_codec_startup,
	.hw_params	= sunxi_codec_hw_params,
	.shutdown	= sunxi_codec_shutdown,
	.set_pll	= sunxi_codec_dai_set_pll,		/* set pllclk */
	.trigger	= sunxi_codec_trigger,
	.prepare	= sunxi_codec_prepare,
};

static struct sunxi_sound_adf_dai_driver sunxi_codec_dai = {
	.name = DRV_NAME,
	.playback	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min       = 8000,
		.rate_max       = 192000,
	},
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 4,
		.rates		= SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE,
		.rate_min       = 8000,
		.rate_max       = 48000,
	},
	.ops		= &sun8iw18_codec_dai_ops,
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

static int sun8iw18_codec_probe(struct sunxi_sound_adf_component *component)
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
	sunxi_sound_pa_init(&sunxi_codec->pa_cfg[0], &default_pa_cfg, CODEC);
	sunxi_sound_pa_init(&sunxi_codec->pa_cfg[1], &default_pa_power_cfg, CODEC);
	sunxi_sound_pa_disable(&sunxi_codec->pa_cfg[0]);
	sunxi_sound_pa_disable(&sunxi_codec->pa_cfg[1]);

	component->addr_base = (void *)SUNXI_CODEC_BASE_ADDR;

	ret = snd_sunxi_clk_init(&sunxi_codec->clk);
	if (ret != 0) {
		snd_err("snd_sunxi_clk_init failed\n");
		goto err_codec_set_clock;
	}

	sunxi_codec_init(component);

#if AW87579_ANALOG_PA
	snd_debug("init aw87579 i2c port.\n");
	int ret = aw87579_init_i2c_device(sunxi_codec->param.twi_dev.bus);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err\n");
		return -ENOMEM;
	}
#endif

	return 0;

err_codec_set_clock:
	snd_sunxi_clk_exit(&sunxi_codec->clk);

	return -1;

}

static void sun8iw18_codec_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_codec_info *sunxi_codec = component->private_data;
	struct sunxi_codec_param *param = &sunxi_codec->param;

#if AW87579_ANALOG_PA
	snd_debug("deinit aw87579 i2c port.\n");
	int ret = aw87579_deinit_i2c_device(sunxi_codec->param.twi_dev.bus);
	if (ret != TWI_STATUS_OK) {
		snd_err("i2c deinit port %d failed.\n", sunxi_codec->param.twi_dev.bus);
	}
#endif

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
	.probe		= sun8iw18_codec_probe,
	.remove		= sun8iw18_codec_remove,
	.read           = sunxi_codec_read,
	.write          = sunxi_codec_write,
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


