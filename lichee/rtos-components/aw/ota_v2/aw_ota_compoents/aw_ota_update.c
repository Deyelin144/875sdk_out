/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "aw_ota_compoents.h"
#include "ota_debug.h"

struct ota_update_context {
    struct OtaStreamOps *stream;
    struct awParserContext *parser;
    int sum_pack_size;
    int read_size_sum; //has been update
};
struct ota_update_context *upHandle = NULL;

__attribute__((weak)) int ota_process_callback(unsigned status, void *arg)
{
    char *dev_name;
    struct _ota_procrss *progressBar;

    switch (status) {
    case OTA_PROCESS_UPDATE_START:
        dev_name = arg;
        OTA_LOG(1, "---%s update start\n", dev_name);
        break;
    case OTA_PROCESS_UPDATE_INC:
        progressBar = arg;
        OTA_LOG(1, "process[chunk/total] -- [%d%%/%d%%]\r", progressBar->chunk_process,
               progressBar->total_process);
        break;
    case OTA_PROCESS_UPDATE_FINISH:
        OTA_LOG(1, "\n---%s update finish\n", dev_name);
        break;
    case OTA_PROCESS_CHECK_START:
        dev_name = arg;
        OTA_LOG(1, "\n---%s check start\n", dev_name);
        break;
    case OTA_PROCESS_CHECK_INC:
        progressBar = arg;
        OTA_LOG(1, "check[chunk/total] -- [%d%%/%d%%]\r", progressBar->chunk_process,
               progressBar->total_process);
        break;
    case OTA_PROCESS_CHECK_FINISH:
        OTA_LOG(1, "\n---%s check finish\n", dev_name);
        break;
    case OTA_PROCESS_FAIL:
        OTA_LOG(1, "\n---%s update fail\n", dev_name);
        break;
    }
}

static void inline put_ota_cb(unsigned status, void *arg)
{
    ota_process_callback(status, arg);
}

int aw_ota_update_init(struct OtaStreamOps *stream, struct awParserContext *parser)
{
    upHandle = malloc(sizeof(struct ota_update_context));
    if (upHandle == NULL) {
        return -1;
    }
    memset(upHandle, 0, sizeof(struct ota_update_context));

    upHandle->stream = stream;
    upHandle->parser = parser;
    return 0;
}

int aw_ota_update_exit(void)
{
    free(upHandle);
    upHandle = NULL;

    return 0;
}

int aw_ota_update(void)
{
#define PER_UPDATE_SIZE 4096 // more date update faster, but processbar reduce accuarcy
    int update_fd = -1;
    int read_size = 0;

    int chunk_pack_size = 0, read_size_sum = 0;

    int write_size = 0;
    struct _ota_procrss progressBar = { 0 };
    int ret = 0;

    char *u_buff = malloc(PER_UPDATE_SIZE);
    if (u_buff == NULL) {
        OTA_ERR("malloc fail\n");
        return -1;
    }
    memset(u_buff, 0, PER_UPDATE_SIZE);
    // get total update size
    for (int i = 0; i < upHandle->parser->chunk_nun; i++) {
        upHandle->sum_pack_size += upHandle->parser->dec_info.chunk[i].size;
    }

    OtaStreamSeek(upHandle->stream, (CONFIG_COMPONENTS_AW_OTA_INFO_SIZE + OTA_MD5_LEN),
                  OTA_STREAM_SEEK_SET);
    for (int i = 0; i < upHandle->parser->chunk_nun; i++) {
        update_fd = open(upHandle->parser->dec_info.chunk[i].device, O_WRONLY);
        if (update_fd < 0) {
            OTA_ERR("%s open fail\n", upHandle->parser->dec_info.chunk[i].device);
            free(u_buff);
            return -1;
        }
        chunk_pack_size = upHandle->parser->dec_info.chunk[i].size;
        // start update one chunk
        put_ota_cb(OTA_PROCESS_UPDATE_START, upHandle->parser->dec_info.chunk[i].device);
        for (read_size = 0, read_size_sum = 0; read_size_sum < chunk_pack_size;) {
            read_size = PER_UPDATE_SIZE;
            if (read_size > (chunk_pack_size - read_size_sum)) {
                read_size = (chunk_pack_size - read_size_sum);
            }

            read_size = OtaStreamRead(upHandle->stream, u_buff, read_size);
            if (read_size == 0)
                break;

            write_size = write(update_fd, u_buff, read_size);
            if (write_size != read_size) {
                put_ota_cb(OTA_PROCESS_FAIL, upHandle->parser->dec_info.chunk[i].device);
                OTA_ERR("write error\n");
                ret = -1;
                break;
            }
            // report process
            read_size_sum += read_size;
            upHandle->read_size_sum += read_size;

            progressBar.chunk_process = read_size_sum * 100 / chunk_pack_size;
            progressBar.total_process = upHandle->read_size_sum * 100 / upHandle->sum_pack_size;

            put_ota_cb(OTA_PROCESS_UPDATE_INC, &progressBar);
        }
        put_ota_cb(OTA_PROCESS_UPDATE_FINISH, upHandle->parser->dec_info.chunk[i].device);
        close(update_fd);
        if (ret < 0) {
            free(u_buff);
            return -1;
        }
    }

    free(u_buff);
    return 0;
}

