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

//#define LOG_TAG "tsoundcontrol"
//#include "tlog.h"
#include "FreeRTOS_POSIX/pthread.h"
#include <types.h>
#include <FreeRTOS.h>
#include "rtosCaptureControl.h"
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
//#include "auGaincom.h"

#include <AudioRecord.h>

typedef struct CaptureCtrlAs_t {
    CaptureCtrl base;
    unsigned long chunk_size;
    int bit;
    void *as_handler;
    int nSampleRate;
    unsigned int nChannelNum;
    unsigned int alsa_fragcount;
    int alsa_can_pause;
    size_t bytes_per_sample;
    CaptureStatus sound_status;
    int mVolume;
    pthread_mutex_t mutex;
    //AudioFrameCallback mAudioframeCallback;
    //void*                pUserData;
} CaptureCtrlAs;

int CAP_BLOCK_MODE = 0;
int CAP_NON_BLOCK_MODE = 1;
static char g_ar_name[12] = "default";
static int openCaptureDevice(CaptureCtrlAs *sc, int mode);
static int closeCaptureDevice(CaptureCtrlAs *sc);
static int setCaptureDeviceParams(CaptureCtrlAs *sc);
static int openCaptureDevice(CaptureCtrlAs *sc, int mode)
{
    int ret = 0;
    logd("openCaptureDevice()\n");
    if (!sc->as_handler) {
        sc->as_handler = AudioRecordCreate(g_ar_name); //hw:audiocodec
        if (!sc->as_handler) {
            loge("open audio device failed");
        }
    } else {
        logd("the audio device has been opened\n");
    }
    return ret;
}

static int closeCaptureDevice(CaptureCtrlAs *sc)
{
    int ret = 0;
    logd("closeCaptureDevice()\n");
    if (sc->as_handler) {
        ret = AudioRecordDestroy(sc->as_handler);
        if (ret < 0) {
            loge("snd_pcm_close failed:%s\n", strerror(errno));
        } else {
            sc->as_handler = NULL;
            logd("alsa-uninit: pcm closed\n");
        }
    }
    return ret;
}

static int setCaptureDeviceParams(CaptureCtrlAs *sc)
{
    int ret = 0;

    logd("setCaptureDeviceParams()\n");
    sc->bytes_per_sample = sc->nChannelNum * sc->bit / 8;
    sc->alsa_fragcount = 8; //cache count
    sc->chunk_size = 1024;

    ret = AudioRecordSetup(sc->as_handler, sc->nSampleRate, sc->nChannelNum, sc->bit);

    logd("bit:%d, SampleRate:%d, ch:%d", sc->bit, sc->nSampleRate, sc->nChannelNum);

    sc->alsa_can_pause = 0;

    logd("setCaptureDeviceParams():sc->alsa_can_pause = %d\n", sc->alsa_can_pause);

    return ret;
}

CaptureCtrl *RTCaptureDeviceCreate()
{
    CaptureCtrlAs *s;
    s = (CaptureCtrlAs *)malloc(sizeof(CaptureCtrlAs));
    logd("RTCaptureDeviceCreate()\n");
    if (s == NULL) {
        loge("malloc CaptureCtrlAs fail.\n");
        return NULL;
    }
    memset(s, 0, sizeof(CaptureCtrlAs));
    s->base.ops = &mCaptureControlOps;
    s->nSampleRate = 8000; //8000
    s->nChannelNum = 1;    //1
    s->bit = 16;
    s->alsa_can_pause = 0;
    s->sound_status = STATUS_STOP;
    s->mVolume = 0;
    //s->mAudioframeCallback = callback;
    //s->pUserData = pUser;
    pthread_mutex_init(&s->mutex, NULL);
    int ret = openCaptureDevice(s, CAP_BLOCK_MODE);
    if (ret != 0) {
        loge("open sound device fail\n");
        free(s);
        s = NULL;
        return NULL;
    }
    return (CaptureCtrl *)&s->base;
}

void RTCaptureDeviceDestroy(CaptureCtrl *s)
{
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    pthread_mutex_lock(&sc->mutex);
    logd("RTCaptureDeviceDestroy(),close sound device\n");
    closeCaptureDevice(sc);
    pthread_mutex_unlock(&sc->mutex);
    pthread_mutex_destroy(&sc->mutex);
    free(sc);
    sc = NULL;
}

