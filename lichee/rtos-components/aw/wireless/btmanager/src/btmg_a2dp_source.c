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

#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "xr_a2dp_api.h"
#include "xr_gap_bt_api.h"
#include "btmg_log.h"
#include "btmg_common.h"
#include "btmg_audio.h"
#include "bt_manager.h"
#include "btmg_common.h"
#include "btmg_avrc.h"

#include "kernel/os/os_time.h"
#include "speex_resampler.h"

/* sub states of BT_AV_STATE_CONNECTED */
enum {
    BT_AV_MEDIA_STATE_IDLE,
    BT_AV_MEDIA_STATE_STARTING,
    BT_AV_MEDIA_STATE_STARTED,
    BT_AV_MEDIA_STATE_SUSPEND,
    BT_AV_MEDIA_STATE_STOPPING,
    BT_AV_MEDIA_STATE_STOPPED,
};

typedef enum {
    PLAY_STOPPED,
    PLAY_STARTING,
    PLAY_STARTED,
    PLAY_STOPPING,
} PlayStatus;

static PlayStatus play_status;
static bool is_init_successed = false;
static bool is_device_connected = false;
static bool is_stopping = false;
static int s_media_state = BT_AV_MEDIA_STATE_IDLE;
static const char *s_a2d_conn_state_str[] = { "Disconnected", "Connecting", "Connected",
                                              "Disconnecting" };
static const char *s_a2d_audio_state_str[] = { "Suspended", "Stopped", "Started" };

uint8_t src_pcm_channels;
uint32_t src_pcm_sampling;
uint8_t dst_pcm_channels;
uint32_t dst_pcm_sampling;
int st_quality;
static uint32_t max_pcm_volume = 100;
static uint32_t soft_volume = 100;

SpeexResamplerState *st;
static SemaphoreHandle_t a2dp_src_mutex;

static uint8_t frame_bytes;
static int16_t *out_data = NULL;
static int16_t *chmap_buf = NULL;
/* max loop in_frames:480, out_frames: 480 * 3(16k->48k) */
static uint32_t out_frames = 480 * 3;

static StaticTimer_t a2dp_timer_data;
static TimerHandle_t a2dp_timer;

static void bt_a2dp_source_volume_scale(int16_t *buffer, int16_t len, unsigned int channels, unsigned int value);
static void a2dptsk_timer_callback(TimerHandle_t xExpiredTimer);

static int32_t bt_a2dp_source_data_cb(uint8_t *data, int32_t len)
{
    int ret = 0;

    if (len <= 0 || data == NULL) {
        return 0;
    }

#ifdef CONFIG_A2DP_USE_AUDIO_SYSTEM
    if (btmg_cb_p[CB_MINOR] && btmg_cb_p[CB_MINOR]->btmg_a2dp_source_cb.audio_data_cb) {
        return btmg_cb_p[CB_MINOR]->btmg_a2dp_source_cb.audio_data_cb(data, len);
    }

    return 0;
#else
    ret = bt_audio_read_unblock(data, len, BT_AUDIO_TYPE_A2DP_SRC);
    if (bt_check_debug_mask(EX_DBG_A2DP_SOURCE_BT_RATE)) {
        static uint64_t data_count = 0;
        int speed = 0;
        int time_ms;

        data_count += ret;
        time_ms = btmg_interval_time((void *)bt_a2dp_source_data_cb, 1000 * 1);
        if (time_ms) {
            speed = data_count * 1000 / time_ms;
            BTMG_INFO("time_ms[%d] cache[%d] speed[%d]",
                       time_ms, bt_audio_get_cache(BT_AUDIO_TYPE_A2DP_SRC), speed);
            data_count = 0;
        }
    }

    if (ret < 0) {
        int i;
        for (i = 0; i < len; i++) {
            data[i] = 0;
        }
        ret = len;
     }

    return ret;
#endif
}

