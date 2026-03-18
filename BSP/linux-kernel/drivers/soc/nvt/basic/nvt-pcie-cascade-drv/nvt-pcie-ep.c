/**
    NVT PCIE EP driver
    @file nvt-pcie-ep.c
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
#include <linux/msi.h>

#include "nvt_pcie_int.h"
#include "pcie-drv-wrap.h"

#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

static int is_init = 0;

static void nvt_ep_gen_info(struct nvt_pcie_chip *chip)
{
//	unsigned int reg;

	if (chip->PCIE_REG_BASE[MEM_PCIE_DBI]) {
		struct nvt_pcie_info *p_info;

		p_info = nvt_pcie_get_info();
		if (p_info) {
			int i;

			p_info->link_masks |= 1<<CHIP_RC;
			p_info->dbi_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI];

			// RC
			for (i=0; i<ATU_MAP_COUNT; i++) {
				printk("%s: addr 0x%llx, size 0x%lx\r\n", __func__,
					chip->rc.phys_map[i].addr,
					chip->rc.phys_map[i].size);
				p_info->pcie_chip_map[CHIP_RC][i].type = chip->rc.phys_map[i].type;
				p_info->pcie_chip_map[CHIP_RC][i].addr = chip->rc.phys_map[i].addr;
				p_info->pcie_chip_map[CHIP_RC][i].size = chip->rc.phys_map[i].size;
			}
			// EP
			for (i=0; i<ATU_MAP_COUNT; i++) {
				printk("%s: addr 0x%llx, size 0x%lx\r\n", __func__,
					chip->ep.phys_map[i].addr,
					chip->ep.phys_map[i].size);
				p_info->pcie_chip_map[CHIP_EP0][i].type = chip->ep.phys_map[i].type;
				p_info->pcie_chip_map[CHIP_EP0][i].addr = chip->ep.phys_map[i].addr;
				p_info->pcie_chip_map[CHIP_EP0][i].size = chip->ep.phys_map[i].size;
			}
		}
	}
}

static int load_rc_inbound_dts(struct nvt_pcie_chip *chip)
{
	int rlen, offset, end;
	const __be32 *cell;
	const int CELL_SIZE = 2;

	cell = of_get_property(chip->dev->of_node, "dma-ranges", &rlen);
	if (cell == NULL) {
		printk("%s: property dma-ranges NOT found\r\n", __func__);
	} else {
		printk("%s: property dma-ranges len %d\r\n", __func__, rlen);
	}

	end = rlen / sizeof(__be32);

	for (offset=0; offset<end; offset+=(1+CELL_SIZE*3)) {
		u32 flag;
		u64 pci_addr;
		u64 cpu_addr;
		u64 range_size;

		flag = of_read_number(cell+offset, 1);
		pci_addr = of_read_number(cell+offset+1, CELL_SIZE);
		cpu_addr = of_read_number(cell+offset+1+CELL_SIZE, CELL_SIZE);
		range_size = of_read_number(cell+offset+1+CELL_SIZE*2, CELL_SIZE);
		printk("%s: offset %d, flag 0x%x, pci addr 0x%llx, cpu addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, flag, pci_addr, cpu_addr, range_size);

		if (flag & 0x01) {	// APB
			chip->rc.phys_map[ATU_MAP_APB].type = ATU_MAP_APB;
			chip->rc.phys_map[ATU_MAP_APB].addr = pci_addr;
			chip->rc.phys_map[ATU_MAP_APB].size = range_size;
		} else {		// MAU
			chip->rc.phys_map[ATU_MAP_MAU].type = ATU_MAP_MAU;
			chip->rc.phys_map[ATU_MAP_MAU].addr = pci_addr;
			chip->rc.phys_map[ATU_MAP_MAU].size = range_size;
		}
	}

	return 0;
}

static int load_ep_inbound_dts(struct nvt_pcie_chip *chip)
{
	int idx;
	int rlen, offset, end;
#ifdef CONFIG_OF
        struct device_node* of_node = of_find_node_by_path("/nvt_ep0");
#endif
	const __be32 *cell;
	const int CELL_SIZE = 2;

#ifdef CONFIG_OF
        if (!of_node) {
		printk("%s: ep dts not found\r\n", __func__);
		return -ENOMEM;
        }
#endif

	cell = of_get_property(of_node, "reg", &rlen);
	if (cell == NULL) {
		printk("%s: property reg NOT found\r\n", __func__);
	} else {
		printk("%s: property reg len %d\r\n", __func__, rlen);
	}

	end = rlen / sizeof(__be32);

	for (offset=0, idx=0; offset<end; offset+=(CELL_SIZE*2), idx++) {
		u64 pci_addr;
		u64 cpu_addr = 0;
		u64 range_size;

		pci_addr = of_read_number(cell+offset, CELL_SIZE);
		range_size = of_read_number(cell+offset+CELL_SIZE, CELL_SIZE);
		printk("%s: offset %d, pci addr 0x%llx, cpu addr 0x%llx, size 0x%llx\r\n",
			__func__, offset, pci_addr, cpu_addr, range_size);

		switch (idx) {
		case 0:
			chip->ep.phys_map[ATU_MAP_MAU].type = ATU_MAP_MAU;
			chip->ep.phys_map[ATU_MAP_MAU].addr = pci_addr;
			chip->ep.phys_map[ATU_MAP_MAU].size = range_size;
			break;
		case 2:
			chip->ep.phys_map[ATU_MAP_APB].type = ATU_MAP_APB;
			chip->ep.phys_map[ATU_MAP_APB].addr = pci_addr;
			chip->ep.phys_map[ATU_MAP_APB].size = range_size;
			break;
		default:
			break;
		}
	}

	return 0;
}

struct nvt_pcie_chip *nvtpcie_edma_chip_ptr_ep = NULL;
static int nvt_ep_probe(struct platform_device *pdev)
{
	struct nvt_pcie_chip *chip_ptr = NULL;
	struct resource *res = NULL;
	int ret = 0;//, i = 0;

	nvtpcie_edma_chip_ptr_ep = kzalloc(sizeof(struct nvt_pcie_chip), GFP_KERNEL);
	if (!nvtpcie_edma_chip_ptr_ep)
		return -ENOMEM;


	chip_ptr = kzalloc(sizeof(struct nvt_pcie_chip), GFP_KERNEL);
	if (!chip_ptr)
		return -ENOMEM;

	chip_ptr->dev			= &pdev->dev;

	if (load_rc_inbound_dts(chip_ptr) != 0) {
		printk("%s: load rc inbound dts fail\r\n", __func__);
		return -ENOMEM;
	}
	if (load_ep_inbound_dts(chip_ptr) != 0) {
		printk("%s: load ep inbound dts fail\r\n", __func__);
		return -ENOMEM;
	}

	// PCIE TOP
	if (pdev->resource->flags == IORESOURCE_MEM) {
	/* setup resource */
		printk("%s: IOMEM found\r\n", __func__);
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (unlikely(!res)) {
			printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
			ret = -ENXIO;
			goto out_free;
		}
	}
	chip_ptr->res[0] = res;
	printk("%s: res %d start 0x%llx\n", __func__, 0, res->start);

	/* reserve the memory region */
	if (!request_mem_region(res->start, resource_size(res),
							pdev->name)) {
			dev_err(&pdev->dev,
					"Unable to get memory/io address region 0x%08llx\n",
					res->start);
			ret = -EBUSY;
			goto req_mem_err;
	}

	chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP] = ioremap(res->start, resource_size(res));
	if (unlikely(chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP] == 0)) {
		printk("%s fails: ioremap fail\n", __FUNCTION__);
		ret = -EIO;
		goto remap_err;
	}

	// PCIE DBI
	if (pdev->resource->flags == IORESOURCE_MEM) {
	/* setup resource */
		printk("%s: IOMEM found\r\n", __func__);
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (unlikely(!res)) {
			printk("%s, %s fails: platform_get_resource IORESOURCE_MEM not OK", pdev->name, __FUNCTION__);
			ret = -ENXIO;
			goto out_free;
		}
	}
	chip_ptr->res[1] = res;
	printk("%s: res %d start 0x%llx\n", __func__, 1, res->start);

	/* reserve the memory region */
	if (!request_mem_region(res->start, resource_size(res),
							pdev->name)) {
			dev_err(&pdev->dev,
					"Unable to get memory/io address region 0x%08llx\n",
					res->start);
			ret = -EBUSY;
			goto req_mem_err;
	}

	chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI] = ioremap(res->start, resource_size(res));
	if (unlikely(chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI] == 0)) {
		printk("%s fails: ioremap fail\n", __FUNCTION__);
		ret = -EIO;
		goto remap_err;
	}

	chip_ptr->irq = platform_get_irq(pdev, 0);
	printk("%s: get irq %d\n", __func__, chip_ptr->irq);

	// tba init
	nvt_pcie_tba_init(chip_ptr,0, 0);

	/* Initialize the lock before use to match the new kernel flow */
	spin_lock_init(&chip_ptr->lock);

	/* Initialize proc files */
	ret = nvt_pcie_proc_init(chip_ptr);
	if (ret < 0)
		goto proc_err;

	// fill info
	nvt_ep_gen_info(chip_ptr);

	nvtpcie_edma_chip_ptr_ep->PCIE_REG_BASE[MEM_PCIE_DBI] = chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI];
       //edma init
	nvt_pcie_edma_init(nvtpcie_edma_chip_ptr_ep,0, 1);

	platform_set_drvdata(pdev, chip_ptr);

	return 0;

