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
#include <sound/snd_core.h>
#include <aw_common.h>
#ifndef CONFIG_KERNEL_FREERTOS
#include <init.h>
#endif

#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) \
	&& !defined(CONFIG_SND_CODEC_AUDIOCODEC_DAC) && !defined(CONFIG_SND_CODEC_AUDIOCODEC_ADC)
extern struct snd_codec sunxi_audiocodec;
#else
#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) && defined(CONFIG_SND_CODEC_AUDIOCODEC_DAC)
extern struct snd_codec sunxi_audiocodec_dac;
#endif
#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) && defined(CONFIG_SND_CODEC_AUDIOCODEC_ADC)
extern struct snd_codec sunxi_audiocodec_adc;
#endif
#endif

#ifdef CONFIG_SND_CODEC_AC108
extern struct snd_codec ac108_codec;
#elif CONFIG_SND_CODEC_AC101S
extern struct snd_codec ac101s_codec;
#elif CONFIG_SND_CODEC_AC101
extern struct snd_codec ac101_codec;
#else
extern struct snd_codec dummy_codec;
#endif
#ifdef CONFIG_SND_PLATFORM_SUNXI_DMIC
extern struct snd_codec dmic_codec;
#endif
#ifdef CONFIG_SND_PLATFORM_SUNXI_SPDIF
extern struct snd_codec spdif_codec;
#endif

int sunxi_soundcard_init(void)
{
	int ret = 0;
	char *card_name = NULL;
	struct snd_codec *audio_codec = NULL;

	UNUSED(ret);
	UNUSED(card_name);

/* ------------------------------------------------------------------------- */
/* AUDIOCODEC */
/* ------------------------------------------------------------------------- */
#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) \
	&& !defined(CONFIG_SND_CODEC_AUDIOCODEC_DAC) && !defined(CONFIG_SND_CODEC_AUDIOCODEC_ADC)
	card_name = "audiocodec";
	audio_codec = &sunxi_audiocodec;

	/* register audiocodec sound card */
	ret = snd_card_register(card_name, audio_codec, SND_PLATFORM_TYPE_CPUDAI);
	if (ret == 0) {
		snd_print("soundcards: audiocodec register success!\n");
	} else {
		snd_err("soundcards: audiocodec register failed!\n");
	}
#else
#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) && defined(CONFIG_SND_CODEC_AUDIOCODEC_DAC)
	card_name = "audiocodecdac";
	audio_codec = &sunxi_audiocodec_dac;

	/* register audiocodec sound card */
	ret = snd_card_register(card_name, audio_codec, SND_PLATFORM_TYPE_CPUDAI_DAC);
	if (ret == 0) {
		snd_print("soundcards: audiocodec dac register success!\n");
	} else {
		snd_err("soundcards: audiocodec dac register failed!\n");
	}
#endif
#if defined(CONFIG_SND_PLATFORM_SUNXI_CPUDAI) && defined(CONFIG_SND_CODEC_AUDIOCODEC_ADC)
	card_name = "audiocodecadc";
	audio_codec = &sunxi_audiocodec_adc;

	/* register audiocodec sound card */
	ret = snd_card_register(card_name, audio_codec, SND_PLATFORM_TYPE_CPUDAI_ADC);
	if (ret == 0) {
		snd_print("soundcards: audiocodec adc register success!\n");
	} else {
		snd_err("soundcards: audiocodec adc register failed!\n");
	}
#endif
#endif

/* ------------------------------------------------------------------------- */
/* DAUDIO */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SND_PLATFORM_SUNXI_DAUDIO0
#ifdef CONFIG_SND_CODEC_AC108
	card_name = "ac108";
	audio_codec = &ac108_codec;
#elif defined(CONFIG_SND_CODEC_AC101S)
	card_name = "ac101s";
	audio_codec = &ac101s_codec;
#elif defined(CONFIG_SND_CODEC_AC101)
	card_name = "ac101";
	audio_codec = &ac101_codec;

#else
	card_name = "snddaudio0";
	audio_codec = &dummy_codec;
#endif
	/* register daudio0 sound card */
	ret = snd_card_register("snddaudio0", audio_codec, SND_PLATFORM_TYPE_DAUDIO0);
	if (ret == 0) {
		snd_print("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* DMIC */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SND_PLATFORM_SUNXI_DMIC
	card_name = "snddmic";
	audio_codec = &dmic_codec;
	/* register dmic sound card */
	ret = snd_card_register(card_name, audio_codec, SND_PLATFORM_TYPE_DMIC);
	if (ret == 0) {
		snd_print("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif

/* ------------------------------------------------------------------------- */
/* SPDIF */
/* ------------------------------------------------------------------------- */
#ifdef CONFIG_SND_PLATFORM_SUNXI_SPDIF
	card_name = "sndspdif";
	audio_codec = &spdif_codec;
	/* register spdif sound card */
	ret = snd_card_register(card_name, audio_codec, SND_PLATFORM_TYPE_SPDIF);
	if (ret == 0) {
		snd_print("soundcards: %s register success!\n", card_name);
	} else {
		snd_err("soundcards: %s register failed!\n", card_name);
	}
#endif
	/* Sound cards list */
	snd_card_list();

	return 0;
}

#ifndef CONFIG_KERNEL_FREERTOS
late_initcall(sunxi_soundcard_init);
#endif
