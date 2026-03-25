/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the people's Republic of China and other countries.
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

/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
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
#ifdef CONFIG_CHERRY_ADB_SOCKET
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "stdlib.h"

#include <lwip/inet.h>
#include "lwip/sockets.h"
#include <semaphore.h>

#include <aw_list.h>
#include <console.h>

#include <pthread.h>

#include "adb_log.h"
#include "misc.h"
#include "adb_socket.h"
#include "usb_osal.h"

#define ADB_SOCKET_RINGBUFF_SIZE (64 * 1024)
// for record forward, 100ms is long time
#define ADB_SOCKET_GET_TIME_OUT 100

#define UPLOAD_START "-->AW_RTOS_SOCKET_UPLOAD_START"
#define UPLOAD_END   "-->AW_RTOS_SOCKET_UPLOAD_END"

enum {
    ADB_FORWARD_THREAD_UNKNOWN = 0,
    ADB_FORWARD_THREAD_PREPARE,
    ADB_FORWARD_THREAD_INIT,
    ADB_FORWARD_THREAD_RUNNING,
    ADB_FORWARD_THREAD_FINISH,
    ADB_FORWARD_THREAD_EXIT,
};

enum {
    ADB_PAUSE_IDLE = 0,
    ADB_PAUSE_DRAIN,
    ADB_PAUSE_FORCE,
};

typedef struct adb_forward {
    int port;
    //dlist_t list;
    struct list_head list;
    void *send_buf;
    unsigned int send_size;
    int is_raw_data;
    void *rb;
    uint8_t force_stop;
    uint8_t pause;
    usb_osal_sem_t stop_sem;
    // adb_queue_ev *queue_recv;
} adb_forward_t;

extern unsigned int gLocalID;
extern int hal_msleep(unsigned int msecs);

static LIST_HEAD(gAdbForwardThreadList);

static adb_forward_t *adb_forward_find(int port)
{
    adb_forward_t *adb_socket = NULL;
    //list_for_each_entry(&gAdbForwardThreadList, a, struct adb_forward, list) {
    list_for_each_entry (adb_socket, &gAdbForwardThreadList, list) {
        if (adb_socket->port == port)
            return adb_socket;
    }
    return NULL;
}

int adb_forward_create_with_rawdata(int port)
{
    adb_forward_t *adb_socket = NULL;

    if (port < 0) {
        adbd_err("unknown port=%d\n", port);
        return -1;
    }

    adb_socket = adb_forward_find(port);
    if (adb_socket != NULL) {
        if (!adb_socket->is_raw_data) {
            adbd_err("adb forward already exist, but not raw data\n");
            return -1;
        }
        return 0;
    }

    adb_socket = calloc(1, sizeof(adb_forward_t));
    if (!adb_socket) {
        adbd_err("no memory\n");
        return -1;
    }

    adb_socket->port = port;

    adb_socket->stop_sem = usb_osal_sem_create(0);
    if (adb_socket->stop_sem == NULL) {
        free(adb_socket);
        return -1;
    }

    adb_socket->is_raw_data = 1;

    list_add(&adb_socket->list, &gAdbForwardThreadList);
    return 0;
}

void adb_forward_destroy(int port)
{
    adb_forward_t *adb_socket = NULL;

    adb_socket = adb_forward_find(port);
    if (!adb_socket) {
        printf("adb forward thread with port:%d, isn't exist\n", port);
        return;
    }

    usb_osal_sem_delete(adb_socket->stop_sem);
    list_del(&adb_socket->list);

    free(adb_socket);
}

static void *adb_socket_task(void *arg)
{
    struct adb_server *aserver = (struct adb_server *)arg;
    adb_forward_t *adb_socket = aserver->priv;
    hal_ringbuffer_t rb = adb_socket->rb;
    uint8_t *socket_buffer = NULL;
    int get_data_size = 0;

    socket_buffer = malloc(MAX_PAYLOAD);
    if (socket_buffer == NULL)
        goto socket_exit;

    while (hal_ringbuffer_is_empty(rb)) {
        if (adb_socket->force_stop == 1)
            goto socket_exit;
        hal_msleep(100);
    }

    usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)UPLOAD_START,
                   (strlen(UPLOAD_START)));

    while (adb_socket->force_stop == 0) {
        get_data_size = hal_ringbuffer_get(rb, socket_buffer, MAX_PAYLOAD, ADB_SOCKET_GET_TIME_OUT);
        if (get_data_size <= 0)
            continue;

        usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)socket_buffer,
                       get_data_size);
        //ZLP
        if (get_data_size == MAX_PAYLOAD)
            usbd_abd_write(aserver->localid, aserver->remoteid, NULL, 0);

        if (adb_socket->pause == ADB_PAUSE_FORCE) {
            usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)UPLOAD_END,
                           strlen(UPLOAD_END));
            adb_socket->pause = ADB_PAUSE_IDLE;
        } else if (adb_socket->pause == ADB_PAUSE_DRAIN) {
            while (hal_ringbuffer_is_empty(rb) != 1) {
                get_data_size =
                        hal_ringbuffer_get(rb, socket_buffer, MAX_PAYLOAD, ADB_SOCKET_GET_TIME_OUT);
                if (get_data_size <= 0) {
                    break;
                }
                usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)socket_buffer,
                               get_data_size);
            }
            usbd_abd_write(aserver->localid, aserver->remoteid, (const uint8_t *)UPLOAD_END,
                           strlen(UPLOAD_END));
            adb_socket->pause = ADB_PAUSE_IDLE;
        }
    }
