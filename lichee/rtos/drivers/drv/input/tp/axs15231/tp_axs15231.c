#include <console.h>
#include <hal_gpio.h>
#include "sunxi-input.h"
#include <sunxi_hal_twi.h>
#include <sunxi-input.h>
#include "compiler.h"
#include "kernel/os/os.h"
#include "../config/tp_board_config.h"

#define TP_AXS15231_DEBUG_ENABLE 1
#if TP_AXS15231_DEBUG_ENABLE
#define TP_AXS15231_DEBUG(fmt, ...) printf("[axs15231 %4d] " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define TP_AXS15231_DEBUG(fmt, ...)
#endif

// #define TP_AXS15231_I2C_ADDR  0x3b
// #define TP_AXS15231_I2C_IDX   TWI_MASTER_1
// #define TP_AXS15231_RESET_PIN GPIOA(8)
// #define TP_AXS15231_IRQ_PIN   GPIOA(7)
#define _WR_LEN 11
#define _RD_LEN 12

typedef struct {
    uint8_t run_flag;
    uint32_t irq_num;
    XR_OS_Semaphore_t irq_sem;
    XR_OS_Thread_t thread;
    struct sunxi_input_dev *input_dev;
    uint8_t write[_WR_LEN];
    uint8_t read[_RD_LEN];
} tp_axs15231_info_t;

static tp_axs15231_info_t *s_tp_info = NULL;

__nonxip_text static hal_irqreturn_t axs15231_ts_irq_handler(void *dev_id)
{
    if (NULL != s_tp_info) {
        XR_OS_SemaphoreRelease(&s_tp_info->irq_sem);
        return HAL_IRQ_OK;
    }
    return HAL_IRQ_ERR;
}

static int tp_axs15231_i2c_init(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = -1;
        goto exit;
    }

    if (tp_board_config_is_support()) {
        ret = tp_board_config_hw_init();
        if (ret < 0) {
            TP_AXS15231_DEBUG("hw init fail.");
            goto exit;
        }
        s_tp_info->irq_num = ret;
        TP_AXS15231_DEBUG("tp irq num: %d.", s_tp_info->irq_num);

        ret = hal_gpio_irq_request(s_tp_info->irq_num, axs15231_ts_irq_handler, IRQ_TYPE_EDGE_FALLING, NULL);
        if (ret < 0) {
            TP_AXS15231_DEBUG("tp irq request fail.");
        }

        ret = hal_gpio_irq_enable(s_tp_info->irq_num);
        if (ret < 0) {
            TP_AXS15231_DEBUG("tp irq enable fail.");
        }
    } else {
        // TODO 代码初始化
    }

    ret = 0;
exit:
    return ret;
}

static int tp_axs15231_i2c_deinit(void)
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


