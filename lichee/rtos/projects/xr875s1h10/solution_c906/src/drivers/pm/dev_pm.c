#include <stdio.h>
#include <string.h>
#include <hal_cmd.h>
#include <sunxi_hal_rtc.h>
#include <sunxi_hal_power.h>
#include "pm_wakesrc.h"
#include <pm_rpcfunc.h>
#include "../drv_log.h"
#include "dev_pm.h"
#include "pm_notify.h"
#include "prcm-sun20iw2/prcm.h"
#include "../led/dev_led.h"
#include "../extio/dev_extio.h"
#include "../rtc/dev_rtc.h"
#include "../sd/dev_sd.h"
#include "../atcmd/at_command_handler.h"
#include "kernel/os/os.h"
#include "../atcmd/mcu_at_device.h"
#include "realize/realize_mcu/realize_mcu.h"
#include "sdk_runtime.h"

#define DRV_VERB 0
#define DRV_INFO 1
#define DRV_DBUG 1
#define DRV_WARN 1
#define DRV_EROR 1

#if DRV_VERB
#define dev_pm_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_pm_verb(fmt, ...)
#endif

#if DRV_INFO
#define dev_pm_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_pm_info(fmt, ...)
#endif

#if DRV_DBUG
#define dev_pm_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_pm_debug(fmt, ...)
#endif

#if DRV_WARN
#define dev_pm_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_pm_warn(fmt, ...)
#endif

#if DRV_EROR
#define dev_pm_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_pm_error(fmt, ...)
#endif

#define DEV_PM_RUNNING 0
#define DEV_PM_RESUME_FINISH 1

extern void platform_watchdog_disable(void);

static volatile char s_init_cb = 0;
static int s_pm_fail = 0;
static int s_cur_backlight_mode = BACKLIGHT_MODE_PWM;//上电默认使用PWM背光
static uint8_t s_goto_hibernation = 0;
static uint8_t s_pm_resume_status = DEV_PM_RUNNING;
static uint8_t s_wakeup_alarm_flag = 0;//是否设置rtc闹钟唤醒，可以防止休眠唤醒的时候上报RTC唤醒的同时，还会触发rtc透传的中断，导致app响应两次闹钟
extern void st7789v3_lcd_pwm_bl_enable(int is_enable);
extern void ococci_wait_lcd_dma_idle(void);
extern int dev_rtc_set_alarm_callback(void);

extern void hal_csi_jpeg_suspend(char mode);
extern void hal_csi_jpeg_resume(char mode);
extern int drv_tp_axs5106l_suspend(void);
extern int drv_tp_axs5106l_resume(void);
extern int touch_key_sleep_check();

void mod_wakeup_pm_suspend(char mode);
void mod_wakeup_pm_resume(char mode);

int dev_pm_wakeupalarm_status_get(void)
{
    return s_wakeup_alarm_flag;
}

int dev_pm_poweroff_status_get(void)
{
    int poweroff_status = sdk_runtime_get_poweroff_status();
    if (poweroff_status < 0 || poweroff_status > 1) {  //只能是0或1
        dev_pm_warn("Suspicious poweroff status: %d\n", poweroff_status);
        poweroff_status = -1;  // 返回错误
    }

    return poweroff_status;
}

blacklight_mode_t dev_lcd_get_backlight_mode(void)
{
	return s_cur_backlight_mode;
}

