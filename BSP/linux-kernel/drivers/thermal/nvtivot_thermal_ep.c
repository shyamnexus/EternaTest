/*
 * Novatek thermal driver.
 *
 * Copyright (C) 2018 Novatek Microelectronics Corp.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <plat/efuse_protected.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>

struct nvt_thermal_dev {
	void __iomem    *reg_base;
	struct clk      *clk;
	unsigned long   trim_value;
};

static const struct of_device_id nvt_thermal_id_table[] = {
	{ .compatible = "nvt,thermal_ep" },
	{}
};
MODULE_DEVICE_TABLE(of, nvt_thermal_id_table);

static int thermal_get_temp(struct thermal_zone_device *thermal,
			    int *temp)
{
	struct nvt_thermal_dev *stdev = thermal->devdata;
	int temp_value;

	temp_value = ioread32(stdev->reg_base + 0x8);
	temp_value = temp_value - stdev->trim_value;

	*temp = temp_value * 1319;
	*temp = *temp / 1000 + 30;
	return 0;
}

static struct thermal_zone_device_ops ops = {
	.get_temp = thermal_get_temp,
};

static int nvt_thermal_probe(struct platform_device *pdev)
{
	struct thermal_zone_device *thermal_zone = NULL;
	struct nvt_thermal_dev *stdev;
	struct resource *res;
	u32 default_trim = 0x0;
	s32 result = 0;
	u16 data = 0;
	u32 chip_id = CHIP_RC;

	if (0 == of_property_read_u32(pdev->dev.of_node, "chip_id", &chip_id)) {
		//read property success
		if (chip_id > nvtpcie_get_ep_count(PCI_DEV_ID_ALL)) {
			return 0; //EP not linked, skip
		}
	}

	stdev = devm_kzalloc(&pdev->dev, sizeof(*stdev), GFP_KERNEL);

	if (!stdev) {
		return -ENOMEM;
	}

	of_property_read_u32(pdev->dev.of_node, "default_trim",
			     &default_trim);

	/* Enable thermal sensor */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (nvtpcie_is_common_pci()) {
		if (0 != nvtpcie_conv_resource((nvtpcie_chipid_t)chip_id, res)) {
			dev_err(&pdev->dev, "conv res failed, res %pR\n", res);
			return -EINVAL;
		}
	}

	if (nvtpcie_is_common_pci()) {
		stdev->reg_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	} else {
		stdev->reg_base = devm_ioremap_resource(&pdev->dev, res);
	}
	if (IS_ERR(stdev->reg_base)) {
		return PTR_ERR(stdev->reg_base);
	}

#ifdef CONFIG_NVT_OTP_EP_SUPPORT
	result = efuse_read_param_ops_ep(0, EFUSE_THERMAL_TRIM_DATA, &data);
#else
	result = -1;
#endif
	if (result == 0) {
		stdev->trim_value = data;
	} else {
		stdev->trim_value = default_trim;
	}

	thermal_zone = thermal_zone_device_register("nvt_thermal_ep", 0, 0,
			stdev, &ops, NULL, 0, 0);
	if (IS_ERR(thermal_zone)) {
		dev_err(&pdev->dev, "thermal zone device is NULL\n");
		return PTR_ERR(thermal_zone);
	}

	stdev->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (!IS_ERR(stdev->clk)) {
		clk_prepare(stdev->clk);
		clk_enable(stdev->clk);
	} else {
		pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));
	}

	/*Eanble controller*/
	iowrite32(0xB, stdev->reg_base + 0x4);

	platform_set_drvdata(pdev, thermal_zone);

	dev_info(&thermal_zone->device, "thermal_ctrl_ep 0x%08x\r\n", ioread32(stdev->reg_base + 0x4));
	dev_info(&thermal_zone->device, "tempature_ep 0x%08x\r\n", ioread32(stdev->reg_base + 0x8));

	return 0;
}

static int nvt_thermal_exit(struct platform_device *pdev)
{
	struct thermal_zone_device *thermal_zone = platform_get_drvdata(pdev);

	thermal_zone_device_unregister(thermal_zone);

	return 0;
}



static struct platform_driver nvt_thermal_driver = {
	.probe = nvt_thermal_probe,
	.remove = nvt_thermal_exit,
	.driver = {
		.name = "nvt_thermal_ep",
		.of_match_table = nvt_thermal_id_table,
	},
};

module_platform_driver(nvt_thermal_driver);

MODULE_AUTHOR("Steven Wang");
MODULE_DESCRIPTION("Novatek thermal ep driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("01.00.07");
