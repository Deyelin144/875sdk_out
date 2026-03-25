#include "http.h"

static char *parse_url_host(char *url)
{
	static char host[64] = {0};

	char *a, *b;

	a = strstr(url, "://");

	if (a) a += strlen("://");
	else return NULL;

	b = strchr(a, '/');

	if (b) ;
	else return NULL;

	http_memcpy(host, a, b - a);

	return host;
}

static char *parse_url_path(char *url)
{
	static char path[256] = {0};

	char *a, *b;

	a = strstr(url, "://");

	if (a) a += strlen("://");
	else return NULL;

	b = strchr(a, '/');

	if (b) ;
	else return NULL;

	http_memcpy(path, b, strlen(b));

	return path;
}

static session_t *network_open(char *url)
{
	int use_ssl = 0;

	if (strstr(url, "https") || strstr(url, "HTTPS")) {
		use_ssl = 1;
	} else {
		use_ssl = 0;
	}

	session_t *session = http_malloc(sizeof(session_t));

	if (session == NULL) {
		HTTP_LOGE("malloc failed.\n");
		goto exit0;
	}

	NetworkContext_t *pNetworkContext = http_malloc(sizeof(NetworkContext_t));

	if (pNetworkContext == NULL) {
		HTTP_LOGE("malloc failed.\n");
		goto exit1;
	}

	if (portNetworkInit(pNetworkContext, use_ssl)) {
		HTTP_LOGE("network init failed.\n");
		goto exit2;
	}

	ServerInfo_t pServerInfo = {
		.pHostName = parse_url_host(url),
		.port = use_ssl? 443:80,
	};

	if (portConnectServer(pNetworkContext, &pServerInfo, 5000, 5000)) {
		HTTP_LOGE("network connect server failed.\n");
		goto exit2;
	}

	session->network_ctx = pNetworkContext;

	return session;

exit2:
	http_free(pNetworkContext);
exit1:
	http_free(session);
exit0:
	return NULL;
}

static int network_close(session_t *session)
{
	if (session)
		portDisconnectServer(session->network_ctx);
	else return -1;

	http_free(session->network_ctx);

	http_free(session);

	return 0;
}

static int network_write(session_t *session, char *data, size_t size)
{
	if (session == NULL || data == NULL)
		return -1;

	int bytes_sent = portTransportSend(session->network_ctx, data, size);

	return bytes_sent;
}

static int network_read(session_t *session, char *recv_buf, size_t read_bytes)
{
	if (session == NULL || recv_buf == NULL)
		return -1;

	int bytes_received = portTransportRecv(session->network_ctx, recv_buf, read_bytes);

	return bytes_received;
}

static int http_read_header(void *session, char *buf, int buf_size)
{
	int ret = -1;
	int bytes_received = 0;
	char *ptr = buf;

_recv_one_byte:
	ret = network_read(session, ptr, 1);
	if (ret > 0 && bytes_received < (buf_size - 1)) {
		ptr ++;
		bytes_received ++;
		if (bytes_received > 4 && (http_memcmp(ptr - 4, "\r\n\r\n", 4) == 0)) {
			HTTP_LOGD("recv response header over.\n");
			return 0;
		}
		goto _recv_one_byte;
	} else {
		if (ret == 0)
			goto _recv_one_byte; // retry
		HTTP_LOGE("recv response failed! bytes_received: %d, buf_size: %d\n", \
				bytes_received, buf_size);
		return -1;
	}
}

static inline int is_http_head_contained(char *response)
{
	HTTP_LOGD("is http header? ");
	if (strstr(response, "HTTP") != NULL && \
			strstr(response, "\r\n\r\n") != NULL) {
		HTTP_LOGD("yes\n");
		return 1;
	}
	HTTP_LOGD("no\n");
	return 0;
}

static inline int is_chunked_transfer_encoding(char *response)
{
    const char *transfer_encoding_key[2] = {"transfer-encoding: ", \
		"Transfer-Encoding: "};
    const char *chunked = "chunked";

	char *start = NULL;
	int i = 0;
    /* find Transfer-Encoding */
	for (i = 0; i < 2; i ++) {
		start = strstr(response, transfer_encoding_key[i]);
		if (start) {
			break;
		}
	}
	if (i == 2) {
		/* can not find Transfer-Encoding, return false */
		return 0;
	}
	HTTP_LOGD("find %s\n", transfer_encoding_key[i]);

    /* find Transfer-Encoding, and find  "chunked" */
	/* skip Transfer-Encoding: " */
    start += strlen(transfer_encoding_key[0]);

    /* checking if Transfer-Encoding contains "chunked" */
    return (strstr(start, chunked) != NULL);
}

