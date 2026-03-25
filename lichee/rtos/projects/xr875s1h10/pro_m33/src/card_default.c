/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
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
#include <sound_v2/sunxi_sound_card.h>
#include <aw_common.h>
#ifndef CONFIG_KERNEL_FREERTOS
#include <init.h>
#endif
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif

#ifdef CONFIG_DRIVER_SYSCONFIG
static int sunxi_sound_get_card_name(const char* prefix, const char* card, char* card_name)
{
	char key_name[32];
	char val_name[32];
	char get_card_name[32];
	int ret;

	snprintf(key_name, sizeof(key_name), "%s_mach", prefix);
	if (card)
		snprintf(val_name, sizeof(val_name), "%s-name", card);
	else
		snprintf(val_name, sizeof(val_name), "%s", "name");

	ret = hal_cfg_get_keyvalue(key_name, val_name, (int32_t *)get_card_name, sizeof(get_card_name) / sizeof(int));
	if (ret) {
		snd_info("soundcards:%s miss.\n", card);
	} else {
		snprintf(card_name, sizeof(get_card_name), "%s", get_card_name);
	}

	return ret;
}
#endif

int sunxi_sound_card_initialize(void)
{
	int ret = 0;
	char card_name[32];

	//maybe unused for compile warning.
	UNUSED(ret);
	UNUSED(card_name);
/* ------------------------------------------------------------------------- */
/* AUDIOCODEC */
/* ------------------------------------------------------------------------- */
#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodec");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodec");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_CPUDAI);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec register success!\n");
	} else {
		snd_err("soundcards: audiocodec register failed!\n");
	}
#else
#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", "dac", card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodecdac");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodecdac");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_CPUDAI_DAC);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec dac register success!\n");
	} else {
		snd_err("soundcards: audiocodec dac register failed!\n");
	}
#endif
#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", "adc", card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodecadc");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodecadc");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_CPUDAI_ADC);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec adc register success!\n");
	} else {
		snd_err("soundcards: audiocodec adc register failed!\n");
	}
#endif
#endif

/* ------------------------------------------------------------------------- */
/* i2s */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S0

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s0", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s0");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s0");
#endif

	/* register i2s0 sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_I2S0);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S1

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s1", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s1");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s1");
#endif

	/* register i2s1 sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_I2S1);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S2

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s2", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s2");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s2");
#endif

	/* register i2s2 sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_I2S2);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S3

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s3", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s3");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s3");
#endif

	/* register i2s3 sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_I2S3);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* DMIC */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_DMIC

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("dmic", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "snddmic");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "snddmic");
#endif

	/* register dmic sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_DMIC);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* OWA */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_OWA

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("owa", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndowa");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndowa");
#endif

	/* register owa sound card */
	ret = sunxi_sound_card_register(card_name, SND_PLATFORM_TYPE_OWA);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

	/* Sound cards list */
	//snd_card_list();

	return 0;
}

int sunxi_sound_card_deinitialize(void)
{
	int ret = 0;
	char card_name[32];

	//maybe unused for compile warning.
	UNUSED(ret);
	UNUSED(card_name);
/* ------------------------------------------------------------------------- */
/* AUDIOCODEC */
/* ------------------------------------------------------------------------- */
#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC) && !defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodec");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodec");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec register success!\n");
	} else {
		snd_err("soundcards: audiocodec register failed!\n");
	}
#else
#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_DAC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", "dac", card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodecdac");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodecdac");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec dac register success!\n");
	} else {
		snd_err("soundcards: audiocodec dac register failed!\n");
	}
#endif

#if defined(CONFIG_SUNXI_SOUND_PLATFORM_CPUDAI) && defined(CONFIG_SUNXI_SOUND_CODEC_AUDIOCODEC_ADC)

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("audiocodec", "adc", card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "audiocodecadc");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "audiocodecadc");
#endif

	/* register audiocodec sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: audiocodec adc register success!\n");
	} else {
		snd_err("soundcards: audiocodec adc register failed!\n");
	}
#endif
#endif

/* ------------------------------------------------------------------------- */
/* i2s */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S0

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s0", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s0");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s0");
#endif

	/* register i2s0 sound card */
	ret = sunxi_sound_card_unregister("sndi2s0");
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S1

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s1", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s1");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s1");
#endif

	/* register i2s1 sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S2

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s2", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s2");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s2");
#endif

	/* register i2s2 sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

#ifdef CONFIG_SUNXI_SOUND_PLATFORM_I2S3

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("i2s3", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndi2s3");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndi2s3");
#endif

	/* register i2s3 sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* DMIC */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_DMIC
#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("dmic", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "snddmic");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "snddmic");
#endif
	/* register dmic sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* OWA */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SUNXI_SOUND_PLATFORM_OWA

#ifdef CONFIG_DRIVER_SYSCONFIG
	ret = sunxi_sound_get_card_name("owa", NULL, card_name);
	if (ret != 0) {
		snd_info("soundcards: sunxi_sound_get_card_name failed, using default name\n");
		snprintf(card_name, sizeof(card_name), "%s", "sndowa");
	}
#else
	snprintf(card_name, sizeof(card_name), "%s", "sndowa");
#endif

	/* register owa sound card */
	ret = sunxi_sound_card_unregister(card_name);
	if (ret == 0) {
		snd_debug("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

	return 0;
}

#ifndef CONFIG_KERNEL_FREERTOS
late_initcall(sunxi_sound_card_initialize);
#endif
