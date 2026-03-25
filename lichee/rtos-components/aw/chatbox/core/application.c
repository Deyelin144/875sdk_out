#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "application.h"
#include "task.h"
#include "log.h"
#include "event_group.h"
#include "queue.h"
#include "protocol.h"
#include "opus_wraper.h"
#include "audio_codec.h"
#include "schedule.h"
#include "ringbuffer.h"
#include "iot.h"
#include "list_buff.h"
#include "sys_time.h"
#include "display.h"
#include "lang_config.h"

#if AUIDO_PROCESS_ENABLE
#include "audio_process.h"
#endif

#if AUDIO_STORAGE_ENABLE
#include "storage.h"
#endif


enum system_events {
    AUDIO_INPUT_EVENT  = (1 << 0),
    AUDIO_OUTPUT_EVENT = (1 << 1),
    SCHEDULER_EVENT    = (1 << 2)
};

enum chatbox_state {
	CB_IDLE,
	CB_CONNECTING,
	CB_LISTENING,
	CB_SPEAKING,
	CB_ABORTING,
};

struct wrapper_buff {
	void *data;
	int size;
};

struct chatbox_priv {
	struct event_group *evt_gp;
	struct thread *app_thread;
	struct audio_codec *codec;
	struct ringbuff *record_rb;
	struct list_buff *play_lb;
	uint32_t last_input_time;
	uint32_t input_timeout_ms;
	struct opus_wraper_decoder *opus_dec;
	uint8_t dec_pcm[OPUS_DECODE_SIZE];
	struct opus_wraper_encoder *opus_enc;

#if AUIDO_PROCESS_ENABLE
	struct audio_process *audio_p;
#endif
	struct schedule *sched;
	enum chatbox_state ch_state;
	struct protocol *protocol;
#if AUDIO_STORAGE_ENABLE
    struct storage *play_storage;
    struct storage *record_storage;
#endif
	enum chatbox_mode chatmode;
	uint32_t vad_last_time;
	uint8_t vad_trigger_count;
	uint8_t vad_trigger_threshold;
	uint32_t vad_detection_window;

};

static struct chatbox_priv *app_priv = NULL;
static int app_schedule_post(struct schedule* sched,
	 schedule_callback_t fn, void* arg);
static void app_handle_state_change(void *user_data);
const char *chatbox_state_to_string(enum chatbox_state state)
{
	switch (state) {
	case CB_IDLE:
		return "CB_IDLE";
	case CB_CONNECTING:
		return "CB_CONNECTING";
	case CB_LISTENING:
		return "CB_LISTENING";
	case CB_SPEAKING:
		return "CB_SPEAKING";
	case CB_ABORTING:
		return "CB_ABORTING";
	}
	return "UNKNOWN";
}

static struct wrapper_buff *app_get_wrapper_buff(void *data, int size)
{
	struct wrapper_buff *buff;

	buff = (struct wrapper_buff *)malloc(sizeof (struct wrapper_buff));
	if (!buff) {
		CHATBOX_ERROR("Failed to allocate memory for wrapper_buff\n");
		return NULL;
	}
	buff->data = malloc(size);
	if (!buff->data) {
		CHATBOX_ERROR("Failed to allocate memory for buff->data\n");
		free(buff);
		return NULL;
	}
	memcpy(buff->data, data, size);
	buff->size = size;
	return buff;
}

static void app_free_wrapeer_buff(struct wrapper_buff *buff)
{
	if (buff) {
		if (buff->data)
			free(buff->data);
		free(buff);
	}
}

static void app_opus_encode_cb(uint8_t *opus, size_t opus_size)
{
	protocol_send_audio(app_priv->protocol, opus, opus_size);
}

