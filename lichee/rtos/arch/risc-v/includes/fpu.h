#ifndef __RISCV_FPU_H__
#define __RISCV_FPU_H__

#if __riscv_xlen == 64
	#define CPU_WORD_LEN 8
#elif __riscv_xlen == 32
	#define CPU_WORD_LEN 4
#else
	#error The toolchain did not define __riscv_xlen
#endif

#if __riscv_flen == 64
	#define FPU_REG_SIZE 8
#elif __riscv_flen == 32
	#define FPU_REG_SIZE 4
#else
	#error toolchain did not define __riscv_flen
#endif

#define RISCV_FPU_GENERAL_REG_CNT (32)
#define RISCV_FPU_CSR_REG_CNT (1)
#define FPU_CONTEXT_SIZE ((RISCV_FPU_GENERAL_REG_CNT * FPU_REG_SIZE) + (RISCV_FPU_CSR_REG_CNT * CPU_WORD_LEN))

#define FPU_CTX_F0_OFFSET (0 * FPU_REG_SIZE)
#define FPU_CTX_F1_OFFSET (1 * FPU_REG_SIZE)
#define FPU_CTX_F2_OFFSET (2 * FPU_REG_SIZE)
#define FPU_CTX_F3_OFFSET (3 * FPU_REG_SIZE)
#define FPU_CTX_F4_OFFSET (4 * FPU_REG_SIZE)
#define FPU_CTX_F5_OFFSET (5 * FPU_REG_SIZE)
#define FPU_CTX_F6_OFFSET (6 * FPU_REG_SIZE)
#define FPU_CTX_F7_OFFSET (7 * FPU_REG_SIZE)
#define FPU_CTX_F8_OFFSET (8 * FPU_REG_SIZE)
#define FPU_CTX_F9_OFFSET (9 * FPU_REG_SIZE)
#define FPU_CTX_F10_OFFSET (10 * FPU_REG_SIZE)
#define FPU_CTX_F11_OFFSET (11 * FPU_REG_SIZE)
#define FPU_CTX_F12_OFFSET (12 * FPU_REG_SIZE)
#define FPU_CTX_F13_OFFSET (13 * FPU_REG_SIZE)
#define FPU_CTX_F14_OFFSET (14 * FPU_REG_SIZE)
#define FPU_CTX_F15_OFFSET (15 * FPU_REG_SIZE)
#define FPU_CTX_F16_OFFSET (16 * FPU_REG_SIZE)
#define FPU_CTX_F17_OFFSET (17 * FPU_REG_SIZE)
#define FPU_CTX_F18_OFFSET (18 * FPU_REG_SIZE)
#define FPU_CTX_F19_OFFSET (19 * FPU_REG_SIZE)
#define FPU_CTX_F20_OFFSET (20 * FPU_REG_SIZE)
#define FPU_CTX_F21_OFFSET (21 * FPU_REG_SIZE)
#define FPU_CTX_F22_OFFSET (22 * FPU_REG_SIZE)
#define FPU_CTX_F23_OFFSET (23 * FPU_REG_SIZE)
#define FPU_CTX_F24_OFFSET (24 * FPU_REG_SIZE)
#define FPU_CTX_F25_OFFSET (25 * FPU_REG_SIZE)
#define FPU_CTX_F26_OFFSET (26 * FPU_REG_SIZE)
#define FPU_CTX_F27_OFFSET (27 * FPU_REG_SIZE)
#define FPU_CTX_F28_OFFSET (28 * FPU_REG_SIZE)
#define FPU_CTX_F29_OFFSET (29 * FPU_REG_SIZE)
#define FPU_CTX_F30_OFFSET (30 * FPU_REG_SIZE)
#define FPU_CTX_F31_OFFSET (31 * FPU_REG_SIZE)
#define FPU_CTX_FCSR_OFFSET (32 * FPU_REG_SIZE)

#ifndef __ASSEMBLY__

#include <stdint.h>

#if __riscv_xlen == 64
typedef uint64_t riscv_reg_t;
typedef riscv_reg_t riscv_csr_t;
typedef riscv_reg_t fpu_cnt_t;
#elif __riscv_xlen == 32
typedef uint32_t riscv_reg_t;
typedef riscv_reg_t riscv_csr_t;
typedef riscv_reg_t fpu_cnt_t;
#else
#error The toolchain did not define __riscv_xlen
#endif

typedef struct
{
    uint32_t f[32];
    riscv_csr_t fcsr;
} riscv_f_ext_state_t;

typedef struct
{
    uint64_t f[32];
    riscv_csr_t fcsr;
} riscv_d_ext_state_t;

typedef struct
{
    uint64_t f[64] __attribute__((aligned(16)));
    uint32_t fcsr;
    /*
    ¦* Reserved for expansion of sigcontext structure.  Currently zeroed
    ¦* upon signal, and must be zero upon sigreturn.
    ¦*/
    uint32_t reserved[3];
} riscv_q_ext_state_t;

typedef struct
{
#if __riscv_flen == 64
    riscv_d_ext_state_t state;
#elif __riscv_flen == 32
    riscv_f_ext_state_t state;
#else
    #error toolchain did not define __riscv_flen
#endif
} fpu_context_t;

#ifdef CONFIG_RECORD_RISCV_FPU_CTX_STATISTICS
extern fpu_cnt_t g_skip_fpu_ctx_save_cnt;
extern fpu_cnt_t g_skip_fpu_off_ctx_restore_cnt, g_skip_fpu_init_ctx_restore_cnt, g_skip_fpu_clean_ctx_restore_cnt;
#endif

#endif /* __ASSEMBLY__ */


#endif /* __RISCV_FPU_H__ */