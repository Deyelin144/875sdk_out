#ifndef _RISCV_IRQFLAGS_H
#define _RISCV_IRQFLAGS_H

#include "csr.h"

/* read interrupt enabled status */
static inline unsigned long arch_local_save_flags(void)
{
    return read_csr(CSR_MSTATUS);
}

/* test flags */
static inline int arch_irqs_disabled_flags(unsigned long flags)
{
    return !(flags & SR_MIE);
}

/* test hardware interrupt enable bit */
static inline int arch_irqs_disabled(void)
{
    return arch_irqs_disabled_flags(arch_local_save_flags());
}

/* unconditionally enable interrupts */
static inline void arch_local_irq_enable(void)
{
    set_csr(CSR_MSTATUS, SR_MIE);
}

/* unconditionally disable interrupts */
static inline void arch_local_irq_disable(void)
{
    clear_csr(CSR_MSTATUS, SR_MIE);
}

/* get status and disable interrupts */
static inline unsigned long arch_local_irq_save(void)
{
    return read_clear_csr(CSR_MSTATUS, SR_MIE);
}

/* set interrupt enabled status */
static inline void arch_local_irq_restore(unsigned long flags)
{
    set_csr(CSR_MSTATUS, flags & SR_MIE);
}

static inline void wait_for_interrupt(void)
{
    __asm__ __volatile__("wfi \n:::memory");
}

#endif /* _ASM_RISCV_IRQFLAGS_H */
