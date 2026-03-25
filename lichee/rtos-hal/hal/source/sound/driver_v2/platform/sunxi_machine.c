#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aw_list.h"
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif

#include <sunxi_hal_common.h>
#include <aw_common.h>
#include "sunxi_machine.h"
#include <sound_v2/sunxi_sound_pcm_common.h>

static LIST_HEAD(gSunxiSoundCardList);

#define DEFAULT_SLOT		2
#define DEFAULT_SLOT_WIDTH		32

#define COMPONENT_CPU_NAME		"sunxi-snd-plat-aaudio"
#define COMPONENT_CODEC_NAME	"sunxi-snd-codec"

struct sunxi_sound_mach_bind {
	unsigned int sysclk;
	int clk_direction;
	int slots;
	int slot_width;
	unsigned int tx_slot_mask;
	unsigned int rx_slot_mask;
};

struct sunxi_sound_mach_bind_props {
	struct sunxi_sound_adf_dai_bind_component cpus;   /* single cpu */
	struct sunxi_sound_adf_dai_bind_component *codecs; /* multi codec */
	struct sunxi_sound_adf_dai_bind_component platforms;
	int mclk_fp;
	unsigned int mclk_fs;
	unsigned int cpu_pll_fs;
	unsigned int codec_pll_fs;
};

struct sunxi_sound_mach_priv {
	struct sunxi_sound_adf_card card;
	struct sunxi_sound_mach_bind_props *bind_props;
	struct sunxi_sound_mach_bind *binds;
	struct sunxi_sound_adf_dai_bind *dai_bind;
	enum snd_platform_type plat_type;
	struct list_head list;
};

static struct sunxi_sound_adf_dai_bind default_dai_bind_param = {
	.dai_fmt	=  SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS,
	.playback_only	= 0,
	.capture_only	= 0,

};

static struct sunxi_sound_mach_bind default_mach_bind_param = {
	.slots	=  DEFAULT_SLOT,
	.slot_width	= DEFAULT_SLOT_WIDTH,
};


static struct sunxi_sound_mach_bind_props defaul_tmach_bind_props_param = {
	.mclk_fs	=  1,
	.mclk_fp	= 1,
	.cpu_pll_fs	= 1,
	.codec_pll_fs = 1,
};

static struct sunxi_sound_adf_card *find_sound_card_by_name(const char *name)
{
	struct sunxi_sound_mach_priv *priv;
	struct sunxi_sound_adf_card *card;

	list_for_each_entry(priv, &gSunxiSoundCardList, list) {
		card = &priv->card;
		if (strlen(card->name) == strlen(name) && !strcmp(card->name, name))
			return card;
	}
	return NULL;
}

static inline void sunxi_sound_card_set_drvdata(struct sunxi_sound_adf_card *card,
					    void *data)
{
	card->drvdata = data;
}

static inline void *sunxi_sound_card_get_drvdata(struct sunxi_sound_adf_card *card)
{
	return card->drvdata;
}

static int sunxi_sound_mach_init_priv(struct sunxi_sound_mach_priv *priv)
{
	struct sunxi_sound_adf_card *card = &priv->card;
	struct sunxi_sound_mach_bind_props *bind_props;
	struct sunxi_sound_adf_dai_bind *dai_bind;
	struct sunxi_sound_mach_bind *binds;

	bind_props = sound_malloc(sizeof(struct sunxi_sound_mach_bind_props));
	dai_bind =  sound_malloc(sizeof(struct sunxi_sound_adf_dai_bind));
	binds = sound_malloc(sizeof(struct sunxi_sound_mach_bind));

	if (!bind_props || !dai_bind || !binds)
		return -ENOMEM;

	dai_bind->cpus = &bind_props->cpus;
	dai_bind->num_cpus = 1;
	dai_bind->codecs = bind_props->codecs;
	dai_bind->num_codecs = 1;
	dai_bind->platforms = &bind_props->platforms;
	dai_bind->num_platforms = 1;

	priv->bind_props = bind_props;
	priv->dai_bind  = dai_bind;
	priv->binds = binds;

	card->dai_bind = dai_bind;
	card->num_binds = 1;

	INIT_LIST_HEAD(&priv->list);

	return 0;

}

static void sunxi_sound_card_clean_resources(struct sunxi_sound_mach_priv *priv)
{

	int i;

	for (i = 0; i < priv->dai_bind->num_codecs; i++) {
		sound_free(priv->dai_bind->codecs[i].name);
		priv->dai_bind->codecs[i].name = NULL;
	}

	if (priv->dai_bind->codecs) {
		sound_free(priv->dai_bind->codecs);
		priv->dai_bind->codecs = NULL;
	}

	if (priv->dai_bind->cpus->name) {
		sound_free(priv->dai_bind->cpus->name);
		priv->dai_bind->cpus->name = NULL;
	}

	if (priv->dai_bind->platforms->name)
	{
		sound_free(priv->dai_bind->platforms->name);
		priv->dai_bind->platforms->name = NULL;
	}

	if (priv->dai_bind->name) {
		sound_free(priv->dai_bind->name);
		priv->dai_bind->name = NULL;
	}

	if (priv->dai_bind) {
		sound_free(priv->dai_bind);
		priv->dai_bind = NULL;
	}

	if (priv->binds) {
		sound_free(priv->binds);
		priv->binds = NULL;
	}

	if (priv->bind_props) {
		sound_free(priv->bind_props);
		priv->bind_props= NULL;
	}

	sunxi_sound_card_set_drvdata(&priv->card, NULL);

	if (priv->card.name) {
		sound_free(priv->card.name);
		priv->card.name = NULL;
	}

}

