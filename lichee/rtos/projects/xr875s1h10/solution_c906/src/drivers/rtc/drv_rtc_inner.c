#include <sunxi_hal_rtc.h>
#include <hal_time.h>
#include "drv_rtc.h"

#define VERB 0
#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if VERB
#define drv_rtc_inner_verb(fmt, ...)  drv_logv(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_inner_verb(fmt, ...)
#endif

#if DBUG
#define drv_rtc_inner_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_inner_debug(fmt, ...)
#endif

#if INFO
#define drv_rtc_inner_info(fmt, ...)  drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_inner_info(fmt, ...)
#endif

#if WARN
#define drv_rtc_inner_warn(fmt, ...)  drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_inner_warn(fmt, ...)
#endif

#if EROR
#define drv_rtc_inner_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_inner_error(fmt, ...)
#endif

#define    CMD_IS_LEAP_YEAR(year)    (!((year) % 4) && (((year) % 100) || !((year) % 400)))
static const int8_t g_cmd_mday[2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, /* normal year */
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }    /* leap year */
};

static rtc_irq_callback s_rtc_irq_callback;

/**
 * @brief  初始化内部rtc
 * @param  fmt 所要设置的时间, 可为 NULL
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_inner_init(drv_rtc_time_fmt_t *fmt, rtc_irq_callback cb)
{
    int ret = DRV_ERROR;
    if (0 != hal_rtc_init()) {
        drv_rtc_inner_error("init hal_rtc err.");
        goto exit;
	}

    if (0 != hal_rtc_register_callback(cb)) {
        drv_rtc_inner_error("hal_rtc_register_callback err.");
        goto exit;
	}

    s_rtc_irq_callback = cb;

	ret = DRV_OK;

exit:
	if (ret != DRV_OK) {
		hal_rtc_deinit();
	}

	return ret;
}

static void drv_rtc_inner_deinit(void)
{
    s_rtc_irq_callback = NULL;

    return;
}

static drv_status_t drv_rtc_inner_suspend(void)
{
    return DRV_OK;
}

static drv_status_t drv_rtc_inner_resume(void)
{
    return DRV_OK;
}

/**
 * @brief  获取rtc时间
 * @param  fmt 所要获取的时间 
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_inner_get_time(drv_rtc_time_fmt_t *fmt)
{
    struct rtc_time rtc_tm = {0};

	if (0 != hal_rtc_gettime(&rtc_tm)) {
        drv_rtc_inner_error("hal_rtc_gettime fail.");
		return -1;
	}

    drv_rtc_inner_info("rtc time %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1, rtc_tm.tm_mday, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    fmt->second = rtc_tm.tm_sec;
    fmt->minute = rtc_tm.tm_min;
    fmt->hour = rtc_tm.tm_hour;
    fmt->day = rtc_tm.tm_mday;
    fmt->week = rtc_tm.tm_wday;
    fmt->month = rtc_tm.tm_mon + 1;
    fmt->year = rtc_tm.tm_year + CMD_YEAR0;

	return 0;
}

/**
 * @brief  设置rtc时间
 * @param  fmt 所要设置的时间 
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_inner_set_time(drv_rtc_time_fmt_t *fmt)
{
    struct rtc_time rtc_tm = {0};
    struct tm t = {0};
    time_t clock = {0};
    struct timeval tv = {0};
    int ret = -1;

    int leap = CMD_IS_LEAP_YEAR(fmt->year);
    if (fmt->month < 1 || fmt->month > 12) {
		drv_rtc_inner_error("invalid month %u\n", fmt->month);
		goto exit;
	}
    fmt->month -= 1;
    if (fmt->hour > 23) {
		fmt->hour -= 24;
		fmt->day += 1;
		if (fmt->day > g_cmd_mday[leap][fmt->month]) {
			fmt->day = 1;
			fmt->month += 1;
			if (fmt->month > 11) {
				fmt->year += 1;
				fmt->month = 0;
			}
		}
	} else if (fmt->hour < 0) {
		fmt->hour += 24;
		fmt->day -= 1;
		if (fmt->day < 1) {
			fmt->month -= 1;
			if (fmt->month < 0) {
				fmt->month = 11;
				fmt->year -= 1;
			}
			leap = CMD_IS_LEAP_YEAR(fmt->year);
			fmt->day = g_cmd_mday[leap][fmt->month];
		}
	}

    memset(&t, 0, sizeof(t));
	t.tm_year = rtc_tm.tm_year = fmt->year - CMD_YEAR0;
	t.tm_mon = rtc_tm.tm_mon = fmt->month;
	t.tm_mday = rtc_tm.tm_mday = fmt->day;
	t.tm_hour = rtc_tm.tm_hour = fmt->hour;
	t.tm_min = rtc_tm.tm_min = fmt->minute;
	t.tm_sec = rtc_tm.tm_sec = fmt->second;

	ret = hal_rtc_settime(&rtc_tm);
    
    clock = mktime(&t);
    localtime_r(&clock, &t);
    tv.tv_sec = clock;
	tv.tv_usec = 0;
    ret = settimeofday(&tv, NULL);

    drv_rtc_inner_info("set time %04d-%02d-%02d %02d:%02d:%02d\n",
        t.tm_year + CMD_YEAR0, t.tm_mon + 1, t.tm_mday,
        t.tm_hour, t.tm_min, t.tm_sec);
exit:
    return ret;
}

/**
 * @brief  获取所设置的报警时间
 * @param  fmt 所要获取的时间
 * @return DRV_OK:成功, 否则失败
 */
