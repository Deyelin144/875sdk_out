#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <sound_v2/sunxi_adf_core.h>
#include <sound_v2/sunxi_sound_pcm_common.h>
#include "sunxi_sound_common.h"

#include <hal_cmd.h>
#include "ac101s.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-codec-ac101s"
#define DRV_NAME			"sunxi-snd-codec-ac101s-dai"

struct realval_to_regval {
	unsigned int real_val;
	unsigned int reg_val;
};

static const struct realval_to_regval g_n_div[] = {
	{1,  0},
	{2,  1},
	{3,  2},
	{4,  3},
	{6,  4},
	{8,  5},
	{12, 6},
	{16, 7},
	{24, 8},
};

static const struct realval_to_regval g_bclk_div[] = {
	{0,  0},
	{1,  1},
	{2,  2},
	{4,  3},
	{6,  4},
	{8,  5},
	{12, 6},
	{16, 7},
	{24, 8},
	{32, 9},
	{48, 10},
	{64, 11},
	{96, 12},
	{128,13},
	{176,14},
	{192,15},
};

static twi_status_t sunxi_ac101s_init_i2c_device(twi_port_t port)
{
	twi_status_t ret = 0;

	ret = hal_twi_init(port);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err ret=%d.\n", ret);
		return ret;
	}

	return TWI_STATUS_OK;
}

static twi_status_t sunxi_ac101s_deinit_i2c_device(twi_port_t port)
{
	twi_status_t ret = 0;

	ret = hal_twi_uninit(port);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err ret=%d.\n", ret);
		return ret;
	}

	return TWI_STATUS_OK;
}

