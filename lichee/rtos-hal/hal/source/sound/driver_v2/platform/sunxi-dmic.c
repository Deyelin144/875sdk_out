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
#include <sunxi_sound_common.h>

#include "sunxi-pcm.h"
#include "sunxi-dmic.h"

#define COMPONENT_DRV_NAME	"sunxi-snd-plat-dmic"
#define DRV_NAME			"sunxi-snd-plat-dmic-dai"

struct sunxi_dmic_info {
	struct sunxi_dma_params capture_dma_param;
	u32 chanmap;

	struct sunxi_dmic_clk clk;
};

static struct audio_reg_label sunxi_reg_labels[] = {
	REG_LABEL(SUNXI_DMIC_EN),
	REG_LABEL(SUNXI_DMIC_SR),
	REG_LABEL(SUNXI_DMIC_CTR),
	/* REG_LABEL(SUNXI_DMIC_DATA), */
	REG_LABEL(SUNXI_DMIC_INTC),
	REG_LABEL(SUNXI_DMIC_INTS),
	REG_LABEL(SUNXI_DMIC_FIFO_CTR),
	REG_LABEL(SUNXI_DMIC_FIFO_STA),
	REG_LABEL(SUNXI_DMIC_CH_NUM),
	REG_LABEL(SUNXI_DMIC_CH_MAP),
	REG_LABEL(SUNXI_DMIC_CNT),
	REG_LABEL(SUNXI_DMIC_DATA0_1_VOL),
	REG_LABEL(SUNXI_DMIC_DATA2_3_VOL),
	REG_LABEL(SUNXI_DMIC_HPF_CTRL),
	REG_LABEL(SUNXI_DMIC_HPF_COEF),
	REG_LABEL(SUNXI_DMIC_HPF_GAIN),
	REG_LABEL_END,
};

static const struct dmic_rate dmic_rate_s[] = {
	{44100, 0x0},
	{48000, 0x0},
	{22050, 0x2},
	/* KNOT support */
	{24000, 0x2},
	{11025, 0x4},
	{12000, 0x4},
	{32000, 0x1},
	{16000, 0x3},
	{8000, 0x5},
};

/*
 * Configure DMA , Chan enable & Global enable
 */
static void sunxi_dmic_enable(struct sunxi_sound_adf_component *component, bool enable)
{
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");
	if (enable) {
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_INTC,
				(0x1 << FIFO_DRQ_EN), (0x1 << FIFO_DRQ_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DMIC_EN,
				(0xFF<<DATA_CH_EN),
				((sunxi_dmic->chanmap)<<DATA_CH_EN));

		sunxi_sound_component_update_bits(component, SUNXI_DMIC_EN,
				(0x1 << GLOBE_EN), (0x1 << GLOBE_EN));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_EN,
				(0x1 << GLOBE_EN), (0x0 << GLOBE_EN));
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_EN,
				(0xFF << DATA_CH_EN),
				(0x0 << DATA_CH_EN));
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_INTC,
				(0x1 << FIFO_DRQ_EN),
				(0x0 << FIFO_DRQ_EN));
	}
}

static void sunxi_dmic_init(struct sunxi_sound_adf_component *component)
{
	snd_debug("\n");
	sunxi_sound_component_write(component,
			SUNXI_DMIC_CH_MAP, DMIC_CHANMAP_DEFAULT);
	sunxi_sound_component_update_bits(component, SUNXI_DMIC_CTR,
			(0x7<<DMICDFEN), (0x7<<DMICDFEN));

	/* set the vol */
	sunxi_sound_component_write(component, SUNXI_DMIC_DATA0_1_VOL, DMIC_DEFAULT_VOL);
	sunxi_sound_component_write(component, SUNXI_DMIC_DATA2_3_VOL, DMIC_DEFAULT_VOL);
}

