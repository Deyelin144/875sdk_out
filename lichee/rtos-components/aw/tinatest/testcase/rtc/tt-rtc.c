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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pm_state.h>
#include <pm_base.h>

#include <tinatest.h>
#include <sunxi_hal_rtc.h>

#include <barrier.h>

static int callback(void)
{
	printf("alarm interrupt\n");
	return 0;
}

extern int pm_trigger_suspend(suspend_mode_t mode); 
//tt rtctester
int tt_rtctest(int argc, char **argv)
{
    int ret = 0;
	unsigned int enable = 1;
	struct rtc_time rtc_tm;
	struct rtc_wkalrm wkalrm;

	hal_rtc_init();
	hal_rtc_register_callback(callback);

	if (hal_rtc_gettime(&rtc_tm))
		printf("sunxi rtc gettime error\n");

	wkalrm.enabled = 1;
	wkalrm.time = rtc_tm;
	if(rtc_tm.tm_sec > 5)
		 rtc_tm.tm_sec -= 5;
	else
		wkalrm.time.tm_sec += 5;

	if (hal_rtc_settime(&rtc_tm))
		printf("sunxi rtc settime error\n");

	if (hal_rtc_setalarm(&wkalrm))
		printf("sunxi rtc setalarm error\n");

	if (hal_rtc_getalarm(&wkalrm))
		printf("sunxi rtc getalarm error\n");

	if (hal_rtc_gettime(&rtc_tm))
		printf("sunxi rtc gettime error\n");

	//if do hal_rtc_alarm_irq_enable and hal_rtc_uninit, alarm will not work
	 hal_rtc_alarm_irq_enable(enable);



	ret = pm_trigger_suspend(PM_MODE_STANDBY);
	if (!ret)
		printf("===== %s: trigger suspend ok, return %d =====\n", __func__, ret);
	else {
		printf("===== %s: trigger suspend fail, return %d =====\n", __func__, ret);
		return -1;
	}


	printf("====rtctest successful!====\n");

	return 0;

}

testcase_init(tt_rtctest, rtctester, rtctester for tinatest);
