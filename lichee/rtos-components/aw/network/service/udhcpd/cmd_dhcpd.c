/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "lwip/opt.h"
#if LWIP_IPV4 && LWIP_DHCPS

#include "cmd_util.h"
#include "cmd_dhcpd.h"
#include "lwip/dhcps.h"
#include "lwip/inet.h"
#include "lwip/netifapi.h"
#include "console.h"

#define CMD_DHCPD_ADDR_START "192.168.51.100"
#define CMD_DHCPD_ADDR_NUM   5
#define CMD_DHCPD_LEASE_TIME (60 * 60 * 12)
#define CMD_DHCPD_OFFER_TIME (5 * 60)
#define CMD_DHCPD_DECLINE_TIME (10 * 60)

static dhcp_server_config_t dhcpd_info;
extern struct netif *g_wlan_netif;

static enum cmd_status dhcpd_start_exec(char *cmd)
{
	struct netif *nif = g_wlan_netif;

	if (dhcpd_info.start_ip == NULL)
		dhcpd_info.start_ip = CMD_DHCPD_ADDR_START;
	if (dhcpd_info.ip_num == 0)
		dhcpd_info.ip_num = CMD_DHCPD_ADDR_NUM;
	if (dhcpd_info.lease_time == 0)
		dhcpd_info.lease_time = CMD_DHCPD_LEASE_TIME;
	if (dhcpd_info.offer_time == 0)
		dhcpd_info.offer_time = CMD_DHCPD_OFFER_TIME;
	if (dhcpd_info.decline_time == 0)
		dhcpd_info.decline_time = CMD_DHCPD_DECLINE_TIME;

	dhcps_config(&dhcpd_info);
	netifapi_dhcps_start(nif);
	return CMD_STATUS_OK;
}

static enum cmd_status dhcpd_stop_exec(char *cmd)
{
	struct netif *nif = g_wlan_netif;

	netifapi_dhcps_stop(nif);
	return CMD_STATUS_OK;
}

static enum cmd_status dhcpd_set_ippool_exec(char *cmd)
{
	int argc;
	char *argv[2];
	ip_addr_t ip_addr_start;
	uint16_t ip_num;

	argc = cmd_parse_argv(cmd, argv, cmd_nitems(argv));
	if (argc != 2) {
		CMD_ERR("invalid dhcp cmd, argc %d\n", argc);
		return CMD_STATUS_INVALID_ARG;
	}
	if (inet_aton(argv[0], &ip_addr_start) == 0 || \
			(ip_num = cmd_atoi(argv[1])) == 0) {
		CMD_ERR("invalid dhcp cmd <%s %s>\n", argv[0], argv[1]);
		return CMD_STATUS_INVALID_ARG;
	}

#ifdef CONFIG_LWIP_V1
	dhcpd_info.start_ip = inet_ntoa(ip_addr_start);
	dhcpd_info.ip_num   = ip_num;
#elif LWIP_IPV4 /* now only for IPv4 */
	dhcpd_info.start_ip = inet_ntoa(ip_addr_start);
	dhcpd_info.ip_num   = ip_num;
#else
	#error "IPv4 not support!"
#endif

	return CMD_STATUS_OK;
}

static enum cmd_status dhcpd_set_lease_time_exec(char *cmd)
{
	int argc;
	char *argv[1];
	int lease_time;

	argc = cmd_parse_argv(cmd, argv, cmd_nitems(argv));
	if (argc != 1) {
		CMD_ERR("invalid dhcp cmd, argc %d\n", argc);
		return CMD_STATUS_INVALID_ARG;
	}
	lease_time = cmd_atoi(argv[0]);
	if (lease_time < 60) {
		CMD_ERR("leasetime must greater than 60\n");
		return CMD_STATUS_INVALID_ARG;
	}

	dhcpd_info.lease_time = lease_time;
	return CMD_STATUS_OK;
}

static enum cmd_status dhcpd_set_offer_time_exec(char *cmd)
{
	int argc;
	char *argv[1];
	int offer_time;

	argc = cmd_parse_argv(cmd, argv, cmd_nitems(argv));
	if (argc != 1) {
		CMD_ERR("invalid dhcp cmd, argc %d\n", argc);
		return CMD_STATUS_INVALID_ARG;
	}
	offer_time = cmd_atoi(argv[0]);
	if (offer_time < 60) {
		CMD_ERR("offertime must greater than 60\n");
		return CMD_STATUS_INVALID_ARG;
	}

	dhcpd_info.offer_time = offer_time;
	return CMD_STATUS_OK;
}

static enum cmd_status dhcpd_set_decline_time_exec(char *cmd)
{
	int argc;
	char *argv[1];
	int decline_time;

	argc = cmd_parse_argv(cmd, argv, cmd_nitems(argv));
	if (argc != 1) {
		CMD_ERR("invalid dhcp cmd, argc %d\n", argc);
		return CMD_STATUS_INVALID_ARG;
	}
	decline_time = cmd_atoi(argv[0]);
	if (decline_time < 60) {
		CMD_ERR("declinetime must greater than 60\n");
		return CMD_STATUS_INVALID_ARG;
	}

	dhcpd_info.decline_time = decline_time;
	return CMD_STATUS_OK;
}

#if CMD_DESCRIBE
#define dhcpd_start_help_info "start the dhcpd server."
#define dhcpd_stop_help_info  "stop the dhcpd server."
#define dhcpd_ippool_help_info "set the dhcpd server ippool, ippool <ip-addr-start> <ip-addr-num>."
#define dhcpd_set_lease_time_help_info "set the set leases time of dhcpd server, leasetime <sec>."
#define dhcpd_set_offer_time_help_info "set the set offer time of dhcpd server, offertime <sec>."
#define dhcpd_set_decline_time_help_info "set the set decline time of dhcpd server, declinetime <sec>."
#endif

/*
 * dhcp commands
 */
static enum cmd_status cmd_dhcpd_help_exec(char *cmd);

static const struct cmd_data g_dhcpd_cmds[] = {
	{ "start",      dhcpd_start_exec,          CMD_DESC(dhcpd_start_help_info) },
	{ "stop",       dhcpd_stop_exec,           CMD_DESC(dhcpd_stop_help_info) },
	{ "ippool",     dhcpd_set_ippool_exec,     CMD_DESC(dhcpd_ippool_help_info) },
	{ "leasetime",  dhcpd_set_lease_time_exec, CMD_DESC(dhcpd_set_lease_time_help_info) },
	{ "offertime",  dhcpd_set_offer_time_exec, CMD_DESC(dhcpd_set_offer_time_help_info) },
	{ "declinetime",  dhcpd_set_decline_time_exec, CMD_DESC(dhcpd_set_decline_time_help_info) },
	{ "help",       cmd_dhcpd_help_exec,       CMD_DESC(CMD_HELP_DESC) },
};

static enum cmd_status cmd_dhcpd_help_exec(char *cmd)
{
	return cmd_help_exec(g_dhcpd_cmds, cmd_nitems(g_dhcpd_cmds), 8);
}

enum cmd_status cmd_dhcpd_exec(char *cmd)
{
	return cmd_exec(cmd, g_dhcpd_cmds, cmd_nitems(g_dhcpd_cmds));
}
static void dhcpd_exec(int argc, char *argv[])
{
	cmd2_main_exec(argc, argv, cmd_dhcpd_exec);
}

FINSH_FUNCTION_EXPORT_CMD(dhcpd_exec, dhcpd, dhcpd testcmd);
#endif /* LWIP_IPV4 && LWIP_DHCPS */