void RTCaptureDeviceSetFormat(CaptureCtrl *s, CdxCapbkCfg *cfg)
{
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    pthread_mutex_lock(&sc->mutex);
    logd("RTCaptureDeviceSetFormat(),sc->sound_status == %d\n", sc->sound_status);
    if (sc->sound_status == STATUS_STOP) {
        sc->nSampleRate = cfg->nSamplerate;
        sc->nChannelNum = cfg->nChannels;
        sc->bit = cfg->nBitpersample;
        sc->bytes_per_sample = sc->nChannelNum * sc->bit / 8;
        logd("RTCaptureDeviceSetFormat()>>>sample_rate:%d,channel_num:%d,sc->bytes_per_sample:%d\n",
             cfg->nSamplerate, cfg->nChannels, sc->bytes_per_sample);
    }
    pthread_mutex_unlock(&sc->mutex);
}

int RTCaptureDeviceStart(CaptureCtrl *s)
{
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    pthread_mutex_lock(&sc->mutex);
    int ret = 0;
    logd("TinaCaptureDeviceStart(): sc->sound_status = %d\n", sc->sound_status);
    if (sc->sound_status == STATUS_START) {
        logd("Capture device already start.\n");
        pthread_mutex_unlock(&sc->mutex);
        return ret;
    } else if (sc->sound_status == STATUS_PAUSE) {
        AudioRecordStart(sc->as_handler);
        sc->sound_status = STATUS_START;
    } else if (sc->sound_status == STATUS_STOP) {
        ret = setCaptureDeviceParams(sc);
        if (ret < 0) {
            loge("setCaptureDeviceParams fail , ret = %d\n", ret);
            pthread_mutex_unlock(&sc->mutex);
            return ret;
        }
        AudioRecordStart(sc->as_handler);
        sc->sound_status = STATUS_START;
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

int RTCaptureDeviceStop(CaptureCtrl *s)
{
    int ret = 0;
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    pthread_mutex_lock(&sc->mutex);
    logd("RTCaptureDeviceStop():sc->sound_status = %d\n", sc->sound_status);
    if (sc->sound_status == STATUS_STOP) {
        logd("Capture device already stopped.\n");
        pthread_mutex_unlock(&sc->mutex);
        return ret;
    } else {
        AudioRecordStop(sc->as_handler);
        sc->sound_status = STATUS_STOP;
    }
    pthread_mutex_unlock(&sc->mutex);
    return ret;
}

int RTCaptureDeviceRead(CaptureCtrl *s, void *pData, int nDataSize)
{
    int ret = 0;
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    logd("TinaCaptureDeviceRead:sc->bytes_per_sample = %d, nDataSize=%d\n", sc->bytes_per_sample,
         nDataSize);
    if (sc->bytes_per_sample == 0) {
        sc->bytes_per_sample = 4;
    }
    if (sc->sound_status == STATUS_STOP || sc->sound_status == STATUS_PAUSE) {
        return ret;
    }
    //logd("TinaCaptureDeviceWrite>>> pData = %p , nDataSize = %d\n",pData,nDataSize);
    int num_frames = nDataSize / sc->bytes_per_sample;
    int res = 0;

    if (!sc->as_handler) {
        loge("MSGTR_AO_ALSA_DeviceConfigurationError\n");
        return -1;
    }

    if (num_frames == 0) {
        loge("num_frames == 0\n");
        return -1;
    }
    logd("[debug]num_frames=%d", num_frames);
    do {
        res = AudioRecordRead(sc->as_handler, pData, nDataSize);
        if (res > 0)
            res = res / sc->bytes_per_sample;
        if (res < 0) {
            loge("MSGTR_AO_ALSA_ReadError,res = %ld\n", res);
        }
    } while (res == 0);
    return res < 0 ? res : res * sc->bytes_per_sample;
}

int RTCaptureDeviceControl(CaptureCtrl *s, int cmd, void *para)
{
    CaptureCtrlAs *sc;
    sc = (CaptureCtrlAs *)s;
    if (sc) {
        return 0;
    } else {
        return -1;
    }
}