int dev_lcd_backlight_switch(blacklight_mode_t type)
{
    printf("dev_lcd_backlight_switch type = %d, s_cur_backlight_mode = %d\n", type, s_cur_backlight_mode);
    if (type == BACKLIGHT_MODE_PWM && s_cur_backlight_mode == BACKLIGHT_MODE_GPIO) {
        st7789v3_lcd_pwm_bl_enable(1);
        int duty = sdk_runtime_get_backlight_percent();
        dev_pm_info("dev_lcd_backlight_switch duty = %d\n",duty);
        sdk_runtime_set_backlight_percent(duty);
        dev_mcu_lcd_bl_lp_set(1);//拉高休眠屏幕背光引脚，低有效
        s_cur_backlight_mode = BACKLIGHT_MODE_PWM;
        return s_cur_backlight_mode;
    } else if (type == BACKLIGHT_MODE_GPIO && s_cur_backlight_mode == BACKLIGHT_MODE_PWM) {
        st7789v3_lcd_pwm_bl_enable(0);
        dev_mcu_lcd_bl_lp_set(0);//拉低休眠屏幕背光引脚
        s_cur_backlight_mode = BACKLIGHT_MODE_GPIO;
        return s_cur_backlight_mode;
    } else {
        dev_pm_error("Unsupported backlight mode: %d\n", type);
        return -1;
    }
}

void pm_lcd_resume(char mode)
{
    dev_pm_info("lcd_resume sucess\n");
}

void pm_lcd_suspend(char mode)
{
    ococci_wait_lcd_dma_idle();//等待屏幕数据发送完成
    if (PM_MODE_HIBERNATION == mode) {	
        // dev_mcu_lcd_reset_set(0);//进入深休关闭显示复位
        // XR_OS_MSleep(100);
	}
    dev_pm_info("lcd_susupend sucess\n");
}

void pm_mcu_suspend(char mode)
{
    int wait_num = 0;
    if (PM_MODE_HIBERNATION == mode) {	
        dev_mcu_sleep_set(2,0);
	}else{
        dev_mcu_sleep_set(1,0);
    }
    while(!at_cmd_queue_is_empty()){         //等待at发送完成，最多1s
        XR_OS_MSleep(10);
        if(++wait_num > 100){
            dev_pm_warn("at_cmd_queue_is_empty timeout\n");
            break;
        }
    }
    mcu_at_uart_suspend();     //关闭at串口
    dev_pm_info("pm_mcu_suspend sucess\n");
}

void pm_mcu_resume(char mode)
{
    mcu_at_uart_resume();      //开启at串口
    dev_pm_info("pm_mcu_resume sucess\n");
}

void pm_other_suspend(char mode)
{
    drv_tp_axs5106l_suspend();
    if (mode == PM_MODE_HIBERNATION) {
        dev_pm_iface_clear_wakeup_alarm();
        hal_rtc_alarm_irq_enable(0);
    }
    dev_pm_info("pm_other_suspend sucess\n");
}

void pm_other_resume(char mode)
{
    if (mode == PM_MODE_HIBERNATION) {
        hal_rtc_alarm_irq_enable(1);
    }
    drv_tp_axs5106l_resume();
    dev_pm_info("pm_other_resume sucess\n");
}

/*
led和mcu 唤醒通知引脚复用，
休眠时切换引脚                          led模式 -->中断模式 
唤醒后，先清除唤醒源，然后切换引脚          中断模式  --> led模式
 */
void pm_led_suspend(char mode)
{
    dev_led_suspend(0);
}

void pm_led_resume(char mode)
{
    dev_led_resume(0);
}

void pm_camera_suspend(char mode)
{
    dev_mcu_cam_pwdn_set(1);
}

void pm_camera_resume(char mode)
{
    dev_mcu_cam_pwdn_set(0);
}


void pm_tfcard_suspend(char mode)
{
    dev_sd_suspend();
}

void pm_tfcard_resume(char mode)
{
    dev_sd_resume();
}

// standby模式唤醒后, 会把这个寄存器的值设置为255, 需要清除这个寄存器的值
void gprcm_reg_clear(char mode)
{
    printf("[%s]clear try times\n", __func__);
#define GPRCM_REG_WRITE_32(reg, value) (*(volatile uint32_t *)(reg) = (value))
    GPRCM_REG_WRITE_32(0x40050210, 0);
}

/**
 * @description: 实际平台唤醒源的值对应的业务的唤醒类型的数组集合，外部填充
 * 外部需要将唤醒源对应的业务的值注册到该数组中，唤醒后pm模块会轮询该数组，匹配成功则将唤醒事件类型回调通知出来
 */
