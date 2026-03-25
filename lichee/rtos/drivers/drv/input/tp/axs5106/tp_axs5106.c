#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <console.h>
#include <hal_gpio.h>
#include <sunxi_hal_twi.h>
#include <sunxi-input.h>
#include "compiler.h"
#include "kernel/os/os.h"
#include "../config/tp_board_config.h"


#include "../../../../../projects/xr875s1h10/solution_c906/src/drivers/drv_log.h"

#define DRV_VERB 0
#define DRV_INFO 1
#define DRV_DBUG 1
#define DRV_WARN 1
#define DRV_EROR 1

#if DRV_VERB
#define dev_tp_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_tp_verb(fmt, ...)
#endif

#if DRV_INFO
#define dev_tp_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_tp_info(fmt, ...)
#endif

#if DRV_DBUG
#define dev_tp_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_tp_debug(fmt, ...)
#endif

#if DRV_WARN
#define dev_tp_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_tp_warn(fmt, ...)
#endif

#if DRV_EROR
#define dev_tp_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_tp_error(fmt, ...)
#endif

#define TP_AXS5106_DEBUG_ENABLE 1
#if TP_AXS5106_DEBUG_ENABLE
#define TP_AXS5106_DEBUG(fmt, ...) printf("[axs5106 %4d] " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define TP_AXS5106_DEBUG(fmt, ...)
#endif

#define TP_AXS5106_I2C_ADDR  0x63
#define TP_AXS5106_I2C_IDX   TWI_MASTER_0

#define TP_USE_USER_RESET_PIN 1     // 使用用户自定义的复位引脚 (不是fex内配置的)

#if TP_USE_USER_RESET_PIN
extern int dev_mcu_tp_reset_set(uint8_t val,uint8_t block);
#else
#define TP_AXS5106_RESET_PIN GPIOA(8)
#define TP_AXS5106_IRQ_PIN   GPIOA(7)
#endif // #if TP_USE_USER_RESET_PIN

#define TP_AXS5106_FIRMWARE_LFS_PATH   "/data/lfs/tp_update.bin"
#define TP_AXS5106_FIRMWARE_FATFS_PATH "/sdmmc/tp_update.bin"

typedef enum {
    GESTURE_REG     = 0x01,
    VERSION_REG     = 0x05,
    CHIPID_REG      = 0x08,
    PROJECT_REG     = 0x0A,
    RESET_REG       = 0xFF,
    EXIT_DEBUG_REG  = 0xA0,
    ENTER_DEBUG_REG = 0xAA,
    WRITE_FLASH_REG = 0x90,
    READ_FLASH_REG  = 0x80,
    CHECK_DEBUG_REG = 0xD1,
    SLEEP_REG       = 0x19,
} axs5106_reg_t;

typedef struct {
    uint8_t run_flag;
    uint32_t irq_num;
    XR_OS_Semaphore_t irq_sem;
    XR_OS_Thread_t thread;
    struct sunxi_input_dev *input_dev;
} tp_axs5106_info_t;

static tp_axs5106_info_t *s_tp_info = NULL;

/*************************
 *   AXS5106 芯片操作函数
 *************************/
/**
 * @brief  AXS5106 芯片复位 
 */
void tp_axs5106_device_reset(void)
{
#if TP_USE_USER_RESET_PIN
    dev_mcu_tp_reset_set(1,1);
    XR_OS_MSleep(1);
    dev_mcu_tp_reset_set(0,1);       
    XR_OS_MSleep(1);               

    uint8_t send_buf[2] = {RESET_REG, 0xff}; // 这步不能去掉, 即使 i2c 提示报错
    tp_board_config_i2c_write(send_buf, 2);
    XR_OS_MSleep(2);
    dev_mcu_tp_reset_set(1,1);        
    XR_OS_MSleep(1);             
#else
    tp_board_config_reset_pin_high();
    XR_OS_MSleep(1);
    tp_board_config_reset_pin_low();
    XR_OS_MSleep(1);

    uint8_t send_buf[2] = {RESET_REG, 0xff}; // 这步不能去掉, 即使 i2c 提示报错
    tp_board_config_i2c_write(send_buf, 2);
    XR_OS_MSleep(2);

    tp_board_config_reset_pin_high();
    XR_OS_MSleep(1);
#endif // #if TP_USE_USER_RESET_PIN
}

/**
 * @brief  检测当前的 TP 芯片是否为 AXS5106 型号
 * @return 0:是, -1:否
 */
