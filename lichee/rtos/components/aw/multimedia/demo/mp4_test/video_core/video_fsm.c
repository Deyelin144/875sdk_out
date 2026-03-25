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

#include <stdlib.h>
#include "rtplayer.h"
#include "cedarx.h"
#include "cdx_interface/cdx_customer_stream.h"
#include "cdx_interface/cdx_audio_decoder.h"
#include "cdx_interface/cdx_video_dec.h"
#include "cdx_interface/cdx_video_rendor.h"
#include "cdx_interface/cdx_audio_rendor.h"
#include "video_core/video_message.h"
#include "hal_thread.h"
#include "video_fsm.h"
#include "ringbuffer.h"
#include "sunxi_hal_timer.h"
#include "hal_sem.h"
#include "hal_time.h"
#include "hal_timer.h"
#include "video_stack.h"
#include "aw-alsa-lib/common.h"
#include "video_debug/vd_log.h"
#include "CdxParserCustomer.h"

typedef int VideoStatus;
typedef VideoStatus(*VideoEventContext)(void *);

typedef enum _Mp4StatusContext {
    VIDEO_STATE_INIT = 0,
    VIDEO_STATE_CONTINUE,
    VIDEO_STATE_PAUSE,
    VIDEO_STATE_MAX,
} Mp4StatusContext;

typedef struct customer_parser_context {
    CdxDataSourceT source;
    pthread_mutex_t mutex;
    CdxParserT *parser;
    CdxStreamT *stream;
    VideoInfo *mpi;
    char uri[32];
} CP_context;
static CP_context *g_cpt = NULL;

typedef struct _media_ringbuff {
    unsigned command;
    unsigned single_size;
    unsigned priv;
    unsigned reserce[13];
    char buff[0];
} media_ringbuff;

// raw data to YUV
#define VIDEO_STACK_SIZE 40
static MemHandle video_stack = {0};
// YUV to RGB
#define DECODE_STACK_SIZE 2
static MemHandle decode_stack = {0};
// RAW data to PCM
#define AUDIO_STACK_SIZE 30
static MemHandle audio_stack = {0};
// PCM to play
#define PCM_STACK_SIZE 2
#define PCM_BUFF_SIZE 8192
static MemHandle pcm_stack = {0};

struct avtimer {
    hal_thread_t video_dec_handle;
    hal_thread_t video_rendor_handle;
    hal_thread_t audio_dec_handle;
    hal_thread_t audio_rendor_handle;
    hal_sem_t avsync_sem;
    hal_mutex_t mux;

    unsigned audio_per_size;
#define AUDIO_ATOM_TIME 5 //ms
    unsigned audio_atom_size;
    unsigned fact_audio_pcm_sum;

    unsigned audio_1s_size;

    unsigned frame_size;

    unsigned insert_frame;
    unsigned force_stop;

    unsigned read_point;
    unsigned video_point;

#define VIDEO_STATUS_NORMAL 0
#define VIDEO_STATUS_PAUSE  1
#define VIDEO_STATUS_STOP   2
    unsigned spec_status;

    long int audio_play_pcm_size;
    bool skip_one_frame;
};
struct avtimer *g_avtimer = NULL;

static int s_player_completed = 0;

extern unsigned char *BuildAACPacketHdr(unsigned char *extraData, int extraDataLen, int uPacketLen,
                                        int *pHdrLen, int channels, int sampleRate);
extern int UpdateAACPacketHdr(unsigned char *pBuf, int uHdrLen, int uPacketLen);

void AvccToAnnexB(unsigned char *dst, unsigned size);

int VideoCommandTell(unsigned *msc)
{
    if ((g_avtimer == NULL) || (g_cpt == NULL)) {
        return -1;
    }

    VideoInfo *mpi = g_cpt->mpi;
    hal_mutex_lock(g_avtimer->mux);
    *msc = (g_avtimer->video_point * (1000 / mpi->Timescale));
    hal_mutex_unlock(g_avtimer->mux);
    return 0;
}

int VideoCommandSize(unsigned *msc)
{
    if (g_cpt == NULL) {
        return -1;
    }

    VideoInfo *mpi = g_cpt->mpi;
    *msc = mpi->Vduration;
    return 0;
}


/*************************************************cb********************************************************/
static video_callback g_cb = NULL;
static void *g_handle = NULL;
static void funtion_cb(video_cb_status status)
{
    if (g_cb != NULL) {
        g_cb(g_handle, status);
    }
}

int video_cb_register(video_callback cb, void *handle)
{
    if (cb == NULL) {
        vlog_error("video cb is null!");
        return -1;
    }
    g_cb = cb;
    g_handle = handle;
    return 0;
}

void video_cb_unregister(void)
{
    g_cb = NULL;
}
/*************************************************rb buff********************************************************/
static void video_ringbuff_free(void)
{
    if (video_stack.addr != NULL) {
        mrb_close(&video_stack);
        video_stack.addr = NULL;
    }

    if (audio_stack.addr != NULL) {
        mrb_close(&audio_stack);
        audio_stack.addr = NULL;
    }

    if (decode_stack.addr != NULL) {
        mrb_close(&decode_stack);
        decode_stack.addr = NULL;
    }

    if (pcm_stack.addr != NULL) {
        mrb_close(&pcm_stack);
        pcm_stack.addr = NULL;
    }
}

