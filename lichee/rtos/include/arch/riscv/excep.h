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

#ifndef  __EXPRISCV_INC__
#define  __EXPRISCV_INC__

#ifndef __ASSEMBLY__

#include <stdint.h>

typedef struct
{
    uint64_t vxl;
    uint64_t vxh;
} riscv_vector_vhl;

typedef struct
{
    riscv_vector_vhl v[32];
    uint64_t vl;
    uint64_t vtype;
    uint64_t vstart;
    uint64_t vxsat;
    uint64_t vxrm;
    /* uint64_t vlenb; */
} riscv_v_ext_state_t;

typedef struct
{
    riscv_v_ext_state_t vectorstatus;
} vector_context_t;

// zero, x2(sp), x3(tp) need not backup.
typedef struct
{
    unsigned long mepc;            // 0 * __SIZEOF_LONG__
    unsigned long x1;              // 1 * __SIZEOF_LONG__
    unsigned long x5;              // 5 * __SIZEOF_LONG__
    unsigned long x6;              // 6 * __SIZEOF_LONG__
    unsigned long x7;              // 7 * __SIZEOF_LONG__
    unsigned long x8;              // 8 * __SIZEOF_LONG__
    unsigned long x9;              // 9 * __SIZEOF_LONG__
    unsigned long x10;             //10 * __SIZEOF_LONG__
    unsigned long x11;             //11 * __SIZEOF_LONG__
    unsigned long x12;             //12 * __SIZEOF_LONG__
    unsigned long x13;             //13 * __SIZEOF_LONG__
    unsigned long x14;             //14 * __SIZEOF_LONG__
    unsigned long x15;             //15 * __SIZEOF_LONG__
    unsigned long x16;             //16 * __SIZEOF_LONG__
    unsigned long x17;             //17 * __SIZEOF_LONG__
    unsigned long x18;             //18 * __SIZEOF_LONG__
    unsigned long x19;             //19 * __SIZEOF_LONG__
    unsigned long x20;             //20 * __SIZEOF_LONG__
    unsigned long x21;             //21 * __SIZEOF_LONG__
    unsigned long x22;             //22 * __SIZEOF_LONG__
    unsigned long x23;             //23 * __SIZEOF_LONG__
    unsigned long x24;             //24 * __SIZEOF_LONG__
    unsigned long x25;             //25 * __SIZEOF_LONG__
    unsigned long x26;             //26 * __SIZEOF_LONG__
    unsigned long x27;             //27 * __SIZEOF_LONG__
    unsigned long x28;             //28 * __SIZEOF_LONG__
    unsigned long x29;             //29 * __SIZEOF_LONG__
    unsigned long x30;             //30 * __SIZEOF_LONG__
    unsigned long x31;             //31 * __SIZEOF_LONG__
    unsigned long mstatus;         //32 * __SIZEOF_LONG__
    unsigned long x2;              // 2 * __SIZEOF_LONG__
    unsigned long x3;              // 3 * __SIZEOF_LONG__
    unsigned long x4;              // 4 * __SIZEOF_LONG__
    unsigned long mscratch;        //33 * __SIZEOF_LONG__
} irq_regs_t;

#endif

#define VECTOR_CTX_V0_V0   0   /* offsetof(vector_context_t, vectorstatus.v[0])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V1_V0   16  /* offsetof(vector_context_t, vectorstatus.v[1])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V2_V0   32  /* offsetof(vector_context_t, vectorstatus.v[2])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V3_V0   48  /* offsetof(vector_context_t, vectorstatus.v[3])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V4_V0   64  /* offsetof(vector_context_t, vectorstatus.v[4])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V5_V0   80  /* offsetof(vector_context_t, vectorstatus.v[5])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V6_V0   96  /* offsetof(vector_context_t, vectorstatus.v[6])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V7_V0   112  /* offsetof(vector_context_t, vectorstatus.v[7])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V8_V0   128  /* offsetof(vector_context_t, vectorstatus.v[8])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V9_V0   144  /* offsetof(vector_context_t, vectorstatus.v[9])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V10_V0   160  /* offsetof(vector_context_t, vectorstatus.v[10])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V11_V0   176  /* offsetof(vector_context_t, vectorstatus.v[11])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V12_V0   192  /* offsetof(vector_context_t, vectorstatus.v[12])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V13_V0   208  /* offsetof(vector_context_t, vectorstatus.v[13])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V14_V0   224  /* offsetof(vector_context_t, vectorstatus.v[14])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V15_V0   240  /* offsetof(vector_context_t, vectorstatus.v[15])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V16_V0   256  /* offsetof(vector_context_t, vectorstatus.v[16])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V17_V0   272  /* offsetof(vector_context_t, vectorstatus.v[17])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V18_V0   288  /* offsetof(vector_context_t, vectorstatus.v[18])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V19_V0   304  /* offsetof(vector_context_t, vectorstatus.v[19])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V20_V0   320  /* offsetof(vector_context_t, vectorstatus.v[20])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V21_V0   336  /* offsetof(vector_context_t, vectorstatus.v[21])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V22_V0   352  /* offsetof(vector_context_t, vectorstatus.v[22])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V23_V0   368  /* offsetof(vector_context_t, vectorstatus.v[23])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V24_V0   384  /* offsetof(vector_context_t, vectorstatus.v[24])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V25_V0   400  /* offsetof(vector_context_t, vectorstatus.v[25])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V26_V0   416  /* offsetof(vector_context_t, vectorstatus.v[26])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V27_V0   432  /* offsetof(vector_context_t, vectorstatus.v[27])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V28_V0   448  /* offsetof(vector_context_t, vectorstatus.v[28])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V29_V0   464  /* offsetof(vector_context_t, vectorstatus.v[29])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V30_V0   480  /* offsetof(vector_context_t, vectorstatus.v[30])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_V31_V0   496  /* offsetof(vector_context_t, vectorstatus.v[31])  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_Vl_V0   512  /* offsetof(vector_context_t, vectorstatus.vl)  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_VTYPE_V0   520  /* offsetof(vector_context_t, vectorstatus.vtype)  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_VSTART_V0   528 /* offsetof(vector_context_t, vectorstatus.vstart)  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_VXSAT_V0   536 /* offsetof(vector_context_t, vectorstatus.vstart)  - offsetof(vector_context_t, vectorstatus.v[0]) */
#define VECTOR_CTX_VXRM_V0   544 /* offsetof(vector_context_t, vectorstatus.vstart)  - offsetof(vector_context_t, vectorstatus.v[0]) */

#endif
