
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

#include <xtensa/hal.h>

#if defined (__cplusplus)
extern "C"
#endif
const struct xthal_MPU_entry
__xt_mpu_init_table[] __attribute__((section(".ResetHandler.text"))) = {
  XTHAL_MPU_ENTRY(0x00000000, 1, XTHAL_AR_NONE, XTHAL_MEM_DEVICE), // unused
  XTHAL_MPU_ENTRY(0x0c000000, 1, XTHAL_AR_RWXrwx, XTHAL_MEM_WRITEBACK), // extra_mem
  XTHAL_MPU_ENTRY(0x0c800000, 1, XTHAL_AR_NONE, XTHAL_MEM_DEVICE), // unused
};

#if defined (__cplusplus)
extern "C"
#endif
const unsigned int
__xt_mpu_init_table_size __attribute__((section(".ResetHandler.text"))) = 3;

#if defined (__cplusplus)
extern "C"
#endif
const unsigned int
__xt_mpu_init_cacheadrdis __attribute__((section(".ResetHandler.text"))) = 254;


#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#include <assert.h>
static_assert(sizeof(__xt_mpu_init_table)/sizeof(__xt_mpu_init_table[0]) == 3, "Incorrect MPU table size");
static_assert(sizeof(__xt_mpu_init_table)/sizeof(__xt_mpu_init_table[0]) <= XCHAL_MPU_ENTRIES, "MPU table too large");
#endif

