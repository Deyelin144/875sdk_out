
#include "fs_adapter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <statfs.h>
#include "os_adapter/os_adapter.h"
#include "recovery_main.h"

static void *fs_adapter_open_exec(int flags, char *buf_path)
{
	int mode = 0x0666;
	int *fd = NULL;
	fd = os_adapter()->calloc(1, sizeof(int));
	if (NULL == fd) {
		LOG_ERR("malloc error\n");
		goto exit;
	}

	switch(flags) {
		case UNIT_FS_RDONLY:
		    *fd = open(buf_path, O_RDONLY, mode); //可读, 不可写, 必须存在, 可在任意位置读取, 文件指针自由移动"rb"
		    break;
		case UNIT_FS_WRONLY | UNIT_FS_CREAT | UNIT_FS_TRUNC:
		case UNIT_FS_WRONLY | UNIT_FS_TRUNC:
		    *fd = open(buf_path, O_WRONLY | O_CREAT | O_TRUNC, mode); //不可读, 可写, 可以不存在, 若存在则必会擦掉原有内容从头写, 文件指针无效"wb"
		    break;
		case UNIT_FS_WRONLY | UNIT_FS_CREAT | UNIT_FS_APPEND:
		case UNIT_FS_WRONLY | UNIT_FS_APPEND:
		    *fd = open(buf_path, O_WRONLY  | O_APPEND, mode); //不可读, 可写, 可以不存在存在, 必不能修改原有内容, 只能在结尾追加写, 文件指针无效"ab"
			if (*fd < 0) {
				*fd = open(buf_path, O_WRONLY  | O_CREAT | O_APPEND, mode);
			}
		    break;
		case UNIT_FS_RDWR:
		    *fd = open(buf_path, O_RDWR, mode); //可读可写, 必须存在, 可在任意位置读写, 读与写共用同一个指针"rb+"
		    break;
		case UNIT_FS_RDWR | UNIT_FS_CREAT | UNIT_FS_TRUNC:
		case UNIT_FS_RDWR | UNIT_FS_TRUNC:
		    *fd = open(buf_path, O_RDWR | O_CREAT | O_TRUNC, mode); //可读可写, 可以不存在, 必会擦掉原有内容从头写, 文件指针只对读有效 (写操作会将文件指针移动到文件尾)"wb+"
		    break;
		case UNIT_FS_RDWR | UNIT_FS_CREAT | UNIT_FS_APPEND:
		case UNIT_FS_RDWR | UNIT_FS_APPEND:
		    *fd = open(buf_path, O_RDWR | O_APPEND, mode); //可读可写, 可以不存在, 必不能修改原有内容, 只能在结尾追加写, 文件指针只对读有效 (写操作会将文件指针移动到文件尾)"ab+"
			if (*fd < 0) {
				*fd = open(buf_path, O_RDWR |  O_CREAT | O_APPEND, mode);
			}	
		    break;
		default:
		    LOG_ERR("Parameter error, Only the following parameters are supported: %d.\n", flags);
		    LOG_ERR("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",\
		            UNIT_FS_RDONLY, \
		            UNIT_FS_WRONLY | UNIT_FS_CREAT | UNIT_FS_TRUNC,\
		            UNIT_FS_WRONLY | UNIT_FS_TRUNC,\
		            UNIT_FS_WRONLY | UNIT_FS_CREAT | UNIT_FS_APPEND,\
		            UNIT_FS_WRONLY | UNIT_FS_APPEND,\
		            UNIT_FS_RDWR,\
		            UNIT_FS_RDWR | UNIT_FS_CREAT | UNIT_FS_TRUNC,\
		            UNIT_FS_RDWR | UNIT_FS_TRUNC,\
		            UNIT_FS_RDWR | UNIT_FS_CREAT | UNIT_FS_APPEND,\
		            UNIT_FS_RDWR | UNIT_FS_APPEND);
			*fd = -1;
		    break;
		}

		if (*fd < 0) {
			os_adapter()->free(fd);
			fd = NULL;
		}
exit:
		return fd;
}

static  void* fs_adapter_open(const char *path, fs_open_flags_t flags)
{
	void *file = NULL;

	file = fs_adapter_open_exec(flags, (char *)path);

    return file;
}


