/*
 * Driver for the Novatek pinmux
 *
 * Copyright (c) 2022, NOVATEK MICROELECTRONIC CORPORATION.  All rights reserved.
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
#include <linux/kconfig.h>
#include "nvt_pinmux.h"
#include <linux/soc/nvt/nvt-info.h>

unsigned long top_reg_addr = 0;
extern unsigned long top_reg_addr;

#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
unsigned long pr_top_reg_addr = 0;
extern unsigned long pr_top_reg_addr;
#endif

#if IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NS02201) || IS_ENABLED(CONFIG_PINCTRL_NS02302) || IS_ENABLED(CONFIG_PINCTRL_NS02301)
#else
static PINMUX_FUNC_ID id_restore = 0x0;
static u32 pinmux_restore = 0x0;
#endif

#if IS_ENABLED(CONFIG_PINCTRL_NA51103)
void nvt_pinmux_show(void)
{

	gpio_func_show();
}
EXPORT_SYMBOL(nvt_pinmux_show);
#endif

#if IS_ENABLED(CONFIG_PINCTRL_NA51055) || IS_ENABLED(CONFIG_PINCTRL_NA51089) || IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NA51090) || IS_ENABLED(CONFIG_PINCTRL_NA51103)
static int is_fastboot = 0; // dummy body for old ic 520, 560, 530, 336, 331
#else
extern unsigned int is_fastboot; //provide from nvt-clk-provider
#endif

static void nvt_pinmux_show_conflict(struct nvt_pinctrl_info *info)
{
	int i;

	pinmux_parsing(info);
	for (i = 0; i < PIN_FUNC_MAX; i++) {
		pr_err("pinmux %-2d config 0x%x\n", i, info->top_pinmux[i].config);
	}

	panic("###### Conflicted Pinmux Setting ######\n");
}


int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count)
{
	struct nvt_pinctrl_info *info;
	int i, j;
	int ret = E_OK;

	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	} else {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	}
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
	if (top_reg_addr && pr_top_reg_addr) {
	#else
	if (top_reg_addr) {
	#endif
		info->top_base = (void *) top_reg_addr;
		#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
		info->pr_top_base = (void *) pr_top_reg_addr;
		#endif
		pinmux_parsing(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function) {
					pinmux_config[j].config = info->top_pinmux[i].config;
				}
			}
		}
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_capture);

int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count)
{
	struct nvt_pinctrl_info *info;
	int i, j;
	int ret = E_OK;

	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	} else {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	}
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
	if (top_reg_addr && pr_top_reg_addr) {
	#else
	if (top_reg_addr) {
	#endif
		info->top_base = (void *) top_reg_addr;
		#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
		info->pr_top_base = (void *) pr_top_reg_addr;
		#endif
		pinmux_parsing(info);

		for (j = 0; j < count; j++) {
			for (i = 0; i < PIN_FUNC_MAX; i++) {
				if (i == pinmux_config[j].pin_function) {
					info->top_pinmux[i].config = pinmux_config[j].config;
				}
			}
		}

		ret = pinmux_init(info);
		if (ret == E_OK) {
#if IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NS02201) || IS_ENABLED(CONFIG_PINCTRL_NS02302) || IS_ENABLED(CONFIG_PINCTRL_NS02301)
			/* Do nothing, because the new architecture has integrated lcd settings into pinmux_init */
#else
			ret = pinmux_set_config(id_restore, pinmux_restore);
#endif
		} else {
			nvt_pinmux_show_conflict(info);
		}
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_update);

