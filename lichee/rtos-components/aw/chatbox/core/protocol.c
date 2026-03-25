#include <string.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "transport.h"
#include "event_group.h"
#include "protocol.h"
#include "mac.h"

#undef TAG
#define TAG "protocol"

/************************/
#define pt_malloc malloc
#define pt_free free
#define pt_memset memset
#define pt_memcpy memcpy
/************************/

#define OPUS_FRAME_DURATION_MS 60

/* event */
#define WEBSOCKET_PROTOCOL_CONNECT_SERVER_EVENT (1 << 0)
#define WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT (1 << 1)

struct binary_protocol3 {
    uint8_t type;
    uint8_t reserved;
    uint16_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

enum transport_session_state {
	INIT,
	OPEN,
	WRITE,
	READ,
	CLOSE,
	DEINIT
};

enum audio_channel_state {
	OPENED,
	CLOSED
};

struct transport {
	trans_context_t *ctx;
	trans_session_t *session;
	int session_state;
	trans_context_t *(*init)(network_protocol_type type);
	int (*deinit)(trans_context_t *ctx);
    trans_session_t *(*open)(trans_context_t *ctx, const char *url, const char *ext_h, cb_set_t *cb_set, void *usr_data);
    int (*close)(trans_session_t *session);
    int (*send_text)(trans_session_t *session, const char *message);
    int (*send_bin)(trans_session_t *session, const uint8_t *data, size_t size);
};

struct protocol {
	struct transport trans;

	char session_id[16];
	int server_sample_rate;
	int server_channels;
	struct event_group *evt;
	int audio_channel_state;

	void *on_incoming_audio_usr_data;
	void *on_incoming_json_usr_data;
	void *on_audio_channel_opened_usr_data;
	void *on_audio_channel_closed_usr_data;
	void *on_network_error_usr_data;

