#ifndef _AUDIO_CODEC_H
#define _AUDIO_CODEC_H

#if __cplusplus
extern "C" {
#endif
struct audio_codec;

struct audio_codec *audio_codec_init(int play_sample, int play_channel,
		int record_sample, int record_channel,
		int (*rx_cb)(uint8_t *data, int size, int is_vad));

void audio_codec_deinit(struct audio_codec *codec);

void audio_codec_set_output_volume(struct audio_codec *codec, int volume);

void audio_codec_stop_output(struct audio_codec *codec);

void audio_codec_start_output(struct audio_codec *codec);

void audio_codec_output_data(struct audio_codec *codec, uint8_t *data, int size);

#if __cplusplus
extern "C" {
#endif
#endif // _AUDIO_CODEC_H