static dev_pm_wakeup_map_t s_wakeup_event_map[] = {
	{PM_WAKEUP_SRC_WKIO0, 	WAKEUP_MCU},//usb、touch、voice在mcu上，需要外部中断来触发
	// {PM_WAKEUP_SRC_WKIO1, 	WAKEUP_PWRKEY},//确认键唤醒
    {PM_WAKEUP_SRC_WKTMR,   WAKEUP_TIMER},
	{PM_WAKEUP_SRC_WLAN, 	WAKEUP_WLAN},
	{PM_WAKEUP_SRC_ALARM0,  WAKEUP_ALARM},
};

/**
 * @description: mcu唤醒源映射
 */
static dev_pm_wakeup_map_t s_wakeup_event_mcu_map[] = {
    {WAKE_SRC_ERROR,        WAKEUP_MCU},
    {WAKE_SRC_NULL,         WAKEUP_MCU},
    {WAKE_SRC_USB,          WAKEUP_MCU_USB},
	{WAKE_SRC_TOUCHKEY, 	WAKEUP_MCU_TOUCH_KEY},
	{WAKE_SRC_VOICE,        WAKEUP_MCU_VOICE},
    {WAKE_SRC_RTC,          WAKEUP_ALARM},
};

/**
 * @description: 休眠前需要处理的事件数组集合，外部填充
 * 外部需要将休眠前需要执行的事件注册到该数组中，pm模块在进入休眠前会按顺序执行该数组中的函数
 */
static pm_module_event_t s_register_sleep_prepare_events[] = {	
    {"lcd", true, pm_lcd_suspend},
    {"led", true, pm_led_suspend},          //
    {"other", true, pm_other_suspend},
    {"tfcard", true, pm_tfcard_suspend},
    {"mcu", true, pm_mcu_suspend},
    {"csi_jpeg", true, hal_csi_jpeg_suspend},
};

/**
 * @description: 唤醒后需要处理的事件数组集合，外部填充
 * 外部需要将唤醒后需要执行的事件注册到该数组中，pm模块在唤醒后会按顺序执行该数组中的函数
 */
static pm_module_event_t s_register_wakeup_events[] = {
    {"clear_gprcm", true, gprcm_reg_clear},
    {"mcu", true, pm_mcu_resume},
    {"lcd", true, pm_lcd_resume},
    {"led", true, pm_led_resume},
    {"other", true, pm_other_resume},
    {"tfcard", true, pm_tfcard_resume},
    {"csi_jpeg", true, hal_csi_jpeg_resume},
};

dev_pm_wakeup_src_t dev_pm_get_wakeup_src()
{
	int ret = dev_pm_iface_get_wakeup_src();
	printf("dev_pm_get_wakeup_src ret = %d\n", ret);

	for (int i = 0; i < sizeof(s_wakeup_event_map)/sizeof(s_wakeup_event_map[0]); i++) {
		if (ret == s_wakeup_event_map[i].wakeup_src) {
			dev_pm_info("wakeup event: %d", s_wakeup_event_map[i].wakeup_event_type);
			ret = s_wakeup_event_map[i].wakeup_event_type;
			break;
        }
	}

    //mcu唤醒源映射
    if ( WAKEUP_MCU == ret && PM_WAKEUP_SRC_WKIO0 == dev_pm_iface_get_wakeup_src()) {
        int mcu_src = dev_mcu_wakeSrc_get();
        for (int i = 0; i < sizeof(s_wakeup_event_mcu_map)/sizeof(s_wakeup_event_mcu_map[0]); i++) {
            if (mcu_src == s_wakeup_event_mcu_map[i].wakeup_src) {
                dev_pm_info("mcu wakeup event: %d", s_wakeup_event_mcu_map[i].wakeup_event_type);
                ret = s_wakeup_event_mcu_map[i].wakeup_event_type;

                //外部rtc唤醒,清空唤醒标志
                if ( get_rtc_chip_type()== DRV_RTC_TYPE_AT8563) {
                    if(ret == WAKEUP_ALARM){
                        dev_pm_info("wakeup by rtc int\n");
                        if (s_wakeup_alarm_flag == 1) {
                            s_wakeup_alarm_flag = 0;
                            dev_pm_info("wakeup by rtc alarm\n");
                            dev_rtc_clear_af();
                        }
                    }
                }
                
                break;
            }
        }
    }

	return (dev_pm_wakeup_src_t)ret;
}

