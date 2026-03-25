#ifndef __PLAYER_DEFINE_H__
#define __PLAYER_DEFINE_H__

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief 播放器播放模式
 */
typedef enum {
    SINGLE_CYCLE,   //单曲循环
    ORDER_CYCLE,    //顺序播放/列表循环
    RANDOM_PLAY,    //随机播放
} play_mode_t;

/**
 * @brief 媒体播放状态
 */
typedef enum
{
    MEDIA_STATE_IDLE,               //未播放
    MEDIA_STATE_PLAYING,            //播放中
    MEDIA_STATE_PAUSED,             //已暂停
} media_play_status_t;

/**
 * @brief 音乐信息
 */
typedef struct
{
    int     index;      //索引
    char *  url;        //播放地址/文件路径
    char *  title;      //歌曲名
    char *  artist;     //歌手
} music_info_t;

/**
 * @brief 音乐播放音源
 */
typedef enum {
    MUSIC_SOURCE_DEFAULT,   //默认音乐播放音源, 通常指云端技能媒体资源播放音源
    //MUSIC_SOURCE_BLUETOOTH, //蓝牙音乐播放音源
    MUSIC_SOURCE_MAX
} music_source_t;

#ifdef __cplusplus
}
#endif

#endif //__PLAYER_DEFINE_H__