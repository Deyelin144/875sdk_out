#ifdef CONFIG_KERNEL_FREERTOS
#include <FreeRTOS.h>
#else
#error "PM do not support the RTOS!!"
#endif
#include <task.h>
#include <errno.h>

#include <string.h>
#include <console.h>
#include <hal_time.h>
#include <inttypes.h>
#include <hal_gpio.h>
#include "pm_debug.h"
#include "pm_base.h"

#include "sunxi_hal_watchdog.h"

#ifdef CONFIG_COMPONENTS_PM_CORE_M33
#include <arch/arm/mach/sun20iw2p1/irqs-sun20iw2p1.h>
#endif

#ifdef CONFIG_COMPONENTS_PM_DEBUG_TOOLS

#ifdef CONFIG_COMPONENTS_PM_RECORD_TIME
struct pm_time_type {
	const char *name;
	const char *info;
};

struct pm_time {
	uint32_t try;
	uint32_t time;
	uint64_t starttime;
	uint64_t endtime;
};

struct pm_time pm_time_record_tb[PM_TIME_MAX];

struct pm_time_type const_time_type_string[PM_TIME_MAX] = {
	{"FREEZE_ATONCE", "freeze task earlier",},
	{"PLAT_VALID", "",},
	{"PLAT_PRE_BEGIN", "",},
	{"FREEZE_APP", "freeze all task",},
	{"FREEZE_SYS", "",},
#ifdef CONFIG_COMPONENTS_PM_CORE_M33
	{"PLAT_BEGIN", "sync susbsys stand",},
#else
	{"PLAT_BEGIN", "",},
#endif
	{"DEV_PREPARED", ""},
	{"DEV_SUSPEND", ""},
	{"PLAT_PREPARE", ""},
	{"DEV_SUSPEND_LATE", ""},
	{"DEV_SUSPEND_NOIRQ", ""},
	{"DEV_SYSCORE_SUSPEND", ""},
	{"PLAT_PREPARE_LATE", ""},
	{"PLAT_SUSPEND_ENTER", ""},
	{"VIRT_LOG_SUSPEND", ""},
	{"PLAT_RESUME_ENTER", ""},
	{"PLAT_WAKE", ""},
	{"DEV_SYSCORE_RESUME", ""},
	{"DEV_RESUME_NOIRQ", ""},
	{"DEV_RESUME_EARLY", ""},
	{"PLAT_AGAIN", ""},
	{"PLAT_FINISH", ""},
	{"DEV_RESUME", ""},
	{"DEV_COMPLETE", ""},
#ifdef CONFIG_COMPONENTS_PM_CORE_M33
	{"PLAT_END", "sync susbsys resume"},
#else
	{"PLAT_END", ""},
#endif
	{"PLAT_AGAIN_LATE", ""},
	{"RESTORE_SYS", ""},
	{"RESTORE_APP", "restore all task"},
	{"RESTORE_ATONCE", "restore user task"},
	{"PLAT_RECOVER", ""},
#ifdef CONFIG_COMPONENTS_PM_CORE_M33
	{"RESUME_END", "m33 report finish"},
#else
	{"RESUME_END", "notify m33 finish"},
#endif
	{"VIRT_LOG_RESUME", ""},
};


void pm_debug_get_starttime (pm_time_type_t type, uint64_t time)
{
	pm_time_record_tb[type].starttime = time;
}

void pm_debug_record_time (pm_time_type_t type, uint64_t time)
{
	pm_time_record_tb[type].endtime = time;
	pm_time_record_tb[type].try++;
	pm_time_record_tb[type].time = (pm_time_record_tb[type].endtime - pm_time_record_tb[type].starttime) / 1000ul;
}

void pm_time_record_clr (void)
{
	memset(pm_time_record_tb, 0, sizeof(pm_time_record_tb));
}

static int cmd_pm_showtime(int argc, char **argv)
{
	uint32_t suspend_time = 0, resume_time = 0;
	printf("------------------------ suspend ---------------------------\n");
	for(int i = 0; i < PM_TIME_PLAT_RESUME_ENTER; i++) {
		suspend_time += pm_time_record_tb[i].time;
		pm_stage("%-20s after %-8d usecs, try:%-4d #%-25s\n",
		const_time_type_string[i].name,
		pm_time_record_tb[i].time,
		pm_time_record_tb[i].try,
		const_time_type_string[i].info);
	}
	printf("suspend use %dus\n", suspend_time);
	printf("------------------------- resueme --------------------------\n");
	for(int i = PM_TIME_PLAT_RESUME_ENTER; i < PM_TIME_MAX; i++) {
		resume_time += pm_time_record_tb[i].time;
		pm_stage("%-20s after %-8d usecs, try:%-4d #%-25s\n",
		const_time_type_string[i].name,
		pm_time_record_tb[i].time,
		pm_time_record_tb[i].try,
		const_time_type_string[i].info);
	}
	printf("resume main time %dus\n", resume_time);
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_showtime, pm_showtime, pm_test_tools);
#endif

