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

#include <errno.h>
#include <io.h>

#include "pm_adapt.h"
#include "pm_debug.h"
#include "pm_suspend.h"
#include "pm_notify.h"
#include "pm_wakelock.h"
#include "pm_subsys.h"
#include "pm_task.h"

#include "pm_rpcfunc.h"
#include <hal_clk.h>

#undef   PM_DEBUG_MODULE
#define  PM_DEBUG_MODULE  PM_DEBUG_M33

#define GPRCM_RV_BOOT_FLAG_REG		(0x400501d8)

int riscv_notify_cb(suspend_mode_t mode, pm_event_t event, void *arg)
{
	int ret = 0;

	switch (event) {
	case PM_EVENT_SYS_PERPARED:
	case PM_EVENT_SYS_FINISHED:
	case PM_EVENT_SUSPEND_FAIL:
		ret = pm_msgtorv_trigger_notify(mode, event);
		break;
	default:
		break;
	}

	return ret;
}

static pm_notify_t riscv_notify = {
	.name = "riscv_notify",
	.pm_notify_cb = riscv_notify_cb,
	.arg = NULL,
};

static pm_subsys_t riscv_subsys = {
	.name = "riscv_subsys",
	.status = PM_SUBSYS_STATUS_NORMAL,
	.xQueue_Signal = NULL,
	.xQueue_Result = NULL,
	.xHandle_Signal = NULL,
	.xHandle_Watch = NULL,
};

static int riscv_trigger_suspend(suspend_mode_t mode)
{
	return pm_msgtorv_trigger_suspend(mode);
}

#define PM_PMU_BASE				(0x40051400)
#define PM_SYS_LOW_POWER_CTRL_REG		(PM_PMU_BASE + 0x0100)
#define PM_RV_WUP_EN_MASK			(0x1 << 8)
#define PM_SYS_LOW_POWER_STATUS_REG		(PM_PMU_BASE + 0x0104)
#define PM_RV_ALIVE				(0x1 << 8)
#define PM_RV_SLEEP				(0x1 << 9)

#define PM_CCMU_AON_BASE			(0x4004c400)
#define PM_DPLL1_OUT_CFG_REG			(PM_CCMU_AON_BASE + 0x00a4)
#define PM_CK1_C906_EN_MASK			(0x1 << 7)
#define PM_CK1_C906_DIV_MASK			(0x7 << 4)
#define PM_SYS_CLK_CFG_REG			(PM_CCMU_AON_BASE + 0x00e0)
#define PM_CKPLL_C906_SEL_MASK			(0x1 << 17)

#define PM_CCMU_BASE				(0x4003c000)
#define PM_CPU_DSP_RV_CLK_GATING_CTRL_REG	(PM_CCMU_BASE + 0x0014)
#define PM_RISCV_CLK_GATING_MASK		(0x1 << 19)
#define PM_CPU_DSP_RV_RST_CTRL_REG		(PM_CCMU_BASE + 0x0018)
#define PM_RISCV_APB_SOFT_RST_MASK		(0x1 << 21)
#define PM_RISCV_CFG_RST_MASK			(0x1 << 19)
#define PM_RISCV_CORE_RST_MASK			(0x1 << 16)
#define PM_RISCV_CLK_CTRL_REG			(PM_CCMU_BASE + 0x0064)
#define PM_RISCV_CLK_EN_MASK			(0x1 << 31)
#define PM_RISCV_CLK_SEL_MASK			(0x3 << 4)
#define PM_RISCV_CLK_DIV_MASK			(0x3 << 0)

#define PM_RISCV_SYS_CFG_BASE			(0x40028000)
#define PM_RISCV_STA_ADDR0_REG			(PM_RISCV_SYS_CFG_BASE + 0x0004)
extern struct spare_rtos_head_t rtos_spare_head;
int c906_freq_store;
static void sw_rv_disable(void)
{
	if (readl(PM_SYS_LOW_POWER_STATUS_REG) & PM_RV_ALIVE) {
		writel(readl(PM_SYS_LOW_POWER_CTRL_REG) & ~PM_RV_WUP_EN_MASK,
			PM_SYS_LOW_POWER_CTRL_REG);

		pm_log("waiting RV power status sleep...\n");
		while(!(readl(PM_SYS_LOW_POWER_STATUS_REG) & PM_RV_SLEEP));
		pm_log("RV fell aleep\n");
	}
}

