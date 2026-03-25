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
#include "core.h"

/* debug */
#define MODULE_NAME "[FTRACE]"
#include "debug.h"

#if DYNAMIC_FTRACE
/* thread list */
static void *last_task_id;
static void *current_lr_stack;
/* table */
static table_t filterlist[TYPE_FILTER_MAX][TYPE_TRACER_MAX];
static uint8_t current_filter_type = TYPE_ONLY_TRACE;
/* task */
static thread_t ftrace_deinit_task_handle;
static thread_t ftrace_init_task_handle;
/* find xip flag */
uint8_t g_find_xipfunc_flag = 1;
/* irq */
uint64_t g_irqs_off_time;
uint64_t g_irqs_on_time;
uint32_t g_irq;
/* frame */

static uint64_t start_record_time;
static uint8_t  record_flag = STOP_RECORD;

#if THREAD_STACK_SAVE_BY_LIST

LIST_HEAD_DEF(lr_stack_thread_list);

typedef struct {
	void *thread;
	lr_stack_t lr_stack;
	struct list_index list;
} lr_stack_item_t;

ADD_ATTRIBUTE \
static void *lr_stack_thread_get(void *task_id)
{
	struct list_index *pos;
	lr_stack_item_t *item = NULL;
	void *thread = task_id;

	if (thread == NULL)
		thread = (void *)get_task_self();

	list_for_each(pos, &lr_stack_thread_list) {
		item = list_entry(pos, lr_stack_item_t, list);
		if (item->thread == thread) {
			return (void*)(&item->lr_stack);
		}
	}

	return NULL;
}

ADD_ATTRIBUTE \
static void lr_stack_thread_free(void *task_id)
{
	struct list_index *pos;
	lr_stack_item_t *item = NULL;
	void *thread = task_id;

	if (thread == NULL)
		thread = (void *)get_task_self();

	list_for_each(pos, &lr_stack_thread_list) {
		item = list_entry(pos, lr_stack_item_t, list);
		if (item->thread == thread) {
				list_del(&item->list);
				ftrace_free(item->lr_stack.stack);
				item->lr_stack.sp = 0;
				ftrace_free(item);
				return ;
		}
	}

	return ;
}

ADD_ATTRIBUTE \
static void lr_stack_thread_free_deleted(void)
{
	struct list_index *pos;
	lr_stack_item_t *item = NULL;
	eTaskState state = eInvalid;

	list_for_each(pos, &lr_stack_thread_list) {
		item = list_entry(pos, lr_stack_item_t, list);
		if (item->thread) {
			state = eTaskGetState(item->thread);
			if (state == eDeleted || state == eInvalid) {
				list_del(&item->list);
				ftrace_free(item->lr_stack.stack);
				item->lr_stack.sp = 0;
				ftrace_free(item);
				return ;
			}
		}
	}

	return ;
}

ADD_ATTRIBUTE \
static void lr_stack_thread_free_unused(void)
{
	struct list_index *pos;
	lr_stack_item_t *item = NULL;

	list_for_each(pos, &lr_stack_thread_list) {
		item = list_entry(pos, lr_stack_item_t, list);
		if (item->thread) {
			if (item->lr_stack.sp == 0) {
				list_del(&item->list);
				ftrace_free(item->lr_stack.stack);
				item->lr_stack.sp = 0;
				ftrace_free(item);
				return ;
			}
		}
	}

	return ;
}

ADD_ATTRIBUTE \
static void *lr_stack_thread_alloc(void)
{
	lr_stack_item_t *item = NULL;

	item = ftrace_malloc(sizeof(lr_stack_item_t));
	if (item == NULL) {
		 return NULL;
	}
	memset(item, 0, sizeof(lr_stack_item_t));
	item->thread = (void *)get_task_self();
	item->lr_stack.stack = ftrace_malloc(LR_STACK_LEN *  sizeof(stack_info_t));
	memset(item->lr_stack.stack, 0, (LR_STACK_LEN *  sizeof(stack_info_t)));
	item->lr_stack.sp = 0; //point lr_stack start

	list_add(&item->list, &lr_stack_thread_list);

	return (void*)(&item->lr_stack);
}

static stack_info_t lr_stack_isr_buf[LR_STACK_LEN] = {0};
static lr_stack_t lr_stack_isr = {
	.stack = lr_stack_isr_buf,
	.sp = 0
};

ADD_ATTRIBUTE \
static inline lr_stack_t *lr_stack_isr_get(void)
{
	return &lr_stack_isr;
}

#define GET_CURRENT_LR_STACK(f) \
({ \
	if (IS_ISR_CONTEXT()) { \
		current_lr_stack = (void *)lr_stack_isr_get(); \
		last_task_id = 0; \
	} else { \
		void *task_id; \
		task_id = (void *)get_task_self(); \
		if (last_task_id != task_id) { \
			current_lr_stack = lr_stack_thread_get(task_id); \
			if (current_lr_stack == NULL) { \
				if (f == FUNC_ENTER) { \
					lr_stack_thread_free_deleted(); \
					current_lr_stack = lr_stack_thread_alloc(); \
					if (current_lr_stack == NULL) { \
						ftstr_print("ABORT! NO MEM, GET LR STACK FAILED!", -1); \
						while(1); \
					} \
				} else { \
					ftstr_print("ABORT! CURRENT THREAD CALL_STACK IS NOT EXIST!", -1); \
					while(1); \
				} \
			} \
			last_task_id = task_id; \
		} \
	} \
	current_lr_stack; \
})

#define FREE_CURRENT_LR_STACK(stack_tmp) \
do { \
	stack_tmp.parent_addr = stack[0].parent_addr; \
	stack_tmp.this_addr = stack[0].this_addr; \
	stack_tmp.entry_val = stack[0].entry_val; \
	stack_tmp.flag = stack[0].flag; \
	lr_stack_thread_free(NULL); \
} while(0)
#endif /* THREAD_STACK_SAVE_BY_LIST */

