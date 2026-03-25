#include <stdlib.h>
#include <errno.h>
#include <hal_time.h>
#include <console.h>
#include <hal_atomic.h>
#include <osal/hal_interrupt.h>
#include <osal/hal_timer.h>

#ifdef CONFIG_DRIVERS_GPIO
#include <hal_gpio.h>
#endif

#ifdef CONFIG_DRIVERS_LPUART
#include <hal_lpuart.h>
#endif

#include <pm_base.h>
#include <pm_adapt.h>
#include <pm_debug.h>
#include <pm_testlevel.h>
#include <pm_rpcfunc.h>
#include <pm_state.h>
#include <pm_wakesrc.h>
#include <pm_wakecnt.h>
#include <pm_devops.h>
#include <pm_task.h>

#ifdef CONFIG_ARCH_ARM_CORTEX_M33
#include <arch/arm/mach/sun20iw2p1/irqs-sun20iw2p1.h>
#include <sunxi_hal_rtc.h>
#elif defined(CONFIG_ARCH_RISCV_C906)
#include <arch/riscv/sun20iw2p1/irqs-sun20iw2p1.h>
#include <sunxi_hal_rtc.h>
#elif defined(CONFIG_ARCH_DSP)
#include <pm_dsp_intc_sun20iw2p1.h>
#include <hal/sunxi_hal_rtc.h>
#endif


static suspend_testlevel_t pm_test_level = PM_SUSPEND_TEST_NONE;
static uint8_t pm_testlevel_recording = 0;

int pm_suspend_test_set(suspend_testlevel_t level)
{
	if (!pm_suspend_testlevel_valid(level))
		return -EINVAL;

	pm_test_level = level;

	return 0;
}


int pm_suspend_test(suspend_testlevel_t level)
{

	if (pm_suspend_testlevel_valid(level) && \
			(pm_test_level == level)) {
		pm_raw("suspend test(%d): wakeup after 5s.\n",
				level);
		hal_mdelay(5000);
		return 1;
	}

	return 0;
}

int pm_test_standby_recording(void)
{
	return pm_testlevel_recording;
}

static void pm_test_set_recording(uint8_t val)
{
	pm_testlevel_recording = val;
	pm_warn("set pm_testlevel_recording: %d\n", pm_testlevel_recording);
}

static int cmd_set_record(int argc, char **argv)
{
	if ((argc != 2) || ((atoi(argv[1]) != 0) && (atoi(argv[1]) != 1))) {
		pm_err("%s: Invalid params for pm_set_record\n", __func__);
		return -EINVAL;
	}

	pm_test_set_recording(atoi(argv[1]));

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_set_record, pm_set_record, pm_test_tools)

#ifdef CONFIG_COMPONENTS_PM_TEST_TOOLS
static hal_spinlock_t pm_testtools_lock;

/* standby_stress */
static TaskHandle_t StandbyStress_xHandle;
static unsigned int times = 10000;
static unsigned int time_to_suspend_ms = 5000;
static unsigned int time_to_wakeup_ms = 5000;
static volatile unsigned int test_cnt = 0;
static volatile unsigned int test_cnt_save = 0;
static unsigned int test_irq_set_in_finish = 0;

#define INIT_IRQ	(-32)
/* test_wakesrc */
int test_irq = INIT_IRQ;
struct pm_wakesrc *test_ws = NULL;

int pm_test_tools_notify_cb(suspend_mode_t mode, pm_event_t event, void *arg)
{
	switch (event) {
	case PM_EVENT_PERPARED:
		break;
	case PM_EVENT_FINISHED:
		hal_spin_lock(&pm_testtools_lock);
		test_cnt ++;
		hal_spin_unlock(&pm_testtools_lock);

		if (test_irq_set_in_finish) {
			pm_warn("enable wakesrc(%p) test_irq: %d\n", test_ws, test_irq);
			pm_set_wakeirq(test_ws);
		}
		break;
	default:
		break;
	}

	return 0;
}

