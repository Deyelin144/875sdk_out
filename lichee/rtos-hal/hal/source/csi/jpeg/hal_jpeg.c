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

#include <stdlib.h>
#include "hal_mem.h"
#include "hal_gpio.h"
#include "hal_clk.h"
#include "hal_timer.h"
#include "hal_log.h"
#include <hal_interrupt.h>
#include "hal_cache.h"
#include "hal_reset.h"
#include "csi_reg.h"
#include "jpeg_reg.h"
#include "hal_def.h"
#include "hal_jpeg.h"
#include "string.h"
#include "csi_camera/csi.h"

#define JPEG_REG_DBG_ON    		 0
#if (JPEG_REG_DBG_ON == 1)
#define JPEG_REG_DBG(REG)		 hal_log_debug("(%p): 0x%8x\n", (uint32_t*)&REG, REG)
#else
#define JPEG_REG_DBG(fmt, arg...)
#endif

#define JPEG_REG_MASK				 (0x02)
#define JPEG_REG_ALL(mask)		 do {			\
	if (mask & JPEG_REG_MASK) {						\
		JPEG_REG_DBG(JPEG->VE_MODE_REG);		\
		JPEG_REG_DBG(JPEG->INPUT_PIC_SIZE);		\
		JPEG_REG_DBG(JPEG->VE_INT_EN_REG);		\
		JPEG_REG_DBG(JPEG->VE_MODE_REG);		\
		JPEG_REG_DBG(JPEG->CSI_OUTPUT_ADDR_Y);	\
		JPEG_REG_DBG(JPEG->CSI_OUTPUT_ADDR_UV);	\
		JPEG_REG_DBG(JPEG->JPE_INPUT_ADDR_Y);	\
		JPEG_REG_DBG(JPEG->JPE_INPUT_ADDR_C);	\
		JPEG_REG_DBG(JPEG->JPEG_PARA0_REG);		\
		JPEG_REG_DBG(JPEG->JPEG_BITRATE_CTRL);	\
		JPEG_REG_DBG(JPEG->OUTSTM_OFFSET);		\
		JPEG_REG_DBG(JPEG->OUTSTM_START_ADDR);	\
		JPEG_REG_DBG(JPEG->OUTSTM_END_ADDR);	\
		JPEG_REG_DBG(JPEG->VE_START_REG);		\
	}												\
} while (0)


jpeg_private *gjpeg_private;

jpeg_private *jpeg_getpriv()
{
	if(gjpeg_private == NULL)
	{
		jpeg_err("jpeg init fail");
		return NULL;
	}else
	{
		return gjpeg_private;
	}
}

void jpeg_setpriv(jpeg_private *jpeg_priv)
{
	gjpeg_private = jpeg_priv;
}

#if CJ_RAM_REMAP
void HAL_SYSCTL_SetCSIJPEGSramShare(SYSCTL_CSI_JPEG_ShareSramType mode)
{
	/* set the sram size to 64k or 32k */
	jpeg_dbg("pay attention to set sram size(%s):%d\n", __func__, __LINE__);
	HAL_SYSCTL_RamUseAsCSI(1);
}
#endif

static void jpeg_set_qtab(uint32_t quality)
{
	jpeg_private *priv = jpeg_getpriv();

	priv->jpegCtx->ctl_ops->setQuantTbl(priv->jpegCtx, quality);

	priv->jpegCtx->dc_value[0] = DefaultDC / priv->jpegCtx->quant_tbl[0][0];
	priv->jpegCtx->dc_value[1] = DefaultDC / priv->jpegCtx->quant_tbl[1][0];
	priv->jpegCtx->dc_value[2] = DefaultDC / priv->jpegCtx->quant_tbl[1][0];
}

static void jpeg_write_qtab(uint32_t base_addr)
{
	uint8_t i = 0;

	jpeg_private *priv = jpeg_getpriv();

	JPEG->QM_INDEX = base_addr;

	uint32_t *tbl = (uint32_t *)&priv->jpegCtx->quant_tbl_aw;
	for (i = 0; i < 128; i++) {
		JPEG->QM_DATA = *tbl++;
	}
}

