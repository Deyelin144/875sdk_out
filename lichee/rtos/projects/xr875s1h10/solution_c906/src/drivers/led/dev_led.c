#include "dev_led.h"
#include "../drv_log.h"
#include "../drv_common.h"

#define VERB 1
#define INFO 1
#define DBUG 1
#define WARN 1
#define EROR 1

#if VERB
#define dev_led_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_led_verb(fmt, ...)
#endif

#if INFO
#define dev_led_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_led_info(fmt, ...)
#endif

#if DBUG
#define dev_led_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_led_debug(fmt, ...)
#endif

#if WARN
#define dev_led_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_led_warn(fmt, ...)
#endif

#if EROR
#define dev_led_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_led_error(fmt, ...)
#endif


int dev_led_init(void)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)
	drv_gpio_lcd_iface_init();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
	drv_pwm_led_iface_init();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_init();
#endif
#endif

	return DRV_OK;
}

void dev_led_deinit()
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)
    drv_gpio_lcd_iface_deinit();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
	drv_pwm_led_iface_deinit();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_deinit();
#endif
#endif
}

void dev_led_suspend(char mode)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
    drv_pwm_led_iface_suspend();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_suspend();
#endif
#endif
}

void dev_led_resume(char mode)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
    drv_pwm_led_iface_resume();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_resume();
#endif
#endif
}

void dev_led_set_bright(int val1, int val2)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#if CFG_LED_GPIO_OPT_REVERSE
    drv_gpio_lcd_iface_set_state(val1 > 0 ? 0 : 1);
#else
    drv_gpio_lcd_iface_set_state(val1 > 0 ? 1 : 0);
#endif

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
	drv_pwm_led_iface_set_duty(val1, val2);
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    if (val1 > 0 && val1 <= 100) {
        drv_rgb_led_iface_set_single(val1, val1, val1, DRV_RGB_LED_SHOW_FOREVER);
    } else {
        drv_rgb_led_iface_set_single(0, 0, 0, DRV_RGB_LED_SHOW_FOREVER);
    }
#endif
#endif
}

int dev_led_get_bright(void)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)
    return (drv_gpio_lcd_iface_get_state() == 0 ? 0 : 100);
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
    return drv_pwm_led_iface_get_duty();
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    return (drv_rgb_led_iface_get_mode() == RGB_LED_MODE_NONE ? 0 : 100);
#endif
#endif

    return 0;
}

void dev_led_set_single(uint8_t data0, uint8_t data1, uint8_t data2, int show_time)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_set_single(data0, data1, data2, show_time);
#endif
#endif
}


void dev_led_set_breath(uint8_t data[][3], int data_len, int show_time, int cycle_time)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)
    
#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_set_breath(data, data_len, show_time, cycle_time);
#endif
#endif
}

void dev_led_set_flowing(uint8_t data[][3], int data_len, int show_time, int cycle_time, int flowing_regress_flag)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_set_flowing(data, data_len, show_time, cycle_time, flowing_regress_flag);
#endif
#endif
}

void dev_led_set_colorful(uint8_t* data, int data_len, int show_time, int cycle_time)
{
#if CFG_LED_EN
#if (CFG_LED_MODE_TYPE == CFG_LED_MODE_GPIO)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_PWM)

#elif (CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)
    drv_rgb_led_iface_set_colorful(data, data_len, show_time, cycle_time);
#endif
#endif
}


#define DEV_LED_TEST	1

#if (DEV_LED_TEST)
#include <console.h>
/***************************************************************
  *  @brief     测试指令
  *  @param     
  *  @note      BRG
  *  @Sample usage: 
  *        dev_led single 255 0 0  1000      //亮1s红灯
  *        dev_led breath 3000 1000 0       //绿色呼吸灯
  *        dev_led breath 3000 1000 1       //变色呼吸灯
  *        dev_led flowing 3000 1000 1      //流水灯
  *        dev_led colorful 3000 1000       //彩虹灯
  *        dev_led blink 3000 1000 1        //蓝色闪烁灯
  *        dev_led light 50 50              //亮度50
 **************************************************************/
