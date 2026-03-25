
#include "upgrade_app.h"
#include "fs_adapter/fs_adapter.h"
#include "os_adapter/os_adapter.h"
#include "recovery_main.h"
#include "ota_msg.h"
// #include "drive/drv_battery/dev_battery.h"

#define MAX_CACHE_SIZE (10 * 1024 * 1024)
static int s_process = 0;

typedef int (*init_cb_t)(upgrade_handle_t *hdl, upgrade_method_t method, char *url);
typedef int (*get_cb_t)(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag);

int app_local_init_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *url)
{
    hdl->app_handle.fp = fs_adapter()->fs_open(url, UNIT_FS_RDONLY);
    if (!hdl->app_handle.fp) {
        LOG_ERR("%s open fail\n", url ? url : "null");
        upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_FS_OPEN);
        return -1;
    }
    return 0;
}

int app_cloud_init_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *url)
{
    if (!hdl->app_handle.http_param) {
        hdl->app_handle.http_param = os_adapter()->calloc(1, sizeof(HTTPParameters));
        if (!hdl->app_handle.http_param) {
            LOG_ERR("http malloc fail\n");
            upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MALLOC_FAIL);
            return -1;
        }
    }

    memset(hdl->app_handle.http_param, 0, sizeof(HTTPParameters));
    strncpy(hdl->app_handle.http_param->Uri, url, sizeof(hdl->app_handle.http_param->Uri) - 1);
    hdl->app_handle.http_param->nTimeout = 10;

    return 0;
}

int app_local_get_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag)
{
    int ret = -1;

    ret = fs_adapter()->fs_read(hdl->app_handle.fp, buffer, buff_size, (unsigned int *)recv_size);
    if (ret == 0 && *recv_size < buff_size) {
        *eof_flag = 1;
        fs_adapter()->fs_close(hdl->app_handle.fp);
        hdl->app_handle.fp = NULL;
    }
    return ret;
}

int app_cloud_get_cb(upgrade_handle_t *hdl, upgrade_method_t method, char *buffer, int buff_size, int *recv_size, char *eof_flag)
{
    int ret = -1;

    ret = HTTPC_get(hdl->app_handle.http_param, buffer,buff_size, (INT32 *)recv_size);
    if (ret == HTTP_CLIENT_SUCCESS) {
        *eof_flag = 0;
        ret = 0;
    } else if (ret == HTTP_CLIENT_EOS) {
        *eof_flag = 1;
        ret = 0;
    } else {
        LOG_ERR("app http get fail ret = %d\n", ret);
        upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_HTTP_GET);
        ret = -1;
    }
    return ret;
}

static int app_recursive_create_dir(char *path)
{
    int ret = -1;
    char *p = NULL;
    char *temp = NULL;
    char dir[263] = {0};
    fs_info_t f_info;

    temp = path;
    p = path;
    p = strstr(temp, "/");
    if (p != NULL) {
        temp = p + 1;
    }
    while (p != NULL) {
        if (NULL != (p = strstr(temp, "/"))) {
            memset(&f_info, 0, sizeof(fs_info_t));
            memset(dir, 0, 256);
            strncpy(dir, path, p - path);
            if (fs_adapter()->stat(dir, &f_info) != 0) {
                if (fs_adapter()->mkdir(dir, 0) != 0 ) {
                    LOG_ERR("mkdir error.\n");
                    goto exit;
                }
            }
            temp = p + 1;
        }
    }
    ret = 0;

exit:
    return ret;
}

static int app_upgrade_write_to_flash(char *buffer, int len, char *path)
{
    void *fp = NULL;
    int ret = -1;
    unsigned int bw = 0;
    const int write_len_once = 8192;
    upgrade_handle_t *hdl = upgrade_logic_get_handle();

    app_recursive_create_dir(path);
    fp = fs_adapter()->fs_open(path, UNIT_FS_WRONLY | UNIT_FS_CREAT | UNIT_FS_TRUNC);
    if (!fp) {
        LOG_ERR("open %s fail\n", path);
        upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_FS_OPEN);
        goto exit;
    }

    // 防止flash剩余空间不够，直接sync清空原文件
    fs_adapter()->fs_sync(fp);

    while (len > 0) {
        int write_len = len > write_len_once ? write_len_once : len;
        ret = fs_adapter()->fs_write(fp, buffer, write_len, &bw);
        if (ret != 0 || write_len != bw) {
            LOG_ERR("write %s fail\n", path ? path : "null");
            upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_FS_WRITE);
            goto exit;
        }
        buffer += write_len;
        len -= write_len;
        if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
            s_process += write_len;
            upgrade_logic_get_cb()->process(UPGRADE_TYPE_APP, hdl->method, hdl, ((long long)s_process * 100) / (hdl->app_handle.file_size_all << 1));
        }
    }
    ret = 0;
