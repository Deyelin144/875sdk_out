#ifndef DUISKILL_PROCESS_H
#define DUISKILL_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

//默认应答提示音文件路径
#ifndef DEFAULT_ACK_PROMPT_PATH
#define DEFAULT_ACK_PROMPT_PATH ("/data/prompt/ack_ok.mp3")
#endif

//默认不支持的技能提示音文件路径
#ifndef DEFAULT_UNSUPPORTED_PROMPT_PATH
#define DEFAULT_UNSUPPORTED_PROMPT_PATH ("/data/prompt/unsupported_function.mp3")
#endif

//默认音量步进值
#ifndef DEFAULT_VOLUME_STEP
#define DEFAULT_VOLUME_STEP (10)
#endif

//默认音量最大值
#ifndef DEFAULT_VOLUME_MIN
#define DEFAULT_VOLUME_MAX  (100)
#endif

//默认音量最小值
#ifndef DEFAULT_VOLUME_MIN
#define DEFAULT_VOLUME_MIN  (0)
#endif

int duiskill_process(void *arg);

int set_device_volume(int vol);

int get_device_volume(void);

#ifdef __cplusplus
}
#endif
#endif
