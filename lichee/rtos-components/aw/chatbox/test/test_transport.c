#include "transport.h"
#include "console.h"

#define URL "ws://echo.websocket.org/"
#define EXT_H "X-Custom-Header: myCustomValue\r\n"

void test_recvd_cb(void *usr_data, const uint8_t *data, size_t size, uint8_t type)
{
	printf("\nsession recvd data!\n");


}

void test_connected_cb(void *usr_data)
{
	printf("\nsession connected!\n");


}

void test_disconnected_cb(void *usr_data)
{
	printf("\nsession disconnected!\n");


}

static cb_set_t cb_set = {
	.recvd_cb = NULL,
	.connected_cb = test_connected_cb,
	.disconnected_cb = test_disconnected_cb
};

trans_context_t *g_trans_ctx;
trans_session_t *g_session_0;

void test_transport_init(int argc, char *argv[])
{
	trans_context_t *trans_ctx = trans_init(WEBSOCKET);

	if (trans_ctx == NULL) {
		printf("1. transport init failed.\n");
		return ;
	}

	printf("1. transport init success.\n");

	g_trans_ctx = trans_ctx;
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_init, t_init, transport test cmd);

void test_transport_open(int argc, char *argv[])
{
	if (g_trans_ctx == NULL) {
		printf("2. transport do not init. please init firstly\n");
		return ;
	}

	trans_session_t *session = trans_open(g_trans_ctx, URL, EXT_H, &cb_set, NULL);

	if (session == NULL) {
		printf("2. transport open failed.\n");
		return ;
	}

	printf("2. transport open success. waiting connect...\n");

	g_session_0 = session;
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_open, t_open, transport test cmd);

void test_transport_close(int argc, char *argv[])
{
	if (g_session_0 == NULL) {
		printf("3. transport do not open. please open firstly\n");
		return ;
	}

	trans_close(g_session_0);

	printf("3. transport close success.\n");
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_close, t_close, transport test cmd);

void test_transport_deinit(int argc, char *argv[])
{
	if (g_trans_ctx == NULL) {
		printf("4. transport do not deinit. please init firstly\n");
		return ;
	}

	trans_deinit(g_trans_ctx);

	printf("4. transport deinit success.\n");
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_deinit, t_deinit, transport test cmd);

void test_transport_write_text(int argc, char *argv[])
{
	if (g_session_0 == NULL) {
		printf("transport do not write. please open firstly\n");
		return ;
	}

	const char *message = "Hello Word! I am AW";

	int bytes_sent = trans_write_text(g_session_0, message);

	if (bytes_sent < 0) {
		printf("write text failed.\n");
		return ;
	}

	printf("write text success. bytes: %d\n", bytes_sent);
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_write_text, t_w_text, transport test cmd);

void test_transport_write_bin(int argc, char *argv[])
{
	if (g_session_0 == NULL) {
		printf("transport do not write. please open firstly\n");
		return ;
	}

	uint8_t data[64] = {0};

	for (int i = 0; i < 64; i ++) {
		data[i] = i;
	}

	int bytes_sent = trans_write_bin(g_session_0, data, sizeof(data));

	if (bytes_sent < 0) {
		printf("write bin failed.\n");
		return ;
	}

	printf("write bin success. bytes: %d\n", bytes_sent);
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_write_bin, t_w_bin, transport test cmd);

void test_transport_read(int argc, char *argv[])
{
	if (g_session_0 == NULL) {
		printf("transport do not write. please open firstly\n");
		return ;
	}

	uint8_t data[64] = {0};
	uint8_t type = 0;

	int bytes_received = trans_read(g_session_0, data, sizeof(data), &type);

	if (bytes_received < 0) {
		printf("read failed.\n");
		return ;
	}

	printf("read success. bytes: %d type: %d\n", bytes_received, type);
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_read, t_r, transport test cmd);

void test_transport_demo(int argc, char *argv[])
{
	printf("1. transport init.\n");
	test_transport_init(0, NULL);

	printf("2. transport open.\n");
	test_transport_open(0, NULL);

	printf("3. transport write_text.\n");
	test_transport_write_text(0, NULL);

	printf("4. transport read.\n");
	test_transport_read(0, NULL);

	printf("5. transport close.\n");
	test_transport_close(0, NULL);

	printf("6. transport deinit.\n");
	test_transport_deinit(0, NULL);
}
FINSH_FUNCTION_EXPORT_CMD(test_transport_demo, t_demo, transport test cmd);

