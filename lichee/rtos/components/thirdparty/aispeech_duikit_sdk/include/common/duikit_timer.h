#ifndef DUIKIT_TIMER_H
#define DUIKIT_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

/*
 * 定时器功能
 * 支持单次触发和周期触发
 * freertos 不要在回调函数中调用任何Timer函数，否则会导致定时器无法正常工作
 */

/*
 * 定时器句柄
 */
typedef struct duikit_timer* duikit_timer_t;

/*
 * 超时回调
 */
typedef void (*duikit_timer_expired_cb)(void *userdata);

/*
 * 定时器配置
 */
typedef struct {
    const char *name;   //定时器名称, 默认为DuikitTimer
    bool auto_reload;   //自动重载，即周期触发
    int timeout;        //超时时间
    duikit_timer_expired_cb cb;
    void *userdata;
} duikit_timer_cfg_t;

/*
 * 创建定时器
 */
DUIKIT_EXPORT duikit_timer_t duikit_timer_create(const duikit_timer_cfg_t *cfg);

/*
 * 开始计时
 */
DUIKIT_EXPORT duikit_err_t duikit_timer_start(duikit_timer_t obj);

/**
 * 改变定时器周期
 */
DUIKIT_EXPORT duikit_err_t duikit_timer_change_period(duikit_timer_t obj, int period);

/*
 * 取消定时
 */
DUIKIT_EXPORT duikit_err_t duikit_timer_stop(duikit_timer_t obj);

/*
 * 删除定时器
 */
DUIKIT_EXPORT duikit_err_t duikit_timer_destroy(duikit_timer_t obj);

#ifdef __cplusplus
}
#endif

#endif