static twi_status_t sunxi_ac101s_read(struct twi_device *twi_dev,
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

static int sunxi_ac101s_write(struct twi_device *twi_dev,
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

static int sunxi_ac101s_update_bits(struct twi_device *twi_dev,
			      unsigned char reg, unsigned char mask, unsigned char value)
{
	unsigned char val_old = 0;
	unsigned char val_new = 0;

	sunxi_ac101s_read(twi_dev, reg, &val_old);
	val_new = (val_old & ~mask) | (value & mask);
	if (val_new != val_old)
		sunxi_ac101s_write(twi_dev, reg, val_new);

	return 0;
}

/*static const char *sunxi_switch_text[] = {"Off", "On"};*/
static const char *sunxi_differ_text[] = {"differ", "single"};

static int sunxi_ac101s_ctl_enum_value_get(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;
	unsigned int val = 0;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid kcontrol type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	sunxi_ac101s_read(&param->twi_dev, adf_control->reg, (unsigned char *)&val);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, val);

	return 0;
}

static int sunxi_ac101s_ctl_enum_value_set(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_ENUMERATED) {
		snd_err("invalid adf_control type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	if (info->value >= adf_control->items) {
		snd_err("invalid adf_control items = %ld.\n", info->value);
		return -EINVAL;
	}

	sunxi_ac101s_update_bits(&param->twi_dev, adf_control->reg,
			(adf_control->mask << adf_control->shift),
			((unsigned int)info->value << adf_control->shift));

	adf_control->value = info->value & adf_control->mask;

	snd_info("mask:0x%x, shift:%d, value:0x%lx\n", adf_control->mask, adf_control->shift, adf_control->value);

	return 0;
}

static int sunxi_ac101s_ctl_value_get(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	unsigned int val = 0;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_INTEGER) {
		snd_err("invalid adf_control type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	sunxi_ac101s_read(&param->twi_dev, adf_control->reg, (unsigned char *)&val);

	sunxi_sound_adf_control_to_sound_ctrl_info(adf_control, info, val);

	return 0;
}

static int sunxi_ac101s_ctl_value_set(struct sunxi_sound_adf_control *adf_control, struct sunxi_sound_control_info *info)
{
	struct sunxi_sound_adf_component *component = adf_control->private_data;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	if (adf_control->type != SUNXI_SOUND_CTRL_PART_TYPE_INTEGER) {
		snd_err("invalid adf_control type = %d.\n", adf_control->type);
		return -EINVAL;
	}

	sunxi_ac101s_update_bits(&param->twi_dev, adf_control->reg,
			(adf_control->mask << adf_control->shift),
			((unsigned int)info->value << adf_control->shift));

	snd_info("mask:0x%x, shitf:%d, value:0x%lx\n",
			adf_control->mask, adf_control->shift, info->value);
	return 0;
}

static struct sunxi_sound_adf_control_new sunxi_ac101s_codec_controls[] = {
	SOUND_CTRL_ENUM_VALUE_EXT("LINEOUT Output Select",
			ARRAY_SIZE(sunxi_differ_text), sunxi_differ_text,
			DAC_ANA_CTRL2, LINEODIFEN, SUNXI_SOUND_CTRL_PART_AUTO_MASK,
			sunxi_ac101s_ctl_enum_value_get, sunxi_ac101s_ctl_enum_value_set),

	SOUND_CTRL_ADF_CONTROL_VALUE_EXT("ADC volume",
			ADC_DVC, ADC_DVC_VOL, 255, 0,
			sunxi_ac101s_ctl_value_get, sunxi_ac101s_ctl_value_set),
	SOUND_CTRL_ADF_CONTROL_VALUE_EXT("DAC volume",
			DAC_DVC, DAC_DVC_VOL, 255, 0,
			sunxi_ac101s_ctl_value_get, sunxi_ac101s_ctl_value_set),
	SOUND_CTRL_ADF_CONTROL_VALUE_EXT("MIC gain volume",
			ADC_ANA_CTRL1, PGA_GAIN_CTRL, 31, 0,
			sunxi_ac101s_ctl_value_get, sunxi_ac101s_ctl_value_set),
	SOUND_CTRL_ADF_CONTROL_VALUE_EXT("LINEOUT volume",
			DAC_ANA_CTRL2, LINEOAMPGAIN, 15, 0,
			sunxi_ac101s_ctl_value_get, sunxi_ac101s_ctl_value_set),

};

static void sunxi_ac101s_codec_init(struct twi_device *twi_dev, struct ac101s_param *param)
{
	snd_debug("\n");

	/* reset */
	sunxi_ac101s_update_bits(&param->twi_dev, CHIP_SOFT_RST, (0xFF<<0), (0x34<<0));

	/* 0x73: ALDOOUT=1.8V, DLDOOUT=1.2V, MICBISA=2.39V */
	sunxi_ac101s_write(&param->twi_dev, PWR_CTRL1, 0x73);
	/* 0x1B: ALDO/DLDO/VREF/IREF Enable, MICBIAS Disable */
	sunxi_ac101s_write(&param->twi_dev, PWR_CTRL2, 0x1b);

	/* disable DAC play */
	sunxi_ac101s_update_bits(&param->twi_dev, SYS_FUNC_CTRL, (0x1<<DAC_PLAY_FUNC_EN), (0x0<<DAC_PLAY_FUNC_EN));

	/* ADC ana gain 31dB */
	sunxi_ac101s_update_bits(&param->twi_dev, ADC_ANA_CTRL1, (0x1F<<PGA_GAIN_CTRL), (0x1F<<PGA_GAIN_CTRL));
	/* ADC dig vol 0dB */
	sunxi_ac101s_update_bits(&param->twi_dev, ADC_DVC, (0xFF<<ADC_DVC_VOL), (0x81<<ADC_DVC_VOL));
}

static int sunxi_ac101s_codec_dai_startup(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");

	/* NULL */

	return 0;
}

static int sunxi_ac101s_codec_dai_set_sysclk(struct sunxi_sound_adf_dai *dai, int clk_id, unsigned int freq, int dir)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	snd_debug("\n");

	/* SYSCLK from mclk, support upto 13MHz */
	sunxi_ac101s_update_bits(&param->twi_dev, SYS_CLK_ENA, 1<<SYSCLK_EN, 1<<SYSCLK_EN);

	param->sysclk_freq = freq;

	return 0;
}

static int sunxi_ac101s_codec_dai_set_clkdiv(struct sunxi_sound_adf_dai *dai, int div_id, int div)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	int i;
	unsigned int n_div;
	unsigned int n_div_regval = 0;

	snd_debug("\n");

	/* note:
	 * 1. ac101s sysclk is (12288000 or 11289600)
	 * 2. div_id = (24576000 or 22579200) / sample_rate
	 */

	/* ADC_CLK & DAC_CLK must 128*FS */
	n_div = div / 256;
	for (i = 0; i < ARRAY_SIZE(g_n_div); i++) {
		if (g_n_div[i].real_val == n_div) {
			n_div_regval = g_n_div[i].reg_val;
			snd_debug("AC101S set N_DIV %u\n", n_div);
			break;
		}
	}
	if (i == ARRAY_SIZE(g_n_div)) {
		snd_err("AC101S don't support N_DIV %u\n", n_div);
		return -EINVAL;
	}
	sunxi_ac101s_update_bits(&param->twi_dev, ADC_CLK_SET, (0xF<<NADC), (n_div_regval<<NADC));
	sunxi_ac101s_update_bits(&param->twi_dev, DAC_CLK_SET, (0xF<<NDAC), (n_div_regval<<NDAC));


	return 0;
}

