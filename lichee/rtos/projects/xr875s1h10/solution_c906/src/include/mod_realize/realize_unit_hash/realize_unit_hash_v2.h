#ifndef _REALIZE_UNIT_HASH_V2_H_
#define _REALIZE_UNIT_HASH_V2_H_
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "errno.h"
#include "../../platform/wrapper/wrapper.h"
#include "../realize_unit_mem/realize_unit_mem.h"

#ifdef CONFIG_HASH_THREAD_SAFETY
#define REALIZE_UNIT_HASH_THREAD_SAFETY CONFIG_HASH_THREAD_SAFETY
#else
#define REALIZE_UNIT_HASH_THREAD_SAFETY 0
#endif

#ifdef CONFIG_HASH_DEBUG
#define REALIZE_UNIT_HASH_DEBUG CONFIG_HASH_DEBUG
#ifdef CONFIG_HASH_DEBUG_TRA
#define REALIZE_UNIT_HASH_DEBUG_TRA CONFIG_HASH_DEBUG_TRA
#else
#define REALIZE_UNIT_HASH_DEBUG_TRA 0
#endif
#else
#define REALIZE_UNIT_HASH_DEBUG 0
#define REALIZE_UNIT_HASH_DEBUG_TRA 0
#endif

#ifdef CONFIG_HASH_DEMO_TEST
#define REALIZE_UNIT_HASH_DEMO_TEST CONFIG_HASH_DEMO_TEST
#else
#define REALIZE_UNIT_HASH_DEMO_TEST 0
#endif

/* heap malloc */
#define realize_unit_hash_malloc_alias(size)  realize_unit_mem_malloc(size)
#define realize_unit_hash_calloc_alias(nmemb, size)  realize_unit_mem_calloc(nmemb, size)
#define realize_unit_hash_realloc_alias(rmem, newsize)  realize_unit_mem_realloc(rmem, newsize)
#define realize_unit_hash_free_alias(p)  realize_unit_mem_free(p)
#define realize_unit_hash_malloc_get_size(p)  realize_unit_mem_get_block_size(p)

/* debug print */
#define realize_unit_hash_log_debug realize_unit_log_debug
#define realize_unit_hash_log_info realize_unit_log_info
#define realize_unit_hash_log_warm realize_unit_log_warn
#define realize_unit_hash_log_error realize_unit_log_error

/* system mechanism */
#if REALIZE_UNIT_HASH_THREAD_SAFETY
#define realize_unit_hash_mutex_create() wrapper_get_handle()->os.mutex_create()
#define realize_unit_hash_mutex_lock(mutex, time_ms) wrapper_get_handle()->os.mutex_lock(mutex, time_ms)
#define realize_unit_hash_mutex_unlock(mutex) wrapper_get_handle()->os.mutex_unlock(mutex)
#define realize_unit_hash_mutex_destroy(mutex_ptr) wrapper_get_handle()->os.mutex_delete(mutex_ptr)
#endif

#define HASH_EINVAL EINVAL
#define realize_unit_hash_set_errno wrapper_get_handle()->os.os_set_errno
#define realize_unit_hash_get_errno wrapper_get_handle()->os.os_get_errno

#define realize_unit_hash_unlikely(x) (x)//__builtin_expect(!!(x), 0)

typedef void (*free_data_cb)(void *data, void *u_data);
typedef int (*traversal_node_cb)(void* obj, void *node, void *u_data);

enum {
    HASH_DEBUG_MEM,
    HASH_DEBUG_MAP,
};

typedef enum {
    HASH_TYPE_UINT32_V2,
	HASH_TYPE_UNUM_PTR_V2,
    HASH_TYPE_STRING_V2,
} hash_type_v2_t;

typedef struct hash_obj {
    void *ctx;
    void *u_data;
    int (*add_node)(struct hash_obj *obj, void *key, void *value, unsigned int value_size);
    void *(*get_node)(struct hash_obj *obj, void *key);
    void *(*get_val_from_key)(struct hash_obj *obj, void *key);
    int (*remove_node)(struct hash_obj *obj, void *key);
    int (*traversal_node)(struct hash_obj *obj);
    int (*set_traversal_cb)(struct hash_obj *obj, traversal_node_cb cb);
    int (*set_free_cb)(struct hash_obj *obj, free_data_cb cb);
    int (*get_hash_type)(struct hash_obj *obj);
    /*****************************操作node的方法*************************************/
    void *(*get_val_from_node)(struct hash_obj *obj, void *node, bool is_cb);
    void *(*get_key_from_node)(struct hash_obj *obj, void *node, bool is_cb);
    /*****************debug 的方法，依赖 REALIZE_UNIT_HASH_DEBUG*********************/
    void (*print_debug_info)(struct hash_obj *obj, int type);
} hash_obj_t;

hash_obj_t *realize_unit_hash_new(hash_type_v2_t type, unsigned int array_size, free_data_cb fcb, traversal_node_cb tcb, void *u_data);
void realize_unit_hash_delete(hash_obj_t **hash);

#endif