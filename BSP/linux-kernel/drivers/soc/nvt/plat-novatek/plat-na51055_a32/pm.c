/*
    NVT Power Management function

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
 */
#include "plat-na51055_a32/hardware.h"
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
#include <linux/of_address.h>
#include <linux/soc/nvt/nvt-info.h>
#include <uapi/linux/psci.h>
#include <asm/suspend.h>
#include <asm/barrier.h>
#include "plat/hardware.h"
#include "s2_firmware.h"

#define DRIVER_VERSION  "1.0.04"

unsigned int nvt_pm_dbg_info = 0;
unsigned int nvt_pm_func_mode = 0;
unsigned int nvt_pm_ddr_clock = 933;
module_param_named(nvt_pm_func_mode, nvt_pm_func_mode, int, S_IRUGO | S_IWUSR);
module_param_named(nvt_pm_ddr_clock, nvt_pm_ddr_clock, int, S_IRUGO | S_IWUSR);
module_param_named(nvt_pm_dbg_info, nvt_pm_dbg_info, int, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(nvt_pm_func_mode, "power down mode"); //0:pd3   1:ddr clock change
MODULE_PARM_DESC(nvt_pm_ddr_clock, "DDR clock dyanamic change level");
MODULE_PARM_DESC(nvt_pm_dbg_info, "NVT pm debug message enable");

static unsigned long nvt_resume_time_start = 0;
static unsigned long nvt_resume_time_end = 0;
unsigned long nvt_wakeup_int_padding = 0;
extern int nvt_cpu_suspend(unsigned long);

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
            if (nvt_wakeup_int_padding == 0) {
				pr_info("SUSPEND MEM...\n");
				cpu_suspend(0, nvt_cpu_suspend);
			} else {
				pr_info("Suspend wakeup force resume !! \n");
			}
			pr_info("A");
#if 0
			wfi();
#endif
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

int nvt_pm_get_pm_func_mode(unsigned int *mode)
{
	*mode = nvt_pm_func_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(nvt_pm_get_pm_func_mode);

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
	//enable cnn2 clk, enable cnn2 sram power,
	writel(readl(NVT_CG_BASE_VIRT+0x7c) | (1<<24), NVT_CG_BASE_VIRT+0x7c);

	//disable cnn2 clk auto gate
	writel(readl(NVT_CG_BASE_VIRT+0xb0) & ~(1<<5), NVT_CG_BASE_VIRT+0xb0);

	//enable cnn2 sram power,
	writel(readl(NVT_TOP_BASE_VIRT+0x1000) & ~(1<<22), NVT_TOP_BASE_VIRT+0x1000);

	//enable cnn2 sram for cpu,
	writel((readl(NVT_CNN2_BASE_VIRT) & ~(0x3<<24))|(0x2<<24), NVT_CNN2_BASE_VIRT);
	memcpy((void *)NVT_IFE_SRAM_BASE_VIRT, (void *)s2_51055_firmware, NVT_51055_S2_FIRMWARE_LEN);
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

	return 0;
}

device_initcall(nvt_pm_init);
MODULE_VERSION(DRIVER_VERSION);
