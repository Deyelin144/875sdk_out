#ifndef DUIKIT_DSP_ADAPTER_H
#define DUIKIT_DSP_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DSP事件定义(输出)
 */
enum
{
    DUIKIT_DSP_EVENT_WAKEUP,        //唤醒事件
    DUIKIT_DSP_EVENT_ASR,           //本地识别事件(命令词)
    DUIKIT_DSP_EVENT_ASR_TIMEOUT,   //本地识别超时事件，超时后需要重新唤醒才可以再次识别
    DUIKIT_DSP_EVENT_VAD_BEGAIN,    //VAD检测到非静音事件
    DUIKIT_DSP_EVENT_VAD_END,       //VAD检测到静音事件
    DUIKIT_DSP_EVENT_WAKEUP_COMMAND,//快捷唤醒事件
};

/**
 * @brief DSP消息定义(输入)
 */
enum
{
    DUIKIT_DSP_MSG_ENABLE_VAD,          //启用本地VAD
    DUIKIT_DSP_MSG_DISABLE_VAD,         //禁用本地VAD
    DUIKIT_DSP_MSG_START_VAD,           //启动本地VAD
    DUIKIT_DSP_MSG_STOP_VAD,            //停止本地VAD
    DUIKIT_DSP_MSG_ONESHOT_START_VAD,   //ONESHOT模式启动本地VAD
};

/**
 * @brief DSP事件数据定义
 */
typedef struct {
    int event;
    char *data;
    int size;
    void *user_data;
} duikit_dsp_event_data_t;

/**
 * @brief DSP事件回调
 *
 * @param event 事件类型
 * @param data 事件数据指针
 * @param size 事件数据长度
 * @param user_data 用户数据指针
 *
 * @return void
 */
typedef void (*duikit_dsp_adapter_event_callback_t)(duikit_dsp_event_data_t *data);

/**
 * @brief DSP音频数据回调(输出算法后的音频，使用本地VAD时，应该只输出人声部分)
 *
 * @param data 音频数据指针
 * @param size 音频数据长度
 * @param user_data 用户数据指针
 *
 * @return void
 */
typedef void (*duikit_dsp_adapter_audio_callback_t)(char *data, int size, void *user_data);

/**
 * @brief DSP适配器初始化配置
 */
typedef struct duikit_dsp_adapter_config
{
    duikit_dsp_adapter_event_callback_t event_cb;   // 事件回调
    duikit_dsp_adapter_audio_callback_t audio_cb;   // 音频回调
    void *                              user_data;  // 用户数据指针
} duikit_dsp_adapter_config_t;

/**
 * @brief DSP适配器
 */
typedef struct duikit_dsp_adapter
{
    /**
     * @brief DSP初始化
     *
     * @param config DSP配置
     *
     * @return 0: 成功，其他：失败
     */
    int (*init)(duikit_dsp_adapter_config_t *config);

    /**
     * @brief DSP去初始化
     *
     * @return 0: 成功，其他：失败
     */
    int (*deinit)(void);

    /**
     * @brief 发送消息给DSP
     *
     * @param cmd 消息命令
     * @param data 消息数据
     * @param size 消息数据大小
     *
     * @return 0: 成功，其他：失败
     */
    int (*send_msg)(int msg, char *data, int size);
} duikit_dsp_adapter_t;

#ifdef __cplusplus
}
#endif

#endif