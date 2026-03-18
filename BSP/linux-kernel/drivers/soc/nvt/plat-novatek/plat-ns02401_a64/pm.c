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
#include <linux/of_address.h>
#include <linux/soc/nvt/nvt-info.h>
#include <uapi/linux/psci.h>
#include <asm/suspend.h>
#include <asm/barrier.h>
#include <plat/nvt-sramctl.h>
#include <plat/s3_firmware.h>

static unsigned long nvt_resume_time_start = 0;
static unsigned long nvt_resume_time_end = 0;
void *s3_data_area = NULL;
void *s3_cg_base = NULL;

typedef unsigned long (psci_fn)(unsigned long, unsigned long,
					unsigned long, unsigned long);
static psci_fn *invoke_psci_fn;

static unsigned long __invoke_psci_fn_smc(unsigned long function_id,
		unsigned long arg0, unsigned long arg1,
		unsigned long arg2)
{
	struct arm_smccc_res res;
	//pr_info("%s(%d)ID %lx, arg0 %lx, arg1 %lx, arg2 %lx\n", __func__, __LINE__, function_id, arg0, arg1, arg2);
	pr_info("smc\n");
	arm_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	return res.a0;
}

static int psci_system_suspend(unsigned long unused)
{
	invoke_psci_fn = __invoke_psci_fn_smc;
	return invoke_psci_fn(PSCI_0_2_FN64_CPU_SUSPEND,
					0, __pa_symbol(cpu_resume), 0);
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
			cpu_suspend(0, psci_system_suspend);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(nvt_pm_enter);

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


#define NVT_SRAM_BASE 0x2f0d40000
#define NVT_SRAM_SIZE 0x10000
#define NVT_CG_BASE   0x2f0020000
#define NVT_CG_SIZE   0x1000

int nvt_copy_sram(void)
{
	void __iomem *virt_sram = NULL;

	virt_sram = ioremap(NVT_SRAM_BASE, NVT_SRAM_SIZE);
	if (!virt_sram) {
		pr_warn("SRAM: Could not map\n");
		return -1;
	}

	memcpy(virt_sram, (void *)&s3_firmware, sizeof(s3_firmware));
	if (nvt_get_suspend_mode()) {
		writel(1, virt_sram + 0xf010);
	} else {
		writel(0, virt_sram + 0xf010);
	}


	writeq(S3_MAGIC_NUMBER, virt_sram + 0xf000);
	writeq(virt_to_phys(s3_data_area), virt_sram + 0xf008);
	iounmap(virt_sram);

	return 0;
}

int nvt_pm_prepare(void)
{
	int ret = 0;

	//IFE_SHARED_SRAM_CLKSEL[4]: 0:SRAM for CPU R/W 1:SRAM for IFE
	//change for cpu use
	writel(readl(s3_cg_base + 0x18) & (~(1<<2)), s3_cg_base + 0x18);

	nvt_disable_sram_shutdown(IFE_SD);

	ret = nvt_copy_sram();

	return ret;
}

void nvt_pm_finish(void)
{
	//IFE_SHARED_SRAM_CLKSEL[4]: 0:SRAM for CPU R/W 1:SRAM for IFE
        //change for ife use
	writel(readl(s3_cg_base + 0x18) | (1<<2), s3_cg_base + 0x18);

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

	pr_info("%s: Total resume time(us) %ld \n", __func__, nvt_resume_time_end - nvt_resume_time_start);

	return;
}
	
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
	s3_data_area = kmalloc(0x10000, GFP_KERNEL);
	s3_cg_base = ioremap(NVT_CG_BASE, NVT_CG_SIZE);
	if (!s3_cg_base) {
		pr_warn("s3 cg: could not map\n\n");
		return -1;
	}
	suspend_set_ops(&nvt_pm_ops);
	register_syscore_ops(&nvt_syscore_ops);

	return 0;
}

device_initcall(nvt_pm_init);
