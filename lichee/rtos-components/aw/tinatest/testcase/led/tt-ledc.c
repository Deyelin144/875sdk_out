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
#include <tinatest.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../../../../../drivers/drv/leds/leds-sunxi.h"

#define DEFAULT_BRIGHTNESS	127
void tt_show_default_rgb()
{
	printf("======show default RGB======\n");

	sunxi_set_leds_brightness(1, DEFAULT_BRIGHTNESS << 8);
	vTaskDelay(1000/portTICK_RATE_MS);

	sunxi_set_leds_brightness(1, DEFAULT_BRIGHTNESS << 16);
	vTaskDelay(1000/portTICK_RATE_MS);

	sunxi_set_leds_brightness(1, DEFAULT_BRIGHTNESS << 0);
	vTaskDelay(1000/portTICK_RATE_MS);

	sunxi_set_leds_brightness(1, 0);

}

//tt ledctest
//or : tt ledctest R 100(set led show red, brightness : 100)
int tt_ledctest(int argc, char **argv)
{
	int brightness = 0;

	printf("========LEDC TEST========\n");

	sunxi_leds_init();

	if(argc < 3)
	{
		tt_show_default_rgb();
		return 0;
	}

	brightness = atoi(argv[2]);

	switch(argv[1][0])
	{
		case 'R' : brightness <<= 8; break;
		case 'G' : brightness <<= 16; break;
		case 'B' : brightness <<= 0; break;
		default  : "parameters err\n";
			   return -1;
	}
	sunxi_set_led_brightness(1, brightness);
	vTaskDelay(1000/portTICK_RATE_MS);
	sunxi_set_led_brightness(1, 0);

	return 0;
}
testcase_init(tt_ledctest, ledctest, ledctest for tinatest);
