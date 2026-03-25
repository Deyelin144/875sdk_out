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

#ifndef __FSM_MP4_H__
#define __FSM_MP4_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define DEFAULT_VIDEO_MESSAGE_TIMEOUT   500

typedef enum {
    MP4_STATUS_INIT_FINISH,
    MP4_STATUS_START_PLAYING,
    MP4_STATUS_PLAY_TO_PAUSE,
    MP4_STATUS_SEEK_FINISH,
    MP4_STATUS_STOP,
    MP4_STATUS_PLAY_FINISH,
    MP4_STATUS_ERROR,
} video_cb_status;

typedef int (*video_callback)(void *handle, video_cb_status status);

int video_cb_register(video_callback cb, void *handle);

void video_cb_unregister(void);

int create_video_task(void);

int VideoCommandPrepare(char *url);

int VideoCommandStart(void);

int VideoCommandContinue(void);

int VideoCommandTell(unsigned *msc);

int VideoCommandSize(unsigned *msc);

int VideoCommandSeek(unsigned msc);

int VideoCommandPause(void);

int VideoCommandStop(void);

int VideoCommandRotate(unsigned char rotate);

int VideoCommandSetWin(unsigned char set, unsigned char autos, int x, int y, int w, int h);

int destory_video_task(void);

#ifdef __cplusplus
}
#endif

#endif
