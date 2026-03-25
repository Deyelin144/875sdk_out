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

#ifndef SUNXI_HAL_SPINOR_H
#define SUNXI_HAL_SPINOR_H

#include <stdint.h>

#ifdef CONFIG_DRIVERS_SPI_NOR_NG

int hal_spi_nor_init(unsigned int id);

int hal_spi_nor_read(unsigned int id, unsigned int addr, unsigned char *buf, unsigned int len);
int hal_spi_nor_write(unsigned int id, unsigned int addr, unsigned char *buf, unsigned int len);
int hal_spi_nor_erase(unsigned int id, unsigned int addr, unsigned int len);

/*
 * In order to be compatible with code which use legacy APIs,
 * provide these legacy APIs below, don't use it in new code!!!
 */
int nor_init(void);
int nor_read(unsigned int addr, char *buf, unsigned int len);
int nor_write(unsigned int addr, char *buf, unsigned int len);
int nor_erase(unsigned int addr, unsigned int len);

int32_t hal_spinor_read_data(uint32_t addr, void *buf, uint32_t cnt);
int32_t hal_spinor_panic_read_data(uint32_t addr, void *buf, uint32_t cnt);
int32_t hal_spinor_panic_program_data(uint32_t addr, const void *buf, uint32_t cnt);
int32_t hal_spinor_panic_erase_sector(uint32_t addr, uint32_t size);

#else

#include <sunxi_hal_common.h>
#include <sunxi_hal_spi.h>
#include <hal_sem.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SUNXI_HAL_SPINOR_API_VERSION 1
#define SUNXI_HAL_SPINOR_DRV_VERSION 0

typedef enum sunxi_hal_spinor_signal_event
{
    ARM_FLASH_EVENT_READY = (1UL << 0),
    ARM_FLASH_EVENT_ERROR = (1UL << 1),
} sunxi_hal_spinor_signal_event_t;

typedef struct sunxi_hal_spinor_status
{
    uint32_t busy:  1;
    uint32_t error: 1;
    uint32_t reserved: 30;
} sunxi_hal_spinor_status_t;

typedef struct _sunxi_hal_spinor_sector_info
{
    uint32_t start;
    uint32_t end;
} sunxi_hal_spinor_sector_info;

typedef struct _sunxi_hal_spinor_info
{
    sunxi_hal_spinor_sector_info *sector_info;
    uint32_t sector_count;
    uint32_t sector_size;
    uint32_t page_size;
    uint32_t program_unit;
    uint8_t  erased_value;
    uint8_t  reserved[3];
} sunxi_hal_spinor_info;

typedef struct sunxi_hal_spinor_capabilities
{
    uint32_t event_ready: 1;
    uint32_t data_width: 2;
    uint32_t erase_chip: 1;
    uint32_t reserved: 28;
} sunxi_hal_spinor_capabilities_t;

int32_t hal_spinor_init(sunxi_hal_spinor_signal_event_t cb_event);
int32_t hal_spinor_deinit(void);
sunxi_hal_version_t hal_spinor_get_version(int32_t dev);
sunxi_hal_spinor_capabilities_t hal_spinor_get_capabilities(void);
sunxi_hal_spinor_status_t hal_spinor_get_status(void);
int32_t hal_spinor_power_control(sunxi_hal_power_state_e state);
int32_t hal_spinor_read_data(uint32_t addr, void *buf, uint32_t cnt);
int32_t hal_spinor_program_data(uint32_t addr, const void *buf, uint32_t cnt);
int32_t hal_spinor_erase_sector(uint32_t addr, uint32_t size);
int32_t hal_spinor_erase_chip(void);
int32_t hal_spinor_sync(void);
sunxi_hal_spinor_info *hal_spinor_get_info(void);
void hal_spinor_signal_event(uint32_t event);
int32_t hal_spinor_control(int32_t dev, uint32_t command, uint32_t arg);

#ifdef CONFIG_DRIVERS_SPINOR_PANIC_WRITE
int32_t hal_spinor_panic_read_data(uint32_t addr, void *buf, uint32_t cnt);
int32_t hal_spinor_panic_program_data(uint32_t addr, const void *buf, uint32_t cnt);
int32_t hal_spinor_panic_erase_sector(uint32_t addr, uint32_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_DRIVERS_SPI_NOR_NG */

#endif  /*SUNXI_HAL_SPINOR_H*/
