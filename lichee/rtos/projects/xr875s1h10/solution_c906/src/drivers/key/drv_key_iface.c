#include "../drv_log.h"
#include "drv_key_iface.h"
#include <hal_gpio.h>
#include <sunxi_hal_gpadc.h>
#include <gpio/gpio.h>
#include "kernel/os/os.h"

#define DRV_VERB 0
#define DRV_INFO 1
#define DRV_DBUG 1
#define DRV_WARN 1
#define DRV_EROR 1

#if DRV_VERB
#define drv_key_iface_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define drv_key_iface_verb(fmt, ...)
#endif

#if DRV_INFO
#define drv_key_iface_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_key_iface_info(fmt, ...)
#endif

#if DRV_DBUG
#define drv_key_iface_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_key_iface_debug(fmt, ...)
#endif

#if DRV_WARN
#define drv_key_iface_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_key_iface_warn(fmt, ...)
#endif

#if DRV_EROR
#define drv_key_iface_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_key_iface_error(fmt, ...)
#endif

static uint32_t s_gpiokey_idx = 0;

int drv_key_iface_gpio_init(hal_irq_handler_t cb)
{
    int ret = 0;
    hal_gpio_pinmux_set_function(KEY_OK, GPIO_MUXSEL_EINT);
    hal_gpio_set_pull(KEY_OK, GPIO_PULL_UP);
    hal_gpio_set_direction(KEY_OK, GPIO_DIRECTION_INPUT);
    hal_gpio_to_irq(KEY_OK, &s_gpiokey_idx);

    ret = hal_gpio_irq_request(s_gpiokey_idx, cb, IRQ_TYPE_EDGE_BOTH, NULL);
    if (ret < 0) {
        drv_key_iface_error("irq request fail.");
        goto exit;
    }

    ret = hal_gpio_irq_enable(s_gpiokey_idx);
    if (ret < 0) {
        drv_key_iface_error("irq enable fail.");
        goto exit;
    }

exit:
    return ret == 0? s_gpiokey_idx : -1;
}

void drv_key_iface_gpio_deinit(void)
{
    hal_gpio_irq_disable(s_gpiokey_idx);
    hal_gpio_irq_free(s_gpiokey_idx);
}

int drv_key_iface_adc_init(gpadc_callback_t cb)
{
    hal_gpadc_status_t status = GPADC_OK;
    status = hal_gpadc_init();
    if (GPADC_OK != status) {
        drv_key_iface_error("adc init fail: %d", status);
        return status;
    }
    status = hal_gpadc_channel_init(ADC_KEY_CHANNEL);
    if (GPADC_OK != status) {
        drv_key_iface_error("adc channel %d init fail: %d.", ADC_KEY_CHANNEL, status);
    }
    return status;
}

void drv_key_iface_adc_deinit(void)
{
    hal_gpadc_channel_exit(ADC_KEY_CHANNEL);
}

uint8_t drv_key_iface_gpio_read(gpio_pin_t pin)
{
    gpio_data_t status = 0;
    hal_gpio_get_data(pin, &status);
    return status;
}

void drv_key_iface_gpio_write(gpio_pin_t pin, uint8_t val)
{
    hal_gpio_set_data(pin, val);
}

uint32_t drv_key_iface_get_time_ms(void)
{
    return XR_OS_TicksToMSecs(XR_OS_GetTicks());
}

void drv_key_iface_power_off(void)
{
    //drv_power_iface_powerlatch_gpio_set(0);
    // pm_enter_mode(PM_MODE_HIBERNATION);
}
