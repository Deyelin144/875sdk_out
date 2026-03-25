#include "dev_sd.h"
#include "kernel/os/os.h"
#include <hal_time.h>
#include "../drv_log.h"
#include "../extio/dev_extio.h"
#include <time.h>
#include <sys/time.h>
#include <hal_gpio.h>
#define TRY_TIME_MAX   3
#define DEV_SD_DECT_PIN     GPIOB(1)

typedef struct {
    int working;
    XR_OS_Thread_t tid;
    XR_OS_Semaphore_t sem;
    drv_sd_state_t state;
    sd_detect_cb cb;
    uint32_t irq;
} dev_sd_t;

static dev_sd_t *s_dev_sd = NULL;

void dev_sd_irq_handler(void *arg)
{
    if (s_dev_sd && XR_OS_SemaphoreIsValid(&s_dev_sd->sem)) {
        XR_OS_SemaphoreRelease(&s_dev_sd->sem);
    }
}

static void _dev_sd_task(void *args)
{
    int ret = -1;
    int num = 0;
    drv_sd_state_t state = SD_STATE_REMOVE;

	s_dev_sd->working= 1;
    s_dev_sd->state = dev_sd_detect_status();

    while (s_dev_sd->working) {
        if (XR_OS_OK != XR_OS_SemaphoreWait(&(s_dev_sd->sem), XR_OS_WAIT_FOREVER)) {
            continue;
        }

        if (dev_sd_detect_status()) {
            printf("SD_STATE_INSERT ============== \n");
            state = SD_STATE_INSERT;
        } else {
            state = SD_STATE_REMOVE;
            printf("SD_STATE_REMOVE ============== \n");
        }

        if (s_dev_sd->state != state) {
            if (state == SD_STATE_INSERT) {
                for (int i = 0; i < TRY_TIME_MAX; i++) {
                    ret = card_detect(1, 0);
                    if (ret == 0) {
                        drv_logw("sd mount suc\n");
                        s_dev_sd->state = SD_STATE_INSERT;
                        break;
                    } else {
                        drv_loge("sd mount fail, ret = %d, try_num = %d.", ret, i + 1);
                    }
                }
            } else if (state == SD_STATE_REMOVE) {
                for (int i = 0; i < TRY_TIME_MAX; i++) {
                    ret = card_detect(0, 0);
                    if (ret == 0) {
                        drv_logw("sd unmount suc\n");
                        s_dev_sd->state = SD_STATE_REMOVE;
                        break;
                    } else {
                        drv_loge("sd unmount fail, ret = %d, try_num = %d.", ret, i + 1);
                    }
                }
            }
            if (0 == ret && NULL != s_dev_sd->cb) {
                s_dev_sd->cb(s_dev_sd->state);
            }
        }
    }
    XR_OS_ThreadDelete(&(s_dev_sd->tid));
}

drv_status_t dev_sd_detect_gpio_init(void)
{
    int ret = -1;

    hal_gpio_set_pull(DEV_SD_DECT_PIN, GPIO_PULL_UP);
    hal_gpio_set_direction(DEV_SD_DECT_PIN, GPIO_DIRECTION_INPUT);
    //hal_gpio_set_data(DEV_SD_DECT_PIN, GPIO_DATA_HIGH);
    hal_gpio_pinmux_set_function(DEV_SD_DECT_PIN,GPIO_MUXSEL_EINT);

    hal_gpio_to_irq(DEV_SD_DECT_PIN, &s_dev_sd->irq);
    hal_gpio_irq_request(s_dev_sd->irq, (hal_irq_handler_t)dev_sd_irq_handler, IRQ_TYPE_EDGE_BOTH, NULL);
    hal_gpio_irq_enable(s_dev_sd->irq);
    return ret;
}