int tp_axs5106_device_check(void)
{
    int ret = 0;
    uint8_t read_buf[3] = {0};

    tp_board_config_i2c_read_with_addr(CHIPID_REG, read_buf, 3);
    printf("tp_axs5106_device_check: %02x %02x %02x\n", read_buf[0], read_buf[1], read_buf[2]);
    /**
     * 0x51,0x06,0x08 = AXS5106S
     * 0x51,0x06,0x01 = AXS5106L
     */
    if ((0x51 == read_buf[0]) && (0x06 == read_buf[1]) && ((0x08 == read_buf[2]) || (0x01 == read_buf[2]))) {
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}

/**
 * @brief  获取 AXS5106 芯片固件版本
 * @param  version 存放版本号, 大小为 2 字节
 */
static void tp_axs5106_version_get(uint16_t *version)
{
    uint8_t read_buf[2] = {0};

    tp_board_config_i2c_read_with_addr(VERSION_REG, read_buf, 2);

    *version = ((read_buf[0] << 8) | read_buf[1]);
}

/**
 * @brief  获取 AXS5106 芯片固件项目 id
 * @param  project_id 存放项目 id, 大小为 2 字节
 */
static void tp_axs5106_project_id_get(uint16_t *project_id)
{
    uint8_t read_buf[2] = {0};

    tp_board_config_i2c_read_with_addr(PROJECT_REG, read_buf, 2);

    *project_id = ((read_buf[0] << 8) | read_buf[1]);
}


/**
 * @brief  AXS5106L 是否处于 debug 模式
 * @return true:debug 模式, false:正常模式
 */
static bool _axs5106l_in_debug_mode(void)
{
    uint8_t buf[3] = {READ_FLASH_REG,0x7f, 0xd1};
    uint8_t data = 0;
    int ret = -1;
    ret = tp_board_config_i2c_write(buf, 3);
    if (ret != 0) {
        dev_tp_error("write flash reg fail.");
        return false;
    }

    ret = tp_board_config_i2c_read_with_addr(READ_FLASH_REG, &data, 1);
    if (ret != 0) {
        dev_tp_error("read flash reg fail.");
        return false;
    }

    dev_tp_info("SFR(D1) value: 0x%x.", data);
    return (0x28 == data) ? true : false;
}

/**
 * @brief  升级步骤 1: 进入 debug 模式
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_enter_debug_mode(void)
{
    int ret = 0;
    uint8_t send_buf[2] = {ENTER_DEBUG_REG, 0x55};

    // tp_axs5106_device_reset();
    dev_mcu_tp_reset_set(1,1);
    XR_OS_MSleep(1);
    dev_mcu_tp_reset_set(0,1);       
    XR_OS_MSleep(1);               

    uint8_t reset_buf[2] = {RESET_REG, 0xff}; // 这步不能去掉, 即使 i2c 提示报错
    tp_board_config_i2c_write(reset_buf, 2);
    XR_OS_MSleep(2);
    dev_mcu_tp_reset_set(1,0);        //进入debug模式需要在复位后的500us - 4ms内发送i2c命令,但AT指令阻塞时间较长,因此进debug的复位不用阻塞指令
    XR_OS_MSleep(1);             
    

    ret = tp_board_config_i2c_write(send_buf, 2);
    XR_OS_MSleep(1); // >=50us

    return ret;
}

static int tp_axs5106_device_exit_debug_mode(void)
{
    int ret = 0;
    uint8_t send_buf[2] = {EXIT_DEBUG_REG, 0x5f};

    ret = tp_board_config_i2c_write(send_buf, 2);

    return ret;
}

/**
 * @brief  升级步骤 2: 解锁写保护
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_unlock_mtpc_wp(void)
{
    int ret = 0;
    uint8_t send_buf[4] = {WRITE_FLASH_REG, 0x6f, 0xff, 0xff};

    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xda;
    send_buf[3] = 0x18;
    ret = tp_board_config_i2c_write(send_buf, 4);

exit:
    return ret;
}

/**
 * @brief  升级步骤 3: 清除 TP 芯片的 flash 内容
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_flash_erase(void)
{
    int ret = 0;
    uint8_t send_buf[4] = {WRITE_FLASH_REG, 0x6f, 0xd6, 0x77};

    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }
    XR_OS_MSleep(260); // >=240ms

    send_buf[3] = 0x00;
    ret = tp_board_config_i2c_write(send_buf, 4);

exit:
    return ret;
}

/**
 * @brief  升级步骤 4: 将固件写入 TP 芯片的 flash
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_fw_write(uint8_t *buf, uint32_t buf_len)
{
    int ret = 0;
    uint8_t send_buf[4] = {WRITE_FLASH_REG, 0x6f, 0xd4, 0x00};

    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd5;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd2;
    send_buf[3] = buf_len & 0xff;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd3;
    send_buf[3] = (buf_len >> 8) & 0xff;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd6;
    send_buf[3] = 0xf4;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd7;
    for (uint32_t i = 0; i < buf_len; i++) {
        send_buf[3] = buf[i];
        ret = tp_board_config_i2c_write(send_buf, 4);

        if (0 != ret) {
            ret = ret = tp_board_config_i2c_write(send_buf, 4);
            if (0 != ret) {
                goto exit;
            }
        }
    }

    send_buf[2] = 0xd6;
    send_buf[3] = 0x00;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  AXS5106 芯片固件升级流程
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_upgrade_process(uint8_t *buf, uint32_t buf_len)
{
    int ret = 0;
    
    ret = tp_axs5106_device_enter_debug_mode();
    if (0 != ret) {
        ret = -1;
        dev_tp_error("enter debug mode fail.");
        goto exit;
    }

    if (!_axs5106l_in_debug_mode()) {
        ret = -1;
        dev_tp_error("enter debug mode fail.");
        goto exit;
    }

    ret = tp_axs5106_device_unlock_mtpc_wp();
    if (0 != ret) {
        ret = -1;
        dev_tp_error("unlock mtpc wp fail.");
        goto exit;
    }

    ret = tp_axs5106_device_flash_erase();
    if (0 != ret) {
        ret = -1;
        dev_tp_error("flash erase fail.");
        goto exit;
    }

    ret = tp_axs5106_device_fw_write(buf, buf_len);
    if (0 != ret) {
        ret = -1;
        dev_tp_error("flash write fail.");
        goto exit;
    }

    ret = 0;
exit:
    tp_axs5106_device_exit_debug_mode();
    XR_OS_MSleep(1);
    return ret;
}

/**
 * @brief  AXS5106 芯片固件升级成功后校验
 * @param  buf 存放升级固件的缓冲区
 * @param  buf_len 缓冲区长度
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_upgrade_verify(uint8_t *buf, uint32_t buf_len)
{
    int ret = 0;
    uint8_t send_buf[4] = {WRITE_FLASH_REG, 0x6f, 0xd4, 0x00};
    uint8_t read_buf = 0;

    ret = tp_axs5106_device_enter_debug_mode();
    if (0 != ret) {
        ret = -1;
        dev_tp_error("enter debug mode fail.");
        goto exit;
    }

    if (!_axs5106l_in_debug_mode()) {
        ret = -1;
        dev_tp_error("enter debug mode fail.");
        goto exit;
    }

    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd5;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd2;
    send_buf[3] = buf_len & 0xff;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd3;
    send_buf[3] = (buf_len >> 8) & 0xff;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[2] = 0xd6;
    send_buf[3] = 0xf1;
    ret = tp_board_config_i2c_write(send_buf, 4);
    if (0 != ret) {
        goto exit;
    }

    send_buf[0] = READ_FLASH_REG;
    send_buf[1] = 0x7f;
    send_buf[2] = 0xd7;
    for (uint32_t i = 0; i < buf_len; i++) {
        ret = tp_board_config_i2c_write(send_buf, 3);
        ret = tp_board_config_i2c_read_with_addr(READ_FLASH_REG, &read_buf, 1);

        if (read_buf != buf[i]) {
            ret = -1;
            dev_tp_error("verify fail, fw[%d]: 0x%x, data: 0x%x.", i, buf[i], read_buf);
            goto exit;
        }
    }

    send_buf[0] = WRITE_FLASH_REG;
    send_buf[1] = 0x6f;
    send_buf[2] = 0xd6;
    send_buf[3] = 0x00;
    ret = tp_board_config_i2c_write(send_buf, 4);

    ret = 0;
exit:
    tp_axs5106_device_exit_debug_mode();
    XR_OS_MSleep(1);
    return ret;
}

/**
 * @brief  AXS5106 芯片固件升级 
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_upgrade(uint8_t *buf, uint32_t buf_len)
{
    int ret = 0;

    uint16_t cur_version = 0;
    uint16_t upgrade_version = 0;
    uint16_t cur_project_id = 0;
    uint16_t upgrade_project_id = 0;

    if ((NULL == buf) || (buf_len < 0x403)) {
        ret = -1;
        goto exit;
    }

    upgrade_version = (buf[0x400] << 8) | buf[0x401];
    tp_axs5106_version_get(&cur_version);
    printf("cur_version: 0x%x, upgrade_version: 0x%x.\n", cur_version, upgrade_version);

    upgrade_project_id = (buf[0x402] << 8) | buf[0x403];
    tp_axs5106_project_id_get(&cur_project_id);
    printf("cur_project_id: 0x%x, upgrade_project_id: 0x%x.\n", cur_project_id, upgrade_project_id);

    if ((cur_version == upgrade_version) && (cur_project_id == upgrade_project_id)) {
        ret = 0;
        goto exit;
    }

    cur_version = 0xbeef;
    ret = tp_axs5106_device_upgrade_process(buf, buf_len);

    // tp_axs5106_device_upgrade_verify(buf, buf_len);

    tp_axs5106_device_reset();
    XR_OS_MSleep(10);

    if (0 != ret) {
        ret = -1;
        goto exit;
    }

    tp_axs5106_version_get(&cur_version);
    dev_tp_info("last_version: 0x%x.", cur_version);
    if (cur_version != upgrade_version) {
        ret = -1;
        dev_tp_error("upgrade fail.");
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}

/**
 * @brief  AXS5106 芯片初始化
 * @return 0:成功, -1:失败
 */
static int tp_axs5106_device_init(void)
{
    int ret = 0;

    tp_axs5106_device_reset();
    XR_OS_MSleep(20);

    ret = tp_axs5106_device_check();
    if (0 != ret) {
        ret = -1;
        dev_tp_error("cur tp chip is not AXS5106.");
        goto exit;
    }

    uint16_t version = 0;
    tp_axs5106_version_get(&version);
    printf("tp cur version: 0x%x.", version);

    ret = 0;
exit:
    return ret;
}

/*************************
 *   TP 驱动操作函数
 *************************/
/**
 * @brief  AXS5106 固件升级入口函数
 */
static void tp_axs5106_upgrade(void)
{
    int ret = -1;
    int fd = -1;

    uint8_t file_type = 0; // 0:tf卡, 1:lfs
    char *buf = NULL;
    struct stat file_stat;

    ret = stat(TP_AXS5106_FIRMWARE_FATFS_PATH, &file_stat);
    if (0 != ret) {
        dev_tp_error("can not find %s.", TP_AXS5106_FIRMWARE_FATFS_PATH);
        ret = stat(TP_AXS5106_FIRMWARE_LFS_PATH, &file_stat);
        if (0 != ret) {
            dev_tp_error("can not find %s.", TP_AXS5106_FIRMWARE_LFS_PATH);
            return;
        }
        file_type = 1;
    }

    if (0 == file_stat.st_size) {
        return;
    }

    buf = (char *)calloc(1, file_stat.st_size);
    if (NULL == buf) {
        dev_tp_error("buf malloc fail.");
        return;
    }

    if (0 == file_type) {
        fd = open(TP_AXS5106_FIRMWARE_FATFS_PATH, O_RDONLY);
    } else if (1 == file_type) {
        fd = open(TP_AXS5106_FIRMWARE_LFS_PATH, O_RDONLY);
    }

    if (fd < 0) {
        free(buf);
        return;
    }
    ret = read(fd, buf, file_stat.st_size);

    /* 建议 I2C 速率为 100K */
    ret = tp_axs5106_device_upgrade(buf, file_stat.st_size);
    if (ret < 0) {
        dev_tp_error("firmware update fail.");
    }
    printf("axs5106 firmware update ret: %d\n", ret);

    close(fd);
    free(buf);
}

__nonxip_text static hal_irqreturn_t axs5106_ts_irq_handler(void *dev_id)
{
    if (NULL != s_tp_info) {
        XR_OS_SemaphoreRelease(&s_tp_info->irq_sem);
        return HAL_IRQ_OK;
    }
    return HAL_IRQ_ERR;
}

static int tp_axs5106_i2c_init(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = -1;
        goto exit;
    }

    if (tp_board_config_is_support()) {
        ret = tp_board_config_hw_init();
        if (ret < 0) {
            dev_tp_error("hw init fail.");
            goto exit;
        }
        s_tp_info->irq_num = ret;
        dev_tp_info("tp irq num: %d.", s_tp_info->irq_num);

        ret = hal_gpio_irq_request(s_tp_info->irq_num, axs5106_ts_irq_handler, IRQ_TYPE_EDGE_FALLING, NULL);
        if (ret < 0) {
            dev_tp_error("tp irq request fail.");
        }

        ret = hal_gpio_irq_enable(s_tp_info->irq_num);
        if (ret < 0) {
            dev_tp_error("tp irq enable fail.");
        }
    } else {
        // TODO 代码初始化
    }

    ret = 0;
exit:
    return ret;
}

