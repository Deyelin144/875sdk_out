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

#include "gadget.h"
#include "hid.h"

static const uint16_t g_str_lang_id[] = {
	0x0304, 0x0409
};

static const uint16_t g_str_manufacturer[] = {
	0x0314, 'A', 'l', 'l', 'w', 'i', 'n', 'n', 'e', 'r'
};

static const uint16_t g_str_product[] = {
	0x030a, 'T', 'i', 'n', 'a',
};

static const uint16_t g_str_serialnumber[] = {
	0x0312, '2', '0', '0', '8', '0', '4', '1', '1'
};

static const uint16_t g_str_config[] = {
	0x0302,
};

static const uint16_t g_str_interface[] = {
	0x031c, 'H', 'I', 'D', ' ', 'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e'
};

static const uint16_t *hid_string_desc[USB_GADGET_MAX_IDX] = {
	g_str_lang_id,
	g_str_manufacturer,
	g_str_product,
	g_str_serialnumber,
	g_str_config,
	g_str_interface,
};

static struct usb_device_descriptor hid_device_desc = {
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	.bcdUSB             = 0x0200,
	.bDeviceClass       = 0x0,
	.bDeviceSubClass    = 0,
	.bDeviceProtocol    = 0,
	.bMaxPacketSize0    = 64,
	/* DYNAMIC */
	.idVendor           = 0x0525,
	/* DYNAMIC */
	.idProduct          = 0xa4ac,
	.bcdDevice          = 0x0409,
	.iManufacturer      = USB_GADGET_MANUFACTURER_IDX,
	.iProduct           = USB_GADGET_PRODUCT_IDX,
	.iSerialNumber      = USB_GADGET_SERIAL_IDX,
	.bNumConfigurations = 1
};

static struct usb_config_descriptor hid_config_desc = {
	.bLength         = USB_DT_CONFIG_SIZE,
	.bDescriptorType = USB_DT_CONFIG,
	.wTotalLength    = USB_DT_CONFIG_SIZE +
			   USB_DT_INTERFACE_SIZE +
			   USB_DT_ENDPOINT_SIZE +
			   USB_DT_ENDPOINT_SIZE +
			   HID_DT_SIZE,
	.bNumInterfaces      = 1,
	.bConfigurationValue = 1,
	.iConfiguration      = USB_GADGET_CONFIG_IDX,
	.bmAttributes        = 0xc0,
	.bMaxPower           = 0xfa                    /* 500mA */
};

static struct usb_interface_descriptor hid_intf_desc  = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC, */
	.bInterfaceNumber = 0,
	.bAlternateSetting  = 0x0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_HID,
	/* .bInterfaceSubClass  = DYNAMIC */
	.bInterfaceSubClass  = 0,
	/* .bInterfaceProtocol  = DYNAMIC */
	.bInterfaceProtocol  = 1,
	/* .iInterface          = DYNAMIC */
	.iInterface = 5
};

static struct usb_endpoint_descriptor hid_hs_ep_out = {
	.bLength         = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x2 | USB_DIR_OUT,
	.bmAttributes     = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize   = 0x0200, /* 512 Bytes */
	.bInterval        = 4
};

static struct usb_endpoint_descriptor hid_hs_ep_in = {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x1 | USB_DIR_IN,
	.bmAttributes     = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize   = 0x0200, /* 512 Bytes */
	.bInterval        = 4
};

static struct usb_endpoint_descriptor hid_fs_ep_out = {
	.bLength         = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x2 | USB_DIR_OUT,
	.bmAttributes     = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize   = 0x0200, /* 512 Bytes */
	.bInterval        = 10
};

static struct usb_endpoint_descriptor hid_fs_ep_in = {
	.bLength          = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType  = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x1 | USB_DIR_IN,
	.bmAttributes     = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize   = 0x0200, /* 512 Bytes */
	.bInterval        = 10
};

static struct hid_descriptor hidg_desc = {
	.bLength = HID_DT_SIZE,
	.bDescriptorType = HID_DT_HID,
	.bcdHID = 0x0102,
	.bCountryCode = 0x00,
	.bNumDescriptors = 0x1,
	/*.desc[0].bDescriptorType      = DYNAMIC */
	/*.desc[0].wDescriptorLength    = DYNAMIC */
	.desc[0].wDescriptorLength = 0x3f00,
};

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

