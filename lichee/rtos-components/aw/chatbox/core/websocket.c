#include "websocket.h"

websocket_ctx_t *websocket_init(void)
{
	websocket_ctx_t *websocket_ctx = \
			(websocket_ctx_t *)websocket_malloc(sizeof(websocket_ctx_t));

	if (websocket_ctx == NULL) {
		WEBS_LOGE("malloc(%d) failed.\n", sizeof(websocket_ctx_t));
		return NULL;
	}

	websocket_memset(websocket_ctx, 0, sizeof(websocket_ctx_t));

	return websocket_ctx;
}

int websocket_deinit(websocket_ctx_t *webs_ctx)
{
	if (webs_ctx == NULL) {
		WEBS_LOGE("invailed param.\n");
		return WEBS_ERROR;
	}

	websocket_free(webs_ctx);

	return WEBS_SUCCESS;
}

static void on_socket_received_text(rws_socket socket, const char *text, const unsigned int length, bool is_finished)
{
	WEBS_LOGI("RX-CB: Socket text: %s\n\n", text);

	websocket_session_t *websocket_session = \
						(websocket_session_t *)rws_socket_get_user_object(socket);

	if (websocket_session && websocket_session->cb_set && websocket_session->cb_set->recvd_cb) {
		WEBS_LOGI("CB: call recvd_cb\n");
		websocket_session->cb_set->recvd_cb(websocket_session->usr_data, (uint8_t *)text, length, TEXT, is_finished);
	} else {
		data_t *data = (data_t *)websocket_malloc(sizeof(data_t));
		if (data == NULL) {
			WEBS_LOGE("malloc(%d) failed.\n", length);
			return ;
		}

		char *data_ptr = (char *)websocket_malloc(length + 1);
		if (data_ptr == NULL) {
			WEBS_LOGE("malloc(%d) failed.\n", length);
			websocket_free(data);
			return ;
		}

		websocket_memcpy(data_ptr, text, length);

		data_ptr[length] = '\0';

		data->type = TEXT;
		data->data_ptr = data_ptr;
		data->data_size = length;
		data->offset = 0;
		data->copy_ptr = data_ptr;

		WEBS_LOGI("CB: send webs queue\n");
		queue_send(websocket_session->webs_queue, (void *)&data);
	}

	return ;
}

static void on_socket_received_bin(rws_socket socket, const void *bin, const unsigned int length, bool is_finished)
{
	WEBS_LOGD("RX-CB: Socket bin: <%02x> - len: %d\n\n", ((char *)bin)[0], length);

	websocket_session_t *websocket_session = \
						(websocket_session_t *)rws_socket_get_user_object(socket);

	if (websocket_session && websocket_session->cb_set && websocket_session->cb_set->recvd_cb) {
		WEBS_LOGD("CB: call recvd_cb\n");
		websocket_session->cb_set->recvd_cb(websocket_session->usr_data, (uint8_t *)bin, length, BIN, is_finished);
	} else {
		data_t *data = (data_t *)websocket_malloc(sizeof(data_t));
		if (data == NULL) {
			WEBS_LOGE("malloc(%d) failed.\n", length);
			return ;
		}

		char *data_ptr = (char *)websocket_malloc(length + 1);
		if (data_ptr == NULL) {
			WEBS_LOGE("malloc(%d) failed.\n", length);
			websocket_free(data);
			return ;
		}

		websocket_memcpy(data_ptr, bin, length);

		data_ptr[length] = '\0';

		data->type = BIN;
		data->data_ptr = data_ptr;
		data->data_size = length;
		data->offset = 0;
		data->copy_ptr = data_ptr;

		WEBS_LOGI("CB: send webs queue\n");
		queue_send(websocket_session->webs_queue, (void *)&data);
	}
}

static void on_socket_connected(rws_socket socket)
{
	WEBS_LOGI("RX-CB: Socket connected\n");

	websocket_session_t *websocket_session = \
						(websocket_session_t *)rws_socket_get_user_object(socket);

	if (websocket_session && websocket_session->cb_set && websocket_session->cb_set->connected_cb) {
		websocket_session->cb_set->connected_cb(websocket_session->usr_data);
	}
}

