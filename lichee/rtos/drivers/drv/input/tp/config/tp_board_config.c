#include <stdio.h>
#include <hal_gpio.h>
#include <sunxi_hal_twi.h>
#include "tp_board_config.h"

#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif

#define TP_CONFIG_DEBUG_ENABLE 0
#if TP_CONFIG_DEBUG_ENABLE
#define TP_CONFIG_DEBUG(fmt, ...) printf("[tp_config %4d] " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define TP_CONFIG_DEBUG(fmt, ...)
#endif

#ifdef CONFIG_DRIVER_SYSCONFIG
static tp_board_config_t s_board_config;
#endif

// static bool s_stop_i2c = false;

// bool tp_board_i2c_control(bool cmd)
// {
//     s_stop_i2c = cmd;
// }

bool tp_board_config_is_support(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    return (1 == s_board_config.tp_used) ? true : false;
#else
    return false;
#endif
}

char *tp_board_config_drv_name_get(void)
{
    return s_board_config.tp_driver_name;
}

int tp_board_config_parse(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    int ret = -1;
    int32_t gpio_count = 0;
    int32_t value = 0;
    char drv_name[32] = {0};
    user_gpio_set_t gpio_cfg[2] = {0};

    memset(&s_board_config, 0, sizeof(tp_board_config_t));
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_used", (void *)&value, 1);
    s_board_config.tp_used = (uint8_t)value;
    TP_CONFIG_DEBUG("tp_used: %d.", s_board_config.tp_used);

    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_driver_name", (void *)&drv_name, 2);
    strncpy(s_board_config.tp_driver_name, drv_name, 32);
    TP_CONFIG_DEBUG("tp_driver_name: %s.", s_board_config.tp_driver_name);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_twi_id", (void *)&value, 1);
    s_board_config.tp_twi_id = (uint16_t)value;
    TP_CONFIG_DEBUG("tp_twi_id: %d.", s_board_config.tp_twi_id);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_addr", (void *)&value, 1);
    s_board_config.tp_addr = (uint16_t)value;
    TP_CONFIG_DEBUG("tp_addr: 0x%x.", s_board_config.tp_addr);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_revert_mode", (void *)&value, 1);
    s_board_config.tp_revert_mode = (uint8_t)value;
    TP_CONFIG_DEBUG("tp_revert_mode: %d.", s_board_config.tp_revert_mode);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_exchange_flag", (void *)&value, 1);
    s_board_config.tp_exchange_flag = (uint8_t)value;
    TP_CONFIG_DEBUG("tp_exchange_flag: %d.", s_board_config.tp_exchange_flag);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_max_x", (void *)&value, 1);
    s_board_config.tp_max_x = (uint16_t)value;
    TP_CONFIG_DEBUG("tp_max_x: %d.", s_board_config.tp_max_x);

    value = 0;
    ret = hal_cfg_get_keyvalue(TP_NODE_NAME, "tp_max_y", (void *)&value, 1);
    s_board_config.tp_max_y = (uint16_t)value;
    TP_CONFIG_DEBUG("tp_max_y: %d.", s_board_config.tp_max_y);

    gpio_count = hal_cfg_get_gpiosec_keycount(TP_NODE_NAME);
    ret = hal_cfg_get_gpiosec_data(TP_NODE_NAME, gpio_cfg, gpio_count);

    for (int i = 0; i < gpio_count; i++) {
        if (0 == strcmp(gpio_cfg[i].name, "tp_int")) {
            s_board_config.tp_int.gpio = (gpio_cfg[i].port - 1) * 32 + gpio_cfg[i].port_num;
            s_board_config.tp_int.port = gpio_cfg[i].port;
            s_board_config.tp_int.port_num = gpio_cfg[i].port_num;
            s_board_config.tp_int.mul_sel = gpio_cfg[i].mul_sel;
            s_board_config.tp_int.pull = gpio_cfg[i].pull;
            s_board_config.tp_int.drv_level = gpio_cfg[i].drv_level;
            s_board_config.tp_int.data = gpio_cfg[i].data;
        } else {
            s_board_config.tp_int.gpio = TP_INVALID_GPIO;
        }

        if (0 == strcmp(gpio_cfg[i].name, "tp_reset")) {
            s_board_config.tp_reset.gpio = (gpio_cfg[i].port - 1) * 32 + gpio_cfg[i].port_num;
            s_board_config.tp_reset.port = gpio_cfg[i].port;
            s_board_config.tp_reset.port_num = gpio_cfg[i].port_num;
            s_board_config.tp_reset.mul_sel = gpio_cfg[i].mul_sel;
            s_board_config.tp_reset.pull = gpio_cfg[i].pull;
            s_board_config.tp_reset.drv_level = gpio_cfg[i].drv_level;
            s_board_config.tp_reset.data = gpio_cfg[i].data;
        } else {
            s_board_config.tp_reset.gpio = TP_INVALID_GPIO;
        }
    }

    TP_CONFIG_DEBUG("tp_int: port %d, port_num %d.", s_board_config.tp_int.port, s_board_config.tp_int.port_num);
    TP_CONFIG_DEBUG("tp_reset: port %d, port_num %d.", s_board_config.tp_reset.port, s_board_config.tp_reset.port_num);

    ret = 0;
    return ret;
#else
    // TODO
    return -1;
#endif
}

