#include <stdio.h>
#include "../drv_log.h"
#include <pm_base.h>
#include "pm_wakesrc.h"
#include <pm_rpcfunc.h>
#include "prcm-sun20iw2/prcm.h"
#include <sunxi_hal_rtc.h>
#include <sunxi_hal_power.h>
#include <time.h>
#include "drv_pm_iface.h"
#include <pm_devops.h>
#include "kernel/os/os.h"
#include "cmd_defs.h"
#include "../rtc/dev_rtc.h"

extern int pm_risv_call_m33_set_time_to_wakeup(int mode);
extern int pm_risv_call_m33_set_io_to_wakeup(uint32_t val, uint32_t edge_mode, uint32_t time_s);
extern int pm_risv_call_m33_clear_wakeupio(uint32_t val);
extern void platform_watchdog_disable(void);
extern int ococci_rv_pm_wakeup_id(void);
extern int net_wlan_set_p2p_wakeupip(int ser, int en, uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3);
extern int net_wlan_set_p2p_hostsleep(int enable);
extern enum cmd_status cmd_wlan_exec(char *cmd);
extern int hal_rtc_register_clear_callback(void);

static int s_delay_wakeup_time = DEV_POWER_DELAY_WAKEUP_TIME;

int dev_pm_iface_get_wakeup_src(void)
{
    return ococci_rv_pm_wakeup_id();
}

int dev_pm_iface_poweroff_src_set(void)
{
    pm_risv_call_m33_set_io_to_wakeup(0, 1, 0);//设置USB唤醒源
    return 0;
}

int dev_pm_iface_set_exit_wakeup_src(dev_pm_wakeup_type_t type)//设置外部中断相关的唤醒源
{
    switch (type) {
        case DEV_PM_WAKEUP_SRC_EXIT:
            // pm_risv_call_m33_set_io_to_wakeup(1, 0, 0);//ENTER唤醒
            break;
        case DEV_PM_WAKEUP_SRC_USB:
            pm_risv_call_m33_set_io_to_wakeup(0, 1, 0);//USB唤醒
            break;
        default:
            break;
    }
    return 0;
}

int dev_pm_iface_clear_exit_wakeup_src(dev_pm_wakeup_type_t type)//清除外部中断相关的唤醒源
{
    switch (type) {
	 	case DEV_PM_WAKEUP_SRC_EXIT:
            // pm_risv_call_m33_clear_wakeupio(1);//关闭ENTER唤醒
            // pm_risv_call_m33_clear_wakeupio(0);     //mcu唤醒 (充电)
	 		break;
	 	case DEV_PM_WAKEUP_SRC_USB:
            pm_risv_call_m33_clear_wakeupio(0);//关闭USB唤醒
	 		break;
	 	default:
	 		break;
	}
	return 0;
}

// 计算星期几，0表示星期一，1表示星期二，依此类推
int calculate_weekday(time64_t time) 
{
    struct tm *tm_time = localtime(&time);
    int weekday = tm_time->tm_wday;
    // 将0表示周日转换为0表示周一
    return (weekday == 0) ? 6 : weekday - 1;
}

void dev_pm_iface_set_wakeup_alarm_innerrtc(char *time)
{
    int ret = -1;
	struct rtc_wkalrm wkalrm = {0};
    struct rtc_time rtc_tm = {0};
    uint32_t year = 0, month = 0, mday = 0, hour = 0, minute = 0;

    if (NULL == time) {
        printf("time is NULL");
        goto exit;
    }

    hal_rtc_alarm_irq_enable(0);
    hal_rtc_register_clear_callback();//清除rtc中断，防止上报两次rtc事件异常

    memset(&wkalrm, 0, sizeof(struct rtc_wkalrm));
    memset(&rtc_tm, 0, sizeof(struct rtc_time));

    sscanf(time, "%d-%d-%d %d:%d", &year, &month, &mday, &hour, &minute);

	wkalrm.time.tm_year = year - 1900;
    wkalrm.time.tm_mon = month - 1;
    wkalrm.time.tm_mday = mday;
    wkalrm.time.tm_hour = hour;
    wkalrm.time.tm_min = minute;
    wkalrm.time.tm_sec = 0;
    
    if (hal_rtc_gettime(&rtc_tm)) {//获取当前时间
		printf("sunxi rtc gettime error\n");
        goto exit;
	}

    rtc_tm.tm_year = rtc_tm.tm_year - 1900;
    rtc_tm.tm_mon = rtc_tm.tm_mon - 1;

    time64_t time_set = convert_tm_to_timestamp(&wkalrm.time);

    int weekday = calculate_weekday(time_set);
    wkalrm.time.tm_wday = weekday;//设置星期

    wkalrm.enabled = 1;
    printf("[ %s ][ %d ]  time : %d-%d-%d %d:%d \n", __FUNCTION__, __LINE__, 
       wkalrm.time.tm_year + 1900,    // 去掉&，年份要+1900
       wkalrm.time.tm_mon + 1 ,        // 月份从0开始，要+1
       wkalrm.time.tm_mday,           // 日期
       wkalrm.time.tm_hour,           // 小时
       wkalrm.time.tm_min);           // 分钟
    if (hal_rtc_setalarm(&wkalrm)) {
        printf("sunxi rtc setalarm error\n");
        goto exit; 
    }

    hal_rtc_alarm_irq_enable(1);
    hal_rtc_enable_wake();
    ret = 0;

exit:
    return ret;
}

