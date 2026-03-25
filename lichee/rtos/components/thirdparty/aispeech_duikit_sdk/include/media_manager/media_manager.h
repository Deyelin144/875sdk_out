#ifndef __MEDIA_MANAGER_H__
#define __MEDIA_MANAGER_H__

#include "player_define.h"
#include "player_adapter.h"

#ifdef __cplusplus
extern "C"{
#endif

#define AISPEECH_MEDIA_MANAGER_VERSION    "V1.0.0"

/**
 * @brief 媒体事件
 */
typedef enum
{
    MEDIA_EVENT_STARTED,            // 播放开始
    MEDIA_EVENT_STOPPED,            // 播放停止
    MEDIA_EVENT_PAUSED,             // 播放暂停
    MEDIA_EVENT_RESUMED,            // 播放恢复
    MEDIA_EVENT_COMPLETED,          // 播放完成
    MEDIA_EVENT_SEEKED,             // 跳播成功
    MEDIA_EVENT_ERROR,              // 播放错误
    MEDIA_EVENT_PLAYLIST_CHANGED,   // 播放列表变化
} media_event_t;

/**
 * @brief 媒体错误码
 */
typedef enum
{
    MEDIA_ERROR_UNKNOWN,        // 未知错误
    MEDIA_ERROR_NOT_SUPPORTED,  // 不支持的格式
    MEDIA_ERROR_IO,             // IO错误，如文件不存在
} media_error_code_t;

/**
 * @brief 媒体类型
 */
typedef enum
{
    MEDIA_TTS,                  // tts类型
    MEDIA_MUSIC,                // music类型
} media_type_t;

/**
 * @brief 媒体事件数据
 */
typedef struct
{
    const char *url;            // 播放地址
    int event;                  // 事件类型
    int error;                  // 错误码
    int type;                   // 媒体类型  tts/music
    void *user_data;            // 用户数据指针
} media_event_data_t;

/**
 * @brief 媒体事件回调
 *
 * @param event 事件类型
 * @param
 */
typedef void (*media_event_callback_t)(media_event_data_t *event_data);

/**
 * @brief 媒体管理器初始化配置
 */
typedef struct media_manager_config {
    media_event_callback_t  event_cb;               //事件回调
    player_adapter_t *      music_player;           //音乐播放器实例，由具体平台根据接口定义适配
    player_adapter_t *      tts_player;             //TTS播放器实例，由具体平台根据接口定义适配
    void *                  user_data;              //用户数据指针，随事件回调返回
    int                     main_task_priority;     //主任务优先级
    int                     main_task_stacksize;    //主任务栈大小
    int                     main_task_core_id;      //主任务运行核心ID
    int                     main_task_ext_stack;    //主任务是否使用扩展栈
} media_manager_config_t;

/**
 * @brief 初始化媒体管理器
 *
 * @param config 初始化配置
 *
 * @return 0:成功, 其他:失败
 */
int media_manager_init(media_manager_config_t *config);

/**
 * @brief 去初始化媒体管理器
 *
 * @return 0:成功, 其他:失败
 */
int media_manager_deinit(void);

/**
 * @brief 播放TTS
 *
 * @param url TTS播放地址/文件路径
 * @param auto_play_music TTS播放结束后是否自动播放音乐
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_play(const char *url, bool auto_play_music);

/**
 * @brief 停止播放TTS
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_stop(void);

/**
 * @brief 开始播放TTS
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_start(void);

/**
 * @brief TTS播放下一条
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_next(void);

/**
 * @brief 设置TTS播放列表
 *
 * @param list 播放列表
 * @param len 播放列表长度
 * @param update 是否覆盖更新播放列表
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_set_playlist(music_info_t *list, int len, bool update);

/**
 * @brief 添加TTS信息到播放列表指定位置(前面)， -1表示添加到列表尾部
 *
 * @param item 音乐信息
 * @param pos 添加节点的位置
 *
 * @return 0:成功, 其他:失败
 *
 * @note 如果当前播放列表为空，则添加后自动开始播放
 */
int media_tts_add_item(music_info_t *item, int pos);

/**
 * @brief 删除播放列表指定位置的TTS信息
 *
 * @param pos 删除节点的位置
 *
 * @return 0:成功, 其他:失败
 */
int media_tts_del_item(int pos);

/**
 * @brief 获取TTS播放列表长度
 *
 * @return TTS播放列表长度
 */
int media_tts_get_playlist_len(void);

/**
 * @brief 开始播放音乐
 *
 * @note 需要先设置播放列表
 *
 * @return 0:成功, 其他:失败
 */
int media_music_start(void);

/**
 * @brief 继续播放音乐
 *
 * @note 用于处理添加新音乐列表后没有TTS播报 或 音乐播放被打断后没有TTS播报的场景
 *
 * @return 0:成功, 其他:失败
 */
int media_music_continue(void);

/**
 * @brief 停止音乐播放
 *
 * @return 0:成功, 其他:失败
 */
int media_music_stop(void);

/**
 * @brief 暂停音乐播放
 *
 * @return 0:成功, 其他:失败
 */
int media_music_pause(void);

/**
 * @brief 暂停音乐播放，可以在下一次TTS播报结束后自动恢复播放
 *
 * @return 0:成功, 其他:失败
 */
int media_music_pause2(void);

/**
 * @brief 恢复播放音乐
 *
 * @return 0:成功, 其他:失败
 */
int media_music_resume(void);

/**
 * @brief 音乐播放下一首
 *
 * @param force 是否忽略播放模式强制播放下一首
 *
 * @return 0:成功, 其他:失败
 */
int media_music_next(bool force);

/**
 * @brief 音乐播放上一首
 *
 * @param force 是否忽略播放模式强制播放上一首
 *
 * @return 0:成功, 其他:失败
 */
int media_music_previous(bool force);

/**
 * @brief 音乐快进
 *
 * @param step 快进步长, 单位:秒
 *
 * @return 0:成功, 其他:失败
 */
int media_music_forward(int step);

/**
 * @brief 音乐快退
 *
 * @param step 快退步长, 单位:秒
 *
 * @return 0:成功, 其他:失败
 */
int media_music_backward(int step);

/**
 * @brief 选择音乐播放
 *
 * @param pos 播放列表索引位置,-1表示重新播放当前歌曲
 *
 * @return 0:成功, 其他:失败
 */
int media_music_select_play(int pos);

/**
 * @brief 选择音乐删除
 *
 * @param pos 播放列表索引位置
 *
 * @return 0:成功, 其他:失败
 */
int media_music_select_remove(int pos);

/**
 * @brief 设置音乐播放位置
 *
 * @param position 播放位置, 单位:秒
 *
 * @return 0:成功, 其他:失败
 */
int media_music_set_position(int position);

/**
 * @brief 获取音乐播放位置
 *
 * @return 音乐播放位置, 单位:秒
 */
int media_music_get_position(void);

/**
 * @brief 获取音乐播放时长
 *
 * @return 音乐播放时长, 单位:秒
 */
int media_music_get_duration(void);

/**
 * @brief 获取音乐播放状态
 *
 * @param type 媒体类型
 *
 * @return 音乐播放状态
 */
int media_get_play_status(media_type_t type);

/**
 * @brief 设置音乐播放列表
 *
 * @param list 播放列表
 * @param len 播放列表长度
 * @param update 是否覆盖更新播放列表
 *
 * @return 0:成功, 其他:失败
 */
int media_music_set_playlist(music_info_t *list, int len, bool update);

/**
 * @brief 添加音乐节点信息到播放列表指定位置(前面)
 *
 * @param item 音乐信息
 * @param pos 添加节点的位置
 *
 * @return 0:成功, 其他:失败
 */
int media_music_add_item_to_playlist(music_info_t *item, int pos);

/**
 * @brief 获取音乐信息
 *
 * @param info 播放列表音乐信息，调用者分配好内存再传入
 * @param pos 播放列表节点位置, -1表示获取当前正在播放的音乐信息
 *
 * @return 音乐信息, 获取失败返回NULL
 */
music_info_t *media_music_get_playlist_item(int pos);

/**
 * @brief 获取音乐播放列表长度
 *
 * @return 音乐播放列表长度
 */
int media_music_get_playlist_len(void);

/**
 * @brief 设置音乐播放模式
 *
 * @param mode 播放模式
 *
 * @return 0:成功, 其他:失败
 */
int media_music_set_play_mode(play_mode_t mode);

/**
 * @brief 获取音乐播放模式
 *
 * @return mode 播放模式
 */
int media_music_get_play_mode(void);

/**
 * @brief 切换音乐播放音源
 *
 * @param source 音源
 *
 * @return 0:成功, 其他:失败
 */
int media_music_switch_source(music_source_t source);

#ifdef __cplusplus
}
#endif

#endif //__MEDIA_MANAGER_H__
