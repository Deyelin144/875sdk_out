#ifndef _REALIZE_UNIT_KV_H_
#define _REALIZE_UNIT_KV_H_

#include "../../platform/gulitesf_config.h"

#define FLASHDB_BLOCK_NUM 12
#define KV_FILE_SYSTEM_DIR CONFIG_COMMON_RW_PATH"/kv"
#define KV_FILE_SYTEM_NAME "env"

int realize_unit_kv_all_sync(void);  //同步所有的kv数据库

typedef struct _kv_obj{
    void *ctx;
    // void *user_data;
    int (*set)(struct _kv_obj *obj, const char *key, const char *value);
    char *(*get)(struct _kv_obj *obj, const char *key);
    int (*del)(struct _kv_obj *obj, const char *key);
    void (*print)(struct _kv_obj *obj);
    int (*reset)(struct _kv_obj *obj);
    int (*sync)(struct _kv_obj *obj);
} kv_obj_t;

kv_obj_t *realize_unit_kv_new(const char *path, const char *name, int is_file_mode);
int realize_unit_kv_delete(kv_obj_t **obj);

#endif
