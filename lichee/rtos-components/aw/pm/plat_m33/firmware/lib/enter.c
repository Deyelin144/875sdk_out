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

#include "type.h"
#include "head.h"
#include "arch.h"
#include "driver_lpsram.h"
#include "driver_hpsram.h"
#include "systick.h"
#include "pm_wakeres.h"
#include "pm_testlevel.h"
#include "pm_base.h"

#include "delay.h"
#include "standby_clk.h"
#include "standby_power.h"
#include "cache.h"
#include "standby_scb.h"
#include <wakeup.h>

void suspend_moment_record(void)
{
	pm_wdt_stop();
	head->suspend_moment = pm_gettime_ns();
}

void resume_moment_record(void)
{
	pm_wdt_start();
	head->resume_moment = pm_gettime_ns();
}

static inline void record_stage(uint32_t val)
{
	if (head->stage_record)
		writel(val, PM_STANDBY_STAGE_M33_REC_REG);
}

static void pm_reg_capture_start(standby_head_t *head)
{
	if (head->reg_dump_addr == (void *)0) return;
	uint32_t *reg_save_addr = head->reg_dump_addr;
	reg_save_addr += 2;
	for (int j = head->reg_dump_addr[0]; j <= head->reg_dump_addr[1]; j += 0x04) {
		*(reg_save_addr++) = readl(j);
	}
}

int standby_enter(void)
{
	record_stage(PM_TEST_RECORDING_ENTER | 0x10);
	/* save SCB regs */
	scb_save();

	/* clean & invalid dcache before psram enter retention
	 * to avoid to flush dada to psram later.
	 */
	standby_CleanInvalidateDCache();
	standby_DisableDCache();
	record_stage(PM_TEST_RECORDING_ENTER | 0x20);

	if (head->hpsram_inited && hpsram_is_running()) {
		if (head->hpsram_crc.crc_enable) {
			head->hpsram_crc.crc_before = hpsram_crc(head);
			hpsram_crc_save(head, HPSRAM_CRC_BEFORE);
		}
		record_stage(PM_TEST_RECORDING_ENTER | 0x30);
		hpsram_enter_retention(head);
		head->hpsram_inited = HPSRAM_SUSPENDED;
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0x40);

	if (head->lpsram_inited && lpsram_is_running()) {
		if (head->lpsram_crc.crc_enable) {
			head->lpsram_crc.crc_before = lpsram_crc(head);
			lpsram_crc_save(head, LPSRAM_CRC_BEFORE);
		}
		record_stage(PM_TEST_RECORDING_ENTER | 0x50);
		lpsram_enter_retention(head);
		head->lpsram_inited = LPSRAM_SUSPENDED;
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0x60);

	/* analog
	 * RTC GPIO: do not need ctrl
	 * AON GPIO: may have clk gating
	 */

	/* close systick. systick rely on DPLL clk */
	cpu_disable_systick();
	record_stage(PM_TEST_RECORDING_ENTER | 0x70);

	/* clk */
	clk_suspend(head);
	record_stage(PM_TEST_RECORDING_ENTER | 0x80);

	/* power. controled by hardware */
	ldo_disable(head);
	record_stage(PM_TEST_RECORDING_ENTER | 0x90);


	if (head->time_to_wakeup_ms > 0) {
		enable_wuptimer(head->time_to_wakeup_ms);
	} else {
		disable_wuptimer();
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0xa0);

	pm_reg_capture_start(head);

	switch (head->mode)  {
		case PM_MODE_ON:
			break;
		case PM_MODE_SLEEP:
			_fw_cpu_sleep();
			break;
		case PM_MODE_STANDBY:
			writel(BOOT_FALG_DEEP_SLEEP, CPU_BOOT_FLAG_REG);
			_fw_cpu_standby();
			writel(BOOT_FALG_COLD_RESET, CPU_BOOT_FLAG_REG);
			break;
		case PM_MODE_HIBERNATION:
			hibernation_mode_set();
			writel(BOOT_FALG_COLD_RESET, CPU_BOOT_FLAG_REG);
			_fw_cpu_standby();
			/* cold reset without return */
			hibernation_mode_restore();
			break;
		default:
			break;
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0xb0);


	ldo_enable(head);
	record_stage(PM_TEST_RECORDING_ENTER | 0xc0);

	clk_resume(head);
	record_stage(PM_TEST_RECORDING_ENTER | 0xd0);

	cpu_enable_systick();
	record_stage(PM_TEST_RECORDING_ENTER | 0xe0);

	if (head->lpsram_inited == LPSRAM_SUSPENDED) {
		lpsram_exit_retention(head);
		record_stage(PM_TEST_RECORDING_ENTER | 0xf0);
		if (head->lpsram_crc.crc_enable) {
			head->lpsram_crc.crc_after = lpsram_crc(head);
			lpsram_crc_save(head, LPSRAM_CRC_AFTER);
			if (head->lpsram_crc.crc_after != head->lpsram_crc.crc_before) {
				writel(LSPSRAM_CRC_FAIL_FLAG, PM_STANDBY_STAGE_PSRAM_REC_REG);
				if (!head->stage_record)
					pm_wdt_restart();
				while(1);
                       }
		}
		head->lpsram_inited = LPSRAM_RESUMED;
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0xf1);

	if(head->hpsram_inited == HPSRAM_SUSPENDED) {
		hpsram_exit_retention(head);
		record_stage(PM_TEST_RECORDING_ENTER | 0xf2);
		if (head->hpsram_crc.crc_enable) {
			head->hpsram_crc.crc_after = hpsram_crc(head);
			hpsram_crc_save(head, HPSRAM_CRC_AFTER);
			if (head->hpsram_crc.crc_after != head->hpsram_crc.crc_before) {
				writel(HSPSRAM_CRC_FAIL_FLAG, PM_STANDBY_STAGE_PSRAM_REC_REG);
				if (!head->stage_record)
					pm_wdt_restart();
				while(1);
                       }
		}
		head->hpsram_inited = HPSRAM_RESUMED;
	}
	record_stage(PM_TEST_RECORDING_ENTER | 0xf3);

	/* init cache, restore SCB regs can't restore cache */
	standby_EnableICache();
	standby_EnableDCache();

	/* restore M33 SCB */
	scb_restore();
	record_stage(PM_TEST_RECORDING_ENTER | 0xf4);

	return 0;
}

