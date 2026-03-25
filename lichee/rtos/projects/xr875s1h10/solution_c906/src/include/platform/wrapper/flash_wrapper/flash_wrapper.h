
#ifndef _FLASH_WRAPPER_H_
#define _FLASH_WRAPPER_H_

#include <stdio.h>
#include <stdint.h>

#define FLASH_NAME_MAX_SIZE 24

/* ===================== flash设备 ========================= */
typedef struct {
    char name[FLASH_NAME_MAX_SIZE];//设备名
    uint32_t addr;//对 Flash 操作的起始地址
    size_t len;//Flash 的总大小
    size_t blk_size;//Flash 块/扇区大小

    struct {
        int (*init)(void);
        int (*read)(long offset, uint8_t *buf, size_t size);
        int (*write)(long offset, const uint8_t *buf, size_t size);
        int (*erase)(long offset, size_t size);
    } ops;//flash 的操作函数, 如果没有 init 初始化过程，第一个操作函数位置可以置空。

    /* 设置写粒度，单位 bit. 
       常见 Flash 写粒度：1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1)/ 64(stm32l4)
       0 表示未生效 */
    size_t write_gran;
} flash_wrapper_dev_t;

/* ===================== flash分区 ========================= */
//一个设备可以分成多个分区
typedef struct {
    char name[FLASH_NAME_MAX_SIZE];//分区名(唯一)
    char flash_name[FLASH_NAME_MAX_SIZE];//分区对应的设备
    long offset;//偏移地址（相对 Flash 设备内部）
    size_t len;//分区大小
} flash_wrapper_partition_t;

/**
 * @brief 获取flash设备表
 * 
 * @return 返回一个设备指针数组
 */
typedef flash_wrapper_dev_t** (*flash_dev_table_t)(void);

/**
 * @brief 获取flash设备的数量
 * 
 * @return flash设备的数量
 */
typedef size_t (*device_table_len_t)(void);

/**
 * @brief 获取flash分区表
 * 
 * @return 分区指针数组
 */
typedef flash_wrapper_partition_t** (*flash_partition_table_t)(void);

/**
 * @brief 获取分区的数量
 * 
 * @return 分区的数量
 */
typedef size_t (*partition_table_len_t)(void);

typedef struct {
    flash_dev_table_t flash_dev_table;
    device_table_len_t device_table_len;
    flash_partition_table_t flash_partition_table;
    partition_table_len_t partition_table_len;
}flash_wrapper_t;


#endif /* _FLASH_WRAPPER_H_ */