exit:
    if (fp) {
        fs_adapter()->fs_close(fp);
    }
    return ret;
}
int app_upgrade_flush_to_flash(upgrade_handle_t *hdl)
{
    int ret = -1;
    char url[263] = {0};
    unsigned char md5[MD5_LEN * 2 + 1] = {0};

    for (int i = 0; i < hdl->app_handle.file_num; i++) {
        if (hdl->app_handle.file_info[i].data) {
            memset(url, 0, sizeof(url));
            sprintf(url, "%s/%s", APP_DIR, hdl->app_handle.file_info[i].name);
            printf("write to flash: %s\n", url);
            hdl->app_handle.upgrade_status = UPGRADE_FAIL_REPLACED;
            upgrade_logic_set_replace(hdl, 1);
            ret = app_upgrade_write_to_flash(hdl->app_handle.file_info[i].data, hdl->app_handle.file_info[i].size, url);
            if (ret != 0) {
                goto exit;
            }
            printf("read after write\n");
            ret = md5_adapter_get_file_md5(url, md5);
            if (strcmp((const char *)md5, hdl->app_handle.file_info[i].md5) != 0) {
                LOG_ERR("md5 not equal.\n");
                printf("write md5: %s\n", hdl->app_handle.file_info[i].md5);
                printf("read  md5: %s\n", md5);
                upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MD5_VERIFY);
                ret = -1;
                goto exit;
            }
            os_adapter()->free(hdl->app_handle.file_info[i].data);
            hdl->app_handle.file_info[i].data = NULL;
            hdl->app_handle.cache_size -= hdl->app_handle.file_info[i].size;
        }
    }
    ret = 0;
exit:
    return ret;
}

static int app_upgrade_process(upgrade_handle_t *hdl, upgrade_method_t method, init_cb_t init_cb, get_cb_t get_cb)
{
    int ret = -1;
    const int write_size_once = 8192;
    int recv_size = 0;
    char eof_flag = 0;
    void *md5_hdl = NULL;
    unsigned char md5[MD5_LEN * 2 + 1] = {0};
    char url[263] = {0};

    if (upgrade_logic_pre_check(hdl, UPGRADE_TYPE_APP) != 0) {
        goto exit;
    }

    s_process = 0;
    if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->start) {
        upgrade_logic_get_cb()->start(UPGRADE_TYPE_APP, method, hdl);
    }

    int writen_len_all = 0;
    for (int i = 0; i < hdl->app_handle.file_num; i++) {
        printf("get app file %s %d\n", hdl->app_handle.file_info[i].name, hdl->app_handle.file_info[i].size);
        if (hdl->app_handle.file_info[i].file_change == 0) {
            printf("file not change\n");
            s_process += (hdl->app_handle.file_info[i].size << 1);
            upgrade_logic_get_cb()->process(UPGRADE_TYPE_APP, method, hdl, ((long long)s_process * 100) / (hdl->app_handle.file_size_all << 1));
            continue;
        }

        memset(url, 0, sizeof(url));
        if (hdl->method == UPGRADE_METHOD_LOCAL) {
            snprintf(url, sizeof(url), "%s/%s", hdl->app_handle.file_info[i].url, hdl->app_handle.file_info[i].name);
        } else if (hdl->method == UPGRADE_METHOD_CLOUD) {
            strcpy(url, hdl->app_handle.file_info[i].url);
        }
        // printf("url is %s\n", url);
        if (init_cb && init_cb(hdl, method, url) != 0) {
            goto exit;
        }

        int size = hdl->app_handle.file_info[i].size;
        if (hdl->max_file_buffer) {
            hdl->app_handle.file_info[i].data = hdl->max_file_buffer;
            hdl->max_file_buffer = NULL;
        }
        if (hdl->app_handle.cache_size + size < MAX_CACHE_SIZE) {
            hdl->app_handle.file_info[i].data = hdl->app_handle.file_info[i].data ?
                hdl->app_handle.file_info[i].data : os_adapter()->calloc(1, size);
            if (!hdl->app_handle.file_info[i].data) {
                LOG_ERR("data malloc fail, flush\n");
                ret = app_upgrade_flush_to_flash(hdl);
                if (ret != 0) {
                    printf("flush fail\n");
                    goto exit;
                }
                hdl->app_handle.file_info[i].data = os_adapter()->calloc(1, size);
                if (!hdl->app_handle.file_info[i].data) {
                    LOG_ERR("data malloc fail again:%d\n", size);
                    upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MALLOC_FAIL);
                    ret = -1;
                    goto exit;
                }
            }
        } else {
            printf("cache full, flush\n");
            ret = app_upgrade_flush_to_flash(hdl);
            if (ret != 0) {
                printf("flush fail\n");
                goto exit;
            }

            hdl->app_handle.file_info[i].data = hdl->app_handle.file_info[i].data ?
                hdl->app_handle.file_info[i].data : os_adapter()->calloc(1, size);
            if (!hdl->app_handle.file_info[i].data) {
                LOG_ERR("data malloc fail again:%d\n", size);
                upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MALLOC_FAIL);
                ret = -1;
                goto exit;
            }
        }

        ret = -1;
        hdl->app_handle.cache_size += size;
        // int writen_size = 0;
        char *buffer_tmp = hdl->app_handle.file_info[i].data;
        md5_hdl = md5_adapter_init();
        if (!md5_hdl) {
            LOG_ERR("md5 init fail\n");
            upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MD5_INIT);
            goto exit;
        }

        while (size > 0) {
            int write_size = size > write_size_once ? write_size_once : size;
            ret = get_cb(hdl, method, buffer_tmp, write_size, &recv_size, &eof_flag);
            if (ret != 0) {
                goto exit;
            }
            ret = md5_adapter_update(md5_hdl, buffer_tmp, recv_size);
            if (ret != 0) {
                LOG_ERR("md5 update fail\n");
                upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MD5_UPDATE);
                goto exit;
            }
            size -= recv_size;
            // writen_size += recv_size;
            buffer_tmp += recv_size;
            writen_len_all += recv_size;

            if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->process) {
                s_process += recv_size;
                upgrade_logic_get_cb()->process(UPGRADE_TYPE_APP, method, hdl, ((long long)s_process * 100) / (hdl->app_handle.file_size_all << 1));
            }

            if (eof_flag) {
                break;
            } else {
                if (size < 0) {
                    LOG_ERR("app download size error\n");
                    upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_APP_FILE_OVERFLOW);
                    ret = -1;
                    goto exit;
                }
            }
        }

        ret = -1;
        memset(md5, 0, sizeof(md5));
        md5_adapter_finish(&md5_hdl, md5);
        md5_hdl = NULL;
        if (hdl->app_handle.fp) {
            fs_adapter()->fs_close(hdl->app_handle.fp);
            hdl->app_handle.fp = NULL;
        }

        if (strcmp((const char *)md5, hdl->app_handle.file_info[i].md5) != 0) {
            LOG_ERR("app md5 check fail %s\n", hdl->app_handle.file_info[i].name ? hdl->app_handle.file_info[i].name : "null");
            printf("file  md5:%s\n", hdl->app_handle.file_info[i].md5);
            printf("cacul md5:%s\n",md5);
            upgrade_set_errno(UPGRADE_TYPE_APP, UPGRADE_ERR_MD5_VERIFY);
            goto exit;
        }
    }

    ret = app_upgrade_flush_to_flash(hdl);
    if (ret != 0) {
        LOG_ERR("app upgrade flush to flash fail\n");
        goto exit;
    }
    ret = 0;
    hdl->app_handle.upgrade_status = UPGRADE_SUCC;
    if (hdl->fw_handle.need_upgrade == 0 || hdl->fw_handle.upgrade_status == UPGRADE_SUCC) {
        upgrade_logic_set_replace(hdl, 0);
    }
    if (upgrade_logic_get_cb() && upgrade_logic_get_cb()->finish) {
        upgrade_logic_get_cb()->finish(UPGRADE_TYPE_APP, method, hdl);
    }