socket_exit:

    if (socket_buffer != NULL)
        free(socket_buffer);

    usb_osal_sem_give(adb_socket->stop_sem);
    pthread_exit(NULL);
}

static int adb_socket_stop(adb_server *aserver)
{
    adb_forward_t *adb_socket = aserver->priv;
    adb_socket->force_stop = 1;
    if (usb_osal_sem_take(adb_socket->stop_sem, 5000) < 0) {
        adbd_err("close socket task fail!\n");
        pthread_exit(&aserver->tid);
    }

    hal_ringbuffer_release(adb_socket->rb);

    adb_forward_destroy(adb_socket->port);
    adb_free(aserver);
    return 0;
}

int adb_forward_send(int port, void *data, unsigned len)
{
    adb_forward_t *adb_socket = adb_forward_find(port);

    if (adb_socket == NULL) {
        adbd_err("adb forward thread with port:%d, isn't exist\n", port);
        return -1;
    }

    if (adb_socket->rb == NULL) {
        adbd_err("adb socket server is not open\n");
        return -1;
    }

    if (adb_socket->pause != ADB_PAUSE_IDLE) {
        adbd_err("wait last forword\n");
        return -1;
    }

    int ret = hal_ringbuffer_wait_put(adb_socket->rb, data, len, 500);
    if (ret < 0) {
        adbd_err("adb socket put data time out\n");
    }

    return ret;
}

int adb_forward_end(int port, int force)
{
    adb_forward_t *adb_socket = adb_forward_find(port);
    if (adb_socket == NULL) {
        adbd_err("adb forward thread with port:%d, isn't exist\n", port);
        return -1;
    }

    if (force == 1)
        adb_socket->pause = ADB_PAUSE_FORCE;
    else
        adb_socket->pause = ADB_PAUSE_DRAIN;
    return 0;
}

struct adb_server *adb_socket_task_create(unsigned int remoteid, int port)
{
    adb_forward_t *adb_socket = adb_forward_find(port);
    if (adb_socket == NULL) {
        adb_forward_create_with_rawdata(port);
        adb_socket = adb_forward_find(port);
        if (adb_socket == NULL)
            return NULL;
    }

    struct adb_server *aserver = adb_malloc(sizeof(struct adb_server));
    if (aserver == NULL)
        return NULL;
    memset(aserver, 0, sizeof(struct adb_server));

    aserver->read_from_pc = NULL;

    aserver->localid = ++gLocalID;
    aserver->remoteid = remoteid;
    aserver->service_type = ADB_SERVICE_TYPE_SOCKET;
    aserver->adb_write = NULL;
    aserver->adb_close = adb_socket_stop;

    adb_socket->rb = hal_ringbuffer_init(ADB_SOCKET_RINGBUFF_SIZE);
    adb_socket->force_stop = 0;
    adb_socket->pause = ADB_PAUSE_IDLE;
    aserver->priv = adb_socket;

    adb_thread_create(&aserver->tid, adb_socket_task, "adb-socket", (void *)aserver,
                      ADB_THREAD_NORMAL_PRIORITY, 0);

    return aserver;
}

static void usage(void)
{
    printf("Usage: af [option]\n");
    printf("-p,          port\n");
    printf("-c,          create adb forward thread with port\n");
    printf("-r           create adb forward thread for sending raw data with port\n");
    printf("-s,          send msg\n");
    printf("-g,          recv msg and recv cnt\n");
    printf("-f,          transfer finish, send upload end flag\n");
    printf("-e,          exit adb forward thread with port\n");
    printf("\n");
}

static int cmd_adbforward(int argc, char *argv[])
{
    int c, port = -1;

    optind = 0;
    while ((c = getopt(argc, argv, "p:rh")) != -1) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'r':
            if (adb_forward_create_with_rawdata(port))
                adbd_err("adb forward init rawdata thread failed\n");
            else
                printf("adb forward init with port:%d success\n", port);
            break;
        case 'h':
        default:
            usage();
            return -1;
        }
    }
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_adbforward, af, adb forward);

#endif /* CONFIG_CHERRY_ADB_SOCKET */