void dev_pm_iface_set_wakeup_alarm_exit_rtc(char *time)
{
    int ret = -1;
	struct rtc_wkalrm wkalrm = {0};
    struct rtc_time rtc_tm = {0};
    uint32_t year = 0, month = 0, mday = 0, hour = 0, minute = 0;
    drv_rtc_time_fmt_t fmt = {0};
    drv_rtc_time_fmt_t cur_fmt = {0};

    if (NULL == time) {
        printf("time is NULL");
        goto exit;
    }

    memset(&wkalrm, 0, sizeof(struct rtc_wkalrm));
    memset(&rtc_tm, 0, sizeof(struct rtc_time));

    sscanf(time, "%d-%d-%d %d:%d", &year, &month, &mday, &hour, &minute);

	fmt.year = year;
    fmt.month = month;
    fmt.day = mday;
    fmt.hour = hour;
    fmt.minute = minute;

    if (dev_rtc_set_alert_time(&fmt) != 0) {//设置当前时间
        printf("sunxi rtc set time error\n");
        goto exit; 
    }

    ret = 0;

exit:
    return ret;
}

void dev_pm_iface_set_delay_wakeup_time(int time)
{
    s_delay_wakeup_time = time;
    pm_risv_call_m33_set_time_to_wakeup(s_delay_wakeup_time);
}

int dev_pm_iface_clear_delay_wakeup_time(void)  
{
    s_delay_wakeup_time = 0;
    pm_risv_call_m33_set_time_to_wakeup(s_delay_wakeup_time);	
    return 0;
}

int dev_pm_iface_clear_wakeup_alarm(void)  
{
    hal_rtc_disable_wake();	//关闭rtc的唤醒源
    //hal_rtc_alarm_irq_enable(0);//失能rtc中断，可以有其他接口	
    return 0;
}

/************************************    指定ip唤醒    *******************************************/
/**
 * @brief 解析IP地址字符串并设置P2P唤醒IP
 * @param session: 会话ID
 * @param enable: 使能标志
 * @param ip_str: IP地址字符串，格式为 "xxx.xxx.xxx.xxx"
 * @return 0成功，非0失败
 */
int dev_pm_iface_set_p2p_wakeup_ip_str(uint8_t session, uint8_t enable, const char *ip_str)
{
    if (!ip_str) {
        printf("Invalid IP string\n");
        return -1;
    }

    // 存储解析后的IP地址
    uint8_t ip[4] = {0};
    int count = 0;
    const char *start = ip_str;
    char *end;

    // 解析IP地址字符串
    while (count < 4 && *start) {
        // 将字符串转换为长整型
        long val = strtol(start, &end, 10);
        
        if (start == end || val < 0 || val > 255) {
            printf("Invalid IP format\n");
            return -1;
        }

        ip[count++] = (uint8_t)val;

        if (count < 4) {
            if (*end != '.') {
                printf("Invalid IP delimiter\n");
                return -1;
            }
            start = end + 1;
        }
    }

    if (count != 4) {
        printf("Incomplete IP address\n");
        return -1;
    }

    printf("Setting wakeup IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    
    // 调用原始接口设置IP
    return net_wlan_set_p2p_wakeupip(session, enable, ip[0], ip[1], ip[2], ip[3]);
}

int dev_pm_iface_net_wlan_set_p2p_hostsleep(int enable)
{
    char cmd[32];
    int ret = -1;
    snprintf(cmd, sizeof(cmd), "p2p_hostsleep %d", enable);
    ret = cmd_wlan_exec(cmd);
    if (ret != CMD_STATUS_OK) {
        printf("p2p_hostsleep failed, enable = %d\n", enable);
        ret = -1;
        goto exit;
    }
    ret = 0;
exit:
    return ret;
}

int dev_pm_iface_set_wlan_filter_type(uint32_t type)
{
    char cmd[32];
    int ret = -1;
    snprintf(cmd, sizeof(cmd), "set_filter_type ft=0x%x", type);
    ret = cmd_wlan_exec(cmd);
    if (ret != CMD_STATUS_OK) {
        printf("set_filter_type failed, type = 0x%x\n", type);
        ret = -1;
        goto exit;
    }
    ret = 0;
exit:
    return ret;
}

int dev_pm_iface_set_lp_param(uint32_t param)
{
    char cmd[32];
    int ret = -1;
    snprintf(cmd, sizeof(cmd), "set_lp_param p=%d", param);
    ret = cmd_wlan_exec(cmd);
    if (ret != CMD_STATUS_OK) {
        printf("set_lp_param failed, p = %d\n", param);
        ret = -1;
        goto exit;
    }
    ret = 0;
exit:
    return ret;
}






