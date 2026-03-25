#include "compiler.h"
#include "kernel/os/os.h"
#include "../drv_log.h"
#include "dev_extio.h"

#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if DBUG
#define dev_extio_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_extio_debug(fmt, ...)
#endif

#if INFO
#define dev_extio_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_extio_info(fmt, ...)
#endif

#if WARN
#define dev_extio_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_extio_warn(fmt, ...)
#endif

#if EROR
#define dev_extio_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_extio_error(fmt, ...)
#endif

extern void drv_extio_aw9523b_ops_register(drv_extio_ops_t *ops);
extern void drv_extio_htr3316_ops_register(drv_extio_ops_t *ops);

#define EXTIO_OPS_REGISTER_MAP          \
    {                                   \
        drv_extio_aw9523b_ops_register, \
        drv_extio_htr3316_ops_register, \
    }

typedef void (*drv_extio_ops_arrays_t)(drv_extio_ops_t *ops);

typedef struct {
    drv_extio_ops_t ops;
    XR_OS_Thread_t thread;
    XR_OS_Semaphore_t sem;
    uint8_t stop_thread;
} dev_extio_t;

static dev_extio_t s_dev_extio;

__nonxip_text static void dev_exito_sem_release(void *args)
{
    if (XR_OS_SemaphoreIsValid(&s_dev_extio.sem)) {
        XR_OS_SemaphoreRelease(&s_dev_extio.sem);
    }
}

static int dev_extio_sem_wait(void)
{
    XR_OS_Status status = XR_OS_OK;
    int ret = -1;

    if (0 == XR_OS_SemaphoreIsValid(&s_dev_extio.sem)) {
        goto exit;
    }

    status = XR_OS_SemaphoreWait(&s_dev_extio.sem, XR_OS_WAIT_FOREVER);
    if (XR_OS_OK == status) {
        ret = 0;
    }

exit:
    return ret;
}

extern void dev_sd_irq_handler(void *arg);

static void dev_extio_thread(void *arg)
{
    uint16_t irq_flag = 0;

    while (!s_dev_extio.stop_thread) {
        if (0 == dev_extio_sem_wait()) {
            if (NULL == s_dev_extio.ops.irq_flag_get) {
                continue;
            }
#ifndef CONFIG_PROJECT_BUILD_RECOVERY
#endif
            dev_sd_irq_handler(NULL);
            irq_flag = 0;
            s_dev_extio.ops.irq_flag_get(&irq_flag);
            dev_extio_debug("extio irq flag: 0x%x.", irq_flag);
        }
    }

    XR_OS_ThreadDelete(&s_dev_extio.thread);
}

int dev_extio_lcd_reset(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    XR_OS_MSleep(100);

    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);
    XR_OS_MSleep(100);

    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    XR_OS_MSleep(120);

    return 0;
}

int dev_extio_lcd_reset_enable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);
    XR_OS_MSleep(20);

    return 0;
}

int dev_extio_lcd_reset_disable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    XR_OS_MSleep(20);

    return 0;
}

int dev_extio_lcd_reset_control(int val)
{
    if (val == 0) {
        s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);
    } else if (val = 1) {
        s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    }
    return 0;
}

int dev_extio_cam_pwdn_ctl(int cmd)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_CAM_PWDN, cmd ? DEV_EXTIO_GPIO_LEVEL_HIGH : DEV_EXTIO_GPIO_LEVEL_LOW);
    printf("cam pwdn %s\n", cmd ? "disable" : "enable");
    return 0;
}


int dev_extio_lcd_bl_lp_enable(void)
{
    // s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_BL_LP, DEV_EXTIO_GPIO_LEVEL_HIGH);
    return 0;
}

int dev_extio_lcd_bl_lp_disable(void)
{
    // s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_BL_LP, DEV_EXTIO_GPIO_LEVEL_LOW);
    return 0;
}

int dev_extio_led_en_enable(void)
{
    return 0;
}

int dev_extio_led_en_disable(void)
{
    return 0;
}

int dev_extio_radar_disable(void)
{
    return 0;
}

int dev_extio_radar_enable(void)
{
    return 0;
}

int dev_extvcc_enable(void)
{
    // s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_EXTVCC_EN, DEV_EXTIO_GPIO_LEVEL_HIGH);
    return 0;
}

int dev_extvcc_disable(void)
{
    // s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_EXTVCC_EN, DEV_EXTIO_GPIO_LEVEL_LOW);
    return 0;
}

int dev_extio_lcd_en_enable(void)
{
    return 0;
}

int dev_extio_lcd_en_disable(void)
{
    return 0;
}

int dev_extio_pa_shdn_enable(void)
{
    return 0;
}

int dev_extio_pa_shdn_disable(void)
{
    return 0;
}

int dev_extio_lcd_rst_enable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);

    return 0;
}

int dev_extio_lcd_rst_disable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_LCD_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);

    return 0;
}

