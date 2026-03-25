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
#include <stdio.h>
#include <stdlib.h>
#include <hal_cmd.h>
#include <hal_thread.h>
#include <sunxi-input.h>
#include "hidd.h"

#define HIDD_VERSION		("AW-V0.1")
#define KEYBOARD_EVENT_SIZE	8

extern int sunxi_gpadc_key_init(void);

hal_thread_t hid_input_thread;
hal_thread_t hid_output_thread;

static inline void hidd_version(void)
{
	printf("hidd version:%s, compiled on: %s %s\n", HIDD_VERSION, __DATE__, __TIME__);
}

static void hid_input_thread_func(void *arg)
{
	int ret = 0, fd = -1, i = 0;
	struct sunxi_input_event event;
	char *buffer = NULL;

	buffer = hal_malloc(KEYBOARD_EVENT_SIZE);
	if (!buffer) {
		hid_err("hid malloc failed\n");
		goto err;
	}

	sunxi_gpadc_key_init();
	fd = sunxi_input_open("gpadc-key");
	if (fd < 0) {
		hid_err("hid open gpadc key failed\n");
		goto buf_err;
	}

	while (1) {
		sunxi_input_readb(fd, &event, sizeof(struct sunxi_input_event));
		hid_debug("get key press:%d\n", event.code);

		if (event.type != INPUT_EVENT_KEY)
			continue;
		if (event.value == 0) {
			hid_debug("keyup\n");
			continue;
		}

		memset(buffer, 0, KEYBOARD_EVENT_SIZE);
		buffer[2] = (uint8_t)event.code;
		ret = usb_gadget_function_write(HID_INT_IN, buffer, KEYBOARD_EVENT_SIZE);
		hid_debug("usb write count:%d\n", ret);

		memset(buffer, 0, KEYBOARD_EVENT_SIZE);
		ret = usb_gadget_function_write(HID_INT_IN, buffer, KEYBOARD_EVENT_SIZE);
		hid_debug("usb write count:%d\n", ret);
	}

buf_err:
	hal_free(buffer);
err:
	hal_thread_stop(hid_input_thread);

	return;
}

static void hid_output_thread_func(void *arg)
{
	int ret = 0, i = 0;
	int size = 512;
	char buf[512];

	while (1) {
		ret = usb_gadget_function_read(HID_INT_OUT, buf, size);
		if (ret < 0) {
			hid_err("hid usb read failed\n");
			break;
		} else {
			hid_debug("read %d bytes from hid\n", ret);
			for (i = 0; i < ret; i++) {
				printf("0x%02x ", buf[i]);
				if (i % 15 == 0 && i != 0)
					printf("\n");
			}
		}
	}

	hal_thread_stop(hid_output_thread);
	return;
}

static struct f_hidg hidg = {
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 1, /* keyboard */
	.report_length = 8,
	.report_desc_length = 63,
	.report_desc = {
		0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)           */
		0x09, 0x06,     /* USAGE (Keyboard)                       */
		0xa1, 0x01,     /* COLLECTION (Application)               */
		0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
		0x19, 0xe0,     /*   USAGE_MINIMUM (Keyboard LeftControl) */
		0x29, 0xe7,     /*   USAGE_MAXIMUM (Keyboard Right GUI)   */
		0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x01,     /*   LOGICAL_MAXIMUM (1)                  */
		0x75, 0x01,     /*   REPORT_SIZE (1)                      */
		0x95, 0x08,     /*   REPORT_COUNT (8)                     */
		0x81, 0x02,     /*   INPUT (Data,Var,Abs)                 */
		0x95, 0x01,     /*   REPORT_COUNT (1)                     */
		0x75, 0x08,     /*   REPORT_SIZE (8)                      */
		0x81, 0x03,     /*   INPUT (Cnst,Var,Abs)                 */
		0x95, 0x05,     /*   REPORT_COUNT (5)                     */
		0x75, 0x01,     /*   REPORT_SIZE (1)                      */
		0x05, 0x08,     /*   USAGE_PAGE (LEDs)                    */
		0x19, 0x01,     /*   USAGE_MINIMUM (Num Lock)             */
		0x29, 0x05,     /*   USAGE_MAXIMUM (Kana)                 */
		0x91, 0x02,     /*   OUTPUT (Data,Var,Abs)                */
		0x95, 0x01,     /*   REPORT_COUNT (1)                     */
		0x75, 0x03,     /*   REPORT_SIZE (3)                      */
		0x91, 0x03,     /*   OUTPUT (Cnst,Var,Abs)                */
		0x95, 0x06,     /*   REPORT_COUNT (6)                     */
		0x75, 0x08,     /*   REPORT_SIZE (8)                      */
		0x15, 0x00,     /*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x65,     /*   LOGICAL_MAXIMUM (101)                */
		0x05, 0x07,     /*   USAGE_PAGE (Keyboard)                */
		0x19, 0x00,     /*   USAGE_MINIMUM (Reserved)             */
		0x29, 0x65,     /*   USAGE_MAXIMUM (Keyboard Application) */
		0x81, 0x00,     /*   INPUT (Data,Ary,Abs)                 */
		0xc0            /* END_COLLECTION                         */

	},
};

