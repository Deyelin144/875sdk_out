#include <stdlib.h>
#include "kfifoqueue.h"
#include "audiofifo.h"
#include "rtplayer.h"
#include "fifolog.h"
#include <semaphore.h>

#define FIFO_START_PRIORITY ((configMAX_PRIORITIES >> 1))

typedef enum audio_status {
    AUDIO_STATUS_PREPARE,
    AUDIO_STATUS_PLAYING,
    AUDIO_STATUS_STOPPED,
    AUDIO_STATUS_ERROR,
} audio_status;

struct AudioFifoImpl {
    struct AudioFifoS base; /* it must be first */
    struct CdxFifoStreamS *fifobase;
    pthread_mutex_t audio_mutex;
    sem_t start_sem;
    audio_status status;
    RTPlayer *player;
    pthread_t start_task;
    char thread_run;
};

static bool s_stop_immediately = false;

int audiofifo_stop_immediately(bool stop_immediately)
{
    s_stop_immediately = stop_immediately;
    return 0;
}

static void player_play_callback(void *userData, int msg, int id, int ext1, int ext2)
{
    printf("call back from RTplayer,msg = %d,id = %d,ext1 = %d,ext2 = %d\n",msg,id,ext1,ext2);
    struct AudioFifoImpl *impl = (struct AudioFifoImpl *)userData;
    switch (msg) {
    case RTPLAYER_NOTIFY_PREPARED: {
        LOGI("RTPLAYER_NOTIFY_PREPARED:has prepared.");
        // don't start play here!!! it will get stuck becasuse of mutex
        impl->status = AUDIO_STATUS_PLAYING;
        sem_post(&impl->start_sem);
        break;
    }
    case RTPLAYER_NOTIFY_PLAYBACK_COMPLETE: {
        LOGI("RTPLAYER_NOTIFY_PLAYBACK_COMPLETE:play complete");
        impl->status = AUDIO_STATUS_STOPPED;
        break;
    }
    case RTPLAYER_NOTIFY_SEEK_COMPLETE: {
        LOGI("RTPLAYER_NOTIFY_SEEK_COMPLETE:seek ok");
        break;
    }
    case RTPLAYER_NOTIFY_MEDIA_ERROR: {
        switch (ext1) {
        case RTPLAYER_MEDIA_ERROR_UNKNOWN: {
            LOGI("erro type:TPLAYER_MEDIA_ERROR_UNKNOWN");
            break;
        }
        case RTPLAYER_MEDIA_ERROR_UNSUPPORTED: {
            LOGI("erro type:TPLAYER_MEDIA_ERROR_UNSUPPORTED");
            break;
        }
        case RTPLAYER_MEDIA_ERROR_IO: {
            LOGI("erro type:TPLAYER_MEDIA_ERROR_IO");
            break;
        }
        }
        LOGI("RTPLAYER_NOTIFY_MEDIA_ERROR");
        impl->status = AUDIO_STATUS_ERROR;
        break;
    }
    case RTPLAYER_NOTIFY_NOT_SEEKABLE: {
        LOGI("info: media source is unseekable.");
        break;
    }
    case RTPLAYER_NOTIFY_DETAIL_INFO: {
        break;
    }
    default: {
        LOGI("warning: unknown callback from RTplayer.");
        break;
    }
    }
}

static int player_play_start(struct AudioFifoImpl *impl)
{
    char url[32];
    sprintf(url, "fifo://%p", impl->fifobase);
    registerCallback(impl->player, impl, player_play_callback);
    setDataSource_url(impl->player, impl, url, 0);
    return prepareAsync(impl->player);
}

static int player_play_stop(struct AudioFifoImpl *impl)
{
    stop(impl->player);
    return 0;
}

/* regist to AudioFifoSetPlayer --> .set_player --> audio_fifo_set_player  */
static int audio_fifo_set_player(struct AudioFifoS *audiofifo, RTPlayer *player)
{
    struct AudioFifoImpl *impl;

    impl = (struct AudioFifoImpl *)audiofifo;
    impl->player = player;
    return 0;
}

static int audio_fifo_start(struct AudioFifoS *audiofifo)
{
    struct AudioFifoImpl *impl;

    impl = (struct AudioFifoImpl *)audiofifo;

    pthread_mutex_lock(&impl->audio_mutex);
    if (impl->status != AUDIO_STATUS_STOPPED) {
        LOGE("fifo already start!");
        goto err;
    }

    /* create kfifo_stream */
    impl->fifobase = kfifo_stream_create();
    if (impl->fifobase == NULL) {
        LOGE("fifobase == NULL");
        goto err;
    }

    impl->status = AUDIO_STATUS_PREPARE;
    pthread_mutex_unlock(&impl->audio_mutex);

    if (player_play_start(impl) != 0) {
        LOGE("AUDIO_STATUS_ERROR");
        impl->status = AUDIO_STATUS_ERROR;
        return -1;
    }

    return 0;

err:
    pthread_mutex_unlock(&impl->audio_mutex);
    return -1;
}

