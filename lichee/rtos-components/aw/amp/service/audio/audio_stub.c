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



MAYBE_STATIC int audio_test1(void)
{
	printf("[%s] line:%d \n", __func__, __LINE__);
	return func_stub(RPCCALL_AUDIO(audio_test1), 1, 0, NULL);
}


MAYBE_STATIC int audio_test2(int arg1, int arg2)
{
	void *args[2] = {0};
	args[0] = (void *)(uintptr_t)arg1;
	args[1] = (void *)(uintptr_t)arg2;

	return func_stub(RPCCALL_AUDIO(audio_test2), 1, ARRAY_SIZE(args), args);
}

MAYBE_STATIC int audio_test3_set(uint8_t *buf, uint32_t len)
{
	void *args[2] = {0};
	args[0] = (void *)buf;
	args[1] = (void *)(uintptr_t)len;

	printf("[%s] line:%d buf=%p, len=%d\n", __func__, __LINE__, buf, len);
	hal_dcache_clean((unsigned long)buf, (unsigned long)len);
	return func_stub(RPCCALL_AUDIO(audio_test3), 1, ARRAY_SIZE(args), args);
}

MAYBE_STATIC int audio_test4_get(uint8_t *buf, uint32_t len)
{
	int ret;
	void *args[2] = {0};
	args[0] = (void *)buf;
	args[1] = (void *)(uintptr_t)len;

	printf("[%s] line:%d buf=%p, len=%d\n", __func__, __LINE__, buf, len);
	hal_dcache_invalidate((unsigned long)buf, (unsigned long)len);
	ret = func_stub(RPCCALL_AUDIO(audio_test4), 1, ARRAY_SIZE(args), args);
	hal_dcache_invalidate((unsigned long)buf, (unsigned long)len);
	return ret;
}

/* AudioTrack */
enum {
	AT_RM_STATE_SETUP = 0,
	AT_RM_STATE_RUNNING,
};

/* AudioRecord */
enum {
	AR_RM_STATE_SETUP = 0,
	AR_RM_STATE_RUNNING,
};

#if defined(CONFIG_AMP_AUDIO_PB_API_UNIQUE)
#include "audio_stub_pb_unique.c"
#elif defined(CONFIG_AMP_AUDIO_PB_API_ALIAS)
#include "audio_stub_pb_alias.c"
#endif

#if defined(CONFIG_AMP_AUDIO_CAP_API_UNIQUE)
#include "audio_stub_cap_unique.c"
#elif defined(CONFIG_AMP_AUDIO_CAP_API_ALIAS)
#include "audio_stub_cap_alias.c"
#endif


#if defined(CONFIG_AMP_AUDIO_UTILS)
#include "audio_stub_utils.c"
#endif