ADD_ATTRIBUTE \
static int ftrace_destruct_frame(char *buf, unsigned int buf_len, ftrace_frame_data_t *data, char *type)
{
	int i = 0;
	char *frame = NULL;

#if DEBUG
	ftstr_print("raw frame: ",-1);
	for (int j = 0; j < 64; j ++) {
		ft8h_print(buf[j]);
	}
	ftstr_print("\n",-1);
#endif
	while (buf_len > 0) {
			frame = &buf[i];
#if DEBUG
			ftstr_print("frame head:",-1);
			ft8h_print(frame[0]);
			ft8h_print(frame[1]);
			ftstr_print("\n",-1);
#endif
		if (( frame[0] == 0xaa && frame[1] == 0xcc ) || \
		   ( frame[0] == 0xcc && frame[1] == 0xaa )) {
			int i1 = 0;
			i1 += 2;
			unsigned int len = *(unsigned int *)(frame + 2);
#if DEBUG
			ftstr_print("frame len:",-1);
			ft32h_print(len,0);
			ftstr_print("\n",-1);
#endif
			if (frame[len - 2] == 0xbb && frame[len - 1] == 0xdd) { //frame is vailed
				if ( frame[0] == 0xaa) {
					*type = FUNC_ENTER;
				} else if ( frame[0] == 0xcc ) {
					*type = FUNC_EXIT;
				}
				i1 += sizeof(unsigned int);
#if DEBUG
				ftstr_print("frame data: ",-1);
				for (int j = 0; j < sizeof(ftrace_frame_data_t); j++) {
					ft8h_print(frame[i1 + j]);
				}
				ftstr_print("\n",-1);
#endif
				memcpy(data, &frame[i1], sizeof(ftrace_frame_data_t));
				buf_len -= len;
				return buf_len;
			} else {
				buf_len -= 2;
			}
		} else {
			i ++;
			buf_len --;
		}
	}
	API_TRACE()
	return 0;
}

ADD_ATTRIBUTE \
static int ftrace_construct_frame(char *buf, ftrace_frame_data_t *data, char type)
{
	unsigned int len = 0;
	unsigned int index = 0, len_index;

	//frame head
	if (type == FUNC_ENTER) {
		buf[index++] = 0xaa;
		buf[index++] = 0xcc;
	} else {
		buf[index++] = 0xcc;
		buf[index++] = 0xaa;
	}

	len_index = index;
	index += sizeof(unsigned int);

	//data
	ftrace_memcpy(&buf[index], (char *)data, sizeof(ftrace_frame_data_t));
	index += sizeof(ftrace_frame_data_t);

	buf[index++] = 0xbb;
	buf[index++] = 0xdd;

	len = index;
	*(unsigned int *)(buf + len_index) = len;

	return len;
}

#define FILL_FRAME_DATA(frame_buf, _call_site, _this_func, \
						sp, val, _irqsoff, f) \
({ \
	ftrace_frame_data_t ftrace_frame_data; \
	ftrace_frame_data.task_handle = (void *)get_task_self(); \
	ftrace_frame_data.call_site = _call_site; \
	ftrace_frame_data.this_func = _this_func; \
	ftrace_frame_data.depth = sp; \
	ftrace_frame_data.time_val = val; \
	ftrace_frame_data.state.irqsoff = _irqsoff? 1 : 0; \
	ftrace_frame_data.state.preempt_depth = GET_IRQ_NEST(); \
	int _len = ftrace_construct_frame(frame_buf, &ftrace_frame_data, f); \
	_len; \
})

#define CALCULATE_TIME(_val, _ms, _us) \
do { \
	_32bit delta; \
	double _time, ns_val1 = 0.0; \
	delta = ftrace_get_timer_delta(); \
	ns_val1 = (double)(1000 * 1000)/ (delta > 0? delta : 1); \
	_time = _val * ns_val1; \
	_ms = (_32bit)(_time / (1000 * 1000)); \
	_us = (_time / 1000) - (_ms * 1000); \
} while(0)

ADD_ATTRIBUTE \
static void
ftrace_dump_one(ftrace_frame_data_t *data, _print print,\
		_8bit tracer_type, _8bit func_type)
{
	_8bit cpu = 0;
	void *call_site = data->call_site;
	void *this_func = data->this_func;
	_64bit val = data->time_val;
	_32bit ms = 0; double us = 0.0;

	CALCULATE_TIME(val, ms, us);
	switch (tracer_type) {
		case TYPE_FUNCTION: {
			void *tcb = data->task_handle;
			task_info_t task_info;
			_8bit irqsoff = data->state.irqsoff;
			_8bit preempt_depth = data->state.preempt_depth;
			double timestamp = ((float)ms / 1000.0f) + (us / 1000000.0f);

			get_task_info(tcb, &task_info, pdFALSE, eInvalid);

			print("%16s-%-6lu [%d]  ", task_info.pcTaskName, \
					task_info.xTaskNumber, cpu);
			print("%c%c%c%d ", irqsoff?'d':'.','.', '.', preempt_depth);
			print("%10.6lf: ", timestamp);
#if DYNAMIC_FTRACE_KALLSYMS
			char call_site_buf[MAX_FUNC_NAME_LEN] = {0};
			char this_func_buf[MAX_FUNC_NAME_LEN] = {0};
			extern int kallsyms_addr2name(void *addr, char *buf);
			kallsyms_addr2name(call_site, call_site_buf);
			kallsyms_addr2name(this_func, this_func_buf);
			print("%s<-%s", this_func_buf + 1, call_site_buf + 1);
#else
			print("%p<-%p", this_func, call_site);
#endif /* DYNAMIC_FTRACE_KALLSYMS */
			print("\r\n");
		}
		break;
		case TYPE_FUNCTION_GRAPH:
		case TYPE_IRQSOFF: {
			_8bit depth = data->depth;

			print("%2d %d)", depth, cpu);
#if DYNAMIC_FTRACE_KALLSYMS
			char this_func_buf[MAX_FUNC_NAME_LEN] = {0};
			extern int kallsyms_addr2name(void *addr, char *buf);
			kallsyms_addr2name(this_func, this_func_buf);
#endif /* DYNAMIC_FTRACE_KALLSYMS */
			if (func_type == FUNC_ENTER) {
				print("%*s |",17, " ");
#if DYNAMIC_FTRACE_KALLSYMS
				print("%*s%s() {", depth + 1, " ", this_func_buf + 1);
#else
				print("%*s%p() {", depth + 1, " ", this_func);
#endif /* DYNAMIC_FTRACE_KALLSYMS */
			} else if (func_type == FUNC_EXIT) {
				print("%4dms %8.3fus |", ms, us);
				print("%*s} /* %s */", depth + 1, " ",
#if DYNAMIC_FTRACE_KALLSYMS
							this_func_buf + 1
#else
							" "
#endif /* DYNAMIC_FTRACE_KALLSYMS */
					 );
			}
			print("\r\n");
		}
		break;
	}
}

