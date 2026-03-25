#ifndef DUIKIT_THREAD_H
#define DUIKIT_THREAD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "duikit_common.h"

/*
 * 线程创建（joinable/detached）
 */

typedef struct
{
    /*
     * 线程名
     * 可选，默认值为"DuikitThread"
     */
    const char *name;
    /*
     * 线程入口函数
     * 必选
     */
    void (*run)(void *args);
    /*
     * 线程入口参数
     * 可选
     */
    void *args;

    /*
     * 优先级
     * 一般情况下pthread实现的系统上使用系统默认值即可
     */
    int priority;

    /*
     * 栈大小
     * 一般情况下pthread实现的系统上使用系统默认值即可
     */
    int stack_size;

    /*
     * 某些RTOS系统支持将栈空间放置在PSRAM，例如ESP32
     */
    int ext_stack;

    /*
     * 运行核id
     * 有些运行RTOS的多核CPU支持指定task运行所在的核，如双核系统，可以指定1或者2
     */
    int core_id;
} duikit_thread_cfg_t;

/*
 * 线程句柄
 */
typedef struct duikit_thread *duikit_thread_t;

/*
 * 创建一个joinable类型的线程，此类线程必须调用duikit_thread_destroy进行清理
 *
 * 类linux平台上调用pthread_create
 * freertos平台上调用xTaskCreate或其他变体
 * windows平台上调用_beginthreadex
 *
 * 返回值
 * 成功 线程句柄指针
 * 失败 NULL
 */
DUIKIT_EXPORT duikit_thread_t duikit_thread_create_joinable(const duikit_thread_cfg_t *cfg);

/*
 * 创建detached线程
 *
 * 注意
 * windows暂不支持
 *
 * 返回值
 * DUIKIT_ERR_OK
 * DUIKIT_ERR_INVAL
 * DUIKIT_ERR_NOMEM
 * DUIKIT_ERR_NOSYS
 */
DUIKIT_EXPORT duikit_err_t duikit_thread_create_detached(const duikit_thread_cfg_t *cfg);

/*
 * 等待线程主动退出并销毁相关资源
 *
 * linux平台上采用pthread_join方式
 * freertos平台上采用二值信号量实现（freertos创建的thread默认是detached）
 * windows平台上采用WaitForSingleObject实现
 */
DUIKIT_EXPORT void duikit_thread_destroy(duikit_thread_t thread);

#ifdef __cplusplus
}
#endif

#endif