static int fs_adapter_close(void *fp)
{
	int ret = 0;
	int fd = *(int *)fp;

    ret = close(fd);
	os_adapter()->free(fp);

	return ret;
}

static int fs_adapter_read(void *fp, void *buf, unsigned int btr, unsigned int *br)
{
	int ret = -1;
	int fd = *(int *)fp;

    if (br == NULL) {
		ret = read(fd, buf, btr);
		ret = (ret >= 0) ? 0 : -1;
	} else {
		*br = read(fd, buf, btr);
		if (*br >= 0) {
			ret = 0;
		}
	}

    return (ret == 0) ? 0 : -1;
}


static int fs_adapter_write(void *fp, void *buf, unsigned int btw, unsigned int *bw)
{
	int ret = -1;
	int fd = *(int *)fp;

    if (bw == NULL) {
		ret = write(fd, buf, btw);
		ret = (ret >= 0) ? 0 : -1;
	} else {
		ret = write(fd, buf, btw);
		if (ret >= 0) {
            *bw = ret;
			ret = 0;
		}
	}

    // 同步文件
    if (fsync(fd) != 0) {
		ret = -1;
        // 处理同步失败的情况
       goto exit;
    }
	ret = 0;
exit:
    return ret;
}

static int fs_adapter_seek(void *fp, unsigned int pos, fs_whence_flags_t whence)
{
	int fd = *(int *)fp;
	int ret = -1;

	ret = lseek(fd, pos, whence);

    return (ret >= 0) ? 0 : -1;
}

static int fs_adapter_tell( void *fp, unsigned int *pos_p)
{
	int ret = -1;
	int fd = *(int *)fp;

    ret = *pos_p = lseek(fd, 0, UNIT_FS_SEEK_CUR);

    return (ret >= 0) ? 0 : -1; 
}


static int fs_adapter_sync(void* fp)
{
    int ret = -1;
	int fd = *(int *)fp;

    ret = fsync(fd);

    return (ret == 0) ? 0 : -1; 
}

static int fs_adapter_eof(void* fp)
{
	struct stat info;
	int fd = *(int *)fp;
	int cur_fd = 0, ret = 0;

	fstat(fd, &info);
	cur_fd = lseek(fd, 0, UNIT_FS_SEEK_CUR);//获取当前文件指针位置
	if (cur_fd == info.st_size) {
		ret = 1;
	}

	return ret;
}


static void* fs_adapter_opendir(const char *path)
{
    DIR *dir = NULL;

    dir = opendir(path);

    return (void *)dir;
}

static int fs_adapter_readdir(void *dir_p, fs_info_t *info)
{
	int ret = -1;
    struct dirent *ptr = NULL; //readdir函数的返回值就存放在这个结构体中

	XR_OS_SetErrno(0);
	ptr = readdir(dir_p);
	if (NULL == ptr) {
		if (errno != 0) {
			perror("readdir failed.");  
			goto exit;
		} else {
			ret = 0;
			goto exit;
		}
	}

    if (DT_REG == ptr->d_type) {
        info->type = UNIT_FS_REG;
    } else if (DT_DIR == ptr->d_type) {
        info->type = UNIT_FS_DIR;
    } else {
        info->type = 0;
    }

	if (strlen(ptr->d_name) < 128) {
	    strncpy(info->name, ptr->d_name, sizeof(info->name) - 1);
    } else {
        if (info->name_ptr != NULL) {
            info->name_ptr = os_adapter()->realloc(info->name_ptr, strlen(ptr->d_name) + 1);
        } else {
            info->name_ptr = os_adapter()->calloc(1, strlen(ptr->d_name) + 1);
        }
        if (NULL == info->name_ptr) {
            LOG_ERR("readdir name_ptr malloc fail\n");
            goto exit;
        }
        strncpy(info->name_ptr, ptr->d_name, strlen(ptr->d_name));
    }
	ret = 0;
	
exit:
    return ret;
}

static int fs_adapter_closedir(void *dir_p)
{
    return closedir(dir_p);
}

static int fs_adapter_mkdir(const char* path, int mode)
{
	int ret = -1;

	if (0 == access(path, F_OK)) {
		printf("%s is existence.\n", path);
	} else {
		ret = mkdir(path, 0777);
	}

    return ret;
}