static void app_handle_vad_state_change(int is_vad)
{
	// VAD filter
	if (is_vad) {
		uint32_t current_time = sys_time_now();
		if (current_time - app_priv->vad_last_time > app_priv->vad_detection_window) {

			app_priv->vad_trigger_count = 1;
		} else {

			app_priv->vad_trigger_count++;
		}

		app_priv->vad_last_time = current_time;

		if (app_priv->vad_trigger_count >= app_priv->vad_trigger_threshold) {
			if (app_priv->ch_state == CB_IDLE && is_vad) {
				app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_CONNECTING);
				app_priv->vad_trigger_count = 0;
		}
#if 0
			if (app_priv->ch_state == CB_SPEAKING && is_vad) {
				CHATBOX_INFO("size %d, vad:%d\n", size, is_vad);
				app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_ABORTING);
				app_priv->vad_trigger_count = 0;
			}
#endif
		}
	} else {
		app_priv->vad_trigger_count = 0;
	}
}

#if AUIDO_PROCESS_ENABLE
static int app_audio_process_cb(uint8_t *data, int size, int is_vad)
{
	app_handle_vad_state_change(is_vad);
	opus_wraper_encoder_encode(app_priv->opus_enc, data,
		size, app_opus_encode_cb);
}
#endif
static void app_input_audio(void)
{
    static uint8_t process_buffer[RECORD_FRAME_SIZE];
    const uint32_t frame_size = RECORD_FRAME_SIZE;
	uint32_t current_time = sys_time_now();
	uint32_t available = 0;

	available = ringbuff_available(app_priv->record_rb);

	if (available >= frame_size ||
	   (available > 0 && (current_time - app_priv->last_input_time) >= app_priv->input_timeout_ms)) {

	    uint32_t read_size = (available >= frame_size) ? frame_size : available;
	    int read = ringbuff_read(app_priv->record_rb, process_buffer, read_size);
#if 0
	    if (read < frame_size) {
	        memset(process_buffer + read, 0, frame_size - read);
	        CHATBOX_DEBUG("Padding %d bytes silence\n", frame_size - read);
	    }
#endif
	CHATBOX_DUMP("read audio:%d, state:%s\n", read, chatbox_state_to_string(app_priv->ch_state));
#if AUDIO_STORAGE_ENABLE
    if(app_priv->record_storage) {
       storage_write(app_priv->record_storage, process_buffer, read);
    }
#endif

#if AUIDO_PROCESS_ENABLE
        audio_process_input(app_priv->audio_p, process_buffer, frame_size);
#else
        opus_wraper_encoder_encode(app_priv->opus_enc,
            process_buffer, read, app_opus_encode_cb);
#endif
	    app_priv->last_input_time = current_time;
	}
}

static void app_output_audio(void)
{
	void *data = NULL;
	unsigned int len = 0;
	size_t pcm_len = 0;

	list_buff_pop(app_priv->play_lb, &data, &len, 0);
	if (len <= 0)
		return;

	opus_wraper_decoder_decode(app_priv->opus_dec,
		data, len,
		app_priv->dec_pcm, OPUS_DECODE_SIZE, &pcm_len);

	list_buff_free_pop_buff(data);

	if (pcm_len > OPUS_DECODE_SIZE) {
		CHATBOX_ERROR("decode buffer over flow %d > %d\n", pcm_len, OPUS_DECODE_SIZE);
		pcm_len = OPUS_DECODE_SIZE;
	}

	CHATBOX_DUMP("app output audio, enc len:%d, dec len:%d\n", len, pcm_len);

#if AUDIO_STORAGE_ENABLE
    if(app_priv->play_storage && pcm_len > 0) {
        storage_write(app_priv->play_storage, app_priv->dec_pcm, pcm_len);
    }
#endif
	if (pcm_len > 0) {
		audio_codec_output_data(app_priv->codec,
			app_priv->dec_pcm, pcm_len);
	}
}
static void app_main_loop(void *data)
{
	unsigned char evt;

	CHATBOX_INFO("app main loop\n");

	while (1) {

		evt = event_group_wait(app_priv->evt_gp,
			AUDIO_INPUT_EVENT | AUDIO_OUTPUT_EVENT | SCHEDULER_EVENT,
			AUDIO_INPUT_EVENT | AUDIO_OUTPUT_EVENT | SCHEDULER_EVENT, 0, APP_MAX_DELAY);

		if (evt & AUDIO_OUTPUT_EVENT) {
			app_output_audio();
		}

		if (evt & AUDIO_INPUT_EVENT) {
			app_input_audio();
		}
		if (evt & SCHEDULER_EVENT) {
			schedule_run(app_priv->sched, NULL);
		}
	}
}

