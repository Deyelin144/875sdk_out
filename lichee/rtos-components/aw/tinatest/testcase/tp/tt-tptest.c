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
#include <stdint.h>
#include <string.h>
#include <tinatest.h>
#include <stdio.h>
#include "sunxi-input.h"

extern int gt911_init(void);

#define GT911_DEV_NAME	"touchscreen"

void tt_tp_test_task(void)
{
	int fd;
	int x = -1, y = -1, key_count = 0;
	struct sunxi_input_event event;
	memset(&event, 0, sizeof(struct sunxi_input_event));

	fd = sunxi_input_open(GT911_DEV_NAME);

	if (fd < 0) {
		printf("gpio key open err\n");
		vTaskDelete(NULL);
		return;
	}

	while(key_count < 400) {
		sunxi_input_read(fd, &event, sizeof(struct sunxi_input_event));
	//	printf("read event %d %d %d", event.type, event.code, event.value);

		if (event.type == INPUT_EVENT_ABS) {
			switch (event.code) {
				case INPUT_ABS_MT_POSITION_XTION_X:
					x = event.value;
					break;
				case INPUT_ABS_MT_POSITION_YTION_Y:
					y = event.value;
					break;
			}
		} else if (event.type == INPUT_EVENT_KEY) {
			if (event.code == BTN_TOUCH && event.value == 0)
				printf("event.code == BTN_TOUCH, value = %d\n", event.value);
		}
		if (x >= 0 && y >= 0) {
			printf("====press (%d, %d)====\n", x, y);
			x = -1;
			y = -1;
			key_count ++;
		}
	}
	printf("=======tptest successful!========\n");
	vTaskDelete(NULL);
}

int tt_tptest(int argc, char **argv)
{
	int ret = -1;

	ret = gt911_init();

	if (ret < 0)
		printf("gt911 init fail\n");

	portBASE_TYPE task_ret;
	task_ret = xTaskCreate(tt_tp_test_task, (signed portCHAR *) "tp_test_task", 1024, NULL, 0, NULL);

	return 0;
}

testcase_init(tt_tptest, tptest, tptest for tinatest);
