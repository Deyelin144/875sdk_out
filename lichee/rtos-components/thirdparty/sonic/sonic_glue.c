/*
 * Copyright (C) 2023 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include "sonic_glue.h"
#ifdef __CONFIG_SONIC
static sonicStream g_stream = NULL;
#endif

/*
* Regarding the difference between sonic_lite and sonic:
* Sonic not only supports variable speed, but also supports pitch change.
* At the same time, sonic supports double speed and structure,
* while sonic_lite only supports double speed.
* In principle, sonic_lite only supports mono.
* Although it can be forced to process two channels, the sound will be distorted.
*/

void *sonicMalloc(int size)
{
    return malloc(size);
}

void *sonicCalloc(int num, int size)
{
    return calloc(num, size);
}

void *sonicRealloc(void *p, int oldNum, int newNum, int size)
{
    return realloc(p, newNum * size);
}

void sonicFree(void *p)
{
    free(p);
}

float xrSonicGetPitch(void)
{
#if defined(__CONFIG_SONIC_LITE)
    return 0;
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return 0;
    }
    return sonicGetPitch(g_stream);
#endif
}

void xrSonicSetPitch(float pitch)
{
#if defined(__CONFIG_SONIC_LITE)
    return;
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return;
    }
    sonicSetPitch(g_stream, pitch);
#endif
}

float xrSonicGetVolume(void)
{
#if defined(__CONFIG_SONIC_LITE)
    return 0;
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return 0;
    }
    return sonicGetVolume(g_stream);
#endif
}

void xrSonicSetVolume(float vol)
{
#if defined(__CONFIG_SONIC_LITE)
    return;
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return;
    }
    sonicSetVolume(g_stream, vol);
#endif
}

float xrSonicGetSpeed(void)
{
#if defined(__CONFIG_SONIC_LITE)
    return sonicGetSpeed();
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return 0;
    }
    return sonicGetSpeed(g_stream);
#endif
}

void xrSonicSetSpeed(float speed)
{
#if defined(__CONFIG_SONIC_LITE)
    sonicSetSpeed(speed);
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return;
    }
    sonicSetSpeed(g_stream, speed);
#endif
}

void xrSonicFlushStream(void)
{
#if defined(__CONFIG_SONIC_LITE)
    sonicFlushStream();
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return;
    }
    sonicFlushStream(g_stream);
#endif
}
/*
* samples : data to be processed, with copy
*/
void xrSonicWriteShortToStream(short *samples, int numSamples)
{
#if defined(__CONFIG_SONIC_LITE)
    sonicWriteShortToStream(samples, numSamples);
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return;
    }
    sonicWriteShortToStream(g_stream, samples, numSamples);
#endif
}

int xrSonicReadShortFromStream(short *samples, int maxSamples)
{
#if defined(__CONFIG_SONIC_LITE)
    return sonicReadShortFromStream(samples, maxSamples);
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("please init Sonic at first\n");
        return 0;
    }
    return sonicReadShortFromStream(g_stream, samples, maxSamples);
#endif
    return 0;
}
/*
* sample : Maximum amount of data processed each time (short)
* return : -1->fail, 0->success
*/
int xrSonicInit(int rate, int channle, int sample)
{
    int ret = -1;
#if defined(__CONFIG_SONIC_LITE)
    ret = sonicInit(rate, sample);
#elif defined(__CONFIG_SONIC)
    g_stream = sonicCreateStream(rate, channle, sample);
    ret = (g_stream == NULL) ? -1 : 0;
#endif
    return ret;
}

void xrSonicDeinit(void)
{
#if defined(__CONFIG_SONIC_LITE)
    sonicDeInit();
#elif defined(__CONFIG_SONIC)
    if (g_stream == NULL) {
        printf("Sonic is not init\n");
        return;
    }
    sonicDestroyStream(g_stream);
    g_stream = NULL;
#endif
}
