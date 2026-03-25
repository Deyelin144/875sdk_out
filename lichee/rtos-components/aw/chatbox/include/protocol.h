#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string.h>
#include "cJSON.h"

#define CONFIG_WEBSOCKET_URL "wss://api.tenclass.net/xiaozhi/v1/"
#define TOKEN "test-token"

typedef struct protocol protocol_t;

enum abort_reason {
    ABORT_REASON_NONE,
    ABORT_REASON_WAKE_WORD_DETECTED
};

enum listening_mode {
    LISTENING_MODE_AUTO_STOP,
    LISTENING_MODE_MANUAL_STOP,
    LISTENING_MODE_ALWAYS_ON
};

#ifndef NETWORK_PROTOCOL_TYPE
#define NETWORK_PROTOCOL_TYPE
typedef enum {
	WEBSOCKET,
	MQTT
} network_protocol_type;
#endif /* NETWORK_PROTOCOL_TYPE */

void protocol_on_incoming_json(protocol_t* protocol,
		void (*callback)(void* user_data, const cJSON* root));
void protocol_on_incoming_audio(protocol_t* protocol,
		void (*callback)(void* user_data, const uint8_t* data, size_t size, int is_finished));
void protocol_on_audio_channel_opened(protocol_t* protocol,
		void (*callback)(int sample_rate, int channels, void* user_data));
void protocol_on_audio_channel_closed(protocol_t* protocol,
		void (*callback)(void* user_data));
void protocol_on_network_error(protocol_t* protocol,
		void (*callback)(void* user_data, const char* message));

protocol_t *protocol_init(network_protocol_type type);
int protocol_deinit(protocol_t *protocol);

int  protocol_open_audio_channel(protocol_t *protocol);
void protocol_close_audio_channel(protocol_t* protocol);
int  protocol_is_audio_channel_opened(protocol_t* protocol);
int  protocol_send_audio(protocol_t* protocol, const uint8_t* data, size_t size);
void protocol_send_wake_word_detected(protocol_t* protocol, const char* wake_word);
void protocol_send_start_listening(protocol_t* protocol, enum listening_mode mode);
void protocol_send_stop_listening(protocol_t* protocol);
void protocol_send_abort_speaking(protocol_t* protocol, enum abort_reason reason);
void protocol_send_iot_descriptors(protocol_t* protocol, const char* descriptors);
void protocol_send_iot_states(protocol_t* protocol, const char* states);
#endif // PROTOCOL_H
