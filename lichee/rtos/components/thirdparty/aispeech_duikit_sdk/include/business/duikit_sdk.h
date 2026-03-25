#ifndef DUIKIT_SDK_H
#define DUIKIT_SDK_H

#include <stdbool.h>

#include "duikit_dsp_adapter.h"
#include "duikit_dialog_process.h"
#include "duikit_offline_process.h"

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
#include "network_manager.h"
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
#include "media_manager.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DUI_SDK_VERSION "1.0.4"

/**
 * @brief DUIKit SDK 事件类型定义
 */
typedef enum
{
    DUIKIT_SDK_EVENT_DIALOG_ONLINE,     //在线对话事件
    DUIKIT_SDK_EVENT_DIALOG_OFFLINE,    //离线对话事件
    DUIKIT_SDK_EVENT_NETWORK,           //网络事件
    DUIKIT_SDK_EVENT_MEDIA,             //媒体事件
    DUIKIT_SDK_EVENT_DSP,               //DSP事件
} duikit_sdk_event_type_t;

/**
 * @brief DUIKit SDK 事件回调函数定义
 *
 * @param event 事件类型
 * @param data 事件数据
 */
typedef void (*duikit_sdk_event_callback_t)(int event, void *data);

/**
 * @brief DUIKit SDK 初始化配置
 */
typedef struct duikit_sdk_config {
    char *                      auth_config;    //授权配置，JSON格式
    duikit_sdk_event_callback_t callback;       //SDK统一事件回调

    duikit_dsp_adapter_t *      dsp_adapter;    //DSP实例，由具体平台按照接口定义实现

#ifdef CONFIG_ADS_COMPONENT_NETWORK_MANAGER
    network_adapter_t *         wifi_adapter;   //WiFi实例，由具体平台按照接口定义实现
#endif

#ifdef CONFIG_ADS_COMPONENT_MEDIA_MANAGER
    player_adapter_t *          music_player;   //音乐播放器实例，由具体平台按照接口定义实现
    player_adapter_t *          tts_player;     //TTS播放器实例，由具体平台按照接口定义实现
#endif

    bool                        use_local_vad;  //默认使用本地VAD
} duikit_sdk_config_t;

/**
 * @brief DUIKit SDK 初始化
 *
 * @param config 初始化配置
 *
 * @return 0: 成功，其他: 失败
 */
int duikit_sdk_init(duikit_sdk_config_t *config);

/**
 * @brief DUIKit SDK 去初始化
 *
 * @return 0: 成功，其他: 失败
 */
int duikit_sdk_deinit(void);

/**
 * @brief DUIKit SDK 唤醒开关设置
 *
 * @param on true: 开启，false: 关闭
 *
 * @return 0: 成功，其他: 失败
 */
int duikit_sdk_wakeup_set(bool on);

/**
 * @brief DUIKit SDK VAD设置
 *
 * @param use_local_vad true: 使用本地VAD，false: 使用云端VAD
 *
 * @return 0: 成功，其他: 失败
 *
 * @note DUIKit SDK没有内置VAD算法，如使用本地VAD，需要适配好dsp_adapter中VAD相关的消息和事件处理
 *
 * @see duikit_dsp_adapter.h
 */
int duikit_sdk_vad_set(bool use_local_vad);

/**
 * @brief DUIKit SDK ONESHOT开关设置
 *
 * @param on true: 开启，false: 关闭
 *
 * @return 0: 成功，其他: 失败
 *
 * @note ONESHOT功能依赖本地VAD，需要适配好dsp_adapter中VAD及ONESHOT相关的消息和事件处理
 *
 * @see duikit_dsp_adapter.h
 */
int duikit_sdk_oneshot_set(bool on);

/**
 * @brief DUIKit SDK 在/离线状态设置
 *
 * @param online true: 在线，false: 离线
 *
 * @return 0: 成功，其他: 失败
 */
int duikit_sdk_online_set(bool online);

/**
 * @brief DUIKit SDK 停止对话
 *
 * @return 0: 成功，其他: 失败
 */
int duikit_sdk_stop_dialog(void);

/**
 * @brief DUIKit SDK 是否处于在线模式
 *
 * @return true: 在线，false: 离线
 */
bool duikit_sdk_is_online(void);

#ifdef __cplusplus
}
#endif

#endif
