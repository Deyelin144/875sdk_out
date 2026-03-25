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
#include <string.h>
#include <unistd.h>

#include <tinatest.h>

#include "time_test.h"
#include "timer_test.h"
#include "queue_test.h"
#include "thread_test.h"
#include "sem_test.h"
#include "waitqueue_test.h"
#include "workqueue_test.h"
#include "atomic_test.h"
#include "cache_test.h"
#include "cfg_test.h"
#include "event_test.h"
#include "interrupt_test.h"
#include "mem_test.h"
#include "mutex_test.h"

// tt osaltester
int tt_osaltest(int argc, char **argv)
{
	int ret = 0;

	printf("=============Test for osal API==============\n");

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_TIME
	printf("=============Test osal time API start!==============\n");
	ret = cmd_osal_time_delay_test(1, NULL);
	if (ret) {
		printf("=============Test time delay API failed!==============\n");
		return ret;
	}

	ret = cmd_osal_time_sleep_test(1, NULL);
	if (ret) {
		printf("=============Test time sleep API failed!==============\n");
		return ret;
	}
	printf("=============Test osal time API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_TIMER
	printf("=============Test osal timer API start!==============\n");
	ret = cmd_osal_timer_test(1, NULL);
	if (ret) {
		printf("=============Test timer API failed!==============\n");
		return ret;
	}
	printf("=============Test osal timer API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_QUEUE
	printf("=============Test osal queue API start!==============\n");
	ret = cmd_osal_queue_test(1, NULL);
	if (ret) {
		printf("=============Test queue API failed!==============\n");
		return ret;
	}

	ret = cmd_osal_mailbox_test(1, NULL);
	if (ret) {
		printf("=============Test mailbox API failed!==============\n");
		return ret;
	}
	printf("=============Test osal queue API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_THREAD
	printf("=============Test osal thread API start!==============\n");
	ret = cmd_osal_thread_test(1, NULL);
	if (ret) {
		printf("=============Test thread API failed!==============\n");
		return ret;
	}
	printf("=============Test osal thread API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_SEM
	printf("=============Test osal sem API start!==============\n");
	ret = cmd_osal_sem_test(1, NULL);
	if (ret) {
		printf("=============Test sem API failed!==============\n");
		return ret;
	}
	printf("=============Test osal sem API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_WAITQUEUE
	printf("=============Test osal waitqueue API start!==============\n");
	ret = cmd_osal_waitqueue_test(1, NULL);
	if (ret) {
		printf("=============Test waitqueue API failed!==============\n");
		return ret;
	}
	printf("=============Test osal waitqueue API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_WORKQUEUE
	printf("=============Test osal workqueue API start!==============\n");
	ret = cmd_osal_workqueue_test(1, NULL);
	if (ret) {
		printf("=============Test workqueue API failed!==============\n");
		return ret;
	}
	printf("=============Test osal workqueue API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_MEM
	printf("=============Test osal mem API start!==============\n");
	ret = cmd_test_mem(1, NULL);
	if (ret) {
		printf("=============Test mem API failed!==============\n");
		return ret;
	}
	printf("=============Test osal mem API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_MUTEX
	printf("=============Test osal mutex API start!==============\n");
	ret = cmd_test_mutex(1, NULL);
	if (ret) {
		printf("=============Test mutex API failed!==============\n");
		return ret;
	}
	printf("=============Test osal mutex API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_CACHE
	printf("=============Test osal cache API start!==============\n");
	ret = cmd_test_cache(1, NULL);
	if (ret) {
		printf("=============Test cache API failed!==============\n");
		return ret;
	}
	printf("=============Test osal cache API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_ATOMIC
	printf("=============Test osal atomic API start!==============\n");
	ret = cmd_test_atomic(1, NULL);
	if (ret) {
		printf("=============Test atomic API failed!==============\n");
		return ret;
	}
	printf("=============Test osal atomic API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_CFG
	printf("=============Test osal cfg API start!==============\n");
	ret = cmd_cfg_test(1, NULL);
	if (ret) {
		printf("=============Test cfg API failed!==============\n");
		return ret;
	}
	printf("=============Test osal cfg API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_EVENT
	printf("=============Test osal event API start!==============\n");
	ret = cmd_event_test(1, NULL);
	if (ret) {
		printf("=============Test event API failed!==============\n");
		return ret;
	}
	printf("=============Test osal event API success!==============\n");
#endif

#ifdef CONFIG_COMPONENTS_OSTEST_OSAL_INTERRUPT
	printf("=============Test osal interrupt API start!==============\n");
	ret = cmd_test_interrupt(1, NULL);
	if (ret) {
		printf("=============Test interrupt API failed!==============\n");
		return ret;
	}
	printf("=============Test osal interrupt API success!==============\n");
#endif

	printf("=============Test osal all API success!==============\n");

	return ret;
}
testcase_init(tt_osaltest, osaltester, osaltester for tinatest);

