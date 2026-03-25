/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY?ˉS TECHNOLOGY (SONY, DTS, DOLBY, AVS OR
 * MPEGLA, ETC.)
 * IN ALLWINNERS?ˉSDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT
 * TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY?ˉS TECHNOLOGY.
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

#include "hal_time.h"
#include "hal_timer.h"

#include "csi_reg/csi_reg.h"
#include "cj_board_cfg.h"
#include "hal_log.h"

#define IS_FLAG(x, y) (((x)&(y)) == y)

#define CSI_DEV_DBG_EN   0
#if (CSI_DEV_DBG_EN == 1)
#define csi_dbg(x, arg...) hal_log_info("[%d:%s]"x, __LINE__, __func__, ##arg)
#else
#define csi_dbg(x, arg...)
#endif

#define csi_err(x, arg...)  hal_log_err("[%d:%s]"x, __LINE__, __func__, ##arg)
#define csi_warn(x, arg...) hal_log_warn("[%d:%s]"x, __LINE__, __func__, ##arg)
#define csi_print(x, arg...) hal_log_info("[%d:%s]"x, __LINE__, __func__, ##arg)

struct csi_fmt {
	unsigned int width;
	unsigned int height;
};

HAL_Status hal_csi_set_fmt(struct csi_fmt *csi_input_fmt);
//HAL_Status hal_csi_sensor_s_stream(unsigned int on);
HAL_Status hal_csi_s_stream(unsigned int on);
HAL_Status hal_sensor_s_stream(unsigned int on);

int hal_csi_probe(void);
void hal_csi_remove(void);
