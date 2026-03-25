#ifndef _REALIZE_UNIT_FS_SORT_H_
#define _REALIZE_UNIT_FS_SORT_H_

#include <stdbool.h>
#include "../../platform/gulitesf_type.h"
#include "realize_unit_fs.h"

#ifdef CONFIG_FS_SORT_SUPPORT
#define BATCH_SIZE 50               // 每次读取的文件数量
#define RECORDS_PER_FILE 50         // 每个索引文件最多存储 50 条记录
#define CACHE_SIZE 50               // 多路归并排序的时候，每次读取文件的个数
#define INDEX_COUNT_LIMIT 20        // 索引文件数量限制，超过20个不采用缓存方式
#define APP_OPTION_DEBUG 1          // 调试开关，主要打印app发起修改后的文件信息
#define READ_DIR_DEBUG 1            // 调试开关，主要打印首次读取目录下排序后的文件信息
#define MULTIWAY_MERGE_SORT_DEBUG 1 // 调试开关，主要打印归并排序后的文件信息
#define READ_INFO_DEBUG 1           // 调试开关，主要打印读取指定目录下的文件信息
#define MANAGE_INFO_DEBUG 1         // 调试开关，主要打印管理文件的信息

/*文件排序方式设置*/
typedef enum {
	FILE_SORT_NONE = 0,          // 不进行排序，按存储顺序返回
    FILE_SORT_BY_NAME_ASC,   	 // 按文件名升序排列
    FILE_SORT_BY_NAME_DESC,      // 按文件名降序排列
    FILE_SORT_BY_SIZE_ASC,       // 按文件大小升序排列
    FILE_SORT_BY_SIZE_DESC,      // 按文件大小降序排列
    FILE_SORT_BY_TIME_ASC,       // 按文件修改时间升序排列
    FILE_SORT_BY_TIME_DESC,      // 按文件修改时间降序排列
    FILE_SORT_MAX,               // 边界标识
} file_sort_type_t;

/*定义文件排序参数设置*/
typedef struct {
    int updatecache;				 // 数值1代表清空全局链表，并且重新生成更新该目录对应的索引文件，目的主要是为了开机和t卡热插拔后，清除缓存，重新进行索引文件的更新。数值2代表用户改变当前目录的排序方式。其余情况传入数字0
	bool withfiletypes;				 //	bool值，指定是否将文件作为fs.Dirent对象返回，默认值为"false" 
	char encode[16];				 // 字符串，指定给回调参数指定的文件名使用哪种编码，默认为"utf8"
    char filter_ext[32];             // 过滤文件扩展名，这里根据需求后续动态调整
    uint16_t start_index;            // 起始索引    
    uint16_t end_index;         	 // 返回的文件数量
    file_sort_type_t sort_type;      // 排序方式
    uint8_t file_type;               // 数值0代表需要文件夹+文件，数值1代表只需要文件，数值2代表只需要文件夹
	char *path;						 // 需要排序的目录
} file_sort_config_t;

/*文件信息的格式*/
typedef struct {
    unsigned int size;
    unit_fs_type_t type;
    char name[256];//文件名目前最大支持256个字符
} sort_file_info_t;

/*单个目录下，对应链表的数据结构*/
typedef struct change_node {
    unit_fs_change_type_t change_type;		// 判断是执行增加，删除，修改中的哪一种操作
	sort_file_info_t *file_info;		    // 对应的文件信息
	char old_name[256]; 		            // RENAME时记录旧文件名
    struct change_node *next;
} change_node_t;

typedef struct dir_cache_node {
    char* dir_path;				    // 文件目录
    change_node_t * changes;	    // 当前文件目录对应的内容
    change_node_t *folder_changes;	// 当前文件目录对应的文件夹内容
} dir_cache_node_t;

/*定义堆节点结构体*/
typedef struct {
	sort_file_info_t info;
    int file_index;
} heap_node_t;

/*定义管理文件结构体*/
typedef struct {
    int index;                      // 索引文件的序号
    int file_count;                 // 索引文件包含的文件数量
    char first_file_name[256];      // 对应索引文件中第一个文件名名称
	char last_file_name[256];       // 对应索引文件中最后一个文件名名称
	unsigned int first_file_size;   // 对应索引文件中第一个文件的大小
	unsigned int last_file_size;    // 对应索引文件中最后一个文件的大小  
} manage_file_info_t;

/*定义管理文件数量结构体*/
typedef struct {
    int folderindex_count;       // 文件夹索引文件的个数
    int fileindex_count;         // 文件索引文件的个数
    int foldertotal_count;       // 文件夹的总数
	int filetotal_count;         // 文件的总数
} manage_file_count_t;

/*定义文件范围信息结构体*/
typedef struct {
    int start_index;  // 起始索引
    int end_index;    // 结束索引
} file_range_t;

/*定义多路归并排序缓存的结构体*/
typedef struct {
    sort_file_info_t cache[CACHE_SIZE];
    int cache_index;  // 当前缓存中可用数据的索引
    int cache_count;  // 当前缓存中有效数据的数量
    void *fd;         // 对应的文件句柄
} file_cache_t;

int realize_unit_fs_sort_files(file_sort_config_t **file_setopt, sort_file_info_t **file_info, int *file_total_count, int *folder_total_count);
int realize_unit_fs_record_change(dir_cache_node_t* dc, unit_fs_change_type_t ct, unit_fs_info_t *file_info, char* old_name);
dir_cache_node_t* realize_unit_fs_path_exist(const char* dir_path);
#endif


#endif