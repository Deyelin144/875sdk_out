/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
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
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "console.h"
#include "inc/sonic.h"
#include "wav_parser.h"
#include "hal_time.h"
#include "ringbuffer.h"
#include "sunxi_tiny_sound_pcm.h"
#include "aw-tiny-sound-utils/common.h"

#define s_log(fmt, arg...) printf("[%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define WAV_MAX_URI_LENGTH 128
/* 48k, 2ch, 16bit, 20ms*/
#define AUDIO_MAX_FRAME_BYTES 3840
#define AUDIO_MESSAGE_CNT  4

struct AudioPlayFmt {
    unsigned Channels;
    unsigned Bits;
    unsigned SampleRate;
    unsigned period_frames;
    unsigned period_cnt;
};

typedef struct {
    hal_ringbuffer_t rb;
    sonicStream stream;
    int fd;

    struct sunxi_pcm *pcm;
    unsigned SampleRate;
    unsigned Channels;
    unsigned Bits;

    uint8_t *file_buff;
    uint8_t *sonic_buff;
    uint8_t *pcm_buff;

    void *wav_thread;
    void *player_thread;
} sonic_t;

static sonic_t *sonic_handle = NULL;
static unsigned char g_wav_path[WAV_MAX_URI_LENGTH] = { 0 };
static float s_speed = 1.0, s_pitch = 1.0;

static void sonic_test_usage()
{
    printf("Usgae: sonic [option]\n");
    printf("-h,          sonic_test help\n");
    printf("-p,          play\n");
    printf("-s,          seek speed, *100\n");
    printf("-v,          set soft volume, *100\n");
    printf("\n");
    printf("example:\n");
    printf("sonic_test -s 150\n");
    printf("sonic_test -p sdmmc/h.wav\n");
    printf("sonic_test only support wav\n");
    printf("\n");
}

static void dump_wav_header(wav_header_t *header)
{
    char *ptr = (char *)&header->riffType;
    printf("riffType:     %c%c%c%c\n", ptr[0], ptr[1], ptr[2], ptr[3]);
    ptr = (char *)&header->waveType;
    printf("waveType:     %c%c%c%c\n", ptr[0], ptr[1], ptr[2], ptr[3]);
    printf("channels:     %u\n", header->numChannels);
    printf("rate:         %u\n", header->sampleRate);
    printf("bits:         %u\n", header->bitsPerSample);
    printf("align:        %u\n", header->blockAlign);
    printf("data size:    %u\n", header->dataSize);
}

static int check_wav_header(wav_header_t *header, wav_hw_params_t *hwparams)
{
    if (!header)
        return -1;
    dump_wav_header(header);

    if (header->riffType != WAV_RIFF)
        return -1;
    if (header->waveType != WAV_WAVE)
        return -1;

    hwparams->rate = header->sampleRate;
    hwparams->channels = header->numChannels;
    /* ignore bit endian */
    switch (header->bitsPerSample) {
    case 8:
        hwparams->format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        hwparams->format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        switch (header->blockAlign / header->numChannels) {
        case 4:
            hwparams->format = SND_PCM_FORMAT_S24_LE;
            break;
        case 3:
            /*hwparams->format = SND_PCM_FORMAT_S24_3LE;*/
        default:
            s_log("unknown format..\n");
            return -1;
        }
        break;
    case 32:
        hwparams->format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        s_log("unknown sampling depth..\n");
        return -1;
    }

    return 0;
}

static void music_decode(void *arg)
{
    sonic_t *handle = (sonic_t *)arg;
    int period_byte = 0;
    int frame_bytes = 0;
    int period_frame = 0;
    int read_eos = 0;
    int ret;

    period_byte = (handle->SampleRate * 20 / 1000);//20ms,frame

    if (handle->Bits == SND_PCM_FORMAT_S16_LE) {
        frame_bytes = 2;
    } else {
        frame_bytes = 4;
    }

    period_byte = (period_byte * handle->Channels * frame_bytes);

    while (1) {
        ret = read(handle->fd, handle->file_buff, period_byte);
        if (ret > 0) {
            /* NOTE!!!
                8bit : use sonicWriteUnsignedCharToStream
                16bit : use sonicWriteShortToStream
                32bit : use sonicWriteFloatToStream
            */
            period_frame = sunxi_sound_pcm_bytes_to_frames(handle->pcm, ret);
            sonicWriteShortToStream(handle->stream, (short *)handle->file_buff, period_frame);
        } else {
            if (ret == 0) {
                sonicFlushStream(handle->stream);
                read_eos = 1;
            } else { // < 0
                goto wait_exit;
            }
        }

        do {
            ret = sonicReadShortFromStream(handle->stream, (short *)handle->sonic_buff,\
                   sunxi_sound_pcm_bytes_to_frames(handle->pcm, period_byte));
            if (ret > 0) {
                ret = sunxi_sound_pcm_frames_to_bytes(handle->pcm, ret);
                hal_ringbuffer_wait_put(handle->rb, handle->sonic_buff, ret, -1);
            }
        } while (ret > 0);

        if (read_eos == 1)
            goto wait_exit;
    }

wait_exit:
    s_log("player is going to finish, use <sonic_test -q> release\n");
    while(1) {
        hal_msleep(1000);
    }
}

