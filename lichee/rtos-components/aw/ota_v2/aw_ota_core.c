/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <awlog.h>
//#include <errno.h>
#include <console.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <flag.h>
#include <aw_upgrade.h>
#include <aw_types.h>
#include "ota_debug.h"
#include "aw_ota_core.h"

extern int cmd_reboot(int argc, char **argv);

int ota_get_key(struct OtaKeyValuePairS *pairs, unsigned pair_count)
{
    struct OtaKeyValuePairS *ota_pairs = NULL;

    if (aw_flag_open()) {
        OTA_ERR("aw_flag_open fail\n");
        return -1;
    }

    ota_pairs = pairs;
    for (int i = 0; i < pair_count; i++) {
        strncpy(ota_pairs->val, aw_get_flag((const char *)ota_pairs->key), OTA_MAX_VAL_LENGHT);

        ota_pairs++;
    }

    aw_flag_close();
    return 0;
}

int ota_set_key(struct OtaKeyValuePairS *pairs, unsigned pair_count)
{
    int ret = -1;
    struct OtaKeyValuePairS *ota_pairs = NULL;

    if (aw_flag_open()) {
        OTA_ERR("aw_flag_open fail\n");
        return -1;
    }

    ota_pairs = pairs;
    for (int i = 0; i < pair_count; i++) {
        ret = aw_flag_write(ota_pairs->key, ota_pairs->val);
        if (ret)
            goto ota_set_key_out;

        ota_pairs++;
    }

    /* flush to flash */
    aw_flag_flush();
    OTA_DBG("reset env sucess!\n");

ota_set_key_out:
    aw_flag_close();
    return ret;
}

struct OtaStreamOps *ota_stream_init(char *url)
{
    struct OtaStreamOps *ota_stream = NULL;

    if (url == NULL)
        return NULL;
#ifdef CONFIG_COMPONENTS_AW_OTA_HTTP
    if (strncmp(url, "http://", 7) == 0) {
        ota_stream = &OtaHttpOps;
    }
#endif
#ifdef CONFIG_COMPONENTS_AW_OTA_FILE
    if (strncmp(url, "file://", 7) == 0) {
        ota_stream = &OtaFileOps;
    }
#endif
#ifdef CONFIG_COMPONENTS_AW_OTA_UART
    if (strncmp(url, "uart://", 7) == 0) {
        ota_stream = &OtaUartOps;
    }
#endif

    if (ota_stream == NULL) {
        OTA_ERR("unknow protocol --%s\n", url);
#ifdef CONFIG_COMPONENTS_AW_OTA_FILE
        OTA_LOG(1, "try file stream\n");
        ota_stream = &OtaFileOps;
#endif
    }

    if (ota_stream != NULL) {
        if (ota_stream->ota_init(url) < 0) {
            ota_stream = NULL;
        }
    }

    return ota_stream;
}

/*
* change to another system
* A-->B, B-->A
* app->rec, rec-->app
*/
int ota_change_load_reboot(void)
{
    struct OtaKeyValuePairS *env_load = NULL;

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
        strcpy((char *)&(env_load[0].val), (const char *)&(env_load[2].val));
    } else {
        strcpy((char *)&(env_load[0].val), (const char *)&(env_load[1].val));
    }

    ota_set_key(env_load, 1);
    cmd_reboot(0, NULL);
    free(env_load);
    return 0; // no return, add for build warning
}