#ifdef CONFIG_COMPONENTS_PM_RECORD_GPIO
static uint8_t pm_gpio_debug_en = 0;

static int cmd_pm_gpio_status_debug(int argc, char **argv)
{
	pm_log("enable pm gpio debug\n");
	pm_gpio_debug_en = 1;

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_gpio_status_debug, pm_gpio_debug, pm tools)

void pm_debug_gpio_status(void)
{
	if(!pm_gpio_debug_en) return;
	pm_gpio_debug_en = 0;
	gpio_muxsel_t function_index_get;
	gpio_pull_status_t pull_get;
	pm_log("gpio         Hiz    PULL\n");
	for(int i = GPIO_PA0; i < GPIO_PA29; i++){
		hal_gpio_get_pull(i, &pull_get);
		hal_gpio_pinmux_get_function(i, &function_index_get);
		if(function_index_get != GPIO_MUXSEL_DISABLED){
			pm_log("gpio-pa%-02d    %-02d     %-02d\n", i - GPIO_PA0, 0, pull_get == GPIO_PULL_DOWN_DISABLED ? 1 : 0);
			continue;
		}
		if(pull_get != GPIO_PULL_DOWN_DISABLED){
			pm_log("gpio-pa%-02d    %-02d     %-02d\n", i - GPIO_PA0, 1, 0);
		}
	}
	for(int i = GPIO_PB0; i < GPIO_PB3; i++){
		hal_gpio_pinmux_get_function(i, &function_index_get);
		if(function_index_get != GPIO_MUXSEL_DISABLED){
			pm_log("gpio-pb%-02d    %-02d     %-02d\n", i - GPIO_PB0, 0, pull_get == GPIO_PULL_DOWN_DISABLED ? 1 : 0);
			continue;
		}
		if(pull_get != GPIO_PULL_DOWN_DISABLED){
			pm_log("gpio-pb%-02d    %-02d     %-02d\n", i - GPIO_PB0, 1, 0);
		}
	}
}
#endif

#ifdef CONFIG_COMPONENTS_PM_RECORD_REG
static uint8_t reg_capture_en = 0;

#define __prcm_base_start    0x40050000
#define __prcm_base_end      0x40050230
#define __ccmu_on_base_start 0x4004c400
#define __ccmu_on_base_end   0x4004c4f0
#define __pmu_base_start     0x40051400
#define __pmu_base_end       0x40051584
#define __rtc_base_start     0x40051000
#define __rtc_base_end       0x40051064

uint32_t register_tb[8] = {
	__prcm_base_start, __prcm_base_end,
	__ccmu_on_base_start, __ccmu_on_base_end,
	__pmu_base_start, __pmu_base_end,
	__rtc_base_start, __rtc_base_end
};

uint8_t *register_name[4] = {"prcm", "ccmu", "pmu", "rtc"};

static uint32_t *reg_capture_addr = NULL;

uint32_t *pm_reg_capture_addr_set(void)
{
	return reg_capture_addr;
}

static void show_help(void)
{
	pm_log("Usage :\n"
			"pm_reg show\n"
			"pm_reg stop\n"
			"pm_reg cpature <val>\n"
			"               1: prcm\n"
			"               2: ccmu\n"
			"               3: pmu\n"
			"               4: rtc\n"
			"pm_reg cpature <begin> <end>\n");
}

