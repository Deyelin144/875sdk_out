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
#include "link.h"

extern int standby_main(void);

char lpsram_name[] = "NONE";
uint8_t lpsram_resp;
uint8_t lpsram_buff;
struct psram_request lpsram_mrq = {
	.cmd.resp = &lpsram_resp,
	.data.buff = &lpsram_buff,
};
struct psram_ctrl ctrl = {
	.mrq = &lpsram_mrq,
};

__attribute__((section(".standby_head"))) standby_head_t  g_standby_head =
{
	.version      = HEAD_VERSION,

	.build_info   = {
		.commit_id    = HEAD_COMMIT_ID,
		.change_id    = HEAD_CHANGE_ID,
		.build_date   = HEAD_BUILD_DATE,
		.build_author = HEAD_BUILD_AUTHOR,
	},

	.enter = standby_main,
	.paras_start =  &_standby_paras_start,
	.paras_end   =  &_standby_paras_end,
	.code_start  =  &_standby_code_start,
	.code_end    =  &_standby_code_end,
	.bss_start   =  &_standby_bss_start,
	.bss_end     =  &_standby_bss_end,
	.stack_limit =  &_standby_stack_limit,
	.stack_base  =  &_standby_stack_base,

	.stage_record = 1,

	.hpsram_inited = 1,
	.lpsram_inited = 1,

	.lpsram_para.name = lpsram_name,
	.lpsram_para.ctrl = &ctrl,

	.lpsram_crc = {
		.crc_start  = 0x08000000,
		.crc_len    = 0x00800000,
		.crc_enable = 0,
		.crc_before = 0,
		.crc_after  = 0,
	},
	.hpsram_crc = {
		.crc_start  = 0x0c000000,
		.crc_len    = 0x00800000,
		.crc_enable = 0,
		.crc_before = 0,
		.crc_after  = 0,
	},

	.mode = 0,
	.wakesrc_active = 0,
	.wakeup_source = 0,
	.time_to_wakeup_ms = 0,
	.cpufreq = 192000000,
	.chipv = 0,
	.rccal_en = 1,

	.pwrcfg = 0,
	.clkcfg = 0,
	.anacfg = 0,

	.suspend_moment = 0,
	.resume_moment = 0,
};

standby_head_t *head = (standby_head_t *)&g_standby_head;