static void sunxi_sound_card_unregister_component(enum snd_platform_type plat_type)
{

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
		{
			sunxi_codec_component_remove();
			sunxi_aaudio_component_remove(plat_type);
			break;
		}
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
		{
			sunxi_codec_dac_component_remove();
			sunxi_aaudio_component_remove(plat_type);
			break;
		}
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
		{
			sunxi_codec_adc_component_remove();
			sunxi_aaudio_component_remove(plat_type);
			break;
		}
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
		{
#if defined(CONFIG_SUNXI_SOUND_CODEC_AC101S)
			sunxi_ac101s_component_remove();
#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AC108)
			sunxi_ac108_component_remove();
#endif

			sunxi_dummy_codec_component_remove();

			sunxi_i2s_component_remove(plat_type);
			break;
		}
		case SND_PLATFORM_TYPE_DMIC:
		{
			sunxi_dummy_codec_component_remove();
			sunxi_dmic_component_remove();
			break;
		}
		case SND_PLATFORM_TYPE_OWA:
		{
			sunxi_dummy_codec_component_remove();
			sunxi_owa_component_remove();
			break;
		}
		default:
			snd_err("Type %d is invaild", plat_type);
			break;
	}

	return;
}

static int sunxi_sound_card_register_component(enum snd_platform_type plat_type)
{
	int ret;

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
		{
			ret = sunxi_codec_component_probe();
			if (ret != 0) {
				snd_err("internal-codec component register failed");
				return ret;
			}

			ret = sunxi_aaudio_component_probe(plat_type);
			if (ret != 0) {
				snd_err("aaudio component register failed");
				return ret;
			}
			break;
		}
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
		{
			ret = sunxi_codec_dac_component_probe();
			if (ret != 0) {
				snd_err("internal-codec component register failed");
				return ret;
			}

			ret = sunxi_aaudio_component_probe(plat_type);
			if (ret != 0) {
				snd_err("sunxi_aaudio_component_probe register failed");
				return ret;
			}
			break;
		}
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
		{

			ret = sunxi_codec_adc_component_probe();
			if (ret != 0) {
				snd_err("internal-codec component register failed");
				return ret;
			}

			ret = sunxi_aaudio_component_probe(plat_type);
			if (ret != 0) {
				snd_err("sunxi_aaudio_component_probe register failed");
				return ret;
			}

			break;
		}
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
		{

#if defined(CONFIG_SUNXI_SOUND_CODEC_AC101S)
			ret = sunxi_ac101s_component_probe();
			if (ret != 0) {
				snd_err("ac101s component register failed");
				return ret;
			}
#endif

#if defined(CONFIG_SUNXI_SOUND_CODEC_AC108)
			ret = sunxi_ac108_component_probe();
			if (ret != 0) {
				snd_err("ac101s component register failed");
				return ret;
			}
#endif

			ret = sunxi_dummy_codec_component_probe();
			if (ret != 0) {
				snd_err("dummy codec component register failed");
				return ret;
			}

			ret = sunxi_i2s_component_probe(plat_type);
			if (ret != 0) {
				snd_err("i2s component register failed");
				return ret;
			}

			break;
		}
		case SND_PLATFORM_TYPE_DMIC:
		{

			ret = sunxi_dummy_codec_component_probe();
			if (ret != 0) {
				snd_err("dummy codec component register failed");
				return ret;
			}

			ret = sunxi_dmic_component_probe();
			if (ret != 0) {
				snd_err("dmic component register failed");
				return ret;
			}

			break;
		}
		case SND_PLATFORM_TYPE_OWA:
		{
			ret = sunxi_dummy_codec_component_probe();
			if (ret != 0) {
				snd_err("dummy codec component register failed");
				return ret;
			}

			ret = sunxi_owa_component_probe();
			if (ret != 0) {
				snd_err("owa component register failed");
				return ret;
			}

			break;
		}
		default:
			ret = -1;
			break;
	}

	return ret;
}

