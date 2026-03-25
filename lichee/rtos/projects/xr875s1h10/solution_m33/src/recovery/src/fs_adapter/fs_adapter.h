
#ifndef __FS_ADAPTER_H__
#define __FS_ADAPTER_H__

typedef struct {
	unsigned int free_space_byte;		//空闲空间
	unsigned int total_space_byte;		//总空间
} fs_space_info_t;

typedef struct {
	char disk[20];
	void *file;
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
} fs_open_flags_t;

typedef enum {
    UNIT_FS_SEEK_SET = 0,   // Seek relative to an absolute position
    UNIT_FS_SEEK_CUR = 1,   // Seek relative to the current file position
    UNIT_FS_SEEK_END = 2,   // Seek relative to the end of the file
} fs_whence_flags_t;

typedef enum {
    UNIT_FS_DIR = 0X04,
    UNIT_FS_REG = 0x08,
} fs_type_t;

typedef struct {
    unsigned int size;
    fs_type_t type;
    char name[128];
    char *name_ptr;
} fs_info_t;

typedef void* (*fs_open_t)(const char *path, fs_open_flags_t flags);
typedef int (*fs_close_t)(void *fp);
typedef	int (*fs_read_t)(void *fp, void *buf, unsigned int btr, unsigned int *br);
typedef	int (*fs_write_t)(void *fp, void *buf, unsigned int btw, unsigned int *bw);
typedef	int (*fs_seek_t)( void *fp, unsigned int pos, fs_whence_flags_t whence);
typedef	int (*fs_tell_t)( void *fp, unsigned int *pos_p);
typedef	int (*fs_sync_t)(void* fp);
typedef int (*fs_eof_t)(void* fp);
typedef void* (*fs_opendir_t)(const char *path);
typedef int (*fs_readdir_t)(void *dir_p, fs_info_t *info);
typedef int (*fs_closedir_t)(void *dir_p);
typedef int (*fs_mkdir_t)(const char* path, int mode);
typedef int (*fs_stat_t)(const char *path, fs_info_t *info);
typedef int (*fs_rename_t)(const char* path_old, const char* path_new);
typedef int (*fs_remove_t)(const char* path);
typedef int (*fs_mount_t)(const char *path, int is_all_mount);
typedef int (*fs_unmount_t)(const char *path);
typedef int (*fs_get_mount_state_t)(const char *path);
typedef int (*fs_space_t)(const char *path, fs_space_info_t *space_info);
typedef int (*fs_format_t)(const char *path);
typedef int (*fs_rmdir_t)(const char *path);

typedef struct {
	fs_open_t fs_open;
	fs_close_t fs_close;
	fs_read_t fs_read;
	fs_write_t fs_write;
	fs_seek_t fs_seek;
	fs_tell_t fs_tell;
	fs_sync_t fs_sync;
	fs_eof_t fs_eof;
	fs_opendir_t opendir;
	fs_readdir_t readdir;
	fs_closedir_t closedir;
	fs_mkdir_t mkdir;
	fs_stat_t stat;
	fs_rename_t rename;
	fs_remove_t remove;
	fs_mount_t mount;
	fs_unmount_t unmount;
	fs_get_mount_state_t get_mount_state;
	fs_space_t space;
	fs_format_t format;
	fs_rmdir_t rmdir;
} fs_adapter_t;

fs_adapter_t *fs_adapter();

#endif
