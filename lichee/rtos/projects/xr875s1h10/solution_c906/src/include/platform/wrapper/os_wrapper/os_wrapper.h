
/**
* @file  wrapper.h  
* @brief GUlite_SF SDK OS抽象层封装
* @date  2022-7-11  
* 为了适配不同的操作系统，将SDK所需要的系统函数抽象成单独一层，不同的OS有不同的实现
*/
#ifndef __OS_WRAPPER_H__
#define __OS_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../gulitesf_config.h"

#ifndef CONFIG_PLATFORM_MAC
#define OS_WRAPPER_FAST_MEM_TEXT __attribute__((section (CONFIG_FAST_MEM_TEXT)))
#define OS_WRAPPER_FAST_MEM_RODATA __attribute__((section (CONFIG_FAST_MEM_RODATA)))
#define OS_WRAPPER_FAST_MEM_DATA __attribute__((section (CONFIG_FAST_MEM_DATA)))
#define OS_WRAPPER_FAST_MEM_BSS __attribute__((section (CONFIG_FAST_MEM_BSS)))

#define OS_WRAPPER_LARGE_MEM_TEXT __attribute__((section (CONFIG_LARGE_MEM_TEXT)))
#define OS_WRAPPER_LARGE_MEM_RODATA __attribute__((section (CONFIG_LARGE_MEM_RODATA)))
#define OS_WRAPPER_LARGE_MEM_DATA __attribute__((section (CONFIG_LARGE_MEM_DATA)))
#define OS_WRAPPER_LARGE_MEM_BSS __attribute__((section (CONFIG_LARGE_MEM_BSS)))

#else
#define OS_WRAPPER_FAST_MEM_TEXT 
#define OS_WRAPPER_FAST_MEM_RODATA 
#define OS_WRAPPER_FAST_MEM_DATA 
#define OS_WRAPPER_FAST_MEM_BSS

#define OS_WRAPPER_LARGE_MEM_TEXT 
#define OS_WRAPPER_LARGE_MEM_RODATA 
#define OS_WRAPPER_LARGE_MEM_DATA 
#define OS_WRAPPER_LARGE_MEM_BSS 
#endif
////////////////////////////////////////////////////////////////////////////
/*****************************thread 优先级*********************************/
////////////////////////////////////////////////////////////////////////////

typedef enum  {
    OS_WRAPPER_PRIORITY_IDLE            = 0,
    OS_WRAPPER_PRIORITY_LOW             = 1,
    OS_WRAPPER_PRIORITY_BELOW_NORMAL    = 2,
    OS_WRAPPER_PRIORITY_NORMAL          = 3,
    OS_WRAPPER_PRIORITY_ABOVE_NORMAL    = 4,
    OS_WRAPPER_PRIORITY_HIGH            = 5,
    OS_WRAPPER_PRIORITY_REAL_TIME       = 6
} os_wrapper_priority;

/**
 * @brief 创建二值信号量
 * @param init_count 二值信号量的初始值，取值0或者1
 * @return 二值信号量的句柄
 */
typedef void* (*signal_mutex_create_t)(int init_count);

/**
 * @brief 创建信号量
 * @param init_count 二值信号量的初始值，取值0或者1
 * @return 二值信号量的句柄
 */
typedef void* (*signal_create_t)(int init_count, unsigned int maxCount);

/**
 * @brief 二值信号量等待信号
 * @param sem 二值信号量的句柄
 * @paran time_ms 等待时间，如果要永远等待下去，需要传入os_wrapper_get_forever_time的返回值
 * @return int
 */
typedef int (*signal_wait_t)(void* sem, long time_ms);

/**
 * @brief 二值信号量释放信号
 * @param mutex 二值信号量的句柄
 * @return int
 */
typedef int (*signal_post_t)(void* sem);

/**
 * @brief 删除二值信号量
 * @param mutex 二值信号量的句柄
 * @return int
 */
typedef int (*signal_delete_t)(void** sem);

/**
 * @brief 创建互斥量，一般用于保护公共变量，处理线程安全问题
 * @param
 * @return 互斥量的句柄
 */
typedef void* (*mutex_create_t)(void);

/**
 * @brief 互斥量加锁
 * @param mutex 目标互斥量的句柄
 * @paran time_ms 等待时间，如果要永远等待下去，需要传入os_wrapper_get_forever_time的返回值
 * @return 为0代表加锁成功，其他代表失败或者超时
 */
typedef int (*mutex_lock_t)(void* mutex, long time_ms);

/**
 * @brief 互斥量解锁
 * @param mutex 目标互斥量
 * @return int
 */
typedef int (*mutex_unlock_t)(void* mutex);

/**
 * @brief 删除互斥量
 * @param mutex 目标互斥量
 * @return int
 */
typedef int (*mutex_delete_t)(void** mutex);

/**
 * @brief 创建消息队列
 *
 * @param queue_len, 消息队列长度
 * @param size, 消息的大小
 * @return 互斥量的句柄
 */ 
typedef void* (*queue_create_t)(unsigned int queue_len, unsigned int size);

/**
 * @brief 消息队列发送消息
 * @param queue 目标消息队列的句柄
 * @param item 要发送的消息体
 * @paran time_ms 等待时间，如果要永远等待下去，需要传入os_wrapper_get_forever_time的返回值
 * @return int
 */
typedef int (*queue_send_t)(void* queue, const void *item, long time_ms);