ADD_ATTRIBUTE \
int ftrace_dump_log(char *buf, unsigned int len)
{
	int row;
	_8bit tracer_type = ftrace_get_tracer_type();
	switch (tracer_type) {
		case TYPE_FUNCTION: {
				const char ftrace_banner0[][72] = {
					"# tracer: function",
					"#",
					"#                              _-----=> irqs-off(.:on d:off)",
					"#                             / _----=> need-resched",
					"#                            | / _---=> hardirq/softirq",
					"#                            || / _--=> preempt-depth(0:thread >0:irq)",
					"#                            ||| /     delay",
					"#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION",
					"#              | |       |   ||||       |         |"
				};
				row = sizeof(ftrace_banner0) / sizeof(ftrace_banner0[0]);
				for (int i = 0; i < row; i++) {
					printf("%s\n", ftrace_banner0[i]);
				}
			}
		break;
#if DYNAMIC_FTRACE_GRAPH || DYNAMIC_FTRACE_IRQSOFF
		case TYPE_IRQSOFF:
		case TYPE_FUNCTION_GRAPH: {
				const char ftrace_banner1[][49] = {
				"# tracer: function_graph",
				"#",
				"#  CPU  DURATION                  FUNCTION CALLS",
				"#  |     |   |                     |   |   |   |"
				};
				row = sizeof(ftrace_banner1) / sizeof(ftrace_banner1[0]);
				for (int i = 0; i < row; i++) {
					printf("%s\n", ftrace_banner1[i]);
				}
			}
		break;
#endif
		default:
		break;
	}
	/************************/
	unsigned int buf_left_len = len;
	char *get_buf = buf;
	for ( ; buf_left_len > 0; ) {
		ftrace_frame_data_t data;
		char func_type;
		memset(&data, 0, sizeof(ftrace_frame_data_t));
		buf_left_len = ftrace_destruct_frame(get_buf, buf_left_len, &data, &func_type);
		get_buf = buf + (len - buf_left_len);

		ftrace_dump_one(&data, printf, tracer_type, func_type);
	}
	return 0;
}

ADD_ATTRIBUTE \
int ftrace_dump_addr_code(void *_start, _32bit len)
{
	volatile _32bit *_addr = (_32bit *)_start;

	for (; (addr_t)_addr < (addr_t)_start + len; _addr ++) {
		INF(("dump addr(%p) code : %x\n", _addr, *(_addr)));
	}
	return 0;
}

ADD_ATTRIBUTE \
static int ftrace_dump_code(addr_t *table_start, addr_t *table_end)
{
	volatile _32bit *func_addr;

	for (int num = 0; table_start < table_end; table_start ++, num ++) {
		INF(("TABLE INDEX: %d ADDR: %p\n", num, table_start));
		func_addr = (_32bit *)FUNC_ADDR(addr_t, table_start);
		for (int j = 0; j < 8; j ++) {
			INF(("dump func addr(%p) code : %x\n",func_addr + j, *(func_addr + j)));
		}
	}
	return 0;
}

ADD_ATTRIBUTE \
void ftrace_dump_replaced_code(void)
{
	ftrace_dump_code((void *)__start_mcount_loc, (void *)__stop_mcount_loc);
}

ADD_ATTRIBUTE \
int ftrace_set_tracer_type(_8bit type)
{
	const char *type_str[TYPE_TRACER_MAX] = {
		"function",
		"function graph",
		"irqsoff"
	};
	volatile addr_t *tracer_cb = (addr_t *)ftrace_tracer_cb;

	INF(("TRACER TYPE -> %s\n", type < TYPE_TRACER_MAX? type_str[type] : "unkown"));
	if (type > TYPE_TRACER_MAX) return -1;

	unsigned long flags; EnterCritical(flags);
	switch (type) {
		case TYPE_FUNCTION:
			*tracer_cb = (addr_t)ftrace_function_tracer;
			break;
#if DYNAMIC_FTRACE_GRAPH
		case TYPE_FUNCTION_GRAPH:
			*tracer_cb = (addr_t)ftrace_function_graph_tracer;
			break;
#endif
#if DYNAMIC_FTRACE_IRQSOFF
		case TYPE_IRQSOFF:
			*tracer_cb = (addr_t)ftrace_irqsoff_tracer;
			break;
#endif
		default:
			ERR(("TRACER TYPE IS NOT ENABLE. USE DEFAULT TRACER.\n"));
			break;
	}
	ExitCritical(flags);
	INF(("TRACER IS -> %p\n", *tracer_cb));
	return 0;
}

ADD_ATTRIBUTE \
int ftrace_get_tracer_type(void)
{
	volatile addr_t *tracer_cb = (addr_t *)ftrace_tracer_cb;

	if (*tracer_cb == (addr_t)ftrace_function_tracer) {
		return TYPE_FUNCTION;
#if DYNAMIC_FTRACE_GRAPH
	} else if (*tracer_cb == (addr_t)ftrace_function_graph_tracer) {
		return TYPE_FUNCTION_GRAPH;
#endif
#if DYNAMIC_FTRACE_IRQSOFF
	} else if (*tracer_cb == (addr_t)ftrace_irqsoff_tracer) {
		return TYPE_IRQSOFF;
#endif
	}

	return -1;
}

#if DYNAMIC_FTRACE_FILTER
ADD_ATTRIBUTE \
int ftrace_set_filter_type(_8bit type)
{
	const char *type_str[TYPE_FILTER_MAX] = {
		"only trace",
		"no trace",
	};

	INF(("FILTER TYPE -> %s\n", type < TYPE_FILTER_MAX? type_str[type] : "unkown"));
	if (type > TYPE_FILTER_MAX) return -1;

	current_filter_type = type;

	return 0;
}

ADD_ATTRIBUTE \
int ftrace_get_filter_type(void)
{
	return current_filter_type;
}

ADD_ATTRIBUTE \
int ftrace_add_filterlist(void *_addr)
{
	_8bit tracer_type = ftrace_get_tracer_type();
	_8bit filter_type = ftrace_get_filter_type();

	if (tracer_type > TYPE_TRACER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN TRACER.\n"));
		return -1;
	}
	if (filter_type > TYPE_FILTER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN FILTER.\n"));
		return -1;
	}
	INF(("TRACER TYPE: %d - TRACER TYPE: %d - FILTER ADDR: %x\n",\
				tracer_type, filter_type,  _addr));
	table_add(&filterlist[filter_type][tracer_type], _addr);

	return 0;
}

