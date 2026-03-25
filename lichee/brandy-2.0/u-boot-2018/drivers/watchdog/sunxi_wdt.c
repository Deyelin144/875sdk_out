// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog driver
 * Copyright (C) 2019 frank <frank@allwinnertech.com>
 */

#include <asm/io.h>
#include <asm/arch/watchdog.h>
#include <common.h>
#include <sunxi_board.h>

static const int wdt_timeout_map[] = {
	[1]  = 0x1,  /* 1s  */
	[2]  = 0x2,  /* 2s  */
	[3]  = 0x3,  /* 3s  */
	[4]  = 0x4,  /* 4s  */
	[5]  = 0x5,  /* 5s  */
	[6]  = 0x6,  /* 6s  */
	[8]  = 0x7,  /* 8s  */
	[10] = 0x8,  /* 10s */
	[12] = 0x9,  /* 12s */
	[14] = 0xA,  /* 14s */
	[16] = 0xB,  /* 16s */
};
#if defined(CONFIG_MACH_SUN20IW2)
#define KEY_FIELD_MAGIC (0x16aa0000)
#define WDT_TIMEOUT_OFFSET	(4)

void hw_watchdog_disable(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)WATCH_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;
	wtmode |= KEY_FIELD_MAGIC;

	writel(wtmode, &wdt->mode);
}

void hw_watchdog_clean(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)WATCH_BASE;

	writel(KEY_FIELD_MAGIC, &wdt->mode);
	isb();
	writel(0, &wdt->irq_en);
	writel(1, &wdt->irq_sta);
	writel(KEY_FIELD_MAGIC, &wdt->cfg);
}

static void watchdog_reset(int timeout)
{
	int timeout_set = timeout;
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)WATCH_BASE;
	unsigned int wtmode;

	hw_watchdog_disable();

	if (timeout > 16) {
		timeout_set = 16;
	}

	if (wdt_timeout_map[timeout_set] == 0) {
		timeout_set++;
	}

	wtmode = KEY_FIELD_MAGIC | (wdt_timeout_map[timeout_set] << WDT_TIMEOUT_OFFSET) | WDT_MODE_EN;

	writel(KEY_FIELD_MAGIC | WDT_CFG_RESET, &wdt->cfg);
	writel(wtmode, &wdt->mode);

	isb();
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdt->ctl);
}

#if defined(CONFIG_SUNXI_RTOS_OTA_RB)
void ota_watchdog_start(void)
{
	hw_watchdog_clean();
	watchdog_reset(WDOG_TIMEOUT);
}
#endif /* CONFIG_SUNXI_RTOS_OTA_RB */
#else
void hw_watchdog_disable(void)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)WATCH_BASE;
	unsigned int wtmode;

	wtmode = readl(&wdt->mode);
	wtmode &= ~WDT_MODE_EN;

	writel(wtmode, &wdt->mode);
}

static void watchdog_reset(int timeout)
{
	struct sunxi_wdog *wdt = (struct sunxi_wdog *)WATCH_BASE;
	unsigned int wtmode;

	hw_watchdog_disable();

	if (wdt_timeout_map[timeout] == 0)
		timeout++;

	wtmode = wdt_timeout_map[timeout] << 4 | WDT_MODE_EN;

	writel(WDT_CFG_RESET, &wdt->cfg);
	writel(wtmode, &wdt->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdt->ctl);
}
#endif /* CONFIG_MACH_SUN20IW2 */


void hw_watchdog_reset(void)
{
	/* open wdog only when normal boot*/
	if (get_boot_work_mode())
		return;

	watchdog_reset(WDOG_TIMEOUT);
}
