#include <stdbool.h>
#include "compiler.h"
#include "kernel/os/os.h"
#include "dev_extio.h"
#include <sunxi_hal_twi.h>

/**
 * AW9523B扩展IO的相关引脚:
 *   P0_0 - P0_7: 可配置为GPIO/LED模式, 对于GPIO模式, 支持push-pull和open-drain
 *   P1_0 - P1_7: 可配置为GPIO/LED模式, 对于GPIO模式, 仅支持push-pull
 * 
 *   P0_0: 输入模式, 用于 KEY_COL0
 *   P0_1: 输入模式, 用于 KEY_COL1
 *   P0_2: 输入模式, 用于 KEY_COL2
 *   P0_3: 输入模式, 用于 RTC_INT
 *   P0_4: 输入模式, 用于 PHONE_DET
 *   P0_5: 输入模式, 用于 FULL_DET
 *   P0_6: 输出模式, 用于 WAKEUP-4G
 *   P0_7: 输出模式, 用于 USB-SWITCH
 * 
 *   P1_0: 输入模式, 用于 KEY_ROW0
 *   P1_1: 输入模式, 用于 KEY_ROW1
 *   P1_2: 输入模式, 用于 KEY_ROW2
 *   P1_3: 
 *   P1_4: 输出模式, 用于 LCD_CS
 *   P1_5: 输出模式, 用于 TP_REST
 *   P1_6: 输出模式, 用于 LCD_REST
 *   P1_7: 输出模式, 用于 CSI-PWDN
 */

#define AW9523B_REG_P0_INPUT     (0x00)
#define AW9523B_REG_P1_INPUT     (0x01)
#define AW9523B_REG_P0_OUTPUT    (0x02)
#define AW9523B_REG_P1_OUTPUT    (0x03)
#define AW9523B_REG_P0_CONFIG    (0x04)
#define AW9523B_REG_P1_CONFIG    (0x05)
#define AW9523B_REG_P0INT_MSK    (0x06)
#define AW9523B_REG_P1INT_MSK    (0x07)
#define AW9523B_REG_CHIP_ID      (0x10)
#define AW9523B_REG_GCR_REG      (0x11)
#define AW9523B_REG_P0WKMD       (0x12)
#define AW9523B_REG_P1WKMD       (0x13)
#define AW9523B_REG_PATEN        (0x14)
#define AW9523B_REG_FDTMR        (0x15)
#define AW9523B_REG_FLTMR        (0x16)
#define AW9523B_REG_P1_0_DIM0    (0x20)
#define AW9523B_REG_P1_1_DIM1    (0x21)
#define AW9523B_REG_P1_2_DIM2    (0x22)
#define AW9523B_REG_P1_3_DIM3    (0x23)
#define AW9523B_REG_P0_0_DIM4    (0x24)
#define AW9523B_REG_P0_1_DIM5    (0x25)
#define AW9523B_REG_P0_2_DIM6    (0x26)
#define AW9523B_REG_P0_3_DIM7    (0x27)
#define AW9523B_REG_P0_4_DIM8    (0x28)
#define AW9523B_REG_P0_5_DIM9    (0x29)
#define AW9523B_REG_P0_6_DIM10   (0x2A)
#define AW9523B_REG_P0_7_DIM11   (0x2B)
#define AW9523B_REG_P1_4_DIM12   (0x2C)
#define AW9523B_REG_P1_5_DIM13   (0x2D)
#define AW9523B_REG_P1_6_DIM14   (0x2E)
#define AW9523B_REG_P1_7_DIM15   (0x2F)
#define AW9523B_REG_SW_RSTN      (0x7F)

#define AW9523B_PORT_NUM              (16)              // aw9523b max led num
#define AW9523B_ALL_PORT_MASK         (0xffff)          // the pin port for P1_7-P0_0
#define AW9523B_GPIO_SET_MASK(val, i) (val | (1 << i))  // bit[15:8] = P1_7 - P1_0, bit[7:0] = P0_1 - P0_0
#define AW9523B_GPIO_CLR_MASK(val, i) (val & ~(1 << i)) // bit[15:8] = P1_7 - P1_0, bit[7:0] = P0_1 - P0_0