static int sunxi_ac101s_codec_dai_set_fmt(struct sunxi_sound_adf_dai *dai, unsigned int fmt)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	unsigned char tx_offset = 0;
	unsigned char i2s_mode = 0;
	unsigned char lrck_polarity = 0;
	unsigned char brck_polarity = 0;

	snd_debug("\n");

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:	/* AC101S Master BCLK & LRCK output */
		snd_debug("AC101S set to work as Master\n");
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_CTRL, 0x3 << LRCK_IOEN, 0x3 << LRCK_IOEN);
	break;
	case SND_SOC_DAIFMT_CBS_CFS:	/* AC101S Slave BCLK & LRCK input */
		snd_debug("AC101S set to work as Slave\n");
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_CTRL, 0x3 << LRCK_IOEN, 0x0 << LRCK_IOEN);
	break;
	default:
		snd_err("AC101S Master/Slave mode config error:%u\n\n",
			(fmt & SND_SOC_DAIFMT_MASTER_MASK) >> 12);
	return -EINVAL;
	}

	/* AC101S config I2S/LJ/RJ/PCM format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		snd_debug("AC101S config I2S format\n");
		i2s_mode = 1;
		tx_offset = 1;
	break;
	case SND_SOC_DAIFMT_RIGHT_J:
		snd_debug("AC101S config RIGHT-JUSTIFIED format\n");
		i2s_mode = 2;
		tx_offset = 0;
	break;
	case SND_SOC_DAIFMT_LEFT_J:
		snd_debug("AC101S config LEFT-JUSTIFIED format\n");
		i2s_mode = 1;
		tx_offset = 0;
	break;
	case SND_SOC_DAIFMT_DSP_A:
		snd_debug("AC101S config PCM-A format\n");
		i2s_mode = 0;
		tx_offset = 1;
	break;
	case SND_SOC_DAIFMT_DSP_B:
		snd_debug("AC101S config PCM-B format\n");
		i2s_mode = 0;
		tx_offset = 0;
	break;
	default:
		snd_err("AC101S I2S format config error:%u\n\n",
			fmt & SND_SOC_DAIFMT_FORMAT_MASK);
	return -EINVAL;
	}

	sunxi_ac101s_update_bits(&param->twi_dev, I2S_FMT_CTRL1,
			   0x3 << MODE_SEL | 0x1 << OFFSET,
			   i2s_mode<<MODE_SEL | tx_offset<<OFFSET);

	/* AC101S config BCLK&LRCK polarity */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		snd_debug("AC101S config: BCLK_normal,LRCK_normal.\n");
		brck_polarity = 0;
		lrck_polarity = 0;
	break;
	case SND_SOC_DAIFMT_NB_IF:
		snd_debug("AC101S config: BCLK_normal,LRCK_invert.\n");
		brck_polarity = 0;
		lrck_polarity = 1;
	break;
	case SND_SOC_DAIFMT_IB_NF:
		snd_debug("AC101S config: BCLK_invert,LRCK_normal.\n");
		brck_polarity = 1;
		lrck_polarity = 0;
	break;
	case SND_SOC_DAIFMT_IB_IF:
		snd_debug("AC101S config: BCLK_invert,LRCK_invert.\n");
		brck_polarity = 1;
		lrck_polarity = 1;
		break;
	default:
		snd_err("AC101S config BCLK/LRCLK polarity error:%u\n",
			(fmt & SND_SOC_DAIFMT_INV_MASK) >> 8);
	return -EINVAL;
	}

	sunxi_ac101s_update_bits(&param->twi_dev, I2S_BCLK_CTRL,  0x1<<BCLK_POLARITY, brck_polarity<<BCLK_POLARITY);
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_LRCK_CTRL1, 0x1<<LRCK_POLARITY, lrck_polarity<<LRCK_POLARITY);

	param->format = fmt;

	return 0;
}

