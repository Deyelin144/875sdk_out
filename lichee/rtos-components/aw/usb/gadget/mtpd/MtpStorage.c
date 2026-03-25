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

#include "MtpStorage.h"
#include "mtp.h"
#include "MtpCommon.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h>


uint64_t MtpStorageGetFreeSpace(struct MtpStorage *storage)
{
	struct statfs stat;
	uint64_t freeSpace;
	if (statfs(storage->mFilePath, &stat))
		return -1;
	freeSpace = (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
	return (freeSpace > storage->mReserveSpace ? freeSpace - storage->mReserveSpace : 0);
}

struct MtpStorage *MtpStorageInit(MtpStorageID id, const char *path,
					const char *description,
					uint64_t reserveSpace,
					uint64_t maxFileSize)
{
	struct MtpStorage *mStorage = NULL;

	mStorage = (struct MtpStorage *)calloc_wrapper(1, sizeof(struct MtpStorage));

	mStorage->mDisk = DiskInit(path);
	if (!mStorage->mDisk) {
		printf("mDisk init failed, path:%s\n", path);
		free_wrapper(mStorage);
		return NULL;
	}

	mStorage->mFilePath = strdup_wrapper(path);

	mStorage->mDescription = strdup_wrapper(description);

	mStorage->mMaxCapacity = mStorage->mDisk->dMaxCap;
	mStorage->mFreeSpace = mStorage->mDisk->dFreeSpace;

	mStorage->mReserveSpace = reserveSpace;
	mStorage->mMaxFileSize = maxFileSize;

	mStorage->mStorageID = id;

	return mStorage;
}

void MtpStorageRelease(struct MtpStorage *mStorage)
{
	if (!mStorage)
		return;
	if (mStorage->mDisk) {
		DiskRelease(mStorage->mDisk);
		mStorage->mDisk = NULL;
	}
	if (mStorage->mFilePath) {
		free_wrapper(mStorage->mFilePath);
		mStorage->mFilePath = NULL;
	}
	if (mStorage->mDescription) {
		free_wrapper(mStorage->mDescription);
		mStorage->mDescription = NULL;
	}
	free_wrapper(mStorage);

	return;
}
