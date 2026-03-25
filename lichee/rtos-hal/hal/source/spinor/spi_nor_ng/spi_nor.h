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

#ifndef __SPI_NOR_H__
#define __SPI_NOR_H__

#include <stdint.h>
#include <hal_mutex.h>

#if defined(CONFIG_USE_SPI_NOR_ON_AMP)
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

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
#ifdef CONFIG_COMPONENTS_PM
#include <pm_devops.h>
#define SPI_NOR_FLASH_PM_SUPPORT
#endif
#endif

#define SNOR_LOG_COLOR_NONE        "\e[0m"
#define SNOR_LOG_COLOR_RED     "\e[31m"
#define SNOR_LOG_COLOR_GREEN       "\e[32m"
#define SNOR_LOG_COLOR_YELLOW  "\e[33m"
#define SNOR_LOG_COLOR_BLUE        "\e[34m"

//#define SNOR_DEBUG

#ifdef SNOR_DEBUG
#define snor_dbg_without_newline(fmt,...) \
    printf(SNOR_LOG_COLOR_BLUE "[" AMP_CPU_ID_STR " SNOR_D][%s:%d] " fmt \
        SNOR_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define snor_dbg(fmt,...) snor_dbg_without_newline(fmt"\n", ##__VA_ARGS__)
#else
#define snor_dbg_without_newline(fmt,...)
#define snor_dbg(fmt, args...)
#endif /* SNOR_DEBUG */

#define snor_info_without_newline(fmt,...) \
    printf(SNOR_LOG_COLOR_GREEN "[" AMP_CPU_ID_STR " SNOR_I][%s:%d] " fmt \
        SNOR_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define snor_info(fmt,...) snor_info_without_newline(fmt"\n", ##__VA_ARGS__)

#define snor_warn_without_newline(fmt,...) \
		printf(SNOR_LOG_COLOR_YELLOW "[ " AMP_CPU_ID_STR " SNOR_W][%s:%d] " fmt \
			SNOR_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define snor_warn(fmt,...) snor_warn_without_newline(fmt"\n", ##__VA_ARGS__)

#define snor_err_without_newline(fmt,...) \
	printf(SNOR_LOG_COLOR_RED "[ " AMP_CPU_ID_STR " SNOR_E][%s:%d] " fmt \
		SNOR_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define snor_err(fmt,...) snor_err_without_newline(fmt"\n", ##__VA_ARGS__)

#define SNOR_DEFAULT_PAGE_SIZE 256
#define SNOR_MAX_JEDEC_ID_LEN 3
#define SNOR_MAX_MODE_FIELD_BYTE_LEN 4
#define SNOR_MAX_MEM_MAP_CNT 6

#define SZ_4K       (4 * 1024)
#define SZ_32K      (32 * 1024)
#define SZ_64K      (64 * 1024)
#define SZ_128K     (128 * 1024)
#define SZ_256K     (256 * 1024)
#define SZ_512K     (512 * 1024)
#define SZ_1M       (1 * 1024 * 1024)
#define SZ_2M       (2 * 1024 * 1024)
#define SZ_4M       (4 * 1024 * 1024)
#define SZ_8M       (8 * 1024 * 1024)
#define SZ_12M      (12 * 1024 * 1024)
#define SZ_14M      (14 * 1024 * 1024)
#define SZ_15M      (15 * 1024 * 1024)
#define SZ_15872K   (15872 * 1024)
#define SZ_16128K   (16128 * 1024)
#define SZ_16M      (16 * 1024 * 1024)
#define SZ_32M      (32 * 1024 * 1024)
#define SZ_64M      (64 * 1024 * 1024)

typedef struct spi_nor_protocol_info
{
	uint8_t addr_field_byte_len;
	uint8_t mode_clocks_num:3;
	uint8_t dummy_clocks_num:5; /* JESD216 spec called wait_states */

	/* 0: not presend, others: specific line num, the same as below */
	uint16_t cmd_field_line_num:3;
	uint16_t addr_field_line_num:3;
	uint16_t payload_field_line_num:3;
} spi_nor_protocol_info_t;

typedef struct spi_nor_rw_proto
{
	spi_nor_protocol_info_t proto;
	uint8_t cmd;
} spi_nor_rw_proto_t;

typedef struct spi_nor_mem_map_proto
{
	spi_nor_protocol_info_t proto;

	uint8_t cmd;
	uint8_t mode[SNOR_MAX_MODE_FIELD_BYTE_LEN];
} spi_nor_mem_map_proto_t;

typedef struct spi_nor_poll_operation
{
	spi_nor_protocol_info_t proto;
	uint8_t cmd;
	uint8_t data_len;
	uint32_t bit_mask;
	uint32_t match_value;
	uint32_t poll_cnt;
} spi_nor_poll_operation_t;

#define SNOR_PYALOAD_DIRECTION_INPUT 0  /* write something to spi nor flash */
#define SNOR_PYALOAD_DIRECTION_OUTPUT 1 /* read something from spi nor flash */

typedef struct spi_nor_operation
{
	spi_nor_protocol_info_t proto;
	uint8_t payload_direction;//0: input, other: output
	uint8_t cmd;
	uint32_t addr;
	uint8_t mode[SNOR_MAX_MODE_FIELD_BYTE_LEN];//Currently only Continuous Read Mode function use it
	uint8_t *payload_buf;
	uint32_t payload_len;

	spi_nor_poll_operation_t *poll_opr;
} spi_nor_operation_t;



typedef struct spi_nor_mem_map
{
	unsigned long mem_addr;
	uint32_t data_addr;
	uint32_t len;
} spi_nor_mem_map_t;

typedef struct spi_nor_mem_map_cfg
{
	spi_nor_mem_map_t maps[SNOR_MAX_MEM_MAP_CNT];
	uint8_t is_valid[SNOR_MAX_MEM_MAP_CNT];

	spi_nor_mem_map_proto_t mmap_proto;
} spi_nor_mem_map_cfg_t;

typedef enum spi_nor_read_proto_type
{
	SNOR_READ_PROTO_GENERAL,
	SNOR_READ_PROTO_FAST,
	//SNOR_READ_PROTO_1_1_1_DTR,

	/* Dual SPI */
	SNOR_READ_PROTO_1_1_2,
	SNOR_READ_PROTO_1_2_2,
	SNOR_READ_PROTO_2_2_2,
	//SNOR_READ_PROTO_1_2_2_DTR,

	/* Quad SPI */
	SNOR_READ_PROTO_1_1_4,
	SNOR_READ_PROTO_1_4_4,
	SNOR_READ_PROTO_4_4_4,
	//SNOR_READ_PROTO_1_4_4_DTR,

	SNOR_READ_PROTO_TYPE_MAX
} spi_nor_read_proto_type_t;

typedef struct spi_nor_info
{
	char *model;
	unsigned char id[SNOR_MAX_JEDEC_ID_LEN];
	unsigned int total_size;
	uint32_t blk_size;
	uint32_t page_size;

	spi_nor_rw_proto_t read_proto[SNOR_READ_PROTO_TYPE_MAX];

	uint32_t hwcaps;

#define SNOR_HWCAPS_READ BIT(0)
#define SNOR_HWCAPS_READ_FAST BIT(1)
#define SNOR_HWCAPS_READ_1_1_2 BIT(2) /* Dual Output*/
#define SNOR_HWCAPS_READ_1_2_2 BIT(3) /* Dual IO */
#define SNOR_HWCAPS_READ_2_2_2 BIT(4) /* current flash rarely used */
#define SNOR_HWCAPS_READ_1_1_4 BIT(5) /* Quad Output */
#define SNOR_HWCAPS_READ_1_4_4 BIT(6) /* Quad IO */
#define SNOR_HWCAPS_READ_4_4_4 BIT(7) /* QPI */
#define SNOR_HWCAPS_PP_1_1_4 BIT(8)

#define SNOR_HWCAPS_4K_ERASE_BLK        BIT(10)
#define SNOR_HWCAPS_32K_ERASE_BLK       BIT(11)
#define SNOR_HWCAPS_64K_ERASE_BLK       BIT(12)

#define SNOR_HWCAPS_SOFT_RESET BIT(13)
#define SNOR_HWCAPS_INDIVIDUAL_PROTECT  BIT(14)
#define SNOR_HWCAPS_3BYTE_ADDR BIT(15)
#define SNOR_HWCAPS_4BYTE_ADDR BIT(16)

#define SNOR_HWCAPS_ALL_ERASE_BLK       (SNOR_HWCAPS_4K_ERASE_BLK | \
											 SNOR_HWCAPS_32K_ERASE_BLK | \
											 SNOR_HWCAPS_64K_ERASE_BLK)
#define SNOR_HWCAPS_GENERAL             (SNOR_HWCAPS_3BYTE_ADDR | \
											SNOR_HWCAPS_ALL_ERASE_BLK | \
											 SNOR_HWCAPS_READ_1_1_2 | \
											 SNOR_HWCAPS_READ_1_2_2 | \
											 SNOR_HWCAPS_READ_1_1_4 | \
											 SNOR_HWCAPS_READ_1_4_4)
} spi_nor_info_t;

typedef struct spi_nor_cfg
{
	uint8_t addr_mode;
	uint8_t erase_4k_cmd;
	uint8_t erase_32k_cmd;
	uint8_t erase_64k_cmd;
	uint8_t write_cmd;

	spi_nor_read_proto_type_t read_proto_type;
	spi_nor_protocol_info_t write_proto;

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	spi_nor_mem_map_cfg_t mmap;
#endif

	uint32_t flag;
} spi_nor_cfg_t;

typedef struct spi_nor_controller_ops
{
	int (*init)(unsigned int id, unsigned int clk_freq);
	int (*exec_opr)(unsigned int controller_id, const spi_nor_operation_t *opr);

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
	int (*set_output_clk_freq)(unsigned int id, unsigned int clk_freq);
	int (*setup_mem_map_proto)(unsigned int id, const spi_nor_mem_map_proto_t *mmap_proto);
	int (*add_mem_map)(unsigned int id, const spi_nor_mem_map_t *mmap);
	int (*del_mem_map)(unsigned int id, const spi_nor_mem_map_t *mmap);
#endif
} spi_nor_controller_ops_t;

typedef struct spi_nor_controller
{
	unsigned int id;
	const spi_nor_controller_ops_t *ops;
} spi_nor_controller_t;

typedef struct spi_nor
{
	struct hal_mutex lock;

	spi_nor_info_t info;
	spi_nor_cfg_t cfg;

	const spi_nor_controller_t *controller;

#ifdef SPI_NOR_FLASH_PM_SUPPORT
	struct pm_device pm_dev;
#endif
} spi_nor_t;

int spi_nor_init(spi_nor_t *nor);

int spi_nor_read(spi_nor_t *nor, unsigned int addr, unsigned int len, unsigned char *buf);
int spi_nor_write(spi_nor_t *nor, unsigned int addr, unsigned int len, const unsigned char *buf);
int spi_nor_erase(spi_nor_t *nor, unsigned int addr, unsigned int len);

#if !defined(CONFIG_USE_SPI_NOR_ON_AMP) || defined(CONFIG_SNOR_ON_AMP_BOOT_CPU)
int spi_nor_add_mem_map(spi_nor_t *nor, unsigned long mem_addr, unsigned int len, unsigned int data_addr);
int spi_nor_del_mem_map(spi_nor_t *nor, unsigned long mem_addr, unsigned int len, unsigned int data_addr);
#else
static inline int spi_nor_add_mem_map(spi_nor_t *nor, unsigned long mem_addr, unsigned int len, unsigned int data_addr)
{
	return -1;
}
static inline int spi_nor_del_mem_map(spi_nor_t *nor, unsigned long mem_addr, unsigned int len, unsigned int data_addr)
{
	return -1;
}
#endif

void dump_spi_nor_object(const spi_nor_t *nor);

#endif /* __SPI_NOR_H__ */