ADD_ATTRIBUTE \
int ftrace_remove_filterlist(void *_addr)
{
	_8bit tracer_type = ftrace_get_tracer_type();
	_8bit filter_type = ftrace_get_filter_type();

	if (tracer_type > TYPE_TRACER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN TRACER.\n"));
		return -1;
	}
	if (filter_type > TYPE_FILTER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN FILTER.\n"));
		return -1;
	}
	INF(("TRACER TYPE: %d - TRACER TYPE: %d - FILTER ADDR: %x\n",\
				tracer_type, filter_type,  _addr));
	table_clear(&filterlist[filter_type][tracer_type], _addr);

	return 0;
}

ADD_ATTRIBUTE \
int ftrace_flush_filterlist(void)
{
	_8bit tracer_type = ftrace_get_tracer_type();
	_8bit filter_type = ftrace_get_filter_type();

	if (tracer_type > TYPE_TRACER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN TRACER.\n"));
		return -1;
	}
	if (filter_type > TYPE_FILTER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN FILTER.\n"));
		return -1;
	}
	INF(("TRACER TYPE: %d - TRACER TYPE: %d\n",\
				tracer_type, filter_type));
	table_flush(&filterlist[filter_type][tracer_type]);

	return 0;
}

ADD_ATTRIBUTE \
int ftrace_dump_filterlist(void)
{
	_8bit tracer_type = ftrace_get_tracer_type();
	_8bit filter_type = ftrace_get_filter_type();

	if (tracer_type > TYPE_TRACER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN TRACER.\n"));
		return -1;
	}
	if (filter_type > TYPE_FILTER_MAX) {
		ERR(("SET FILTER FAILED! UNKOWN FILTER.\n"));
		return -1;
	}
	INF(("TRACER TYPE: %d - TRACER TYPE: %d\n",\
				tracer_type, filter_type));
	table_dump(&filterlist[filter_type][tracer_type]);

	return 0;
}
#endif /* DYNAMIC_FTRACE_FILTER */

#if DYNAMIC_FTRACE_IRQSOFF && DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
ADD_ATTRIBUTE \
static addr_t ftrace_irq_find_xipfunc(addr_t start_addr)
{
	addr_t cur_addr = start_addr + 16;

#if DEBUG
	ftstr_print("[FT] addr: - 16bit inst ***\n",-1);
	ft32h_print(*(_32bit*)&cur_addr, *(_32bit*)cur_addr);
	ftstr_print("\r\n",-1);
#endif
	for (int i = 0; i < MAX_INST_NUM; i ++) {
		if (IS_16BIT_INST(cur_addr)) { //16bit
#if DEBUG
			ftstr_print("[FT] index - addr: - 16bit inst\n",-1);
			ft32h_print(i,0);
			ft32h_print(*(_32bit*)&cur_addr, *(_32bit*)cur_addr);
			ftstr_print("\r\n",-1);
#endif
			if (IS_16BIT_J_INST(cur_addr)) { //j inst
				addr_t call_addr;
				call_addr = parse_j_addr(cur_addr);
#if DEBUG
				ftstr_print("[FT] call func addr:",-1);
				ft32h_print(*(_32bit*)&call_addr,0);
				ftstr_print("\r\n",-1);
#endif
				if (IS_XIP_ADDR(addr_t,call_addr)) { //check xip addr
					//call xip func. bkpt or exception
					return call_addr;
				}
			} else if (i > 1 && IS_FUNC_INST_END(cur_addr)) { //is func tail
#if DEBUG
				ftstr_print("[FT]func end ***\n",-1);
#endif
				return 0;
			}
			cur_addr += 2;
		} else if (IS_32BIT_INST(cur_addr)) { //32bit
#if DEBUG
			ftstr_print("[FT] index - addr: - 32bit inst\n",-1);
			ft32h_print(i,0);
			ft32h_print(*(_32bit*)&cur_addr, *(_32bit*)cur_addr);
			ftstr_print("\r\n",-1);
#endif
			if (IS_32BIT_J_INST(cur_addr)) { //j inst
				addr_t call_addr;
				call_addr = parse_j_addr(cur_addr);
#if DEBUG
				ftstr_print("[FT] call func addr:",-1);
				ft32h_print(*(_32bit*)&call_addr,0);
				ftstr_print("\r\n",-1);
#endif
				if (IS_XIP_ADDR(addr_t,call_addr)) { //check xip addr
					//call xip func. bkpt or exception
					return call_addr;
				}
			} else if (i > 1 && IS_FUNC_INST_END(cur_addr)) { //is func tail
#if DEBUG
				ftstr_print("[FT]func end ***\n",-1);
#endif
				return 0;
			}
			cur_addr += 4;
		}
	}

	return 0;
}

#define FIND_CALL_XIPFUNC(irq, this_func) \
do { \
	if (g_find_xipfunc_flag) { \
		addr_t call_addr; \
		call_addr = ftrace_irq_find_xipfunc((addr_t)this_func); \
		if (call_addr) { \
			ftstr_print("[FT] FOUND XIP FUNC IN ISR -> XIP FUNC ADDR\n",-1); \
			ft32h_print((*(_32bit*)&this_func), *((_32bit*)&this_func + 1)); \
			ftstr_print("->", strlen("->")); \
			ft32h_print((*(_32bit*)&call_addr), *((_32bit*)&call_addr + 1)); \
			ftstr_print("\r\n", -1); \
			ftstr_print("[FT] IRQ: ",-1); \
			ft32h_print(irq, 0); \
			ftstr_print("\r\n", -1); \
			while(1); \
		} else { \
			/* ftstr_print("[FT] NOT FOUND XIP FUNC IN ISR\n",-1); */ \
		} \
	} \
} while(0)
#endif /* DYNAMIC_FTRACE_IRQSOFF && DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK */

