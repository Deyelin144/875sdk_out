#include "compiler.h"
#include <sunxi_hal_twi.h>
#include "../drv_log.h"
#include "drv_extio_iface.h"

#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if DBUG
#define drv_extio_iface_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_extio_iface_debug(fmt, ...)
#endif

#if INFO
#define drv_extio_iface_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_extio_iface_info(fmt, ...)
#endif

#if WARN
#define drv_extio_iface_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_extio_iface_warn(fmt, ...)
#endif

#if EROR
#define drv_extio_iface_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_extio_iface_error(fmt, ...)
#endif

#define CFG_EXTIO_IRQ_PIN GPIOA(11)

static uint32_t s_extio_irq_num = 0;
static drv_extio_iface_irq_cb_t s_extio_irq_cb = NULL;

int drv_extio_iface_i2c_init(drv_extio_param_t *param)
{
    if (NULL == param) {
        drv_extio_iface_error("invalid extio param.");
        return -1;
    }

    twi_status_t twi_status = TWI_STATUS_OK;

    twi_status = hal_twi_init(param->twi_id);

    if (TWI_STATUS_OK != twi_status) {
        drv_extio_iface_error("i2c init fail: %d.", twi_status);
        return -1;
    }

    return 0;
}

int drv_extio_iface_i2c_deinit(drv_extio_param_t *param)
{
    if (NULL == param) {
        drv_extio_iface_error("invalid extio param.");
        return -1;
    }

    twi_status_t twi_status = TWI_STATUS_OK;

    twi_status = hal_twi_uninit(param->twi_id);

    if (TWI_STATUS_OK != twi_status) {
        return -1;
    }

    return 0;
}

int drv_extio_iface_i2c_send(drv_extio_param_t *param, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_extio_iface_error("invalid extio param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_extio_iface_error("invalid len.");
        ret = -1;
        goto exit;
    }

    struct twi_msg msgs;
    msgs.addr = param->dev_addr;
    msgs.flags = 0;
    msgs.buf = buf;
    msgs.len = len;

    twi_status = hal_twi_write(param->twi_id, &msgs, 1);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        drv_extio_iface_error("i2c send fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

int drv_extio_iface_i2c_recv(drv_extio_param_t *param, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_extio_iface_error("invalid extio param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_extio_iface_error("invalid len.");
        ret = -1;
        goto exit;
    }

    struct twi_msg msgs;
    msgs.addr = param->dev_addr;
    msgs.flags = TWI_M_RD;
    msgs.buf = buf;
    msgs.len = len;

    twi_status = hal_twi_write(param->twi_id, &msgs, 1);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        drv_extio_iface_error("i2c recv fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

int drv_extio_iface_i2c_recv_with_addr(drv_extio_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_extio_iface_error("invalid extio param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_extio_iface_error("invalid len.");
        ret = -1;
        goto exit;
    }

    struct twi_msg msgs[2];
    msgs[0].addr = param->dev_addr;
    msgs[0].flags = 0;
    msgs[0].buf = &reg_addr;
    msgs[0].len = 1;

    msgs[1].addr = param->dev_addr;
    msgs[1].flags = TWI_M_RD;
    msgs[1].buf = buf;
    msgs[1].len = len;

    twi_status = hal_twi_write(param->twi_id, msgs, 2);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        drv_extio_iface_error("i2c recv fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

__nonxip_text static hal_irqreturn_t drv_extio_iface_rtc_irq_handler(void *arg)
{
    if (NULL != s_extio_irq_cb) {
        s_extio_irq_cb(NULL);
    }

    return HAL_IRQ_OK;
}

int drv_extio_iface_irq_init(drv_extio_iface_irq_cb_t irq_cb)
{
    int ret = 0;
    uint32_t irq_num = 0;

    ret = hal_gpio_pinmux_set_function(CFG_EXTIO_IRQ_PIN, GPIO_MUXSEL_EINT);
    ret = hal_gpio_set_direction(CFG_EXTIO_IRQ_PIN, GPIO_DIRECTION_INPUT);
    ret = hal_gpio_set_pull(CFG_EXTIO_IRQ_PIN, GPIO_PULL_UP);
    ret = hal_gpio_to_irq(CFG_EXTIO_IRQ_PIN, &irq_num);
    if (ret < 0) {
        drv_extio_iface_error("get irq num fail: %d.", ret);
        return -1;
    }

    ret = hal_gpio_irq_request(irq_num, drv_extio_iface_rtc_irq_handler, IRQ_TYPE_EDGE_FALLING, NULL);
    if (ret < 0) {
        drv_extio_iface_error("request irq num fail: %d.", ret);
        return -1;
    }

    ret = hal_gpio_irq_enable(irq_num);
    if (ret < 0) {
        drv_extio_iface_error("request irq num fail: %d.", ret);
        hal_gpio_irq_free(irq_num);
        return -1;
    }

    s_extio_irq_num = irq_num;
    s_extio_irq_cb = irq_cb; 

    return 0;
}

void drv_extio_iface_irq_deinit(void)
{
    if (0 == s_extio_irq_num) {
        return;
    }

    hal_gpio_irq_disable(s_extio_irq_num);
    hal_gpio_irq_free(s_extio_irq_num);
}
