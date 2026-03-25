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
#include "cmd_util.c"
#include "console.h"
#include "config.h"

#if DYNAMIC_FTRACE && DYNAMIC_FTRACE_CMD
#include "core.h"
#include <stdio.h>

#if CMD_TASK_STACK_SIZE > 1024
#define FTRACE_CMD_TASK_STACK_SIZE        CMD_TASK_STACK_SIZE
#else
#if defined(CONFIG_ARCH_SUN20IW2) && defined(CONFIG_ARCH_RISCV_RV64)
#define FTRACE_CMD_TASK_STACK_SIZE        (10 * 1024)
#else
#define FTRACE_CMD_TASK_STACK_SIZE        (4 * 1024)
#endif
#endif /* CONFIG_CMD_TASK_STACK_SIZE > 1 */

#define FTRACE_CMD_TASK_PRI          (CMD_TASK_PRI)
#define FTRACE_CMD_TASK_NAME         "ftrace cmd"

struct cmd_ftrace_common {
	thread_t thread;
	char arg_buf[1024];
};

static struct cmd_ftrace_common cmd_ftrace;
/*
 * main commands
 */
static enum cmd_status cmd_main_help_exec(char *cmd);

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_set_tracer_type(char *cmd)
{
	int tracer_type;
	int cnt,ret = -1;

	CMD_DBG("[FT] CMD: %s\n", cmd);

	cnt = sscanf(cmd, "%d", &tracer_type);
	if (cnt != 1) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}

	switch(tracer_type) {
		case 0:
			CMD_DBG("[FT] CMD: %s TYPE: %d\n", cmd, TYPE_FUNCTION);
			ret = ftrace_set_tracer_type(TYPE_FUNCTION);
			break;
#if DYNAMIC_FTRACE_GRAPH
		case 1:
			CMD_DBG("[FT] CMD: %s TYPE: %d\n", cmd, TYPE_FUNCTION_GRAPH);
			ret = ftrace_set_tracer_type(TYPE_FUNCTION_GRAPH);
			break;
#endif
#if DYNAMIC_FTRACE_IRQSOFF
		case 2:
			CMD_DBG("[FT] CMD: %s TYPE: %d\n", cmd, TYPE_IRQSOFF);
			ret = ftrace_set_tracer_type(TYPE_IRQSOFF);
			break;
#endif
		default :
			CMD_DBG("[FT] UNKOWN TYPE!\n");
			break;
	}
	if (ret) {
		CMD_ERR("[FT] SET TRACER TYPE FAILED. RET %d\n", ret);
		return CMD_STATUS_FAIL;
	}
	CMD_DBG("[FT] FLUSH OLD DATA IN RING BUF!!!\n");
	logger_flush();

	return CMD_STATUS_OK;
}

#if DYNAMIC_FTRACE_FILTER
ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_set_filter_type(char *cmd)
{
	int filter_type;
	int cnt, ret = -1;

	CMD_DBG("[FT] CMD: %s\n", cmd);

	cnt = sscanf(cmd, "%d", &filter_type);
	if (cnt != 1) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}

	switch(filter_type) {
		case 0:
			CMD_DBG("[FT] CMD: %s TYPE: %d\n", cmd, TYPE_ONLY_TRACE);
			ret = ftrace_set_filter_type(TYPE_ONLY_TRACE);
			break;
		case 1:
			CMD_DBG("[FT] CMD: %s TYPE: %d\n", cmd, TYPE_NO_TRACE);
			ret = ftrace_set_filter_type(TYPE_NO_TRACE);
			break;
		default :
			ret = -1;
			CMD_DBG("[FT] UNKOWN TYPE!\n");
			break;
	}
	if (ret) {
		CMD_ERR("[FT] SET FILTER TYPE FAILED. RET %d\n", ret);
		return CMD_STATUS_FAIL;
	}
	CMD_DBG("[FT] FLUSH OLD DATA IN RING BUF!!!\n");
	logger_flush();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_add_filterlist(char *cmd)
{
	enum {
		ADDR,
		NAME
	};
	char func_addr_type;
	char filter_funcname[MAX_FUNC_NAME_LEN];
	unsigned long filter_funcaddr;
	int cnt;

	CMD_DBG("[FT] CMD: %s\n",cmd);

	CMD_DBG("[FT] CURRENT FILTER TYPE:%d LIST:\n",ftrace_get_filter_type());
	ftrace_dump_filterlist();

	cnt = sscanf(cmd, "at=%d %s", &func_addr_type, filter_funcname);
	if (cnt != 2) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}
	CMD_DBG("[FT] at=%d func=%s\n", func_addr_type, filter_funcname);
	if (func_addr_type == ADDR) {
		cnt = sscanf(filter_funcname, "0x%x", &filter_funcaddr);
		if (cnt != 1) {
			CMD_ERR("cnt %d\n", cnt);
			return CMD_STATUS_INVALID_ARG;
		}
		goto add_filterlist;
	}

	CMD_DBG("[FT] SET FILTER FUNC NAME: %s\n", filter_funcname);
