#ifndef DUIKIT_SEMAPHORE_H
#define DUIKIT_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

/*
 * 支持二值信号量和计数信号量
 * 支持超时等待
 */

/*
 * 信号量句柄
 */
typedef struct duikit_semaphore* duikit_semaphore_t;

/*
 * 创建二值信号量
 */
#define duikit_semaphore_create_binary() duikit_semaphore_create_count(0, 1)

/*
 * 创建计数型信号量
 */
DUIKIT_EXPORT duikit_semaphore_t duikit_semaphore_create_count(int value, int max);

/*
 * 等待信号量
 * 
 * 返回值
 * duikit_ERR_OK
 * duikit_ERR_TIMEOUT
 */
DUIKIT_EXPORT duikit_err_t duikit_semaphore_take(duikit_semaphore_t obj, int timeout);

/*
 * 释放信号量
 *
 * 返回值
 * duikit_ERR_OK
 * duikit_ERR_FULL 当前信号量槽已满
 */
DUIKIT_EXPORT duikit_err_t duikit_semaphore_give(duikit_semaphore_t obj);

/*
 * 获取可用信号数
 */
DUIKIT_EXPORT int duikit_semaphore_get_value(duikit_semaphore_t obj);

/*
 * 销毁信号量
 * 
 * 注意
 * 禁止在有线程在等待时进行销毁操作
 */
DUIKIT_EXPORT void duikit_semaphore_destroy(duikit_semaphore_t obj);

#ifdef __cplusplus
}
#endif

#endif
