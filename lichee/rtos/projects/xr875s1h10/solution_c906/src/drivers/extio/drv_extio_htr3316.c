#include <stdbool.h>
#include "compiler.h"
#include "kernel/os/os.h"
#include "../drv_log.h"
#include "dev_extio.h"
#include "drv_extio_iface.h"

#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if DBUG
#define drv_extio_htr3316_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_extio_htr3316_debug(fmt, ...)
#endif

#if INFO
#define drv_extio_htr3316_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_extio_htr3316_info(fmt, ...)
#endif

#if WARN
#define drv_extio_htr3316_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_extio_htr3316_warn(fmt, ...)
#endif

#if EROR
#define drv_extio_htr3316_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_extio_htr3316_error(fmt, ...)
#endif

/**
 * htr3316扩展IO的相关引脚:
 *   P0_0 - P0_7: 可配置为GPIO/LED模式, 对于GPIO模式, 支持push-pull和open-drain
 *   P1_0 - P1_7: 可配置为GPIO/LED模式, 对于GPIO模式, 仅支持push-pull
 * 
 *   P0_0: 输入模式, 用于 KEY2
 *   P0_1: 输入模式, 用于 KEY1
 *   P0_2: 输入模式, 用于 耳机检测 PH_IN
 *   P0_3: 输入模式, 用于 咪头检测 MIC_EN
 *   P0_4: 输入模式, 用于 满电检测 FULL_DET
 *   P0_5: 输入模式, 用于 充电检测 VBUS_DET
 *   P0_6: 输入模式, 用于 外部RTC中断检测 RTC_INT
 *   P0_7: 输入模式, 用于 雷达状态监测 RADER_DET
 * 
 *   P1_0: 输出模式, 用于 LCD-RST
 *   P1_1: 输出模式, 用于 预留TP复位
 *   P1_2: 输出模式, 用于 显示供电使能
 *   P1_3: 输出模式, 用于 低功耗时背光显示
 *   P1_4: 输出模式, 用于 雷达供电使能
 *   P1_5: 输出模式, 用于 RGB灯供电使能
 *   P1_6: 输出模式, 用于 外部3.3V供电使能
 *   P1_7: 输出模式, 用于 功放使能
 */

#define htr3316_REG_P0_INPUT     (0x00)
#define htr3316_REG_P1_INPUT     (0x01)
#define htr3316_REG_P0_OUTPUT    (0x02)
#define htr3316_REG_P1_OUTPUT    (0x03)
#define htr3316_REG_P0_CONFIG    (0x04)
#define htr3316_REG_P1_CONFIG    (0x05)
#define htr3316_REG_P0INT_MSK    (0x06)
#define htr3316_REG_P1INT_MSK    (0x07)
#define htr3316_REG_CHIP_ID      (0x10)
#define htr3316_REG_GCR_REG      (0x11)
#define htr3316_REG_P0WKMD       (0x12)
#define htr3316_REG_P1WKMD       (0x13)
#define htr3316_REG_PATEN        (0x14)
#define htr3316_REG_FDTMR        (0x15)
#define htr3316_REG_FLTMR        (0x16)
#define htr3316_REG_P1_0_DIM0    (0x20)
#define htr3316_REG_P1_1_DIM1    (0x21)
#define htr3316_REG_P1_2_DIM2    (0x22)
#define htr3316_REG_P1_3_DIM3    (0x23)
#define htr3316_REG_P0_0_DIM4    (0x24)
#define htr3316_REG_P0_1_DIM5    (0x25)
#define htr3316_REG_P0_2_DIM6    (0x26)
#define htr3316_REG_P0_3_DIM7    (0x27)
#define htr3316_REG_P0_4_DIM8    (0x28)
#define htr3316_REG_P0_5_DIM9    (0x29)
#define htr3316_REG_P0_6_DIM10   (0x2A)
#define htr3316_REG_P0_7_DIM11   (0x2B)
#define htr3316_REG_P1_4_DIM12   (0x2C)
#define htr3316_REG_P1_5_DIM13   (0x2D)
#define htr3316_REG_P1_6_DIM14   (0x2E)
#define htr3316_REG_P1_7_DIM15   (0x2F)
#define htr3316_REG_SW_RSTN      (0x7F)

