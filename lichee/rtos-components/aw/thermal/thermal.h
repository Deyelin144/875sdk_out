/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __THERMAL_H__
#define __THERMAL_H__

#include <aw_list.h>
#include <hal_timer.h>
#include <hal_atomic.h>
#include <queue.h>
#include <task.h>
#include <hal_thread.h>
#include <hal_mutex.h>

#define THERMAL_PARA			("thermal")

/* use value which < 0 K, to indicate an invalid/uninitialized temperature. Refer to linux kernel */
#define THERMAL_TEMP_INVALID		(-274000)
#define THERMAL_DEFAULT_CRITICAL_TEMP	(105000)

/* poling interval to check temperature */
#define POLLING_INTERVAL		(MS_TO_OSTICK(1000))

#define THERMAL_LOG_ERR_MASK		(0x1)
#define THERMAL_LOG_WARN_MASK		(0x2)
#define THERMAL_LOG_DBG_MASK		(0x4)
#define THERMAL_LOG_INF_MASK		(0x8)

extern int thermal_log_level;
#define thermal_err(fmt, arg...) do { if (thermal_log_level & THERMAL_LOG_ERR_MASK) printf(fmt, ##arg); } while(0)
#define thermal_warn(fmt, arg...) do { if (thermal_log_level & THERMAL_LOG_WARN_MASK) printf(fmt, ##arg); } while(0)
#define thermal_dbg(fmt, arg...) do { if (thermal_log_level & THERMAL_LOG_DBG_MASK) printf(fmt, ##arg); } while(0)
#define thermal_info(fmt, arg...) do { if (thermal_log_level & THERMAL_LOG_INF_MASK) printf(fmt, ##arg); } while(0)

#define THERMAL_CRIT_TEMP_SWITCH_MASK	(0x1 << 0)
#define THERMAL_EMUL_TEMP_SWITCH_MASK	(0x1 << 1)

typedef enum  {
    OS_PRIORITY_IDLE            = HAL_THREAD_PRIORITY_LOWEST,
    OS_PRIORITY_LOW             = HAL_THREAD_PRIORITY_LOWEST + 1,
    OS_PRIORITY_BELOW_NORMAL    = HAL_THREAD_PRIORITY_MIDDLE - 1,
    OS_PRIORITY_NORMAL          = HAL_THREAD_PRIORITY_MIDDLE,
    OS_PRIORITY_ABOVE_NORMAL    = HAL_THREAD_PRIORITY_MIDDLE + 1,
    OS_PRIORITY_HIGH            = HAL_THREAD_PRIORITY_HIGHEST - 1,
    OS_PRIORITY_REAL_TIME       = HAL_THREAD_PRIORITY_HIGHEST
} OS_Priority;

struct thermal_zone_device_ops {
	int (*get_temp)(void *data, int *temp);
	void (*shutdown)(void);
};

struct thermal_zone_device {
	void *priv;
	TaskHandle_t  xHandle;
	QueueHandle_t xQueue;
	struct thermal_zone_device_ops *ops;

	struct thermal_zone *zone;
};

struct thermal_zone_ops {
	int (*set_emul_temp)(void *data, const int emul_temp);
	int (*set_crit_temp)(void *data, const int crit_temp);
	int (*set_func_switch)(void *data, const int func_switch_mask);
};

struct thermal_zone {
	char *type;
	char mode;
	int id;
	int last_temperature;
	int temperature;
	int emul_temperature;
	int crit_temperature;
	int func_switch;

	struct thermal_zone_ops *ops;
	struct thermal_zone_device *zone_dev;
	unsigned int polling_interval;
	osal_timer_t polling_timer;
	hal_mutex_t lock;
	struct list_head node;
};

int thermal_init(void);
void thermal_zone_deinit(void);
struct thermal_zone_device *thermal_zone_device_register(char *zone_name,
							struct thermal_zone_device_ops *ops,
							void *priv);
int thermal_zone_device_unregister(struct thermal_zone_device *zone_dev);


#endif