int dev_extio_tp_rst_enable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);

    return 0;
}

int dev_extio_tp_rst_disable(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);

    return 0;
}

int dev_extio_tp_reset(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    XR_OS_MSleep(50);

    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);
    XR_OS_MSleep(50);

    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_HIGH);
    XR_OS_MSleep(50);

    return 0;
}

int dev_extio_tp_suspend(void)
{
    s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_TP_RESET, DEV_EXTIO_GPIO_LEVEL_LOW);
    return 0;
}

uint8_t dev_extio_full_det_get(void)/*充满检测引脚，充满为低电平*/
{
    return (DEV_EXTIO_GPIO_LEVEL_HIGH == s_dev_extio.ops.gpio_get(DEV_EXTIO_PIN_FULL_DET)) ? 0 : 1;
}

int dev_extio_get_mic_state(void)
{
    return 0;
}

int dev_extio_get_charge_state(void)
{
    return (DEV_EXTIO_GPIO_LEVEL_LOW == s_dev_extio.ops.gpio_get(DEV_EXTIO_PIN_CHARGE_DET)) ? 0 : 1;
}

int dev_extio_get_sd_state(void)
{
    return (DEV_EXTIO_GPIO_LEVEL_LOW == s_dev_extio.ops.gpio_get(DEV_EXTIO_PIN_SD_DET)) ? 1 : 0;
}

int dev_extio_pin_sate_get(dev_exito_pin_t pin)
{
    if (DEV_EXTIO_PIN_NUM == pin) {
        dev_extio_error("invalid pin.");
        return -1;
    }

    return (DEV_EXTIO_GPIO_LEVEL_LOW == s_dev_extio.ops.gpio_get(pin)) ? 0 : 1;
}

int dev_extio_set_pin_state(dev_exito_pin_t pin, dev_extio_gpio_level_t level)
{
    int ret = -1;
    if (DEV_EXTIO_PIN_NUM == pin) {
        dev_extio_error("invalid pin.");
        goto exit;
    }

    ret = s_dev_extio.ops.gpio_set(pin, level);
exit:
    return ret;
}

void dev_extio_init_radar1_pin(dev_extio_gpio_direction_t dir, bool interrupt_flag, dev_extio_gpio_level_t level)//0:output, 1:input
{
    s_dev_extio.ops.gpio_init(DEV_EXTIO_PIN_RADAR1, dir, interrupt_flag);
    if (dir == DEV_EXTIO_GPIO_DIR_OUTPUT) {
        s_dev_extio.ops.gpio_set(DEV_EXTIO_PIN_RADAR1, level);
    }
}

int dev_extio_get_irq(void)
{
    int irq_flag = 0;
    s_dev_extio.ops.irq_flag_get(&irq_flag);
    return irq_flag;
}

int dev_extio_suspend(void)
{
    // s_dev_extio.ops.suspend();
    // dev_extio_clear_irq();
}

int dev_extio_resume(void)
{
    // s_dev_extio.ops.resume();
    // dev_extio_clear_irq();
}

int dev_extio_init(void)
{
    static int inited = 0;
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;
    drv_extio_ops_arrays_t extio_ops_map[] = EXTIO_OPS_REGISTER_MAP;

    if (inited) {
        dev_extio_info("extio initd");
        return 0;
    }

    extio_ops_map[0](&s_dev_extio.ops);

    status = XR_OS_SemaphoreCreate(&s_dev_extio.sem, 0, 1);
    if (status != XR_OS_OK) {
        dev_extio_error("sem create fail: %d.", status);
    }

    ret = s_dev_extio.ops.init(dev_exito_sem_release);
    if (0 != ret) {
        dev_extio_error("extio device init fail: %d.", ret);
        goto exit;
    }
    s_dev_extio.stop_thread = 0;
    
    status = XR_OS_ThreadCreate(&s_dev_extio.thread,
                                "dev_extio_thread",
                                dev_extio_thread,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                4096);
    if (status != XR_OS_OK) {
        dev_extio_error("key_scan_task create fail: %d.", status);
    }

    ret = 0;
    inited = 1;

exit:
    return ret;
}

void dev_extio_deinit(void)
{
    s_dev_extio.ops.deinit();
    if (XR_OS_ThreadIsValid(&s_dev_extio.thread) && 0 == s_dev_extio.stop_thread) {
        s_dev_extio.stop_thread = 1;
        while (XR_OS_ThreadIsValid(&s_dev_extio.thread)) {
            XR_OS_MSleep(50);
        }
    }
    
    if (XR_OS_SemaphoreIsValid(&s_dev_extio.sem)) {
        XR_OS_SemaphoreDelete(&s_dev_extio.sem); 
    }
}

extern uint16_t aw9523b_port_interrupt_clear(void);
void dev_extio_clear_irq(void)
{
    // aw9523b_port_interrupt_clear();
}
