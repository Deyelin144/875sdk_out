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
#include "port_.h"

/* debug */
#define MODULE_NAME "[PORT]"
#include "debug.h"

#if DYNAMIC_FTRACE

//inst port
//push multiple registers
//addi sp,sp,-16
ADD_ATTRIBUTE \
int ftrace_convert_caddi_sp_n16(addr_t *start)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = 0;

	typedef union {
		_16bit caddi_instr;
		struct {
			_16bit op : 2;
			_16bit nzimm46875 : 5;
			_16bit sp : 5;
			_16bit nzimm9 : 1;
			_16bit funct3 : 3;
		} INST;
	} CADDI_T;
	CADDI_T CADDI;

	betracked_func_addr = FUNC_ADDR(addr_t, start);
	fix_addr = betracked_func_addr + 0;

	CADDI.INST.op = 0b01;
	CADDI.INST.nzimm46875 = 0b11111;
	CADDI.INST.sp = 0b00010;
	CADDI.INST.nzimm9 = 0b1;
	CADDI.INST.funct3 = 0b011;

	*((_16bit *)fix_addr) = CADDI.caddi_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}
// sd a0,8(sp)
ADD_ATTRIBUTE \
int ftrace_convert_csd_a0_to_sp(addr_t *start)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = 0;

	typedef union {
		_16bit csd_instr;
		struct {
			_16bit op : 2;
			_16bit src : 5;
			_16bit imm_offset5386 : 6;
			_16bit funct3 : 3;
		} INST;
	} CSD_T;
	CSD_T CSD;

	betracked_func_addr = FUNC_ADDR(addr_t, start);
	fix_addr = betracked_func_addr + 1 * 2;

	CSD.INST.op = 0b10;
	CSD.INST.src = 0b01010;
	CSD.INST.imm_offset5386 = 0b001000; //8(sp)
	CSD.INST.funct3 = 0b111;

	*((_16bit *)fix_addr) = CSD.csd_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}
//mv a0,ra
ADD_ATTRIBUTE \
int ftrace_convert_cmv_ra_to_a0(addr_t *start)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = 0;

	typedef union {
		_16bit cmv_instr;
		struct {
			_16bit op : 2;
			_16bit src : 5;
			_16bit dst : 5;
			_16bit funct4 : 4;
		} INST;
	} CMV_T;
	CMV_T CMV;

	betracked_func_addr = FUNC_ADDR(addr_t, start);
	fix_addr = betracked_func_addr + 2 * 2;

	CMV.INST.op = 0b10;
	CMV.INST.src = 0b00001; //x1(ra)
	CMV.INST.dst = 0b01010; //x10(a0)
	CMV.INST.funct4 = 0b1000;

	*((_16bit *)fix_addr) = CMV.cmv_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}

ADD_ATTRIBUTE \
__attribute__((weak)) void ftrace_tracer(void *call_site)
{
	(void)call_site;
	ERR(("[FT] NOT SUPPORT FTARCE, FUNCTION IS NOT COMPELETE.\n"));
}

