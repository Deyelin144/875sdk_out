#ifndef DUIKIT_TIME_H
#define DUIKIT_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "duikit_common.h"

/*
 * 延时(毫秒)
 */
DUIKIT_EXPORT void duikit_delayms(int ms);

/*
* 获取系统启动时间(毫秒)
*/
unsigned long long duikit_boot_time_ms_get(void);

/*
 * 获取当前时间(毫秒)
 */
unsigned long long duikit_time_ms_get(void);

#ifdef __cplusplus
}
#endif

#endif
