#ifndef __AUDIO_PROCESS_H__
#define __AUDIO_PROCESS_H__

struct audio_process;

struct audio_process *audio_process_init(int sample, int channel);

int audio_process_deinit(struct audio_process *audio_p);

int audio_process_input(struct audio_process *audio_p, uint8_t *data, int len);

int audio_process_register_output(struct audio_process *audio_p,
     int (*cb)(uint8_t *data, int size, int is_vad));

int audio_process_start(struct audio_process *audio_p);

int audio_process_stop(struct audio_process *audio_p);

#endif