static int sunxi_dmic_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
				 struct sunxi_sound_pcm_hw_params *params,
				 struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;
	int i;

	snd_debug("\n");
	/* if clk rst */
	sunxi_dmic_init(component);

	/* sample resolution & sample fifo format */
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_FIFO_CTR,
				(0x1 << DMIC_SAMPLE_RESOLUTION),
				(0x0 << DMIC_SAMPLE_RESOLUTION));
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_FIFO_CTR,
				(0x1 << DMIC_FIFO_MODE),
				(0x1 << DMIC_FIFO_MODE));
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_FIFO_CTR,
				(0x1 << DMIC_SAMPLE_RESOLUTION),
				(0x1 << DMIC_SAMPLE_RESOLUTION));
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_FIFO_CTR,
				(0x1 << DMIC_FIFO_MODE),
				(0x0 << DMIC_FIFO_MODE));
		break;
	default:
		snd_err( "Invalid format set\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(dmic_rate_s); i++) {
		if (dmic_rate_s[i].samplerate == params_rate(params)) {
			sunxi_sound_component_update_bits(component, SUNXI_DMIC_SR,
			(7<<DMIC_SR), (dmic_rate_s[i].rate_bit<<DMIC_SR));
			break;
		}
	}

	/* oversamplerate adjust */
	if (params_rate(params) >= 24000) {
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_CTR,
			(1<<DMIC_OVERSAMPLE_RATE), (1<<DMIC_OVERSAMPLE_RATE));
	} else {
		sunxi_sound_component_update_bits(component, SUNXI_DMIC_CTR,
			(1<<DMIC_OVERSAMPLE_RATE), (0<<DMIC_OVERSAMPLE_RATE));
	}

	sunxi_dmic->chanmap = (1<<params_channels(params)) - 1;

	sunxi_sound_component_write(component, SUNXI_DMIC_HPF_CTRL, sunxi_dmic->chanmap);

	/* DMIC num is M+1 */
	sunxi_sound_component_update_bits(component, SUNXI_DMIC_CH_NUM,
		(7<<DMIC_CH_NUM), ((params_channels(params)-1)<<DMIC_CH_NUM));

	return 0;
}

