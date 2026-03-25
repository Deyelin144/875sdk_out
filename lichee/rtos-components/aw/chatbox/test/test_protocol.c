#include "protocol.h"
#include "console.h"
#include "stdio.h"

protocol_t *g_protocol;

static void test_protocol_rx_audio(void* user_data, const uint8_t* data, size_t size, int is_finished)
{
	printf("%s cb-\n", __func__);


}

static void test_protocol_incoming_json(void* user_data, const cJSON* root)
{
	printf("%s cb-\n", __func__);


}

static void test_protocol_audio_channel_opened(int sample_rate, int channels, void* user_data)
{
	printf("%s cb-\n", __func__);


}

static void test_protocol_audio_channel_closed(void* user_data)
{
	printf("%s cb-\n", __func__);


}

static void test_protocol_network_error(void* user_data, const char* message)
{
	printf("%s cb-\n", __func__);


}

void test_protocol_init(int argc, char *argv[])
{
	g_protocol = protocol_init(WEBSOCKET);

	if (g_protocol == NULL) {
		printf("protocol init failed.\n");
		return ;
	}

#if 1
	protocol_on_incoming_audio(g_protocol, test_protocol_rx_audio);

	protocol_on_incoming_json(g_protocol, test_protocol_incoming_json);

	protocol_on_audio_channel_opened(g_protocol, test_protocol_audio_channel_opened);

	protocol_on_audio_channel_closed(g_protocol, test_protocol_audio_channel_closed);

	protocol_on_network_error(g_protocol, test_protocol_network_error);
#endif

	printf("protocol init success.\n");
}
FINSH_FUNCTION_EXPORT_CMD(test_protocol_init, p_init, protocol test cmd);

void test_protocol_open_audio_ch(int argc, char *argv[])
{
	if (g_protocol == NULL) {
		printf("protocol do not init. init it firstly.\n");
		return ;
	}

	if (protocol_open_audio_channel(g_protocol)) {
		printf("protocol open audio ch failed.\n");
		return ;
	}

	printf("protocol open audio ch success.\n");
}
FINSH_FUNCTION_EXPORT_CMD(test_protocol_open_audio_ch, p_open_audio_ch, protocol test cmd);

void test_protocol_close_audio_ch(int argc, char *argv[])
{
	if (g_protocol == NULL) {
		printf("protocol do not init. init it firstly.\n");
		return ;
	}

	protocol_close_audio_channel(g_protocol);
}
FINSH_FUNCTION_EXPORT_CMD(test_protocol_close_audio_ch, p_close_audio_ch, protocol test cmd);

void test_protocol_send_audio(int argc, char *argv[])
{
	if (g_protocol == NULL) {
		printf("protocol do not init. init it firstly.\n");
		return ;
	}

	uint8_t data[64] = {0};

	for (int i = 0; i < sizeof(data); i ++) {
		data[i] = i;
	}

	int bytes_sent = protocol_send_audio(g_protocol, data, sizeof(data));

	if (bytes_sent < 0) {
		printf("write bin failed.\n");
		return ;
	}

	printf("write audio success. bytes: %d\n", bytes_sent);
}
FINSH_FUNCTION_EXPORT_CMD(test_protocol_send_audio, p_send_audio, protocol test cmd);

void test_protocol_deinit(int argc, char *argv[])
{
	if (g_protocol == NULL) {
		printf("protocol do not init. init it firstly.\n");
		return ;
	}

	if (protocol_deinit(g_protocol))
	{
		printf("protocol deinit failed.\n");
		return ;
	}

	g_protocol = NULL;

	printf("protocol deinit success.\n");
}
FINSH_FUNCTION_EXPORT_CMD(test_protocol_deinit, p_deinit, protocol test cmd);
