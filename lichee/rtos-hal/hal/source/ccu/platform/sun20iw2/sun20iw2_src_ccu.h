/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 *the the People's Republic of China and other countries.
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


#ifndef __SUN20IW2_SRC_CCU_H__
#define __SUN20IW2_SRC_CCU_H__

enum sun20iw2_src_ccu_clk_id
{
	/*
	 Currently Xtensa toolchain don't support to use value 0 as _parent parameter
	 into the macor CLK_HW_INFO. Otherwise the toolchain will crash. So we can let
	 the id of first clk in clock source controller is 1 to avoid this issue, or we
	 can let a clk using the first id 0 which will not as _parent parameter
	 into the macor CLK_HW_INFO.

	 The error log is below:
	 Signal: Segmentation fault in Writing WHIRL file phase.
	 Error: Signal Segmentation fault in phase Writing WHIRL file -- processing aborted
	 xt-xcc ERROR:	/opt/RI-2020.5-linux/XtensaTools/libexec/xcc/wgen died due to signal 4
	 xt-xcc ERROR:	core dumped
	 */
	CLK_SRC_HOSC_24M,
	CLK_SRC_UNKNOWN,
	CLK_SRC_USELESS,
	CLK_SRC_HOSC_24576K,
	CLK_SRC_HOSC_26M,
	CLK_SRC_HOSC_32M,
	CLK_SRC_HOSC_40M,
	CLK_SRC_LOSC,
	CLK_SRC_RC_LF,
	CLK_SRC_RC_HF,

	/* the advanced divider hw not support, using the clk source to implement*/
	CLK_SRC_HOSC_DIV_32K,

	CLK_SRC_NUMBER,
};

/* Some fake clocks has same frequency, so we reuse the enum value */
#define CLK_SRC_BLE_32M_DIV_32K CLK_SRC_HOSC_DIV_32K
#define CLK_SRC_RCCAL_OUT_32K CLK_SRC_HOSC_DIV_32K

#endif /* __SUN20IW2_SRC_CCU_H__ */
