#include "drv_rgb_led_iface.h"
#include <stdio.h>
#include <sunxi_hal_ledc.h>
#include "../../drv_log.h"
#include "kernel/os/os.h"
#include "kernel/os/os_time.h"
#include <hal_gpio.h>
#include "sys/interrupt.h"
#include <math.h>
#include "task.h"
#include "semphr.h"
//  #include "cmsis_gcc.h"
// #include <cmsis_compiler.h>
#include "hal_interrupt.h"


#if (CFG_LED_EN && CFG_LED_MODE_TYPE == CFG_LED_MODE_RGB)

#define VERB 1
#define INFO 1
#define DBUG 1
#define WARN 1
#define EROR 1

#if VERB
#define dev_rgb_led_verb(fmt, ...) drv_logv(fmt, ##__VA_ARGS__);
#else
#define dev_rgb_led_verb(fmt, ...)
#endif

#if INFO
#define dev_rgb_led_info(fmt, ...) drv_logi(fmt, ##__VA_ARGS__);
#else
#define dev_rgb_led_info(fmt, ...)
#endif

#if DBUG
#define dev_rgb_led_debug(fmt, ...) drv_logd(fmt, ##__VA_ARGS__);
#else
#define dev_rgb_led_debug(fmt, ...)
#endif

#if WARN
#define dev_rgb_led_warn(fmt, ...) drv_logw(fmt, ##__VA_ARGS__);
#else
#define dev_rgb_led_warn(fmt, ...)
#endif

#if EROR
#define dev_rgb_led_error(fmt, ...) drv_loge(fmt, ##__VA_ARGS__);
#else
#define dev_rgb_led_error(fmt, ...)
#endif


#define RGB_LED_INDICATE_PIN GPIOA(11)


#define RGB_LED_NUM_MAX         60   // 灯珠数目、渐变色颜色值最大数

static uint8_t s_led_task_run = 0;
static uint8_t s_led_ready = 1;
static uint8_t s_interact_run = 0;
static uint8_t s_led_sleep = 0;

static XR_OS_Mutex_t s_led_mutex;
static XR_OS_Thread_t s_led_tid;
static XR_OS_Semaphore_t s_led_sem;

typedef struct {
    int show_time;
    int cycle_time;
	int color_data_len;
    uint8_t color_data[RGB_LED_NUM_MAX][3];
	uint8_t colorful_led_data[RGB_LED_NUM_MAX][RGB_LED_NUM][3];
	drv_rgb_led_mode_t curr_mode;
	int flowing_regress_flag;
} rgb_led_info_t;

static rgb_led_info_t s_rgb_led_info = {0};


void led_pin_init()
{
	hal_gpio_set_pull(RGB_LED_INDICATE_PIN, GPIO_PULL_UP);
    hal_gpio_set_direction(RGB_LED_INDICATE_PIN, GPIO_DIRECTION_OUTPUT);
    hal_gpio_pinmux_set_function(RGB_LED_INDICATE_PIN,GPIO_MUXSEL_OUT);
    hal_gpio_set_data(RGB_LED_INDICATE_PIN, GPIO_DATA_LOW);
}

void led_pin_deinit()
{
	hal_gpio_pinmux_set_function(RGB_LED_INDICATE_PIN,GPIO_MUXSEL_DISABLED);
	hal_gpio_set_pull(RGB_LED_INDICATE_PIN, GPIO_PULL_DOWN_DISABLED);
}

/*
 *
	GPIO Base Address: 0x4004A400
	PA_DATA Offset Address: 0x0010
 *
 */
void led_pin_set_high()
{
	// hal_gpio_set_data(RGB_LED_INDICATE_PIN, GPIO_DATA_HIGH);		//580ns
	uint32_t reg;
	reg = hal_readl(0x4004a410);
	reg &= ~(0x1 << 11);		//0xfffff7ff
	hal_writel(reg | 0x1 << 11, 0x4004a410);	//321ns			//reg |= 0x0800
}

