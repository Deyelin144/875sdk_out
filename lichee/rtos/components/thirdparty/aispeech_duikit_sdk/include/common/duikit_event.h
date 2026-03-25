#ifndef DUIKIT_EVENT_H
#define DUIKIT_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

/*
 * 事件组
 *
 * 用于线程间事件同步(每一个事件占用一个bit位)
 * 支持与(&)同步(所有事件都满足)和或(|)同步(至少一个事件满足)
 * 支持等待超时
 */

/*
 * 考虑到平台兼容问题当前只支持24bit值[23:0]
 */
typedef uint32_t duikit_event_bits_t;

/*
 * 事件组句柄
 */
typedef struct duikit_event* duikit_event_t;

/*
 * 创建事件组
 *
 * 返回值
 * 事件组句柄
 */
#define duikit_event_create() duikit_event_create_with_value(0U)

/*
 * 创建事件组并带初值
 */
DUIKIT_EXPORT duikit_event_t duikit_event_create_with_value(duikit_event_bits_t bits);

/*
 * 设置事件
 *
 * rbits获取设置之前的事件组信息（可选）
 */
DUIKIT_EXPORT duikit_err_t duikit_event_set_bits(duikit_event_t obj,
    duikit_event_bits_t bits, duikit_event_bits_t *rbits);

/*
 * 清除事件
 *
 * rbits获取清除之前的事件组信息（可选）
 */
DUIKIT_EXPORT duikit_err_t duikit_event_clear_bits(duikit_event_t obj,
    duikit_event_bits_t bits, duikit_event_bits_t *rbits);

/*
 * 等待事件
 *
 * all 是否等待所有事件
 * clear 是否清除等待的事件
 *
 * rbits获取接口返回时原有的事件组信息（可选）
 *
 * 返回值
 * SP_ERR_OK
 * SP_ERR_TIMEOUT
 */
DUIKIT_EXPORT duikit_err_t duikit_event_wait_bits(duikit_event_t obj,
    duikit_event_bits_t bits, duikit_event_bits_t *rbits, bool all, bool clear, int timeout);

/*
 * 获取事件集合
 */
DUIKIT_EXPORT duikit_event_bits_t duikit_event_get_bits(duikit_event_t obj);

/*
 * 销毁事件组
 *
 * 注意
 * 禁止在有线程仍在wait时进行销毁操作
 */
DUIKIT_EXPORT void duikit_event_destroy(duikit_event_t obj);

#ifdef __cplusplus
}
#endif

#endif
