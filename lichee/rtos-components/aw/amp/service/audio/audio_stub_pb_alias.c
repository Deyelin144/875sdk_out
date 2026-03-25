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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <reent.h>

#include "sunxi_amp.h"
#include <hal_cache.h>
#include <hal_mutex.h>
#include <console.h>

#include "include/audio_amp.h"

struct AudioTrackRM {
	void *at;
	hal_mutex_t mutex;
	uint8_t state;
};

tAudioTrackRM *AudioTrackCreateRM(const char *name)
{
	void *at;
	tAudioTrackRM *atrm;
	void *args[2] = {0};
	int len;
	char *align_name;

	len = strlen(name) + 1;
	align_name = amp_align_malloc(len);
	if (!align_name)
		return NULL;

	memset(align_name, 0, len);
	memcpy(align_name, name, len);
	args[0] = (void *)align_name;
	args[1] = (void *)(uintptr_t)len;

	hal_dcache_clean((unsigned long)align_name, (unsigned long)len);
	at = (void *)func_stub(RPCCALL_AUDIO(AudioTrackCreateRM), 1, ARRAY_SIZE(args), args);
	amp_align_free(align_name);

	if (!at)
		return NULL;
	atrm = malloc(sizeof(tAudioTrackRM));
	if (!atrm)
		goto err;
	atrm->at = at;

	atrm->mutex = hal_mutex_create();

	atrm->state = AT_RM_STATE_SETUP;

	return atrm;
err:
	if (atrm)
		free(atrm);
	return NULL;
}

int AudioTrackDestroyRM(tAudioTrackRM *atrm)
{
	int ret;
	void *args[1] = {0};

	printf("[%s] line:%d \n", __func__, __LINE__);
	hal_mutex_lock(atrm->mutex);
	args[0] = (void *)atrm->at;
	ret = (int)func_stub(RPCCALL_AUDIO(AudioTrackDestroyRM), 1, ARRAY_SIZE(args), args);
	printf("[%s] line:%d ret=%d\n", __func__, __LINE__, ret);

	hal_mutex_unlock(atrm->mutex);
	hal_mutex_delete(atrm->mutex);

	free(atrm);

	printf("[%s] line:%d \n", __func__, __LINE__);
	return ret;
}

static int AudioTrackControlRM(tAudioTrackRM *atrm, uint32_t control)
{
	int ret;
	void *args[2] = {0};

	args[0] = (void *)atrm->at;
	args[1] = (void *)(uintptr_t)control;

	hal_mutex_lock(atrm->mutex);
	ret = (int)func_stub(RPCCALL_AUDIO(AudioTrackControlRM), 1, ARRAY_SIZE(args), args);
	hal_mutex_unlock(atrm->mutex);

	return ret;
}

int AudioTrackStartRM(tAudioTrackRM *atrm)
{
	return AudioTrackControlRM(atrm, 1);
}

int AudioTrackStopRM(tAudioTrackRM *atrm)
{
	return AudioTrackControlRM(atrm, 0);
}

int AudioTrackSetupRM(tAudioTrackRM *atrm, uint32_t rate, uint8_t channels, uint8_t bits)
{
	int ret;
	void *args[4] = {0};

	args[0] = (void *)atrm->at;
	args[1] = (void *)(uintptr_t)rate;
	args[2] = (void *)(uintptr_t)channels;
	args[3] = (void *)(uintptr_t)bits;

	hal_mutex_lock(atrm->mutex);
	ret = (int)func_stub(RPCCALL_AUDIO(AudioTrackSetupRM), 1, ARRAY_SIZE(args), args);
	hal_mutex_unlock(atrm->mutex);

	return ret;
}

int AudioTrackWriteRM(tAudioTrackRM *atrm, void *data, uint32_t size)
{
	int ret;
	void *args[3] = {0};

	args[0] = (void *)atrm->at;
	args[1] = (void *)data;
	args[2] = (void *)(uintptr_t)size;

	hal_mutex_lock(atrm->mutex);
	hal_dcache_clean((unsigned long)data, (unsigned long)size);
	ret = (int)func_stub(RPCCALL_AUDIO(AudioTrackWriteRM), 1, ARRAY_SIZE(args), args);
	hal_mutex_unlock(atrm->mutex);

	return ret;
}