ADD_ATTRIBUTE \
int _ftrace_function_tracer_entry(void *call_site, void *this_func)
{
#if DYNAMIC_FTRACE
#if DEBUG
	ftstr_print("[FT]", strlen("[FT]"));
	ft32h_print((*(uint32_t*)&call_site), *((uint32_t*)&call_site + 1));
	ftstr_print("->", strlen("->"));
	ft32h_print((*(uint32_t*)&this_func), *((uint32_t*)&this_func + 1));
	ftstr_print("\r\n", strlen("\r\n"));
#endif
	_8bit irqsoff = GET_IRQOFF();
	_64bit current_val = (ftrace_get_time() - start_record_time);
	unsigned long flags; EnterCritical(flags); {
		char frame_buf[FRAME_TOTAL_LEN] = {0};
		int len = FILL_FRAME_DATA(frame_buf, call_site, this_func,\
				0, current_val, irqsoff, FUNC_ENTER);
#if DYNAMIC_FTRACE_SAVE_TO_RINGBUF
		FTRACE_SAVE_LOG(frame_buf,len);
#endif /* DYNAMIC_FTRACE_SAVE_TO_RINGBUF */
	} ExitCritical(flags);
#endif /* DYNAMIC_FTRACE */
	return 0;
}

#if DYNAMIC_FTRACE_GRAPH
ADD_ATTRIBUTE \
void * _ftrace_function_graph_exit(void)
{
	_64bit current_val = ftrace_get_time();

	int8_t sp;
	stack_info_t *stack;
#if EXIT_FREE_LR_STACK
	stack_info_t stack_tmp;
#endif
	unsigned long flags; EnterCritical(flags); {
		current_lr_stack = GET_CURRENT_LR_STACK(FUNC_EXIT);
		stack = ((lr_stack_t *)current_lr_stack)->stack;
		((lr_stack_t *)current_lr_stack)->sp --;
		sp = ((lr_stack_t *)current_lr_stack)->sp;
		if (sp < 0) {
			ftstr_print("[FT] SP < 0. EXIT ERROR! ABORT!\n",-1);
			while(1);
		}
#if EXIT_FREE_LR_STACK
		else if (sp == 0 && record_flag == STOP_RECORD) {
			FREE_CURRENT_LR_STACK(stack_tmp);
			stack = &stack_tmp;
		}
#endif
#if DYNAMIC_FTRACE_FILTER
		if (FUNCTION_GRAPH_IS_TRACE(stack[sp].flag)) {
			FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_UNKOWN);
			goto exit_record;
		} else if (FUNCTION_GRAPH_IS_NOTRACE(stack[sp].flag)) {
			FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_UNKOWN);
			goto exit_no_record;
		}
	exit_record:
#endif /* DYNAMIC_FTRACE_FILTER */
		{
			_64bit cost_val;
			char frame_buf[FRAME_TOTAL_LEN] = {0};

			cost_val = current_val - stack[sp].entry_val;
			int len = FILL_FRAME_DATA(frame_buf, (void *)(stack[sp].parent_addr), \
					(void *)(stack[sp].this_addr), sp, cost_val, 0, FUNC_EXIT);
#if DYNAMIC_FTRACE_SAVE_TO_RINGBUF
			FTRACE_SAVE_LOG(frame_buf, len);
#endif
		}
#if DYNAMIC_FTRACE_FILTER
	exit_no_record: {}
#endif /* DYNAMIC_FTRACE_FILTER */
#if DEBUG
#endif
	} ExitCritical(flags);
	return (void *)(stack[sp].parent_addr);
}

ADD_ATTRIBUTE \
int _ftrace_function_graph_tracer_entry(void *call_site, void *this_func, unsigned char flag)
{
#if DEBUG
	ftstr_print("[FT]", strlen("[FT]"));
	ft32h_print((*(uint32_t*)&call_site), *((uint32_t*)&call_site + 1));
	ftstr_print("->", strlen("->"));
	ft32h_print((*(uint32_t*)&this_func), *((uint32_t*)&this_func + 1));
	ftstr_print("\r\n", strlen("\r\n"));
#endif
	int8_t sp;
	stack_info_t *stack;
	unsigned long flags; EnterCritical(flags); {
		current_lr_stack = GET_CURRENT_LR_STACK(FUNC_ENTER);
		stack = ((lr_stack_t *)current_lr_stack)->stack;
		sp = ((lr_stack_t *)current_lr_stack)->sp;
		if (sp >= LR_STACK_LEN) {
			ftstr_print("[FT] LR STACK is full. sp: ", -1); ft8h_print(sp);
			ftstr_print("ABORT!", -1);
			while(1);
		} else {
			stack[sp].parent_addr = call_site;
			stack[sp].this_addr = this_func;
			((lr_stack_t *)current_lr_stack)->sp ++;
		}
#if DYNAMIC_FTRACE_FILTER
		int filter_type = ftrace_get_filter_type();
		if (!(FUNCTION_GRAPH_IS_NOTRACE(flag)) && ((FUNCTION_GRAPH_IS_TRACE(flag)) || \
			FUNCTION_GRAPH_IS_TRACE(stack[(sp>0?(sp-1):0)].flag))) {
			FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_TRACE);
			goto record;
		} else if (!(FUNCTION_GRAPH_IS_TRACE(flag)) && ((FUNCTION_GRAPH_IS_NOTRACE(flag)) || \
				FUNCTION_GRAPH_IS_NOTRACE(stack[(sp>0?(sp-1):0)].flag))) {
			FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_NOTRACE);
			goto no_record;
		} else {
			FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, filter_type == TYPE_NO_TRACE ?\
					FUNCTION_GRAPH_TRACE : FUNCTION_GRAPH_NOTRACE);
			if (filter_type == TYPE_NO_TRACE) goto record; else goto no_record;
		}
	record:
#endif /* DYNAMIC_FTRACE_FILTER */
		{
			char frame_buf[FRAME_TOTAL_LEN] = {0};
			int len  = \
			FILL_FRAME_DATA(frame_buf, call_site, this_func,\
					sp, 0, 0, FUNC_ENTER);
#if DYNAMIC_FTRACE_SAVE_TO_RINGBUF
			FTRACE_SAVE_LOG(frame_buf,len);
#endif /* DYNAMIC_FTRACE_SAVE_TO_RINGBUF */
		}
#if DYNAMIC_FTRACE_FILTER
	no_record: {}
#endif /* DYNAMIC_FTRACE_FILTER */
	} ExitCritical(flags);
	stack[sp].entry_val = ftrace_get_time();
	return 0;
}
#endif /* DYNAMIC_FTRACE_GRAPH */

