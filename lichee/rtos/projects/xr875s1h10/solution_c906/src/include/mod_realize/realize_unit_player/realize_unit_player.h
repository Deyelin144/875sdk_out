#ifndef _REALIZE_UNIT_PLAYER_H_
#define _REALIZE_UNIT_PLAYER_H_

#define PLAYER_STREAM_MODE "this is a stream player"

#define PLAYER_EXEC_THREAD_STACK 1024 * 2 + 512
#define PLAYER_RECV_MAG_THREAD_STACK 1024 * 2

#define MEDIA_URL_LEN 512
#define MEDIA_NAME_LEN 256

typedef enum {
	MSG_PLAY,
	MSG_STOP,
	MSG_PAUSE,
	MSG_RESUME,
	MSG_SEEK,
	MSG_EXIT,
} media_msg_t;

typedef enum {
	PLAYER_PLAY_NOT_BLOCK = 0,					//不阻塞播放
	PLAYER_PLAY_BLOCK,							//阻塞播放
	PLAYER_FORCED_RELEASE_BLOCK_AND_BLOCK,		//强行打断前面阻塞后获得播放器阻塞播放
	PLAYER_FORCED_RELEASE_BLOCK_AND_NOT_BLOCK,	//强行打断前面阻塞后获得播放器不阻塞播放
	PLATER_FORCED_PAUSE_BLOCK,					//强行打断前面阻塞后阻塞暂停播放器
	PLATER_FORCED_STOP_BLOCK,					//强行打断前面阻塞后阻塞停止播放器
	PLATER_FORCED_PAUSE,						//强行打断前面阻塞后暂停播放器
	PLATER_FORCED_STOP,							//强行打断前面阻塞后停止播放器
	PLATER_FORCED_NUM = 0XFFFFFFFE,				//扩展枚举大小为32位
} player_block_t;

typedef enum {
	MEDIA_PLAYER_NOT_PAUSED = 0,
	MEDIA_PLAYER_PAUSED,
} player_pause_status_t;

typedef struct {
	int (*player_play_cb)(void *info);
	int (*player_process_cb)(void *info, int player_status);
	int (*player_stop_cb)(void *info, int completed_normally);
	int (*player_error_cb)(void *info);
} player_play_cb_t;

typedef struct {
	char audio_format[12];
	unsigned int channels;
	unsigned int bits;
	unsigned int samplerate;
} media_audio_info_t;

typedef struct {
	char *data;
	unsigned int data_len;
	char exit_immediately;
} media_stream_msg_t;

typedef struct {
	float rate;
	unsigned int end_time;
	unsigned int play_offset;
	unsigned int total_offset;
	char url[MEDIA_URL_LEN + 1];
    char name[MEDIA_NAME_LEN + 1];
	unsigned int stop_statue;
	unsigned char block;
	media_audio_info_t audio_info;
} base_play_info_t;

typedef struct {
	unsigned char is_create;
	player_pause_status_t paused;
	base_play_info_t curr_info;
} player_record_t;

typedef struct {
	media_msg_t cmd;
	base_play_info_t info;
} media_msg_data_t;

int _realize_unit_player_send_msg(const char* name, int line, media_msg_t cmd, const char *url, player_block_t block, const char *audio_name);
#define realize_unit_player_send_msg(cmd, url, block, param)  _realize_unit_player_send_msg(__func__ , __LINE__, cmd, url, block, param)
void realize_unit_player_init(player_play_cb_t *play_cb);
void realize_unit_player_deinit();
base_play_info_t *realize_unit_player_get_cur_info();
int _realize_unit_player_stream_send(const char *name, int line, media_stream_msg_t *msg);
#define realize_unit_player_stream_send(msg) _realize_unit_player_stream_send(__func__ , __LINE__, msg)
int realize_unit_player_stream_is_full(void);
void realize_unit_player_set_info(base_play_info_t *play_info);

#endif