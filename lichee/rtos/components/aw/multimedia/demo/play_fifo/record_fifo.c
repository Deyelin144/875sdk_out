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
#include "ringbuffer.h"
#include "xrecord.h"
// #include "rtosCaptureControl.h"
#include "cedarx.h"

#define PLAYFIFO_DEFAULT_PRIORITY HAL_THREAD_PRIORITY_APP
#define PLAYFIFO_BUF_SIZE         1024

typedef struct {
    RTPlayer *fifo_player;
    struct AudioFifoS *audiofifo;
    char fifo_buff[PLAYFIFO_BUF_SIZE];
} _fifo_player;

typedef struct
{
    pthread_t play_task_threadID;
    hal_ringbuffer_t record_rb;
    XRecord *xrecorder;

    _fifo_player *xrplayer;
} _record_fifo;

static _record_fifo *record_fifo;
extern int hal_msleep(unsigned int msecs);
extern void* RTCaptureDeviceCreate();
int g_recorder_register = 0;

static void *play_task(void *arg)
{
    unsigned int act_read;

    struct AudioFifoS *audiofifo = record_fifo->xrplayer->audiofifo;
    char *fifo_buff = record_fifo->xrplayer->fifo_buff;
    hal_ringbuffer_t record_rb = record_fifo->record_rb;
    
    AudioFifoStart(audiofifo);
    
    while (1) {
        act_read = hal_ringbuffer_get(record_rb, fifo_buff, PLAYFIFO_BUF_SIZE, -1);
        if (act_read <= 0) {
            printf("some err happen\n");
            while(1) {
                hal_msleep(50000);
            }
        }
        
        AudioFifoPutData(audiofifo, fifo_buff, act_read);
    }
}

static _fifo_player *fifo_player_init(void)
{
    _fifo_player *xrplayer = malloc(sizeof(_fifo_player));
    if (xrplayer == NULL)
        return NULL;
    memset(xrplayer, 0, sizeof(_fifo_player));

    xrplayer->fifo_player = player_init();
    if (xrplayer->fifo_player == NULL) {
        free(xrplayer);
        return NULL;
    }

    xrplayer->audiofifo = audio_fifo_create();
    if (xrplayer->audiofifo == NULL) {
        player_deinit(xrplayer->fifo_player);
        free(xrplayer);
        return NULL;
    }

    AudioFifoSetPlayer(xrplayer->audiofifo, xrplayer->fifo_player);
    return xrplayer;
}

static void fifo_player_deinit(_fifo_player *xrplayer)
{
    AudioFifoStop(xrplayer->audiofifo, 1);
    audio_fifo_destroy(xrplayer->audiofifo);
    player_deinit(xrplayer->fifo_player);
    free(xrplayer);
}

static XRecord *xrrecord_init(void)
{
    XRecord *xrecorder = NULL;
    CaptureCtrl *cap = NULL;

#ifdef CONFIG_LIB_MULTIMEDIA_CROP
    /* After calling once, there is no need to call it again */
    if (g_recorder_register == 0) {
        g_recorder_register = 1;
        CedarxWriterListInit();
        CedarxWriterRegisterCallback();

        CedarxMuxerListInit();
        CedarxMuxerRegisterAmr();
        CedarxMuxerRegisterPcm();
        CedarxMuxerRegisterMp3();

        CedarxEncoderListInit();
        CedarxEncoderRegisterAmr();
        CedarxEncoderRegisterPcm();
        CedarxEncoderRegisterMp3();
        CedarxEncoderRegisterAac();
    }
#endif

    xrecorder = XRecordCreate();
    if (xrecorder == NULL)
        return NULL;

    cap = (void *)(uintptr_t)RTCaptureDeviceCreate();
    if (cap == NULL) {
        XRecordDestroy(xrecorder);
        return NULL;
    }

    XRecordSetAudioCap(xrecorder, cap);
    return xrecorder;
}

static void xrrecord_deinit(XRecord *xrecorder)
{
    XRecordStop(xrecorder);
    XRecordDestroy(xrecorder);
}

static int record_fifo_init(void)
{
    XRecordConfig audioConfig;
    hal_ringbuffer_t record_rb = NULL;
    XRecord *xrecorder = NULL;
    _fifo_player *xrplayer = NULL;

    record_fifo = malloc(sizeof(_record_fifo));
    if (record_fifo == NULL)
        return -1;
    memset(record_fifo, 0, sizeof(_record_fifo));
    
    record_rb = hal_ringbuffer_init(PLAYFIFO_BUF_SIZE * 10);
    if (record_rb == NULL)
        goto record_fifo_init_err;
    
    xrecorder = xrrecord_init();
    if (xrecorder == NULL)
        goto record_fifo_init_err;
    
    xrplayer = fifo_player_init();
    if (xrplayer == NULL)
        goto record_fifo_init_err;
    
    record_fifo->xrecorder = xrecorder;
    record_fifo->record_rb = record_rb;
    record_fifo->xrplayer = xrplayer;

    return 0;

record_fifo_init_err:
    if (xrecorder != NULL)
        xrrecord_deinit(xrecorder);
    if (record_rb != NULL)
        hal_ringbuffer_release(record_rb);
    if (xrplayer != NULL)
        fifo_player_deinit(xrplayer);

    free(record_fifo);
    record_fifo = NULL;
}

