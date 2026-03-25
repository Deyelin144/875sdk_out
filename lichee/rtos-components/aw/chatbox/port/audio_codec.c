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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "audio_codec.h"
#include <AudioSystem.h>
#include "log.h"
#include "task.h"
#include "ringbuffer.h"
#include "audio_process.h"

/*
**************** sysconfig ***************************
[audiocodec]
dacl_vol    = 129
dacr_vol    = 129
lineout_vol = 5
lineoutl_en = 1
lineoutr_en = 1
mic1_gain   = 31
mic2_gain   = 0    // not used
mic3_gain   = 20
mic1_en     = 1
mic2_en     = 0   // not used
mic3_en     = 1
mad_bind_en = 0
pa_pin_msleep   = 10
pa_pin      = port:PA0<1><default><1><1>

[audio_hw_rv]
pcm_cap_rate 		= 16000
pcm_cap_channels 	= 2
pcm_cap_card  		= "hw:audiocodecadc"
pcm_cap_card_num  	= 1
pcm_cap_device_num  = 0

[audio_hw_dsp]
pcm_cap_rate 		= 16000
pcm_cap_channels 	= 2
pcm_cap_bits  		= 16
pcm_cap_period_size  	= 320
pcm_cap_periods  	= 4
pcm_cap_silence_timems  = 300
pcm_cap_card  		= "hw:audiocodecadc"
pcm_cap_card_num  	= 0
pcm_cap_device_num  = 0
*/
extern int msleep(unsigned int msecs);

#define AS_DEFAULT_BITS         16
#define AS_CHAT_SUPPORT_CHANNAL 1

//#define AUDIO_PROCESS_SELF

typedef struct {
    tAudioRecord *ar;
    struct thread *record_thread;
    uint8_t *pcm_buff;
    int buff_bytes;

    int sample;
    int channel;
} _record_t;

typedef struct {
    tAudioTrack *at;
    struct ringbuff *player_rb;
    struct thread *player_thread;
    uint8_t *pcm_buff;
    int buff_bytes;

    int sample;
    int channel;
} _player_t;

struct audio_codec {
    int (*record_cb)(uint8_t *data, int size, int is_vad);

    _record_t *record;
    _player_t *player;
#ifdef AUDIO_PROCESS_SELF
    struct audio_process *audio_p;
#endif
};

static uint8_t *record_data = NULL;
static uint32_t record_bytes = 0;

static void audio_codec_record_thread(void *data)
{
    struct audio_codec *codec = (struct audio_codec *)data;
    while (codec->record == NULL) {
        msleep(10);
    }

    _record_t *record = codec->record;
    uint8_t *record_buff = record->pcm_buff;
    int ret = 0;

    CHATBOX_INFO("record start\n");
    while (1) {
        ret = AudioRecordRead(record->ar, record_buff, record->buff_bytes);
        if (ret != record->buff_bytes) {
            CHATBOX_ERROR("get record data fail!\n");
            continue;
        }

#ifdef AUDIO_PROCESS_SELF
        ret = audio_process_input(codec->audio_p, record_buff, record->buff_bytes);
        if (ret != record->buff_bytes) {
            CHATBOX_WARNG("audio process rb full\n");
        }
#else
        codec->record_cb(record_buff, record->buff_bytes, 1);
#endif
    }
}

static void *audio_codec_record_init(struct audio_codec *codec, int sample, int channel)
{
    _record_t *record = NULL;
    struct thread *record_thread = NULL;
    tAudioRecord *ar = NULL;
    struct ringbuff *record_rb = NULL;
    uint8_t *pcm_buff = NULL;
    int buff_bytes = 0;
    int ret;

    ar = AudioRecordCreate("default");
    if (ar == NULL)
        return NULL;

    channel *= 2; // FOR AEC
    ret = AudioRecordSetup(ar, sample, channel, AS_DEFAULT_BITS);
    if (ret < 0)
        goto record_init_err;

    buff_bytes = ((sample * 20 / 1000) * channel * 2); // 20ms
    pcm_buff = malloc(buff_bytes);
    if (pcm_buff == NULL)
        goto record_init_err;

    record = malloc(sizeof(_record_t));
    if (record == NULL)
        goto record_init_err;

    record_thread =
            thread_create(audio_codec_record_thread, codec, "chat_audio_record", (1 * 1024), 18);
    if (record_thread == NULL)
        goto record_init_err;

    record->ar = ar;
    record->record_thread = record_thread;
    record->sample = sample;
    record->channel = channel;
    record->pcm_buff = pcm_buff;
    record->buff_bytes = buff_bytes;

    return record;
record_init_err:
    if (ar != NULL)
        AudioRecordDestroy(ar);

    if (record_thread != NULL)
        thread_stop(record_thread);

    if (pcm_buff != NULL)
        free(pcm_buff);

    if (record != NULL)
        free(record);

    return NULL;
}

static void audio_codec_record_deinit(struct audio_codec *codec)
{
    _record_t *record = codec->record;

    AudioRecordStop(record->ar);
    AudioRecordDestroy(record->ar);
    free(record->pcm_buff);
    free(record);
    codec->record = NULL;

    free(record_data);
    record_data = NULL;
    record_bytes = 0;
}

static void audio_codec_player_thread(void *data)
{
    struct audio_codec *codec = (struct audio_codec *)data;
    while (codec->player == NULL) {
        msleep(10);
    }

    _player_t *player = codec->player;
    uint8_t *pcm_buff = player->pcm_buff;
    struct ringbuff *player_rb = player->player_rb;
    int ret = 0;

    CHATBOX_INFO("player_start\n");
    while (1) {
        ret = ringbuff_read(player_rb, pcm_buff, player->buff_bytes);
        if (ret <= 0) {
            ret = player->buff_bytes;
            memset(pcm_buff, 0, player->buff_bytes);
        }

        AudioTrackWrite(player->at, pcm_buff, ret);
    }
}

