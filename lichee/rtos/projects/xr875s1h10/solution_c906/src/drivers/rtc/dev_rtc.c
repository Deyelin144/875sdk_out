#include "dev_rtc.h"
#include "kernel/os/os.h"
#include "../drv_log.h"
#include <hal_cmd.h>

#define VERB 0
#define DBUG 1
#define INFO 1
#define WARN 1
#define EROR 1

#if VERB
#define dev_rtc_verb(fmt, ...)  drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_rtc_verb(fmt, ...)
#endif

#if DBUG
#define dev_rtc_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_rtc_debug(fmt, ...)
#endif

#if INFO
#define dev_rtc_info(fmt, ...)  drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_rtc_info(fmt, ...)
#endif

#if WARN
#define dev_rtc_warn(fmt, ...)  drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_rtc__warn(fmt, ...)
#endif

#if EROR
#define dev_rtc_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_rtc_error(fmt, ...)
#endif

#define DEV_RTC_OPS_MAP { \
	drv_rtc_at8563_get_ops, \
}

#define DEV_RTC_DETECT_FUNC_MAP {\
	drv_rtc_at8563_chip_check,\
}

typedef drv_status_t (*rtc_chip_check)(void);

typedef struct {
    uint8_t task_runing;
    XR_OS_Thread_t tid;
	XR_OS_Semaphore_t sem;
    irq_callback irq_cb;       // 上层回调中断接口
    drv_rtc_type_t chip_type;  // 当前芯片型号
    drv_rtc_ops_t* ops;        // 模块操作接口集
} dev_rtc_t;

/*
    1. R128 和 mcu 都未接外部晶振,导致RTC不准
    2. 缺少引脚,外部RTC的中断线硬件上断开的,无法使用外部RTC的闹钟功能
    
    因此,方案同时采用内部RTC和外部RTC
    内部RTC用于闹钟和获取时间
    外部RTC用于校准 内部RTC
    每次休眠唤醒,从外部RTC获取时间,并校准内部RTC
*/
static dev_rtc_t *s_dev_rtc = NULL;
static drv_rtc_type_t s_rtc_chip = DRV_RTC_TYPE_MAX;

static dev_rtc_t *s_dev_cali_rtc = NULL;

drv_rtc_type_t get_rtc_chip_type(void)
{
    printf("rtc chip type %d\n", s_rtc_chip);
    return s_rtc_chip;
}

static drv_rtc_type_t _rtc_chip_detect()
{
    int i;
	rtc_chip_check rtc_detect_func_map[] = DEV_RTC_DETECT_FUNC_MAP;
    for(i = 0; i < sizeof(rtc_detect_func_map)/sizeof(rtc_chip_check); i++) {
		if (0 == rtc_detect_func_map[i]()) {
			printf("rtc i2c det %d\n", i);
			return i;
		}
	}
    return DRV_RTC_TYPE_MAX;
}

int dev_rtc_irq_sem_relase(void)
{
    if (XR_OS_SemaphoreIsValid(&s_dev_rtc->sem)) {
        XR_OS_SemaphoreRelease(&s_dev_rtc->sem);
    }
    return 0;
}

static void _dev_inner_rtc_task(void *args)
{
	s_dev_rtc->task_runing = 1;

    while (s_dev_rtc->task_runing) {
        if (XR_OS_OK != XR_OS_SemaphoreWait(&(s_dev_rtc->sem), XR_OS_WAIT_FOREVER)) {
            continue;
        }

        if (s_dev_rtc->irq_cb) {
            s_dev_rtc->irq_cb(NULL);
        }
    }

    XR_OS_ThreadDelete(&(s_dev_rtc->tid));
}

static void _dev_rtc_task(void *args)
{
	s_dev_rtc->task_runing = 1;
    drv_rtc_time_fmt_t current_time = {0};
    struct tm t = {0};
    time_t clock = {0};
    struct timeval tv = {0};

    while (s_dev_rtc->task_runing) {
        if (XR_OS_OK != XR_OS_SemaphoreWait(&(s_dev_rtc->sem), XR_OS_WAIT_FOREVER)) {
            continue;
        }

        if (s_dev_rtc->irq_cb) {
            s_dev_rtc->irq_cb(NULL);
        }
    }

    XR_OS_ThreadDelete(&(s_dev_rtc->tid));
}

