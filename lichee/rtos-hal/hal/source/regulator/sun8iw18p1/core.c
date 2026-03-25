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

#include <sunxi_hal_regulator.h>
#include <sunxi_hal_regulator_private.h>
int hal_regulator_get(unsigned int request_flag, struct regulator_dev *rdev)
{
	return 0;
}

#if 0
static int hal_regulator_enable(struct regulator_dev *rdev)
{
	struct pwm_regulator_info *info = rdev->pwm;
	int rc;

	rc = hal_pwm_init();
	rc |= hal_pwm_config(info->chanel, info->period_ns, info->period_ns);
	rc |= hal_pwm_set_polarity(info->chanel, info->polarity);
	if (rc)
		goto out;

	rc = hal_pwm_enable(info->chanel);
	if (!rc)
		return HAL_REGULATOR_STATUS_OK;

out:
	return HAL_REGULATOR_STATUS_ERROR;
}

static hal_regulator_set_voltage(struct regulator_dev *rdev, int *target_uV)
{
	struct pwm_regulator_info *pwm_info = rdev->pwm;
	unsigned int duty_ns = 0;
	int rc;

	if (*target_uV <= pwm_info->vol_base)
		duty_ns = 0;
	else if (*target_uV >= pwm_info->vol_max)
		duty_ns = pwm_info->period_ns;
	else if (pwm_info->vol_base < *target_uV &&
			*target_uV < pwm_info->vol_max) {
	/* Div 1000 for convert to mV */
		duty_ns = ((*target_uV - pwm_info->vol_base) / 1000)
			* pwm_info->period_ns
			/ ((pwm_info->vol_max - pwm_info->vol_base) / 1000);
	}

	printf("duty_ns:%d\tperiod_ns:%d\n", duty_ns, pwm_info->period_ns);
	rc = hal_pwm_config(pwm_info->chanel, duty_ns, pwm_info->period_ns);
	if (rc)
		return HAL_REGULATOR_STATUS_ERROR;

	return HAL_REGULATOR_STATUS_OK;
}

static struct regulator_ops pwm_regulator_voltage_continuous_ops = {
	.set_voltage	= pwm_regulator_set_voltage,
	.enable		= pwm_regulator_enable,
};

hal_regulator_status_t hal_regulator_init(void *data, unsigned int request_flag,
					  struct regulator_dev *rdev)
{
	int rc;

	rdev->flag = request_flag;

	switch (REGULATOR_TYPE(rdev->flag)) {
	case PWM_REGULATOR:
		rdev->pwm = (struct pwm_regulator_info *)data;
		rdev->ops = &pwm_regulator_voltage_continuous_ops;
		break;
	default:
		goto out;
	}

	rc = rdev->ops->enable(rdev);
	if (!rc)
		return HAL_REGULATOR_STATUS_OK;

out:
	return HAL_REGULATOR_STATUS_ERROR;
}

hal_regulator_status_t hal_regulator_set_voltage(struct regulator_dev *rdev,
					       int *target_uV)
{
	int rc;

	rc = rdev->ops->set_voltage(rdev, *target_uV);
	if (!rc)
		return HAL_REGULATOR_STATUS_OK;

	return HAL_REGULATOR_STATUS_ERROR;
}
#endif
