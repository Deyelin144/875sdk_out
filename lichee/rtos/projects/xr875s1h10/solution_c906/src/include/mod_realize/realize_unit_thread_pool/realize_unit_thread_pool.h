#ifndef __REALIZE_UNIT_THREAD_POOL_H__
#define __REALIZE_UNIT_THREAD_POOL_H__

#include "../realize_unit_log/realize_unit_log.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#define THREAD_POOL_NAME_MAX     20

#define THREAD_POOL_ON_DEBUG 0

#if THREAD_POOL_ON_DEBUG
#define thread_pool_log_debug(format, ...)
#define thread_pool_log_info(format, ...)
#else
#define thread_pool_log_debug realize_unit_log_debug
#define thread_pool_log_info realize_unit_log_info
#endif
#define thread_pool_log_warn realize_unit_log_warn
#define thread_pool_log_error realize_unit_log_error

#define thread_pool_malloc realize_unit_mem_malloc
#define thread_pool_calloc realize_unit_mem_calloc
#define thread_pool_free realize_unit_mem_free

typedef struct _thread_pool {
    void *ctx;
    char name[THREAD_POOL_NAME_MAX + 1];
    int (*create)(struct _thread_pool *obj, unsigned char min_num, unsigned char max_num, int thread_stack_size, unsigned char priority);
    int (*destroy)(struct _thread_pool *obj);
    int (*add)(struct _thread_pool *obj, void (*func)(void *), void *arg);
} thread_pool_obj_t;


thread_pool_obj_t *realize_unit_thread_pool_new(char* name);
int realize_unit_thread_pool_delete(thread_pool_obj_t **obj);

#endif // !__REALIZE_UNIT_THREAD_POOL_H__