#define htr3316_PORT_NUM              (16)              // htr3316 max led num
#define htr3316_ALL_PORT_MASK         (0xffff)          // the pin port for P1_7-P0_0
#define htr3316_GPIO_SET_MASK(val, i) (val | (1 << i))  // bit[15:8] = P1_7 - P1_0, bit[7:0] = P0_7 - P0_0
#define htr3316_GPIO_CLR_MASK(val, i) (val & ~(1 << i)) // bit[15:8] = P1_7 - P1_0, bit[7:0] = P0_7 - P0_0

#define htr3316_REG_PRINT_ENABLE      0

typedef enum {
    htr3316_MODE_LED        = 0,
    htr3316_MODE_GPIO       = 1,
    htr3316_MODE_SINGLE_KEY = 2,
    htr3316_MODE_MATRIX_KEY = 3,
} htr3316_chip_mode_t;

typedef struct {
    char *name;
    uint8_t index;
    dev_extio_gpio_direction_t direction;
    dev_extio_gpio_level_t level;           //初始化时默认电平，输出模式有效
    bool interrupt_flag;
    uint8_t used;      //0:预留，未使用，1：有定义，需要使用
} htr3316_pin_map_t;

static drv_extio_param_t s_extio_htr3316_param = {
    .twi_id   = 1,
    .twi_freq = 100 * 1000,
    .dev_addr = 0x58,
};

static dev_extio_notify_t s_htr3316_notify = NULL;

static htr3316_pin_map_t s_pin_map[DEV_EXTIO_PIN_NUM] = {
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

static void htr3316_soft_reset(void)
{
    uint8_t reg_val = 0;
    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_SW_RSTN, &reg_val, 1);
    XR_OS_MSleep(2);
}

/**
 * @brief  获取 htr3316 所有端口的模式
 * @param  p0_val P0 端口的模式
 * @param  p1_val P1 端口的模式
 * @return 0:成功, -1:失败
 */
static int htr3316_port_mode_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0WKMD, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  设置 htr3316 所有端口的模式
 * @param  mask 状态掩码
 * @param  mode 工作模式, 0:led, 1:gpio
 */
static void htr3316_port_mode_set(uint16_t mask, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    for (uint8_t i = 0; i < htr3316_PORT_NUM; i++) {
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

    reg_val[0] = htr3316_REG_P0WKMD;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  设置 htr3316 P0 端口的驱动输出模式
 * @param  mode 工作模式, 0:OD, 1:PP
 */
static void htr3316_port_p0_output_mode_set(uint8_t mode)
{
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_GCR_REG, &reg_val[1], 1);

    if (mode) {
        reg_val[1] |= 0x01 << 4;
    } else {
        reg_val[1] &= ~(0x01 << 4);
    }

    reg_val[0] = htr3316_REG_GCR_REG;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 2);
}

/**
 * @brief  获取 htr3316 所有端口的方向
 * @param  p0_val P0 端口的模式
 * @param  p1_val P1 端口的模式
 * @return 0:成功, -1:失败
 */
static int htr3316_port_direction_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_CONFIG, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  设置 htr3316 所有端口的方向
 * @param  mask 状态掩码
 * @param  mode 工作模式, 0:output, 1:input
 */
static void htr3316_port_direction_all_set(uint16_t mask, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    for (uint8_t i = 0; i < htr3316_PORT_NUM; i++) {
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

    reg_val[0] = htr3316_REG_P0_CONFIG;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  设置 htr3316 指定端口的方向
 * @param  index 端口索引
 * @param  mode 工作模式, 0:output, 1:input
 */
static void htr3316_port_direction_set(uint8_t index, uint8_t mode)
{
    uint8_t reg_val[3] = {0};

    htr3316_port_direction_get(&reg_val[1], &reg_val[2]);

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

    reg_val[0] = htr3316_REG_P0_CONFIG;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  获取 htr3316 指定端口的输入状态
 * @param  index 端口索引
 * @return 0:低电平, 1:高电平
 */
static int htr3316_port_input_state_get(uint8_t index)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_INPUT, &reg_val[0], 1);
    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P1_INPUT, &reg_val[1], 1);

    if (index < 8)
        ret = reg_val[0] & (0x1 << index);
    else
        ret = reg_val[1] & (0x1 << (index - 8));

    return (ret > 0) ? 1 : 0;
}

/**
 * @brief  设置 htr3316 指定端口的输出状态
 * @param  index 端口索引
 * @param  val 输出状态, 0:low, 1:high
 */
