/*
void fpu_ctx_test_read_reg(fpu_reg_t *reg_buf);
void fpu_ctx_test_write_reg(const fpu_reg_t *reg_buf);
*/

#define FPU_REG_SIZE CONFIG_FPU_GENERAL_REG_SIZE

.global fpu_ctx_test_read_reg
.global fpu_ctx_test_write_reg

fpu_ctx_test_read_reg:
	vstr s0, [r1, #0 * FPU_REG_SIZE]
	vstr s1, [r1, #1 * FPU_REG_SIZE]
	vstr s2, [r1, #2 * FPU_REG_SIZE]
	vstr s3, [r1, #3 * FPU_REG_SIZE]
	vstr s4, [r1, #4 * FPU_REG_SIZE]
	vstr s5, [r1, #5 * FPU_REG_SIZE]
	vstr s6, [r1, #6 * FPU_REG_SIZE]
	vstr s7, [r1, #7 * FPU_REG_SIZE]
	vstr s8, [r1, #8 * FPU_REG_SIZE]
	vstr s9, [r1, #9 * FPU_REG_SIZE]
	vstr s10, [r1, #10 * FPU_REG_SIZE]
	vstr s11, [r1, #11 * FPU_REG_SIZE]
	vstr s12, [r1, #12 * FPU_REG_SIZE]
	vstr s13, [r1, #13 * FPU_REG_SIZE]
	vstr s14, [r1, #14 * FPU_REG_SIZE]
	vstr s15, [r1, #15 * FPU_REG_SIZE]
	vstr s16, [r1, #16 * FPU_REG_SIZE]
	bx lr

fpu_ctx_test_write_reg:
	vldr s0, [r0, #0 * FPU_REG_SIZE]
	vldr s1, [r0, #1 * FPU_REG_SIZE]
	vldr s2, [r0, #2 * FPU_REG_SIZE]
	vldr s3, [r0, #3 * FPU_REG_SIZE]
	vldr s4, [r0, #4 * FPU_REG_SIZE]
	vldr s5, [r0, #5 * FPU_REG_SIZE]
	vldr s6, [r0, #6 * FPU_REG_SIZE]
	vldr s7, [r0, #7 * FPU_REG_SIZE]
	vldr s8, [r0, #8 * FPU_REG_SIZE]
	vldr s9, [r0, #9 * FPU_REG_SIZE]
	vldr s10, [r0, #10 * FPU_REG_SIZE]
	vldr s11, [r0, #11 * FPU_REG_SIZE]
	vldr s12, [r0, #12 * FPU_REG_SIZE]
	vldr s13, [r0, #13 * FPU_REG_SIZE]
	vldr s14, [r0, #14 * FPU_REG_SIZE]
	vldr s15, [r0, #15 * FPU_REG_SIZE]
	vldr s16, [r0, #16 * FPU_REG_SIZE]
	vldr s17, [r0, #17 * FPU_REG_SIZE]
	vldr s18, [r0, #18 * FPU_REG_SIZE]
	vldr s19, [r0, #19 * FPU_REG_SIZE]
	vldr s20, [r0, #20 * FPU_REG_SIZE]
	vldr s21, [r0, #21 * FPU_REG_SIZE]
	vldr s22, [r0, #22 * FPU_REG_SIZE]
	vldr s23, [r0, #23 * FPU_REG_SIZE]
	vldr s24, [r0, #24 * FPU_REG_SIZE]
	vldr s25, [r0, #25 * FPU_REG_SIZE]
	vldr s26, [r0, #26 * FPU_REG_SIZE]
	vldr s27, [r0, #27 * FPU_REG_SIZE]
	vldr s28, [r0, #28 * FPU_REG_SIZE]
	vldr s29, [r0, #29 * FPU_REG_SIZE]
	vldr s30, [r0, #30 * FPU_REG_SIZE]
	vldr s31, [r0, #31 * FPU_REG_SIZE]
	bx lr