static int audio_fifo_put_data(struct AudioFifoS *audiofifo, void *inData, int dataLen)
{
    uint32_t avail;
    uint32_t in_len;
    uint32_t reserve_len;
    uint32_t has_in_len = 0;
    struct AudioFifoImpl *impl;

    impl = (struct AudioFifoImpl *)audiofifo;

    pthread_mutex_lock(&impl->audio_mutex);

    while ((has_in_len != dataLen) && (impl->status <= AUDIO_STATUS_PLAYING)) {
        reserve_len = dataLen - has_in_len;
        CdxFifoStreamLock(impl->fifobase);
        avail = CdxFifoStreamAvail(impl->fifobase);
        in_len = avail > reserve_len ? reserve_len : avail;
        CdxFifoStreamIn(impl->fifobase, (char *)inData + has_in_len, in_len);
        has_in_len += in_len;
        CdxFifoStreamUnlock(impl->fifobase);

        if (has_in_len != dataLen) {
            usleep(10 * 1000);
        }
    }
    pthread_mutex_unlock(&impl->audio_mutex);
    return has_in_len;
}

static int audio_fifo_stop(struct AudioFifoS *audiofifo, bool stop_immediately)
{
    int ret;
    unsigned int waittime = 0;
    struct AudioFifoImpl *impl;

    impl = (struct AudioFifoImpl *)audiofifo;

    pthread_mutex_lock(&impl->audio_mutex);
    if ((impl->status == AUDIO_STATUS_STOPPED) || (impl->status == AUDIO_STATUS_ERROR))
        goto err;
    CdxFifoStreamSeteos(impl->fifobase);

    if (stop_immediately) {
        LOGI("stop immediately");
        player_play_stop(impl);
    } else {
        while (s_stop_immediately == false
            && (impl->status <= AUDIO_STATUS_PLAYING) 
            && (CdxFifoStreamValid(impl->fifobase) != 0)) {
            usleep(100 * 1000);
        }
        while (s_stop_immediately == false
                && (impl->status <= AUDIO_STATUS_PLAYING) 
                && (waittime < 5000)) {
            usleep(200 * 1000);
            waittime += 200;
        }
        if (impl->status <= AUDIO_STATUS_PLAYING)
            player_play_stop(impl);
        LOGI("fifo play finish");
    }

    //等待cdx退出后再释放资源，否则可能会死机
    while (impl->status == AUDIO_STATUS_PREPARE) {
            usleep(30 * 1000);
    }

err:
    impl->status = AUDIO_STATUS_STOPPED;
    //    OS_MutexUnlock(&impl->audio_mutex);

    pthread_mutex_unlock(&impl->audio_mutex);

    return 0;
}

static const struct AudioFifoOpsS AudioFifoOps = {
    .set_player = audio_fifo_set_player,
    .start = audio_fifo_start,
    .put_data = audio_fifo_put_data,
    .stop = audio_fifo_stop,
};

static void *fifo_start(void *arg)
{
    struct AudioFifoImpl *impl = arg;
    /*
 * In fact, this thread is no longer used after starting the player,
 * but in order to avoid unexpected timing,
 * it is closed in the audio_fifo_destroy.
*/
    while (1) {
        sem_wait(&impl->start_sem);
        if (CdxFifoStreamIseos(impl->fifobase) && impl->thread_run == 0) {
            LOGE("audiofifo exit==========\n");
            break;
        }
        if (impl->status == AUDIO_STATUS_PLAYING) {
            printf("fifo_start prepare==============\n");
            start(impl->player);
        } else {
            LOGE("fifo_start unknow status -- %d\n", impl->status);
        }
    }
    return NULL;
}

struct AudioFifoS *audio_fifo_create()
{
    struct AudioFifoImpl *impl;

    impl = malloc(sizeof(*impl));
    if (impl == NULL) {
        LOGE("fail");
        return NULL;
    }
    memset(impl, 0, sizeof(*impl));

    impl->status = AUDIO_STATUS_STOPPED;
    impl->base.ops = &AudioFifoOps; /* regist the function */
    pthread_mutex_init(&impl->audio_mutex, NULL);
    sem_init(&impl->start_sem, 0, 0);
    impl->thread_run = 1;
    s_stop_immediately = false;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param sched;
    sched.sched_priority = FIFO_START_PRIORITY;
    pthread_attr_setschedparam(&attr, &sched);
    pthread_attr_setstacksize(&attr, 1024);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&(impl->start_task), &attr, fifo_start, impl)) {
        LOGE("play task create failed. exit!");
        pthread_mutex_destroy(&impl->audio_mutex);
        sem_destroy(&impl->start_sem);
        free(impl);
        return NULL;
    }

    return &impl->base;
}

int audio_fifo_destroy(struct AudioFifoS *audiofifo)
{
    struct AudioFifoImpl *impl;

    impl = (struct AudioFifoImpl *)audiofifo;
    
    impl->thread_run = 0;
    sem_post(&impl->start_sem);
    pthread_cancel(impl->start_task);
    sem_destroy(&impl->start_sem);
    pthread_mutex_destroy(&impl->audio_mutex);

    if (impl->fifobase != NULL) {
        kfifo_stream_destroy(impl->fifobase);
        impl->fifobase = NULL;
    }
    free(impl);
    LOGI("done");
    return 0;
}