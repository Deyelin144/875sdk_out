#ifndef __M33_DRV_EXTIO_H__
#define __M33_DRV_EXTIO_H__

#ifdef  CONFIG_EXTERNAL_DCDC_EN_IO_PA_X
#ifdef CONFIG_DSP_USE_EXTIO_DCDC_EN_IO
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

int dev_extvcc_enable(void);//开启外设供电使能
int dev_extvcc_disable(void);//关闭外设供电使能

#endif /* __DRV_EXTIO_H__ */
#endif
#endif
