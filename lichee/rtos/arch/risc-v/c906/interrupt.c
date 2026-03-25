/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the People's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECqHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
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

#include <stdlib.h>
#include "rv_interrupt.h"
#include "irqdesc.h"
#include "irq_internal.h"
#include "irqflags.h"
#include "excep.h"
#include "irqs.h"
#include <csr.h>

#include <hal_debug.h>

void plic_handle_irq(irq_regs_t *);

/* test hardware interrupt enable bit */
unsigned long arch_irq_is_disable(void)
{
    return arch_irqs_disabled_flags(arch_local_save_flags());
}
/*
 * Generic no controller implementation
 */
extern struct irq_controller plic_controller;

struct irq_desc irq_desc[NR_IRQS]  =
{
    [0 ... NR_IRQS - 1] = {
        .action         = NULL,
    }
};

void handle_level_irq(uint32_t irq, struct irq_desc *desc)
{
    struct irq_controller *controller = &plic_controller;
    struct irq_action *action;

    traverse_action_of_desc(desc, action)
    {
        hal_irqreturn_t res;
        res = action->handler(action->dev_id);
        action->irq_nums ++;

        switch (res)
        {
            case HAL_IRQ_OK:
            /* Fall through - to add to randomness */
            case HAL_IRQ_ERR:
                break;
            default:
                break;
        }
    }

    controller->irq_eoi(irq);
}

struct irq_desc *irq_to_desc(uint32_t irq)
{
    return (irq < NR_IRQS) ? irq_desc + irq : NULL;
}

static int initialize_irq(uint32_t irq, struct irq_desc *desc, struct irq_action *new)
{
    struct irq_action *old, **old_ptr;
    unsigned long flags;
    int ret, nested;

    if (!desc)
    {
        printf("no desc for this irq %d.", irq);
        return -1;
    }

    flags = arch_local_irq_save();
    old_ptr = &desc->action;
    old = *old_ptr;
    if (old)
    {
        /* add new interrupt at end of irq queue */
        do
        {
            old_ptr = &old->next;
            old = *old_ptr;
        } while (old);
    }

    *old_ptr = new;

    arch_local_irq_restore(flags);

    return 0;
}

int irq_request(uint32_t irq, hal_irq_handler_t handler,
                void *data)
{
    struct irq_action *action;
    struct irq_desc *desc;
    int retval;

    if (irq == IRQ_INVALID)
    {
        printf("irq invalided!");
        return -1;
    }

    desc = irq_to_desc(irq);
    if (!desc)
    {
        printf("no desc for this irq %d.!", irq);
        return -1;
    }

    if (!handler)
    {
        printf("handler parameter is null");
        return -1;
    }

    action = calloc(sizeof(struct irq_action), 1);
    if (!action)
    {
        printf("no free buffer.");
        return -1;
    }

    action->handler = handler;
    action->dev_id = data;
    action->irq_nums = 0;

    retval = initialize_irq(irq, desc, action);
    if (retval)
    {
        free(action);
        action = NULL;
    }
    else
    {
        retval = irq;
    }

    return retval;
}

int generic_handle_irq(uint32_t irq)
{
    struct irq_desc *desc = irq_to_desc(irq);
    if (!desc)
    {
        printf("fatal error, desc is null for irq %d.", irq);
        hal_sys_abort();
        return -1;
    }
    generic_handle_irq_desc(irq, desc);
    return 0;
}

static struct irq_action *uninitialize_irq(uint32_t irq, struct irq_desc *desc, void *dev_id)
{
    struct irq_action *action, **action_ptr;
    unsigned long flags;

    flags = arch_local_irq_save();

    action_ptr = &desc->action;
    for (; ;)
    {
        action = *action_ptr;

        if (!action)
        {
            printf("Trying to free already-free IRQ %d", irq);
            arch_local_irq_restore(flags);
            return NULL;
        }
        break;
    }

    /* Found it - now remove it from the list of entries: */
    *action_ptr = action->next;

    /* If this was the last handler, shut down the IRQ line: */
    if (!desc->action)
    {
        /* Only shutdown. Deactivate after synchronize_hardirq() */
        void hal_disable_irq(int32_t irq);
        hal_disable_irq(irq);
    }

    arch_local_irq_restore(flags);

    return action;
}

int irq_free(uint32_t irq, void *dev_id)
{
    struct irq_desc *desc = irq_to_desc(irq);
    struct irq_action *action;

    if (!desc)
    {
        printf("no desc for irq %d.", irq);
        return -1;
    }

    action = uninitialize_irq(irq, desc, dev_id);

    if (!action)
    {
        printf("find no desc for irq %d.", irq);
        return -1;
    }

    free(action);
    return 0;
}

void remove_irq(uint32_t irq, struct irq_action *act)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (desc)
    {
        uninitialize_irq(irq, desc, act->dev_id);
    }
}

int setup_irq(uint32_t irq, struct irq_action *act)
{
    int retval;
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc || !act)
    {
        printf("cant find irq descriptor.");
        return -1;
    }

    retval = initialize_irq(irq, desc, act);

    return retval;
}

void show_irqs(void)
{
    int i;
    struct irq_desc *desc = NULL;
    struct irq_action *action = NULL;;

    printf(" irqno    handler        active.\n");
    printf(" -------------------------------\n");
    for (i = 0; i < NR_IRQS; i ++)
    {
        int line = 0;
        desc = irq_to_desc(i);
        if (!desc || desc->action == NULL)
        {
            continue;
        }

        printf("%6d", i);
        traverse_action_of_desc(desc, action)
        {
            printf("      0x%lx     0x%08lx\n", (unsigned long)action->handler, action->irq_nums);
        }
    }
    printf(" -------------------------------\n");
}

void handle_arch_irq(irq_regs_t *regs)
{
    unsigned long mip = read_csr(CSR_MIP);

    if (!(mip & (MIE_MTIE | MIE_MEIE)))
    {
        printf("sip status error.");
        hal_sys_abort();
    }

    plic_handle_irq(regs);

    return;
}

int arch_request_irq(int32_t irq, hal_irq_handler_t handler,
                     void *data)
{
    return irq_request(irq, handler, data);
}

void arch_free_irq(int32_t irq)
{
    irq_free(irq, NULL);
}