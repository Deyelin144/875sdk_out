
/**
* @file  fs_wrapper.h  
* @brief GUlite_SF SDK FS抽象层封装
* @date  2022-7-14  
* 为了适配不同的操作系统，将SDK所需要的系统函数抽象成单独一层，不同的FS有不同的实现
*/
#ifndef __FS_WRAPPER_H__
#define __FS_WRAPPER_H__

#include "../../../mod_realize/realize_unit_fs/realize_unit_fs.h"

/**
 * @brief 打开文件
 * @param path 文件路径
 * @param fp 打开模式
 * @return 文件句柄
 */
typedef void* (*fs_open_t)(const char *path, unit_fs_open_flags_t flags);

/**
 * @brief 关闭文件
 * @param 文件句柄
 * @return int 0 succ
 */
typedef int (*fs_close_t)(void *fp);

/**
 * @brief 读文件
 * @param fp 文件句柄
 * @param buf 文件内容指针
 * @param btr 需要读取的字节数
 * @param br 实际读到的字节数
 * @return int 0 succ
 */
typedef	int (*fs_read_t)(void *fp, void *buf, unsigned int btr, unsigned int *br);

/**
 * @brief 写文件
 * @param fp 文件句柄
 * @param buf 文件内容指针
 * @param btw 需要写入的字节数
 * @param bw 实际写入的字节数
 * @return int 0 succ
 */
typedef	int (*fs_write_t)(void *fp, void *buf, unsigned int btw, unsigned int *bw);

/**
 * @brief 定位到文件的指定位置
 * @param fp 文件句柄
 * @param pos 想要偏移的位置
 * @param whence 偏移的起始终位置
 * @return int 0 succ
 */
typedef	int (*fs_seek_t)( void *fp, unsigned int pos, unit_fs_whence_flags_t whence);

/**
 * @brief 获取当前文件指针位置
 * @param fp 文件句柄
 * @param pos_p 文件指针位置
 * @return int 0 succ
 */
typedef	int (*fs_tell_t)( void *fp, unsigned int *pos_p);

/**
 * @brief 执行同步操作
 * @param fp 文件句柄
 * @param path_new 新名称
 * @return int 0 succ
 */
typedef	int (*fs_sync_t)(void* fp);

/**
 * @brief 判断是否到文件末尾
 * @param fp 文件句柄
 * @return int 1 文件末尾
 */
typedef int (*fs_eof_t)(void* fp);

/**
 * @brief 打开一个文件夹
 *
 * @param path 文件路径
 * @return 文件夹描述句柄
 */
typedef void* (*fs_opendir_t)(const char *path);

/**
 * @brief 读取文件夹
 * @param dir_p 文件夹描述句柄
 * @param info 获取文件夹的信息
 * @return int 0 succ
 */
typedef int (*fs_readdir_t)(void *dir_p, unit_fs_info_t *info);

/**
 * @brief 关闭文件夹
 * @param dir_p 文件夹描述句柄
 * @return int 0 succ
 */
typedef int (*fs_closedir_t)(void *dir_p);

/**
 * @brief 新建文件夹
 * @param path 路径
 * @param path 权限
 * @return int 0 succ
 */
typedef int (*fs_mkdir_t)(const char* path, int mode);

/**
 * @brief 获取文件或者文件夹的状态以及信息
 * @param path 路径
 * @param info 获取到的信息
 * @return int 0 succ
 */
typedef int (*fs_stat_t)(const char *path, unit_fs_info_t *info);

/**
 * @brief 重命名文件或者文件夹
 * @param path_old 旧名称
 * @param path_new 新名称
 * @return int 0 succ
 */
typedef int (*fs_rename_t)(const char* path_old, const char* path_new);

/**
 * @brief 删除文件或者文件夹
 * @param path 路径
 * @param path_new 新名称
 * @return int 0 succ
 */
typedef int (*fs_remove_t)(const char* path);

/**
 * @brief 挂载文件系统
 * 目前只支持tf卡与lfs-falsh
 * @param path 要挂载的路径（fatfs表示tf卡, lfs表示flash user, lfs_ota标识flash ota）
 * @return int 0 succ
 */
typedef int (*fs_mount_t)(const char *path, int is_all_mount);

/**
 * @brief 卸载文件系统
 * 目前只支持tf卡与lfs-falsh
 * @param path 要卸载的路径（fatfs表示tf卡, lfs表示flash user, lfs_ota标识flash ota）
 * @return int 0 succ
 */
typedef int (*fs_unmount_t)(const char *path);

/**
 * @brief 获取文件系统挂载状态
 * 目前只支持tf卡与lfs-falsh
 * @param void
 * @return int 所有文件系统的挂载状态
 */
typedef int (*fs_get_mount_state_t)(const char *path);

/**
 * @brief 获取文件系统挂载的实际空间
 * 目前只支持tf卡与lfs-falsh
 * @param space_info 实际空间的输出信息
 * @return int 0 succ
 */
typedef int (*fs_space_t)(const char *path, unit_fs_space_t *space_info);

/**
 * @brief 格式化磁盘
 * 目前只支持tf卡与lfs-flash
 * @param 
 * @return int 0 succ
 */
typedef int (*fs_format_t)(const char *path);

/**
 * @brief 删除非空目录
 * @param 
 * @return int 0 succ
 */
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
} fs_wrapper_t;


#endif
