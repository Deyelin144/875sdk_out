#include "dev_battery.h"
#include <sunxi_hal_power.h>
#include "kernel/os/os.h"
#include "compiler.h"
#include "../atcmd/mcu_at_device.h"

static struct power_dev rdev;
typedef void (*irq_callback)(void *arg);
static irq_callback s_detect_cb = NULL;
static int s_battery_inited = 0;
static XR_OS_Thread_t s_dev_battery_thread;
static XR_OS_Semaphore_t s_dev_battery_sem;

__nonxip_text void dev_battery_irq_handler(void *args)
{
    if (XR_OS_SemaphoreIsValid(&s_dev_battery_sem)) {
        XR_OS_SemaphoreRelease(&s_dev_battery_sem);
    }
}

static char s_isCharge = 0;
static char s_isChargeFull = 0;
static char s_update_directly = 0;         //直接更新
static void _dev_battery_task(void *arg)
{
    while(1) {
        if (XR_OS_OK != XR_OS_SemaphoreWait(&s_dev_battery_sem, XR_OS_WAIT_FOREVER)) {
            continue;
        }

        if(0 == s_update_directly){
            dev_mcu_charge_info_get(&s_isCharge, &s_isChargeFull);
        }

        if(s_detect_cb != NULL)
        {
            s_detect_cb(NULL);  //如果充电拔插状态改变，需要通知到电池管理模块
        }
        s_update_directly = 0;
        drv_logi("battery isCharge:%d, isChargeFull : %d\n", s_isCharge, s_isChargeFull);
    }
}

void dev_battery_init(irq_callback cb)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;

    if (s_battery_inited == 1) {
        ret = 0;
        drv_loge("battery inited already.");
        goto exit;
    }

    ret = hal_power_init();
    if (ret) {
        drv_loge("battery init fail.");
        goto exit;
    }

    ret = hal_power_get(&rdev);
    if (ret) {
        drv_loge("battery get fail.");
        goto exit;
    }

    s_detect_cb = cb;

    status = XR_OS_SemaphoreCreate(&s_dev_battery_sem, 1, 1);
    if (XR_OS_OK != status) {
        drv_loge("semaphore create fail: %d.", status);
        goto exit;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&s_dev_battery_thread,
                                "dev_battery_task",
                                _dev_battery_task,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                1024 * 2)) {
        drv_loge("create task err.....");
        goto exit;
    }

    s_battery_inited = 1;
    drv_logi("battery init success.");
    ret = 0;
exit:
	if (ret != 0) {
        if (XR_OS_SemaphoreIsValid(&s_dev_battery_sem)) {
            XR_OS_SemaphoreDelete(&s_dev_battery_sem);
        }
	}
}

int dev_battery_get_voltage(void)
{
    return hal_power_get_vbat(&rdev);
}

int dev_battery_get_charge_state(void)
{
    return s_isCharge;
}

int dev_battery_get_charge_full_state(void)
{
    return s_isChargeFull;
}

void dev_battery_bat_info_set(char isCharge, char isChargeFull)
{
    s_isCharge      = isCharge;
    s_isChargeFull  = isChargeFull;
    s_update_directly = 1;
    dev_battery_irq_handler(NULL);
}



