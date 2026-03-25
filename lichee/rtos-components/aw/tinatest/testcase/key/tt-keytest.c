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
#include <tinatest.h>
#include <stdio.h>
#include "sunxi-input.h"

#if defined(CONFIG_DRIVERS_GPADC_KEY)
#include "../../../../../../drivers/drv/input/keyboard/gpadc-key.h"
#define DEVICE_NAME "gpadc-key"
#define KEY1 INPUT_KEY_VOLUMEUP
#define KEY2 INPUT_KEY_VOLUMEDOWN
#define KEY3 INPUT_KEY_MENU
#define KEY4 INPUT_KEY_PLAYPAUSE
#define KEY5 INPUT_KEY_POWER
#define NUM_OF_KEYS 4
#endif

#if defined(CONFIG_DRIVERS_SUNXI_KEYBOARD)
#include "../../../../../../drivers/drv/input/keyboard/sunxi-keyboard.h"
#define DEVICE_NAME "sunxi-keyboard"
#define KEY1 INPUT_KEY_VOLUMEUP
#define KEY2 INPUT_KEY_VOLUMEDOWN
#define KEY3 INPUT_KEY_POWER
#define KEY4 INPUT_KEY_PLAYPAUSE
#define KEY5 INPUT_KEY_MICMUTE
#define NUM_OF_KEYS 5
#endif

int tt_keytest(int argc, char **argv)
{
    int i;
    int key_count = 0;
    int fd = -1;
    int press_key_code[NUM_OF_KEYS] = {0};
    struct sunxi_input_event event;

    printf("========Now is %s test!========\n", DEVICE_NAME);
	
#if defined(CONFIG_DRIVERS_GPADC_KEY)
	sunxi_gpadc_key_init();
#endif

#if defined(CONFIG_DRIVERS_SUNXI_KEYBOARD)
    sunxi_keyboard_init();
#endif
    fd = sunxi_input_open(DEVICE_NAME);
    if (fd < 0) {
	    printf("====keyboard open err====\n");
	    return -1;
    }

	while (key_count < NUM_OF_KEYS) {
		sunxi_input_readb(fd, &event, sizeof(struct sunxi_input_event));

		if (event.type != INPUT_EVENT_KEY)
			continue;
		if (event.value == 0){
			printf("keyup!\n");
			continue;
		}

	    switch (event.code) {
		    case KEY1 :
			    printf("KEY1(%d) press\n", KEY1);
			    break;
		    case KEY2 :
			    printf("KEY2(%d) press\n", KEY2);
			    break;
		    case KEY3 :
			    printf("KEY3(%d) press\n", KEY3);
			    break;
		    case KEY4 :
			    printf("KEY4(%d) press\n", KEY4);
			    break;
		    case KEY5 :
			    printf("KEY5(%d) press\n", KEY5);
			    break;

		    default :
			    printf("unknow key press\n");
			    break;
	    }

	    for (i = 0; i < NUM_OF_KEYS; i++) {
		    if (press_key_code[i] == event.code)
			    break;
	    }

	    if (i == NUM_OF_KEYS)
		    press_key_code[key_count++] = event.code;
    }

    printf("all keys press\n");

    return 0;
}
testcase_init(tt_keytest, keytest, keytest for tinatest);
