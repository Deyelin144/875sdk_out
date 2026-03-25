/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arch/arm/mach/sun20iw2p1/irqs-sun20iw2p1.h>
#include <io.h>
#include <hal_time.h>
#include <hal_osal.h>
#include <console.h>
#include <hal_prcm.h>
#include <hal_gpio.h>
#ifdef CONFIG_DRIVER_SYSCONFIG
#include <script.h>
#include <hal_cfg.h>
#endif
#ifdef CONFIG_DRIVERS_EFUSE
#include <sunxi_hal_efuse.h>
#endif

#include <pm_suspend.h>
#include <pm_debug.h>
#include <pm_wakecnt.h>
#include <pm_wakesrc.h>
#include <pm_systeminit.h>
#include <pm_subsys.h>
#include <pm_syscore.h>
#include <pm_devops.h>
#include <pm_testlevel.h>
#include <pm_wakesrc.h>
#include <pm_wakecnt.h>
#include <pm_wakelock.h>
#include <pm_state.h>
#include <pm_init.h>

#include <pm_m33_platops.h>
#include <pm_m33_cpu.h>
#include <pm_m33_wakesrc.h>
#include <pm_rpcfunc.h>

#include <head.h>
#include <pm_firmware.h>

#ifdef CONFIG_DRIVERS_LSPSRAM
#include "sunxi_hal_lspsram.h"
#endif

#if 0
static struct arm_CMX_core_regs vault_arm_registers;
#endif

#undef   PM_DEBUG_MODULE
#define  PM_DEBUG_MODULE  PM_DEBUG_M33

#if 0
void dump_reg(void)
{
	volatile uint32_t s,e,i;

	for (s=0x400501c0,e=0x400501c8; s<=e; s+=4) {
		pm_raw("0x%08x: 0x%08x\n", s, readl(s));
	}
	hal_mdelay(10000);
}
#endif

#define MODE_BAK_INITIALIZATION		(PM_MODE_ON)

#define GPRCM_RTC_LDO_CTRL_REG		(0x40050020)
#define GPRCM_AON_LDO_CTRL_REG		(0x40050040)
#define GPRCM_EXT_LDO_CTRL_REG		(0x40050024)

#define SUN20I_CHIPV_START	(22)
#define SUN20I_CHIPV_LEN	(6)
#define SUN20I_CHIPV_B		(11)

struct pm_lpsram {
	lpsram_para_t *para;
	uint32_t inited;
};
static struct pm_lpsram lpsram;

struct pm_hpsram {
	 __dram_para_t para;
	uint32_t inited;
};
static struct pm_hpsram hpsram;

static uint32_t subsys_pwr_remain = 0;
static uint32_t subsys_again_abort = 0;
static uint32_t pm_mode_keep_changed = 0;
static uint32_t common_syscore_suspend_enabled = 1;
static suspend_mode_t suspend_mode_bak = MODE_BAK_INITIALIZATION;

static uint32_t chipv = 0;
static uint32_t rccal_en = 1;

static SemaphoreHandle_t platops_mutex;

#ifdef CONFIG_PM_SIMULATED_RUNNING
int wakeup_wait_loop(void)
{
	int i = 0;
	uint32_t wakesrc;

	pm_dbg("pending:0x%08x\n", pm_wakesrc_read_pending());
	pm_dbg("active:0x%08x\n", pm_wakesrc_get_active());

	while (1) {
		wakesrc = pm_wakesrc_check_irqs();
		if (wakesrc) {
			pm_log("wakeup(0x%08x)...\n", wakesrc);
			return 0;
		}
	}
}

int wakeup_check_callback(void)
{
	return pm_wakesrc_check_irqs();
}

static void nvic_mark_all_irq(void)
{
	uint32_t addr = 0xe000e100;
	volatile uint32_t i = 300000;

	for (;addr <= 0xe000e13c; addr+=4) {
		writel(0, addr);
	}

	//while (i--) ;
}

static void _cpu_sleep(suspend_mode_t mode)
{
	__cpu_sleep(0U);
}

static void _cpu_standby(suspend_mode_t mode)
{
	//nvic_mark_all_irq();
	__cpu_standby(0U);
}

static void _cpu_hibernation(suspend_mode_t mode)
{
	__cpu_hibernation(0U);
}

