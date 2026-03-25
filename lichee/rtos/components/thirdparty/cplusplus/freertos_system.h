/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
* Change Logs:
* Date           Author       Notes
* 2014-12-03     Bernard      Add copyright header.
* 2014-12-29     Bernard      Add cplusplus initialization for ARMCC.
* 2016-06-28     Bernard      Add _init/_fini routines for GCC.
* 2016-10-02     Bernard      Add WEAK for cplusplus_system_init routine.
*/

#ifndef FREERTOS_SYSTEM_H
#define FREERTOS_SYSTEM_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <portmacro.h>
#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>
#include <port_misc.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>

#define RT_EOK  0

#define RT_WEAK __attribute__((weak))

ssize_t rt_tick_from_millisecond(ssize_t millisec);

#endif  /*FREERTOS_SYSTEM_H*/