int pinmux_set_config(PINMUX_FUNC_ID id, u32 pinmux)
{
#if IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NS02201) || IS_ENABLED(CONFIG_PINCTRL_NS02302) || IS_ENABLED(CONFIG_PINCTRL_NS02301)
	PIN_GROUP_CONFIG pinmux_cfg[1] = {0};
	int ret = E_OK;

	switch (id) {
	case PINMUX_FUNC_ID_LCD:
		pinmux_cfg[0].pin_function = PIN_FUNC_SEL_LCD;
		pinmux_cfg[0].config = pinmux;
		ret = nvt_pinmux_update(pinmux_cfg, 1);
		break;

#if IS_ENABLED(CONFIG_PINCTRL_NS02302) || IS_ENABLED(CONFIG_PINCTRL_NS02301)
#else
	case PINMUX_FUNC_ID_LCD2:
		pinmux_cfg[0].pin_function = PIN_FUNC_SEL_LCD2;
		pinmux_cfg[0].config = pinmux;
		ret = nvt_pinmux_update(pinmux_cfg, 1);
		break;
#endif

	case PINMUX_FUNC_ID_TV:
		break;

	case PINMUX_FUNC_ID_HDMI:
		break;

	default:
		break;
	}

	return ret;
#else
	struct nvt_pinctrl_info *info;
	int ret = E_OK;

	if (in_interrupt() || in_atomic() || irqs_disabled()) {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_ATOMIC);
	} else {
		info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	}
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
	if (top_reg_addr && pr_top_reg_addr) {
	#else
	if (top_reg_addr) {
	#endif
		info->top_base = (void *) top_reg_addr;
		#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
		info->pr_top_base = (void *) pr_top_reg_addr;
		#endif
		ret = pinmux_set_host(info, id, pinmux);

		if (id <= PINMUX_FUNC_ID_LCD2) {
			id_restore = id;
			pinmux_restore = pinmux;
		}
	} else {
		pr_err("invalid pinmux address\n");
		ret = -ENOMEM;
	}

	kfree(info);

	return ret;
#endif
}
EXPORT_SYMBOL(pinmux_set_config);

#if IS_ENABLED(CONFIG_PM) && (IS_ENABLED(CONFIG_PINCTRL_NS02201) || IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NA51089) || IS_ENABLED(CONFIG_PINCTRL_NS02302))

static int nvt_pinmux_suspend(struct device *device)
{
#if (! IS_ENABLED(CONFIG_PINCTRL_NA51089))
	struct nvt_pinctrl_info *info = dev_get_drvdata(device);

	pr_info("%s: enter \n", __func__);

	if (!nvt_get_suspend_mode()) {
		return 0;
	}

	//restore Pinmux
	pinmux_parsing(info);

	//restore pad
	pad_suspend(info);

	pr_info("%s: successfully\n", __func__);
#endif

	return 0;

}

static int nvt_pinmux_resume(struct device *device)
{
#if (! IS_ENABLED(CONFIG_PINCTRL_NA51089))
	struct nvt_pinctrl_info *info = dev_get_drvdata(device);
	struct device_node *top_np;
	struct nvt_power_info *power;
	int nr_power = 0;
	u32 power_config[2] = {0, 0};

	pr_info("%s: enter \n", __func__);

	if (!nvt_get_suspend_mode()) {
		return 0;
	}

	//power init
	for_each_child_of_node(device->of_node,  top_np) {
		if (!of_get_property(top_np, "power_config", NULL)) {
			continue;
		}

		nr_power++;
	}

	power = kzalloc(nr_power * sizeof(struct nvt_power_info), GFP_KERNEL);
	if (!power) {
		dev_err(device, "nvt power mem alloc fail\n");
		return -ENOMEM;
	}
	nr_power = 0;
	for_each_child_of_node(device->of_node, top_np) {
		if (!of_property_read_u32_array(top_np, "power_config", power_config, 2)) {
			power[nr_power].pad_power_id = power_config[0];
			power[nr_power].pad_power = power_config[1];
			nr_power++;
		}
	}

	if (nr_power) {
		power_init(power, nr_power, info);
	}

	//Pinmux resume
	if (pinmux_init(info)) {
		nvt_pinmux_show_conflict(info);
	}

	//pad resume
	pad_resume(info);

	kfree(power);

	pr_info("%s: successfully\n", __func__);
#endif

	return 0;
}


const struct dev_pm_ops nvt_pinmux_pm_ops = {
		.suspend_noirq = nvt_pinmux_suspend,
		.resume_noirq = nvt_pinmux_resume,
};

