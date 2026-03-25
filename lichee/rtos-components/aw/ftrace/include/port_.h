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
#ifndef __PORT__H
#define __PORT__H

#include "config.h"

#if DYNAMIC_FTRACE
extern char __XIP_Base[];
extern char __XIP_End[];
extern char __start_mcount_loc[];
extern char __stop_mcount_loc[];

//lib c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//usr types
#include "types.h"

#include <barrier.h>
//print
#include <hal_uart.h>
void ftrace_print_32hex(int h1, int h2);
void ftrace_print_8hex(char h1);
void ftrace_print_string(char *s, int _s_len);
int  ftrace_printf(const char* format, ...);
#define ftstr_print ftrace_print_string
#define ft32h_print ftrace_print_32hex
#define ft8h_print  ftrace_print_8hex
#define ft_printf   ftrace_printf

#define FUNC_NUM(table_start, table_end, table_entry_size) \
({ \
	int c = (table_end - table_start) / (table_entry_size); \
	c;\
})
#define FUNC_ADDR(type, a) (*(type *)a)

//cache
#include <hal_cache.h>
#ifndef CACHELINE_LEN
#define CACHELINE_LEN 64
#endif
#define HAL_DCACHE_CLEAN(a,s) \
do { \
	addr_t a_align = a; \
	if (a_align & (CACHELINE_LEN - 1)) { \
		a_align &= CACHELINE_LEN; \
	} \
	hal_dcache_clean(a_align,s); \
} while(0)
#define HAL_ICACHE_INVAILED_ALL() \
do { \
	hal_icache_invalidate_all(); \
} while(0)
#define DSB() isb()
#define ISB() dsb()

//cc
#if defined(__GNUC__)
#ifdef __always_inline
#undef __always_inline    /* already defined in <sys/cdefs.h> */
#define __always_inline    inline __attribute__((always_inline))
#endif

#ifndef __noinline
#define __noinline  __attribute__((__noinline__))
#endif

#define __weak      __attribute__((weak))
#endif

//irq
#include "hal_interrupt.h"
#define IS_ISR_CONTEXT() \
	hal_interrupt_get_nest()

#define C906_GET_IRQ(irq) \
({ \
	extern uint32_t g_irq; \
	g_irq; \
})

#define GET_IRQ_NEST()  hal_interrupt_get_nest()
#define GET_IRQOFF()    hal_interrupt_is_disable()

//time port
#include "port_time.h"

//xip port
#define XIP_START_ADDR __XIP_Base
#define XIP_END_ADDR __XIP_End

//os
#include "hal_time.h"
#include "hal_thread.h"
#define INVALID_HANDLE       0       /* invalid handle */
#define task_msleep(m) hal_msleep(m)
#define task_info_t TaskStatus_t
#define thread_t hal_thread_t
#define get_task_info(t,i,f,e) vTaskGetInfo(t,i,f,e)
#define get_task_self()  hal_thread_self()
#define task_create(t,d,n,s,p) hal_thread_create(t,d,n,s,p)
#define task_delete(t) hal_thread_stop(t)
#define task_is_valid(t) ((t) != INVALID_HANDLE)
#define task_set_invalid(t) ((t) = INVALID_HANDLE)

#define FTRACE_INIT_TASK_NAME "ftrace init"
#define FTRACE_INIT_TASK_STACK_SIZE INIT_TASK_STACK_SIZE
#define FTRACE_INIT_TASK_PRI INIT_TASK_PRI
#define FTRACE_DEINIT_TASK_NAME "ftrace deinit"
#define FTRACE_DEINIT_TASK_STACK_SIZE DEINIT_TASK_STACK_SIZE
#define FTRACE_DEINIT_TASK_PRI DEINIT_TASK_PRI

//atomic
#include "hal_atomic.h"
#define EnterCritical(flags) \
do { \
	flags = hal_enter_critical(); \
} while(0)

#define ExitCritical(flags) \
do { \
	hal_exit_critical(flags); \
} while(0)

//inst
extern _8bit ftrace_inst_block[];
extern _8bit ftrace_inst_nop[];
#define ADDR_SIZE sizeof(addr_t)
#define NOP_INST 0x0001
#define NOP_NUM 6
#define NOP_SIZE sizeof(_16bit)
#define INST_IS_NOP(ADDR) ((*(_16bit *)ADDR) == NOP_INST)
#define INST_NOP(ADDR) do { (*(_16bit *)ADDR) = NOP_INST; } while(0)
#define INST_IS_JAL(ADDR) (((*(_32bit *)ADDR) & 0x7F) == 0X6F)
#define INST_JAL_SIZE sizeof(_32bit)
#define INST_IS_LI(ADDR) (((*(_16bit *)ADDR) & 0xE000) == 0X4000 && ((*(_16bit *)ADDR) & 0x3) == 0x1)
#define INST_LI_SIZE sizeof(_16bit)
int ftrace_convert_caddi_sp_n16(addr_t*);
int ftrace_convert_csd_a0_to_sp(addr_t*);
int ftrace_convert_cmv_ra_to_a0(addr_t*);
int ftrace_convert_jal(_8bit*);
int ftrace_convert_cld_a0_from_sp(addr_t*);
int ftrace_convert_caddi_sp_p16(addr_t*);
int ftrace_convert_cli_t2_imm(_8bit*,_8bit);
#define INST_1(a) ftrace_convert_caddi_sp_n16(a)
#define INST_2(a) ftrace_convert_csd_a0_to_sp(a)
#define INST_3(a) ftrace_convert_cmv_ra_to_a0(a)
#define INST_4(a) ftrace_convert_jal(a)
#define INST_5(a) ftrace_convert_cld_a0_from_sp(a)
#define INST_6(a) ftrace_convert_caddi_sp_p16(a)
#define INST_LI(a,i) ftrace_convert_cli_t2_imm(a,i)
#define INST_JAL(a) INST_4(a)

