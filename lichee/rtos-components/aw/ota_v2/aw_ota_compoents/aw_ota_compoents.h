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
#ifndef _AW_OTA_COMPOENTS_H_
#define _AW_OTA_COMPOENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "aw_ota_core.h"
#include "md5.h"

#define OTA_MD5_LEN (32)

#define MAX_OTA_MESSAGE_COUNT 6

struct awOtaUpdateChunkParam {
    char md5[OTA_MD5_LEN + 1];
    unsigned size;
    char device[64]; //pack shell max lenght is 32, so 64 is enough
};

struct awOtaPackInfo {
    struct awOtaUpdateChunkParam chunk[MAX_OTA_MESSAGE_COUNT];
    struct OtaKeyValuePairS pairs[MAX_OTA_MESSAGE_COUNT];
};

struct awParserContext {
    struct OtaStreamOps *stream;
    char *raw_info;
    struct awOtaPackInfo dec_info;
    char chunk_nun;
    char env_num;
};

enum _ota_process {
    OTA_PROCESS_UPDATE_START = 0,
    OTA_PROCESS_UPDATE_INC,
    OTA_PROCESS_UPDATE_FINISH,
    OTA_PROCESS_CHECK_START,
    OTA_PROCESS_CHECK_INC,
    OTA_PROCESS_CHECK_FINISH,
    OTA_PROCESS_FAIL,
};

struct _ota_procrss {
    unsigned chunk_process;
    unsigned total_process;
};

struct awParserContext *aw_ota_parser_init(struct OtaStreamOps *stream);
int aw_ota_parser_probe(void);
int aw_ota_parser_decode(void);
int aw_ota_parser_exit(void);

int aw_ota_update_init(struct OtaStreamOps *stream, struct awParserContext *parser);
int aw_ota_update(void);
int aw_ota_update_exit(void);
int aw_ota_check_after_update(void);

extern void OTA_MD5Init(struct MD5Context *ctx);
extern void OTA_MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);
extern void OTA_MD5Final(unsigned char digest[16], struct MD5Context *ctx);

int ota_init(char *url);
int ota_update(void);

#ifdef __cplusplus
}
#endif

#endif