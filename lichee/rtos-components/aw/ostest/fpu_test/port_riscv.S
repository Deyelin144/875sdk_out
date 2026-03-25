/*
void fpu_ctx_test_read_reg(fpu_reg_t *reg_buf);
void fpu_ctx_test_write_reg(const fpu_reg_t *reg_buf);
*/

#define FPU_REG_SIZE CONFIG_FPU_GENERAL_REG_SIZE
#if FPU_REG_SIZE == 8
	#define store_reg fsd
	#define load_reg fld
#elif FPU_REG_SIZE == 4
	#define store_reg fsw
	#define load_reg flw
#else
	#error Unsupported FPU general register size!
#endif

.global fpu_ctx_test_read_reg
.global fpu_ctx_test_write_reg

fpu_ctx_test_read_reg:
	store_reg  f0, 0(a0)
	store_reg  f1, 1 * FPU_REG_SIZE(a0)
	store_reg  f2, 2 * FPU_REG_SIZE(a0)
	store_reg  f3, 3 * FPU_REG_SIZE(a0)
	store_reg  f4, 4 * FPU_REG_SIZE(a0)
	store_reg  f5, 5 * FPU_REG_SIZE(a0)
	store_reg  f6, 6 * FPU_REG_SIZE(a0)
	store_reg  f7, 7 * FPU_REG_SIZE(a0)
	store_reg  f8, 8 * FPU_REG_SIZE(a0)
	store_reg  f9, 9 * FPU_REG_SIZE(a0)
	store_reg  f10, 10 * FPU_REG_SIZE(a0)
	store_reg  f11, 11 * FPU_REG_SIZE(a0)
	store_reg  f12, 12 * FPU_REG_SIZE(a0)
	store_reg  f13, 13 * FPU_REG_SIZE(a0)
	store_reg  f14, 14 * FPU_REG_SIZE(a0)
	store_reg  f15, 15 * FPU_REG_SIZE(a0)
	store_reg  f16, 16 * FPU_REG_SIZE(a0)
	store_reg  f17, 17 * FPU_REG_SIZE(a0)
	store_reg  f18, 18 * FPU_REG_SIZE(a0)
	store_reg  f19, 19 * FPU_REG_SIZE(a0)
	store_reg  f20, 20 * FPU_REG_SIZE(a0)
	store_reg  f21, 21 * FPU_REG_SIZE(a0)
	store_reg  f22, 22 * FPU_REG_SIZE(a0)
	store_reg  f23, 23 * FPU_REG_SIZE(a0)
	store_reg  f24, 24 * FPU_REG_SIZE(a0)
	store_reg  f25, 25 * FPU_REG_SIZE(a0)
	store_reg  f26, 26 * FPU_REG_SIZE(a0)
	store_reg  f27, 27 * FPU_REG_SIZE(a0)
	store_reg  f28, 28 * FPU_REG_SIZE(a0)
	store_reg  f29, 29 * FPU_REG_SIZE(a0)
	store_reg  f30, 30 * FPU_REG_SIZE(a0)
	store_reg  f31, 31 * FPU_REG_SIZE(a0)
	ret

fpu_ctx_test_write_reg:
	load_reg  f0, 0(a0)
	load_reg  f1, 1 * FPU_REG_SIZE(a0)
	load_reg  f2, 2 * FPU_REG_SIZE(a0)
	load_reg  f3, 3 * FPU_REG_SIZE(a0)
	load_reg  f4, 4 * FPU_REG_SIZE(a0)
	load_reg  f5, 5 * FPU_REG_SIZE(a0)
	load_reg  f6, 6 * FPU_REG_SIZE(a0)
	load_reg  f7, 7 * FPU_REG_SIZE(a0)
	load_reg  f8, 8 * FPU_REG_SIZE(a0)
	load_reg  f9, 9 * FPU_REG_SIZE(a0)
	load_reg  f10, 10 * FPU_REG_SIZE(a0)
	load_reg  f11, 11 * FPU_REG_SIZE(a0)
	load_reg  f12, 12 * FPU_REG_SIZE(a0)
	load_reg  f13, 13 * FPU_REG_SIZE(a0)
	load_reg  f14, 14 * FPU_REG_SIZE(a0)
	load_reg  f15, 15 * FPU_REG_SIZE(a0)
	load_reg  f16, 16 * FPU_REG_SIZE(a0)
	load_reg  f17, 17 * FPU_REG_SIZE(a0)
	load_reg  f18, 18 * FPU_REG_SIZE(a0)
	load_reg  f19, 19 * FPU_REG_SIZE(a0)
	load_reg  f20, 20 * FPU_REG_SIZE(a0)
	load_reg  f21, 21 * FPU_REG_SIZE(a0)
	load_reg  f22, 22 * FPU_REG_SIZE(a0)
	load_reg  f23, 23 * FPU_REG_SIZE(a0)
	load_reg  f24, 24 * FPU_REG_SIZE(a0)
	load_reg  f25, 25 * FPU_REG_SIZE(a0)
	load_reg  f26, 26 * FPU_REG_SIZE(a0)
	load_reg  f27, 27 * FPU_REG_SIZE(a0)
	load_reg  f28, 28 * FPU_REG_SIZE(a0)
	load_reg  f29, 29 * FPU_REG_SIZE(a0)
	load_reg  f30, 30 * FPU_REG_SIZE(a0)
	load_reg  f31, 31 * FPU_REG_SIZE(a0)
	ret
