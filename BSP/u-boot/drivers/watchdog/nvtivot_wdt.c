// SPDX-License-Identifier: GPL-2.0
/*
 * Watchdog driver for Novatek IVOT SoCs
 *
 * Copyright (C) 2020 Novatek Inc.
 * Author: Howard Chang
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

#define WDT_REG_ADDR(ofs)       (IOADDR_WDT_REG_BASE+(ofs))
#define WDT_GETREG(ofs)         INW(WDT_REG_ADDR(ofs))
#define WDT_SETREG(ofs,value)   OUTW(WDT_REG_ADDR(ofs), (value))

#define CG_REG_ADDR(ofs)       (IOADDR_CG_REG_BASE+(ofs))
#define CG_GETREG(ofs)         INW(CG_REG_ADDR(ofs))
#define CG_SETREG(ofs,value)   OUTW(CG_REG_ADDR(ofs), (value))

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02302_A64)//stbc domain
#define STBC_CG_REG_ADDR(ofs)       (IOADDR_STBC_CG_REG_BASE+(ofs))
#define STBC_CG_GETREG(ofs)         INW(STBC_CG_REG_ADDR(ofs))
#define STBC_CG_SETREG(ofs,value)   OUTW(STBC_CG_REG_ADDR(ofs), (value))
#endif

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
#define CG_ENABLE_OFS 0x140 //stbc domain
#define CG_RESET_OFS 0x9C
#define WDT_POS (1 << 16) //stbc domain
#define WDT_RST (1 << 4)

#define WDT_CLK_DIV	(2048)
#define WDT_TIMEOUT_MAX	(178)

#elif defined(CONFIG_TARGET_NS02302_A64)
#define CG_ENABLE_OFS 0x70 //stbc domain
#define CG_RESET_OFS 0x90
#define WDT_POS (1 << 0) //stbc domain
#define WDT_RST (1 << 9)

#define WDT_CLK_DIV     (2048)
#define WDT_TIMEOUT_MAX (178)

#elif defined(CONFIG_TARGET_NS02301)
#define CG_ENABLE_OFS 0x20 + 0x100000
#define CG_RESET_OFS 0x28 + 0x100000
#define WDT_POS (1 << 17)
#define WDT_RST (1 << 17)

#define WDT_CLK_DIV     (2048)
#define WDT_TIMEOUT_MAX (178)

#elif defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
#define CG_ENABLE_OFS 0x74
#define CG_RESET_OFS 0x84
#define WDT_POS (1 << 17)
#define WDT_RST (1 << 17)

#define WDT_CLK_DIV	(1024)
#define WDT_TIMEOUT_MAX	(89)

#elif defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103)
#define CG_ENABLE_OFS 0x7C
#define CG_RESET_OFS 0x9C
#define WDT_POS (1 << 4)
#define WDT_RST (1 << 12)

#define WDT_CLK_DIV	(2048)
#define WDT_TIMEOUT_MAX	(178)

#else
#define CG_ENABLE_OFS 0x400
#define CG_RESET_OFS 0x9C
#define WDT_POS (1 << 24)
#define WDT_RST (1 << 4)

#define WDT_CLK_DIV	(2048)
#define WDT_TIMEOUT_MAX	(178)
#endif

#define WDT_CTRL_OFS 0x0
#define WDT_TRIG_OFS 0x8

static void nvtviot_wdt_init(void)
{
	u32 reg_value;
	u32 timeout, timeout_unit = CONFIG_NVT_WDT_TIMEOUT;

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64) || defined(CONFIG_TARGET_NS02302_A64)
	reg_value = STBC_CG_GETREG(CG_ENABLE_OFS);
	STBC_CG_SETREG(CG_ENABLE_OFS, reg_value | WDT_POS);
#else
	reg_value = CG_GETREG(CG_ENABLE_OFS);
	CG_SETREG(CG_ENABLE_OFS, reg_value | WDT_POS);
#endif
	reg_value = CG_GETREG(CG_RESET_OFS);
	CG_SETREG(CG_RESET_OFS, reg_value | WDT_RST);

	if ((timeout_unit > WDT_TIMEOUT_MAX) || !timeout_unit) {
		printf("invalid timer config %d, set to %ds\n", timeout_unit, WDT_TIMEOUT_MAX);
		timeout = 0xFF;
	} else {
		timeout = (timeout_unit * 12000000 / WDT_CLK_DIV) >> 12;
	}
	//ext reset
	reg_value = (0x5A960012 | (timeout << 8));

	WDT_SETREG(WDT_CTRL_OFS, reg_value);

	udelay(80);

	WDT_SETREG(WDT_CTRL_OFS, reg_value | 0x1);
}

static void nvtviot_wdt_reset(void)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	/*Kick the dog*/
	WDT_SETREG(WDT_TRIG_OFS, 0x1);
#endif
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	nvtviot_wdt_reset();
}

void hw_watchdog_init(void)
{
	nvtviot_wdt_init();
}
#endif
