#ifndef _OPUS_WRAPPER_H_
#define _OPUS_WRAPPER_H_

#if __cplusplus
extern "C" {
#endif
struct opus_wraper_decoder;

struct opus_wraper_decoder* opus_wraper_decoder_create(int sample_rate,
		int channels, int duration_ms);

void opus_wraper_decoder_destroy(struct opus_wraper_decoder *dec);

int opus_wraper_decoder_decode(struct opus_wraper_decoder *dec,
		const uint8_t* opus, size_t opus_size,
		uint8_t* pcm, size_t pcm_bytes, size_t *dec_size);

void opus_wraper_decoder_reset_state(struct opus_wraper_decoder* dec);

struct opus_wraper_encoder;

struct opus_wraper_encoder* opus_wraper_encoder_create(int sample_rate,
		int channels, int duration_ms);

void opus_wraper_encoder_destroy(struct opus_wraper_encoder* enc);

int opus_wraper_encoder_encode(struct opus_wraper_encoder* enc,
		uint8_t* pcm, size_t pcm_bytes,
		void (*handler)(uint8_t* opus, size_t opus_size));

int opus_wraper_encoder_is_buffer_empty(const struct opus_wraper_encoder* enc);

void opus_wraper_encoder_reset_state(struct opus_wraper_encoder* enc);

#if __cplusplus
extern "C" {
#endif
#endif // _opus_wraper_WRAPPER_H_
