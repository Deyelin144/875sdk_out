
#include "upgrade_logic.h"
#include <stdlib.h>
#include <string.h>
#include <sunxi_hal_power.h>
#include <console.h>
#include <statfs.h>
#include "fs_adapter/fs_adapter.h"
#include "cjson/cJSON.h"
#include "md5_adapter/md5_adapter.h"
#include "wifi_adapter/wifi_adapter.h"
#include "os_adapter/os_adapter.h"
#include "recovery_main.h"
#include "ota_msg.h"
#include "blkpart.h"
#include "drivers/battery/dev_battery.h"
#include "drivers/led/dev_led.h"
#include "native_app_ota.h"

extern int cmd_reboot(int argc, char **argv);
extern int cmd_fw_setenv(int argc, char ** argv);
extern int get_md5sum(char *path, unsigned int file_size, unsigned char decrypt[16]);

#define UPGRADE_SPACE_BUFFER_BLKCNT 8 // 预留32k，若空间小于32K，也认为空间不够
#define BATTERY_DIFF 10
// 若发现空间不够，则删除配置的一些文件，腾出空间升级
typedef struct {
    fs_type_t type;
    char *path;
} clean_info_t;
static clean_info_t s_clean_info[] = {
    {UNIT_FS_DIR, APP_DIR"/img_cache"}
};

static upgrade_handle_t s_upgrade_hdl = {0};
static upgrade_cb_t s_upgrade_cb = {0};

static ota_note_t s_ota_note_default = {
    "Upgrading in preparation...",
    "Upgrading, please wait...",
    "Do not perform any operation during the upgrade",
    "Upgrade successful. Restarting\n\nPlease be patient...",
    "Upgrade failed please try again later",
    "Cancel",
    "retry",
    "Network disconnected",
    "Press #00F5EA OK# to start networking",
    "Loading...",
    "Press",
    "to fresh",
    "Loading...",
    "Refresh successfully",
    "enter the password",
    "Connecting...",
    "Connection successful",
    "Network connection failure",
    "Incorrect password or too far away",
    "Try again",
    "Connected",
    "Connected",
    "Saved Wi-Fi",
    "Delete network",
    "delete this network?",
    "delete",
    "Only supports 2.4GHz network",
    "Wi-Fi",
    "Low battery. Keep battery >60%.",
};

upgrade_handle_t *upgrade_logic_get_handle(void)
{
    return &s_upgrade_hdl;
}

upgrade_cb_t *upgrade_logic_get_cb(void)
{
    return &s_upgrade_cb;
}

void upgrade_set_errno(upgrade_type_t type, errno_t err)
{
    if (type == UPGRADE_TYPE_COMMON) {
        s_upgrade_hdl.err = err;
        LOG_ERR("common error: %d\n", err);
    } else if (type == UPGRADE_TYPE_FW) {
        s_upgrade_hdl.fw_handle.err = err;
        LOG_ERR("fw error: %d\n", err);
    } else if (type == UPGRADE_TYPE_APP) {
        s_upgrade_hdl.app_handle.err = err;
        LOG_ERR("app error: %d\n", err);
    }
}

static int compare_size(const void *a, const void *b)
{
    return ((file_info_t *)a)->size >= ((file_info_t *)b)->size ? -1 : 1;
}

static int upgrade_logic_organize(upgrade_handle_t *hdl)
{
    int ret = -1;
    char url[263] = {0};

    hdl->max_file_size = 0;

    printf("%s: app %d %d\n", __func__, hdl->app_handle.need_upgrade, hdl->app_handle.upgrade_status);
    if (hdl->app_handle.need_upgrade == 1 && hdl->app_handle.upgrade_status != UPGRADE_SUCC) {
        hdl->app_handle.file_size_all = 0;
        unsigned char md5[MD5_LEN * 2 + 1] = {0};
        long start = os_adapter()->get_time_ms();
        qsort(hdl->app_handle.file_info, hdl->app_handle.file_num, sizeof(file_info_t), compare_size);
        for (int i = 0; i < hdl->app_handle.file_num; i++) {
            memset(md5, 0, sizeof(md5));
            memset(url, 0, sizeof(url));
            snprintf(url, sizeof(url), "%s/%s", APP_DIR, hdl->app_handle.file_info[i].name);
            // 文件不存在或md5不相等
            char out_md5[16 + 1] = {0};
            ret = get_md5sum(url, hdl->app_handle.file_info[i].size, out_md5);
            if (ret != 0) {
                hdl->app_handle.file_info[i].file_change = 1; // need upgrade
                hdl->max_file_size = hdl->app_handle.file_info[i].size > hdl->max_file_size ?
                    hdl->app_handle.file_info[i].size : hdl->max_file_size;
            } else {
                for (int i = 0; i < 16; i++) {
                    sprintf((char *)&md5[i * 2], "%02X", out_md5[i]);
                }
                if (strcmp(md5, hdl->app_handle.file_info[i].md5) != 0) {
                    hdl->app_handle.file_info[i].file_change = 1; // need upgrade
                    hdl->max_file_size = hdl->app_handle.file_info[i].size > hdl->max_file_size ?
                        hdl->app_handle.file_info[i].size : hdl->max_file_size;
                }
            }

            // if (md5_adapter_get_file_md5(url, md5) != 0 || strcmp((char *)md5, hdl->app_handle.file_info[i].md5) != 0) {
            //     hdl->app_handle.file_info[i].file_change = 1; // need upgrade
            //     hdl->max_file_size = hdl->app_handle.file_info[i].size > hdl->max_file_size ?
            //         hdl->app_handle.file_info[i].size : hdl->max_file_size;
            // }
            hdl->app_handle.file_size_all += hdl->app_handle.file_info[i].size;
        }
        printf("md5 time = %ld\n", os_adapter()->get_time_ms() - start);
    }

    ret = -1;
    printf("%s: fw %d %d\n", __func__, hdl->fw_handle.need_upgrade, hdl->fw_handle.upgrade_status);
    if (hdl->fw_handle.need_upgrade == 1 && hdl->fw_handle.upgrade_status != UPGRADE_SUCC) {
        for (int i = 0; i < hdl->fw_handle.fw_num; i++) {
            printf("upgrade part[%d] %s\n", i,hdl->fw_handle.fw_info[i].part_name);
            if (strcmp(hdl->fw_handle.fw_info[i].part_name, "NULL") == 0) {
                printf("part %s size: %lu\n", hdl->fw_handle.fw_info[i].part_name, 55000);  //mcu文件大小先固定
                hdl->fw_handle.file_size_all += 55000;
                continue;
            }
            struct part *blkpart = NULL;
            blkpart = get_part_by_name(hdl->fw_handle.fw_info[i].part_name);
            if (!blkpart) {
                LOG_ERR("get %s part failed\n", hdl->fw_handle.fw_info[i].part_name);
                goto exit;
            }
            printf("part %s size: %lu\n", hdl->fw_handle.fw_info[i].part_name, blkpart->bytes);
            hdl->max_file_size = hdl->max_file_size > blkpart->bytes ? hdl->max_file_size : blkpart->bytes;
            hdl->fw_handle.file_size_all += blkpart->bytes;
        }
    }

    printf("max file size %d\n", hdl->max_file_size);
    if (hdl->max_file_size > 0) {
        if (hdl->max_file_buffer) {
            void *ptr = os_adapter()->realloc(hdl->max_file_buffer, hdl->max_file_size);
            if (!ptr) {
                LOG_ERR("realloc max file failed\n");
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
                goto exit;
            }
            hdl->max_file_buffer = ptr;
            memset(hdl->max_file_buffer, 0, hdl->max_file_size);
        } else {
            hdl->max_file_buffer = os_adapter()->calloc(1, hdl->max_file_size);
            if (!hdl->max_file_buffer) {
                LOG_ERR("malloc max file failed\n");
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
                goto exit;
            }
        }
    } else {
        // app若是需要升级，但是内容完全一样，也显示一下进度和升级成功页面
        if (!hdl->app_handle.need_upgrade) {
            goto exit;
        }
    }
    ret = 0;
exit:
    return ret;
}

static int upgrade_recursive_rm_dir(char *path)
{
    int ret = -1;
    void *dir = NULL;
    fs_info_t info = {0};
    char *subpath = NULL;
    char *info_name = NULL;

    if (NULL == path) {
        LOG_ERR("path is null\n");
        goto exit;
    }

    if (path[strlen(path)-1] == '/') {
       path[strlen(path)-1] = '\0';
    }

    if (NULL == (dir = fs_adapter()->opendir(path))) {
        LOG_ERR("Dir open failed:%s.\n", path);
        goto exit;
    }

    while (1) {
        memset(&info, 0, sizeof(fs_info_t));
        if (fs_adapter()->readdir(dir, &info) < 0) {
            break;
        }

        if (0 == strcmp(info.name, ".") || 0 == strcmp(info.name, "..")) {
            continue;
        }

        info_name = info.name_ptr == NULL ? info.name :info.name_ptr;
        if (strlen(info_name) == 0) {
            printf("readdir end\n");
            break;
        }

        subpath = os_adapter()->calloc(1, strlen(info_name) + strlen(path) + 2);
        // try_sram_calloc((void **)&subpath, strlen(info_name) + strlen(path) + 2);
        if (NULL == subpath) {
            LOG_ERR("Out of memory.\n");
            goto exit;
        }

        sprintf(subpath, "%s/%s", path, info_name);
        if (UNIT_FS_DIR == info.type) {
            upgrade_recursive_rm_dir(subpath);
        } else {
            printf("clean files rm %s\n", subpath);
            if (fs_adapter()->remove(subpath) != 0) {
                LOG_ERR("remove %s failed.\n", subpath);
            }
        }
        try_sram_free(subpath);
        subpath = NULL;
        os_adapter()->free(info.name_ptr);
        info.name_ptr = NULL;
    }
    ret = 0;

exit:
    os_adapter()->free(info.name_ptr);
    info.name_ptr = NULL;
    if (dir) {
        fs_adapter()->closedir(dir);
    }
    fs_adapter()->rmdir(path);
    return ret;
}

