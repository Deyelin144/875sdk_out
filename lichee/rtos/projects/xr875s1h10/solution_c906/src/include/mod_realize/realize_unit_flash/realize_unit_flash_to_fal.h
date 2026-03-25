/**
 * 此文件对接FAL库
*/
#ifndef _REALIZE_UNIT_FLASH_H_
#define _REALIZE_UNIT_FLASH_H_
 
#include <stdio.h>
#include "../../platform/wrapper/flash_wrapper/flash_wrapper.h"
#define FAL_PART_HAS_TABLE_CFG
#define FAL_FLASH_DEV_TABLE
/**
 * @brief 获取flash设备表
 * @return 返回一个指针数组,每个指针指向一个
*/
struct fal_flash_dev **realize_unit_flash_to_fal_flash_dev_table();
size_t realize_unit_flash_to_fal_device_table_len();

#ifdef FAL_PART_HAS_TABLE_CFG/* FAL_PART_HAS_TABLE_CFG */
#define FAL_PART_TABLE

struct fal_partition *realize_unit_flash_to_fal_partition_table();
size_t realize_unit_flash_to_fal_partition_table_len();

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _REALIZE_UNIT_FLASH_H_ */