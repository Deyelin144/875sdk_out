/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY��S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS��SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY��S TECHNOLOGY.
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
#include <console.h>
#include <stdlib.h>
#include "FreeRTOS_POSIX/utils.h"
#include "rtplayer.h"
#include "audiofifo.h"
#include "fifolog.h"

#define PLAYFIFO_DEFAULT_PRIORITY 17
#define PLAYFIFO_MAX_FILE_LEN     64
#define PLAYFIFO_BUF_SIZE         1024

pthread_t play_task_threadID;
char file_url[PLAYFIFO_MAX_FILE_LEN];
unsigned g_loop_cnt = 0;
unsigned g_force_stop = 0;

static void *play_task(void *arg)
{
    FILE *fp;
    int ret = 0;
    int i = 0;
    char *file_buffer;
    struct AudioFifoS *audiofifo;
    RTPlayer *fifo_player;
    unsigned int act_read;

FIFO_RESTART:
    // 1. create a music player
    fifo_player = player_init();
    if (fifo_player == NULL) {
        LOGE("error: player_create fail");
        return NULL;
    }

    // 2. create a fifo handle
    audiofifo = audio_fifo_create();
    if (audiofifo == NULL) {
        LOGE("Error:audio_fifo_create failed\n");
        player_deinit(fifo_player);
        return NULL;
    }

    // 3. associate player and fifo handle
    AudioFifoSetPlayer(audiofifo, fifo_player);

    // 4. fifo start
    AudioFifoStart(audiofifo);

    file_buffer = (char *)malloc(PLAYFIFO_BUF_SIZE);
    if (file_buffer == NULL) {
        LOGE("file buffer malloc fail");
        ret = -1;
        goto err1;
    }

    fp = fopen(file_url, "rb+");
    if (fp == NULL) {
        LOGE("error:file : %s fopen failed", file_url);
        ret = -1;
        goto err2;
    }

    i = 0;
    while (1) {
        act_read = fread(file_buffer, 1, PLAYFIFO_BUF_SIZE, fp);
        // 5. put data to fifo, it will play automatically
        AudioFifoPutData(audiofifo, file_buffer, act_read);
        i++;
        if (act_read != PLAYFIFO_BUF_SIZE) {
            LOGI("fread_cnt:%d, act_read:%d", i, act_read);
            break;
        }

        if (g_force_stop == 1) {
            break;
        }
    }

    // 6. stop play
    AudioFifoStop(audiofifo, g_force_stop);
    fclose(fp);
err2:
    free(file_buffer);
err1:
    // 7. destroy fifo handle
    audio_fifo_destroy(audiofifo);
    // 8. destroy plauer
    player_deinit(fifo_player);

    // for test
    if (g_loop_cnt > 1) {
        g_loop_cnt--;
        goto FIFO_RESTART;
    }

    pthread_exit(NULL);
}

static int cmd_play_fifo(int argc, char **argv)
{
    int c = -1;
    /* create the play task Thread  */
    while ((c = getopt(argc, argv, "l:p:q")) != -1) {
        switch (c) {
        case 'l':
            g_loop_cnt = atoi(optarg);
            if (g_loop_cnt < 0) {
                g_loop_cnt = 0x7fffffff;
            }
            break;
        case 'p':
            memcpy(file_url, optarg, PLAYFIFO_MAX_FILE_LEN);
            break;
        case 'q':
            g_loop_cnt = 0;
            g_force_stop = 1;
            return 0;
        default:
            printf("Usgae: play_fifo [option]\n");
            printf("-p,          target file path\n");
            printf("-l,          loop count\n");
            printf("-q,          force stop\n");
            printf("\n");
            printf("example:\n");
            printf("play_fifo -l 3 -p data/boot.mp3\n");
            printf("\n");
            return -1;
        }
    }

    g_force_stop = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param sched;
    sched.sched_priority = PLAYFIFO_DEFAULT_PRIORITY;
    pthread_attr_setschedparam(&attr, &sched);
    pthread_attr_setstacksize(&attr, 2048);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&play_task_threadID, &attr, play_task, NULL)) {
        LOGE("play task create failed. exit!");
        return -1;
    }
    pthread_setname_np(play_task_threadID, "PlayTaskThread");

    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_play_fifo, play_fifo, play_fifo_demo);