static int fs_adapter_stat(const char *path, fs_info_t *info)
{
	int ret = -1;
	struct stat st;//定义结构体变量，保存所获取的文件属性

	memset(&st, 0, sizeof(struct stat));

	ret = stat(path, &st);
	if (0 != ret) {
		perror("Problem getting information");
		switch (errno) {  
			case ENOENT:  
				LOG_ERR("File %s not found.\n", path);  
				break;  
			case EINVAL:  
                LOG_ERR("Invalid parameter to _stat.\n");  
				break;  
			default:  
				/* Should never be reached. */  
				// LOG_ERR("Unexpected error in stat. ret = %d\n", ret);  
                break;
		}  

		goto exit;
	}
	if ((st.st_mode >> 12) & UNIT_FS_REG) {
		info->type = UNIT_FS_REG;
	} else if ((st.st_mode >> 12) & UNIT_FS_DIR) {
		info->type = UNIT_FS_DIR;
	} else {
		info->type = 0;
	}
	info->size = st.st_size;
	if (strlen(path) < 128) {
        strncpy(info->name, path, sizeof(info->name) - 1);
    } else {
        if (info->name_ptr != NULL) {
            info->name_ptr = os_adapter()->realloc(info->name_ptr, strlen(path) + 1);
        } else {
            info->name_ptr = os_adapter()->calloc(1, strlen(path) + 1);
        }
        if (NULL == info->name_ptr) {
            LOG_ERR("stat name_ptr malloc fail");
            goto exit;
        }
        strncpy(info->name_ptr, path, strlen(path));
    }

	ret = 0;
exit:
    return ret;
}

static int fs_adapter_rename(const char* path_old, const char* path_new)
{
    return rename(path_old, path_new);
}

static int fs_adapter_remove(const char* path)
{
    return remove(path);
}

static int fs_adapter_mount(const char *path, int is_all_mount)
{
	int ret = -1;

	ret = 0;
    return ret;
}

static int fs_adapter_unmount(const char *path)
{
	int ret = -1;

	ret = 0;
	return ret;
}

static int fs_adapter_get_mount_state(const char *path)
{
	return 1;
}

static int fs_adapter_space(const char* path, fs_space_info_t *space_info)
{
	int ret = -1;

    struct statfs fsinfo = {0};
    ret = statfs(path, &fsinfo);
    if (ret != 0) {
        LOG_ERR("statfs error");
        goto exit;
    }
    space_info->free_space_byte = fsinfo.f_bfree * fsinfo.f_bsize;
    space_info->total_space_byte = fsinfo.f_blocks * fsinfo.f_bsize;

	ret = 0;
exit:
	return ret;
}

static int fs_adapter_format(const char *path)
{
	int ret = -1;

	ret = 0;

	return ret;
}

static int fs_adapter_rmdir(const char *path)
{
	int ret = -1;

	ret = rmdir(path);

	return ret;
}

static fs_adapter_t s_fs_adapter;

fs_adapter_t *fs_adapter()
{
	s_fs_adapter.fs_open = fs_adapter_open;
	s_fs_adapter.fs_close = fs_adapter_close;
	s_fs_adapter.fs_read = fs_adapter_read;
	s_fs_adapter.fs_write = fs_adapter_write;
	s_fs_adapter.fs_sync = fs_adapter_sync;
	s_fs_adapter.fs_seek = fs_adapter_seek;
	s_fs_adapter.fs_tell = fs_adapter_tell;
	s_fs_adapter.fs_eof = fs_adapter_eof;
	s_fs_adapter.opendir = fs_adapter_opendir;
	s_fs_adapter.readdir = fs_adapter_readdir;
	s_fs_adapter.closedir = fs_adapter_closedir;
	s_fs_adapter.mkdir = fs_adapter_mkdir;
	s_fs_adapter.rename = fs_adapter_rename;
	s_fs_adapter.stat = fs_adapter_stat;
	s_fs_adapter.remove = fs_adapter_remove;
	s_fs_adapter.mount = fs_adapter_mount;
	s_fs_adapter.unmount = fs_adapter_unmount;
	s_fs_adapter.get_mount_state = fs_adapter_get_mount_state;
	s_fs_adapter.space = fs_adapter_space;
	s_fs_adapter.format = fs_adapter_format;
	s_fs_adapter.rmdir = fs_adapter_rmdir;

	return &s_fs_adapter;
}
