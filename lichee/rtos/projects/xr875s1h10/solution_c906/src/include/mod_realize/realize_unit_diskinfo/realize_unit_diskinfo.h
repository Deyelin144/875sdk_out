#ifndef _REALIZE_UNIT_DISKINFO_H_
#define _REALIZE_UNIT_DISKINFO_H_


typedef struct {
	char mounted[64];     			//磁盘名
	unsigned long int blocks;		//磁盘总内存
	unsigned long int used;			//磁盘已用空间
	unsigned long int available;		//磁盘可用空间
	unsigned int capacity;				//磁盘已用内存百分比
} unit_drives_info_t;

int realize_unit_diskinfo_get_drives_space(int *disk_num, unit_drives_info_t** drives_info);
int realize_unit_diskinfo_get_mount_stat();
#endif