static int tp_axs5106_i2c_deinit(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = -1;
        goto exit;
    }

    if (tp_board_config_is_support()) {
        hal_gpio_irq_disable(s_tp_info->irq_num);
        hal_gpio_irq_free(s_tp_info->irq_num);
        tp_board_config_hw_deinit(s_tp_info->irq_num);
    } else {
        // TODO 代码反初始化
    }

    ret = 0;
exit:
    return ret;
}

static void tp_axs5106_data_update(void)
{
    uint16_t event = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t value = 0;

    uint8_t read_buf[8] = {0}; // (PonitNum * 6 + 2) Byte

    tp_board_config_i2c_read_with_addr(GESTURE_REG, read_buf, 8);

    /**
     * 接收到的数据格式如下:
     *
     */
    event = read_buf[2] >> 6;
    x = ((read_buf[2] & 0x0f) << 8) + read_buf[3];
    y = ((read_buf[4] & 0x0f) << 8) + read_buf[5];

    dev_tp_info("event: %d, x: %d, y: %d.", event, x, y);

    // if (1 == tp_board_config_tp_revert_mode_get()) {
    //     x = tp_board_config_tp_max_x_get() - x;
    // } else if (2 == tp_board_config_tp_revert_mode_get()) {
    //     y = tp_board_config_tp_max_y_get() - y;
    // }

    // if (1 == tp_board_config_tp_exchange_flag_get()) {
    //     value = x;
    //     x = y;
    //     y = value;
    // }

    input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_POSITION_X, x);
    input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_POSITION_Y, y);
    if (1 == event) {
        input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_PRESSURE, 0); // release
    } else if (2 == event) {
        input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_PRESSURE, 15); // press
    }
    input_sync(s_tp_info->input_dev);
}

