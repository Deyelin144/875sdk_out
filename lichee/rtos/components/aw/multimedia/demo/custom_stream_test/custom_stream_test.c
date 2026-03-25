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
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>
#include <string.h>
#include <console.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <stdlib.h>
#include <hal_time.h>
#include "FreeRTOS_POSIX/utils.h"
#include "rtplayer.h"
#include "xplayer.h"

extern unsigned char test_mp3[122760];
extern int CedarxStreamRegisterPsram(void);
extern void CustomerStreamDestory(CdxStreamT *stream);
extern cdx_int32 CustomerStreamSetUrl(CdxStreamT *stream, char *url);
extern CdxStreamT *CustomerStreamCreate(void);

static void *CTplayer = NULL;
static int first_init = 0;
static int play_finish = 0;

static void callbackCTplayer(void *userData, int msg, int id, int ext1, int ext2)
{
    switch (msg) {
    case RTPLAYER_NOTIFY_PREPARED: {
        printf("RTPLAYER_NOTIFY_PREPARED:has prepared.\n");
        break;
    }
    case RTPLAYER_NOTIFY_PLAYBACK_COMPLETE: {
        printf("RTPLAYER_NOTIFY_PLAYBACK_COMPLETE:play complete\n");
        play_finish = 1;
        break;
    }
    case RTPLAYER_NOTIFY_SEEK_COMPLETE: {
        printf("RTPLAYER_NOTIFY_SEEK_COMPLETE:seek ok\n");
        break;
    }
    case RTPLAYER_NOTIFY_MEDIA_ERROR: {
        switch (ext1) {
        case RTPLAYER_MEDIA_ERROR_UNKNOWN: {
            printf("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN\n");
            break;
        }
        case RTPLAYER_MEDIA_ERROR_UNSUPPORTED: {
            printf("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED\n");
            break;
        }
        case RTPLAYER_MEDIA_ERROR_IO: {
            printf("erro type:TPLAYER_MEDIA_ERROR_IO\n");
            break;
        }
        }
        printf("RTPLAYER_NOTIFY_MEDIA_ERROR\n");
        break;
    }
    case RTPLAYER_NOTIFY_NOT_SEEKABLE: {
        printf("info: media source is unseekable.\n");
        break;
    }
    case RTPLAYER_NOTIFY_DETAIL_INFO: {
        //printf("detail info: %d\n", flag);
        break;
    }
    default: {
        printf("warning: unknown callback from RTplayer.\n");
        break;
    }
    }
}

int parta_player_test(int argc, char *argv[])
{
    if (CTplayer == NULL) {
        printf("aplayer is not init\n");
        return -1;
    }
    int c = 0;
    optind = 0;

    while ((c = getopt(argc, argv, "asTp:")) != -1) {
        switch (c) {
        case 'p':
            setDataSource_url(CTplayer, NULL, optarg, 0);
            prepare(CTplayer);
            start(CTplayer);
            break;
        case 'T':
            pause_l(CTplayer);
            break;
        case 'a':
            start(CTplayer);
            break;
        case 's':
            reset(CTplayer);
            break;
        case 'h':
            break;
        default:
            return -1;
        }
    }

    return 0;
}

int custom_player_test(int argc, char *argv[])
{
    int c = 0;
    optind = 0;
    unsigned p_msc = 0;
    unsigned t_msc = 0;
    unsigned seek_msc = 0;
    char music_url[64];

    while ((c = getopt(argc, argv, "cp")) != -1) {
        switch (c) {
        case 'p':
            CTplayer = player_init();
            if (first_init == 0) {
                first_init = 1;
                CedarxStreamRegisterPsram();
            }
            if (CTplayer == NULL) {
                printf("CTplayer create fail!\n");
                return -1;
            }
            registerCallback(CTplayer, NULL, callbackCTplayer);
            sprintf(music_url, "psram://addr=%d&length=%d", (int)(uintptr_t)(test_mp3),
                    sizeof(test_mp3));
            setDataSource_url(CTplayer, NULL, music_url, 0);
            prepare(CTplayer);
            play_finish = 0;
            start(CTplayer);
            while (play_finish == 0) {
                hal_msleep(10);
            }
            player_deinit(CTplayer);
            break;
        case 'c':
            CTplayer = player_init();
            if (CTplayer == NULL) {
                printf("CTplayer create fail!\n");
                return -1;
            }

            CdxStreamT *test_stream = CustomerStreamCreate();
            if (test_stream == NULL) {
                printf("test_stream create fail!\n");
                player_deinit(CTplayer);
                return -1;
            }

            registerCallback(CTplayer, NULL, callbackCTplayer);
            CustomerStreamSetUrl(test_stream, "file://data/boot.mp3");
            sprintf(music_url, "customer://%p", test_stream);
            setDataSource_url(CTplayer, NULL, music_url, 0);

            prepare(CTplayer);
            play_finish = 0;
            start(CTplayer);
            while (play_finish == 0) {
                hal_msleep(10);
            }

            player_deinit(CTplayer);
            break;
        case 'h':
            break;
        default:
            return -1;
        }
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(custom_player_test, ct_player, custom stream player test);