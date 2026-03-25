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

#include <string.h>
#include "flash_chip.h"
#include "../flash_debug.h"

int DefaultFlashInit(struct FlashChip *chip)
{
	PCHECK(chip);
	chip->setBurstWrap = defaultSetBurstWrap;
	chip->continuousReadReset = defaultContinuousReadReset;
	chip->writeEnable = defaultWriteEnable;
	chip->writeDisable = defaultWriteDisable;
	chip->readStatus = defaultReadStatus;
	chip->erase = defaultErase;
	chip->jedecID = defaultGetJedecID;
	chip->pageProgram = defaultPageProgram;
	chip->read = defaultRead;

	chip->driverWrite = defaultDriverWrite;
	chip->driverRead = defaultDriverRead;
	chip->setFreq = defaultSetFreq;
	chip->switchReadMode = defaultSwitchReadMode;
	chip->xipDriverCfg = defaultXipDriverCfg;
	chip->enableXIP = defaultEnableXIP;
	chip->disableXIP = defaultDisableXIP;
	chip->isBusy = defaultIsBusy;
	chip->control = defaultControl;
	chip->minEraseSize = defaultGetMinEraseSize;
	chip->writeStatus = defaultWriteStatus;
	chip->enableQPIMode = defaultEnableQPIMode;
	chip->disableQPIMode = defaultDisableQPIMode;
	//chip->enableReset = defaultEnableReset;
	chip->reset = defaultReset;

	chip->suspendErasePageprogram = defaultSuspendErasePageprogram;
	chip->resumeErasePageprogram = defaultResumeErasePageprogram;
	chip->isSuspend = defaultIsSuspend;
	chip->powerDown = NULL;
	chip->releasePowerDown = defaultReleasePowerDown;
	chip->uniqueID = NULL;
#ifdef CONFIG_FLASH_POWER_DOWN_PROTECT
	chip->initLock = defaultInitLock;
	chip->setGlobalBlockLockState = defaultSetGlobalBlockLockState;
	chip->lockBlock = defaultLockBlock;
	chip->unLockBlock = defaultUnLockBlock;
	chip->readBlockLockStatus = defaultReadBlockLockStatus;
#endif
	/*TODO: a NULL interface for showing invalid interface*/

	FLASH_DEBUG("default chip inited");

	return 0;
}

static int DefaultFlashDeinit(struct FlashChip *chip)
{
	PCHECK(chip);

//	HAL_Free(chip);

	return 0;
}

static struct FlashChip *DefaultFlashCtor(struct FlashChip *chip, uint32_t arg)
{
	int32_t i;
	const FlashChipCfg *list;

	PCHECK(chip);

	list = FlashChipGetCfgList(&i);

	while (--i > 0) {
		FLASH_DEBUG("default ctor: 0x%x = 0x%x ?\n", list[i].mJedec, arg);
		if (list[i].mJedec == arg)
			break;
	}

	const FlashChipCfg *cfg = &list[i];

	if (i == 0) {
		FLASH_ALERT("use default flash chip mJedec 0x%x", cfg->mJedec);
	}

	memcpy(&chip->cfg, cfg, sizeof(FlashChipCfg));
	chip->mPageSize = 256;
	chip->mFlashStatus = 0;
	chip->mDummyCount = 1;

	return chip;
}

FlashChipCtor DefaultFlashChip = {
		.mJedecId = 0,
		.enumerate = DefaultFlashCtor,
		.init = DefaultFlashInit,
		.destory = DefaultFlashDeinit,
};
