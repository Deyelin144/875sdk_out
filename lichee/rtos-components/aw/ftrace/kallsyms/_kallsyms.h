#ifndef ___KALLSYMS_H
#define ___KALLSYMS_H

#include "config.h"

#if DYNAMIC_FTRACE_KALLSYMS
#include <string.h>
#include "types.h"

extern const addr_t kallsyms_addresses[] __attribute__((weak));
extern const uint8_t kallsyms_names[] __attribute__((weak));
extern const uint16_t kallsyms_token_index[] __attribute__((weak));
extern const uint8_t kallsyms_token_table[] __attribute__((weak));
extern const uint32_t kallsyms_markers[] __attribute__((weak));
extern const uint8_t kallsyms_seqs_of_names[] __attribute__((weak));
extern uint32_t kallsyms_num_syms;

unsigned long kallsyms_lookup_name(const char *name);
int kallsyms_addr2name(void *addr, char *buf);
#endif /* DYNAMIC_FTRACE_KALLSYMS */
#endif /* ___KALLSYMS_H */

