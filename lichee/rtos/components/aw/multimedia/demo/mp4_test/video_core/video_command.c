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

#include "video_core/video_message.h"
#include "../cdx_interface/cdx_video_rendor.h"
#include <stdlib.h>
#include <string.h>

#define VIDEO_MAX_URI_LENGTH              512

int VideoCommandPrepare(char *url)
{
    if (url == NULL) {
        return -1;
    }

    video_message msg = {0};
    msg.command = VIDEO_COMMAND_PREPARE;
    msg.pri = malloc(VIDEO_MAX_URI_LENGTH);
    if (msg.pri == NULL) {
        return -1;
    }
    memset(msg.pri, 0, VIDEO_MAX_URI_LENGTH);
    strncpy(msg.pri, url, VIDEO_MAX_URI_LENGTH);

    int ret = video_command_message_send(&msg, 1);
    free(msg.pri);
    return ret;
}

int VideoCommandContinue(void)
{
    video_message msg = {0};
    msg.command = VIDEO_COMMAND_CONTINUE;

    return video_command_message_send(&msg, 0);
}

int VideoCommandStart(void)
{
    video_message msg = {0};
    msg.command = VIDEO_COMMAND_CONTINUE;

    return video_command_message_send(&msg, 0);
}

int VideoCommandStop(void)
{
    video_message msg = {0};
    msg.command = VIDEO_COMMAND_STOP;

    return video_command_message_send(&msg, 0);
}

// int VideoCommandTell(unsigned *msc)
// {
//     // if ((g_avtimer == NULL) || (g_cpt == NULL)) {
//     //     return -1;
//     // }

//     // VideoInfo *mpi = g_cpt->tmpCsrParser->mpi;
//     // *msc = (g_avtimer->video_point * (1000 / mpi->Timescale));
//     return 0;
// }

// int VideoCommandSize(unsigned *msc)
// {
//     // if (g_cpt == NULL) {
//     //     return -1;
//     // }

//     // VideoInfo *mpi = g_cpt->tmpCsrParser->mpi;
//     // *msc = mpi->Vduration;
//     return 0;
// }

int VideoCommandPause(void)
{
    video_message msg = {0};
    msg.command = VIDEO_COMMAND_PAUSE;

    return video_command_message_send(&msg, 0);
}

int VideoCommandSeek(unsigned msc)
{
    unsigned *seek_msc= NULL;

    video_message msg = {0};
    msg.command = VIDEO_COMMAND_SEEK;
    seek_msc = malloc(sizeof(unsigned));
    if (seek_msc == NULL) {
        return -1;
    }

    seek_msc[0] = msc;
    msg.pri = seek_msc;

    int ret = video_command_message_send(&msg, 1);
    free(seek_msc);
    return ret;
}

int VideoCommandRotate(unsigned char rotate)
{
    return Mp4VideoRotate(rotate);
}

int VideoCommandSetWin(unsigned char set, unsigned char autos, int x, int y, int w, int h)
{
    int ret = -1;

    if (set) {
        ret = Mp4VideoAutoSize(0);
    } else if (autos) {
        ret = Mp4VideoAutoSize(1);
    }

    ret = Mp4VideoSetWin(set, x, y, w, h);

exit:
    return ret;
}