ADD_ATTRIBUTE \
int ftrace_convert_jal(_8bit *_fix_addr)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = (addr_t)_fix_addr;
	addr_t ftrace_func_entry_addr = (addr_t)ftrace_tracer;
	addr_t PC;
	long offset;

	typedef union {
		_32bit jal_instr;
		struct {
			_32bit op : 7;
			_32bit rd : 5;
			_32bit imm : 20;
		} INST;
	} JAL_T;
	JAL_T JAL;

	typedef union {
		_32bit offset;
		struct {
			_32bit bit_0 : 1;
			_32bit bit_1_10 : 10;
			_32bit bit_11 : 1;
			_32bit bit_12_19 : 8;
			_32bit bit_20 : 1;
		} BIT;
	} BIT_T;
	BIT_T OFFSET;

	JAL.INST.op = 0b1101111;
	JAL.INST.rd = 0b00001; //x1(ra)

	PC = fix_addr; //addi, sd, mv (16bit * 3)
	offset = ftrace_func_entry_addr - PC;
	OFFSET.offset = offset;

	JAL.INST.imm = OFFSET.BIT.bit_12_19 | (OFFSET.BIT.bit_11 << 8) | \
				   (OFFSET.BIT.bit_1_10 << 9) | (OFFSET.BIT.bit_20 << 19) ; //x10(a0)

	*((_32bit *)fix_addr) = JAL.jal_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_32bit *)fix_addr, *(_32bit *)fix_addr));

	return 0;
}
//ld a0,8(sp)
ADD_ATTRIBUTE \
int ftrace_convert_cld_a0_from_sp(addr_t *start)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = 0;

	typedef union {
		_16bit cld_instr;
		struct {
			_16bit op : 2;
			_16bit imm4386 : 5;
			_16bit rd : 5;
			_16bit imm5 : 1;
			_16bit funct3 : 3;
		} INST;
	} CLD_T;
	CLD_T CLD;

	betracked_func_addr = FUNC_ADDR(addr_t, start);
	fix_addr = betracked_func_addr + 5 * 2;

	CLD.INST.op = 0b10;
	CLD.INST.imm4386 = 0b01000;
	CLD.INST.rd = 0b01010; //a0(x10)
	CLD.INST.imm5 = 0b0;
	CLD.INST.funct3 = 0b011;

	*((_16bit *)fix_addr) = CLD.cld_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}
//addi sp,sp, 16
ADD_ATTRIBUTE \
int ftrace_convert_caddi_sp_p16(addr_t *start)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = 0;

	typedef union {
		_16bit caddi_instr;
		struct {
			_16bit op : 2;
			_16bit nzimm46875 : 5;
			_16bit sp : 5;
			_16bit nzimm9 : 1;
			_16bit funct3 : 3;
		} INST;
	} CADDI_T;
	CADDI_T CADDI;

	betracked_func_addr = FUNC_ADDR(addr_t, start);
	fix_addr = betracked_func_addr + 6 * 2;

	CADDI.INST.op = 0b01;
	CADDI.INST.nzimm46875 = 0b10000;
	CADDI.INST.sp = 0b00010;
	CADDI.INST.nzimm9 = 0b0;
	CADDI.INST.funct3 = 0b011;

	*((_16bit *)fix_addr) = CADDI.caddi_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}

ADD_ATTRIBUTE \
int ftrace_convert_cli_t2_imm(_8bit *_fix_addr, _8bit imm6)
{
	addr_t betracked_func_addr = 0;
	addr_t fix_addr = (addr_t)_fix_addr;

	typedef union {
		_16bit cli_instr;
		struct {
			_16bit op : 2;
			_16bit imm40 : 5;
			_16bit rd : 5;
			_16bit imm5 : 1;
			_16bit funct3 : 3;
		} INST;
	} CLI_T;
	CLI_T CLI;

	CLI.INST.op = 0b01;
	CLI.INST.imm40 = imm6 & 0x1f;
	CLI.INST.rd = 7;
	CLI.INST.imm5 = imm6 & 0x20;
	CLI.INST.funct3 = 0b010;

	*((_16bit *)fix_addr) = CLI.cli_instr;
	DBG(("[FT] FIX_ADDR(%p) CODE: %x\n", (_16bit *)fix_addr, *(_16bit *)fix_addr));

	return 0;
}