static int sunxi_sound_parse_daifmt(enum snd_platform_type plat_type, const char* prefix, unsigned int *retfmt)
{
	char i2s_name[16];
	int ret;
	unsigned int daifmt = 0;
	int32_t tmp_val;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;

#ifdef CONFIG_DRIVER_SYSCONFIG

	snprintf(i2s_name, sizeof(i2s_name), "%s%d", prefix, tdm_num);
	ret = hal_cfg_get_keyvalue(i2s_name, "audio_format", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("i2s:audio_format miss.\n");
		daifmt |= SND_SOC_DAIFMT_I2S;
	} else {
		daifmt |= tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "signal_inversion", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("i2s:signal_inversion miss.\n");
		daifmt |= SND_SOC_DAIFMT_NB_NF;
	} else {
		switch (tmp_val) {
		case 4:
			daifmt |= SND_SOC_DAIFMT_IB_IF;
			break;
		case 3:
			daifmt |= SND_SOC_DAIFMT_IB_NF;
			break;
		case 2:
			daifmt |= SND_SOC_DAIFMT_NB_IF;
			break;
		case 1:
			daifmt |= SND_SOC_DAIFMT_NB_NF;
			break;
		default:
			snd_err("i2s:signal_inversion err.\n");
			break;
		}
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "i2s_master", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("i2s:i2s_master miss.\n");
		daifmt |= SND_SOC_DAIFMT_CBS_CFS;
	} else {
		switch (tmp_val) {
		case 1:
			daifmt |= SND_SOC_DAIFMT_CBM_CFM;
			break;
		case 2:
			daifmt |= SND_SOC_DAIFMT_CBS_CFM;
			break;
		case 3:
			daifmt |= SND_SOC_DAIFMT_CBM_CFS;
			break;
		case 4:
			daifmt |= SND_SOC_DAIFMT_CBS_CFS;
			break;
		default:
			snd_err("i2s:i2s_master err.\n");
			break;
		}

	}
#else
	daifmt = default_dai_bind_param.dai_fmt;
#endif

	*retfmt = daifmt;

	return 0;

}

static int __sunxi_sound_parse_daistream(const char* prefix, const char* type_name, struct sunxi_sound_adf_dai_bind *dai_bind)
{
	char daistream[32];
	char stream_name[32];
	int ret;
	int32_t tmp_val;

#ifdef CONFIG_DRIVER_SYSCONFIG

	snprintf(daistream, sizeof(daistream), "%s_mach", prefix);

	if (type_name)
		snprintf(stream_name, sizeof(stream_name), "%s-playback-only", type_name);
	else
		snprintf(stream_name, sizeof(stream_name), "%s", "playback-only");

	ret = hal_cfg_get_keyvalue(daistream, stream_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:%s miss.\n", stream_name);
		dai_bind->playback_only = default_dai_bind_param.playback_only;
	} else {
		dai_bind->playback_only = 1;
	}

	if (type_name)
		snprintf(stream_name, sizeof(stream_name), "%s-capture-only", type_name);
	else
		snprintf(stream_name, sizeof(stream_name), "%s", "capture-only");

	ret = hal_cfg_get_keyvalue(daistream, stream_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:%s miss.\n", stream_name);
		dai_bind->capture_only = default_dai_bind_param.capture_only;
	} else {
		dai_bind->capture_only = 1;
	}
#else
	dai_bind->playback_only = default_dai_bind_param.playback_only;
	dai_bind->capture_only = default_dai_bind_param.capture_only;
#endif

	return 0;
}

static int sunxi_sound_parse_daistream(enum snd_platform_type plat_type,
			struct sunxi_sound_adf_dai_bind *dai_bind)
{
	int ret;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;
	char prefix_name[16];
	char *type_name = NULL;
	char stream_name[16];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			type_name = NULL;
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			snprintf(stream_name, sizeof(stream_name), "%s", "dac");
			type_name = stream_name;
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			snprintf(stream_name, sizeof(stream_name), "%s", "adc");
			type_name = stream_name;
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
			type_name = NULL;
			snprintf(prefix_name, sizeof(prefix_name), "i2s%d", tdm_num);
			break;
		case SND_PLATFORM_TYPE_DMIC:
			type_name = NULL;
			snprintf(prefix_name, sizeof(prefix_name), "%s", "dmic");
			break;
		case SND_PLATFORM_TYPE_OWA:
			type_name = NULL;
			snprintf(prefix_name, sizeof(prefix_name), "%s", "owa");
			break;
		default:
			ret = -1;
			break;
	}

	ret = __sunxi_sound_parse_daistream(prefix_name, type_name, dai_bind);
	if (ret < 0)
		goto err;

err:
	return ret;
}


static int sunxi_sound_parse_tdm_slot(enum snd_platform_type plat_type, const char* prefix, struct sunxi_sound_mach_bind *binds)
{
	char i2s_name[16];
	int ret;
	int32_t tmp_val , pcm_lrck_period = 0;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;

#ifdef CONFIG_DRIVER_SYSCONFIG

	snprintf(i2s_name, sizeof(i2s_name), "%s%d", prefix, tdm_num);

	ret = hal_cfg_get_keyvalue(i2s_name, "pcm_lrck_period", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:pcm_lrck_period miss.\n");
		binds->slots = default_mach_bind_param.slots;
	} else {
		pcm_lrck_period = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "slot_width_select", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:slot_width_select miss.\n");
		binds->slot_width = default_mach_bind_param.slot_width;
	} else {
		binds->slot_width = tmp_val;
		binds->slots = pcm_lrck_period / binds->slot_width;
	}
#else
	binds->slot_width = default_mach_bind_param.slot_width;
	binds->slots = default_mach_bind_param.slots;
#endif

	return 0;
}