static void bt_a2dp_source_cb(xr_a2d_cb_event_t event, xr_a2d_cb_param_t *param)
{
    dev_node_t *dev_node = NULL;
    int ret = -1;

    switch (event) {
    case XR_A2D_CONNECTION_STATE_EVT: {
        bda2str(param->conn_stat.remote_bda, bda_str);
        BTMG_INFO("a2dp source %s, dev:%s", s_a2d_conn_state_str[param->conn_stat.state], bda_str);
        if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_CONNECTED) {
            is_device_connected = true;
            dev_node = btmg_dev_list_find_device(connected_devices, bda_str);
            if (dev_node == NULL) {
                btmg_dev_list_add_device(connected_devices, NULL, bda_str, A2DP_SRC_DEV);
            } else {
                dev_node->profile |= A2DP_SRC_DEV;
            }
            BTMG_DEBUG("add device %s into connected_devices", bda_str);
            if ((ret = xr_bt_gap_set_role(param->conn_stat.remote_bda, XR_BT_ROLE_MASTER)) != XR_OK) {
                BTMG_WARNG("set role failed: %d", ret);
            }
        }

        if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_DISCONNECTED) {
            is_device_connected = false;
            dev_node = btmg_dev_list_find_device(connected_devices, bda_str);
            if (dev_node != NULL) {
                BTMG_DEBUG("remove device %s from connected_devices", bda_str);
                dev_node->profile &= ~A2DP_SRC_DEV;
                if (connected_devices->sem_flag) {
#ifdef DEINIT_DEV_DISCONECT
                    XR_OS_SemaphoreRelease(&(connected_devices->sem));
#endif
                } else {
                    btmg_dev_list_remove_device(connected_devices, bda_str);
                }
            }
        }

        if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.conn_state_cb) {
            if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_DISCONNECTED) {
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.conn_state_cb(bda_str,
                                                             BTMG_A2DP_SOURCE_DISCONNECTED);
               if (play_status == PLAY_STARTING || play_status == PLAY_STOPPING) {
                   BTMG_WARNG("The status may not switch, and it is set proactively");
                   play_status = PLAY_STOPPED;
               }
            } else if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_CONNECTING) {
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.conn_state_cb(bda_str, BTMG_A2DP_SOURCE_CONNECTING);
            } else if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_CONNECTED) {
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.conn_state_cb(bda_str, BTMG_A2DP_SOURCE_CONNECTED);
            } else if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_DISCONNECTING) {
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.conn_state_cb(bda_str,
                                                             BTMG_A2DP_SOURCE_DISCONNECTING);
            }
        }
#ifdef CONFIG_A2DP_USE_AUDIO_SYSTEM
        if (btmg_cb_p[CB_MINOR] && btmg_cb_p[CB_MINOR]->btmg_a2dp_source_cb.conn_state_cb) {
            if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_DISCONNECTED) {
                btmg_cb_p[CB_MINOR]->btmg_a2dp_source_cb.conn_state_cb(bda_str,
                                                             BTMG_A2DP_SOURCE_DISCONNECTED);
            } else if (param->conn_stat.state == XR_A2D_CONNECTION_STATE_CONNECTED) {
                btmg_cb_p[CB_MINOR]->btmg_a2dp_source_cb.conn_state_cb(bda_str, BTMG_A2DP_SOURCE_CONNECTED);
            }
        }
#endif
        break;
    }
    case XR_A2D_AUDIO_STATE_EVT: {
        bda2str(param->audio_stat.remote_bda, bda_str);
        if (btmg_cb_p[CB_MAIN] && btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.audio_state_cb) {
            if (param->audio_stat.state == XR_A2D_AUDIO_STATE_REMOTE_SUSPEND) {
                BTMG_INFO("AUDIO_SUSPENDED");
                play_status = PLAY_STOPPED;
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.audio_state_cb(bda_str, BTMG_A2DP_SOURCE_AUDIO_SUSPENDED);
            } else if (param->audio_stat.state == XR_A2D_AUDIO_STATE_STOPPED) {
                BTMG_INFO("AUDIO_STOPPED");
                play_status = PLAY_STOPPED;
                xTimerStop(a2dp_timer, 0);
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.audio_state_cb(bda_str, BTMG_A2DP_SOURCE_AUDIO_STOPPED);
            } else if (param->audio_stat.state == XR_A2D_AUDIO_STATE_STARTED) {
                BTMG_INFO("AUDIO_STARTED");
                play_status = PLAY_STARTED;
                xTimerStop(a2dp_timer, 0);
                btmg_cb_p[CB_MAIN]->btmg_a2dp_source_cb.audio_state_cb(bda_str, BTMG_A2DP_SOURCE_AUDIO_STARTED);
            }
        }
        break;
    }
    case XR_A2D_PROF_STATE_EVT:
        BTMG_DEBUG("a2dp source %s success",
                  (param->a2d_prof_stat.init_state == XR_A2D_INIT_SUCCESS) ? "init" : "deinit");
        if (param->a2d_prof_stat.init_state == XR_A2D_INIT_SUCCESS) {
            is_init_successed = true;
        }
        break;
    case XR_A2D_MEDIA_CTRL_ACK_EVT:
        BTMG_DEBUG("a2dp media ctrl ack");
        if (param->media_ctrl_stat.status != XR_A2D_MEDIA_CTRL_ACK_SUCCESS) {
            BTMG_ERROR("a2dp media ctrl ack failure, cmd: %d, status:%d",
                       param->media_ctrl_stat.cmd, param->media_ctrl_stat.status);
        }
        break;
    default:
        BTMG_ERROR("unhandled evt %d", event);
        break;
    }
}

