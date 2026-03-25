
#ifndef __GCC_RISCV_DSP_H__
#define __GCC_RISCV_DSP_H__

#ifndef __riscv_dsp
#error "dsp require -march the 'p' extension"
#endif

typedef signed char int8x4_t __attribute__ ((vector_size (4)));
typedef unsigned char uint8x4_t __attribute__ ((vector_size (4)));
typedef signed char int8x8_t __attribute__ ((vector_size (8)));
typedef unsigned char uint8x8_t __attribute__ ((vector_size (8)));

typedef signed short int16x2_t __attribute__ ((vector_size (4)));
typedef unsigned short uint16x2_t __attribute__ ((vector_size (4)));
typedef signed short int16x4_t __attribute__ ((vector_size (8)));
typedef unsigned short uint16x4_t __attribute__ ((vector_size (8)));

typedef signed int int32x1_t;
typedef unsigned int uint32x1_t;
typedef signed int int32x2_t __attribute__ ((vector_size (8)));
typedef unsigned int uint32x2_t __attribute__ ((vector_size (8)));


#if __riscv_xlen == 32

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sadd16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_add16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_ssub16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_sub16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_scras16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_cras16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_scrsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_crsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sstas16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_stas16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sstsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_stsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_uadd16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_add16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_usub16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_sub16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ucras16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_cras16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ucrsa16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_crsa16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ustas16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_stas16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ustsa16(uint16x2_t a, uint16x2_t b) {
  return (uint16x2_t) __builtin_riscv_stsa16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sslli16(int16x2_t a, const int b) {
  return __builtin_riscv_slli16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_uslli16(uint16x2_t a, const int b) {
  return (uint16x2_t) __builtin_riscv_slli16_v2hi((int16x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_ssll16(int16x2_t a, int b) {
  return __builtin_riscv_sll16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_usll16(uint16x2_t a, int b) {
  return (uint16x2_t) __builtin_riscv_sll16_v2hi((int16x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long sadd64(long long a, long long b) {
  return __builtin_riscv_add64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long ssub64(long long a, long long b) {
  return __builtin_riscv_sub64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long uadd64(unsigned long long a, unsigned long long b) {
  return (unsigned long long) __builtin_riscv_add64((long long)a, (long long)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long usub64(unsigned long long a, unsigned long long b) {
  return (unsigned long long) __builtin_riscv_sub64((long long)a, (long long)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_sadd8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_add8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_ssub8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_sub8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_uadd8(uint8x4_t a, uint8x4_t b) {
  return (uint8x4_t) __builtin_riscv_add8_v4qi((int8x4_t)a, (int8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_usub8(uint8x4_t a, uint8x4_t b) {
  return (uint8x4_t) __builtin_riscv_sub8_v4qi((int8x4_t)a, (int8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_sslli8(int8x4_t a, const int b) {
  return __builtin_riscv_slli8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_uslli8(uint8x4_t a, const int b) {
  return (uint8x4_t) __builtin_riscv_slli8_v4qi((int8x4_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_ssll8(int8x4_t a, int b) {
  return __builtin_riscv_sll8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_usll8(uint8x4_t a, int b) {
  return (uint8x4_t) __builtin_riscv_sll8_v4qi((int8x4_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kabs16(int16x2_t a) {
  return __builtin_riscv_kabs16_v2hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_srai16(int16x2_t a, const int b) {
  return __builtin_riscv_srai16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_srai16_u(int16x2_t a, const int b) {
  return __builtin_riscv_srai16_u_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kslli16(int16x2_t a, const int b) {
  return __builtin_riscv_kslli16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sclip16(int16x2_t a, const int b) {
  return __builtin_riscv_sclip16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sra16(int16x2_t a, int b) {
  return __builtin_riscv_sra16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sra16_u(int16x2_t a, int b) {
  return __builtin_riscv_sra16_u_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_ksll16(int16x2_t a, int b) {
  return __builtin_riscv_ksll16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kslra16(int16x2_t a, int b) {
  return __builtin_riscv_kslra16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kslra16_u(int16x2_t a, int b) {
  return __builtin_riscv_kslra16_u_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_radd16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_radd16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_rcras16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_rcras16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_rcrsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_rcrsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_rstas16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_rstas16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_rstsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_rstsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_rsub16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_rsub16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kadd16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kadd16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kcras16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kcras16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kcrsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kcrsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_ksub16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_ksub16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kstas16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kstas16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_kstsa16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kstsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_khm16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khm16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_khmx16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmx16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_smax16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smax16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_smin16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smin16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sunpkd810(int8x4_t a) {
  return __builtin_riscv_sunpkd810_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sunpkd820(int8x4_t a) {
  return __builtin_riscv_sunpkd820_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sunpkd830(int8x4_t a) {
  return __builtin_riscv_sunpkd830_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sunpkd831(int8x4_t a) {
  return __builtin_riscv_sunpkd831_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x2_t v_sunpkd832(int8x4_t a) {
  return __builtin_riscv_sunpkd832_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smul16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smul16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smulx16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smulx16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smbb16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smbb16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smbt16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smbt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smtt16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smtt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smds(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smds_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smdrs(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smdrs_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smxds(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smxds_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmda(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmda_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmxda(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmxda_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smmwb(int a, int16x2_t b) {
  return __builtin_riscv_smmwb_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smmwb_u(int a, int16x2_t b) {
  return __builtin_riscv_smmwb_u_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smmwt(int a, int16x2_t b) {
  return __builtin_riscv_smmwt_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smmwt_u(int a, int16x2_t b) {
  return __builtin_riscv_smmwt_u_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmwb2(int a, int16x2_t b) {
  return __builtin_riscv_kmmwb2_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmwb2_u(int a, int16x2_t b) {
  return __builtin_riscv_kmmwb2_u_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmwt2(int a, int16x2_t b) {
  return __builtin_riscv_kmmwt2_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmwt2_u(int a, int16x2_t b) {
  return __builtin_riscv_kmmwt2_u_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmabb(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmabb_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmabt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmabt_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmatt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmatt_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmada(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmada_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmaxda(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmaxda_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmads(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmads_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmadrs(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmadrs_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmaxds(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmaxds_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmsda(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmsda_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmsxda(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kmsxda_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawb(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawb_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawb_u(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawb_u_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawb2(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawb2_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawb2_u(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawb2_u_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawt(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawt_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawt_u(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawt_u_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawt2(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawt2_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_kmmawt2_u(int t, int a, int16x2_t b) {
  return __builtin_riscv_kmmawt2_u_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smaqa(int t, int8x4_t a, int8x4_t b) {
  return __builtin_riscv_smaqa_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int v_smaqa_su(int t, int8x4_t a, uint8x4_t b) {
  return __builtin_riscv_smaqa_su_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smal(long long a, int16x2_t b) {
  return __builtin_riscv_smal_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalbb(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalbb_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalbt(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalbt_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smaltt(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smaltt_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalda(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalda_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalxda(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalxda_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalds(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalds_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smaldrs(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smaldrs_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smalxds(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smalxds_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smslda(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smslda_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long v_smslxda(long long t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smslxda_v2hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_kabs8(int8x4_t a) {
  return __builtin_riscv_kabs8_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_srai8(int8x4_t a, const int b) {
  return __builtin_riscv_srai8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_srai8_u(int8x4_t a, const int b) {
  return __builtin_riscv_srai8_u_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_kslli8(int8x4_t a, const int b) {
  return __builtin_riscv_kslli8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_sclip8(int8x4_t a, const int b) {
  return __builtin_riscv_sclip8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_sra8(int8x4_t a, int b) {
  return __builtin_riscv_sra8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_sra8_u(int8x4_t a, int b) {
  return __builtin_riscv_sra8_u_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_ksll8(int8x4_t a, int b) {
  return __builtin_riscv_ksll8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_kslra8(int8x4_t a, int b) {
  return __builtin_riscv_kslra8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_kslra8_u(int8x4_t a, int b) {
  return __builtin_riscv_kslra8_u_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_radd8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_radd8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_rsub8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_rsub8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_kadd8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_kadd8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_ksub8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_ksub8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_khm8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_khm8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_khmx8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_khmx8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_smin8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_smin8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x4_t v_smax8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_smax8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmabb(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmabb_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmabt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmabt_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmatt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmatt_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmbb(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmbb_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmbt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmbt_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmtt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmtt_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmbb(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmbb_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmbt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmbt_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmtt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmtt_si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_scmpeq16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_cmpeq16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ucmpeq16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_cmpeq16_v2hi((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_clrs16(int16x2_t a) {
  return __builtin_riscv_clrs16_v2hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_clo16(int16x2_t a) {
  return __builtin_riscv_clo16_v2hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_clz16(int16x2_t a) {
  return __builtin_riscv_clz16_v2hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_uclip16(int16x2_t a, const int b) {
  return __builtin_riscv_uclip16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_scmplt16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_scmplt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_scmple16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_scmple16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_swap16(uint16x2_t a) {
  return __builtin_riscv_swap16_v2hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_srli16(uint16x2_t a, const int b) {
  return __builtin_riscv_srli16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_srli16_u(uint16x2_t a, const int b) {
  return __builtin_riscv_srli16_u_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_srl16(uint16x2_t a, int b) {
  return __builtin_riscv_srl16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_srl16_u(uint16x2_t a, int b) {
  return __builtin_riscv_srl16_u_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_umax16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umax16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_umin16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umin16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_uradd16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_uradd16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ucmple16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ucmple16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ucmplt16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ucmplt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ukadd16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ukadd16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ursub16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ursub16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_urcras16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_urcras16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_urcrsa16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_urcrsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_urstas16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_urstas16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_urstsa16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_urstsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_uksub16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_uksub16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ukcras16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ukcras16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ukcrsa16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ukcrsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ukstas16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ukstas16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_ukstsa16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_ukstsa16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_zunpkd810(uint8x4_t a) {
  return __builtin_riscv_zunpkd810_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_zunpkd820(uint8x4_t a) {
  return __builtin_riscv_zunpkd820_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_zunpkd830(uint8x4_t a) {
  return __builtin_riscv_zunpkd830_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_zunpkd831(uint8x4_t a) {
  return __builtin_riscv_zunpkd831_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x2_t v_zunpkd832(uint8x4_t a) {
  return __builtin_riscv_zunpkd832_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umul16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umul16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umulx16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umulx16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned int v_pkbb16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_pkbb16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned int v_pkbt16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_pkbt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned int v_pktb16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_pktb16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned int v_pktt16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_pktt16_v2hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned int v_umaqa(unsigned int t, uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_umaqa_si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_scmpeq8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_cmpeq8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_ucmpeq8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_cmpeq8_v4qi((int8x4_t)a, (int8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_clrs8(int8x4_t a) {
  return __builtin_riscv_clrs8_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_clo8(int8x4_t a) {
  return __builtin_riscv_clo8_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_clz8(int8x4_t a) {
  return __builtin_riscv_clz8_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_uclip8(int8x4_t a, const int b) {
  return __builtin_riscv_uclip8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_scmplt8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_scmplt8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_scmple8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_scmple8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_swap8(uint8x4_t a) {
  return __builtin_riscv_swap8_v4qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_srli8(uint8x4_t a, const int b) {
  return __builtin_riscv_srli8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_srli8_u(uint8x4_t a, const int b) {
  return __builtin_riscv_srli8_u_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_srl8(uint8x4_t a, int b) {
  return __builtin_riscv_srl8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_srl8_u(uint8x4_t a, int b) {
  return __builtin_riscv_srl8_u_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_umin8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_umin8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_umax8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_umax8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_uradd8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_uradd8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_ukadd8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_ukadd8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_ucmple8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_ucmple8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_ursub8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_ursub8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_uksub8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_uksub8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x4_t v_ucmplt8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_ucmplt8_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pbsad(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_pbsad_v4qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pbsada(unsigned long t, uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_pbsada_si(t, a, b);
}

#endif

#if __riscv_xlen == 64

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sadd16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_add16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_ssub16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_sub16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_scras16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_cras16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_scrsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_crsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sstas16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_stas16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sstsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_stsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_uadd16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_add16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_usub16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_sub16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ucras16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_cras16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ucrsa16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_crsa16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ustas16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_stas16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ustsa16(uint16x4_t a, uint16x4_t b) {
  return (uint16x4_t) __builtin_riscv_stsa16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sslli16(int16x4_t a, const int b) {
  return __builtin_riscv_slli16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_uslli16(uint16x4_t a, const int b) {
  return (uint16x4_t) __builtin_riscv_slli16_v4hi((int16x4_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_ssll16(int16x4_t a, int b) {
  return __builtin_riscv_sll16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_usll16(uint16x4_t a, int b) {
  return (uint16x4_t) __builtin_riscv_sll16_v4hi((int16x4_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long add32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_add32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sub32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_sub32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long cras32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_cras32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long crsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_crsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long stas32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_stas32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long stsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_stsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sadd32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_add32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_ssub32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_sub32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_scras32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_cras32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_scrsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_crsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sstas32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_stas32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sstsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_stsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_uadd32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_add32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_usub32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_sub32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ucras32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_cras32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ucrsa32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_crsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ustas32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_stas32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ustsa32(uint32x2_t a, uint32x2_t b) {
  return (uint32x2_t) __builtin_riscv_stsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long slli32(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_slli32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sslli32(int32x2_t a, const int b) {
  return __builtin_riscv_slli32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_uslli32(uint32x2_t a, const int b) {
  return (uint32x2_t) __builtin_riscv_slli32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sll32(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_sll32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_ssll32(int32x2_t a, int b) {
  return __builtin_riscv_sll32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_usll32(uint32x2_t a, int b) {
  return (uint32x2_t) __builtin_riscv_sll32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_sadd8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_add8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_ssub8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_sub8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_uadd8(uint8x8_t a, uint8x8_t b) {
  return (uint8x8_t) __builtin_riscv_add8_v8qi((int8x8_t)a, (int8x8_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_usub8(uint8x8_t a, uint8x8_t b) {
  return (uint8x8_t) __builtin_riscv_sub8_v8qi((int8x8_t)a, (int8x8_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_sslli8(int8x8_t a, const int b) {
  return __builtin_riscv_slli8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_uslli8(uint8x8_t a, const int b) {
  return (uint8x8_t) __builtin_riscv_slli8_v8qi((int8x8_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_ssll8(int8x8_t a, int b) {
  return __builtin_riscv_sll8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_usll8(uint8x8_t a, int b) {
  return (uint8x8_t) __builtin_riscv_sll8_v8qi((int8x8_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_smul8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_smul8(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_smulx8(int8x4_t a, int8x4_t b) {
  return __builtin_riscv_smulx8(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kabs16(int16x4_t a) {
  return __builtin_riscv_kabs16_v4hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_srai16(int16x4_t a, const int b) {
  return __builtin_riscv_srai16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_srai16_u(int16x4_t a, const int b) {
  return __builtin_riscv_srai16_u_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kslli16(int16x4_t a, const int b) {
  return __builtin_riscv_kslli16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sclip16(int16x4_t a, const int b) {
  return __builtin_riscv_sclip16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sra16(int16x4_t a, int b) {
  return __builtin_riscv_sra16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sra16_u(int16x4_t a, int b) {
  return __builtin_riscv_sra16_u_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_ksll16(int16x4_t a, int b) {
  return __builtin_riscv_ksll16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kslra16(int16x4_t a, int b) {
  return __builtin_riscv_kslra16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kslra16_u(int16x4_t a, int b) {
  return __builtin_riscv_kslra16_u_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_radd16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_radd16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_rcras16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_rcras16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_rcrsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_rcrsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_rstas16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_rstas16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_rstsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_rstsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_rsub16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_rsub16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kadd16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kadd16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kcras16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kcras16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kcrsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kcrsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_ksub16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_ksub16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kstas16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kstas16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_kstsa16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kstsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_khm16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_khm16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_khmx16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_khmx16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_smax16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smax16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_smin16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smin16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sunpkd810(int8x8_t a) {
  return __builtin_riscv_sunpkd810_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sunpkd820(int8x8_t a) {
  return __builtin_riscv_sunpkd820_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sunpkd830(int8x8_t a) {
  return __builtin_riscv_sunpkd830_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sunpkd831(int8x8_t a) {
  return __builtin_riscv_sunpkd831_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int16x4_t v_sunpkd832(int8x8_t a) {
  return __builtin_riscv_sunpkd832_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smul16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smul16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smulx16(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_smulx16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smbb16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smbb16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smbt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smbt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smtt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smtt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smds(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smds_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smdrs(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smdrs_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smxds(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smxds_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmda(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmda_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmxda(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmxda_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmbb16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmbb16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmbt16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmbt16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmtt16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmtt16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmbb16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmbb16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmbt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmbt16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmtt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmtt16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khmbb16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_khmbb16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khmbt16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_khmbt16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khmtt16(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_khmtt16((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_khmbb16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_khmbb16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_khmbt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_khmbt16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_khmtt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_khmtt16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kabs32(unsigned long a) {
  return (unsigned long) __builtin_riscv_kabs32((int32x2_t)a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kabs32(int32x2_t a) {
  return __builtin_riscv_kabs32(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sclip32(int32x2_t a, const int b) {
  return __builtin_riscv_sclip32_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai32(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_srai32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai32_u(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_srai32_u((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslli32(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_kslli32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_srai32(int32x2_t a, const int b) {
  return __builtin_riscv_srai32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_srai32_u(int32x2_t a, const int b) {
  return __builtin_riscv_srai32_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kslli32(int32x2_t a, const int b) {
  return __builtin_riscv_kslli32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra32(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_sra32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra32_u(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_sra32_u((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksll32(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_ksll32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra32(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_kslra32((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra32_u(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_kslra32_u((int32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sra32(int32x2_t a, int b) {
  return __builtin_riscv_sra32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_sra32_u(int32x2_t a, int b) {
  return __builtin_riscv_sra32_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_ksll32(int32x2_t a, int b) {
  return __builtin_riscv_ksll32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kslra32(int32x2_t a, int b) {
  return __builtin_riscv_kslra32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kslra32_u(int32x2_t a, int b) {
  return __builtin_riscv_kslra32_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmwb(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_smmwb_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmwb_u(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_smmwb_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmwt(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_smmwt_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmwt_u(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_smmwt_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmwb2(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmwb2_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmwb2_u(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmwb2_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmwt2(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmwt2_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmwt2_u(int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmwt2_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmul(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smmul_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smmul_u(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smmul_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kwmmul(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kwmmul_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kwmmul_u(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kwmmul_u_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kadd32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kadd32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kcras32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kcras32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kcrsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kcrsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kadd32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kadd32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kcras32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kcras32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kcrsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kcrsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smax32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_smax32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smin32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_smin32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kstas32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kstas32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kstsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kstsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksub32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ksub32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long radd32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_radd32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rcras32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_rcras32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rcrsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_rcrsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rstas32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_rstas32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rstsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_rstsa32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rsub32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_rsub32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smax32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smax32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smin32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smin32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kstas32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kstas32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kstsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kstsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_ksub32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_ksub32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_radd32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_radd32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_rcras32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_rcras32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_rcrsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_rcrsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_rstas32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_rstas32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_rstsa32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_rstsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_rsub32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_rsub32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmabb(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmabb_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmabt(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmabt_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmatt(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmatt_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmada(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmada_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmaxda(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmaxda_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmads(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmads_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmadrs(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmadrs_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmaxds(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmaxds_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmsda(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmsda_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmsxda(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kmsxda_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmabb16(unsigned long t, unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmabb16((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmabt16(unsigned long t, unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmabt16((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kdmatt16(unsigned long t, unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_kdmatt16((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmabb16(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmabb16(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmabt16(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmabt16(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kdmatt16(int32x2_t t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_kdmatt16(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawb(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawb_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawb_u(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawb_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawb2(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawb2_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawb2_u(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawb2_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawt(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawt_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawt_u(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawt_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawt2(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawt2_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmawt2_u(int32x2_t t, int32x2_t a, int16x4_t b) {
  return __builtin_riscv_kmmawt2_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmac(int32x2_t t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmmac_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmac_u(int32x2_t t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmmac_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmsb(int32x2_t t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmmsb_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_kmmsb_u(int32x2_t t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmmsb_u_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smaqa(int32x2_t t, int8x8_t a, int8x8_t b) {
  return __builtin_riscv_smaqa_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int32x2_t v_smaqa_su(int32x2_t t, int8x8_t a, uint8x8_t b) {
  return __builtin_riscv_smaqa_su_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long sraiw_u(int a, const int b) {
  return __builtin_riscv_sraiw_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmda32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmda32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmxda32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmxda32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmda32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmda32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmxda32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmxda32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smbb32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smbb32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smbt32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smbt32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smtt32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smtt32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smds32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smds32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smdrs32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smdrs32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smxds32(unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_smxds32((int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smbb32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smbb32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smbt32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smbt32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smtt32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smtt32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smds32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smds32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smdrs32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smdrs32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smxds32(int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smxds32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smal(long a, int16x4_t b) {
  return __builtin_riscv_smal_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalbb(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalbb_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalbt(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalbt_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smaltt(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smaltt_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalda(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalda_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalxda(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalxda_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalds(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalds_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smaldrs(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smaldrs_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smalxds(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smalxds_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smslda(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smslda_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smslxda(long t, int16x4_t a, int16x4_t b) {
  return __builtin_riscv_smslxda_v4hi(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smar64(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smar64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_smsr64(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_smsr64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmar64(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmar64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmsr64(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmsr64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmabb32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmabb32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmabt32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmabt32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmatt32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmatt32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmada32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmada32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmaxda32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmaxda32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmads32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmads32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmadrs32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmadrs32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmaxds32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmaxds32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmsda32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmsda32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmsxda32(long long t, unsigned long a, unsigned long b) {
  return (long long) __builtin_riscv_kmsxda32((long)t, (int32x2_t)a, (int32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmabb32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmabb32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmabt32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmabt32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmatt32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmatt32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmada32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmada32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmaxda32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmaxda32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmads32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmads32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmadrs32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmadrs32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmaxds32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmaxds32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmsda32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmsda32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kmsxda32(long t, int32x2_t a, int32x2_t b) {
  return __builtin_riscv_kmsxda32(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_kabs8(int8x8_t a) {
  return __builtin_riscv_kabs8_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_srai8(int8x8_t a, const int b) {
  return __builtin_riscv_srai8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_srai8_u(int8x8_t a, const int b) {
  return __builtin_riscv_srai8_u_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_kslli8(int8x8_t a, const int b) {
  return __builtin_riscv_kslli8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_sclip8(int8x8_t a, const int b) {
  return __builtin_riscv_sclip8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_sra8(int8x8_t a, int b) {
  return __builtin_riscv_sra8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_sra8_u(int8x8_t a, int b) {
  return __builtin_riscv_sra8_u_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_ksll8(int8x8_t a, int b) {
  return __builtin_riscv_ksll8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_kslra8(int8x8_t a, int b) {
  return __builtin_riscv_kslra8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_kslra8_u(int8x8_t a, int b) {
  return __builtin_riscv_kslra8_u_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_radd8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_radd8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_rsub8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_rsub8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_kadd8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_kadd8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_ksub8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_ksub8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_khm8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_khm8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_khmx8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_khmx8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_smin8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_smin8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
int8x8_t v_smax8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_smax8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmabb(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmabb_di(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmabt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmabt_di(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmatt(int t, int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmatt_di(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmbb(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmbb_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmbt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmbt_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_khmtt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_khmtt_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmbb(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmbb_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmbt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmbt_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long v_kdmtt(int16x2_t a, int16x2_t b) {
  return __builtin_riscv_kdmtt_di(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_umul8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_umul8(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_umulx8(uint8x4_t a, uint8x4_t b) {
  return __builtin_riscv_umulx8(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_scmpeq16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_cmpeq16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ucmpeq16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_cmpeq16_v4hi((int16x4_t)a, (int16x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_clrs16(int16x4_t a) {
  return __builtin_riscv_clrs16_v4hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_clo16(int16x4_t a) {
  return __builtin_riscv_clo16_v4hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_clz16(int16x4_t a) {
  return __builtin_riscv_clz16_v4hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_uclip16(int16x4_t a, const int b) {
  return __builtin_riscv_uclip16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_scmplt16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_scmplt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_scmple16(int16x4_t a, int16x4_t b) {
  return __builtin_riscv_scmple16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_swap16(uint16x4_t a) {
  return __builtin_riscv_swap16_v4hi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_srli16(uint16x4_t a, const int b) {
  return __builtin_riscv_srli16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_srli16_u(uint16x4_t a, const int b) {
  return __builtin_riscv_srli16_u_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_srl16(uint16x4_t a, int b) {
  return __builtin_riscv_srl16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_srl16_u(uint16x4_t a, int b) {
  return __builtin_riscv_srl16_u_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_umax16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_umax16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_umin16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_umin16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_uradd16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_uradd16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ucmple16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ucmple16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ucmplt16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ucmplt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ukadd16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ukadd16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ursub16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ursub16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_urcras16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_urcras16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_urcrsa16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_urcrsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_urstas16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_urstas16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_urstsa16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_urstsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_uksub16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_uksub16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ukcras16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ukcras16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ukcrsa16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ukcrsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ukstas16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ukstas16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_ukstsa16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_ukstsa16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_zunpkd810(uint8x8_t a) {
  return __builtin_riscv_zunpkd810_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_zunpkd820(uint8x8_t a) {
  return __builtin_riscv_zunpkd820_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_zunpkd830(uint8x8_t a) {
  return __builtin_riscv_zunpkd830_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_zunpkd831(uint8x8_t a) {
  return __builtin_riscv_zunpkd831_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint16x4_t v_zunpkd832(uint8x8_t a) {
  return __builtin_riscv_zunpkd832_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umul16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umul16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umulx16(uint16x2_t a, uint16x2_t b) {
  return __builtin_riscv_umulx16(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_clrs32(int32x2_t a) {
  return __builtin_riscv_clrs32_v2si(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_clo32(int32x2_t a) {
  return __builtin_riscv_clo32_v2si(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_clz32(int32x2_t a) {
  return __builtin_riscv_clz32_v2si(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_uclip32(int32x2_t a, const int b) {
  return __builtin_riscv_uclip32_v2si(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_pkbb16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_pkbb16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_pkbt16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_pkbt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_pktb16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_pktb16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_pktt16(uint16x4_t a, uint16x4_t b) {
  return __builtin_riscv_pktt16_v4hi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli32(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_srli32((uint32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli32_u(unsigned long a, const int b) {
  return (unsigned long) __builtin_riscv_srli32_u((uint32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_srli32(uint32x2_t a, const int b) {
  return __builtin_riscv_srli32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_srli32_u(uint32x2_t a, const int b) {
  return __builtin_riscv_srli32_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl32(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_srl32((uint32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl32_u(unsigned long a, int b) {
  return (unsigned long) __builtin_riscv_srl32_u((uint32x2_t)a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_srl32(uint32x2_t a, int b) {
  return __builtin_riscv_srl32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_srl32_u(uint32x2_t a, int b) {
  return __builtin_riscv_srl32_u(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukadd32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ukadd32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukcras32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ukcras32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukcrsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ukcrsa32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukstas32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ukstas32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukstsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ukstsa32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uksub32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_uksub32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umax32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_umax32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umin32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_umin32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uradd32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_uradd32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urcras32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_urcras32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urcrsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_urcrsa32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urstas32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_urstas32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urstsa32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_urstsa32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ursub32(unsigned long a, unsigned long b) {
  return (unsigned long) __builtin_riscv_ursub32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ukadd32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukadd32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ukcras32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukcras32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ukcrsa32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukcrsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ukstas32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukstas32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ukstsa32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukstsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_uksub32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_uksub32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umax32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_umax32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umin32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_umin32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_uradd32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_uradd32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_urcras32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_urcras32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_urcrsa32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_urcrsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_urstas32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_urstas32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_urstsa32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_urstsa32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_ursub32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ursub32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint32x2_t v_umaqa(uint32x2_t t, uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_umaqa_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long pkbb32(unsigned long a, unsigned long b) {
  return (unsigned long long) __builtin_riscv_pkbb32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long pkbt32(unsigned long a, unsigned long b) {
  return (unsigned long long) __builtin_riscv_pkbt32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long pktb32(unsigned long a, unsigned long b) {
  return (unsigned long long) __builtin_riscv_pktb32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long pktt32(unsigned long a, unsigned long b) {
  return (unsigned long long) __builtin_riscv_pktt32((uint32x2_t)a, (uint32x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pkbb32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_pkbb32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pkbt32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_pkbt32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pktb32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_pktb32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pktt32(uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_pktt32(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_ukmar64(unsigned long t, uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukmar64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_ukmsr64(unsigned long t, uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_ukmsr64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_umar64(unsigned long t, uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_umar64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_umsr64(unsigned long t, uint32x2_t a, uint32x2_t b) {
  return __builtin_riscv_umsr64_v2si(t, a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_scmpeq8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_cmpeq8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_ucmpeq8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_cmpeq8_v8qi((int8x8_t)a, (int8x8_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_clrs8(int8x8_t a) {
  return __builtin_riscv_clrs8_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_clo8(int8x8_t a) {
  return __builtin_riscv_clo8_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_clz8(int8x8_t a) {
  return __builtin_riscv_clz8_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_uclip8(int8x8_t a, const int b) {
  return __builtin_riscv_uclip8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_scmplt8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_scmplt8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_scmple8(int8x8_t a, int8x8_t b) {
  return __builtin_riscv_scmple8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_swap8(uint8x8_t a) {
  return __builtin_riscv_swap8_v8qi(a);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_srli8(uint8x8_t a, const int b) {
  return __builtin_riscv_srli8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_srli8_u(uint8x8_t a, const int b) {
  return __builtin_riscv_srli8_u_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_srl8(uint8x8_t a, int b) {
  return __builtin_riscv_srl8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_srl8_u(uint8x8_t a, int b) {
  return __builtin_riscv_srl8_u_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_umin8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_umin8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_umax8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_umax8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_uradd8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_uradd8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_ukadd8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_ukadd8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_ucmple8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_ucmple8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_ursub8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_ursub8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_uksub8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_uksub8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
uint8x8_t v_ucmplt8(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_ucmplt8_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pbsad(uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_pbsad_v8qi(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long v_pbsada(unsigned long t, uint8x8_t a, uint8x8_t b) {
  return __builtin_riscv_pbsada_di(t, a, b);
}

#endif

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long add16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_add16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_add16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sub16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sub16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sub16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long cras16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_cras16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_cras16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long crsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_crsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_crsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long stas16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_stas16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_stas16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long stsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_stsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_stsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long slli16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_slli16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_slli16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sll16(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sll16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sll16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long add8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_add8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_add8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sub8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sub8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sub8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long slli8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_slli8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_slli8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sll8(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sll8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sll8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long smul8(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_smul8((int8x4_t)a, (int8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long smulx8(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_smulx8((int8x4_t)a, (int8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kabs16(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kabs16_v4hi((int16x4_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kabs16_v2hi((int16x2_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srai16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srai16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai16_u(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srai16_u_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srai16_u_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslli16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslli16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslli16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sclip16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sclip16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sclip16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra16(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sra16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sra16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra16_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sra16_u_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sra16_u_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksll16(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ksll16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ksll16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra16(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslra16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslra16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra16_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslra16_u_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslra16_u_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long radd16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_radd16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_radd16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rcras16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rcras16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rcras16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rcrsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rcrsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rcrsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rstas16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rstas16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rstas16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rstsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rstsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rstsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rsub16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rsub16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rsub16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kadd16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kadd16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kadd16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kcras16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kcras16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kcras16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kcrsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kcrsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kcrsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksub16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ksub16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ksub16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kstas16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kstas16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kstas16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kstsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kstsa16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kstsa16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khm16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_khm16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_khm16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khmx16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_khmx16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_khmx16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smax16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smax16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_smax16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smin16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smin16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_smin16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sunpkd810(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sunpkd810_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sunpkd810_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sunpkd820(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sunpkd820_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sunpkd820_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sunpkd830(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sunpkd830_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sunpkd830_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sunpkd831(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sunpkd831_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sunpkd831_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sunpkd832(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sunpkd832_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sunpkd832_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long smul16(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_smul16((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long smulx16(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_smulx16((int16x2_t)a, (int16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smbb16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smbb16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smbb16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smbt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smbt16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smbt16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smtt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smtt16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smtt16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smds(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smds_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smds_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smdrs(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smdrs_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smdrs_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smxds(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smxds_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smxds_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmda(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmda_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmda_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmxda(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmxda_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmxda_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sclip32(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sclip32_v2si((int32x2_t)a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_sclip32_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmwb(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmwb_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmwb_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmwb_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmwb_u_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmwb_u_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmwt(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmwt_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmwt_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmwt_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmwt_u_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmwt_u_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmwb2(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmwb2_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmwb2_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmwb2_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmwb2_u_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmwb2_u_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmwt2(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmwt2_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmwt2_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmwt2_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmwt2_u_v2si((int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmwt2_u_si(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmul(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmul_v2si((int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmul_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smmul_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smmul_u_v2si((int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smmul_u_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kwmmul(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kwmmul_v2si((int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kwmmul_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kwmmul_u(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kwmmul_u_v2si((int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kwmmul_u_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmabb(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmabb_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmabb_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmabt(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmabt_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmabt_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmatt(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmatt_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmatt_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmada(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmada_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmada_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmaxda(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmaxda_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmaxda_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmads(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmads_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmads_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmadrs(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmadrs_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmadrs_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmaxds(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmaxds_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmaxds_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmsda(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmsda_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmsda_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmsxda(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmsxda_v2si((int32x2_t)t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmsxda_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawb(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawb_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawb_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawb_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawb_u_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawb_u_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawb2(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawb2_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawb2_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawb2_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawb2_u_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawb2_u_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawt(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawt_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawt_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawt_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawt_u_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawt_u_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawt2(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawt2_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawt2_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmawt2_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmawt2_u_v2si((int32x2_t)t, (int32x2_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmawt2_u_si(t, a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmac(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmac_v2si((int32x2_t)t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmac_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmac_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmac_u_v2si((int32x2_t)t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmac_u_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmsb(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmsb_v2si((int32x2_t)t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmsb_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kmmsb_u(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kmmsb_u_v2si((int32x2_t)t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmmsb_u_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smaqa(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smaqa_v2si((int32x2_t)t, (int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smaqa_si(t, (int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smaqa_su(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smaqa_su_v2si((int32x2_t)t, (int8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smaqa_su_si(t, (int8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long mulsr64(int a, int b) {
  return __builtin_riscv_mulsr64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smal(long long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smal_v4hi(a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smal_v2hi(a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kadd64(long long a, long long b) {
  return __builtin_riscv_kadd64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long ksub64(long long a, long long b) {
  return __builtin_riscv_ksub64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long radd64(long long a, long long b) {
  return __builtin_riscv_radd64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long rsub64(long long a, long long b) {
  return __builtin_riscv_rsub64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalbb(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalbb_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalbb_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalbt(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalbt_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalbt_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smaltt(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smaltt_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smaltt_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalda(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalda_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalda_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalxda(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalxda_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalxda_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalds(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalds_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalds_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smaldrs(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smaldrs_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smaldrs_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smalxds(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smalxds_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smalxds_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smslda(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smslda_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smslda_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smslxda(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smslxda_v4hi(t, (int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smslxda_v2hi(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smar64(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smar64_v2si(t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smar64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long smsr64(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_smsr64_v2si(t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_smsr64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmar64(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kmar64_v2si(t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmar64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long long kmsr64(long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kmsr64_v2si(t, (int32x2_t)a, (int32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kmsr64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kabs8(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kabs8_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kabs8_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srai8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srai8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srai8_u(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srai8_u_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srai8_u_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslli8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslli8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslli8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sclip8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sclip8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sclip8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra8(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sra8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sra8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long sra8_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_sra8_u_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_sra8_u_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksll8(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ksll8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ksll8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra8(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslra8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslra8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kslra8_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kslra8_u_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kslra8_u_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long radd8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_radd8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_radd8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long rsub8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_rsub8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_rsub8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long kadd8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_kadd8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_kadd8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ksub8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ksub8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ksub8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khm8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_khm8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_khm8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long khmx8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_khmx8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_khmx8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smin8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smin8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_smin8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long smax8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_smax8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_smax8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long smaddr32(int t, int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_maddr32_di(t, a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_maddr32_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long smsubr32(int t, int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_msubr32_di(t, a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_msubr32_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long umaddr32(unsigned int t, unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_maddr32_di((int)t, (int)a, (int)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_maddr32_si((int)t, (int)a, (int)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long umsubr32(unsigned int t, unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_msubr32_di((int)t, (int)a, (int)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_msubr32_si((int)t, (int)a, (int)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kabsw(int a) {
#if __riscv_xlen == 64
  return __builtin_riscv_kabsw_di(a);
#elif __riscv_xlen == 32
  return __builtin_riscv_kabsw_si(a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kslliw(int a, const int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kslliw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kslliw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kaddh(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kaddh_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kaddh_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long ksubh(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ksubh_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ksubh_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kaddw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kaddw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kaddw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long ksubw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ksubw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ksubw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long raddw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_raddw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_raddw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long rsubw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_rsubw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_rsubw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long maxw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_maxw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_maxw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long minw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_minw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_minw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kslraw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kslraw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kslraw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kslraw_u(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kslraw_u_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kslraw_u_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long ksllw(int a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ksllw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ksllw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmabb(int t, unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmabb_di(t, (int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmabb_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmabt(int t, unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmabt_di(t, (int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmabt_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmatt(int t, unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmatt_di(t, (int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmatt_si(t, (int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long khmbb(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_khmbb_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_khmbb_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long khmbt(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_khmbt_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_khmbt_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long khmtt(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_khmtt_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_khmtt_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmbb(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmbb_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmbb_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmbt(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmbt_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmbt_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long kdmtt(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_kdmtt_di((int16x2_t)a, (int16x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_kdmtt_si((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long srai_u(long a, const int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_srai_u_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_srai_u_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long sra_u(long a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_sra_u_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_sra_u_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long ave(long a, long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ave_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ave_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long wexti(unsigned long long a, const int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_wexti_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_wexti_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
long wext(unsigned long long a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_wext_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_wext_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umul8(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_umul8((uint8x4_t)a, (uint8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umulx8(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_umulx8((uint8x4_t)a, (uint8x4_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long cmpeq16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_cmpeq16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_cmpeq16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clrs16(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clrs16_v4hi((int16x4_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clrs16_v2hi((int16x2_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clo16(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clo16_v4hi((int16x4_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clo16_v2hi((int16x2_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clz16(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clz16_v4hi((int16x4_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clz16_v2hi((int16x2_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uclip16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uclip16_v4hi((int16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uclip16_v2hi((int16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long scmplt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_scmplt16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_scmplt16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long scmple16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_scmple16_v4hi((int16x4_t)a, (int16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_scmple16_v2hi((int16x2_t)a, (int16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long swap16(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_swap16_v4hi((uint16x4_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_swap16_v2hi((uint16x2_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli16(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srli16_v4hi((uint16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srli16_v2hi((uint16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli16_u(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srli16_u_v4hi((uint16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srli16_u_v2hi((uint16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl16(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srl16_v4hi((uint16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srl16_v2hi((uint16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl16_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srl16_u_v4hi((uint16x4_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srl16_u_v2hi((uint16x2_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umax16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_umax16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_umax16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umin16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_umin16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_umin16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uradd16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uradd16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uradd16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ucmple16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ucmple16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ucmple16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ucmplt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ucmplt16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ucmplt16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukadd16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukadd16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukadd16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ursub16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ursub16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ursub16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urcras16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_urcras16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_urcras16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urcrsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_urcrsa16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_urcrsa16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urstas16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_urstas16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_urstas16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long urstsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_urstsa16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_urstsa16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uksub16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uksub16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uksub16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukcras16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukcras16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukcras16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukcrsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukcrsa16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukcrsa16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukstas16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukstas16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukstas16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukstsa16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukstsa16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukstsa16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long zunpkd810(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_zunpkd810_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_zunpkd810_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long zunpkd820(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_zunpkd820_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_zunpkd820_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long zunpkd830(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_zunpkd830_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_zunpkd830_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long zunpkd831(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_zunpkd831_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_zunpkd831_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long zunpkd832(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_zunpkd832_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_zunpkd832_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umul16(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_umul16((uint16x2_t)a, (uint16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umulx16(unsigned int a, unsigned int b) {
  return (unsigned long long) __builtin_riscv_umulx16((uint16x2_t)a, (uint16x2_t)b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clrs32(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clrs32_v2si((int32x2_t)a);
#elif __riscv_xlen == 32
  return __builtin_riscv_clrs32_si(a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clo32(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clo32_v2si((int32x2_t)a);
#elif __riscv_xlen == 32
  return __builtin_riscv_clo32_si(a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clz32(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clz32_v2si((int32x2_t)a);
#elif __riscv_xlen == 32
  return __builtin_riscv_clz32_si(a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uclip32(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uclip32_v2si((int32x2_t)a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_uclip32_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pkbb16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_pkbb16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pkbb16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pkbt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_pkbt16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pkbt16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pktb16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_pktb16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pktb16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pktt16(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_pktt16_v4hi((uint16x4_t)a, (uint16x4_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pktt16_v2hi((uint16x2_t)a, (uint16x2_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umaqa(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_umaqa_v2si((uint32x2_t)t, (uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_umaqa_si(t, (uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long ukadd64(unsigned long long a, unsigned long long b) {
  return __builtin_riscv_ukadd64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long uksub64(unsigned long long a, unsigned long long b) {
  return __builtin_riscv_uksub64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long uradd64(unsigned long long a, unsigned long long b) {
  return __builtin_riscv_uradd64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long ursub64(unsigned long long a, unsigned long long b) {
  return __builtin_riscv_ursub64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long ukmar64(unsigned long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ukmar64_v2si(t, (uint32x2_t)a, (uint32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ukmar64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long ukmsr64(unsigned long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ukmsr64_v2si(t, (uint32x2_t)a, (uint32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ukmsr64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umar64(unsigned long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_umar64_v2si(t, (uint32x2_t)a, (uint32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_umar64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long umsr64(unsigned long long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_umsr64_v2si(t, (uint32x2_t)a, (uint32x2_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_umsr64_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long long mulr64(unsigned int a, unsigned int b) {
  return __builtin_riscv_mulr64(a, b);
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long cmpeq8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_cmpeq8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_cmpeq8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clrs8(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clrs8_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clrs8_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clo8(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clo8_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clo8_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long clz8(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_clz8_v8qi((int8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_clz8_v4qi((int8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uclip8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uclip8_v8qi((int8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uclip8_v4qi((int8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long scmplt8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_scmplt8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_scmplt8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long scmple8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_scmple8_v8qi((int8x8_t)a, (int8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_scmple8_v4qi((int8x4_t)a, (int8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long swap8(unsigned long a) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_swap8_v8qi((uint8x8_t)a);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_swap8_v4qi((uint8x4_t)a);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli8(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srli8_v8qi((uint8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srli8_v4qi((uint8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srli8_u(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srli8_u_v8qi((uint8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srli8_u_v4qi((uint8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl8(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srl8_v8qi((uint8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srl8_v4qi((uint8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long srl8_u(unsigned long a, int b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_srl8_u_v8qi((uint8x8_t)a, b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_srl8_u_v4qi((uint8x4_t)a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umin8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_umin8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_umin8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long umax8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_umax8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_umax8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uradd8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uradd8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uradd8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukadd8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ukadd8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ukadd8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ucmple8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ucmple8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ucmple8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ursub8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ursub8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ursub8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uksub8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_uksub8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_uksub8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ucmplt8(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return (unsigned long) __builtin_riscv_ucmplt8_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return (unsigned long) __builtin_riscv_ucmplt8_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pbsad(unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_pbsad_v8qi((uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pbsad_v4qi((uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long bitrevi(unsigned long a, const int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_bitrevi_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_bitrevi_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long bitrev(unsigned long a, int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_bitrev_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_bitrev_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long bpick(unsigned long a, unsigned long b, unsigned long c) {
#if __riscv_xlen == 64
  return __builtin_riscv_bpick_di(a, b, c);
#elif __riscv_xlen == 32
  return __builtin_riscv_bpick_si(a, b, c);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long insb(unsigned long t, char a, const int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_insb_di(t, a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_insb_si(t, a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long pbsada(unsigned long t, unsigned long a, unsigned long b) {
#if __riscv_xlen == 64
  return __builtin_riscv_pbsada_di(t, (uint8x8_t)a, (uint8x8_t)b);
#elif __riscv_xlen == 32
  return __builtin_riscv_pbsada_si(t, (uint8x4_t)a, (uint8x4_t)b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukaddh(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ukaddh_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ukaddh_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uksubh(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_uksubh_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_uksubh_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ukaddw(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ukaddw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ukaddw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uksubw(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_uksubw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_uksubw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long uraddw(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_uraddw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_uraddw_si(a, b);
#endif
}

__extension__ static __inline __attribute__ ((__always_inline__))
unsigned long ursubw(unsigned int a, unsigned int b) {
#if __riscv_xlen == 64
  return __builtin_riscv_ursubw_di(a, b);
#elif __riscv_xlen == 32
  return __builtin_riscv_ursubw_si(a, b);
#endif
}

#endif
