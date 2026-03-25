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

static enum cmd_status btcli_hidd_help(char *cmd);

/* $hidd init */
enum cmd_status btcli_hidd_init(char *cmd)
{
    return (btmg_hid_device_init() == 0) ? CMD_STATUS_OK : CMD_STATUS_FAIL;
}

/* $hidd deinit */
enum cmd_status btcli_hidd_deinit(char *cmd)
{
    return (btmg_hid_device_deinit() == 0) ? CMD_STATUS_OK : CMD_STATUS_FAIL;
}

/* $hidd connect <bd_addr> */
enum cmd_status btcli_hidd_connect(char *cmd)
{
    int argc;
    char *argv[1];

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc < 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    return (btmg_hid_device_connect(argv[0]) == 0) ? CMD_STATUS_OK : CMD_STATUS_FAIL;
}

/* $hidd disconnect */
static enum cmd_status btcli_hidd_disconnect(char *cmd)
{
    int argc;
    char *argv[1];

//no need mac
#if 0
    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc < 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }
    CMD_DBG("hidd_disconnect %s.\n", argv[0]);
#endif

    return (btmg_hid_device_disconnect(argv[0]) == 0) ? CMD_STATUS_OK : CMD_STATUS_FAIL;
}

/* $hidd mouse <up/down/left/right/left_key/right_key/wheel_up/wheel_down> */
static enum cmd_status btcli_hidd_mouse(char *cmd)
{
    int argc;
    char *argv[2];

    uint8_t buttons = 0;
    char dx = 0;
    char dy = 0;
    char wheel = 0;

    argc = cmd_parse_argv(cmd, argv, 1);
    if (argc < 1) {
        CMD_ERR("invalid param number %d\n", argc);
        return CMD_STATUS_INVALID_ARG;
    }

    if (!strcmp(argv[0], "up")) {
        dy = -10;
    } else if (!strcmp(argv[0], "down")) {
        dy = 10;
    } else if (!strcmp(argv[0], "left")) {
        dx = -10;
    } else if (!strcmp(argv[0], "right")) {
        dx = 10;
    } else if (!strcmp(argv[0], "left_key")) {
        buttons = 1;
    } else if (!strcmp(argv[0], "right_key")) {
        buttons = 2;
    } else if (!strcmp(argv[0], "middle_key")) {
        buttons = 3;
    } else if (!strcmp(argv[0], "wheel_up")) {
        wheel = -10;
    } else if (!strcmp(argv[0], "wheel_down")) {
        wheel = 10;
    } else {
        CMD_ERR("invalid param %s\n", argv[0]);
        return CMD_STATUS_INVALID_ARG;
    }

    return (btmg_hid_device_send_mouse(buttons, dx, dy, wheel) == 0) ? CMD_STATUS_OK :
                                                                       CMD_STATUS_FAIL;
}

/*
    $hidd init
    $hidd deinit
    $hidd connect <bd_addr>
    $hidd disconnect
    $hidd mouse <up/down/left/right/left_key/right_key/wheel_up/wheel_down>
*/

static const struct cmd_data hid_device_cmds[] = {
    { "init", btcli_hidd_init, CMD_DESC("No parameters") },
    { "deinit", btcli_hidd_deinit, CMD_DESC("No parameters") },
    { "connect", btcli_hidd_connect, CMD_DESC("<bd_addr>") },
    { "disconnect", btcli_hidd_disconnect, CMD_DESC("[bd_addr]") },
    { "mouse", btcli_hidd_mouse,
      CMD_DESC("<ctrl:up/down/left/right/left_key/middle_key/right_key/wheel_up/wheel_down>") },
    { "help", btcli_hidd_help, CMD_DESC(CMD_HELP_DESC) },
};

static enum cmd_status btcli_hidd_help(char *cmd)
{
    return cmd_help_exec(hid_device_cmds, cmd_nitems(hid_device_cmds), 10);
}

enum cmd_status btcli_hidd(char *cmd)
{
    return cmd_exec(cmd, hid_device_cmds, cmd_nitems(hid_device_cmds));
}
