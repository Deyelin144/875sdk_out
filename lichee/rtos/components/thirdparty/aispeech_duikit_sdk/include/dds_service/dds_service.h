#ifndef __DDS_SERVICE_H__
#define __DDS_SERVICE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "dds.h"

typedef int (*dds_service_cb_t) (void *userdata, struct dds_msg *msg);

typedef struct dds_service_config {
    char *auth_cfg;
    int main_task_priority;
    int main_task_stack_size;
    int main_task_core_id;
    int main_task_ext_stack;
    int register_task_priority;
    int register_task_stack_size;
    int register_task_core_id;
    int register_task_ext_stack;
    dds_service_cb_t dds_cb;
    void *userdata;
} dds_service_config_t;

extern int dds_service_init(dds_service_config_t *config);

extern int dds_service_deinit(void);

extern int dds_service_speech_start(int enable_cloud_vad);

extern int dds_service_speech_stop(void);

extern int dds_service_speech_cancel(const char *record_id, const char *session_id);

extern int dds_service_speech_feed(char *buf, int len);

extern int dds_service_reset(void);

extern int dds_service_full_duplex_is_enabled(void);

extern int dds_service_tts_stream_data_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif
