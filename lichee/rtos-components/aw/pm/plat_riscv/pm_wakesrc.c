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

#include <osal/hal_interrupt.h>
#include <arch/riscv/sun20iw2p1/irqs-sun20iw2p1.h>

#include <hal_gpio.h>

#include "io.h"
#include <errno.h>

#include "pm_debug.h"
#include "pm_adapt.h"
#include "pm_rv_wakesrc.h"
#include "pm_wakesrc.h"
#include "pm_wakeres.h"
#include "pm_devops.h"
#include "pm_notify.h"
#include "pm_state.h"
#include "pm_wakecnt.h"

#include "hal_prcm.h"

#undef  PM_DEBUG_MODULE
#define PM_DEBUG_MODULE  PM_DEBUG_WAKESRC

#define ws_containerof(ptr_module) \
        __containerof(ptr_module, struct pm_wakesrc_settled, list)

#define INVALID_IRQn -20

static volatile int pm_wakeup_irq  = INVALID_IRQn;

static struct pm_wakesrc_settled wakesrc_array[PM_WAKEUP_SRC_MAX] = {
	{PM_WAKEUP_SRC_WKIO0,    AR200A_WUP_IRQn,      "SRC_WKIO0"    , PM_RES_STANDBY_NULL,    }, //0  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO1,    AR200A_WUP_IRQn,      "SRC_WKIO1"    , PM_RES_STANDBY_NULL,    }, //1  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO2,    AR200A_WUP_IRQn,      "SRC_WKIO2"    , PM_RES_STANDBY_NULL,    }, //2  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO3,    AR200A_WUP_IRQn,      "SRC_WKIO3"    , PM_RES_STANDBY_NULL,    }, //3  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO4,    AR200A_WUP_IRQn,      "SRC_WKIO4"    , PM_RES_STANDBY_NULL,    }, //4  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO5,    AR200A_WUP_IRQn,      "SRC_WKIO5"    , PM_RES_STANDBY_NULL,    }, //5  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO6,    AR200A_WUP_IRQn,      "SRC_WKIO6"    , PM_RES_STANDBY_NULL,    }, //6  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO7,    AR200A_WUP_IRQn,      "SRC_WKIO7"    , PM_RES_STANDBY_NULL,    }, //7  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO8,    AR200A_WUP_IRQn,      "SRC_WKIO8"    , PM_RES_STANDBY_NULL,    }, //8  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKIO9,    AR200A_WUP_IRQn,      "SRC_WKIO9"    , PM_RES_STANDBY_NULL,    }, //9  AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_WKTMR,    AR200A_WUP_IRQn,      "SRC_WKTMR"    , PM_RES_STANDBY_NULL,    }, //10 AR200A_WUP_IRQn,
	{PM_WAKEUP_SRC_ALARM0,   ALARM0_IRQn,          "SRC_ALARM0"   , PM_RES_STANDBY_NULL,    }, //11 ALARM0_IRQn,
	{PM_WAKEUP_SRC_ALARM1,   ALARM1_IRQn,          "SRC_ALARM1"   , PM_RES_STANDBY_NULL,    }, //12 ALARM1_IRQn,
	{PM_WAKEUP_SRC_WKTIMER0, WKUP_TIMER_IRQ0_IRQn, "SRC_WKTIMER0,", PM_RES_STANDBY_NULL,    }, //13 WKUP_TIMER_IRQ0_IRQn,
	{PM_WAKEUP_SRC_WKTIMER1, WKUP_TIMER_IRQ1_IRQn, "SRC_WKTIMER1,", PM_RES_STANDBY_NULL,    }, //14 WKUP_TIMER_IRQ1_IRQn,
	{PM_WAKEUP_SRC_WKTIMER2, WKUP_TIMER_IRQ2_IRQn, "SRC_WKTIMER2,", PM_RES_STANDBY_NULL,    }, //15 WKUP_TIMER_IRQ2_IRQn,
	{PM_WAKEUP_SRC_LPUART0,  LPUART0_IRQn,         "SRC_LPUART0"  , PM_RES_STANDBY_NULL,    }, //16 LPUART0_IRQn,
	{PM_WAKEUP_SRC_LPUART1,  LPUART1_IRQn,         "SRC_LPUART1"  , PM_RES_STANDBY_NULL,    }, //17 LPUART1_IRQn,
	{PM_WAKEUP_SRC_GPADC,    GPADC_IRQn,           "SRC_GPADC,"   , PM_RES_STANDBY_NULL,    }, //18 GPADC_IRQn,
	{PM_WAKEUP_SRC_MAD,      MAD_WAKE_IRQn,        "SRC_MAD,"     , PM_RES_STANDBY_NULL,    }, //19 MAD_WAKE_IRQn,
	{PM_WAKEUP_SRC_WLAN,     WLAN_IRQn,            "SRC_WLAN"     , PM_RES_STANDBY_NULL,    }, //20 WLAN_IRQn,
	{PM_WAKEUP_SRC_BT,       BTCOEX_IRQn,          "SRC_BT"       , PM_RES_STANDBY_NULL,    }, //21 BTCOEX_IRQn,
	{PM_WAKEUP_SRC_BLE,      BLE_LL_IRQn,          "SRC_BLE"      , PM_RES_STANDBY_NULL,    }, //22 BLE_LL_IRQn,
	{PM_WAKEUP_SRC_GPIOA,    GPIOA_IRQn,           "SRC_GPIOA"    , PM_RES_STANDBY_NULL,    }, //23 GPIOA_IRQn, NO USE
	{PM_WAKEUP_SRC_GPIOB,    GPIOB_IRQn,           "SRC_GPIOB"    , PM_RES_STANDBY_NULL,    }, //24 GPIOB_IRQn,
	{PM_WAKEUP_SRC_GPIOC,    GPIOC_IRQn,           "SRC_GPIOC"    , PM_RES_STANDBY_NULL,    }, //25 GPIOC_IRQn,
	{PM_WAKEUP_SRC_DEVICE,   INVALID_IRQn,         "SRC_DEVICE"   , PM_RES_STANDBY_NULL,    }, //26 all active device or IRQn,
};

static hal_spinlock_t pm_wakeup_rec_lock = {
	.owner = 0,
	.counter = 0,
	.spin_lock = {
		.slock = 0,
	},
};

int cmd_rpcconsole_ext(int argc, char **argv);

static int rv_get_arm_wakeup_irq(void)
{
	char *argv[3] = {
		"rpccli",
		"arm",
		"get_wakeup_irq",
    };

	return cmd_rpcconsole_ext(3, argv);
}

static int rpccli_arm_pm_get_wupsrc(void)
{
	char *argv[3] = {
		"rpccli",
		"arm",
		"pm_get_wupsrc",
    };

	return cmd_rpcconsole_ext(3, argv);
}

static int pm_wakesrc_get_wakeup_irq(void)
{
	if (rv_get_arm_wakeup_irq() >= 0) {
		return rv_get_arm_wakeup_irq() + 16;
	} else if (rv_get_arm_wakeup_irq() == -1) {
		return -1;
	} else {
		return pm_wakeup_irq;
	}
}

static uint8_t pm_irq_is_wakeup_riscv(const int irq)
{
	uint32_t val = 0;
	int id;

	for (id = PM_WAKEUP_SRC_BASE; id < PM_WAKEUP_SRC_MAX; id++) {
		if (wakesrc_array[id].irq == irq)
			val = 1;
	}

	return val;
}

static void pm_wakesrc_rec_wakeup_irq(const int irq)
{
	int id_sum = 0;;
	struct pm_wakesrc *ws = NULL;

	/* Do not lead waste of resources for lock in irq handler */
	if (pm_wakeup_irq == INVALID_IRQn) {
		ws = pm_wakesrc_find_registered_by_irq(irq);
		if (ws && (pm_wakeup_irq == INVALID_IRQn)) {
			pm_wakeup_irq = irq;
		}
	}
}

static void pm_wakesrc_clear_wakeup_irq(void)
{
	hal_spin_lock(&pm_wakeup_rec_lock);
	pm_wakeup_irq = INVALID_IRQn;
	hal_spin_unlock(&pm_wakeup_rec_lock);
}

struct pm_wakeup_ops pm_wakeup_ops_riscv = {
	.get_wakeup_irq		= pm_wakesrc_get_wakeup_irq,
	.irq_is_wakeup_armed	= pm_irq_is_wakeup_riscv,
	.record_wakeup_irq	= pm_wakesrc_rec_wakeup_irq,
	.clear_wakeup_irq	= pm_wakesrc_clear_wakeup_irq,
};

struct pm_wakesrc_settled *pm_wakesrc_get_by_id(wakesrc_id_t id)
{
	if (!wakesrc_id_valid(id))
		return NULL;

	return &wakesrc_array[id];
}

static int pm_wakeup_ops_riscv_init(void)
{
	int ret;

	ret = pm_wakeup_ops_register(&pm_wakeup_ops_riscv);
	if (ret) {
		pm_err("riscv register pm_wakeup_ops failed\n");
		return -EFAULT;
	}

	return 0;
}

int pm_riscv_wakesrc_init(void)
{
	pm_wakeup_ops_riscv_init();
	return 0;
}

int ococci_rv_pm_wakeup_id(void)
{
	int id;
	int irq;
	uint8_t wakeup_src = 0;
	irq = pm_wakesrc_get_wakeup_irq();
	printf("ococci_rv_pm_wakeup_id irq = %d\n", irq);
	if (irq == ALARM0_IRQn) {
		printf("ococci_rv_pm_wakeup_id alarm0\n");
		id = PM_WAKEUP_SRC_ALARM0;
	} else if (irq == AR200A_WUP_IRQn) {
		printf("ococci_rv_pm_wakeup_id wakeupio or timer\n");
		id = rpccli_arm_pm_get_wupsrc();
	} else if (irq == WLAN_IRQn) {
		printf("ococci_rv_pm_wakeup_id wlan\n");
		id = PM_WAKEUP_SRC_WLAN;
	} else {
		printf("ococci_rv_pm_wakeup_id unknown irq\n");
		id == PM_WAKEUP_SRC_MAX;
	}

	return id;
}