static int sunxi_sound_parse_tdm_clk(enum snd_platform_type plat_type, const char* prefix,
		struct sunxi_sound_mach_bind_props *bind_props)
{
	char i2s_name[16];
	int ret;
	int32_t tmp_val;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;

#ifdef CONFIG_DRIVER_SYSCONFIG
	snprintf(i2s_name, sizeof(i2s_name), "%s%d", prefix, tdm_num);

	ret = hal_cfg_get_keyvalue(i2s_name, "cpu-pll-fs", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:cpu-pll-fs miss.\n");
		bind_props->cpu_pll_fs = defaul_tmach_bind_props_param.cpu_pll_fs;
	} else {
		bind_props->cpu_pll_fs = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "codec-pll-fs", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:codec-pll-fs miss.\n");
		bind_props->codec_pll_fs = defaul_tmach_bind_props_param.codec_pll_fs;
	} else {
		bind_props->codec_pll_fs = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "mclk-fp", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:mclk-fp miss.\n");
		bind_props->mclk_fp = defaul_tmach_bind_props_param.mclk_fp;
	} else {
		bind_props->mclk_fp = tmp_val;
	}

	ret = hal_cfg_get_keyvalue(i2s_name, "mclk-fs", (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:mclk-fs miss.\n");
		bind_props->mclk_fs = defaul_tmach_bind_props_param.mclk_fs;
	} else {
		bind_props->mclk_fs = tmp_val;
	}
#else
	bind_props->cpu_pll_fs = defaul_tmach_bind_props_param.cpu_pll_fs;
	bind_props->codec_pll_fs = defaul_tmach_bind_props_param.codec_pll_fs;
	bind_props->mclk_fp = defaul_tmach_bind_props_param.mclk_fp;
	bind_props->mclk_fs = defaul_tmach_bind_props_param.mclk_fs;
#endif

	return 0;
}

static int sunxi_sound_parse_dai_name(const char* prefix, const char* dai_name,
			struct sunxi_sound_adf_dai_bind_component *dbc)
{
	char component_name[32];
	char key_name[32];
	char id_name[32];
	int ret, id;
	int32_t tmp_val;

#ifdef CONFIG_DRIVER_SYSCONFIG
	snprintf(key_name, sizeof(key_name), "%s_mach", prefix);
	ret = hal_cfg_get_keyvalue(key_name, (char*)dai_name, (int32_t *)component_name, sizeof(component_name) / sizeof(int));
	if (ret) {
		snd_info("mach:%s miss.\n", dai_name);
		dbc->name = NULL;
		return 0;
	} else {
		dbc->name = sunxi_sound_strdup_const(component_name);
		if (!dbc->name)
			return -ENOMEM;
	}

	snprintf(id_name, sizeof(id_name), "%s-id", dai_name);
	ret = hal_cfg_get_keyvalue(key_name, id_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:%s miss.\n", dai_name);
		id = 0;
	} else {
		id = tmp_val;
	}
#else
	dbc->name = COMPONENT_CPU_NAME;
	id = 0;
	snd_err("Please USE sysconfig to config cpu name to match your soundcard, default name is %s", dbc->name);
#endif

	if (dbc->name) {
		ret = sunxi_sound_adf_find_dai_name(dbc->name, id, &dbc->bind_name);
		if (ret != 0) {
			snd_err("mach:sunxi_sound_adf_find_dai_name err.\n");
			return ret;
		}
	}
	return 0;
}

static int sunxi_sound_parse_cpus_bind(enum snd_platform_type plat_type,
			struct sunxi_sound_adf_dai_bind_component *cpus)
{
	int ret;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;
	char prefix_name[16];
	char dai_name[16];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			snprintf(dai_name, sizeof(dai_name), "%s", "cpu");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			snprintf(dai_name, sizeof(dai_name), "cpu-%s", "dac");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			snprintf(dai_name, sizeof(dai_name), "cpu-%s", "adc");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
			snprintf(dai_name, sizeof(dai_name), "%s", "cpu");
			snprintf(prefix_name, sizeof(prefix_name) ,"i2s%d", tdm_num);
			break;
		case SND_PLATFORM_TYPE_DMIC:
			snprintf(dai_name, sizeof(dai_name), "%s", "cpu");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "dmic");
			break;
		case SND_PLATFORM_TYPE_OWA:
			snprintf(dai_name, sizeof(dai_name), "%s", "cpu");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "owa");
			break;
		default:
			ret = -1;
			snd_err("mach: parse_cpus_bind type %d invaild.\n", plat_type);
			goto err;
	}

	ret = sunxi_sound_parse_dai_name(prefix_name, dai_name, cpus);
	if (ret < 0)
		goto err;

err:
	return ret;
}