static void tp_axs15231_data_update()
{
    uint8_t point_number = 0;
    uint8_t event = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t offset = 0;

    tp_board_config_i2c_write(s_tp_info->write, _WR_LEN);
    tp_board_config_i2c_read(s_tp_info->read, _RD_LEN);

    /**
     * 接收数据的缓冲区大小 = (point_number * 6) + 3
     * 
     * 接收到的数据格式如下:
     * buf[0]  - gesture
     * buf[1]  - point num
     * 
     * buf[2]  - event + x 坐标高位
     * buf[3]  - x 坐标低位
     * buf[4]  - id + y 坐标高位
     * buf[5]  - y 坐标地位
     * buf[6]  - weight
     * buf[7]  - area
     * 
     * buf[8]  - event + x 坐标高位
     * buf[9]  - x 坐标低位
     * buf[10] - id + y 坐标高位
     * buf[11] - y 坐标地位
     * buf[12] - weight
     * buf[13] - area
     */

    point_number = s_tp_info->read[1] & 0x0f;
    // printf("ptn= %d\n", point_number);
    if (point_number > 2) {
        TP_AXS15231_DEBUG("invalid point number: %d.", point_number);
        return;
    }

    for (uint8_t i = 0; i < point_number; i++) {
        uint16_t offset = i * 6;
        // printf("ost=%d\n", offset);
        event = s_tp_info->read[2 + offset] >> 6;
        // /*触摸适配横屏，进行坐标偏移*/
        // y = 320 - (((s_tp_info->read[2 + offset] & 0x0f) << 8) + s_tp_info->read[3 + offset]);
        // x = ((s_tp_info->read[4 + offset] & 0x0f) << 8) + s_tp_info->read[5 + offset];
        /*厂家已经帮忙确定固件原点，无需坐标偏移*，但是因为竖屏变为横屏，xy坐标需要交换*/
        y = ((s_tp_info->read[2 + offset] & 0x0f) << 8) + s_tp_info->read[3 + offset];
        x = ((s_tp_info->read[4 + offset] & 0x0f) << 8) + s_tp_info->read[5 + offset];

        // x = ((s_tp_info->read[2 + offset] & 0x0f) << 8) + s_tp_info->read[3 + offset];
        // y = ((s_tp_info->read[4 + offset] & 0x0f) << 8) + s_tp_info->read[5 + offset];

        if (1 == tp_board_config_tp_revert_mode_get()) {
            x = tp_board_config_tp_max_x_get() - x;
        } else if (2 == tp_board_config_tp_revert_mode_get()) {
            y = tp_board_config_tp_max_y_get() - y;
        }
        if (1 == tp_board_config_tp_exchange_flag_get()) {
            x = x + y;
            y = x - y;
            x = x - y;
        }

        input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_POSITION_X, x);
        input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_POSITION_Y, y);
        if (1 == event) {
            // printf("XY=%d,%d\n", x, y);
            input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_PRESSURE, 0); // release
        } else if (2 == event) {
            // printf("xy=%d,%d\n", x, y);
            input_report_abs(s_tp_info->input_dev, INPUT_ABS_MT_PRESSURE, 15); // press
        }
        input_sync(s_tp_info->input_dev);
    }
}

static void tp_axs15231_task(void *args)
{
    XR_OS_Status status = XR_OS_OK;

    while (1 == s_tp_info->run_flag) {
        status = XR_OS_SemaphoreWait(&s_tp_info->irq_sem, XR_OS_WAIT_FOREVER);
        if (XR_OS_OK == status) {
            tp_axs15231_data_update();
        }
    }

    XR_OS_ThreadDelete(&s_tp_info->thread);
}

static int tp_axs15231_task_start(void)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;

    if (NULL == s_tp_info) {
        return -1;
    }

    if (1 == XR_OS_ThreadIsValid(&s_tp_info->thread)) {
        TP_AXS15231_DEBUG("tp task is running.");
        ret = 0;
        goto exit;
    }

    s_tp_info->run_flag = 1;
    status = XR_OS_ThreadCreate(&s_tp_info->thread,
                                "axs15231 task",
                                tp_axs15231_task,
                                NULL,
                                XR_OS_PRIORITY_ABOVE_NORMAL,
                                (2 * 1024));
    if (XR_OS_OK != status) {
        s_tp_info->run_flag = 0;
        ret = -1;
        TP_AXS15231_DEBUG("tp task create fail: %d.", status);
    } else {
        ret = 0;
    }

exit:
    return ret;
}

static int tp_axs15231_task_stop(void)
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

int tp_axs15231_hang_up(void)
{
    int ret = -1;
    uint8_t send_buf[13] = {0xb5, 0xab, 0x5a, 0xa5, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xc0};

    /**
     * 对于挂起的时间, 由 send_buf[12] 决定
     * 1. 0x10: 93ms
     * 2. 0x20: 300ms
     * 3. 0x90: 800ms
     * 4. 0xb0: 900ms
     */
    ret = tp_board_config_i2c_write(send_buf, 13);
    if (ret < 0) {
        TP_AXS15231_DEBUG("tp hang up fail.");
    }

    return ret;
}


/*************************
 *   AXS15231 芯片操作函数
 *************************/
/**
 * @brief  带有 flash 的 AXS15231 版本获取
 */
static int tp_axs15231_device_version_get(uint8_t *version)
{
    int ret = 0;
    uint8_t cur_version = 0;

    uint8_t try_cnt = 3;
    uint8_t read_version_cmd[11] = {0x5a, 0xa5, 0xab, 0xb5, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x89};

    while (try_cnt--) {
        ret = tp_board_config_i2c_write(read_version_cmd, 11);
        ret = tp_board_config_i2c_read(&cur_version, 1);

        if ((ret == 0) && (0 != cur_version)) {
            break;
        }
        XR_OS_MSleep(10);
    }

    if (ret < 0) {
        printf("get cur_version fail.");
        ret = -1;
        goto exit;
    }

    printf("[axs15231] cur_version: 0x%.2x.\n", cur_version);
    if (NULL != version) {
        *version = cur_version;
    }
exit:
    return ret;
}