static void on_socket_disconnected(rws_socket socket)
{
	WEBS_LOGI("RX-CB: Socket disconnected\n");

	rws_error error = rws_socket_get_error(socket);
	if(error) {
		WEBS_LOGI("CB: Socket disconnect with code, error: %i, %s\n",
				rws_error_get_code(error),
				rws_error_get_description(error));
	}

	websocket_session_t *websocket_session = \
						(websocket_session_t *)rws_socket_get_user_object(socket);

	if (websocket_session && websocket_session->cb_set && websocket_session->cb_set->disconnected_cb) {
		websocket_session->cb_set->disconnected_cb(websocket_session->usr_data);
	}
}

static char *websocket_get_scheme(const char *url)
{
	static char scheme[4] = {0};

	if (strncmp(url, "ws://", 5) == 0) {
		strncpy(scheme, "ws", 2);
	} else if (strncmp(url, "wss://", 6) == 0) {
		strncpy(scheme, "wss", 3);
	}

	return scheme;
}

static char *websocket_get_host(const char *url)
{
	static char host[256] = {0};

	char *host_start = strstr(url, "://");

	if (host_start == NULL)
		return NULL;

	host_start += 3; // Skip the "://"

	char *host_end = strchr(host_start, '/');

	if (host_end == NULL)
		host_end = strchr(host_start, '\0'); // If no '/', take till end of string

	size_t len = host_end - host_start;

	strncpy(host, host_start, len);

	return host;
}

static char *websocket_get_path(const char *url)
{
	static char path[256] = {0};

	char *a0 = strstr(url, "://");
	if (a0 == NULL)
		return NULL;

	char *a1 = strchr(a0 + 3, '/');
	if (a1 == NULL)
		return NULL;

	char *path_start = strchr(a1, '/');
	if (path_start == NULL)
		return NULL;

	strncpy(path, path_start, strlen(path_start));

	return path; // Return everything from the '/' to the end
}

static int websocket_get_port(const char *url)
{
	int port = 80;

	if (strncmp(url, "ws://", 5) == 0) {
		return 80;
	} else if (strncmp(url, "wss://", 6) == 0) {
		return 443;
	}

	return port;
}

static int websocket_get_url_info(const char *url, 
		char **scheme, char **host, char **path, int *port)
{
	*scheme = websocket_get_scheme(url);
	printf("scheme: %s\n",*scheme);

	*host = websocket_get_host(url);
	printf("host: %s\n",*host);

	*path = websocket_get_path(url);
	printf("path: %s\n",*path);

	*port = websocket_get_port(url);
	printf("port: %d\n",*port);

	return 0;
}

websocket_session_t *websocket_open(websocket_ctx_t *webs_ctx, const char *url, 
		const char *ext_h, cb_set_t *cb_set, void *usr_data)
{
	websocket_session_t *websocket_session = \
			(websocket_session_t *)websocket_malloc(sizeof(websocket_session_t));

	if (websocket_session == NULL) {
		WEBS_LOGE("malloc(%d) failed.\n", sizeof(websocket_session_t));
		return NULL;
	}

	websocket_memset(websocket_session, 0, sizeof(websocket_session_t));

	websocket_session->ctx = webs_ctx;
	websocket_session->cb_set = cb_set;
	websocket_session->usr_data = usr_data;
	websocket_memcpy(websocket_session->url, url, sizeof(websocket_session->url));
	websocket_memcpy(websocket_session->ext_head, ext_h, sizeof(websocket_session->ext_head));

	rws_socket _socket = rws_socket_create();

	if (_socket == NULL) {
		WEBS_LOGE("use rws. rws socket create failed. websocket open failed.\n");
		goto exit0;
	}

	static char *scheme = "ws", *host, *path; int port = 80;
	websocket_get_url_info(websocket_session->url, &scheme, &host, &path, &port);
	rws_socket_set_url(_socket, scheme, host, port, path);

	WEBS_LOGI("user header: \n%s\n", websocket_session->ext_head);
	rws_socket_set_ext_head(_socket, websocket_session->ext_head);

	rws_socket_set_on_disconnected(_socket, &on_socket_disconnected);
	rws_socket_set_on_connected(_socket, &on_socket_connected);
	rws_socket_set_on_received_text(_socket, &on_socket_received_text);
	rws_socket_set_on_received_bin(_socket, &on_socket_received_bin);

	websocket_session->net_conn = (void *)_socket;

	if (rws_socket_connect(_socket) == rws_false) {
		WEBS_LOGE("rws socket connect failed.\n");
		goto exit1;
	}

	rws_socket_set_user_object(_socket, websocket_session);

	websocket_session->webs_queue = queue_create("webs_queue", sizeof(char *), WEBS_QUEUE_SIZE);

	return websocket_session;

exit1:
	rws_socket_delete(_socket);
exit0:
	websocket_free(websocket_session);
	return NULL;
}

