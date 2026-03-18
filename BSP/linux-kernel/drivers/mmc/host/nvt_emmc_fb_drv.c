/**
    NVT eMMC fast boot driver layer

    @file       nvt_emmc_fb_drv.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include "nvt_mmchost.h"
#include "nvt_mmc.h"

#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>


#define MMC_FB_PROC_PATH        "nvt_info/nvt_mmc_fb"


static struct nvt_mmc_config nvt_emmc_config_data = {
	.get_cd     = nvt_mmc_get_cd_polled,
	.get_ro     = nvt_mmc_get_ro_polled,
	.wires      = 4,
	.max_freq   = 160000000,
	.caps       = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
};

static const struct of_device_id nvt_emmc_fb_of_dt_ids[] = {
	{ .compatible = "nvt,nvt_emmc", .data = &nvt_emmc_config_data },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_emmc_fb_of_dt_ids);

static int __init nvt_emmc_fb_drv_probe(struct platform_device *pdev)
{
    return nvt_mmcsd_probe(pdev, nvt_emmc_fb_of_dt_ids, MMC_FB_PROC_PATH);
}

static int __exit nvt_emmc_fb_drv_remove(struct platform_device *pdev)
{
    return nvt_mmcsd_remove(pdev);
}

#ifdef CONFIG_PM
static const struct dev_pm_ops nvt_emmc_fb_pm = {
	.suspend        = nvt_mmcsd_suspend,
	.resume         = nvt_mmcsd_resume,
};

#define nvt_emmc_fb_pm_ops (&nvt_emmc_fb_pm)
#else
#define nvt_emmc_fb_pm_ops NULL
#endif

static struct platform_driver nvt_emmc_fb_driver = {
	.probe		= nvt_emmc_fb_drv_probe,
	.remove		= __exit_p(nvt_emmc_fb_drv_remove),
	.driver		= {
		.name	= "nvt_mmc_fb",
		.owner	= THIS_MODULE,
		.pm	= nvt_emmc_fb_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = nvt_emmc_fb_of_dt_ids,
#endif
	},
};

static int __init nvt_emmc_fb_init(void)
{
	return platform_driver_register(&nvt_emmc_fb_driver);
}
module_init(nvt_emmc_fb_init);

static void __exit nvt_emmc_fb_exit(void)
{
	platform_driver_unregister(&nvt_emmc_fb_driver);
}
module_exit(nvt_emmc_fb_exit);

MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("eMMC fast boot driver for NOVATEK MMC controller");
MODULE_ALIAS("nvt_emmc_fb_drv");
MODULE_VERSION(MMC_MODULE_VER);