#if DYNAMIC_FTRACE_IRQSOFF
// already Entercritical
ADD_ATTRIBUTE \
void * _ftrace_irqsoff_exit(void)
{
	uint64_t current_val;
	current_val = ftrace_get_time();

	int8_t sp;
	stack_info_t *stack;
	current_lr_stack = GET_CURRENT_LR_STACK(FUNC_EXIT);
	stack = ((lr_stack_t *)current_lr_stack)->stack;
	((lr_stack_t *)current_lr_stack)->sp --;
	sp = ((lr_stack_t *)current_lr_stack)->sp;
	if (sp < 0) {
		ftstr_print("[FT] SP < 0. EXIT ERROR!\n",strlen("[FT] SP < 0. EXIT ERROR!\n"));
		while(1);
		return NULL;
	}
#if DYNAMIC_FTRACE_FILTER
	if (FUNCTION_GRAPH_IS_TRACE(stack[sp].flag)) {
		FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_UNKOWN);
		goto exit_record;
	} else if (FUNCTION_GRAPH_IS_NOTRACE(stack[sp].flag)) {
		FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_UNKOWN);
		goto exit_no_record;
	}
exit_record:
#endif /* DYNAMIC_FTRACE_FILTER */
	{
		_64bit cost_val;
		char frame_buf[FRAME_TOTAL_LEN] = {0};

		cost_val = current_val - stack[sp].entry_val;
		int len  = \
		FILL_FRAME_DATA(frame_buf, (void *)(stack[sp].parent_addr), \
				(void *)(stack[sp].this_addr), sp, cost_val, 0, FUNC_EXIT);
#if DYNAMIC_FTRACE_SAVE_TO_RINGBUF
		FTRACE_SAVE_LOG(frame_buf, len);
#endif
	}
#if DYNAMIC_FTRACE_FILTER
exit_no_record: {}
#endif /* DYNAMIC_FTRACE_FILTER */
#if DEBUG
#endif
	return (void *)(stack[sp].parent_addr);
}

ADD_ATTRIBUTE \
int _ftrace_irqsoff_tracer_entry(void *call_site, void *this_func, _8bit flag)
{
#if DEBUG
	ftstr_print("[FT]", strlen("[FT]"));
	ft32h_print((*(uint32_t*)&call_site), *((uint32_t*)&call_site + 1));
	ftstr_print("->", strlen("->"));
	ft32h_print((*(uint32_t*)&this_func), *((uint32_t*)&this_func + 1));
	ftstr_print("\r\n", strlen("\r\n"));
#endif
	if (IS_ISR_CONTEXT()) {
		_32bit irq = C906_GET_IRQ();
#if DEBUG
		ftstr_print("[FT] ENTRY ISR. IRQ:", -1);ft32h_print(irq, 0);\
			ftstr_print("\r\n", -1);
#endif
#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
		FIND_CALL_XIPFUNC(irq, this_func);
#endif
	} /* if (IS_ISR_CONTEXT()) */
	else { /* is thread context */
	}
	int8_t sp;
	stack_info_t *stack;
	current_lr_stack = GET_CURRENT_LR_STACK(FUNC_ENTER);
	stack = ((lr_stack_t *)current_lr_stack)->stack;
	sp = ((lr_stack_t *)current_lr_stack)->sp;
	if (sp >= LR_STACK_LEN) {
		ftstr_print("[FT] ABORT! LR STACK is full. sp:", -1);
		ft32h_print(sp, 0);
		while(1);
	} else {
		stack[sp].parent_addr = call_site;
		stack[sp].this_addr = this_func;
		((lr_stack_t *)current_lr_stack)->sp ++;
	}
#if DYNAMIC_FTRACE_FILTER
	int filter_type = ftrace_get_filter_type();
	if (!(FUNCTION_GRAPH_IS_NOTRACE(flag)) && ((FUNCTION_GRAPH_IS_TRACE(flag)) || \
		FUNCTION_GRAPH_IS_TRACE(stack[(sp>0?(sp-1):0)].flag))) {
		FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_TRACE);
		goto record;
	} else if (!(FUNCTION_GRAPH_IS_TRACE(flag)) && ((FUNCTION_GRAPH_IS_NOTRACE(flag)) || \
			FUNCTION_GRAPH_IS_NOTRACE(stack[(sp>0?(sp-1):0)].flag))) {
		FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, FUNCTION_GRAPH_NOTRACE);
		goto no_record;
	} else {
		FUNCTION_GRAPH_CLEAR_AND_SET(stack[sp].flag, filter_type == TYPE_NO_TRACE ?\
				FUNCTION_GRAPH_TRACE : FUNCTION_GRAPH_NOTRACE);
		if (filter_type == TYPE_NO_TRACE) goto record; else goto no_record;
	}
record:
#endif /* DYNAMIC_FTRACE_FILTER */
	{
		char frame_buf[FRAME_TOTAL_LEN] = {0};
		int len  = \
		FILL_FRAME_DATA(frame_buf, call_site, this_func,\
				sp, 0, 0, FUNC_ENTER);
#if DYNAMIC_FTRACE_SAVE_TO_RINGBUF
		FTRACE_SAVE_LOG(frame_buf,len);
#endif /* DYNAMIC_FTRACE_SAVE_TO_RINGBUF */
	}
#if DYNAMIC_FTRACE_FILTER
no_record: {}
#endif /* DYNAMIC_FTRACE_FILTER */
	stack[sp].entry_val = ftrace_get_time();
	return 0;
}
#endif /* DYNAMIC_FTRACE_IRQSOFF */

