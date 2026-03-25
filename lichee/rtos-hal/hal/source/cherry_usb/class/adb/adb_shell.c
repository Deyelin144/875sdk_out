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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "adb_shell.h"
#include "adb_log.h"

extern unsigned int gLocalID;

int adb_console_write(const void *buf, size_t len, void *private_data)
{
    struct adb_server *aserver = (struct adb_server *)private_data;

    if (len > MAX_PAYLOAD) {
        uart_printf("adb max payloade is %d\n", MAX_PAYLOAD);
        return -1;
    }

    usbd_abd_write(aserver->localid, aserver->remoteid, buf, len);

    return len;
}

int adb_console_read(void *buf, size_t len, void *private_data)
{
    struct adb_server *aserver = (struct adb_server *)private_data;
    /* adb shell will wait for a long time, so we wait for ever */
    /* actually , shell read one byte once, so we don't care return size is excepted */
    int size = hal_ringbuffer_get(aserver->read_from_pc, buf, len, -1);
    if (size <= 0)
        *(char *)buf = 0;

    return 1;
}

static int adb_shell_write(adb_server *ser, unsigned char *buf, int len)
{
    return hal_ringbuffer_wait_put(ser->read_from_pc, buf, len, -1);
}

static int adb_shell_close(adb_server *aserver)
{
    if (aserver->priv != NULL)
        adb_free(aserver->priv);

    cli_console_destory(aserver->console);

    if (aserver->read_from_pc)
        hal_ringbuffer_release(aserver->read_from_pc);

    adb_free(aserver);
    return 0;
}

static int adb_console_init(void *private_data)
{
    return 1;
}

static int adb_console_deinit(void *private_data)
{
    return 1;
}

static device_console adb_console = {
    .name = "adb-console",
    .write = adb_console_write,
    .read = adb_console_read,
    .init = adb_console_init,
    .deinit = adb_console_deinit
};

void prvUARTCommandConsoleTask(void *pvParameters);
static void *console_task(void *arg)
{
    struct adb_msg msg;
    struct adb_server *aserver = (struct adb_server *)arg;

    pthread_set_console(pthread_self(), (cli_console *)aserver->console);
    // while(1)
    prvUARTCommandConsoleTask(NULL);
    // prvUARTCommandConsoleTask will exit by self
    return NULL;
}

static void *console_monitor(void *arg)
{
    struct adb_msg msg;
    struct adb_server *aserver = (struct adb_server *)arg;
    while (1) {
        int ret;
        ret = adb_thread_wait(aserver->tid, NULL);
        if (!ret)
            break;
        if (ret == -1)
            adbd_debug("pthread_timedjoin_np reutrn -1\n");
    }
    adbd_debug("adb shell exit\n");
    msg.command = A_CLSE;
    msg.arg0 = aserver->localid;
    msg.arg1 = aserver->remoteid;
    msg.data_length = 0;
    adb_send_msg((struct adb_packet *)&msg);

    cli_console_deinit(aserver->console);
    aserver->tid = NULL;
    aserver->mid = NULL;
    pthread_exit(NULL);
    return NULL;
}

portBASE_TYPE prvCommandEntry(char *pcWriteBuffer, size_t xWriteBufferLen,
                              const char *pcCommandString);
static void *console_once(void *arg)
{
    struct adb_msg msg;
    struct adb_server *aserver = (struct adb_server *)arg;
    TaskHandle_t handle;

    handle = xTaskGetCurrentTaskHandle();
    cli_task_set_console(handle, aserver->console);
    prvCommandEntry(NULL, 0, (const char *)aserver->priv);

    msg.command = A_CLSE;
    msg.arg0 = aserver->localid;
    msg.arg1 = aserver->remoteid;
    msg.data_length = 0;
    adb_send_msg((struct adb_packet *)&msg);

    cli_console_deinit(aserver->console);

    aserver->tid = NULL;
    pthread_exit(NULL);
    return NULL;
}

struct adb_server *adb_shell_once(unsigned int remoteid, const char *command)
{
    struct adb_server *aserver = adb_malloc(sizeof(struct adb_server));
    if (aserver == NULL)
        return NULL;
    memset(aserver, 0, sizeof(struct adb_server));

    aserver->localid = ++gLocalID;
    aserver->remoteid = remoteid;
    aserver->service_type = ADB_SERVICE_TYPE_SHELL;
    aserver->adb_close = adb_shell_close;
    aserver->console = cli_console_create(&adb_console, "adb-cli-once", (void *)aserver);
    if (!cli_console_check_invalid(aserver->console)) {
        adb_free(aserver);
        return NULL;
    }
    cli_console_init(aserver->console);

    aserver->console->init_flag = 1;
    aserver->console->task = NULL;
    aserver->console->alive = 1;

    aserver->priv = adb_malloc(32);
    strncpy((char *)aserver->priv, command, 32);

    adb_thread_create(&aserver->tid, console_once, "adb-shell-once", (void *)aserver,
                      ADB_THREAD_LOW_PRIORITY, 0);
    return aserver;
}

struct adb_server *adb_shell_task_create(unsigned int remoteid)
{
    struct adb_server *aserver = adb_malloc(sizeof(struct adb_server));
    if (aserver == NULL)
        return NULL;
    memset(aserver, 0, sizeof(struct adb_server));

    aserver->read_from_pc = hal_ringbuffer_init(ADB_SHELL_RINGBUFF_SIZE);

    aserver->localid = ++gLocalID;
    aserver->remoteid = remoteid;
    aserver->service_type = ADB_SERVICE_TYPE_SHELL;
    aserver->adb_write = adb_shell_write;
    aserver->adb_close = adb_shell_close;

    aserver->console = cli_console_create(&adb_console, "adb-cli-console", (void *)aserver);
    if (!cli_console_check_invalid(aserver->console)) {
        adb_free(aserver);
        return NULL;
    }

    cli_console_init(aserver->console);
    aserver->console->init_flag = 1;
    aserver->console->task = NULL;
    aserver->console->alive = 1;

    adb_thread_create(&aserver->tid, console_task, "adb-shell", (void *)aserver,
                      ADB_THREAD_NORMAL_PRIORITY, 1);
    adb_thread_create(&aserver->mid, console_monitor, "adb-shell-monitor", (void *)aserver,
                      ADB_THREAD_NORMAL_PRIORITY, 0);

    return aserver;
}