void hal_hid_set_interface(uint8_t bInterfaceNumber,
			   uint8_t bInterfaceSubClass,
			   uint8_t bInterfaceProtocol,
			   uint8_t iInterface)
{
	hid_intf_desc.bInterfaceNumber = bInterfaceNumber;
	hid_intf_desc.bInterfaceSubClass = bInterfaceSubClass;
	hid_intf_desc.bInterfaceProtocol = bInterfaceProtocol;
	hid_intf_desc.iInterface = iInterface;
}

void hal_hid_set_vid_pid(uint16_t idVendor, uint16_t idProduct)
{
	hid_device_desc.idVendor = idVendor;
	hid_device_desc.idProduct = idProduct;
}

void hal_hid_set_hid_report(struct f_hidg *source_hidg)
{
	memcpy(&hidg, source_hidg, sizeof(struct f_hidg) + source_hidg->report_desc_length);
}

int hid_standard_req(struct usb_ctrlrequest *crq)
{
	gadget_debug("[%s] line:%d standard req:%u\n", __func__, __LINE__, crq->bRequest);
	switch (crq->bRequest) {
	case USB_REQ_SET_CONFIGURATION:
	/* init int-in ep */
	hal_udc_ep_enable(hid_hs_ep_in.bEndpointAddress,
			  hid_hs_ep_in.wMaxPacketSize,
			  hid_hs_ep_in.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK);
	/* init int-out ep */
	hal_udc_ep_enable(hid_hs_ep_out.bEndpointAddress,
			  hid_hs_ep_out.wMaxPacketSize,
			  hid_hs_ep_out.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK);
		break;
	default:
		break;
	}
	return 0;
}

int hid_class_req(struct usb_ctrlrequest *crq)
{
	unsigned short value = le16_to_cpu(crq->wValue);
	unsigned short length = le16_to_cpu(crq->wLength);
	char get_report_buf[512];
	char dt_hid_buf[512];
	char dt_report_buf[512];

	gadget_debug("%s ctrl_request: bRequestType:0x%x bRequest:0x%x Value:0x%x\n",
			__func__, crq->bRequestType, crq->bRequest, value);
	switch ((crq->bRequestType << 8) | crq->bRequest) {
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		| HID_REQ_GET_REPORT):
		gadget_debug("get_report\n");

		/* send an empty report */
		length = min_t(unsigned, length, hidg.report_length);
		memset(get_report_buf, 0x0, length);

		hal_udc_ep_set_buf(0 | USB_DIR_IN, &get_report_buf, length);
		goto respond;
		break;
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		| HID_REQ_GET_PROTOCOL):
		gadget_debug("get_protocol\n");

		goto stall;
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		| HID_REQ_SET_REPORT):
		gadget_debug("set_report | wLength=%d\n", length);
		goto stall;
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8
		| HID_REQ_SET_PROTOCOL):
		gadget_debug("set_protocol\n");
		goto stall;
		break;
	case ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8
		| USB_REQ_GET_DESCRIPTOR):
		switch (value >> 8) {
		case HID_DT_HID:
		{
			struct hid_descriptor hidg_desc_copy = hidg_desc;

			gadget_debug("USB_REQ_GET_DESCRIPTOR: HID\n");
			hidg_desc_copy.desc[0].bDescriptorType = HID_DT_REPORT;
			hidg_desc_copy.desc[0].wDescriptorLength =
					cpu_to_be16(hidg.report_desc_length);
			length = min_t(unsigned short, length,
					hidg_desc_copy.bLength);
			memcpy(dt_hid_buf, &hidg_desc_copy, length);

			hal_udc_ep_set_buf(0 | USB_DIR_IN, dt_hid_buf, length);
			goto respond;
			break;
		}
		case HID_DT_REPORT:
			gadget_debug("USB_REQ_GET_DESCRIPTOR: REPORT\n");
			length = min_t(unsigned short, length,
					hidg.report_desc_length);
			memcpy(dt_report_buf, hidg.report_desc, length);

			hal_udc_ep_set_buf(0 | USB_DIR_IN, dt_report_buf, length);
			goto respond;
			break;
		default:
			gadget_debug("Unknown descriptor request 0x%x\n",
					value >> 8);
			goto stall;
			break;
		}
		break;
	default:
		gadget_debug("Unknown request 0x%x\n", crq->bRequest);
		goto stall;
		break;
	}
