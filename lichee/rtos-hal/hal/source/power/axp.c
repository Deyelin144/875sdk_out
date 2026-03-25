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

/*
 * power ops
 */
#include <sunxi_hal_power.h>
#include "type.h"

/* battery ops */
int hal_power_get_bat_cap(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_rest_cap))
		return rdev->bat_ops->get_rest_cap(rdev);
	pr_err("can't read bat_cap\n");
	return -1;
}

int hal_power_get_coulumb_counter(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_coulumb_counter))
		return rdev->bat_ops->get_coulumb_counter(rdev);
	pr_err("can't read coulumb_counter\n");
	return -1;
}

int hal_power_get_bat_present(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_bat_present))
		return rdev->bat_ops->get_bat_present(rdev);
	pr_err("can't read bat_present\n");
	return -1;
}

int hal_power_get_bat_online(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_bat_online))
		return rdev->bat_ops->get_bat_online(rdev);
	pr_err("can't read bat_online\n");
	return -1;
}

int hal_power_get_bat_status(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_bat_status))
		return rdev->bat_ops->get_bat_status(rdev);
	pr_err("can't read bat_status\n");
	return -1;
}

int hal_power_get_bat_health(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_bat_health))
		return rdev->bat_ops->get_bat_health(rdev);
	pr_err("can't read bat_health\n");
	return -1;
}

int hal_power_get_vbat(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_vbat))
		return rdev->bat_ops->get_vbat(rdev);
	pr_err("can't read vbat\n");
	return -1;
}

int hal_power_get_ibat(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_ibat))
		return rdev->bat_ops->get_ibat(rdev);
	pr_err("can't read ibat\n");
	return -1;
}

int hal_power_get_disibat(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_disibat))
		return rdev->bat_ops->get_disibat(rdev);
	pr_err("can't read disibat\n");
	return -1;
}

int hal_power_get_temp(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_temp))
		return rdev->bat_ops->get_temp(rdev);
	pr_err("can't read disibat\n");
	return -1;
}

int hal_power_get_temp_ambient(struct power_dev *rdev)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->get_temp_ambient))
		return rdev->bat_ops->get_temp_ambient(rdev);
	pr_err("can't read disibat\n");
	return -1;
}

int hal_power_set_chg_cur(struct power_dev *rdev, int cur)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->set_chg_cur))
		return rdev->bat_ops->set_chg_cur(rdev, cur);
	pr_err("can't set_chg_cur\n");
	return -1;
}

int hal_power_set_chg_vol(struct power_dev *rdev, int vol)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->set_chg_vol))
		return rdev->bat_ops->set_chg_vol(rdev, vol);
	pr_err("can't set_chg_vol\n");
	return -1;
}

int hal_power_set_batfet(struct power_dev *rdev, int onoff)
{
	if ((rdev) && (rdev->bat_ops) && (rdev->bat_ops->set_batfet))
		return rdev->bat_ops->set_batfet(rdev, onoff);
	pr_err("can't set_batfet\n");
	return -1;
}

/* usb ops */
int hal_power_get_usb_status(struct power_dev *rdev)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->get_usb_status))
		return rdev->usb_ops->get_usb_status(rdev);
	pr_err("can't read usb_status\n");
	return -1;
}

int hal_power_get_usb_ihold(struct power_dev *rdev)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->get_usb_ihold))
		return rdev->usb_ops->get_usb_ihold(rdev);
	pr_err("can't read usb_ihold\n");
	return -1;
}

int hal_power_get_usb_vhold(struct power_dev *rdev)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->get_usb_vhold))
		return rdev->usb_ops->get_usb_vhold(rdev);
	pr_err("can't read usb_vhold\n");
	return -1;
}

int hal_power_set_usb_ihold(struct power_dev *rdev, int cur)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->set_usb_ihold))
		return rdev->usb_ops->set_usb_ihold(rdev, cur);
	pr_err("can't set_usb_ihold\n");
	return -1;
}

int hal_power_set_usb_vhold(struct power_dev *rdev, int vol)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->set_usb_vhold))
		return rdev->usb_ops->set_usb_vhold(rdev, vol);
	pr_err("can't set_usb_vhold\n");
	return -1;
}

int hal_power_get_cc_status(struct power_dev *rdev)
{
	if ((rdev) && (rdev->usb_ops) && (rdev->usb_ops->get_cc_status))
		return rdev->usb_ops->get_cc_status(rdev);
	pr_err("can't get_cc_status\n");
	return -1;
}