static int _axs5106l_enter_sleep(void) 
{
    XR_OS_Status status = XR_OS_OK;
    uint8_t send_buf[2] = {SLEEP_REG, 0x03};
    status = tp_board_config_i2c_write(send_buf, 2);
    return status;
}

int drv_tp_axs5106l_suspend(void)
{
    XR_OS_Status status = -1;
    status = _axs5106l_enter_sleep();
    return status;
}

int drv_tp_axs5106l_resume(void)
{
    tp_axs5106_device_reset();
    return 0;
}

static void tp_axs5106_task(void *args)
{
    XR_OS_Status status = XR_OS_OK;

    if (NULL == s_tp_info) {
        goto exit;
    }

    while (1 == s_tp_info->run_flag) {
        status = XR_OS_SemaphoreWait(&s_tp_info->irq_sem, XR_OS_WAIT_FOREVER);
        if (XR_OS_OK == status) {
            tp_axs5106_data_update();
        }
    }

exit:
    XR_OS_ThreadDelete(&s_tp_info->thread);
}

static int tp_axs5106_task_start(void)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;

    if (NULL == s_tp_info) {
        ret = -1;
        goto exit;
    }

    if (1 == XR_OS_ThreadIsValid(&s_tp_info->thread)) {
        dev_tp_error("tp task is running.");
        ret = 0;
        goto exit;
    }

    s_tp_info->run_flag = 1;
    status = XR_OS_ThreadCreate(&s_tp_info->thread,
                                "tp_axs5106_task",
                                tp_axs5106_task,
                                NULL,
                                XR_OS_PRIORITY_ABOVE_NORMAL,
                                (2 * 1024));
    if (XR_OS_OK != status) {
        s_tp_info->run_flag = 0;
        ret = -1;
        dev_tp_error("tp task create fail: %d.", status);
    } else {
        ret = 0;
    }

