#ifndef DUIKIT_DIALOG_PROCESS_H
#define DUIKIT_DIALOG_PROCESS_H

#include "dds.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef CONFIG_COMPONENT_DUIKIT_ENABLE_CLOUD_VAD
#define CONFIG_COMPONENT_DUIKIT_ENABLE_CLOUD_VAD
#endif

#define DUIKIT_DP_EV_IN_RESET                               0x100 // 强制复位
#define DUIKIT_DP_EV_IN_WAKEUP                              0x101 // 唤醒事件
#define DUIKIT_DP_EV_IN_WAKEUP_PROMPT_PLAY_FIN              0x102 // 唤醒提示音播报结束
#define DUIKIT_DP_EV_IN_VAD_BEGAIN                          0x103 // VAD算法监测到静音到非静音状态变化，即表示用户开始说话
#define DUIKIT_DP_EV_IN_VAD_END                             0x104 // VAD算法监测到非静音到静音状态变化，即表示用户说话结束
#define DUIKIT_DP_EV_IN_TTS_PLAY_START                      0x105 // 对话回复TTS播报开始(用于给DUI上报会话状态)
#define DUIKIT_DP_EV_IN_TTS_PLAY_FIN                        0x106 // 对话回复TTS播报结束
#define DUIKIT_DP_EV_IN_MULTIPLE_DIALOG_PROMPT_PLAY_FIN     0x107 // 多轮对话监听开始提示音播报结束
#define DUIKIT_DP_EV_IN_ONESHOT_WAKEUP                      0x108 // ONESHOT唤醒事件

#define DUIKIT_DP_EV_OUT_PLAY_WAKEUP_PROMPT                 0X201 // 播放唤醒提示音，需要在提示音播报完毕时，发送DUIKIT_DP_EV_OUT_PLAY_WAKEUP_PROMPT事件通知DuiKit
#define DUIKIT_DP_EV_OUT_VAD_BEGAIN_TIMEOUT                 0X202 // 唤醒后未说话（监听超时）
#define DUIKIT_DP_EV_OUT_PLAY_TTS                           0x203 // 播放对话回复TTS
#define DUIKIT_DP_EV_OUT_STOP_TTS                           0x204 // 停止播放对话回复TTS
#define DUIKIT_DP_EV_OUT_PLAY_MULTIPLE_DIALOG_PROMPT        0x205 // 播放多轮对话监听开始提示音
#define DUIKIT_DP_EV_OUT_STOP_MULTIPLE_DIALOG_PROMPT        0x206 // 停止播放多轮对话监听开始提示音
#define DUIKIT_DP_EV_OUT_DUI_RESPONSE                       0x207 // DUI云端响应信息
#define DUIKIT_DP_EV_OUT_VAD_END_TIMEOUT                    0X208 // 连续说话超过60，检测说话超时（强制结束对话，对话结果仍会下发）
#define DUIKIT_DP_EV_OUT_START_VAD                          0X209 // 开始VAD检测，用于启动本地VAD
#define DUIKIT_DP_EV_OUT_STOP_VAD                           0X20a // 停止VAD检测，通常发生在对话被打断（如唤醒/RESET），仅在使用本地VAD时才会抛出
#define DUIKIT_DP_EV_OUT_ONESHOT_WAKEUP                     0x20b // ONESHOT唤醒事件
#define DUIKIT_DP_EV_OUT_ERROR                              0x20c // 对话出现错误，如网络异常导致无法收到云端回复
#define DUIKIT_DP_EV_OUT_DIALOG_TIMEOUT                     0x20d // 对话监听超时（适用于全双工模式下，10s未命中任何有效意图超时后触发）
#define DUIKIT_DP_EV_OUT_AUTH_SUCCESS                       0x20e // 授权成功，如需支持仅联网授权一次，需要由平台从事件回调的dds_msg中获取encryptProfile并保存，下次启动时直接通过授权配置传入，即可实现无网络也能校验授权

typedef struct {
    int event;
    struct dds_msg *msg;
    void *user_data;
} duikit_dp_event_data_t;

typedef void (*duikit_dp_event_callback_t)(duikit_dp_event_data_t *data);

typedef struct duikit_dp_config {
    char *auth_cfg;
    int cache_time;
    bool use_full_duplex;
    bool use_local_vad;
    duikit_dp_event_callback_t callback;
    void *user_data;

    int msg_task_priority;
    int msg_task_stacksize;
    int msg_task_core_id;
    int msg_task_ext_stack;

    int audio_task_priority;
    int audio_task_stacksize;
    int audio_task_core_id;
    int audio_task_ext_stack;

    int dds_task_priority;
    int dds_task_stacksize;
    int dds_task_core_id;
    int dds_task_ext_stack;

    int dds_register_task_priority;
    int dds_register_task_stacksize;
    int dds_register_task_core_id;
    int dds_register_task_ext_stack;
} duikit_dp_config_t;

typedef struct duikit_dp_event {
    int type;
    struct dds_msg *msg;
} duikit_dp_event_t;

extern int duikit_dialog_process_init(duikit_dp_config_t *config);

extern int duikit_dialog_process_deinit(void);

extern int duikit_dialog_process_feed(char *data, int size);

extern int duikit_dialog_process_send(duikit_dp_event_t *event);

extern int duikit_dialog_process_vad_set(bool use_local_vad);

extern bool duikit_dialog_should_end_session(void);

#ifdef __cplusplus
}
#endif

#endif //DUIKIT_DIALOG_PROCESS_H
