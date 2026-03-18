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
#include <plat/thermal_int.h>
#include <linux/soc/nvt/rcw_macro.h>

static void __iomem *thermal_regbase;
static unsigned long   thermal_trim;

struct nvt_thermal_dev {
	void __iomem    *reg_base;
	struct clk      *clk;
	unsigned long   trim_value;
};

static const struct of_device_id nvt_thermal_id_table[] = {
	{ .compatible = "nvt,thermal" },
	{}
};
MODULE_DEVICE_TABLE(of, nvt_thermal_id_table);

#if IS_BUILTIN(CONFIG_NVTIVOT_THERMAL)
EXPORT_SYMBOL(nvt_thermal_get_temp);

int nvt_thermal_get_temp(int *temp)
{
    int temp_value;
    
	temp_value = ioread32(thermal_regbase + 0x8);
	temp_value = temp_value - thermal_trim;
    
#if (defined CONFIG_NVT_IVOT_PLAT_NA51102 || defined CONFIG_NVT_IVOT_PLAT_NS02201)
    *temp = temp_value * 954;
	*temp = *temp / 1000 + 30;
#elif (defined CONFIG_NVT_IVOT_PLAT_NS02301)
	*temp = temp_value * 510;
	*temp = *temp / 1000 + 30;
#else
	*temp = temp_value * 1319;
	*temp = *temp / 1000 + 30;
#endif

	return 0;
}
#endif

static int thermal_get_temp(struct thermal_zone_device *thermal,
			    int *temp)
{
	struct nvt_thermal_dev *stdev = thermal->devdata;
	int temp_value;

	temp_value = ioread32(stdev->reg_base + 0x8);
	temp_value = temp_value - stdev->trim_value;

#if (defined CONFIG_NVT_IVOT_PLAT_NA51102 || defined CONFIG_NVT_IVOT_PLAT_NS02201)
    *temp = temp_value * 954;
	*temp = *temp / 1000 + 30;
#elif (defined CONFIG_NVT_IVOT_PLAT_NS02301)
	*temp = temp_value * 510;
	*temp = *temp / 1000 + 30;
#else
	*temp = temp_value * 1319;
	*temp = *temp / 1000 + 30;
#endif

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
	s32 result = EFUSE_INVALIDATE_ERR;
	u16 data = 0;
	int ret;

	stdev = devm_kzalloc(&pdev->dev, sizeof(*stdev), GFP_KERNEL);
	if (!stdev) {
		return -ENOMEM;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "default_trim", &default_trim);
	if (ret) {
		pr_warn("%s: %s not found default_trim dts node\n", __func__, dev_name(&pdev->dev));
	}

	/* Enable thermal sensor */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	stdev->reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(stdev->reg_base)) {
		return PTR_ERR(stdev->reg_base);
	}
    thermal_regbase = (stdev->reg_base);
	
	#ifdef CONFIG_NVT_OTP_SUPPORT
	result = efuse_readParamOps(EFUSE_THERMAL_TRIM_DATA, &data);
	#else
	pr_alert("CONFIG_NVT_OTP_SUPPORT is disabled, use default trim\r\n");
	#endif

	if (result == EFUSE_SUCCESS) {
        #if (defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201) )
			data = data & 0xFF;
			if (data > 110 || data < 70) {
				//pr_err(" Thermal Trim data range  error THERMAL > 110 or THERMAL < 70 ==> [%d]\r\n", (int)data);
				stdev->trim_value = default_trim;
			} else {
				stdev->trim_value = data ;
			}
        #elif (defined CONFIG_NVT_IVOT_PLAT_NA51090 || defined CONFIG_NVT_IVOT_PLAT_NA51103)
			data = data & 0x3FF;
			if (data > 271 || data < 191) {
				//pr_err(" Thermal Trim data range  error THERMAL > 271 or THERMAL < 191 ==> [%d]\r\n", (int)data);
				stdev->trim_value = default_trim;
			} else {
				stdev->trim_value = data ;
			}
		#elif (defined CONFIG_NVT_IVOT_PLAT_NS02301) 
			data = data & 0x1FF;
			if (data > 221 || data < 161) {
				//pr_err(" Thermal Trim data range  error THERMAL > 221 or THERMAL < 161 ==> [%d]\r\n", (int)data);
				stdev->trim_value = default_trim;
			} else {
				//pr_err(" trim = [%d]\r\n", (int)data);
				stdev->trim_value = data ;
			}
        #else
			stdev->trim_value = data;
        #endif
	} else {
		//pr_err("not found out thermal trim, use default\r\n");
		stdev->trim_value = default_trim;
	}

    thermal_trim = stdev->trim_value;
	thermal_zone = thermal_zone_device_register("nvt_thermal", 0, 0, stdev, &ops, NULL, 0, 0);
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

	dev_info(&thermal_zone->device, "thermal_ctrl 0x%08x\r\n", ioread32(stdev->reg_base + 0x4));
	dev_info(&thermal_zone->device, "tempature 0x%08x\r\n", ioread32(stdev->reg_base + 0x8));

	return 0;
}