static int video_ringbuff_malloc(VideoInfo *mpi)
{
    unsigned video_rb_size = 0;
    unsigned audio_rb_size = 0;
    unsigned vdec_rb_size = 0;
    unsigned pcm_rb_size = 0;
    if (mpi == NULL) {
        return -1;
    }

    // may be only audio
    if (mpi->max_video_size != 0) {
        video_rb_size = (sizeof(media_ringbuff) + mpi->max_video_size + mpi->DinfoSize);
        video_rb_size += (video_rb_size % CACHELINE_LEN) ? \
                            (CACHELINE_LEN - video_rb_size % CACHELINE_LEN) : 0;
        if (mrb_open(&video_stack, video_rb_size, VIDEO_STACK_SIZE) < 0) {
            goto video_ringbuff_malloc_error;
        }

        vdec_rb_size = mpi->Width * mpi->Height * MP4_DEC_COLOR_DEPTH / 8;
        vdec_rb_size += (vdec_rb_size % CACHELINE_LEN) ? \
                            (CACHELINE_LEN - vdec_rb_size % CACHELINE_LEN) : 0;
        if (mrb_open(&decode_stack, vdec_rb_size, DECODE_STACK_SIZE) < 0) {
            goto video_ringbuff_malloc_error;
        }
    }

    // may be only video
    if (mpi->max_audio_size != 0) {
        audio_rb_size = (sizeof(media_ringbuff) + mpi->max_audio_size + mpi->AinfoSize);
        // audio_rb_size += (audio_rb_size % CACHELINE_LEN) ? \
        //                     (CACHELINE_LEN - audio_rb_size % CACHELINE_LEN) : 0;
        if (mrb_open(&audio_stack, audio_rb_size, AUDIO_STACK_SIZE) < 0) {
            goto video_ringbuff_malloc_error;
        }

        pcm_rb_size = (PCM_BUFF_SIZE + sizeof(media_ringbuff));
        if (mrb_open(&pcm_stack, pcm_rb_size, PCM_STACK_SIZE) < 0) {
            goto video_ringbuff_malloc_error;
        }
    }

    return 0;
video_ringbuff_malloc_error:
    video_ringbuff_free();
    return -1;
}
/*********************************video init**************************************/
extern void start_malloc_trace(void);
extern void finish_malloc_trace_and_show(void);
// extern void show_heap_info(void);
static int VideoDisplayInit(VideoInfo *mpi)
{
    if (mpi == NULL)
        return -1;

    if (VideoDecInit(mpi->Width, mpi->Height) < 0) {
        vlog_error("video decode init fail.");
        return -1;
    }

    if (VideoPlayInit(mpi->Width, mpi->Height) < 0) {
        vlog_error("video play init fail.");
        VideoDecDeinit();
        return -1;
    }

    return 0;
}

static int VideoDisplayDeinit(void)
{
    VideoDecDeinit();
    VideoPlayDeinit();
}

/***********************************audio Init***********************************/
static int VideoAudioInit(VideoInfo *mpi)
{
    if (mpi == NULL)
        return -1;

    struct AudioPlayFmt s_aud_fmt = {0};
    s_aud_fmt.Channels = mpi->Channels;
    s_aud_fmt.Bits = mpi->Bits;
    s_aud_fmt.SampleRate = mpi->SampleRate;
    // 20ms
    s_aud_fmt.period_frames = (mpi->SampleRate * 20 / 1000);
    // 80ms
    s_aud_fmt.period_cnt = 4;

    if (VideoAudioPlayInit(&s_aud_fmt) < 0)
        return -1;

    if (VideoAudioDecInit(mpi->AudioType) < 0) {
        VideoAudioPlayDeinit();
        return -1;
    }

    return 0;
}

static void VideoAudioDeinit(void)
{
    VideoAudioPlayDeinit();
    VideoAudioDecDeinit();
}

/**************************************AVtimer**********************************/

static struct avtimer *Mp4AVtimerInit(VideoInfo *mpi)
{
    if (mpi == NULL) {
        return NULL;
    }

    struct avtimer *av_temp = malloc(sizeof(struct avtimer));
    if (av_temp == NULL) {
        goto Mp4AVtimerInit_Global_Error;
    }
    memset(av_temp, 0, sizeof(struct avtimer));

    av_temp->avsync_sem = hal_sem_create(1);
    if (av_temp->avsync_sem == NULL) {
        goto Mp4AVtimerInit_Sem_Error;
    }

    av_temp->mux = hal_mutex_create();
    if (av_temp->mux == NULL) {
        goto Mp4AVtimerInit_Mux_Error;
    }

    av_temp->audio_1s_size = (mpi->SampleRate * mpi->Bits * mpi->Channels / 8);
    av_temp->frame_size = (mpi->Bits * mpi->Channels / 8);
    av_temp->audio_per_size = (av_temp->audio_1s_size / mpi->Timescale);
    av_temp->audio_atom_size = (av_temp->audio_1s_size * AUDIO_ATOM_TIME / 1000);
    av_temp->audio_play_pcm_size = 0;
    av_temp->skip_one_frame = false;

    return av_temp;
Mp4AVtimerInit_Mux_Error:
    hal_sem_delete(av_temp->avsync_sem);
Mp4AVtimerInit_Sem_Error:
    free(av_temp);
Mp4AVtimerInit_Global_Error:
    return NULL;
}

static void Mp4AVtimerDeinit(struct avtimer *avt)
{
    hal_mutex_delete(avt->mux);
    hal_sem_delete(avt->avsync_sem);
    free(avt);
}