static int sunxi_sound_parse_platform_bind(enum snd_platform_type plat_type,
			struct sunxi_sound_adf_dai_bind_component *plat)
{
	int ret;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;
	char prefix_name[16];
	char dai_name[16];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			snprintf(dai_name, sizeof(dai_name), "%s", "plat");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			snprintf(dai_name, sizeof(dai_name), "plat-%s", "dac");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			snprintf(dai_name, sizeof(dai_name), "plat-%s", "adc");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
			snprintf(dai_name, sizeof(dai_name), "%s", "plat");
			snprintf(prefix_name, sizeof(prefix_name) ,"i2s%d", tdm_num);
			break;
		case SND_PLATFORM_TYPE_DMIC:
			snprintf(dai_name, sizeof(dai_name), "%s", "plat");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "dmic");
			break;
		case SND_PLATFORM_TYPE_OWA:
			snprintf(dai_name, sizeof(dai_name), "%s", "plat");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "owa");
			break;
		default:
			ret = -1;
			snd_err("mach: parse_platform_bind type %d invaild.\n", plat_type);
			goto err;
	}

	ret = sunxi_sound_parse_dai_name(prefix_name, dai_name, plat);
	if (ret < 0)
		goto err;

err:
	return ret;
}

static int sunxi_sound_get_codecs_bind(const char* prefix, const char* dai_name,
			struct sunxi_sound_adf_dai_bind *dai_bind)
{
	char component_name[32];
	char key_name[32];
	char id_name[32];
	char codecnum_name[32];
	int ret, codec_num = 0, i;
	int32_t tmp_val;
	int *id = NULL;
	struct sunxi_sound_adf_dai_bind_component *component = NULL;

#ifdef CONFIG_DRIVER_SYSCONFIG
	snprintf(key_name, sizeof(key_name), "%s_mach", prefix);
	snprintf(codecnum_name, sizeof(codecnum_name), "%s-num", dai_name);
	ret = hal_cfg_get_keyvalue(key_name, codecnum_name, (int32_t *)&tmp_val, 1);
	if (ret) {
		snd_info("mach:%s miss.\n", codecnum_name);
		codec_num = ret;
	} else {
		codec_num = tmp_val;
	}
#else
	codec_num = 1;
#endif

	if (codec_num <= 0) {
		snd_err("%s %d is invaild\n", codecnum_name, codec_num);
		return codec_num;
	}


	component = sound_malloc(codec_num * sizeof(*component));
	if (!component) {
		ret = -ENOMEM;
		goto err;
	}

	id = sound_malloc(codec_num * sizeof(*id));
	if (!id) {
		ret = -ENOMEM;
		goto err;
	}

	dai_bind->codecs = component;
	dai_bind->num_codecs = codec_num;

	for (i = 0; i < codec_num; i++) {
#ifdef CONFIG_DRIVER_SYSCONFIG
		ret = hal_cfg_get_keyvalue(key_name, (char*)dai_name, (int32_t *)component_name, sizeof(component_name) / sizeof(int));
		if (ret) {
			snd_info("mach:%s miss.\n", dai_name);
			component[i].name = NULL;
		} else {
			component[i].name = sunxi_sound_strdup_const(component_name);
			if (!component[i].name) {
				ret = -ENOMEM;
				goto err;
			}
		}

		snprintf(id_name, sizeof(id_name), "%s-id-%d", dai_name, i);
		ret = hal_cfg_get_keyvalue(key_name, id_name, (int32_t *)&tmp_val, 1);
		if (ret) {
			snd_info("mach:%s miss.\n", id_name);
			id[i] = i;
		} else {
			id[i] = tmp_val;
		}
#else
		component[i].name =COMPONENT_CODEC_NAME;
		id[i] = i;
		snd_err("Please USE sysconfig to config codec name to match your soundcard, default name is %s", component[i].name);
#endif

	}

	ret = sunxi_sound_adf_get_dai_bind_codecs(dai_bind, id);
	if (ret != 0) {
		snd_err("mach:sunxi_sound_adf_get_dai_bind_codecs err.\n");
		goto err;
	}

	return 0;

err:
	for (i = 0; i < codec_num; i++) {
		if (component[i].name) {
			sound_free(component[i].name);
			component[i].name = NULL;
		}
	}
	if (component)
		sound_free(component);
	if (id)
		sound_free(id);

	return ret;
}


static int sunxi_sound_parse_codecs_bind(enum snd_platform_type plat_type,
				struct sunxi_sound_adf_dai_bind *dai_bind)
{
	int ret;
	int tdm_num = plat_type - SND_PLATFORM_TYPE_I2S0;
	char prefix_name[16];
	char dai_name[16];