drv_status_t drv_rtc_inner_get_alert_time(drv_rtc_time_fmt_t *fmt)
{
    struct rtc_wkalrm wkalrm = {0};

	if (0 != hal_rtc_getalarm(&wkalrm)) {
        drv_rtc_inner_error("hal_rtc_getalarm fail.");
		return -1;
	}

    drv_rtc_inner_info("alarm time %04d-%02d-%02d %02d:%02d:%02d\n",
           wkalrm.time.tm_year + CMD_YEAR0, wkalrm.time.tm_mon + 1, wkalrm.time.tm_mday,
           wkalrm.time.tm_hour, wkalrm.time.tm_min, wkalrm.time.tm_sec);

    fmt->second = wkalrm.time.tm_sec;
    fmt->minute = wkalrm.time.tm_min;
    fmt->hour = wkalrm.time.tm_hour;
    fmt->day = wkalrm.time.tm_mday;
    fmt->month = wkalrm.time.tm_mon + 1;
    fmt->year = wkalrm.time.tm_year + CMD_YEAR0;

	return 0;
}

/**
 * @brief  设置报警时间
 * @param  fmt 所要设置的时间
 * @return DRV_OK:成功, 否则失败
 */
static drv_status_t drv_rtc_inner_set_alert_time(drv_rtc_time_fmt_t *fmt)
{
    struct rtc_time rtc_tm = {0};
    drv_rtc_time_fmt_t now_fmt;
    struct tm t = {0};
    time_t clock = {0};
    struct timeval tv = {0};
    int ret = -1;

	rtc_tm.tm_year = fmt->year - CMD_YEAR0;
	rtc_tm.tm_mon = fmt->month - 1;
	rtc_tm.tm_mday = fmt->day;
	rtc_tm.tm_hour = fmt->hour;
	rtc_tm.tm_min = fmt->minute;
    rtc_tm.tm_sec = fmt->second;

    struct rtc_wkalrm wkalrm = {0};
    wkalrm.enabled = 1;
    wkalrm.time = rtc_tm;

    printf("alarm time %04d-%02d-%02d %02d:%02d:%02d\n",
           wkalrm.time.tm_year + CMD_YEAR0, wkalrm.time.tm_mon + 1, wkalrm.time.tm_mday,
           wkalrm.time.tm_hour, wkalrm.time.tm_min, wkalrm.time.tm_sec);

    if (0 != hal_rtc_setalarm(&wkalrm)) {
        drv_rtc_inner_error("hal_rtc_setalarm err.");
        return -1;
	}

    if (0 != hal_rtc_register_callback(s_rtc_irq_callback)) {
        drv_rtc_inner_error("hal_rtc_register_callback err.");
        return -1;
	}

    if (0 != hal_rtc_alarm_irq_enable(1)) {
        drv_rtc_inner_error("hal_rtc_alarm_irq_enable err.");
        return -1;
	}

    drv_rtc_inner_get_time(&now_fmt);
    memset(&t, 0, sizeof(t));
	t.tm_year    =  now_fmt.year - CMD_YEAR0;
    t.tm_mon     =  now_fmt.month - 1;
	t.tm_mday    =  now_fmt.day;
	t.tm_hour    =  now_fmt.hour;
	t.tm_min     =  now_fmt.minute;
	t.tm_sec     =  now_fmt.second;

    clock = mktime(&t);
    localtime_r(&clock, &t);
    tv.tv_sec = clock;
	tv.tv_usec = 0;
    ret = settimeofday(&tv, NULL);

    printf("set time %04d-%02d-%02d %02d:%02d:%02d\n",
        rtc_tm.tm_year + CMD_YEAR0, rtc_tm.tm_mon + 1, rtc_tm.tm_mday,
        rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	return 0;
}

/**
 * @brief 获取中断标志
 * @return DRV_OK:成功, 否则失败 
 */
static uint8_t drv_rtc_inner_get_af()
{
    return 0;
}

/**
 * @brief 清除中断标志
 * @return DRV_OK:成功, 否则失败 
 */
void drv_rtc_inner_clear_af(void)
{

}

/**
 * @brief 设置闹钟暂停
 * @return DRV_OK:成功, 否则失败 
 */
static drv_status_t drv_rtc_inner_clear_alert_time(void)
{
    return hal_rtc_alarm_irq_enable(0);
}

int dev_rtc_set_alarm_callback(void)
{
    if (0 != hal_rtc_register_callback(s_rtc_irq_callback)) {
        drv_rtc_inner_error("hal_rtc_register_callback err.");
        return -1;
	}
    return 0;
}

void drv_rtc_inner_get_ops(drv_rtc_ops_t* ops)
{
    ops->init           = drv_rtc_inner_init;
    ops->deinit         = drv_rtc_inner_deinit;
    ops->suspend        = drv_rtc_inner_suspend;
    ops->resume         = drv_rtc_inner_resume;
    ops->get_time       = drv_rtc_inner_get_time;
    ops->set_time       = drv_rtc_inner_set_time;
    ops->get_alert_time = drv_rtc_inner_get_alert_time;
    ops->set_alert_time = drv_rtc_inner_set_alert_time;
    ops->get_alert_flag = drv_rtc_inner_get_af;
    ops->clear_alert_flag = drv_rtc_inner_clear_af;
	ops->clear_alert_time = drv_rtc_inner_clear_alert_time;
}