/********************************************task*********************************************/
static void video_decode(void *arg)
{
    struct avtimer *avt = arg;

    media_ringbuff *s_video_rb = NULL;
    media_ringbuff *s_dec_data = NULL;

    while (1) {
        s_video_rb = mrb_read_data(&video_stack);
        if (s_video_rb != NULL) {
            s_dec_data = mrb_write_data(&decode_stack);
            if (s_dec_data != NULL) {
                switch(s_video_rb->command) {
                case VIDEO_COMMAND_CONTINUE:
                case VIDEO_COMMAND_PAUSE:
                case VIDEO_COMMAND_SEEK:
                    if (avt->force_stop == 1) {
                        s_dec_data->command = s_video_rb->command;
                        mrb_write_finish(&decode_stack);
                        break;
                    }

                    if (s_video_rb->single_size != 0) {
                        if (Mp4VideoDecFrame(s_video_rb->buff, s_video_rb->single_size, s_dec_data->buff) < 0) {
                            s_dec_data->single_size = 0;
                        } else {
                            s_dec_data->single_size = 1;
                        }
                    } else {
                        s_dec_data->single_size = 0;
                    }
                    s_dec_data->command = s_video_rb->command;
                    mrb_write_finish(&decode_stack);
                    break;
                case VIDEO_COMMAND_STOP:
                    s_dec_data->command = s_video_rb->command;
                    hal_mutex_lock(avt->mux);
                    avt->video_dec_handle = NULL;
                    hal_mutex_unlock(avt->mux);
                    mrb_write_finish(&decode_stack);
                    mrb_read_finish(&video_stack);
                    hal_thread_stop(NULL);
                    break;
                default:
                    s_dec_data->command = s_video_rb->command;
                    mrb_write_finish(&decode_stack);
                    break;
                }
            } else {
                vlog_error("get decode_stack fail!\n");
            }
            mrb_read_finish(&video_stack);
            if (s_video_rb->command == VIDEO_COMMAND_PAUSE) {
                mrb_pause(&video_stack);
            }
        }
    }
}

static void video_rendor(void *arg)
{
    struct avtimer *avt = arg;
    media_ringbuff *s_dec_data = NULL;
    bool need_skip = false;

    while(1) {
        s_dec_data = mrb_read_data(&decode_stack);
        if (s_dec_data != NULL) {
            if (s_dec_data->single_size == 0) {
                mrb_read_finish(&decode_stack);
                continue;
            }
            switch(s_dec_data->command) {
            case VIDEO_COMMAND_CONTINUE:
            // case VIDEO_COMMAND_PAUSE:
            case VIDEO_COMMAND_SEEK:
                if (avt->force_stop == 1) {
                    mrb_read_finish(&decode_stack);
                    break;
                }
MP4_RENDOR_REWAIT:
                if (hal_sem_timedwait(avt->avsync_sem, 1000) < 0) {
                    hal_mutex_lock(avt->mux);
                    if (avt->spec_status == VIDEO_STATUS_PAUSE) {
                        hal_mutex_unlock(avt->mux);
                        vlog_debug("pause\n");
                        goto MP4_RENDOR_REWAIT;
                    }
                    hal_mutex_unlock(avt->mux);
                    vlog_error("video rendor timeout or finsih\n");
                } else {
                    hal_mutex_lock(avt->mux);
                    if (avt->force_stop == 1) {
                        hal_mutex_unlock(avt->mux);
                        mrb_read_finish(&decode_stack);
                        break;
                    }

                    if (avt->insert_frame > 0) {
                        avt->insert_frame --;
                        hal_mutex_unlock(avt->mux);
                        goto MP4_RENDOR_REWAIT;
                    }

                    avt->video_point ++;
                    need_skip = avt->skip_one_frame;
                    hal_mutex_unlock(avt->mux);
                    if (!need_skip) {
                        Mp4VideoPlay(s_dec_data->buff);
                    } else {
                        hal_mutex_lock(avt->mux);
                        avt->skip_one_frame = false;
                        hal_mutex_unlock(avt->mux);
                    }
                }
                mrb_read_finish(&decode_stack);
                // printf("cur %d total %d\n", (g_avtimer->video_point * (1000 / g_cpt->mpi->Timescale)), g_cpt->mpi->Vduration);
                break;
            case VIDEO_COMMAND_STOP:
                mrb_read_finish(&decode_stack);
                hal_mutex_lock(avt->mux);
                avt->video_rendor_handle = NULL;
                hal_mutex_unlock(avt->mux);
                hal_thread_stop(NULL);
                break;
            default:
                mrb_read_finish(&decode_stack);
                break;
            }

            // if (s_dec_data->command == VIDEO_COMMAND_PAUSE) {
            //     mrb_pause(&decode_stack);
            // }
        }
    }
}

static void audio_decode(void *arg)
{
    struct avtimer *avt = arg;
    if (avt == NULL) {
        return;
    }

    media_ringbuff *audio_dec_rb = NULL;
    media_ringbuff *audio_pcm_rb = NULL;

    while (1) {
        audio_dec_rb = mrb_read_data(&audio_stack);
        if (audio_dec_rb != NULL) {
            vlog_debug("a_decode : %d\n",audio_dec_rb->command);
            audio_pcm_rb = mrb_write_data(&pcm_stack);
            if (audio_pcm_rb != NULL) {
                audio_pcm_rb->command = audio_dec_rb->command;
                switch (audio_dec_rb->command) {
                case VIDEO_COMMAND_CONTINUE:
                case VIDEO_COMMAND_PAUSE:
                case VIDEO_COMMAND_SEEK:
                    if (audio_dec_rb->single_size != 0) {
                        CustomerDecoderBuffFill(audio_dec_rb->buff, audio_dec_rb->single_size);
                        Mp4AudioDecFrame(audio_pcm_rb->buff, &audio_pcm_rb->single_size);
                        if (audio_pcm_rb->single_size > PCM_BUFF_SIZE) {
                            vlog_error("PCM_BUFF_SIZE is not large enough!!!,maybe crash!!!\n");
                        }
                        audio_pcm_rb->priv = audio_dec_rb->priv;
                    } else {
                        audio_pcm_rb->single_size = 0;
                    }
                    mrb_write_finish(&pcm_stack);
                    break;
                case VIDEO_COMMAND_STOP:
                    mrb_write_finish(&pcm_stack);
                    hal_mutex_lock(avt->mux);
                    avt->audio_dec_handle = NULL;
                    hal_mutex_unlock(avt->mux);
                    hal_thread_stop(NULL);
                    break;
                // case VIDEO_COMMAND_PAUSE:
                //     mrb_write_finish(&pcm_stack);
                //     break;
                default:
                    break;
                }
            } else {
                vlog_error("get audio_pcm_rb fail!\n");
            }
            mrb_read_finish(&audio_stack);

            if (audio_dec_rb->command == VIDEO_COMMAND_PAUSE) {
                mrb_pause(&audio_stack);
            }
        }
    }
}