int dev_pm_poweroff_src_set(void)
{
    printf("dev_pm_poweroff_src_set\n");
    dev_pm_iface_clear_delay_wakeup_time();
    dev_pm_iface_clear_wakeup_alarm();
    // dev_pm_iface_poweroff_src_set();
}

int dev_pm_set_exit_wakeup_src(dev_pm_wakeup_type_t type)
{
    dev_pm_info("dev_pm_set_exit_wakeup_src = %d\n", type);
    dev_pm_iface_set_exit_wakeup_src(type);
}

int dev_pm_clear_exit_wakeup_src(dev_pm_wakeup_type_t type)
{
    dev_pm_info("dev_pm_clear_exit_wakeup_src type = %d\n", type);
    dev_pm_iface_clear_exit_wakeup_src(type);
}

void dev_pm_set_timer_wakeup_src(dev_pm_timer_wakeup_t *timer_wakeup)
{
    dev_pm_info("timer_wakeup->type = %d, timer_wakeup->param.delay_ms = %u\n", timer_wakeup->type, timer_wakeup->param.delay_ms);
	if (DEV_PM_TIMER_WAKEUP_DELAY_MODE == timer_wakeup->type) {
		dev_pm_iface_set_delay_wakeup_time(timer_wakeup->param.delay_ms);//进入休眠后多少ms唤醒.
	} else if (DEV_PM_TIMER_WAKEUP_ALARM_MODE == timer_wakeup->type) {
        if (get_rtc_chip_type() == DRV_RTC_TYPE_AT8563) {
            dev_pm_iface_set_wakeup_alarm_exit_rtc(timer_wakeup->param.alarm);
        } else {
            dev_pm_iface_set_wakeup_alarm_innerrtc(timer_wakeup->param.alarm);
        }
        s_wakeup_alarm_flag = 1;
	}
	return 0;
}

int dev_pm_clear_timer_wakeup_src(dev_pm_wakeup_type_t type)
{
    dev_pm_info("dev_pm_clear_timer_wakeup_src type = %d\n", type);
    if (DEV_PM_TIMER_WAKEUP_DELAY_MODE == type) {
		dev_pm_iface_clear_delay_wakeup_time();	
	} else if (DEV_PM_TIMER_WAKEUP_ALARM_MODE == type) {
        if (get_rtc_chip_type() == DRV_RTC_TYPE_MAX) {
            dev_pm_iface_clear_wakeup_alarm();
        }
	} else if (DEV_PM_TIMER_WAKEUP_MAX_MODE == type) {
		dev_pm_iface_clear_delay_wakeup_time();
        if (get_rtc_chip_type() == DRV_RTC_TYPE_MAX) {
            dev_pm_iface_clear_wakeup_alarm();
        }
	}	
	return 0;
}

static int dev_power_notify_cb(suspend_mode_t mode, pm_event_t event, void *arg)
{
	switch (event) {
    case PM_EVENT_SYS_PERPARED://系统整体休眠动作入口
        printf("========PM_EVENT_SYS_PERPARED\n");
        break;
	case PM_EVENT_PERPARED://核个体休眠动作入口
        printf("========PM_EVENT_PERPARED\n");
        //dev_extio_clear_irq();//清掉扩展io的中断，目的是扩展io中引脚恢复高电平，才能正常进入休眠
        /*清除wakeupio的pending，目的是因为在休眠过程中按下的按键中断一直保存，导致定时器唤醒的同时上报按键，
        为什么不需要清除定时器的pending，因为在设置wakeupio开启后立刻执行了timer的回调，这个时候系统处于running状态，不会影响休眠计数，
        中断回调里会清除timer的pending*/
        // clear_wakeup_io_pending();
        // pm_enable_wakeio_irq();//开启wakupio中断，防止休眠过程中，wakupio中断被触发，导致下一次无法唤醒
		break;
	case PM_EVENT_FINISHED://核个体休眠动作出口
		printf("========PM_EVENT_FINISHED\n");
		break;
	case PM_EVENT_SYS_FINISHED://系统整体休眠动作出口
        s_pm_resume_status = DEV_PM_RESUME_FINISH;
        printf("========PM_EVENT_SYS_FINISHED\n");
		break;
    case PM_EVENT_SUSPEND_FAIL:
		printf("========PM_EVENT_SUSPEND_FAIL\n");
        s_pm_fail = 1;
		break;
	default:
		break;
	}

	return 0;
}