proc_err:
	nvt_pcie_proc_remove(chip_ptr);
remap_err:
	res = chip_ptr->res[0];
	if (res != NULL) {
		release_mem_region(res->start, (res->end - res->start + 1));
		chip_ptr->res[0]= NULL;
	}

	res = chip_ptr->res[1];
	if (res != NULL) {
		release_mem_region(res->start, (res->end - res->start + 1));
		chip_ptr->res[1]= NULL;
	}

req_mem_err:
out_free:
	kfree(chip_ptr);

	return ret;
}

static int nvt_ep_remove(struct platform_device *pdev)
{
	struct nvt_pcie_chip *chip_ptr = platform_get_drvdata(pdev);
	struct resource *res;

	nvt_pcie_proc_remove(chip_ptr);

	if (chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP] != NULL) {
		iounmap(chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP]);
		chip_ptr->PCIE_REG_BASE[MEM_PCIE_TOP] = NULL;
	}

	res = chip_ptr->res[0];
	if (res != NULL) {
		release_mem_region(res->start, (res->end - res->start + 1));
		chip_ptr->res[0] = NULL;
	}

	if (chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI] != NULL) {
		iounmap(chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI]);
		chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI] = NULL;
	}

	res = chip_ptr->res[1];
	if (res != NULL) {
		release_mem_region(res->start, (res->end - res->start + 1));
		chip_ptr->res[1] = NULL;
	}

	platform_set_drvdata(pdev, NULL);
	kfree(chip_ptr);
	kfree(nvtpcie_edma_chip_ptr_ep);

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
static const struct of_device_id nvt_ep_pin_match[] = {
	{ .compatible = "nvt,nvt_ep_drv" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_ep_pin_match);
#endif

// report status to pcie-drv-wrap
int is_nvt_ep_init(void)
{
	return is_init;
}

static struct platform_driver nvt_ep_drv = {
	.probe		= nvt_ep_probe,
	.remove		= nvt_ep_remove,
	.driver		= {
		.name	= "nvt_ep_drv",
		.owner	= THIS_MODULE,
		.pm = NVT_PCIE_PMOPS,
#ifdef CONFIG_OF
		.of_match_table = nvt_ep_pin_match,
#endif
	},
};

static int __init nvt_ep_init(void)
{
	struct nvt_pcie_info *p_info;

	p_info = nvt_pcie_get_info();
	if (p_info) {
		memset(p_info, 0, sizeof(struct nvt_pcie_info));
	}

	is_init = 1;

	return platform_driver_register(&nvt_ep_drv);
}

static void __exit nvt_ep_exit(void)
{
	platform_driver_unregister(&nvt_ep_drv);
}

arch_initcall(nvt_ep_init);
module_exit(nvt_ep_exit);


