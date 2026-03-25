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

#include <include/pm_base.h>
#include <include/pm_devops.h>
#include <include/pm_task.h>
#include <include/pm_wakecnt.h>
#include <include/pm_wakelock.h>
#include <include/pm_wakesrc.h>
#include <include/pm_state.h>
#include <include/pm_subsys.h>
#include <osal/hal_timer.h>

/* devops  */
int pm_devops_register(struct pm_device *dev)
{
	return 0;
}
int pm_devops_unregister(struct pm_device *dev)
{
	return 0;
}

/* pm task */
int pm_task_register(TaskHandle_t xHandle, pm_task_type_t type)
{
	return 0;
}

int pm_task_unregister(TaskHandle_t xHandle)
{
	return 0;
}

int pm_task_attach_timer(osal_timer_t timer, TaskHandle_t xHandle, uint32_t attach)
{
	return 0;
}

/* wakecnt */
void pm_wakecnt_inc(struct pm_wakesrc *ws) {}

void pm_stay_awake(struct pm_wakesrc *ws) {}

void pm_relax(struct pm_wakesrc *ws, pm_relax_type_t wakeup) {}

/* wakelock */
void pm_wakelocks_setname(struct wakelock *wl, const char *name) {}

int  pm_wakelocks_acquire(struct wakelock *wl, enum pm_wakelock_t type, uint32_t timeout)
{
	return 0;
}

int  pm_wakelocks_release(struct wakelock *wl)
{
	return 0;
}
void pm_wakelocks_showall(void) {}

/* wakesrc */
struct pm_wakesrc *pm_wakesrc_register(const int irq, const char *name, const unsigned int type)
{
	return NULL;
}

int pm_wakesrc_unregister(struct pm_wakesrc *ws)
{
	return 0;
}

int pm_set_wakeirq(struct pm_wakesrc *ws)
{
	return 0;
}
int pm_clear_wakeirq(struct pm_wakesrc *ws)
{
	return 0;
}

int pm_wakesrc_is_disabled(struct pm_wakesrc *ws)
{
	return 0;
}

int pm_wakesrc_is_inexistent(char *name)
{
	return 1;
}

uint32_t pm_subsys_check_in_status(pm_subsys_status_t status)
{
	return 0;
}

int pm_moment_has_been_recorded(pm_moment_t moment)
{
	return 0;
}

uint64_t pm_moment_interval_us(pm_moment_t start, pm_moment_t end)
{
	return 0;
}

void pm_moment_clear(uint32_t clear_mask)
{

}

int pm_notify_register(void *nt)
{
	return 0;
}

int pm_notify_unregister(int id)
{
	return 0;
}

int pm_suspend_assert(void)
{
	return 0;
}