int tp_board_config_hw_init(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    int ret = -1;
    uint32_t tp_irq_num = 0;
    twi_status_t twi_status = TWI_STATUS_OK;

    if (0 == s_board_config.tp_used) {
        ret = -1;
        goto exit;
    }

    twi_status = hal_twi_init(s_board_config.tp_twi_id);
    if (TWI_STATUS_OK != twi_status) {
        ret = -1;
        TP_CONFIG_DEBUG("twi init fail: %d.", twi_status);
        goto exit;
    }

    if (TP_INVALID_GPIO != s_board_config.tp_reset.gpio) {
        if (GPIO_MUXSEL_OUT != s_board_config.tp_reset.mul_sel) {
            s_board_config.tp_reset.mul_sel = GPIO_MUXSEL_OUT;
        }
        ret = hal_gpio_set_direction(s_board_config.tp_reset.gpio, s_board_config.tp_reset.mul_sel);
        ret = hal_gpio_set_driving_level(s_board_config.tp_reset.gpio, s_board_config.tp_reset.drv_level);
        ret = hal_gpio_set_pull(s_board_config.tp_reset.gpio, s_board_config.tp_reset.pull);
        hal_gpio_set_data(s_board_config.tp_reset.gpio, GPIO_DATA_HIGH);
    }

    if (TP_INVALID_GPIO != s_board_config.tp_int.gpio) {
        if (GPIO_MUXSEL_EINT != s_board_config.tp_int.mul_sel) {
            s_board_config.tp_int.mul_sel = GPIO_MUXSEL_EINT;
        }
        ret = hal_gpio_pinmux_set_function(s_board_config.tp_int.gpio, s_board_config.tp_int.mul_sel);
        ret = hal_gpio_set_driving_level(s_board_config.tp_int.gpio, s_board_config.tp_int.drv_level);
        ret = hal_gpio_set_pull(s_board_config.tp_int.gpio, s_board_config.tp_int.pull);
        ret = hal_gpio_to_irq(s_board_config.tp_int.gpio, &tp_irq_num);
        if (ret < 0) {
            ret = -1;
            TP_CONFIG_DEBUG("gpio irq request fail.");
            goto exit;
        }
    } else {
        ret = -1;
        goto exit;
    }

    ret = tp_irq_num;

exit:
    return ret;
#else
    // TODO
    return -1;
#endif
}

