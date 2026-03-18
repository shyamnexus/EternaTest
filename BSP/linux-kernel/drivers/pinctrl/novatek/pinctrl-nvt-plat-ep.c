/*
 * Driver for the Novatek pinmux
 *
 * Copyright (c) 2019, NOVATEK MICROELECTRONIC CORPORATION.  All rights reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "nvt_pinmux.h"
#include <linux/soc/nvt/nvt-pcie-lib.h>

unsigned long top_reg_addr_ep[64] = {};
unsigned long pad_reg_addr_ep[64] = {};

static PINMUX_FUNC_ID id_restore = 0x0;
static u32 pinmux_restore[64] = {};

unsigned long get_top_addr(int ep_ch)
{
	if (ep_ch + CHIP_EP0 > nvtpcie_get_ep_count(PCI_DEV_ID_ALL) || ep_ch + CHIP_EP0 > CHIP_MAX) {
		pr_err("invalid ep id %d\r\n", ep_ch + CHIP_EP0);
		return 0;
	}

	if (top_reg_addr_ep[ep_ch])
		return top_reg_addr_ep[ep_ch];
	else
		return 0;
}
EXPORT_SYMBOL(get_top_addr);

unsigned long get_pad_addr(int ep_ch)
{
	if (ep_ch + CHIP_EP0 > nvtpcie_get_ep_count(PCI_DEV_ID_ALL) || ep_ch + CHIP_EP0 > CHIP_MAX) {
		pr_err("invalid ep id %d\r\n", ep_ch + CHIP_EP0);
		return 0;
	}

	if (pad_reg_addr_ep[ep_ch])
		return pad_reg_addr_ep[ep_ch];
	else
		return 0;
}
EXPORT_SYMBOL(get_pad_addr);

static void nvt_pinmux_show_conflict(struct nvt_pinctrl_info *info)
{
	int i;

	pinmux_parsing_ep(info);
	for (i = 0; i < PIN_FUNC_MAX; i++) {
		pr_err("pinmux %-2d config 0x%x\n", i, info->top_pinmux[i].config);
	}

	panic("###### Conflicted Pinmux Setting ######\n");
}


int nvt_pinmux_capture_ep(PIN_GROUP_CONFIG *pinmux_config, int count, int ep_ch)
{
	struct nvt_pinctrl_info *info;
	int i, j, ret = E_OK;
	unsigned long top_addr;

	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	top_addr = get_top_addr(ep_ch);

	if (top_addr) {
		info->top_base = (void*) top_addr;
		info->ep_ch = ep_ch;
		pinmux_parsing_ep(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function)
					pinmux_config[j].config = info->top_pinmux[i].config;
			}
		}
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_capture_ep);

int nvt_pinmux_update_ep(PIN_GROUP_CONFIG *pinmux_config, int count, int ep_ch)
{
	struct nvt_pinctrl_info *info;
	int i, j, ret = E_OK;
	unsigned long top_addr;

	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	top_addr = get_top_addr(ep_ch);

	if (top_addr) {
		info->top_base = (void*) top_addr;
		info->ep_ch = ep_ch;
		pinmux_parsing_ep(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function)
					info->top_pinmux[i].config = pinmux_config[j].config;
			}
		}

		ret = pinmux_init_ep(info);
		if (ret == E_OK)
			ret = pinmux_set_config(id_restore, pinmux_restore[ep_ch]);
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_update_ep);


int pinmux_set_config_ep(PINMUX_FUNC_ID id, u32 pinmux, int ep_ch)
{
	struct nvt_pinctrl_info *info;
	int ret;
	unsigned long top_addr;

	if (in_interrupt() || in_atomic() || irqs_disabled())
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	else
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	top_addr = get_top_addr(ep_ch);

	if (top_addr) {
		info->top_base = (void*) top_addr;
		info->ep_ch = ep_ch;
		ret = pinmux_set_host(info, id, pinmux);

		if (id <= PINMUX_FUNC_ID_LCD2) {
			id_restore = id;
			pinmux_restore[ep_ch] = pinmux;
		}
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(pinmux_set_config_ep);


static int nvt_get_resource(struct platform_device *pdev, struct nvt_pinctrl_info *info, void __iomem **res, int res_idx)
{
	struct resource *nvt_mem_base;

	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, res_idx);

#if IS_ENABLED(CONFIG_PCI)
	if (-1 == nvtpcie_conv_resource(info->ep_ch + CHIP_EP0, nvt_mem_base)) {
		dev_err(&pdev->dev, "conv ep resource failed\n");
		return -EINVAL;
	}
#endif

	*res = devm_ioremap(&pdev->dev, nvt_mem_base->start, resource_size(nvt_mem_base));
	if (IS_ERR(*res)) {
		dev_err(&pdev->dev, "fail to get pinmux mem base\n");
		return -ENOMEM;
	}

	return 0;

}

static int nvt_pinmux_probe(struct platform_device *pdev)
{
	struct nvt_pinctrl_info *info;
	struct nvt_gpio_info *gpio;
	struct device_node *top_np;
	u32 value;
	u32 pad_config[4] = {0, 0, 0, 0};
	u32 gpio_config[2] = {0, 0};
	int nr_pinmux = 0, nr_pad = 0, nr_gpio = 0;
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret = 0;

	info = devm_kzalloc(&pdev->dev, sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		dev_err(&pdev->dev, "nvt pinmux mem alloc fail\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "chip_id", &info->ep_ch);
	if (ret == 0) {
		info->ep_ch = info->ep_ch - CHIP_EP0;
	}

	if (info->ep_ch + CHIP_EP0 > nvtpcie_get_ep_count(PCI_DEV_ID_ALL) || info->ep_ch + CHIP_EP0 > CHIP_MAX) {
		dev_err(&pdev->dev, "invalid ep id %d\r\n", info->ep_ch + CHIP_EP0);
		return -EINVAL;
	}

	for_each_child_of_node(pdev->dev.of_node,  top_np) {
		if (!of_get_property(top_np, "gpio_config", NULL))
			continue;

		nr_gpio++;
	}

	gpio = devm_kzalloc(&pdev->dev, nr_gpio * sizeof(struct nvt_gpio_info), GFP_KERNEL);
	if (!gpio) {
		dev_err(&pdev->dev, "nvt gpio mem alloc fail\n");
		return -ENOMEM;
	}

	nr_gpio = 0;

	if (nvt_get_resource(pdev, info, &info->top_base, 0) != 0) {
		dev_err(&pdev->dev, "get top_ep resource fail\r\n");
		return -EINVAL;
	}

	top_reg_addr_ep[info->ep_ch] = (unsigned long) info->top_base;

	if (nvt_get_resource(pdev, info, &info->pad_base, 1) != 0) {
		dev_err(&pdev->dev, "get pad_ep resource fail\r\n");
		return -EINVAL;
	}

	pad_reg_addr_ep[info->ep_ch] = (unsigned long) info->pad_base;

	if (nvt_get_resource(pdev, info, &info->gpio_base, 2) != 0) {
		dev_err(&pdev->dev, "get gpio_ep resource fail\r\n");
		return -EINVAL;
	}

	for_each_child_of_node(pdev->dev.of_node, top_np) {
		if (!of_property_read_u32(top_np, "pinmux", &value)) {
			info->top_pinmux[nr_pinmux].pin_function = nr_pinmux;
			info->top_pinmux[nr_pinmux].config = value;
			nr_pinmux++;
		}

		if (!of_property_read_u32_array(top_np, "pad_config", pad_config, 4)) {
			info->pad[nr_pad].pad_ds_pin = pad_config[0];
			info->pad[nr_pad].driving = pad_config[1];
			info->pad[nr_pad].pad_gpio_pin = pad_config[2];
			info->pad[nr_pad].direction = pad_config[3];
			nr_pad++;
		}

		if (!of_property_read_u32_array(top_np, "gpio_config", gpio_config, 2)) {
			gpio[nr_gpio].gpio_pin = gpio_config[0];
			gpio[nr_gpio].direction = gpio_config[1];
			nr_gpio++;
		}
	}

	if (nr_pinmux == 0)
		return -ENOMEM;

	pinmux_preset_ep(info);

	if (pinmux_init_ep(info))
		nvt_pinmux_show_conflict(info);

	if (nr_pad)
		pad_init_ep(info, nr_pad);

	if (nr_gpio)
		gpio_init(gpio, nr_gpio, info);

	if(nvt_pinmux_proc_init_ep(info))
		return -ENOMEM;

	pinmux_config->pin_function = PIN_FUNC_I2C;
	pinmux_config->config = 0x0;
	ret = nvt_pinmux_capture_ep(pinmux_config, 1, info->ep_ch);
	if (ret) {
    	pr_info("I2C capture ep err\n");
	} else {
		ret = pinmux_parsing_i2c_ep((uint32_t)pinmux_config[0].config, info->ep_ch);
	}

	return 0;
}

static int nvt_pinctrl_remove(struct platform_device *pdev)
{

	return 0;
}

static const struct of_device_id nvt_pinctrl_of_match[] = {
	{ .compatible = "nvt,nvt_top_ep", },
	{ },
};

static struct platform_driver nvt_pinctrl_driver = {
	.driver = {
		.name = "nvt-pinctrl-ep",
		.of_match_table = nvt_pinctrl_of_match,
	},
	.probe = nvt_pinmux_probe,
	.remove = nvt_pinctrl_remove,
};

static int __init nvt_pinctrl_init(void)
{
	return platform_driver_register(&nvt_pinctrl_driver);
}
module_init(nvt_pinctrl_init);

static void __exit nvt_pinctrl_exit(void)
{
	platform_driver_unregister(&nvt_pinctrl_driver);
}
module_exit(nvt_pinctrl_exit);

MODULE_DESCRIPTION("Novatek pinctrl ep driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.01.003");
MODULE_DEVICE_TABLE(of, nvt_pinctrl_of_match);
