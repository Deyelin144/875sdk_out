/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for FLASH drivers
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_FLASH_H_
#define ZEPHYR_INCLUDE_DRIVERS_FLASH_H_

/**
 * @brief FLASH internal Interface
 * @defgroup flash_internal_interface FLASH internal Interface
 * @ingroup io_interfaces
 * @{
 */

#include <errno.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flash memory parameters. Contents of this structure suppose to be
 * filled in during flash device initialization and stay constant
 * through a runtime.
 */
struct flash_parameters {
	const size_t write_block_size;
	uint8_t erase_value; /* Byte value of erased flash */
};

struct flash_pages_info {
	off_t start_offset; /* offset from the base of flash address */
	size_t size;
	uint32_t index;
};

/**
 *  @brief  Read data from flash
 *
 *  All flash drivers support reads without alignment restrictions on
 *  the read offset, the read size, or the destination address.
 *
 *  @param  dev             : flash dev
 *  @param  offset          : Offset (byte aligned) to read
 *  @param  data            : Buffer to store read data
 *  @param  len             : Number of bytes to read.
 *
 *  @return  0 on success, negative errno code on fail.
 */
int flash_read(const struct device *dev, off_t offset, void *data,
			 size_t len);

/**
 *  @brief  Write buffer into flash memory.
 *
 *  All flash drivers support a source buffer located either in RAM or
 *  SoC flash, without alignment restrictions on the source address.
 *  Write size and offset must be multiples of the minimum write block size
 *  supported by the driver.
 *
 *  Any necessary write protection management is performed by the driver
 *  write implementation itself.
 *
 *  @param  dev             : flash device
 *  @param  offset          : starting offset for the write
 *  @param  data            : data to write
 *  @param  len             : Number of bytes to write
 *
 *  @return  0 on success, negative errno code on fail.
 */
int flash_write(const struct device *dev, off_t offset,
			  const void *data,
			  size_t len);

/**
 *  @brief  Erase part or all of a flash memory
 *
 *  Acceptable values of erase size and offset are subject to
 *  hardware-specific multiples of page size and offset. Please check
 *  the API implemented by the underlying sub driver, for example by
 *  using flash_get_page_info_by_offs() if that is supported by your
 *  flash driver.
 *
 *  Any necessary erase protection management is performed by the driver
 *  erase implementation itself.
 *
 *  @param  dev             : flash device
 *  @param  offset          : erase area starting offset
 *  @param  size            : size of area to be erased
 *
 *  @return  0 on success, negative errno code on fail.
 *
 *  @see flash_get_page_info_by_offs()
 *  @see flash_get_page_info_by_idx()
 */
int flash_erase(const struct device *dev, off_t offset, size_t size);

/**
 *  @brief  Get the size and start offset of flash page at certain flash offset.
 *
 *  @param  dev flash device
 *  @param  offset Offset within the page
 *  @param  info Page Info structure to be filled
 *
 *  @return  0 on success, -EINVAL if page of the offset doesn't exist.
 */
int flash_get_page_info_by_offs(const struct device *dev,
					  off_t offset,
					  struct flash_pages_info *info);

/**
 *  @brief  Get pointer to flash_parameters structure
 *
 *  Returned pointer points to a structure that should be considered
 *  constant through a runtime, regardless if it is defined in RAM or
 *  Flash.
 *  Developer is free to cache the structure pointer or copy its contents.
 *
 *  @return pointer to flash_parameters structure characteristic for
 *          the device.
 */
const struct flash_parameters *flash_get_parameters(const struct device *dev);

/**
 *  @brief  Get the minimum write block size supported by the driver
 *
 *  The write block size supported by the driver might differ from the write
 *  block size of memory used because the driver might implements write-modify
 *  algorithm.
 *
 *  @param  dev flash device
 *
 *  @return  write block size in bytes.
 */
size_t flash_get_write_block_size(const struct device *dev);

/**
 * @brief Flash partition
 *
 * This structure represents a fixed-size partition on a flash device.
 * Each partition contains one or more flash sectors.
 */
struct flash_area {
	/** ID number */
	uint8_t fa_id;
	/** Provided for compatibility with MCUboot */
	uint8_t fa_device_id;
	uint16_t pad16;
	/** Start offset from the beginning of the flash device */
	off_t fa_off;
	/** Total size */
	size_t fa_size;
	/**
	 * Name of the flash device, suitable for passing to
	 * device_get_binding().
	 */
	const char *fa_dev_name;
};

/**
 * @brief Structure for transfer flash sector boundaries
 *
 * This template is used for presentation of flash memory structure. It
 * consumes much less RAM than @ref flash_area
 */
struct flash_sector {
	/** Sector offset from the beginning of the flash device */
	off_t fs_off;
	/** Sector size in bytes */
	size_t fs_size;
};

/**
 * @brief Retrieve partitions flash area from the flash_map.
 *
 * Function Retrieves flash_area from flash_map for given partition.
 *
 * @param[in]  id ID of the flash partition.
 * @param[out] fa Pointer which has to reference flash_area. If
 * @p ID is unknown, it will be NULL on output.
 *
 * @return  0 on success, -EACCES if the flash_map is not available ,
 * -ENOENT if @p ID is unknown.
 */
int flash_area_open(uint8_t id, const struct flash_area **fa);

/**
 * Retrieve info about sectors within the area.
 *
 * @param[in]  fa_id    Given flash area ID
 * @param[out] sectors  buffer for sectors data
 * @param[in,out] count On input Capacity of @p sectors, on output number of
 * sectors Retrieved.
 *
 * @return  0 on success, negative errno code on fail. Especially returns
 * -ENOMEM if There are too many flash pages on the flash_area to fit in the
 * array.
 */
int flash_area_get_sectors(int fa_id, uint32_t *count,
			   struct flash_sector *sectors);

/* it is unused, so make it zero const */
#define FLASH_AREA_ID(label) 0

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_FLASH_H_ */