#define AW9523B_REG_PRINT_ENABLE      0

typedef enum {
    AW9523B_MODE_LED        = 0,
    AW9523B_MODE_GPIO       = 1,
    AW9523B_MODE_SINGLE_KEY = 2,
    AW9523B_MODE_MATRIX_KEY = 3,
} aw9523b_chip_mode_t;

typedef struct {
    uint16_t twi_id;
    uint32_t twi_freq;
    uint16_t dev_addr;
} drv_extio_param_t;

typedef struct {
    char *name;
    uint8_t index;
    dev_extio_gpio_direction_t direction;
    dev_extio_gpio_level_t level;           //初始化时默认电平，输出模式有效
    bool interrupt_flag;
    uint8_t used;      //0:预留，未使用，1：有定义，需要使用
} aw9523b_pin_map_t;

static drv_extio_param_t s_extio_aw9523b_param = {
    .twi_id   = 0,
    .twi_freq = 100 * 1000,
    .dev_addr = 0x58,
};

static dev_extio_notify_t s_aw9523b_notify = NULL;

static aw9523b_pin_map_t s_pin_map[DEV_EXTIO_PIN_NUM] = {
    [DEV_EXTIO_PIN_RADAR1]          = {"radar1",     0,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  true,  1}, // P0_0
    [DEV_EXTIO_PIN_RADAR2]          = {"radar2",     1,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  true,  1}, // P0_1
    [DEV_EXTIO_PIN_SD_POWER]        = {"sd_power",   2,  DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_LOW,  false, 1}, // P0_2
    [DEV_EXTIO_PIN_SD_DET]          = {"sd_det",     3,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  true,  1}, // P0_3
    [DEV_EXTIO_PIN_RTC_INT]         = {"rtc_irq",    4,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  true,  0}, // P0_4
    [DEV_EXTIO_PIN_FULL_DET]        = {"full_det",   5,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  false, 1}, // P0_5
    [DEV_EXTIO_PIN_CHARGE_DET]      = {"charge_det", 6,  DEV_EXTIO_GPIO_DIR_INPUT,  DEV_EXTIO_GPIO_LEVEL_LOW,  true,  1}, // P0_6
    [DEV_EXTIO_PIN_LCD_BL_LP]       = {"lcd_bl_lp",  7,  DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 1}, // P0_7
    [DEV_EXTIO_PIN_MOTOR1]          = {"motor1",     8,  DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 0}, // P1_0
    [DEV_EXTIO_PIN_MOTOR1]          = {"motor2",     9,  DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 0}, // P1_1
    [DEV_EXTIO_PIN_MOTOR1]          = {"motor3",     10, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 0}, // P1_2
    [DEV_EXTIO_PIN_MOTOR1]          = {"motor4",     11, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 0}, // P1_3
    [DEV_EXTIO_PIN_EXTVCC_EN]       = {"extvcc_en",  12, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_HIGH, false, 1}, // P1_4
    [DEV_EXTIO_PIN_TP_RESET]        = {"tp_reset",   13, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_LOW,  false, 0}, // P1_5
    [DEV_EXTIO_PIN_LCD_RESET]       = {"lcd_reset",  14, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_LOW,  false, 1}, // P1_6
    [DEV_EXTIO_PIN_CAM_PWDN]        = {"cam_pwdn",   15, DEV_EXTIO_GPIO_DIR_OUTPUT, DEV_EXTIO_GPIO_LEVEL_LOW,  false, 1}, // P1_7
};


int drv_extio_iface_i2c_init(drv_extio_param_t *param)
{
    if (NULL == param) {
        printf("invalid extio param.\n");
        return -1;
    }

    twi_status_t twi_status = TWI_STATUS_OK;

    twi_status = hal_twi_init(param->twi_id);

    if (TWI_STATUS_OK != twi_status) {
        printf("i2c init fail: %d.\n", twi_status);
        return -1;
    }

    return 0;
}