//find xip_func in isr port
#if DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK
ADD_ATTRIBUTE \
static addr_t get_j16_calladdr(addr_t inst_addr)
{
	addr_t call_addr;
	uint16_t inst = *(uint16_t*)inst_addr;

	typedef union {
		_16bit j_instr;
		struct {
			_16bit op : 2;
			_16bit bit_5 : 1;
			_16bit bit_1_3 : 3;
			_16bit bit_7 : 1;
			_16bit bit_6 : 1;
			_16bit bit_10 : 1;
			_16bit bit_8_9 : 2;
			_16bit bit_4 : 1;
			_16bit bit_11 : 1;
			_16bit funct : 3;
		} INST;
	} J_T;
	J_T J;

	typedef union {
		_16bit offset;
		struct {
			_16bit bit_0 : 1;
			_16bit bit_1_3 : 3;
			_16bit bit_4 : 1;
			_16bit bit_5 : 1;
			_16bit bit_6 : 1;
			_16bit bit_7 : 1;
			_16bit bit_8_9 : 2;
			_16bit bit_10 : 1;
			_16bit bit_11 : 1;
			_16bit sign_extend : 4;
		} BIT;
	} BIT_T;
	BIT_T OFFSET;

	J.j_instr = inst;

	OFFSET.BIT.bit_0 = 0b0;
	OFFSET.BIT.bit_1_3 = J.INST.bit_1_3;
	OFFSET.BIT.bit_4 = J.INST.bit_4;
	OFFSET.BIT.bit_5 = J.INST.bit_5;
	OFFSET.BIT.bit_6 = J.INST.bit_6;
	OFFSET.BIT.bit_7 = J.INST.bit_7;
	OFFSET.BIT.bit_8_9 = J.INST.bit_8_9;
	OFFSET.BIT.bit_10 = J.INST.bit_10;
	OFFSET.BIT.bit_11 = J.INST.bit_11;
	OFFSET.BIT.sign_extend = J.INST.bit_11? 0b1111 : 0b0000;

	call_addr = inst_addr + OFFSET.offset;

	return call_addr;
}

ADD_ATTRIBUTE \
static addr_t get_jal32_calladdr(addr_t inst_addr)
{
	addr_t call_addr;
	uint32_t inst = *(uint32_t*)inst_addr;

	typedef union {
		_32bit jal_instr;
		struct {
			_32bit op : 7;
			_32bit rd : 5;
			_32bit bit_12_19 : 8;
			_32bit bit_11 : 1;
			_32bit bit_1_10 : 10;
			_32bit bit_20 : 1;
		} INST;
	} JAL_T;
	JAL_T JAL;

	typedef union {
		_32bit offset;
		struct {
			_32bit bit_0 : 1;
			_32bit bit_1_10 : 10;
			_32bit bit_11 : 1;
			_32bit bit_12_19 : 8;
			_32bit bit_20 : 1;
			_32bit sign_extend : 11;
		} BIT;
	} BIT_T;
	BIT_T OFFSET;

	JAL.jal_instr = inst;

	OFFSET.BIT.bit_0 = 0b0;
	OFFSET.BIT.bit_1_10 = JAL.INST.bit_1_10;
	OFFSET.BIT.bit_11 = JAL.INST.bit_11;
	OFFSET.BIT.bit_12_19 = JAL.INST.bit_12_19;
	OFFSET.BIT.bit_20 = JAL.INST.bit_20;
	OFFSET.BIT.sign_extend = JAL.INST.bit_20? 0x7ff : 0x000;

	call_addr = inst_addr + OFFSET.offset;

	return call_addr;
}

ADD_ATTRIBUTE \
addr_t parse_j_addr(addr_t inst_addr)
{
	addr_t call_addr = 0;

	if (IS_16BIT_J_INST(inst_addr)) {
		if (IS_C_J(inst_addr)) {
			call_addr = get_j16_calladdr(inst_addr);
		} else if (IS_C_JALR(inst_addr)) {
		//to do
		} else if (IS_C_JR(inst_addr)) {
		//to do
		}
	} else if (IS_32BIT_J_INST(inst_addr)) {
		if (IS_JAL(inst_addr)) {
			call_addr = get_jal32_calladdr(inst_addr);
		} else if (IS_JALR(inst_addr)) {
			addr_t last_32bit_inst_addr = inst_addr - 4;
			if (IS_AUIPC(last_32bit_inst_addr)) {
				uint32_t imm20; uint64_t rd, rs; uint16_t offset;
				imm20 = (*(uint32_t*)last_32bit_inst_addr & 0xfffff000);
				rd = last_32bit_inst_addr + imm20;
				offset = (*(uint32_t*)inst_addr & 0xfff00000) >> 20;
				rs = rd;
				call_addr = rs + offset;
			}
		}
	}

	return call_addr;
}
#endif /* DYNAMIC_FTRACE_ISR_CALL_XIPFUNC_CHECK */

