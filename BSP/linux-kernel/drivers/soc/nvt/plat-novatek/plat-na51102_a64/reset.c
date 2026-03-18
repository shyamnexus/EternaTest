/**
    NVT reset function
    This file will setup reset function interface
    @file       nvt-reset.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/pm.h>
#include <linux/reboot.h>
#include <linux/delay.h>

#include <asm/io.h>

#include <linux/soc/nvt/nvt-io.h>
#include <plat/hardware.h>
#include <plat/pwbc_int.h>
/* A64 reboot func in atf */  
/*
#define WDT_CG_EN_OFS     0x140
#define WDT_CG_RESET_OFS  0x9C
#define WDT_CG_BIT        16
#define WDT_RST_BIT       4
#define WDT_MAN_OFS       0xC
#define REG_REMAP_SIZE	  256
#define STBC_REG_BASE     0x2f0580000
static void nvt_trigger_wdt_external(void)
{
	uint32_t reg;
	void __iomem *CG_BASE = NULL;
	void __iomem *WDT_BASE = NULL;
	void __iomem *STBC_BASE = NULL;

	CG_BASE = ioremap(NVT_CG_BASE_PHYS, REG_REMAP_SIZE);
	if (CG_BASE == NULL) {
		printk("%s: remap CG base 0x%lx fail\r\n", __func__, NVT_CG_BASE_PHYS);
		goto exit;
	}
	WDT_BASE = ioremap(NVT_WDT_BASE_PHYS, REG_REMAP_SIZE);
	if (WDT_BASE == NULL) {
		printk("%s: remap WDT base 0x%lx fail\r\n", __func__, NVT_WDT_BASE_PHYS);
		goto exit;
	}

	STBC_BASE = ioremap(STBC_REG_BASE, REG_REMAP_SIZE);
	if (STBC_BASE == NULL) {
		printk("%s: remap STBC base 0x%lx fail\r\n", __func__, STBC_REG_BASE);
		goto exit;
	}

	reg = nvt_readl(CG_BASE + WDT_CG_RESET_OFS);
	reg &= ~(0x1 << WDT_RST_BIT);
	nvt_writel(reg, CG_BASE + WDT_CG_RESET_OFS);

	udelay(50);

	reg = nvt_readl(CG_BASE + WDT_CG_RESET_OFS);
	reg |= (0x1 << WDT_RST_BIT);
	nvt_writel(reg, CG_BASE + WDT_CG_RESET_OFS);

	reg = nvt_readl(STBC_BASE + WDT_CG_EN_OFS);
	reg |= (0x1 << WDT_CG_BIT);
	nvt_writel(reg, STBC_BASE + WDT_CG_EN_OFS);

	nvt_writel(0x5a960112, WDT_BASE);

	udelay(80);

	nvt_writel(0x5a960113, WDT_BASE);

	nvt_writel(0x1, WDT_BASE + WDT_MAN_OFS);

exit:
	if (CG_BASE) iounmap(CG_BASE);
	if (WDT_BASE) iounmap(WDT_BASE);
	if (STBC_BASE) iounmap(STBC_BASE);
}

static int nvt_notify_reboot(struct notifier_block *nb, unsigned long action, void *data)
{
	if (action == SYS_RESTART) {
		pr_info("restarting.....\n");
		nvt_trigger_wdt_external();
	}

	return NOTIFY_DONE;
}
*/
/*
static void nvt_power_off_prepare(void)
{
#ifdef CONFIG_POWER_RESET_NA51102
	nvt_pwbc_power_control(0);
#endif
}
*/
/*
static struct notifier_block nvt_reboot_notifier = {
	.notifier_call  = nvt_notify_reboot,
	.next       = NULL,
	.priority   = INT_MAX,
};
*/
/*
static void nvt_power_off_done(void)
{
	pr_info(" Power off \n");
}
*/

static int __init nvt_poweroff_setup(void)
{
	pr_info("*** %s \n", __func__);

	//pm_power_off = nvt_power_off_done;
	//pm_power_off_prepare = nvt_power_off_prepare;
	//register_reboot_notifier(&nvt_reboot_notifier);

	return 0;
}

/* It will be executed when restart done */
void nvt_ivot_restart(enum reboot_mode mode, const char *cmd)
{
	pr_info("done\n");
	/* Waiting for core0 rtc flow ready */
	mdelay(5000);
}

arch_initcall(nvt_poweroff_setup);
