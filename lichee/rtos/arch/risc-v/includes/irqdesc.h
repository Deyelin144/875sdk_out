#ifndef __IRQDESC_H
#define __IRQDESC_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "irq_internal.h"

struct irq_desc
{
    struct irq_action *action;    /* IRQ action list */
};

void handle_level_irq(uint32_t irq, struct irq_desc *desc);

static inline void generic_handle_irq_desc(uint32_t irq, struct irq_desc *desc)
{
    handle_level_irq(irq, desc);
}

int generic_handle_irq(uint32_t irq);

/* Test to see if a driver has successfully requested an irq */
static inline int irq_desc_has_action(struct irq_desc *desc)
{
    return desc->action != NULL;
}

#define traverse_action_of_desc(desc, action)                      \
    for (action = desc->action; action; action = action->next)

#endif