exit:
    return ret;
}

static int tp_axs5106_task_stop(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = -1;
        goto exit;
    }

    s_tp_info->run_flag = 0;
    XR_OS_SemaphoreRelease(&s_tp_info->irq_sem);
    while (1 == XR_OS_ThreadIsValid(&s_tp_info->thread)) {
        XR_OS_MSleep(50);
    }

    ret = 0;
exit:
    return ret;
}

int tp_axs5106_init(void)
{
    int ret = 0;
    struct sunxi_input_dev *input_dev;

    s_tp_info = (tp_axs5106_info_t *)malloc(sizeof(tp_axs5106_info_t));
    if (NULL == s_tp_info) {
        dev_tp_error("tp info malloc fail.");
        ret = -1;
        goto exit;
    }
    memset(s_tp_info, 0, sizeof(tp_axs5106_info_t));

    if (XR_OS_OK != XR_OS_SemaphoreCreate(&s_tp_info->irq_sem, 0, 5)) {
        dev_tp_error("tp irq sem create fail.");
        ret = -1;
        goto exit;
    }

    ret = tp_axs5106_i2c_init();
    if (ret < 0) {
        dev_tp_error("i2c init fail.");
        ret = -1;
        goto exit;
    }

    ret = tp_axs5106_device_init();
    // if (ret < 0) {
    //     TP_AXS5106_DEBUG("axs5106 device init fail.");
    //     ret = -1;
    //     goto exit;
    // }

    tp_axs5106_upgrade();

    input_dev = sunxi_input_allocate_device();
    if (NULL == input_dev) {
        dev_tp_error("input dev alloc fail.");
        ret = -1;
        goto exit;
    }

    input_dev->name = TP_DRIVER_NAME;
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_POSITION_X);
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_POSITION_Y);
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_PRESSURE);
    sunxi_input_register_device(input_dev);
    s_tp_info->input_dev = input_dev;

    ret = tp_axs5106_task_start();

    ret = 0;
    return ret;

