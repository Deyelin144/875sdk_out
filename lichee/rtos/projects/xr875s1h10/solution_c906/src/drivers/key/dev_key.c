#include <string.h>
#include "../drv_log.h"
#include "drv_key_iface.h"
#include "dev_key.h"
#include "drv_key.h"
#include "kernel/os/os.h"
#include "compiler.h"


#define DRV_VERB 0
#define DRV_INFO 1
#define DRV_DBUG 1
#define DRV_WARN 1
#define DRV_EROR 1

#if DRV_VERB
#define dev_key_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_key_verb(fmt, ...)
#endif

#if DRV_INFO
#define dev_key_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_key_info(fmt, ...)
#endif

#if DRV_DBUG
#define dev_key_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_key_debug(fmt, ...)
#endif

#if DRV_WARN
#define dev_key_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_key_warn(fmt, ...)
#endif

#if DRV_EROR
#define dev_key_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_key_error(fmt, ...)
#endif

#define SCAN_TASK_STACK_SIZE 2 * 1024
#define SCAN_TIME_MS         5

#define DEV_KEY_OPS_MAP {   \
    drv_key_adc_ops_register,    \
}

// 添加防抖计数器结构
typedef struct {
    uint8_t count;         // 当前计数
    uint8_t last_state;    // 上一次状态
    uint8_t stable_state;  // 稳定状态(0:释放 1:按下)
    uint8_t reported;      // 是否已上报
    uint8_t initialized;   // 初始化标志
} key_debounce_t;

/*
 * 注意:
 * 1. sg_key_mag 和 drv_key_adc_nbtn 链表是一一对应的
 * 2. 若后续需要添加按键, 需要注意放置的位置, 以满足第 1 点要求
 */

 static drv_key_adc_map_t sg_key_map[] = {
    {ADC_KEY_GROUP_0, 0, 0}, // 占位
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_UP,    2990},  // 上,这个值是adc值
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_DOWN,  3540},  // 下
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_LEFT,  2085},  // 左
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_RIGHT, 3330},  // 右
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_AGING, 1620},  // 上键 + 左键
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_TEST,  2386},  // 上键 + 下键
    {ADC_KEY_GROUP_0, ADC_KEY_INDEX_BACK,  1410},  // 返回键
    {ADC_KEY_GROUP_0, GPIO_KEY_INDEX_POWER,  9999},  //OK键
};

//组合键配置
typedef struct {
    uint8_t key_index;      // 组合键在sg_key_map中的索引
    uint32_t adc_min;
    uint32_t adc_max;
} combo_key_cfg_t;

static const combo_key_cfg_t combo_keys[] = {
    {ADC_KEY_INDEX_AGING, 1600, 1700}, // 上+左
    {ADC_KEY_INDEX_TEST,  2300, 2400}, // 上+下
};

typedef struct {
    uint8_t task_running;
    dev_key_config_info_t config_info;
    drv_key_ops_t ops;

    XR_OS_Thread_t scan_task;
    XR_OS_Semaphore_t scan_irq_sem;
} dev_key_t;

typedef void (*drv_key_ops)(drv_key_ops_t* ops);

static dev_key_t *s_dev_key = NULL;

__attribute__((weak)) void sdk_key_event_report(unsigned short btn_id, int state)
{
    (void)btn_id;
    (void)state;
}

/**
 * @brief  adc 按键检测回调
 * @param  arg 回调私有参数传递
 */
static void _dev_key_scan_irq_cb(void *args)
{
    //printf("******************key scan irq cb***********************************\n");
    // if (XR_OS_SemaphoreIsValid(&s_dev_key->scan_irq_sem)) {
    //     XR_OS_SemaphoreRelease(&s_dev_key->scan_irq_sem);
    // }
}

// 辅助函数：获取按键值
static uint32_t dev_key_get_adcbutton_value(uint8_t key_index)
{
    switch (key_index) {
        case ADC_KEY_INDEX_UP:    return 38;
        case ADC_KEY_INDEX_DOWN:  return 40;
        case ADC_KEY_INDEX_LEFT:  return 37;
        case ADC_KEY_INDEX_RIGHT: return 39;
        case ADC_KEY_INDEX_AGING: return 113;
        case ADC_KEY_INDEX_TEST:  return 112;
        case ADC_KEY_INDEX_BACK:  return 27;
        default: return 0;
    }
}

extern uint32_t hal_ococci_read_adc(hal_gpadc_channel_t channel, uint32_t cycle);
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
extern void recovery_keyboard_handler(uint8_t key_state, uint32_t key_data);
#endif

