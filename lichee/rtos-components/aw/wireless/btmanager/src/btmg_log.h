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

#ifndef _BTMG_LOG_H_
#define _BTMG_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef enum {
    MSG_NONE = 0,
    MSG_ERROR,
    MSG_WARNING,
    MSG_INFO,
    MSG_DEBUG,
    MSG_MSGDUMP,
} bt_log_level_t;

enum ex_debug_mask {
    EX_DBG_A2DP_SINK_BT_RATE = 1 << 0,
    EX_DBG_A2DP_SINK_AUDIO_WRITE_RATE = 1 << 1,
    EX_DBG_A2DP_SOURCE_APP_WRITE_RATE = 1 << 2,
    EX_DBG_A2DP_SOURCE_BT_RATE = 1 << 3,
    EX_DBG_MASK_MAX = 1 << 4,
};


extern bt_log_level_t log_level;

#define BTMG_MSGDUMP(fmt, ...)                                                                     \
    {                                                                                              \
        if (log_level >= MSG_MSGDUMP)                                                              \
            printf("BTMGM[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);                   \
    }

#define BTMG_DEBUG(fmt, ...)                                                                       \
    {                                                                                              \
        if (log_level >= MSG_DEBUG)                                                                \
            printf("BTMGD[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);                   \
    }

#define BTMG_INFO(fmt, ...)                                                                        \
    {                                                                                              \
        if (log_level >= MSG_INFO)                                                                 \
            printf("BTMGI[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);                   \
    }

#define BTMG_WARNG(fmt, ...)                                                                       \
    {                                                                                              \
        if (log_level >= MSG_WARNING)                                                              \
            printf("BTMGW[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);                   \
    }

#define BTMG_ERROR(fmt, ...)                                                                       \
    {                                                                                              \
        if (log_level >= MSG_ERROR)                                                                \
            printf("BTMGE[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);                   \
    }

int bt_set_debug_level(bt_log_level_t level);
bt_log_level_t bt_get_debug_level(void);
int  bt_get_debug_mask(void);
bt_log_level_t bt_set_debug_mask(int mask);
int  bt_check_debug_mask(int mask);


#if __cplusplus
}; // extern "C"
#endif

#endif
