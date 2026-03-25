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

// #include <pcm.h>
#include "soundControl.h"
#ifndef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include "speex_resampler.h"
#else
#include "AudioSystem.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/*
typedef int (*AudioFrameCallback)(void* pUser, void* para);

typedef struct SoundPcmData
{
    unsigned char* pData;
    int   nSize;
    unsigned int samplerate;
    unsigned int channels;
    int accuracy;
} SoundPcmData;
*/

typedef enum SoundStatus_t
{
    STATUS_START = 0,
    STATUS_PAUSE ,
    STATUS_STOP
}SoundStatus;

#ifndef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
typedef struct SoundCtrlContext_t
{
    SoundCtrl                   base;
    snd_pcm_t                   *alsa_handler;
    snd_pcm_uframes_t           chunk_size;
    snd_pcm_format_t            alsa_format;
    snd_pcm_hw_params_t         *alsa_hwparams;
    snd_pcm_access_t            alsa_access_type;
    snd_pcm_stream_t            alsa_open_mode;
    int                nSampleRate;
    unsigned int                nChannelNum;
    unsigned int                         alsa_fragcount;
    int                         alsa_can_pause;
    size_t                      bytes_per_sample;
    SoundStatus                 sound_status;
    int                         mVolume;
    pthread_mutex_t             mutex;
    SpeexResamplerState         *resampler;
    int                         nTargetSampleRate;
    unsigned int                nTargetChannelNum;
    void                        *buffer_out;
    int                         buffer_out_size;
    int                         callback_cnt;
    int                         alsa_card_num;
    //AudioFrameCallback mAudioframeCallback;
    //void*                pUserData;
}SoundCtrlContext;
#else
typedef struct SoundCtrlContext_t
{
    SoundCtrl                   base;
    void                        *as_handler;

    unsigned long               chunk_size;
    unsigned int                nbit;
    int                         nSampleRate;
    unsigned int                nChannelNum;
    unsigned int                alsa_fragcount;
    int                         alsa_can_pause;
    size_t                      bytes_per_sample;
    SoundStatus                 sound_status;
    int                         mVolume;
    pthread_mutex_t             mutex;
    int                         callback_cnt;
    int                         alsa_card_num;
    //AudioFrameCallback mAudioframeCallback;
    //void*                pUserData;
}SoundCtrlContext;
#endif

SoundCtrl* RTSoundDeviceCreate(int card);

void RTSoundDeviceDestroy(SoundCtrl* s);

void RTSoundDeviceSetFormat(SoundCtrl* s,CdxPlaybkCfg* cfg);

int RTSoundDeviceStart(SoundCtrl* s);

int RTSoundDeviceStop(SoundCtrl* s);

int RTSoundDevicePause(SoundCtrl* s);

int RTSoundDeviceFlush(SoundCtrl* s,void *block);

int RTSoundDeviceWrite(SoundCtrl* s, void* pData, int nDataSize);

int RTSoundDeviceReset(SoundCtrl* s);

int RTSoundDeviceGetCachedTime(SoundCtrl* s);
int RTSoundDeviceGetFrameCount(SoundCtrl* s);
int RTSoundDeviceSetPlaybackRate(SoundCtrl* s,const XAudioPlaybackRate *rate);

int RTSoundDeviceSetVolume(SoundCtrl* s,int volume);

int RTSoundDeviceControl(SoundCtrl* s, int cmd, void* para);

static SoundControlOpsT mSoundControlOps =
{
    .destroy          =   RTSoundDeviceDestroy,
    .setFormat        =   RTSoundDeviceSetFormat,
    .start            =   RTSoundDeviceStart,
    .stop             =   RTSoundDeviceStop,
    .pause            =   RTSoundDevicePause,
    .flush            =  RTSoundDeviceFlush,
    .write            =   RTSoundDeviceWrite,
    .reset            =   RTSoundDeviceReset,
    .getCachedTime    =   RTSoundDeviceGetCachedTime,
    .getFrameCount    =   RTSoundDeviceGetFrameCount,
    .setPlaybackRate  =   RTSoundDeviceSetPlaybackRate,
    .control          =   RTSoundDeviceControl,
};

#define SOUNDDEVICE_CREATE      1
#define SOUNDDEVICE_DESTROY     2
#define SOUNDDEVICE_WRITE_PRE   3
#define SOUNDDEVICE_WRITE       4

typedef void (* RTSoundDevice_cb_t)(void *, void *, int, int, int);
//void callback(void *context, void *arg, int status, int cnt, int frame);
void RTSoundDeviceSetCallback(RTSoundDevice_cb_t cb, void *arg);

int RTSoundDeviceGetCardNum(void *context);

#ifdef __cplusplus
}
#endif

#endif