static int app_audio_receive_cb(uint8_t *data, int size, int is_vad)
{
	int written;
#if !AUIDO_PROCESS_ENABLE
	app_handle_vad_state_change(is_vad);
#endif

	if (app_priv->ch_state != CB_LISTENING &&
		app_priv->chatmode == CHAT_MODE_AUTO ) {
		return size;
	}

	if(app_priv->ch_state == CB_CONNECTING)
		return size;

    written = ringbuff_write(app_priv->record_rb, data, size);
    if (written >= 0) {
        event_group_set_bit(app_priv->evt_gp, AUDIO_INPUT_EVENT);
    }

	if (written != size) {
		CHATBOX_WARNG("written %d bytes, expect %d bytes\n", written, size);
	}

    return written == size ? 0 : -1;
}

static void app_abort_speaking(enum abort_reason reason, enum chatbox_state old_state)
{
	protocol_send_abort_speaking(app_priv->protocol, reason);
	audio_codec_stop_output(app_priv->codec);
	list_buff_clear(app_priv->play_lb, free);
	opus_wraper_decoder_reset_state(app_priv->opus_dec);
	opus_wraper_encoder_reset_state(app_priv->opus_enc);

	app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_LISTENING);
}
static int app_keep_listening(void)
{
	//TODO,
	return 1;
}
static void app_state_change(enum chatbox_state new_state)
{
	enum chatbox_state old_state = app_priv->ch_state;
	if (!app_priv || !app_priv->evt_gp) return;

	CHATBOX_INFO("state change %s -->%s \n",
			chatbox_state_to_string(old_state),
			chatbox_state_to_string(new_state));

	app_priv->ch_state = new_state;

	switch(app_priv->ch_state) {
		case CB_IDLE:
			display_set_status(lang_strings[STANDBY]);
			display_set_emotion("neutral");
			list_buff_clear(app_priv->play_lb, free);
			ringbuff_reset(app_priv->record_rb);
			if (old_state == CB_LISTENING) {
				protocol_send_stop_listening(app_priv->protocol);
			}
			break;
		case CB_CONNECTING:
			display_set_status(lang_strings[CONNECTING]);
			display_set_emotion("neutral");
			display_set_chat_message("system", "");
			if (old_state == CB_IDLE) {
				protocol_open_audio_channel(app_priv->protocol);
				if (app_priv->chatmode == CHAT_MODE_REALTIME)
					protocol_send_start_listening(app_priv->protocol, LISTENING_MODE_ALWAYS_ON);
			}
			break;
		case CB_LISTENING:
			display_set_status(lang_strings[LISTENING]);
			display_set_emotion("neutral");
			if (old_state == CB_IDLE ||
				old_state == CB_SPEAKING ||
				old_state == CB_CONNECTING || old_state == CB_ABORTING) {
				if (app_priv->chatmode == CHAT_MODE_AUTO)
					protocol_send_start_listening(app_priv->protocol, LISTENING_MODE_AUTO_STOP);
			}
			break;
		case CB_SPEAKING:
			display_set_status(lang_strings[SPEAKING]);
			break;
		case CB_ABORTING:
			if (old_state == CB_SPEAKING)
				app_abort_speaking(ABORT_REASON_NONE, old_state);
			break;
	}
}

static void app_handle_state_change(void *user_data)
{
	enum chatbox_state new_state = (enum chatbox_state)user_data;
	app_state_change(new_state);
}

static void app_handle_tts_sentence_start(void *user_data)
{
	char *text = (char *)user_data;
	display_set_chat_message("assistant", text);
	free(text);
}
static void app_handle_sst(void *user_data)
{
	char *text = (char *)user_data;
	display_set_chat_message("user", text);
	free(text);
}
static void app_handle_llm(void *user_data)
{
	char *text = (char *)user_data;
	display_set_emotion(text);
	free(text);
}

static int app_schedule_post(struct schedule* sched,
	 schedule_callback_t fn, void* arg)
{
	schedule_post(sched, fn, arg);
	event_group_set_bit(app_priv->evt_gp, SCHEDULER_EVENT);
	return 0;
}
static char* copy_json_string(cJSON *item)
{
    char *str = malloc(strlen(item->valuestring) + 1);
    if (str) strcpy(str, item->valuestring);
    return str;
}
static void app_protocol_incoming_json(void *user_data, const cJSON *root)
{
	CHATBOX_DUMP("incoming json\n");

	cJSON *type = cJSON_GetObjectItem(root, "type");

	if (!type) return;

	if (strcmp(type->valuestring, "tts") == 0) {
		cJSON *state = cJSON_GetObjectItem(root, "state");
		if (!state) return;
		if (strcmp(state->valuestring, "start") == 0) {
			if (app_priv->ch_state == CB_IDLE ||
				app_priv->ch_state == CB_LISTENING) {
				app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_SPEAKING);
			}
		} else if (strcmp(state->valuestring, "stop") == 0) {
			if(app_priv->ch_state == CB_SPEAKING) {
				if (app_keep_listening()) {
					app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_LISTENING);
				} else {
					app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_IDLE);
				}
			}
		} else if (strcmp(state->valuestring, "sentence_start") == 0) {
			cJSON *text = cJSON_GetObjectItem(root, "text");
			if (text) {
				char *text_str = copy_json_string(text);
				app_schedule_post(app_priv->sched,
					app_handle_tts_sentence_start, (void*)text_str);
			}
		}
	} else if (strcmp(type->valuestring, "stt") == 0) {
		cJSON * text = cJSON_GetObjectItem(root, "text");
		if (text) {
			char *text_str = copy_json_string(text);
			app_schedule_post(app_priv->sched, app_handle_sst, (void*)text_str);
		}
	} else if (strcmp(type->valuestring, "llm") == 0) {
		cJSON * emotion = cJSON_GetObjectItem(root, "emotion");
		if (emotion != NULL) {
			char *text_str = copy_json_string(emotion);
			app_schedule_post(app_priv->sched, app_handle_llm, (void*)text_str);
		}
	} else if (strcmp(type->valuestring, "iot") == 0) {
		//TODO, handle iot
	}
}

static void app_protocol_rx_audio(void *user_data, const uint8_t *data, size_t size, int is_finished)
{
	int ret;

	CHATBOX_DUMP("rx play audio %d bytes, state:%s\n",
			size, chatbox_state_to_string(app_priv->ch_state));

	if (!is_finished) {
		CHATBOX_WARNG("rx play audio is frag, %d\n", size);
	}

	ret = list_buff_append(app_priv->play_lb, (void *)data, size, BUFF_STORE_COPY);

    if (ret == 0) {
        event_group_set_bit(app_priv->evt_gp, AUDIO_OUTPUT_EVENT);
    }
}

static void app_protocol_audio_channel_opened(int sample_rate, int channels, void* user_data)
{
	CHATBOX_INFO("audio channel opened\n");

	if (app_priv->ch_state == CB_CONNECTING)
		app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_LISTENING);

	protocol_send_iot_descriptors(app_priv->protocol, generate_iot_descript());
}


static void app_protocol_audio_channel_closed(void *user_data)
{
	app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_IDLE);
}

static void app_protocol_network_error(void *user_data, const char *message)
{
	app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_IDLE);
}

void app_start_listening(void)
{
	if (app_priv->ch_state == CB_LISTENING)
		return;

	if (app_priv->ch_state == CB_IDLE)
		app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_CONNECTING);

	if (app_priv->ch_state == CB_SPEAKING)
		app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_LISTENING);
}

void app_stop_listening(void)
{
	if (app_priv->ch_state == CB_IDLE)
		return;

	if (app_priv->ch_state == CB_LISTENING | app_priv->ch_state == CB_SPEAKING)
		app_schedule_post(app_priv->sched, app_handle_state_change, (void *)CB_IDLE);
}

