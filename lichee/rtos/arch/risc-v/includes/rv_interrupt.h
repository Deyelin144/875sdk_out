#ifndef _RV_INTERRUPT_H
#define _RV_INTERRUPT_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <hal_interrupt.h>

struct irq_action
{
    hal_irq_handler_t handler;
    void *dev_id;
    struct irq_action *next;
    unsigned long irq_nums;
};

#define IRQ_INVALID    (1U << 31)

int arch_request_irq(int32_t irq, hal_irq_handler_t handler,
                     void *data);
void arch_free_irq(int32_t irq);

/* implemented in armvxxx/ riscv/xx */
void arch_enable_irq(int32_t irq);
void arch_disable_irq(int32_t irq);

void arch_enable_all_irq(void);
void arch_disable_all_irq(void);

unsigned long xport_interrupt_disable(void);
void xport_interrupt_enable(unsigned long flag);
unsigned long arch_irq_is_disable(void);

#endif