int drv_extio_iface_i2c_deinit(drv_extio_param_t *param)
{
    if (NULL == param) {
        printf("invalid extio param.\n");
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
        printf("invalid extio param.\n");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        printf("invalid len.\n");
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
        printf("i2c send fail: %d.\n", twi_status);
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
        printf("invalid extio param.\n");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        printf("invalid len.\n");
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
        printf("i2c recv fail: %d.\n", twi_status);
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
        printf("invalid extio param.\n");
        ret = -1;
        goto exit;
    }

    if (0 == len) {
        printf("invalid len.\n");
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
        printf("i2c recv fail: %d.\n", twi_status);
        ret = -1;
    }

exit:
    return ret;
}


static void aw9523b_soft_reset(void)
{
    uint8_t reg_val = 0;
    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_SW_RSTN, &reg_val, 1);
    XR_OS_MSleep(2);
}

/**
 * @brief  获取 aw9523b 所有端口的模式
 * @param  p0_val P0 端口的模式
 * @param  p1_val P1 端口的模式
 * @return 0:成功, -1:失败
 */
static int aw9523b_port_mode_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0WKMD, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  设置 aw9523b 所有端口的模式
 * @param  mask 状态掩码
 * @param  mode 工作模式, 0:led, 1:gpio
 */
static void aw9523b_port_mode_set(uint16_t mask, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    for (uint8_t i = 0; i < AW9523B_PORT_NUM; i++) {
        if (mask & (0x1 << i)) {
            if (mode) {
                if (i < 8)
                    reg_val[1] |= 0x1 << i;
                else
                    reg_val[2] |= 0x1 << (i - 8);
            } else {
                if (i < 8)
                    reg_val[1] &= ~(0x1 << i);
                else
                    reg_val[2] &= ~(0x1 << (i - 8));
            }
        }
    }

    reg_val[0] = AW9523B_REG_P0WKMD;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  设置 aw9523b P0 端口的驱动输出模式
 * @param  mode 工作模式, 0:OD, 1:PP
 */
static void aw9523b_port_p0_output_mode_set(uint8_t mode)
{
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_GCR_REG, &reg_val[1], 1);

    if (mode) {
        reg_val[1] |= 0x01 << 4;
    } else {
        reg_val[1] &= ~(0x01 << 4);
    }

    reg_val[0] = AW9523B_REG_GCR_REG;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 2);
}

/**
 * @brief  获取 aw9523b 所有端口的方向
 * @param  p0_val P0 端口的模式
 * @param  p1_val P1 端口的模式
 * @return 0:成功, -1:失败
 */
static int aw9523b_port_direction_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0_CONFIG, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  设置 aw9523b 所有端口的方向
 * @param  mask 状态掩码
 * @param  mode 工作模式, 0:output, 1:input
 */
static void aw9523b_port_direction_all_set(uint16_t mask, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    for (uint8_t i = 0; i < AW9523B_PORT_NUM; i++) {
        if (mask & (0x1 << i)) {
            if (mode) {
                if (i < 8)
                    reg_val[1] |= 0x1 << i;
                else
                    reg_val[2] |= 0x1 << (i - 8);
            } else {
                if (i < 8)
                    reg_val[1] &= ~(0x1 << i);
                else
                    reg_val[2] &= ~(0x1 << (i - 8));
            }
        }
    }

    reg_val[0] = AW9523B_REG_P0_CONFIG;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  设置 aw9523b 指定端口的方向
 * @param  index 端口索引
 * @param  mode 工作模式, 0:output, 1:input
 */
static void aw9523b_port_direction_set(uint8_t index, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    aw9523b_port_direction_get(&reg_val[1], &reg_val[2]);

    if (mode) {
        if (index < 8)
            reg_val[1] |= 0x1 << index;
        else
            reg_val[2] |= 0x1 << (index - 8);
    } else {
        if (index < 8)
            reg_val[1] &= ~(0x1 << index);
        else
            reg_val[2] &= ~(0x1 << (index - 8));
    }

    reg_val[0] = AW9523B_REG_P0_CONFIG;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  获取 aw9523b 指定端口的输入状态
 * @param  index 端口索引
 * @param  input_state 所有端口的输入状态
 * @return 0:低电平, 1:高电平
 */
static int aw9523b_port_input_state_get(uint8_t index)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0_INPUT, &reg_val[0], 1);
    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P1_INPUT, &reg_val[1], 1);

    if (index < 8)
        ret = reg_val[0] & (0x1 << index);
    else
        ret = reg_val[1] & (0x1 << (index - 8));

    return (ret > 0) ? 1 : 0;
}

