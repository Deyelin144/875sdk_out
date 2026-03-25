#ifndef __DEV_PM_IFACE_H__
#define __DEV_PM_IFACE_H__  

#include <stdint.h>
#include <pm_base.h>

#define CODEC_ADC_MBIAS_CTRL_REG           (0x4004B000 + 0x0008)
#define CODEC_ADC_MBIAS_EN                 (1 << 7)
#define CODEC_ADC_MBIAS_2_5V               (0x3 << 5)

#define DEV_POWER_REG_WRITE_32(reg, value) (*(volatile uint32_t *)(reg) = (value))
#define DEV_POWER_REG_READ_32(reg)         (*(volatile uint32_t *)(reg))

#define DEV_POWER_DELAY_WAKEUP_TIME (60 * 1000)

typedef enum {
	DEV_PM_TIMER_WAKEUP_DELAY_MODE = 0,
	DEV_PM_TIMER_WAKEUP_ALARM_MODE,	
	DEV_PM_TIMER_WAKEUP_MAX_MODE
} dev_pm_timer_type_t;

typedef enum {
    DEV_PM_WAKEUP_SRC_NONE = 0,    // No wakeup source  
    DEV_PM_WAKEUP_SRC_EXIT,        // Key wakeup source  
    DEV_PM_WAKEUP_SRC_RTC,         // RTC wakeup source
    DEV_PM_WAKEUP_SRC_USB,         // USB wakeup source    
    DEV_PM_WAKEUP_SRC_WLAN,        // WLAN wakeup source
    DEV_PM_WAKEUP_SRC_USER,        // 用户自定义唤醒源    
    DEV_PM_WAKEUP_SRC_MAX          // Max wakeup source    
} dev_pm_wakeup_type_t; 

int dev_pm_iface_get_wakeup_src(void);//获取底层唤醒源
int dev_pm_iface_set_exit_wakeup_src(dev_pm_wakeup_type_t type);//设置外部中断相关的唤醒源
void dev_pm_iface_set_delay_wakeup_time(int time);//设置定时器的唤醒源
int dev_pm_iface_clear_delay_wakeup_time(void);//清除定时器的唤醒源
void dev_pm_iface_set_wakeup_alarm_exit_rtc(char *time);//设置闹钟的唤醒源
void dev_pm_iface_set_wakeup_alarm_innerrtc(char *time);
int dev_pm_iface_clear_wakeup_alarm(void);//清除闹钟的唤醒源
int dev_pm_iface_clear_exit_wakeup_src(dev_pm_wakeup_type_t type);//清除外部中断相关的唤醒源
int dev_iface_pm_init();//初始化pm
int dev_pm_iface_set_p2p_wakeup_ip_str(uint8_t session, uint8_t enable, const char *ip_str);//设置指定的ip唤醒
int dev_pm_iface_net_wlan_set_p2p_hostsleep(int enable);
int dev_pm_iface_set_wlan_filter_type(uint32_t type);//设置休眠后唤醒包的过滤条件
int dev_pm_iface_set_lp_param(uint32_t param);//设置休眠后wifi的dtim参数
int dev_pm_iface_poweroff_src_set(void);

#endif