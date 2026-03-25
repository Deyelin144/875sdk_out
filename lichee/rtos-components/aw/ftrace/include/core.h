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
#ifndef __CORE_H
#define __CORE_H

#include "config.h"

#if DYNAMIC_FTRACE
#include "types.h"
#include "port_.h"
#include "_list.h"
#include "_kallsyms.h"
#include "table.h"

enum {
	TYPE_FUNCTION,
	TYPE_FUNCTION_GRAPH,
	TYPE_IRQSOFF,

	TYPE_TRACER_MAX
};

enum {
	TYPE_ONLY_TRACE,
	TYPE_NO_TRACE,

	TYPE_FILTER_MAX
};

typedef enum {
	REPLACE_CODE,
	REPLACE_NOP
} REPLACE_POLICY;

enum {
	FUNC_ENTER = 1,
	FUNC_EXIT = 2
};

enum {
	START_RECORD = 1,
	STOP_RECORD = 2
};

#define _BIT(v, n) (v << n)
#define FUNCTION_GRAPH_UNKOWN	(_BIT(0,1) | _BIT(0,0)) //00
#define FUNCTION_GRAPH_TRACE	(_BIT(0,1) | _BIT(1,0)) //01
#define FUNCTION_GRAPH_NOTRACE	(_BIT(1,1) | _BIT(0,0)) //10

#define FUNCTION_GRAPH_CLEAR_AND_SET(f, v) do { (f) = (f) & ~0x03; (f) |= (v);} while(0)
#define FUNCTION_GRAPH_IS_TRACE(f) (((f) & FUNCTION_GRAPH_TRACE) == FUNCTION_GRAPH_TRACE)
#define FUNCTION_GRAPH_IS_NOTRACE(f) (((f) & FUNCTION_GRAPH_NOTRACE) == FUNCTION_GRAPH_NOTRACE)

#define FUNCTION_ENTER_TRACER(f) do { } while(0)
#define FUNCTION_EXIT_TRACER(f) do { } while(0)

/* lr stack */
#if DYNAMIC_FTRACE_GRAPH || DYNAMIC_FTRACE_IRQSOFF
typedef struct {
	void *parent_addr;
	void *this_addr;
	_64bit entry_val;
	uint8_t flag;
} stack_info_t;

typedef struct {
	stack_info_t *stack;
	int8_t sp;
} lr_stack_t;
#define LR_STACK_LEN 50
#endif /* DYNAMIC_FTRACE_GRAPH || DYNAMIC_FTRACE_IRQSOFF */

/* frame */
typedef struct {
	void *task_handle;
	void *call_site;
	void *this_func;
	_64bit time_val;
	_8bit  depth;
	struct {
		_8bit irqsoff : 1;
		_8bit need_resched : 1;
		_8bit hard_soft_irq : 1;
		_8bit preempt_depth : 5;
	} state;
} ftrace_frame_data_t;

enum ftrace_log_offset {
	TASK_PID_OFFSET = 12,
	TASK_PID_TO_CPU_PAD = 3,
	CPU_OFFSET = 23,
	CPU_TO_IRQFALG_PAD = 2,
	IRQ_FLAG_OFFSET = 29,
	IRQ_TO_TIMESTAMP_PAD = 4,
	TIMESTAMP_OFFSET = 37,
	TIMESTAMP_TO_FUNCTION_PAD = 2,
	FUNCTION_OFFSET = 48,
};

#define FRAME_HEAD_LEN 2
#define FRAME_TAIL_LEN 2
#define FRAME_LEN_SIZE sizeof(_32bit) //frame len
#define FRAME_DATE_LEN sizeof(ftrace_frame_data_t)
#define FRAME_TOTAL_LEN \
(FRAME_HEAD_LEN + FRAME_TAIL_LEN + FRAME_LEN_SIZE + FRAME_DATE_LEN)

typedef int (*_print)(const char *fmt, ...);
#define MAX_FUNC_NAME_LEN 64
/* *.S */
extern char ftrace_tracer_cb[];
extern void ftrace_function_tracer(void);
extern void ftrace_function_graph_tracer(void);
extern void ftrace_irqsoff_tracer(void);

int ftrace_set_tracer_type(uint8_t type);
int ftrace_get_tracer_type(void);
int ftrace_set_filter_type(_8bit type);
int ftrace_get_filter_type(void);
int ftrace_add_filterlist(void *_addr);
int ftrace_remove_filterlist(void *_addr);
int ftrace_flush_filterlist(void);
int ftrace_dump_filterlist(void);
int ftrace_init(void);
int ftrace_deinit(void);
void ftrace_info_show(void);
void ftrace_dump_replaced_code(void);
int ftrace_dump_addr_code(void *_start, _32bit len);
int ftrace_dump_log(char *buf, unsigned int len);
#endif /* DYNAMIC_FTRACE */
#endif /* __CORE_H */