static pm_notify_t dev_power_notify = {
	.name = "dev_power",
	.pm_notify_cb = dev_power_notify_cb,
	.arg = NULL,
};


static int dev_pm_sleep_mode_enter(void)
{
    int ret = 0;

    /**
     * val      : 0 => wakeup IO 0  
     * edge_mode: 1 => 1:positive edge, 0:negative edge
     * time_s   : 2 => [0 - 3] seconds, 0 means 300ms
     */
    // ret = pm_set_wupio(4, 0, 2);//电源键唤醒
    // if (ret < 0) {
    //     drv_loge("set wakeup4 io fail: %d.", ret);
    // }
    // ret = pm_set_wupio(6, 0, 2);
    // if (ret < 0) {
    //     drv_loge("set wakeup6 io fail: %d.", ret);
    // }

    ret = pm_trigger_suspend(PM_MODE_SLEEP);
    if (0 != ret) {
        dev_pm_error("pm trigger fail: %d.", ret);
    }

    return ret;
}

static int dev_pm_standby_mode_enter(void)
{
    int ret = -1;

    char *ip = sdk_runtime_get_p2p_wakeup_ip();
    printf("dev_pm_iface_set_p2p_wakeup_ip_str: %s\n", ip);
    if (ip != NULL && ip[0] != '\0') {
        dev_pm_iface_set_p2p_wakeup_ip_str(0, 1, ip);
    } else {
        dev_pm_iface_set_p2p_wakeup_ip_str(0, 1, "255.255.255.255");
    }
    int ret1 = -1, ret2 = -1, ret3 = -1;
    int retry = 0;
    for (retry = 0; retry < 5; retry++) {
        ret1 = dev_pm_iface_net_wlan_set_p2p_hostsleep(1);
        ret2 = dev_pm_iface_set_wlan_filter_type(0x3f);
        ret3 = dev_pm_iface_set_lp_param(10);
        if (ret1 == 0 && ret2 == 0 && ret3 == 0) {
            break; // 三项都成功，退出循环
        } else {
            dev_pm_warn("pm_standby_mode_enter: wlan param set fail, retry %d (ret1=%d, ret2=%d, ret3=%d)\n", retry+1, ret1, ret2, ret3);
            hal_msleep(10);
        }
    }
    if (ret1 != 0 || ret2 != 0 || ret3 != 0) {
        dev_pm_error("pm_standby_mode_enter: wlan param set fail after 5 retries! (ret1=%d, ret2=%d, ret3=%d)\n", ret1, ret2, ret3);
    }

    ret = pm_trigger_suspend(PM_MODE_STANDBY);
    if (0 != ret) {
        dev_pm_error("pm trigger fail: %d.", ret);
    }

    return ret;
}

static int dev_pm_hibernation_mode_enter(void)
{
    int ret = 0;
    int retry_cnt = 10;

    // 深休时忽略计数锁, 直接休眠, 避免因为有外设还在持有计数锁导致的休眠失败
    pm_wakelock_ignore(1);

    while(retry_cnt--) {
        ret = pm_trigger_suspend(PM_MODE_HIBERNATION);
        printf("[%s]ret:%d.\n",__FUNCTION__, ret);

        pm_wakelocks_showall();

        if (0 != ret) {
            dev_pm_error("pm trigger fail: %d.", ret);
            hal_msleep(100);
        } else {
            break;
        }
    }
    printf("pm enter hibernation.\n");
    return ret;
}