int dev_led(int argc, const char **argv)
{
    if (2 > argc) {
        printf("param num is invalid.\n");
        return -1;
    }

	if  (0 == memcmp(argv[1], "single", strlen("single"))) {
        // B R G showtime
        dev_led_set_single(atoi(argv[2]), atoi(argv[3]),atoi(argv[4]), atoi(argv[5]));
    } else if (0 == memcmp(argv[1], "breath", strlen("breath"))) {
        if (atoi(argv[4]) == 0){        //绿色呼吸灯
            uint8_t rgb_led_data[6][3] = {{0, 10, 0}, {0, 50, 0}, {0, 100, 0}, {0, 0150, 0}, {0, 200, 0},{0, 250, 0}};
            dev_led_set_breath(rgb_led_data, 6, atoi(argv[2]), atoi(argv[3]));
        }else if (atoi(argv[4]) == 1){        //单色变色呼吸灯
            uint8_t rgb_led_data[5][3] = {{0, 10, 0}, {10, 10, 0}, {0, 0, 10}, {10, 10, 10}, {0, 10, 10}};
            dev_led_set_breath(rgb_led_data, 5, atoi(argv[2]), atoi(argv[3]));
        }
    } else if (0 == memcmp(argv[1], "flowing", strlen("flowing"))) {
        uint8_t rgb_led_data[5][3] = {{0, 10, 0}, {10, 10, 0}, {0, 0, 10}, {10, 10, 10}, {0, 10, 255}};
        //void dev_led_set_flowing(uint8_t data[][3], int data_len, int show_time, int cycle_time, int flowing_regress_flag)
        dev_led_set_flowing(rgb_led_data, 5, atoi(argv[2]), atoi(argv[3]),atoi(argv[4]));
    } else if (0 == memcmp(argv[1], "colorful", strlen("colorful"))) {  
        //void dev_led_set_colorful(uint8_t* data, int data_len, int show_time, int cycle_time)
        uint8_t rgbdata[] = {10, 0, 0,    0, 10,0,    0, 0, 10,    10,10,10,
                             50, 0, 0,    0, 50,0,    0, 0, 50,    50,50,50,
                             100, 0, 0,   0, 100,0,   0, 0, 100,   100,100,100,
                             150, 0, 0,   0, 150,0,   0, 0, 150,   150,150,150,
                             200, 0, 0,   0, 200,0,   0, 0, 200,   200,200,200,
                             250, 0, 0,   0, 250,0,   0, 0, 250,   250,250,250,
                             200, 0, 0,   0, 200,0,   0, 0, 200,   200,200,200,
                             150, 0, 0,   0, 150,0,   0, 0, 150,   150,150,150,
                             100, 0, 0,   0, 100,0,   0, 0, 100,   100,100,100,
                             50, 0, 0,    0, 50,0,    0, 0, 50,    50,50,50,
                             10, 0, 0,    0, 10,0,    0, 0, 10,    10,10,10,
                            };
        dev_led_set_colorful(rgbdata, 11, atoi(argv[2]), atoi(argv[3]));
    } else if (0 == memcmp(argv[1], "blink", strlen("blink"))) {  
       uint8_t rgb_led_data[6][3] = {{0, 0, 250}, {0, 0, 0}, {0, 0, 250}, {0, 0, 0}, {0, 0, 250},{0, 0, 0}};
        dev_led_set_breath(rgb_led_data, 6, atoi(argv[2]), atoi(argv[3]));
    }else if (0 == memcmp(argv[1], "light", strlen("light"))) {  
        dev_led_set_bright(atoi(argv[2]), atoi(argv[3]));
    }else if (0 == memcmp(argv[1], "set", strlen("set"))) {  
#ifdef CONFIG_PROJECT_SOLUTION_C906
        printf("json: %s\n",argv[2]);
        printf("sdk mode does not support transparent lamp command\n");
#endif
    }


    return 0;
}

FINSH_FUNCTION_EXPORT_CMD(dev_led, dev_led, decvie ctrl);

#endif /* DEV_LED_TEST */