static int cmd_pm_show_register(int argc, char **argv)
{
	uint8_t debug_level;
	static uint8_t *reg_name;
	uint32_t *reg_show_addr;
	uint32_t reg_num;
	static uint32_t reg_begin, reg_end;
	if (argc < 2 || argc > 5) {
		show_help();
		goto failed;
	}
	if (!strcmp(argv[1], "cpature")) {
		reg_capture_en = 1;
		pm_log("reg capture %s\n", "begin");
		if (argc == 3) {
			debug_level = atoi(argv[2]) - 1;
			if (debug_level > 3) debug_level = 3;
			reg_begin = register_tb[2 * debug_level];
			reg_end = register_tb[2 * debug_level + 1];
			reg_name = register_name[debug_level];
		} else if (argc == 4) {
			reg_begin = strtoul(argv[2], NULL, 16);
			reg_end = strtoul(argv[3], NULL, 16);
			if (reg_end < reg_begin || reg_end == 0xffffffff || reg_begin == 0xffffffff || reg_end % 4 || reg_begin % 4) {
				pm_log("parameters are illegal, please check\n");
				show_help();
				goto failed;
			}
			reg_name = "other";
		}
		reg_num = (reg_end - reg_begin) / 4 + 1;
		if (reg_capture_addr) {
			free(reg_capture_addr);
			reg_capture_addr = NULL;
		}
		reg_capture_addr = (uint32_t *)malloc((reg_num + 2) * sizeof(uint32_t));
		if(reg_capture_addr == NULL){
			pm_log("reg capture buffer malloc failed\n");
			goto failed;
		}
		memset((uint8_t *)reg_capture_addr, 0x00, (reg_num + 2) * sizeof(uint32_t));
		reg_capture_addr[0] = reg_begin;
		reg_capture_addr[1] = reg_end;
		pm_log("cpature reg[%s]:(0x%08x) - (0x%08x):\n", reg_name, reg_begin, reg_end);
	} else if (!strcmp(argv[1], "stop")) {
		pm_log("reg capture %s\n", "stop");
		reg_capture_en = 0;
		if (reg_capture_addr) {
			free(reg_capture_addr);
			reg_capture_addr = NULL;
		}
	} else if (!strcmp(argv[1], "show")) {
		if(!reg_capture_en) return -1;
		reg_show_addr = reg_capture_addr;
		reg_show_addr += 2;
		pm_log("show reg[%s]:(0x%08x) - (0x%08x):\n", reg_name, reg_begin, reg_end);
		for (uint32_t j = reg_capture_addr[0]; j <= reg_capture_addr[1]; j += 4) {
			printf("reg[0x%08x] : 0x%08x ", j, *(reg_show_addr++));
			if (((j - reg_capture_addr[0]) / 4 + 1) % 3 == 0)
				printf("\n");
		}
		printf("\n");
	} else {
		show_help();
		goto failed;
	}

	return 0;
failed:
	return -1;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_show_register, pm_reg, pm tools)

#endif

#ifdef CONFIG_COMPONENTS_PM_WATCHDOG

static uint8_t g_pm_wdg_en = 0;

#ifdef CONFIG_COMPONENTS_PM_CORE_M33
static hal_irqreturn_t pm_watchdog_callback(void *data)
{
#define DSP_BOOT_FLAG_REG	(0x400501cc)
#define RV_BOOT_FLAG_REG	(0x400501d8)
	uint32_t val;

	writel(0x429b0000, DSP_BOOT_FLAG_REG);
	writel(0x429b0000, RV_BOOT_FLAG_REG);
	/* clear pending */
	val = readl(0x40020404);
	writel(val, 0x40020404);

	return 0;
}
#endif

void pm_debug_watchdog_feed(void)
{
	hal_watchdog_feed();
}

void pm_debug_watchdog_stop(int timeout)
{
	hal_watchdog_stop(timeout);
}

void pm_debug_watchdog_start(int timeout)
{
	hal_watchdog_start(timeout);
}

void pm_debug_use_watchdog(void)
{
	if (!g_pm_wdg_en)
		return;
	hal_watchdog_stop(10);
	hal_watchdog_start(10);
#ifdef CONFIG_COMPONENTS_PM_CORE_M33
#define M33_RWDOG_IRQ_EN 0x40020400
	uint32_t irq = CPU_SYS_IRQ_OUT0_IRQn;
	writel(0x1, M33_RWDOG_IRQ_EN);
	hal_request_irq(irq, pm_watchdog_callback, "pm_wdg", NULL);
	hal_enable_irq(irq);
#endif
}

static void pm_wdg_show_help(void)
{
	pm_log("Usage :\n"
			"pm_wdg val\n"
			"		1: enable\n"
			"		0: disable\n");
}

static int cmd_pm_debug_use_watchdog(int argc, char **argv)
{
	uint8_t pm_wdg_en;
	if (argc != 2) {
		pm_wdg_show_help();
		goto failed;
	}
	pm_wdg_en = atoi(argv[1]);
	if (pm_wdg_en) {
		g_pm_wdg_en = 1;
		hal_watchdog_suspend_sta_set(1);
	} else {
		g_pm_wdg_en = 0;
		hal_watchdog_suspend_sta_set(0);
	}
	return 0;
failed:
	return -1;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_debug_use_watchdog, pm_wdg, pm tools)

void pm_clear_boot_flag(void)
{
#ifdef CONFIG_COMPONENTS_PM_CORE_M33
#define RV_BOOT_FLAG 0x400501d8
#define DSP_BOOT_FLAG 0x400501cc
	if (readl(RV_BOOT_FLAG) == 1)
		writel(0x429b0000, RV_BOOT_FLAG);
	if (readl(DSP_BOOT_FLAG) == 1)
		writel(0x429b0000, DSP_BOOT_FLAG);
#endif
}

#endif

void pm_debug_init(void)
{
	pm_clear_boot_flag();
}

#endif
