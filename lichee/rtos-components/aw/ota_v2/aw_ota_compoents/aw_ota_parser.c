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
#include "aw_ota_compoents.h"
#include "ota_debug.h"

static struct awParserContext *awParserHandle = NULL;

const static char parser_delim[3] = { 0x0a, 0x09, 0x00 };

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_AB
#define OTA_CURRENT_LOAD_A 0
#define OTA_CURRENT_LOAD_B 1
const static char step_delim[] = "[step2]";

static int ota_get_current_load(void)
{
    struct OtaKeyValuePairS *env_load = NULL;
    int ret = 0;

    env_load = malloc(sizeof(struct OtaKeyValuePairS) * 3);
    if (env_load == NULL) {
        return -1;
    }
    memset(env_load, 0, sizeof(struct OtaKeyValuePairS) * 3);

    strcpy((char *)&(env_load[0].key), "loadparts");
    strcpy((char *)&(env_load[1].key), "loadpartA");
    strcpy((char *)&(env_load[2].key), "loadpartB");

    ota_get_key(env_load, 3);
    OTA_DBG("%s %s\n", &(env_load[0].key), &(env_load[0].val));
    OTA_DBG("%s %s\n", &(env_load[1].key), &(env_load[1].val));
    OTA_DBG("%s %s\n", &(env_load[2].key), &(env_load[2].val));

    if (strcmp((const char *)&(env_load[0].val), (const char *)&(env_load[1].val)) == 0) {
        ret = OTA_CURRENT_LOAD_A;
    } else {
        ret = OTA_CURRENT_LOAD_B;
    }

    free(env_load);
    return ret;
}
#endif

struct awParserContext *aw_ota_parser_init(struct OtaStreamOps *stream)
{
    if (stream == NULL) {
        return NULL;
    }
    awParserHandle = malloc(sizeof(struct awParserContext));
    if (awParserHandle == NULL) {
        return NULL;
    }
    memset(awParserHandle, 0, sizeof(struct awParserContext));

    awParserHandle->stream = stream;
    return awParserHandle;
}

int aw_ota_parser_probe(void)
{
    char *parser_info = NULL;
    char pack_md5[OTA_MD5_LEN + 1] = { 0 };
    unsigned char decrypt[16] = { 0 };
    unsigned char cal_md5[OTA_MD5_LEN + 1] = { 0 };

    parser_info = malloc(CONFIG_COMPONENTS_AW_OTA_INFO_SIZE);
    if (parser_info == NULL) {
        return -1;
    }
    memset(parser_info, 0, CONFIG_COMPONENTS_AW_OTA_INFO_SIZE);
    // get md5
    OtaStreamSeek(awParserHandle->stream, 0, OTA_STREAM_SEEK_SET);
    OtaStreamRead(awParserHandle->stream, pack_md5, OTA_MD5_LEN);
    // get update info
    OtaStreamRead(awParserHandle->stream, parser_info, CONFIG_COMPONENTS_AW_OTA_INFO_SIZE);
    OTA_DBG("ota pack md5:%s\n", pack_md5);
    OTA_DBG("ota pack info:\n%s\n", parser_info);

    // check info
    md5(parser_info, CONFIG_COMPONENTS_AW_OTA_INFO_SIZE, decrypt);
    for (int i = 0; i < 16; i++) {
        sprintf((cal_md5 + (2 * i)), "%02x", decrypt[i]);
    }
    OTA_DBG("cal ota pack md5:%s\n", cal_md5);
    if (strncmp(pack_md5, cal_md5, OTA_MD5_LEN) != 0) {
        free(parser_info);
        OTA_ERR("pack header md5 check fail\n");
        return -1;
    }

#ifdef CONFIG_COMPONENTS_AW_OTA_V2_AB
    char *info_delim = NULL;

    info_delim = strstr(parser_info, step_delim);
    if (info_delim == NULL) {
        OTA_ERR("ab parser step file error\n");
        free(parser_info);
        return -1;
    }

    if (ota_get_current_load() == OTA_CURRENT_LOAD_A) {
        info_delim[0] = '\0';
    } else {
        memset(parser_info, 0, (info_delim - parser_info));
        memmove(parser_info, info_delim, (info_delim - parser_info + 1));
    }
    OTA_DBG("after ota pack info:\n%s\n", parser_info);
#endif
    awParserHandle->raw_info = parser_info;
    return 0;
}

