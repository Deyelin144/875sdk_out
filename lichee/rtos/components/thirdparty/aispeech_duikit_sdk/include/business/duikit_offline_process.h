#ifndef DUIKIT_OFFLINE_PROCESS_H
#define DUIKIT_OFFLINE_PROCESS_H

#define DUIKIT_OP_EV_IN_RESET                               0x100 // 强制复位
#define DUIKIT_OP_EV_IN_WAKEUP                              0x101 // 唤醒事件
#define DUIKIT_OP_EV_IN_ASR_RESULT                          0x102 // 语音识别结果
#define DUIKIT_OP_EV_IN_ASR_TIMEOUT                         0x103 // 语音识别超时

#define DUIKIT_OP_EV_OUT_PLAY_WAKEUP_PROMPT                 0X201 // 播放唤醒提示音，需要在提示音播报完毕时，发送DUIKIT_DP_EV_OUT_PLAY_WAKEUP_PROMPT事件通知DuiKit
#define DUIKIT_OP_EV_OUT_ASR_RESULT                         0X202 // 语音识别结果，
#define DUIKIT_OP_EV_OUT_ASR_TIMEOUT                        0X203 // 语音识别超时

typedef struct {
    int event;
    void *data;
    void *user_data;
} duikit_op_event_data_t;

typedef void (*duikit_op_event_callback_t)(duikit_op_event_data_t *data);

typedef struct duikit_op_config {
    duikit_op_event_callback_t callback;
    void *user_data;
} duikit_op_config_t;

int duikit_offline_process_init(duikit_op_config_t *config);

int duikit_offline_process_deinit(void);

int duikit_offline_process_set(int type, void *data);

#endif
