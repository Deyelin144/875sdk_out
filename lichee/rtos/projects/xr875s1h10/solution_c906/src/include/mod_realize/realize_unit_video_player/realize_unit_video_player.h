#ifndef _REALIZE_UNIT_VIDEO_PLAYER_H_
#define _REALIZE_UNIT_VIDEO_PLAYER_H_

#define VIDEO_PLAYER_EXEC_THREAD_STACK 1024 * 2 + 512
#define VIDEO_PLAYER_RECV_MAG_THREAD_STACK 1024 * 2

#define VIDEO_PLAYER_URL_LEN 512
#define VIDEO_PLAYER_NAME_LEN 256

typedef enum {
	VIDEO_PLAYER_PLAY,
	VIDEO_PLAYER_STOP,
	VIDEO_PLAYER_PAUSE,
	VIDEO_PLAYER_RESUME,
	VIDEO_PLAYER_SEEK,
    VIDEO_PLAYER_RATE,
    VIDEO_PLAYER_WIN_SIZE,
	VIDEO_PLAYER_EXIT,
} video_player_msg_t;

typedef enum {
    VIDEO_PLAYER_STATUS_PAUSE = 0, //APLAYER_STATES_PAUSE,
    VIDEO_PLAYER_STATUS_PLAYING = 1, //APLAYER_STATES_PLAYING,
    VIDEO_PLAYER_STATUS_PREPARING = 2, //APLAYER_STATES_PREPARING,
    VIDEO_PLAYER_STATUS_PREPARED = 3, //APLAYER_STATES_PREPARED,
    VIDEO_PLAYER_STATUS_STOPPED = 4,  //APLAYER_STATES_STOPPED,
    VIDEO_PLAYER_STATUS_RESUME = 5, //APLAYER_STATES_RESUME,
    VIDEO_PLAYER_STATUS_INIT = 6, //APLAYER_STATES_INIT,
    VIDEO_PLAYER_STATUS_NONE = 7, //APLAYER_STATES_NONE,
    VIDEO_PLAYER_STATUS_COMPLETED = 8, //APLAYER_STATES_COMPLETED,
    VIDEO_PLAYER_STATUS_ERROR = 9, //APLAYER_STATES_ERROR,
} video_player_state_t;

typedef enum {
	VIDEO_PLAYER_NOT_PAUSED = 0,
	VIDEO_PLAYER_PAUSED,
} video_player_pause_status_t;

typedef enum {
	/* Display in the window at the original size of the video,
	 * can't overflow the window */
	VIDEO_PLAYER_ORIGINAL_SIZE,
	/* Scale to full screen by video ratio, the video show normal */
	VIDEO_PLAYER_FULL_SCREEN_TO_VIDEO_WINDOW,
	/* Scale to full screen by screen ratio, the video may be distorted */
	VIDEO_PLAYER_FULL_SCREEN_TO_SCREEN_SIZE,
	/* Forced to display at 4:3 ratio, the video may be distorted */
	VIDEO_PLAYER_4R3MODE,
	/* Forced to display at 16:9 ratio, the video may be distorted */
	VIDEO_PLAYER_16R9MODE,
	/* User defined mode */
	VIDEO_PLAYER_USER_DEFINED,
} video_player_scaled_mode_e;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	video_player_scaled_mode_e scale_mode;
} video_player_win_size_t;

typedef struct {
	int (*video_player_play_cb)(void *info);
	int (*video_player_process_cb)(void *info, int player_status);
	int (*video_player_stop_cb)(void *info, char completed_normally);
	int (*video_player_error_cb)(void *info);
} video_player_play_cb_t;

typedef struct {
	char url[VIDEO_PLAYER_URL_LEN + 1];
    char name[VIDEO_PLAYER_NAME_LEN + 1];
	video_player_win_size_t win_size;
	double rate;
	unsigned int play_ofs;
	unsigned int total_size;
	unsigned int play_statue;
	int timeout;
} video_player_base_play_info_t;

typedef int (*video_player_create)(void *ctx, video_player_play_cb_t *play_cb);
typedef int (*video_player_send_msg)(void *ctx, video_player_msg_t cmd, char *url, char *arg, const char *name, int line);
typedef video_player_base_play_info_t *(*video_player_get_cur_info)(void *ctx);
typedef int (*video_player_destroy)(void *ctx);

typedef struct {
	void *ctx;
	video_player_create create;
	video_player_send_msg send_msg;
	video_player_get_cur_info get_cur_info;
	video_player_destroy destroy;
} video_player_obj_t;

video_player_obj_t *realize_unit_video_player_new(void);
void realize_unit_video_player_delete(video_player_obj_t **video_palyer_obj);


#endif