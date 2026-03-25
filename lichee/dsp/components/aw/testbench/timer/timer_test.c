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
#include <stdint.h>

#include <interrupt.h>
#include <hal_timer.h>

#define TEST_COUNT	10

static SemaphoreHandle_t g_sem = NULL;
static int g_times = 0;

static void timer_handler(void *data)
{
	BaseType_t ret;
	BaseType_t xHigherProTaskWoken = pdFALSE;
	ret = xSemaphoreGiveFromISR(g_sem, &xHigherProTaskWoken);
	if (ret == pdPASS) {
		portYIELD_FROM_ISR(xHigherProTaskWoken);
	}
}

static int timer_demo(void)
{
	uint32_t irq_num;

	hal_timer_init();

	hal_timer_set_pres(TIMER0, TMR_2_PRES);
	hal_timer_set_mode(TIMER0, TMR_CONTINUE_MODE);
	hal_timer_set_interval(TIMER0, 0x0EE0);

	irq_num = hal_timer_id_to_irq(TIMER0);
	printf("irq_num = %d\n", irq_num);

	hal_timer_irq_request(irq_num, timer_handler, 0, NULL);
	hal_timer_irq_enable(irq_num);

	return 0;
}

static void test_task(void *pdata)
{
	uint32_t irq_num = hal_timer_id_to_irq(TIMER0);

	g_sem = xSemaphoreCreateBinary();
	if (!g_sem) {
		printf("Create semaphore failed!\n");
		goto end;
	}

	if (!timer_demo()) {
		printf("Set timer test success! Begins...\n");
	} else {
		printf("Set timer test failed!\n");
		goto end;
	}

	while (1) {
		xSemaphoreTake(g_sem, portMAX_DELAY);
		printf("Ding Dong! %d times!\n", g_times);

		if (g_times == TEST_COUNT) {
			hal_timer_irq_disable(irq_num);
			hal_timer_irq_free(irq_num);

			printf("Timer test success, exiting...\n");
			break;
		}
	}

end:
	vTaskDelete(NULL);
}

void timer_test(void)
{
	uint32_t err = 0;

	err = xTaskCreate(test_task, "timer_test", 0x1000, NULL, 1, NULL);
	if (err != pdPASS) {
		printf("Create timer test task failed!\n");
		return;
	}
}