static int pm_m33_enter(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		_cpu_sleep(mode);
		break;
	case PM_MODE_STANDBY:
		_cpu_standby(mode);
		break;
	case PM_MODE_HIBERNATION:
		_cpu_hibernation(mode);
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#else
#define DRAM_PARA		"dram_para"
int pm_lpsram_para_init(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	int ret = 0;
	int32_t val = 0;

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_no_lpsram", &val, 1);
	if (!ret) {
#ifdef CONFIG_DRIVERS_LSPSRAM
		lpsram.inited = !(!!(uint32_t)val);
#else
		lpsram.inited = 0;
#endif
	} else {
		pm_err("get %s dram_no_lpsram failed, skip LSPSRAM standby\n", DRAM_PARA);
		lpsram.inited = 0;
		return -ENODEV;
	}
#endif

#if 0
/* ensure that psram init before pm */
#ifdef CONFIG_DRIVERS_LSPSRAM
	lpsram.para = psram_chip_get_para();
	if (!lpsram.para) {
		pm_err("get lspsram para failed, skip LSPSRAM standby\n");
		lpsram.inited = 0;
		return -ENODEV;
	} else {
		lpsram.inited = 1;
		return 0;
	}
#else
	pm_warn("lspsram does not exist, skip LSPSRAM standby\n");
	head->lpsram_inited = 0;

#endif
#endif
}

int pm_hpsram_para_init(void)
{
#ifdef CONFIG_DRIVER_SYSCONFIG
	int ret = 0;
	int32_t val = 0;

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_clk", &val, 1);
	if (!ret) {
		hpsram.para.dram_clk = (uint32_t)val;
		if (val == 0) {
			pm_warn("get %s dram_clk: %d. hspsram does not exist, skip HSPSRAM standby\n", DRAM_PARA, val);
			hpsram.inited = 0;
			return 0;
		}
	} else {
		pm_err("get %s dram_clk failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_type", &val, 1);
	if (!ret) {
		hpsram.para.dram_type = (uint32_t)val;
	} else {
		pm_err("get %s dram_type failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_zq", &val, 1);
	if (!ret) {
		hpsram.para.dram_zq = (uint32_t)val;
	} else {
		pm_err("get %s dram_zq failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_odt_en", &val, 1);
	if (!ret) {
		hpsram.para.dram_odt_en = (uint32_t)val;
	} else {
		pm_err("get %s dram_odt_en failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_para1", &val, 1);
	if (!ret) {
		hpsram.para.dram_para1 = (uint32_t)val;
	} else {
		pm_err("get %s dram_para1 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_para2", &val, 1);
	if (!ret) {
		hpsram.para.dram_para2 = (uint32_t)val;
	} else {
		pm_err("get %s dram_para2 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_mr0", &val, 1);
	if (!ret) {
		hpsram.para.dram_mr0 = (uint32_t)val;
	} else {
		pm_err("get %s dram_mr0 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_mr1", &val, 1);
	if (!ret) {
		hpsram.para.dram_mr1 = (uint32_t)val;
	} else {
		pm_err("get %s dram_mr1 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_mr2", &val, 1);
	if (!ret) {
		hpsram.para.dram_mr2 = (uint32_t)val;
	} else {
		pm_err("get %s dram_mr2 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_mr3", &val, 1);
	if (!ret) {
		hpsram.para.dram_mr3 = (uint32_t)val;
	} else {
		pm_err("get %s dram_mr3 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr0", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr0 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr0 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr1", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr1 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr1 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr2", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr2 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr2 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr3", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr3 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr3 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr4", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr4 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr4 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr5", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr5 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr5 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr6", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr6 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr6 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr7", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr7 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr7 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr8", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr8 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr8 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr9", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr9 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr9 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr10", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr10 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr10 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr11", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr11 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr11 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr12", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr12 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr12 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	ret = hal_cfg_get_keyvalue(DRAM_PARA, "dram_tpr13", &val, 1);
	if (!ret) {
		hpsram.para.dram_tpr13 = (uint32_t)val;
	} else {
		pm_err("get %s dram_tpr13 failed, skip HSPSRAM standby\n", DRAM_PARA);
		hpsram.inited = 0;
		return -ENODEV;
	}

	/* for standby.bin. set bit[28] to 0 for resume, do not run readind and writing test */
	hpsram.para.dram_tpr13 &= ~(0x1 << 28);
	hpsram.inited = 1;

	return 0;

#else
	pm_warn("hspsram para does not exist. skip HSPSRAM standby\n");
	hpsram.inited = 0;

	return 0;
#endif
}

#define RTC_WUPTIMER_CTRL_REG		(0x40051508)
#define RTC_WUPTIMER_VAL_REG		(0x4005150c)
#define PM_PMU_REG_BASE				(0x40051400)
#define PM_RTC_IO_WAKE_EN			(PM_PMU_REG_BASE + 0x110)
#define PM_RTC_IO_WAKE_DEB_CLK		(PM_PMU_REG_BASE + 0x114)
#define PM_RTC_IO_WAKE_ST			(PM_PMU_REG_BASE + 0x118)
#define PM_RTC_IO_HOLD_CTRL			(PM_PMU_REG_BASE + 0x11c)
#define PM_RTC_IO_WAKE_GEN			(PM_PMU_REG_BASE + 0x120)
#define PM_RTC_IO_WUP_DEB_CYCLES0	(PM_PMU_REG_BASE + 0x124)
#define PM_RTC_IO_WUP_DEB_CYCLES1	(PM_PMU_REG_BASE + 0x128)
#define PM_PRCM_RCOSC_DET_REG		(0x40050140)

// #define PM_WAKEUP_SRC_WKIO0     HAL_BIT(0)                  /* 0x00000001 */
// #define PM_WAKEUP_SRC_WKIO1     HAL_BIT(1)                  /* 0x00000002 */
// #define PM_WAKEUP_SRC_WKIO2     HAL_BIT(2)                  /* 0x00000004 */
// #define PM_WAKEUP_SRC_WKIO3     HAL_BIT(3)                  /* 0x00000008 */
// #define PM_WAKEUP_SRC_WKIO4     HAL_BIT(4)                  /* 0x00000010 */
// #define PM_WAKEUP_SRC_WKIO5     HAL_BIT(5)                  /* 0x00000020 */
// #define PM_WAKEUP_SRC_WKIO6     HAL_BIT(6)                  /* 0x00000040 */
// #define PM_WAKEUP_SRC_WKIO7     HAL_BIT(7)                  /* 0x00000080 */
// #define PM_WAKEUP_SRC_WKIO8     HAL_BIT(8)                  /* 0x00000100 */
// #define PM_WAKEUP_SRC_WKIO9     HAL_BIT(9)                  /* 0x00000200 */
// #define PM_WAKEUP_SRC_WKTIMER   HAL_BIT(10)                 /* 0x00000400 */

typedef struct {
	int time_to_wakeup_ms;
	int wupio_set;
	int wakeup_src;
	struct pm_wakesrc *wup_src;
} _pmu_wup;
static _pmu_wup wup_handle = {0};

static hal_irqreturn_t wupirq_callback(void *data)
{
	volatile uint32_t val = 0x0;

	pm_wakecnt_inc(wup_handle.wup_src);
	pm_dbg("call %s\n", __func__);
	printf("*******wupirq_callback*********\n");
	/* wakeup io */
	val = readl(PM_RTC_IO_WAKE_ST);
	if (val & 0x3ff) {
		pm_err("wup io 0x%x\n", val);
		for(int i = 0; i <= PM_WAKEUP_SRC_WKIO9; i++){
			if(val & (1 << i)){
				wup_handle.wakeup_src = i;
				break;
			}
		}
		/*clear pending*/
		writel(0, PM_RTC_IO_WAKE_ST);
		writel(val, PM_RTC_IO_WAKE_ST);
		writel(0, PM_RTC_IO_WAKE_ST);
	}

	/*readl irq status and timer val*/
	val = readl(RTC_WUPTIMER_VAL_REG);
	if (val & (0x1 << 31)) {
		pm_err("wup timer 0x%x\n", val);

		/*clear pending*/
		writel(0x80000000, RTC_WUPTIMER_VAL_REG);
		while(readl(RTC_WUPTIMER_VAL_REG) != 0);

		wup_handle.wakeup_src = PM_WAKEUP_SRC_WKTMR;
	}

	if (wup_handle.wupio_set != 0) {
		// is wakeup ,no need wupio irq again
		/*disable wup io irq*/
		writel(0, PM_RTC_IO_WAKE_GEN);
	}

	if (wup_handle.time_to_wakeup_ms != 0) {
		// is wakeup ,no need wuptimer irq again
		/*disable wuptimer*/
		writel(0x0, RTC_WUPTIMER_CTRL_REG);
	}
	pm_dbg("call %s end\n", __func__);

	return 0;
}

static int pm_wup_register(void)
{
	int ret = 0;

	if (wup_handle.wup_src != NULL) {
		//writel(0x0, RTC_WUPTIMER_CTRL_REG);
		printf("wakesrc time_to_wakeup_ms already registered\n");
		return 0;
	}
		
	ret = hal_request_irq(AR200A_WUP_IRQn, wupirq_callback, "pm wup callback", NULL);
	if (ret) {
		pm_err("%s[%d]: request irq failed\n", __func__, __LINE__);
		return -EFAULT;
	}

	wup_handle.wup_src = pm_wakesrc_register(AR200A_WUP_IRQn, "wup", PM_WAKESRC_ALWAYS_WAKEUP);
	if (wup_handle.wup_src == NULL) {
		pm_err("%s[%d]: register wakesrc failed\n", __func__, __LINE__);
		return -EINVAL;
	}
	/* dsiable wuptimer */
	writel(0x0, RTC_WUPTIMER_CTRL_REG);
	hal_enable_irq(AR200A_WUP_IRQn);

	pm_set_wakeirq(wup_handle.wup_src);

	return 0;
}

static int pm_wup_unregister(void)
{
	if (wup_handle.wup_src == NULL)
		return -1;
	if ((wup_handle.time_to_wakeup_ms == 0) && (wup_handle.wupio_set == 0)) {
		pm_clear_wakeirq(wup_handle.wup_src);
		pm_wakesrc_unregister(wup_handle.wup_src);
		hal_disable_irq(AR200A_WUP_IRQn);
		hal_free_irq(AR200A_WUP_IRQn);
		wup_handle.wup_src = NULL;
		printf("wakesrc time_to_wakeup_ms unregistered\n");
	}

	return 0;
}

int pm_set_time_to_wakeup_ms(unsigned int ms)
{
	int ret;

	wup_handle.time_to_wakeup_ms = ms;

	if (wup_handle.time_to_wakeup_ms != 0) {
		if (pm_wup_register() < 0) {
			wup_handle.time_to_wakeup_ms = 0;
		}

		pm_warn("wakesrc time_to_wakeup_ms registered, type: ALWAYS\n");

	} else {
		pm_wup_unregister();

		pm_warn("wakesrc time_to_wakeup_ms unregistered\n");
	}

	pm_warn("set time_to_wakeup_ms: %d\n", wup_handle.time_to_wakeup_ms);

	return 0;
}

static int cmd_set_time_to_wakeup_ms(int argc, char **argv)
{
	int ret;
	int val = -1;

	if (argc != 2) {
		pm_err("%s[%d]: invalid param(argc:%d)\n", __func__, __LINE__, argc);
		return -EINVAL;
	}

	val = atoi(argv[1]);
	if (val < 0) {
		pm_err("%s[%d]: invalid param(val:%d)\n", __func__, __LINE__, val);
		return -EINVAL;
	}

	ret = pm_set_time_to_wakeup_ms((unsigned int)val);
	if (ret)
		pm_err("cmd set time_to_wakeup_ms failed\n");

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_set_time_to_wakeup_ms, time_to_wakeup_ms, pm tools)

struct wupio_map {
	uint32_t no;
	gpio_pin_t pin;
};

static struct wupio_map sun20i_wupio_map[10] = {
	{0, GPIO_PA11},
	{1, GPIO_PA12},
	{2, GPIO_PA13},
	{3, GPIO_PA14},
	{4, GPIO_PA18},
	{5, GPIO_PA19},
	{6, GPIO_PA20},
	{7, GPIO_PA21},
	{8, GPIO_PA22},
	{9, GPIO_PA23},
};

typedef enum {
	WUP_IO_DBC_FAST_MODE = 0,
	WUP_IO_DBC_SLOW_MODE = 1,
} wup_io_dbc_mode_t;

static void pm_gprcm_set_wup_io_dbc(uint32_t io, wup_io_dbc_mode_t wup_io_dbc_mode, uint32_t dbc_clk_sel, uint32_t dbc_cycle)
{
	uint32_t rdata32 = 0;

	/* step 1: configure mode & dbc_clk_sel */
	if(wup_io_dbc_mode == WUP_IO_DBC_SLOW_MODE)
		rdata32 = (dbc_clk_sel << 28) | (wup_io_dbc_mode << io);
	else
		rdata32 = (dbc_clk_sel << 24) | (wup_io_dbc_mode << io);
	writel(rdata32, PM_RTC_IO_WAKE_DEB_CLK);

	/* step 2: configure debounce cycle */
	if (io < 8){
		rdata32 = readl(PM_RTC_IO_WUP_DEB_CYCLES0);
		rdata32 &= ~(0xf << (io * 4));
		rdata32 |= (dbc_cycle & 0xf) << (io * 4);
		writel(rdata32, PM_RTC_IO_WUP_DEB_CYCLES0);
	} else {
		rdata32 = readl(PM_RTC_IO_WUP_DEB_CYCLES1);
		rdata32 &= ~(0xf << ((io - 8) * 4));
		rdata32 |= (dbc_cycle & 0xf) << ((io - 8) * 4);
		writel(rdata32, PM_RTC_IO_WUP_DEB_CYCLES1);
	}
}

/* time_s range 0-3 */
int pm_set_wupio(uint32_t val, uint32_t edge_mode, uint32_t time_s)
{
	uint32_t cycle;
	uint32_t clk_with_div;
	int ret;

	if ((val > 9) || (edge_mode > 1) || (time_s > 3)) {
		printf("invalid wupio param\n");
		return -EINVAL;
	}

	if (pm_wup_register() < 0) {
		printf("register wup irq err\n");
		return -EINVAL;
	}

	wup_handle.wupio_set |= (0x1 << val);

	/* set test gpio input and hold */
	if (hal_gpio_set_pull(sun20i_wupio_map[val].pin, GPIO_PULL_UP))
		printf("pm test fail to set gpio pull\n");
	if (hal_gpio_pinmux_set_function(sun20i_wupio_map[val].pin, GPIO_MUXSEL_IN))
		printf("pm test fail to set gpio function\n");

	hal_usleep(100);


	/* set wupio debounce, the maximum time is 16 * step.
	 * clk = LFCLK / 2^clk_sel
	 * cycle = clk * T_s
	 * cycle range 1-16, and the time step is 1 / clk.
	 */
	if ((time_s >= 1) && (time_s <= 3)) {
		/* set clk_sel to 13 and the clk will be closer to 5. time_sec range 1-3 */
		clk_with_div = ((readl(PM_PRCM_RCOSC_DET_REG) >> 8) & 0xfffff) * 10 / 8192;
		cycle = time_s * clk_with_div - 1;
		if (cycle > 15) {
			cycle = 15;
			printf("cycle exceed limit, set cycle to 16\n");
		}
		pm_gprcm_set_wup_io_dbc(val, WUP_IO_DBC_SLOW_MODE, 0xd, cycle);
		printf("clk_sel: 13, debounce cycle: %d\n", cycle + 1);
	} else {
		/* the wakeup threshold is closed to 200ms */
		pm_gprcm_set_wup_io_dbc(val, WUP_IO_DBC_FAST_MODE, 0xb, 0);//修改为0xb，在LFCLK为44KHz时，去抖约为45ms
		printf("clk_sel: 13, debounce cycle: 1\n");
	}

	hal_usleep(100);
	writel(readl(PM_RTC_IO_HOLD_CTRL) | (0x1 << val), PM_RTC_IO_HOLD_CTRL);

	/* set wupio irq cfg. edge mode */
	writel(0x1 << val, PM_RTC_IO_WAKE_ST);
	writel((readl(PM_RTC_IO_WAKE_EN) | (0x1 << val)) | (edge_mode << (val + 16)), PM_RTC_IO_WAKE_EN);
	writel(0x1, PM_RTC_IO_WAKE_GEN);

	printf("*********set wkio: %d**********\n", val);

	return 0;
}

int pm_clear_wupio(uint32_t val)
{
	uint32_t rdata32;
	int ret;

	if (val > 9) {
		printf("invalid wupio param\n");
		return -EINVAL;
	}

	writel(readl(PM_RTC_IO_HOLD_CTRL) & ~(0x1 << val), PM_RTC_IO_HOLD_CTRL);
	writel((readl(PM_RTC_IO_WAKE_EN) & ~(0x1 << val)), PM_RTC_IO_WAKE_EN);

	rdata32 = readl(PM_RTC_IO_WUP_DEB_CYCLES1);
	rdata32 &= ~(0xf << ((val - 8) * 4));
	writel(rdata32, PM_RTC_IO_WUP_DEB_CYCLES1);

	wup_handle.wupio_set &= ~(0x1 << val);

	pm_wup_unregister();

	return 0;
}

static int cmd_pm_set_wupio(int argc, char **argv)
{
	uint32_t wupio;
	uint32_t edge;
	uint32_t time_s;

	if ((argc != 4) || (atoi(argv[1]) < 0) || (atoi(argv[2]) < 0) || (atoi(argv[3]) < 0)) {
		printf("invalid param\n");
		return -EINVAL;
	}

	wupio = (uint32_t)atoi(argv[1]);
	edge = (uint32_t)atoi(argv[2]);
	time_s = (uint32_t)atoi(argv[3]);

	return pm_set_wupio(wupio, edge, time_s);
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_wupio, pm_set_wupio, pm_set_wupio)

int cmd_pm_clear_wupio(int argc, char **argv)
{
	uint32_t wupio;

	if ((argc != 2) || (atoi(argv[1]) < 0)) {
		printf("invalid param\n");
		return -EINVAL;
	}

	wupio = (uint32_t)atoi(argv[1]);

	return pm_clear_wupio(wupio);
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_clear_wupio, pm_clear_wupio, pm_test_tools)

int cmd_pm_get_wupsrc(int argc, char **argv)
{
	printf("wup wakeup src is 0x%x\n", wup_handle.wakeup_src);
	return wup_handle.wakeup_src;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_get_wupsrc, pm_get_wupsrc, pm_get_wupsrc)

uint32_t pm_get_rccal_wuptimer_en(void)
{
	uint32_t val;

	if (platops_mutex)
		xSemaphoreTake(platops_mutex, portMAX_DELAY);
	val = rccal_en;
	if (platops_mutex)
		xSemaphoreGive(platops_mutex);

	return val;
}

void pm_set_rccal_wuptimer_en(uint32_t val)
{
	if (platops_mutex)
		xSemaphoreTake(platops_mutex, portMAX_DELAY);
	if (val)
		rccal_en = 1;
	else
		rccal_en = 0;
	if (platops_mutex)
		xSemaphoreGive(platops_mutex);
}

static int cmd_pm_set_rccal_wuptimer_en(int argc, char **argv)
{
	int val = -1;

	if (argc != 2) {
		pm_err("%s[%d]: invalid param(argc:%d)\n", __func__, __LINE__, argc);
		return -EINVAL;
	}

	val = atoi(argv[1]);
	if (val < 0) {
		pm_err("%s[%d]: invalid param(val:%d)\n", __func__, __LINE__, val);
		return -EINVAL;
	}

	pm_set_rccal_wuptimer_en(val);

	printf("now rccal_en is: %d\n", pm_get_rccal_wuptimer_en());

	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_pm_set_rccal_wuptimer_en, pm_set_rccal_en, pm tools)

extern void arch_nvic_irq_set_priority(int32_t irq, uint32_t priority);
extern uint32_t arch_nvic_irq_get_priority(int32_t irq);
extern int get_cpu_freq(uint32_t *cpu_freq);
extern uint32_t arch_nvic_get_enable_irq(int32_t irq);
static uint32_t nvic_irq_enable_status_save = 0;
static uint32_t nvic_irq_msgbox_priority_save = 0;
static uint32_t nvic_irq_priority_save[PM_WAKEUP_SRC_MAX] = {0};

static int pm_m33_enter(suspend_mode_t mode)
{
	pm_debug_get_starttime (PM_TIME_PLAT_SUSPEND_ENTER, hal_gettime_ns());
	int ret = 0;
	uint32_t wakesrc;
	uint32_t rtc_ldo_ctrl_bak;
	uint32_t aon_ldo_ctrl_bak;
	char *tmp_ptr;
	unsigned char *standby_bin_start=_standby_bin_start;
	unsigned char *standby_bin_end=&_standby_bin_end;
	struct pm_wakesrc_settled *ws_settled = NULL;

	lpsram_para_t *lpsram_para_orig;
	lpsram_para_t *lpara;
	uint8_t *resp_save;
	uint8_t *buff_save;
	char *name_save;
	struct psram_ctrl *ctrl_save;
	struct psram_request *mrq_save;

	standby_head_t *head = (standby_head_t *)standby_bin_start;
	standby_head_t *head_sram = NULL;

	void (*standby_main)(void) = head->enter;

	pm_log("wakesrc_active:0x%08x\n", pm_wakesrc_get_active());

	pm_debug_gpio_status();

	/* WARNING: If the data needs to be changed once,
	 * a new value shouble be copied each time it goes to bin,
	 * otherwise the last copied value will remain all the time.
	 */
	/*copy code to sram*/
	memcpy(head->code_start,
		head->code_start - head->paras_start + standby_bin_start,
		head->code_end - head->code_start);

	/* clear bss segment */
	memset(head->bss_start, 0, head->bss_end - head->bss_start);

	wup_handle.wakeup_src = 0;
	if (wup_handle.wupio_set != 0) {
		writel(0x1, PM_RTC_IO_WAKE_GEN);
	}

	/*todo: set param */
	head->mode = mode;
	head->wakesrc_active = pm_wakesrc_get_active();
	head->anacfg = pm_wakeres_get_anacfg();
	head->clkcfg = pm_wakeres_get_clkcfg();
	head->pwrcfg = pm_wakeres_get_pwrcfg();
	head->time_to_wakeup_ms = wup_handle.time_to_wakeup_ms;
	head->chipv = chipv;
	head->rccal_en = rccal_en;
	ret = get_cpu_freq(&head->cpufreq);
	ret = 0;

#ifdef CONFIG_COMPONENTS_PM_TEST_TOOLS
	if (pm_test_standby_recording()) {
		head->stage_record = 1;
	} else {
		head->stage_record = 0;
	}
#endif

	if (lpsram.inited && lpsram.para) {
	/* save point addr in SRAM */
		resp_save = (uint8_t *)head->lpsram_para.ctrl->mrq->cmd.resp;
		buff_save = (uint8_t *)head->lpsram_para.ctrl->mrq->data.buff;
		name_save = (char *)head->lpsram_para.name;
		ctrl_save = (struct psram_ctrl *)head->lpsram_para.ctrl;
		mrq_save = (struct psram_request *)head->lpsram_para.ctrl->mrq;

		lpsram_para_orig = lpsram.para;

		memcpy(resp_save, lpsram_para_orig->ctrl->mrq->cmd.resp, sizeof(uint8_t));
		memcpy(buff_save, lpsram_para_orig->ctrl->mrq->data.buff, sizeof(uint8_t));

		memcpy(mrq_save, lpsram_para_orig->ctrl->mrq, sizeof(struct psram_request));
		mrq_save->cmd.resp = resp_save;
		mrq_save->data.buff = buff_save;

		memcpy(ctrl_save, lpsram_para_orig->ctrl, sizeof(struct psram_ctrl));
		ctrl_save->mrq = mrq_save;

		lpara = (lpsram_para_t *)&head->lpsram_para;
		memcpy(lpara, lpsram_para_orig, sizeof(lpsram_para_t));
		lpara->name = name_save;
		lpara->ctrl = ctrl_save;

		head->lpsram_inited = 1;
	} else {
		pm_warn("LSPSRAM will not standby\n");
		head->lpsram_inited = 0;
	}

	if (hpsram.inited) {
		memcpy(&head->hpsram_para, &hpsram.para, sizeof(__dram_para_t));
		head->hpsram_inited = 1;
	} else {
		pm_warn("HSPSRAM will not standby\n");
		head->hpsram_inited = 0;
	}

	/* wakesrc res will not be covered */
	pm_log("subsys_pwr_remain: 0x%x\n", subsys_pwr_remain);
	if (subsys_pwr_remain & (0x1 << PM_SUBSYS_ID_RISCV)) {
		head->pwrcfg |= (0x1 << PM_RES_POWER_RV_LDO);
		head->clkcfg |= (0x1 << PM_RES_CLOCK_APB_NO_NEED_TO_32K);
		head->clkcfg |= (0x1 << PM_RES_CLOCK_AHB_NO_NEED_TO_32K);
	}
	if (subsys_pwr_remain & (0x1 << PM_SUBSYS_ID_DSP)) {
		head->pwrcfg |= (0x1 << PM_RES_POWER_DSP_LDO);
		head->clkcfg |= (0x1 << PM_RES_CLOCK_APB_NO_NEED_TO_32K);
		head->clkcfg |= (0x1 << PM_RES_CLOCK_AHB_NO_NEED_TO_32K);
		/* if dsp run in hpsram */
#ifdef CONFIG_ARCH_SUN20IW2P1
#if defined(CONFIG_ARCH_ARMV8M_DEFAULT_BOOT_DSP) || defined(CONFIG_COMMAND_BOOT_DSP)
#if CONFIG_ARCH_DSP_START_ADDRESS
		if ((CONFIG_ARCH_DSP_START_ADDRESS & 0x0f000000) == 0x0c000000) {
			pm_warn("HSPSRAM will not standby\n");
			head->hpsram_inited = 0;
			head->clkcfg |= (0x1 << PM_RES_CLOCK_DPLL3);
		}
#else
	pm_warn("fail to check the DSP running address, DSP may run in PSRAM\n");
#endif /* CONFIG_ARCH_DSP_START_ADDRESS */
#endif
#endif /* CONFIG_ARCH_SUN20IW2P1 */
	}

	/* set the priority of wakeirq to wakeup m33 from WFI when PRIMASK is set to 1 */
	if (mode == PM_MODE_SLEEP) {
		for (int i = PM_WAKEUP_SRC_BASE; i < PM_WAKEUP_SRC_MAX; i++) {
			if (head->wakesrc_active & (0x1 << i)) {
				/* set prioprity to wakeup from WFI */
				ws_settled = pm_wakesrc_get_by_id(i);
				nvic_irq_priority_save[i] = arch_nvic_irq_get_priority(ws_settled->irq);
				arch_nvic_irq_set_priority(ws_settled->irq, 0x0);
			}
		}
		/* M33 MSGBOX irq */
		nvic_irq_msgbox_priority_save = arch_nvic_irq_get_priority(CPU_SYS_IRQ_OUT1_IRQn);
		arch_nvic_irq_set_priority(CPU_SYS_IRQ_OUT1_IRQn, 0x0);
	} else {
		nvic_irq_enable_status_save = 0;
		for (int i = PM_WAKEUP_SRC_BASE; i < PM_WAKEUP_SRC_MAX; i++) {
			if ((head->wakesrc_active & (0x1 << i)) == 0) {
				ws_settled = pm_wakesrc_get_by_id(i);
				nvic_irq_enable_status_save |= ((!!arch_nvic_get_enable_irq(ws_settled->irq)) << i);
				if (nvic_irq_enable_status_save & (0x1 << i))
					hal_disable_irq(ws_settled->irq);

			}
		}
	}

	head->reg_dump_addr = pm_reg_capture_addr_set();

	/*copy head to sram*/
	memcpy(head->paras_start, head,
		head->paras_end - head->paras_start);

	rtc_ldo_ctrl_bak = readl(GPRCM_RTC_LDO_CTRL_REG);
	aon_ldo_ctrl_bak = readl(GPRCM_AON_LDO_CTRL_REG);
	if (mode == PM_MODE_HIBERNATION) {
		if (chipv > SUN20I_CHIPV_B)
			HAL_PRCM_SetRTCLDOSlpVolt(PRCM_RTC_LDO_SLEEP_VOLT_675MV);
		else
			HAL_PRCM_SetRTCLDOSlpVolt(PRCM_RTC_LDO_SLEEP_VOLT_775MV);
	} else if (mode == PM_MODE_STANDBY) {
		HAL_PRCM_SetRTCLDOActVolt(PRCM_RTC_LDO_ACTIVE_VOLT_675MV);
		HAL_PRCM_SetAONLDOSlpVoltage(1150);
	}

	pm_log("jump to standby bin\n");
	/*to sram run*/
	standby_main();
	pm_log("exit standby bin\n");

	writel((readl(GPRCM_RTC_LDO_CTRL_REG) & ~(0x7f << 0))
		| (rtc_ldo_ctrl_bak & (0x7f << 0)), GPRCM_RTC_LDO_CTRL_REG);
	writel((readl(GPRCM_AON_LDO_CTRL_REG) & ~(0x3ff << 4))
		| (aon_ldo_ctrl_bak & (0x3ff << 4)), GPRCM_AON_LDO_CTRL_REG);

	head_sram = head->paras_start;
	pm_moment_record(PM_MOMENT_ENTER_WFI, head_sram->suspend_moment);
	pm_moment_record(PM_MOMENT_EXIT_WFI, head_sram->resume_moment);

	if (mode == PM_MODE_SLEEP) {
		arch_nvic_irq_set_priority(CPU_SYS_IRQ_OUT1_IRQn, nvic_irq_msgbox_priority_save);
		nvic_irq_msgbox_priority_save = 0;
		for (int i = PM_WAKEUP_SRC_BASE; i < PM_WAKEUP_SRC_MAX; i++) {
			if (head->wakesrc_active & (0x1 << i)) {
				/* restore prioprity */
				ws_settled = pm_wakesrc_get_by_id(i);
				arch_nvic_irq_set_priority(ws_settled->irq, nvic_irq_priority_save[i]);
				nvic_irq_priority_save[i] = 0;
			}
		}
	} else {
		for (int i = PM_WAKEUP_SRC_BASE; i < PM_WAKEUP_SRC_MAX; i++) {
			if ((head->wakesrc_active & (0x1 << i)) == 0) {
				ws_settled = pm_wakesrc_get_by_id(i);
				if (nvic_irq_enable_status_save & (0x1 << i))
					hal_enable_irq(ws_settled->irq);

			}
		}
	}

	if (mode != PM_MODE_SLEEP)
		pm_systeminit();

	pm_dbg("wakesrc pending:0x%08x\n", pm_wakesrc_read_pending());
	pm_debug_get_starttime (PM_TIME_PLAT_RESUME_ENTER, head_sram->resume_moment);
	pm_debug_record_time (PM_TIME_PLAT_RESUME_ENTER, hal_gettime_ns());
	pm_debug_record_time (PM_TIME_PLAT_SUSPEND_ENTER, head_sram->suspend_moment);

	return ret;
}

#endif

static int pm_m33_valid(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int pm_m33_pre_begin(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		pm_wakesrc_clear_wakeup_irq();
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#include <backtrace.h>
/* subsys pwr ctrl */
static void pm_m33_resume_all(void)
{
	subsys_again_abort = 1;

	/* use in other core has suspended/resumed */
	subsys_pwr_remain = 0;
	common_syscore_suspend_enabled = 1;

	/* use for suspend */
	pm_wakesrc_unmask_affinity(PM_WS_AFFINITY_DSP);
	pm_wakesrc_unmask_affinity(PM_WS_AFFINITY_RISCV);
	pm_mode_keep_changed = 0;

	/* keep until the whole system wakeup to avoid subsys sync misjudgement due to mask changs */
	pm_subsys_notify_unmask(PM_SUBSYS_ID_DSP, 1);
	pm_subsys_notify_unmask(PM_SUBSYS_ID_RISCV, 1);

}

/**
 * 1. remove power remain.
 * 2. unmask subcore irq.
 * 3. vote for common syscore suspend.
 * 4. unmask subsys notifier.
 */
static void pm_m33_subsys_remain(int remain, pm_subsys_id_t id)
{
	if (!remain) {
		subsys_pwr_remain &= ~(0x1 << id);
		pm_wakesrc_unmask_affinity(id);
		common_syscore_suspend_enabled |= (0x1 << id);
		pm_subsys_notify_unmask(id, 0);
		pm_mode_keep_changed &= ~(0x1 << id);
	} else {
		subsys_pwr_remain |= 0x1 << id;
		pm_wakesrc_mask_affinity(id);
		common_syscore_suspend_enabled = 0;
		pm_subsys_notify_mask(id);
		pm_mode_keep_changed |= (0x1 << id);
	}
}

static int pm_m33_begin(suspend_mode_t mode)
{
	int ret = 0;
	int id = 0;
	int awake_subsys = 0;
	int num = 0;
	uint32_t subcnt;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		if (pm_suspend_assert()) {
			pm_dbg("%s(%d): pm assert!\n", __func__, __LINE__);
			ret = -EBUSY;
			break;
		}

		/* update subcnt before triggering the suspend of other cores */
		subcnt = pm_wakecnt_subcnt_update();
		pm_dbg("subcnt save: %d\n", subcnt);
		/* the other cores will not be trigger to suspend if they have soft_wakesrc.
		 * if the core doesn't want to be awake, all soft_wakesrc must be disabled.
		 */
#ifdef CONFIG_PM_SUBSYS_DSP_SUPPORT
		num = pm_msgtodsp_check_wakesrc_num(PM_WAKESRC_SOFT_WAKEUP);
		if (num)
			pm_log("begin: DSP soft wakesrc num: %d\n", num);
		pm_log("check dsp subsys event...\n");
		while (pm_subsys_event_pending(PM_SUBSYS_ID_DSP)) {
			hal_msleep(2);
		}
#endif
#ifdef CONFIG_PM_SUBSYS_RISCV_SUPPORT
		num = pm_msgtorv_check_wakesrc_num(PM_WAKESRC_SOFT_WAKEUP);
		if (num)
			pm_log("begin: RISCV soft wakesrc num: %d\n", num);
		pm_log("check rv subsys event...\n");
		while (pm_subsys_event_pending(PM_SUBSYS_ID_RISCV)) {
			hal_msleep(2);
		}
#endif
		pm_log("subsys event over\n");

		awake_subsys = pm_subsys_check_in_status(PM_SUBSYS_STATUS_KEEP_AWAKE);
		if (awake_subsys) {
			pm_dbg("begin: awake_subsys sum: 0x%x\n", awake_subsys);
			for (id = PM_SUBSYS_ID_BASE; id < PM_SUBSYS_ID_MAX; id++) {
				if (awake_subsys & (1 << id)) {
					pm_log("begin: remain subsys: %d\n", id);
					pm_m33_subsys_remain(1, id);
				}
			}
		}

		if (pm_mode_keep_changed && (mode != PM_MODE_SLEEP)) {
			pm_suspend_mode_change(PM_MODE_SLEEP);
			if (suspend_mode_bak == MODE_BAK_INITIALIZATION)
				suspend_mode_bak = mode;
		}

		/* suspend all subsys.
		 * the suspend mode for subsys remain as it is.
		 * form their point of view they will be powerdown, and such as the fact.
		 */
		ret = pm_subsys_suspend_sync(mode);
		if (ret) {
			pm_err("susbsys suspend failed, return %d\n", ret);
			break;
		}

		/* take over irq */
		ret = pm_wakesrc_prepared();
		pm_log("plat BEGIN end\n");
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_m33_prepare(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_m33_prepare_late(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		/* suspend common syscore */
		if (mode == PM_MODE_SLEEP) {
			if(common_syscore_suspend_enabled) {
				ret = pm_common_syscore_suspend(mode);
			} else
				pm_warn("common syscore will not be suspended\n");
		} else
			ret = pm_common_syscore_suspend(mode);

#ifdef CONFIG_DRIVERS_LSPSRAM
		if (lpsram.inited)
			lpsram.para = psram_chip_get_para();
#endif

		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int pm_m33_wake(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		/* resume common syscore */
		if (mode == PM_MODE_SLEEP) {
			if(common_syscore_suspend_enabled) {
				ret = pm_common_syscore_resume(mode);
			} else
				pm_warn("common syscore will not be resumed\n");
		} else
			ret = pm_common_syscore_resume(mode);
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}


static int pm_m33_finish(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		pm_log("wakeup_event: 0x%x\n", pm_wakesrc_get_event());
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}

/**
1. subsys
2. pm mode
3. subsys notify mask judge for resume
*/
static int pm_m33_end(suspend_mode_t mode)
{
	int ret = 0;
	int id = 0;
	int online_subsys = 0;
	int awake_subsys = 0;
	int num = 0;
	struct pm_wakesrc_settled *pmws = NULL;
	uint32_t pending_remain = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		online_subsys = pm_subsys_check_online();

		/* ensure that the mode of resume notification is consistent with the mode of suspend notification */
		if (suspend_mode_bak != MODE_BAK_INITIALIZATION) {
			pm_suspend_mode_change(suspend_mode_bak);
			mode = suspend_mode_bak;
			suspend_mode_bak = MODE_BAK_INITIALIZATION;
		}

		/* release irq */
		ret = pm_wakesrc_complete();
		if (ret)
			pm_err("%s(%d): release irq fialed\n", __func__, __LINE__);

		/* Warning: pay attention to the boot flag, which means recovery.
		 * resume with boot flag all the time.
		 * the flag is cleared when resume or arch init.
		 * when dsp wakes up, it will directly modify the boot reg, and the boot flag does not work.
		 */
		ret = pm_subsys_resume_sync(mode);
		if (ret)
			pm_err("%s(%d): subsys resume sync failed\n", __func__, __LINE__);

		/* irq like MAD may fail to clear pending on M33 until it has been handlered on another core */
		/* FIXME: record and clear the pendind of irq on another core only */
		pending_remain = pm_wakesrc_read_pending();
		for (id = PM_WAKEUP_SRC_BASE; id < PM_WAKEUP_SRC_MAX; id++) {
			if (!(pending_remain & (0x1 << id)))
				continue;

			pmws = pm_wakesrc_get_by_id(id);
			if (pmws)
				hal_interrupt_clear_pending(pmws->irq);
		}

		/* suspend again late assert:
		 * 1. if all subsys offline last turn:
		 * 	(1) wakecnt has not changed.
		 * 	(2) someone needs to resume to handler events, but requires the system to stay sleep until the handler completes.
		 * 2. if someone online last turn:
		 * 	(1) wakecnt has not changed.
		 * 	(2) online subsys suspend assert return false.
		 * 	(3) someone online last turn, but this turn dose not need to keep awake.
		 */
		if (online_subsys) {
			pm_log("end: online subsys sum: 0x%x\n", online_subsys);
			for (id = PM_SUBSYS_ID_BASE; id < PM_SUBSYS_ID_MAX; id++) {
				if (online_subsys & (1 << id)) {
#ifdef CONFIG_PM_SUBSYS_DSP_SUPPORT
					if (id == PM_SUBSYS_ID_DSP) {
						num = pm_msgtodsp_check_wakesrc_num(PM_WAKESRC_SOFT_WAKEUP);
						if (num)
							pm_log("end: DSP soft wakesrc num: %d\n", num);
					}
#endif
#ifdef CONFIG_PM_SUBSYS_RISCV_SUPPORT
					if (id == PM_SUBSYS_ID_RISCV) {
						num = pm_msgtorv_check_wakesrc_num(PM_WAKESRC_SOFT_WAKEUP);
						if (num)
							pm_log("end: RISCV soft wakesrc num: %d\n", num);
					}
#endif
					pm_log("check online subsys(%d) event...\n", id);
					while (pm_subsys_event_pending(id)) {
						hal_msleep(2);
					}
					/* status will change after check soft wakesrc num */
					awake_subsys = pm_subsys_check_in_status(PM_SUBSYS_STATUS_KEEP_AWAKE);
					pm_dbg("end: awake_subsys sum: 0x%x\n", awake_subsys);
					if (!(awake_subsys & (0x1 << id))) {
						pm_log("end: restore subsys: %d\n", id);
						pm_m33_subsys_remain(0, id);
					}
				}
			}
		}
		pm_log("plat END end\n");

		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int pm_m33_recover(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
		break;
	case PM_MODE_HIBERNATION:
		break;
	case PM_MODE_ON:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;

}

static int pm_m33_again(suspend_mode_t mode)
{
	int ret = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
		/* may need to distinguish whether srcs of might wakeup inpr */
		if (pm_always_wakeup() || pm_wakecnt_subcnt_has_changed())
			break;

		pm_log("again judgement...\n");
		/* if cnt is not changed, system will suspend again */
		while (1) {
			while (pm_wakecnt_in_progress() || pm_wakelocks_refercnt(0)) {
				if (pm_wakecnt_has_changed())
					break;
				hal_msleep(2);
			}

			if (pm_wakecnt_has_changed()) {
				break;
			} else if (!pm_wakecnt_in_progress() && !pm_wakelocks_refercnt(0)) {
				ret = -EAGAIN; /* Try again */
				pm_wakesrc_clear_wakeup_irq();
				break;
			}
		}
		break;
	case PM_MODE_HIBERNATION:
	case PM_MODE_ON:
	default:
		/* return 0, to wakeup */
		break;
	}

	return ret;
}

static int pm_m33_again_late(suspend_mode_t mode)
{
	int ret = 0;
	int subsys_assert = 0;
	int subsys_assert_save = 0;

	switch (mode) {
	case PM_MODE_SLEEP:
	case PM_MODE_STANDBY:
		/* make sure the cnt has changed before, pay attention to the task priority */
		if (pm_suspend_assert()) {
			pm_dbg("%s(%d): pm assert!\n", __func__, __LINE__);
			break;
		}

		pm_log("again late judgement...\n");
		/* subcnt will check on their own core in this case.  */
		subsys_assert_save = subsys_assert;
#ifdef CONFIG_PM_SUBSYS_DSP_SUPPORT
		subsys_assert += pm_msgtodsp_check_subsys_assert(mode);
		if (subsys_assert_save != subsys_assert) {
			pm_warn("subsys DSP suspend assert!\n");
			subsys_assert_save = subsys_assert;
		}
#endif
#ifdef CONFIG_PM_SUBSYS_RISCV_SUPPORT
		subsys_assert += pm_msgtorv_check_subsys_assert(mode);
		if (subsys_assert_save != subsys_assert) {
			pm_warn("subsys RISCV suspend assert!\n");
			subsys_assert_save = subsys_assert;
		}
#endif
		/* subcore thinks the system should wakeup */
		if (subsys_assert)
			subsys_again_abort = 1;

		if (!subsys_again_abort) {
			pm_wakesrc_clear_wakeup_irq();
			ret = -EAGAIN;
		}
		break;
	case PM_MODE_HIBERNATION:
	case PM_MODE_ON:
	default:
		/* return 0, to wakeup */
		break;
	}

	/* clear all again flag in the end */
	if (!ret) {
		pm_m33_resume_all();
	}
	subsys_again_abort = 0;

	return ret;
}

static suspend_ops_t pm_m33_suspend_ops = {
	.name  = "pm_m33_suspend_ops",
	.valid = pm_m33_valid,
	.pre_begin = pm_m33_pre_begin,
	.begin = pm_m33_begin,
	.prepare = pm_m33_prepare,
	.prepare_late = pm_m33_prepare_late,
	.enter = pm_m33_enter,
	.wake = pm_m33_wake,
	.finish = pm_m33_finish,
	.end = pm_m33_end,
	.recover = pm_m33_recover,
	.again = pm_m33_again,
	.again_late = pm_m33_again_late,
};


int pm_m33_platops_init(void)
{
	int i;
	int ret;
	uint8_t data[(SUN20I_CHIPV_LEN + 7) / 8] = {0};

	chipv = 0;
#ifdef CONFIG_DRIVERS_EFUSE
	ret = hal_efuse_read_ext(SUN20I_CHIPV_START, SUN20I_CHIPV_LEN, data);
	if (ret) {
		pm_err("get chipv failed, return %d\n", ret);
	} else {
		for (i = 0; i < ((SUN20I_CHIPV_LEN + 7) / 8); i++) {
			pm_dbg("pm get chipv data[%d]: 0x%x\n", i, data[i]);
			chipv |= ((uint32_t)data[i]) << (i * 8);
		}
	}
#endif
	pm_dbg("pm get chipv: 0x%x\n", chipv);

	platops_mutex = xSemaphoreCreateMutex();
	if (!platops_mutex)
		pm_err("platops: create mutex failed.\n");

	if (readl(PM_STANDBY_STAGE_PSRAM_REC_REG) == LSPSRAM_CRC_FAIL_FLAG) {
		pm_err("!!! ERROR: LSPSRAM crc failed at last standby !!!\n");
		writel(0, PM_STANDBY_STAGE_PSRAM_REC_REG);
	} else if (readl(PM_STANDBY_STAGE_PSRAM_REC_REG) == LSPSRAM_RESUME_FAIL_FLAG) {
		pm_err("!!! ERROR: LSPSRAM resume failed at last standby !!!\n");
		writel(0, PM_STANDBY_STAGE_PSRAM_REC_REG);
	} else if (readl(PM_STANDBY_STAGE_PSRAM_REC_REG) == HSPSRAM_CRC_FAIL_FLAG) {
		pm_err("!!! ERROR: HSPSRAM crc failed at last standby !!!\n");
		writel(0, PM_STANDBY_STAGE_PSRAM_REC_REG);
	}


	return pm_platops_register(&pm_m33_suspend_ops);
}

int pm_m33_platops_deinit(void)
{
	chipv = 0;

	return pm_platops_register(NULL);
}


int pm_test_enter(void)
{
#if 0
	/*
	 * we must set AR800A_BOOT_ARG, it provides a space to hold registers
	 * before M33 will be closed.
	 * and must be place at sram.
	 */
	writel((uint32_t)&vault_arm_registers, 0x40050000 + 0x1c8);
#endif

	pm_m33_enter(PM_MODE_STANDBY);

	while (1);
}

void pm_restore_ext_ldo(void)
{
	/* restore ext_ldo to 3.3V */
	if ((readl(GPRCM_EXT_LDO_CTRL_REG) & (0x3 << 4)) == (0x1 << 4)) {
		writel(readl(GPRCM_EXT_LDO_CTRL_REG) & ~(0x3 << 4), GPRCM_EXT_LDO_CTRL_REG);
	}
}
