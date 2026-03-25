#ifndef __HTTP_H
#define __HTTP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "port_network.h"

/**************************/
#define HTTP_DEBUG  0
#define HTTP_INFO   0
#if HTTP_DEBUG > 0
#define HTTP_LOGD(fmt, arg...) printf("[HTTP][DEBUG]"fmt, ##arg)
#else
#define HTTP_LOGD(fmt, arg...)
#endif

#if HTTP_INFO > 0
#define HTTP_LOGI(fmt, arg...) printf("[HTTP][INFO]"fmt, ##arg)
#else
#define HTTP_LOGI(fmt, arg...)
#endif

#define HTTP_LOGE(fmt, arg...) printf("[HTTP][ERROR]"fmt, ##arg)
/*************************/

/************************/
#define http_malloc malloc
#define http_free free
#define http_memset memset
#define http_memcpy memcpy
#define http_memcmp memcmp
/************************/

#define HTTP_HEADER_LEN 256

typedef struct {
	int is_chunked;
	size_t content_length;
	void *buf;
} response_t;

typedef struct {
	NetworkContext_t *network_ctx;
} session_t;

typedef struct {
	session_t *session;
	response_t response;
} http_ctx_t;

http_ctx_t *http_open(char *url);

int http_post(http_ctx_t *ctx, char *url, char *ext_h, char *post_data, size_t post_data_size);

int http_recv_response(http_ctx_t *ctx, char **recv_buf);

int http_close(http_ctx_t *ctx);
#endif /* __HTTP_H */
