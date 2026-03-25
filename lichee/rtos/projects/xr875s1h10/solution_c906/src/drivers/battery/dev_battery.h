#ifndef _DEV_BATTERY_H_
#define _DEV_BATTERY_H_
#include "../drv_common.h"

void dev_battery_init(irq_callback cb);
int dev_battery_get_voltage(void);
int dev_battery_get_charge_state(void);
void dev_battery_charge_detect_cb(void);
void dev_battery_irq_handler(void *args);
void dev_battery_bat_info_set(char isCharge, char isChargeFull);
#endif