static int upgrade_clean_files(void)
{
    int ret = -1;
    int clean_num = sizeof(s_clean_info) / sizeof(clean_info_t);

    printf("clean num = %d\n", clean_num);
    for (int i = 0; i < clean_num; i++) {
        if (s_clean_info[i].type == UNIT_FS_DIR) {
            ret = upgrade_recursive_rm_dir(s_clean_info[i].path);
        } else {
            ret = fs_adapter()->remove(s_clean_info[i].path);
        }
    }
    ret = 0;
    return ret;
}

static int upgrade_cal_need_space(upgrade_handle_t *hdl)
{
    int total_blkcnt = 0;
    int block_size = 4096;

    struct statfs fsinfo = {0};
    int ret = statfs(APP_DIR, &fsinfo);
    if (ret == 0) {
        block_size = fsinfo.f_bsize;
    }

    for (int i = 0; i < hdl->app_handle.file_num; i++) {
        if (strstr(hdl->app_handle.file_info[i].name, "mcu") != NULL) {
            printf("calculate size,skip mcu\n");
            continue;
        }
        total_blkcnt += ((hdl->app_handle.file_info[i].size + block_size - 1) / block_size); // 使用block cnt计算剩余所需空间
    }
    printf("need blkcnt = %d, file_size_all = %d\n", total_blkcnt, hdl->app_handle.file_size_all);
    return total_blkcnt;
}

static int upgrade_cal_remain_space(upgrade_handle_t *hdl)
{
    int available_blkcnt = 0;
    int ret = -1;
    int i = 0;
    char realpath[263] = {0};
    struct statfs fsinfo = {0};
    fs_info_t info = {0};
    int block_size = 4096;

    ret = statfs(APP_DIR, &fsinfo);
    if (ret == 0) {
        block_size = fsinfo.f_bsize;
        available_blkcnt += fsinfo.f_bavail;
    }
    // printf("%lu %lu %llu %llu %lu %lu\n", fsinfo.f_bsize, fsinfo.f_blocks, fsinfo.f_bfree, fsinfo.f_bavail, fsinfo.f_files, fsinfo.f_ffree);
    printf("lfs avail blkcnt = %llu, lfs all blkcnt = %lu\n", fsinfo.f_bavail, fsinfo.f_blocks);

    for (i = 0; i < hdl->app_handle.file_num; i++) {
        memset(&info, 0, sizeof(fs_info_t));
        memset(realpath, 0, sizeof(realpath));
        sprintf(realpath, APP_DIR"/%s", hdl->app_handle.file_info[i].name);
        ret = fs_adapter()->stat(realpath, &info);
        if (ret != 0) {
            printf("new file: %s.\n", realpath);
            continue;
        }
        available_blkcnt += ((info.size + block_size - 1) / block_size);
        os_adapter()->free(info.name_ptr);
        info.name_ptr = NULL;
    }

    printf("total available blkcnt = %d.\n", available_blkcnt);

exit:
    return available_blkcnt;
}

void upgrade_logic_set_replace(upgrade_handle_t *hdl, unsigned char replace)
{
    void *fp = NULL;

    printf("set replace:%u\n", replace);

    if (replace == 0) {
        printf("remove %s\n", UPGRADE_STATUS_FILE);
        fs_adapter()->remove(UPGRADE_STATUS_FILE);
        goto exit;
    }

    if (hdl->fw_handle.upgrade_status != UPGRADE_FAIL_REPLACED && hdl->app_handle.upgrade_status != UPGRADE_FAIL_REPLACED) {
        printf("no need to set replace\n");
        goto exit;
    }

    fs_info_t info = {0};
    if (fs_adapter()->stat(UPGRADE_STATUS_FILE, &info) == 0) {
        // printf("replace file exsit\n");
        goto exit;
    }

    fp = fs_adapter()->fs_open(UPGRADE_STATUS_FILE, UNIT_FS_WRONLY | UNIT_FS_TRUNC | UNIT_FS_CREAT);
    if (!fp) {
        LOG_ERR("open %s failed\n", UPGRADE_STATUS_FILE);
        goto exit;
    }

exit:
    if (fp) {
        fs_adapter()->fs_close(fp);
    }
    return;
}

unsigned char upgrade_logic_get_replace(void)
{
    bool replace = 0;

    fs_info_t info = {0};
    if (fs_adapter()->stat(UPGRADE_STATUS_FILE, &info) != 0) {
        // printf("stat failed\n");
        goto exit;
    }

    replace = 1;

exit:
    return replace;
}

static int upgrade_msg_file_update(upgrade_handle_t *hdl)
{
    int ret = -1;
    cJSON *root = NULL;
    char *root_str = NULL;
    void *fp = NULL;
    cJSON *fw = NULL;
    cJSON *app = NULL;
    cJSON *rcvy = NULL;
    cJSON *mcu_b = NULL;
    char name[strlen(APP_DIR) + strlen(UPGRADE_MSG_FILE_COMPLETE) + 5];

    printf("%s %d\n", __func__, __LINE__);
    root = cJSON_CreateObject();
    if (!root) {
        LOG_ERR("cJSON_CreateObject failed\n");
        goto exit;
    }

    printf("%s %d\n", __func__, __LINE__);

    cJSON_AddStringToObject(root, "method", hdl->method == UPGRADE_METHOD_LOCAL ? "local" : "cloud");

    rcvy = cJSON_CreateObject();
    if (!rcvy) {
        LOG_ERR("cJSON_CreateObject failed\n");
        goto exit;
    }
    cJSON_AddItemToObject(root, "rcvy", rcvy);
    cJSON_AddStringToObject(rcvy, "version", RECOVERY_VERSION);

    printf("%s %d\n", __func__, __LINE__);

    mcu_b = cJSON_CreateObject();
    if (!mcu_b) {
        LOG_ERR("cJSON_CreateObject failed\n");
        goto exit;
    }
    cJSON_AddItemToObject(root, "mcu_b", mcu_b);
    cJSON_AddStringToObject(mcu_b, "version", get_bootloader_version_string());

    printf("%s %d\n", __func__, __LINE__);

    if (hdl->fw_handle.need_upgrade) {
        fw = cJSON_CreateObject();
        if (!fw) {
            LOG_ERR("cJSON_CreateObject failed\n");
            goto exit;
        }
        cJSON_AddStringToObject(fw, "status", hdl->fw_handle.upgrade_status == UPGRADE_SUCC ? "success" : "fail");
        cJSON_AddStringToObject(fw, "version", hdl->fw_handle.version);
        cJSON_AddItemToObject(root, "fw", fw);
    }

    printf("%s %d\n", __func__, __LINE__);

    if (hdl->app_handle.need_upgrade) {
        app = cJSON_CreateObject();
        if (!app) {
            LOG_ERR("cJSON_CreateObject failed\n");
            goto exit;
        }
        cJSON_AddStringToObject(app, "status", hdl->app_handle.upgrade_status == UPGRADE_SUCC ? "success" : "fail");
        cJSON_AddStringToObject(app, "version", hdl->app_handle.version);
        cJSON_AddItemToObject(root, "app", app);
    }

    printf("upgrade_msg_file_update: %s\n", hdl->msg_file ? hdl->msg_file : "null");
    if (hdl->msg_file) {
        fs_adapter()->remove(hdl->msg_file);
        // fp = fs_adapter()->fs_open(hdl->msg_file, UNIT_FS_WRONLY | UNIT_FS_TRUNC | UNIT_FS_CREAT);
    }
    memset(name, 0, sizeof(name));
    sprintf(name, "%s/%s", APP_DIR, UPGRADE_MSG_FILE_COMPLETE);
    fp = fs_adapter()->fs_open(name, UNIT_FS_WRONLY | UNIT_FS_TRUNC | UNIT_FS_CREAT);
    if (!fp) {
        LOG_ERR("fs_open failed %s\n", hdl->msg_file ? hdl->msg_file : "null");
        goto exit;
    }
    printf("%s %d\n", __func__, __LINE__);
    root_str = cJSON_PrintUnformatted(root);
    if (root_str) {
        unsigned int bw = 0;
        ret = fs_adapter()->fs_write(fp, root_str, strlen(root_str), &bw);
        if (ret != 0 || bw != strlen(root_str)) {
            LOG_ERR("fs_write failed\n");
            goto exit;
        }
    }
    fs_adapter()->fs_close(fp);
    fp = NULL;

    printf("%s %d\n", __func__, __LINE__);

    // if (hdl->msg_file) {
    //     printf("%s %d\n", __func__, __LINE__);
    //     memset(name, 0, sizeof(name));
    //     printf("%s %d\n", __func__, __LINE__);
    //     sprintf(name, "%s/%s", APP_DIR, UPGRADE_MSG_FILE_COMPLETE);
    //     printf("%s %d\n", __func__, __LINE__);
    //     fs_adapter()->rename(hdl->msg_file, name);
    // }
    printf("%s %d\n", __func__, __LINE__);
    ret = 0;
exit:
    if (fp) {
        fs_adapter()->fs_close(fp);
    }
    free(root_str);
    cJSON_Delete(root);
    printf("%s %d\n", __func__, __LINE__);
    return ret;
}

/***************************************************************
  *  @brief     MCU ota升级状态通知app
  *  @param   @hdl 升级句柄
  *  @param   @status 0:成功，1：失败
  *  @return  0:成功，-1：失败
  *  @note      
  *  @Sample usage: 
 **************************************************************/