void pm_c906_close(void)
{
	hal_clk_t clk_ck1_c906, clk_ck3_c906;
	hal_clk_t clk_c906_gate, clk_c906_cfg;

	clk_ck1_c906 = hal_clock_get(HAL_SUNXI_AON_CCU, CLK_CK1_C906);
	hal_clock_disable(clk_ck1_c906);
	// default use clk_ck3_c906, but we can close all
	clk_ck3_c906 = hal_clock_get(HAL_SUNXI_AON_CCU, CLK_CK3_C906);
	hal_clock_disable(clk_ck3_c906);

	clk_c906_gate = hal_clock_get(HAL_SUNXI_CCU, CLK_RISCV_GATE);
	hal_clock_disable(clk_c906_gate);

	clk_c906_cfg = hal_clock_get(HAL_SUNXI_CCU, CLK_RISCV_CFG);
	hal_clock_disable(clk_c906_cfg);
}

extern int sun20i_boot_c906(void);
int pm_boot_c906(void)
{
	uint32_t reg_val;

	if (readl(PM_SYS_LOW_POWER_STATUS_REG) & PM_RV_ALIVE) {
		pm_log("---c906 alive, skip boot---\n");
		return 0;
	}

	writel(0x429b0001, GPRCM_RV_BOOT_FLAG_REG);

	pm_log("---pm resume boot c906---\n");

	/* set core in reset state */
	reg_val = readl(PM_CPU_DSP_RV_RST_CTRL_REG);
	writel(reg_val & ~PM_RISCV_CORE_RST_MASK, PM_CPU_DSP_RV_RST_CTRL_REG);
	sun20i_boot_c906();

	pm_log("boot c906 end\n");

	return 0;
}

#define GPRCM_SYS_PRIV_REG0	(0x40050200)
#define RV_SUSPEND_OK		(PM_SUBSYS_ACTION_OK_FLAG)
#define RV_SUSPEND_FAILED	(PM_SUBSYS_ACTION_FAILED_FLAG)
#define RV_RESUME_TRIGGER	(0x0)
#define RV_RESUME_OK		(0x16aa0000)
int riscv_trigger_resume(suspend_mode_t mode)
{
	/* try poweron risc-v */
	int ret;

	/* ensure that c906 core rst is in reset state before open RV_SW
	 * release the core rst after start addr is set
	 * the start addr reg can only be writed in this case:
	 *     1. RV_SW on;
	 *     2. c906 gating on.
	 */
	pm_c906_close();
	ret = pm_boot_c906();

	if (ret)
		pm_err("resume boot c906 failed\n");
	else
		writel(RV_RESUME_TRIGGER, GPRCM_SYS_PRIV_REG0);

	return ret;
}