//ringbuf port
static void *_xl;
ADD_ATTRIBUTE \
int logger_init(void)
{
	xrlog_initparam param;

	param.loop = 1;
	param.size = MAX_BUF_SIZE;
	_xl = xrlog_init(&param);
	INF(("xrlog init @ %p RINGBUF SIZE: %d\n", _xl, param.size));
	return 0;
}

ADD_ATTRIBUTE \
void logger_deinit(void)
{
	INF(("xrlog deinit @ %p\n", _xl));
	xrlog_deinit();
}

ADD_ATTRIBUTE \
int logger_read(char *buf, unsigned int size)
{
	int len = 0, ret = 0;

	size = size > MAX_BUF_SIZE? MAX_BUF_SIZE : size;

	if (!buf)
		return -1;

	do {
		ret = xrlog_read(buf, size);
		if (ret > 0) len = ret;
	} while (ret);

	return len;
}

ADD_ATTRIBUTE \
void logger_flush(void)
{
	xrlog_flush();
}

//print
#define PRINT_HEX(h) \
do { \
    int x; \
    int c; \
    for (x = 0; x < 8; x++) \
    { \
        c = (h >> 28) & 0xf; \
        if (c < 10) \
        { \
            hal_uart_put_char(CONFIG_CLI_UART_PORT, '0' + c); \
        } \
        else \
        { \
            hal_uart_put_char(CONFIG_CLI_UART_PORT, 'a' + c - 10); \
        } \
        h <<= 4; \
    } \
} while(0)

#define PRINT_8HEX(h) \
do { \
    int x; \
    int c; \
    for (x = 0; x < 2; x++) \
    { \
        c = (h >> 4) & 0xf; \
        if (c < 10) \
        { \
            hal_uart_put_char(CONFIG_CLI_UART_PORT, '0' + c); \
        } \
        else \
        { \
            hal_uart_put_char(CONFIG_CLI_UART_PORT, 'a' + c - 10); \
        } \
        h <<= 4; \
    } \
} while(0)

ADD_ATTRIBUTE \
void ftrace_print_string(char *s, int _s_len)
{
#if defined(CONFIG_DRIVERS_UART) && !defined(CONFIG_DISABLE_ALL_UART_LOG)
	int s_len = _s_len;

	if (s_len == 0) {
		return ;
	} else if (s_len == -1) {
		s_len = strlen(s);
	}

	for (int i = 0; i < s_len; i ++) {
		hal_uart_put_char(CONFIG_CLI_UART_PORT, s[i]);
	}
#endif
}

ADD_ATTRIBUTE \
void ftrace_print_32hex(int h1, int h2)
{
#if defined(CONFIG_DRIVERS_UART) && !defined(CONFIG_DISABLE_ALL_UART_LOG)
	PRINT_HEX(h1);
	hal_uart_put_char(CONFIG_CLI_UART_PORT, ' ');
	PRINT_HEX(h2);
	hal_uart_put_char(CONFIG_CLI_UART_PORT, ' ');
#endif
}

ADD_ATTRIBUTE \
void ftrace_print_8hex(char h1)
{
#if defined(CONFIG_DRIVERS_UART) && !defined(CONFIG_DISABLE_ALL_UART_LOG)
	PRINT_8HEX(h1);
	hal_uart_put_char(CONFIG_CLI_UART_PORT, ' ');
#endif
}
#endif /* DYNAMIC_FTRACE */
