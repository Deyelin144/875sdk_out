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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _JPEG_ENC_H_
#define _JPEG_ENC_H_

#define DefaultDC       (1024)

#define NUM_QUANT_TBLS  2
#define DCTSIZE         8       /* The basic DCT block is 8x8 samples */
#define DCTSIZE2        64      /* DCTSIZE squared; # of elements in a block */

#define JpgYUV420	0
#define JpgYUV444	1
#define JpgYUV422	2

struct jpeg_ctl_ops {
	void (*writeHeader)(void *handle);
	int  (*setParameter)(void *handle, int indexType, void *param);
	void  (*setQuantTbl)(void *handle, int quality);
};

typedef struct JpegCtx {
	char     			*BaseAddr;
	unsigned short    	image_width;
	unsigned short    	image_height;
	unsigned short    	quant_tbl[2][DCTSIZE2 * 2];
	unsigned short    	quant_tbl_aw[DCTSIZE2 * 4]; /* modify to 256 word */
	int					dc_value[3];

	int 			  	JpgColorFormat;//0:420, 1:444, 2:422
	int               	quality;

	struct jpeg_ctl_ops *ctl_ops;
} JpegCtx;


typedef enum {
	VENC_IndexParamJpegQuality = 0,
	VENC_IndexParamJpegEncMode,
	VENC_IndexParamSetVsize,
	VENC_IndexParamSetHsize,

} VENC_IndexType;

JpegCtx *JpegEncCreate();

//JpegCtx *JpegCtx();

void JpegEncDestory(void *handle);


#endif /* _JPEG_ENC_H_ */

#ifdef __cplusplus
}
#endif /* __cplusplus */
