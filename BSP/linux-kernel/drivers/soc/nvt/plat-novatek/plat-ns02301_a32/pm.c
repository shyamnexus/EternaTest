/*
    NVT Power Management function

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/psci.h>
#include <linux/arm-smccc.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>
#include <linux/of_address.h>
#include <linux/soc/nvt/nvt-info.h>
#include <uapi/linux/psci.h>
#include <asm/suspend.h>
#include <asm/barrier.h>
#include "plat/hardware.h"
#include "s2_firmware.h"

#define DRIVER_VERSION  "1.0.03"

unsigned int nvt_pm_dbg_info = 0;
module_param_named(nvt_pm_dbg_info, nvt_pm_dbg_info, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nvt_pm_dbg_info, "NVT pm debug message enable");

enum ns02301_board_types {
	TPYE_NS02301_567_BOARD,
	TPYE_NS02301_568_BOARD,
};

static unsigned long nvt_resume_time_start = 0;
static unsigned long nvt_resume_time_end = 0;
static unsigned long nvt_board_568 = 0;
unsigned long nvt_wakeup_int_padding = 0;

extern int nvt_cpu_suspend(unsigned long);

static int nvt_load_s2_firmware(void)
{
	unsigned int value;

	value = readl((void *)0xfd142010);
	value |= 0x20;
	writel(value ,(void *)0xfd142010);

	value = readl((void *)0xfd142004);
	value &= 0xffff;
	value |= 0x70000;
	writel(value, (void *)0xfd142004);

	value = readl((void *)0xfd142010);
	value |= 0x4;
	writel(value ,(void *)0xfd142010);

	while (1) {
		value = readl((void *)0xfd142010);
		if ((value & 0x4) == 0)
			break;
	}
	value = readl(NVT_PR_CG_BASE_VIRT + 0x20);
	value |= ((1 << 22) | (1 << 23) | (1 << 24) | (1 << 25));
	writel(value, NVT_PR_CG_BASE_VIRT + 0x20);

	value = readl(NVT_PR_CG_BASE_VIRT + 0x20);
	value |= ((1 << 0) | (1 << 6) | (1 << 13));
	writel(value, NVT_PR_CG_BASE_VIRT + 0x20);

	value = readl(NVT_PR_CG_BASE_VIRT + 0x10);
	value &= 0xFFFFFF00;
	value |= 0xc2;
	writel(value, NVT_PR_CG_BASE_VIRT + 0x10);

	value = readl(NVT_PR_TOP_BASE_VIRT + 0x28);
	value &= ~(0xF0);
	writel(value, NVT_PR_TOP_BASE_VIRT + 0x28);

	value = readl(NVT_PR_SRAM_BASE_VIRT);
	value &= ~((1<<4)|(1<<2)|(1<<1)|(1<<0));
	writel(value, NVT_PR_SRAM_BASE_VIRT);

	memcpy((void *)NVT_PR_TCM_SRAM_BASE_VIRT, (const void *)&s2_02301_firmware, NVT_02301_S2_FIRMWARE_LEN);

	return 0;
}

static int nvt_check_current_board(void)
{
	void *fdt = initial_boot_params;
	int offset;

	unsigned int *cell = NULL;

	offset = fdt_path_offset(fdt,"/mmc@f0420000");
	if (offset < 0) {
		return -1;
	}
	cell = (unsigned int*)fdt_getprop((const void*)fdt, offset, "cd_gpio", NULL);

	if (*cell == 0x31000000) //P_GPIO(17) as cd gpio
		return TPYE_NS02301_567_BOARD;
	else
		return TPYE_NS02301_568_BOARD;
}

static int nvt_suspend(void)
{
	return 0;
}

static void nvt_resume(void)
{
	return;
}

static struct syscore_ops nvt_syscore_ops = {
	.suspend = nvt_suspend,
	.resume = nvt_resume,
};

int nvt_pm_enter(suspend_state_t state)
{
	switch (state) {
		case PM_SUSPEND_STANDBY:
			pr_info("STANDBY...\n");
			cpu_do_idle();
			break;
		case PM_SUSPEND_MEM:
			pr_info("SUSPEND MEM...\n");
			cpu_suspend(0, nvt_cpu_suspend);
			pr_info("A");
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(nvt_pm_enter);

int nvt_pm_set_wakeup_int_padding(void)
{
	nvt_wakeup_int_padding = 1;
	return 0;
}
EXPORT_SYMBOL_GPL(nvt_pm_set_wakeup_int_padding);

static int nvt_pm_valid(suspend_state_t state)
{
	switch (state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		return 1;
	default:
		return 0;
	}
}

int nvt_pm_prepare(void)
{
	int ret = 0;

	nvt_load_s2_firmware();	
	if (nvt_board_568)
		writel(0x1,(void __iomem *) (NVT_PR_TCM_SRAM_BASE_VIRT+0x10));
	return ret;
}

void nvt_pm_finish(void)
{
	return;
}

void nvt_pm_wake(void)
{
	nvt_resume_time_start = nvt_get_time();
	return;
}

void nvt_pm_end(void)
{
	nvt_resume_time_end = nvt_get_time();

	nvt_wakeup_int_padding = 0;

	if(nvt_pm_dbg_info == 1 ){
		pr_info("%s: Total resume time(us) %ld \n", __func__, nvt_resume_time_end - nvt_resume_time_start);
	}

	return;
}
EXPORT_SYMBOL_GPL(nvt_pm_end);

static const struct platform_suspend_ops nvt_pm_ops = {
	.valid      = nvt_pm_valid,
	.enter      = nvt_pm_enter,
	.prepare    = nvt_pm_prepare,
	.finish     = nvt_pm_finish,
	.wake       = nvt_pm_wake,
	.end        = nvt_pm_end,
};

static int __init nvt_pm_init(void)
{
	suspend_set_ops(&nvt_pm_ops);
	register_syscore_ops(&nvt_syscore_ops);

	if (nvt_check_current_board() == TPYE_NS02301_568_BOARD) {
		nvt_board_568 = 1;
	}

	return 0;
}

device_initcall(nvt_pm_init);
MODULE_VERSION(DRIVER_VERSION);

