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

#include <string.h>
#include <stdlib.h>
#include "audio_process.h"
#include "ringbuffer.h"
#include "log.h"
#include "echo_cancellation.h"
#include "noise_suppression.h"
#include "webrtc_vad.h"

//webrtc only support 10ms
#define AS_PROCESS_PERIOD_MS 10
#define AS_AEC_DEBUG         0

struct audio_process {
    void *pAecmInst;    // AEC
    NsHandle *pNS_inst; // NS
    VadInst *pVadInst;  // VAD

    struct ringbuff *process_rb;

    short *out_data;  //aec
    short *near_data; //aec
    short *far_data;  //aec
    uint8_t *pcm_buff;

    struct thread *process_thread;
    int (*process_cb)(uint8_t *data, int size, int is_vad);

#if AS_AEC_DEBUG
    FILE *f_near;
    FILE *f_far;
    FILE *f_out;
#endif
    int sample;
    int channel;
};

extern int msleep(unsigned int msecs);

static void audio_codec_process_thread(void *data)
{
    struct audio_process *audio_p = (struct audio_process *)data;
    if (audio_p == NULL) {
        msleep(10);
    }

    short *out_data = audio_p->out_data;
    short *near_data = audio_p->near_data;
    short *far_data = audio_p->far_data;
    uint8_t *pcm_buff = audio_p->pcm_buff;

    int process_bytes;
    int process_frame;
    int ret;

#if AS_AEC_DEBUG
    int test_size = 0;
#endif

    CHATBOX_INFO("process start\n");

    process_frame = (audio_p->sample * AS_PROCESS_PERIOD_MS / 1000);
    process_bytes = (process_frame * audio_p->channel * 2);

    while (1) {
        ret = ringbuff_available(audio_p->process_rb);
        // x2 for aec
        if (ret < (process_bytes * 2)) {
            msleep(10);
            continue;
        }

        ret = ringbuff_read(audio_p->process_rb, pcm_buff, (process_bytes * 2));
        if (ret != (process_bytes * 2)) {
            CHATBOX_ERROR("get record data fail!\n");
            continue;
        }

        for (int i = 0; i < process_frame; i++) {
            near_data[i] = ((short *)pcm_buff)[2 * i];
            far_data[i] = ((short *)pcm_buff)[2 * i + 1];
        }

        ret = WebRtcAec_BufferFarend(audio_p->pAecmInst, far_data, process_frame);
        if (ret < 0) {
            CHATBOX_ERROR("Aec BufferFarend failed!\n");
            break;
        }
#if AS_AEC_DEBUG
        if (process->f_far != NULL) {
            fwrite(far_data, 1, process_bytes, audio_p->f_far);
            fwrite(near_data, 1, process_bytes, audio_p->f_near);
        }
#endif
        ret = WebRtcAec_Process(audio_p->pAecmInst, near_data, NULL, out_data, NULL, process_frame,
                                10, 0);
        if (ret < 0) {
            CHATBOX_ERROR("Aec Process failed!\n");
            break;
        }
#if AS_AEC_DEBUG
        if (audio_p->f_far != NULL) {
            fwrite(out_data, 1, record_frame_bytes, audio_p->f_out);
            test_size += record_frame_bytes;

            if (test_size >= 500 * 1024) {
                fclose(audio_p->f_far);
                fclose(audio_p->f_near);
                fclose(audio_p->f_out);
                CHATBOX_INFO("aec file save\n");
                audio_p->f_far = NULL;
            }
        }
#endif

        ret = WebRtcNs_Process(audio_p->pNS_inst, out_data, NULL, near_data, NULL);
        if (ret < 0) {
            CHATBOX_ERROR("ns Process failed!\n");
            continue;
        } else {
            out_data = near_data;
        }

        ret = WebRtcVad_Process(audio_p->pVadInst, audio_p->sample, out_data, process_frame);

        if (audio_p->process_cb != NULL)
            audio_p->process_cb((uint8_t *)out_data, process_bytes, ret);
    }
}

static void *audio_process_aec_init(int sample, int channel)
{
    void *pAecmInst = NULL;
    AecConfig AecConfig;
    int ret;

    ret = WebRtcAec_Create(&pAecmInst);
    if (ret < 0)
        return NULL;

    ret = WebRtcAec_Init(pAecmInst, sample, sample);
    if (ret < 0)
        goto process_aec_init_err;

    memset(&AecConfig, 0, sizeof(AecConfig));
    AecConfig.nlpMode = kAecNlpAggressive;
    ret = WebRtcAec_set_config(pAecmInst, AecConfig);
    if (ret < 0)
        goto process_aec_init_err;

    return pAecmInst;

process_aec_init_err:
    if (pAecmInst != NULL)
        WebRtcAec_Free(pAecmInst);
}

static NsHandle *audio_process_ns_init(int sample, int channel)
{
    NsHandle *pNS_inst = NULL;
    int ret;

    ret = WebRtcNs_Create(&pNS_inst);
    if (ret < 0)
        goto process_init_ns_err;

    ret = WebRtcNs_Init(pNS_inst, sample);
    if (ret < 0)
        goto process_init_ns_err;

    //mode: 0: Mild, 1: Medium , 2: Aggressive
    ret = WebRtcNs_set_policy(pNS_inst, 1);
    if (ret < 0)
        goto process_init_ns_err;

    return pNS_inst;

process_init_ns_err:
    if (pNS_inst != NULL)
        WebRtcNs_Free(pNS_inst);

    return NULL;
}

