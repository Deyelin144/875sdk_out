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
#include <string.h>

#include "cmd_util.h"
#include "bt_manager.h"

static enum cmd_status btcli_sink_help(char *cmd);

void btcli_a2dp_sink_connection_state_cb(const char *bd_addr,
                                         btmg_a2dp_sink_connection_state_t state)
{
    if (state == BTMG_A2DP_SINK_DISCONNECTED) {
        CMD_DBG("A2DP sink disconnected with device: %s\n", bd_addr);
        btmg_adapter_set_scanmode(BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
    } else if (state == BTMG_A2DP_SINK_CONNECTING) {
        CMD_DBG("A2DP sink connecting with device: %s\n", bd_addr);
    } else if (state == BTMG_A2DP_SINK_CONNECTED) {
        btmg_adapter_set_scanmode(BTMG_SCAN_MODE_NONE);
        CMD_DBG("A2DP sink connected with device: %s\n", bd_addr);
    } else if (state == BTMG_A2DP_SINK_DISCONNECTING) {
        CMD_DBG("A2DP sink disconnecting with device: %s\n", bd_addr);
    }
}

void btcli_a2dp_sink_audio_state_cb(const char *bd_addr, btmg_a2dp_sink_audio_state_t state)
{
    if (state == BTMG_A2DP_SINK_AUDIO_SUSPENDED) {
        CMD_DBG("A2DP sink audio suspended with device: %s\n", bd_addr);
    } else if (state == BTMG_A2DP_SINK_AUDIO_STOPPED) {
        CMD_DBG("A2DP sink audio stopped with device: %s\n", bd_addr);
    } else if (state == BTMG_A2DP_SINK_AUDIO_STARTED) {
        CMD_DBG("A2DP sink audio started with device: %s\n", bd_addr);
    }
}

void btcli_a2dp_sink_stream_cb(const char *bd_addr, uint16_t channels, uint16_t sampling,
                               uint8_t *data, uint32_t len)
{
    (void)data;

#if 0
    CMD_DBG("A2DP sink device: %s, pcm_config: channels:%d, sampling:%d, data len:%d\n", bd_addr,
             channels, sampling, len);
#endif
}

/* btcli a2dp_snk connect <device mac> */
enum cmd_status btcli_sink_connect(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc < 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    btmg_a2dp_sink_connect(argv[0]);

    return CMD_STATUS_OK;
}

/* btcli a2dp_snk disconnect <device mac> */
enum cmd_status btcli_sink_disconnect(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc < 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    btmg_a2dp_sink_disconnect(argv[0]);

    return CMD_STATUS_OK;
}

static const struct cmd_data a2dp_sink_cmds[] = {
    { "connect",    btcli_sink_connect,    CMD_DESC("<device mac>")},
    { "disconnect", btcli_sink_disconnect, CMD_DESC("<device mac>")},
    { "help",       btcli_sink_help,       CMD_DESC(CMD_HELP_DESC) },
};

/* btcli a2dp_snk help */
static enum cmd_status btcli_sink_help(char *cmd)
{
	return cmd_help_exec(a2dp_sink_cmds, cmd_nitems(a2dp_sink_cmds), 10);
}

enum cmd_status btcli_a2dp_sink(char *cmd)
{
    return cmd_exec(cmd, a2dp_sink_cmds, cmd_nitems(a2dp_sink_cmds));
}
