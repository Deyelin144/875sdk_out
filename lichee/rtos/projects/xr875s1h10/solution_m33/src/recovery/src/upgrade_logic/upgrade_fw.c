
#include "upgrade_fw.h"
#include "fs_adapter/fs_adapter.h"
#include "os_adapter/os_adapter.h"
#include "recovery_main.h"
#include "ota_msg.h"

// 包含了lwip的头文件，以下几个是lwip的宏
#ifdef open
#undef open
#endif

#ifdef write
#undef write
#endif

#ifdef read
#undef read
#endif

#ifdef close
#undef colse
#endif

#include <fcntl.h>
#include <unistd.h>

typedef int (*init_cb_t)(upgrade_handle_t *hdl, upgrade_method_t method, char *url);
typedef int (*get_cb_t)(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag);

int fw_local_init_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *url)
{
    hdl->fw_handle.fp = fs_adapter()->fs_open(url, UNIT_FS_RDONLY);
    if (!hdl->fw_handle.fp) {
        LOG_ERR("%s open fail\n", url ? url : "null");
        upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_OPEN);
        return -1;
    }
    return 0;
}
int fw_cloud_init_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *url)
{
    if (!hdl->fw_handle.http_param) {
        hdl->fw_handle.http_param = os_adapter()->calloc(1, sizeof(HTTPParameters));
        if (!hdl->fw_handle.http_param) {
            LOG_ERR("http malloc fail\n");
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MALLOC_FAIL);
            return -1;
        }
    }

    memset(hdl->fw_handle.http_param, 0, sizeof(HTTPParameters));
    strncpy(hdl->fw_handle.http_param->Uri, url, sizeof(hdl->fw_handle.http_param->Uri) - 1);
    hdl->fw_handle.http_param->nTimeout = 10;

    return 0;
}

int fw_local_get_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag)
{
    int ret = -1;

    ret = fs_adapter()->fs_read(hdl->fw_handle.fp, buffer, buff_size, (unsigned int *)recv_size);
    if (ret != 0) {
        LOG_ERR("read fail\n");
        upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_READ);
    } else if (ret == 0 && *recv_size < buff_size) {
        *eof_flag = 1;
        fs_adapter()->fs_close(hdl->fw_handle.fp);
        hdl->fw_handle.fp = NULL;
    }
    return ret;
}

int fw_cloud_get_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag)
{
    int ret = -1;

    ret = HTTPC_get(hdl->fw_handle.http_param, buffer,buff_size, (INT32 *)recv_size);
    if (ret == HTTP_CLIENT_SUCCESS) {
        *eof_flag = 0;
        ret = 0;
    } else if (ret == HTTP_CLIENT_EOS) {
        *eof_flag = 1;
        os_adapter()->free(hdl->fw_handle.http_param);
        hdl->fw_handle.http_param = NULL;
        ret = 0;
    } else {
        os_adapter()->free(hdl->fw_handle.http_param);
        hdl->fw_handle.http_param = NULL;
        LOG_ERR("fw http get fail ret = %d\n", ret);
        upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_HTTP_GET);
        ret = -1;
    }
    return ret;
}

