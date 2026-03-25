#ifndef __REALIZE_UNIT_CACHE_H__
#define __REALIZE_UNIT_CACHE_H__

#define ON_DEBUG 1

#if ON_DEBUG
#define cache_log_debug(format, ...)
#define cache_log_info(format, ...)
#else
#define cache_log_debug realize_unit_log_debug
#define cache_log_info realize_unit_log_info
#endif
#define cache_log_warn realize_unit_log_warn
#define cache_log_error realize_unit_log_error

#define cache_malloc realize_unit_mem_malloc
#define cache_calloc realize_unit_mem_calloc
#define cache_free realize_unit_mem_free


typedef enum {
    CACHE_KEY_UINT_32 = 0,			//32位整型
	CACHE_KEY_UINT_64,				//64位整型
    CACHE_KEY_STRING,
} cache_key_type_t;

typedef struct {
	void (*free_data)(void *u_data, void *data);
	// void (*travel)(void *u_data, void *data);
} cache_cb_t;

typedef struct _cache_obj {
	void *ctx;
	void *user_data;
	cache_cb_t cb;
	int (*cache_create)(struct _cache_obj *obj);
	int (*cache_set)(struct _cache_obj *cache_obj, void* key, void *data, int data_size);
	void *(*cache_get)(struct _cache_obj *cache_obj, void* key);
	void (*cache_destroy)(struct _cache_obj *cache_obj);
} cache_obj_t;

cache_obj_t *realize_unit_cache_new(int capacity, cache_key_type_t key_type, void *user_data, cache_cb_t *cb);
void realize_unit_cache_delete(cache_obj_t **obj);

#endif
