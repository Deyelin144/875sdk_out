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
#include <stdlib.h>
#include <stdio.h>
#include <tinatest.h>

#include "thermal_external.h"

int tt_thermal(int argc, char **argv)
{
	int times = strtol(CONFIG_THERMAL_LOOP_TIMES, NULL, 0);

	int ret = 0;
	int num;
	int *id_buf;
	int *temp_buf;

	printf("=============TEST FOR THERMAL===========\n");

	while(times) {
		times --;

		num = thermal_external_get_zone_number();

		temp_buf = malloc(num * sizeof(int));
		if (temp_buf == NULL) {
			printf("temp_buf malloc fail\n");
			return -1;
		}
		id_buf = malloc(num * sizeof(int));
		if (id_buf == NULL) {
			printf("id_buf malloc fail\n");
			free(temp_buf);
			return -1;
		}

		ret = thermal_external_get_temp(id_buf, temp_buf, num);

		if(ret) {
			printf("get temp fail!\n");
			return ret;
		}

		printf("get thermal list:\n");
		for(int i=0; i < num; i++) {
			printf("thermal zone%d: %d\n", id_buf[i], temp_buf[i]);
		}

		printf("======LOOP TESTED OK TIMES:%d======\n", strtol(CONFIG_THERMAL_LOOP_TIMES, NULL, 0) - times);
		printf("======LOOP LEFT TIMES:%d======\n", times);

		//hal_mdelay(500);
	}

	printf("============TEST FOR THERMAL OK!============\n");

	free(temp_buf);
	free(id_buf);

	return ret;
fail:
	printf("============TEST FOR THERMAL FAIL!============\n");
	return ret;
}
testcase_init(tt_thermal, thermal, thermal for tinatest);