static int fw_upgrade_process(upgrade_handle_t *hdl, upgrade_method_t method, init_cb_t init_cb, get_cb_t get_cb)
{
    int recv_size = 0;
    char eof_flag = 0;
    unsigned char *image_buffer = NULL;
    int image_max_size = hdl->max_file_size;
    int ret = -1;
    const int write_len_once = 8192;
    void *md5_hdl = NULL;
    char fw_md5[MD5_LEN * 2 + 1] = {0};
    int dev_fp = -1;
    int progress = 0;
    int progress_per = 0;

    if (upgrade_logic_pre_check(hdl, UPGRADE_TYPE_FW) != 0) {
        goto exit;
    }

    if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->start) {
        upgrade_logic_get_cb()->start(UPGRADE_TYPE_FW, method, hdl);
    }
    printf("fw file_size_all = %d\n", hdl->fw_handle.file_size_all);
    image_buffer = (unsigned char *)hdl->max_file_buffer;
    for (int i = 0; i < hdl->fw_handle.fw_num; i++) {
        if (strlen(hdl->fw_handle.fw_info[i].fex_name) == 0) {
            continue;
        }
        printf("fex_name: %s\n", hdl->fw_handle.fw_info[i].fex_name);
        recv_size = 0;
        eof_flag = 0;
        if (init_cb && init_cb(hdl, method, hdl->fw_handle.fw_info[i].url) != 0) {
            goto exit;
        }
        memset(image_buffer, 0, image_max_size);
        int writen_len = 0;
        md5_hdl = md5_adapter_init();
        if (!md5_hdl) {
            LOG_ERR("md5 init fail\n");
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_INIT);
            goto exit;
        }
        
        printf("image download\n");
        int cnt = 0;
        unsigned char *image_buffer_tmp = image_buffer;
        int image_max_size_tmp = image_max_size;
        while (image_max_size_tmp > 0) {
            ret = get_cb(hdl, method, (char *)image_buffer_tmp, write_len_once , &recv_size, &eof_flag);
            if (ret != 0) {
                goto exit;
            }
            image_max_size_tmp -= recv_size;
            writen_len += recv_size;
            progress += recv_size;
            if (cnt++ % 20 == 0) {
                printf("get len : %d\n", writen_len);
            }
            if (recv_size > 0) {
                ret = md5_adapter_update(md5_hdl, (char *)image_buffer_tmp, recv_size);
                if (ret != 0) {
                    LOG_ERR("md5 update fail\n");
                    upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_UPDATE);
                    goto exit;
                }
            }
            if (eof_flag) {
                printf("get len : %d\n", writen_len);
                printf("end\n");
                break;
            } else {
                if (image_max_size_tmp <= 0) {
                    LOG_ERR("image overflow\n");
                    upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_IMAGE_OVERFLOW);
                    ret = -1;
                    goto exit;
                }
            }
            image_buffer_tmp += recv_size;
            if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
                progress_per = ((long long)progress * 100) / (hdl->fw_handle.file_size_all << 1);
                if (progress_per < 1) {      //mcu文件占比太小，耗时长,进度条显示不友好，这里特殊处理一下
                    progress_per = 1;
                }
                upgrade_logic_get_cb()->process(UPGRADE_TYPE_FW, method, hdl, progress_per);
            }
        }
        md5_adapter_finish(&md5_hdl, (unsigned char *)fw_md5);
        if (hdl->fw_handle.fp) {
            fs_adapter()->fs_close(hdl->fw_handle.fp);
            hdl->fw_handle.fp = NULL;
        }
        ret = -1;
        if (strcmp(fw_md5, hdl->fw_handle.fw_info[i].md5) != 0) {
            LOG_ERR("md5 not match\n");
            printf("fw    md5 : %s\n", hdl->fw_handle.fw_info[i].md5);
            printf("cacul md5 : %s\n", fw_md5);
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_VERIFY);
            goto exit;
        }

        // 这里已经破坏了fash数据
        printf("write to flash\n");
        hdl->fw_handle.upgrade_status = UPGRADE_FAIL_REPLACED;
        hdl->fw_handle.fw_info[i].upgrade_status = UPGRADE_FAIL_REPLACED;
        upgrade_logic_set_replace(hdl, 1);

        //mcu升级 特殊处理
        if (strstr(hdl->fw_handle.fw_info[i].fex_name,"mcu")) {
            printf("[ %s ][ %d ]   mcu upgrade\n",__FUNCTION__,__LINE__);
            //等待mcu内部升级
            int mcu_offset = 0;
            int progress_temp = progress;
            int upgrade_stete = 0;
            mcu_firmware_update_reset_state();
            ret = mcu_firmware_update_load_info(image_buffer, writen_len );
            if (ret != 0) {
                LOG_ERR("mcu upgrade fail\n");
                upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_WRITE);
                goto exit;
            }
            ret = -1;
            while(1){
                mcu_offset = mcu_firmware_update_get_offset();
                upgrade_stete = mcu_firmware_update_get_state();
                if (mcu_offset < 0 || upgrade_stete < 0) {
                    LOG_ERR("mcu upgrade fail\n");
                    mcu_firmware_update_reset_state();
                    upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_WRITE);
                    goto exit;
                }

                progress = progress_temp + mcu_offset;
                if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
                    progress_per = ((long long)progress * 100) / (hdl->fw_handle.file_size_all << 1);
                    if (progress_per < 1) {      //mcu文件占比太小，耗时长,进度条显示不友好，这里特殊处理一下
                        progress_per = 1;
                    }
                    upgrade_logic_get_cb()->process(UPGRADE_TYPE_FW, method, hdl, progress_per);
                }

                if (0 == upgrade_stete) {
                    hdl->fw_handle.fw_info[i].upgrade_status = UPGRADE_SUCC;
                    printf("mcu upgrade succ\n");
                    break;
                }
                
                XR_OS_MSleep(100);
            }

            continue;
        }

        printf("start open %s\n", hdl->fw_handle.fw_info[i].part_name);
        dev_fp = open(hdl->fw_handle.fw_info[i].part_name, O_RDWR | O_CREAT | O_TRUNC);
        if (dev_fp < 0) {
            LOG_ERR("open fail");
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_OPEN);
            goto exit;
        }

        cnt = 0;
        unsigned int writen_len_tmp = writen_len;
        image_buffer_tmp = image_buffer;
        while (writen_len_tmp > 0) {
            if (cnt++ % 20 == 0) {
                printf("write len : %d\n", writen_len_tmp);
            }
            int write_len = writen_len_tmp > write_len_once ? write_len_once : writen_len_tmp;
            int writen = write(dev_fp, image_buffer_tmp, write_len);
            if (writen != write_len) {
                LOG_ERR("write fail %d %d", writen, write_len);
                upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_WRITE);
                goto exit;
            }
            writen_len_tmp -= write_len;
            image_buffer_tmp += write_len;
            progress += write_len;
            if (writen_len_tmp <= 0) {
                printf("write len : %d\n", writen_len_tmp);
            }
            if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
                progress_per = ((long long)progress * 100) / (hdl->fw_handle.file_size_all << 1);
                if (progress_per < 1) {      //mcu文件占比太小，耗时长,进度条显示不友好，这里特殊处理一下
                    progress_per = 1;
                }
                upgrade_logic_get_cb()->process(UPGRADE_TYPE_FW, method, hdl, progress_per);
            }
        }

        // 读出来校验
        printf("read after write\n");
        ret = lseek(dev_fp, 0, SEEK_SET);
        if (ret == -1) {
            LOG_ERR("lseek fail");
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_SEEK);
            goto exit;
        }
        ret = -1;

        md5_hdl = md5_adapter_init();
        if (!md5_hdl) {
            LOG_ERR("md5 init fail\n");
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_INIT);
            goto exit;
        }
        memset(image_buffer, 0, image_max_size);
        image_buffer_tmp = image_buffer;
        writen_len_tmp = writen_len;
        while (writen_len_tmp > 0) {
            int read_len = writen_len_tmp > write_len_once ? write_len_once : writen_len_tmp;
            int reads = read(dev_fp, image_buffer_tmp, read_len);
            if (reads < 0) {
                LOG_ERR("read file error\n");
                upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_FS_READ);
                goto exit;
            }
            if (reads == 0) {
                break;
            }
            ret = md5_adapter_update(md5_hdl, (char  *)image_buffer_tmp, read_len);
            if (ret != 0) {
                LOG_ERR("md5 update fail\n");
                upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_UPDATE);
                goto exit;
            }
    
            writen_len_tmp -= reads;
            image_buffer_tmp += reads;
        }
        close(dev_fp);

        ret = -1;
        char md5_read[MD5_LEN * 2 + 1] = {0};
        md5_adapter_finish(&md5_hdl, (unsigned char *)md5_read);
        if (strcmp(md5_read, fw_md5) != 0) {
            LOG_ERR("verify after write error\n");
            printf("cacul md5 : %s\n", fw_md5);
            printf("read  md5 : %s\n", md5_read);
            upgrade_set_errno(UPGRADE_TYPE_FW, UPGRADE_ERR_MD5_VERIFY);
            goto exit;
        }
        hdl->fw_handle.fw_info[i].upgrade_status = UPGRADE_SUCC;
    }

    if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
        upgrade_logic_get_cb()->process(UPGRADE_TYPE_FW, method, hdl, 100);
    }

    ret = 0;
    hdl->fw_handle.upgrade_status = UPGRADE_SUCC;
    if (hdl->fw_handle.need_upgrade == 0 || hdl->fw_handle.upgrade_status == UPGRADE_SUCC) {
        upgrade_logic_set_replace(hdl, 0);
    }
    if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->finish) {
        upgrade_logic_get_cb()->finish(UPGRADE_TYPE_FW, method, hdl);
    }
    image_buffer = NULL;
