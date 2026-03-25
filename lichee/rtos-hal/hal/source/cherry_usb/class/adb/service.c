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

#include "adb_shell.h"
#include "adb_sync.h"
#include "adb_socket.h"
#include "adb_log.h"
#include <sys/time.h>

const char *support_feature = "device::"
                              "ro.product.name=Tina;"
                              "ro.product.model=sun20iw2;"
                              "ro.product.device=R128";

static LIST_HEAD(g_adb_services);
adb_ev g_ev_handle;
unsigned int gLocalID = 0;
pthread_t g_event_tid;

struct adb_server *find_adb_service(unsigned int localid)
{
    struct adb_server *ser = NULL, *temp_a = NULL;
    list_for_each_entry_safe (ser, temp_a, &g_adb_services, list) {
        if (ser->localid == localid)
            return ser;
    }
    return NULL;
}

void adb_service_delte(struct adb_server *ser)
{
    list_del(&ser->list);
}

extern void adb_send_okay(struct adb_packet *packet, uint32_t localid, uint32_t remoteid);
void adb_send_close(struct adb_packet *packet, uint32_t localid, uint32_t remoteid);

void adb_g_event_init()
{
    g_ev_handle = adb_event_init();
}

void adb_g_event_deinit(pthread_t event_tid)
{
    // adb_event_set(g_ev_handle, ADB_EV_EXIT);
    // USB_LOG_INFO("wait g_event thread exit ...\r\n");
    // adb_thread_wait(event_tid, NULL);
    // USB_LOG_INFO("g_event thread exit\r\n");
    // adb_event_release(g_ev_handle);
}

void adb_service_destroy(unsigned int localid)
{
}

void adb_service_destroy_all(void)
{
    struct adb_server *ser = NULL, *tmp_ser = NULL;
    list_for_each_entry_safe (ser, tmp_ser, &g_adb_services, list) {
        adb_service_destroy(ser->localid);
    }
    return;
}

static void adbd_send_connect(adb_packet *packet)
{
    packet->msg.command = A_CNXN;
    packet->msg.arg0 = A_VERSION;
    packet->msg.arg1 = MAX_PAYLOAD;
    packet->msg.data_length = strlen(support_feature);
    memcpy(packet->payload, support_feature, strlen(support_feature));
    adb_send_msg(packet);
}

static void adbd_send_okay(adb_packet *packet, unsigned int localid)
{
    struct adb_msg adb_ok_msg;
    unsigned long *zeropage;

    adb_ok_msg.command = A_OKAY;
    adb_ok_msg.arg1 = packet->msg.arg0;
    adb_ok_msg.arg0 = localid;
    adb_ok_msg.data_length = 0;

    if (((packet->msg.data_length % WINUSB_MAX_MPS) == 0) &&
        (packet->msg.data_length != 0)) {
        adb_event_get(g_ev_handle, packet);
        zeropage = (unsigned long *)packet;
        if (*zeropage != ZEROPAGE_MAGIC)
            adbd_err("expect to get ZLP");
    }

    adb_send_msg((adb_packet *)&adb_ok_msg);
}

static int adbd_create_new_service(const char *name, unsigned int remoteid)
{
    struct adb_server *ser = NULL;

    if (!strncmp(name, "shell:", 6)) {
#ifdef CONFIG_COMPONENTS_MULTI_CONSOLE
        if (name[6] == 0x00) {
            ser = adb_shell_task_create(remoteid);
        } else {
            ser = adb_shell_once(remoteid, (const char *)&name[6]);
        }
#else
        return -1;
#endif
    } else if (!strncmp(name, "sync:", 5)) {
        ser = adb_sync_task_create(remoteid);
#ifdef CONFIG_CHERRY_ADB_SOCKET
    } else if (!strncmp(name, "tcp:", 4)) {
        int port = atoi(name + 4);
        char *rename = strchr(name + 4, ':');
		if (rename == 0) {
			ser = adb_socket_task_create(remoteid, port);
		} else {
			adbd_err("unknown name");
		}
#endif
    } else {
    }

    if (ser != NULL) {
        list_add(&ser->list, &g_adb_services);
        return ser->localid;
    } else {
        return -1;
    }
}

static void *adbd_pack_handle(void *arg)
{
    adb_packet *p;
    int connect_state = 0;
    char *adb_type;
    unsigned int localid;
    struct adb_server *ser;

    p = adb_malloc(sizeof(adb_packet));
    if (p == NULL)
        goto adbd_pack_handle_exit;
    memset(p, 0, sizeof(adb_packet));

    usbd_adb_prepare_finish();

    while (1) {
        adb_event_get(g_ev_handle, p);
        print_apacket("recv", p);
        switch (p->msg.command) {
        case A_SYNC:
            if (p->msg.arg0) {
                adb_send_msg(p);
            } else {
                USB_LOG_ERR("SYNC, offline...arg0=%d\n", p->msg.arg0);
                adb_send_msg(p);
            }
            break;
        case A_CNXN:
            connect_state = 1;
            adbd_send_connect(p);
            break;
        case A_OPEN:
            if (connect_state != 1)
                break;
            adb_type = (char *)p->payload;
            adb_type[p->msg.data_length > 0 ? p->msg.data_length - 1 : 0] = 0;

            localid = adbd_create_new_service(adb_type, p->msg.arg0);
            if (localid < 0)
                break;

            adbd_send_okay(p, localid);
            break;
        case A_OKAY:
            break;
        case A_CLSE:
            localid = p->msg.arg1;
            ser = find_adb_service(localid);
            if (ser != NULL) {
                adb_service_delte(ser);
                if (ser->adb_close != NULL) {
                    ser->adb_close(ser);
                }
            }
            break;
        case A_WRTE:
            localid = p->msg.arg1;
            ser = find_adb_service(localid);
            if ((ser != NULL) && (ser->adb_write != NULL)) {
                ser->adb_write(ser, p->payload, p->msg.data_length);
            }
            adbd_send_okay(p, localid);
            break;
        case A_AUTH:
            break;
        default:
            break;
        }
    }
adbd_pack_handle_exit:
    if (p != NULL)
        adb_free(p);
    pthread_exit(NULL);
}

void adb_event_destory(void)
{
}

void adb_event_create(void)
{
    adb_g_event_init();
    gLocalID = 0;
    adb_thread_create(&g_event_tid, adbd_pack_handle, "adb-event", NULL, ADB_THREAD_HIGH_PRIORITY,
                      0);
}