#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#include "../drv_common.h"
#include "drv_rtc_iface.h"
#include <hal_time.h>
#include <time.h>
#include <sys/time.h>

#define    CMD_YEAR0                 1900 /* the first year for struct tm::tm_year */

typedef int (*rtc_irq_callback)(void);

typedef enum {
    DRV_RTC_TYPE_AT8563 = 0,
    DRV_RTC_TYPE_MAX,
} drv_rtc_type_t;

typedef struct {
    uint8_t second; // 秒, 注意:范围为 0-59
    uint8_t minute; // 分, 注意:范围为 0-59
    uint8_t hour;   // 时, 注意:范围为 0-23
    uint8_t day;    // 日
    uint8_t week;   // 星期, 注意:周日/一/二/三/四/五/六 分别为 0/1/2/3/4/5/6
    uint8_t month;  // 月
    uint16_t year;   // 年, 注意:范围为 0-9999
} drv_rtc_time_fmt_t;

typedef struct
{
    void (*deinit)(void);
    drv_status_t (*init)(drv_rtc_time_fmt_t *fmt, rtc_irq_callback cb);
    drv_status_t (*get_time)(drv_rtc_time_fmt_t *fmt);
    drv_status_t (*set_time)(drv_rtc_time_fmt_t *fmt);
    drv_status_t (*set_alert_time)(drv_rtc_time_fmt_t *fmt);
    drv_status_t (*get_alert_time)(drv_rtc_time_fmt_t *fmt);
    drv_status_t (*clear_alert_time)(void);
    uint8_t (*get_alert_flag)(void);
    void (*clear_alert_flag)(void);
    drv_status_t (*suspend)(void);
    drv_status_t (*resume)(void);
} drv_rtc_ops_t;


typedef void (*drv_rtc_ops_handler_t)(drv_rtc_ops_t* ops);

void drv_rtc_at8563_get_ops(drv_rtc_ops_t* ops);
drv_status_t drv_rtc_at8563_chip_check(void);
void drv_rtc_inner_get_ops(drv_rtc_ops_t* ops);

#endif /* __DRV_RTC_H__ */