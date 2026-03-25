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
#include <stdio.h>
#include <string.h>
#include <hal_cache.h>

#include "sunxi_amp.h"
#include "xrbtc.h"


#ifdef CONFIG_ARCH_ARM_ARMV8M

static int xrbtc_hci_c2h_reg(uint8_t hciType, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen);
static int xrbtc_hci_h2c_cb_reg(uint8_t status, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen);

static int _xrbtc_init(void)
{
	return xrbtc_init();
}

static int _xrbtc_deinit(void)
{
	return xrbtc_deinit();
}

static int _xrbtc_enable(void)
{
	return xrbtc_enable();
}

static int _xrbtc_disable(void)
{
	return xrbtc_disable();
}

extern MAYBE_STATIC int bt_event_notify(uint8_t event, uint8_t value, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen);
static int xrbtc_hci_c2h_reg(uint8_t hciType, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	return bt_event_notify(0, hciType, pBuffStart, buffOffset, buffLen);
}

static int xrbtc_hci_h2c_cb_reg(uint8_t status, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	return bt_event_notify(1, status, pBuffStart, buffOffset, buffLen);
}

static int _xrbtc_hci_init(xrbtc_hci_c2h hci_c2h, xrbtc_hci_h2c_cb hci_h2c_cb)
{
	return xrbtc_hci_init(xrbtc_hci_c2h_reg, xrbtc_hci_h2c_cb_reg);
#if 0
	return xrbtc_hci_init(hci_c2h, hci_h2c_cb);
#endif
}

static int _xrbtc_hci_deinit(void)
{
	return xrbtc_hci_deinit();
}

static int _xrbtc_hci_h2c(uint8_t hciType, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	ssize_t ret = -1;
	hal_dcache_invalidate((unsigned long)(pBuffStart + buffOffset), buffLen);
	ret = xrbtc_hci_h2c(hciType, pBuffStart, buffOffset, buffLen);
	return ret;
}

static int _xrbtc_hci_c2h_cb(uint8_t status, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	ssize_t ret = -1;
	hal_dcache_invalidate((unsigned long)(pBuffStart + buffOffset), buffLen);
	ret = xrbtc_hci_c2h_cb(status, pBuffStart, buffOffset, buffLen);
	return ret;
}

static int32_t _xrbtc_sdd_init(uint32_t size)
{
	return xrbtc_sdd_init(size);
}

static int32_t _xrbtc_sdd_write(uint8_t *data, uint32_t len)
{
	int ret = -1;
	hal_dcache_invalidate((unsigned long) data, len);
	ret = xrbtc_sdd_write(data, len);
	return ret;
}

sunxi_amp_func_table bt_table[] = {
	{.func = (void *)&_xrbtc_init, .args_num = 0, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_deinit, .args_num = 0, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_enable, .args_num = 0, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_disable, .args_num = 0, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_hci_init, .args_num = 2, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_hci_deinit, .args_num = 0, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_hci_h2c, .args_num = 4, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_hci_c2h_cb, .args_num = 4, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_sdd_init, .args_num = 1, .return_type = RET_POINTER},
	{.func = (void *)&_xrbtc_sdd_write, .args_num = 2, .return_type = RET_POINTER},
};

#elif (defined CONFIG_ARCH_RISCV_RV64)
extern int bt_event_notify(uint8_t event, uint8_t value, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen);
static int _bt_event_notify(uint8_t event, uint8_t value, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	ssize_t ret = -1;
	hal_dcache_invalidate((unsigned long)(pBuffStart + buffOffset), buffLen);
	ret = bt_event_notify(event, value, pBuffStart, buffOffset, buffLen);
	return ret;
}

sunxi_amp_func_table bt_table[] = {
	{.func = (void *)_bt_event_notify, .args_num = 5, .return_type = RET_POINTER},
};

#endif
