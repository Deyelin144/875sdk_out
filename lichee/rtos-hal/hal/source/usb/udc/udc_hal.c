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

#include "udc.h"
#include "usb_phy.h"
#include "udc_platform.h"

void hal_udc_device_desc_init(void *device_desc)
{
	udc_device_desc_init(device_desc);
}

void hal_udc_config_desc_init(void *config_desc, uint32_t len)
{
	udc_config_desc_init(config_desc, len);
}

void hal_udc_string_desc_init(const void *string_desc)
{
	udc_string_desc_init(string_desc);
}

void hal_udc_register_callback(udc_callback_t user_callback)
{
	udc_register_callback(user_callback);
}

void hal_udc_ep_enable(uint8_t ep_addr, uint16_t maxpacket, uint32_t ts_type)
{
	udc_ep_enable(ep_addr, maxpacket, ts_type);
}

void hal_udc_ep_disable(uint8_t ep_addr)
{
	udc_ep_disable(ep_addr);
}

int32_t hal_udc_ep_read(uint8_t ep_addr, void *buf, uint32_t len)
{
	return udc_ep_read(ep_addr, buf, len);
}

int32_t hal_udc_ep_write(uint8_t ep_addr, void *buf, uint32_t len)
{
	return udc_ep_write(ep_addr, buf, len);
}
void hal_udc_ed_test(const char *buf, size_t count)
{
	udc_ed_test(buf, count);
}

void hal_udc_phy_range_show(int usbc_num)
{
	USBPHY_REGISTER_T *musb_phy = usbc_get_musb_phy();
	usb_phy_range_show((unsigned long)musb_phy);
}

void hal_udc_phy_range_set(int usbc_num, int val)
{
	USBPHY_REGISTER_T *musb_phy = usbc_get_musb_phy();
	usb_phy_range_set((unsigned long)musb_phy, val);
}

void hal_udc_ep_set_buf(uint8_t ep_addr, void *buf, uint32_t len)
{
	udc_ep_set_buf(ep_addr, buf, len);
}

int32_t hal_udc_init(void)
{
	return sunxi_udc_init();
}

int32_t hal_udc_deinit(void)
{
	return sunxi_udc_deinit();
}