static void riscv_subsys_do_signal(void *arg)
{
	int ret;
	pm_subsys_msg_t msg;
	BaseType_t xReturned;
	pm_subsys_t *subsys = (pm_subsys_t *) arg;

	while (1) {
		if (pdPASS == xQueueReceive(subsys->xQueue_Signal, &msg, portMAX_DELAY)) {
			switch (msg.action) {
			/* about suspend*/
			case PM_SUBSYS_ACTION_TO_SUSPEND:
				if (PM_SUBSYS_STATUS_NORMAL != subsys->status) {
					pm_warn("SUSPEND WARNING: subsys(%s) status is %d, will not be suspened.\n",
						subsys->name ? subsys->name : "NULL", subsys->status);
					msg.action = PM_SUBSYS_RESULT_SUSPEND_FAILED;
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					break;
				}
				subsys->status = PM_SUBSYS_STATUS_SUSPENDING;
				ret = riscv_trigger_suspend(msg.mode);
				if (ret) {
					subsys->status = PM_SUBSYS_STATUS_NORMAL;
					msg.action = PM_SUBSYS_RESULT_SUSPEND_REJECTED;
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					pm_err("subsys(%s) trigger suspend(%s) failed(%d).\n",
							subsys->name ? subsys->name : "NULL",
							pm_mode2string(msg.mode),
							ret);
				}
				break;
			/* about resume*/
			case PM_SUBSYS_ACTION_TO_RESUME:
				if (PM_SUBSYS_STATUS_SUSPENDED != subsys->status) {
					pm_warn("RESUME WARNING: subsys(%s) status is %d, do not need to resume.\n",
						subsys->name ? subsys->name : "NULL", subsys->status);
					msg.action = PM_SUBSYS_RESULT_RESUME_FAILED;
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					break;
				}
				subsys->status = PM_SUBSYS_STATUS_RESUMING;
				ret = riscv_trigger_resume(msg.mode);
				if (ret) {
					pm_err("subsys(%s) trigger resume(%s) failed(%d).\n",
							subsys->name ? subsys->name : "NULL",
							pm_mode2string(msg.mode),
							ret);
				}
				break;
			case PM_SUBSYS_ACTION_KEEP_AWAKE:
				subsys->status = PM_SUBSYS_STATUS_KEEP_AWAKE;
				msg.action = PM_SUBSYS_RESULT_NOTHING;
				pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
				pm_warn("subsys(%s) keeps awake\n", subsys->name ? subsys->name : "NULL");
				break;
			case PM_SUBSYS_ACTION_TO_NORMAL:
				subsys->status = PM_SUBSYS_STATUS_NORMAL;
				msg.action = PM_SUBSYS_RESULT_NOTHING;
				pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
				pm_log("subsys(%s) change status to normal\n", subsys->name ? subsys->name : "NULL");
				break;
			default:
				break;
			}
		}
	}
}

static pm_subsys_result_t riscv_check_to_suspend_result(void)
{
	if (readl(GPRCM_SYS_PRIV_REG0) == RV_SUSPEND_OK) {
		writel(PM_SUBSYS_ACTION_HAS_GOT_RESULT, GPRCM_SYS_PRIV_REG0);
		return PM_SUBSYS_RESULT_SUSPEND_OK;
	}

	if (readl(GPRCM_SYS_PRIV_REG0) == RV_SUSPEND_FAILED) {
		writel(PM_SUBSYS_ACTION_HAS_GOT_RESULT, GPRCM_SYS_PRIV_REG0);
		return PM_SUBSYS_RESULT_SUSPEND_FAILED;
	}

	/*we read prcm to get result*/
	return PM_SUBSYS_RESULT_NOP;
}

static pm_subsys_result_t riscv_check_to_resume_result(void)
{
	if (readl(GPRCM_SYS_PRIV_REG0) == RV_RESUME_OK) {
		writel(PM_SUBSYS_ACTION_HAS_GOT_RESULT, GPRCM_SYS_PRIV_REG0);
		return PM_SUBSYS_RESULT_RESUME_OK;
	}

	/*we read rtc to get result*/
	return PM_SUBSYS_RESULT_NOP;
}