static int sunxi_ac101s_codec_dai_set_tdm_slot(struct sunxi_sound_adf_dai *dai,
				      unsigned int tx_mask, unsigned int rx_mask,
				      int slots, int slot_width)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;
	unsigned int lrck_width_map;

	snd_debug("\n");

	/* slot width */
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_FMT_CTRL2, 0x7<<SW, (slot_width/4-1)<<SW);

	/* bclk num of per channel
	 * I2S/RIGHT_J/LEFT_J	-> lrck long total is lrck_width_map * 2
	 * DSP_A/DSP_B		-> lrck long total is lrck_width_map * 1
	 */
	switch (param->format & SND_SOC_DAIFMT_FORMAT_MASK) {
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
	snd_debug("lrck period val %u\n", lrck_width_map);
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_LRCK_CTRL1,
			   0x3<<LRCK_PERIODH, ((lrck_width_map) >> 8)<<LRCK_PERIODL);
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_LRCK_CTRL2,
			   0xFF<<LRCK_PERIODL, (lrck_width_map)<<LRCK_PERIODL);

	param->slots = slots;
	param->slot_width = slot_width;

	return 0;
}

static int sunxi_ac101s_codec_dai_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				 struct sunxi_sound_pcm_hw_params *params,
				 struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;
	unsigned int bclk_ratio = 0, bclk_div_regval = 0;

	int i;
	u8 channels, channels_en, sample_resolution;

	snd_debug("\n");

	/* AC101S set channels */
	channels = params_channels(params);
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (channels < 2) {
			sunxi_ac101s_update_bits(&param->twi_dev, I2S_RX_MIX_SRC,
					   0x3<<RX_MIX_GAIN, 0x0<<RX_MIX_GAIN);
			sunxi_ac101s_update_bits(&param->twi_dev, I2S_RX_MIX_SRC,
					   0x3<<RX_MIX_SRC, 0x1<<RX_MIX_SRC);
		} else {
			sunxi_ac101s_update_bits(&param->twi_dev, I2S_RX_MIX_SRC,
					   0x3<<RX_MIX_GAIN, 0x3<<RX_MIX_GAIN);
			sunxi_ac101s_update_bits(&param->twi_dev, I2S_RX_MIX_SRC,
					   0x3<<RX_MIX_SRC, 0x3<<RX_MIX_SRC);
		}
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_SLOT_CTRL, 0x3<<RX_CHSEL, (channels-1)<<RX_CHSEL);
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_CTRL, 0x1<<RXEN, 0x1<<RXEN);
	} else {
		channels_en = (1<<channels) - 1;
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_SLOT_CTRL, 0x3<<TX_CHSEL, (channels-1)<<TX_CHSEL);
		sunxi_ac101s_write(&param->twi_dev, I2S_TX_CTRL, channels_en);
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_CTRL, 0x1<<TXEN, 0x1<<TXEN);
	}

	/* AC101S set sample resorution */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		sample_resolution = 8;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		sample_resolution = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sample_resolution = 24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		sample_resolution = 32;
		break;
	default:
		snd_err("AC101S don't supported the sample resolution: %u\n", params_format(params));
		return -EINVAL;
	}
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_FMT_CTRL2, 0x7<<SR, (sample_resolution/4-1)<<SR);

	/* set bclk div: ratio = sysclk / sample_rate / slots / slot_width */
	if ((param->format & SND_SOC_DAIFMT_MASTER_MASK) == 1) {
		bclk_ratio = param->sysclk_freq / params_rate(params) / param->slots / param->slot_width;
		for (i = 0; i < ARRAY_SIZE(g_bclk_div); i++) {
			if (g_bclk_div[i].real_val == bclk_ratio) {
				bclk_div_regval = g_bclk_div[i].reg_val;
				snd_debug("AC101S set BCLK %u\n", bclk_ratio);
				break;
			}
		}
		if (i == ARRAY_SIZE(g_bclk_div)) {
			snd_err("AC101S don't support BCLK %u\n", bclk_ratio);
			return -EINVAL;
		}
		sunxi_ac101s_update_bits(&param->twi_dev, I2S_BCLK_CTRL, 0xf<<BCLKDIV, bclk_div_regval<<BCLKDIV);
	}

	/* AC101S Globle enable */
	if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_ac101s_update_bits(&param->twi_dev, DAC_ANA_CTRL1, 3<<VRDA_EN, 3<<VRDA_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, DAC_DIG_CTRL, 1<<DAC_DIG_EN, 1<<DAC_DIG_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, SYS_FUNC_CTRL, 1<<DAC_ANA_OUT_EN, 1<<DAC_ANA_OUT_EN);
	}
	sunxi_ac101s_update_bits(&param->twi_dev, I2S_CTRL, 0x1<<I2SGEN, 0x1<<I2SGEN);

	return 0;
}