static void music_play(void *arg)
{
    sonic_t *handle = (sonic_t *)arg;
    uint8_t *pcm_buff = handle->pcm_buff;
    int period_byte = 0;
    int frame_bytes = 0;
    int ret;

    period_byte = (handle->SampleRate * 20 / 1000);//20ms,frame

    if (handle->Bits == SND_PCM_FORMAT_S16_LE) {
        frame_bytes = 2;
    } else {
        frame_bytes = 4;
    }

    period_byte = (period_byte * handle->Channels * frame_bytes);

    while (1) {
        ret = hal_ringbuffer_get(handle->rb, pcm_buff, period_byte, -1);
        if (ret < 0) {
            s_log("get pcm data error!\n");
            while(1);
        }

        ret = sunxi_pcm_write(handle->pcm, pcm_buff,
            sunxi_sound_pcm_bytes_to_frames(handle->pcm, ret),
            sunxi_sound_pcm_frames_to_bytes(handle->pcm, 1));

        if (ret < 0) {
            s_log("audio play error!\n");
        }
    }
}

static int sonic_file_init(sonic_t *handle, char *path)
{
    wav_header_t wav_header;
    wav_hw_params_t wav_hwparams;
    int fd = -1;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    read(fd, &wav_header, sizeof(wav_header_t));
    if (check_wav_header(&wav_header, &wav_hwparams) != 0) {
        close(fd);
        return -1;
    }

    handle->SampleRate = wav_hwparams.rate;
    handle->Channels = wav_hwparams.channels;
    handle->Bits = wav_hwparams.format;

    return fd;
}

static int sonic_player_init(sonic_t *handle)
{
    struct sunxi_pcm *pcm;
    int period_frame = 0;
    int buff_frame = 0;
    int ret = 0;

    /* open card */
    ret = sunxi_pcm_open(&pcm, 0, 0, SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0)
        return -1;

    period_frame = (handle->SampleRate * 20 / 1000);//20ms
    buff_frame = (period_frame * 4);//80ms

    ret = sunxi_set_param(pcm, handle->Bits,
                            handle->SampleRate, handle->Channels,
                            period_frame, buff_frame);
    if (ret < 0)
        goto player_init_err;

    ret = sunxi_pcm_prepare(pcm);
    if (ret < 0)
        goto player_init_err;

    handle->pcm = pcm;

    return 0;

player_init_err:
    if (pcm != NULL)
        sunxi_pcm_close(sonic_handle->pcm);

    return -1;
}

static int sonic_buff_init(sonic_t *handle)
{
    int period_byte = 0;
    int frame_bytes = 0;
    uint8_t *file_buff = NULL;
    uint8_t *pcm_buff = NULL;
    uint8_t *sonic_buff = NULL;

    period_byte = (handle->SampleRate * 20 / 1000);//20ms,frame

    if (handle->Bits == SND_PCM_FORMAT_S16_LE) {
        frame_bytes = 2;
    } else {
        frame_bytes = 4;
    }

    period_byte = (period_byte * handle->Channels * frame_bytes);

    file_buff = malloc(period_byte);
    if (file_buff == NULL)
        return -1;

    pcm_buff = malloc(period_byte);
    if (pcm_buff == NULL) {
        free(file_buff);
        return -1;
    }

    sonic_buff = malloc(period_byte);
    if (sonic_buff == NULL) {
        free(file_buff);
        free(pcm_buff);
        return -1;
    }

    handle->file_buff = file_buff;
    handle->pcm_buff = pcm_buff;
    handle->sonic_buff = sonic_buff;

    return 0;
}

static int sonic_sonic_init(sonic_t *handle)
{
    sonicStream stream;
    stream = sonicCreateStream(handle->SampleRate, handle->Channels, AUDIO_MAX_FRAME_BYTES);
    if (stream == NULL)
        return -1;

    sonicSetSpeed(stream, s_speed);
    sonicSetPitch(stream, s_pitch);

    handle->stream = stream;

    return 0;
}