static void __jpeg_set_fmt_hw()
{
	jpeg_private *jpeg_priv = jpeg_getpriv();

	jpeg_priv->jpeg_cfg.jpeg_en = 1;
	jpeg_priv->jpeg_cfg.quality = 65;  /* 0 ~ 99*/
	jpeg_priv->jpeg_cfg.jpeg_bitrate_en = 0;

	if (jpeg_priv->jpeg_cfg.input_mode == PIX_FMT_IN_JPEG)
		jpeg_priv->jpeg_cfg.sensor_out_type = 1;  /* 0:OUT_YUV420 NV12  1:OUT_JPEG */
	else
		jpeg_priv->jpeg_cfg.sensor_out_type = 0;  /* 0:OUT_YUV420 NV12  1:OUT_JPEG */

#if JPEG_MPART_ENABLE
	jpeg_priv->jpeg_cfg.mem_part_en = 1;
#else
	jpeg_priv->jpeg_cfg.mem_part_en = 0;
#endif
	jpeg_priv->jpeg_cfg.mem_part_num = JPEG_MEM_BLOCK2;

}

HAL_Status hal_jpeg_set_fmt(struct jpeg_fmt *jpeg_output_fmt)
{
	jpeg_private *jpeg_priv = jpeg_getpriv();

	jpeg_priv->jpeg_cfg.pic_size_width = jpeg_output_fmt->width;
	jpeg_priv->jpeg_cfg.pic_size_height = jpeg_output_fmt->height;
	jpeg_priv->jpeg_cfg.jpeg_mode = jpeg_output_fmt->line_mode;
	jpeg_priv->jpeg_cfg.input_mode = jpeg_output_fmt->input_mode;
	jpeg_priv->jpeg_cfg.output_mode = jpeg_output_fmt->output_mode;

}

HAL_Status hal_csi_set_addr(void *addr)
{
	jpeg_private *priv = jpeg_getpriv();
	char *addr_uv;
	uint32_t reg_val;

	addr_uv = addr + (priv->jpeg_cfg.pic_size_width * priv->jpeg_cfg.pic_size_height);
	jpeg_dbg("[%s], width = %d, height = %d\n", __func__, priv->jpeg_cfg.pic_size_width,
			priv->jpeg_cfg.pic_size_height);
	cj_dbg("[%s], ADDR_Y = 0x%08x, ADDR_UV = 0x%08x\n", __func__, addr, addr_uv);

	JPEG->CSI_OUTPUT_ADDR_Y = (unsigned long)addr;
	JPEG->CSI_OUTPUT_ADDR_UV = (unsigned long)addr_uv;

	/* if not set stride, csi data can not write to addr */
	reg_val = ((priv->jpeg_cfg.pic_size_width / 16 & 0x7ff) << 16) |\
				((priv->jpeg_cfg.pic_size_width / 16 & 0x7ff) << 0);
	JPEG->CSI_OUTPUT_STRIDE = reg_val;

	return HAL_OK;
}

HAL_Status hal_jpeg_set_addr(unsigned int index)
{
	jpeg_private *jpeg_priv = jpeg_getpriv();
	jpeg_cfg_param *cfg = &(jpeg_priv->jpeg_cfg);

	JPEG->OUTSTM_OFFSET = jpeg_priv->memPartEn ? jpeg_priv->jpgOutStmOffset : 0;
	JPEG->OUTSTM_START_ADDR = jpeg_priv->jpgOutStmAddr[index];
	JPEG->OUTSTM_END_ADDR = jpeg_priv->jpgOutStmAddr[index] + jpeg_priv->jpgMemSize - 1;

	cj_dbg("[%s], OUTSTREAM_BUFF_ADDR = 0x%08x, size = %d\n", __func__,
			JPEG->OUTSTM_START_ADDR, jpeg_priv->jpgMemSize);

	return HAL_OK;
}

