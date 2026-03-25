#ifndef _BARRERY_WRAPPER_H_
#define _BARRERY_WRAPPER_H_

#include "../../gulitesf_config.h"

typedef enum {
    REALIZE_UNIT_BATTERY_OPER_CALI_VALUE_READ,
    REALIZE_UNIT_BATTERY_OPER_CALI_VALUE_WRITE
} battery_cali_val_oper_t;

typedef struct {
	unsigned int full;
	unsigned int low;
} battery_range_t;

// 使用查表法时需要定义以下结构体
typedef struct {
	unsigned int voltage;
	unsigned int percent;
} voltage_ele_t;

typedef struct {
    voltage_ele_t *discharge_vol_table;
    voltage_ele_t *charge_vol_table;
    unsigned int table_size;
} battery_curve_t;
// 使用查表法时需要定义以上结构体

typedef void (*event_callback)(void *arg);

//进行驱动设置初始化
typedef int (*batt_init_t)(event_callback cb, char *info);
//进行驱动设置销毁
typedef int (*batt_destroy_t)(void);
//进行驱动设置,开始接收触发事件
typedef int (*batt_start_event_t)(unsigned int low_value, unsigned int high_value, event_callback cb);
//进行驱动设置,停止接收触发事件
typedef int (*batt_stop_event_t)(void);
//进行驱动设置获取adc数据
typedef int (*batt_get_val_t)(void);
//进行驱动设置获取充电状态
typedef int (*batt_get_charg_state_t)(unsigned char *state);
//进行驱动设置获取充满状态
typedef int (*batt_get_full_state_t)(unsigned char *state);
typedef battery_range_t *(*batt_get_range_t)(void);
typedef int(*batt_get_capacity_t)(void);
// 使用查表法时，需要实现以下函数
typedef int (*batt_get_voltage_t)(unsigned int);
typedef battery_curve_t *(*batt_get_batt_curve)(void);
typedef int (*batt_oper_cali_val_t)(int, battery_cali_val_oper_t);

typedef struct {
	batt_init_t init;
	batt_destroy_t destroy;
	batt_start_event_t start_event;
	batt_stop_event_t stop_event;
	batt_get_val_t get_val;
	batt_get_charg_state_t get_charg_state;
	batt_get_full_state_t get_full_state;
	batt_get_range_t get_range;
	batt_get_capacity_t get_capacity;
    batt_get_voltage_t get_voltage;
    batt_get_batt_curve get_battery_curve;
    batt_oper_cali_val_t oper_cali_val;
} battery_wrapper_t;


#endif