static inline int parse_content_length(char *response)
{
    const char *content_length_key[2] = {"Content-Length: ", \
		"content-length: "};
	int content_length = -1;

	char *start = NULL;
	int i = 0;
	for (i = 0; i < 2; i ++) {
		start = strstr(response, content_length_key[i]);
		if (start) {
			break;
		}
	}
	if (i == 2) {
		return -1;
	}

	start += strlen(content_length_key[0]);
	content_length = atoi(start);

	HTTP_LOGD("content_length: %d\n", content_length);

	return content_length;
}

static int http_parse_header(char *buf, response_t *response)
{
    HTTP_LOGD("\nRAW Response header:\n %s\n", buf);

	if (is_http_head_contained(buf)) {
		if (is_chunked_transfer_encoding(buf)) {
			response->is_chunked = 1;
			HTTP_LOGD("Response is chunked transfer encoding\n");
		} else {
			response->is_chunked = 0;
			response->content_length = parse_content_length(buf);
			HTTP_LOGI("Response is not chunked transfer encoding\n");
		}
	}

	return 0;
}

http_ctx_t *http_open(char *url)
{
	http_ctx_t *http_ctx = (http_ctx_t *)http_malloc(sizeof(http_ctx_t));

	if (http_ctx == NULL) {
		HTTP_LOGE("malloc failed.\n");
		goto exit0;
	}

	session_t *session = network_open(url);

	if (session == NULL) {
		HTTP_LOGE("network_open failed.\n");
		goto exit1;
	}

	http_ctx->session = session;

	return http_ctx;
exit1:
	http_free(http_ctx);
exit0:
	return NULL;
}

int http_post(http_ctx_t *ctx, char *url, char *ext_h, char *post_data, size_t post_data_size)
{
	if (ctx == NULL) {
		HTTP_LOGE("network not open.\n");
		return -1;
	}

	{
		char data[512] = {0};
		snprintf(data, sizeof(data),\
				"POST %s HTTP/1.1\r\n"
				"Host: %s\r\n"
				"Content-Length: %zu\r\n"
				"Connection: close\r\n"
				"%s"
				"\r\n",
				parse_url_path(url), parse_url_host(url),\
				post_data_size, ext_h);

		size_t size = strlen(data);
		HTTP_LOGI("http post header:\n %s \n size: %d\n", data, size);
		int bytes_sent = network_write(ctx->session, data, size);
		if (bytes_sent < 0) return -1;
	}

	if (post_data) {
		HTTP_LOGI("http post data:\n %s\n", post_data);
		char *data = post_data;
		size_t size = post_data_size;
		int bytes_sent = network_write(ctx->session, data, size);
		if (bytes_sent < 0) return -1;
	}

	return 0;
}

int http_recv_response(http_ctx_t *ctx, char **recv_buf)
{
	if (ctx == NULL || recv_buf == NULL)
		return -1;

	{
		char data[512] = {0};
		int ret = http_read_header(ctx->session, data, 512);
		if (ret) return -1;

		ret = http_parse_header(data, &ctx->response);
		if (ret) return -1;
	}

	char *buf = (char *)http_malloc(ctx->response.content_length);

	if (buf == NULL) {
		HTTP_LOGE("malloc failed.\n");
		return -1;
	}
	http_memset(buf, 0, ctx->response.content_length);

	{
		int left = ctx->response.content_length;
		char *ptr = buf;
		int bytes_received = 0;

		while (left > 0) {
			bytes_received = network_read(ctx->session, ptr, left);
			if (bytes_received > 0) {
				HTTP_LOGD("left: %d read: %d\n", left, bytes_received);
				left -= bytes_received;
				ptr += bytes_received;
			}
		}
	}
	ctx->response.buf = (void *)buf;

	*recv_buf = buf;

	HTTP_LOGI("Response body:\n %s\n", buf);

	return 0;
}

int http_close(http_ctx_t *ctx)
{
	if (ctx == NULL)
		return -1;

	network_close(ctx->session);

	if (ctx->response.buf)
		http_free(ctx->response.buf);

	http_free(ctx);

	return 0;
}

