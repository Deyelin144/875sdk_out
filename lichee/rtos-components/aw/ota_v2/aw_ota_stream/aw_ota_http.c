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
#include "HTTPCUsr_api.h"
#include "mbedtls/mbedtls.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct ota_stream_context {
    HTTPParameters client_params;
    HTTP_CLIENT http_client_info;
    security_client cert; //for ssl
    char offset_header[32];
    unsigned offset;
    unsigned hadRead;
};
static struct ota_stream_context *ota_http_handle = NULL;

const char mbedtls_ota_cas_pem[] = "use youeself";
const size_t mbedtls_ota_cas_pem_len = sizeof(mbedtls_ota_cas_pem);

static void *ota_get_certs(void)
{
    if (ota_http_handle == NULL) {
        return NULL;
    }
    ota_http_handle->cert.pCa = (char *)mbedtls_ota_cas_pem;
    ota_http_handle->cert.nCa = mbedtls_ota_cas_pem_len;
    return &(ota_http_handle->cert);
}

void *ota_set_http_header(void)
{
    // add customer header here
    // add range
    sprintf(ota_http_handle->offset_header,
            "Range: bytes=%d-",
            ota_http_handle->offset);
    return ota_http_handle->offset_header;
}

static int ota_stream_http_init(char *url)
{
    int ret = 0;

    ota_http_handle = malloc(sizeof(struct ota_stream_context));
    if (ota_http_handle == NULL) {
        return -1;
    }
    memset(ota_http_handle, 0, sizeof(struct ota_stream_context));

    if (strncmp(url, "https://", sizeof("https://")) == 0) {
        HTTPC_Register_user_certs(ota_get_certs);
    }

    strncpy(ota_http_handle->client_params.Uri, url, HTTP_CLIENT_MAX_URL_LENGTH);

    ret = HTTPC_open(&ota_http_handle->client_params);
    if (ret != 0) {
        goto ota_stream_http_init_error;
    }

    ret = HTTPC_request(&ota_http_handle->client_params, ota_set_http_header);
    if (ret != 0) {
        goto ota_stream_http_init_error;
    }

    return 0;
ota_stream_http_init_error:
    if (ota_http_handle != NULL) {
        HTTPC_close(&ota_http_handle->client_params);

        free(ota_http_handle);
        ota_http_handle = NULL;
    }

    return -1;
}

static int ota_stream_http_exit(void)
{
    if (ota_http_handle != NULL) {
        HTTPC_close(&ota_http_handle->client_params);

        free(ota_http_handle);
        ota_http_handle = NULL;
    }
    return 0;
}

static int ota_stream_http_read(char *buff, unsigned size)
{
    int http_get_size = 0;
    int ret = -1;

    // equal to HTTPC_read
    ret = HTTPC_get(&ota_http_handle->client_params, buff, size, &http_get_size);
    if (ret != HTTP_CLIENT_SUCCESS && ret != HTTP_CLIENT_EOS) {
        http_get_size = 0;
    }
    ota_http_handle->hadRead += http_get_size;

    return http_get_size;
}

static int ota_stream_http_seek(int offset, unsigned whence)
{
    int ret = 0;
    char *pData_buf = NULL;
    unsigned pData_lenght = 0;

    if (whence == OTA_STREAM_SEEK_SET) {
        if (ota_http_handle->hadRead == offset) {
            OTA_DBG("no need seek\n");
            return 0;
        }
        ota_http_handle->offset = offset;
    } else if (whence == OTA_STREAM_SEEK_CUR) {
        ota_http_handle->offset = (ota_http_handle->hadRead + offset);
    } else if (OTA_STREAM_SEEK_END) {
        // TODO,should read Content-Length
        OTA_ERR("not support now\n");
    }

    // http seek, should reconnect
    ota_http_handle->hadRead = ota_http_handle->offset;
    HTTPC_close(&ota_http_handle->client_params);
    ret = HTTPC_open(&ota_http_handle->client_params);
    if (ret != 0) {
        OTA_ERR("seek reconnect fail\n");
        return -1;
    }
    // set seek header
    ret = HTTPC_request(&ota_http_handle->client_params, ota_set_http_header);
    if (ret != 0) {
        OTA_ERR("seek request fail\n");
        return -1;
    }

    return ret;
}

static int ota_stream_http_ctrl(void *arg, unsigned cmd)
{
    return 0;
}

struct OtaStreamOps OtaHttpOps = {
    .ota_init = ota_stream_http_init,
    .ota_exit = ota_stream_http_exit,
    .ota_read = ota_stream_http_read,
    .ota_seek = ota_stream_http_seek,
    .ota_ctrl = ota_stream_http_ctrl,
};