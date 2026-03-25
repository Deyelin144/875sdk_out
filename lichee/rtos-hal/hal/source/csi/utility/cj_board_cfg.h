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

#ifndef __CJ_BOARD_CFG_H__
#define __CJ_BOARD_CFG_H__

#include <stdio.h>
#include "hal_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CJ_DEV_DBG_EN   1
#if (CJ_DEV_DBG_EN == 1)
#define cj_dbg(x, arg...) hal_log_info("[%d:%s]"x, __LINE__, __func__, ##arg)
#define cj_dbg(x, arg...)
#endif

#define cj_err(x, arg...)  hal_log_err("[%d:%s]"x, __LINE__, __func__, ##arg)
#define cj_warn(x, arg...) hal_log_warn("[%d:%s]"x, __LINE__, __func__, ##arg)
#define cj_print(x, arg...) hal_log_info("[%d:%s]"x, __LINE__, __func__, ##arg)


#ifdef CONFIG_DRIVER_SYSCONFIG
#define SYS_GPIO_NUM 14

#define SET_CSI0 "csi0"
#define KEY_CSI_DEV_TWI_ID "vip_dev0_twi_id"
#define KEY_CSI_DEV_PWDN_GPIO "vip_dev0_pwdn"
#define KEY_CSI_DEV_RESET_GPIO "vip_dev0_reset"
#else
/* sensor cfg */
#define SENSOR_I2C_ID           TWI_MASTER_0  // refer pinmux
#define SENSOR_RESET_PIN        GPIO_PA15
#define SENSOR_POWERDOWN_PIN    GPIO_PA14  //GPIO_PA14
#endif

#if defined CONFIG_CSI_CAMERA_GC0308
#include "sensor/drv_gc0308.h"
#define SENSOR_FUNC_INIT		hal_gc0308_init
#define SENSOR_FUNC_DEINIT		hal_gc0308_deinit
#define SENSOR_FUNC_IOCTL		hal_gc0308_ioctl
#define SENSOR_FUNC_MBUD_CFG		hal_gc0308_g_mbus_config
#define SENSOR_FUNC_WIN_SIZE		hal_gc0308_win_sizes
#elif defined CONFIG_CSI_CAMERA_GC0328c
#include "sensor/drv_gc0328c.h"
#define SENSOR_FUNC_INIT		hal_gc0328c_init
#define SENSOR_FUNC_DEINIT		hal_gc0328c_deinit
#define SENSOR_FUNC_IOCTL		hal_gc0328c_ioctl
#define SENSOR_FUNC_MBUD_CFG		hal_gc0328c_g_mbus_config
#define SENSOR_FUNC_WIN_SIZE		hal_gc0328c_win_sizes
#elif defined CONFIG_CSI_CAMERA_GC2145
#include "sensor/drv_gc2145.h"
#define SENSOR_FUNC_INIT		hal_gc2145_init
#define SENSOR_FUNC_DEINIT		hal_gc2145_deinit
#define SENSOR_FUNC_IOCTL		hal_gc2145_ioctl
#define SENSOR_FUNC_MBUD_CFG		hal_gc2145_g_mbus_config
#define SENSOR_FUNC_WIN_SIZE		hal_gc2145_win_sizes
#endif

#define ALIGN_8B(x)     (((x) + (7)) & ~(7))
#define ALIGN_16B(x)    (((x) + (15)) & ~(15))
#define ALIGN_32B(x)    (((x) + (31)) & ~(31))
#define ALIGN_1K(x)     (((x) + (1023)) & ~(1023))
#define ALIGN_4K(x)     (((x) + (4095)) & ~(4095))

#define CSI_JPEG_CLK		96*1000*1000

#ifdef __cplusplus
}
#endif
#endif

