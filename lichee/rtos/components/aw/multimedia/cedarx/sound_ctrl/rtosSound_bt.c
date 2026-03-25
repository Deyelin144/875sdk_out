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

#ifdef CONFIG_SUN20IW2_BT_CONTROLLER
#include "rtosSound.h"
#include <stdlib.h>
#include <bt_manager.h>
#include <osal/hal_atomic.h>

int hal_msleep(unsigned int msecs);

typedef enum BtCardState_t
{
    BT_STATE_START = 0,
    BT_STATE_PAUSE ,
    BT_STATE_STOP
} BtCardState;

typedef struct _BtCtrlContext {
    int nTargetSampleRate;
    char nChannelNum;
    unsigned bt_lock_data;
    hal_spinlock_t bt_lock;
} BtCtrlContext;

static bt_a2dp_source_audio_state_cb app_state_cb = NULL;
static BtCardState cdx_bt_state = BT_STATE_STOP;
static BtCtrlContext *btc_handle = NULL;

static void cedarx_bt_state_report(const char *bd_addr, btmg_a2dp_source_audio_state_t state)
{
    if (app_state_cb != NULL) {
        app_state_cb(bd_addr, state);
    }

    if (state == BTMG_A2DP_SOURCE_AUDIO_STARTED) {
        cdx_bt_state = BT_STATE_START;
    } else if (state == BTMG_A2DP_SOURCE_AUDIO_STOPPED) {
        cdx_bt_state = BT_STATE_STOP;
    } else if (state == BTMG_A2DP_SOURCE_AUDIO_SUSPENDED) {
        cdx_bt_state = BT_STATE_PAUSE;
    }
}

static int bt_state_wait(BtCardState wait_state)
{
    int wait_cnt = 0;
    do {
        if (cdx_bt_state == wait_state) {
            break;
        }
        hal_msleep(10);
        wait_cnt ++;
    } while(wait_cnt < 100);

    if (wait_cnt >= 100) {
        CARDERR("bt wait %d tate fail!\n", wait_state);
        return -1;
    }
    return 0;
}

static void *openBtDevice(int mode)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);
    if (btc_handle != NULL) {
        return (void *)btc_handle;
    }

    BtCtrlContext *btc = malloc(sizeof(BtCtrlContext));

    if (btc == NULL) {
        CARDERR("as sound card malloc fail\n");
        return NULL;
    }
    memset(btc, 0, sizeof(BtCtrlContext));

    // check bt state
    btmg_callback_t *old_cb = NULL;
    old_cb = btmg_main_callback_get();
    if (old_cb == NULL) {
        CARDERR("may be bt source is not init!\n")
        free(btc);
        return NULL;
    }
    // force stop a2dp source,actually, bt should stop in app
    btmg_a2dp_source_play_stop(1);
    cdx_bt_state = BT_STATE_STOP;
    // reset a2dp source cb
    btc->bt_lock_data = hal_spin_lock_irqsave(&(btc->bt_lock));
    app_state_cb = old_cb->btmg_a2dp_source_cb.audio_state_cb;
    old_cb->btmg_a2dp_source_cb.audio_state_cb = cedarx_bt_state_report;
    hal_spin_unlock_irqrestore(&(btc->bt_lock), btc->bt_lock_data);

    btc_handle = btc;
    return (void *)btc;
}

static void closeSoundDevice(SoundCtrl *s)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);

    RtCtrlContext *rtContext = (RtCtrlContext *)s;

    free(rtContext);
}

static void setSoundFormat(SoundCtrl *s, CdxPlaybkCfg *cfg)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);
    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    BtCtrlContext *btc = (BtCtrlContext *)rtContext->handle;
    if ((cfg->nChannels == btc_handle->nChannelNum) &&
        (cfg->nSamplerate == btc_handle->nTargetSampleRate)) {
        CARDLOG("no need reset Format\n");
        return;
    }

    if (cdx_bt_state != BT_STATE_STOP) {
        btmg_a2dp_source_play_stop(1);
        bt_state_wait(BT_STATE_STOP);
    }

    btmg_a2dp_source_set_audio_param(cfg->nChannels, cfg->nSamplerate);
    btc_handle->nChannelNum = cfg->nChannels;
    btc_handle->nTargetSampleRate = cfg->nSamplerate;
}

static int startSoundDevice(SoundCtrl *s)
{
    int ret = 0;
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    BtCtrlContext *btc = (BtCtrlContext *)rtContext->handle;

    if (cdx_bt_state == BT_STATE_START) {
        CARDLOG("Sound device already start.\n");
    } else if ((cdx_bt_state == BT_STATE_STOP) || (cdx_bt_state == STATUS_PAUSE)) {
        btmg_a2dp_source_play_start();
        bt_state_wait(BT_STATE_START);
    } else {}

    return ret;
}

static int stopSoundDevice(SoundCtrl *s)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);
    // cedarx no stop bt again
    // RtCtrlContext *rtContext = (RtCtrlContext *)s;

    // BtCtrlContext *btc = (BtCtrlContext *)rtContext->handle;

    // if (cdx_bt_state == BT_STATE_STOP) {
    // 	CARDLOG("Sound device already stopped.\n");

    // } else {
    // 	btmg_a2dp_source_play_stop(0);

    // 	bt_state_wait(BT_STATE_STOP);
    // }
    return 0;
}

static int pauseSoundDevice(SoundCtrl *s)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);

    RtCtrlContext *rtContext = (RtCtrlContext *)s;
    BtCtrlContext *btc = (BtCtrlContext *)rtContext->handle;

    if (cdx_bt_state == BT_STATE_START) {
        btmg_a2dp_source_play_stop(1);
        bt_state_wait(BT_STATE_STOP);
    } else {
        CARDLOG("RTSoundDevicePause(): pause in an invalid status,status = %d\n", cdx_bt_state);
    }
    return 0;
}

static int flushSoundDevice(SoundCtrl *s, void *block)
{
    CARDLOG("%s : %d\n", __FUNCTION__, __LINE__);
    // cedarx no stop bt again
    // btmg_a2dp_source_play_stop(0);

    return 0;
}

static int writeSoundDevice(SoundCtrl *s, void *pData, int nDataSize)
{
    int res = 0;
    btmg_a2dp_source_send_data(pData, nDataSize);

    return nDataSize;
}

static int getSoundeDeviceCache(SoundCtrl *s)
{
    return 0;
}

static int setSoundDeviceVol(unsigned *vol)
{
    unsigned btvol = (unsigned)(*vol);
    return btmg_a2dp_source_set_volume(btvol);
}

static int getSoundDeviceVol(unsigned *vol)
{
    return btmg_avrc_get_absolute_volume(vol);
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
    //to do
    CARDLOG("\n");
    return 0;
}

static int GetSoundDeviceFrameCount(SoundCtrl *s)
{
    //to do
    CARDLOG("\n");
    return 0;
}

const SoundControlOpsT BtOps = {
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

int CedarxRenderRegisterBt(void)
{
    return CedarxRenderRegister(openBtDevice, &BtOps, "Bt");
}
#endif