drv_status_t dev_rtc_init(drv_rtc_time_fmt_t *fmt, irq_callback cb)
{
	int ret = DRV_ERROR;
    // drv_rtc_type_t chip_type = DRV_RTC_TYPE_MAX;
    // drv_rtc_ops_handler_t ops_map[DRV_RTC_TYPE_MAX] = DEV_RTC_OPS_MAP;

    if (s_dev_rtc) {
        dev_rtc_error("rtc has init, return!");
        goto exit;
    }

    // rtc
    s_dev_rtc = (dev_rtc_t *)malloc(sizeof(dev_rtc_t));
    if (NULL == s_dev_rtc) {
        dev_rtc_error("malloc fail");
        goto exit;
    }
    memset(s_dev_rtc, 0, sizeof(dev_rtc_t));

    s_dev_rtc->ops = malloc(sizeof(drv_rtc_ops_t));
    if (NULL == s_dev_rtc->ops) {
        goto exit;
    }
    memset(s_dev_rtc->ops, 0, sizeof(drv_rtc_ops_t));

    if (XR_OS_OK != XR_OS_SemaphoreCreate(&s_dev_rtc->sem, 0, 5)) {
        dev_rtc_error("sem create err.");
        goto exit;
    }

    //校准rtc,只调用接口
    s_dev_cali_rtc = (dev_rtc_t *)malloc(sizeof(dev_rtc_t));
    if (NULL == s_dev_cali_rtc) {
        dev_rtc_error("malloc fail");
        goto exit;
    }
    memset(s_dev_cali_rtc, 0, sizeof(dev_rtc_t));
    s_dev_cali_rtc->ops = malloc(sizeof(drv_rtc_ops_t));
    if (NULL == s_dev_cali_rtc->ops) {
        goto exit;
    }
    memset(s_dev_cali_rtc->ops, 0, sizeof(drv_rtc_ops_t));


    // 判断rtc类型
    // chip_type = _rtc_chip_detect(); // 深休起来，跑这行代码会耗时
    // printf("rtc chip type:%d\n", chip_type);
    // s_rtc_chip = chip_type;

    s_rtc_chip = DRV_RTC_TYPE_MAX;      //默认使用内部RTC

    drv_rtc_inner_get_ops(s_dev_rtc->ops);
    drv_rtc_at8563_get_ops(s_dev_cali_rtc->ops);
    if (XR_OS_OK != XR_OS_ThreadCreate(&(s_dev_rtc->tid),
                                "_dev_rtc_task",
                                _dev_rtc_task,
                                NULL,
                                XR_OS_PRIORITY_NORMAL,
                                1024 * 2)) {
        dev_rtc_error("create task err.....");
        goto exit;
    }


    s_dev_rtc->irq_cb = cb;
    s_dev_rtc->ops->init(fmt, dev_rtc_irq_sem_relase);
    s_dev_cali_rtc->ops->init(fmt, NULL);

	ret = DRV_OK;

exit:
	if (ret != DRV_OK) {
        if (s_dev_rtc) {
			if (XR_OS_SemaphoreIsValid(&s_dev_rtc->sem)) {
				XR_OS_SemaphoreDelete(&s_dev_rtc->sem);
			}
            free(s_dev_rtc->ops);
            s_dev_rtc->ops = NULL;
            free(s_dev_rtc);
			s_dev_rtc = NULL;
		}

        if(s_dev_cali_rtc) {
            free(s_dev_cali_rtc->ops);
            s_dev_cali_rtc->ops = NULL;
            free(s_dev_cali_rtc);
            s_dev_cali_rtc = NULL;
        }
	}

	return ret;
}