static pm_notify_t pm_test_tools = {
	.name = "pm_test_tools",
	.pm_notify_cb = pm_test_tools_notify_cb,
	.arg = NULL,
};

static void standby_stress_usage()
{
	char buff[] = \
	"standby_stress <times> <period to suspend(ms)> <period to wakeup(ms)>\n"
	"for example: standby_stress 10000 3000 3000\n"
	;

	printf("%s", buff);
}

static void pm_standby_stress_task(void *nouse)
{
	int ret;
	int cnt = 1;
	int irq;

	pm_test_set_recording(1);
	printf("===== standby_stress start =====\n");
	while (cnt <= times) {
		while(pm_state_get() != PM_STATUS_RUNNING);
		hal_spin_lock(&pm_testtools_lock);
		test_cnt_save = test_cnt;
		hal_spin_unlock(&pm_testtools_lock);
		printf("===== standby_stress cnt: %d run atfter %dms... =====\n", cnt, time_to_suspend_ms);
		hal_msleep(time_to_suspend_ms);

		ret = pm_trigger_suspend(PM_MODE_STANDBY);
		if (ret) {
			printf("===== %s: trigger suspend failed, return %d =====\n", __func__, ret);
		} else {
			if (pm_state_get() == PM_STATUS_RUNNING) {
				/* if status hasn't change, wait this round complete, or may this round completed already. */
				while(pm_state_get() == PM_STATUS_RUNNING);
			}
		}

		while(pm_state_get() != PM_STATUS_RUNNING);
		pm_hal_get_wakeup_irq(&irq);
		printf("===== pm_wakeup_irq: %d =====\n", irq);
		printf("===== standby_stress cnt: %d complete =====\n", cnt);
		cnt++;
	}
	printf("===== standby_stress end =====\n");
	pm_test_set_recording(0);

	vTaskDelete(NULL);
}