#if DYNAMIC_FTRACE_KALLSYMS
	filter_funcaddr = kallsyms_lookup_name(filter_funcname);
#endif
	CMD_DBG("[FT] FUNC NAME -> FUNC ADDR %x\n", filter_funcaddr);
add_filterlist:
	ftrace_add_filterlist((void *)filter_funcaddr);
	ftrace_dump_filterlist();

	CMD_DBG("[FT] FLUSH OLD DATA IN RING BUF!!!\n");
	logger_flush();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_remove_filterlist(char *cmd)
{
	enum {
		ADDR,
		NAME
	};
	char func_addr_type;
	char filter_funcname[MAX_FUNC_NAME_LEN];
	unsigned long filter_funcaddr;
	int cnt;

	CMD_DBG("[FT] CMD: %s\n",cmd);

	CMD_DBG("[FT] CURRENT FILTER TYPE:%d LIST:\n",ftrace_get_filter_type());
	ftrace_dump_filterlist();

	cnt = sscanf(cmd, "at=%d %s", &func_addr_type, filter_funcname);
	if (cnt != 2) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}
	if (func_addr_type == ADDR) {
		cnt = sscanf(filter_funcname, "0x%x", &filter_funcaddr);
		if (cnt != 1) {
			CMD_ERR("cnt %d\n", cnt);
			return CMD_STATUS_INVALID_ARG;
		}
		goto remove_filterlist;
	}

	memset(filter_funcname, 0, MAX_FUNC_NAME_LEN);
	memcpy(filter_funcname, cmd, strlen(cmd));
	CMD_DBG("[FT] SET FILTER FUNC NAME: %s\n", filter_funcname);
#if DYNAMIC_FTRACE_KALLSYMS
	filter_funcaddr = kallsyms_lookup_name(filter_funcname);
#endif
	CMD_DBG("[FT] FUNC NAME -> FUNC ADDR %x\n", filter_funcaddr);
