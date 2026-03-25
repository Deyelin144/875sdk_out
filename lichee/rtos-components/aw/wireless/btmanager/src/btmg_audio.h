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

#ifndef _BTMG_AUDIO_H_
#define _BTMG_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_ctrl/sys_ctrl.h"

typedef enum {
    BT_AUDIO_EVENT_A2DP_SNK_START = 0,
    BT_AUDIO_EVENT_A2DP_SNK_STOP,
    BT_AUDIO_EVENT_HFP_START,
    BT_AUDIO_EVENT_HFP_STOP,
} bt_app_audio_events;

typedef enum {
    BT_AUDIO_TYPE_NONE = 0,
    BT_AUDIO_TYPE_A2DP_SNK,
    BT_AUDIO_TYPE_A2DP_SRC,
    BT_AUDIO_TYPE_HFP
} bt_audio_type_t;

typedef enum {
    IO_TYPE_RB = 1, /* I/O through ringbuffer */
    IO_TYPE_CB,     /* I/O through callback */
} io_type_t;

int bt_audio_init(bt_audio_type_t type, uint16_t cache_time);
int bt_audio_deinit(bt_audio_type_t type);
int bt_audio_write(uint8_t *data, uint32_t len, uint32_t timeout, bt_audio_type_t type);
int bt_audio_read(uint8_t *data, uint32_t len, uint32_t timeout, bt_audio_type_t type);
int bt_audio_write_unblock(uint8_t *data, uint32_t len, bt_audio_type_t type);
int bt_audio_read_unblock(uint8_t *data, uint32_t len, bt_audio_type_t type);
int bt_audio_reset(bt_audio_type_t type);
int bt_audio_get_cache(bt_audio_type_t type);
void bt_audio_event_handle(event_msg *msg);
void bt_audio_pcm_config(uint32_t samplerate, uint8_t channels, bt_audio_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* _BTMG_AUDIO_H_ */
