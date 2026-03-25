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

#ifndef _RISCV_PMU_H
#define _RISCV_PMU_H

typedef enum
{
    L1_ICACHE_ACCESS_COUNTER = 0x1,
    L1_ICACHE_MISS_COUNTER,
    IUTLB_MISS_COUNTER,
    DUTLB_MISS_COUNTER,
    JTLB_MISS_COUNTER,
    CONDITIONAL_BRANCH_MISPREDICT_COUNTER,
    CONDITIONAL_BRANCH_INSTRUCTION_COUNTER,
    STORE_INST_COUNTER = 0xb,
    L1_DCACHE_READ_ACCESS_COUNTER,
    L1_DCACHE_READ_MISS_COUNTER,
    L1_DCACHE_WRITE_ACCESS_COUNTER,
    L1_DCACHE_WRITE_MISS_COUNTER,
    ALU_INST_COUNTER = 0x1d,
    LOAD_STORE_INST_COUNTER,
    VECTOR_INST_COUNTER,
    CSR_ACCESS_INST_COUNTER,
    SYNC_INST_COUNTER,
    LOAD_STORE_UNALIGN_ACCESS_INST_COUNTER,
    INTERRUPT_COUNTER,
    INTERRUPT_OFF_CYCLE_COUNTER,
    ECALL_INST_COUNTER,
    LONG_JUMP_INST_COUNTER,
    FRONTEND_STALLED_CYCLE_COUNTER,
    BACKEND_STALLED_CYCLE_COUNTER,
    SYNC_STALLED_CYCLE_COUNTER,
    FLOAT_POINT_INST_COUNTER,
} RISCV_PMU_EVENT;

typedef struct
{
    unsigned long l1_icache_access;
    unsigned long l1_icache_miss;
    unsigned long i_uTLB;
    unsigned long d_uTLB;
    unsigned long j_TLB;
    unsigned long branch_mispredict;
    unsigned long branch_inst;
    unsigned long store_inst;
    unsigned long l1_dcache_read_access;
    unsigned long l1_dcache_read_miss;
    unsigned long l1_dcache_write_access;
    unsigned long l1_dcache_write_miss;

    unsigned long alu_inst;
    unsigned long load_store_inst;
    unsigned long vector_inst;
    unsigned long csr_access_inst;
    unsigned long sync_inst;
    unsigned long load_store_unalign_access_inst;
    unsigned long interrupt_num;
    unsigned long interrupt_off_cycle;
    unsigned long ecall_inst;
    unsigned long long_jump_inst;
    unsigned long frontend_stall_cycle;
    unsigned long backend_stall_cycle;
    unsigned long sync_stall_cycle;
    unsigned long float_point_inst;
} rv_pmu_t;

int get_riscv_pmu_info(rv_pmu_t *pmu);

#endif
