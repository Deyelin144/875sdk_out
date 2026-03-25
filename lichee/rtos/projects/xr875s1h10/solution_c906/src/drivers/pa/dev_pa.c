#include <string.h>
#include <hal_gpio.h>
#include <hal_time.h>
#include "compiler.h"
#include "../drv_log.h"
#include "dev_pa.h"
#include "../atcmd/mcu_at_device.h"

// #define DEV_EARPHONE_PA_PIN     GPIOB(3)

/**
 * @brief  功放控制引脚初始化 
 * @return 0: 成功 其他值: 失败
 */
int dev_pa_pin_init(void)
{
    int ret = 0;

    // ret = hal_gpio_pinmux_set_function(DEV_EARPHONE_PA_PIN, GPIO_MUXSEL_OUT);
    // if (0 != ret) {
    //     drv_loge("pa set function fail.");
    // }

    // ret = hal_gpio_set_driving_level(DEV_EARPHONE_PA_PIN, GPIO_DRIVING_LEVEL1);
    // if (0 != ret) {
    //     drv_loge("pa set driving level fail.");
    // }

    // ret = hal_gpio_set_pull(DEV_EARPHONE_PA_PIN, GPIO_PULL_UP);
    // if (0 != ret) {
    //     drv_loge("pa set pull fail.");
    // }

    // ret = hal_gpio_set_data(DEV_EARPHONE_PA_PIN, GPIO_DATA_LOW);
    // if (0 != ret) {
    //     drv_loge("pa set data fail.");
    // }

    return ret;
}

/**
 * @brief  功放控制引脚控制
 * @param  enable 1:开启功放, 0:关闭功放
 */
void dev_pa_pin_control(int enable)
{
    int ret = 0;

    // ret = hal_gpio_set_data(DEV_EARPHONE_PA_PIN, enable? GPIO_DATA_LOW : GPIO_DATA_HIGH);
    ret = dev_mcu_pa_set(enable);

    if (0 != ret) {
        drv_loge("pa set data fail.");
    }
}