static int sunxi_ac101s_codec_dai_prepare(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
#if 0
	struct snd_codec *codec = dai->component;
	struct ac101s_priv *ac101s = codec->private_data;
	struct ac101s_param *param = &ac101s->param;
#endif

	snd_debug("\n");

	return 0;
}

static int sunxi_ac101s_codec_dai_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd, struct sunxi_sound_adf_dai *dai)
{
#if 0
	struct snd_codec *codec = dai->component;
	struct ac101s_priv *ac101s = codec->private_data;
	struct ac101s_param *param = &ac101s->param;
#endif

	snd_debug("\n");

	return 0;
}

static int sunxi_ac101s_codec_dai_hw_free(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	snd_debug("\n");

	if(dataflow->stream == SNDRV_PCM_STREAM_CAPTURE){
		sunxi_ac101s_update_bits(&param->twi_dev, PWR_CTRL2, 1<<MBIAS_EN, 0<<MBIAS_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, ADC_DIG_CTRL, 1<<ADC_DIG_EN, 0<<ADC_DIG_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, ADC_ANA_CTRL1, 1<<ADC_GEN, 0<<ADC_GEN);
	} else if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		sunxi_ac101s_update_bits(&param->twi_dev, DAC_ANA_CTRL1, 3<<VRDA_EN, 0<<VRDA_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, DAC_DIG_CTRL, 1<<DAC_DIG_EN, 0<<DAC_DIG_EN);
		sunxi_ac101s_update_bits(&param->twi_dev, SYS_FUNC_CTRL, 1<<DAC_ANA_OUT_EN, 0<<DAC_ANA_OUT_EN);
	}

	return 0;
}

static void sunxi_ac101s_codec_dai_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");

	/* NULL */

	return;
}

static struct sunxi_sound_adf_dai_ops sunxi_ac101s_codec_dai_ops = {
	.startup	= sunxi_ac101s_codec_dai_startup,
	.set_sysclk	= sunxi_ac101s_codec_dai_set_sysclk,
	.set_clkdiv	= sunxi_ac101s_codec_dai_set_clkdiv,
	.set_fmt	= sunxi_ac101s_codec_dai_set_fmt,
	.set_tdm_slot	= sunxi_ac101s_codec_dai_set_tdm_slot,	/* set slot num and width */
	.hw_params	= sunxi_ac101s_codec_dai_hw_params,
	.prepare	= sunxi_ac101s_codec_dai_prepare,
	.trigger	= sunxi_ac101s_codec_dai_trigger,
	.hw_free	= sunxi_ac101s_codec_dai_hw_free,
	.shutdown	= sunxi_ac101s_codec_dai_shutdown,
};