exit:
    if (md5_hdl) {
        md5_adapter_finish(&md5_hdl, md5);
    }
    for (int i = 0; i < hdl->app_handle.file_num; i++) {
        if (hdl->app_handle.file_info[i].data) {
            os_adapter()->free(hdl->app_handle.file_info[i].data);
            hdl->app_handle.file_info[i].data = NULL;
        }
    }
    hdl->app_handle.cache_size = 0;
    if (ret != 0 && upgrade_logic_get_cb() && upgrade_logic_get_cb()->error) {
        upgrade_logic_get_cb()->error(UPGRADE_TYPE_APP, method, hdl);
    }
    return ret;
}

static int app_upgrade_start(upgrade_handle_t *hdl)
{
    int ret = -1;

    if (hdl->method == UPGRADE_METHOD_LOCAL) {
        ret = app_upgrade_process(hdl, hdl->method, app_local_init_cb, app_local_get_cb);
        if (hdl->app_handle.fp) {
            fs_adapter()->fs_close(hdl->app_handle.fp);
            hdl->app_handle.fp = NULL;
        }
    } else if (hdl->method == UPGRADE_METHOD_CLOUD) {
        ret = app_upgrade_process(hdl, hdl->method, app_cloud_init_cb, app_cloud_get_cb);
        os_adapter()->free(hdl->app_handle.http_param);
        hdl->app_handle.http_param = NULL;
    }
    printf("app upgrade end\n");
    return ret;
}

int upgrade_logic_app_start(void)
{
    int ret = -1;

    if (!upgrade_logic_get_handle()->app_handle.need_upgrade ||
        upgrade_logic_get_handle()->app_handle.upgrade_status == UPGRADE_SUCC) {
        printf("app no need to upgrade\n");
        ret = 0;
        goto exit;
    }

    ret = app_upgrade_start(upgrade_logic_get_handle());
    if (ret != 0) {
        goto exit;
    }

    ret = 0;
exit:
    return ret;
}