/*********************************************************************************
  *Copyright(C),2014-2023,GuRobot
  *Author:  weigang
  *Date:  2023.07.13
  *Description:  HashMap 函数
  *			基于hashMap的文件操作
  *History:
**********************************************************************************/
#ifndef _REALIZE_UNIT_HASH_CMBFILE_H_
#define _REALIZE_UNIT_HASH_CMBFILE_H_

#include <stdint.h>
#ifdef INDEPENDENCE //独立编译
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define realize_unit_log_error printf
typedef struct __info {
    char *name;
    int offset;
    int size;
} unit_fs_file_info_t;

#define realize_unit_mem_calloc calloc
#define realize_unit_mem_free free
#define realize_unit_mem_strdup strdup
#define UNIT_FS_SEEK_SET SEEK_SET
#else //集成到GuliteOS
#include "../realize_unit_fs/realize_unit_fs.h"
#endif
//头文件最大是100K，假设一个文件名长度为10，约可以存放5000个文件
#define HASHMAP_FILE_MAX_COUNT 5 * 1024     //最多可以存放5000个文件
#define HASHMAP_FILE_MAX_FILE_NAME_SIZE 5 * 20 * 1024  //文件名存储区BUFF100K

// 合并文件格式
//  |字段描述 |GCMB|子文件个数|正文偏移|   索引区  |文件名存储区|正文区|
//  |字节数   |4   |   2    |  4    | 子文件数*4|          ｜    ｜ 

//  索引区存放子文件文件名的偏移
//  |文件名偏移|
//  |   4    |

//   文件名存储区，单个文件名格式,文件名长度不超过255字节,文件名头部不超过65535字节
//  |文件名长度|文件偏移|文件长度|重复Hash值的下一个文件名偏移|文件名|
//  | 1      |  4    |    4 |          4              |n   |

//  正文区，单个文件格式
//  文件内容顺序排列


//写入逻辑
//1. 写入文件头部，包括文件头标识，子文件个数，正文偏移，索引区
//2. 写入文件名存储区，包括文件名长度，文件偏移，文件长度，重复Hash值的下一个文件名偏移，文件名
//3. 更新索引区，写入文件
//4. 写入文件正文区，包括文件内容， 按顺序写入

typedef struct {
    uint8_t file_name_length;  //1个字节
    uint32_t file_offset;        //4个字节
    uint32_t file_length;        //4个字节
    uint32_t next_node_offset; //4个字节
    uint32_t next_node_index; //4个字节
    char* file_name;               //文件名长度不超过255字节
} gu_file_name_area_t;

typedef struct file_name_index_struct {
    uint32_t first_node_index;
    uint32_t file_name_offset; //4个字节
} gu_file_name_index_t;

typedef struct hash_cmbfile_struct {
    unsigned short file_count;      //2个字节
    uint32_t* file_name_index;       //子文件数*4个字节
    gu_file_name_area_t* file_name_area; //文件名存储区
    uint32_t* file_name_index_first_node_index; //索引区内容辅助字段
    uint32_t cur_file_name_area_offset; //文件名存储区当前偏移
    uint32_t cur_content_offset; //当前正文偏移
    uint32_t cur_file_index; //当前文件索引
} gu_hash_cmbfile_t;


/**
 * 新建一个hashMap文件
 * @param cmb_head 初始化的hashMap文件头部
 * @return
 *      文件句柄
 * 实现:
 *
 */
int realize_unit_hash_cmbfile_init_file(gu_hash_cmbfile_t** cmb_head, uint32_t file_count);

/**
 * 新建一个添加文件到hashMap文件的头部，head内存需要提前申请好
 * @param filename 要加入合并文件的文件名
 *        file_body_length  要加入合并文件的文件内容的长度
 *        cmb_head  合并文件的头部，传出参数
 *        cmb_head_length  合并文件的头部的长度，传出参数
 * @return
 * 实现:
 *
 */
int realize_unit_hash_cmbfile_add_file_to_head(gu_hash_cmbfile_t* cmb_head, const char* filename, const uint32_t content_size);


/**
 * 更新文件索引区
 * @param cmb_fp 文件句柄
 *        cmb_head  合并文件的头部
 * @return
 * 实现:
 *
 */
int realize_unit_hash_cmbfile_write_header(void* cmb_fp, gu_hash_cmbfile_t* cmb_head);

/**
 * 从hashMap的合并文件读取所有文件的基本信息
 * @param fp 文件指针
 * @return
 * 实现:
 *
 */
// void * realize_unit_hash_cmbfile_get_compose_all_file_info(void* fp)


/**
 * 新建一个添加文件到hashMap文件的头部，head内存需要提前申请好
 * @param filename 要加入合并文件的文件名
 *        file_body_length  要加入合并文件的文件内容的长度
 *        cmb_head  合并文件的头部，传出参数
 *        cmb_head_length  合并文件的头部的长度，传出参数
 * @return
 * 实现:
 *
 */
int realize_unit_hash_cmbfile_write_file_body(void* cmb_fp, char* file_body, uint32_t file_body_length);

int realize_unit_hash_cmbfile_destory(gu_hash_cmbfile_t** cmb_head);
/**
 * 从hashMap的合并文件读取文件的基本信息
 * @param fp 文件指针
 *        name  文件名
 *      file_info  文件信息
 * @return
 * 实现:
 *
 */
unit_fs_file_info_t* realize_unit_hash_cmbfile_get_compose_info(void* fp, const char* name);

#endif

