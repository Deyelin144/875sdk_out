#include "compiler.h"
#include <sunxi_hal_twi.h>
#include "../drv_log.h"
#include "drv_rtc_iface.h"

#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if DBUG
#define drv_rtc_iface_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_iface_debug(fmt, ...)
#endif

#if INFO
#define drv_rtc_iface_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_iface_info(fmt, ...)
#endif

#if WARN
#define drv_rtc_iface_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_iface_warn(fmt, ...)
#endif

#if EROR
#define drv_rtc_iface_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_rtc_iface_error(fmt, ...)
#endif

int drv_rtc_iface_i2c_init(drv_rtc_param_t *param)
{
    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        return -1;
    }

    twi_status_t twi_status = TWI_STATUS_OK;

    twi_status = hal_twi_init(param->twi_id);

    if (TWI_STATUS_OK != twi_status) {
        drv_rtc_iface_error("i2c init fail: %d.", twi_status);
        return -1;
    }

    return 0;
}

int drv_rtc_iface_i2c_deinit(drv_rtc_param_t *param)
{
    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        return -1;
    }

    twi_status_t twi_status = TWI_STATUS_OK;

    twi_status = hal_twi_uninit(param->twi_id);

    if (TWI_STATUS_OK != twi_status) {
        return -1;
    }

    return 0;
}

int drv_rtc_iface_i2c_send(drv_rtc_param_t *param, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_rtc_iface_error("invalid len.");
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
        drv_rtc_iface_error("i2c send fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

int drv_rtc_iface_i2c_recv(drv_rtc_param_t *param, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_rtc_iface_error("invalid len.");
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
        drv_rtc_iface_error("i2c recv fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

int drv_rtc_iface_i2c_recv_with_addr(drv_rtc_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        drv_rtc_iface_error("invalid len.");
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
        drv_rtc_iface_error("i2c recv fail: %d.", twi_status);
        ret = -1;
    }

exit:
    return ret;
}

int drv_rtc_iface_i2c_send_with_addr(drv_rtc_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len)
{
    int ret = -1;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (NULL == param) {
        drv_rtc_iface_error("invalid rtc param.");
        return -1;
    }
    if (0 == len) {
        drv_rtc_iface_error("invalid len.");
        return -1;
    }

    // 组包：寄存器地址 + 数据
    uint8_t send_buf[len + 1];
    send_buf[0] = reg_addr;
    memcpy(&send_buf[1], buf, len);

    struct twi_msg msgs;
    msgs.addr = param->dev_addr;
    msgs.flags = 0;
    msgs.buf = send_buf;
    msgs.len = len + 1;

    twi_status = hal_twi_write(param->twi_id, &msgs, 1);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        drv_rtc_iface_error("i2c send fail: %d.", twi_status);
        ret = -1;
    }

    return ret;
}