int set_upgrade_status_msg_app_file(upgrade_handle_t *hdl,int status)
{
    int ret = -1;
    void *fp = NULL;    
    char *root_str = NULL;
    cJSON *root = NULL;
    if (hdl->method == UPGRADE_METHOD_CLOUD) {
        if (hdl->fw_handle.upgrade_status == UPGRADE_SUCC || hdl->app_handle.upgrade_status == UPGRADE_SUCC) {
            fp = fs_adapter()->fs_open(UPGRADE_STATUS_MSG_APP_FILE, UNIT_FS_WRONLY | UNIT_FS_TRUNC | UNIT_FS_CREAT);
            if (!fp) {
                LOG_ERR("open %s failed\n", UPGRADE_STATUS_MSG_APP_FILE);
                goto exit;
            }
        }
    }
    root = cJSON_CreateObject();
    if (!root) {
        LOG_ERR("cJSON_CreateObject failed\n");
        goto exit;
    }

    cJSON_AddStringToObject(root, "status", status == 0 ? "success" : "fail");
    root_str = cJSON_PrintUnformatted(root);
    if (root_str) {
        unsigned int bw = 0;
        ret = fs_adapter()->fs_write(fp, root_str, strlen(root_str), &bw);
        if (ret != 0 || bw != strlen(root_str)) {
            LOG_ERR("fs_write failed\n");
            goto exit;
        }
    }

    ret = 0;
exit:
    if (fp) {
        fs_adapter()->fs_close(fp);
        fp = NULL;
    }

    if (root) {
        cJSON_Delete(root);
        root = NULL;
    }

    if (root_str) {
        free(root_str);
        root_str = NULL;
    }

    return ret;
}

extern bool mcu_jump_to_app(uint8_t arg);
int upgrade_logic_reboot(upgrade_handle_t *hdl, upgrade_mode_t mode)
{
    int ret = -1;
    int retry = 0;
    wifi_adapter_deinit();
    os_adapter()->msleep(100);
    if (mode == UPGRADE_MODE_NORMAL) {
        if (hdl->fw_handle.upgrade_status == UPGRADE_FAIL_REPLACED && hdl->app_handle.upgrade_status == UPGRADE_FAIL_REPLACED) {
            LOG_ERR("upgrade fail and replaced, normal mode is not allowed to enter, reboot to recovery\n");
            goto exit;
        }

        if (upgrade_logic_get_replace() == 1) {
            LOG_ERR("replace file exist, normal mode is not allowed to enter, reboot to recovery\n");
            goto exit;
        }

        upgrade_msg_file_update(hdl);

        printf("%s %d\n", __func__, __LINE__);

        //云端升级成功,写入升级状态文件,用于通知APP,触发小程序通知
        if (hdl->method == UPGRADE_METHOD_CLOUD) {
            if (hdl->fw_handle.upgrade_status == UPGRADE_SUCC || hdl->app_handle.upgrade_status == UPGRADE_SUCC) {
               ret = set_upgrade_status_msg_app_file(hdl, 0);
               if (ret) {
                   LOG_ERR("set_upgrade_status_msg_app_file failed\n");
               }
            }
        }

        printf("%s %d\n", __func__, __LINE__);

        upgrade_logic_deinit();

        while(retry<3){
            ret = mcu_jump_to_app(1);
            if(ret){
                XR_OS_MSleep(100);
                break;
            }
            retry++;
        }

        printf("reboot to normal mode\n");
        char *env_argv[] = {
            "fw_setenv",
            "loadparts",
            "arm-ococci@:dsp-ococci@:rv-ococci@:config@0xc000000:"};
        cmd_fw_setenv(3, env_argv);
    }

    cmd_reboot(1, NULL);
    ret = 0;
exit:
    return ret;
}

static void upgrade_buff_to_upper(char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] = buf[i] - 'a' + 'A';
        }
    }
}

static int upgrade_check_bat(upgrade_method_t method, int adc_threshold, int adc_charge_threshold)
{
    if (method == UPGRADE_METHOD_LOCAL) {
        return 0;
    }

    const int cnt = 2;
    int total_adc = 0;
    int tmp_cnt = cnt;
    while (tmp_cnt-- > 0) {
        extern int dev_battery_get_voltage(void);
        total_adc += dev_battery_get_voltage();
    }
    char is_charge = dev_mcu_charge_det_get();
    int threshold = is_charge ? adc_charge_threshold : adc_threshold;
    printf("is charge = %d, threshold = %d\n", is_charge, threshold);
    if((total_adc / cnt) < (threshold - BATTERY_DIFF)){
        printf("adc too low %d than %d\n", total_adc / cnt, (threshold - BATTERY_DIFF));
        return -1;
    }
    printf("now adc %d need %d\n", total_adc / cnt, threshold - BATTERY_DIFF);
    return 0;
}

int upgrade_logic_get_otaimg_info(char *dst_name, char *cur_name, ota_img_info_t *img_info)
{
    int ret = -1;
    char *point = NULL;
    char *p = NULL;
    char *q = NULL;
    unsigned char flag = 0;

    if (!dst_name || !cur_name || !img_info) {
        LOG_ERR("item is null %p %p %p\n", dst_name, cur_name, img_info);
        goto exit;
    }

    point = strstr(dst_name, ".");
    if (!point) {
        LOG_ERR("name is not end with .\n");
        goto exit;
    }

    {
        char pre[point - dst_name + 1];
        memset(pre, 0, point - dst_name + 1);
        strncpy(pre, dst_name, point - dst_name);
        char sub[strlen(dst_name) - strlen(pre)];
        memset(sub, 0, strlen(dst_name) - strlen(pre));
        strcpy(sub, point + 1);

        if ((p = strstr(cur_name, pre)) && (q = strstr(cur_name, sub))) {
            char *p1 = p + strlen(pre);
            if (p1[0] == '_') {
                char *p2 = strchr(p1 + 1, '_');
                if (p2) {
                    char w[p2 - p1];
                    memset(w, 0, sizeof(w));
                    memcpy(w, p1 + 1, p2 - p1 - 1);

                    char h[q - p2];
                    memset(h, 0, sizeof(h));
                    memcpy(h, p2 + 1, q - p2 - 1);

                    img_info->w = atoi(w);
                    img_info->h = atoi(h);
                    snprintf(img_info->path, OTA_IMG_PATH_LEN, "P:"APP_DIR"/%s", cur_name);
                    flag = 1;
                }
            }
        }
    }

    if (flag == 1) {
        printf("find ota img %s\n", img_info->path);
        ret = 0;
    }
exit:
    return ret;
}
static int check_msg_file_md5(upgrade_handle_t *hdl)
{
    int ret = -1;
    void *dir_hdl = NULL;
    unsigned char msg_find = 0;

    dir_hdl = fs_adapter()->opendir(APP_DIR);
    if (!dir_hdl) {
        LOG_ERR("opendir failed\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_FS_OPEN);
        goto exit;
    }

    while (1) {
        fs_info_t info = {0};
        ret = fs_adapter()->readdir(dir_hdl, &info);
        if (ret < 0) {
            LOG_ERR("readdir failed\n");
            upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_FS_READ);
            goto exit;
        }
        // printf("name:%s\n", info.name);
        if (ret == 0 && strlen(info.name) == 0 && !info.name_ptr) {
            if (msg_find == 0) {
                LOG_ERR("file not find %s\n", hdl->msg_file ? hdl->msg_file : "null");
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MSG_FILE_NOT_FIND);
                ret = -1;
                goto exit;
            }
            break;
        }

        char *name = info.name_ptr ? info.name_ptr : info.name;
        char prefix[strlen(UPGRADE_MSG_FILE_PREFIX) + 1];
        memcpy(prefix, name, strlen(UPGRADE_MSG_FILE_PREFIX));
        prefix[strlen(UPGRADE_MSG_FILE_PREFIX)] = '\0';
        char suffix[strlen(UPGRADE_MSG_FILE_SUFFIX) + 1];
        memcpy(suffix, &name[strlen(name) - strlen(UPGRADE_MSG_FILE_SUFFIX)], strlen(UPGRADE_MSG_FILE_SUFFIX));
        suffix[strlen(UPGRADE_MSG_FILE_SUFFIX)] = '\0';
        if (msg_find == 0 && strcmp(prefix, UPGRADE_MSG_FILE_PREFIX) == 0 && strcmp(suffix, UPGRADE_MSG_FILE_SUFFIX) == 0 && 
            ((strlen(name) - strlen(UPGRADE_MSG_FILE_SUFFIX) - strlen(UPGRADE_MSG_FILE_PREFIX)) == (MD5_LEN * 2))) {
            unsigned char file_md5[MD5_LEN * 2 + 1];
            char *md5_pos = name + strlen(UPGRADE_MSG_FILE_PREFIX);
            memcpy(file_md5, md5_pos, MD5_LEN * 2);
            file_md5[MD5_LEN * 2] = '\0';
            unsigned char file_md5_caculated[MD5_LEN * 2 + 1];
            hdl->msg_file = os_adapter()->calloc(1, strlen(name) + strlen(APP_DIR) + 3);
            if (!hdl->msg_file) {
                LOG_ERR("os_adapter()->calloc failed\n");
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
                ret = -1;
                goto exit;
            }
            sprintf(hdl->msg_file, "%s/%s", APP_DIR, name);
            printf("msg_file:%s\n", hdl->msg_file);
            ret = md5_adapter_get_file_md5(hdl->msg_file, file_md5_caculated);
            if (ret != 0) {
                LOG_ERR("md5_adapter_get_file_md5 failed\n");
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MD5_GET_FAIL);
                goto exit;
            }
            upgrade_buff_to_upper((char *)file_md5, MD5_LEN * 2);
            if (strcmp((const char *)file_md5, (const char *)file_md5_caculated) != 0) {
                LOG_ERR("md5 not match\n");
                printf("md5_file:%s\n", file_md5);
                printf("md5_cacu:%s\n", file_md5_caculated);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MD5_VERIFY);
                ret = -1;
                goto exit;
            }
            msg_find = 1;
        } else {
            if (upgrade_logic_get_otaimg_info("ota_check.bin", name, &hdl->img.ota_check) == 0 ||
                upgrade_logic_get_otaimg_info("ota_delete.bin", name, &hdl->img.ota_delete) == 0 ||
                upgrade_logic_get_otaimg_info("ota_end.bin", name, &hdl->img.ota_end) == 0 ||
                upgrade_logic_get_otaimg_info("ota_net_err.bin", name, &hdl->img.ota_net_err) == 0 ||
                upgrade_logic_get_otaimg_info("ota_question.bin", name, &hdl->img.ota_question) == 0 ||
                upgrade_logic_get_otaimg_info("ota_right_btn.bin", name, &hdl->img.ota_right_btn) == 0 ||
                upgrade_logic_get_otaimg_info("ota_warn.bin", name, &hdl->img.ota_warn) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_1.bin", name, &hdl->img.ota_wifi_1) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_2.bin", name, &hdl->img.ota_wifi_2) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_3.bin", name, &hdl->img.ota_wifi_3) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_connected.bin", name, &hdl->img.ota_wifi_connected) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_info.bin", name, &hdl->img.ota_wifi_info) == 0 ||
                upgrade_logic_get_otaimg_info("ota_wifi_lock.bin", name, &hdl->img.ota_wifi_lock) == 0 ||
                upgrade_logic_get_otaimg_info("ota_back_btn.bin", name, &hdl->img.ota_back_btn) == 0) {
                ;
            }
        }

        os_adapter()->free(info.name_ptr);
        info.name_ptr = NULL;
    }
    ret = 0;