static struct sunxi_sound_adf_dai_driver sunxi_ac101s_codec_dai = {
	.name = DRV_NAME,
	.playback	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 48000,
	},
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= SNDRV_PCM_RATE_8000_48000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 48000,
	},
	.ops		= &sunxi_ac101s_codec_dai_ops,
};

/* suspend and resume */
#ifdef CONFIG_COMPONENTS_PM

static int sunxi_ac101s_codec_suspend(struct sunxi_sound_adf_component *component)
{

	snd_debug("\n");

	return 0;
}

static int sunxi_ac101s_codec_resume(struct sunxi_sound_adf_component *component)
{
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;

	snd_debug("\n");

	sunxi_ac101s_codec_init(&ac101s->param.twi_dev, param);

	return 0;
}

#else

static int sunxi_ac101s_codec_suspend(struct sunxi_sound_adf_component *component)
{
	return 0;
}

static int sunxi_ac101s_codec_resume(struct sunxi_sound_adf_component *component)
{
	return 0;
}

#endif

static int sunxi_ac101s_codec_probe(struct sunxi_sound_adf_component *component)
{
	twi_status_t ret = 0;
	struct ac101s_priv *ac101s = NULL;
	struct ac101s_param default_param = {
		.twi_dev		= AC101S_CHIP_TWI_CFG,
		.format			= 	AC101S_DAUDIO_FORMAT |
							(AC101S_DAUDIO_SIG_INV << SND_SOC_DAIFMT_SIG_SHIFT) | 
							(AC101S_DAUDIO_MASTER << SND_SOC_DAIFMT_MASTER_SHIFT),
	};

	snd_debug("\n");

	ac101s = sound_malloc(sizeof(struct ac101s_priv));
	if (!ac101s) {
		snd_err("no memory\n");
		return -ENOMEM;
	}

	snd_debug("codec para init.\n");
	component->private_data = (void *)ac101s;
	ac101s->param = default_param;

	snd_debug("init ac101s i2c port.\n");
	ret = sunxi_ac101s_init_i2c_device(ac101s->param.twi_dev.bus);
	if (ret != TWI_STATUS_OK) {
		snd_err("init i2c err\n");
		ret = -EFAULT;
		goto err_twi_init;
	}

	snd_debug("ac101s codec register finished.\n");

	sunxi_ac101s_codec_init(&ac101s->param.twi_dev, &ac101s->param);

	return 0;

err_twi_init:
	sound_free(ac101s);

	return ret;
}

static void sunxi_ac101s_codec_remove(struct sunxi_sound_adf_component *component)
{
	struct ac101s_priv *ac101s = component->private_data;
	struct ac101s_param *param = &ac101s->param;
	int ret = 0;

	snd_debug("\n");

	snd_debug("deinit ac101s i2c port.\n");
	ret = sunxi_ac101s_deinit_i2c_device(param->twi_dev.bus);
	if (ret != TWI_STATUS_OK) {
		snd_err("i2c deinit port %d failed.\n", param->twi_dev.bus);
	}
	sound_free(ac101s);
	component->private_data = NULL;

	return;
}

static struct sunxi_sound_adf_component_driver sunxi_ac101s_codec_component_dev = {
	.name		= COMPONENT_DRV_NAME,
	.probe		= sunxi_ac101s_codec_probe,
	.remove		= sunxi_ac101s_codec_remove,
	.suspend 	= sunxi_ac101s_codec_suspend,
	.resume 	= sunxi_ac101s_codec_resume,
	.controls       = sunxi_ac101s_codec_controls,
	.num_controls   = ARRAY_SIZE(sunxi_ac101s_codec_controls),
};