static int sonic_thread_init(sonic_t *handle)
{
    void *player_thread_handle = NULL;
    void *wav_thread_handle = NULL;

    wav_thread_handle = hal_thread_create(music_decode, handle, "wav_demo", 1024,
                                            HAL_THREAD_PRIORITY_APP);
    if (wav_thread_handle == NULL) {
        s_log("wav thread create fail!");
        return -1;
    }

    player_thread_handle = hal_thread_create(music_play, handle, "music_play", 1024,
                                            (HAL_THREAD_PRIORITY_APP + 1));
    if (player_thread_handle == NULL) {
        s_log("sonic_thread create fail!");
        hal_thread_stop(wav_thread_handle);
        wav_thread_handle = NULL;
        return -1;
    }

    handle->wav_thread = wav_thread_handle;
    handle->player_thread = player_thread_handle;

    return 0;
}

static int sonic_example_init(char *path)
{
    sonic_handle = malloc(sizeof(sonic_t));
    if (sonic_handle == NULL)
        return -1;
    memset(sonic_handle, 0, sizeof(sonic_t));

    sonic_handle->rb = hal_ringbuffer_init(AUDIO_MAX_FRAME_BYTES * AUDIO_MESSAGE_CNT);
    if (sonic_handle->rb == NULL)
        goto sonic_init_err;

    sonic_handle->fd = sonic_file_init(sonic_handle, path);
    if (sonic_handle->fd < 0)
        goto sonic_init_err;

    if (sonic_player_init(sonic_handle) < 0)
        goto sonic_init_err;

    if (sonic_buff_init(sonic_handle) < 0)
        goto sonic_init_err;

    if (sonic_sonic_init(sonic_handle) < 0)
        goto sonic_init_err;

    if (sonic_thread_init(sonic_handle) < 0)
        goto sonic_init_err;

    return 0;
sonic_init_err:
    if (sonic_handle != NULL) {
        if (sonic_handle->rb != NULL)
            hal_ringbuffer_release(sonic_handle->rb);

        if (sonic_handle->fd > 0)
            close(sonic_handle->fd);

        if (sonic_handle->pcm != NULL)
            sunxi_pcm_close(sonic_handle->pcm);

        if (sonic_handle->file_buff != NULL)
            free(sonic_handle->file_buff);

        if (sonic_handle->pcm_buff != NULL)
            free(sonic_handle->pcm_buff);

        if (sonic_handle->sonic_buff != NULL)
            free(sonic_handle->sonic_buff);

        if (sonic_handle->stream != NULL)
            sonicDestroyStream(sonic_handle->stream);

        free(sonic_handle);
        sonic_handle = NULL;
    }

    return -1;
}

static int sonic_example_deinit(void)
{
    if (sonic_handle == NULL) {
        s_log("not init \n");
        return -1;
    } else {
        hal_thread_stop(sonic_handle->wav_thread);
        hal_thread_stop(sonic_handle->player_thread);

        hal_ringbuffer_release(sonic_handle->rb);
        close(sonic_handle->fd);
        sunxi_pcm_close(sonic_handle->pcm);

        free(sonic_handle->file_buff);
        free(sonic_handle->pcm_buff);
        free(sonic_handle->sonic_buff);

        sonicDestroyStream(sonic_handle->stream);

        free(sonic_handle);
        sonic_handle = NULL;
    }

    return 0;
}

static int sonic_example_change_speed(float speed)
{
    if ((sonic_handle == NULL) || (sonic_handle->stream == NULL))
        return -1;

    sonicSetSpeed(sonic_handle->stream, speed);
    s_log("set speed %f\n", s_speed);
    return 0;
}

static int sonic_example_change_pitch(float pitch)
{
    if ((sonic_handle == NULL) || (sonic_handle->stream == NULL))
        return -1;

    sonicSetPitch(sonic_handle->stream, pitch);
    s_log("set pitch %f\n", pitch);
    return 0;
}

int sonic_test(int argc, char *argv[])
{
    int c = 0;
    optind = 0;

    while ((c = getopt(argc, argv, "qhp:s:t:")) != -1) {
        switch (c) {
        case 'p':
            if (sonic_example_init(optarg) < 0) {
                s_log("sonic init example error\n");
                return -1;
            }

            s_log(">>>>>>>>>>>>>>speed : %f, pitch : %f\n", s_speed, s_pitch);
            break;
        case 'q':
            sonic_example_deinit();
            s_log("play finish\n");
            break;
        case 's':
            s_speed = atoi(optarg);
            s_speed /= 100;
            sonic_example_change_speed(s_speed);
            break;
        case 't':
            s_pitch = atoi(optarg);
            s_pitch /= 100;
            sonic_example_change_pitch(s_pitch);
            break;
        case 'h':
            sonic_test_usage();
            break;
        default:
            return -1;
        }
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(sonic_test, sonic_test, sonic test demo);
