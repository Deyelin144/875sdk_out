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

#ifdef CONFIG_COMPONENTS_AW_AUDIO_SYSTEM
#include "rtosSound.h"
#include "aw-alsa-lib/control.h"
#include <stdlib.h>
#include <AudioSystem.h>

typedef struct _AsCtrlContext {
    tAudioTrack *as_handler;
    SoundStatus sound_status;
    int nTargetSampleRate;
    char nChannelNum;
    char bits;
    char bytes_per_sample;
} AsCtrlContext;

extern int AudioTrackTbufCtrl(tAudioTrack *at, int onoff);

static void *openAsDevice(int mode)
{
    AsCtrlContext *asc = NULL;
    CARDLOG("openSoundDevice()\n");

    asc = malloc(sizeof(AsCtrlContext));
    if (asc == NULL) {
        CARDERR("as sound card malloc fail\n");
        return NULL;
    }
    memset(asc, 0, sizeof(AsCtrlContext));

    asc->as_handler = AudioTrackCreateWithStreamNoMix("default", AUDIO_STREAM_MUSIC);
    if (asc->as_handler == NULL) {
        CARDERR("the audio device open fail!\n");
        free(asc);
        return NULL;
    }
    AudioTrackTbufCtrl(asc->as_handler, 0);

    asc->sound_status = STATUS_STOP;
    return (void *)asc;
}

static void closeSoundDevice(SoundCtrl *s)
{
    CARDLOG("closeSoundDevice()\n");

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    AudioTrackDestroy(asc->as_handler);
    free(asc);
    // release RTSoundDeviceCreate
    free(rtContext);
}

static void setSoundFormat(SoundCtrl *s, CdxPlaybkCfg *cfg)
{
    CARDLOG("RTSoundDeviceSetFormat()\n");

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->sound_status == STATUS_STOP) {
        asc->nTargetSampleRate = cfg->nSamplerate;
        asc->nChannelNum = (char)cfg->nChannels;
        asc->bits = (char)cfg->nBitpersample;
        asc->bytes_per_sample = asc->nChannelNum * asc->bits / 8;
        AudioTrackSetup(asc->as_handler, asc->nTargetSampleRate, asc->nChannelNum, asc->bits);
    }
}

static int startSoundDevice(SoundCtrl *s)
{
    int ret = 0;
    CARDLOG("TinaSoundDeviceStart()\n");

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->sound_status == STATUS_START) {
        CARDLOG("Sound device already start.\n");
    } else if (asc->sound_status == STATUS_PAUSE) {
        AudioTrackStart(asc->as_handler);
        asc->sound_status = STATUS_START;
    } else if (asc->sound_status == STATUS_STOP) {
        AudioTrackStart(asc->as_handler);
        asc->sound_status = STATUS_START;
    }

    return ret;
}

static int stopSoundDevice(SoundCtrl *s)
{
    CARDLOG("RTSoundDeviceStop()\n");

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->sound_status == STATUS_STOP) {
        CARDLOG("Sound device already stopped.\n");
    } else {
        AudioTrackStop(asc->as_handler);
        asc->sound_status = STATUS_STOP;
    }
    return 0;
}

static int pauseSoundDevice(SoundCtrl *s)
{
    CARDLOG("RTSoundDevicePause()%d\n");

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->sound_status == STATUS_START) {
        AudioTrackStop(asc->as_handler);
        asc->sound_status = STATUS_PAUSE;
    } else {
        CARDLOG("RTSoundDevicePause(): pause in an invalid status,status = %d\n", asc->sound_status);
    }
    return 0;
}

static int flushSoundDevice(SoundCtrl *s, void *block)
{
    CARDLOG("to do");
    return 0;
}

static int writeSoundDevice(SoundCtrl *s, void *pData, int nDataSize)
{
    int res = 0;
    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->bytes_per_sample == 0) {
        asc->bytes_per_sample = 4;
    }
    if (asc->sound_status == STATUS_STOP || asc->sound_status == STATUS_PAUSE) {
        return res;
    }

    // There will be problems if nDataSize is not a multiple of 4,
    // but the value returned by cedarx must be a multiple of 4,
    // so no special processing is done here.
    int num_frames = nDataSize / asc->bytes_per_sample;

    if (!asc->as_handler) {
        CARDERR("MSGTR_AO_ALSA_DeviceConfigurationError\n");
        return -1;
    }

    if (num_frames == 0) {
        CARDERR("num_frames == 0\n");
        return -1;
    }

    return AudioTrackWrite(asc->as_handler, pData, nDataSize);
}

static int getSoundeDeviceCache(SoundCtrl *s)
{
    int ret = 0;
    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    AsCtrlContext *asc = (AsCtrlContext *)rtContext->handle;

    if (asc->as_handler) {
        ret = AudioTrackDelay(asc->as_handler);
        if (ret < 0) {
            CARDERR("AudioTrackDelay return %d\n", ret);
        }
        ret = 0;
    }
    return ret;
}

static int setSoundDeviceVol(unsigned *vol)
{
    int ret = 0;
    uint32_t volume_value = 0;
    unsigned defvol = (unsigned)(*vol);
    // cedrax vol range is 0 ~ 100，dac vol range is 0 ~ 10
    defvol = (defvol * 10 / 100);

    volume_value = ((defvol << 16) | defvol);
    ret = softvol_control_with_streamtype(AUDIO_STREAM_MUSIC, &volume_value, 1);
    if (ret != 0) {
        CARDERR("error:set softvol failed!\n");
    }

    return ret;
}

static int getSoundDeviceVol(unsigned *vol)
{
    uint32_t volume_value = 0;
    int ret = softvol_control_with_streamtype(AUDIO_STREAM_MUSIC, &volume_value, 0);
    if (ret != 0) {
        CARDERR("get softvol range failed:%d\n", ret);
    }

    return (volume_value & 0xffff);
}

static int ctrlSoundDevice(SoundCtrl *s, int cmd, void *para)
{
    switch (cmd) {
    case SOUND_CONTROL_SET_VOLUME:
        setSoundDeviceVol(para);
        break;
    case SOUND_CONTROL_GET_VOLUME:
        getSoundDeviceVol(para);
        break;
    default:
        break;
    }
}

static int SetSoundDeviceRate(SoundCtrl *s, const XAudioPlaybackRate *rate)
{
    CARDLOG("\n");
    return 0;
}

static int GetSoundDeviceFrameCount(SoundCtrl *s)
{
    CARDLOG("\n");
    return 0;
}

const SoundControlOpsT AsOps = {
    .destroy = closeSoundDevice,
    .setFormat = setSoundFormat,
    .start = startSoundDevice,
    .stop = stopSoundDevice,
    .pause = pauseSoundDevice,
    .flush = flushSoundDevice,
    .write = writeSoundDevice,
    .reset = stopSoundDevice,
    .getCachedTime = getSoundeDeviceCache,
    .getFrameCount = GetSoundDeviceFrameCount,
    .setPlaybackRate = SetSoundDeviceRate,
    .control = ctrlSoundDevice,
};

int CedarxRenderRegisterAudioSystem(void)
{
    return CedarxRenderRegister(openAsDevice, &AsOps, "AudioSystem");
}
#endif