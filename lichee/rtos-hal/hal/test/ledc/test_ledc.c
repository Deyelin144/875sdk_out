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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <hal_timer.h>
#include <hal_cmd.h>
#include "sunxi_hal_ledc.h"

#define DEFAULT_BRIGHTNESS	127

void show_default_rgb(void)
{
	printf("======show default RGB======\n");
	sunxi_set_led_brightness(1, DEFAULT_BRIGHTNESS << 8);
	sunxi_set_led_brightness(1, DEFAULT_BRIGHTNESS << 16);
	sunxi_set_led_brightness(1, DEFAULT_BRIGHTNESS << 0);
	sunxi_set_led_brightness(1, 0);
}

int ledc_test_single(int argc, char **argv)
{
	int brightness = 0;
	int led_num;
	int err;

	printf("========SINGLE LEDC TEST========\n");

	err = hal_ledc_init();
	if (err) {
		printf("ledc init error\n");
		return -1;
	}

	if(argc < 3)
	{
		show_default_rgb();
		printf("uasge : hal_ledc [led_num] [RGB] [rgb_brightness], eg: hal_ledc 1 R 100\n");
		return -1;
	}

	led_num = atoi(argv[1]);
	if (led_num < 1 || led_num > 1024)
	{
		printf("The led_num you entered should be between 1 and 1024\n");
	}

	brightness = atoi(argv[3]);

	switch(argv[2][0])
	{
		case 'R' : brightness <<= 8; break;
		case 'G' : brightness <<= 16; break;
		case 'B' : brightness <<= 0; break;
		default  : printf("parameters err\n");
			   return -1;
	}

	err = sunxi_set_led_brightness(led_num, brightness);
	if (err) {
		printf("set all led error\n");
		return -1;
	}

	printf("led is %d\n", led_num);
	printf("brightness is %d\n", brightness);

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(ledc_test_single, hal_ledc, drv single ledc test_code);

int ledc_test_all(int argc, char **argv)
{
	int brightness = 0;
	int led_num;
	int err;

	printf("========ALL LEDC TEST========\n");

	err = hal_ledc_init();
	if (err) {
		printf("ledc init error\n");
		return -1;
	}

	if(argc < 3)
	{
		printf("uasge : hal_ledc_all [led_num] [RGB] [rgb_brightness], eg: hal_ledc_all 34 R 100\n");
		return -1;
	}

	led_num = atoi(argv[1]);
	if (led_num < 1 || led_num > 1024)
	{
		printf("The led_num you entered should be between 1 and 1024\n");
	}

	brightness = atoi(argv[3]);

	switch(argv[2][0])
	{
		case 'R' : brightness <<= 8; break;
		case 'G' : brightness <<= 16; break;
		case 'B' : brightness <<= 0; break;
		default  : printf("parameters err\n");
			   return -1;
	}

	err = sunxi_set_all_led(led_num, brightness);
	if (err) {
		printf("set all led error\n");
		return -1;
	}

	return 0;
}

FINSH_FUNCTION_EXPORT_CMD(ledc_test_all, hal_ledc_all, drv all ledc test_code);