int hidd_main(void)
{
	int ret = 0;

	hidd_version();

	/* uint8_t bInterfaceNumber, uint8_t bInterfaceSubClass,
	 * uint8_t bInterfaceProtocol, uint8_t iInterface
	 */
	hal_hid_set_interface(0, 0, 1, 5);

	/* uint16_t idVendor, uint16_t idProduct
	 */
	hal_hid_set_vid_pid(0x0525, 0xa4ac);
	/* struct f_hidg
	 */
	hal_hid_set_hid_report(&hidg);

	ret = usb_gadget_hid_init();
	if (ret < 0) {
		hid_err("usb_gadget_hid_init failed\n");
		return ret;
	}

	ret = usb_gadget_function_enable("hid");
	if (ret < 0) {
		hid_err("usb_gadget_function_enable hid failed\n");
		goto gadget_err;
	}

	hid_input_thread = hal_thread_create(hid_input_thread_func, NULL,
			"hid_input_thread", 2048, HAL_THREAD_PRIORITY_APP);
	if (!hid_input_thread) {
		hid_err("hid deamon thread create failed\n");
		goto enable_err;
	}

	hid_output_thread = hal_thread_create(hid_output_thread_func, NULL,
			"hid_output_thread", 2048, HAL_THREAD_PRIORITY_APP);
	if (!hid_output_thread) {
		hid_err("hid deamon thread create failed\n");
		goto thread_err;
	}

	hid_info("hid daemon service init successful\n");

	return 0;

thread_err:
	hal_thread_stop(hid_input_thread);
	hid_input_thread = NULL;
enable_err:
	usb_gadget_function_disable("hid");
gadget_err:
	usb_gadget_hid_deinit();

	return ret;
}

int hidd_exit(void)
{
	hid_info("FreeRTOS hidd exit\n");

	hal_thread_stop(hid_input_thread);
	hid_input_thread = NULL;

	hal_thread_stop(hid_output_thread);
	hid_output_thread = NULL;

	usb_gadget_function_disable("hid");
	usb_gadget_hid_deinit();

	return 0;
}

static void usage(void)
{
	printf("Usgae: hidd [option]\n");
	printf("-v,          hidd version\n");
	printf("-h,          hidd help\n");
	printf("-d,          hidd debug level\n");
	printf("\n");
}

int cmd_hidd(int argc, char *argv[])
{
	int ret = 0, c;

	optind = 0;
	while ((c = getopt(argc, argv, "vhd:")) != -1) {
		switch (c) {
		case 'v':
			hidd_version();
			return 0;
		case 'd':
			g_hidd_debug_mask = atoi(optarg);
			return 0;
		case 'h':
		default:
			usage();
			return 0;
		}
	}

	ret = hidd_main();

	return ret;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_hidd, hidd, hidd service);
