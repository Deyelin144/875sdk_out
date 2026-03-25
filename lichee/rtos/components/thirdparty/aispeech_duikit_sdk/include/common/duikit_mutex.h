#ifndef DUIKIT_MUTEX_H
#define DUIKIT_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

/*
 * 互斥锁用于互斥访问
 */

/*
 * 互斥锁句柄
 */
typedef struct duikit_mutex* duikit_mutex_t;

/*
 * 创建互斥锁
 *
 * 返回值
 * 互斥锁句柄
 */
DUIKIT_EXPORT duikit_mutex_t duikit_mutex_create();

/*
 * 上锁
 *
 * 返回值
 * SP_ERR_OK
 */
DUIKIT_EXPORT duikit_err_t duikit_mutex_lock(duikit_mutex_t obj);

/*
 * 尝试上锁
 *
 * 返回值
 * SP_ERR_OK    成功获取到锁
 * SP_ERR_EMPTY 未获取到锁
 */
DUIKIT_EXPORT duikit_err_t duikit_mutex_trylock(duikit_mutex_t obj);

/*
 * 开锁
 *
 * 返回值
 * SP_ERR_OK
 */
DUIKIT_EXPORT duikit_err_t duikit_mutex_unlock(duikit_mutex_t obj);

/*
 * 销毁互斥锁
 */
DUIKIT_EXPORT void duikit_mutex_destroy(duikit_mutex_t obj);

#ifdef __cplusplus
}
#endif

#endif