	switch (plat_type) {
		case SND_PLATFORM_TYPE_CPUDAI:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_DAC:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs-dac");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_CPUDAI_ADC:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs-adc");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "audiocodec");
			break;
		case SND_PLATFORM_TYPE_I2S0:
		case SND_PLATFORM_TYPE_I2S1:
		case SND_PLATFORM_TYPE_I2S2:
		case SND_PLATFORM_TYPE_I2S3:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs");
			snprintf(prefix_name, sizeof(prefix_name), "i2s%d", tdm_num);
			break;
		case SND_PLATFORM_TYPE_DMIC:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "dmic");
			break;
		case SND_PLATFORM_TYPE_OWA:
			snprintf(dai_name, sizeof(dai_name), "%s", "codecs");
			snprintf(prefix_name, sizeof(prefix_name), "%s", "owa");
			break;
		default:
			ret = -1;
			snd_err("mach: parse_codecs_bind type %d invaild.\n", plat_type);
			goto err;
	}

	ret = sunxi_sound_get_codecs_bind(prefix_name, dai_name, dai_bind);
	if (ret < 0) {
		goto err;
	}

err:
	return ret;
}

static int sunxi_sound_set_dai_bind_name(struct sunxi_sound_adf_dai_bind *dai_bind)
{
	char name[64];
	int i;
	int len = 0;

	if (dai_bind->cpus->name == NULL || dai_bind->codecs->name == NULL) {
		snd_err("cpus name or codec name is NULL\n");
		return -EINVAL;
	}

	snprintf(name, sizeof(name), "%s-", dai_bind->cpus->name);
	len = sizeof(name) - strlen(dai_bind->cpus->name) - 1;
	for (i = 0; i < dai_bind->num_codecs; i++) {
		if (len < strlen(dai_bind->codecs[i].name) -1)
			break;
		strncat(name, dai_bind->codecs[i].name, strlen(dai_bind->codecs[i].name));
		if (i != dai_bind->num_codecs - 1)
			strncat(name, "-", 2);
		len -= strlen(dai_bind->codecs[i].name) + 2;
	}

	dai_bind->name = sunxi_sound_strdup_const(name);
	if (!dai_bind->name) {
		snd_err("malloc failed");
		return -ENOMEM;
	}
	dai_bind->stream_name = dai_bind->name;
	return 0;
}

static int sunxi_sound_canonicalize_platform(struct sunxi_sound_adf_dai_bind *dai_bind)
{
	char name[48];

	/* Assumes platform == cpu */
	if (!dai_bind->platforms->name && dai_bind->cpus->name) {
		snprintf(name, sizeof(name), "%s-dma", dai_bind->cpus->name);
		dai_bind->platforms->name = sunxi_sound_strdup_const(name);
		if (!dai_bind->platforms->name) {
			snd_err("malloc failed");
			return -ENOMEM;
		}
	}

	if (!dai_bind->platforms->name)
		dai_bind->num_platforms = 0;

	return 0;
}

static int sunxi_sound_hw_params(struct sunxi_sound_pcm_dataflow *dataflow,
			struct sunxi_sound_pcm_hw_params *hw_params)
{
	struct sunxi_sound_adf_pcm_running_param *rtp = dataflow->priv_data;
	struct sunxi_sound_adf_dai *codec_dai;
	struct sunxi_sound_adf_dai *cpu_dai = adf_rtp_to_cpu(rtp, 0);
	struct sunxi_sound_mach_priv *priv = sunxi_sound_card_get_drvdata(rtp->card);
	struct sunxi_sound_adf_dai_bind *dai_bind = priv->dai_bind + rtp->num;
	struct sunxi_sound_mach_bind_props *dai_props = priv->bind_props + rtp->num;
	struct sunxi_sound_mach_bind *binds = priv->binds;

	unsigned int mclk;
	unsigned int cpu_pll_clk, codec_pll_clk;
	unsigned int cpu_bclk_ratio, codec_bclk_ratio;
	unsigned int freq_point;
	int cpu_clk_div, codec_clk_div;
	int i, ret = 0;

	switch (params_rate(hw_params)) {
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
	case 192000:
		freq_point = 24576000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
	case 176400:
		freq_point = 22579200;
		break;
	default:
		snd_err("Invalid rate %d\n", params_rate(hw_params));
		return -EINVAL;
	}

