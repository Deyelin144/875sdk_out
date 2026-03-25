#ifndef __TRANS_H
#define __TRANS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "log.h"
#include "websocket.h"

/**************************/
#define TRANS_DEBUG  0
#define TRANS_INFO   0

#if TRANS_DEBUG > 0
#define TRANS_LOGD(fmt, arg...) printf("[TRANS][DEBUG]"fmt, ##arg)
#else
#define TRANS_LOGD(fmt, arg...)
#endif

#if TRANS_INFO > 0
#define TRANS_LOGI(fmt, arg...) printf("[TRANS][INFO]"fmt, ##arg)
#else
#define TRANS_LOGI(fmt, arg...)
#endif

#define TRANS_LOGE(fmt, arg...) printf("[TRANS][ERROR]"fmt, ##arg)
/*************************/

/************************/
#define trans_malloc malloc
#define trans_free free
#define trans_memset memset
#define trans_memcpy memcpy
/************************/

enum trans_ret_code {
	TRANS_SUCCESS = 0,
	TRANS_ERROR = -1,
	TRANS_ERROR_MEM = -2,
};

typedef websocket_ctx_t ctx_t;
typedef websocket_session_t ses_t;

#ifndef NETWORK_PROTOCOL_TYPE
#define NETWORK_PROTOCOL_TYPE
typedef enum {
	WEBSOCKET,
	MQTT
} network_protocol_type;
#endif /* NETWORK_PROTOCOL_TYPE */

typedef struct {
	ctx_t *(*init)(void);
	int (*deinit)(ctx_t *ctx);
	ses_t *(*open)(ctx_t *ctx, const char *url, const char *ext_h, cb_set_t *cb_set, void *usr_data);
	int (*close)(ses_t *session);
	int (*write_text)(ses_t *session, const char *message);
	int (*write_bin)(ses_t *session, const uint8_t *data, size_t size);
	int (*read)(ses_t *session, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type);
} trans_inf_t;

typedef struct {
	ctx_t *inf_ctx;
	trans_inf_t *inf;
	uint32_t reserve;
} trans_context_t;

typedef struct {
	trans_context_t *trans_ctx;
	ses_t *inf_sess;
	void *user_obj;
} trans_session_t;

trans_context_t *trans_init(network_protocol_type type);

int trans_deinit(trans_context_t *trans_ctx);

trans_session_t *trans_open(trans_context_t *trans_ctx, const char *url, const char *ext_h,
		cb_set_t *cb_set, void *usr_data);

int trans_close(trans_session_t *trans_sess);

int trans_write_text(trans_session_t *trans_sess, const char *message);

int trans_write_bin(trans_session_t *trans_sess, const uint8_t *data, size_t size);

int trans_read(trans_session_t *trans_sess, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type);

int trans_set_userobj(trans_session_t *trans_sess, void *user_obj);

void *trans_get_userobj(trans_session_t *trans_sess);
#endif /* __TRANS_H */

