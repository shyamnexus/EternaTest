/*
 * SW Interrupt driver on CC hw
 *
 * Copyright (c) 2022 - Novatek Microelectronics Corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>

#include <plat/hardware.h>
#include <plat/nvt_cc.h>
#include "nvt_cc_int.h"

#define DRV_VERSION             "1.00.000"
#define MODULE_NAME		        "nvt_cc"
#define MODULE_MINOR_COUNT	1

#define NVTCC_OF_DECLARE_DRIVER(name, compat, fn) \
	static void __init name##_of_cc_init_driver(struct device_node *np) \
{                                                               \
	of_node_clear_flag(np, OF_POPULATED);                   \
	fn(np);                                                 \
}                                                               \
OF_DECLARE_1(clk, name, compat, name##_of_cc_init_driver)


struct nvt_cc_data {
	void __iomem            *base;
	int			irq;

	struct class		*p_class;
	struct device		*pdevice;	// from device_create()

	struct cdev		cdev;
	dev_t			dev_id;
};

#if 1
static irqreturn_t drv_isr(int irq, void *devid)
{

	//struct nvt_cc_data *pdata = (struct nvt_cc_data*)devid;
	//u32	reg;

	nvt_cc_isr();
	nvt_cc_ist();
	return IRQ_HANDLED;
}
#endif

static void drv_init(struct nvt_cc_data *pdata)
{
}

static int file_open(struct inode *inode, struct file *file)
{
	struct nvt_cc_data *pdata;

	pdata = container_of(inode->i_cdev, struct nvt_cc_data, cdev);
	file->private_data = pdata;

	return 0;
}


struct file_operations cdev_fops = {
	.owner	= THIS_MODULE,
	.open    = file_open,
};

static void __init nvt_cc_module_init(struct device_node *np)
{
	struct nvt_cc_data *data;
	int ret = 0;
	CC_DEVICE  cc_device;

	printk("%s: driver probed\r\n", __func__);

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		printk("%s: alloc driver data fail\r\n", __func__);
		ret = -ENOMEM;
		goto err_out;
	}
	data->base = ioremap(NVT_CC_BASE_PHYS, 0x1000);
	if (!data->base) {
		printk("%s: base addr remap fail\r\n", __func__);
		ret = -ENODEV;
		goto err_free_mem;
	}
	cc_device.reg_base = data->base;
	nvt_cc_config(&cc_device);

	//Dynamic to allocate Device ID
	ret = alloc_chrdev_region(&data->dev_id, 0, MODULE_MINOR_COUNT, MODULE_NAME);
	if (ret) {
		printk("%s: alloc chrdev fail\r\n", __func__);
		goto err_unmap;
	}

	cdev_init(&data->cdev, &cdev_fops);
	data->cdev.owner = THIS_MODULE;


	data->irq = of_irq_get(np, 0);
	ret = request_irq(data->irq, drv_isr, IRQF_SHARED | IRQF_TRIGGER_HIGH, "CC_IRQ", data);
	if (ret) {
		printk("%s: request irq fail\r\n", __func__);
		goto err_free_irq;
	}
	drv_init(data);
	nvt_cc_open();

	goto err_out;

err_free_irq:
	free_irq(data->irq, data);
//err_destroy_class:
	//class_destroy(data->p_class);
//err_del_cdev:
	cdev_del(&data->cdev);
//err_unreg_chrdev:
	unregister_chrdev_region(data->dev_id, MODULE_MINOR_COUNT);
err_unmap:
	iounmap(data->base);
err_free_mem:
err_out:
	return;
}

//module_platform_driver(nvt_cc_driver);

NVTCC_OF_DECLARE_DRIVER(nvt_cc, "nvt,nvt_cc", nvt_cc_module_init);

