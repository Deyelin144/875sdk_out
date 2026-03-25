
#ifndef __OS_ADAPTER_H__
#define __OS_ADAPTER_H__

#include <stdbool.h>

typedef enum  {
    OS_ADAPTER_PRIORITY_IDLE            = 0,
    OS_ADAPTER_PRIORITY_LOW             = 1,
    OS_ADAPTER_PRIORITY_BELOW_NORMAL    = 2,
    OS_ADAPTER_PRIORITY_NORMAL          = 3,
    OS_ADAPTER_PRIORITY_ABOVE_NORMAL    = 4,
    OS_ADAPTER_PRIORITY_HIGH            = 5,
    OS_ADAPTER_PRIORITY_REAL_TIME       = 6
} os_adapter_priority;

typedef void* (*signal_mutex_create_t)(int init_count);
typedef void* (*signal_create_t)(int init_count, unsigned int maxCount);
typedef int (*signal_wait_t)(void* sem, long time_ms);
typedef int (*signal_post_t)(void* sem);
typedef int (*signal_delete_t)(void** sem);
typedef void* (*mutex_create_t)(void);
typedef int (*mutex_lock_t)(void* mutex, long time_ms);
typedef int (*mutex_unlock_t)(void* mutex);
typedef int (*mutex_delete_t)(void** mutex);
typedef void* (*queue_create_t)(unsigned int queue_len, unsigned int size);
typedef int (*queue_send_t)(void* queue, const void *item, long time_ms);
typedef int (*queue_recv_t)(void* queue, void *item, long time_ms);
typedef int (*queue_delete_t)(void** queue);
typedef int (*queue_check_t)(void* queue);
typedef int (*thread_delete_t)(void** thread_handle);
typedef int (*thread_create_t)(void **thread, void* thread_func, void* param, const char* name, int prior, int stack_depth);
typedef int (*thread_set_name_t)(void* thread, const char *name);
typedef int (*thread_is_valid_t)(void* thread_handle);
typedef int (*thread_set_priority_t)(void* thread, unsigned char priority);
typedef int (*thread_get_priority_t)(void* thread, unsigned long* priority);
typedef long (*thread_get_tid_t)(void* thread);
typedef long (*thread_get_free_stack_t)(void* thread);
typedef long (*get_time_ms_t)(void);
typedef void (*msleep_t)(long time_ms);
typedef long (*get_forever_time_t)(void);
typedef void (*get_timestamp_t)(char *timestamp);
typedef void* (*timer_create_t)(bool repeat, void* func, void *arg, int period_ms);
typedef int (*timer_delete_t)(void** timer);
typedef int (*timer_start_t)(void* timer);
typedef int (*timer_stop_t)(void* timer);
typedef int (*timer_is_active_t)(void* timer);
typedef int (*timer_change_period_t)(void* timer, int period_ms);
typedef int (*timer_get_time_ms_t)(void* timer);
typedef int (*get_errno_t)(void);
typedef void (*set_errno_t)(int err);
typedef void* (*malloc_t)(size_t size);
typedef void* (*realloc_t)(void *rmem, size_t newsize);
typedef void* (*calloc_t)(size_t nmemb, size_t size);
typedef void (*free_t)(void* p);

typedef struct {
	signal_mutex_create_t signal_mutex_create;
	signal_create_t signal_create;
	signal_wait_t signal_wait;
	signal_post_t signal_post;
	signal_delete_t signal_delete;
	mutex_create_t mutex_create;
	mutex_lock_t mutex_lock;
	mutex_unlock_t mutex_unlock;
	mutex_delete_t mutex_delete;
	queue_create_t queue_create;
	queue_send_t queue_send;
	queue_recv_t queue_recv;
	queue_delete_t queue_delete;
	queue_check_t queue_check;
	thread_delete_t thread_delete;
	thread_create_t thread_create;
	thread_set_name_t thread_set_name;
	thread_is_valid_t thread_is_valid;
	thread_set_priority_t thread_set_priority;
	thread_get_priority_t thread_get_priority;
	thread_get_tid_t thread_get_tid;
	thread_get_free_stack_t thread_get_free_stack;
	get_time_ms_t get_time_ms;
	msleep_t msleep;
	get_forever_time_t get_forever_time;
	get_timestamp_t get_timestamp;
	timer_create_t timer_create;
	timer_delete_t timer_delete;
	timer_start_t timer_start;
	timer_stop_t timer_stop;
	timer_is_active_t timer_is_active;
	timer_change_period_t timer_change_period;
    timer_get_time_ms_t timer_get_time_ms;
	get_errno_t os_get_errno;
	set_errno_t os_set_errno;
    malloc_t malloc;
    realloc_t realloc;
    calloc_t calloc;
    free_t free;
} os_adapter_t;

os_adapter_t *os_adapter();

#endif