#endif

int nvt_pinmux_probe(struct platform_device *pdev)
{
	struct nvt_pinctrl_info *info;
	struct nvt_gpio_info *gpio;
	struct nvt_power_info *power;
	struct resource *nvt_mem_base;
	struct device_node *top_np;
	u32 value;
	u32 pad_config[4] = {0, 0, 0, 0};
	u32 gpio_config[3] = {0, 0, 0};
	u32 power_config[2] = {0, 0};
	int nr_pinmux = 0, nr_pad = 0, nr_gpio = 0, nr_power = 0;
#if IS_ENABLED(CONFIG_PINCTRL_NS02201)
	PIN_GROUP_CONFIG pinmux_config[4];
	int ret = 0;
#elif IS_ENABLED(CONFIG_PINCTRL_NA51102)
	PIN_GROUP_CONFIG pinmux_config[2];
	int ret = 0;
#endif

	info = devm_kzalloc(&pdev->dev, sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		dev_err(&pdev->dev, "nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	for_each_child_of_node(pdev->dev.of_node,  top_np) {
		if (!of_get_property(top_np, "gpio_config", NULL)) {
			continue;
		}

		nr_gpio++;
	}

	gpio = devm_kzalloc(&pdev->dev, nr_gpio * sizeof(struct nvt_gpio_info), GFP_KERNEL);
	if (!gpio) {
		dev_err(&pdev->dev, "nvt gpio mem alloc fail\n");
		return -ENOMEM;
	}

	nr_gpio = 0;

	for_each_child_of_node(pdev->dev.of_node,  top_np) {
		if (!of_get_property(top_np, "power_config", NULL)) {
			continue;
		}

		nr_power++;
	}

	power = devm_kzalloc(&pdev->dev, nr_power * sizeof(struct nvt_power_info), GFP_KERNEL);
	if (!power) {
		dev_err(&pdev->dev, "nvt power mem alloc fail\n");
		return -ENOMEM;
	}

	nr_power = 0;

	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	info->top_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->top_base)) {
		dev_err(&pdev->dev, "fail to get pinmux mem base\n");
		return -ENOMEM;
	}

	top_reg_addr = (unsigned long) info->top_base;

	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	info->pad_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->pad_base)) {
		dev_err(&pdev->dev, "fail to get pad mem base\n");
		return -ENOMEM;
	}

	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	info->gpio_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->gpio_base)) {
		dev_err(&pdev->dev, "fail to get gpio mem base\n");
		return -ENOMEM;
	}

#if IS_ENABLED(CONFIG_PINCTRL_NS02301)
	/* PR_TOP */
	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	info->pr_top_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->pr_top_base)) {
		dev_err(&pdev->dev, "fail to get pr_top mem base\n");
		return -ENOMEM;
	}

	pr_top_reg_addr = (unsigned long) info->pr_top_base;

	/* PR_PAD */
	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	info->pr_pad_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->pr_pad_base)) {
		dev_err(&pdev->dev, "fail to get pr_pad mem base\n");
		return -ENOMEM;
	}

	/* PR_GPIO */
	nvt_mem_base = platform_get_resource(pdev, IORESOURCE_MEM, 5);
	info->pr_gpio_base = devm_ioremap_resource(&pdev->dev, nvt_mem_base);
	if (IS_ERR(info->pr_gpio_base)) {
		dev_err(&pdev->dev, "fail to get pr_gpio mem base\n");
		return -ENOMEM;
	}
#endif

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
			gpio[nr_gpio].level = gpio_config[1];
			nr_gpio++;
		}

		if (!of_property_read_u32_array(top_np, "power_config", power_config, 2)) {
			power[nr_power].pad_power_id = power_config[0];
			power[nr_power].pad_power = power_config[1];
			nr_power++;
		}
	}

	if (nr_pinmux == 0) {
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, info);

	pinmux_preset(info);

	pad_preset(info);

	if (is_fastboot) {
		// rtos has initialized, by pass pinmux_init
		pr_info("skip pinmux_init.\n");
	} else {
		if (pinmux_init(info)) {
			nvt_pinmux_show_conflict(info);
		}
	}

	if (nr_pad) {
		pad_init(info, nr_pad);
	}