static int cmd_standby_stress(int argc, char **argv)
{
	BaseType_t xReturned;

	if (argc >= 2)
		times = (unsigned int)atoi(argv[1]);
	if (argc >= 3)
		time_to_suspend_ms = (unsigned int)atoi(argv[2]);
	if (argc >= 4)
		time_to_wakeup_ms = (unsigned int)atoi(argv[3]);

	if ((argc > 4) || (times < 0) || (time_to_suspend_ms < 0) || (time_to_wakeup_ms <= 0)) {

		printf("%s: Invalid params for standby_stress\n", __func__);
		standby_stress_usage();
		return -EINVAL;
	}

	printf("standby stress\ntimes: %d\ntime_to_suspend_ms: %d\ntime_to_wakeup_ms: %d\n",
		times, time_to_suspend_ms, time_to_wakeup_ms);

	pm_hal_set_time_to_wakeup_ms(time_to_wakeup_ms);

	xReturned = xTaskCreate(pm_standby_stress_task, "standby_stress", (10 * 1024) / sizeof(StackType_t), NULL,
		PM_TASK_PRIORITY, &StandbyStress_xHandle);
	if (pdPASS != xReturned) {
		printf("create thread failed\n");
		return -EPERM;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_standby_stress, standby_stress, pm_test_tools)


#ifndef CONFIG_ARCH_DSP
static void pm_task_timer_handler(void *data)
{
	printf("pm_task timer callback\n");
}

static void pm_task_taste(void *data)
{
	while(1) {
		printf("pm_task_taste...\n");
		hal_msleep(1000);
	}
}

static TaskHandle_t pm_task_taste_xHandle;
static osal_timer_t task_timer = NULL;
static int cmd_pm_task_taste(int argc, char **argv)
{
	BaseType_t xReturned;
	hal_status_t ret;
	int err;

	if ((argc != 2) || (atoi(argv[1]) > 2) || (atoi(argv[1]) < 0)) {
		pm_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	if (atoi(argv[1]) == 2) {
		err = pm_task_attach_timer(task_timer, NULL, 0);
		if (err)
			printf("pm_task detach timer failed, return %d\n", err);
		printf("timer detaches\n");

		err = pm_task_attach_timer(task_timer, NULL, 1);
		if (err)
			printf("pm_task detach timer failed, return %d\n", err);
		printf("timer attaches\n");

		err = pm_task_attach_timer(task_timer, NULL, 0);
		if (err)
			printf("pm_task detach timer failed, return %d\n", err);
		printf("timer detaches\n");

		return err;	
	}


	if (atoi(argv[1]) == 0) {
		if (!task_timer || !pm_task_taste_xHandle) {
			printf("timer or pm_task has not resgitered\n");
			return -EINVAL;
		} else {
			err = pm_task_attach_timer(task_timer, pm_task_taste_xHandle, 0);
			if (err)
				printf("pm_task detach timer failed, return %d\n", err);
			return err;
		}
	}

	xReturned = xTaskCreate(pm_task_taste, "pm_task_taste", (10 * 1024) / sizeof(StackType_t), NULL,
		PM_TASK_PRIORITY, &pm_task_taste_xHandle);
	if (pdPASS != xReturned) {
		printf("create pm_task taste failed\n");
		return -EPERM;
	}

	err = pm_task_register(pm_task_taste_xHandle, PM_TASK_TYPE_SYS);
	if (err) {
		printf("pm_task register failed\n");
		return err;
	}

	task_timer = osal_timer_create("pm_task timer", pm_task_timer_handler, NULL,
			MS_TO_OSTICK(300), OSAL_TIMER_FLAG_PERIODIC);
	ret = osal_timer_start(task_timer);
	if (ret != HAL_OK) {
		printf("create pm_task timer failed, return: %d\n", ret);
		return -EFAULT;
	}

	err = pm_task_attach_timer(task_timer, pm_task_taste_xHandle, 1);
	if (ret) {
		printf("pm_task attach timer failed, return: %d\n", err);
		return err;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_task_taste, pm_task_taste, pm_test_tools)
#endif

static int cmd_pm_init_wakesrc(int argc, char **argv)
{
	int type;
	int irq;
	int ret = 0;

	if (test_irq != INIT_IRQ || test_ws) {
		printf("test_ws has registered\n");
		return -EFAULT;
	}

	if (argc != 3 || !(wakesrc_type_valid(atoi(argv[2])))) {
		printf("Invlaid para\n");
		return -EINVAL;
	}

	irq = atoi(argv[1]);
	type = atoi(argv[2]);
	printf("prepare to init test_ws irq: %d, ws type: %d\n", irq, type);

	test_ws = pm_wakesrc_register(irq, "test_ws", type);
	if (!test_ws) {
		printf("register failed\n");
		return -EFAULT;
	} else {
		test_irq = irq;
		printf("test_ws register: 0x%p, irq: %d\n", test_ws, test_irq);
	}

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_init_wakesrc, pm_init_wakesrc, pm_test_tools)

static int cmd_pm_deinit_wakesrc(int argc, char **argv) {
	int ret;

	if (test_irq == INIT_IRQ || !test_ws) {
		printf("test_ws is not registered\n");
		return -EFAULT;
	}

	ret = pm_wakesrc_unregister(test_ws);
	if (ret) {
		printf("unregister failed, ret: %d\n", ret);
		return -EFAULT;
	} else {
		printf("test_ws unregister ok\n");
		test_irq = INIT_IRQ;
		test_ws = NULL;
	}

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_deinit_wakesrc, pm_deinit_wakesrc, pm_test_tools)

static int cmd_pm_enable_wakesrc(int argc, char **argv) {
	int ret = 0;

	if (test_irq == INIT_IRQ || !test_ws) {
		printf("test_ws is not registered\n");
		return -EFAULT;
	}

	ret = pm_set_wakeirq(test_ws);
	if (ret)
		printf("enable test_ws failed\n");
	else
		printf("enable test_ws irq: %d\n", test_irq);

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_enable_wakesrc, pm_enable_wakesrc, pm_test_tools)

static int cmd_pm_disable_wakesrc(int argc, char **argv) {
	int ret = 0;

	if (test_irq == INIT_IRQ || !test_ws) {
		printf("test_ws is not registered\n");
		return -EFAULT;
	}

	ret = pm_clear_wakeirq(test_ws);
	if (ret)
		printf("disable test_ws failed\n");
	else
		printf("disable test_ws irq: %d\n", test_irq);

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_disable_wakesrc, pm_disable_wakesrc, pm_test_tools)

static int cmd_pm_stay_awake(int argc, char **argv) {
	pm_stay_awake(test_ws);

	if (test_irq != INIT_IRQ || test_ws)
		printf("stay_awake action down\n");
	else
		printf("test_ws is not registered\n");

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_stay_awake, pm_stay_awake, pm_test_tools)

static int cmd_pm_relax(int argc, char **argv) {
	pm_relax(test_ws, PM_RELAX_WAKEUP);

	if (test_irq != INIT_IRQ || test_ws)
		printf("relax down\n");
	else
		printf("test_ws is not registered\n");

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_relax, pm_relax, pm_test_tools)

static int cmd_pm_soft_wakeup(int argc, char **argv) {
	int action;
	int keep_ws_enabled;
	int ret = 0;

	if (test_irq == INIT_IRQ || !test_ws) {
		printf("test_ws is not registered\n");
		return -EFAULT;
	}

	if (argc != 3 || !(wakesrc_action_valid(atoi(argv[1])))) {
		printf("Invlaid para\n");
		return -EINVAL;
	}

	action = atoi(argv[1]);
	keep_ws_enabled = atoi(argv[2]);
	printf("soft wakeup action: %d, keep_ws_enabled: %d\n", action, keep_ws_enabled);

	if (action == PM_WAKESRC_ACTION_WAKEUP_SYSTEM)
		ret = pm_wakesrc_soft_wakeup(test_ws, PM_WAKESRC_ACTION_WAKEUP_SYSTEM, keep_ws_enabled);
	else if (action == PM_WAKESRC_ACTION_SLEEPY)
		ret = pm_wakesrc_soft_wakeup(test_ws, PM_WAKESRC_ACTION_SLEEPY, keep_ws_enabled);

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_soft_wakeup, pm_soft_wakeup, pm_test_tools)

static int cmd_test_irq_set_in_finish(int argc, char **argv)
{
	if ((argc != 2) || ((atoi(argv[1]) != 0) && (atoi(argv[1]) != 1))) {
		pm_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	test_irq_set_in_finish = atoi(argv[1]);
	pm_warn("set test_irq_set_in_finish: %d\n", test_irq_set_in_finish);

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_test_irq_set_in_finish, test_irq_set_in_finish, pm_test_tools)

#ifdef CONFIG_DRIVERS_RTC
static int cmd_pm_set_rtc_alarm(int argc, char **argv)
{
	unsigned int enable = 1;
	unsigned int sec = 0;
	unsigned int min = 0;
	struct rtc_time rtc_tm;
	struct rtc_wkalrm wkalrm;

	if ((argc != 3) || (atoi(argv[1]) > 60) || (atoi(argv[2]) > 60)) {
		pm_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	min = atoi(argv[1]);
	printf("min: %d\n", sec);
	sec = atoi(argv[2]);
	printf("sec: %d\n", sec);

	if (hal_rtc_gettime(&rtc_tm))
	{
		printf("sunxi rtc gettime error\n");
	}

	wkalrm.enabled = 1;
	wkalrm.time = rtc_tm;
	if (min >= (60 - rtc_tm.tm_min)) {
		wkalrm.time.tm_hour += 1;
		wkalrm.time.tm_min = min - (60 - rtc_tm.tm_min);
	} else {
		wkalrm.time.tm_min = min + rtc_tm.tm_min;
	}

	if (sec >= (60 - rtc_tm.tm_sec)) {
		wkalrm.time.tm_min += 1;
		wkalrm.time.tm_sec = sec - (60 - rtc_tm.tm_sec);
	} else {
		wkalrm.time.tm_sec = sec + rtc_tm.tm_sec;
	}

	 if (hal_rtc_setalarm(&wkalrm))
	 {
		printf("sunxi rtc setalarm error\n");
	 }

	hal_rtc_alarm_irq_enable(enable);

	printf("alarm %ds later\n", sec);

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_rtc_alarm, pm_set_rtc_alarm, pm_test_tools)
#endif /* CONFIG_DRIVERS_RTC */

#ifdef CONFIG_DRIVERS_GPIO
static hal_irqreturn_t gpio_callback(void *dev)
{
	struct pm_wakesrc *ws;

	ws = pm_wakesrc_find_registered_by_irq(GPIOA_IRQn);

	if (ws)
		pm_wakecnt_inc(ws);

	return 0;
}

static int cmd_pm_set_gpio(int argc, char **argv)
{
	uint32_t irq_no;

	if (hal_gpio_pinmux_set_function(GPIO_PA19, GPIO_MUXSEL_EINT)) {
		printf("pm test fail to set gpio function\n");
		return -EFAULT;
	}

	if (hal_gpio_to_irq(GPIO_PA19, &irq_no) < 0) {
		printf("pm test fail to request gpio to irq\n");
		return -EFAULT;
	}

	if (hal_gpio_irq_request(irq_no, gpio_callback, IRQ_TYPE_EDGE_RISING, NULL) < 0) {
		printf("pm test fail to request irq\n");
		return -EFAULT;
	}

	if (hal_gpio_irq_enable(irq_no)) {
		printf("pm test fail to enable gpio irq\n");
		return -EFAULT;
	}

	printf("set gpio funcion to EINT ok\n");

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_gpio, pm_set_gpio, pm_test_tools)
#endif /* CONFIG_DRIVERS_GPIO */

#ifdef CONFIG_DRIVERS_LPUART
#define WAKEUP_LEN 1

static void compare_callback(void *arg)
{
	printf("data compare success!\n");
}

static int cmd_pm_set_lpuart(int argc, char **argv)
{
	int ret = 0;
	char cmp[WAKEUP_LEN + 1] = "a";
	lpuart_port_t lpuart_port = LPUART_0;
	_lpuart_config_t lpuart_config;

	ret = hal_lpuart_init(lpuart_port);
	if (ret) {
		printf("hal_lpuart_init failed\n");
		return -1;
	}

	lpuart_multiplex(lpuart_port, UART_0);
	if (ret) {
		printf("lpuart_multiplex failed\n");
		return -1;
	}

	lpuart_config.word_length = LPUART_WORD_LENGTH_8,
	lpuart_config.msb_bit     = LPUART_MSB_BIT_0,
	lpuart_config.parity      = LPUART_PARITY_NONE,
	lpuart_config.baudrate = LPUART_BAUDRATE_9600;
	ret = hal_lpuart_control(lpuart_port, (void *)&lpuart_config);
	if (ret) {
		printf("hal_lpuart_control failed\n");
		return -1;
	}

	hal_lpuart_rx_cmp(lpuart_port, WAKEUP_LEN, (uint8_t *)cmp);
	if (ret) {
		printf("hal_lpuart_rx_cmp failed\n");
		return -1;
	}
	/*
	ret = hal_lpuart_enable_rx_data(lpuart_port, NULL, NULL);
	if (ret) {
		printf("hal_lpuart_enable_rx_data failed\n");
		return -1;
	}
	*/
	hal_lpuart_enable_rx_cmp(lpuart_port, compare_callback, NULL);
	if (ret) {
		printf("hal_lpuart_enable_rx_cmp failed\n");
		return -1;
	}

	printf("set lpuart %d ok\n", lpuart_port);

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_lpuart, pm_set_lpuart, pm_test_tools)
#endif

int pm_test_tools_init(void)
{
	int ret;

	ret = pm_notify_register(&pm_test_tools);
	if (ret < 0) {
		printf("%s: register pm_notify failed\n", __func__);
		return ret;
	}

	hal_spin_lock_init(&pm_testtools_lock);

	return 0;
}
#endif /* CONFIG_COMPONENTS_PM_TEST_TOOLS */
