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

#ifndef	__IMAGE_HEADER__H__
#define	__IMAGE_HEADER__H__

#define IH_MAGIC	(0x48495741)
#define TH_MAGIC	(0x4854)

/* Image Header(128B) */
typedef struct image_header {
	uint32_t ih_magic;             /* Image Header Magic Number */
	uint16_t ih_hversion;          /* Image Header Version:     */
	uint16_t ih_pversion;          /* Image Payload Version:    */
	uint16_t ih_hchksum;           /* Image Header Checksum     */
	uint16_t ih_dchksum;           /* Image Data Checksum       */
	uint32_t ih_hsize;             /* Image Header Size         */
	uint32_t ih_psize;             /* Image Payload Size        */
	uint32_t ih_tsize;             /* Image TLV Size            */
	uint32_t ih_load;              /* Image Load Address        */
	uint32_t ih_ep;                /* Image Entry Point         */
	uint32_t ih_imgattr;           /* Image Attribute           */
	uint32_t ih_nxtsecaddr;        /* Next Section Address      */
	uint8_t  ih_name[16];          /* Image Name                */
	uint32_t ih_priv[18];          /* Image Private Data        */
} image_header_t;

/* TLV Header(32B) */
typedef struct tlv_header {
	uint16_t th_magic;
	uint16_t th_size;
	uint16_t th_pkey_type;
	uint16_t th_pkey_size;
	uint16_t th_sign_type;
	uint16_t th_sign_size;
	uint32_t th_priv[5];
} tlv_header_t;

int sunxi_image_header_check(image_header_t *ih, void *pbuffer);
int sunxi_tlv_header_check(tlv_header_t *th);
int sunxi_image_header_magic_check(image_header_t *ih);
int sunxi_image_header_dump(image_header_t *ih);

#endif	//__IMAGE_HEADER__H__