#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
#define IS_16BIT_INST(addr) \
(((*(uint16_t*)addr) & 0x0003) != 0b11)

#define IS_32BIT_INST(addr) \
((((*(uint32_t*)addr) & 0x0003) == 0b11) && \
(((*(uint32_t*)addr) & 0x001f) != 0b11111))

//J INST
#define IS_C_J(addr) \
((*(uint16_t*)addr & 0xe003) == 0xa001)

/* is rv32ic
#define IS_C_JAL(addr) \
(*(uint16_t*)addr & 0x2001 == 0x2001)
*/

#define IS_C_JALR(addr) \
((*(uint16_t*)addr & 0xf07f) == 0x9002)

#define IS_C_JR(addr) \
((*(uint16_t*)addr & 0xf07f) == 0x8002)

//rv64
#define IS_16BIT_J_INST(addr) \
(IS_C_J(addr)/* || IS_C_JAL(addr)*/ || IS_C_JALR(addr) || IS_C_JR(addr))

#define IS_JAL(addr) \
((*(uint32_t*)addr & 0x0000007f) == 0x0000006f)

#define IS_JALR(addr) \
((*(uint32_t*)addr & 0x0000007f) == 0x00000067)

#define IS_AUIPC(addr) \
((*(uint32_t*)addr & 0x0000007f) == 0x00000017)

//rv64
#define IS_32BIT_J_INST(addr) \
(IS_JAL(addr) || IS_JALR(addr))

//inst block end
#define IS_C_ADDI_SP(addr) \
(((*(uint16_t*)addr & 0xe003) == 0x0001) && ((*(uint16_t*)addr & 0x1000) == 0x1000))

#define IS_C_ADDI16_SP(addr) \
(((*(uint16_t*)addr & 0xe003) == 0x6001) && ((*(uint16_t*)addr & 0x1000) == 0x1000))

#define IS_C_ADDI4_SP(addr) \
(((*(uint16_t*)addr & 0xe003) == 0x0000) && ((*(uint16_t*)addr & 0x0400) == 0x0400))

#define IS_C_ADDIW_SP(addr) \
(((*(uint16_t*)addr & 0xe003) == 0x2001) && ((*(uint16_t*)addr & 0x1000) == 0x1000))

#define IS_16BIT_SP_INST(addr) \
(IS_C_ADDI_SP(addr) || IS_C_ADDI16_SP(addr) || IS_C_ADDI4_SP(addr) || IS_C_ADDIW_SP(addr))

#define IS_ADDI_SP(addr) \
((*(uint32_t*)addr & 0x00007fff) == 0x00000013)

#define IS_ADDIW_SP(addr) \
((*(uint32_t*)addr & 0x00007fff) == 0x0000201b)

#define IS_32BIT_SP_INST(addr) \
(IS_ADDI_SP(addr) || IS_ADDIW_SP(addr))

#define IS_NOP_INST(addr) \
()

#define IS_FUNC_INST_END(addr) \
(IS_16BIT_SP_INST(addr) || IS_32BIT_SP_INST(addr))

#define IS_XIP_ADDR(type,addr) \
((type)addr > (type)XIP_START_ADDR && (type)addr < (type)XIP_END_ADDR)

#define MAX_INST_NUM FIND_MAX_INST_NUM
addr_t parse_j_addr(addr_t inst_addr);
#endif /* DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK */

//ringbuf
#include "xr_logger.h"
#define FTRACE_SAVE_LOG(buf, len) \
do { \
	xrlog_write(buf, len); \
} while (0)
int logger_init(void);
void logger_deinit(void);
int logger_read(char *buf, unsigned int size);
void logger_flush(void);

//util
ADD_ATTRIBUTE \
static inline void ftrace_memcpy(char *d, char *s, int l)
{
	while (l--) *d++ = *s++; \
}

#define ftrace_malloc(s) malloc(s)
#define ftrace_free(p) free(p)

#endif /* DYNAMIC_FTRACE */
#endif /* __PORT__H */