static int nvt_thermal_exit(struct platform_device *pdev)
{
	struct thermal_zone_device *thermal_zone = platform_get_drvdata(pdev);

	thermal_zone_device_unregister(thermal_zone);

	return 0;
}


#ifdef CONFIG_PM

static int thermal_enable(struct thermal_zone_device *thermal, int en)
{
	struct nvt_thermal_dev *stdev = thermal->devdata;

	if( en==1 ) {
		iowrite32(0xB, stdev->reg_base + 0x4);
	}else{
		iowrite32(0, stdev->reg_base + 0x4);
	}

	return 0;
}

static int thermal_is_enable(struct thermal_zone_device *thermal)
{
	struct nvt_thermal_dev *stdev = thermal->devdata;
	int thml_en_value, thmal_en ;

	thml_en_value =  ioread32(stdev->reg_base + 0x4);

	if(thml_en_value == 0xB ){
		thmal_en = 1 ;
	} else {
		thmal_en = 0 ;
	}


	return thmal_en;
}


static int nvt_thermal_suspend(struct device *dev)
{
	struct thermal_zone_device *thml_zdev = dev_get_drvdata(dev);
	struct nvt_thermal_dev	*stdev	= thml_zdev->devdata;

	if (stdev == NULL) {
		printk("nvt thermal is null !! \r\n ");
		return -ENODEV;
	}

	if(	thermal_is_enable(thml_zdev) ){
		//thermal disable 
		thermal_enable( thml_zdev , 0 );
		clk_disable(stdev->clk);
	} 

	clk_unprepare(stdev->clk);

	return 0;
}

static int nvt_thermal_resume(struct device *dev)
{
	struct thermal_zone_device *thml_zdev = dev_get_drvdata(dev);
	struct nvt_thermal_dev	*stdev	= thml_zdev->devdata;

	if (stdev == NULL) {
		return -ENODEV;
	}

	clk_prepare(stdev->clk);
	clk_enable(stdev->clk);
	thermal_enable( thml_zdev , 1 );

	return 0;
}

static const struct dev_pm_ops nvt_thermal_pm = {
	.suspend        = nvt_thermal_suspend,
	.resume         = nvt_thermal_resume,
};

#define nvt_thermal_pm_ops (&nvt_thermal_pm)
#else
#define nvt_thermal_pm_ops NULL
#endif


static struct platform_driver nvt_thermal_driver = {
	.probe = nvt_thermal_probe,
	.remove = nvt_thermal_exit,
	.driver = {
		.name = "nvt_thermal",
		.of_match_table = nvt_thermal_id_table,
		.pm             = nvt_thermal_pm_ops,
	},
};

module_platform_driver(nvt_thermal_driver);

MODULE_AUTHOR("Steven Wang");
MODULE_DESCRIPTION("Novatek thermal driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("01.00.15");
