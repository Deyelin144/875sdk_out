#ifdef CONFIG_KERNEL_FREERTOS
#include <FreeRTOS.h>
#else
#error "PM do not support the RTOS!!"
#endif
#include <task.h>
#include <errno.h>

#include <hal_time.h>
#include "pm_debug.h"
#include "pm_base.h"
#include "pm_platops.h"

#undef  PM_DEBUG_MODULE
#define PM_DEBUG_MODULE  PM_DEBUG_PLATOPS

static suspend_ops_t *suspend_ops = NULL;

int pm_platops_register(suspend_ops_t *ops)
{
	pm_dbg("platops: register ops %s(%p) ok\n",
			(ops && ops->name)?ops->name:"UNKOWN", ops);

	suspend_ops = ops;

	return 0;
}

int pm_platops_call(suspend_ops_type_t type, suspend_mode_t mode)
{
	int ret = 0;

	//pm_trace_func("%d, %d", type, mode);

	if (!suspend_ops || !pm_suspend_mode_valid(mode)) {
		pm_invalid();
		return -EINVAL;
	}

#if 0
	/* check suspend*/
	ret = pm_suspend_assert();
	if (ret)
		return ret;
#endif

	switch (type) {
	case PM_SUSPEND_OPS_TYPE_VALID:
		if (suspend_ops->valid) {
			pm_debug_get_starttime (PM_TIME_PLAT_VALID, hal_gettime_ns());
			ret = suspend_ops->valid(mode);
			pm_debug_record_time (PM_TIME_PLAT_VALID, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_PRE_BEGIN:
		if (suspend_ops->pre_begin) {
			pm_debug_get_starttime (PM_TIME_PLAT_PRE_BEGIN, hal_gettime_ns());
			ret = suspend_ops->pre_begin(mode);
			pm_debug_record_time (PM_TIME_PLAT_PRE_BEGIN, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_BEGIN:
		if (suspend_ops->begin) {
			pm_debug_get_starttime (PM_TIME_PLAT_BEGIN, hal_gettime_ns());
			ret = suspend_ops->begin(mode);
			pm_debug_record_time (PM_TIME_PLAT_BEGIN, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_PREPARE:
		if (suspend_ops->prepare) {
			pm_debug_get_starttime (PM_TIME_PLAT_PREPARE, hal_gettime_ns());
			ret = suspend_ops->prepare(mode);
			pm_debug_record_time (PM_TIME_PLAT_PREPARE, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_PREPARE_LATE:
		if (suspend_ops->prepare_late) {
			pm_debug_get_starttime (PM_TIME_PLAT_PREPARE_LATE, hal_gettime_ns());
			ret = suspend_ops->prepare_late(mode);
			pm_debug_record_time (PM_TIME_PLAT_PREPARE_LATE, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_ENTER:
		if (suspend_ops->enter)
			ret = suspend_ops->enter(mode);
		break;
	case PM_SUSPEND_OPS_TYPE_WAKE:
		if (suspend_ops->wake) {
			pm_debug_get_starttime (PM_TIME_PLAT_WAKE, hal_gettime_ns());
			ret = suspend_ops->wake(mode);
			pm_debug_record_time (PM_TIME_PLAT_WAKE, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_FINISH:
		if (suspend_ops->finish) {
			pm_debug_get_starttime (PM_TIME_PLAT_FINISH, hal_gettime_ns());
			ret = suspend_ops->finish(mode);
			pm_debug_record_time (PM_TIME_PLAT_FINISH, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_END:
		if (suspend_ops->end) {
			pm_debug_get_starttime (PM_TIME_PLAT_END, hal_gettime_ns());
			ret = suspend_ops->end(mode);
			pm_debug_record_time (PM_TIME_PLAT_END, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_RECOVER:
		if (suspend_ops->recover) {
			pm_debug_get_starttime (PM_TIME_PLAT_RECOVER, hal_gettime_ns());
			ret = suspend_ops->recover(mode);
			pm_debug_record_time (PM_TIME_PLAT_RECOVER, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_AGAIN:
		if (suspend_ops->again) {
			pm_debug_get_starttime (PM_TIME_PLAT_AGAIN, hal_gettime_ns());
			ret = suspend_ops->again(mode);
			pm_debug_record_time (PM_TIME_PLAT_AGAIN, hal_gettime_ns());
		}
		break;
	case PM_SUSPEND_OPS_TYPE_AGAIN_LATE:
		if (suspend_ops->again_late) {
			pm_debug_get_starttime (PM_TIME_PLAT_AGAIN_LATE, hal_gettime_ns());
			ret = suspend_ops->again_late(mode);
			pm_debug_record_time (PM_TIME_PLAT_AGAIN_LATE, hal_gettime_ns());
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