/**
 * @brief  设置 aw9523b 指定端口的输出状态
 * @param  index 端口索引
 * @param  val 输出状态, 0:low, 1:high
 */
static void aw9523b_port_output_state_set(uint8_t index, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0_OUTPUT, &reg_val[1], 2);

    if (val) {
        if (index < 8)
            reg_val[1] |= 0x1 << index;
        else
            reg_val[2] |= 0x1 << (index - 8);
    } else {
        if (index < 8)
            reg_val[1] &= ~(0x1 << index);
        else
            reg_val[2] &= ~(0x1 << (index - 8));
    }

    reg_val[0] = AW9523B_REG_P0_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  获取 aw9523b 所有端口的中断使能状态
 * @param  p0_val P0 端口的状态
 * @param  p1_val P1 端口的状态
 * @return 0:成功, -1:失败
 */
static int aw9523b_port_interrupt_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0INT_MSK, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  配置 aw9523b 所有端口的中断
 * @param  mask 状态掩码
 * @param  val 0:disable, 1:enable
 */
static void aw9523b_port_interrupt_all_enable(uint16_t mask, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0INT_MSK, &reg_val[1], 2);

    for (uint8_t i = 0; i < AW9523B_PORT_NUM; i++) {
        if (mask & (0x1 << i)) {
            if (!val) {
                if (i < 8)
                    reg_val[1] |= 0x1 << i;
                else
                    reg_val[2] |= 0x1 << (i - 8);
            } else {
                if (i < 8)
                    reg_val[1] &= ~(0x1 << i);
                else
                    reg_val[2] &= ~(0x1 << (i - 8));
            }
        }
    }

    reg_val[0] = AW9523B_REG_P0INT_MSK;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  配置 aw9523b 指定端口的中断
 * @param  index 端口索引
 * @param  val 0:disable, 1:enable
 * @note   使能中断后, 高到低 / 低到高都会触发中断
 */
static void aw9523b_port_interrupt_enable(uint8_t index, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0INT_MSK, &reg_val[1], 2);

    if (!val) {
        if (index < 8)
            reg_val[1] |= 0x1 << index;
        else
            reg_val[2] |= 0x1 << (index - 8);
    } else {
        if (index < 8)
            reg_val[1] &= ~(0x1 << index);
        else
            reg_val[2] &= ~(0x1 << (index - 8));
    }

    reg_val[0] = AW9523B_REG_P0INT_MSK;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 3);
}

/**
 * @brief  清除 aw9523b 所有端口的中断状态
 * @return 当前的中断标志位
 */
static uint16_t aw9523b_port_interrupt_clear(void)
{
    int ret = -1;
    uint8_t reg_val[2] = {0};

    // 一旦触发中断, 必须分开读一次 P0 和 P1 的全部端口来清除中断标志位
    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P0_INPUT, &reg_val[0], 1);
    if (0 > ret) {
        printf("recv P0 err\n");
    }
    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_P1_INPUT, &reg_val[1], 1);
    if (0 > ret) {
        printf("recv P1 err\n");
    }

    return (reg_val[0] | (reg_val[1] << 8));
}

/**
 * @brief  读取 aw9523b 芯片 id
 * @return 0:是该款芯片, -1:未知芯片
 */
static int aw9523b_read_chipid(void)
{
    int ret = 0;
    uint8_t reg_val = 0;

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_aw9523b_param, AW9523B_REG_CHIP_ID, &reg_val, 1);
    if (0 != ret) {
        printf("read chipid fail.\n");
        return -1;
    }
    printf("chip id: 0x%x\n", reg_val);

    if (0x23 == reg_val) {
        return 0;
    }

    return -1;
}

/**
 * @brief  aw9523b 芯片以 GPIO 模式初始化 
 */