void led_pin_set_low()
{	
	// hal_gpio_set_data(RGB_LED_INDICATE_PIN, GPIO_DATA_LOW);
	uint32_t reg;
	reg = hal_readl(0x4004a410);
	reg &= ~(0x1 << 11);
	hal_writel(reg, 0x4004a410);								//reg | 0x0 << 11,
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
void led_send_data0()
{
	// dev_rgb_led_info("发送0的码----------------------------------");
	led_pin_set_high();
	led_pin_set_low();

	// 140个nop
	for (volatile int i = 0; i < 22; i++) {
		__asm__ __volatile__ ("nop" ::: "memory");
	}
}

void led_send_data1()
{
	// dev_rgb_led_info("发送1的码----------------------------------");
	led_pin_set_high();
	// 183个nop
	for (volatile int i = 0; i < 22; i++) {
		__asm__ __volatile__ ("nop" ::: "memory");
	}

	led_pin_set_low();
}
#pragma GCC pop_options

static void rgb_indicate_reset()
{
	led_pin_set_low();
	hal_udelay(90);
}

void color_debug(uint32_t send_color_data)
{
	static uint32_t real_data = 0;
	printf("send_color_data : ");
	for (int i = 31; i >= 0; i--) {
        // 提取第i位的值（0或1）
        uint32_t bit = (send_color_data >> i) & 1;
        printf("%u", bit);

        // 每4位添加空格（不在最低位后添加）
        if (i % 4 == 0 && i != 0) 
            printf(" ");
    }
    printf("\n");

	printf("real_data : ");
	for (int i = 0; i < 24; i++) {
		// 每4位添加空格（不在最低位后添加）
        if (i % 4 == 0 && i != 0) 
            printf(" ");
		if((send_color_data & (0x800000 >> i)) == 0) 
		{
			printf("0");
		} else {
			printf("1");
		}
	}
	printf("\n");
}
#endif


#pragma GCC push_options
#pragma GCC optimize ("O0")  // 临时关闭优化
static void led_set_rgb_data(uint8_t rgb_r,uint8_t rgb_g,uint8_t rgb_b)
{
	uint8_t rgb_idx;
	uint8_t rgb_data[3] = {0};
	rgb_data[0] = rgb_g;
	rgb_data[1] = rgb_r; 
	rgb_data[2] = rgb_b;

	XR_OS_MutexLock(&s_led_mutex, XR_OS_WAIT_FOREVER);
	rgb_indicate_reset();
	portENTER_CRITICAL();

    int led_num = 0;
    for (led_num = 0; led_num < RGB_LED_NUM; led_num++) {
        for(rgb_idx = 0; rgb_idx < 3; rgb_idx++) {
            if((rgb_data[rgb_idx] & 0x80) == 0) led_send_data0(); else led_send_data1(); // 1000 0000
            if((rgb_data[rgb_idx] & 0x40) == 0) led_send_data0(); else led_send_data1(); // 0100 0000
            if((rgb_data[rgb_idx] & 0x20) == 0) led_send_data0(); else led_send_data1(); // 0010 0000
            if((rgb_data[rgb_idx] & 0x10) == 0) led_send_data0(); else led_send_data1(); // 0001 0000
            if((rgb_data[rgb_idx] & 0x08) == 0) led_send_data0(); else led_send_data1(); // 0000 1000
            if((rgb_data[rgb_idx] & 0x04) == 0) led_send_data0(); else led_send_data1(); // 0000 0100
            if((rgb_data[rgb_idx] & 0x02) == 0) led_send_data0(); else led_send_data1(); // 0000 0010
            if((rgb_data[rgb_idx] & 0x01) == 0) led_send_data0(); else led_send_data1(); // 0000 0001
        }
	}
	portEXIT_CRITICAL();
	XR_OS_MutexUnlock(&s_led_mutex);
}
#pragma GCC pop_options  // 恢复原有优化级别

static void led_set_rgb_data_flowing(uint8_t data[][3], int data_len)
{
	uint8_t rgb_data[RGB_LED_NUM_MAX][3];

	OS_MutexLock(&s_led_mutex, XR_OS_WAIT_FOREVER);

	for (int i = 0; i < data_len; i++) {
		rgb_data[i][0] = data[i][1];
		rgb_data[i][1] = data[i][0]; 
		rgb_data[i][2] = data[i][2];
	}

	// dev_rgb_led_info("data_len = %d", data_len);
	// for (int i = 0; i < data_len; i++) {
	// 	dev_rgb_led_info("data [%d, %d, %d]", rgb_data[i][0], rgb_data[i][1], rgb_data[i][2]);
	// }
	// printf("\n \n");

	rgb_indicate_reset();

	portENTER_CRITICAL();

    for (int i = 0; i < data_len; i++) {
        for(int rgb_idx = 0; rgb_idx < 3; rgb_idx++) {
            if((rgb_data[i][rgb_idx] & 0x80) == 0) led_send_data0(); else led_send_data1(); // 1000 0000
            if((rgb_data[i][rgb_idx] & 0x40) == 0) led_send_data0(); else led_send_data1(); // 0100 0000
            if((rgb_data[i][rgb_idx] & 0x20) == 0) led_send_data0(); else led_send_data1(); // 0010 0000
            if((rgb_data[i][rgb_idx] & 0x10) == 0) led_send_data0(); else led_send_data1(); // 0001 0000
            if((rgb_data[i][rgb_idx] & 0x08) == 0) led_send_data0(); else led_send_data1(); // 0000 1000
            if((rgb_data[i][rgb_idx] & 0x04) == 0) led_send_data0(); else led_send_data1(); // 0000 0100
            if((rgb_data[i][rgb_idx] & 0x02) == 0) led_send_data0(); else led_send_data1(); // 0000 0010
            if((rgb_data[i][rgb_idx] & 0x01) == 0) led_send_data0(); else led_send_data1(); // 0000 0001
        }
	}
	portEXIT_CRITICAL();
	XR_OS_MutexUnlock(&s_led_mutex);
}

//==============================单色灯============================================================

static void led_show_single(void)
{
	dev_rgb_led_info("...");

	int show_time = s_rgb_led_info.show_time;
	int all_show_time = 0;
	int sleep_once_time = 10; // 10ms


	led_set_rgb_data(s_rgb_led_info.color_data[0][0], 
					 s_rgb_led_info.color_data[0][1], 
					 s_rgb_led_info.color_data[0][2]);

	// 000==关灯,退出。
	if (s_rgb_led_info.color_data[0][0] == 0 && 
		s_rgb_led_info.color_data[0][1] == 0 && 
		s_rgb_led_info.color_data[0][2] == 0) {
		return;
	}

	// 短亮
	if (show_time > 0) {
		while (all_show_time < show_time) {
			// 外部触发退出
			if (0 == s_interact_run) {
				break;
			}
			XR_OS_MSleep(sleep_once_time);
			all_show_time += sleep_once_time;
		}

		// 关灯
		led_set_rgb_data(0, 0, 0);
	} else if (show_time == DRV_RGB_LED_SHOW_FOREVER) {
		while (1) {
			// 外部触发退出
			if (0 == s_interact_run) {
				return;
			}
			XR_OS_MSleep(30);
		}
	}
}

//==============================呼吸灯============================================================

static void led_show_breath(void)
{
	dev_rgb_led_info("...");

	int breath_step = s_rgb_led_info.color_data_len;
	int interval_time = s_rgb_led_info.cycle_time / breath_step;
	int show_time = s_rgb_led_info.show_time;
	int all_show_time = 0;
	int sleep_once_time = 10; // 10ms

	while (1) {
		for (int i = 0; i < breath_step; i++) {
			led_set_rgb_data(s_rgb_led_info.color_data[i][0], 
							 s_rgb_led_info.color_data[i][1], 
							 s_rgb_led_info.color_data[i][2]);

			int cur_show_time = 0;

			while (cur_show_time < interval_time) {
				XR_OS_MSleep(sleep_once_time);
				cur_show_time += sleep_once_time;
				all_show_time += sleep_once_time;

				// 外部触发退出
				if (0 == s_interact_run) {
					led_set_rgb_data(0, 0, 0);
					return;
				}

				// 显示总时长到
				if (show_time > 0 && all_show_time >= show_time) {
					led_set_rgb_data(0, 0, 0);
					return;
				}
			}
		}
	}
}

//==============================流水灯============================================================

static void led_show_flowing(void)
{
	dev_rgb_led_info("...");

	int flowing_step = s_rgb_led_info.color_data_len;
	int interval_time = s_rgb_led_info.cycle_time / flowing_step;
	int show_time = s_rgb_led_info.show_time;
	int all_show_time = 0;
	int sleep_once_time = 10; // 10ms

	uint8_t rgb_data[RGB_LED_NUM][3];

	int flowing_regress_flag = 0;

	while (1) {
		for (int i = 0; i < flowing_step; i++) {
			memset(rgb_data, 0, RGB_LED_NUM * 3);

			if (s_rgb_led_info.flowing_regress_flag) {
				if (flowing_regress_flag == 0) {
					memcpy(rgb_data[i % RGB_LED_NUM], s_rgb_led_info.color_data[i], 3);
				} else {
					memcpy(rgb_data[(flowing_step - i - 1) % RGB_LED_NUM], s_rgb_led_info.color_data[flowing_step - i - 1], 3);
				}
			} else {
				memcpy(rgb_data[i % RGB_LED_NUM], s_rgb_led_info.color_data[i], 3);
			}

			led_set_rgb_data_flowing(rgb_data, RGB_LED_NUM);

			int cur_show_time = 0;

			while (cur_show_time < interval_time) {
				XR_OS_MSleep(sleep_once_time);
				cur_show_time += sleep_once_time;
				all_show_time += sleep_once_time;

				// 外部触发退出
				if (0 == s_interact_run) {
					led_set_rgb_data(0, 0, 0);
					return;
				}

				// 显示总时长到
				if (show_time > 0 && all_show_time >= show_time) {
					led_set_rgb_data(0, 0, 0);
					return;
				}
			}
		}

		flowing_regress_flag = ~flowing_regress_flag;
	}
}

//==============================================================================================

//==============================七彩灯============================================================

static void led_show_colorful(void)
{
	dev_rgb_led_info("...");

	int flowing_step = s_rgb_led_info.color_data_len;
	int interval_time = s_rgb_led_info.cycle_time / flowing_step;
	int show_time = s_rgb_led_info.show_time;
	int all_show_time = 0;
	int sleep_once_time = 10; // 10ms

	uint8_t rgb_data[RGB_LED_NUM][3];

	while (1) {
		for (int i = 0; i < flowing_step; i++) {
			memset(rgb_data, 0, RGB_LED_NUM * 3);

			memcpy(rgb_data, s_rgb_led_info.colorful_led_data[i], RGB_LED_NUM * 3);

			led_set_rgb_data_flowing(rgb_data, RGB_LED_NUM);

			int cur_show_time = 0;

			while (cur_show_time < interval_time) {
				XR_OS_MSleep(sleep_once_time);
				cur_show_time += sleep_once_time;
				all_show_time += sleep_once_time;

				// 外部触发退出
				if (0 == s_interact_run) {
					led_set_rgb_data(0, 0, 0);
					return;
				}

				// 显示总时长到
				if (show_time > 0 && all_show_time >= show_time) {
					led_set_rgb_data(0, 0, 0);
					return;
				}
			}
		}

		// 一轮灯效执行完成，保持灯光不变
		if (show_time == DRV_RGB_LED_SHOW_MODE1) {
			while (0 != s_interact_run) {
				XR_OS_MSleep(20);
			}

			led_set_rgb_data(0, 0, 0);
			return;
		}
	}
}

//==============================================================================================

void led_task(void *arg)
{
	led_pin_init();
	led_pin_set_low();

	while(1) {
		s_led_ready = 1;
		if (XR_OS_OK != XR_OS_SemaphoreWait(&s_led_sem, XR_OS_WAIT_FOREVER)) {
			goto next_loop;
		}
		if (s_led_sleep) {
			dev_rgb_led_error("led_task sleep");
			continue;
		}
		s_led_ready = 0;
		s_interact_run = 1;

		dev_rgb_led_info("led_task run.");

		switch(s_rgb_led_info.curr_mode) {

		case RGB_LED_MODE_BREATH:
			led_show_breath();
			s_rgb_led_info.curr_mode = RGB_LED_MODE_NONE;
			break;

		case RGB_LED_MODE_SINGLE:
			led_show_single();
			s_rgb_led_info.curr_mode = RGB_LED_MODE_NONE;
			break;

		case RGB_LED_MODE_FLOWING:
			led_show_flowing();
			s_rgb_led_info.curr_mode = RGB_LED_MODE_NONE;
			break;

		case RGB_LED_MODE_COLORFUL:
			led_show_colorful();
			s_rgb_led_info.curr_mode = RGB_LED_MODE_NONE;
			break;

		case RGB_LED_MODE_NONE:
			break;

		default:
			dev_rgb_led_warn("unknown action.");
			break;
		}

next_loop:
		if (1 != s_led_task_run) {
			break;
		}
		XR_OS_MSleep(10);
	}

    led_pin_set_low();
    led_pin_deinit();

	dev_rgb_led_info("led_task exit");
	XR_OS_ThreadDelete(&s_led_tid);
}

/////////////////////////////////////////////////////////////////////////////////////////////

drv_rgb_led_mode_t drv_rgb_led_iface_get_mode(void)
{
	return s_rgb_led_info.curr_mode;
}

int drv_rgb_led_iface_init(void)
{
	if (s_led_task_run == 1) {
		dev_rgb_led_warn("rgb_led had init.");
		return 0;
	}

	if (XR_OS_OK != XR_OS_MutexCreate(&s_led_mutex)) {
		dev_rgb_led_error("create s_led_mutex fail");
		goto err;
	}

	if (XR_OS_OK != XR_OS_SemaphoreCreate(&s_led_sem, 0, 1)) {
		dev_rgb_led_error("create s_led_sem fail");
		goto err;
	}

	s_led_task_run = 1;
	if (XR_OS_OK != XR_OS_ThreadCreate(&s_led_tid,
								 "led_task",
								 led_task,
								 NULL,
								 XR_OS_PRIORITY_ABOVE_NORMAL,
								 1024 * 2)) {
		dev_rgb_led_error("create led_task fail.");
		goto err;
	}

	return 0;

err:
	if (XR_OS_MutexIsValid(&s_led_mutex)) {
	    XR_OS_MutexDelete(&s_led_mutex);
	}

	if (XR_OS_SemaphoreIsValid(&s_led_sem)) {
		XR_OS_SemaphoreDelete(&s_led_sem);
	}

	s_led_task_run = 0;

	return -1;
}

int drv_rgb_led_iface_deinit(void)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("rgb_led had deinit.");
		return 0;
	}

	if (drv_rgb_led_iface_get_mode() != RGB_LED_MODE_NONE) {
		drv_rgb_led_iface_set_single(0, 0, 0, DRV_RGB_LED_SHOW_FOREVER);

		while (drv_rgb_led_iface_get_mode() != RGB_LED_MODE_NONE) {
			XR_OS_MSleep(10);
		}
	
		dev_rgb_led_info("rgb led off success.");
	}
	
	s_led_task_run = 0;
	while (XR_OS_ThreadIsValid(&s_led_tid)) {
		XR_OS_MSleep(10);
	}

    XR_OS_MutexDelete(&s_led_mutex);
	XR_OS_SemaphoreDelete(&s_led_sem);

	return 0;
}