void dev_rtc_deinit(void)
{
    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_warn("rtc dev not init.");
        return;
    }

    if (XR_OS_ThreadIsValid(&(s_dev_rtc->tid))) {
        s_dev_rtc->task_runing = 0;
        while(XR_OS_ThreadIsValid(&(s_dev_rtc->tid))) {
            hal_msleep(50);
        }
    }

    if (XR_OS_SemaphoreIsValid(&s_dev_rtc->sem)) {
        XR_OS_SemaphoreDelete(&s_dev_rtc->sem);
    }

    s_dev_rtc->ops->deinit();
    free(s_dev_rtc->ops);
    s_dev_rtc->ops = NULL;
    free(s_dev_rtc);
	s_dev_rtc = NULL;

    if(s_dev_cali_rtc) {
        s_dev_cali_rtc->ops->deinit();
        free(s_dev_cali_rtc->ops);
        s_dev_cali_rtc->ops = NULL;
        free(s_dev_cali_rtc);
        s_dev_cali_rtc = NULL;
    }
}

drv_status_t dev_rtc_irq_cb_register(irq_callback cb)
{
    if (NULL == s_dev_rtc) {
        dev_rtc_warn("rtc dev not init.");
        return DRV_ERROR;
    }

	s_dev_rtc->irq_cb = cb;
	return DRV_OK;
}

drv_status_t dev_rtc_set_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;
    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->set_time(fmt);

exit:
    return drv_status;
}

drv_status_t dev_rtc_get_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->get_time(fmt);

exit:
    return drv_status;
}

drv_status_t dev_rtc_set_alert_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->set_alert_time(fmt);

exit:
    return drv_status;
}

drv_status_t dev_rtc_get_alert_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->get_alert_time(fmt);

exit:
    return drv_status;
}

drv_status_t dev_rtc_set_alert_stop(void)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->clear_alert_time();

exit:
    return drv_status;
}

drv_status_t dev_rtc_clear_af(void)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_rtc || NULL == s_dev_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_rtc->ops->clear_alert_flag();

exit:
    return drv_status;
}


//*******************  校准rtc    ***********************/
drv_status_t dev_cali_rtc_set_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;
    if (NULL == s_dev_cali_rtc || NULL == s_dev_cali_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_cali_rtc->ops->set_time(fmt);

exit:
    return drv_status;
}

drv_status_t dev_cali_rtc_get_time(drv_rtc_time_fmt_t *fmt)
{
    drv_status_t drv_status = DRV_OK;

    if (NULL == s_dev_cali_rtc || NULL == s_dev_cali_rtc->ops) {
        dev_rtc_error("rtc dev not init.");
        goto exit;
    }
    s_dev_cali_rtc->ops->get_time(fmt);

exit:
    return drv_status;
}

static int cmd_rtc_get_time(int argc, char **argv)
{
    drv_rtc_time_fmt_t fmt = {0};
    drv_rtc_time_fmt_t cail_fmt = {0};
    int ret1 = dev_rtc_get_time(&fmt);
    int ret2 = dev_cali_rtc_get_time(&cail_fmt);

    if ( !ret1 && !ret2) {
        printf("[RTC] %04d-%02d-%02d %02d:%02d:%02d\n",
                    fmt.year, fmt.month, fmt.day,
                    fmt.hour, fmt.minute, fmt.second);
        printf("[CAIL RTC] %04d-%02d-%02d %02d:%02d:%02d\n",
                    cail_fmt.year, cail_fmt.month, cail_fmt.day,
                    cail_fmt.hour, cail_fmt.minute, cail_fmt.second);
    } else {
        printf("[RTC] get rtc time failed\n");
        return -1;
    }
    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_rtc_get_time, rtc_get_time, test_get_rtc_time);


static int cmd_rtc_set_time(int argc, char **argv)
{
    int ret =-1;
    drv_rtc_time_fmt_t fmt = {0};
    drv_rtc_time_fmt_t cail_fmt = {0};

    fmt.year = 2025;
    fmt.month = 9;
    fmt.day = 19;
    fmt.hour = 18;
    fmt.minute = 00;
    fmt.second = 00;
    fmt.week = 5;    
    if(atoi(argv[1]) ==  0){
        ret = dev_rtc_set_time(&fmt);
    }else if(atoi(argv[1]) ==  1){
        ret = dev_cali_rtc_set_time(&fmt);
    }
    
    if(0!=ret){
        printf("set rtc time failed\n");
        return -1;
    }
    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(cmd_rtc_set_time, rtc_set_time, test_rtc_set_time);