static int aw_ota_quoted_string(char *target, const char *input, unsigned max_size)
{
    // input must be xxx="yyyy" or xxx:"yyyy"
    char *p_start = NULL;
    char *p_end = NULL;
    unsigned target_length = 0;

    p_start = strchr(input, '"');
    if (p_start == NULL) {
        return -1;
    }
    p_start++;

    p_end = strchr(p_start, '"');
    if (p_end == NULL) {
        return -1;
    }

    target_length = p_end - p_start;
    if (target_length > (max_size - 1)) {
        return -1;
    }

    strncpy(target, p_start, target_length);
    target[target_length] = '\0'; // no need, is init in aw_ota_parser_init

    return 0;
}

static int aw_ota_env_string(struct OtaKeyValuePairS *pairs, const char *input)
{
    // input must be env:xxxx=yyyyyy
    char *p_equal = NULL;
    char *p_end = NULL;
    unsigned key_length = 0, value_length = 0;

    p_equal = strchr(input, '=');
    if (p_equal == NULL) {
        return -1;
    }
    key_length = (p_equal - input - 4);
    strncpy(pairs->key, (input + 4), key_length);
    pairs->key[key_length] = '\0'; // no need, is init in aw_ota_parser_init
    p_equal++;

    if (*p_equal == '\0') {
        return 0;
    }

    value_length = p_end - p_equal;
    strcpy(pairs->val, p_equal);

    return 0;
}

int aw_ota_parser_decode(void)
{
    char *token = NULL;
    char *info_leak = NULL;
    struct awOtaPackInfo *p_info = &(awParserHandle->dec_info);
    char value_buff[16] = { 0 };
    char awParserChunkNum = 0, awParserEnvNum = 0;

    // token = strtok(awParserHandle->raw_info, parser_delim); // it maybe memleak
    token = strtok_r(awParserHandle->raw_info, parser_delim, &info_leak);

    while (token != NULL) {
        if (strstr(token, "image-md5") == token) {
            aw_ota_quoted_string(p_info->chunk[awParserChunkNum].md5, token, (OTA_MD5_LEN + 1));
            OTA_DBG("%-18s:<%s>\n", "get chunk md5", p_info->chunk[awParserChunkNum].md5);
        } else if (strstr(token, "image-size") == token) {
            aw_ota_quoted_string(value_buff, token, 16);
            p_info->chunk[awParserChunkNum].size = atoi(value_buff);
            OTA_DBG("%-18s:<%d>\n", "get chunk size", p_info->chunk[awParserChunkNum].size);
        } else if (strstr(token, "device") == token) {
            aw_ota_quoted_string(p_info->chunk[awParserChunkNum].device, token, 64);
            OTA_DBG("%-18s:<%s>\n", "get chunk device", p_info->chunk[awParserChunkNum].device);
            awParserChunkNum++;
        } else if (strstr(token, "env") == token) {
            aw_ota_env_string(&(p_info->pairs[awParserEnvNum]), token);
            OTA_DBG("get env key: <%s>\tvalue <%s>\n", p_info->pairs[awParserEnvNum].key,
                    p_info->pairs[awParserEnvNum].val);
            awParserEnvNum++;
        } else {
        } // skip
        token = strtok_r(NULL, parser_delim, &info_leak);
    }

    OTA_DBG("total chunk %d, total env %d\n", awParserChunkNum, awParserEnvNum);
    awParserHandle->chunk_nun = awParserChunkNum;
    awParserHandle->env_num = awParserEnvNum;
    return 0;
}

int aw_ota_parser_exit(void)
{
    if (awParserHandle->raw_info != NULL) {
        free(awParserHandle->raw_info);
    }

    free(awParserHandle);
    awParserHandle = NULL;

    return 0;
}