ADD_ATTRIBUTE \
static int ftrace_judge_trace(void *_addr, _8bit tracer_type, _8bit *_flag)
{
#if DYNAMIC_FTRACE_FILTER
	_8bit filter_type = ftrace_get_filter_type();

	if (filter_type > TYPE_FILTER_MAX) return REPLACE_NOP;

	void *addr = _addr;
	_8bit flag = *_flag;
	switch (tracer_type) {
		case TYPE_FUNCTION: {
			int find0 = 0, find1 = 0;
			if (filter_type == TYPE_ONLY_TRACE) {
				find0 = table_find(&filterlist[filter_type][tracer_type], addr);
				if (find0 == 0 || find0 == 1) {
					return REPLACE_CODE;
				} else {
					return REPLACE_NOP;
				}
			} else if (filter_type == TYPE_NO_TRACE) {
				find1 = table_find(&filterlist[filter_type][tracer_type], addr);
				if (find1 == 0 || find1 == -1) {
					return REPLACE_CODE;
				} else {
					return REPLACE_NOP;
				}
			}
		}
		break;
#if DYNAMIC_FTRACE_IRQSOFF || DYNAMIC_FTRACE_GRAPH
		case TYPE_IRQSOFF:
		case TYPE_FUNCTION_GRAPH: {
			int find0 = 0, find1 = 0;
			if (filter_type == TYPE_ONLY_TRACE) {
				find0 = table_find(&filterlist[filter_type][tracer_type], addr);
				if (find0 == 0 || find0 == 1) {
					FUNCTION_GRAPH_CLEAR_AND_SET(flag, FUNCTION_GRAPH_TRACE);
				} else if (find0 == -1) {
					FUNCTION_GRAPH_CLEAR_AND_SET(flag, FUNCTION_GRAPH_UNKOWN);
				}
			} else if (filter_type == TYPE_NO_TRACE) {
				find1 = table_find(&filterlist[filter_type][tracer_type], addr);
				if (find1 == 0) {
					FUNCTION_GRAPH_CLEAR_AND_SET(flag, FUNCTION_GRAPH_TRACE);
				} else if (find1 == 1) {
					FUNCTION_GRAPH_CLEAR_AND_SET(flag, FUNCTION_GRAPH_NOTRACE);
				} else if (find1 == -1) {
					FUNCTION_GRAPH_CLEAR_AND_SET(flag, FUNCTION_GRAPH_UNKOWN);
				}
			}
			*_flag = flag;
		}
		break;
#endif /* DYNAMIC_FTRACE_IRQSOFF || DYNAMIC_FTRACE_GRAPH */
	}
#endif /* DYNAMIC_FTRACE_FILTER */

	return REPLACE_CODE;
}

ADD_ATTRIBUTE \
static void ftrace_convert_nop_all(void *start, void *end)
{
	addr_t *table_start = (addr_t *)start;
	addr_t *table_end = (addr_t *)end;
	addr_t *table_entry = table_start;

	for (; table_entry < table_end; table_entry ++) {
		volatile _8bit *func_addr =  (_8bit *)(*table_entry);
		volatile _8bit *fix_addr = func_addr;
		_8bit *inst_block = ftrace_inst_nop;
		if ((addr_t)func_addr >= (addr_t)XIP_START_ADDR && \
				(addr_t)func_addr <= (addr_t)XIP_END_ADDR)
			continue;
		while ( fix_addr < func_addr + NOP_NUM * NOP_SIZE ) {
			if (!INST_IS_NOP(fix_addr)) {
				*(_16bit*)fix_addr = *(_16bit*)inst_block;
			}
			fix_addr += NOP_SIZE;
		}
		HAL_DCACHE_CLEAN((addr_t)func_addr, NOP_NUM * NOP_SIZE);
		HAL_ICACHE_INVAILED_ALL();
		DSB();
		ISB();
	}
#if DEBUG
	//ftrace_dump_code(table_start, table_end);
#endif
}

ADD_ATTRIBUTE \
static void ftrace_convert_inst(void * start, void *end)
{
	addr_t *table_start = (addr_t *)start;
	addr_t *table_end = (addr_t *)end;
	addr_t *table_entry = table_start;

	_8bit _16bit_inst_num = 0;
	_8bit *inst_block = ftrace_inst_block;
	_8bit *inst_nop = ftrace_inst_nop;
	while (*(_16bit *)inst_block != NOP_INST && _16bit_inst_num < NOP_NUM) {
		_16bit_inst_num ++;
		inst_block += NOP_SIZE;
	}
	if (_16bit_inst_num >= NOP_NUM) {
		DBG(("[FT][ERROR] INST NUM >= %d(NOP NUM)\n", NOP_NUM));
	} else {
		_8bit tracer_type = ftrace_get_tracer_type();
		_8bit flag = 0;

		for (; table_entry < table_end; table_entry ++) {
			_8bit *func_addr =  (_8bit *)(*table_entry);
			_8bit *fix_addr = func_addr;
			_8bit judgement = REPLACE_CODE;
			if ((addr_t)func_addr >= (addr_t)XIP_START_ADDR && \
					(addr_t)func_addr <= (addr_t)XIP_END_ADDR)
				continue;
			judgement = ftrace_judge_trace((void *)func_addr, tracer_type, &flag);
			if (judgement == REPLACE_CODE) goto replace_code;
			else if (judgement == REPLACE_NOP) goto replace_nop;
replace_code:
			inst_block = ftrace_inst_block;
			while ( fix_addr < func_addr + NOP_NUM * NOP_SIZE && \
					*(_16bit *)inst_block !=  NOP_INST) {
				if (INST_IS_JAL(inst_block)) {
					INST_JAL(fix_addr);
					fix_addr += INST_JAL_SIZE;
					inst_block += INST_JAL_SIZE;
				} else if (INST_IS_LI(inst_block)) {
					INST_LI(fix_addr, flag & 0x3f);
					fix_addr += INST_LI_SIZE;
					inst_block += INST_LI_SIZE;
				} else {
					*(_16bit*)fix_addr = *(_16bit*)inst_block;
					fix_addr += NOP_SIZE;
					inst_block += NOP_SIZE;
				}
			}
			goto sync;
replace_nop:
			inst_nop = ftrace_inst_nop;
			while ( fix_addr < func_addr + NOP_NUM * NOP_SIZE ) {
				if (!INST_IS_NOP(fix_addr)) {
					*(_16bit*)fix_addr = *(_16bit*)inst_nop;
				}
				fix_addr += NOP_SIZE;
			}
			goto sync;
sync:
			HAL_DCACHE_CLEAN((addr_t)func_addr, NOP_NUM * NOP_SIZE);
			HAL_ICACHE_INVAILED_ALL();
			DSB();
			ISB();
		}
	}
#if DEBUG
	//ftrace_dump_code(table_start, table_end);
#endif
}

ADD_ATTRIBUTE \
static int _ftrace_init(void)
{
	unsigned long flags;

	INF(("FTRACE INIT INST BLOCK........\n"));
	EnterCritical(flags);
	ftrace_convert_inst((void *)__start_mcount_loc, (void *)__stop_mcount_loc);
	ExitCritical(flags);
	INF(("FTRACE INIT INST BLOCK OVER.\n"));
	return 0;
}

