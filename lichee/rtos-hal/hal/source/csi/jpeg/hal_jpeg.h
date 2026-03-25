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

#ifndef HAL_JPEG_H
#define HAL_JPEG_H

#include "jpeg_reg.h"
#include "csi/hal_csi_jpeg.h"
#include "jpeglib.h"
#include "jpegenc.h"

#define JPEG_DEV_DBG_EN   0
#if (JPEG_DEV_DBG_EN == 1)
#define jpeg_dbg(x, arg...) hal_log_info("[jpeg_debug]"x, ##arg)
#else
#define jpeg_dbg(x, arg...)
#endif

#define jpeg_err(x, arg...)  hal_log_err("[jpeg_err]"x, ##arg)
#define jpeg_warn(x, arg...) hal_log_warn("[jpeg_warn]"x, ##arg)
#define jpeg_print(x, arg...) hal_log_info("[jpeg]"x, ##arg)

#define SYSCTL_CSI_JPEG_SHARE_SRAM_SHIFT (8)
#define SYSCTL_CSI_JPEG_SHARE_SRAM_MASK  (0x3 << SYSCTL_CSI_JPEG_SHARE_SRAM_SHIFT)
typedef struct {
	JPEG_Mode			encMode;
	uint8_t				jpgOutBufId;
	uint8_t 			jpgOutBufNum;
	uint32_t  			jpgOutStmAddr[JPEG_BUFF_CNT_MAX];
	uint32_t 			jpgOutStmSize;
	uint32_t			jpgMemSize;
	uint32_t			jpgOutStmOffset;

	uint8_t				memPartEn;
	uint8_t 			memPartNum;
	uint32_t			memPartSize;
	uint32_t			memPartCnt;
	uint32_t			memPartOffSet;
	uint32_t			memCurSize;

	uint8_t 			jpgVeEn;
	enum csi_state			state;
	//CSI_JPEG_IRQCallback cb;

	JpegCtx 			*jpegCtx;
	jpeg_cfg_param			jpeg_cfg;
} jpeg_private;

typedef enum {
	SYSCTL_CSI_JPEG_SHARE_32K ,
	SYSCTL_CSI_JPEG_SHARE_64K ,
} SYSCTL_CSI_JPEG_ShareSramType;

struct jpeg_fmt {
	unsigned int width;
	unsigned int height;
	unsigned int line_mode;
	unsigned int input_mode;
	unsigned int output_mode;
};

jpeg_private *jpeg_getpriv();

void jpeg_setpriv(jpeg_private *jpeg_priv);

HAL_Status hal_jpeg_config(jpeg_cfg_param *cfg);

void hal_jpeg_reset(void);

void hal_jpeg_write_header(uint8_t *baseAddr);

int hal_jpeg_init(void);

void hal_jpeg_deinit(void);

HAL_Status hal_jpeg_s_stream(unsigned int on);

HAL_Status hal_jpeg_set_fmt(struct jpeg_fmt *jpeg_output_fmt);

HAL_Status hal_csi_set_addr(void *addr);

HAL_Status hal_jpeg_set_addr(unsigned int index);

HAL_Status hal_csi_jpeg_set_addr(void *csi_addr, struct csi_jpeg_mem *jpeg_mem,
		unsigned char jpeg_buff_num);

HAL_Status hal_jpeg_config_addr(jpeg_cfg_param *cfg);

HAL_Status hal_csi_jpeg_scale(void);

#endif  /*HAL_JPEG_H*/
