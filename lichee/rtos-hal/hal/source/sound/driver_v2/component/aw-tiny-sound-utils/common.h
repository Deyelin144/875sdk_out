#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	struct sunxi_pcm *handle;
	unsigned int card;
	unsigned int device;
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int channels;
	unsigned int format_bits;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

	snd_pcm_uframes_t frame_bytes;
	snd_pcm_uframes_t chunk_size;

	unsigned in_aborting;
	unsigned int capture_duration;
} sound_mgr_t;

void sunxi_pcm_xrun(struct sunxi_pcm *handle);
void sunxi_do_pause(struct sunxi_pcm *handle);
void sunxi_do_other_test(struct sunxi_pcm *handle);

sound_mgr_t *sunxi_sound_mgr_create(void);
void sunxi_sound_mgr_release(sound_mgr_t *mgr);
int sunxi_set_param(struct sunxi_pcm *handle, snd_pcm_format_t format,
			unsigned int rate, unsigned int channels,
			snd_pcm_uframes_t period_size,
			snd_pcm_uframes_t buffer_size);

snd_pcm_sframes_t sunxi_pcm_write(struct sunxi_pcm *handle, char *data,
	      snd_pcm_uframes_t frames_total,
	      unsigned int frame_bytes);
snd_pcm_sframes_t sunxi_pcm_read(struct sunxi_pcm *handle, const char *data,
	     snd_pcm_uframes_t frames_total,
	     unsigned int frame_bytes);


#ifdef __cplusplus
};  // extern "C"
#endif
#endif /* __COMMON_H */
