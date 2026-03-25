#ifndef _DEV_LED_H_
#define _DEV_LED_H_
#include "./rgb_led/drv_rgb_led_iface.h"

/**
 * @brief LED设备 初始化
 * @return 0:成功; 其它:失败
 */
int dev_led_init(void);

/**
 * @brief LED设备 反初始化
 */
void dev_led_deinit();

/**
 * @brief LED设备挂起
 * @return none
 */
void dev_led_suspend(char mode);

/**
 * @brief LED设备恢复
 * @return none
 */
void dev_led_resume(char mode);

/**
 * @brief 设置led亮度
 * @param val[in]: 亮度值 0~100
 * @return none
 */
void dev_led_set_bright(int val1, int val2);

/**
 * @brief 获取led亮度
 * @return 亮度值 0~100
 */
int dev_led_get_bright(void);

/**
 * @brief LED设置单色灯
 * @return none
 */
void dev_led_set_single(uint8_t data0, uint8_t data1, uint8_t data2, int show_time);

/**
 * @brief LED设置呼吸灯
 * @return none
 */
void dev_led_set_breath(uint8_t data[][3], int data_len, int show_time, int cycle_time);

/**
 * @brief LED设置流水灯
 * @return none
 */
void dev_led_set_flowing(uint8_t data[][3], int data_len, int show_time, int cycle_time, int flowing_regress_flag);

/**
 * @brief LED设置七彩灯
 * @return none
 */
void dev_led_set_colorful(uint8_t *data, int data_len, int show_time, int cycle_time);

#endif