remove_filterlist:
	ftrace_remove_filterlist((void *)filter_funcaddr);
	ftrace_dump_filterlist();

	CMD_DBG("[FT] FLUSH OLD DATA IN RING BUF!!!\n");
	logger_flush();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_flush_filterlist(char *cmd)
{
	(void)cmd;

	ftrace_flush_filterlist();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_dump_filterlist(char *cmd)
{
	(void)cmd;

	ftrace_dump_filterlist();

	return CMD_STATUS_OK;
}
#endif /* DYNAMIC_FTRACE */

#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
extern int g_find_xipfunc_flag;
ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_find_xf_in_isr(char *cmd)
{
	int flag;
	int cnt;

	CMD_DBG("[FT] CMD: %s\n", cmd);

	cnt = sscanf(cmd, "%d", &flag);
	if (cnt != 1) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}

	g_find_xipfunc_flag = flag;

	return CMD_STATUS_OK;
}
#endif /* DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK */

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_on(char* cmd)
{
	(void)cmd;
	int ret = -1;

	ret = ftrace_init();

	logger_init();

	ftrace_info_show();

	return ret? CMD_STATUS_FAIL : CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_off(char* cmd)
{
	(void)cmd;
	int ret = -1;

	ret = ftrace_deinit();

	logger_deinit();

	return ret? CMD_STATUS_FAIL : CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_dump_replaced_code(char *cmd)
{
	(void)cmd;

	CMD_DBG("[FT] CMD: %s\n", cmd);

	ftrace_dump_replaced_code();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_dump_addr_code(char* cmd)
{
	int cnt;
	addr_t start;
	uint32_t len;

	CMD_DBG("[FT] CMD: %s\n", cmd);

	cnt = sscanf(cmd, "s=%x l=%d", &start, &len);
	if (cnt != 2) {
		CMD_ERR("cnt %d\n", cnt);
		return CMD_STATUS_INVALID_ARG;
	}

	start &= 0xffffffff;
	ftrace_dump_addr_code((void *)start, len);

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_dump_log(char* cmd)
{
	char *buf = NULL;
	unsigned int size = MAX_BUF_SIZE;
	unsigned int read_len = 0;

	buf = malloc(size + 1);
	if (buf == NULL) {
		CMD_DBG("[FT] FTRACE GET, BUT NO MEM!!\n");
		return CMD_STATUS_FAIL;
	}
	memset(buf, 0, size + 1);
	read_len = logger_read(buf, size);
	CMD_DBG("[FT] BUF LEN: %d\n", read_len);

	ftrace_dump_log(buf, read_len);
	free(buf);

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_flush_log(char* cmd)
{
	(void)cmd;

	CMD_DBG("[FT] FLUSH OLD DATA IN RING BUF!!!\n");
	logger_flush();

	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_info_show(char* cmd)
{
	(void)cmd;

	ftrace_info_show();

	return CMD_STATUS_OK;
}

static const struct cmd_data ftrace_cmds[] = {
#if DYNAMIC_FTRACE
	{ "current_tracer", cmd_ftrace_set_tracer_type, \
		CMD_DESC(\
			"set tracer type. "\
			"0.function 1.function graph 2.irqsoff.\r\n"
			"**********{ ftrace current_tracer 0(0/1/2) }.") },
	{ "on", cmd_ftrace_on, \
		CMD_DESC(\
			"enable ftrace.\r\n"\
			"**********{ ftrace on }.") },
	{ "off", cmd_ftrace_off, \
		CMD_DESC(\
			"disable ftrace.\r\n"\
			"**********{ ftrace off }.") },
	{ "dump_replaced_code", cmd_ftrace_dump_replaced_code, \
		CMD_DESC(\
			"dump all replaced instructions.\r\n"\
			"**********{ ftrace dump_replaced_code }.") },
	{ "dump_addr_code", cmd_ftrace_dump_addr_code, \
		CMD_DESC(\
			"dump instructions in addr. "\
			"s=0x10000000:func addr, l=16:len.\r\n"\
			"**********{ ftrace dump_addr_code s=0x10000000 l=16 }.") },
	{ "dump_log", cmd_ftrace_dump_log, \
		CMD_DESC(\
			"dump recorded log.\r\n"\
			"**********{ ftrace dump_log }.") },
	{ "flush_log", cmd_ftrace_flush_log, \
		CMD_DESC(\
			"flush recorded log.\r\n"\
			"**********{ ftrace flush_log }.") },
	{ "info", cmd_ftrace_info_show, \
		CMD_DESC(\
			"show info of ftrace.\r\n"\
			"**********{ ftrace info }.") },
#if DYNAMIC_FTRACE_FILTER
	{ "current_filter", cmd_ftrace_set_filter_type, \
		CMD_DESC(\
			"set filter type. "\
			"0.only trace 1.no trace.\r\n"\
			"**********{ ftrace current_filter 0(0/1) }.") },
	{ "add_filterlist", cmd_ftrace_add_filterlist, \
		CMD_DESC(\
			"add function to filterlist. "\
			"at=0:func addr 1:func name.\r\n"\
			"**********{ ftrace add_filterlist at=1(0/1) dhcp_start }.") },
	{ "remove_filterlist", cmd_ftrace_remove_filterlist, \
		CMD_DESC(\
			"remove function to filterlist. "\
			"at=0:func addr 1:func name.\r\n"\
			"**********{ ftrace remove_filterlist at=1(0/1) dhcp_start }.") },
	{ "flush_filterlist", cmd_ftrace_flush_filterlist, \
		CMD_DESC(\
			"flush filterlist.\r\n"
			"**********{ ftrace flush_filterlist }.") },
	{ "dump_filterlist", cmd_ftrace_dump_filterlist, \
		CMD_DESC(\
			"dump filterlist.\r\n"
			"**********{ ftrace dump_filterlist }.") },
#endif /* DYNAMIC_FTRACE_FILTER */
#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
	{ "find_xipfunc_in_isr", cmd_ftrace_find_xf_in_isr, \
		CMD_DESC(\
		"enable find xip func in isr. "\
		"0:disable - 1:enable.\r\n"\
		"**************{ ftrace find_xipfunc_in_isr 1(0/1) }.") },
#endif
#endif /* DYNAMIC_FTRACE */
	{ "help",          cmd_main_help_exec,       CMD_DESC(CMD_HELP_DESC) },
};

ADD_ATTRIBUTE \
static enum cmd_status cmd_main_help_exec(char *cmd)
{
	return cmd_help_exec(ftrace_cmds, cmd_nitems(ftrace_cmds), 8);
}

ADD_ATTRIBUTE \
static void ftrace_cmd_task(void *arg)
{
	enum cmd_status status;
	char *cmd = (char *)arg;

	CMD_LOG(1, "<ftrace> <request> <cmd : %s>\n", cmd);
	status = cmd_exec(cmd, ftrace_cmds, cmd_nitems(ftrace_cmds));
	if (status != CMD_STATUS_OK)
		CMD_LOG(1, "<ftrace> <response : fail> <%s>\n", cmd);
	else
		CMD_LOG(1, "<ftrace> <response : success> <%s>\n", cmd);
	task_set_invalid(cmd_ftrace.thread);
	task_delete(NULL);
}

ADD_ATTRIBUTE \
static enum cmd_status cmd_ftrace_exec(char *cmd)
{
	if (cmd_strcmp(cmd, "help") == 0) {
		cmd_write_respond(CMD_STATUS_OK, "OK");
		return cmd_main_help_exec(cmd);
	}

	if (task_is_valid(cmd_ftrace.thread)) {
		CMD_ERR("ftrace cmd task is running\n");
		return CMD_STATUS_FAIL;
	}

	memset(cmd_ftrace.arg_buf, 0, sizeof(cmd_ftrace.arg_buf));
	cmd_strlcpy(cmd_ftrace.arg_buf, cmd, sizeof(cmd_ftrace.arg_buf));

	cmd_ftrace.thread = \
			task_create(ftrace_cmd_task,
					    (void *)cmd_ftrace.arg_buf,
						FTRACE_CMD_TASK_NAME,
			            FTRACE_CMD_TASK_STACK_SIZE,
						FTRACE_CMD_TASK_PRI);
	if (cmd_ftrace.thread == NULL) {
		CMD_ERR("ftrace cmd task create failed\n");
		return CMD_STATUS_FAIL;
	}
	return CMD_STATUS_OK;
}

ADD_ATTRIBUTE \
static void ftrace_exec(int argc, char *argv[])
{
	cmd2_main_exec(argc, argv, cmd_ftrace_exec);
}

FINSH_FUNCTION_EXPORT_CMD(ftrace_exec, ftrace, ftrace cmd);
#endif /* DYNAMIC_FTRACE && DYNAMIC_FTRACE_CMD */
