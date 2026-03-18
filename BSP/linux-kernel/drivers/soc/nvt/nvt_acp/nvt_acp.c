/**
    NVT acp function
    NVT acp driver
    @file       nvt-acp.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <asm/ftrace.h>
#include <linux/soc/nvt/nvt-acp.h>

#define HISTROY_COUNT	16
#define MODULE_COUNT	16
#define TRACE_LEVEL	8 /* Maximum is 8 */

extern int nvtacp_proc_init(struct device *dev);
extern int nvtacp_proc_exit(void);

struct nvtacp_histroy_t {
	phys_addr_t	address[HISTROY_COUNT];
	void		*backtrace[TRACE_LEVEL];
	int		addr_index;
};

struct nvtacp_t {
	struct device		*dev;
	spinlock_t		lock;
	struct nvtacp_histroy_t histroy[MODULE_COUNT];
};

struct resource *res;
struct nvtacp_t *nvtacp = NULL;

void nvtacp_dump_stack(void);

static inline void nvtacp_setup_trace(void **trace)
{
	int i;

	for (i = 0; i < TRACE_LEVEL; i++) {
		switch(i) {
			case 0:
				trace[0] = return_address(0);
				break;
			case 1:
				trace[1] = return_address(1);
				break;
			case 2:
				trace[2] = return_address(2);
				break;
			case 3:
				trace[3] = return_address(3);
				break;
			case 4:
				trace[4] = return_address(4);
				break;
			case 5:
				trace[5] = return_address(5);
				break;
			case 6:
				trace[6] = return_address(6);
				break;
			case 7:
				trace[7] = return_address(7);
				break;
			default:
				break;
		}
	}
}

void nvtacp_dump_stack()
{
	int i, m;
	for (m = 0; m < MODULE_COUNT; m++) {
		if (!nvtacp->histroy[m].backtrace[0])
			continue;

		dev_info(nvtacp->dev, "----- module[%d] -----\n", m);
		for (i = 0; i < HISTROY_COUNT; i++) {

			if (!nvtacp->histroy[m].address[i])
				continue;

			dev_info(nvtacp->dev, "addr[%d]: 0x%llx\n", i, nvtacp->histroy[m].address[i]);

		}

		dev_info(nvtacp->dev, "----- module[%d] : backtrace -----\n", m);
		for (i = 0; i < TRACE_LEVEL; i++)
			dev_info(nvtacp->dev, "%pS\n", nvtacp->histroy[m].backtrace[i]);
	}
}

phys_addr_t nvtacp_get_address(phys_addr_t addr)
{
	static int module_index = 0;
	int i;

	if (addr + res->start > res->end) {
		dev_err(nvtacp->dev, "addr 0x%llx > ACP address range [0x%llx ~ 0x%llx]\n", addr, res->start, res->end);
		return ((phys_addr_t) - 1);
	}
	
	/* Check if module already used ACP before */
	for (i = 0; i < MODULE_COUNT; i++) {
		if (!nvtacp->histroy[i].backtrace[0])
			continue;
		if ((unsigned long)nvtacp->histroy[i].backtrace[0] == (unsigned long)return_address(0)) {
			int index = nvtacp->histroy[i].addr_index;
			nvtacp->histroy[i].address[index] = addr;
			nvtacp->histroy[i].addr_index = (index + 1) >= HISTROY_COUNT ? 0 : (index + 1);
			break;
		}
	}

	if (i >= MODULE_COUNT) {
		int index = nvtacp->histroy[module_index].addr_index;
		nvtacp->histroy[module_index].address[0] = addr;
		nvtacp_setup_trace(nvtacp->histroy[module_index].backtrace);
		module_index = (module_index + 1) >= MODULE_COUNT ? 0 : (module_index + 1);
		nvtacp->histroy[module_index].addr_index = (index + 1) >= HISTROY_COUNT ? 0 : (index + 1);
	}

	return addr + res->start;
}
EXPORT_SYMBOL(nvtacp_get_address);

static int nvtacp_probe(struct platform_device *pdev)
{
	int ret = 0;

	nvtacp = devm_kzalloc(&pdev->dev, sizeof(struct nvtacp_t), GFP_KERNEL);
	if (nvtacp == NULL) {
		dev_err(&pdev->dev, "memory alloc fail !!!\n");
		return -ENOMEM;
	}
	nvtacp->dev = &pdev->dev;

	platform_set_drvdata(pdev, nvtacp);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		return -ENODEV;
	}

	spin_lock_init(&nvtacp->lock);

	ret = nvtacp_proc_init(&pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "nvtacp_proc_init fail\n");
		return ret;
	}

	dev_info(&pdev->dev, "nvt acp init success\n");

	return 0;
}

static int nvtacp_remove(struct platform_device *pdev)
{
	nvtacp_proc_exit();
	return 0;
}

static struct of_device_id xxx_match_table[] = {
	{.compatible = "nvt,nvt_acp"},
	{}
};

static struct platform_driver nvtacp_driver = {
	.driver = {
		.name   = "nvt_acp",
		.owner = THIS_MODULE,
		.of_match_table = xxx_match_table,
	},
	.probe      = nvtacp_probe,
	.remove     = nvtacp_remove,
};


int __init nvtacp_init(void)
{
	return platform_driver_register(&nvtacp_driver);
}

void __exit nvtacp_exit(void)
{
	platform_driver_unregister(&nvtacp_driver);
}

arch_initcall(nvtacp_init);
module_exit(nvtacp_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt acp driver");
MODULE_VERSION("1.00.000");
MODULE_LICENSE("GPL");