static void audio_rendor(void *arg)
{
    struct avtimer *avt = arg;
    if (avt == NULL) {
        return;
    }

    long int ideal_size = 0;
    unsigned duration_size = 0;
    unsigned play_offset = 0;
    unsigned play_size = 0;
    media_ringbuff *s_pcm_rb = NULL;

    while (1) {
        s_pcm_rb = mrb_read_data(&pcm_stack);
        if (s_pcm_rb != NULL) {
            vlog_debug("a_rendor : %d\n",s_pcm_rb->command);
            switch(s_pcm_rb->command) {
            case VIDEO_COMMAND_SEEK:
                avt->fact_audio_pcm_sum = 0;
            case VIDEO_COMMAND_CONTINUE:
                duration_size = s_pcm_rb->single_size;
                play_size = avt->audio_atom_size;
                play_offset = 0;

                do {
                    if (duration_size < play_size) {
                        play_size = duration_size;
                    }

                    Mp4AudioPlay((s_pcm_rb->buff + play_offset), play_size);

                    duration_size -= play_size;
                    play_offset += play_size;
                    avt->fact_audio_pcm_sum += play_size;
                    if (avt->fact_audio_pcm_sum >= avt->audio_per_size) {
                        avt->fact_audio_pcm_sum -= avt->audio_per_size;
                        hal_sem_post(avt->avsync_sem);
                    }
                } while(duration_size > 0);

                avt->audio_play_pcm_size += s_pcm_rb->single_size;
                ideal_size = avt->video_point * avt->audio_per_size;
                if (avt->audio_play_pcm_size < ideal_size) {
                    for (; ideal_size - avt->audio_play_pcm_size > avt->audio_per_size; ideal_size -= avt->audio_per_size) {
                        hal_mutex_lock(avt->mux);
                        avt->insert_frame++;
                        hal_mutex_unlock(avt->mux);
                        if (ideal_size < avt->audio_play_pcm_size) {
                            break;
                        }
                    }
                } else {
                    for (; avt->audio_play_pcm_size - ideal_size > avt->audio_per_size; ideal_size += avt->audio_per_size) {
                        hal_mutex_lock(avt->mux);
                        avt->skip_one_frame = true;
                        hal_sem_post(avt->avsync_sem);
                        hal_mutex_unlock(avt->mux);
                        if (ideal_size > avt->audio_play_pcm_size) {
                            break;
                        }
                    }
                }
                break;
            case VIDEO_COMMAND_STOP:
                avt->force_stop = 1;
                // release sem make sure close video rendor
                hal_sem_post(avt->avsync_sem);
                Mp4AudioStop();
                hal_mutex_lock(avt->mux);
                avt->spec_status = VIDEO_STATUS_NORMAL;
                avt->audio_rendor_handle = NULL;
                hal_mutex_unlock(avt->mux);
                hal_thread_stop(NULL);
                break;
            case VIDEO_COMMAND_PAUSE:
                hal_mutex_lock(avt->mux);
                if (avt->spec_status != VIDEO_STATUS_PAUSE) {
                    avt->spec_status = VIDEO_STATUS_PAUSE;
                    funtion_cb(MP4_STATUS_PLAY_TO_PAUSE);
                } else {
                    avt->spec_status = VIDEO_STATUS_NORMAL;
                }
                hal_mutex_unlock(avt->mux);
                Mp4AudioPause();
                break;
            }
            mrb_read_finish(&pcm_stack);

            // if (s_pcm_rb->command == VIDEO_COMMAND_PAUSE) {
            //     mrb_pause(&pcm_stack);
            // }
        }
    }
}

static int Mp4TaskCreat(struct avtimer *avt)
{
    if (avt == NULL)
        return -1;

    avt->video_dec_handle = hal_thread_create(video_decode,
                                            avt,
                                            "video_dec",
                                            4096,
                                            20);
    if (avt->video_dec_handle == NULL)
        goto Mp4TaskCreat_Video_dec_error;

    avt->video_rendor_handle = hal_thread_create(video_rendor,
                                                avt,
                                                "video_rendor",
                                                1024,
                                                20);
    if (avt->video_rendor_handle == NULL)
        goto Mp4TaskCreat_Video_Rendor_error;

    avt->audio_dec_handle = hal_thread_create(audio_decode,
                                        avt,
                                        "audio_dec",
                                        1024,
                                        20);
    if (avt->audio_dec_handle == NULL)
        goto Mp4TaskCreat_Audio_dec_error;

    avt->audio_rendor_handle = hal_thread_create(audio_rendor,
                                        avt,
                                        "audio_rendor",
                                        1024,
                                        20);
    if (avt->audio_rendor_handle == NULL)
        goto Mp4TaskCreat_Audio_rendor_error;

    return 0;
Mp4TaskCreat_Audio_rendor_error:
    hal_thread_stop(avt->audio_dec_handle);
Mp4TaskCreat_Audio_dec_error:
    hal_thread_stop(avt->video_rendor_handle);
Mp4TaskCreat_Video_Rendor_error:
    hal_thread_stop(avt->video_dec_handle);
Mp4TaskCreat_Video_dec_error:
    return -1;
}