int sunxi_ac101s_component_probe()
{
	int ret;
	snd_debug("\n");

	ret= sunxi_sound_adf_register_component(&sunxi_ac101s_codec_component_dev, &sunxi_ac101s_codec_dai, 1);
	if (ret != 0) {
		snd_err("sunxi_sound_adf_register_component failed");
		return ret;
	}

	return 0;
}

void sunxi_ac101s_component_remove()
{
	snd_debug("\n");
	sunxi_sound_adf_unregister_component(&sunxi_ac101s_codec_component_dev);
}


/* for ac101s debug */
static void sunxi_ac101s_reg_dump_usage(void)
{
	printf("\n\n=========sunxi ac101s debug===========\n");
	printf("Usage: sunxi_ac101s_reg [option]\n");
	printf("\t-l,	 ac101s dev list\n");
	printf("\t-h,	 tools help\n");
	printf("\t-d,	 ac101s dev addr(hex)\n");
	printf("\t-r,	 ac101s reg addr(hex)\n");
	printf("\t-n,	 ac101s reg read num(hex)\n");
	printf("\t-s,	 ac101s show all regs\n");
	printf("\t-w,	 ac101s write reg val(hex)\n");
	printf("\n");
}

static void sunxi_ac101s_chip_list(void)
{
	unsigned int chip_num = AC101S_CHIP_NUM;
	struct twi_device twi_dev = AC101S_CHIP_TWI_CFG;
	int i = 0;

	printf("\n\n");
	printf("========= ac101s show =========\n");
	printf("\tac101s dev num:\t%d\n", chip_num);
	for (i = 0; i < chip_num; i++)
		printf("\t%d i2c%d-0x%02x\n", i, twi_dev.bus, twi_dev.addr);
	printf("===============================\n");
}

static void sunxi_ac101s_reg_show(void)
{
	struct twi_device twi_dev = AC101S_CHIP_TWI_CFG;
	unsigned char read_command;
	unsigned char read_data[1] = {0x0};
	twi_status_t ret = 0;
	unsigned int j = 0;

	printf("\n====== ac101s chip [i2c%d-0x%02x] ======\n", twi_dev.bus, twi_dev.addr);
	for (j = 0; j <= AC101S_REG_MAX; j++) {
		if (j % 8 == 0)
			printf("\n");
		read_command = 0x0 + j;
		ret = sunxi_ac101s_read(&twi_dev, read_command, read_data);
		if (ret != TWI_STATUS_OK) {
			snd_err("[i2c%d-0x%02x] read [REG-0x%02x,val-0x%02x] ret = %d.\n",
				twi_dev.bus, twi_dev.addr,
				read_command, read_data[0], ret);
		}
		printf("[0x%02x]: 0x%02x  ", read_command, read_data[0]);
	}
	printf("\n========================================\n");
}

