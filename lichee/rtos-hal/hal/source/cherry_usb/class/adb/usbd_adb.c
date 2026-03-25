/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_adb.h"
#include <console.h>
#include "adb_shell.h"
#include "adb_log.h"
#include "hal_sem.h"

#define ADB_OUT_EP_IDX 0
#define ADB_IN_EP_IDX  1

#define ADB_STATE_READ_MSG  0
#define ADB_STATE_READ_DATA 1

#define ADB_STATE_WRITE_MSG  0
#define ADB_STATE_WRITE_DATA 1

#define CACHE_ALGIN_SIZE   64
#define ADB_ALIGN_UP(x, y) (((x) + y - 1) & ~(y - 1))
#define ADB_PACKET_SIZE    ADB_ALIGN_UP(sizeof(struct adb_packet), CACHE_ALGIN_SIZE)

extern struct adb_server *find_adb_service(unsigned int localid);
struct usbd_adb {
    uint8_t state;
    uint8_t read_state;
    uint8_t write_state;
    bool writable;
    uint32_t localid;
    hal_sem_t tx_lock;
    hal_sem_t adb_prepare;
};

static struct usbd_adb *adb_client = NULL;
static struct usbd_endpoint adb_ep_data[2];
static struct adb_packet *tx_packet = NULL;
static struct adb_packet *rx_packet = NULL;

static inline uint32_t adb_packet_checksum(struct adb_packet *packet)
{
    uint32_t sum = 0;
    uint32_t i;

    for (i = 0; i < packet->msg.data_length; ++i) {
        sum += (uint32_t)(packet->payload[i]);
    }

    return sum;
}

extern adb_ev g_ev_handle;
void usbd_adb_bulk_out(uint8_t ep, uint32_t nbytes)
{
    (void)ep;
    // hal_dcache_invalidate(rx_packet, ADB_PACKET_SIZE); // no use DMA now
    if (adb_client->read_state == ADB_STATE_READ_MSG) {
        if (nbytes != sizeof(struct adb_msg)) {
            if (nbytes == 0) {
                unsigned long zeropage = ZEROPAGE_MAGIC;
                adb_event_set(g_ev_handle, (adb_packet *)&zeropage);
            }
            // USB_LOG_ERR("invalid adb msg size:%d\r\n", nbytes);
            usbd_ep_start_read(adb_ep_data[ADB_OUT_EP_IDX].ep_addr, (uint8_t *)&rx_packet->msg,
                               sizeof(struct adb_msg));
            return;
        }

        if (rx_packet->msg.data_length) {
            /* setup next out ep read transfer */
            adb_client->read_state = ADB_STATE_READ_DATA;
            usbd_ep_start_read(adb_ep_data[ADB_OUT_EP_IDX].ep_addr, rx_packet->payload,
                               rx_packet->msg.data_length);
        } else {
            adb_event_set(g_ev_handle, rx_packet);
            adb_client->read_state = ADB_STATE_READ_MSG;
            /* setup first out ep read transfer */
            usbd_ep_start_read(adb_ep_data[ADB_OUT_EP_IDX].ep_addr, (uint8_t *)&rx_packet->msg,
                               sizeof(struct adb_msg));
        }
    } else if (adb_client->read_state == ADB_STATE_READ_DATA) {
        adb_event_set(g_ev_handle, rx_packet);
        adb_client->read_state = ADB_STATE_READ_MSG;
        usbd_ep_start_read(adb_ep_data[ADB_OUT_EP_IDX].ep_addr, (uint8_t *)&rx_packet->msg,
                           sizeof(struct adb_msg));
    }
}

void usbd_adb_bulk_in(uint8_t ep, uint32_t nbytes)
{
    (void)ep;
    (void)nbytes;
    int cnt = 0;

    if (adb_client->write_state == ADB_STATE_WRITE_MSG) {
        if (nbytes != sizeof(struct adb_msg)) {
            USB_LOG_DBG("zero pack!\n");
            goto usbd_adb_bulk_in_error;
        }
        if (tx_packet->msg.data_length) {
            adb_client->write_state = ADB_STATE_WRITE_DATA;
            usbd_ep_start_write(adb_ep_data[ADB_IN_EP_IDX].ep_addr, tx_packet->payload,
                                tx_packet->msg.data_length);
        } else {
            hal_sem_post(adb_client->tx_lock);
        }
    } else {
        if (nbytes != tx_packet->msg.data_length) {
            USB_LOG_ERR("error data size!\n");
            goto usbd_adb_bulk_in_error;
        }
        adb_client->write_state = ADB_STATE_WRITE_MSG;
        hal_sem_post(adb_client->tx_lock);
    }

    return;
usbd_adb_bulk_in_error:
    hal_sem_getvalue(adb_client->tx_lock, &cnt);
    if (cnt == 0)
        hal_sem_post(adb_client->tx_lock);
    adb_client->write_state = ADB_STATE_WRITE_MSG;
}

