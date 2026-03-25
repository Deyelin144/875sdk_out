#include <stdlib.h>
#include "mfbd/mfbd.h"
//#include "dev_key.h"
#include "drv_key.h"
#include "drv_key_iface.h"
#include <sunxi_hal_gpadc.h>
#include "../drv_log.h"

#define DRV_VERB 0
#define DRV_DBUG 1
#define DRV_INFO 1
#define DRV_WARN 1
#define DRV_EROR 1

#if DRV_VERB
#define drv_key_adc_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define drv_key_adc_verb(fmt, ...)
#endif

#if DRV_DBUG
#define drv_key_adc_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define drv_key_adc_debug(fmt, ...)
#endif

#if DRV_INFO
#define drv_key_adc_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define drv_key_adc_info(fmt, ...)
#endif

#if DRV_WARN
#define drv_key_adc_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define drv_key_adc_warn(fmt, ...)
#endif

#if DRV_EROR
#define drv_key_adc_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define drv_key_adc_error(fmt, ...)
#endif
typedef struct {
    uint32_t cur_group;  // 当前扫描的组
    uint32_t cur_time;   // 当前按下的时间
    uint32_t key_code;   // 按下的键值，adc值
    uint8_t io_code;     // 按下IO的键值
    irq_callback irq_cb;
} key_code_info_t;

static key_code_info_t sg_key_info = {
    ADC_KEY_GROUP_0,
    0,
    0,
    3,
    NULL,
};

static hal_irqreturn_t _drv_key_gpio_irq_callback(void *args)
{
    return 0;
}

static hal_irqreturn_t _drv_key_adc_irq_callback(uint32_t data_type, uint32_t data)
{
    return 0;
}

static void _drv_key_adc_scan(void)
{

}

static void _drv_key_adc_init(uint8_t key_type, irq_callback cb)
{
    sg_key_info.irq_cb = cb;

    drv_key_iface_adc_init(_drv_key_adc_irq_callback);
    drv_key_iface_gpio_init(_drv_key_gpio_irq_callback);
}

static void _drv_key_adc_deinit(void)
{    
    drv_key_iface_adc_deinit();
    drv_key_iface_gpio_deinit();
    sg_key_info.irq_cb = NULL;
}

void drv_key_adc_ops_register(drv_key_ops_t* ops)
{
    if (NULL != ops) {
        ops->name   = "drv_key_adc";
        ops->init   = _drv_key_adc_init;
        ops->deinit = _drv_key_adc_deinit;
        ops->scan   = _drv_key_adc_scan;
    }
}