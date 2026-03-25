#ifndef _REALIZE_UNIT_FS_H_
#define _REALIZE_UNIT_FS_H_

#include "../../platform/gulitesf_type.h"

#define READ_MAX_SZIE CONFIG_FS_READ_MAX_SZIE
#define JSON_LEN   4

enum {
	REALIZE_UNIT_FS_FILE_NOR,
	REALIZE_UNIT_FS_FILE_COMPOSE,
};

typedef struct __info {
    char *name;
    int offset;
    int size;
} unit_fs_file_info_t;

typedef struct {
	void *buffer;
	int buffer_size;
	int offset;
	int len;
	int is_cur_pos;
	int pos;
    int index;
} unit_fs_data_info_t;

typedef struct {
	char *path;
	int force;
	int recursive;
	int maxretries;
	int retrydelay;
    int mode;
} unit_fs_dir_opt_t;

typedef struct {
	unsigned int free_space_byte;		//空闲空间
	unsigned int total_space_byte;		//总空间
} unit_fs_space_t;

typedef struct {
	gulite_unumber_t addr;
	gulite_unumber_t size;
	gulite_unumber_t offset;
} unit_fs_mem_file_t;

typedef struct {
	char disk[20];
    union {
		void *file;
		unit_fs_mem_file_t *mem_file;
    } fp;
} file_handle_t;

typedef struct {
	char disk[20];
	void *dir;
} dir_handle_t;

typedef enum {
    UNIT_FS_RDONLY = 1,      //以只读方式打开文件
    UNIT_FS_WRONLY = 2,      //以只写方式打开文件               
    UNIT_FS_RDWR = 3,        //以可读写方式打开文件. 上述三种旗标是互斥的, 也就是不可同时使用, 但可与下列的旗标利用OR(|)运算符组合.
    UNIT_FS_CREAT = 0x0100,  //若欲打开的文件不存在则自动建立该文件.
    UNIT_FS_EXCL  = 0x0200,  //如果O_CREAT 也被设置, 此指令会去检查文件是否存在. 文件若不存在则建立该文件, 否则将导致打开文件错误. 此外, 若O_CREAT 与O_EXCL 同时设置, 并且欲打开的文件为符号连接, 则会打开文件失败.
    UNIT_FS_TRUNC = 0x0400,  //若文件存在并且以可写的方式打开时, 此旗标会令文件长度清为0, 而原来存于该文件的资料也会消失.
    UNIT_FS_APPEND = 0x0800, //当读写文件时会从文件尾开始移动, 也就是所写入的数据会以附加的方式加入到文件后面.
} unit_fs_open_flags_t;

typedef enum {
    UNIT_FS_SEEK_SET = 0,   // Seek relative to an absolute position
    UNIT_FS_SEEK_CUR = 1,   // Seek relative to the current file position
    UNIT_FS_SEEK_END = 2,   // Seek relative to the end of the file
} unit_fs_whence_flags_t;

typedef enum {
    UNIT_FS_DIR = 0X04,
    UNIT_FS_REG = 0x08,
} unit_fs_type_t;

typedef struct {
    unsigned int size;
    unit_fs_type_t type;
    char name[128];
    char *name_ptr; // 大于127字节文件名使用malloc分配, wrapper层分配，引擎使用完后释放
} unit_fs_info_t;

typedef enum { 
	ADD = 0, 	//增加文件/文件夹
	DEL, 		//删除文件/文件夹
	UPDATE, 	//修改文件/文件夹
	RENAME 		//重命名文件/文件夹
} unit_fs_change_type_t;

void* realize_unit_fs_open(const char *path, unit_fs_open_flags_t flags);
int realize_unit_fs_close(void *fp);
int realize_unit_fs_read(void *fp, void *buf, unsigned int btr, unsigned int *br);
int realize_unit_fs_write(void *fp, void *buf, unsigned int btw, unsigned int *bw);
int realize_unit_fs_seek(void *fp, unsigned int pos, unit_fs_whence_flags_t whence);
int realize_unit_fs_tell( void *fp, unsigned int *pos_p);
int realize_unit_fs_sync(void* fp);
int realize_unit_fs_eof(void* fp);
void* realize_unit_fs_opendir(const char *path);
int realize_unit_fs_readdir(void *dir_p, unit_fs_info_t *info);
int realize_unit_fs_closedir(void *dir_p);
int realize_unit_fs_mkdir(const char* path);
int realize_unit_fs_stat(const char *path, unit_fs_info_t *info);
int realize_unit_fs_rename(const char* path_old, const char* path_new);
int realize_unit_fs_remove(const char* path);
int realize_unit_fs_mount(const char* p, int is_all_mount);
int realize_unit_fs_unmount(const char* path);
int realize_unit_fs_get_mount_state(char *disk);
int realize_unit_fs_space(const char* path, unit_fs_space_t *space_info);
int realize_unit_fs_format(const char* path);

int realize_unit_fs_init(void);
unit_fs_file_info_t * realize_unit_fs_get_compose_info(const char *path, const char *name);
int realize_unit_fs_check_file_type(const char *path, char *file_path, char *filename);
int realize_unit_fs_read_file(const char *path, unsigned char **buf, unsigned int *len);
int realize_unit_fs_cp_file(const char *src_path, const char *des_path, int mode);
int realize_unit_fs_rm(char *path, unit_fs_dir_opt_t *info);
int realize_unit_fs_remove_dir(char *path, unit_fs_dir_opt_t *info);
int realize_unit_fs_create_dir(unit_fs_dir_opt_t *dir_info);

/* log系统专用fs函数 */
void* realize_unit_fs_open_log(const char *path, unit_fs_open_flags_t flags);
int realize_unit_fs_close_log(void *fp);
int realize_unit_fs_remove_log(const char* path);
int realize_unit_fs_rename_log(const char* path_old, const char* path_new);
int realize_unit_fs_seek_log(void *fp, unsigned int pos, unit_fs_whence_flags_t whence);
int realize_unit_fs_tell_log( void *fp, unsigned int *pos_p);
int realize_unit_fs_write_log(void *fp, void *buf, unsigned int btw, unsigned int *bw);
int realize_unit_fs_sync_log(void* fp);
int realize_unit_fs_read_log(void *fp, void *buf, unsigned int btr, unsigned int *br);
int realize_unit_fs_space_log(const char* path, unit_fs_space_t *space_info);

#endif