//检查是否为组合键
static int8_t detect_combo_key(uint32_t adc_value) {
    for (int i = 0; i < sizeof(combo_keys)/sizeof(combo_keys[0]); i++) {
        if (adc_value >= combo_keys[i].adc_min && adc_value <= combo_keys[i].adc_max)
            return combo_keys[i].key_index;
    }
    return -1;
}

//主扫描任务
__nonxip_text void _dev_key_scan_task(void *arg)
{
    uint32_t adc_value;
    key_debounce_t debounce[sizeof(sg_key_map)/sizeof(sg_key_map[0])] = {0};
    int current_pressed_key = -1;
    int is_combo_key = 0;
    int first_scan = 1; // 标记首次扫描
    uint8_t all_keys_released;
    uint8_t power_key_state;
    key_debounce_t *power_deb;
    int8_t combo_key;
    int8_t closest_key;

    while (1) {
#ifndef CONFIG_PROJECT_BUILD_RECOVERY
        extern unsigned char native_upgrade_is_ota_running(void);
        if (native_upgrade_is_ota_running()) {
            XR_OS_MSleep(1000);
            continue;;
        }
#endif
        adc_value = hal_ococci_read_adc(ADC_KEY_CHANNEL, DRV_ADC_READ_CYCLE_COUNT);
        all_keys_released = (adc_value >= DRV_ADC_RELEASE_VALUE);
        // 首次启动时，不上报任何按键事件，只做初始化
        if (first_scan) {
            // 初始化所有防抖结构体
            for (int i = 0; i < sizeof(debounce)/sizeof(debounce[0]); i++) {
                debounce[i].count = 0;
                debounce[i].last_state = 0;
                debounce[i].stable_state = 0;
                debounce[i].reported = 1;
                debounce[i].initialized = 1;
            }
            current_pressed_key = -1;
            is_combo_key = 0;
            // 检查是否所有按键都已释放，如果是则结束初始化
            // 初始化前就按下厂测按键，会导致无法进入厂测，屏蔽if条件判断
            // if (all_keys_released) {
                first_scan = 0;
            // }
            XR_OS_MSleep(KEY_DEBOUNCE_DELAY);
            continue;
        }
        
        // 检测GPIO按键(POWER键)，按下为1.释放为0
        power_key_state = (drv_key_iface_gpio_read(KEY_OK) == 0) ? 1 : 0;
        power_deb = &debounce[GPIO_KEY_INDEX_POWER];
        
        // POWER键防抖处理
        if (!power_deb->initialized) {// 初始化时不上报状态
            power_deb->last_state = power_key_state;
            power_deb->stable_state = power_key_state;
            power_deb->initialized = 1;
            power_deb->reported = 1;    // 标记为已上报，防止初始化时触发释放事件
        } else if (power_key_state == power_deb->last_state) {
            // 按键状态稳定处理
            if (power_deb->count < KEY_DEBOUNCE_TIME) {
                power_deb->count++;
                // 达到防抖时间后触发事件
                if (power_deb->count >= KEY_DEBOUNCE_TIME) {
                    // 状态稳定（之前的稳定状态是按下，现在变成弹起，或者相反），或当前状态还未上报过
                    if (power_deb->stable_state != power_key_state || !power_deb->reported) {
                        power_deb->stable_state = power_key_state;
                        power_deb->reported = 1;
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
                        recovery_keyboard_handler(power_key_state, 36);
#else
                        keyboard_handler(power_key_state, 36);
                        sdk_key_event_report(36, power_key_state);
#endif
                        dev_key_info("POWER key %s\n", power_key_state ? "pressed" : "released");
                    }
                }
            }
        } else {
            // 状态变化，重置防抖
            power_deb->count = 0;
            power_deb->last_state = power_key_state;
            power_deb->reported = 0;
        }

        // 1. 释放逻辑
        if (all_keys_released) {
            if (current_pressed_key != -1) {
                key_debounce_t *prev = &debounce[current_pressed_key];
                if (prev->stable_state) {
                    prev->stable_state = 0;
                    prev->reported = 1;
                    prev->count = 0;
                    uint32_t btn_value = dev_key_get_adcbutton_value(sg_key_map[current_pressed_key].index);
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
                    recovery_keyboard_handler(0, btn_value);
#else
                    keyboard_handler(0, btn_value);
                    sdk_key_event_report(btn_value, 0);
#endif
                    dev_key_info("%s key %d released, adc=%d\n",
                        is_combo_key ? "Combo" : "Single",
                        sg_key_map[current_pressed_key].index, adc_value);
                    current_pressed_key = -1;
                    is_combo_key = 0;
                }
            }
            XR_OS_MSleep(KEY_DEBOUNCE_DELAY);
            continue;
        }

        // 2. 检查组合键
        combo_key = detect_combo_key(adc_value);
        if (combo_key != -1) {
            key_debounce_t *deb = &debounce[combo_key];
            if (current_pressed_key == -1) {
                deb->count++;
                if (deb->count >= KEY_DEBOUNCE_TIME) {
                    deb->stable_state = 1;
                    deb->reported = 1;
                    uint32_t btn_value = dev_key_get_adcbutton_value(sg_key_map[combo_key].index);
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
                    recovery_keyboard_handler(1, btn_value);
#else
                    keyboard_handler(1, btn_value);
                    sdk_key_event_report(btn_value, 1);
#endif
                    dev_key_info("Combo key %d pressed, adc=%d\n", sg_key_map[combo_key].index, adc_value);
                    current_pressed_key = combo_key;
                    is_combo_key = 1;
                }
            }
            XR_OS_MSleep(KEY_DEBOUNCE_DELAY);
            continue;
        }

        // 3. 检查单键
        closest_key = -1;
        for (int i = 1; i < sizeof(sg_key_map)/sizeof(sg_key_map[0]) - 1; i++) {
            uint32_t diff = abs(adc_value - sg_key_map[i].adc);
            if (diff <= DRV_ADC_VAL_RANGE) {
                closest_key = i;
                break;
            }
        }
        if (closest_key != -1) {
            key_debounce_t *deb = &debounce[closest_key];
            if (current_pressed_key == -1) {
                deb->count++;
                if (deb->count >= KEY_DEBOUNCE_TIME) {
                    deb->stable_state = 1;
                    deb->reported = 1;
                    uint32_t btn_value = dev_key_get_adcbutton_value(sg_key_map[closest_key].index);
#ifdef CONFIG_PROJECT_BUILD_RECOVERY
                    recovery_keyboard_handler(1, btn_value);
#else
                    keyboard_handler(1, btn_value);
                    sdk_key_event_report(btn_value, 1);
#endif
                    dev_key_info("Single key %d pressed, adc=%d\n", sg_key_map[closest_key].index, adc_value);
                    current_pressed_key = closest_key;
                    is_combo_key = 0;
                }
            }
        }
        XR_OS_MSleep(KEY_DEBOUNCE_DELAY);
    }
}

