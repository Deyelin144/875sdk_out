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

#include "console.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "hal_time.h"
#include "hal_sem.h"
#include "hal_queue.h"
#include "sunxi_hal_timer.h"
#include <AudioSystem.h>
#include "hal_timer.h"
#include "cdx_interface/cdx_video_rendor.h"
#include "cdx_interface/cdx_video_dec.h"
#include "video_core/video_fsm.h"
#include "xplayer.h"

static video_callback g_customer_cb = NULL;
static unsigned g_rotate = 0;
static unsigned g_autosize = 0;
static unsigned g_video_create = 0;
extern int Mp4VideoRotate(unsigned char rotate);

static void cdx_video_test_usage()
{
    printf("Usgae: cdx_video_test [option]\n");
    printf("-h,          cdx_video_test help\n");
    printf("-p,          play\n");
    printf("-s,          seek, msec\n");
    printf("-d,          duration, msec\n");
    printf("-q,          stop play\n");
    printf("-T,          pause/continue\n");
    printf("-S,          current position\n");
    printf("\n");
    printf("example:\n");
    printf("cdx_video_test -p file://sdmmc/2-320x240-xvid.mp4\n");
    printf("cdx_video_test -c\n");
    printf("get total play time:\n");
    printf("cdx_video_test -d\n");
    printf("seek:\n");
    printf("cdx_video_test -s 50000\n");
    printf("\n");
}

int video_customer_callback(void *handle, video_cb_status status)
{
    printf("video_cb_status : %d\n", status);
    switch (status) {
    case MP4_STATUS_INIT_FINISH:
        printf("video init finsih,please use [cdx_video_test -c] start playing\n");
        break;
    case MP4_STATUS_START_PLAYING:
        break;
    case MP4_STATUS_PLAY_TO_PAUSE:
        break;
    case MP4_STATUS_SEEK_FINISH:
        break;
    case MP4_STATUS_STOP:
        printf("video play finish\n");
        break;
    case MP4_STATUS_ERROR:
        printf("some error happen\n");
        break;
    default:
        break;
    }
}

// only for video test,rtplayer test may be play fail
extern int XPlayer_List_Init_Flag;
void cedarx_register(void)
{
    if (XPlayer_List_Init_Flag == 0) {
        XPlayer_List_Init_Flag = 1;
        int CedarxStreamListInit(void);
        int CedarxStreamRegisterFile(void);
        int CedarxStreamRegisterHttp(void);
        int CedarxStreamRegisterTcp(void);
        int CedarxStreamRegisterCustomer(void);

        int CedarxParserListInit(void);
        int CedarxParserRegisterAAC(void);
        int CedarxParserRegisterMP3(void);
        int CedarxParserRegisterM4A(void);
        int CedarxParserRegisterM3U(void);
        int CedarxParserRegisterOGG(void);

        int CedarxDecoderListInit(void);
        int CedarxDecoderRegisterAAC(void);
        int CedarxDecoderRegisterMP3(void);
        int CedarxDecoderRegisterWAV(void);

        CedarxStreamListInit();
        CedarxStreamRegisterFile();
        CedarxStreamRegisterHttp();
        CedarxStreamRegisterTcp();
        CedarxStreamRegisterCustomer();

        CedarxParserListInit();
        CedarxParserRegisterAAC();
        CedarxParserRegisterMP3();
        CedarxParserRegisterM4A();
        CedarxParserRegisterM3U();
        CedarxParserRegisterOGG();

        CedarxDecoderListInit();
        CedarxDecoderRegisterAAC();
        CedarxDecoderRegisterMP3();
        CedarxDecoderRegisterWAV();

#ifdef CONFIG_LIB_MULTIMEDIA_CROP
        int CedarxRenderListInit(void);
        int CedarxRenderRegisterAudioSystem(void);

        CedarxRenderListInit();
        CedarxRenderRegisterAudioSystem();
#endif
    }
}

int cedarx_video_test(int argc, char *argv[])
{
    int c = 0;
    optind = 0;
    unsigned p_msc = 0;
    unsigned t_msc = 0;
    unsigned seek_msc = 0;

    if (g_customer_cb == NULL) {
        g_customer_cb = video_customer_callback;
        video_cb_register(g_customer_cb, NULL);
    }

    HttpStreamBufferConfig httpConfig;
    memset(&httpConfig, 0, sizeof(HttpStreamBufferConfig));
    httpConfig.maxBufferSize = (256 * 1024);
    httpConfig.thresholdSize = (64 * 1024);
    httpConfig.maxProtectAreaSize = 0;
    XPlayerSetHttpBuffer(NULL, &httpConfig);

    while ((c = getopt(argc, argv, "STracdhql:s:p:")) != -1) {
        switch (c) {
        case 'q':
            // force stop
            // it will be warn if playing already stop
            if (VideoCommandStop() < 0) {
                return -1;
            }
            destory_video_task();
            g_video_create = 0;
            printf("video play stop\n");
            break;
        case 'S':
            if (VideoCommandTell(&p_msc) < 0) {
                return -1;
            }
            printf("current position : %dms\n", p_msc);
            break;
        case 'T':
            if (VideoCommandPause() < 0) {
                return -1;
            }
            break;
        case 'c':
            if (VideoCommandStart() < 0) {
                return -1;
            }
            break;
        case 'd':
            if (VideoCommandSize(&t_msc) < 0) {
                printf("error: no media message!\n");
                return -1;
            }
            printf("total duration is %dms\n", t_msc);
            break;
        case 's':
            seek_msc = atoi(optarg);
            if (VideoCommandSeek(seek_msc) < 0) {
                printf("video seek fail!\n");
                return -1;
            }
            break;
        case 'r':
            g_rotate ^= 1;
            // 0:Vertical screen  1 : Horizontal screen
            Mp4VideoRotate(g_rotate);
            break;
        case 'a':
            g_autosize ^= 1;
            // 0:original size 1:adjust screen
            Mp4VideoAutoSize(g_autosize);
            break;
        case 'p':
            if (g_video_create == 0) {
                cedarx_register();
                if (create_video_task() < 0) {
                    printf("create video task fail\n");
                    return -1;
                }
                g_video_create = 1;
            }
            if (VideoCommandPrepare(optarg) < 0) {
                printf("video start fail\n");
                destory_video_task();
                g_video_create = 0;
                return -1;
            }
            break;
        case 'h':
            cdx_video_test_usage();
            break;
        default:
            return -1;
        }
    }

    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cedarx_video_test, cdx_video_test, video test demo);
