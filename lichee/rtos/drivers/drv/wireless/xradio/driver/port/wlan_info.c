/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
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
#ifdef CONFIG_ARCH_SUN20IW2P1

#include "net/wlan/wlan_info.h"
#include <stdio.h>
#define WLAN_SYSLOG   printf

extern uint32_t __RAM_LENGTH[];
extern uint32_t __FLASH_LENGTH[];
extern uint32_t __PSRAM_LENGTH[];
extern uint32_t __HPSRAM_LENGTH[];

void get_sysinfo(void)
{
#ifdef CONFIG_ARCH_RISCV_RV64
#define SYS_CORE "[RV-SYS]"
#elif defined(CONFIG_ARCH_ARM_ARMV8M)
#define SYS_CORE "[ARM-SYS]"
#endif
	WLAN_SYSLOG(SYS_CORE " OS Toolchain optimisation: "CONFIG_TOOLCHAIN_OPTIMISATION"\n");
	WLAN_SYSLOG(SYS_CORE " RAM    size: %6d KB\n", ((uint32_t)__RAM_LENGTH) / 1024);
#if defined(CONFIG_XIP)
	WLAN_SYSLOG(SYS_CORE " FLASH  size: %6d KB\n", ((uint32_t)__FLASH_LENGTH) / 1024);
#endif
#if defined(CONFIG_PSRAM)
	WLAN_SYSLOG(SYS_CORE " PSRAM  size: %6d KB\n", ((uint32_t)__PSRAM_LENGTH) / 1024);
#endif
#if defined(CONFIG_HPSRAM)
	WLAN_SYSLOG(SYS_CORE " HPSRAM size: %6d KB\n", ((uint32_t)__HPSRAM_LENGTH) / 1024);
#endif
#undef SYS_CORE
}

#ifdef CONFIG_ARCH_ARM_ARMV8M
#if (CONFIG_MBUF_IMPL_MODE == 0)
#include "sys/mbuf_0.h"
#endif
int wlan_get_sysinfo_m33(void)
{
	get_sysinfo();
	mb_mem_info();

	return 0;
}
#endif

int wlan_get_sysinfo(void)
{
#ifdef CONFIG_ARCH_RISCV_RV64
	get_sysinfo();
#endif
	wlan_get_sysinfo_m33();

	return 0;
}

#endif /* CONFIG_ARCH_SUN20IW2P1 */
