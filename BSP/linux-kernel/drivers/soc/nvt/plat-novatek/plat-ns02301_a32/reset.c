/**
    NVT reset function
    This file will setup reset function interface
    @file       nvt-reset.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/pm.h>
#include <linux/reboot.h>
#include <linux/delay.h>

#include <asm/io.h>

#include <linux/soc/nvt/nvt-io.h>
#include <linux/of.h>
#include <plat/hardware.h>
#include <plat/rtc_int.h>
#include <plat/nvt_cc.h>

#define WDT_CG_EN_OFS     0x20
#define WDT_CG_RESET_OFS  0x28
#define WDT_CG_BIT        17
#define WDT_RST_BIT       17
#define WDT_MAN_OFS       0xC
#define REG_REMAP_SIZE	  256
#define WDT_DATA_OFS      0x84
static void nvt_trigger_wdt(void)
{
	uint32_t reg;
	void __iomem *CG_BASE = NULL;
	void __iomem *WDT_BASE = NULL;

	CG_BASE = ioremap(NVT_PR_CG_BASE_PHYS, REG_REMAP_SIZE);
	if (CG_BASE == NULL) {
		printk("%s: remap CG base 0x%x fail\r\n", __func__, NVT_CG_BASE_PHYS);
		goto exit;
	}
	WDT_BASE = ioremap(NVT_WDT_BASE_PHYS, REG_REMAP_SIZE);
	if (WDT_BASE == NULL) {
		printk("%s: remap WDT base 0x%x fail\r\n", __func__, NVT_WDT_BASE_PHYS);
		goto exit;
	}


	reg = nvt_readl(CG_BASE + WDT_CG_RESET_OFS);
	reg &= ~(0x1 << WDT_RST_BIT);
	nvt_writel(reg, CG_BASE + WDT_CG_RESET_OFS);

	udelay(50);

	reg = nvt_readl(CG_BASE + WDT_CG_RESET_OFS);
	reg |= (0x1 << WDT_RST_BIT);
	nvt_writel(reg, CG_BASE + WDT_CG_RESET_OFS);

	reg = nvt_readl(CG_BASE + WDT_CG_EN_OFS);
	reg |= (0x1 << WDT_CG_BIT);
	nvt_writel(reg, CG_BASE + WDT_CG_EN_OFS);

	nvt_writel(0x0, WDT_BASE + WDT_DATA_OFS);
	nvt_writel(0x5a960102, WDT_BASE);

	udelay(80);

	nvt_writel(0x5a960103, WDT_BASE);

	nvt_writel(0x1, WDT_BASE + WDT_MAN_OFS);

exit:
	if (CG_BASE) iounmap(CG_BASE);
	if (WDT_BASE) iounmap(WDT_BASE);
}

/* Do reset at machine_restart() stage
static int nvt_notify_reboot(struct notifier_block *nb, unsigned long action, void *data)
{
	if (action == SYS_RESTART) {
		pr_info("restarting.....\n");
		nvt_trigger_wdt_external();
	}

	return NOTIFY_DONE;
}
*/

static void nvt_power_off_prepare(void)
{
#ifdef CONFIG_RTC_DRV_NA51055
	nvt_rtc_power_control(0);
#endif
}

/*
static struct notifier_block nvt_reboot_notifier = {
	.notifier_call  = nvt_notify_reboot,
	.next       = NULL,
	.priority   = INT_MAX,
};
*/

static void nvt_power_off_done(void)
{
	u32 reg[2];
	struct device_node* of_node = of_find_compatible_node(NULL, NULL, "nvt,nvt_at80c51");
	if (of_property_read_u32_array(of_node, "reg", reg, 2) == 0) {
		u32 *p_reg = ioremap(reg[0], reg[1]);
		// check 8051 rstn
		if (p_reg && p_reg[0] & 0x1) {
#if IS_ENABLED(CONFIG_NVT_CC)
			CC_CMD cmd = {0};
			pr_info("at80c51 is available. do olny main part poweroff\n");
			cmd.cmd_buf[0] = 1; // MCU_CC_TYPE_SYSTEM
			cmd.cmd_buf[1] = 1; // MCU_CC_SYSCMD_ID_SHUTDOWN
			cmd.cmd_buf[2] = 0;
			cc_send_core_cmd(CC_CORE_MCU, &cmd);
#else
			pr_err("CONFIG_NVT_CC must be enabled for mcu turn off main part.\n");
#endif
			iounmap(p_reg);
		}
	} else {
		pr_info(" Power off\n");
	}
}

static int __init nvt_poweroff_setup(void)
{
	pr_info("*** %s \n", __func__);

	pm_power_off = nvt_power_off_done;
        pm_power_off_prepare = nvt_power_off_prepare;
	//register_reboot_notifier(&nvt_reboot_notifier);

	return 0;
}

/* It will be executed when restart done */
void nvt_ivot_restart(enum reboot_mode mode, const char *cmd)
{
	pr_info("done\n");
	/* Waiting for core0 rtc flow ready */
	mdelay(5000);
	nvt_trigger_wdt();
}

arch_initcall(nvt_poweroff_setup);