	/* for cpudai pll clk */
	cpu_pll_clk	= freq_point * dai_props->cpu_pll_fs;
	codec_pll_clk	= freq_point * dai_props->codec_pll_fs;
	cpu_clk_div	= cpu_pll_clk / params_rate(hw_params);
	codec_clk_div	= codec_pll_clk / params_rate(hw_params);
	snd_info("freq point   : %u\n", freq_point);
	snd_info("cpu pllclk   : %u\n", cpu_pll_clk);
	snd_info("codec pllclk : %u\n", codec_pll_clk);
	snd_info("cpu clk_div  : %u\n", cpu_clk_div);
	snd_info("codec clk_div: %u\n", codec_clk_div);

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_pll) {
		ret = sunxi_sound_adf_dai_set_pll(cpu_dai, dataflow->stream, 0,
					  cpu_pll_clk, cpu_pll_clk);
		if (ret) {
			snd_err("cpu_dai set pllclk failed\n");
			return ret;
		}
	} else if (cpu_dai->component->driver->set_pll) {
		ret = sunxi_sound_adf_component_set_pll(cpu_dai->component, dataflow->stream, 0,
						cpu_pll_clk, cpu_pll_clk);
		if (ret) {
			snd_err("cpu_dai set pllclk failed\n");
			return ret;
		}
	}

	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_pll) {
			ret = sunxi_sound_adf_dai_set_pll(codec_dai, dataflow->stream, 0,
						  codec_pll_clk, codec_pll_clk);
			if (ret) {
				snd_err("codec_dai set pllclk failed\n");
				return ret;
			}
		} else if (codec_dai->component->driver->set_pll) {
			ret = sunxi_sound_adf_component_set_pll(codec_dai->component, dataflow->stream, 0,
							codec_pll_clk, codec_pll_clk);
			if (ret) {
				snd_err("codec_dai set pllclk failed\n");
				return ret;
			}
		}
	}

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_clkdiv) {
		ret = sunxi_sound_adf_dai_set_clkdiv(cpu_dai, 0, cpu_clk_div);
		if (ret) {
			snd_err("cpu_dai set clk_div failed\n");
			return ret;
		}
	}

	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_clkdiv) {
			ret = sunxi_sound_adf_dai_set_clkdiv(codec_dai, 0, codec_clk_div);
			if (ret) {
				snd_err("cadec_dai set clk_div failed.\n");
				return ret;
			}
		}
	}

	/* use for i2s/pcm only */
	if (!(binds->slots && binds->slot_width))
		return 0;

	/* for cpudai & codecdai mclk */
	if (dai_props->mclk_fp)
		mclk = (freq_point >> 1) * dai_props->mclk_fs;
	else
		mclk = params_rate(hw_params) * dai_props->mclk_fs;
	cpu_bclk_ratio = cpu_pll_clk / (params_rate(hw_params) * binds->slot_width * binds->slots);
	codec_bclk_ratio = codec_pll_clk / (params_rate(hw_params) * binds->slot_width * binds->slots);
	snd_debug("mclk            : %u\n", mclk);
	snd_debug("cpu_bclk_ratio  : %u\n", cpu_bclk_ratio);
	snd_debug("codec_bclk_ratio: %u\n", codec_bclk_ratio);

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_sysclk) {
		ret = sunxi_sound_adf_dai_set_sysclk(cpu_dai, 0, mclk, SUNXI_SOUND_ADF_CLOCK_OUT);
		if (ret) {
			snd_err("cpu_dai set sysclk(mclk) failed\n");
			return ret;
		}
	}
	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_sysclk) {
			ret = sunxi_sound_adf_dai_set_sysclk(codec_dai, 0, mclk, SUNXI_SOUND_ADF_CLOCK_IN);
			if (ret) {
				snd_err("cadec_dai set sysclk(mclk) failed\n");
				return ret;
			}
		}
	}

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_bclk_ratio) {
		ret = sunxi_sound_adf_dai_set_bclk_ratio(cpu_dai, cpu_bclk_ratio);
		if (ret) {
			snd_err("cpu_dai set bclk failed\n");
			return ret;
		}
	}
	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_bclk_ratio) {
			ret = sunxi_sound_adf_dai_set_bclk_ratio(codec_dai, codec_bclk_ratio);
			if (ret) {
				snd_err("codec_dai set bclk failed\n");
				return ret;
			}
		}
	}

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_fmt) {
		ret = sunxi_sound_adf_dai_set_fmt(cpu_dai, dai_bind->dai_fmt);
		if (ret) {
			snd_err("cpu dai set fmt failed\n");
			return ret;
		}
	}
	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_fmt) {
			ret = sunxi_sound_adf_dai_set_fmt(codec_dai, dai_bind->dai_fmt);
			if (ret) {
				snd_err("codec dai set fmt failed\n");
				return ret;
			}
		}
	}

	if (cpu_dai->driver->ops && cpu_dai->driver->ops->set_tdm_slot) {
		ret = sunxi_sound_adf_set_tdm_slot(cpu_dai, 0, 0, binds->slots, binds->slot_width);
		if (ret) {
			snd_err("cpu dai set tdm slot failed\n");
			return ret;
		}
	}
	for_each_rtp_codec_dais(rtp, i, codec_dai) {
		if (codec_dai->driver->ops && codec_dai->driver->ops->set_tdm_slot) {
			ret = sunxi_sound_adf_set_tdm_slot(codec_dai, 0, 0, binds->slots,
							binds->slot_width);
			if (ret) {
				snd_err("codec dai set tdm slot failed\n");
				return ret;
			}
		}
	}

	return 0;

}

static struct sunxi_sound_adf_ops sunxi_sound_mach_ops = {
	.hw_params = sunxi_sound_hw_params,
};