HAL_Status hal_csi_jpeg_set_addr(void *csi_addr, struct csi_jpeg_mem *jpeg_mem,
		unsigned char jpeg_buff_num)
{
	jpeg_private *jpeg_priv = jpeg_getpriv();
	HAL_Status status = 0;
	int index = 0;

	if (jpeg_priv->jpeg_cfg.jpeg_mode == JPEG_MOD_OFFLINE) {

		jpeg_priv->jpeg_cfg.csi_output_addr_y
				= (unsigned long)csi_addr;                     /* CSI_OUTPUT_Y_ADDR */
		jpeg_priv->jpeg_cfg.csi_output_addr_uv
				= (unsigned long)csi_addr + (jpeg_priv->jpeg_cfg.pic_size_width \
					* jpeg_priv->jpeg_cfg.pic_size_height);    /* CSI_OUTPUT_UV_ADDR */
		cj_dbg("[%s], jpeg_priv->jpeg_cfg.csi_output_addr_y = 0x%08x\n", __func__,
				jpeg_priv->jpeg_cfg.csi_output_addr_y);
		cj_dbg("[%s], jpeg_priv->jpeg_cfg.csi_output_addr_uv = 0x%08x\n", __func__,
				jpeg_priv->jpeg_cfg.csi_output_addr_uv);

		if ((jpeg_priv->jpeg_cfg.output_mode == PIX_FMT_OUT_JPEG)
				|| (jpeg_priv->jpeg_cfg.output_mode == PIX_FMT_OUT_MAX)) {
			/* ouput yuv and jpeg, set csi and jpeg */
			jpeg_priv->jpeg_cfg.jpeg_input_addr_y
					= jpeg_priv->jpeg_cfg.csi_output_addr_y;   /* JPEG_INPUT_Y_ADDR */
			jpeg_priv->jpeg_cfg.jpeg_input_addr_uv
					= jpeg_priv->jpeg_cfg.csi_output_addr_uv;  /* JPEG_INPUT_UV_ADDR */
		}
	}

	jpeg_priv->jpeg_cfg.outstream_buff_num = jpeg_buff_num;    /* JPEG_OUTSTM_NUM */
	jpeg_priv->jpeg_cfg.outstream_buff_offset = 0;             /* JPEG_OUTSTM_OFFSET */
	jpeg_priv->jpeg_cfg.outstream_buff_size
			= (uint32_t)jpeg_mem[index].buf.size;              /* JPEG_OUTSTM_SIZE */
	for(index = 0; index < jpeg_buff_num && jpeg_mem[index].buf.addr; index++) {
		jpeg_priv->jpeg_cfg.outstream_buff_addr[index]
				= (unsigned long)jpeg_mem[index].buf.addr;     /* JPEG_OUTSTM_ADDR */
		cj_dbg("[%s], jpeg_priv->jpeg_cfg.outstream_buff_addr[%d] = 0x%08x\n",
				__func__, index, jpeg_priv->jpeg_cfg.outstream_buff_addr[index]);
	}

	status = hal_jpeg_config_addr(&jpeg_priv->jpeg_cfg);
	if (status != 0)
		cj_err("csi jpeg set address failed\n");

	cj_dbg("set csi jpeg address config OK******\n");
	return status;
}

HAL_Status hal_jpeg_config_addr(jpeg_cfg_param *cfg)
{
	uint32_t reg_val;
	if(cfg == NULL) {
		jpeg_err("[%s], ERROR: cfg NULL\n", __func__);
		return HAL_ERROR;
	}

	jpeg_private *priv = jpeg_getpriv();

	if (cfg->jpeg_mode == JPEG_MOD_OFFLINE) {

		if (cfg->input_mode != PIX_FMT_IN_NV12) {
			JPEG->CSI_OUTPUT_ADDR_Y = cfg->csi_output_addr_y;
			JPEG->CSI_OUTPUT_ADDR_UV = cfg->csi_output_addr_uv;

			reg_val = ((cfg->pic_size_width	/16		& 0x7ff) << 16)		|\
			      ((cfg->pic_size_width/16  	& 0x7ff) << 0);
			JPEG->CSI_OUTPUT_STRIDE = reg_val;
		}

		if ((cfg->output_mode == PIX_FMT_OUT_JPEG)
				|| (cfg->output_mode == PIX_FMT_OUT_MAX)) {
			/* output yuv and jpeg, both set */
			reg_val = ((cfg->pic_size_width /16 	& 0x7ff) << 16);
			JPEG->JPE_STRIDE_CTRL = reg_val;

			reg_val = ((cfg->pic_size_width /16 	& 0xfff) << 0);
			JPEG->JPE_STRIDE_CTRL_1 = reg_val;

			JPEG->JPE_INPUT_ADDR_Y = cfg->jpeg_input_addr_y;
			JPEG->JPE_INPUT_ADDR_C = cfg->jpeg_input_addr_uv;
		}
	}

	JPEG->OUTSTM_OFFSET = cfg->outstream_buff_offset;
	JPEG->OUTSTM_START_ADDR = cfg->outstream_buff_addr[0];
	JPEG->OUTSTM_END_ADDR = cfg->outstream_buff_addr[0] + JPEG_OUTPUT_BUFF_SIZE - 1;
	return HAL_OK;
}