int tp_board_config_hw_deinit(uint32_t irq_num)
{
    hal_twi_uninit(s_board_config.tp_twi_id);
    memset(&s_board_config, 0, sizeof(tp_board_config_t));
}

int tp_board_config_i2c_read(uint8_t *buf, uint32_t len)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    int ret = -1;
    struct twi_msg msgs;
    twi_status_t twi_status = TWI_STATUS_OK;

    // if (s_stop_i2c) {
    //     return 0;
    // }

    if (0 == s_board_config.tp_used) {
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        ret = -1;
        goto exit;
    }

    msgs.addr = s_board_config.tp_addr;
    msgs.flags = TWI_M_RD;
    msgs.buf = buf;
    msgs.len = len;

    twi_status = hal_twi_write(s_board_config.tp_twi_id, &msgs, 1);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        ret = -1;
    }

exit:
    return ret;
#else
    // TODO
    return -1;
#endif
}

int tp_board_config_i2c_read_with_addr(uint8_t addr, uint8_t *buf, uint32_t len)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    int ret = -1;
    struct twi_msg msgs[2];
    twi_status_t twi_status = TWI_STATUS_OK;

    if (0 == s_board_config.tp_used) {
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        ret = -1;
        goto exit;
    }

    msgs[0].addr = s_board_config.tp_addr;
    msgs[0].flags = 0;
    msgs[0].buf = &addr;
    msgs[0].len = 1;

    msgs[1].addr = s_board_config.tp_addr;
    msgs[1].flags = TWI_M_RD;
    msgs[1].buf = buf;
    msgs[1].len = len;

    twi_status = hal_twi_write(s_board_config.tp_twi_id, msgs, 2);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        ret = -1;
    }

exit:
    return ret;
#else
    // TODO
    return -1;
#endif
}

int tp_board_config_i2c_write(uint8_t *buf, uint32_t len)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    int ret = -1;
    struct twi_msg msgs;
    twi_status_t twi_status = TWI_STATUS_OK;

    // if (s_stop_i2c) {
    //     return 0;
    // }

    if (0 == s_board_config.tp_used) {
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        ret = -1;
        goto exit;
    }

    msgs.addr = s_board_config.tp_addr;
    msgs.flags = 0;
    msgs.buf = buf;
    msgs.len = len;

    twi_status = hal_twi_write(s_board_config.tp_twi_id, &msgs, 1);
    if (TWI_STATUS_OK == twi_status) {
        ret = 0;
    } else {
        ret = -1;
    }

exit:
    return ret;
#else
    // TODO
    return -1;
#endif
}

uint8_t tp_board_config_tp_revert_mode_get(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    if (0 == s_board_config.tp_used) {
        return 0;
    }

    return s_board_config.tp_revert_mode;
#else
    return 0;
#endif
}

uint8_t tp_board_config_tp_exchange_flag_get(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    if (0 == s_board_config.tp_used) {
        return 0;
    }

    return s_board_config.tp_exchange_flag;
#else
    return 0;
#endif
}

uint16_t tp_board_config_tp_max_x_get(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    if (0 == s_board_config.tp_used) {
        return 0;
    }

    return s_board_config.tp_max_x;
#else
    return 0;
#endif
}

uint16_t tp_board_config_tp_max_y_get(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
    if (0 == s_board_config.tp_used) {
        return 0;
    }

    return s_board_config.tp_max_y;
#else
    return 0;
#endif
}

void tp_board_config_reset_pin_high(void)
{
    if (TP_INVALID_GPIO != s_board_config.tp_reset.gpio) {
        hal_gpio_set_data(s_board_config.tp_reset.gpio, GPIO_DATA_HIGH);
    }
}

void tp_board_config_reset_pin_low(void)
{
    if (TP_INVALID_GPIO != s_board_config.tp_reset.gpio) {
        hal_gpio_set_data(s_board_config.tp_reset.gpio, GPIO_DATA_LOW);
    }
}