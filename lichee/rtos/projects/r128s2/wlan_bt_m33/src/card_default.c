#include <sound/snd_core.h>
/* #include "drivers/hal/source/sound/platform/sunxi-daudio.h" */


/* #ifdef CONFIG_SND_CODEC_DUMMY */
extern struct snd_codec dummy_codec;
/* #endif */

void snd_core_version(void);
void snd_pcm_version(void);
int sunxi_soundcard_init(void)
{
	int ret = 0;
	char *card_name = NULL;
	struct snd_codec *audio_codec = NULL;

	snd_core_version();
	snd_pcm_version();


        //register daudio0 sound card
	card_name = "snddaudio0";

	audio_codec = &dummy_codec;
	ret = snd_card_register(card_name, audio_codec,
					SND_PLATFORM_TYPE_DAUDIO0);
	if (ret != 0)
		snd_err("card: %s registere failed.\n", card_name);




	snd_card_list();

	return ret;
}