static void dev_pm_wakeup_handle(dev_power_off_mode_t mode)
{
    int ret1 = -1, ret2 = -1, ret3 = -1;
    int retry = 0;
    for (retry = 0; retry < 5; retry++) {
        ret1 = dev_pm_iface_net_wlan_set_p2p_hostsleep(0);
        ret2 = dev_pm_iface_set_wlan_filter_type(0x0);
        ret3 = dev_pm_iface_set_lp_param(1);
        if (ret1 == 0 && ret2 == 0 && ret3 == 0) {
            break; // 三项都成功，退出循环
        } else {
            dev_pm_warn("wakeup_handle: wlan param set fail, retry %d (ret1=%d, ret2=%d, ret3=%d)\n", retry+1, ret1, ret2, ret3);
            hal_msleep(10);
        }
    }
    if (ret1 != 0 || ret2 != 0 || ret3 != 0) {
        dev_pm_error("wakeup_handle: wlan param set fail after 5 retries! (ret1=%d, ret2=%d, ret3=%d)\n", ret1, ret2, ret3);
    }
    (void)sdk_runtime_sync_time_from_rtc();
    dev_pm_on(mode);
}

int dev_pm_on(dev_power_off_mode_t mode)
{
    // hal_uart_enable_rx(1); // 使能uart1 rx中断
}

int dev_pm_goto_sleep(dev_power_off_mode_t mode)
{
    int ret = 0;
    uint8_t wait_time = 0;
    int i = 0;
    s_pm_fail = 0;
    s_pm_resume_status = DEV_PM_RUNNING;

    printf("dev_pm_goto_sleep mode = %d\n", mode);

    // 触摸后要求不能进休眠，引擎时序不能完全避免，SDK增加TK时间检测，避免触摸后进入休眠
    if (touch_key_sleep_check() && (mode == DEV_POWER_OFF_MODE_STANDBY)) {
        dev_pm_error("sleep failed cause touch key\n");
        return DEV_PM_ERROR_SUPPEND_FAIL;
    }

    if (DEV_POWER_OFF_MODE_HIBERNATION == mode) {
        s_goto_hibernation = 1;
    }


    for (i = 0; i < sizeof(s_register_sleep_prepare_events)/sizeof(s_register_sleep_prepare_events[0]); i++) {	
		if (s_register_sleep_prepare_events[i].enabled) {
			dev_pm_info("start execute sleep event %s", s_register_sleep_prepare_events[i].name);
			s_register_sleep_prepare_events[i].eventHandler(mode);
		}
	}

    if (!s_init_cb) {
        pm_notify_register(&dev_power_notify);
        s_init_cb = 1;
    }
retry_sleep:
    dev_pm_set_exit_wakeup_src(DEV_PM_WAKEUP_SRC_USB);
    switch (mode) {
        case DEV_POWER_OFF_MODE_SLEEP:
            dev_pm_info("enter sleep mode.");
            ret =  dev_pm_sleep_mode_enter();
            break;
        case DEV_POWER_OFF_MODE_STANDBY:
            dev_pm_info("enter standby mode.");
            ret = dev_pm_standby_mode_enter();
            break;
        case DEV_POWER_OFF_MODE_HIBERNATION:
            dev_pm_info("enter hibernation mode.");
            platform_watchdog_disable();
            ret =  dev_pm_hibernation_mode_enter();
            break;
        default:
            dev_pm_info("not support mode.");
            ret = DEV_PM_ERROR_SUPPEND_FAIL;
    }

    while (s_pm_resume_status != DEV_PM_RESUME_FINISH) {
        hal_msleep(5);
        wait_time++;
        if (0 == wait_time % 1000) {
            printf("[%s] wait power suspend finish.\n", __FUNCTION__);
            wait_time = 0;
            break;
        }
    }
    
    if (s_pm_fail == 1 || ret != 0) {
        s_pm_fail = 0;
        dev_pm_info("pm suspend failed\n");
        ret = DEV_PM_ERROR_SUPPEND_FAIL;
        goto exit;
    }

exit:   
    if (PM_MODE_HIBERNATION != mode || (ret != 0 && PM_MODE_HIBERNATION == mode)) {
		for (i = 0; i < sizeof(s_register_wakeup_events)/sizeof(s_register_wakeup_events[0]); i++) {
			if (s_register_wakeup_events[i].enabled) {
				dev_pm_info("start execute wakeup event %s\n", s_register_wakeup_events[i].name);
				s_register_wakeup_events[i].eventHandler(mode);
			}
		}
	}
    dev_pm_wakeup_handle(mode);
    s_goto_hibernation = 0;

    if ((s_pm_fail == 1 || ret != 0 )&& WAKEUP_MCU_TOUCH_KEY == dev_pm_get_wakeup_src()) { 
        dev_pm_error("sleep failed cause touch key\n");
        realize_mcu_send_app_msg("touch_key", 1, 5);
    }

    return ret;
}

