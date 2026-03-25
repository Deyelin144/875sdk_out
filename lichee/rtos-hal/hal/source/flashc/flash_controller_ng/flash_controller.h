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

#ifndef __FLASH_CONTROLLER_H__
#define __FLASH_CONTROLLER_H__

#include <stdint.h>
#include <hal_mutex.h>

#include <sunxi_hal_flashctrl.h>

#include <hal_clk.h>

#ifdef CONFIG_DISABLE_INTERRUPT_WHEN_USING_INDIRECT_MODE
#include <hal_atomic.h>
#endif

#if defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP)
#ifdef CONFIG_ARCH_SUN20IW2

#if defined(CONFIG_ARCH_ARM)
#define AMP_CPU_ID_STR "1"
#elif defined(CONFIG_ARCH_RISCV)
#define AMP_CPU_ID_STR "2"
#elif defined(CONFIG_ARCH_DSP)
#define AMP_CPU_ID_STR "3"
#else
#define AMP_CPU_ID_STR "0"
#endif

#else
#define AMP_CPU_ID_STR "0"
#endif
#endif

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#define FLASH_CONTROLLER_PM_SUPPORT
#endif
#endif

//#define DEBUG_FC_OPERATION

/* flash contoller module log macro */
#define FC_LOG_COLOR_NONE        "\e[0m"
#define FC_LOG_COLOR_RED     "\e[31m"
#define FC_LOG_COLOR_GREEN       "\e[32m"
#define FC_LOG_COLOR_YELLOW  "\e[33m"
#define FC_LOG_COLOR_BLUE        "\e[34m"

//#define FLASH_CONTROLLER_DEBUG

#ifdef FLASH_CONTROLLER_DEBUG
#define fc_dbg_without_newline(fmt,...) \
    printf(FC_LOG_COLOR_BLUE "[" AMP_CPU_ID_STR " FC_D][%s:%d] " fmt \
        FC_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define fc_dbg(fmt,...) fc_dbg_without_newline(fmt"\n", ##__VA_ARGS__)
#else
#define fc_dbg_without_newline(fmt,...)
#define fc_dbg(fmt, args...)
#endif /* FLASH_CONTROLLER_DEBUG */

#define fc_info_without_newline(fmt,...) \
    printf(FC_LOG_COLOR_GREEN "[" AMP_CPU_ID_STR " FC_I][%s:%d] " fmt \
        FC_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define fc_info(fmt,...) fc_info_without_newline(fmt"\n", ##__VA_ARGS__)

#define fc_warn_without_newline(fmt,...) \
			printf(FC_LOG_COLOR_YELLOW "[ " AMP_CPU_ID_STR " FC_W][%s:%d] " fmt \
				FC_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define fc_warn(fmt,...) fc_warn_without_newline(fmt"\n", ##__VA_ARGS__)

#define fc_err_without_newline(fmt,...) \
	printf(FC_LOG_COLOR_RED "[" AMP_CPU_ID_STR " FC_E][%s:%d] " fmt \
		FC_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define fc_err(fmt,...) fc_err_without_newline(fmt"\n", ##__VA_ARGS__)

enum fc_driver_ret_code
{
	FC_RET_CODE_OK = 0,
	FC_RET_CODE_HW_BUSY = 5,
	FC_RET_CODE_MUTEX_INIT_FAILED,
	FC_RET_CODE_MUTEX_LOCK_FAILED,
	FC_RET_CODE_MUTEX_UNLOCK_FAILED,
	FC_RET_CODE_AMP_LOCK_TIMEOUT,
	FC_RET_CODE_CHECK_READ_FIFO_TIMEOUT,
	FC_RET_CODE_CHECK_WRITE_FIFO_TIMEOUT,
	FC_RET_CODE_WAIT_FC_IDLE_TIMEOUT,
	FC_RET_CODE_POLL_OPR_TIMEOUT,
	FC_RET_CODE_GET_CLK_FAILED,
	FC_RET_CODE_SET_CLK_FREQ_FAILED,
	FC_RET_CODE_SET_CLK_PARENT_FAILED,
	FC_RET_CODE_ENABLE_CLK_FAILED,
	FC_RET_CODE_DISABLE_CLK_FAILED,
	FC_RET_CODE_INVALID_CMD_LINE,
	FC_RET_CODE_INVALID_ADDR_LINE,
	FC_RET_CODE_INVALID_DUMMY_LINE,
	FC_RET_CODE_INVALID_PAYLOAD_LINE,
	FC_RET_CODE_INVALID_ALL_LINE,
	FC_RET_CODE_INVALID_ADDR_LEN,
	FC_RET_CODE_INVALID_DUMMY_LEN,
	FC_RET_CODE_INVALID_PAYLOAD_BUF,
	FC_RET_CODE_INVALID_PAYLOAD_LEN,
	FC_RET_CODE_MAX
};

typedef struct fc_spi_timing_cfg
{
	uint8_t clk_polarity:1;
	uint8_t clk_phase:1;
	uint8_t cs_active_low:1;
	uint8_t rx_lsb_first:1;
} fc_spi_timing_cfg_t;


#define FLASH_CONTROLLER_MEM_MAP_NUM 6
#define FLASH_CONTROLLER_MEM_MAP_ADDR_ALIGN 16


typedef struct flash_controller
{
	struct hal_mutex mutex;

#ifdef CONFIG_USE_FLASH_CONTROLLER_ON_AMP
	int amp_lock_id;
#endif

#ifdef CONFIG_DISABLE_INTERRUPT_WHEN_USING_INDIRECT_MODE
	unsigned long irq_flags;
	hal_spinlock_t spinlock;
#endif

	unsigned long reg_base_addr;
	unsigned long start_addr;
	unsigned long end_addr;

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
	hal_clk_type_t input_clk_type;
	hal_clk_id_t input_clk_id;
	unsigned int input_clk_freq;

	const void *pinmux_arr;
	unsigned int pin_num;

	fc_spi_timing_cfg_t spi_timing_cfg;
#endif

#ifdef FLASH_CONTROLLER_PM_SUPPORT
	struct pm_device pm_dev;
#endif
} flash_controller_t;

void dump_fc_reg(const flash_controller_t *fc);

int flash_controller_init(flash_controller_t *fc, unsigned int clk_freq);
int flash_controller_exec_operation(flash_controller_t *fc, const fc_operation_t *opr_info);

#if !defined(CONFIG_USE_FLASH_CONTROLLER_ON_AMP) || defined(CONFIG_FC_ON_AMP_BOOT_CPU)
int flash_controller_set_output_clk_freq(flash_controller_t *fc, unsigned int clk_freq);
int flash_controller_setup_mem_map_opr(flash_controller_t *fc, const fc_mem_map_operation_t *opr);
int flash_controller_add_mem_map(flash_controller_t *fc, const fc_mem_map_info_t *mmap);
int flash_controller_del_mem_map(flash_controller_t *fc, const fc_mem_map_info_t *mmap);
#endif

#endif /* __FLASH_CONTROLLER_H__ */