static void Mp4TaskDestory(struct avtimer *avt)
{
    int stop_cnt = 0;
    stop_cnt = (VIDEO_STACK_SIZE * 10);
    do {
        hal_msleep(10);
        stop_cnt --;
    } while((stop_cnt > 0) && ((avt->audio_dec_handle != NULL)
                || (avt->video_rendor_handle != NULL)
                || (avt->video_dec_handle != NULL)
                || (avt->audio_rendor_handle != NULL)));
    hal_mutex_lock(avt->mux);
    if (avt->audio_dec_handle != NULL) {
        vlog_error("audio_dec_handle is hard close!\n");
        hal_thread_stop(avt->audio_dec_handle);
        avt->audio_dec_handle = NULL;
    }

    if (avt->video_rendor_handle != NULL) {
        vlog_error("video_rendor_handle is hard close!\n");
        hal_thread_stop(avt->video_rendor_handle);
        avt->video_rendor_handle = NULL;
    }

    if (avt->video_dec_handle != NULL) {
        vlog_error("video_dec_handle is hard close!\n");
        hal_thread_stop(avt->video_dec_handle);
        avt->video_dec_handle = NULL;
    }

    if (avt->audio_rendor_handle != NULL) {
        vlog_error("audio_rendor_handle is hard close!\n");
        hal_thread_stop(avt->audio_rendor_handle);
        avt->audio_rendor_handle = NULL;
    }
    hal_mutex_unlock(avt->mux);
}

/********************************************FSM*****************************/
static int mp4_seek_to_time(VideoInfo *mpi, unsigned msc)
{
    unsigned i = 0;
    unsigned seek_frame = 0;
    unsigned seek_point = 0;
    long long seek_offset = 0;
    // unsigned *current_position = &(g_cpt->tmpCsrParser->mpi->Position);

    if (g_avtimer == NULL) {
        vlog_error("NULL g_avtimer!\n");
        return -1;
    }

    hal_mutex_lock(g_avtimer->mux);
    seek_frame = (msc * mpi->Timescale / 1000);
    for (seek_point = 0; seek_point < mpi->sample_count; seek_point++) {
        if (mpi->sample_map[seek_point].type == SAMPLE_IS_VIDEO) {
            if (i == seek_frame) {
                g_avtimer->video_point = seek_frame;
                /* find I frame */
                do {
                    if (mpi->sample_map[seek_point].priv != VIDEO_IS_IFRAME) {
                        seek_point --;
                        seek_offset -= mpi->sample_map[seek_point].size;
                        if (mpi->sample_map[seek_point].type == SAMPLE_IS_VIDEO) {
                            g_avtimer->video_point --;
                        }
                    } else {
                        break;
                    }
                } while(seek_point > 0);
                break;
            } else {
                i ++;
            }
        }
        seek_offset += mpi->sample_map[seek_point].size;
    }

    if (seek_point == mpi->sample_count) {
        vlog_error("seek error");
        hal_mutex_unlock(g_avtimer->mux);
        return -1;
    }
    g_avtimer->fact_audio_pcm_sum = 0;
    g_avtimer->read_point = seek_point;
    seek_offset += mpi->start_offset;
    g_avtimer->audio_play_pcm_size = g_avtimer->video_point * g_avtimer->audio_1s_size / mpi->Timescale;
    hal_mutex_unlock(g_avtimer->mux);
    CdxStreamSeek(g_cpt->stream, seek_offset, SEEK_SET);
    return 0;
}

static int VideoInfoFree(VideoInfo *mpi)
{
    if (mpi == NULL)
        return -1;

    if (mpi->sample_map != NULL)
        free(mpi->sample_map);

    if (mpi->Ainfo != NULL)
        free(mpi->Ainfo);

    if (mpi->Dinfo != NULL)
        free(mpi->Dinfo);

    free(mpi);
    return 0;
}

static int mp4_force_stop(void)
{
    Mp4TaskDestory(g_avtimer);
    Mp4AVtimerDeinit(g_avtimer);
    g_avtimer = NULL;

    VideoAudioDeinit();
    VideoDisplayDeinit();

    video_ringbuff_free();

    // CdxParserClose(g_cpt->parser);
    CdxStreamClose(g_cpt->stream);
    VideoInfoFree(g_cpt->mpi);
    free(g_cpt);
    g_cpt = NULL;
}

static inline int video_global_create(void)
{
    g_cpt = malloc(sizeof(CP_context));
    if (g_cpt == NULL) {
        vlog_error("CP context malloc fail.");
        return -1;
    }

    memset(g_cpt, 0, sizeof(CP_context));
    return 0;
}