static int sunxi_sound_parse_bind_link(struct sunxi_sound_mach_priv *priv)
{
	struct sunxi_sound_adf_dai_bind *dai_bind = priv->dai_bind;
	struct sunxi_sound_mach_bind_props *bind_props = priv->bind_props;
	int ret, i;

	ret = sunxi_sound_parse_daifmt(priv->plat_type, "i2s", &dai_bind->dai_fmt);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_daifmt failed");
		goto err;
	}

	ret = sunxi_sound_parse_daistream(priv->plat_type, dai_bind);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_daistream failed");
		goto err;
	}

	ret = sunxi_sound_parse_tdm_slot(priv->plat_type, "i2s", priv->binds);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_tdm_slot failed");
		goto err;
	}

	ret = sunxi_sound_parse_cpus_bind(priv->plat_type, dai_bind->cpus);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_cpus_bind failed");
		goto err;
	}

	ret = sunxi_sound_parse_codecs_bind(priv->plat_type, dai_bind);
	if (ret < 0) {
		if (ret == -SUNXI_ERROEPROBE)
			goto err;
		dai_bind->codecs = sound_malloc(sizeof(*(dai_bind->codecs)));
		if (!dai_bind->codecs) {
			snd_err("malloc failed");
			ret = -ENOMEM;
			goto err;
		}
		dai_bind->num_codecs = 1;
		dai_bind->codecs->name = sunxi_sound_strdup_const("sunxi-snd-codec-dummy");
		if (!dai_bind->codecs->name) {
			snd_err("malloc failed");
			ret = -ENOMEM;
			goto err;
		}
		dai_bind->codecs->bind_name = "sunxi-snd-codec-dummy-dai";
	}

	ret = sunxi_sound_parse_platform_bind(priv->plat_type, dai_bind->platforms);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_platform_bind failed");
		goto err;
	}

	ret = sunxi_sound_parse_tdm_clk(priv->plat_type, "i2s", bind_props);
	if (ret < 0) {
		snd_err("sunxi_sound_parse_tdm_clk failed");
		goto err;
	}

	ret = sunxi_sound_set_dai_bind_name(dai_bind);
	if (ret < 0) {
		snd_err("sunxi_sound_set_dai_bind_name failed\n");
		goto err;
	}

	dai_bind->ops = &sunxi_sound_mach_ops;

	snd_info("name   : %s\n", dai_bind->stream_name);
	snd_info("format : %x\n", dai_bind->dai_fmt);
	snd_info("cpu    : %s\n", dai_bind->cpus->bind_name);

	for (i = 0; i < dai_bind->num_codecs; i++)
		snd_info("codec[%d] : %s\n", i, dai_bind->codecs[i].bind_name);

	ret = sunxi_sound_canonicalize_platform(dai_bind);
	if (ret < 0) {
		snd_err("sunxi_sound_canonicalize_platform failed");
		goto err;
	}
err:
	return ret;
}


int sunxi_sound_card_register(const char* name, enum snd_platform_type plat_type)
{
	struct sunxi_sound_mach_priv *priv;
	struct sunxi_sound_adf_card *card;
	int ret;

	card = find_sound_card_by_name(name);
	if (card != NULL) {
		snd_err("card:%s already registered\n", name);
		return -1;
	}

	priv = sound_malloc(sizeof(struct sunxi_sound_mach_priv));
	if (priv == NULL) {
		snd_err("card:%s malloc failed\n", name);
		return -ENOMEM;
	}
	card = &priv->card;

	card->name = sunxi_sound_strdup_const(name);
	if (!card->name) {
		snd_err("card:%s malloc failed\n", name);
		return -ENOMEM;
	}

	ret = sunxi_sound_mach_init_priv(priv);
	if (ret < 0) {
		snd_err("card:%s sunxi_sound_mach_init_priv failed\n", name);
		goto err;
	}
	ret = sunxi_sound_card_register_component(plat_type);
	if (ret < 0) {
		snd_err("card:%s sunxi_sound_card_register_component failed\n", name);
		goto err;
	}

	priv->plat_type = plat_type;

	ret = sunxi_sound_parse_bind_link(priv);
	if (ret < 0) {
		snd_err("card:%s sunxi_sound_parse_bind_link failed\n", name);
		goto err;
	}

	sunxi_sound_card_set_drvdata(card, priv);

	ret = sunxi_sound_adf_register_card(card);
	if (ret < 0) {
		snd_err("card:%s sunxi_sound_adf_register_card failed\n", name);
		goto err;
	}

	list_add(&priv->list, &gSunxiSoundCardList);

	return 0;

err:
	sunxi_sound_card_clean_resources(priv);
	return ret;
}

int sunxi_sound_card_unregister(const char* name)
{
	struct sunxi_sound_adf_card *card;
	struct sunxi_sound_mach_priv *priv;


	card = find_sound_card_by_name(name);
	if (card == NULL) {
		snd_err("card:%s already unregistered\n", name);
		return -EINVAL;
	}

	snd_info("\n");

	priv = sunxi_sound_card_get_drvdata(card);
	if (priv == NULL) {
		snd_err("card:%s priv is null\n", name);
		return -EINVAL;
	}

	snd_info("\n");

	sunxi_sound_card_unregister_component(priv->plat_type);

	snd_info("\n");

	sunxi_sound_adf_unregister_card(card);

	snd_info("\n");

	sunxi_sound_card_clean_resources(priv);

	list_del(&priv->list);
	sound_free(priv);
	return 0;
}


