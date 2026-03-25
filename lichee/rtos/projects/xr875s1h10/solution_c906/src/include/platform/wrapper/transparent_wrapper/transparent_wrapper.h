#ifndef _DRV_TRANSPARENT_WRAPPER_H_
#define _DRV_TRANSPARENT_WRAPPER_H_

#include "../../../mod_realize/realize_unit_transparent/realize_unit_transparent.h"
#include <pthread.h>
#define TRANSPARENT_MAX_SIZE 20

/* 
   cb是unit层传下来的回调函数
   typedef void (*unit_transparent_cb_t)(void *obj, char *data);
 */
typedef int (*transparent_create_t)(void *obj, unit_transparent_cb_t cb);
typedef char* (*transparent_get_t)(char *json_str);
typedef int (*transparent_set_t)(char *json_str);
typedef int (*transparent_destroy_t)(void);

typedef struct {
    char name[TRANSPARENT_MAX_SIZE];
    transparent_create_t create;
    transparent_get_t get;
    transparent_set_t set;
    transparent_destroy_t destroy;
} transparent_wrapper_info_t;

typedef struct {
    int transparent_num;
    transparent_wrapper_info_t *info;
} transparent_wrapper_t;

#endif