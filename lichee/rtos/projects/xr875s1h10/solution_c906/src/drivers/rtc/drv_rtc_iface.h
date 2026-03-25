#ifndef __DRV_RTC_IFACE_H__
#define __DRV_RTC_IFACE_H__

#include <stdint.h>

typedef struct {
    uint16_t twi_id;
    uint32_t twi_freq;
    uint16_t dev_addr;
} drv_rtc_param_t;

/**
 * @brief  I2C 初始化
 * @param  param 配置参数
 * @return 0:成功, -1:失败
 */
int drv_rtc_iface_i2c_init(drv_rtc_param_t *param);

/**
 * @brief I2C 注销
 * @param  param 配置参数
 * @return 0:成功, -1:失败
 */
int drv_rtc_iface_i2c_deinit(drv_rtc_param_t *param);

/**
 * @brief  I2C 发送数据
 * @param  param 配置参数
 * @param  buf 发送缓冲区
 * @param  len 发送缓冲区长度
 * @return 0:成功, -1:失败
 * @note   buf[0] 可以写入 reg_addr
 */
int drv_rtc_iface_i2c_send(drv_rtc_param_t *param, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 接收数据
 * @param  param 配置参数
 * @param  buf 接收缓冲区
 * @param  len 接收缓冲区长度
 * @return 0:成功, -1:失败
 * @note   buf[0] 可以写入 reg_addr
 */
int drv_rtc_iface_i2c_recv(drv_rtc_param_t *param, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 接收数据
 * @param  param 配置参数
 * @param  reg_addr 外设的寄存器地址
 * @param  buf 接收缓冲区
 * @param  len 接收缓冲区长度
 * @return 0:成功, -1:失败
 */
int drv_rtc_iface_i2c_recv_with_addr(drv_rtc_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len);

/**
 * @brief  I2C 发送数据
 * @param  param 配置参数
 * @param  reg_addr 外设的寄存器地址
 * @param  buf 发送缓冲区
 * @param  len 发送缓冲区长度
 * @return 0:成功, -1:失败
 */
int drv_rtc_iface_i2c_send_with_addr(drv_rtc_param_t *param, uint8_t reg_addr, uint8_t *buf, uint32_t len);

#endif