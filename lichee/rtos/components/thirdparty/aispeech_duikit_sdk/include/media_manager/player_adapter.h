#ifndef __PLAYER_ADAPTER_H__
#define __PLAYER_ADAPTER_H__

#include "player_define.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief 播放器事件
 */
typedef enum
{
    PLAYER_ADAPTER_EVENT_STARTED,   // 播放开始
    PLAYER_ADAPTER_EVENT_STOPPED,   // 播放停止
    PLAYER_ADAPTER_EVENT_PAUSED,    // 播放暂停
    PLAYER_ADAPTER_EVENT_RESUMED,   // 播放恢复
    PLAYER_ADAPTER_EVENT_COMPLETED, // 播放完成
    PLAYER_ADAPTER_EVENT_SEEKED,    // 跳播成功
    PLAYER_ADAPTER_EVENT_ERROR      // 播放错误
} player_adapter_event_t;

/**
 * @brief 播放器错误码
 */
typedef enum
{
    PLAYER_ADAPTER_ERROR_UNKNOWN,       // 未知错误
    PLAYER_ADAPTER_ERROR_NOT_SUPPORTED, // 不支持的格式
    PLAYER_ADAPTER_ERROR_IO,            // IO错误
} player_adapter_ext_t;

/**
 * @brief 播放器事件回调函数
 *
 * @param url 播放地址/文件路径
 * @param event 播放器事件
 * @param ext 扩展参数, 播放错误时为错误码
 * @param user_data 用户数据
 */
typedef void (*player_adapter_event_callback_t)(const char *url,
        int event, int ext, void *user_data);

/**
 * @brief 播放器适配器
 */
typedef struct
{
    /**
     * @brief 播放器名称
     */
    char *name;

    /**
     * @brief 初始化播放器
     *
     * @param ev_cb 事件回调函数
     * @param user_data 用户数据
     *
     * @return 0:成功, 其他:失败
     */
    int (*init)(player_adapter_event_callback_t ev_cb, void *user_data);

    /**
     * @brief 去初始化播放器
     *
     * @return 0:成功, 其他:失败
     */
    int (*deinit)(void);

    /**
     * @brief 播放
     *
     * @param url 播放地址/文件路径
     * @param position 播放位置，0: 从头开始播放，其他: 从指定位置开始播放
     *
     * @return 0:成功, 其他:失败
     */
    int (*play)(const char *url, int position);

    /**
     * @brief 停止播放
     *
     * @return 0:成功, 其他:失败
     */
    int (*stop)(void);

    /**
     * @brief 暂停播放
     *
     * @return 0:成功, 其他:失败
     */
    int (*pause)(void);

    /**
     * @brief 恢复播放
     *
     * @return 0:成功, 其他:失败
     */
    int (*resume)(void);

    /**
     * @brief 跳播
     *
     * @param position 播放位置, [0, duration]
     *
     * @return 0:成功, 其他:失败
     */
    int (*seek)(int position);

    /**
     * @brief 开始播放音乐列表
     *
     * @return 0:成功, 其他:失败
     */
    int (*start)(void);

    /**
     * @brief 播放下一首
     *
     * @param force 是否忽略播放模式强制播放下一首
     *
     * @return 0:成功, 其他:失败
     */
    int (*next)(bool force);

    /**
     * @brief 播放上一首
     *
     * @param force 是否忽略播放模式强制播放上一首
     *
     * @return 0:成功, 其他:失败
     */
    int (*previous)(bool force);

    /**
     * @brief 设置播放模式
     *
     * @return 0:成功, 其他:失败
     */
    int (*set_play_mode)(play_mode_t mode);

    /**
     * @brief 获取播放模式
     *
     * @return 播放模式
     */
    play_mode_t (*get_play_mode)(void);

    /**
     * @brief 播放列表指定位置的音乐
     *
     * @param pos
     *
     * @return 0:成功, 其他:失败
     */
    int (*select_play)(int pos);

    /**
     * @brief 设置播放位置
     *
     * @return 当前播放位置
     */
    int (*get_position)(void);

    /**
     * @brief 获取播放位置
     *
     * @return 播放位置
     */
    int (*get_duration)(void);
    /**
     * @brief 设置播放列表
     *
     * @param list 播放列表
     * @param len 播放列表长度
     * @param update 是否更新当前播放列表
     *
     * @return 0:成功, 其他:失败
     */
    int (*set_playlist)(music_info_t *list, int len, bool update);

    /**
     * @brief 添加音乐节点信息到播放列表指定位置(前面)
     *
     * @param item 音乐信息
     * @param pos 添加节点的位置
     *
     * @return 0:成功, 其他:失败
     */
    int (*add_item)(music_info_t *item, int pos);

    /**
     * @brief 删除播放列表指定位置的音乐节点信息
     *
     * @param pos 要删除的音乐节点位置
     *
     * @return 0:成功, 其他:失败
     */
    int (*del_item)(int pos);

    /**
     * @brief 从播放列表中获取指定索引位置的音乐信息
     *
     * @param pos 节点位置, -1: 当前播放音乐信息
     *
     * @return 音乐信息
     */
    music_info_t *(*get_playlist_item)(int pos);

    /**
     * @brief 获取播放列表长度
     *
     * @return 播放列表长度
     */
    int (*get_playlist_len)(void);
} player_adapter_t;

#ifdef __cplusplus
}
#endif

#endif //__PLAYER_ADAPTER_H__
