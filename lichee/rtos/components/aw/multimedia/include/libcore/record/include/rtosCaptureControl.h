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

#ifndef RTOS_SOUND_CONTROL_H
#define RTOS_SOUND_CONTROL_H

#include "captureControl2.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef int (*AudioFrameCallback)(void* pUser, void* para);

typedef struct CapturePcmData
{
    unsigned char* pData;
    int   nSize;
    unsigned int samplerate;
    unsigned int channels;
    int accuracy;
} CapturePcmData;
*/

typedef enum CaptureStatus_t
{
    STATUS_START = 0,
    STATUS_PAUSE ,
    STATUS_STOP
}CaptureStatus;
// no used again
typedef struct CaptureCtrlContext_t
{
    CaptureCtrl                   base;
    unsigned long           chunk_size;
    unsigned int            alsa_format;
    void                   *alsa_handler;
    // snd_pcm_access_t            alsa_access_type;
    // snd_pcm_stream_t            alsa_open_mode;
    int                nSampleRate;
    unsigned int                nChannelNum;
    unsigned int                         alsa_fragcount;
    int                         alsa_can_pause;
    size_t                      bytes_per_sample;
    CaptureStatus                 sound_status;
    int                         mVolume;
    pthread_mutex_t             mutex;
    //AudioFrameCallback mAudioframeCallback;
    //void*                pUserData;
}CaptureCtrlContext;

CaptureCtrl* RTCaptureDeviceCreate();

void RTCaptureDeviceDestroy(CaptureCtrl* s);

void RTCaptureDeviceSetFormat(CaptureCtrl* s,CdxCapbkCfg* cfg);

int RTCaptureDeviceStart(CaptureCtrl* s);

int RTCaptureDeviceStop(CaptureCtrl* s);

int RTCaptureDeviceRead(CaptureCtrl* s, void* pData, int nDataSize);
#if 0
int RTCaptureDevicePause(CaptureCtrl* s);

int RTCaptureDeviceFlush(CaptureCtrl* s,void *block);

int RTCaptureDeviceWrite(CaptureCtrl* s, void* pData, int nDataSize);

int RTCaptureDeviceReset(CaptureCtrl* s);

int RTCaptureDeviceGetCachedTime(CaptureCtrl* s);
int RTCaptureDeviceGetFrameCount(CaptureCtrl* s);
int RTCaptureDeviceSetPlaybackRate(CaptureCtrl* s,const XAudioPlaybackRate *rate);

int RTCaptureDeviceSetVolume(CaptureCtrl* s,int volume);

int RTCaptureDeviceControl(CaptureCtrl* s, int cmd, void* para);
#endif
static CaptureControlOpsT mCaptureControlOps =
{
    .destroy          =   RTCaptureDeviceDestroy,
    .setFormat        =   RTCaptureDeviceSetFormat,
    .start            =   RTCaptureDeviceStart,
    .stop             =   RTCaptureDeviceStop,
    .read             =   RTCaptureDeviceRead,
#if 0
    .pause            =   RTCaptureDevicePause,
    .flush            =  RTCaptureDeviceFlush,
    .write            =   RTCaptureDeviceWrite,
    .reset            =   RTCaptureDeviceReset,
    .getCachedTime    =   RTCaptureDeviceGetCachedTime,
    .getFrameCount    =   RTCaptureDeviceGetFrameCount,
    .setPlaybackRate  =   RTCaptureDeviceSetPlaybackRate,
    .control          =   RTCaptureDeviceControl,
#endif    
};

#ifdef __cplusplus
}
#endif

#endif
