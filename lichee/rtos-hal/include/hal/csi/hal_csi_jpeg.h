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

#ifndef __HAL_CSI_JPEG_H_
#define __HAL_CSI_JPEG_H_

#include <aw_list.h>
#include <hal_def.h>

/* Output Stream memory Partition Mode, only used in Online Mode */
#define JPEG_MPART_ENABLE			(0)

#if JPEG_MPART_ENABLE
#define JPEG_OUTPUT_BUFF_SIZE 			(8*1024)
#else
#define JPEG_OUTPUT_BUFF_SIZE  		(50*1024)
#endif

#define JPEG_HEADER_LEN		623

enum pix_output_fmt_mode_t {
	PIX_FMT_OUT_NV12 = 0x1,
	PIX_FMT_OUT_JPEG = 0x2,
	PIX_FMT_OUT_MAX = 0x3,
};

enum pix_input_fmt_mode_t {
	PIX_FMT_IN_RAW  = 0x0,
	PIX_FMT_IN_NV12 = 0x1,
	PIX_FMT_IN_JPEG = 0x2,
};

enum line_mode_t {
	OFFLINE_MODE = 0,
	ONLINE_MODE,
};

struct csi_jpeg_buf {
	unsigned int size;
	void *addr;
};

typedef struct {
	uint8_t buff_index; 	/* Indicate which buffer the currently encoded part jpeg is stored in */
	uint32_t buff_offset; 	/* Indicate the offset of the current part of jpeg in the buffer */
	uint8_t tail; 			/* Indicates whether it is the last part of a jpeg image */
	uint32_t size; 			/* Indicate the size of the current part of jpeg encoding */
} jpeg_mpartbuffinfo;

struct csi_jpeg_mem {
	unsigned int index;
	struct csi_jpeg_buf buf;
	jpeg_mpartbuffinfo mpart_info;
	struct list_head list;
};

typedef void (*CapStatusCb)(struct csi_jpeg_mem *jpeg_mem);

struct csi_jpeg_fmt {
	unsigned int width;
	unsigned int height;
	enum line_mode_t line_mode;
	enum pix_input_fmt_mode_t input_mode;
	enum pix_output_fmt_mode_t output_mode;
	bool scale; //scale 1/2
	CapStatusCb cb;
	unsigned char fps; //reserve
};


void hal_csi_sensor_get_sizes(unsigned int *width, unsigned int *height);
void hal_csi_jpeg_set_fmt(struct csi_jpeg_fmt *intput_fmt);
int hal_csi_jpeg_reqbuf(unsigned int count);
int hal_csi_jpeg_freebuf(void);
struct csi_jpeg_mem *hal_jpegenc_get_yuvbuf(void);
struct csi_jpeg_mem *hal_csi_dqbuf(unsigned int timeout_msec);
struct csi_jpeg_mem *hal_jpeg_dqbuf(unsigned int timeout_msec);
void hal_csi_qbuf(void);
void hal_jpeg_qbuf(void);
void hal_csi_jpeg_s_stream(unsigned int on);
void hal_csi_sensor_s_stream(unsigned int on);
HAL_Status hal_csi_jpeg_probe(void);
HAL_Status hal_csi_jpeg_remove(void);

#endif
