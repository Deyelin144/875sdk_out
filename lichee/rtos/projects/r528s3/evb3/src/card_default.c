#include <sound/snd_core.h>
#include "rtos-hal/hal/source/sound/platform/sunxi-daudio.h"

#ifdef CONFIG_SND_CODEC_AC108
extern struct snd_codec ac108_codec;
#endif
#ifdef CONFIG_SND_CODEC_SUN8IW20_AUDIOCODEC
extern struct snd_codec sunxi_audiocodec;
#endif
#ifdef CONFIG_SND_CODEC_DUMMY
extern struct snd_codec dummy_codec;
#endif

void snd_core_version(void);
void snd_pcm_version(void);
int sunxi_soundcard_init(void)
{
	int ret = 0;
	char *card_name = NULL;
	struct snd_codec *audio_codec = NULL;

	snd_core_version();
	snd_pcm_version();
#ifdef CONFIG_SND_CODEC_SUN8IW20_AUDIOCODEC
        //register audiocodec sound card
	card_name = "audiocodec";
	audio_codec = &sunxi_audiocodec;
	ret = snd_card_register(card_name, audio_codec,
					SND_PLATFORM_TYPE_CPUDAI);
	if (ret != 0)
		snd_err("card: %s registere failed.\n", card_name);
#endif

#if defined(CONFIG_SND_PLATFORM_SUNXI_DAUDIO)
        //register daudio0 sound card
	card_name = "snddaudio0";
#ifdef CONFIG_SND_CODEC_AC108
	audio_codec = &ac108_codec;
#else
	audio_codec = &dummy_codec;
#endif
	ret = snd_card_register(card_name, audio_codec,
					SND_PLATFORM_TYPE_DAUDIO0);
	if (ret != 0)
		snd_err("card: %s registere failed.\n", card_name);

        //register daudio2 sound card
	card_name = "snddaudio2";
	audio_codec = &dummy_codec;
	ret = snd_card_register(card_name, audio_codec,
					SND_PLATFORM_TYPE_DAUDIO2);
	if (ret != 0)
		snd_err("card: %s registere failed.\n", card_name);
#endif

#if defined(CONFIG_SND_PLATFORM_SUNXI_DMIC)
        //register dmic sound card
	card_name = "snddmic";
	audio_codec = &dummy_codec;
	ret = snd_card_register(card_name, audio_codec,
					SND_PLATFORM_TYPE_DMIC);
	if (ret != 0)
		snd_err("card: %s registere failed.\n", card_name);
#endif
	snd_card_list();

	return ret;
}

