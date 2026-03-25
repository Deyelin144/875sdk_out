#ifndef __DRV_KEY_IFACE_H__
#define __DRV_KEY_IFACE_H__

#include "drv_key.h"
#include "dev_key.h"
#include "hal_gpio.h"
#include "../../../../../../../rtos-hal/hal/source/gpio/sun8iw20/platform-gpio.h"
#include <sunxi_hal_gpadc.h>

#define KEY_OK  GPIOA(11)
#define ADC_KEY_CHANNEL    GP_CH_1

#define DRV_ADC_READ_CYCLE_COUNT      5
// 添加防抖相关定义
#define KEY_DEBOUNCE_TIME       3    // 防抖次数(与延时时间配合使用)
#define KEY_DEBOUNCE_DELAY      10   // 每次检测延时(ms)
#define DRV_ADC_VAL_RANGE       50   // adc 按键的键值浮动范围
#define DRV_ADC_RELEASE_VALUE   3800 // 释放 adc 按键时读到的最低阈值

typedef enum {
    ADC_KEY_INDEX_UP    = 0x01, // 上
    ADC_KEY_INDEX_DOWN  = 0x02, // 下
    ADC_KEY_INDEX_LEFT  = 0x03, // 左
    ADC_KEY_INDEX_RIGHT = 0x04, // 右
    ADC_KEY_INDEX_AGING = 0x05, // 老化测试 
    ADC_KEY_INDEX_TEST  = 0x06, // 测试模式 
    ADC_KEY_INDEX_BACK  = 0x07, // 返回
    GPIO_KEY_INDEX_POWER = 0x08, // 电源键/OK键
} drv_key_adc_index_t;

typedef enum {
    ADC_KEY_GROUP_0 = 0,
    ADC_KEY_GROUP_NUM,
} drv_key_adc_group_t;

typedef struct {
    uint32_t group; // 所在组别, 参考 drv_key_adc_group_t
    uint32_t index; // 按键索引, 参考 drv_key_adc_index_t
    uint32_t adc;   // 对应的 adc 值
} drv_key_adc_map_t;

int drv_key_iface_gpio_init(hal_irq_handler_t cb);

void drv_key_iface_gpio_deinit(void);

uint8_t drv_key_iface_gpio_read(gpio_pin_t pin);

void drv_key_iface_gpio_write(gpio_pin_t pin, uint8_t val);

uint32_t drv_key_iface_adc_filter(uint32_t ad_value);

int drv_key_iface_adc_init(gpadc_callback_t cb);

void drv_key_iface_adc_deinit(void);

/**
 * @brief  获取当前的系统时间
 * 
 * @return 当前的时间, 单位 ms
 */
uint32_t drv_key_iface_get_time_ms(void);

/**
 * @brief 关机
 */
void drv_key_iface_power_off(void);

#endif  /* __DRV_KEY_IFACE_H__ */
