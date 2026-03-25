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

#ifndef _USB_DMA_H
#define _USB_DMA_H

#define DMA_OTG_TYPE 0

#define USB_DMA_CHANN_NUM 8

#define usb_get_dma_tx_config(dma_burst, ep_no) \
	(((dma_burst & 0x7ff) << 16) | (0x00 << 4) | (ep_no & 0x0f))

#define usb_get_dma_rx_config(dma_burst, ep_no) \
	(((dma_burst & 0x7ff) << 16) | (0x01 << 4) | (ep_no & 0x0f))

struct usb_dma_manage {
	uint32_t chan;
	uint32_t used;
	uint32_t config;
	uint32_t sdram_add;
};

void init_usb_dma(void);

int32_t usb_dma_setup(uint32_t chan, uint32_t config);
int32_t usb_dma_start(uint32_t chan, uint32_t sdram_add, uint32_t len);
void usb_dma_stop(uint32_t chan);

int32_t usb_dma_request(void);
uint32_t usb_dma_finish(uint32_t chan);
int32_t usb_dma_release(uint32_t chan);

void usb_dma_irq_enable(uint32_t chan);
void usb_dma_irq_disable(uint32_t chan);

void usb_dma_clr_irq_status(uint32_t chan);
uint32_t usb_dma_get_irq_status(uint32_t chan);
struct usb_dma_manage *usb_dma_get_ch_info(uint32_t chan);

#endif