static void htr3316_port_output_state_set(uint8_t index, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_OUTPUT, &reg_val[1], 2);

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

    reg_val[0] = htr3316_REG_P0_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  获取 htr3316 所有端口的中断使能状态
 * @param  p0_val P0 端口的状态
 * @param  p1_val P1 端口的状态
 * @return 0:成功, -1:失败
 */
static int htr3316_port_interrupt_get(uint8_t *p0_val, uint8_t *p1_val)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0INT_MSK, reg_val, 2);

    if (NULL != p0_val) {
        *p0_val = reg_val[0];
    }

    if (NULL != p1_val) {
        *p1_val = reg_val[1];
    }

    return ret;
}

/**
 * @brief  配置 htr3316 所有端口的中断
 * @param  mask 状态掩码
 * @param  val 0:disable, 1:enable
 */
static void htr3316_port_interrupt_all_enable(uint16_t mask, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0INT_MSK, &reg_val[1], 2);

    for (uint8_t i = 0; i < htr3316_PORT_NUM; i++) {
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

    reg_val[0] = htr3316_REG_P0INT_MSK;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  配置 htr3316 指定端口的中断
 * @param  index 端口索引
 * @param  val 0:disable, 1:enable
 * @note   使能中断后, 高到低 / 低到高都会触发中断
 */
static void htr3316_port_interrupt_enable(uint8_t index, uint8_t val)
{
    uint8_t reg_val[3] = {0};

    drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0INT_MSK, &reg_val[1], 2);

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

    reg_val[0] = htr3316_REG_P0INT_MSK;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 3);
}

/**
 * @brief  清除 htr3316 所有端口的中断状态
 * @return 当前的中断标志位
 */
uint16_t htr3316_port_interrupt_clear(void)
{
    int ret = -1;
    uint8_t reg_val[2] = {0};

    // 一旦触发中断, 必须分开读一次 P0 和 P1 的全部端口来清除中断标志位
    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_INPUT, &reg_val[0], 1);
    if (0 > ret) {
        drv_extio_htr3316_error("recv P0 err");
    }
    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P1_INPUT, &reg_val[1], 1);
    if (0 > ret) {
        drv_extio_htr3316_error("recv P1 err");
    }

    return (reg_val[0] | (reg_val[1] << 8));
}

/**
 * @brief  读取 htr3316 芯片 id
 * @return 0:是该款芯片, -1:未知芯片
 */
static int htr3316_read_chipid(void)
{
    int ret = 0;
    uint8_t reg_val = 0;

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_CHIP_ID, &reg_val, 1);
    if (0 != ret) {
        drv_extio_htr3316_error("read chipid fail.");
        return -1;
    }
    drv_extio_htr3316_debug("chip id: 0x%x", reg_val);

    if (0x23 == reg_val) {
        return 0;
    }

    return -1;
}

/**
 * @brief  htr3316 芯片以 GPIO 模式初始化 
 */
static void htr3316_gpio_mode_init(void)
{
    htr3316_port_interrupt_all_enable(htr3316_ALL_PORT_MASK, 0);
    htr3316_port_interrupt_clear();

    htr3316_port_mode_set(htr3316_ALL_PORT_MASK, 1);

    htr3316_port_direction_all_set(htr3316_ALL_PORT_MASK, 0);
    htr3316_port_p0_output_mode_set(1);
}

/**
 * @brief  htr3316 芯片以矩阵按键模式初始化 
 */



static void htr3316_matrix_key_mode_init(void)
{
    htr3316_port_interrupt_all_enable(htr3316_ALL_PORT_MASK, 0);
    htr3316_port_interrupt_clear();

    htr3316_port_mode_set(htr3316_ALL_PORT_MASK, 1);

    htr3316_port_direction_all_set(htr3316_ALL_PORT_MASK, 1);

    htr3316_port_interrupt_all_enable(htr3316_ALL_PORT_MASK, 1);
    htr3316_port_interrupt_clear();
}
/**
 * @brief  htr3316 芯片初始化后单独设置耳机检测引脚 
 */
void htr3316_earphone_gpio_init(void)
{
    // htr3316_port_direction_set(DEV_EXTIO_PIN_EARPHONE_DET, 1); //输入模式
    // s_pin_map[DEV_EXTIO_PIN_EARPHONE_DET].interrupt_flag = true;
    // htr3316_port_interrupt_enable(s_pin_map[DEV_EXTIO_PIN_EARPHONE_DET].index, 1); //开启中断
    // htr3316_port_interrupt_clear();
}