	void (*on_incoming_audio)(void* user_data, const uint8_t* data, size_t size, int is_finished);
	void (*on_incoming_json)(void* user_data, const cJSON* root);
	void (*on_audio_channel_opened)(int sample_rate, int channels, void* user_data);
	void (*on_audio_channel_closed)(void* user_data);
	void (*on_network_error)(void* user_data, const char* message);

};

void protocol_parse_server_hello(protocol_t* protocol, const cJSON* root);
int protocol_open(protocol_t *protocol);
int protocol_close(protocol_t *protocol);

static long long last_incoming_time;

static long long time_s(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

	return ts.tv_sec;
}

static int is_timeout(void)
{
	int timeout_s = 120;

	int duration = time_s() - last_incoming_time;

	if (duration > timeout_s) {
		CHATBOX_INFO(TAG"-Channel timeout %lld seconds\n", duration);
		return 1;
	}

	return 0;
}

static void on_session_connected_cb(void *usr_data)
{
	protocol_t *protocol = (protocol_t *)usr_data;

	CHATBOX_INFO(TAG"-session connected\n");

	/* notify */
	event_group_set_bit(protocol->evt, WEBSOCKET_PROTOCOL_CONNECT_SERVER_EVENT);

}

static void on_session_disconnected_cb(void *usr_data)
{
	protocol_t *protocol = (protocol_t *)usr_data;

	CHATBOX_INFO(TAG"-session disconnected\n");

	protocol_close(protocol);

	if (protocol->on_audio_channel_closed) {
		protocol->on_audio_channel_closed(protocol->on_audio_channel_closed_usr_data);
	}

	protocol->audio_channel_state = CLOSED;
}

static void on_session_received_cb(void *usr_data, const uint8_t *data, size_t size, uint8_t type, int is_finished)
{
	protocol_t *protocol = (protocol_t *)usr_data;

	switch (type) {
		case TEXT: {
			CHATBOX_DUMP(TAG"-recv text from session, text: %s\n", data);
			/* process json data */
			cJSON *root = NULL;
			root = cJSON_Parse(data);
			if (root == NULL) {
				CHATBOX_ERROR(TAG"-json parse error. data: %s\n", data);
				return ;
			}

			cJSON *type = cJSON_GetObjectItem(root, "type");
			if (type) {
				if (strcmp(type->valuestring, "hello") == 0) {
					/* parse server hello */
					protocol_parse_server_hello(protocol, (const cJSON *)root);
				} else {
					if (protocol->on_incoming_json) {
						protocol->on_incoming_json(protocol->on_incoming_json_usr_data, root);
					}
				}
			} else {
				CHATBOX_ERROR(TAG"-Missing message type, data: %s", data);
				return ;
			}

			cJSON_Delete(root);
		} break;
		case BIN:
			if (protocol->on_incoming_audio) {
				protocol->on_incoming_audio(protocol->on_incoming_audio_usr_data, data, size, is_finished);
			}
			break;
	}

	last_incoming_time = time_s();

}

protocol_t *protocol_init(network_protocol_type type)
{
	protocol_t *protocol = (protocol_t *)pt_malloc(sizeof(protocol_t));

	if (protocol == NULL) {
		CHATBOX_INFO("malloc(%d) failed.\n", sizeof(protocol_t));
		goto exit0;
	}
	pt_memset(protocol, 0, sizeof(protocol_t));

	/* trans init */
	protocol->trans.init		=	trans_init;
	protocol->trans.deinit		=	trans_deinit;
	protocol->trans.open		=	trans_open;
	protocol->trans.close		=	trans_close;
	protocol->trans.send_text	=	trans_write_text;
	protocol->trans.send_bin	=	trans_write_bin;

	trans_context_t *trans_ctx = protocol->trans.init(type);

	if (trans_ctx == NULL) {
		CHATBOX_INFO("transfer init failed.\n");
		goto exit1;
	}

	protocol->trans.ctx = trans_ctx;

	/* event group init */
	protocol->evt = event_group_create();

	if (protocol->evt == NULL) {
		CHATBOX_INFO("event group create failed.\n");
		goto exit2;
	}

	return protocol;

exit2:
	protocol->trans.deinit(protocol->trans.ctx);
	protocol->trans.ctx = NULL;
exit1:
	pt_free(protocol);
exit0:
	return NULL;
}

int protocol_open(protocol_t *protocol)
{
	/* trans open */
	static cb_set_t cb_set = {
		.recvd_cb			= on_session_received_cb,
		.connected_cb		= on_session_connected_cb,
		.disconnected_cb	= on_session_disconnected_cb
	};

#define URL CONFIG_WEBSOCKET_URL
	char mac[18] = {0};
	if (get_dev_mac(mac, 18)) {
		CHATBOX_ERROR("get mac failed.\n");
		return -1;
	}
	char *uuid = get_uuid();
	if (uuid == NULL) {
		CHATBOX_ERROR("get uuid failed.\n");
		return -1;
	}

	char ext_h[256] = {0};
	snprintf(ext_h, sizeof(ext_h), \
			"Authorization: Bearer %s\r\n"
			"Protocol-Version: 1\r\n"
			"Device-Id: %s\r\n"
			"Client-Id: %s\r\n",
			TOKEN, mac, uuid);

	protocol->trans.session = protocol->trans.open(protocol->trans.ctx, 
			URL, ext_h, &cb_set, protocol);

	if (protocol->trans.session == NULL) {
		CHATBOX_INFO("transfer open failed.\n");
		goto exit0;
	}

exit0:
	return 0;
}

int protocol_close(protocol_t *protocol)
{
	if (protocol == NULL) {
		CHATBOX_ERROR("not init protocol.\n");
		return -1;
	}

	if (protocol->trans.session)
		protocol->trans.close(protocol->trans.session);
	else {
		CHATBOX_ERROR("not open protocol->trans.session.\n");
		return -1;
	}

	protocol->trans.session = NULL;

	CHATBOX_ERROR("protocol close.\n");

	return 0;
}

int protocol_deinit(protocol_t *protocol)
{
	if (protocol == NULL) {
		CHATBOX_ERROR("not init protocol.\n");
		return -1;
	}

	if (protocol->trans.session)
		protocol->trans.close(protocol->trans.session);

	protocol->trans.session = NULL;

	if (protocol->trans.ctx)
		protocol->trans.deinit(protocol->trans.ctx);
	else {
		CHATBOX_ERROR("not open protocol->trans.ctx.\n");
		return -1;
	}

	protocol->trans.ctx = NULL;

	if (protocol->evt)
		event_group_delete(protocol->evt);
	else {
		CHATBOX_ERROR("not create protocol->evt.\n");
		return -1;
	}

	protocol->evt = NULL;

	pt_free(protocol);

	CHATBOX_ERROR("protocol deinit.\n");

	return 0;
}

int protocol_tx_text(protocol_t* protocol, const char *message)
{
	int bytes_sent = 0;

	if (protocol == NULL || message == NULL) {
		CHATBOX_ERROR("invailed param.\n");
		return -1;
	}

	if (protocol->trans.send_text) {
		bytes_sent = protocol->trans.send_text(protocol->trans.session, message);
	}

	return bytes_sent;
}

int protocol_tx_bin(protocol_t* protocol, char *data, size_t data_size)
{
	int bytes_sent = 0;

	if (protocol == NULL || data == NULL || data_size == 0) {
		CHATBOX_ERROR("invailed param.\n");
		return -1;
	}

	if (protocol->trans.send_bin) {
		bytes_sent = protocol->trans.send_bin(protocol->trans.session, data, data_size);
	}

	return bytes_sent;
}

void protocol_on_incoming_json(protocol_t* protocol,
		void (*callback)(void* user_data, const cJSON* root))
{
	protocol->on_incoming_json_usr_data = NULL;

    protocol->on_incoming_json = callback;
}

void protocol_on_incoming_audio(protocol_t* protocol,
		void (*callback)(void* user_data, const uint8_t* data, size_t size, int is_finished))
{
	protocol->on_incoming_audio_usr_data = NULL;

    protocol->on_incoming_audio = callback;
}

void protocol_on_audio_channel_opened(protocol_t* protocol,
		void (*callback)(int sample_rate, int channel, void* user_data))
{
	protocol->on_audio_channel_opened_usr_data = NULL;

    protocol->on_audio_channel_opened = callback;
}

void protocol_on_audio_channel_closed(protocol_t* protocol,
		void (*callback)(void* user_data))
{
	protocol->on_audio_channel_closed_usr_data = NULL;

    protocol->on_audio_channel_closed = callback;
}

void protocol_on_network_error(protocol_t* protocol,
		void (*callback)(void* user_data, const char* message))
{
	protocol->on_network_error_usr_data = NULL;

    protocol->on_network_error = callback;
}

void protocol_parse_server_hello(protocol_t* protocol, const cJSON* root)
{
	cJSON *transport = cJSON_GetObjectItem(root, "transport");
	if (transport == NULL || strcmp(transport->valuestring, "websocket") != 0) {
		CHATBOX_ERROR(TAG"-Unsupported transport: %s", transport->valuestring);
		return;
	}

	cJSON *audio_params = cJSON_GetObjectItem(root, "audio_params");
	if (audio_params != NULL) {
		cJSON *sample_rate = cJSON_GetObjectItem(audio_params, "sample_rate");
		if (sample_rate != NULL) {
			protocol->server_sample_rate = sample_rate->valueint;
		} else {
			CHATBOX_ERROR(TAG"-sample_rate is NULL.\n");
			return;
		}
		cJSON *channels = cJSON_GetObjectItem(audio_params, "channels");
		if (channels != NULL) {
			protocol->server_channels = channels->valueint;
		} else {
			CHATBOX_ERROR(TAG"-channels is NULL.\n");
			return;
		}
	} else {
		CHATBOX_ERROR(TAG"-audio_params is NULL.\n");
		return;
	}

	cJSON *session_id = cJSON_GetObjectItem(root, "session_id");
	if (session_id != NULL) {
		if (strlen(session_id->valuestring) < sizeof(protocol->session_id)) {
			pt_memcpy(protocol->session_id, session_id->valuestring, strlen(session_id->valuestring));
		} else
			CHATBOX_ERROR(TAG"-session_id is too long. %d > 16bytes\n", strlen(session_id->valuestring));
	} else {
		CHATBOX_ERROR(TAG"-session_id is NULL.\n");
		return;
	}

	/* notify */
	event_group_set_bit(protocol->evt, WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT);
}

int protocol_open_audio_channel(protocol_t *protocol)
{
	if (protocol == NULL) {
		CHATBOX_ERROR("invailed param. protocol is NULL.\n");
		return -1;
	}

	if (protocol_open(protocol)) {
		CHATBOX_ERROR("protocol open failed.\n");
		goto exit0;
	}

	/* wait for connecting server */
	{
		CHATBOX_INFO(TAG"-connecting server ...\n");
		unsigned char bits = event_group_wait(protocol->evt, \
				WEBSOCKET_PROTOCOL_CONNECT_SERVER_EVENT, 1, 0, 10000);
		if (!(bits & WEBSOCKET_PROTOCOL_CONNECT_SERVER_EVENT)) {
			CHATBOX_ERROR(TAG"-Failed to connect server. timeout!\n");
			goto exit1;
		}
		CHATBOX_INFO(TAG"-connecting server success.\n");
	}

	/* app protocol handshake */
	// Send hello message to describe the client
	// keys: message type, version, audio_params (format, sample_rate, channels)
	{
		CHATBOX_INFO(TAG"-handshake with server ...\n");
		char message[256] = {0};
		snprintf(message, sizeof(message), \
				"{"
				"\"type\":\"hello\","
				"\"version\": 1,"
				"\"transport\":\"websocket\","
				"\"audio_params\":{"
				"\"format\":\"opus\", \"sample_rate\":16000, \"channels\":1, \"frame_duration\":%d"
				"}}",
				OPUS_FRAME_DURATION_MS);

		if (protocol->trans.send_text(protocol->trans.session, message) < 0) {
			CHATBOX_ERROR("protocol handshake failed.\n");
			goto exit1;
		}

		/* wait for server hello */
		unsigned char bits = event_group_wait(protocol->evt, \
				WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT, 1, 0, 10000);
		if (!(bits & WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT)) {
			CHATBOX_ERROR(TAG"-Failed to receive server hello. timeout!\n");
			goto exit1;
		}
		CHATBOX_INFO(TAG"-handshake with server over.\n");
	}

	if (protocol->on_audio_channel_opened) {
		protocol->on_audio_channel_opened(protocol->server_sample_rate, protocol->server_channels, \
				protocol->on_audio_channel_opened_usr_data);
	}

	protocol->audio_channel_state = OPENED;

	return 0;
exit1:
	protocol_close(protocol);
exit0:
	return -1;
}

void protocol_close_audio_channel(protocol_t* protocol)
{
	if (protocol == NULL) {
		CHATBOX_ERROR("invailed param.\n");
		return ;
	}

	protocol_close(protocol);

	protocol->audio_channel_state = CLOSED;

	return ;
}

int  protocol_is_audio_channel_opened(protocol_t* protocol)
{
	return (protocol->audio_channel_state == OPENED? 1 : 0) || !is_timeout();
}

int protocol_send_audio(protocol_t* protocol, const uint8_t* data, size_t size)
{
	int bytes_sent = 0;

	if (protocol == NULL || data == NULL || size == 0) {
		CHATBOX_ERROR("invailed param.\n");
		return -1;
	}

	if (protocol->trans.send_bin) {
		bytes_sent = protocol->trans.send_bin(protocol->trans.session, data, size);
	}

	return bytes_sent;
}

void protocol_send_abort_speaking(protocol_t* protocol, enum abort_reason reason)
{
    char message[256] = {0};
    snprintf(message, sizeof(message), "{\"session_id\":\"%s\",\"type\":\"abort\"",
			protocol->session_id);
    if (reason == ABORT_REASON_WAKE_WORD_DETECTED) {
        strncat(message, ",\"reason\":\"wake_word_detected\"",
				sizeof(message) - strlen(message) - 1);
    }
    strncat(message, "}", sizeof(message) - strlen(message) - 1);
    protocol->trans.send_text(protocol->trans.session, message);
}

void protocol_send_wake_word_detected(protocol_t* protocol, const char* wake_word)
{
    char json[256] = {0};
    snprintf(json, sizeof(json),
			"{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"detect\",\"text\":\"%s\"}",
			protocol->session_id, wake_word);
    protocol->trans.send_text(protocol->trans.session, json);
}

void protocol_send_start_listening(protocol_t* protocol, enum listening_mode mode)
{
    char message[256] = {0};
    snprintf(message,
			sizeof(message), "{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"start\"",
			protocol->session_id);
    if (mode == LISTENING_MODE_ALWAYS_ON) {
        strncat(message, ",\"mode\":\"realtime\"", sizeof(message) - strlen(message) - 1);
    } else if (mode == LISTENING_MODE_AUTO_STOP) {
        strncat(message, ",\"mode\":\"auto\"", sizeof(message) - strlen(message) - 1);
    } else {
        strncat(message, ",\"mode\":\"manual\"", sizeof(message) - strlen(message) - 1);
    }
    strncat(message, "}", sizeof(message) - strlen(message) - 1);
    protocol->trans.send_text(protocol->trans.session, message);
}

void protocol_send_stop_listening(protocol_t* protocol)
{
    char message[256] = {0};
    snprintf(message, sizeof(message),
			"{\"session_id\":\"%s\",\"type\":\"listen\",\"state\":\"stop\"}",
				protocol->session_id);
    protocol->trans.send_text(protocol->trans.session, message);
}

void protocol_send_iot_descriptors(protocol_t* protocol, const char* descriptors)
{
    cJSON* root = cJSON_Parse(descriptors);
	if (root == NULL) {
		CHATBOX_ERROR(TAG"-Failed to parse IoT descriptors: %s", descriptors);
		return;
	}

	if (!cJSON_IsArray(root)) {
		CHATBOX_ERROR(TAG"-IoT descriptors should be an array");
		cJSON_Delete(root);
		return;
	}

	int arraySize = cJSON_GetArraySize(root);
	for (int i = 0; i < arraySize; ++i) {
		cJSON* descriptor = cJSON_GetArrayItem(root, i);
		if (descriptor == NULL) {
			CHATBOX_ERROR(TAG"-Failed to get IoT descriptor at index %d", i);
			continue;
		}

		cJSON* messageRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(messageRoot, "session_id", protocol->session_id);
		cJSON_AddStringToObject(messageRoot, "type", "iot");
		cJSON_AddBoolToObject(messageRoot, "update", true);

		cJSON* descriptorArray = cJSON_CreateArray();
		cJSON_AddItemToArray(descriptorArray, cJSON_Duplicate(descriptor, 1));
		cJSON_AddItemToObject(messageRoot, "descriptors", descriptorArray);

		char* message = cJSON_PrintUnformatted(messageRoot);
		if (message == NULL) {
			CHATBOX_ERROR(TAG"-Failed to print JSON message for IoT descriptor at index %d", i);
			cJSON_Delete(messageRoot);
			continue;
		}

		protocol->trans.send_text(protocol->trans.session, message);

		cJSON_free(message);
		cJSON_Delete(messageRoot);
	}

	cJSON_Delete(root);
}

void protocol_send_iot_states(protocol_t* protocol, const char* states)
{
    char message[256] = {0};
    snprintf(message,
			sizeof(message), "{\"session_id\":\"%s\",\"type\":\"iot\",\"states\":%s}",
				protocol->session_id, states);
    protocol->trans.send_text(protocol->trans.session, message);
}