static void aw9523b_gpio_mode_init(void)
{
    aw9523b_port_interrupt_all_enable(AW9523B_ALL_PORT_MASK, 0);
    aw9523b_port_interrupt_clear();

    aw9523b_port_mode_set(AW9523B_ALL_PORT_MASK, 1);

    aw9523b_port_direction_all_set(AW9523B_ALL_PORT_MASK, 0);
    aw9523b_port_p0_output_mode_set(1);
}

/**
 * @brief  aw9523b 芯片以矩阵按键模式初始化 
 */
static void aw9523b_matrix_key_mode_init(void)
{
    aw9523b_port_interrupt_all_enable(AW9523B_ALL_PORT_MASK, 0);
    aw9523b_port_interrupt_clear();

    aw9523b_port_mode_set(AW9523B_ALL_PORT_MASK, 1);

    aw9523b_port_direction_all_set(AW9523B_ALL_PORT_MASK, 1);

    aw9523b_port_interrupt_all_enable(AW9523B_ALL_PORT_MASK, 1);
    aw9523b_port_interrupt_clear();
}
/**
 * @brief  aw9523b 芯片初始化后单独设置耳机检测引脚 
 */
void aw9523b_earphone_gpio_init(void)
{
    // aw9523b_port_direction_set(DEV_EXTIO_PIN_EARPHONE_DET, 1); //输入模式
    // s_pin_map[DEV_EXTIO_PIN_EARPHONE_DET].interrupt_flag = true;
    // aw9523b_port_interrupt_enable(s_pin_map[DEV_EXTIO_PIN_EARPHONE_DET].index, 1); //开启中断
    // aw9523b_port_interrupt_clear();
}

/**
 * @brief aw9523b 芯片检测耳机检测GPIO引脚电平
 * @return 1:未触发,0:触发
 */
uint8_t aw9523b_read_earphone_gpio(void)
{
    uint8_t value = 0;
    // value = aw9523b_port_input_state_get(DEV_EXTIO_PIN_EARPHONE_DET);
    return value;
}


/**
 * @brief  aw9523b 芯片初始化
 * @param  mode 初始化的模式
 */
static void aw9523b_chip_init(aw9523b_chip_mode_t mode)
{
    switch (mode) {
    case AW9523B_MODE_LED:
        break;

    case AW9523B_MODE_GPIO:
        aw9523b_gpio_mode_init();
        break;

    case AW9523B_MODE_SINGLE_KEY:
        break;

    case AW9523B_MODE_MATRIX_KEY:
        break;

    default:
        break;
    }
}

/**
 * @brief  aw9523b 中断处理函数
 * @param  args 回调私有参数传递
 */
__nonxip_text static void aw9523b_irq_cb(void *args)
{
    if (NULL != s_aw9523b_notify) {
        s_aw9523b_notify(NULL);
    }
}

/**
 * @brief  aw9523b 芯片注销
 */
static void drv_extio_aw9523b_deinit(void)
{
    aw9523b_port_interrupt_all_enable(AW9523B_ALL_PORT_MASK, 0);
    aw9523b_port_interrupt_clear();
    
    drv_extio_iface_i2c_deinit(&s_extio_aw9523b_param);
}

static int drv_extio_aw9523b_suspend(void)
{
    // TODO
    return 0;
}

static int drv_extio_aw9523b_resume(void)
{
    // TODO
    return 0;
}

static int drv_extio_aw9523b_gpio_init(dev_exito_pin_t pin, dev_extio_gpio_direction_t dir, bool interrupt_flag)
{
    if (DEV_EXTIO_GPIO_DIR_OUTPUT == dir) {
        aw9523b_port_direction_set(s_pin_map[pin].index, 0);
    } else if (DEV_EXTIO_GPIO_DIR_INPUT == dir) {
        aw9523b_port_direction_set(s_pin_map[pin].index, 1);

        if (interrupt_flag) {
            s_pin_map[pin].interrupt_flag = true;
            aw9523b_port_interrupt_enable(s_pin_map[pin].index, 1);
            aw9523b_port_interrupt_clear();
        }else{
            s_pin_map[pin].interrupt_flag = false;
            aw9523b_port_interrupt_enable(s_pin_map[pin].index, 0);
        }
    }

    // drv_extio_aw9523b_debug("gpio [%s] set: %s.", s_pin_map[pin].name, (DEV_EXTIO_GPIO_DIR_OUTPUT == dir) ? "output" : "input");
    msleep(5);
    return 0;
}

