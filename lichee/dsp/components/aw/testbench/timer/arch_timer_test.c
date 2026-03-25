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

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdio.h>

#include <hal_timer.h>

#define TIMER_NUM	1
#define TEST_COUNT	10

static SemaphoreHandle_t g_sem = NULL;
static int g_times = 0;

static void timer_handler(void *arg)
{
	BaseType_t ret;
	BaseType_t xHigherProTaskWoken = pdFALSE;

	ret = xSemaphoreGiveFromISR(g_sem, &xHigherProTaskWoken);
	if (ret == pdPASS) {
		portYIELD_FROM_ISR(xHigherProTaskWoken);
	}

	if (++g_times < TEST_COUNT)
		hal_arch_timer_update(TIMER_NUM, 1000 * g_times);
	else
		hal_arch_timer_update(TIMER_NUM, 0);
}

static int timer_test(void)
{
	if (hal_arch_timer_init(TIMER_NUM, timer_handler, NULL) < 0) {
		printf("Initialize arch timer%d failed!\n", TIMER_NUM);
		return -1;
	}

	if (hal_arch_timer_start(TIMER_NUM, 5000) < 0) {
		printf("Start arch timer%d failed!\n", TIMER_NUM);
		return -1;
	}

	return 0;
}

static void test_task(void *pdata)
{
	g_sem = xSemaphoreCreateBinary();
	if (!g_sem) {
		printf("Create semaphore failed!\n");
		goto end;
	}

	if (!timer_test()) {
		printf("Set arch timer test success! Begins after 5s...\n");
	} else {
		printf("Set arch timer test failed!\n");
		goto end;
	}

	while (1) {
		xSemaphoreTake(g_sem, portMAX_DELAY);
		printf("Ding Dong! %ds passed!\n", g_times);

		if (g_times == TEST_COUNT) {
			printf("Timer test success, exiting...\n");
			break;
		}
	}

end:
	vTaskDelete(NULL);
}

void arch_timer_test(void)
{
	uint32_t err = 0;

	err = xTaskCreate(test_task, "arch_timer", 0x1000, NULL, 1, NULL);
	if (err != pdPASS) {
		printf("Create arch timer test task failed!\n");
		return;
	}
}
