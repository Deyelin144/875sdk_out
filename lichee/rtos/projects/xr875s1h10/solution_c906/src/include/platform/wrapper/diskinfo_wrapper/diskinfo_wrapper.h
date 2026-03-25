#ifndef __DISKINFO_WRAPPER_H__
#define __DISKINFO_WRAPPER_H__

#include "../../../mod_realize/realize_unit_diskinfo/realize_unit_diskinfo.h"

/**
 * @brief 获取已挂载磁盘信息
 * @param 
 * @return int 0 succ
 */
typedef int (*diskinfo_get_info_t)(int *disk_num, unit_drives_info_t** drives_info);

typedef struct {
	diskinfo_get_info_t get_disk_info;
} diskinfo_wrapper_t;


#endif