static int drv_extio_aw9523b_gpio_set(dev_exito_pin_t pin, dev_extio_gpio_level_t level)
{
    if (DEV_EXTIO_GPIO_LEVEL_LOW == level) {
        aw9523b_port_output_state_set(s_pin_map[pin].index, 0);
    } else if (DEV_EXTIO_GPIO_LEVEL_HIGH == level) {
        aw9523b_port_output_state_set(s_pin_map[pin].index, 1);
    }
    // drv_extio_aw9523b_debug("gpio [%s] set: %d.", s_pin_map[pin].name, level);

    return 0;
}

static dev_extio_gpio_level_t drv_extio_aw9523b_gpio_get(dev_exito_pin_t pin)
{
    int ret = 0;
    ret = aw9523b_port_input_state_get(s_pin_map[pin].index);
    // drv_extio_aw9523b_debug("gpio [%s] get: %d.", s_pin_map[pin].name, ret);
    return (0 == ret) ? DEV_EXTIO_GPIO_LEVEL_LOW : DEV_EXTIO_GPIO_LEVEL_HIGH;
}

static int drv_extio_aw9523b_irq_flag_get(uint16_t *irq_flag)
{
    int ret = 0;

    uint16_t tmp_irq_flag = 0;

    tmp_irq_flag = aw9523b_port_interrupt_clear();

    if (NULL != irq_flag) {
        *irq_flag = tmp_irq_flag;
    } else {
        ret = -1;
    }

    return ret;
}


/**
 * @brief  aw9523b 芯片初始化
 * @return 0:成功, -1:失败
 */
static int drv_extio_aw9523b_init(dev_extio_notify_t cb)
{
    s_aw9523b_notify = cb;
    drv_extio_iface_i2c_init(&s_extio_aw9523b_param);
    XR_OS_MSleep(50);
   
    if (0 != aw9523b_read_chipid()) {
        printf("aw9523b init fail.\n");
        drv_extio_aw9523b_deinit();
        return -1;
    }

    // set gpio output low
    uint8_t reg_val[2] = {0};
    reg_val[0] = AW9523B_REG_P0_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 2);

    reg_val[0] = AW9523B_REG_P1_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_aw9523b_param, reg_val, 2);

    aw9523b_chip_init(AW9523B_MODE_GPIO);

    //初始化扩展io的引脚
    for (int i = 0; i < sizeof(s_pin_map) / sizeof(s_pin_map[0]); i++) {
        if (s_pin_map[i].used) {
            drv_extio_aw9523b_gpio_init(s_pin_map[DEV_EXTIO_PIN_EXTVCC_EN].index, s_pin_map[DEV_EXTIO_PIN_EXTVCC_EN].direction, s_pin_map[DEV_EXTIO_PIN_EXTVCC_EN].interrupt_flag);

            if (s_pin_map[i].direction == DEV_EXTIO_GPIO_DIR_OUTPUT) {
                drv_extio_aw9523b_gpio_set(s_pin_map[i].index, s_pin_map[i].level);
            }
        }
    }

    printf("aw9523b init success.\n");

    return 0;
}

void drv_extio_aw9523b_ops_register(drv_extio_ops_t* ops)
{
    if (NULL != ops) {
        ops->name         = "aw9523b";
        ops->init         = drv_extio_aw9523b_init;
        ops->deinit       = drv_extio_aw9523b_deinit;
        ops->suspend      = drv_extio_aw9523b_suspend;
        ops->resume       = drv_extio_aw9523b_resume;
        ops->gpio_init    = drv_extio_aw9523b_gpio_init;
        ops->gpio_set     = drv_extio_aw9523b_gpio_set;
        ops->gpio_get     = drv_extio_aw9523b_gpio_get;
        ops->irq_flag_get = drv_extio_aw9523b_irq_flag_get;
    } else {
        printf("ops is NULL.\n");
    }
}