static int sunxi_dmic_trigger(struct sunxi_sound_pcm_dataflow *dataflow, int cmd, struct sunxi_sound_adf_dai *dai)
{
	int ret = 0;
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");
	switch (cmd) {
	case	SNDRV_PCM_TRIGGER_START:
	case	SNDRV_PCM_TRIGGER_RESUME:
	case	SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		sunxi_dmic_enable(component, true);
		break;
	case	SNDRV_PCM_TRIGGER_STOP:
	case	SNDRV_PCM_TRIGGER_SUSPEND:
	case	SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		sunxi_dmic_enable(component, false);
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
static int sunxi_dmic_prepare(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;

	snd_debug("\n");
	sunxi_sound_component_update_bits(component, SUNXI_DMIC_FIFO_CTR,
			(1<<DMIC_FIFO_FLUSH), (1<<DMIC_FIFO_FLUSH));

	sunxi_sound_component_write(component, SUNXI_DMIC_INTS,
		(1<<FIFO_OVERRUN_IRQ_PENDING) | (1<<FIFO_DATA_IRQ_PENDING));

	sunxi_sound_component_write(component, SUNXI_DMIC_CNT, 0x0);

	return 0;
}

static int sunxi_dmic_startup(struct sunxi_sound_pcm_dataflow *dataflow,
		struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");
	if (dataflow->stream == SNDRV_PCM_STREAM_CAPTURE) {
		sunxi_sound_adf_dai_set_dma_data(dai, dataflow, &sunxi_dmic->capture_dma_param);
	} else if (dataflow->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		snd_err("dmic only support capture\n");
		return -EINVAL;
	}

	return 0;
}

static void sunxi_dmic_shutdown(struct sunxi_sound_pcm_dataflow *dataflow, struct sunxi_sound_adf_dai *dai)
{
	snd_debug("\n");
}


static int sunxi_dmic_dai_set_pll(struct sunxi_sound_adf_dai *dai, int pll_id, int source,
				 unsigned int freq_in, unsigned int freq_out)
{
	int ret;
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");

	ret = snd_sunxi_dmic_clk_set_rate(&sunxi_dmic->clk, 0, freq_in, freq_out);
	if (ret < 0) {
		snd_err("snd_sunxi_dmic_clk_set_rate failed\n");
		return -1;
	}

	return 0;
}

/* Dmic module init status */
static int sunxi_dmic_dai_probe(struct sunxi_sound_adf_dai *dai)
{
	struct sunxi_sound_adf_component *component = dai->component;
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");

	/* pcm_new will using the dma_param about the cma and fifo params. */
	sunxi_sound_adf_dai_init_dma_data(dai,
				  NULL,
				  &sunxi_dmic->capture_dma_param);

	sunxi_dmic_init(component);

	return 0;
}

static struct sunxi_sound_adf_dai_ops sunxi_dmic_dai_ops = {
	/* call by machine */
	.set_pll = sunxi_dmic_dai_set_pll,	/* set pllclk */
	/* call by asoc */
	.startup = sunxi_dmic_startup,
	.trigger = sunxi_dmic_trigger,
	.prepare = sunxi_dmic_prepare,
	.hw_params = sunxi_dmic_hw_params,
	.shutdown = sunxi_dmic_shutdown,
};

static struct sunxi_sound_adf_dai_driver sunxi_dmic_dai = {
	.id		= 1,
	.name		= DRV_NAME,
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max	= 8,
		.rates		= SUNXI_DMIC_RATES,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min	= 8000,
		.rate_max	= 48000,
	},
	.probe		= sunxi_dmic_dai_probe,
	.ops		= &sunxi_dmic_dai_ops,
};

static int sunxi_dmic_gpio_init(bool enable)
{
	snd_debug("\n");
	if (enable) {
		/*CLK*/
		if (g_dmic_gpio.clk.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.clk.gpio,
						     g_dmic_gpio.clk.mux);
		/*DATA0*/
		if (g_dmic_gpio.din0.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din0.gpio,
						     g_dmic_gpio.din0.mux);
		/*DATA1*/
		if (g_dmic_gpio.din1.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din1.gpio,
						     g_dmic_gpio.din1.mux);
		/*DATA2*/
		if (g_dmic_gpio.din2.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din2.gpio,
						     g_dmic_gpio.din2.mux);
		/*DATA3*/
		if (g_dmic_gpio.din3.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din3.gpio,
						     g_dmic_gpio.din3.mux);
	} else {
		/*CLK*/
		if (g_dmic_gpio.clk.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.clk.gpio,
						     GPIO_MUXSEL_DISABLED);
		/*DATA0*/
		if (g_dmic_gpio.din0.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din0.gpio,
						     GPIO_MUXSEL_DISABLED);
		/*DATA1*/
		if (g_dmic_gpio.din1.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din1.gpio,
						     GPIO_MUXSEL_DISABLED);
		/*DATA2*/
		if (g_dmic_gpio.din2.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din2.gpio,
						     GPIO_MUXSEL_DISABLED);
		/*DATA3*/
		if (g_dmic_gpio.din3.mux >= 0)
			hal_gpio_pinmux_set_function(g_dmic_gpio.din3.gpio,
						     GPIO_MUXSEL_DISABLED);
	}

	return 0;
}

#ifdef CONFIG_DRIVER_SYSCONFIG
static int sunxi_sound_parse_dmic_dma_params(const char* prefix, const char* type_name,
			struct sunxi_dmic_info *info)
{
	char key_name[32];
	char cma_name[32];
	int ret;
	int32_t tmp_val;

	snprintf(key_name, sizeof(key_name), "%s_plat", prefix);

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
static struct sunxi_spdif_info default_dmic_param = {
	.playback_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
	.capture_dma_param	= {
		.cma_kbytes = SUNXI_SOUND_CMA_MAX_KBYTES,
	},
};
#endif

static void sunxi_dmic_params_init(struct sunxi_dmic_info *info)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	sunxi_sound_parse_dmic_dma_params("dmic", NULL ,info);
#else
	*info = default_dmic_param;
#endif
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

static int sunxi_dmic_suspend(struct sunxi_sound_adf_component *component)
{
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");

	sunxi_sound_save_reg(sunxi_reg_labels, (void *)component, snd_read_func);
	snd_sunxi_dmic_clk_disable(&sunxi_dmic->clk);

	return 0;
}

static int sunxi_dmic_resume(struct sunxi_sound_adf_component *component)
{
	struct sunxi_dmic_info *sunxi_dmic = component->private_data;

	snd_debug("\n");

	snd_sunxi_dmic_clk_enable(&sunxi_dmic->clk);
	sunxi_dmic_init(component);
	sunxi_sound_echo_reg(sunxi_reg_labels, (void *)component, snd_write_func);

	return 0;
}

#else

static int sunxi_dmic_suspend(struct sunxi_sound_adf_component *component)
{
	return 0;
}

static int sunxi_dmic_resume(struct sunxi_sound_adf_component *component)
{
	return 0;
}

#endif

/* dmic probe */
static int sunxi_dmic_probe(struct sunxi_sound_adf_component *component)
{
	int ret;
	struct sunxi_dmic_info *sunxi_dmic;

	snd_debug("\n");
	sunxi_dmic = sound_malloc(sizeof(struct sunxi_dmic_info));
	if (!sunxi_dmic) {
		snd_err("no memory\n");
		return -ENOMEM;
	}
	component->private_data = (void *)sunxi_dmic;

	/* mem base */
	component->addr_base = (void *)SUNXI_DMIC_MEMBASE;

	/* clk */
	ret = snd_sunxi_dmic_clk_init(&sunxi_dmic->clk);
	if (ret != 0) {
		snd_err("snd_sunxi_dmic_clk_init failed\n");
		goto err_dmic_set_clock;
	}

	/* pinctrl */
	sunxi_dmic_gpio_init(true);

	sunxi_dmic_params_init(sunxi_dmic);

	/* dma config */
	sunxi_dmic->capture_dma_param.src_maxburst = 8;
	sunxi_dmic->capture_dma_param.dst_maxburst = 8;
	sunxi_dmic->capture_dma_param.dma_addr = (dma_addr_t)component->addr_base + SUNXI_DMIC_DATA;
	sunxi_dmic->capture_dma_param.dma_drq_type_num = DRQSRC_DMIC;

	return 0;

err_dmic_set_clock:
	snd_sunxi_dmic_clk_exit(&sunxi_dmic->clk);

	return -1;
}

static void sunxi_dmic_remove(struct sunxi_sound_adf_component *component)
{
	struct sunxi_dmic_info *sunxi_dmic;

	snd_debug("\n");
	sunxi_dmic = component->private_data;
	if (!sunxi_dmic)
		return;

	snd_sunxi_dmic_clk_exit(&sunxi_dmic->clk);
	sunxi_dmic_gpio_init(false);

	sound_free(sunxi_dmic);
	component->private_data = NULL;

	return;
}

static struct sunxi_sound_adf_component_driver sunxi_dmic_component_dev = {
	.name		= COMPONENT_DRV_NAME,
	.probe		= sunxi_dmic_probe,
	.remove		= sunxi_dmic_remove,
	.suspend 	= sunxi_dmic_suspend,
	.resume 	= sunxi_dmic_resume,
};

int sunxi_dmic_component_probe()
{
	int ret;
	char name[48];
	snd_debug("\n");

	ret= sunxi_sound_adf_register_component(&sunxi_dmic_component_dev, &sunxi_dmic_dai, 1);
	if (ret != 0) {
		snd_err("sunxi_sound_adf_register_component failed");
		return ret;
	}

	snprintf(name, sizeof(name), "%s-dma", sunxi_dmic_component_dev.name);
	ret = sunxi_pcm_dma_platform_register(name);
	if (ret != 0) {
		snd_err("sunxi_pcm_dma_platform_register failed");
		return ret;
	}
	return 0;
}

void sunxi_dmic_component_remove()
{
	char name[48];

	snd_debug("\n");
	sunxi_sound_adf_unregister_component(&sunxi_dmic_component_dev);
	snprintf(name, sizeof(name), "%s-dma", sunxi_dmic_component_dev.name);
	sunxi_pcm_dma_platform_unregister(name);
}

#ifdef SUNXI_DMIC_DEBUG_REG
/* for debug */
#include <console.h>
int cmd_dmic_dump(void)
{
	int dmic_num = 0;
	void *membase;
	int i = 0;

	membase = (void *)SUNXI_DMIC_MEMBASE ;

	while (sunxi_reg_labels[i].name != NULL) {
		printf("%-20s[0x%03x]: 0x%-10x\n",
			sunxi_reg_labels[i].name,
			sunxi_reg_labels[i].address,
			snd_readl(membase + sunxi_reg_labels[i].address));
		i++;
	}
}
FINSH_FUNCTION_EXPORT_CMD(cmd_dmic_dump, dmic, dmic dump reg);
#endif /* SUNXI_DMIC_DEBUG_REG */
