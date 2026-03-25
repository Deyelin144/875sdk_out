#ifndef __REALIZE_UNIT_LOG_UPLOAD__H__
#define __REALIZE_UNIT_LOG_UPLOAD__H__

#include <stdio.h>
#include "../../platform/gulitesf_config.h"
#include "realize_unit_log.h"

#if CONFIG_ELOG_HANDLE_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *url;
    char *uuid;
    char *version;
    int timeout;
    int is_open;
    int size;
} unit_log_upload_cfg;


void realize_unit_log_upload_set_abort(char abort);
char realize_unit_log_upload_get_init(void);
char realize_unit_log_upload_get_status(void);
void realize_unit_log_upload_inc_size(int size);
int realize_unit_log_upload_get_max_buffer_size(void);
void realize_unit_log_upload_get_buffer(char *buffer, int max_len, int *len);
int realize_unit_log_upload_buffer(unit_log_upload_cfg *cfg, char *buffer, int len, int type);
int realize_unit_log_upload_control(unit_log_upload_cfg *cfg);

#ifdef __cplusplus
}
#endif

#endif

#endif