void app_storage_enable(char enable)
{
#if AUDIO_STORAGE_ENABLE
    if(app_priv->record_storage) storage_enable(app_priv->record_storage, enable);
    if(app_priv->play_storage) storage_enable(app_priv->play_storage, enable);
#endif
}

void app_set_chatmode(enum chatbox_mode mode)
{
	if (app_priv) {
		CHATBOX_INFO("set chat mode to %d\n", mode);
		app_priv->chatmode = mode;
	}
}
void app_start(void)
{
	CHATBOX_INFO("app start\n");
	app_priv = (struct chatbox_priv *)malloc(sizeof (struct chatbox_priv));
	if (!app_priv) {
		CHATBOX_ERROR("no mem, create app priv failed\n");
		return ;
	}

	app_priv->ch_state = CB_IDLE;

	display_init();

	app_priv->evt_gp = event_group_create();

	app_priv->sched = schedule_create();

	app_priv->record_rb = ringbuff_init(RECORD_BUFF_SIZE);

	app_priv->play_lb = list_buff_create();

	app_priv->input_timeout_ms = 100;

	app_priv->app_thread = thread_create(app_main_loop,
			NULL, "main_loop", 4096, 18);

	app_priv->opus_enc = opus_wraper_encoder_create(RECORD_AUDIO_SAMPLE,
			RECORD_AUDIO_CHANNEL, OPUS_FRAME_DURATION_MS);

	app_priv->opus_dec = opus_wraper_decoder_create(PLAY_AUDIO_SAMPLE,
			PLAY_AUDIO_CHANNEL, OPUS_FRAME_DURATION_MS);

#if AUDIO_STORAGE_ENABLE
    app_priv->play_storage = audio_storage_init(AUDIO_PLAY_PATH);
    app_priv->record_storage = audio_storage_init(AUDIO_RECORD_PATH);
#endif

#if AUIDO_PROCESS_ENABLE
	//AEC/NS/AEC/AGC/VAD initinit
	app_priv->audio_p = audio_process_init(RECORD_AUDIO_SAMPLE, RECORD_AUDIO_CHANNEL);
	audio_process_register_output(app_priv->audio_p, app_audio_process_cb);
#endif

	app_priv->codec = audio_codec_init(PLAY_AUDIO_SAMPLE, PLAY_AUDIO_CHANNEL,
		RECORD_AUDIO_SAMPLE, RECORD_AUDIO_CHANNEL, app_audio_receive_cb);

	app_priv->vad_last_time = 0;
	app_priv->vad_trigger_count = 0;
	app_priv->vad_trigger_threshold = 3; 
	app_priv->vad_detection_window = 500;

	app_priv->protocol = protocol_init(WEBSOCKET);

	protocol_on_incoming_audio(app_priv->protocol, app_protocol_rx_audio);

	protocol_on_incoming_json(app_priv->protocol, app_protocol_incoming_json);

	protocol_on_audio_channel_opened(app_priv->protocol,
			app_protocol_audio_channel_opened);

	protocol_on_audio_channel_closed(app_priv->protocol,
			app_protocol_audio_channel_closed);

	protocol_on_network_error(app_priv->protocol, app_protocol_network_error);
}

void app_stop(void)
{
	CHATBOX_INFO("app stop\n");

	if (!app_priv) return ;

	protocol_close_audio_channel(app_priv->protocol);
	protocol_deinit(app_priv->protocol);
	audio_codec_deinit(app_priv->codec);

	ringbuff_deinit(app_priv->record_rb);
	list_buff_destroy(app_priv->play_lb, free);

	opus_wraper_encoder_destroy(app_priv->opus_enc);
	opus_wraper_decoder_destroy(app_priv->opus_dec);

#if AUIDO_PROCESS_ENABLE
	audio_process_deinit(app_priv->audio_p);
#endif

#if AUDIO_STORAGE_ENABLE
    storage_deinit(app_priv->play_storage);
    storage_deinit(app_priv->record_storage);
#endif
	schedule_destroy(app_priv->sched);
	thread_stop(app_priv->app_thread);
	event_group_delete(app_priv->evt_gp);
	free(app_priv);

	display_deinit();
	app_priv = NULL;
}
