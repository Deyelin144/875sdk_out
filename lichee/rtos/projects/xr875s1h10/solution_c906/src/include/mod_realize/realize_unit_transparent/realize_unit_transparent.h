#ifndef _REALIZE_UNIT_TRANSPARENT_H_
#define _REALIZE_UNIT_TRANSPARENT_H_

#include "../../platform/gulitesf_config.h"
typedef struct{
    void (*transparent_cb)(void *obj, char *data);
} transparent_cb_t;

typedef void (*unit_transparent_cb_t)(void *obj, char *data);
typedef char* (*unit_transparent_get_t)(void *tp_obj, char *json_str);
typedef int (*unit_transparent_set_t)(void *tp_obj, char *json_str);

typedef struct {
    void *ctx;
    void *user_data;
    unit_transparent_get_t get;
    unit_transparent_set_t set;
} unit_transparent_obj_t;

unit_transparent_obj_t *realize_unit_transparent_new(char *mod, void *user_data, transparent_cb_t cb);
void realize_unit_transparent_delete(unit_transparent_obj_t **obj);

#endif