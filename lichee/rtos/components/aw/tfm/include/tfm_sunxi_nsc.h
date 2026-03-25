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

#ifndef __TFM_SUNXI_NSC_FUNCTION_H__
#define __TFM_SUNXI_NSC_FUNCTION_H__

typedef void ( *Callback_t ) ( void );
uint32_t tfm_sunxi_nsc_func(Callback_t pxCallback);

int tfm_sunxi_efuse_write(char *key_name, unsigned char *key_data, unsigned int key_bit_len);

int tfm_sunxi_aes_with_hardware(crypto_aes_req_ctx_t *aes_ctx);
int tfm_sunxi_aes_encrypt_with_hardware_ssk(uint8_t *dst_addr, uint8_t *src_addr, int len);
int tfm_sunxi_keybox_store(void *in_buf);
int tfm_sunxi_check_hash(const char *name, uint8_t *hash);

void tfm_sunxi_flashenc_set_nonce(uint8_t *nonce);
void tfm_sunxi_flashenc_config(uint8_t id, uint32_t saddr, uint32_t eaddr, uint32_t *key);
void tfm_sunxi_flashenc_set_region(uint8_t id, uint32_t saddr, uint32_t eaddr);
void tfm_sunxi_flashenc_set_key(uint8_t id, uint32_t *key);
void tfm_sunxi_flashenc_set_ssk_key(uint8_t id);
void tfm_sunxi_flashenc_enable(uint8_t id);
void tfm_sunxi_flashenc_disable(uint8_t id);

#endif /* __TFM_SUNXI_NSC_FUNCTION_H__ */