ADD_ATTRIBUTE \
static int _ftrace_deinit(void)
{
	unsigned long flags;

	INF(("FTRACE DEINIT INST BLOCK........\n"));
	EnterCritical(flags);
	ftrace_convert_nop_all((void *)__start_mcount_loc, (void *)__stop_mcount_loc);
	ExitCritical(flags);
	INF(("FTRACE DEINIT INST BLOCK OVER.\n"));

	return 0;
}

ADD_ATTRIBUTE \
static void ftrace_deinit_task(void *arg)
{
	(void)arg;
	_ftrace_deinit();
	record_flag = STOP_RECORD;
	DBG(("FTRACE DEINIT OVER, DELETE TASK.\n"));
	task_set_invalid(ftrace_deinit_task_handle);
	task_delete(NULL);
}

ADD_ATTRIBUTE \
static void ftrace_init_task(void *arg)
{
	(void)arg;
	_ftrace_init();
	record_flag = START_RECORD;
	start_record_time = ftrace_get_time();
	DBG(("FTRACE INIT OVER, DELETE TASK.\n"));
	task_set_invalid(ftrace_init_task_handle);
	task_delete(NULL);
}

ADD_ATTRIBUTE \
int ftrace_init(void)
{
	if (task_is_valid(ftrace_init_task_handle)) {
		ERR(("ftrace init task is running\n"));
		return -1;
	}
	INF(("FTARCE INIT TASK CREATE START!\n"));
	ftrace_init_task_handle = \
			task_create(ftrace_init_task,
					    NULL,
						FTRACE_INIT_TASK_NAME,
			            FTRACE_INIT_TASK_STACK_SIZE,
						FTRACE_INIT_TASK_PRI);
	if (ftrace_init_task_handle) {
		INF(("FTRACE INIT TASK CREATE SUCCESS.\n"));
		return 0;
	}
	ERR(("FTRACE INIT TASK CREATE FAILED.\n"));
	return -1;
}

ADD_ATTRIBUTE \
int ftrace_deinit(void)
{
	if (task_is_valid(ftrace_deinit_task_handle)) {
		ERR(("ftrace deinit task is running\n"));
		return -1;
	}
	INF(("FTARCE DEINIT TASK CREATE START!\n"));
	ftrace_deinit_task_handle = \
			task_create(ftrace_deinit_task,
						NULL,
						FTRACE_DEINIT_TASK_NAME,
						FTRACE_DEINIT_TASK_STACK_SIZE,
						FTRACE_DEINIT_TASK_PRI);
	if (ftrace_deinit_task_handle) {
		INF(("FTRACE DEINIT TASK CREATE SUCCESS.\n"));
		return 0;
	}
	ERR(("FTRACE DEINIT TASK CREATE FAILED.\n"));
	return -1;
}

ADD_ATTRIBUTE \
void ftrace_info_show(void)
{
	INF(("========================================\n"));
	INF(("nop num: %d\n", NOP_NUM));
	INF(("ftrace func num: %d\n", FUNC_NUM(__start_mcount_loc,\
				__stop_mcount_loc, sizeof(addr_t))));
	INF(("functable start: %x end: %x\n", __start_mcount_loc,\
				__stop_mcount_loc));
	INF(("xip start: %x xip end: %x\n", __XIP_Base, __XIP_End));
#if DYNAMIC_FTRACE_KALLSYMS
	INF(("ftrace kallsyms_addresses:   %p\n", kallsyms_addresses));
	INF(("ftrace kallsyms_names:       %p\n", kallsyms_names));
	INF(("ftrace kallsyms_token_index: %p\n", kallsyms_token_index));
	INF(("ftrace kallsyms_token_table: %p\n", kallsyms_token_table));
	INF(("ftrace kallsyms_seqs_of_names: %p\n", kallsyms_seqs_of_names));
	INF(("ftrace kallsyms_num_syms:    %d\n", kallsyms_num_syms));
	INF(("ftrace kallsyms test find name(%s) to addr: %lx\n","dhcp_start",\
				kallsyms_lookup_name("dhcp_start")));
#endif /* DYNAMIC_FTRACE_KALLSYMS */
	INF(("ftrace function tracer       %s\n", \
				DYNAMIC_FTRACE? "enable":"disable"));
	INF(("ftrace function graph tracer %s\n", \
				DYNAMIC_FTRACE_GRAPH? "enable":"disable"));
	INF(("ftrace irqsoff tracer        %s\n",\
				DYNAMIC_FTRACE_IRQSOFF? "enable":"disable"));
	const char *tracer_type_str[TYPE_TRACER_MAX] = {
		"function",
		"function graph",
		"irqsoff"
	};
	_8bit tracer_type = ftrace_get_tracer_type();
	INF(("ftrace tracer type select:   %s\n", tracer_type < TYPE_TRACER_MAX?\
				tracer_type_str[tracer_type] : "unkown"));
#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
	INF(("find xipfunc in isr          %s\n",\
				DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK? "enable":"disable"));
	INF(("find max inst num: %d\n", MAX_INST_NUM));
#endif
#if DYNAMIC_FTRACE_FILTER
	const char *filter_type_str[TYPE_FILTER_MAX] = {
		"only trace",
		"no trace",
	};
	_8bit filter_type = ftrace_get_filter_type();
	INF(("ftrace filter type:          %s\n", filter_type < TYPE_FILTER_MAX?\
				filter_type_str[filter_type] : "unkown"));
	INF(("filter table size:           %d\n", MAX_TABLE_SIZE));
#endif
	INF(("ring buf size:               %d\n", MAX_BUF_SIZE));
	INF(("frame data size:             %d\n", sizeof(ftrace_frame_data_t)));
#if THREAD_STACK_SAVE_BY_LIST
	INF(("lr stack info size:          %d\n", sizeof(stack_info_t)));
#endif /* THREAD_STACK_SAVE_BY_LIST */
	INF(("init task stack size:        %d\n", INIT_TASK_STACK_SIZE));
	INF(("deinit task stack size:      %d\n", DEINIT_TASK_STACK_SIZE));
	INF(("cmd task stack size:         %d\n", CMD_TASK_STACK_SIZE));
	INF(("cmd task pri:                %d\n", CMD_TASK_PRI));
	INF(("%s %s \n",__DATE__, __TIME__));
	INF(("========================================\n"));
}

ADD_ATTRIBUTE \
int ftrace_main(void)
{
	ftrace_info_show();
	return 0;
}
#endif /* DYNAMIC_FTRACE */
