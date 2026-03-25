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
#include "aw_ota_stream.h"
#include "ota_debug.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct ota_stream_context {
    int fd;
#define MAX_FILE_STREAM_URL 256
    char url[MAX_FILE_STREAM_URL];
};
static struct ota_stream_context *ota_file_handle = NULL;

static int ota_stream_file_init(char *url)
{
    if (ota_file_handle != NULL) {
        OTA_ERR("stream already init\n");
        return -1;
    }

    ota_file_handle = malloc(sizeof(struct ota_stream_context));
    if (ota_file_handle == NULL) {
        return -1;
    }
    memset(ota_file_handle, 0, sizeof(struct ota_stream_context));

    if (strncmp(url, "file://", 7) == 0) {
        strcpy(ota_file_handle->url, (url + 7));
    } else {
        strcpy(ota_file_handle->url, url);
    }
    ota_file_handle->fd = open(ota_file_handle->url, O_RDONLY);
    if (ota_file_handle->fd < 0) {
        return -1;
    }
    return 0;
}

static int ota_stream_file_exit(void)
{
    close(ota_file_handle->fd);
    free(ota_file_handle);
    ota_file_handle = NULL;
    return 0;
}

static int ota_stream_file_read(char *buff, unsigned size)
{
    return read(ota_file_handle->fd, buff, size);
}

static int ota_stream_file_seek(int offset, unsigned whence)
{
    int ret = 0;
    if (whence == OTA_STREAM_SEEK_SET) {
        ret = lseek(ota_file_handle->fd, offset, SEEK_SET);
    } else if (whence == OTA_STREAM_SEEK_CUR) {
        ret = lseek(ota_file_handle->fd, offset, SEEK_CUR);
    } else if (OTA_STREAM_SEEK_END) {
        ret = lseek(ota_file_handle->fd, offset, SEEK_END);
    }
    return ret;
}

struct OtaStreamOps OtaFileOps = {
    .ota_init = ota_stream_file_init,
    .ota_exit = ota_stream_file_exit,
    .ota_read = ota_stream_file_read,
    .ota_seek = ota_stream_file_seek,
};