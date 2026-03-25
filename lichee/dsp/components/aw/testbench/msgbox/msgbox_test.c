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
#include <stdint.h>
#include <queue.h>
#include <sunxi_hal_common.h>
#include <hal_msgbox.h>

#define TASK_TEST_STACK_LEN 1024
#define MESSAGE_QUEUE_LEN   1000
static TaskHandle_t test_task;
static StackType_t  test_stk[TASK_TEST_STACK_LEN];
static volatile int send_finish = 1;
static StaticTask_t tcb_test;
struct messagebox *msgbox_init_sx(enum msgbox_direction dir);

static QueueHandle_t q_handle;
static u32 msg_queue[MESSAGE_QUEUE_LEN];
static StaticQueue_t msg_ctrl_queue;

/* static struct messagebox *msg; */
static struct msg_channel *send_channel;
static struct msg_channel *rev_channel;

static int send_callback(unsigned long v, void *p)
{
	send_finish = 1;

	return 0;
}

static int arisc_send_to_cpu(struct msg_channel *ch, unsigned char *d, int len)
{
	int result;

	while(!send_finish);
	send_finish = 0;

	result = msgbox_channel_send_data(ch, d, len);
	while(!send_finish);

	return result;
}

static int rev_callback(unsigned long v, void *p)
{
	BaseType_t ret;
	BaseType_t xHigherProTaskWoken = pdFALSE;
	ret = xQueueSendFromISR(q_handle, &v, &xHigherProTaskWoken);
	if (ret == pdPASS) {
		portYIELD_FROM_ISR(xHigherProTaskWoken);
	}

	return 0;
}

static void task_test(void *parg)
{
	printf("msgbox test start\n");
	q_handle = xQueueCreateStatic(MESSAGE_QUEUE_LEN, sizeof(uint32_t),
				      (uint8_t *)msg_queue, &msg_ctrl_queue);

	rev_channel = msgbox_alloc_channel(
		msgbox_cpu, 0, MSGBOX_CHANNEL_RECEIVE, rev_callback, NULL);
	send_channel = msgbox_alloc_channel(msgbox_cpu, 1, MSGBOX_CHANNEL_SEND,
					    send_callback, NULL);

	for (;;) {
		u32 rev;

		if (pdPASS == xQueueReceive(q_handle, &rev, portMAX_DELAY)) {
			printf("rev:%d\n", rev);
			arisc_send_to_cpu(send_channel, (unsigned char *)&rev,
					  sizeof(rev));
		}
	}
}

void init_test_msgbox(void)
{
	messagebox_init();
	test_task = xTaskCreateStatic(task_test, "for_test",
				      TASK_TEST_STACK_LEN, NULL,
				      configMAX_PRIORITIES - 4, test_stk,
				      &tcb_test);

}