uint8_t dev_pm_is_goto_hibernation()
{
    return s_goto_hibernation;
}

#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#endif

#ifdef CONFIG_COMPONENTS_PM
static int ocucci_rtc_resume(struct pm_device *dev, suspend_mode_t mode)
{
    printf("ocucci_rtc_resume mode = %d\n", mode);
    int ret = dev_pm_iface_get_wakeup_src();

    if (ret != PM_WAKEUP_SRC_ALARM0 && s_wakeup_alarm_flag == 1) {//这里代表设置了闹钟唤醒进入休眠，但是被其他情况唤醒了，这时候要保留闹钟的中断上报给app
        dev_rtc_set_alarm_callback();
        s_wakeup_alarm_flag = 0;
    }
    return 0;
}

static int ocucci_rtc_suspend(struct pm_device *dev, suspend_mode_t mode)
{
    return 0;
}

struct pm_devops pm_ocucci_rtc_ops = {
       .suspend = ocucci_rtc_suspend,
       .resume = ocucci_rtc_resume,
};

struct pm_device pm_ocucci_rtc = {
       .name = "pm_ocucci_rtc_ops",
       .ops = &pm_ocucci_rtc_ops,
};
#endif

void dev_pm_init(void)
{
    // 先注册，后休眠，先唤醒
#ifdef CONFIG_COMPONENTS_PM
    if (get_rtc_chip_type() == DRV_RTC_TYPE_MAX) {//使用内部rtc才注册
        pm_devops_register(&pm_ocucci_rtc);
    }
#endif
}

static int cmd_pm_goto_sleep(int argc, char **argv)
{
    uint8_t mode;

    if (argc < 2) {
        printf("Usage: pm_goto_sleep <mode>\n");  // 更准确的使用说明
        printf("mode: 2-standby, 3-hibernation\n");  // 添加参数说明
        return -1;
    }

    printf("pm_goto_sleep\n");

    mode = strtol(argv[1], NULL, 0);
    printf("mode = %d", mode);

    // int ret = dev_pm_goto_sleep(mode);
    // if (ret != 0) {
    //     printf("cmd go to sleep failed\n");
    // }
    dev_lcd_backlight_switch(BACKLIGHT_MODE_GPIO);
    dev_pm_goto_sleep(mode);
    dev_lcd_backlight_switch(BACKLIGHT_MODE_PWM);  
    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_pm_goto_sleep, pm_goto_sleep, occucci pm test);

// 添加唤醒源设置命令，pm_set_exit_wakeup 1
static int cmd_pm_set_exit_wakeup(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: pm_set_exit_wakeup <type>\n");
        printf("type: 1-EXTI, 3-USB\n");
        return -1;
    }

    uint8_t type = strtol(argv[1], NULL, 0);
    dev_pm_set_exit_wakeup_src(type);
    printf("Set exit wakeup source: %d\n", type);
    
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_exit_wakeup, pm_set_exit_wakeup, set exit wakeup source);

