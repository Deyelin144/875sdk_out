#ifndef __AISPEECH_DSP_MANAGER_H__
#define __AISPEECH_DSP_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dsp Manager 原始音频回调
 *
 * @param data 音频数据指针
 *
 * @param size 数据长度
 * @param user_data 用户数据指针
 *
 * @return void
 */
typedef void (*dsp_manager_callback_t)(char *data, int size, void *user_data);

/**
 * @brief Dsp Manager 消息事件回调
 *
 * @param cmd  消息指令
 * @param data 消息数据：唤醒事件/离线词命中事件等
 *
 * @param size 数据长度
 * @param user_data 用户数据指针
 *
 * @return void
 */
typedef void (*dsp_manager_msg_callback_t)(int cmd, char *data, int size, void *user_data);

//DSP指令集
enum {
    //recv
    DSP_CMD_ALG_MSG,                //算法消息
    DSP_CMD_REQUEST_PROFILE,
    DSP_CMD_RESPONSE_PROFILE,
    DSP_CMD_AUTH_RESULT,            //授权结果
    DSP_CMD_ASR_TIMEOUT,            //ASR超时
    DSP_CMD_VAD_STATUS_CHANGED,     //VAD状态变化
    DSP_CMD_REQUEST_WAKEUPENV,      //dsp请求wakeupenv
    DSP_CMD_RESPONSE_WAKEUPENV,     //c906发送wakeupenv
    DSP_CMD_SEND_HEARTBEAT,         //dsp发送心跳包

    //send
    DSP_CMD_ALG_THRESH_SET = 100,   //算法阈值设置
    DSP_CMD_VAD_STATUS_SET,         //VAD状态设置

    DSP_CMD_MAX = 255,
};

/**
 * @brief Dsp Manager 初始化配置
 */
typedef struct dsp_manager_config
{
    char *auth_profile;                     //DSP算法授权Profile，与auth_profile_path二选一
    char *auth_profile_path;                //DSP算法授权文件路径，与auth_profile二选一
    dsp_manager_callback_t raw_audio_cb;    //原始音频数据回调
    dsp_manager_callback_t alg_audio_cb;    //算法后音频数据回调
    dsp_manager_callback_t vad_audio_cb;    //vad算法后音频数据回调
    dsp_manager_msg_callback_t msg_cb;      //DSP消息回调
    void *user_data;                        //用户数据指针，随回调函数返回
    int vad_pause_time;                     //vad pasuse time

    int raw_audio_recv_task_prio;          //原始音频数据接收任务的优先级, >0有效, 否则使用默认优先级15
    int alg_audio_recv_task_prio;          //算法后音频数据接收任务的优先级, >0有效, 否则使用默认优先级15
    int vad_audio_recv_task_prio;          //vad算法后音频数据接收任务的优先级, >0有效, 否则使用默认优先级15
    int msg_recv_task_prio;                //DSP消息数据接收任务的优先级, >0有效, 否则使用默认优先级15
} dsp_manager_config_t;


/**
 * @brief Dsp Manager 初始化
 *
 * @return 0：成功，others：失败
 */
int dsp_manager_init(dsp_manager_config_t *config);

/**
 * @brief Dsp Manager 反初始化
 *
 * @return 0：成功，others：失败
 */
int dsp_manager_deinit(void);

/**
 * @brief 向Dsp发送消息
 *
 * @param cmd  发送给dsp的指令
 * @param data 发送给dsp的数据，可为空
 * @return 0：成功，others：失败
 */
int dsp_manager_send(int cmd, void *data, int data_len);

/**
 * @brief 启动VAD
 *
 * @return 0：成功，others：失败
 */
int dsp_manager_vad_start(void);

/**
 * @brief Oneshot模式启动VAD
 *
 * @return 0：成功，others：失败
 */
int dsp_manager_oneshot_vad_start(void);

/**
 * @brief 停止VAD
 *
 * @return 0：成功，others：失败
 */
int dsp_manager_vad_stop(void);

/**
 * @brief 获取设备ID
 *
 * @param void
 *
 * @return 成功：设备ID，失败：calloc失败
 */
char *dsp_manager_device_id_get(void);

#ifdef __cplusplus
}
#endif

#endif // __AISPEECH_DSP_MANAGER_H__