int aw_ota_md5sum(char *path, unsigned int file_size, unsigned char decrypt[16])
{
#define MD5SUM_BUFFER_SIZE 4096
    int device_fd = -1;
    unsigned char *u_buff = NULL;
    struct MD5Context context;

    int read_size = 0;
    int read_size_sum = 0;

    struct _ota_procrss progressBar = { 0 };

    u_buff = (unsigned char *)malloc(MD5SUM_BUFFER_SIZE);
    if (u_buff == NULL) {
        return -1;
    }

    device_fd = open(path, O_RDONLY);
    if (!device_fd) {
        free(u_buff);
        return -1;
    }

    OTA_MD5Init(&context);
    put_ota_cb(OTA_PROCESS_CHECK_START, path);
    for (read_size = 0, read_size_sum = 0; read_size_sum < file_size;) {
        read_size = MD5SUM_BUFFER_SIZE;
        if (read_size > (file_size - read_size_sum)) {
            read_size = (file_size - read_size_sum);
        }

        read_size = read(device_fd, u_buff, read_size);
        if (read_size == 0)
            break;

        OTA_MD5Update(&context, u_buff, read_size);
        // report process
        read_size_sum += read_size;
        upHandle->read_size_sum += read_size;

        progressBar.chunk_process = read_size_sum * 100 / file_size;
        progressBar.total_process = upHandle->read_size_sum * 100 / upHandle->sum_pack_size;

        put_ota_cb(OTA_PROCESS_CHECK_INC, &progressBar);
    }

    put_ota_cb(OTA_PROCESS_CHECK_FINISH, path);
    OTA_MD5Final(decrypt, &context);
    close(device_fd);
    free(u_buff);

    return 0;
}

int aw_ota_check_after_update(void)
{
    unsigned char decrypt[16] = { 0 };
    unsigned char cal_md5[OTA_MD5_LEN + 1] = { 0 };

    char *pack_name = NULL;
    char *pack_md5 = NULL;
    unsigned pack_size = 0;

    struct OtaKeyValuePairS Pairs = { 0 };

    upHandle->read_size_sum = 0;

    for (int i = 0; i < upHandle->parser->chunk_nun; i++) {
        pack_name = upHandle->parser->dec_info.chunk[i].device;
        pack_size = upHandle->parser->dec_info.chunk[i].size;
        pack_md5 = upHandle->parser->dec_info.chunk[i].md5;

        // get_md5sum(pack_name, pack_size, decrypt);//I don't process
        if (aw_ota_md5sum(pack_name, pack_size, decrypt) < 0) {
            OTA_ERR("fet pack header md5 fail\n");
            return -1;
        }

        for (int j = 0; j < 16; j++) {
            sprintf((cal_md5 + (2 * j)), "%02x", decrypt[j]);
        }
        OTA_DBG("%s - cal ota pack md5:%s\n", pack_name, cal_md5);

        if (strncmp(pack_md5, cal_md5, OTA_MD5_LEN) != 0) {
            OTA_ERR("pack header md5 check fail\n");
            return -1;
        }
    }
    OTA_DBG("check pass\n");

    if (upHandle->parser->env_num != 0) {
        if (ota_set_key(upHandle->parser->dec_info.pairs, upHandle->parser->env_num) < 0) {
            OTA_ERR("pset env fail\n");
            return -1;
        }
    }
    return 0;
}