void drv_rgb_led_iface_set_single(uint8_t data0, uint8_t data1, uint8_t data2, int show_time)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("led_task is not running.");
		return;
	}

	dev_rgb_led_info("data [%d, %d, %d]", data0, data1, data2);

	while(0 == s_led_ready || 1 == s_interact_run) {
		s_interact_run = 0;
		XR_OS_MSleep(10);
	}

	s_rgb_led_info.curr_mode = RGB_LED_MODE_SINGLE;
	s_rgb_led_info.show_time = show_time;
	s_rgb_led_info.cycle_time = 0;
	s_rgb_led_info.color_data[0][0] = data0;
	s_rgb_led_info.color_data[0][1] = data1;
	s_rgb_led_info.color_data[0][2] = data2;
	s_rgb_led_info.color_data_len = 3;

	XR_OS_SemaphoreRelease(&s_led_sem);
}

void drv_rgb_led_iface_set_breath(uint8_t data[][3], int data_len, int show_time, int cycle_time)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("led_task is not running.");
		return;
	}

	for (int i = 0; i < data_len; i++) {
		dev_rgb_led_info("data %d [%d, %d, %d]", i, data[i][0], data[i][1], data[i][2]);
	}

	int valid_data_len = (data_len <= RGB_LED_NUM_MAX ? data_len : RGB_LED_NUM_MAX);

	while(0 == s_led_ready || 1 == s_interact_run) {
		s_interact_run = 0;
		XR_OS_MSleep(10);
	}

	s_rgb_led_info.curr_mode = RGB_LED_MODE_BREATH;
	s_rgb_led_info.show_time = show_time;
	s_rgb_led_info.cycle_time = cycle_time;
	memcpy(s_rgb_led_info.color_data, data, valid_data_len * 3);
	s_rgb_led_info.color_data_len = valid_data_len;

	XR_OS_SemaphoreRelease(&s_led_sem);
}