static inline int video_global_init(char *url)
{
    // printf("---%s\n", url);
    CdxStreamT *crstream = CustomerStreamCreate();
    if (crstream == NULL) {
        vlog_error("create customer stream fail.");
        return -1;
    }

    CustomerStreamSetUrl(crstream, url);
    sprintf(g_cpt->uri, "customer://%p", crstream);
    g_cpt->source.uri = g_cpt->uri;

    g_cpt->mutex = FREERTOS_POSIX_MUTEX_INITIALIZER;

    if (CdxParserPrepare(&g_cpt->source, 0, &g_cpt->mutex, NULL,
                         &g_cpt->parser, &g_cpt->stream, NULL, NULL) < 0) {
        vlog_error("get parser fail\n");
        CdxStreamClose(crstream);
        return -1;
    }

    VideoInfo *mpi = malloc(sizeof(VideoInfo));
    if (mpi == NULL) {
        vlog_error("mpi malloc fail\n");
        CdxParserClose(g_cpt->parser);
        return -1;
    }
    memset(mpi, 0, sizeof(VideoInfo));
    if (CdxParserControl(g_cpt->parser, CDX_PSR_CMD_GET_CUSTIMER, (void *)mpi) < 0) {
        vlog_error("create map fail\n");
        CdxParserClose(g_cpt->parser);
        return -1;
    }
    CdxParserControl(g_cpt->parser, CDX_PSR_CMD_CLOSE_BUT_STREAM, NULL);
    CdxParserClose(g_cpt->parser);
    g_cpt->mpi = mpi;
    return 0;
}

static inline void video_message_dump(void)
{
    VideoInfo *Vinfo = g_cpt->mpi;
    if (Vinfo == NULL)
        return;

    vlog_alway("video type : %d\n", Vinfo->VideoType);
    vlog_alway("video Height : %d\n", Vinfo->Height);
    vlog_alway("video width : %d\n", Vinfo->Width);
    vlog_alway("video scale : %d\n", Vinfo->Timescale);
    if (Vinfo->Channels != 0) {
        if (Vinfo->AudioType == AUDIO_TYPE_MP3) {
            vlog_alway("audio type may be mp3\n");
        } else {
            vlog_alway("audio type may be aac\n");
        }
        vlog_alway("audio channel : %d\n", Vinfo->Channels);
        vlog_alway("audio sample : %d\n", Vinfo->SampleRate);
        vlog_alway("audio bits : %d\n", Vinfo->Bits);
    } else {
        vlog_alway("no audio message.\n");
    }

    if ((Vinfo->AudioType == AUDIO_TYPE_AAC) && (Vinfo->Ainfo != NULL)) {
        unsigned headlen = 0;
        unsigned char *AACHEADER = BuildAACPacketHdr(Vinfo->Ainfo,
                                                    Vinfo->AinfoSize,
                                                    0,
                                                    &headlen,
                                                    Vinfo->Channels,
                                                    Vinfo->SampleRate);
        if (AACHEADER == NULL) {
            vlog_error("build AAC packet header fail.");
            return;
        }

        free(Vinfo->Ainfo);
        Vinfo->Ainfo = AACHEADER;
        Vinfo->AinfoSize = headlen;
    }
#if 0
    printf("\tnum\t\tsize\ttype\tpriv\n");
    for (int i = 0; i < Vinfo->sample_count; i++) {
        printf("\t%d\t\t%d\t%d\t%d\n", i, Vinfo->sample_map[i].size, Vinfo->sample_map[i].type, Vinfo->sample_map[i].priv);
    }
#endif
}

static VideoStatus VideoInit(void *msg)
{
    if (msg == NULL)
       return VIDEO_STATE_INIT;

    video_message *dmsg = msg;
    if (dmsg->command != VIDEO_COMMAND_PREPARE) {
        vlog_error("invalid cmd: %d.", dmsg->command);
        funtion_cb(MP4_STATUS_STOP);
        return VIDEO_STATE_INIT;
    }

    if (video_global_create() < 0) {
        funtion_cb(MP4_STATUS_ERROR);
        return VIDEO_STATE_INIT;
    }

    if (video_global_init((char *)(dmsg->pri)) < 0) {
        free(g_cpt);
        funtion_cb(MP4_STATUS_ERROR);
        return VIDEO_STATE_INIT;
    }

    video_message_dump();

    VideoInfo *Vinfo = g_cpt->mpi;

    if (video_ringbuff_malloc(Vinfo) < 0) {
        vlog_error("malloc ringbuff fail!");
        goto Mp4Init_Data_error;
    }

    if (VideoDisplayInit(Vinfo) < 0) {
        vlog_error("display init fail.");
        goto Mp4Init_Display_error;
    }
    if (VideoAudioInit(Vinfo) < 0) {
        vlog_error("audio init fail.");
        goto Mp4Init_Audio_error;
    }
    struct avtimer *s_avt = Mp4AVtimerInit(Vinfo);
    if (s_avt == NULL) {
        goto Mp4Init_AVT_error;
    }
    if (Mp4TaskCreat(s_avt) < 0) {
        vlog_error("create mp4 task fail");
        goto Mp4Init_TCreat_error;
    }
    g_avtimer = s_avt;

    CdxStreamSeek(g_cpt->stream, Vinfo->start_offset, SEEK_SET);

    funtion_cb(MP4_STATUS_INIT_FINISH);
    return VIDEO_STATE_CONTINUE;

Mp4Init_TCreat_error:
    Mp4AVtimerDeinit(s_avt);
Mp4Init_AVT_error:
    VideoAudioDeinit();
Mp4Init_Audio_error:
    VideoDisplayDeinit();
Mp4Init_Display_error:
    video_ringbuff_free();
Mp4Init_Data_error:
    // CdxParserClose(g_cpt->parser);
    CdxStreamClose(g_cpt->stream);
    free(g_cpt);
    g_cpt = NULL;
    vlog_alway("cedarx_video_exit\n");
    funtion_cb(MP4_STATUS_ERROR);
    return VIDEO_STATE_INIT;
}

