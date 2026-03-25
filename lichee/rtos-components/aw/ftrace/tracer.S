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
#include "config.h"
#define RV_CC __riscv
#if RV_CC && DYNAMIC_FTRACE
.text
.global ftrace_function_tracer
.align 2
ftrace_function_tracer:
	addi sp,sp,-80
	sd t0,72(sp)
	sd ra,64(sp)
	sd a0,56(sp)
	sd a1,48(sp)
	sd a2,40(sp)
	sd a3,32(sp)
	sd a4,24(sp)
	sd a5,16(sp)
	sd a6,8(sp)
	sd a7,0(sp)

	mv a0,t0
	mv a1,ra

	jal ra,_ftrace_function_tracer_entry

	ld t0,72(sp)
	ld ra,64(sp)
	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	addi sp,sp,80
	jr ra

#if DYNAMIC_FTRACE_GRAPH
.text
.global ftrace_function_graph_exit
.align 2
ftrace_function_graph_exit:
	addi sp,sp,-64
	sd a0,56(sp)
	sd a1,48(sp)
	sd a2,40(sp)
	sd a3,32(sp)
	sd a4,24(sp)
	sd a5,16(sp)
	sd a6,8(sp)
	sd a7,0(sp)

	jal ra,_ftrace_function_graph_exit

	mv ra, a0

	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	addi sp,sp,64
	jr ra

.data
.align 8
ff_exit_Const:
	.quad ftrace_function_graph_exit

.text
.global ftrace_function_graph_tracer
.align 2
ftrace_function_graph_tracer:
	addi sp,sp,-80
	sd t0,72(sp)
	sd ra,64(sp)
	sd a0,56(sp)
	sd a1,48(sp)
	sd a2,40(sp)
	sd a3,32(sp)
	sd a4,24(sp)
	sd a5,16(sp)
	sd a6,8(sp)
	sd a7,0(sp)

	mv a0,t0
	mv a1,ra
	mv a2,t2

	jal ra,_ftrace_function_graph_tracer_entry

	ld t0,72(sp)
	ld ra,64(sp)
	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	la t0,ff_exit_Const
	ld t0,0(t0)

	addi sp,sp,80
	jr ra
#endif /* DYNAMIC_FTRACE_GRAPH */

#if DYNAMIC_FTRACE_IRQSOFF
#include "irqsoff.h"
.data
.global g_irqsoff_flag
.align 8
g_irqsoff_flag:
	.byte IRQS_ON

.text
.global ftrace_irqsoff_exit
.align 2
ftrace_irqsoff_exit:
	addi sp,sp,-64
	sd a0,56(sp)
	sd a1,48(sp)
	sd a2,40(sp)
	sd a3,32(sp)
	sd a4,24(sp)
	sd a5,16(sp)
	sd a6,8(sp)
	sd a7,0(sp)

	jal ra,_ftrace_irqsoff_exit

	mv ra, a0

	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	addi sp,sp,64
	jr ra

.data
.align 8
irqsoff_exit_Const:
	.quad ftrace_irqsoff_exit

.text
.global ftrace_irqsoff_tracer
.align 2
ftrace_irqsoff_tracer:
	addi sp,sp,-80
	sd t0,72(sp)
	sd ra,64(sp)
	sd a0,56(sp)
	sd a1,48(sp)
	sd a2,40(sp)
	sd a3,32(sp)
	sd a4,24(sp)
	sd a5,16(sp)
	sd a6,8(sp)
	sd a7,0(sp)

	la a0,g_irqsoff_flag
	lbu a0,0(a0)
	li a1, IRQS_OFF
	beq a0,a1,irqsoff_trace
	j return

irqsoff_trace:
	mv a0,t0
	mv a1,ra
	mv a2,t2

	jal ra,_ftrace_irqsoff_tracer_entry

	ld t0,72(sp)
	ld ra,64(sp)
	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	la t0,irqsoff_exit_Const
	ld t0,0(t0)

	addi sp,sp,80
	jr ra

return:
	ld t0,72(sp)
	ld ra,64(sp)
	ld a0,56(sp)
	ld a1,48(sp)
	ld a2,40(sp)
	ld a3,32(sp)
	ld a4,24(sp)
	ld a5,16(sp)
	ld a6,8(sp)
	ld a7,0(sp)

	addi sp,sp,80
	jr ra
#endif /* DYNAMIC_FTRACE_IRQSOFF */

.data
.global ftrace_tracer_cb
.align 8
ftrace_tracer_cb:
	.quad ftrace_function_tracer

.text
.global ftrace_tracer
.align 2
ftrace_tracer:
	la t1,ftrace_tracer_cb
	ld t1,0(t1)
	jr t1

.data
.global ftrace_inst_block
.align 2
ftrace_inst_block:
	mv t0,ra
	li t2,0
	jal ra,ftrace_tracer
	mv ra,t0
	nop

.data
.global ftrace_inst_nop
.align 2
ftrace_inst_nop:
	nop

#endif /* RV_CC && DYNAMIC_FTRACE */