// 添加定时器唤醒设置命令，pm_set_timer_wakeup 0 30000，pm_set_timer_wakeup 1 "2024-01-01 12:00:00"
// pm_set_timer_wakeup 1 "2025-12-18 21:25
static int cmd_pm_set_timer_wakeup(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: pm_set_timer_wakeup <type> <value>\n");
        printf("type: 0-delay mode, 1-alarm mode\n");
        printf("value: delay_ms for type 0, time string for type 1\n");
        return -1;
    }

    dev_pm_timer_wakeup_t timer_cfg = {0};
    timer_cfg.type = strtol(argv[1], NULL, 0);
    
    if (timer_cfg.type == DEV_PM_TIMER_WAKEUP_DELAY_MODE) {
        timer_cfg.param.delay_ms = strtol(argv[2], NULL, 0);
    } else if (timer_cfg.type == DEV_PM_TIMER_WAKEUP_ALARM_MODE) {
        for(int i=0;i<argc;i++){
            printf("dargv[%d]: %s\n", i, argv[i]);
        }
        // strncpy(timer_cfg.param.alarm, argv[2], sizeof(timer_cfg.param.alarm) - 1);

        sprintf(timer_cfg.param.alarm,"%s %s",argv[2]+1,argv[3]);
        // sprintf(timer_cfg.param.alarm,"%s %s",argv[2],argv[3]);
        printf("Set alarm wakeup: %s\n",timer_cfg.param.alarm);
    }
    dev_pm_set_timer_wakeup_src(&timer_cfg);
    printf("Set timer wakeup: type=%d\n", timer_cfg.type);
    
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_timer_wakeup, pm_set_timer_wakeup, set timer wakeup source);

// 添加清除唤醒源命令，pm_clear_wakeup
static int cmd_pm_clear_wakeup(int argc, char **argv)
{
    // 清除退出唤醒源
    dev_pm_clear_exit_wakeup_src(DEV_PM_WAKEUP_SRC_EXIT);
    
    // 清除所有定时器唤醒源
    dev_pm_clear_timer_wakeup_src(DEV_PM_TIMER_WAKEUP_MAX_MODE);
    
    dev_pm_info("All wakeup sources cleared\n");
    
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_clear_wakeup, pm_clear_wakeup, clear wakeup source);

static int cmd_pm_time_test(int argc, char **argv)
{
    if (2 > argc) {
        printf("param num is invalid.\n");
        return -1;
    }
    if(0 == memcmp(argv[1], "clear", strlen("clear"))) {
        dev_pm_poweroff_src_set();
    } else if (0 == memcmp(argv[1], "set", strlen("set"))) {
        
    }
    
    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_time_test, pm_time_test, pm_time_test);

void pm_test_task(void *arg)
{
    // 等待40秒
    hal_msleep(40000);
    
    while (1) {
        // 1. 设置唤醒源
        // 设置按键唤醒 (GPIO1作为按键输入)
        dev_pm_set_exit_wakeup_src(DEV_PM_WAKEUP_SRC_EXIT);
        
        // 设置定时器唤醒 (10秒后唤醒)
        dev_pm_timer_wakeup_t timer_cfg = {
            .type = DEV_PM_TIMER_WAKEUP_DELAY_MODE,
            .param.delay_ms = 30000,  // 30秒
        };
        dev_pm_set_timer_wakeup_src(&timer_cfg);
        
        // 2. 进入待机模式
        dev_pm_info("Enter standby mode...\n");
        dev_pm_goto_sleep(DEV_POWER_OFF_MODE_STANDBY);
        
        // 3. 系统被唤醒后
        dev_pm_info("System wakeup, src = %d\n", dev_pm_get_wakeup_src());
        
        // 4. 清除所有唤醒源
        dev_pm_clear_exit_wakeup_src(DEV_PM_WAKEUP_SRC_EXIT);
        dev_pm_clear_timer_wakeup_src(DEV_PM_TIMER_WAKEUP_MAX_MODE);
        
        // 5. 延时一段时间，让系统稳定
        hal_msleep(1000);
    }
}