/**
 * @brief 消息队列接收消息
 * @param queue 目标消息队列的句柄
 * @param item 要接收的消息体指针
 * @paran time_ms 等待时间，如果要永远等待下去，需要传入os_wrapper_get_forever_time的返回值
 * @return int
 */
typedef int (*queue_recv_t)(void* queue, void *item, long time_ms);

/**
 * @brief 删除消息队列
 * @param queue 目标消息队列
 * @return int
 */
typedef int (*queue_delete_t)(void** queue);

/**
 * @brief 获取消息队列个数
 * @param queue 目标消息队列
 * @return int
 */
typedef int (*queue_check_t)(void* queue);

/**
 * @brief 此函数一般用于freeRTOS, 在task末尾调用vTaskDelete(NULL),其他OS一般用不到
 * @param thread_handle 线程句柄
 * @return void
 */
typedef int (*thread_delete_t)(void** thread_handle);

/**
 * @brief 启动线程
 * @param thread 线程的句柄，NULL代表启动线程失败
 * @param thread_func 线程函数，void ()(void*)
 * @param param 线程参数
 * @param name 线程名称
 * @param prior 线程优先级，数值越大优先级越高
 * @param stack_depth 线程栈深度，注意，单位为字（4bytes）
 * @return 0:success, -1:fail
 */
typedef int (*thread_create_t)(void **thread, void* thread_func, void* param, const char* name, int prior, int stack_depth);

/**
 * @brief 线程命名,此函数一般用于linux和mac平台, 长度不要超过15个字符
 * @param thread 线程句柄
 * @param name 线程名称
 * @return 为0代表命名成功,其他代表失败
 */
typedef int (*thread_set_name_t)(void* thread, const char *name);

/**
 * @brief 判断线程是否还存在
 * @param thread_handle 线程句柄
 * @return void
 */
typedef int (*thread_is_valid_t)(void* thread_handle);

/**
 * @brief 修改任务优先级
 * @param thread 线程句柄
 * @param priority 线程优先级
 * @return void
 */
typedef int (*thread_set_priority_t)(void* thread, unsigned char priority);

/**
 * @brief 获取任务优先级
 * @param thread 线程句柄
 * @param priority 线程优先级
 * @return void
 */
typedef int (*thread_get_priority_t)(void* thread, unsigned long* priority);


/**
 * @brief 获取当前任务句柄
 * @return long int 句柄整形
 */
typedef long (*thread_get_tid_t)(void* thread);

/**
 * @brief 获取当前任务栈剩余大小
 * @return long int 句柄整形
 */
typedef long (*thread_get_free_stack_t)(void* thread);

/**
 * @brief 获取从开机到当前时刻的持续时间
 * @param 
 * @return 从开机到当前时刻的持续时间，单位为毫秒
 */
typedef long (*get_time_ms_t)(void);

/**
 * @brief 睡眠
 * @param time_ms 毫秒数
 * @return void
 */
typedef void (*msleep_t)(long time_ms);

/**
 * @brief 在信号量等待信号，或者互斥量加锁的时候，如果要持续等待到获得信号或者加锁成功为止，需要调用此函数，并将结果作为参数传入对应函数中；
 * 例如：os_wrapper_wait_signal(mutex, os_wrapper_get_forever_time());
 * @param 
 * @return 二值信号量的句柄
 */
typedef long (*get_forever_time_t)(void);

/**
 * @brief 获取时间戳
 * @param timestamp 时间戳字符串
 * @return void
 */
typedef void (*get_timestamp_t)(char *timestamp);

/**
 * @brief 创建定时器
 * @param handle 出参，timer的句柄
 * @param repeat true代表定时器将重复触发，false代表只触发一次
 * @param func 定时器触发时执行的函数
 * @param arg
 * @param period_ms 定时间隔，单位为毫秒
 * @return void*
 */
typedef void* (*timer_create_t)(bool repeat, void* func, void *arg, int period_ms);

/**
 * @brief 删除定时器
 * @param handle timer的句柄
 * @return int 成功与否
 */
typedef int (*timer_delete_t)(void** timer);

/**
 * @brief 启动定时器
 * @param handle timer的句柄
 * @return void
 */

typedef int (*timer_start_t)(void* timer);

/**
 * @brief 停止定时器
 * @param handle timer的句柄
 * @return int
 */
typedef int (*timer_stop_t)(void* timer);

/**
 * @brief 停止定时器
 * @param handle timer的句柄
 * @return int
 */
typedef int (*timer_is_active_t)(void* timer);

/**
 * @brief 修改定时器周期时间
 * @param handle timer的句柄
 * @param period_ms timer的新周期时间
 * @return int
 */
typedef int (*timer_change_period_t)(void* timer, int period_ms);

/**
 * @brief 获取定时器当前剩余时间
 * @param handle timer的句柄
 * @return int
 */
typedef int (*timer_get_time_ms_t)(void* timer);

/**
 * @brief 获取errno
 * @param void
 * @return int
 */
typedef int (*get_errno_t)(void);

/**
 * @brief set errno
 * @param int 
 * @return void
 */
typedef void (*set_errno_t)(int err);

typedef void *(*spinlock_create_t)(void);
typedef void (*spinlock_delete_t)(void **lock);
typedef void (*spinlock_lock_t)(void *lock);
typedef void (*spinlock_unlock_t)(void *lock);


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
	spinlock_create_t spinlock_create;
	spinlock_delete_t spinlock_delete;
	spinlock_lock_t spinlock_lock;
	spinlock_unlock_t spinlock_unlock;
} os_wrapper_t;




#ifdef __cplusplus
}
#endif

#endif