static VideoStatus VideoContinue(void *msg)
{
    int ret = -1;

    if (msg == NULL)
        return VIDEO_STATE_CONTINUE;

    video_message *dmsg = msg;
    CdxStreamT *s = g_cpt->stream;
    VideoInfo *mpi = g_cpt->mpi;
    media_ringbuff *rb_point = NULL;
    unsigned *seek_time = NULL;

    if ((s == NULL) || (mpi == NULL))
        return VIDEO_STATE_CONTINUE;

    switch (dmsg->command) {
        case VIDEO_COMMAND_PREPARE:
            return VIDEO_STATE_CONTINUE;
        case VIDEO_COMMAND_CONTINUE:
            break;
        case VIDEO_COMMAND_STOP:
            // 关闭子线程
            rb_point = mrb_insert_data_lock(&video_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                mrb_insert_data_unlock(&video_stack);
                printf("video_stack insert stop cmd\n");
            }
            rb_point = mrb_insert_data_lock(&audio_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                mrb_insert_data_unlock(&audio_stack);
                printf("audio_stack insert stop cmd\n");
            }
            mp4_force_stop();
            video_command_message_reset();
            if (s_player_completed == 0) {
                funtion_cb(MP4_STATUS_STOP);
            } else {
                funtion_cb(MP4_STATUS_PLAY_FINISH);
                s_player_completed = 0;
            }
            return VIDEO_STATE_INIT;
        case VIDEO_COMMAND_PAUSE:
            // pause video
            // xTimerStop(g_avtimer->timer, portMAX_DELAY);
            rb_point = mrb_insert_data_lock(&video_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                rb_point->single_size = 0;
                mrb_insert_data_unlock(&video_stack);
                printf("video_stack insert pause cmd\n");
            }
            // pause audio
            rb_point = mrb_insert_data_lock(&audio_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                rb_point->single_size = 0;
                mrb_insert_data_unlock(&audio_stack);
                printf("audio_stack insert pause cmd\n");
            }
            return VIDEO_STATE_PAUSE;
        case VIDEO_COMMAND_SEEK:
            seek_time = dmsg->pri;
            if (seek_time[0] > mpi->Vduration) {
                vlog_error("seek time %dms is longer than limit %dms", seek_time[0], mpi->Vduration);
                break;
            }

            ret = mp4_seek_to_time(mpi, seek_time[0]);
            funtion_cb(MP4_STATUS_SEEK_FINISH);
            if (ret == 0) {
                mrb_reset(&video_stack);
                // mrb_reset(&decode_stack);
                mrb_reset(&audio_stack);
                // mrb_reset(&pcm_stack);
            }
            break;
        default:
            break;
    }

    if (g_avtimer->read_point == 0) {
        funtion_cb(MP4_STATUS_START_PLAYING);
    }

    /* get current sample */
    unsigned read_size = mpi->sample_map[g_avtimer->read_point].size;
    if (mpi->VideoType == VIDEO_TYPR_MPEG4) {
        if (mpi->sample_map[g_avtimer->read_point].priv == VIDEO_IS_IFRAME) {
            read_size += mpi->DinfoSize;
        }
    }

    /* read video data */
    if (mpi->sample_map[g_avtimer->read_point].type == SAMPLE_IS_VIDEO) {
        rb_point = mrb_write_data(&video_stack);
        if (rb_point != NULL) {
            if (mpi->VideoType == VIDEO_TYPR_MPEG4) {
                rb_point->single_size = read_size;
                if (mpi->sample_map[g_avtimer->read_point].priv == VIDEO_IS_IFRAME) {
                    memcpy(rb_point->buff, mpi->Dinfo, mpi->DinfoSize);
                    ret = CdxStreamRead(s, (rb_point->buff + mpi->DinfoSize), (read_size - mpi->DinfoSize));
                    if (ret != read_size - mpi->DinfoSize) {
                        printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size - mpi->DinfoSize);
                        goto error;
                    }
                } else {
                    ret = CdxStreamRead(s, rb_point->buff, read_size);
                    if (ret != read_size) {
                        printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size);
                        goto error;
                    }
                }
            } else { // h264
                if (mpi->first_flag == 0) {
                    read_size += mpi->DinfoSize;
                    memcpy(rb_point->buff, mpi->Dinfo, mpi->DinfoSize);
                    ret = CdxStreamRead(s, (rb_point->buff + mpi->DinfoSize), (read_size - mpi->DinfoSize));
                    if (ret != read_size - mpi->DinfoSize) {
                        printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size - mpi->DinfoSize);
                        goto error;
                    }
                    AvccToAnnexB((rb_point->buff + mpi->DinfoSize), (read_size - mpi->DinfoSize));
                    mpi->first_flag = 1;
                } else {
                    ret = CdxStreamRead(s, rb_point->buff, read_size);
                    if (ret != read_size) {
                        printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size);
                        goto error;
                    }
                    AvccToAnnexB(rb_point->buff, read_size);
                }
                rb_point->single_size = read_size;
            }

            rb_point->command = dmsg->command;
            mrb_write_finish(&video_stack);
        }
    /* read audio data */
    } else if (mpi->sample_map[g_avtimer->read_point].type == SAMPLE_IS_AUDIO) {
        rb_point = mrb_write_data(&audio_stack);
        if (rb_point != NULL) {
            if ((mpi->AudioType == AUDIO_TYPE_AAC) && (mpi->AinfoSize != 0)) {
                ret = UpdateAACPacketHdr(mpi->Ainfo, mpi->AinfoSize, read_size);
                if (ret != 0) {
                    printf("%s %d UpdateAACPacketHdr err %d\n", __func__, __LINE__, ret);
                    goto error;
                }
                memcpy(rb_point->buff, mpi->Ainfo, mpi->AinfoSize);
                rb_point->single_size = (read_size + mpi->AinfoSize);
                ret = CdxStreamRead(s, (rb_point->buff + mpi->AinfoSize), read_size);
                if (ret != read_size) {
                    printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size);
                    goto error;
                }
            } else {
                rb_point->single_size = read_size;
                ret = CdxStreamRead(s, rb_point->buff, read_size);
                if (ret != read_size) {
                    printf("%s %d CdxStreamRead err %d %d\n", __func__, __LINE__, ret, read_size);
                    goto error;
                }
            }
            rb_point->command = dmsg->command;
            rb_point->priv = mpi->sample_map[g_avtimer->read_point].priv;
            mrb_write_finish(&audio_stack);
        }
    }

    g_avtimer->read_point ++;
    if (g_avtimer->read_point >= mpi->sample_count) {
        video_message msg = {0};
        msg.command = VIDEO_COMMAND_STOP;
        // funtion_cb(MP4_STATUS_PLAY_FINISH);
        s_player_completed = 1;
        video_command_message_send(&msg, 0);
    } else {
        VideoCommandContinue();
    }
    return VIDEO_STATE_CONTINUE;

