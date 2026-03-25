/* application.h */
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#if __cplusplus
}; // extern "C"
#endif

#define RECORD_AUDIO_SAMPLE         (16000)
#define RECORD_AUDIO_CHANNEL        (1)
#define PLAY_AUDIO_SAMPLE           (24000)
#define PLAY_AUDIO_CHANNEL          (1)

#define OPUS_FRAME_DURATION_MS      (60)
#define SAMPLE_BYTES                (2)        // 16bit

//input audio frame
#define RECORD_FRAME_SAMPLES       (RECORD_AUDIO_SAMPLE * OPUS_FRAME_DURATION_MS / 1000)
#define RECORD_FRAME_SIZE          (RECORD_FRAME_SAMPLES * RECORD_AUDIO_CHANNEL * SAMPLE_BYTES)

//output audio frame
#define PLAY_FRAME_SAMPLES         (PLAY_AUDIO_SAMPLE * OPUS_FRAME_DURATION_MS / 1000)
#define PLAY_FRAME_SIZE            (PLAY_FRAME_SAMPLES * PLAY_AUDIO_CHANNEL * SAMPLE_BYTES)

#define OPUS_DECODE_SIZE            (4096)
#define RECORD_BUFF_SIZE            (RECORD_AUDIO_SAMPLE * 2 * 5)
#define PLAY_BUFF_SIZE              (PLAY_AUDIO_SAMPLE * 2 * 5)

#define APP_MAX_DELAY 				0xFFFFFFFF

#define AUIDO_PROCESS_ENABLE        1
#define AUDIO_STORAGE_ENABLE        1
#define REALTIME_MODE_ENABLE        1


#if AUDIO_STORAGE_ENABLE
#define AUDIO_RECORD_PATH       "/data/audio_record.pcm"
#define AUDIO_PLAY_PATH      "/data/audio_play.pcm"
#endif

enum chatbox_mode {
    CHAT_MODE_AUTO,
    CHAT_MODE_MANUAL,
    CHAT_MODE_REALTIME
};

void app_start(void);

void app_stop(void);

void app_start_listening(void);

void app_stop_listening(void);

void app_storage_enable(char enable);

void app_set_chatmode(enum chatbox_mode mode);

#if __cplusplus
}; // extern "C"
#endif

#endif /* __APPLICATION_H__ */