HAL_Status hal_csi_jpeg_scale(void)
{
	uint32_t reg_val;

	reg_val = ((1	& 0x1) << 11)	|\
			  ((1	& 0x1) << 10);
	HAL_MODIFY_REG(JPEG->VE_MODE_REG, (1<<11)|(1<<10), reg_val);

	return HAL_OK;
}

HAL_Status hal_jpeg_config(jpeg_cfg_param *cfg)
{
	uint32_t reg_val;
	if(cfg == NULL) {
		jpeg_err("[%s], ERROR: cfg NULL\n", __func__);
		return HAL_ERROR;
	}
	jpeg_private *priv = jpeg_getpriv();

#ifdef CONFIG_ARCH_HAVE_DCACHE
	if (priv->jpeg_cfg.jpeg_mode == JPEG_MOD_OFFLINE) {
		hal_dcache_invalidate((uint32_t)cfg->csi_output_addr_y,
				cfg->pic_size_width * cfg->pic_size_height);
		hal_dcache_invalidate((uint32_t)cfg->csi_output_addr_uv,
				cfg->pic_size_width * cfg->pic_size_height / 2);
	} else {
		hal_dcache_invalidate((uint32_t)cfg->csi_output_addr_y, 4);
		hal_dcache_invalidate((uint32_t)cfg->csi_output_addr_uv, 4);
	}
#endif

	priv->jpgMemSize = cfg->outstream_buff_size;
	priv->jpgOutStmOffset = cfg->outstream_buff_offset;
	priv->jpgOutBufNum = cfg->outstream_buff_num ? cfg->outstream_buff_num : 1;
	for (int i = 0; i < priv->jpgOutBufNum; i++) {
#ifdef CONFIG_ARCH_HAVE_DCACHE
	hal_dcache_invalidate((uint32_t)cfg->outstream_buff_addr[i], cfg->outstream_buff_size);
#endif
		priv->jpgOutStmAddr[i] = cfg->outstream_buff_addr[i];
	}

	priv->jpgVeEn = cfg->jpeg_en;
	priv->encMode = cfg->jpeg_en ? cfg->jpeg_mode : JPEG_MOD_OFFLINE;

	priv->jpegCtx->JpgColorFormat = JpgYUV420;
	priv->jpegCtx->quality = cfg->quality;
	priv->jpegCtx->image_height = cfg->pic_size_height;
	priv->jpegCtx->image_width = cfg->pic_size_width;

	priv->memPartEn = cfg->mem_part_en;
	if (priv->memPartEn) {
		priv->memPartNum = 1 << (cfg->mem_part_num + 1);
		priv->memPartSize = priv->jpgMemSize / priv->memPartNum;
		priv->memPartOffSet = 0;
	}

	if (cfg->jpeg_en) {

#if CJ_RAM_REMAP
		SYSCTL_CSI_JPEG_ShareSramType type = SYSCTL_CSI_JPEG_SHARE_32K;
		if (cfg->pic_size_width > JPEG_SRAM_SWITCH_THR_W ||
					cfg->pic_size_height > JPEG_SRAM_SWITCH_THR_H) {
  #if SUPPORT_JPEG_SHARE_64K
			type = SYSCTL_CSI_JPEG_SHARE_64K;
  #else
			jpeg_err("JPEG share ram not enough\n");
			return HAL_ERROR;
  #endif
		}

		HAL_SYSCTL_SetCSIJPEGSramShare(type);
#else
		jpeg_err("JPEG share ram not exist\n");
		return HAL_ERROR;
#endif
	}

	reg_val = ((cfg->mem_part_en		& 0x1) << 16)		|\
			  ((cfg->mem_part_num		& 0x3) << 14)		|\
			  ((0						& 0x1) << 13)		|\
			  ((0						& 0x1) << 12)		|\
			  ((0						& 0x1) << 11)		|\
			  ((0						& 0x1) << 10)		|\
			  ((cfg->jpeg_mode			& 0x1) << 9)		|\
			  ((cfg->sensor_out_type	& 0x1) << 8)		|\
			  ((1  						& 0x1) << 7)		|\
			  ((1				  		& 0x1) << 6);

	JPEG->VE_MODE_REG = reg_val;

	reg_val = ((cfg->pic_size_width/8	& 0x7ff) << 16)		|\
			  ((cfg->pic_size_height/8	& 0x7ff) << 0);
	JPEG->INPUT_PIC_SIZE = reg_val;

	reg_val = ((1	& 0x1) << 2)		|\
			  ((1	& 0x1) << 1)		|\
			  ((1	& 0x1) << 0);
	JPEG->VE_INT_EN_REG = reg_val;

	jpeg_set_qtab(cfg->quality);

	JPEG->JPEG_PARA0_REG = 3<<30 | priv->jpegCtx->dc_value[1]<<16 | priv->jpegCtx->dc_value[0];
	JPEG->JPEG_BITRATE_CTRL = cfg->jpeg_bitrate_en ? 0xC0000840 : 0x00000840;

	jpeg_write_qtab(0x0);

	JPEG_REG_ALL(JPEG_REG_MASK);

	priv->state = CSI_STATE_READY;

	for(int i = 0; i < cfg->outstream_buff_num && cfg->outstream_buff_addr[i]; i++)
		hal_jpeg_write_header((uint8_t *)((unsigned long)cfg->outstream_buff_addr[i]
					- JPEG_HEADER_LEN));

	return HAL_OK;
}