drv_status_t dev_sd_detect_gpio_deinit()
{
    int ret = -1;
#if (CFG_LTE_RI_IO_IRQ == CFG_LTE_RI_IRQ_MODE)
    if (0 != (ret = hal_gpio_irq_disable(s_dev_sd->irq))) {
        return ret;
    }
   
    if (0 != (ret = hal_gpio_irq_free(s_dev_sd->irq))) {
        return ret;
    }
#endif
    return ret;
}


drv_status_t dev_sd_detect_init(void)
{
	int ret = -1;

    if (s_dev_sd) {
        drv_loge("rtc has init, return!");
        goto exit;
    }

    s_dev_sd = (dev_sd_t *)malloc(sizeof(dev_sd_t));
    if (NULL == s_dev_sd) {
        drv_loge("malloc fail");
        goto exit;
    }
    memset(s_dev_sd, 0, sizeof(dev_sd_t));

    dev_sd_detect_gpio_init();

    if (XR_OS_OK != XR_OS_SemaphoreCreate(&s_dev_sd->sem, 0, 1)) {
        drv_loge("sem create err.");
        goto exit;
    }

    if (XR_OS_OK != XR_OS_ThreadCreate(&(s_dev_sd->tid),
                                    "_dev_sd_task",
                                    _dev_sd_task,
                                    NULL,
                                    XR_OS_PRIORITY_NORMAL,
                                    1024 * 2)) {
        drv_loge("create task err.....");
        goto exit;
    }

	ret = 0;

exit:
	if (ret != 0) {
        if (s_dev_sd) {
			if (XR_OS_SemaphoreIsValid(&s_dev_sd->sem)) {
				XR_OS_SemaphoreDelete(&s_dev_sd->sem);
			}

            free(s_dev_sd);
			s_dev_sd = NULL;
		}
	}

	return ret;
}

void dev_sd_detect_cb_register(sd_detect_cb cb)
{
    if (NULL != s_dev_sd) {
        s_dev_sd->cb = cb;
    }
}

void dev_sd_detect_deinit(void)
{
    if (NULL == s_dev_sd) {
        drv_logw("rtc dev not init.");
        return;
    }

    if (XR_OS_ThreadIsValid(&(s_dev_sd->tid))) {
        s_dev_sd->working= 0;
        while(XR_OS_ThreadIsValid(&(s_dev_sd->tid))) {
            hal_msleep(50);
        }
    }

    if (XR_OS_SemaphoreIsValid(&s_dev_sd->sem)) {
        XR_OS_SemaphoreDelete(&s_dev_sd->sem);
    }

    dev_sd_detect_gpio_deinit();
    free(s_dev_sd);
	s_dev_sd = NULL;
}

int dev_sd_detect_status(void)
{
    int status;
    hal_gpio_get_data(DEV_SD_DECT_PIN,&status); 
    return !status;         // 引脚电平低为插入，此处取反
}

void dev_sd_suspend(void)
{
    int ret = -1;
    if (NULL == s_dev_sd) {
        drv_logw("sd dev not init.");
        return;
    }

    ret = hal_gpio_irq_disable(s_dev_sd->irq);
    if (ret < 0)
    {
        drv_loge("disable irq error, irq num:%d, error num: %d", s_dev_sd->irq, ret);
        return;
    } else {
        drv_logi("Test hal_gpio_irq_disable API success!");
    }

    ret = hal_gpio_irq_free(s_dev_sd->irq);
    if (ret < 0)
    {
        drv_loge("free irq error, error num: %d", ret);
        return;
    } else {
        drv_logi("Test hal_gpio_irq_free API success!");
    }

    card_detect(0, 0);
}

void dev_sd_resume(void)
{
    if (NULL == s_dev_sd) {
        drv_logw("sd dev not init.");
        return;
    }

    hal_gpio_to_irq(DEV_SD_DECT_PIN, &s_dev_sd->irq);
    hal_gpio_irq_request(s_dev_sd->irq, (hal_irq_handler_t)dev_sd_irq_handler, IRQ_TYPE_EDGE_BOTH, NULL);
    hal_gpio_irq_enable(s_dev_sd->irq);

    card_detect(1, 0);
}