int websocket_close(websocket_session_t *webs_sess)
{
	if (webs_sess == NULL) {
		return WEBS_ERROR;
	}

	rws_socket _socket = (rws_socket)webs_sess->net_conn;

	rws_socket_disconnect_and_release(_socket);

	queue_delete(webs_sess->webs_queue);

	websocket_free(webs_sess);

	return WEBS_SUCCESS;
}

int websocket_write_text(websocket_session_t *webs_sess, const char *message)
{
	if (webs_sess == NULL) {
		return WEBS_ERROR;
	}

	rws_socket _socket = (rws_socket)webs_sess->net_conn;

	if (rws_socket_is_connected(_socket)) {
		WEBS_LOGI("TX-<message: %s>\n", message);
		if (rws_socket_send_text(_socket, message) != rws_true) {
			WEBS_LOGE("write error.\n");
			return WEBS_ERROR;
		}
	}

	return WEBS_SUCCESS;
}

int websocket_write_bin(websocket_session_t *webs_sess, const uint8_t *data, size_t size)
{
#if 1
	if (webs_sess == NULL) {
		return WEBS_ERROR;
	}

	rws_socket _socket = (rws_socket)webs_sess->net_conn;

	if (rws_socket_is_connected(_socket)) {
		WEBS_LOGD("TX-<bin: %02x %02x %02x %02x %02x - size: %d>\n", data[0], data[1], data[2], data[3], data[4], size);
		if (rws_socket_send_bin_start(_socket, data, size) != rws_true) {
			WEBS_LOGE("write error.\n");
			return WEBS_ERROR;
		}
		if (rws_socket_send_bin_finish(_socket, data, 0) != rws_true) {
			WEBS_LOGE("write error.\n");
			return WEBS_ERROR;
		}
	}
#endif

	return WEBS_SUCCESS;
}

int websocket_read(websocket_session_t *webs_sess, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type)
{
	size_t bytes_received = 0;
	int offset = 0;
	data_t *data = NULL;
	static data_t *lave_data = NULL;

	if (webs_sess == NULL || recv_buffer == NULL) {
		WEBS_LOGE("invailed param.\n");
		return WEBS_ERROR;
	}

	while(read_bytes > 0) {
		if (lave_data == NULL) {
			if (queue_recv(webs_sess->webs_queue, (void *)&data, 5000)) {
				WEBS_LOGE("websocket_read(., 5000ms) timeout.\n");
				return -1;
			}
		} else {
			data = lave_data;
		}

		if (data != NULL) {
			size_t data_size = data->data_size;

			*type = data->type;

			if (data_size > read_bytes) {
				data->offset += read_bytes;
				data->data_size -= read_bytes;

				websocket_memcpy(recv_buffer + offset, data->copy_ptr, read_bytes);
				offset += read_bytes;
				read_bytes -= read_bytes;

				data->copy_ptr += data->offset;
				lave_data = data;
				bytes_received += read_bytes;
				break;
			} else {
				websocket_memcpy(recv_buffer + offset, data->copy_ptr, data_size);
				offset += data_size;
				read_bytes -= data_size;

				websocket_free(data->data_ptr);
				websocket_free(data);

				lave_data = NULL;
				bytes_received += data_size;
				continue;
			}
		} else {
			break;
		}
	}

	return bytes_received;
}