#if IS_ENABLED(CONFIG_PINCTRL_NS02201)
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	pinmux_config[0].config = 0x0;
	pinmux_config[1].pin_function = PIN_FUNC_I2CII;
	pinmux_config[1].config = 0x0;
	pinmux_config[2].pin_function = PIN_FUNC_I2CIII;
	pinmux_config[2].config = 0x0;
	pinmux_config[3].pin_function = PIN_FUNC_I2CIV;
	pinmux_config[3].config = 0x0;
	ret = nvt_pinmux_capture(pinmux_config, 4);
	if (ret) {
		pr_info("I2C capture err\n");
	} else {
		ret = pinmux_parsing_i2c((uint32_t)pinmux_config[0].config);
		ret = pinmux_parsing_i2cII((uint32_t)pinmux_config[1].config);
		ret = pinmux_parsing_i2cIII((uint32_t)pinmux_config[2].config);
		ret = pinmux_parsing_i2cIV((uint32_t)pinmux_config[3].config);
	}
#elif IS_ENABLED(CONFIG_PINCTRL_NA51102)
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	pinmux_config[0].config = 0x0;
	pinmux_config[1].pin_function = PIN_FUNC_I2CII;
	pinmux_config[1].config = 0x0;
	ret = nvt_pinmux_capture(pinmux_config, 2);
	if (ret) {
		pr_info("I2C capture err\n");
	} else {
		ret = pinmux_parsing_i2c((uint32_t)pinmux_config[0].config);
		ret = pinmux_parsing_i2cII((uint32_t)pinmux_config[1].config);
	}
#endif

	if (nr_gpio) {
		gpio_init(gpio, nr_gpio, info);
	}

	if (nr_power) {
		power_init(power, nr_power, info);
	}

	if (nvt_pinmux_proc_init()) {
		return -ENOMEM;
	}

	dev_info(&pdev->dev, "Novatek pinmux driver init successfully: %s\n", DRV_VERSION);

#if IS_ENABLED(CONFIG_PINCTRL_NA51090) || IS_ENABLED(CONFIG_PINCTRL_NA51103)
	dev_info(&pdev->dev, "chip version 0x%x\n", nvt_get_chip_ver(NVT_PERIPHERAL_PHYS_BASE));
#elif IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NS02302)
	dev_info(&pdev->dev, "chip version 0x%x\n", nvt_get_chip_ver());
#endif

	return 0;
}

int nvt_pinctrl_remove(struct platform_device *pdev)
{

	return 0;
}

static const struct of_device_id nvt_pinctrl_of_match[] = {
	{ .compatible = "nvt,nvt_top", },
	{ },
};

static struct platform_driver nvt_pinctrl_driver = {
	.driver = {
		.name = "nvt-pinctrl",
		.of_match_table = nvt_pinctrl_of_match,
#if IS_ENABLED(CONFIG_PM) && (IS_ENABLED(CONFIG_PINCTRL_NS02201) || IS_ENABLED(CONFIG_PINCTRL_NA51102) || IS_ENABLED(CONFIG_PINCTRL_NA51089) || IS_ENABLED(CONFIG_PINCTRL_NS02302))
		.pm = &nvt_pinmux_pm_ops,
#endif
	},
	.probe = nvt_pinmux_probe,
	.remove = nvt_pinctrl_remove,
};

static int __init nvt_pinctrl_init(void)
{
	return platform_driver_register(&nvt_pinctrl_driver);
}
arch_initcall(nvt_pinctrl_init);

static void __exit nvt_pinctrl_exit(void)
{
	platform_driver_unregister(&nvt_pinctrl_driver);
}
module_exit(nvt_pinctrl_exit);

MODULE_DESCRIPTION("Novatek pinctrl driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRV_VERSION);
MODULE_DEVICE_TABLE(of, nvt_pinctrl_of_match);
