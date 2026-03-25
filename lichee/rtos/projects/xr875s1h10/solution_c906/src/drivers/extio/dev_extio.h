#ifndef __DRV_EXTIO_H__
#define __DRV_EXTIO_H__

#include <stdbool.h>

typedef enum {
    DEV_EXTIO_PIN_RADAR1              = 0,    //雷达检测1
    DEV_EXTIO_PIN_RADAR2              = 1,    //雷达检测2
    DEV_EXTIO_PIN_SD_POWER            = 2,    //T卡供电
    DEV_EXTIO_PIN_SD_DET              = 3,    //T卡检测
    DEV_EXTIO_PIN_RTC_INT             = 4,    //时钟中断
    DEV_EXTIO_PIN_FULL_DET            = 5,    //充满检测
    DEV_EXTIO_PIN_CHARGE_DET          = 6,    //充电检测
    DEV_EXTIO_PIN_LCD_BL_LP           = 7,    //显示低功耗
    DEV_EXTIO_PIN_MOTOR1              = 8,    //马达1
    DEV_EXTIO_PIN_MOTOR2              = 9,    //马达2
    DEV_EXTIO_PIN_MOTOR3              = 10,   //马达3
    DEV_EXTIO_PIN_MOTOR4              = 11,   //马达4
    DEV_EXTIO_PIN_EXTVCC_EN           = 12,   //外部供电使能
    DEV_EXTIO_PIN_TP_RESET            = 13,   //TP复位
    DEV_EXTIO_PIN_LCD_RESET           = 14,   //显示复位
    DEV_EXTIO_PIN_CAM_PWDN            = 15,   //摄像头供电
    DEV_EXTIO_PIN_NUM,
} dev_exito_pin_t;

typedef enum {
    DRV_EXTIO_TYPE_AW9523B = 0,
    DRV_EXTIO_TYPE_HTR3316 = 1,
} dev_exito_type_t;

typedef enum {
    DEV_EXTIO_GPIO_DIR_OUTPUT = 0,
    DEV_EXTIO_GPIO_DIR_INPUT  = 1,
} dev_extio_gpio_direction_t;

typedef enum {
    DEV_EXTIO_GPIO_LEVEL_LOW  = 0,
    DEV_EXTIO_GPIO_LEVEL_HIGH = 1,
} dev_extio_gpio_level_t;

typedef void (*dev_extio_notify_t)(void *arg);

typedef struct {
    char *name;
    void (*deinit)(void);
    int (*init)(dev_extio_notify_t cb);
    int (*suspend)(void);
    int (*resume)(void);

    // 在 GPIO 模式下工作的扩展 IO
    int (*gpio_init)(dev_exito_pin_t pin, dev_extio_gpio_direction_t dir, bool interrupt_flag);
    int (*gpio_set)(dev_exito_pin_t pin, dev_extio_gpio_level_t level);
    dev_extio_gpio_level_t (*gpio_get)(dev_exito_pin_t pin);

    int (*irq_flag_get)(uint16_t *irq_flag);
} drv_extio_ops_t;

/**
 * @brief  扩展 IO 初始化
 * @return 0:成功, -1:失败
 */
int dev_extio_init(void);

/**
 * @brief  扩展 IO 注销
 */
void dev_extio_deinit(void);

/**
 * @brief  LCD RESET
 * @return 0:成功, -1:失败
 */
int dev_extio_lcd_reset(void);

/**
 * @brief  TP RESET
 * @return 0:成功, -1:失败
 */
int dev_extio_tp_reset(void);

/**
 * @brief  获取满电检测状态
 * @return 0:未满电, 1:满电 
 */
uint8_t dev_extio_full_det_get(void);

/**
 * @brief 获取扩展 IO 引脚的状态 
 */
int dev_extio_pin_sate_get(dev_exito_pin_t pin);

/**
 * @brief 获取扩展 IO 引脚的状态 
 * @param pin 引脚
 * @param level 状态
 * @return 0：成功，否则失败
 */
int dev_extio_set_pin_state(dev_exito_pin_t pin, dev_extio_gpio_level_t level);

/**
 * @brief 获取咪头的状态 
 * @return 1：使能，0：失能
 */
int dev_extio_get_mic_state(void);

/**
 * @brief 获取充电状态 
 * @return 1：充电，0：未充电
 */
int dev_extio_get_charge_state(void);

/**
 * @brief 获取sd卡状态 
 * @return 1：插入，0：空闲
 */
int dev_extio_get_sd_state(void);

int dev_extio_lcd_bl_lp_enable(void);//拉高屏幕休眠背光引脚
int dev_extio_lcd_bl_lp_disable(void);//拉低屏幕休眠背光引脚

int dev_extio_led_en_enable(void);//开启led灯使能电源
int dev_extio_led_en_disable(void);//关闭led灯使能电源

int dev_extio_radar_enable(void);//开启雷达使能电源
int dev_extio_radar_disable(void);//关闭雷达使能电源

int dev_extvcc_enable(void);//开启外设供电使能
int dev_extvcc_disable(void);//关闭外设供电使能

int dev_extio_lcd_en_enable(void);//开启显示供电使能
int dev_extio_lcd_en_disable(void);//关闭显示供电使能

int dev_extio_pa_shdn_enable(void);//功放使能开启
int dev_extio_pa_shdn_disable(void);//功放使能关闭

int dev_extio_suspend(void);//拓展io的中断引脚deinit
int dev_extio_resume(void);//初始化拓展io的中断引脚

int dev_extio_lcd_rst_enable(void);//开启lcd复位引脚使能
int dev_extio_lcd_rst_disable(void);//关闭lcd复位引脚使能

int dev_extio_tp_rst_enable(void);//开启tp复位引脚使能
int dev_extio_tp_rst_disable(void);//关闭tp复位引脚使能

void dev_extio_clear_irq(void);//清除扩展io各个引脚的中断标志位

void dev_extio_init_radar1_pin(dev_extio_gpio_direction_t dir, bool interrupt_flag, dev_extio_gpio_level_t level);//设置雷达radar1引脚方向
int dev_extio_get_irq(void);//获取扩展io被触发的中断号

int dev_extio_cam_pwdn_ctl(int cmd);

#endif /* __DRV_EXTIO_H__ */