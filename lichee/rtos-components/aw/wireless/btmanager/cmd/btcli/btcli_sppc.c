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
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "cmd_util.h"
#include "bt_manager.h"

static enum cmd_status btcli_sppc_help(char *cmd);

void btcli_sppc_conn_status_cb(const char *bd_addr, btmg_spp_connection_state_t state)
{
    if (state == BTMG_SPP_DISCONNECTED) {
        CMD_DBG("spp client disconnected with device: %s\n", bd_addr);
    } else if (state == BTMG_SPP_CONNECTING) {
        CMD_DBG("spp client connecting with device: %s\n", bd_addr);
    } else if (state == BTMG_SPP_CONNECTED) {
        CMD_DBG("spp client connected with device: %s\n", bd_addr);
    } else if (state == BTMG_SPP_DISCONNECTING) {
        CMD_DBG("spp client disconnecting with device: %s\n", bd_addr);
    } else if (state == BTMG_SPP_CONNECT_FAILED) {
        CMD_DBG("spp client connect with device: %s failed!\n", bd_addr);
    } else if (state == BTMG_SPP_DISCONNEC_FAILED) {
        CMD_DBG("spp client disconnect with device: %s failed!\n", bd_addr);
    }
}

void btcli_sppc_recvdata_cb(const char *bd_addr, char *data, int data_len)
{
    char recv_data[1024];

    if (data_len > 1023) {
        CMD_DBG("data_len is too large, intercept 1023 bytes\n");
        data_len = 1023;
    }
    memcpy(recv_data, data, data_len);
    recv_data[data_len] = '\0';

    CMD_DBG("sppc recv from dev:[%s],[len=%d][data:%s]\n", bd_addr, data_len,
            recv_data);
}

/* btcli sppc connect <device mac> */
static enum cmd_status btcli_sppc_connect(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc != 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    btmg_sppc_connect(argv[0]);

    return CMD_STATUS_OK;
}

/* btcli sppc disconnect <device mac> */
static enum cmd_status btcli_sppc_disconnect(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc != 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    btmg_sppc_disconnect(argv[0]);

    return CMD_STATUS_OK;
}

/* btcli sppc write <data> */
static enum cmd_status btcli_sppc_write(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc != 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    btmg_sppc_write(argv[0], strlen(argv[0]));

    return CMD_STATUS_OK;
}

static const struct cmd_data sppc_cmds[] = {
    { "connect",    btcli_sppc_connect,     CMD_DESC("<device mac>")},
    { "disconnect", btcli_sppc_disconnect,  CMD_DESC("<device mac>")},
    { "write",      btcli_sppc_write,       CMD_DESC("<data>")},
    { "help",       btcli_sppc_help,      CMD_DESC(CMD_HELP_DESC)},
};

/* btcli sppc help */
static enum cmd_status btcli_sppc_help(char *cmd)
{
	return cmd_help_exec(sppc_cmds, cmd_nitems(sppc_cmds), 10);
}

enum cmd_status btcli_sppc(char *cmd)
{
    return cmd_exec(cmd, sppc_cmds, cmd_nitems(sppc_cmds));
}
