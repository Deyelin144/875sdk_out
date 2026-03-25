/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef USBD_ADB_H
#define USBD_ADB_H

#include <stdint.h>
#include <stdbool.h>
#include <cli_console.h>
#include "usb_config.h"
#include "hal_queue.h"
#include "ringbuffer.h"

// clang-format off
#define ADB_DESCRIPTOR_INIT(bFirstInterface, in_ep, out_ep, wMaxPacketSize) \
	USB_INTERFACE_DESCRIPTOR_INIT(bFirstInterface, 0x00, 0x02, 0xff, 0x42, 0x01, 0x02), \
	USB_ENDPOINT_DESCRIPTOR_INIT(in_ep, 0x02, wMaxPacketSize, 0x00), \
	USB_ENDPOINT_DESCRIPTOR_INIT(out_ep, 0x02, wMaxPacketSize, 0x00)
// clang-format on

#ifdef __cplusplus
extern "C" {
#endif

#define ZEROPAGE_MAGIC 0X6f72657a //zero

#ifdef CONFIG_USB_HS
#define WINUSB_MAX_MPS 512
#else
#define WINUSB_MAX_MPS 64
#endif

enum {
    ADB_SERVICE_TYPE_UNKNOWN = 0,
    ADB_SERVICE_TYPE_SHELL,
    ADB_SERVICE_TYPE_SYNC,
    ADB_SERVICE_TYPE_SOCKET,
};

#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_AUTH 0x48545541

#define A_VERSION 0x01000000

#define MAX_PAYLOAD_V1 (4 * 1024)
#define MAX_PAYLOAD_V2 (256 * 1024)
#define MAX_PAYLOAD    MAX_PAYLOAD_V1

typedef struct adb_server adb_server;
typedef struct adb_msg adb_msg;
typedef struct adb_packet adb_packet;
typedef pthread_t adb_thread_t;
typedef hal_queue_t adb_ev;

struct adb_msg {
    uint32_t command;     /* command identifier constant (A_CNXN, ...) */
    uint32_t arg0;        /* first argument */
    uint32_t arg1;        /* second argument */
    uint32_t data_length; /* length of payload (0 is allowed) */
    uint32_t data_crc32;  /* crc32 of data payload */
    uint32_t magic;       /* command ^ 0xffffffff */
};

struct adb_packet {
    struct adb_msg msg;
    uint8_t payload[MAX_PAYLOAD];
};

typedef int (*awrite)(adb_server *ser, unsigned char *buf, int len);
typedef int (*aclose)(adb_server *ser);

struct adb_server {
    cli_console *console;
    pthread_t tid;
    pthread_t mid;
    void *read_from_pc; /* queue, ringbuff ...*/

    unsigned int localid;
    unsigned int remoteid;
    unsigned int service_type;

    awrite adb_write;
    aclose adb_close;

    void *priv;

    struct list_head list;
};

struct usbd_interface *usbd_adb_init_intf(struct usbd_interface *intf, uint8_t in_ep,
                                          uint8_t out_ep);

bool usbd_adb_can_write(void);
void adb_send_msg(struct adb_packet *packet);
int usbd_abd_write(uint32_t localid, uint32_t remoteid, const uint8_t *data, uint32_t len);
void usbd_adb_close(uint32_t localid, uint32_t remoteid);
int usbd_adb_prepare_finish(void);

/* too mush adb connect should bigger */
#define ADB_MAX_PACKAGE 5
static inline adb_ev adb_event_init(void)
{
    return hal_queue_create("adb queue", sizeof(adb_packet), ADB_MAX_PACKAGE);
}

static inline int adb_event_delete(adb_ev event)
{
    return hal_queue_delete(event);
}

static inline int adb_event_set(adb_ev event, adb_packet *packet)
{
    return hal_queue_send_wait(event, packet, -1);
}

static inline int adb_event_get(adb_ev event, adb_packet *packet)
{
    return hal_queue_recv(event, packet, -1);
}

#ifdef __cplusplus
}
#endif

#endif /* USBD_ADB_H */