int cmd_sunxi_ac101s_reg(int argc, char ** argv)
{
	unsigned int chip_num = AC101S_CHIP_NUM;
	twi_status_t ret = -1;
	unsigned int i;
	const struct option long_option[] = {
		{"help", 0, NULL, 'h'},
		{"list", 0, NULL, 'l'},
		{"addr", 1, NULL, 'd'},
		{"reg", 1, NULL, 'r'},
		{"num", 1, NULL, 'n'},
		{"show", 0, NULL, 's'},
		{"write", 1, NULL, 'w'},
		{NULL, 0, NULL, 0},
	};
	struct twi_device twi_dev = AC101S_CHIP_TWI_CFG;
	/*unsigned char reset_cmd[2] = {0x0, 0x12};*/
	unsigned char write_cmd[2] = {0x0, 0x0};
	unsigned char read_cmd[1] = {0x0};
	unsigned char read_data[1] = {0x0};
	unsigned int num = 1;
	unsigned int twi_addr = twi_dev.addr;
	bool wr_flag = 0;

	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hlsd:r:n:w:", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
			sunxi_ac101s_reg_dump_usage();
			goto ac101s_reg_exit;
		case 'l':
			sunxi_ac101s_chip_list();
			goto ac101s_reg_exit;
		case 's':
			sunxi_ac101s_reg_show();
			goto ac101s_reg_exit;
		case 'd':
			if (isdigit(*optarg)) {
				sscanf(optarg, "0x%x", &twi_addr);
				printf("\ntwi_addr slave address is 0x%02x.\n", twi_addr);
			} else
				fprintf(stderr, "twi addr is not a digital value.\n");
			break;
		case 'r':
			if (isdigit(*optarg)) {
				sscanf(optarg, "0x%hhx", &read_cmd[0]);
				write_cmd[0] = read_cmd[0];
				printf("\nreg is 0x%02x.\n", read_cmd[0]);
			} else
				fprintf(stderr, "reg is not a digital value.\n");
			break;
		case 'n':
			if (isdigit(*optarg)) {
				sscanf(optarg, "0x%x", &num);
				printf("\nnum is %d.\n", num);
			} else
				fprintf(stderr, "num is not a digital value.\n");
			break;
		case 'w':
			if (isdigit(*optarg)) {
				wr_flag = 1;
				sscanf(optarg, "0x%02x 0x%02x", (unsigned int *)&write_cmd[0], (unsigned int *)&write_cmd[1]);
				printf("\nwrite reg is 0x%02x, val is 0x%02x.\n",
					write_cmd[0], write_cmd[1]);
			} else
				fprintf(stderr, "write val is not a digital value.\n");
			break;
		default:
			fprintf(stderr, "Invalid switch or option needs an argument.\n");
			break;
		}
	}

	//checkout i2c port and addr.
	for (i = 0; i < chip_num; i++) {
		if (twi_addr == twi_dev.addr)
			break;
	}
	if (i >= chip_num) {
		fprintf(stderr, "the addr is error.\n");
		goto ac101s_reg_exit;
	}
	if ((read_cmd[0] > AC101S_REG_MAX) || (write_cmd[0] > AC101S_REG_MAX)) {
		fprintf(stderr, "the reg is over 0x%02x error.\n", AC101S_REG_MAX);
		goto ac101s_reg_exit;
	}

	if (wr_flag) {
		ret = sunxi_ac101s_write(&twi_dev, write_cmd[0], write_cmd[1]);
		if (ret != TWI_STATUS_OK) {
			snd_err("write error [REG-0x%02x,val-0x%02x] ret = %d.\n",
				write_cmd[0], write_cmd[1], ret);
		}
		ret = sunxi_ac101s_read(&twi_dev, write_cmd[0], read_data);
		if (ret != TWI_STATUS_OK) {
			snd_err("write error [I2C%d-0x%0x] REG=0x%02x, val=0x%02x] ret = %d.\n",
				twi_dev.bus, twi_dev.addr, write_cmd[0], read_data[0], ret);
			goto ac101s_reg_exit;
		}
		if (read_data[0] == write_cmd[1]) {
			printf("write success, [I2C%d-0x%0x] REG=0x%02x, val=0x%02x] ret = %d.\n",
				twi_dev.bus, twi_dev.addr, write_cmd[0], read_data[0], ret);
		} else {
			printf("write val:0x%02x failed, [I2C%d-0x%0x] REG=0x%02x, val=0x%02x] ret = %d.\n",
				write_cmd[1], twi_dev.bus, twi_dev.addr, write_cmd[0], read_data[0], ret);
		}
	} else {
		for (i = 0; i < num; i++) {
			ret = sunxi_ac101s_read(&twi_dev, read_cmd[0], read_data);
			if (ret != TWI_STATUS_OK) {
				snd_err("read error [I2C%d-0x%0x] REG=0x%02x, val=0x%02x] ret = %d.\n",
					twi_dev.bus, twi_dev.addr, read_cmd[0], read_data[0], ret);
				goto ac101s_reg_exit;
			} else {
				printf("read success. [I2C%d-0x%0x] REG=0x%02x, val=0x%02x].\n",
					twi_dev.bus, twi_dev.addr, read_cmd[0], read_data[0]);
			}
		}
	}

ac101s_reg_exit:
	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_sunxi_ac101s_reg, sunxi_ac101s_reg, ac101sregsdump);
