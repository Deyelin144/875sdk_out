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
#include "aw_ota_compoents.h"
#include "ota_debug.h"
#include "console.h"

static struct OtaStreamOps *ota_stream = NULL;

int ota_init(char *url)
{
    struct awParserContext *ota_parser = NULL;

    // 1. init stream
    ota_stream = ota_stream_init(url);
    if (ota_stream == NULL) {
        OTA_ERR("stream init fail -- %s\n", url);
        return -1;
    }
    // 2. init parser
    ota_parser = aw_ota_parser_init(ota_stream);
    if (ota_parser == NULL) {
        OtaStreamExit(ota_stream);
        OTA_ERR("parser init fail\n");
        return -1;
    }
    // 3. check pack
    if (aw_ota_parser_probe() < 0) {
        aw_ota_parser_exit();
        OtaStreamExit(ota_stream);
        OTA_ERR("update pack fail\n");
        return -1;
    }

    // 4. init update
    aw_ota_update_init(ota_stream, ota_parser);

    OTA_LOG(1, "===ota init succeed!===\n");
    return 0;
}

int ota_update(void)
{
    int try_time = 0;
    // 1. get update info from parser
    aw_ota_parser_decode();
    // 2. update
    for (try_time = 0; try_time < 3; try_time++) {
        if (aw_ota_update() == 0)
            break;
    }
    if (try_time >= 3) {
        goto ota_update_error;
    }
    // 3. check and reboot
    if (aw_ota_check_after_update() == 0) {
        ota_change_load_reboot();
    }
ota_update_error:
    // 4. maybe fail, free
    OTA_ERR("OTA update fail\n");
    aw_ota_update_exit();
    aw_ota_parser_exit();
    OtaStreamExit(ota_stream);
    ota_stream = NULL;
    return 0;
}

static int aw_ota_v2_cmd(int argc, char **argv)
{
    if (argc != 2) {
        OTA_ERR("please input aw_ota_v2 <url>\n");
        OTA_ERR("----------example:aw_ota_v2 data/ota-package\n");
        return -1;
    }

    if (ota_init(argv[1]) == 0) {
        ota_update();
    }

    return 0;
}
FINSH_FUNCTION_EXPORT_CMD(aw_ota_v2_cmd, aw_ota_v2, Tina RTOS ota ver2);