int tp_axs15231_init(void)
{
    int ret = 0;
    struct sunxi_input_dev *input_dev;

    s_tp_info = (tp_axs15231_info_t *)malloc(sizeof(tp_axs15231_info_t));
    if (NULL == s_tp_info) {
        TP_AXS15231_DEBUG("tp info malloc fail.");
        return -1;
    }
    memset(s_tp_info, 0, sizeof(tp_axs15231_info_t));

    uint8_t tmp_buf[_WR_LEN] = {0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00};
    memcpy(s_tp_info->write, tmp_buf, _WR_LEN);

    if (XR_OS_OK != XR_OS_SemaphoreCreate(&s_tp_info->irq_sem, 0, 5)) {
        TP_AXS15231_DEBUG("tp irq sem create fail.");
        ret = -1;
        goto exit;
    }

    ret = tp_axs15231_i2c_init();
    if (ret < 0) {
        TP_AXS15231_DEBUG("i2c init fail.");
        ret = -1;
        goto exit;
    }

    tp_axs15231_device_version_get(NULL);

    input_dev = sunxi_input_allocate_device();
    if (NULL == input_dev) {
        TP_AXS15231_DEBUG("input dev alloc fail.");
        ret = -1;
        goto exit;
    }

    input_dev->name = TP_DRIVER_NAME;
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_POSITION_X);
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_POSITION_Y);
    input_set_capability(input_dev, INPUT_EVENT_ABS, INPUT_ABS_MT_PRESSURE);
    sunxi_input_register_device(input_dev);
    s_tp_info->input_dev = input_dev;

    ret = tp_axs15231_task_start();

    ret = 0;
    return ret;

exit:
    if (NULL != s_tp_info) {
        free(s_tp_info);
        s_tp_info = NULL;
    }
    return ret;
}

int tp_axs15231_deinit(void)
{
    int ret = -1;

    if (NULL == s_tp_info) {
        ret = 0;
        goto exit;
    }

    tp_axs15231_task_stop();
    tp_axs15231_i2c_deinit();
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
static bool s_tp_test_run = false;
static void tp_test_task(void *parm)
{
    int fd = -1;
    int x = -1;
    int y = -1;
    struct sunxi_input_event event;
    memset(&event, 0, sizeof(struct sunxi_input_event));

    fd = sunxi_input_open(TP_DRIVER_NAME);
    if (fd < 0) {
        TP_AXS15231_DEBUG("sunxi input open fail.");
        goto exit;
    }

    s_tp_test_run = true;
    while (s_tp_test_run) {
        sunxi_input_read(fd, &event, sizeof(struct sunxi_input_event));
        // TP_AXS15231_DEBUG("type: %d, code: %d, value: %d.", event.type, event.code, event.value);
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
            TP_AXS15231_DEBUG("==== x=%d\ty=%d ====", x, y);
            x = -1;
            y = -1;
        }
    }
    TP_AXS15231_DEBUG("exit.");
exit:
    vTaskDelete(NULL);
}

static void test_tp_axs15231_cmd(int argc, char **argv)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;
    XR_OS_Thread_t s_tp_test_thread = {NULL};

    if (2 != argc) {
        printf("usage: %s <on|off>.\n", argv[0]);
        return;
    }

    if (0 == strcmp(argv[1], "on")) {
        status = XR_OS_ThreadCreate(&s_tp_test_thread,
                                    "axs15231 test",
                                    tp_test_task,
                                    NULL,
                                    XR_OS_PRIORITY_NORMAL,
                                    (2 * 1024));

        if (XR_OS_OK != status) {
            TP_AXS15231_DEBUG("create axs15231 test fail: %d.", status);
        }
    } else if (0 == strcmp(argv[1], "off")) {
        s_tp_test_run = false;
    }
}
FINSH_FUNCTION_EXPORT_CMD(test_tp_axs15231_cmd, test_tp_axs15231, tp axs15231 test cmd);
#endif /* CONFIG_DRIVERS_TEST_TOUCHSCREEN */
