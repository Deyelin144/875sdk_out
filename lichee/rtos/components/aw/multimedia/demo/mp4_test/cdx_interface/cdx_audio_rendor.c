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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AudioSystem.h>
#include "cdx_audio_rendor.h"
#include "hal_time.h"

struct AudioPlayManager {
    //The snd interface implementation already has a mutex, so no additional mutex is created
    tAudioTrack *as_handler;
    // unsigned bytes_per_sample;
    // unsigned size_to_frame;
    // unsigned pause_status;
    // unsigned char align[8];
    // unsigned rebyte;
};

static struct AudioPlayManager *adpm = NULL;
static int s_paused_flag = 0;

int Mp4AudioPause(void)
{
    if (adpm == NULL) {
        return -1;
    }

    s_paused_flag = 1;
    AudioTrackStop(adpm->as_handler);
    return 0;
}

int Mp4AudioStop(void)
{
    if (adpm == NULL) {
        return -1;
    }

    AudioTrackStop(adpm->as_handler);

    return 0;
}

int Mp4AudioPlay(unsigned char *pcm_buff, unsigned pcm_size)
{
    if (adpm == NULL) {
        return -1;
    }

    if (s_paused_flag) {
        AudioTrackStart(adpm->as_handler);
        s_paused_flag = 0;
    }

    AudioTrackWrite(adpm->as_handler, pcm_buff, pcm_size);
    return 0;
}

int VideoAudioPlayInit(struct AudioPlayFmt *afmt)
{
        if (afmt == NULL)
        return -1;

    tAudioTrack *as_handler = AudioTrackCreateWithStreamNoMix("default", AUDIO_STREAM_MUSIC);
    if (as_handler == NULL) {
        return -1;
    }

    AudioTrackSetup(as_handler, afmt->SampleRate, afmt->Channels, afmt->Bits);

    if (AudioTrackStart(as_handler) < 0) {
        AudioTrackDestroy(as_handler);
        return -1;
    }

    adpm = malloc(sizeof(struct AudioPlayManager));
    if (adpm == NULL) {
        AudioTrackStop(as_handler);
        AudioTrackDestroy(as_handler);
        return -1;
    }
    memset(adpm, 0, sizeof(struct AudioPlayManager));
    adpm->as_handler = as_handler;
    return 0;
}

int VideoAudioPlayDeinit(void)
{
    if (adpm == NULL) {
        return -1;
    }

    if (adpm->as_handler != NULL) {
        AudioTrackStop(adpm->as_handler);
        AudioTrackDestroy(adpm->as_handler);
    }

    free(adpm);
    adpm = NULL;
    return 0;
}