static VadInst *audio_process_vad_init(int sample, int channel)
{
    VadInst *pVadInst = NULL;
    int ret;

    ret = WebRtcVad_Create(&pVadInst);
    if (ret < 0)
        goto process_init_vad_err;

    ret = WebRtcVad_Init(pVadInst);
    if (ret < 0)
        goto process_init_vad_err;

    // Aggressiveness mode (0, 1, 2, or 3).
    ret = WebRtcVad_set_mode(pVadInst, 3);
    if (ret < 0)
        goto process_init_vad_err;

    return pVadInst;

process_init_vad_err:
    if (pVadInst != NULL)
        WebRtcVad_Free(pVadInst);

    return NULL;
}

static int audio_process_buff_init(struct audio_process *audio_p)
{
    short *out_data = NULL;
    short *far_data = NULL;
    short *near_data = NULL;
    uint8_t *pcm_buff = NULL;
    struct ringbuff *process_rb;
    int process_bytes;

    process_bytes = (audio_p->sample * AS_PROCESS_PERIOD_MS / 1000);
    process_bytes = (process_bytes * audio_p->channel * 2);

    out_data = (short *)malloc(process_bytes);
    if (out_data == NULL)
        goto process_init_data_err;

    far_data = (short *)malloc(process_bytes);
    if (far_data == NULL)
        goto process_init_data_err;

    near_data = (short *)malloc(process_bytes);
    if (near_data == NULL)
        goto process_init_data_err;

    // *2 for aec
    pcm_buff = malloc(process_bytes * 2);
    if (pcm_buff == NULL)
        goto process_init_data_err;

    process_rb = ringbuff_init(process_bytes * 20);
    if (process_rb == NULL)
        goto process_init_data_err;

    audio_p->far_data = far_data;
    audio_p->near_data = near_data;
    audio_p->out_data = out_data;
    audio_p->pcm_buff = pcm_buff;
    audio_p->process_rb = process_rb;

    return 0;

process_init_data_err:
    if (out_data != NULL)
        free(out_data);

    if (far_data != NULL)
        free(far_data);

    if (near_data != NULL)
        free(near_data);

    if (pcm_buff != NULL)
        free(pcm_buff);

    if (process_rb != NULL)
        ringbuff_deinit(process_rb);

    return -1;
}

static void audio_process_buff_deinit(struct audio_process *audio_p)
{
    free(audio_p->far_data);
    free(audio_p->near_data);
    free(audio_p->out_data);
    free(audio_p->pcm_buff);
    ringbuff_deinit(audio_p->process_rb);
}

struct audio_process *audio_process_init(int sample, int channel)
{
    void *pAecmInst = NULL;
    NsHandle *pNS_inst = NULL;
    VadInst *pVadInst = NULL;
    struct audio_process *audio_p = NULL;
    struct thread *process_thread;

    pAecmInst = audio_process_aec_init(sample, channel);
    if (pAecmInst == NULL)
        goto process_init_err;

    pNS_inst = audio_process_ns_init(sample, channel);
    if (pAecmInst == NULL)
        goto process_init_err;

    pVadInst = audio_process_vad_init(sample, channel);
    if (pVadInst == NULL)
        goto process_init_err;

    audio_p = malloc(sizeof(struct audio_process));
    if (audio_p == NULL)
        goto process_init_err;

    audio_p->pAecmInst = pAecmInst;
    audio_p->pNS_inst = pNS_inst;
    audio_p->pVadInst = pVadInst;
    audio_p->sample = sample;
    audio_p->channel = channel;

    if (audio_process_buff_init(audio_p) < 0)
        goto process_init_err;

#if AS_AEC_DEBUG
    audio_p->f_near = fopen("sdmmc/near.pcm", "wb");
    audio_p->f_far = fopen("sdmmc/far.pcm", "wb");
    audio_p->f_out = fopen("sdmmc/out.pcm", "wb");
    if ((audio_p->f_near == NULL) || (audio_p->f_far == NULL) || (audio_p->f_out == NULL)) {
        CHATBOX_ERROR("test file create fail\n");
        while (1)
            ;
    }
    CHATBOX_INFO("file init sucess\n");
#endif

    process_thread = thread_create(audio_codec_process_thread, audio_p, "chat_audio_process",
                                   (4 * 1024), 18);
    if (process_thread == NULL) {
        audio_process_buff_deinit(audio_p);
        CHATBOX_ERROR("audio process thread create err\n");
        goto process_init_err;
    }
    audio_p->process_thread = process_thread;

    return audio_p;

process_init_err:
    if (pAecmInst != NULL)
        WebRtcAec_Free(pAecmInst);

    if (pNS_inst != NULL)
        WebRtcNs_Free(pNS_inst);

    if (pVadInst != NULL)
        WebRtcVad_Free(pVadInst);

    return NULL;
}

int audio_process_deinit(struct audio_process *audio_p)
{
    thread_stop(audio_p->process_thread);
    WebRtcAec_Free(audio_p->pAecmInst);
    WebRtcNs_Free(audio_p->pNS_inst);
    audio_process_buff_deinit(audio_p);
    free(audio_p);

    return 0;
}

int audio_process_input(struct audio_process *audio_p, uint8_t *data, int len)
{
    if ((audio_p == NULL) || (audio_p->process_rb == NULL))
        return 0;

    return ringbuff_write(audio_p->process_rb, data, len);
}

int audio_process_register_output(struct audio_process *audio_p,
                                  int (*cb)(uint8_t *data, int size, int is_vad))
{
    if (audio_p == NULL) {
        CHATBOX_ERROR("please init audio process at first!\n");
        return -1;
    }

    audio_p->process_cb = cb;
    return 0;
}

int audio_process_start(struct audio_process *audio_p)
{
    CHATBOX_ERROR("TODO\n");
}

int audio_process_stop(struct audio_process *audio_p)
{
    CHATBOX_ERROR("TODO\n");
}
