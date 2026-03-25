/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
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

#ifndef _DRIVER_CHIP_HAL_XIP_H_
#define _DRIVER_CHIP_HAL_XIP_H_

#include "hal_flashctrl.h"
#include "./flashchip/flash_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_XIP
extern char __XIP_Base[];
extern char __XIP_End[];
#define FLASH_XIP_START_ADDR                    ((uintptr_t)(__XIP_Base))
#define FLASH_XIP_END_ADDR                      (roundup2(((uintptr_t)(__XIP_End)+1), 64))
#define FLASH_XIP_END_ADDR_FAKE                 ((uintptr_t)(__XIP_End))

#define FLASH_XIP_ALIGN_OFFSET                  0x1000
#define FLASH_XIP_USER_START_ADDR               (roundup2(((uintptr_t)(__XIP_End)+65), FLASH_XIP_ALIGN_OFFSET))
#define FLASH_XIP_USER_END_ADDR                 FC_GetXipUserEndAddr()
#define FLASH_XIP_USER_ADDR_STEP                (64) /* must big than read cache line */
#define FLASH_XIP_USER_START_ADDR_FAKE          ((uintptr_t)(__XIP_End))
#else
#define FLASH_XIP_START_ADDR                    ((uintptr_t)(0x10000000))
#define FLASH_XIP_END_ADDR                      (roundup2(((uintptr_t)(0x10000000)+1), 64))
#define FLASH_XIP_END_ADDR_FAKE                 ((uintptr_t)(0x10000000))

#define FLASH_XIP_ALIGN_OFFSET                  0x1000
#define FLASH_XIP_USER_START_ADDR               (roundup2(((uintptr_t)(0x10000000)+65), FLASH_XIP_ALIGN_OFFSET))
#define FLASH_XIP_USER_END_ADDR                 FC_GetXipUserEndAddr()
#define FLASH_XIP_USER_ADDR_STEP                (64) /* must big than read cache line */
#define FLASH_XIP_USER_START_ADDR_FAKE          ((uintptr_t)(0x10000000))
#endif

extern char __PSRAM_Base[];
extern char __PSRAM_End[];
extern char __PSRAM_Top[];

#ifdef CONFIG_PSRAM
#define PSRAM_MAPPING_ADDR    ((uint32_t)(CONFIG_PSRAM_START))
#define PSRAM_START_ADDR      ((uint32_t)(__PSRAM_Base))
#define PSRAM_END_ADDR        ((uint32_t)(__PSRAM_Top))
#else
//#define PSRAM_START_ADDR      ((uint32_t)(CONFIG_PSRAM_START))
//#define PSRAM_END_ADDR        (PSRAM_START_ADDR)
#endif

struct XipDrv;

int HAL_Xip_setCmd(struct XipDrv *xip, InstructionField *cmd, InstructionField *addr, InstructionField *dummy, InstructionField *data);
int HAL_Xip_setDelay(struct XipDrv *xip, Flashc_Delay *delay);
int HAL_Xip_setAddr(struct XipDrv *xip, uint32_t addr);
int HAL_Xip_setContinue(struct XipDrv *xip, uint32_t continueMode, void *arg);
int HAL_Xip_setDummyData(struct XipDrv *xip, uint32_t cont_dummyh, uint32_t cont_dummyl,
		uint32_t ex_cont_dummyh, uint32_t ex_cont_dummyl);
/**
 * @brief Initializes XIP module.
 * @note XIP is a module that cpu can run the code in flash but not ram.
 * @param flash: flash number, this flash must have been initialized, and must
 *               be connected to the flash controller pin.
 * @param xaddr: XIP code start address.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_Xip_Init(uint32_t flash, uint32_t xaddr);

/**
 * @brief Deinitializes XIP module.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_Xip_Deinit(uint32_t flash);

void HAL_Xip_SetDbgMask(uint8_t dbg_mask);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_CHIP_HAL_XIP_H_ */