static void riscv_subsys_do_watch(void *arg)
{
	int ret;
	pm_subsys_msg_t msg;
	pm_subsys_t *subsys = (pm_subsys_t *) arg;

	const TickType_t xFrequency = 10;
	TickType_t xLastWakeTime;


	while (1) {
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		switch (subsys->status) {
		case PM_SUBSYS_STATUS_SUSPENDING:
			ret = riscv_check_to_suspend_result();
			switch (ret)  {
				case PM_SUBSYS_RESULT_SUSPEND_OK:
					subsys->status = PM_SUBSYS_STATUS_SUSPENDED;
					msg.action = PM_SUBSYS_RESULT_SUSPEND_OK;
					sw_rv_disable();
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					break;
				case PM_SUBSYS_RESULT_SUSPEND_FAILED:
					subsys->status = PM_SUBSYS_STATUS_NORMAL;
					msg.action = PM_SUBSYS_RESULT_SUSPEND_FAILED;
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					break;
				default:
					break;
			}
			break;
		case PM_SUBSYS_STATUS_RESUMING:
			ret = riscv_check_to_resume_result();
			switch (ret)  {
				case PM_SUBSYS_RESULT_RESUME_OK:
					subsys->status = PM_SUBSYS_STATUS_NORMAL;
					msg.action = PM_SUBSYS_RESULT_RESUME_OK;
					pm_subsys_report_result(PM_SUBSYS_ID_RISCV, &msg);
					break;
				default:
					break;
			}
			break;
		default:
			break;
		}
	}
}

int riscv_subsys_init(void)
{
	int ret = 0;
	BaseType_t xReturned;
	pm_subsys_t *subsys = &riscv_subsys;

	subsys->xQueue_Signal = xQueueCreate(1, sizeof( pm_subsys_msg_t ));
	if (NULL == subsys->xQueue_Signal) {
		pm_err("Create riscv subsys Signal xQueue failed.\n");
		ret = -EPERM;
		goto err_xQueue_Signal;
	}

	subsys->xQueue_Result = xQueueCreate(1, sizeof( pm_subsys_msg_t ));
	if (NULL == subsys->xQueue_Result) {
		pm_err("Create riscv subsys Result xQueue failed.\n");
		ret = -EPERM;
		goto err_xQueue_Result;
	}

	xReturned = xTaskCreate(
			riscv_subsys_do_signal,
			"riscv_subsys_signal",
			(1 * 1024) / sizeof(StackType_t),
			(void *)subsys,
			PM_TASK_PRIORITY + 1,
			(TaskHandle_t * const)&subsys->xHandle_Signal);

	if (pdPASS != xReturned) {
		pm_err("create riscv signal thread failed\n");
		ret = -EPERM;
		goto err_xHandle_Signal;
	}


	xReturned = xTaskCreate(
			riscv_subsys_do_watch,
			"riscv_subsys_watch",
			(1 * 1024) / sizeof(StackType_t),
			(void *)subsys,
			PM_TASK_PRIORITY + 1,
			(TaskHandle_t * const)&subsys->xHandle_Watch);

	if (pdPASS != xReturned) {
		pm_err("create riscv watch thread failed\n");
		ret = -EPERM;
		goto err_xHandle_Watch;
	}

	/* the task can not freeze. */
	ret = pm_task_register(subsys->xHandle_Signal, PM_TASK_TYPE_PM);
	if (ret) {
		pm_err("register riscv signal pm_task failed\n");
		goto err_Register_Signal_task;
	}

	ret = pm_task_register(subsys->xHandle_Watch, PM_TASK_TYPE_PM);
	if (ret) {
		pm_err("register riscv watch pm_task failed\n");
		goto err_Register_Watch_task;
	}

	/* register subsys driver to pm*/
	ret = pm_subsys_register(PM_SUBSYS_ID_RISCV, subsys);
	if (ret) {
		pm_err("register riscv subsys failed\n");
		goto err_Register_subsys_dsp;
	}

	goto out;

err_Register_subsys_dsp:
	pm_task_unregister(subsys->xHandle_Watch);
err_Register_Watch_task:
	pm_task_unregister(subsys->xHandle_Signal);
err_Register_Signal_task:
	vTaskDelete(subsys->xHandle_Watch);
err_xHandle_Watch:
	vTaskDelete(subsys->xHandle_Signal);
err_xHandle_Signal:
	vQueueDelete(subsys->xQueue_Result);
err_xQueue_Result:
	vQueueDelete(subsys->xQueue_Signal);
err_xQueue_Signal:
out:
	return ret;
}


int pm_riscv_init(void)
{
	pm_notify_register(&riscv_notify);

	riscv_subsys_init();
	return 0;
}


