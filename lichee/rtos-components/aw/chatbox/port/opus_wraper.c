#include "opus_wraper.h"
#include "log.h"
#include "opus.h"
#include "stdlib.h"
#include <string.h>

#define OPUS_APPLICATION OPUS_APPLICATION_VOIP
#define OPUS_COMPLEXITY  2
#define OPUS_SIGNAL_TYPE OPUS_SIGNAL_VOICE

struct opus_wraper_decoder {
    OpusDecoder *opusdec;
    uint8_t *opus_data;
    int duration_bytes;
    int duration_frame;
    int sample_rate;
    int channels;
};

struct opus_wraper_encoder {
    OpusEncoder *opusenc;
    uint8_t *opus_data;
    int duration_bytes;
    int sample_rate;
    int channels;
};

struct opus_wraper_decoder *opus_wraper_decoder_create(int sample_rate, int channels,
                                                       int duration_ms)
{
    struct opus_wraper_decoder *dec = NULL;
    OpusDecoder *opusdec = NULL;
    uint8_t *opus_data = NULL;
    int duration_bytes = 0;
    int duration_frame = 0;
    int err;

    dec = malloc(sizeof(struct opus_wraper_decoder));
    if (dec == NULL)
        return NULL;

    opusdec = opus_decoder_create(sample_rate, 1, &err);
    if (opusdec == NULL)
        goto opus_dec_exit;

    duration_frame = (sample_rate * duration_ms / 1000);
    duration_bytes = (duration_frame * channels * 2);
    if (duration_bytes <= 0)
        goto opus_dec_exit;

    opus_data = malloc(duration_bytes);
    if (opus_data == NULL)
        goto opus_dec_exit;

    dec->opusdec = opusdec;
    dec->duration_frame = duration_frame;
    dec->duration_bytes = duration_bytes;
    dec->opus_data = opus_data;
    dec->sample_rate = sample_rate;
    dec->channels = channels;

    return dec;

opus_dec_exit:
    return NULL;
}

void opus_wraper_decoder_destroy(struct opus_wraper_decoder *dec)
{
    opus_decoder_destroy(dec->opusdec);
    free(dec->opus_data);
    free(dec);
}

int opus_wraper_decoder_decode(struct opus_wraper_decoder *dec, const uint8_t *opus,
                               size_t opus_size, uint8_t *pcm, size_t pcm_size, size_t *dec_size)
{
    int ret = 0;

    ret = opus_decode(dec->opusdec, opus, opus_size, (short *)dec->opus_data, dec->duration_frame,
                      0);
    if (ret <= 0) {
        *dec_size = 0;
        return -1;
    }

    ret = (ret * dec->channels * 2);

    if (ret > pcm_size) {
        CHATBOX_WARNG("buff is not big enough\n");
        ret = pcm_size;
    }

    memcpy(pcm, dec->opus_data, ret);
    *dec_size = ret;

    return ret;
}

void opus_wraper_decoder_reset_state(struct opus_wraper_decoder *dec)
{
    // nothing
}

struct opus_wraper_encoder *opus_wraper_encoder_create(int sample_rate, int channels,
                                                       int duration_ms)
{
    struct opus_wraper_encoder *enc = NULL;
    OpusEncoder *opusenc = NULL;
    uint8_t *opus_data = NULL;
    int duration_bytes = 0;
    int err;

    enc = malloc(sizeof(struct opus_wraper_encoder));
    if (enc == NULL)
        return NULL;

    duration_bytes = (sample_rate * duration_ms / 1000 * channels * 2);
    if (duration_bytes <= 0)
        goto opus_enc_exit;

    opus_data = malloc(duration_bytes);
    if (opus_data == NULL)
        goto opus_enc_exit;

    CHATBOX_INFO("opus version:%s\n", opus_get_version_string());
    opusenc = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION, &err);
    if (err != OPUS_OK)
        goto opus_enc_exit;

    opus_encoder_ctl(opusenc, OPUS_SET_BITRATE(OPUS_AUTO));
    opus_encoder_ctl(opusenc, OPUS_SET_COMPLEXITY(OPUS_COMPLEXITY));
    opus_encoder_ctl(opusenc, OPUS_SET_SIGNAL(OPUS_SIGNAL_TYPE));

    enc->opusenc = opusenc;
    enc->opus_data = opus_data;
    enc->duration_bytes = duration_bytes;
    enc->sample_rate = sample_rate;
    enc->channels = channels;

    return enc;
opus_enc_exit:
    return NULL;
}

void opus_wraper_encoder_destroy(struct opus_wraper_encoder *enc)
{
    opus_encoder_destroy(enc->opusenc);
    free(enc->opus_data);
    free(enc);
}

int opus_wraper_encoder_encode(struct opus_wraper_encoder *enc, uint8_t *pcm, size_t pcm_size,
                               void (*handler)(uint8_t *opus, size_t opus_size))
{
    int ret = 0;
    int pcm_frame = 0;

    pcm_frame = (pcm_size / enc->channels / 2);

    ret = opus_encode(enc->opusenc, (short *)pcm, pcm_frame,
                        enc->opus_data, enc->duration_bytes);
    if (ret <= 0) {
        return -1;
    }

    if (handler != NULL) {
        handler(enc->opus_data, ret);
    }

    return ret;
}

int opus_wraper_encoder_is_buffer_empty(const struct opus_wraper_encoder *enc)
{
    return 0;
}

void opus_wraper_encoder_reset_state(struct opus_wraper_encoder *enc)
{
    ;
}