exit:
    if (md5_hdl) {
        md5_adapter_finish(&md5_hdl, (unsigned char *)fw_md5);
    }

    if (image_buffer) {
        memset(image_buffer, 0, image_max_size);
    }
    if (dev_fp >= 0) {
        close(dev_fp);
    }
    if (ret != 0 && upgrade_logic_get_cb() && upgrade_logic_get_cb()->error) {
        upgrade_logic_get_cb()->error(UPGRADE_TYPE_FW, method, hdl);
    }
    return ret;
}

static int fw_upgrade_start(upgrade_handle_t *hdl)
{
    int ret = -1;

    if (hdl->method == UPGRADE_METHOD_LOCAL) {
        ret = fw_upgrade_process(hdl, hdl->method, fw_local_init_cb, fw_local_get_cb);
        if (hdl->fw_handle.fp) {
            fs_adapter()->fs_close(hdl->fw_handle.fp);
            hdl->fw_handle.fp = NULL;
        }
    } else if (hdl->method == UPGRADE_METHOD_CLOUD) {
        ret = fw_upgrade_process(hdl, hdl->method, fw_cloud_init_cb, fw_cloud_get_cb);
        os_adapter()->free(hdl->fw_handle.http_param);
        hdl->fw_handle.http_param = NULL;
    }
    printf("fw upgrade end\n");
    return ret;
}

int upgrade_logic_fw_start(void)
{
    int ret = -1;

    if (!upgrade_logic_get_handle()->fw_handle.need_upgrade ||
        upgrade_logic_get_handle()->fw_handle.upgrade_status == UPGRADE_SUCC) {
        printf("fw no need to upgrade\n");
        ret = 0;
        goto exit;
    }

    ret = fw_upgrade_start(upgrade_logic_get_handle());
    if (ret != 0) {
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}