exit:
    if (ret != 0) {
        if (hdl->msg_file) {
            os_adapter()->free(hdl->msg_file);
            hdl->msg_file = NULL;
        }
    }
    if (dir_hdl) {
        fs_adapter()->closedir(dir_hdl);
    }
    return ret;
}

static int upgrade_logic_get_info(upgrade_handle_t *hdl)
{
    int ret = -1;
    void *fp = NULL;
    char *file_buffer = NULL;
    unsigned int br = 0;
    const int read_len_once = 4 * 1024;
    cJSON *root = NULL;

    ret = check_msg_file_md5(hdl);
    if (ret != 0) {
        goto exit;
    }

    ret = -1;
    fs_info_t info = {0};
    if (fs_adapter()->stat(hdl->msg_file, &info) != 0) {
        LOG_ERR("stat failed\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_FS_STAT);
        goto exit;
    }
    os_adapter()->free(info.name_ptr);
    info.name_ptr = NULL;

    file_buffer = os_adapter()->calloc(1, info.size + 1);
    if (!file_buffer) {
        LOG_ERR("os_adapter()->calloc failed\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
        goto exit;
    }

    fp = fs_adapter()->fs_open(hdl->msg_file, UNIT_FS_RDONLY);
    if (!fp) {
        LOG_ERR("fopen failed\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_FS_OPEN);
        goto exit;
    }

    int read_len = 0;
    while (info.size > 0) {
        ret = fs_adapter()->fs_read(fp, file_buffer + read_len, info.size > read_len_once ? read_len_once : info.size, &br);
        if (ret != 0) {
            LOG_ERR("fread failed\n");
            upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_FS_READ);
            goto exit;
        }
        read_len += br;
        info.size -= br;
    }

    ret = -1;
    root = cJSON_Parse(file_buffer);
    if (!root) {
        LOG_ERR("cJSON_Parse failed\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
        goto exit;
    }

    printf("upgrade info: %s\n", file_buffer);

    cJSON *item = cJSON_GetObjectItem(root, "battery");
    if (!item) {
        LOG_ERR("battery is null\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
        goto exit;
    }
    cJSON *adc_threshold = cJSON_GetObjectItem(item, "adc_threshold");
    cJSON *adc_charge_threshold = cJSON_GetObjectItem(item, "adc_charge_threshold");
    if (!adc_threshold || !adc_charge_threshold) {
        LOG_ERR("battery item is null %p %p \n", adc_threshold, adc_charge_threshold);
    }
    hdl->adc_threshold = adc_threshold->valueint;
    hdl->adc_charge_threshold = adc_charge_threshold->valueint;
    printf("battery threshold:%d %d\n", hdl->adc_threshold, hdl->adc_charge_threshold);
    dev_battery_init(NULL);

    item = cJSON_GetObjectItem(root, "display");
    if (item) {
        hdl->disp.enable = 1;
        cJSON *sub_item = cJSON_GetObjectItem(item, "type");
        hdl->disp.type = sub_item ? sub_item->valueint : -1;
        sub_item = cJSON_GetObjectItem(item, "bl_mode");
        hdl->disp.bl_mode = sub_item ? sub_item->valueint : -1;
        sub_item = cJSON_GetObjectItem(item, "te_support");
        hdl->disp.te_support = sub_item ? sub_item->valueint : -1;
        sub_item = cJSON_GetObjectItem(item, "hor_res");
        hdl->disp.hor_res = sub_item ? sub_item->valueint : -1;
        sub_item = cJSON_GetObjectItem(item, "ver_res");
        hdl->disp.ver_res = sub_item ? sub_item->valueint : -1;
        sub_item = cJSON_GetObjectItem(item, "inch");
        hdl->disp.inch = sub_item ? sub_item->valuedouble : -1;
        sub_item = cJSON_GetObjectItem(item, "rotation");
        hdl->disp.rotation = sub_item ? sub_item->valueint : -1;
    }
    printf("display enable:%d type:%d bl_mode:%d te:%d hor_res:%d ver_res:%d inch:%f rotation:%d\n", hdl->disp.enable,
        hdl->disp.type, hdl->disp.bl_mode, hdl->disp.te_support, hdl->disp.hor_res,
        hdl->disp.ver_res, hdl->disp.inch, hdl->disp.rotation);

    item = cJSON_GetObjectItem(root, "led");
    if (item) {
        hdl->led.enable = 1;
        dev_led_init();
    }
    printf("led enable:%d\n", hdl->led.enable);

    item = cJSON_GetObjectItem(root, "tp");
    if (item) {
        hdl->tp.enable = 1;
        cJSON *subitem = cJSON_GetObjectItem(item, "type");
        hdl->tp.type = subitem ? subitem->valueint : -1;
    }
    printf("tp enable:%d type:%d\n", hdl->tp.enable, hdl->tp.type);

    item = cJSON_GetObjectItem(root, "encoder");
    if (item) {
        hdl->encoder.enable = 1;
    }
    printf("encoder enable:%d\n", hdl->encoder.enable);

    item = cJSON_GetObjectItem(root, "key");
    if (item) {
        hdl->key.enable = 1;
        cJSON *subitem = cJSON_GetObjectItem(item, "type");
        hdl->key.type = subitem ? subitem->valueint : -1;
    }
    printf("key enable:%d type:%d\n", hdl->key.enable, hdl->key.type);

    item = cJSON_GetObjectItem(root, "button");
    if (item) {
        hdl->btn.enable = 1;
        cJSON *subitem = cJSON_GetObjectItem(item, "back");
        hdl->btn.back = subitem ? subitem->valueint : -1;
        subitem = cJSON_GetObjectItem(item, "ok");
        hdl->btn.ok = subitem ? subitem->valueint : -1;
        subitem = cJSON_GetObjectItem(item, "left");
        hdl->btn.left = subitem ? subitem->valueint : -1;
        subitem = cJSON_GetObjectItem(item, "up");
        hdl->btn.up = subitem ? subitem->valueint : -1;
        subitem = cJSON_GetObjectItem(item, "right");
        hdl->btn.right = subitem ? subitem->valueint : -1;
        subitem = cJSON_GetObjectItem(item, "down");
        hdl->btn.down = subitem ? subitem->valueint : -1;
    }
    printf("button enable:%d back:%d ok:%d left:%d up:%d right:%d down:%d\n",
        hdl->btn.enable, hdl->btn.back, hdl->btn.ok, hdl->btn.left, hdl->btn.up, hdl->btn.right, hdl->btn.down);

    item = cJSON_GetObjectItem(root, "method");
    if (!item) {
        LOG_ERR("method is null\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
        goto exit;
    }
    if (strcmp(item->valuestring, "local") == 0) {
        hdl->method = UPGRADE_METHOD_LOCAL;
    } else if (strcmp(item->valuestring, "cloud") == 0) {
        hdl->method = UPGRADE_METHOD_CLOUD;
    } else {
        LOG_ERR("method is invalid\n");
        upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_INVALID);
        goto exit;
    }
    printf("method: %d\n", hdl->method);

    item = cJSON_GetObjectItem(root, "language");
    if (!item) {
        printf("language is null\n");
        try_sram_calloc((void **)&hdl->note.upgrade_preparing, strlen(s_ota_note_default.upgrade_preparing) + 1);
        try_sram_calloc((void **)&hdl->note.upgrading, strlen(s_ota_note_default.upgrading) + 1);
        try_sram_calloc((void **)&hdl->note.upgrading_notop, strlen(s_ota_note_default.upgrading_notop) + 1);
        try_sram_calloc((void **)&hdl->note.upgrade_succ_rebooting, strlen(s_ota_note_default.upgrade_succ_rebooting) + 1);
        try_sram_calloc((void **)&hdl->note.upgrade_fail, strlen(s_ota_note_default.upgrade_fail) + 1);
        try_sram_calloc((void **)&hdl->note.upgrade_cancel, strlen(s_ota_note_default.upgrade_cancel) + 1);
        try_sram_calloc((void **)&hdl->note.upgrade_fail_retry, strlen(s_ota_note_default.upgrade_fail_retry) + 1);
        try_sram_calloc((void **)&hdl->note.help_connect, strlen(s_ota_note_default.help_connect) + 1);
        try_sram_calloc((void **)&hdl->note.ok_to_connect, strlen(s_ota_note_default.ok_to_connect) + 1);
        try_sram_calloc((void **)&hdl->note.loading, strlen(s_ota_note_default.loading) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_press, strlen(s_ota_note_default.wifi_press) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_fresh, strlen(s_ota_note_default.wifi_fresh) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_freshing, strlen(s_ota_note_default.wifi_freshing) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_fresh_succ, strlen(s_ota_note_default.wifi_fresh_succ) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_enter_pwd, strlen(s_ota_note_default.wifi_enter_pwd) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_connecting, strlen(s_ota_note_default.wifi_connecting) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_connect_succ, strlen(s_ota_note_default.wifi_connect_succ) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_connect_fail, strlen(s_ota_note_default.wifi_connect_fail) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_conn_fail_info, strlen(s_ota_note_default.wifi_conn_fail_info) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_retry_once, strlen(s_ota_note_default.wifi_retry_once) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_connected, strlen(s_ota_note_default.wifi_connected) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_list_connected, strlen(s_ota_note_default.wifi_list_connected) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_saved, strlen(s_ota_note_default.wifi_saved) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_rm_this_net, strlen(s_ota_note_default.wifi_rm_this_net) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_rm_sure, strlen(s_ota_note_default.wifi_rm_sure) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_rm, strlen(s_ota_note_default.wifi_rm) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_only_2g, strlen(s_ota_note_default.wifi_only_2g) + 1);
        try_sram_calloc((void **)&hdl->note.wifi_WIFI, strlen(s_ota_note_default.wifi_WIFI) + 1);
        try_sram_calloc((void **)&hdl->note.bat_low, strlen(s_ota_note_default.bat_low) + 1);
        if (!get_real_ptr(hdl->note.upgrade_preparing) || !get_real_ptr(hdl->note.upgrading) || !get_real_ptr(hdl->note.upgrading_notop) ||
            !get_real_ptr(hdl->note.upgrade_succ_rebooting) || !get_real_ptr(hdl->note.upgrade_fail) || !get_real_ptr(hdl->note.upgrade_cancel) ||
            !get_real_ptr(hdl->note.upgrade_fail_retry) || !get_real_ptr(hdl->note.help_connect) || !get_real_ptr(hdl->note.ok_to_connect) ||
            !get_real_ptr(hdl->note.loading) || !get_real_ptr(hdl->note.wifi_press) || !get_real_ptr(hdl->note.wifi_fresh) ||
            !get_real_ptr(hdl->note.wifi_freshing) || !get_real_ptr(hdl->note.wifi_fresh_succ) || !get_real_ptr(hdl->note.wifi_enter_pwd) ||
            !get_real_ptr(hdl->note.wifi_connecting) || !get_real_ptr(hdl->note.wifi_connect_succ) || !get_real_ptr(hdl->note.wifi_connect_fail) ||
            !get_real_ptr(hdl->note.wifi_conn_fail_info) || !get_real_ptr(hdl->note.wifi_retry_once) || !get_real_ptr(hdl->note.wifi_connected) ||
            !get_real_ptr(hdl->note.wifi_list_connected) || !get_real_ptr(hdl->note.wifi_saved) || !get_real_ptr(hdl->note.wifi_rm_this_net) ||
            !get_real_ptr(hdl->note.wifi_rm_sure) || !get_real_ptr(hdl->note.wifi_rm) || !get_real_ptr(hdl->note.wifi_only_2g) ||
            !get_real_ptr(hdl->note.wifi_WIFI) || !get_real_ptr(hdl->note.bat_low)) {
            printf("malloc fail %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p\n",
                hdl->note.upgrade_preparing, hdl->note.upgrading, hdl->note.upgrading_notop, hdl->note.upgrade_succ_rebooting,
                hdl->note.upgrade_fail, hdl->note.upgrade_cancel, hdl->note.upgrade_fail_retry, hdl->note.help_connect,
                hdl->note.ok_to_connect, hdl->note.loading, hdl->note.wifi_press, hdl->note.wifi_fresh, hdl->note.wifi_freshing,
                hdl->note.wifi_fresh_succ, hdl->note.wifi_enter_pwd, hdl->note.wifi_connecting, hdl->note.wifi_connect_succ,
                hdl->note.wifi_connect_fail, hdl->note.wifi_conn_fail_info, hdl->note.wifi_retry_once, hdl->note.wifi_connected,
                hdl->note.wifi_list_connected, hdl->note.wifi_saved, hdl->note.wifi_rm_this_net,
                hdl->note.wifi_rm_sure, hdl->note.wifi_rm, hdl->note.wifi_only_2g, hdl->note.wifi_WIFI, hdl->note.bat_low);
            goto exit;
        }
        strcpy(get_real_ptr(hdl->note.upgrade_preparing), s_ota_note_default.upgrade_preparing);
        strcpy(get_real_ptr(hdl->note.upgrading), s_ota_note_default.upgrading);
        strcpy(get_real_ptr(hdl->note.upgrading_notop), s_ota_note_default.upgrading_notop);
        strcpy(get_real_ptr(hdl->note.upgrade_succ_rebooting), s_ota_note_default.upgrade_succ_rebooting);
        strcpy(get_real_ptr(hdl->note.upgrade_fail), s_ota_note_default.upgrade_fail);
        strcpy(get_real_ptr(hdl->note.upgrade_cancel), s_ota_note_default.upgrade_cancel);
        strcpy(get_real_ptr(hdl->note.upgrade_fail_retry), s_ota_note_default.upgrade_fail_retry);
        strcpy(get_real_ptr(hdl->note.help_connect), s_ota_note_default.help_connect);
        strcpy(get_real_ptr(hdl->note.ok_to_connect), s_ota_note_default.ok_to_connect);
        strcpy(get_real_ptr(hdl->note.loading), s_ota_note_default.loading);
        strcpy(get_real_ptr(hdl->note.wifi_press), s_ota_note_default.wifi_press);
        strcpy(get_real_ptr(hdl->note.wifi_fresh), s_ota_note_default.wifi_fresh);
        strcpy(get_real_ptr(hdl->note.wifi_freshing), s_ota_note_default.wifi_freshing);
        strcpy(get_real_ptr(hdl->note.wifi_fresh_succ), s_ota_note_default.wifi_fresh_succ);
        strcpy(get_real_ptr(hdl->note.wifi_enter_pwd), s_ota_note_default.wifi_enter_pwd);
        strcpy(get_real_ptr(hdl->note.wifi_connecting), s_ota_note_default.wifi_connecting);
        strcpy(get_real_ptr(hdl->note.wifi_connect_succ), s_ota_note_default.wifi_connect_succ);
        strcpy(get_real_ptr(hdl->note.wifi_connect_fail), s_ota_note_default.wifi_connect_fail);
        strcpy(get_real_ptr(hdl->note.wifi_conn_fail_info), s_ota_note_default.wifi_conn_fail_info);
        strcpy(get_real_ptr(hdl->note.wifi_retry_once), s_ota_note_default.wifi_retry_once);
        strcpy(get_real_ptr(hdl->note.wifi_connected), s_ota_note_default.wifi_connected);
        strcpy(get_real_ptr(hdl->note.wifi_list_connected), s_ota_note_default.wifi_list_connected);
        strcpy(get_real_ptr(hdl->note.wifi_saved), s_ota_note_default.wifi_saved);
        strcpy(get_real_ptr(hdl->note.wifi_rm_this_net), s_ota_note_default.wifi_rm_this_net);
        strcpy(get_real_ptr(hdl->note.wifi_rm_sure), s_ota_note_default.wifi_rm_sure);
        strcpy(get_real_ptr(hdl->note.wifi_rm), s_ota_note_default.wifi_rm);
        strcpy(get_real_ptr(hdl->note.wifi_only_2g), s_ota_note_default.wifi_only_2g);
        strcpy(get_real_ptr(hdl->note.wifi_WIFI), s_ota_note_default.wifi_WIFI);
        strcpy(get_real_ptr(hdl->note.bat_low), s_ota_note_default.bat_low);
    } else {
        cJSON *upgrade_preparing = cJSON_GetObjectItem(item, "upgrade_preparing");
        if (!upgrade_preparing) {
            printf("upgrade_preparing is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrade_preparing, upgrade_preparing ? (strlen(upgrade_preparing->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrade_preparing) + 1));

        cJSON *upgrading = cJSON_GetObjectItem(item, "upgrading");
        if (!upgrading) {
            printf("upgrading is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrading, upgrading ? (strlen(upgrading->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrading) + 1));

        cJSON *upgrading_notop = cJSON_GetObjectItem(item, "upgrading_notop");
        if (!upgrading_notop) {
            printf("upgrading_notop is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrading_notop, upgrading_notop ? (strlen(upgrading_notop->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrading_notop) + 1));

        cJSON *upgrade_succ_rebooting = cJSON_GetObjectItem(item, "upgrade_succ_rebooting");
        if (!upgrade_succ_rebooting) {
            printf("upgrade_succ_rebooting is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrade_succ_rebooting, upgrade_succ_rebooting ?
            (strlen(upgrade_succ_rebooting->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrade_succ_rebooting) + 1));

        cJSON *upgrade_fail = cJSON_GetObjectItem(item, "upgrade_fail");
        if (!upgrade_fail) {
            printf("upgrade_fail is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrade_fail, upgrade_fail ? (strlen(upgrade_fail->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrade_fail) + 1));

        cJSON *upgrade_cancel = cJSON_GetObjectItem(item, "upgrade_cancel");
        if (!upgrade_cancel) {
            printf("upgrade_cancel is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrade_cancel, upgrade_cancel ? (strlen(upgrade_cancel->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrade_cancel) + 1));

        cJSON *upgrade_fail_retry = cJSON_GetObjectItem(item, "upgrade_fail_retry");
        if (!upgrade_fail_retry) {
            printf("upgrade_fail_retry is null\n");
        }
        try_sram_calloc((void **)&hdl->note.upgrade_fail_retry, upgrade_fail_retry ? (strlen(upgrade_fail_retry->valuestring) + 1) :
            (strlen(s_ota_note_default.upgrade_fail_retry) + 1));

        cJSON *help_connect = cJSON_GetObjectItem(item, "help_connect");
        if (!help_connect) {
            printf("help_connect is null\n");
        }
        try_sram_calloc((void **)&hdl->note.help_connect, help_connect ? (strlen(help_connect->valuestring) + 1) :
            (strlen(s_ota_note_default.help_connect) + 1));

        cJSON *ok_to_connect = cJSON_GetObjectItem(item, "ok_to_connect");
        if (!ok_to_connect) {
            printf("ok_to_connect is null\n");
        }
        try_sram_calloc((void **)&hdl->note.ok_to_connect, ok_to_connect ? (strlen(ok_to_connect->valuestring) + 1) :
            (strlen(s_ota_note_default.ok_to_connect) + 1));

        cJSON *loading = cJSON_GetObjectItem(item, "loading");
        if (!loading) {
            printf("loading is null\n");
        }
        try_sram_calloc((void **)&hdl->note.loading, loading ? (strlen(loading->valuestring) + 1) :
            (strlen(s_ota_note_default.loading) + 1));

        cJSON *wifi_press = cJSON_GetObjectItem(item, "wifi_press");
        if (!wifi_press) {
            printf("wifi_press is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_press, wifi_press ? (strlen(wifi_press->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_press) + 1));

        cJSON *wifi_fresh = cJSON_GetObjectItem(item, "wifi_fresh");
        if (!wifi_fresh) {
            printf("wifi_fresh is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_fresh, wifi_fresh ? (strlen(wifi_fresh->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_fresh) + 1));

        cJSON *wifi_freshing = cJSON_GetObjectItem(item, "wifi_freshing");
        if (!wifi_freshing) {
            printf("wifi_freshing is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_freshing, wifi_freshing ? (strlen(wifi_freshing->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_freshing) + 1));

        cJSON *wifi_fresh_succ = cJSON_GetObjectItem(item, "wifi_fresh_succ");
        if (!wifi_fresh_succ) {
            printf("wifi_fresh_succ is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_fresh_succ, wifi_fresh_succ ? (strlen(wifi_fresh_succ->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_fresh_succ) + 1));

        cJSON *wifi_enter_pwd = cJSON_GetObjectItem(item, "wifi_enter_pwd");
        if (!wifi_enter_pwd) {
            printf("wifi_enter_pwd is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_enter_pwd, wifi_enter_pwd ? (strlen(wifi_enter_pwd->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_enter_pwd) + 1));

        cJSON *wifi_connecting = cJSON_GetObjectItem(item, "wifi_connecting");
        if (!wifi_connecting) {
            printf("wifi_connecting is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_connecting, wifi_connecting ? (strlen(wifi_connecting->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_connecting) + 1));

        cJSON *wifi_connect_succ = cJSON_GetObjectItem(item, "wifi_connect_succ");
        if (!wifi_connect_succ) {
            printf("wifi_connect_succ is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_connect_succ, wifi_connect_succ ? (strlen(wifi_connect_succ->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_connect_succ) + 1));

        cJSON *wifi_connect_fail = cJSON_GetObjectItem(item, "wifi_connect_fail");
        if (!wifi_connect_fail) {
            printf("wifi_connect_fail is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_connect_fail, wifi_connect_fail ? (strlen(wifi_connect_fail->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_connect_fail) + 1));

        cJSON *wifi_conn_fail_info = cJSON_GetObjectItem(item, "wifi_conn_fail_info");
        if (!wifi_conn_fail_info) {
            printf("wifi_conn_fail_info is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_conn_fail_info, wifi_conn_fail_info ? (strlen(wifi_conn_fail_info->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_conn_fail_info) + 1));

        cJSON *wifi_retry_once = cJSON_GetObjectItem(item, "wifi_retry_once");
        if (!wifi_retry_once) {
            printf("wifi_retry_once is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_retry_once, wifi_retry_once ? (strlen(wifi_retry_once->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_retry_once) + 1));

        cJSON *wifi_connected = cJSON_GetObjectItem(item, "wifi_connected");
        if (!wifi_connected) {
            printf("wifi_connected is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_connected, wifi_connected ? (strlen(wifi_connected->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_connected) + 1));

        cJSON *wifi_list_connected = cJSON_GetObjectItem(item, "wifi_list_connected");
        if (!wifi_list_connected) {
            printf("wifi_list_connected is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_list_connected, wifi_list_connected ? (strlen(wifi_list_connected->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_list_connected) + 1));

        cJSON *wifi_saved = cJSON_GetObjectItem(item, "wifi_saved");
        if (!wifi_saved) {
            printf("wifi_saved is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_saved, wifi_saved ? (strlen(wifi_saved->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_saved) + 1));

        cJSON *wifi_rm_this_net = cJSON_GetObjectItem(item, "wifi_rm_this_net");
        if (!wifi_rm_this_net) {
            printf("wifi_rm_this_net is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_rm_this_net, wifi_rm_this_net ? (strlen(wifi_rm_this_net->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_rm_this_net) + 1));

        cJSON *wifi_rm_sure = cJSON_GetObjectItem(item, "wifi_rm_sure");
        if (!wifi_rm_sure) {
            printf("wifi_rm_sure is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_rm_sure, wifi_rm_sure ? (strlen(wifi_rm_sure->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_rm_sure) + 1));

        cJSON *wifi_rm = cJSON_GetObjectItem(item, "wifi_rm");
        if (!wifi_rm) {
            printf("wifi_rm is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_rm, wifi_rm ? (strlen(wifi_rm->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_rm) + 1));

        cJSON *wifi_only_2g = cJSON_GetObjectItem(item, "wifi_only_2g");
        if (!wifi_only_2g) {
            printf("wifi_only_2g is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_only_2g, wifi_only_2g ? (strlen(wifi_only_2g->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_only_2g) + 1));

        cJSON *wifi_WIFI = cJSON_GetObjectItem(item, "wifi_WIFI");
        if (!wifi_WIFI) {
            printf("wifi_WIFI is null\n");
        }
        try_sram_calloc((void **)&hdl->note.wifi_WIFI, wifi_WIFI ? (strlen(wifi_WIFI->valuestring) + 1) :
            (strlen(s_ota_note_default.wifi_WIFI) + 1));

        cJSON *bat_low = cJSON_GetObjectItem(item, "bat_low");
        if (!bat_low) {
            printf("bat_low is null\n");
        }
        try_sram_calloc((void **)&hdl->note.bat_low, bat_low ? (strlen(bat_low->valuestring) + 1) :
            (strlen(s_ota_note_default.bat_low) + 1));
        if (!get_real_ptr(hdl->note.upgrade_preparing) || !get_real_ptr(hdl->note.upgrading) || !get_real_ptr(hdl->note.upgrading_notop) ||
            !get_real_ptr(hdl->note.upgrade_succ_rebooting) || !get_real_ptr(hdl->note.upgrade_fail) || !get_real_ptr(hdl->note.upgrade_cancel) ||
            !get_real_ptr(hdl->note.upgrade_fail_retry) || !get_real_ptr(hdl->note.help_connect) || !get_real_ptr(hdl->note.ok_to_connect) || 
            !get_real_ptr(hdl->note.loading) || !get_real_ptr(hdl->note.wifi_press) || !get_real_ptr(hdl->note.wifi_fresh) || !get_real_ptr(hdl->note.wifi_freshing) ||
            !get_real_ptr(hdl->note.wifi_fresh_succ) || !get_real_ptr(hdl->note.wifi_enter_pwd) || !get_real_ptr(hdl->note.wifi_connecting) ||
            !get_real_ptr(hdl->note.wifi_connect_succ) || !get_real_ptr(hdl->note.wifi_connect_fail) || !get_real_ptr(hdl->note.wifi_conn_fail_info) ||
            !get_real_ptr(hdl->note.wifi_retry_once) || !get_real_ptr(hdl->note.wifi_connected) || !get_real_ptr(hdl->note.wifi_list_connected) ||
            !get_real_ptr(hdl->note.wifi_saved) || !get_real_ptr(hdl->note.wifi_rm_this_net) || !get_real_ptr(hdl->note.wifi_rm_sure) ||
            !get_real_ptr(hdl->note.wifi_rm) || !get_real_ptr(hdl->note.wifi_only_2g) || !get_real_ptr(hdl->note.wifi_WIFI) || !get_real_ptr(hdl->note.bat_low)) {
            printf("malloc fail %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p\n",
                hdl->note.upgrade_preparing, hdl->note.upgrading, hdl->note.upgrading_notop, hdl->note.upgrade_succ_rebooting,
                hdl->note.upgrade_fail, hdl->note.upgrade_cancel, hdl->note.upgrade_fail_retry, hdl->note.help_connect,
                hdl->note.ok_to_connect, hdl->note.loading, hdl->note.wifi_press, hdl->note.wifi_fresh, hdl->note.wifi_freshing,
                hdl->note.wifi_fresh_succ, hdl->note.wifi_enter_pwd, hdl->note.wifi_connecting, hdl->note.wifi_connect_succ,
                hdl->note.wifi_connect_fail, hdl->note.wifi_conn_fail_info, hdl->note.wifi_retry_once, hdl->note.wifi_connected,
                hdl->note.wifi_list_connected, hdl->note.wifi_saved, hdl->note.wifi_rm_this_net,
                hdl->note.wifi_rm_sure, hdl->note.wifi_rm, hdl->note.wifi_only_2g, hdl->note.wifi_WIFI, hdl->note.bat_low);
            goto exit;
        }
        strcpy(get_real_ptr(hdl->note.upgrade_preparing), upgrading ? upgrade_preparing->valuestring : s_ota_note_default.upgrade_preparing);
        strcpy(get_real_ptr(hdl->note.upgrading), upgrading ? upgrading->valuestring : s_ota_note_default.upgrading);
        strcpy(get_real_ptr(hdl->note.upgrading_notop), upgrading_notop ? upgrading_notop->valuestring : s_ota_note_default.upgrading_notop);
        strcpy(get_real_ptr(hdl->note.upgrade_succ_rebooting), upgrade_succ_rebooting ? upgrade_succ_rebooting->valuestring :
            s_ota_note_default.upgrade_succ_rebooting);
        strcpy(get_real_ptr(hdl->note.upgrade_fail), upgrade_fail ? upgrade_fail->valuestring : s_ota_note_default.upgrade_fail);
        strcpy(get_real_ptr(hdl->note.upgrade_cancel), upgrade_cancel ? upgrade_cancel->valuestring : s_ota_note_default.upgrade_cancel);
        strcpy(get_real_ptr(hdl->note.upgrade_fail_retry), upgrade_fail_retry ? upgrade_fail_retry->valuestring : s_ota_note_default.upgrade_fail_retry);
        strcpy(get_real_ptr(hdl->note.help_connect), help_connect ? help_connect->valuestring : s_ota_note_default.help_connect);
        strcpy(get_real_ptr(hdl->note.ok_to_connect), ok_to_connect ? ok_to_connect->valuestring : s_ota_note_default.ok_to_connect);
        strcpy(get_real_ptr(hdl->note.loading), loading ? loading->valuestring : s_ota_note_default.loading);
        strcpy(get_real_ptr(hdl->note.wifi_press), wifi_press ? wifi_press->valuestring : s_ota_note_default.wifi_press);
        strcpy(get_real_ptr(hdl->note.wifi_fresh), wifi_fresh ? wifi_fresh->valuestring : s_ota_note_default.wifi_fresh);
        strcpy(get_real_ptr(hdl->note.wifi_freshing), wifi_freshing ? wifi_freshing->valuestring : s_ota_note_default.wifi_freshing);
        strcpy(get_real_ptr(hdl->note.wifi_fresh_succ), wifi_fresh_succ ? wifi_fresh_succ->valuestring : s_ota_note_default.wifi_fresh_succ);
        strcpy(get_real_ptr(hdl->note.wifi_enter_pwd), wifi_enter_pwd ? wifi_enter_pwd->valuestring : s_ota_note_default.wifi_enter_pwd);
        strcpy(get_real_ptr(hdl->note.wifi_connecting), wifi_connecting ? wifi_connecting->valuestring : s_ota_note_default.wifi_connecting);
        strcpy(get_real_ptr(hdl->note.wifi_connect_succ), wifi_connect_succ ? wifi_connect_succ->valuestring : s_ota_note_default.wifi_connect_succ);
        strcpy(get_real_ptr(hdl->note.wifi_connect_fail), wifi_connect_fail ? wifi_connect_fail->valuestring : s_ota_note_default.wifi_connect_fail);
        strcpy(get_real_ptr(hdl->note.wifi_conn_fail_info), wifi_conn_fail_info ? wifi_conn_fail_info->valuestring : s_ota_note_default.wifi_conn_fail_info);
        strcpy(get_real_ptr(hdl->note.wifi_retry_once), wifi_retry_once ? wifi_retry_once->valuestring : s_ota_note_default.wifi_retry_once);
        strcpy(get_real_ptr(hdl->note.wifi_connected), wifi_connected ? wifi_connected->valuestring : s_ota_note_default.wifi_connected);
        strcpy(get_real_ptr(hdl->note.wifi_list_connected), wifi_list_connected ? wifi_list_connected->valuestring : s_ota_note_default.wifi_list_connected);
        strcpy(get_real_ptr(hdl->note.wifi_saved), wifi_saved ? wifi_saved->valuestring : s_ota_note_default.wifi_saved);
        strcpy(get_real_ptr(hdl->note.wifi_rm_this_net), wifi_rm_this_net ? wifi_rm_this_net->valuestring : s_ota_note_default.wifi_rm_this_net);
        strcpy(get_real_ptr(hdl->note.wifi_rm_sure), wifi_rm_sure ? wifi_rm_sure->valuestring : s_ota_note_default.wifi_rm_sure);
        strcpy(get_real_ptr(hdl->note.wifi_rm), wifi_rm ? wifi_rm->valuestring : s_ota_note_default.wifi_rm);
        strcpy(get_real_ptr(hdl->note.wifi_only_2g), wifi_only_2g ? wifi_only_2g->valuestring : s_ota_note_default.wifi_only_2g);
        strcpy(get_real_ptr(hdl->note.wifi_WIFI), wifi_WIFI ? wifi_WIFI->valuestring : s_ota_note_default.wifi_WIFI);
        strcpy(get_real_ptr(hdl->note.bat_low), bat_low ? bat_low->valuestring : s_ota_note_default.bat_low);
    }

    native_app_ota_create_init_page();

    item = cJSON_GetObjectItem(root, "wifi_list");
    if (item) {
        int wifi_num = cJSON_GetArraySize(item);
        cJSON *wifi_item = NULL;
        wifi_info_t wifi_info;
        for (int i = 0; i < wifi_num; i++) {
            wifi_item = cJSON_GetArrayItem(item, i);
            if (!wifi_item) {
                continue;
            }
            cJSON *ssid_item = cJSON_GetObjectItem(wifi_item, "ssid");
            cJSON *password_item = cJSON_GetObjectItem(wifi_item, "pwd");
            cJSON *sec_type_item = cJSON_GetObjectItem(wifi_item, "sec_type");
            if (!ssid_item || !password_item || !sec_type_item) {
                printf("wifi some item is null %p %p %p\n", ssid_item, password_item, sec_type_item);
                continue;
            }
            memset(&wifi_info, 0, sizeof(wifi_info_t));
            strncpy(wifi_info.ssid, ssid_item->valuestring, sizeof(wifi_info.ssid) - 1);
            strncpy(wifi_info.pwd, password_item->valuestring, sizeof(wifi_info.pwd) - 1);
            wifi_info.sec_type = sec_type_item->valueint;
            wifi_adapter_add_wifi_to_list(&wifi_info);
        }
    }

    // fw 升级信息
    item = cJSON_GetObjectItem(root, "fw");
    if (item) {
        cJSON *version_item = cJSON_GetObjectItem(item, "version");
        if (!version_item) {
            LOG_ERR("fw version not found\n");
            goto exit;
        }
        hdl->fw_handle.version = os_adapter()->calloc(1, strlen(version_item->valuestring) + 1);
        if (!hdl->fw_handle.version) {
            LOG_ERR("fw version malloc fail\n");
            goto exit;
        }
        strcpy(hdl->fw_handle.version, version_item->valuestring);

        cJSON *fw_info = cJSON_GetObjectItem(item, "fw_info");
        if (!fw_info) {
            LOG_ERR("fw_info item not found\n");
            goto exit;
        }
        int fw_num = cJSON_GetArraySize(fw_info);
        hdl->fw_handle.fw_num = fw_num;
        hdl->fw_handle.fw_info = os_adapter()->calloc(1, sizeof(fw_info_t) * fw_num);
        if (!hdl->fw_handle.fw_info) {
            LOG_ERR("calloc fw_info_t fail\n");
            upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
            goto exit;
        }

        printf("fw_num:%d\n", fw_num);
        for (int i = 0; i < fw_num; i++) {
            cJSON *fw_item = cJSON_GetArrayItem(fw_info, i);
            if (!fw_item) {
                continue;
            }
            cJSON *fex_name_item = cJSON_GetObjectItem(fw_item, "fex_name");
            cJSON *part_name_item = cJSON_GetObjectItem(fw_item, "part_name");
            cJSON *url_item = cJSON_GetObjectItem(fw_item, "url");
            cJSON *is_force_item = cJSON_GetObjectItem(fw_item, "is_force");
            cJSON *md5_item = cJSON_GetObjectItem(fw_item, "md5");
            if (!fex_name_item || !part_name_item || !url_item || !is_force_item || !md5_item) {
                LOG_ERR("fw some item is null %p %p %p %p %p\n",
                    fex_name_item, part_name_item, url_item, is_force_item, md5_item);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
                goto exit;
            }
    
            hdl->fw_handle.fw_info[i].url = os_adapter()->calloc(1, strlen(url_item->valuestring) + 1);
            if (!hdl->fw_handle.fw_info[i].url) {
                LOG_ERR("os_adapter()->calloc url failed %p\n", hdl->fw_handle.fw_info[i].url);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
                goto exit;
            }
    
            hdl->fw_handle.fw_info[i].is_force = is_force_item->valueint;
            strncpy(hdl->fw_handle.fw_info[i].fex_name, fex_name_item->valuestring, sizeof(hdl->fw_handle.fw_info[i].fex_name) - 1);
            strncpy(hdl->fw_handle.fw_info[i].part_name, part_name_item->valuestring, sizeof(hdl->fw_handle.fw_info[i].part_name) - 1);
            strncpy(hdl->fw_handle.fw_info[i].md5, md5_item->valuestring, sizeof(hdl->fw_handle.fw_info[i].md5));
            upgrade_buff_to_upper(hdl->fw_handle.fw_info[i].md5, sizeof(hdl->fw_handle.fw_info[i].md5) - 1);
            strcpy(hdl->fw_handle.fw_info[i].url, url_item->valuestring);
            hdl->fw_handle.need_upgrade = 1;
        }
    }

    // app 升级信息
    item = cJSON_GetObjectItem(root, "app");
    if (item) {
        cJSON *is_force_item = cJSON_GetObjectItem(item, "is_force");
        cJSON *file_list_item = cJSON_GetObjectItem(item, "file_list");
        cJSON *version_item = cJSON_GetObjectItem(item, "version");
        if (!file_list_item || !is_force_item || !version_item) {
            LOG_ERR("app some item is null %p %p %p\n", file_list_item, is_force_item, version_item);
            upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
            goto exit;
        }

        int file_num = cJSON_GetArraySize(file_list_item);
        if (file_num == 0) {
            printf("file num is 0\n");
            ret = 0;
            goto exit;
        }
        hdl->app_handle.file_num = file_num;
        hdl->app_handle.is_force = is_force_item->valueint;
        hdl->app_handle.file_info = os_adapter()->calloc(1, file_num * sizeof(file_info_t));
        hdl->app_handle.version = os_adapter()->calloc(1, strlen(version_item->valuestring) + 1);
        if (!hdl->app_handle.file_info || !hdl->app_handle.version) {
            LOG_ERR("os_adapter()->calloc file info failed\n");
            upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
            goto exit;
        }

        strcpy(hdl->app_handle.version, version_item->valuestring);

        cJSON *array_item = NULL;
        for (int i = 0; i < file_num; i++) {
            array_item = cJSON_GetArrayItem(file_list_item, i);
            if (!array_item) {
                LOG_ERR("array item is null %p\n", array_item);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
                goto exit;
            }

            cJSON *name = cJSON_GetObjectItem(array_item, "name");
            cJSON *md5 = cJSON_GetObjectItem(array_item, "md5");
            cJSON *url = cJSON_GetObjectItem(array_item, "url");
            cJSON *size = cJSON_GetObjectItem(array_item, "size");
            if (!name || !md5 || !url || !size) {
                LOG_ERR("file list some item is null %p %p %p %p\n", name, md5, url, size);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_JSON_NULL);
                goto exit;
            }

            hdl->app_handle.file_info[i].name = os_adapter()->calloc(1, strlen(name->valuestring) + 1);
            hdl->app_handle.file_info[i].url = os_adapter()->calloc(1, strlen(url->valuestring) + 1);
            if (!hdl->app_handle.file_info[i].name || !hdl->app_handle.file_info[i].url) {
                LOG_ERR("os_adapter()->calloc failed %p %p\n", hdl->app_handle.file_info[i].name, hdl->app_handle.file_info[i].url);
                upgrade_set_errno(UPGRADE_TYPE_COMMON, UPGRADE_ERR_MALLOC_FAIL);
                goto exit;
            }
            strcpy(hdl->app_handle.file_info[i].name, name->valuestring);
            strcpy(hdl->app_handle.file_info[i].url, url->valuestring);
            strcpy(hdl->app_handle.file_info[i].md5, md5->valuestring);
            upgrade_buff_to_upper(hdl->app_handle.file_info[i].md5, sizeof(hdl->app_handle.file_info[i].md5) - 1);
            hdl->app_handle.file_info[i].size = size->valueint;
        }
        hdl->app_handle.need_upgrade = 1;
    }

    if (upgrade_logic_organize(hdl) != 0) {
        goto exit;
    }

    ret = 0;
exit:
    if (fp) {
        fs_adapter()->fs_close(fp);
    }
    cJSON_Delete(root);
    os_adapter()->free(file_buffer);
    return ret;
}

// static int upgrade_mount_fatfs(void)
// {
//     if (fs_adapter()->get_mount_state(FS_TYPE_FAT) == 1) {
//         printf("fatfs already mount\n");
//         return 0;
//     }

//     if (fs_adapter()->mount(FS_TYPE_FAT, 0) != 0) {
//         LOG_ERR("mount fatfs failed\n");
//         return -1;
//     }
//     dev_sd_detect_init(); // sd卡检测

//     return 0;
// }

int upgrade_logic_pre_check(upgrade_handle_t *hdl, upgrade_type_t type)
{
    int ret = -1;

    mcu_clear_rx_buffer();

    // step1:电量检测
    if ((hdl->bat_chk_succ == 0 && upgrade_check_bat(hdl->method, hdl->adc_threshold, hdl->adc_charge_threshold) != 0)) {
        LOG_ERR("bat check failed\n");
        upgrade_set_errno(type, UPGRADE_ERR_BAT_LOW);
        goto exit;
    }
    hdl->bat_chk_succ = 1; // 一次完整升级只检查一次电量

    mcu_clear_rx_buffer();
    
    // step2:空间检测
    // 在固件升级之前就要检测空间是否足够，不够的话不升级固件
    if (hdl->app_handle.need_upgrade == 1 && hdl->app_handle.upgrade_status != UPGRADE_SUCC) {
        upgrade_clean_files(); // 直接删除
        if (upgrade_cal_need_space(hdl) + UPGRADE_SPACE_BUFFER_BLKCNT > upgrade_cal_remain_space(hdl)) {
            LOG_ERR("space is not enough\n");
            upgrade_set_errno(type, UPGRADE_ERR_APP_FILE_OVERFLOW);
            goto exit;
        }
    }
    ret = 0;
exit:
    return ret;
}

int upgrade_logic_init(upgrade_cb_t *cb)
{
    int ret = -1;

    if (cb) {
        s_upgrade_cb.start = cb->start;
        s_upgrade_cb.process = cb->process;
        s_upgrade_cb.finish = cb->finish;
        s_upgrade_cb.error = cb->error;
    }

    ret = 0;
    return ret;
}

int upgrade_logic_deinit(void)
{
    printf("%s %d\n", __func__, __LINE__);
    os_adapter()->free(s_upgrade_hdl.msg_file);
    try_sram_free(s_upgrade_hdl.note.upgrade_preparing);
    try_sram_free(s_upgrade_hdl.note.upgrading);
    try_sram_free(s_upgrade_hdl.note.upgrading_notop);
    try_sram_free(s_upgrade_hdl.note.upgrade_succ_rebooting);
    try_sram_free(s_upgrade_hdl.note.upgrade_fail);
    try_sram_free(s_upgrade_hdl.note.upgrade_cancel);
    try_sram_free(s_upgrade_hdl.note.upgrade_fail_retry);
    try_sram_free(s_upgrade_hdl.note.help_connect);
    try_sram_free(s_upgrade_hdl.note.ok_to_connect);
    try_sram_free(s_upgrade_hdl.note.loading);
    try_sram_free(s_upgrade_hdl.note.wifi_press);
    try_sram_free(s_upgrade_hdl.note.wifi_fresh);
    try_sram_free(s_upgrade_hdl.note.wifi_freshing);
    try_sram_free(s_upgrade_hdl.note.wifi_fresh_succ);
    try_sram_free(s_upgrade_hdl.note.wifi_enter_pwd);
    try_sram_free(s_upgrade_hdl.note.wifi_connecting);
    try_sram_free(s_upgrade_hdl.note.wifi_connect_succ);
    try_sram_free(s_upgrade_hdl.note.wifi_connect_fail);
    try_sram_free(s_upgrade_hdl.note.wifi_conn_fail_info);
    try_sram_free(s_upgrade_hdl.note.wifi_retry_once);
    try_sram_free(s_upgrade_hdl.note.wifi_connected);
    try_sram_free(s_upgrade_hdl.note.wifi_list_connected);
    try_sram_free(s_upgrade_hdl.note.wifi_saved);
    try_sram_free(s_upgrade_hdl.note.wifi_rm_this_net);
    try_sram_free(s_upgrade_hdl.note.wifi_rm_sure);
    try_sram_free(s_upgrade_hdl.note.wifi_rm);
    try_sram_free(s_upgrade_hdl.note.wifi_only_2g);
    try_sram_free(s_upgrade_hdl.note.wifi_WIFI);
    try_sram_free(s_upgrade_hdl.note.bat_low);
    os_adapter()->free(s_upgrade_hdl.max_file_buffer);
    printf("%s %d %d\n", __func__, __LINE__, s_upgrade_hdl.fw_handle.fw_num);

    os_adapter()->free(s_upgrade_hdl.fw_handle.version);
    for (int i = 0; i < s_upgrade_hdl.fw_handle.fw_num; i++) {
        os_adapter()->free(s_upgrade_hdl.fw_handle.fw_info[i].url);
    }
    printf("%s %d\n", __func__, __LINE__);
    os_adapter()->free(s_upgrade_hdl.fw_handle.fw_info);
    if (s_upgrade_hdl.fw_handle.fp) {
        fs_adapter()->fs_close(s_upgrade_hdl.fw_handle.fp);
    }
    printf("%s %d\n", __func__, __LINE__);
    os_adapter()->free(s_upgrade_hdl.fw_handle.http_param);
    os_adapter()->free(s_upgrade_hdl.fw_handle.image_buffer);
    os_adapter()->free(s_upgrade_hdl.app_handle.version);
    os_adapter()->free(s_upgrade_hdl.app_handle.http_param);
    printf("%s %d %d\n", __func__, __LINE__, s_upgrade_hdl.app_handle.file_num);
    for (int i = 0; i < s_upgrade_hdl.app_handle.file_num; i++) {
        os_adapter()->free(s_upgrade_hdl.app_handle.file_info[i].name);
        os_adapter()->free(s_upgrade_hdl.app_handle.file_info[i].url);
        os_adapter()->free(s_upgrade_hdl.app_handle.file_info[i].data);
    }
    printf("%s %d\n", __func__, __LINE__);
    os_adapter()->free(s_upgrade_hdl.app_handle.file_info);
    memset(&s_upgrade_hdl, 0, sizeof(upgrade_handle_t));
    return 0;
}

int upgrade_logic_get_config(void)
{
    int ret = -1;

    ret = upgrade_logic_get_info(&s_upgrade_hdl);
    if (ret != 0) {
        goto exit;
    }
    ret = 0;
exit:
    return ret;
}

