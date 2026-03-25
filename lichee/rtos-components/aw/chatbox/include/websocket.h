#ifndef __WEBSOCK_H
#define __WEBSOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "librws.h"
#include "queue.h"
#include "log.h"

#define WEBS_QUEUE_SIZE 256

/**************************/
#define WEBS_DEBUG  0
#define WEBS_INFO   0

#if WEBS_DEBUG > 0
#define WEBS_LOGD(fmt, arg...) printf("[WEBSOCKET][DEBUG]"fmt, ##arg)
#else
#define WEBS_LOGD(fmt, arg...)
#endif

#if WEBS_INFO > 0
#define WEBS_LOGI(fmt, arg...) printf("[WEBSOCKET][INFO]"fmt, ##arg)
#else
#define WEBS_LOGI(fmt, arg...)
#endif

#define WEBS_LOGE(fmt, arg...) printf("[WEBSOCKET][ERROR]"fmt, ##arg)
/*************************/

/************************/
#define websocket_malloc malloc
#define websocket_free free
#define websocket_memset memset
#define websocket_memcpy memcpy
/************************/

enum websocket_ret_code {
	WEBS_SUCCESS = 0,
	WEBS_ERROR = -1,
	WEBS_ERROR_MEM = -2,
};

enum websocket_data_type {
	TEXT,
	BIN
};

typedef struct {
	void (*recvd_cb)(void *usr_data, const uint8_t *data, size_t size, uint8_t type, int is_finished);
	void (*connected_cb)(void *usr_data);
	void (*disconnected_cb)(void *usr_data);
} cb_set_t;

typedef struct {
	char type;
	char *data_ptr;
	size_t data_size;
	size_t offset;
	char *copy_ptr;
} data_t;

typedef struct {
	uint32_t reserve;
} websocket_ctx_t;

typedef struct {
	websocket_ctx_t *ctx;
	void *net_conn;
	cb_set_t *cb_set;
	void *usr_data;
	struct queue *webs_queue;
	char url[128];
	char ext_head[256];
} websocket_session_t;

websocket_ctx_t *websocket_init(void);

int websocket_deinit(websocket_ctx_t *ctx);

websocket_session_t *websocket_open(websocket_ctx_t *webs_ctx, const char *url, 
		const char *ext_h, cb_set_t *cb_set, void *usr_data);

int websocket_close(websocket_session_t *session);

int websocket_write_text(websocket_session_t *session, const char *message);

int websocket_write_bin(websocket_session_t *session, const uint8_t *data, size_t size);

int websocket_read(websocket_session_t *webs_sess, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type);
#endif /* __WEBSOCK_H */