btmg_err bt_a2dp_source_init(void)
{
    xr_err_t ret;

    is_init_successed = false;
    is_device_connected = false;
    is_stopping = false;
    play_status = PLAY_STOPPED;

    BTMG_DEBUG("");

    a2dp_src_mutex = xSemaphoreCreateMutex();

    if ((ret = xr_a2d_register_callback(&bt_a2dp_source_cb)) != XR_OK) {
        BTMG_ERROR("a2d_source_reg_cb return failed: %d", ret);
        return BT_FAIL;
    }

    if ((ret = xr_a2d_source_register_data_callback(bt_a2dp_source_data_cb)) != XR_OK) {
        BTMG_ERROR("a2d_source_reg_data_cb return failed: %d", ret);
        return BT_FAIL;
    }

    if ((ret = xr_a2d_source_init()) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    src_pcm_channels = 2;
    src_pcm_sampling = 16000;
    dst_pcm_channels = 2;
    dst_pcm_sampling = 44100;
    st_quality = 3;

    a2dp_timer = xTimerCreateStatic("a2dp_static_timer", pdMS_TO_TICKS(2500), pdTRUE, NULL,
                                    a2dptsk_timer_callback, &a2dp_timer_data);

    bt_audio_init(BT_AUDIO_TYPE_A2DP_SRC, 0);

    if (out_data == NULL) {
        out_data = calloc(out_frames, sizeof(int16_t) * dst_pcm_channels);
    }

    return BT_OK;
}

btmg_err bt_a2dp_source_deinit(void)
{
    xr_err_t ret;

    BTMG_INFO("");

    if ((ret = xr_a2d_source_deinit()) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    xTimerDelete(a2dp_timer, 0);

    bt_audio_deinit(BT_AUDIO_TYPE_A2DP_SRC);

    xSemaphoreTake(a2dp_src_mutex, portMAX_DELAY);
    if (st != NULL) {
        speex_resampler_destroy(st);
        st = NULL;
    }

    if (out_data != NULL) {
        free(out_data);
        out_data = NULL;
    }
    if (chmap_buf != NULL) {
        free(chmap_buf);
        chmap_buf = NULL;
    }

    xSemaphoreGive(a2dp_src_mutex);
    vSemaphoreDelete(a2dp_src_mutex);

    play_status = PLAY_STOPPED;

    BTMG_INFO("");

    return BT_OK;
}

btmg_err bt_a2dp_source_connect(const char *addr)
{
    xr_err_t ret;
    xr_bd_addr_t remote_bda = {0};

    str2bda(addr, remote_bda);

    if ((ret = xr_a2d_source_connect(remote_bda)) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_a2dp_source_disconnect(const char *addr)
{
    xr_err_t ret;
    xr_bd_addr_t remote_bda = {0};

    if (!is_device_connected) {
        BTMG_ERROR("");
        return BT_FAIL;
    }

    str2bda(addr, remote_bda);

    if ((ret = xr_a2d_source_disconnect(remote_bda)) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    return BT_OK;
}

btmg_err bt_a2dp_source_set_audio_param(uint8_t channels, uint32_t sampling)
{
    int err;

    src_pcm_channels = channels;
    src_pcm_sampling = sampling;
    frame_bytes = src_pcm_channels * 2;

    BTMG_DEBUG("");

    if (st != NULL) {
        speex_resampler_destroy(st);
        st = NULL;
    }

    st = speex_resampler_init_frac(src_pcm_channels,
                src_pcm_sampling, dst_pcm_sampling,
                src_pcm_sampling, dst_pcm_sampling,
                st_quality, &err);

    if (!st || err != 0) {
        BTMG_ERROR("resample fail,st:%p, ret:%d", st, err);
        return BT_FAIL;
    }

    speex_resampler_set_rate_frac(st,
            src_pcm_sampling, dst_pcm_sampling,
            src_pcm_sampling, dst_pcm_sampling);

    if (src_pcm_channels != dst_pcm_channels) {
        if (chmap_buf != NULL) {
            BTMG_DEBUG("unexpected chmap_buf");
            free(chmap_buf);
            chmap_buf = NULL;
        }
        chmap_buf = calloc(out_frames, sizeof(int16_t) * dst_pcm_channels);
        if (!chmap_buf) {
            BTMG_ERROR("no memory");
            return BT_ERR_NO_MEMORY;
        }
    }

    return BT_OK;
}

int bt_a2dp_source_send_data(uint8_t *data, int len)
{
    int ret = 0;
    int in_frames = 0, residue = 0, written = 0, ofs = 0;
    int out_data_len = 0;
    uint32_t _out_frames = out_frames;
    uint8_t *out = (uint8_t *)out_data;

    if (bt_avrc_get_peer_abs_vol_support() == false) {
        bt_a2dp_source_volume_scale((int16_t *)data, len, src_pcm_channels, soft_volume);
    }

    if (!is_device_connected) {
        BTMG_ERROR("Device disconnected, send fail!");
        XR_OS_MSleep(10);
        return 0;
    }

	if (is_stopping) {
        XR_OS_MSleep(10);
        return 0;
	}

    if (bt_check_debug_mask(EX_DBG_A2DP_SOURCE_APP_WRITE_RATE)) {
        static uint64_t data_count = 0;
        int speed = 0;
        int time_ms;

        data_count += len;
        time_ms = btmg_interval_time((void *)bt_a2dp_source_send_data, 1000 * 1);
        if (time_ms) {
            speed = data_count * 1000 / time_ms;
            BTMG_INFO("time_ms[%d] cache[%d] speed[%d]",
                       time_ms, bt_audio_get_cache(BT_AUDIO_TYPE_A2DP_SRC), speed);
            data_count = 0;
        }
    }
    residue = len / frame_bytes;
    xSemaphoreTake(a2dp_src_mutex, portMAX_DELAY);
    bool speex = false;
    while (1) {
        if (!is_device_connected || is_stopping) {
            xSemaphoreGive(a2dp_src_mutex);
            return written;
        }
        if(!speex) {
            _out_frames = out_frames;
            in_frames = residue;
            ret = speex_resampler_process_interleaved_int(st,
                            ((int16_t *)data) + ofs, &in_frames,
                            out_data, &_out_frames);
            if (ret != 0) {
                BTMG_ERROR("resample error, ret=%d", ret);
                xSemaphoreGive(a2dp_src_mutex);
                return written;
            }

        if (src_pcm_channels != dst_pcm_channels && chmap_buf != NULL) {
            int i = 0;
            for (; i < _out_frames; i++) {
                chmap_buf[2 * i] = out_data[i];
                chmap_buf[2 * i + 1] = out_data[i];
            }
            out = (uint8_t *)chmap_buf;
        }
        out_data_len = _out_frames * dst_pcm_channels * 2;
            speex = true;
        }
        ret = bt_audio_write(out, out_data_len, 200, BT_AUDIO_TYPE_A2DP_SRC);
        if (ret < 0) {
            BTMG_ERROR("bt_audio_write fail");
            XR_OS_MSleep(100);
            xSemaphoreGive(a2dp_src_mutex);
            return written;
        } else if (ret > 0) {
            residue -= in_frames;
            ofs += in_frames * src_pcm_channels;
            written += out_data_len;
            speex = false;
        } else if (ret == 0) {
            XR_OS_MSleep(10);
            if (!is_device_connected || is_stopping)
                break;
        }
        if (residue <= 0)
            break;
    }
    xSemaphoreGive(a2dp_src_mutex);

    return written;
}

static void a2dptsk_timer_callback(TimerHandle_t xExpiredTimer)
{
    BTMG_ERROR("a2dpplay -- avdtp control failed, timeout!");

    bt_a2dp_source_disconnect(bda_str);

    xTimerStop(a2dp_timer, 0);
}

btmg_err bt_a2dp_source_play_start(void)
{
    xr_err_t ret;

    BTMG_DEBUG("");
    is_stopping = false;

    if (play_status == PLAY_STARTED) {
        BTMG_WARNG("Now already started");
        return BT_ERR_ALREADY_DONE;
    }

    if (play_status == PLAY_STARTING) {
        BTMG_ERROR("NOW starting ....");
        return BT_ERR_IN_PROCESS;
    }

    if (play_status == PLAY_STOPPING) {
        BTMG_ERROR("NOW stoping ....");
        return BT_ERR_IN_PROCESS;
    }

#ifndef CONFIG_A2DP_USE_AUDIO_SYSTEM
    bt_audio_reset(BT_AUDIO_TYPE_A2DP_SRC);
#endif

    if ((ret = xr_a2d_media_ctrl(XR_A2D_MEDIA_CTRL_START)) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    play_status = PLAY_STARTING;

    xTimerStart(a2dp_timer, 10);

    return BT_OK;
}

btmg_err bt_a2dp_source_play_stop(bool drop)
{
    xr_err_t ret;

    BTMG_DEBUG("");

    is_stopping = true;

    if (play_status != PLAY_STARTED) {
        BTMG_ERROR("play_status != PLAY_STARTED");
        return BT_FAIL;
    }

#ifndef CONFIG_A2DP_USE_AUDIO_SYSTEM
    if (drop == false) {
        int flush_time = 20;
        while (flush_time > 0 || bt_audio_get_cache(BT_AUDIO_TYPE_A2DP_SRC) != 0) {
            if (!is_device_connected) {
                break;
            }
            XR_OS_MSleep(10);
            flush_time--;
        }
    }
#endif

    /*me Bluetooth headsets will lose sound when playing audio less than 1 second.
     This can be improved by delaying 250ms after sending data before sending stop..*/
    if (drop == false) {
        XR_OS_MSleep(250);
    }
    if ((ret = xr_a2d_media_ctrl(XR_A2D_MEDIA_CTRL_STOP)) != XR_OK) {
        BTMG_ERROR("return failed: %d", ret);
        return BT_FAIL;
    }

    play_status = PLAY_STOPPING;

    xTimerStart(a2dp_timer, 10);

    return BT_OK;
}

bool bt_a2dp_source_is_ready(void)
{
    if (is_init_successed && is_device_connected) {
        return true;
    }

    return false;
}

/**
 * Convert loudness to audio volume change in dB.
 *
 * @param value The volume loudness value.
 * @return This function returns the audio volume change
 *   in dB which corresponds to the given loudness value. */
double audio_loudness_to_decibel(double value)
{
    return 10 * log2(value);
}

int audio_pcm_volume_to_level(unsigned int value)
{
    /* If value is 0, it will cause a crash due to illegal calculation.
    When the value is 1, it has been tested to achieve a similar mute effect. */
    if (value == 0) {
        value = 1;
    }

    double level = audio_loudness_to_decibel(1.0 * value / max_pcm_volume);

    return MIN(MAX(level, -96.0), 96.0) * 100;
}

/**
 * Scale S16_2LE PCM signal.
 *
 * Neutral value for scaling factor is 1.0. It is possible to increase
 * signal gain by using scaling factor values greater than 1, however,
 * clipping will most certainly occur.
 *
 * @param buffer Address to the buffer where the PCM signal is stored.
 * @param channels The number of channels in the buffer.
 * @param frames The number of PCM frames in the buffer.
 * @param ch1 The scaling factor for 1st channel.
 * @param ch1 The scaling factor for 2nd channel. */
void audio_scale_s16_2le(int16_t *buffer, int channels, size_t frames, double ch1, double ch2) {
    switch (channels) {
    case 1:
        if (ch1 != 0 && ch1 != 1)
            while (frames--)
                buffer[frames] *= ch1;
        break;
    case 2:
        if ((ch1 != 0 && ch1 != 1) || (ch2 != 0 && ch2 != 1))
            while (frames--) {
                buffer[2 * frames] *= ch1;
                buffer[2 * frames + 1] *= ch2;
            }
        break;
    default:
        BTMG_ERROR("not support channels");
    }
}

static void bt_a2dp_source_volume_scale(int16_t *buffer, int16_t len, unsigned int channels, unsigned int value)
{
    double ch1_scale = 0;
    double ch2_scale = 0;
    double level;

    size_t frames = len / channels / 2;
    level = audio_pcm_volume_to_level(value);

    /* scaling based on the decibel formula pow(10, dB / 20) */
    ch1_scale = pow(10, (0.01 * level) / 20);
    ch2_scale = ch1_scale;

    audio_scale_s16_2le(buffer, channels, frames, ch1_scale, ch2_scale);
}

btmg_err bt_a2dp_source_set_volume(uint32_t volume)
{
    if (bt_avrc_get_peer_abs_vol_support()) {
        return bt_avrc_set_absolute_volume(volume);
    } else {
        soft_volume = volume;
    }

    return BT_OK;
}