void drv_rgb_led_iface_set_flowing(uint8_t data[][3], int data_len, int show_time, int cycle_time, int flowing_regress_flag)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("led_task is not running.");
		return;
	}

	for (int i = 0; i < data_len; i++) {
		dev_rgb_led_info("data %d [%d, %d, %d]", i,  data[i][0], data[i][1], data[i][2]);
	}

	int valid_data_len = (data_len <= RGB_LED_NUM_MAX ? data_len : RGB_LED_NUM_MAX);

	while(0 == s_led_ready || 1 == s_interact_run) {
		s_interact_run = 0;
		XR_OS_MSleep(10);
	}

	s_rgb_led_info.curr_mode = RGB_LED_MODE_FLOWING;
	s_rgb_led_info.show_time = show_time;
	s_rgb_led_info.cycle_time = cycle_time;
	s_rgb_led_info.flowing_regress_flag = flowing_regress_flag;
	memcpy(s_rgb_led_info.color_data, data, valid_data_len * 3);
	s_rgb_led_info.color_data_len = valid_data_len;

	XR_OS_SemaphoreRelease(&s_led_sem);
}

void drv_rgb_led_iface_set_colorful(uint8_t *data, int data_len, int show_time, int cycle_time)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("led_task is not running.");
		return;
	}

	for (int i = 0; i < data_len; i++) {
		dev_rgb_led_info("num: %d", i);
		for (int j = 0; j < RGB_LED_NUM; j++) {
			dev_rgb_led_info("data [%d, %d, %d]", data[(i * RGB_LED_NUM * 3) + (j * 3) + 0], 
												  data[(i * RGB_LED_NUM * 3) + (j * 3) + 1], 
												  data[(i * RGB_LED_NUM * 3) + (j * 3) + 2]);
		}
	}

	int valid_data_len = (data_len <= RGB_LED_NUM_MAX ? data_len : RGB_LED_NUM_MAX);

	while(0 == s_led_ready || 1 == s_interact_run) {
		s_interact_run = 0;
		XR_OS_MSleep(10);
	}

	s_rgb_led_info.curr_mode = RGB_LED_MODE_COLORFUL;
	s_rgb_led_info.show_time = show_time;
	s_rgb_led_info.cycle_time = cycle_time;
	memcpy(s_rgb_led_info.colorful_led_data, data, valid_data_len * RGB_LED_NUM * 3);
	s_rgb_led_info.color_data_len = valid_data_len;

	XR_OS_SemaphoreRelease(&s_led_sem);
}

void drv_rgb_led_iface_suspend(void)
{
	if (s_led_task_run == 0) {
		dev_rgb_led_warn("led_task is not running.");
		return;
	}

	drv_rgb_led_mode_t mode = drv_rgb_led_iface_get_mode();
	dev_rgb_led_info("led mode is %d", mode);

	if (mode == RGB_LED_MODE_BREATH || mode == RGB_LED_MODE_FLOWING ||  mode == RGB_LED_MODE_COLORFUL) {
		drv_rgb_led_iface_set_single(0, 0, 0, DRV_RGB_LED_SHOW_FOREVER);

	}
	
	while (drv_rgb_led_iface_get_mode() != RGB_LED_MODE_NONE) {
		XR_OS_MSleep(10);
	}

	led_pin_set_low();
	s_led_sleep = 1;
	led_pin_deinit();
	dev_rgb_led_info("rgb led off success.");
}

void drv_rgb_led_iface_resume(void)
{
	led_pin_init();
	s_led_sleep = 0;
}

void drv_rgb_led_iface_set_color(uint8_t r, uint8_t g, uint8_t b)
{
	led_set_rgb_data(r, g, b);
}