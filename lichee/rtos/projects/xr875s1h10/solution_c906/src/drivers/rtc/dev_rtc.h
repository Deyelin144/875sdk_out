#ifndef _DEV_RTC_H_
#define _DEV_RTC_H_

#include "drv_rtc.h"

drv_status_t dev_rtc_init(drv_rtc_time_fmt_t *fmt, irq_callback cb);

void dev_rtc_deinit(void);

drv_status_t dev_rtc_irq_cb_register(irq_callback cb);

drv_status_t dev_rtc_set_time(drv_rtc_time_fmt_t *fmt);

drv_status_t dev_rtc_get_time(drv_rtc_time_fmt_t *fmt);

drv_status_t dev_rtc_set_alert_time(drv_rtc_time_fmt_t *fmt);

drv_status_t dev_rtc_get_alert_time(drv_rtc_time_fmt_t *fmt);

drv_status_t dev_rtc_set_alert_stop(void);
drv_status_t dev_rtc_clear_af(void);

drv_rtc_type_t get_rtc_chip_type(void);

drv_status_t dev_cali_rtc_set_time(drv_rtc_time_fmt_t *fmt);

drv_status_t dev_cali_rtc_get_time(drv_rtc_time_fmt_t *fmt);

#endif