void adb_notify_handler(uint8_t event, void *arg)
{
    (void)arg;

    switch (event) {
    case USBD_EVENT_RESET:
        break;
    case USBD_EVENT_CONFIGURED:
        adb_client->read_state = ADB_STATE_READ_MSG;
        /* setup first out ep read transfer */
        hal_sem_post(adb_client->adb_prepare);
        break;

    default:
        break;
    }
}

int usbd_adb_prepare_finish(void)
{
    hal_sem_timedwait(adb_client->adb_prepare, -1);
    hal_sem_timedwait(adb_client->tx_lock, -1);
    tx_packet->msg.command = A_SYNC;
    tx_packet->msg.arg0 = 1;
    tx_packet->msg.arg1 = 1;
    tx_packet->msg.data_length = 0;

    tx_packet->msg.data_crc32 = 0;
    tx_packet->msg.magic = tx_packet->msg.command ^ 0xffffffff;
    print_apacket("send", tx_packet);
    usbd_ep_start_write(adb_ep_data[ADB_IN_EP_IDX].ep_addr, (uint8_t *)&tx_packet->msg,
                        sizeof(struct adb_msg));

    usbd_ep_start_read(adb_ep_data[ADB_OUT_EP_IDX].ep_addr, (uint8_t *)&rx_packet->msg,
                       sizeof(struct adb_msg));
}

struct usbd_interface *usbd_adb_init_intf(struct usbd_interface *intf, uint8_t in_ep,
                                          uint8_t out_ep)
{
    adb_client = adb_malloc(sizeof(struct usbd_adb));
    if (adb_client == NULL)
        return NULL;
    memset(adb_client, 0, sizeof(struct usbd_adb));

    tx_packet = hal_malloc_coherent(sizeof(struct adb_packet) + CACHE_ALGIN_SIZE);
    if (tx_packet == NULL) {
        adb_free(adb_client);
        return NULL;
    }
    memset(tx_packet, 0, sizeof(struct adb_packet));

    rx_packet = hal_malloc_coherent(sizeof(struct adb_packet) + CACHE_ALGIN_SIZE);
    if (rx_packet == NULL) {
        adb_free(adb_client);
        return NULL;
    }
    memset(rx_packet, 0, sizeof(struct adb_packet));
    adb_client->tx_lock = hal_sem_create(1);
    adb_client->adb_prepare = hal_sem_create(0);

    intf->class_interface_handler = NULL;
    intf->class_endpoint_handler = NULL;
    intf->vendor_handler = NULL;
    intf->notify_handler = adb_notify_handler;

    adb_ep_data[ADB_OUT_EP_IDX].ep_addr = out_ep;
    adb_ep_data[ADB_OUT_EP_IDX].ep_cb = usbd_adb_bulk_out;
    adb_ep_data[ADB_IN_EP_IDX].ep_addr = in_ep;
    adb_ep_data[ADB_IN_EP_IDX].ep_cb = usbd_adb_bulk_in;

    usbd_add_endpoint(&adb_ep_data[ADB_OUT_EP_IDX]);
    usbd_add_endpoint(&adb_ep_data[ADB_IN_EP_IDX]);

    extern void adb_event_create(void);
    adb_event_create();

    return intf;
}

int usbd_abd_write(uint32_t localid, uint32_t remoteid, const uint8_t *data, uint32_t len)
{
    hal_sem_timedwait(adb_client->tx_lock, -1);

    if (len == 0) {
        usbd_ep_start_write(adb_ep_data[ADB_IN_EP_IDX].ep_addr, NULL, 0);
    }

    tx_packet->msg.command = A_WRTE;
    tx_packet->msg.arg0 = localid;
    tx_packet->msg.arg1 = remoteid;
    tx_packet->msg.data_length = len;
    memcpy(tx_packet->payload, data, len);

    tx_packet->msg.data_crc32 = adb_packet_checksum(tx_packet);
    tx_packet->msg.magic = tx_packet->msg.command ^ 0xffffffff;
    // hal_dcache_clean(tx_packet, ADB_PACKET_SIZE);
    print_apacket("send", tx_packet);
    usbd_ep_start_write(adb_ep_data[ADB_IN_EP_IDX].ep_addr, (uint8_t *)&tx_packet->msg,
                        sizeof(struct adb_msg));
    return 0;
}

void adb_send_msg(struct adb_packet *packet)
{
    hal_sem_timedwait(adb_client->tx_lock, -1);
    memcpy(tx_packet, packet, sizeof(struct adb_packet));
    tx_packet->msg.data_crc32 = adb_packet_checksum(tx_packet);
    tx_packet->msg.magic = tx_packet->msg.command ^ 0xffffffff;
    // hal_dcache_clean(tx_packet, ADB_PACKET_SIZE);
    print_apacket("send", tx_packet);
    usbd_ep_start_write(adb_ep_data[ADB_IN_EP_IDX].ep_addr, (uint8_t *)&tx_packet->msg,
                        sizeof(struct adb_msg));
}