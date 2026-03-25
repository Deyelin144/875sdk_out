#ifndef _DEV_PM_H_
#define _DEV_PM_H_

#include "drv_pm_iface.h"

#define DEV_PM_NAME_MAX_SIZE (20)

typedef enum {
    DEV_POWER_OFF_MODE_NONE        = 0,
    DEV_POWER_OFF_MODE_SLEEP       = 1, /**< 设备低功耗 sleep */
    DEV_POWER_OFF_MODE_STANDBY     = 2, /**< 设备低功耗 standby */
    DEV_POWER_OFF_MODE_HIBERNATION = 3, /**< 设备低功耗 hibernation */
    DEV_POWER_OFF_MODE_DOWN        = 4, /**< 设备掉电 */
} dev_power_off_mode_t;

typedef enum {
    DEV_PM_ERROR_NONE = 0,    //No error
    DEV_PM_ERROR_SUPPEND_FAIL = -1,       //Suspend failure
    DEV_PM_ERROR_UNKOWN_WAKEUP = -2,    //Unknown wakeup source
    DEV_PM_ERROR_MAX           //Max error
} dev_pm_error_t;

typedef struct {
	dev_pm_timer_type_t type;
	union {
		uint32_t delay_ms;
		char alarm[20];	
	} param;	
} dev_pm_timer_wakeup_t;

typedef enum {
	WAKEUP_NORMAL,	
    WAKEUP_MCU,
    WAKEUP_MCU_USB,
    WAKEUP_MCU_VOICE,
    WAKEUP_MCU_TOUCH_KEY,
    WAKEUP_MCU_RTC,
    WAKEUP_PWRKEY,
    WAKEUP_TIMER,
    WAKEUP_WLAN,
	WAKEUP_ALARM,
} dev_pm_wakeup_src_t;

typedef enum {
    DEV_EXTIO_RADAR1,
    DEV_EXTIO_RADAR2,
    DEV_EXTIO_CHARGE,
    DEV_EXTIO_MAX,
} dev_pm_extio_wakeup_src_t;

typedef struct {
	/* 系统唤醒源的返回值 */
	int wakeup_src;
	/* 实际业务对应平台唤醒源的唤醒事件类型*/
	dev_pm_wakeup_src_t wakeup_event_type;
} dev_pm_wakeup_map_t;

typedef void (*eventHandler_t)(char mode);//mode:休眠模式

/*休眠前或唤醒后需要处理的事件结构体*/
typedef struct {
   /* 事件名称 */
   char name[DEV_PM_NAME_MAX_SIZE];
   /* 是否使能该事件 */
   bool enabled;
   /* 指向实际的事件的函数指针 */
   eventHandler_t eventHandler;
} pm_module_event_t;

typedef enum {
   BACKLIGHT_MODE_PWM = 0,		// 硬件PWM控制背光
   BACKLIGHT_MODE_GPIO,			// GPIO控制背光
   BACKLIGHT_MAX
} blacklight_mode_t;

int dev_pm_set_exit_wakeup_src(dev_pm_wakeup_type_t type);//设置外部io中断的唤醒源
int dev_pm_clear_exit_wakeup_src(dev_pm_wakeup_type_t type);//清除外部io中断的唤醒源
void dev_pm_set_timer_wakeup_src(dev_pm_timer_wakeup_t *timer_wakeup);//设置定时器的唤醒源
int dev_pm_clear_timer_wakeup_src(dev_pm_wakeup_type_t type);//清除定时器的唤醒源
uint8_t dev_pm_is_goto_hibernation();//判断是否进入深休
int dev_pm_goto_sleep(dev_power_off_mode_t mode);
dev_pm_wakeup_src_t dev_pm_get_wakeup_src();
int dev_pm_on(dev_power_off_mode_t mode);
void dev_pm_init(void);
int dev_lcd_backlight_switch(blacklight_mode_t type);
int dev_pm_poweroff_status_get(void);
int dev_pm_wakeupalarm_status_get(void);


#endif