error:
    funtion_cb(MP4_STATUS_ERROR);
    return VIDEO_STATE_CONTINUE;
}

VideoStatus VideoPause(void *msg)
{
    if (msg == NULL) {
        vlog_error("NULL msg in pause status!\n");
        return VIDEO_STATE_PAUSE;
    }

    video_message *dmsg = msg;
    media_ringbuff *rb_point = NULL;
    unsigned *seek_time = NULL;
    VideoInfo *mpi = g_cpt->mpi;

    switch (dmsg->command) {
        case VIDEO_COMMAND_STOP:
            rb_point = mrb_insert_data_lock(&video_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                mrb_insert_data_unlock(&video_stack);
                mrb_continue(&video_stack);
            }
            rb_point = mrb_insert_data_lock(&audio_stack);
            if (rb_point != NULL) {
                rb_point->command = dmsg->command;
                mrb_insert_data_unlock(&audio_stack);
                mrb_continue(&audio_stack);
            }
            mp4_force_stop();
            // video_command_message_reset();
            if (s_player_completed == 0) {
                funtion_cb(MP4_STATUS_STOP);
            } else {
                s_player_completed = 0;
                funtion_cb(MP4_STATUS_PLAY_FINISH);
            }
            return VIDEO_STATE_INIT;
        case VIDEO_COMMAND_PAUSE:
            rb_point = mrb_insert_data_lock(&video_stack);
            if (rb_point != NULL) {
                rb_point->command = VIDEO_COMMAND_CONTINUE;
                mrb_insert_data_unlock(&video_stack);
                mrb_continue(&video_stack);
                // mrb_continue(&decode_stack);
            }

            rb_point = mrb_insert_data_lock(&audio_stack);
            if (rb_point != NULL) {
                rb_point->command = VIDEO_COMMAND_CONTINUE;
                mrb_insert_data_unlock(&audio_stack);
                mrb_continue(&audio_stack);
                // mrb_continue(&pcm_stack);
            }
            funtion_cb(MP4_STATUS_START_PLAYING);
            VideoCommandContinue();
            return VIDEO_STATE_CONTINUE;
        case VIDEO_COMMAND_SEEK:
            seek_time = dmsg->pri;
            if (seek_time[0] > mpi->Vduration) {
                vlog_error("seek time %dms is longer than limit %dms", seek_time[0], mpi->Vduration);
                break;
            }

            if (mp4_seek_to_time(mpi, seek_time[0]) < 0) {
                vlog_error("seek err");
                break;
            } else {
                funtion_cb(MP4_STATUS_SEEK_FINISH);

                mrb_reset(&video_stack);
                // mrb_reset(&decode_stack);
                mrb_reset(&audio_stack);
                // mrb_reset(&pcm_stack);

                VideoCommandContinue();
                funtion_cb(MP4_STATUS_START_PLAYING);
                return VIDEO_STATE_CONTINUE;
            }
        default:
            break;
    }
    return VIDEO_STATE_PAUSE;
}

const VideoEventContext VideoEvent[VIDEO_STATE_MAX] = {
    VideoInit,
    VideoContinue,
    VideoPause,
};

static void *fsm_video_handle = NULL;
static void fsm_video(void *arg)
{
    VideoStatus mstatus = VIDEO_STATE_INIT;
    video_message fsm_video_msg;
    memset(&fsm_video_msg, 0, sizeof(video_message));

    while(1) {
        video_command_message_recv(&fsm_video_msg);
/*
        VideoEvent[VIDEO_STATE_INIT]      = VideoInit
        VideoEvent[VIDEO_STATE_CONTINUE]  = VideoContinue
        VideoEvent[VIDEO_STATE_PAUSE]     = VideoPause

        execute function base on currernt status,and return new status
*/
        mstatus = VideoEvent[mstatus](&fsm_video_msg);
        // some command can be block, release
        if (fsm_video_msg.sem != NULL) {
            hal_sem_post(fsm_video_msg.sem);
        }

        memset(&fsm_video_msg, 0, sizeof(video_message));
    }
}

int create_video_task(void)
{
    if (video_command_message_create() < 0) {
        vlog_error("create_video_task fail");
        return -1;
    }

    fsm_video_handle = hal_thread_create(fsm_video, NULL, "video_thread", 2048, 20);
    if (fsm_video_handle == NULL) {
        video_command_message_destory();
        vlog_error("create_video_task fail");
        return -1;
    }
    return 0;
}

int destory_video_task(void)
{
    if (NULL != fsm_video_handle) {
        hal_thread_stop(fsm_video_handle);
        fsm_video_handle = NULL;
    }

    video_command_message_destory();

    return 0;
}