static void record_fifo_deinit(void)
{
    if (record_fifo == NULL)
        return;

    if (record_fifo->play_task_threadID != NULL)
        pthread_cancel(record_fifo->play_task_threadID);

    xrrecord_deinit(record_fifo->xrecorder);
    fifo_player_deinit(record_fifo->xrplayer);
    hal_ringbuffer_release(record_fifo->record_rb);

    free(record_fifo);
    record_fifo = NULL;
}

static unsigned recorder_cb(void *buf, uint32_t size)
{
    int ret = 0;

    if ((record_fifo == NULL) || (record_fifo->record_rb == NULL))
        return 0;

    ret = hal_ringbuffer_put(record_fifo->record_rb, buf, size);
    if (ret != size) {
        printf("record_rb is full, may be not big enough\n");
    }

    return ret;
}

static int recorder_start(char *record_type)
{
    XRecordConfig audioConfig;
    XRECODER_AUDIO_ENCODE_TYPE type;
    char cb_url[64] = {0};

    if (strcmp("amr", record_type) == 0) {
        audioConfig.nChan = 1;
        audioConfig.nSamplerate = 8000;
        audioConfig.nSamplerBits = 16;
        audioConfig.nBitrate = 12200;
        type = XRECODER_AUDIO_ENCODE_AMR_TYPE;
    } else if (strcmp("mp3", record_type) == 0) {
        audioConfig.nChan = 1;
        audioConfig.nSamplerate = 16000;
        audioConfig.nSamplerBits = 16;
        audioConfig.nBitrate = 32000;
        type = XRECODER_AUDIO_ENCODE_MP3_TYPE;
    } else if (strcmp("aac", record_type) == 0) {
        audioConfig.nChan = 2;
        audioConfig.nSamplerate = 16000;
        audioConfig.nSamplerBits = 16;
        audioConfig.nBitrate = 64000;
        type = XRECODER_AUDIO_ENCODE_AAC_TYPE;
        
    } else {
        printf("not support format %s\n", record_type);
        return -1;
    }
    
    snprintf(cb_url, 64, "callback://%p", recorder_cb);
    
    XRecordSetDataDstUrl(record_fifo->xrecorder, cb_url, NULL, NULL);
    
    XRecordSetAudioEncodeType(record_fifo->xrecorder, type, &audioConfig);
    
    XRecordPrepare(record_fifo->xrecorder);
    
    XRecordStart(record_fifo->xrecorder);
    
    return 0;
}

static int record_fifo_start(char *record_type)
{
    pthread_attr_t attr;
    struct sched_param sched;

    if (record_fifo == NULL)
        return -1;
    if (recorder_start(record_type) < 0)
        return -1;
    
    pthread_attr_init(&attr);
    sched.sched_priority = PLAYFIFO_DEFAULT_PRIORITY;
    pthread_attr_setschedparam(&attr, &sched);
    pthread_attr_setstacksize(&attr, 2048);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    if (pthread_create(&(record_fifo->play_task_threadID), &attr, play_task, record_fifo)) {
        LOGE("play task create failed. exit!");
        return -1;
    }
    
    pthread_setname_np(record_fifo->play_task_threadID, "PlayTaskThread");
    

    
    return 0;
}

extern void start_malloc_trace(void);
void finish_malloc_trace_and_show(void);

static int cmd_record_fifo(int argc, char **argv)
{
    char record_type[5] = {0};
    int c = -1;
    /* create the play task Thread  */
    while ((c = getopt(argc, argv, "r:p:q")) != -1) {
        switch (c) {
        case 'p':
            strncpy(record_type, optarg, 4);
            break;
        case 'q':
            record_fifo_deinit();
            return 0;
        case 'r':
            record_fifo_deinit();
            strncpy(record_type, optarg, 4);
            hal_msleep(100); // wait resource release
            break;
        default:
            printf("Usgae: record_fifo [option]\n");
            printf("-p,          target record format\n");
            printf("-q,          force stop\n");
            printf("-r,          reset to other format\n");
            printf("\n");
            printf("example:\n");
            printf("record_fifo -p aac\n");
            printf("\n");
            return -1;
        }
    }

    if (record_fifo_init() < 0) {
        printf("init recorder fail\n");
        return -1;
    }
    printf("%s, %d, record_type = %s\n", __func__, __LINE__, record_type);
    if (record_fifo_start(record_type) < 0) {
        record_fifo_deinit();
    }
    
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_record_fifo, record_fifo, record_fifo_demo);