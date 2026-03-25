/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
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

#include <sunxi_hal_power.h>
#include <sunxi_hal_power_private.h>
#include "../gpadc_power.h"
#include <stdlib.h>

static int power_init = -1;
static struct power_dev *rdev;

int hal_power_init(void)
{
	int pmu_id;

	rdev = malloc(sizeof(struct power_dev));
	if (!rdev) {
		printf("%s:malloc rdev failed\n", __func__);
		return -1;
	}

	rdev->config = malloc(sizeof(struct power_supply));
	if (!rdev->config) {
		printf("%s:malloc rdev.config failed\n", __func__);
		return -1;
	}
	pmu_id = gpadc_get_power(rdev);
	if (pmu_id == GPADC_POWER) {
		gpadc_init_power(rdev);
		power_init = 1;
	} else {
		pr_err("hal power init gpadc failed\n");
		power_init = 0;
	}
	return 0;
}

void hal_power_deinit(void)
{
	free(rdev);
}


int hal_power_set_chg_plg_cb(void *user_callback)
{
    gpadc_set_vbus_chg_plg_cb((gpadc_callback_t)user_callback);
    return 0;
}


int hal_power_get(struct power_dev *rdev)
{
	if (power_init != 1) {
		pr_err("hal power init failed\n");
		return 0;
	}
	gpadc_get_power(rdev);
	rdev->config = malloc(sizeof(struct power_supply));
	if (!rdev->config) {
		printf("%s:malloc failed\n", __func__);
		return -1;
	}
	return 0;
}

#define VBAT_GET_CNT    1
int hal_power_low_power_detect(unsigned int low_power_value)
{
    struct power_dev rdev;
    int usb_status = 0;
    unsigned int bat_value = 0;
    uint8_t cnt = 0;
    
    memset(&rdev, 0, sizeof(struct power_dev));
    if (0 != hal_power_get(&rdev)) {
        printf("power get rdev fail.\n");
        return 2;
    }
    usb_status = rdev.usb_ops->get_usb_status(&rdev);
    if (POWER_SUPPLY_STATUS_CHARGING == usb_status || POWER_SUPPLY_STATUS_FULL == usb_status) {
        printf("[%s]usb charge, do not detect.\n",__FUNCTION__, bat_value, low_power_value);
        return 0;
    } else {
        for (cnt = 0; cnt < VBAT_GET_CNT; cnt++) {
            bat_value = rdev.bat_ops->get_vbat(&rdev);
        }
         if (bat_value <= low_power_value) {
            printf("[%s]low power, shut down.(%d %d).\n", __FUNCTION__, bat_value, low_power_value);
            return 1;
         } else {
            printf("[%s]not low power(%d %d).\n", __FUNCTION__, bat_value, low_power_value);
         }
    }

    return 0;
}

int hal_power_put(struct power_dev *rdev)
{
	free(rdev->config);

	return 0;
}