static void *audio_codec_player_init(struct audio_codec *codec, int sample, int channel)
{
    _player_t *player = NULL;
    struct thread *player_thread = NULL;
    struct ringbuff *player_rb = NULL;
    tAudioTrack *at = NULL;
    uint8_t *pcm_buff = NULL;
    int ret;
    int buff_bytes = 0;

    at = AudioTrackCreateWithStream("default", AUDIO_STREAM_SYSTEM);
    if (at == NULL)
        return NULL;

    ret = AudioTrackSetup(at, sample, channel, AS_DEFAULT_BITS);
    if (ret < 0)
        goto player_init_err;

    buff_bytes = (sample * channel * 2 * 20 / 1000); // 20ms
    player_rb = ringbuff_init(buff_bytes * 100);     //2s
    if (player_rb == NULL)
        goto player_init_err;

    pcm_buff = malloc(buff_bytes);
    if (pcm_buff == NULL)
        goto player_init_err;

    player = malloc(sizeof(_player_t));
    if (player == NULL)
        goto player_init_err;

    player_thread =
            thread_create(audio_codec_player_thread, codec, "chat_audio_player", (1 * 1024), 18);
    if (player_thread == NULL)
        goto player_init_err;

    player->at = at;
    player->player_rb = player_rb;
    player->player_thread = player_thread;
    player->sample = sample;
    player->channel = channel;
    player->pcm_buff = pcm_buff;
    player->buff_bytes = buff_bytes;

    return player;

player_init_err:

    if (at != NULL)
        AudioTrackDestroy(at);

    if (player_rb != NULL)
        ringbuff_deinit(player_rb);

    if (player_thread != NULL)
        thread_stop(player_thread);

    if (pcm_buff != NULL) {
        free(pcm_buff);
    }

    if (player != NULL)
        free(player);

    return NULL;
}

static void audio_codec_player_deinit(struct audio_codec *codec)
{
    _player_t *player = codec->player;

    AudioTrackStop(player->at);
    AudioTrackDestroy(player->at);
    ringbuff_deinit(player->player_rb);
    free(player->pcm_buff);
    free(player);
    codec->player = NULL;
}

struct audio_codec *audio_codec_init(int play_sample, int play_channel, int record_sample,
                                     int record_channel,
                                     int (*rx_cb)(uint8_t *data, int size, int is_vad))
{
    struct audio_codec *codec;

    if (play_channel > AS_CHAT_SUPPORT_CHANNAL || record_channel > AS_CHAT_SUPPORT_CHANNAL) {
        CHATBOX_ERROR("max support channel is %d\n", AS_CHAT_SUPPORT_CHANNAL);
        return NULL;
    }

    codec = malloc(sizeof(struct audio_codec));
    if (codec == NULL)
        return NULL;
    memset(codec, 0, sizeof(struct audio_codec));

    codec->record = audio_codec_record_init(codec, record_sample, record_channel);
    if (codec->record == NULL) {
        CHATBOX_ERROR("audio record init error\n");
        goto exit;
    }
    codec->record_cb = rx_cb;

#ifdef AUDIO_PROCESS_SELF
    codec->audio_p = audio_process_init(record_sample, record_channel);
    if (codec->audio_p == NULL) {
        CHATBOX_ERROR("audio process init error\n");
    }
    audio_process_register_output(codec->audio_p, rx_cb);
#endif

    codec->player = audio_codec_player_init(codec, play_sample, play_channel);
    if (codec->player == NULL) {
        CHATBOX_ERROR("audio player init error\n");
        goto exit;
    }

    return codec;
exit:
    if (codec->record != NULL)
        audio_codec_record_deinit(codec);

#ifdef AUDIO_PROCESS_SELF
    if (codec->audio_p != NULL)
        audio_process_deinit(codec->audio_p);
#endif

    if (codec->player != NULL)
        audio_codec_player_deinit(codec);

    free(codec);
    return NULL;
}

void audio_codec_deinit(struct audio_codec *codec)
{
    audio_codec_record_deinit(codec);
    audio_codec_player_deinit(codec);
#ifdef AUDIO_PROCESS_SELF
    audio_process_deinit(codec->audio_p);
#endif

    free(codec);
}

void audio_codec_set_output_volume(struct audio_codec *codec, int volume)
{
    // volume range [0 ~ 100], as range [0 ~ 10]
    int as_volume = (volume / 10);

    volume = (uint32_t)(as_volume | (as_volume << 16));

    softvol_control_with_streamtype(AUDIO_STREAM_SYSTEM, &volume, 1);
}

void audio_codec_output_data(struct audio_codec *codec, uint8_t *data, int size)
{
    int ret = ringbuff_write(codec->player->player_rb, data, size);
    if (ret != size)
        CHATBOX_WARNG("player rb over--\n");

    return;
}

void audio_codec_stop_output(struct audio_codec *codec)
{
    if ((codec == NULL) || (codec->player == NULL)) {
        CHATBOX_ERROR("\n");
    }

    AudioTrackStop(codec->player->at);
}

void audio_codec_start_output(struct audio_codec *codec)
{
    if ((codec == NULL) || (codec->player == NULL)) {
        CHATBOX_ERROR("\n");
    }

    AudioTrackStart(codec->player->at);
}