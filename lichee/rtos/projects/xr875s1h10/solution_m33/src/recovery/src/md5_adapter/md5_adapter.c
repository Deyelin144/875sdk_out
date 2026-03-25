
#include "md5_adapter.h"
#include <stdio.h>
#include "fs_adapter/fs_adapter.h"
#include "os_adapter/os_adapter.h"
#include "md5.h"

void OTA_MD5Init(struct MD5Context *ctx);
void OTA_MD5Final(unsigned char digest[16], struct MD5Context *ctx);
void OTA_MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);

void *md5_adapter_init(void)
{
    void *hdl = NULL;
    int ret = -1;

    hdl = os_adapter()->calloc(1, sizeof(struct MD5Context));
    if (!hdl) {
        printf("md5 hdl malloc fail\n");
        goto exit;
    }

    OTA_MD5Init(hdl);
    return hdl;
    // hdl = os_adapter()->calloc(1, sizeof(mbedtls_md5_context));
    // if (!hdl) {
    //     printf("md5 hdl malloc fail\n");
    //     goto exit;
    // }

    // mbedtls_md5_init((mbedtls_md5_context *)hdl);
    // mbedtls_md5_starts((mbedtls_md5_context *)hdl);
    ret = 0;

exit:
    return ret ? NULL : hdl;
}

int md5_adapter_update(void *hdl, char *data, int in_len)
{
    int ret = -1;

    // mbedtls_md5_update(hdl, (const unsigned char *)data, in_len);
    OTA_MD5Update(hdl, (const unsigned char *)data, in_len);

    ret = 0;
    return ret;
}

void md5_adapter_finish(void **hdl, unsigned char *out)
{
    unsigned char buf[MD5_LEN] = {0};

    // mbedtls_md5_finish(*hdl, buf);
    // mbedtls_md5_free(*hdl);
    OTA_MD5Final(buf, *hdl);
    os_adapter()->free(*hdl);
    *hdl = NULL;

    for (int i = 0; i < MD5_LEN; i++) {
        sprintf((char *)&out[i * 2], "%02X", buf[i]);
    }
}

int md5_adapter_get_file_md5(char *file_path, unsigned char *out)
{
    void *hdl = NULL;
    int ret = -1;
    char *swap_buf = NULL;
    void *fp = NULL;
    unsigned int br = 0;
    const int  buf_size = 8192;

    hdl = md5_adapter_init();
    if (!hdl) {
        printf("md5 hdl is null\n");
        goto exit;
    }

    fs_info_t info = {0};
    if (fs_adapter()->stat(file_path, &info) != 0) {
        printf("md5 file stat fail\n");
        ret = -1;
        goto exit;
    }
    os_adapter()->free(info.name_ptr);
    swap_buf = os_adapter()->calloc(1, buf_size);
    if (!swap_buf) {
        printf("md5 swap buf malloc fail\n");
        ret = -1;
        goto exit;
    }

    fp = fs_adapter()->fs_open(file_path, UNIT_FS_RDONLY);
    if (!fp) {
        printf("md5 file open fail\n");
        ret = -1;
        goto exit;
    }
    for (int i = 0; i < info.size; i += buf_size) {
        if (fs_adapter()->fs_read(fp, swap_buf, buf_size, &br) != 0) {
            printf("md5 file read fail\n");
            ret = -1;
            goto exit;
        }

        md5_adapter_update(hdl, swap_buf, br);
        if (fs_adapter()->fs_eof(fp)) {
            break;
        }
    }

    ret = 0;
exit:
    if (hdl) {
        md5_adapter_finish(&hdl, out);
    }
    if (fp) {
        fs_adapter()->fs_close(fp);
    }
    os_adapter()->free(swap_buf);
    return ret;
}
