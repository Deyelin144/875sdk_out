#ifndef _LIBEVENT_PLATFORM_CONFIG_H_
#define _LIBEVENT_PLATFORM_CONFIG_H_

#include "tc_iot_libs_inc.h"

#if defined(PLATFORM_FREERTOS)
#if defined(PLATFORM_XR875_RTOS)
#include "libevent_config_xr875.h"
#else
#include "libevent_config_freertos.h"
#endif  // PLATFORM_XR875_RTOS
#elif defined(PLATFORM_RTTHREAD)
#include "libevent_config_rtthread.h"
#elif defined(PLAT_USE_THREADX)
#ifndef NSIG
#define NSIG 32
#endif

typedef int off_t;
typedef int pid_t;
#include "libevent_config_asr.h"
#elif defined(PLAT_USE_LITEOS)
#include "libevent_config_liteos.h"
#else
#include "libevent_config_linux.h"
#endif

#endif /* \_LIBEVENT_PLATFORM_CONFIG_H_ */