HAL_Status hal_jpeg_s_stream(unsigned int on)
{
	jpeg_private *jpeg_priv = jpeg_getpriv();

	if (on) {
		hal_ve_rst_ctl_release();
		jpeg_dbg("hal_ve_rst_ctl_release\n");
		if (jpeg_priv->jpeg_cfg.output_mode == PIX_FMT_OUT_NV12) {
			hal_jpeg_clk_en();
		} else {
			__jpeg_set_fmt_hw();
			hal_jpeg_config(&jpeg_priv->jpeg_cfg);
		}
	} else {
		hal_ve_rst_ctl_reset();
		jpeg_dbg("hal_ve_rst_ctl_reset\n");
	}

	return 0;
}

void hal_jpeg_reset(void)
{
	jpeg_dbg("[CSI_JPEG]%s:%d, pay attention\n", __func__, __LINE__);

	hal_reset_type_t reset_type = HAL_SUNXI_RESET;
	hal_reset_id_t	reset_id = RST_CSI_JPE;

	struct reset_control *reset;

	reset = hal_reset_control_get(reset_type, reset_id);
	if (hal_reset_control_reset(reset))
	{
		jpeg_err("csi jpeg reset  failed!\n");
		return ;
	}
	hal_reset_control_put(reset);
}

void hal_jpeg_write_header(uint8_t *baseAddr)
{
	jpeg_private *priv = jpeg_getpriv();

	if (baseAddr == NULL) {
		jpeg_err("[%s], invalid handle\n", __func__);
		return;
	}
	priv->jpegCtx->BaseAddr = (char*)baseAddr;
	priv->jpegCtx->ctl_ops->writeHeader(priv->jpegCtx);
}

int hal_jpeg_init(void)
{
	jpeg_private *jpeg_priv;

	jpeg_priv = malloc(sizeof(jpeg_private));
	if (!jpeg_priv) {
		jpeg_err("%s malloc faild\n", __func__);
		return -1;
	}
	memset(jpeg_priv, 0, sizeof(jpeg_private));
	jpeg_setpriv(jpeg_priv);

	jpeg_priv->jpegCtx = JpegEncCreate();
	if (!jpeg_priv->jpegCtx) {
		jpeg_err("jpeg encode create fail\n");
		return HAL_ERROR;
	}

	return 0;
}

void hal_jpeg_deinit(void)
{
	jpeg_private *jpeg_priv = jpeg_getpriv();

	if(jpeg_priv->jpegCtx)
	{
		JpegEncDestory(jpeg_priv->jpegCtx);
		jpeg_priv->jpegCtx = NULL;
	}

	free(jpeg_priv);
	jpeg_setpriv(NULL);
}

