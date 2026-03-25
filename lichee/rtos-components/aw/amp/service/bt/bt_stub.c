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
#include <reent.h>
#include <hal_cache.h>

#include "sunxi_amp.h"
#include "xrbtc.h"

#ifdef CONFIG_ARCH_RISCV_RV64

#include "bt_ctrl.h"

MAYBE_STATIC int xrbtc_init(void)
{
	return func_stub(RPCCALL_BT(xrbtc_init), 1, 0, NULL);
}

MAYBE_STATIC int xrbtc_deinit(void)
{
	return func_stub(RPCCALL_BT(xrbtc_deinit), 1, 0, NULL);
}

MAYBE_STATIC int xrbtc_enable(void)
{
	return func_stub(RPCCALL_BT(xrbtc_enable), 1, 0, NULL);
}

MAYBE_STATIC int xrbtc_disable(void)
{
	return func_stub(RPCCALL_BT(xrbtc_disable), 1, 0, NULL);
}

extern int bt_event_register(uint8_t event, bt_event_cb_func cb);
MAYBE_STATIC int xrbtc_hci_init(xrbtc_hci_c2h hci_c2h, xrbtc_hci_h2c_cb hci_h2c_cb)
{
	if (hci_c2h)
		bt_event_register(BT_EVENT_ID_C2H, (void *)hci_c2h);
	if (hci_h2c_cb)
		bt_event_register(BT_EVENT_ID_C2H_CB, (void *)hci_h2c_cb);

	void *args[2] = {0};
	args[0] = hci_c2h;
	args[1] = hci_h2c_cb;
	return func_stub(RPCCALL_BT(xrbtc_hci_init), 1, ARRAY_SIZE(args), (void *)&args);
}

MAYBE_STATIC int xrbtc_hci_deinit(void)
{
	return func_stub(RPCCALL_BT(xrbtc_hci_deinit), 1, 0, NULL);
}

MAYBE_STATIC int xrbtc_hci_h2c(uint8_t hciType, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	int ret = -1;
	void *args[4] = {0};
	char *buf_ali;
	buf_ali = amp_align_malloc(buffLen + buffOffset);
	if (!buf_ali) {
		return -1;
	}
	memcpy(buf_ali, pBuffStart, buffLen + buffOffset);
	args[0] = (void *)(uintptr_t)hciType;
	args[1] = buf_ali;
	args[2] = (void *)(uintptr_t)buffOffset;
	args[3] = (void *)(uintptr_t)buffLen;
	hal_dcache_clean((unsigned long)(uintptr_t)buf_ali,(unsigned long)(uintptr_t)(buffLen + buffOffset));
	ret = func_stub(RPCCALL_BT(xrbtc_hci_h2c), 1, ARRAY_SIZE(args), (void *)&args);
	amp_align_free(buf_ali);

	return ret;
}

MAYBE_STATIC int xrbtc_hci_c2h_cb(uint8_t status, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	int ret = -1;
	void *args[4] = {0};
	char *buf_ali;
	buf_ali = amp_align_malloc(buffLen + buffOffset);
	if (!buf_ali) {
		return -1;
	}
	memcpy(buf_ali, pBuffStart, buffLen + buffOffset);
	args[0] =(void *)(uintptr_t)status;
	args[1] = buf_ali;
	args[2] = (void *)(uintptr_t)buffOffset;
	args[3] = (void *)(uintptr_t)buffLen;
	hal_dcache_clean((unsigned long)(uintptr_t)buf_ali, (unsigned long)(uintptr_t)(buffLen + buffOffset));
	ret = func_stub(RPCCALL_BT(xrbtc_hci_c2h_cb), 1, ARRAY_SIZE(args), (void *)&args);
	amp_align_free(buf_ali);

	return ret;
}

MAYBE_STATIC int32_t xrbtc_sdd_init(uint32_t size)
{
	int ret = -1;
	void *args[1] = {0};
	args[0] = (void *)(uintptr_t)size;
	ret = func_stub(RPCCALL_BT(xrbtc_sdd_init), 1, ARRAY_SIZE(args), (void *)&args);
	return ret;
}

MAYBE_STATIC int32_t xrbtc_sdd_write(uint8_t *data, uint32_t len)
{
	int ret = -1;
	void *args[2] = {0};
	char *buf_ali;
	buf_ali = amp_align_malloc(len);
	if (!buf_ali) {
		return -1;
	}
	memcpy(buf_ali, data, len);
	args[0] = buf_ali;
	args[1] = (void *)(uintptr_t)len;
	hal_dcache_clean((unsigned long)(uintptr_t)buf_ali, len);
	ret = func_stub(RPCCALL_BT(xrbtc_sdd_write), 1, ARRAY_SIZE(args), (void *)&args);
	amp_align_free(buf_ali);

	return ret;
}

#elif (defined CONFIG_ARCH_ARM_ARMV8M)
MAYBE_STATIC int bt_event_notify(uint8_t event, uint8_t value, const uint8_t * pBuffStart, uint32_t buffOffset, uint32_t buffLen)
{
	int ret = -1;
	void *args[5] = {0};
	char *buf_ali;
	buf_ali = amp_align_malloc(buffLen + buffOffset);
	if (!buf_ali) {
		return -1;
	}
	memcpy(buf_ali, pBuffStart, buffLen + buffOffset);
	args[0] = (void *)(uintptr_t)event;
	args[1] = (void *)(uintptr_t)value;
	args[2] = buf_ali;
	args[3] = (void *)(uintptr_t)buffOffset;
	args[4] = (void *)(uintptr_t)buffLen;
	hal_dcache_clean((unsigned long)(uintptr_t)buf_ali, (unsigned long)(uintptr_t)buffLen + buffOffset);
	ret = func_stub(RPCCALL_BT(bt_event_notify), 1, ARRAY_SIZE(args), (void *)&args);
	amp_align_free(buf_ali);

	return ret;
}

#endif