stall:
	return -EOPNOTSUPP;
respond:
	return 0;
}

int hid_desc_init(struct usb_function_driver *fd)
{
	uint32_t config_desc_len = 0;
	int i = 0, ret = 0;
	void *buf = NULL;
	uint16_t *str;

	config_desc_len = hid_config_desc.bLength + hid_intf_desc.bLength
			+ hid_hs_ep_out.bLength + hid_hs_ep_in.bLength
			+ hidg_desc.bLength;
	gadget_info("config len:%u", config_desc_len);
	fd->config_desc = calloc(1, config_desc_len);
	if (!fd->config_desc) {
		gadget_err("no memory.\n");
		goto err;
	}

	hid_intf_desc.bInterfaceSubClass = hidg.bInterfaceSubClass;
	hid_intf_desc.bInterfaceProtocol = hidg.bInterfaceProtocol;

	buf = fd->config_desc;
	memcpy(buf, &hid_config_desc, hid_config_desc.bLength);
	buf += hid_config_desc.bLength;
	memcpy(buf, &hid_intf_desc, hid_intf_desc.bLength);
	buf += hid_intf_desc.bLength;
	/* hid descriptor start */
	struct hid_descriptor hidg_desc_copy = hidg_desc;
	hidg_desc_copy.desc[0].bDescriptorType = HID_DT_REPORT;
	hidg_desc_copy.desc[0].wDescriptorLength =
		cpu_to_le16(hidg.report_desc_length);

	memcpy(buf, &hidg_desc_copy, hidg_desc_copy.bLength);
	buf += hidg_desc_copy.bLength;
	memcpy(buf, &hid_hs_ep_out, hid_hs_ep_out.bLength);
	/* hid descriptor end */
	buf += hid_hs_ep_out.bLength;
	memcpy(buf, &hid_hs_ep_in, hid_hs_ep_in.bLength);

	hal_udc_device_desc_init(&hid_device_desc);
	hal_udc_config_desc_init(fd->config_desc, config_desc_len);

	for (i = 0; i < USB_GADGET_MAX_IDX; i++) {
		str = fd->strings[i] ? fd->strings[i] : (uint16_t *)hid_string_desc[i];
		hal_udc_string_desc_init(str);
	}

	fd->class_req = hid_class_req;
	fd->standard_req = hid_standard_req;
	fd->ep_addr = calloc(3, sizeof(uint8_t));
	if (!fd->ep_addr) {
		gadget_err("no memory.\n");
		goto err;
	}
	fd->ep_addr[0] = 0;
	fd->ep_addr[1] = hid_hs_ep_in.bEndpointAddress;
	fd->ep_addr[2] = hid_hs_ep_out.bEndpointAddress;

	gadget_info("usb hid desc init success\n");

	return 0;
err:
	if (fd->config_desc) {
		free(fd->config_desc);
		fd->config_desc = NULL;
	}

	if (fd->ep_addr) {
		free(fd->ep_addr);
		fd->ep_addr = NULL;
	}

	return 0;
}

int hid_desc_deinit(struct usb_function_driver *fd)
{
	if (fd->config_desc) {
		free(fd->config_desc);
		fd->config_desc = NULL;
	}

	if (fd->ep_addr) {
		free(fd->ep_addr);
		fd->ep_addr = NULL;
	}

	for (int i = 0; i < USB_GADGET_MAX_IDX; i++) {
		if (fd->strings[i] != NULL) {
			free(fd->strings[i]);
			fd->strings[i] = NULL;
		}
	}

	return 0;
}

struct usb_function_driver hid_usb_func = {
	.name        = "hid",
	.desc_init   = hid_desc_init,
	.desc_deinit = hid_desc_deinit,
};

int usb_gadget_hid_init(void)
{
	return usb_gadget_function_register(&hid_usb_func);
}

int usb_gadget_hid_deinit(void)
{
	return usb_gadget_function_unregister(&hid_usb_func);
}
