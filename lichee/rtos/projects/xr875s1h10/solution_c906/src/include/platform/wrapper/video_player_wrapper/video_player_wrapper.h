#ifndef __VIDEO_PLAYER_WRAPPER_H__
#define __VIDEO_PLAYER_WRAPPER_H__

#include "../../../mod_realize/realize_unit_video_player/realize_unit_video_player.h"

/**
 * @brief 创建播放器
 * @param info
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_create_t)(void **player_handle);

/**
 * @brief 开始播放
 * @param info
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_play_t)(void *player_handle, video_player_base_play_info_t *play_info);

/**
 * @brief 停止播放
 * @param info
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_stop_t)(void *player_handle);

/**
 * @brief 暂停播放
 * @param info
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_pasue_t)(void *player_handle);

/**
 * @brief 恢复播放
 * @param info
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_resume_t)(void *player_handle, char *url);

/**
 * @brief 销毁播放器
 * @param 
 * @return 成功则返回0，失败返回-1
 */
typedef int (*video_player_destroy_t)(void **player_handle);

/**
 * @brief 获取播放器状态
 * @param 
 * @return 
 */
typedef video_player_state_t (*video_player_get_status_t)(void *player_handle);

/**
 * @brief 获取当前播放进度
 * @param 
 * @return 
 */
typedef long (*video_player_get_curr_offset_t)(void *player_handle);

/**
 * @brief 获取歌曲大小
 * @param 
 * @return 
 */
typedef long (*video_player_get_size_t)(void *player_handle);

/**
 * @brief 设置播放偏移
 * @param value
 * @return 
 */
typedef int (*video_player_seek_t)(void *player_handle, long value);

/**
 * @brief 设置视频窗口大小
 * @param win_size
 * @return 
 */
typedef int (*video_player_set_win_size_t)(void *player_handle, video_player_win_size_t *win_size);

/**
 * @brief 设置视频播放速度
 * @param rate
 * @return 
 */
typedef int (*video_player_set_rate_t)(void *player_handle, float rate);

typedef struct {
	video_player_create_t create;
    video_player_play_t play;
	video_player_destroy_t destroy;
	video_player_stop_t stop;
	video_player_pasue_t pasue;
	video_player_resume_t resume;
	video_player_get_status_t get_status;
	video_player_get_curr_offset_t get_curr_offset;
	video_player_get_size_t get_size;
	video_player_seek_t seek;
	video_player_set_win_size_t set_win_size;
    video_player_set_rate_t set_rate;
} video_player_wrapper_t;



#endif // ! __VIDEO_PLAYER_WRAPPER_H__

