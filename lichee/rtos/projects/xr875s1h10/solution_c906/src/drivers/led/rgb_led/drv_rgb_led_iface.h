#ifndef __DRV_RGB_LED_IFACE_H__
#define __DRV_RGB_LED_IFACE_H__

#include "../../drv_common.h"
#define DRV_RGB_LED_SHOW_FOREVER	-1
#define DRV_RGB_LED_SHOW_MODE1      -2 // 用于多彩灯，一轮多彩灯后灯光常亮

#ifdef CFG_LED_RGB_LED_NUM
#define RGB_LED_NUM     		CFG_LED_RGB_LED_NUM
#else
#define RGB_LED_NUM     		(4) 
#endif

typedef enum {
	RGB_LED_MODE_NONE, 					// 处于关灯状态
	RGB_LED_MODE_SINGLE,
	RGB_LED_MODE_BREATH,
	RGB_LED_MODE_FLOWING,
	RGB_LED_MODE_COLORFUL,
} drv_rgb_led_mode_t;

#if (CFG_LED_EN && CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)

// 初始化
int drv_rgb_led_iface_init(void);

// 反初始化
int drv_rgb_led_iface_deinit(void);

// 设置单色灯,show_time==DRV_RGB_LED_SHOW_FOREVER表示长亮
void drv_rgb_led_iface_set_single(uint8_t data0, uint8_t data1, uint8_t data2, int show_time);

// 设置呼吸灯,show_time==DRV_RGB_LED_SHOW_FOREVER表示长亮, cycle_time：一次呼吸时长
void drv_rgb_led_iface_set_breath(uint8_t data[][3], int data_len, int show_time, int cycle_time);

// 设置流水灯,show_time==DRV_RGB_LED_SHOW_FOREVER表示长亮, cycle_time：一次呼吸时长， flowing_regress_flag：反转标记
void drv_rgb_led_iface_set_flowing(uint8_t data[][3], int data_len, int show_time, int cycle_time, int flowing_regress_flag);

// 设置七彩灯,show_time==DRV_RGB_LED_SHOW_FOREVER表示长亮, cycle_time：一次呼吸时长
void drv_rgb_led_iface_set_colorful(uint8_t *data, int data_len, int show_time, int cycle_time);

// 获取指示灯状态
drv_rgb_led_mode_t drv_rgb_led_iface_get_mode(void);

// 休眠关灯
void drv_rgb_led_iface_suspend(void);

// 休眠恢复
void drv_rgb_led_iface_resume(void);

//设置rgb灯颜色
void drv_rgb_led_iface_set_color(uint8_t r, uint8_t g, uint8_t b);

#endif

#endif