exit:
    if (NULL != s_tp_info) {
        free(s_tp_info);
        s_tp_info = NULL;
    }
    return ret;
}

int tp_axs5106_deinit(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = 0;
        goto exit;
    }

    tp_axs5106_task_stop();
    tp_axs5106_i2c_deinit();
    XR_OS_SemaphoreDelete(&s_tp_info->irq_sem);

    // TODO 取消该设备的注册
    // TODO 释放该设备分配的资源

    free(s_tp_info);
    s_tp_info = NULL;

    ret = 0;
exit:
    return ret;
}

#ifdef CONFIG_DRIVERS_TEST_TOUCHSCREEN
static XR_OS_Thread_t s_tp_test_thread;
static void tp_test_task(void *parm)
{
    int fd = -1;
    int x = -1;
    int y = -1;

    if (sunxi_input_init() < 0) {
        dev_tp_error("sunxi_input_init fail.");
        goto exit;
    }

    struct sunxi_input_event event;
    memset(&event, 0, sizeof(struct sunxi_input_event));

    fd = sunxi_input_open(TP_DRIVER_NAME);
    if (fd < 0) {
        dev_tp_error("sunxi_input_open fail.");
        goto exit;
    }

    while (1) {
        sunxi_input_read(fd, &event, sizeof(struct sunxi_input_event));
        // TP_AXS5106_DEBUG("read event, type: %d, code: %d, value: %d.", event.type, event.code, event.value);

        if (event.type == INPUT_EVENT_ABS) {
            switch (event.code) {
            case INPUT_ABS_MT_POSITION_X:
                x = event.value;
                break;
            case INPUT_ABS_MT_POSITION_Y:
                y = event.value;
                break;
            }
        }

        if ((x >= 0) && (y >= 0)) {
            TP_AXS5106_DEBUG("==== press point (%d, %d) ====", x, y);
            x = -1;
            y = -1;
        }
    }

exit:
    XR_OS_ThreadDelete(&s_tp_test_thread);
}

static void cmd_axs5106_tp_test(int argc, char **argv)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;

    status = XR_OS_ThreadCreate(&s_tp_test_thread,
                                "tp_test_task",
                                tp_test_task,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                (3 * 1024));

    if (XR_OS_OK != status) {
        dev_tp_error("create tp_test_task fail: %d.", status);
    }
}
FINSH_FUNCTION_EXPORT_CMD(cmd_axs5106_tp_test, tp_axs5106, tp axs5106 test task);

#endif /* CONFIG_DRIVERS_TEST_TOUCHSCREEN */