int dev_key_init(dev_key_config_info_t * info)
{
    int ret = -1;
    XR_OS_Status status = XR_OS_OK;
    drv_key_ops key_ops_map[] = DEV_KEY_OPS_MAP;

    if (NULL != s_dev_key) {
        dev_key_error("dev key has been init!");
        ret = 0;
        goto exit;
    }

    s_dev_key = (dev_key_t *)malloc(sizeof(dev_key_t));
    if (NULL == s_dev_key) {
        dev_key_error("malloc fail.");
        goto exit;
    }
    memset(s_dev_key, 0, sizeof(dev_key_t));
    memcpy(&s_dev_key->config_info, info, sizeof(dev_key_config_info_t));
//    s_dev_key->config_info.type = 0;

    key_ops_map[s_dev_key->config_info.type](&s_dev_key->ops);

    // status = XR_OS_SemaphoreCreate(&s_dev_key->scan_irq_sem, 0, 5);
    // if (status != XR_OS_OK) {
    //     dev_key_error("key_scan_sem create fail: %d.", status);
    //     goto exit;
    // }

    s_dev_key->ops.init(s_dev_key->config_info.type, _dev_key_scan_irq_cb);

    s_dev_key->task_running = 1;
    status = XR_OS_ThreadCreate(&s_dev_key->scan_task,
                             "_dev_key_scan_task",
                             _dev_key_scan_task,
                             NULL,
                             XR_OS_PRIORITY_HIGH,
                             SCAN_TASK_STACK_SIZE);
    if (status != XR_OS_OK) {
        dev_key_error("key_scan_task create fail: %d.", status);
    }

    ret = 0;
exit:
    if (0 != ret) {
        free(s_dev_key);
        s_dev_key = NULL;
    }
    return ret;
}

void dev_key_deinit(void)
{
    s_dev_key->task_running = 0;

    while (XR_OS_ThreadIsValid(&s_dev_key->scan_task)) {
        XR_OS_MSleep(100);
    }

    if (NULL != s_dev_key) {
        s_dev_key->ops.deinit();
    }
    free(s_dev_key);
}

void dev_key_suspend(void)
{
    // TODO
    return;
}

void dev_key_resume(void)
{
    // TODO
    return;
}