/**
 * @brief htr3316 芯片检测耳机检测GPIO引脚电平
 * @return 1:未触发,0:触发
 */
uint8_t htr3316_read_earphone_gpio(void)
{
    uint8_t value = 0;
    // value = htr3316_port_input_state_get(DEV_EXTIO_PIN_EARPHONE_DET);
    return value;
}


/**
 * @brief  htr3316 芯片初始化
 * @param  mode 初始化的模式
 */
static void htr3316_chip_init(htr3316_chip_mode_t mode)
{
    switch (mode) {
    case htr3316_MODE_LED:
        break;

    case htr3316_MODE_GPIO:
        htr3316_gpio_mode_init();
        break;

    case htr3316_MODE_SINGLE_KEY:
        break;

    case htr3316_MODE_MATRIX_KEY:
        break;

    default:
        break;
    }
}

/**
 * @brief  htr3316 中断处理函数
 * @param  args 回调私有参数传递
 */
__nonxip_text static void htr3316_irq_cb(void *args)
{
    if (NULL != s_htr3316_notify) {
        s_htr3316_notify(NULL);
    }
}

static int drv_extio_htr3316_gpio_init(dev_exito_pin_t pin, dev_extio_gpio_direction_t dir, bool interrupt_flag)
{
    if (DEV_EXTIO_GPIO_DIR_OUTPUT == dir) {
        htr3316_port_direction_set(s_pin_map[pin].index, 0);
    } else if (DEV_EXTIO_GPIO_DIR_INPUT == dir) {
        htr3316_port_direction_set(s_pin_map[pin].index, 1);

        if (interrupt_flag) {
            s_pin_map[pin].interrupt_flag = true;
            htr3316_port_interrupt_enable(s_pin_map[pin].index, 1);
            htr3316_port_interrupt_clear();
        }else{
            s_pin_map[pin].interrupt_flag = false;
            htr3316_port_interrupt_enable(s_pin_map[pin].index, 0);
        }
    }

    // drv_extio_htr3316_debug("gpio [%s] set: %s.", s_pin_map[pin].name, (DEV_EXTIO_GPIO_DIR_OUTPUT == dir) ? "output" : "input");
    msleep(5);
    return 0;
}

static int drv_extio_htr3316_gpio_set(dev_exito_pin_t pin, dev_extio_gpio_level_t level)
{
    if (DEV_EXTIO_GPIO_LEVEL_LOW == level) {
        htr3316_port_output_state_set(s_pin_map[pin].index, 0);
    } else if (DEV_EXTIO_GPIO_LEVEL_HIGH == level) {
        htr3316_port_output_state_set(s_pin_map[pin].index, 1);
    }
    // drv_extio_htr3316_debug("gpio [%s] set: %d.", s_pin_map[pin].name, level);

    return 0;
}

static dev_extio_gpio_level_t drv_extio_htr3316_gpio_get(dev_exito_pin_t pin)
{
    int ret = 0;
    ret = htr3316_port_input_state_get(s_pin_map[pin].index);
    // drv_extio_htr3316_debug("gpio [%s] get: %d.", s_pin_map[pin].name, ret);
    return (0 == ret) ? DEV_EXTIO_GPIO_LEVEL_LOW : DEV_EXTIO_GPIO_LEVEL_HIGH;
}

static int drv_extio_htr3316_irq_flag_get(uint16_t *irq_flag)
{
    int ret = 0;

    uint16_t tmp_irq_flag = 0;

    tmp_irq_flag = htr3316_port_interrupt_clear();

    if (NULL != irq_flag) {
        *irq_flag = tmp_irq_flag;
    } else {
        ret = -1;
    }

    return ret;
}

/**
 * @brief  htr3316 芯片注销
 */
static void drv_extio_htr3316_deinit(void)
{
    htr3316_port_interrupt_all_enable(htr3316_ALL_PORT_MASK, 0);
    htr3316_port_interrupt_clear();

    for (int i = 0; i < sizeof(s_pin_map)/sizeof(s_pin_map[0]); i++) {
        if (s_pin_map[i].used) {
            if (s_pin_map[i].direction == DEV_EXTIO_GPIO_DIR_OUTPUT) {
                drv_extio_htr3316_gpio_set(s_pin_map[i].index, DEV_EXTIO_GPIO_LEVEL_LOW);
            }
        }
    }
    
    drv_extio_iface_i2c_deinit(&s_extio_htr3316_param);
    drv_extio_iface_irq_deinit();
}

