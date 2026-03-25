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

#include <ffs.h>
#include <log.h>
#include <stddef.h>
#include <sunxi_hal_regulator.h>
#include <sunxi_hal_regulator_private.h>

static int voltage2val(struct regulator_desc *info, int voltage, u8 *reg_val)
{
	const struct regulator_linear_range *range;
	int i;

	if (!info->linear_ranges) {
		*reg_val = (voltage - info->min_uv + info->step1_uv - 1)
			/ info->step1_uv;
		return 0;
	}

	for (i = 0; i < info->n_linear_ranges; i++) {
		int linear_max_uV;

		range = &info->linear_ranges[i];
		linear_max_uV = range->min_uV +
			(range->max_sel - range->min_sel) * range->uV_step;

		if (!(voltage <= linear_max_uV && voltage >= range->min_uV))
			continue;

		/* range->uV_step == 0 means fixed voltage range */
		if (range->uV_step == 0) {
			*reg_val = 0;
		} else {
			*reg_val = (voltage - range->min_uV) / range->uV_step;
		}

		*reg_val += range->min_sel;
		return 0;
	}

	return -1;
}

static int val2voltage(struct regulator_desc *info, u8 reg_val, int *voltage)
{
	const struct regulator_linear_range *range = info->linear_ranges;
	int i;

	if (!info->linear_ranges) {
		*voltage = info->min_uv + info->step1_uv * reg_val;
		return 0;
	}

	for (i = 0; i < info->n_linear_ranges; i++, range++) {
		if (!(reg_val <= range->max_sel && reg_val >= range->min_sel))
			continue;

		*voltage = (reg_val - range->min_sel) * range->uV_step +
			   range->min_uV;
		return 0;
	}

	return -1;
}

static int axp_regulator_set_voltage(struct regulator_dev *rdev, int target_uV)
{
	unsigned char id = REGULATOR_ID(rdev->flag);
	struct regulator_desc *pd = (struct regulator_desc *)rdev->private;
	struct regulator_desc *info = &pd[id];
	u8 val;

	if (voltage2val(info, target_uV, &val))
		return -1;

	val <<= ffs(info->vol_mask) - 1;
	return hal_axp_byte_update(rdev, info->vol_reg, val, info->vol_mask);
}

static int axp_regulator_get_voltage(struct regulator_dev *rdev, int *vol_uV)
{
	unsigned char id = REGULATOR_ID(rdev->flag);
	struct regulator_desc *pd = (struct regulator_desc *)rdev->private;
	struct regulator_desc *info = &pd[id];
	u8 val;
	int ret;

	ret = hal_axp_byte_read(rdev, info->vol_reg, &val);
	if (ret)
		return ret;

	val &= info->vol_mask;
	val >>= ffs(info->vol_mask) - 1;
	if (val2voltage(info, val, vol_uV))
		return -1;

	return 0;
}

static int axp_regulator_enable(struct regulator_dev *rdev)
{
	unsigned char id = REGULATOR_ID(rdev->flag);
	struct regulator_desc *pd = (struct regulator_desc *)rdev->private;
	struct regulator_desc *info = &pd[id];
	u8 reg_val = 0;

	reg_val = info->enable_val;
	if (!reg_val)
		reg_val = info->enable_mask;

	return hal_axp_byte_update(rdev, info->enable_reg,
			       reg_val, info->enable_mask);
}

static int axp_regulator_disable(struct regulator_dev *rdev)
{
	unsigned char id = REGULATOR_ID(rdev->flag);
	struct regulator_desc *pd = (struct regulator_desc *)rdev->private;
	struct regulator_desc *info = &pd[id];
	u8 reg_val = 0;

	reg_val = info->disable_val;
	if (!reg_val)
		reg_val = ~info->enable_mask;

	return hal_axp_byte_update(rdev, info->enable_reg,
			       reg_val, info->enable_mask);
}

struct regulator_ops axp_regulator_ops = {
	.set_voltage	= axp_regulator_set_voltage,
	.get_voltage	= axp_regulator_get_voltage,
	.enable		= axp_regulator_enable,
	.disable	= axp_regulator_disable,
};
