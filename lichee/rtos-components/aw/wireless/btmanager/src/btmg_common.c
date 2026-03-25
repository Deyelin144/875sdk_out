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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hal_time.h>

#include "xr_bt_defs.h"
#include "btmg_common.h"
#include "btmg_log.h"
#include "btmg_a2dp_sink.h"
#include "btmg_a2dp_source.h"
#include "btmg_spp_client.h"
#include "btmg_spp_server.h"
#include "btmg_hfp_hf.h"
#include "btmg_hfp_ag.h"
#include "btmg_hid_device.h"

#define INTERVAL_TIME_NUM 5

struct interval_time {
    uint64_t start;
    void *tag;
    bool enable;
};

static struct interval_time diff_time[INTERVAL_TIME_NUM];

uint64_t btmg_interval_time(void *tag, uint64_t expect_time_ms)
{
    static bool init = false;

    int i;
    if (init == false) {
        memset(diff_time, 0, sizeof(diff_time));
        for (i = 0; i < INTERVAL_TIME_NUM; i++) {
            diff_time[i].enable = false;
            diff_time[i].tag = NULL;
        }
        init = true;
    }

    for (i = 0; i < INTERVAL_TIME_NUM; i++) {
        if (diff_time[i].enable == true && diff_time[i].tag == tag)
            break;
    }

    if (i >= INTERVAL_TIME_NUM) {
        for (i = 0; i < INTERVAL_TIME_NUM; i++) {
            if (diff_time[i].enable == false && diff_time[i].tag == NULL) {
                diff_time[i].enable = true;
                diff_time[i].tag = tag;
                //diff_time[i].start = XR_OS_GetTicks();
                diff_time[i].start = hal_gettime_ns();
                return 0;
            }
        }
    }

    for (i = 0; i < INTERVAL_TIME_NUM; i++) {
        if (diff_time[i].enable == true && diff_time[i].tag == tag) {
            uint64_t cur_time;
            uint64_t t = 0;
            //cur_time = XR_OS_GetTicks();
            cur_time = hal_gettime_ns();
            t = (cur_time - diff_time[i].start + t) / 1000000 ;

            if (t >= expect_time_ms) {
                //diff_time[i].start = XR_OS_GetTicks();
                diff_time[i].start = hal_gettime_ns();
                return t;
            }
        }
    }
    return 0;
}


int str2bda(const char *strmac, xr_bd_addr_t bda)
{
    uint8_t i;
    uint8_t *p;
    char *str, *next;

    if (strmac == NULL)
        return -1;

    p = (uint8_t *)bda;
    str = (char *)strmac;

    for (i = 0; i < 6; ++i) {
        p[i] = str ? strtoul(str, &next, 16) : 0;
        if (str)
            str = (*next) ? (next + 1) : next;
    }

    return 0;
}

void bda2str(xr_bd_addr_t bda, const char *bda_str)
{
    if (bda == NULL || bda_str == NULL) {
        return;
    }

    uint8_t *p = bda;
    sprintf((uint8_t *)bda_str, "%02x:%02x:%02x:%02x:%02x:%02x", p[0], p[1], p[2], p[3], p[4],
            p[5]);
}

static const char *cmd_to_name(int cmd)
{
    switch (cmd) {
    case A2DP_SRC_DEV:
        return "A2DP_SRC_DEV";
    case A2DP_SNK_DEV:
        return "A2DP_SNK_DEV";
    case SPP_CLIENT_DEV:
        return "SPP_CLIENT_DEV";
    case HFP_HF_DEV:
        return "HFP_HF_DEV";
    case HFP_AG_DEV:
        return "HFP_AG_DEV";
    default:
        return "UNKNOWN CMD";
    }
    return "NULL";
}

bool btmg_disconnect_dev_list(dev_list_t *dev_list)
{
    dev_node_t *dev_node = NULL;
    int status_err = 0;

    if (!dev_list) {
        BTMG_ERROR("dev_list is null");
        return false;
    }

    if (dev_list->list_cleared) {
        BTMG_ERROR("dev_list is cleared, nothing could be done");
        return false;
    }

    dev_node = dev_list->head;

    while (dev_node != NULL) {
        dev_list->sem_flag = 1;
#ifdef CONFIG_BT_A2DP_ENABLE
        if (dev_node->profile & A2DP_SRC_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(A2DP_SRC_DEV));
            status_err = bt_a2dp_source_disconnect(dev_node->dev_addr);
            if (status_err == 0) {
                if (XR_OS_SemaphoreWait(&dev_list->sem, 8000) != XR_OS_OK) {
                    BTMG_ERROR("A2DP_SRC_DEV SemaphoreWait Fail!");
                } else {
                    BTMG_INFO("A2DP_SRC_DEV SemaphoreWait OK!");
                }
            }
            BTMG_INFO("");
        }
        if (dev_node->profile & A2DP_SNK_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(A2DP_SNK_DEV));
            status_err = bt_a2dp_sink_disconnect(dev_node->dev_addr);
            if (status_err == 0) {
                if (XR_OS_SemaphoreWait(&dev_list->sem, 8000) != XR_OS_OK) {
                    BTMG_ERROR("A2DP_SNK_DEV SemaphoreWait Fail!");
                } else {
                    BTMG_INFO("A2DP_SNK_DEV SemaphoreWait OK!");
                }
            }
            BTMG_INFO("");
        }
#endif
#ifdef CONFIG_BT_SPP_ENABLED
        if (dev_node->profile & SPP_CLIENT_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(SPP_CLIENT_DEV));
            status_err = bt_sppc_disconnect(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("SPP_CLIENT_DEV SemaphoreWait OK!");
        }
        if (dev_node->profile & SPP_SERVER_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(SPP_SERVER_DEV));
            status_err = bt_spps_disconnect(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("SPP_SERVER_DEV SemaphoreWait OK!");
        }
#endif

#ifdef CONFIG_BT_HFP_CLIENT_ENABLE
        if (dev_node->profile & HFP_HF_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(HFP_HF_DEV));
            status_err = bt_hfp_hf_disconnect_audio(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("HFP_HF_DEV audio SemaphoreWait OK!");
            status_err = bt_hfp_hf_disconnect(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("HFP_HF_DEV SemaphoreWait OK!");
        }
#endif

#ifdef CONFIG_BT_HFP_AG_ENABLE
        if (dev_node->profile & HFP_AG_DEV) {
            BTMG_INFO(" disconect %s ing", cmd_to_name(HFP_AG_DEV));
            status_err = bt_hfp_ag_disconnect_audio(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("HFP_AG_DEV audio SemaphoreWait OK!");
            status_err = bt_hfp_ag_disconnect(dev_node->dev_addr);
            if (!status_err) {
                XR_OS_SemaphoreWait(&dev_list->sem, 8000);
            }
            BTMG_DEBUG("HFP_AG_DEV SemaphoreWait OK!");
        }
#endif
        //No need to remove, in core deinit func clear
        dev_node = dev_node->next;
    }

    dev_list->sem_flag = 0;
    BTMG_INFO("FINISH!");
    return true;
}