/**
 * @brief  htr3316 芯片初始化
 * @return 0:成功, -1:失败
 */
static int drv_extio_htr3316_init(dev_extio_notify_t cb)
{
    s_htr3316_notify = cb;
    drv_extio_iface_i2c_init(&s_extio_htr3316_param);
    XR_OS_MSleep(50);
    drv_extio_iface_irq_init((drv_extio_iface_irq_cb_t)htr3316_irq_cb);
    XR_OS_MSleep(50);

    if (0 != htr3316_read_chipid()) {
        drv_extio_htr3316_error("htr3316 init fail.");
        drv_extio_htr3316_deinit();
        return -1;
    }

    // htr3316_soft_reset();

    // set gpio output low
    uint8_t reg_val[2] = {0};
    reg_val[0] = htr3316_REG_P0_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 2);

    reg_val[0] = htr3316_REG_P1_OUTPUT;
    drv_extio_iface_i2c_send(&s_extio_htr3316_param, reg_val, 2);

    htr3316_chip_init(htr3316_MODE_GPIO);

    //初始化扩展io的引脚
    for (int i = 0; i < sizeof(s_pin_map) / sizeof(s_pin_map[0]); i++) {
        if (s_pin_map[i].used) {
            drv_extio_htr3316_gpio_init(s_pin_map[i].index, s_pin_map[i].direction, s_pin_map[i].interrupt_flag);

            if (s_pin_map[i].direction == DEV_EXTIO_GPIO_DIR_OUTPUT) {
                drv_extio_htr3316_gpio_set(s_pin_map[i].index, s_pin_map[i].level);
            }
        }
    }

    drv_extio_htr3316_info("htr3316 init success.");

    return 0;
}

static int drv_extio_htr3316_suspend(void)
{
    // TODO
    drv_extio_iface_irq_deinit();
    return 0;
}

static int drv_extio_htr3316_resume(void)
{
    // TODO
    drv_extio_iface_irq_init(htr3316_irq_cb);
    return 0;
}

void drv_extio_htr3316_ops_register(drv_extio_ops_t* ops)
{
    if (NULL != ops) {
        ops->name         = "htr3316";
        ops->init         = drv_extio_htr3316_init;
        ops->deinit       = drv_extio_htr3316_deinit;
        ops->suspend      = drv_extio_htr3316_suspend;
        ops->resume       = drv_extio_htr3316_resume;
        ops->gpio_init    = drv_extio_htr3316_gpio_init;
        ops->gpio_set     = drv_extio_htr3316_gpio_set;
        ops->gpio_get     = drv_extio_htr3316_gpio_get;
        ops->irq_flag_get = drv_extio_htr3316_irq_flag_get;
    } else {
        drv_extio_htr3316_error("ops is NULL.");
    }
}

#if htr3316_REG_PRINT_ENABLE
#include <console.h>
static void cmd_extio_htr3316_print(int argc, char **argv)
{
    int ret = 0;
    uint8_t reg_val[2] = {0};

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_OUTPUT, reg_val, 2);
    if (0 == ret) {
        printf("Output P0: 0x%.2x, Output P1: 0x%.2x.\n", reg_val[0], reg_val[1]);
    }

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0_CONFIG, reg_val, 2);
    if (0 == ret) {
        printf("Config P0: 0x%.2x, Config P1: 0x%.2x.\n", reg_val[0], reg_val[1]);
    }

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_GCR_REG, reg_val, 1);
    if (0 == ret) {
        printf("GCR: 0x%.2x.\n", reg_val[0]);
    }

    ret = drv_extio_iface_i2c_recv_with_addr(&s_extio_htr3316_param, htr3316_REG_P0INT_MSK, reg_val, 2);
    if (0 == ret) {
        printf("IRQ Enable P0: 0x%.2x, Config P1: 0x%.2x.\n", reg_val[0], reg_val[1]);
    }
    printf("Input state: 0x%.2x.\n", htr3316_port_interrupt_clear());
}
FINSH_FUNCTION_EXPORT_CMD(cmd_extio_htr3316_print, extio_htr3316_print, extio htr3316 reg print);
#endif

