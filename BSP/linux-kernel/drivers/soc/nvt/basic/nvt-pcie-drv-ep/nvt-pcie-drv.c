/**
    NVT PCIE Common ep driver
    To handle NVT PCIE Common ep driver: connection and isr handling
    @file nvt-pcie-drv.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/dma-mapping.h>

#include "nvt_pcie_int.h"
//#include "pcie-drv-wrap.h"

#define DRV_VERSION		"0.00.030"

#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

static int is_init = 0;

// additionl region beside dts
static struct nvt_pcie_region ep_regions[MEM_PRIVATE_COUNT] = {
	// MEM_EP_MSIX_TBL
	{"EP_MSIX_TBL",	0x610020000,	0x10000},
	// MEM_EP_ATU
	{"EP_ATU",	0x610000000,	0x20000},
	// MEM_EP_CFG
	{"EP_CFG",	0x600000000,	0x10000},
};

static int nvt_pcie_probe(struct platform_device *pdev)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = NULL;
	struct resource *res = NULL;
	int ret = 0, i = 0;

	printk("%s: start \n", __func__);

	nvt_pcie_chip_ptr = kzalloc(sizeof(struct nvt_pcie_chip), GFP_KERNEL);
	if (!nvt_pcie_chip_ptr)
		goto out_free;

	nvt_pcie_chip_ptr->is_rc =	true;
	nvt_pcie_chip_ptr->dev =	&pdev->dev;

	for (i=0; i<MEM_DTS_COUNT; i++) {
		if (pdev->resource->flags == IORESOURCE_MEM) {
			/* setup resource */
			//printk("%s: IOMEM found\r\n", __func__);
			res = platform_get_resource(pdev, IORESOURCE_MEM, i);
			if (unlikely(!res)) {
				printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
				ret = -ENXIO;
				goto out_free;
			}

			/* FIXME hard code */
			if (resource_size(res) == 0)
				continue;
		}
		nvt_pcie_chip_ptr->res[i] = res;
		//printk("%s: res %d start 0x%llx\n", __func__, i, res->start);
		nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = ioremap(res->start, resource_size(res));
		if (unlikely(nvt_pcie_chip_ptr->PCIE_REG_BASE[i] == 0)) {
			printk("%s fails: ioremap fail\n", __FUNCTION__);
			ret = -EIO;
			goto remap_err;
		}
	}

	for (i=0; i<MEM_PRIVATE_COUNT; i++) {
		nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] = ioremap(ep_regions[i].addr, ep_regions[i].size);
		if (unlikely(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] == 0)) {
			printk("%s fails: ioremap fail\n", __FUNCTION__);
			ret = -EIO;
			goto remap_err;
		}
	}

	/* Basic data structure initialization */
	/* irq structure init. */
	nvt_pcie_chip_ptr->irq_summary	= NR_HW_IRQ_TOTAL;

	/* Initialize the lock before use to match the new kernel flow */
	spin_lock_init(&nvt_pcie_chip_ptr->lock);

	/* Mask all IRQ*/
	writel(~0, (void *)(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP] + 0x24));

	nvt_pcie_chip_ptr->irq = platform_get_irq(pdev, 0);

	platform_set_drvdata(pdev, nvt_pcie_chip_ptr);

	ret = nvt_pcie_ep_proc_init(nvt_pcie_chip_ptr);
	if(ret < 0)
		goto proc_err;

	dev_info(&pdev->dev, "Register %s successfully\n", __func__);
	return 0;
proc_err:
	nvt_pcie_ep_proc_remove(nvt_pcie_chip_ptr);

	for (i=0; i<MEM_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = NULL;
		}
	}

remap_err:
	for (i=0; i<MEM_DTS_COUNT; i++) {
		res = nvt_pcie_chip_ptr->res[i];
		if (res != NULL) {
			release_mem_region(res->start, (res->end - res->start + 1));
			nvt_pcie_chip_ptr->res[i]= NULL;
		}
	}

out_free:
	if (nvt_pcie_chip_ptr->va_msix_dummy_addr) {
		dma_free_coherent(&pdev->dev, NR_HW_IRQ_TOTAL*4,
				nvt_pcie_chip_ptr->va_msix_dummy_addr,
				nvt_pcie_chip_ptr->dma_msix_dummy_addr);
	}
	kfree(nvt_pcie_chip_ptr);

	return ret;
}

static int nvt_pcie_remove(struct platform_device *pdev)
{
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);
	struct resource *res;
	int i;

	nvt_pcie_ep_proc_remove(nvt_pcie_chip_ptr);

	for (i=0; i<MEM_DTS_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[i] = NULL;
		}

		res = nvt_pcie_chip_ptr->res[i];
		if (res != NULL) {
			release_mem_region(res->start, (res->end - res->start + 1));
			nvt_pcie_chip_ptr->res[i] = NULL;
		}
	}
	for (i=0; i<MEM_PRIVATE_COUNT; i++) {
		if (nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] != NULL) {
			iounmap(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i]);
			nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_DTS_COUNT+i] = NULL;
		}
	}
	if (nvt_pcie_chip_ptr->va_msix_dummy_addr) {
		dma_free_coherent(&pdev->dev, NR_HW_IRQ_TOTAL*4,
				nvt_pcie_chip_ptr->va_msix_dummy_addr,
				nvt_pcie_chip_ptr->dma_msix_dummy_addr);
	}
	platform_set_drvdata(pdev, NULL);
	kfree(nvt_pcie_chip_ptr);
	return 0;
}

#ifdef CONFIG_PM
static int nvt_pcie_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);

	return 0;
}
static int nvt_pcie_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = platform_get_drvdata(pdev);

	return 0;
}
static const struct dev_pm_ops nvt_pcie_pmops = {
	.suspend	= nvt_pcie_suspend,
	.resume		= nvt_pcie_resume,
};
#define NVT_PCIE_PMOPS &nvt_pcie_pmops
#else
#define NVT_PCIE_PMOPS NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_OF
static const struct of_device_id nvt_pcie_pin_match[] = {
	{ .compatible = "nvt,nvt_pcie_ep" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_pcie_pin_match);
#endif

// report status to pcie-drv-wrap
int is_nvt_pcie_init(void)
{
	return is_init;
}

static struct platform_driver nvt_pcie_drv = {
	.probe		= nvt_pcie_probe,
	.remove		= nvt_pcie_remove,
	.driver		= {
		.name	= "nvt_pcie_drv",
		.owner	= THIS_MODULE,
		.pm = NVT_PCIE_PMOPS,
#ifdef CONFIG_OF
		.of_match_table = nvt_pcie_pin_match,
#endif
	},
};
#if 1
static int __init nvt_pcie_init(void)
{
	printk("%s: start \n", __func__);

	is_init = 1;
	printk("%s: end\n", __func__);
	return platform_driver_register(&nvt_pcie_drv);
}

static void __exit nvt_pcie_exit(void)
{
      printk("%s: res nvt_pcie_exitnvt_pcie_exitnvt_pcie_exitnvt_pcie_exit \n", __func__);
	platform_driver_unregister(&nvt_pcie_drv);
}

arch_initcall(nvt_pcie_init);
module_exit(nvt_pcie_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NVT PCIE Common ep driver for nvt na51090 SOC");
MODULE_VERSION(DRV_VERSION);
#endif
