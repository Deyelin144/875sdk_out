#ifndef __DEV_KEY_H__
#define __DEV_KEY_H__
#include <stdint.h>
#include "drv_key.h"

typedef enum {
    DEV_KEY_TYPE_ADC = 0,
    DEV_KEY_TYPE_MATRIX,
    DEV_KEY_TYPE_MAX,
} dev_key_type_t;


typedef struct {
    dev_key_type_t type;
} dev_key_config_info_t;

// typedef struct {
//     uint32_t channel;     // adc 通道
//     uint32_t irq_mode;    // adc 中断模式
//     uint32_t low_value;   // 低于该值触发中断
//     uint32_t high_value;  // 高于该值触发中断
//     void (*irq_callback)(void *args, uint16_t status, uint32_t adc_value);  // adc 中断回调函数
// } drv_adc_key_config_t;


/**
 * @brief 按键设备初始化
 * @param info 配置信息
 * @return DRV_OK:成功, 否则失败 
 */
int dev_key_init(dev_key_config_info_t * info);

/**
 * @brief 按键设备反初始化
 */
void dev_key_deinit(void);

/**
 * @brief 按键设备休眠
 */
void dev_key_suspend(void);

/**
 * @brief 按键设备唤醒
 */
void dev_key_resume(void);



#endif  /* __DEV_KEY_H__ */
