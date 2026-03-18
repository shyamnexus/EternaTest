/**
    NVT SD/eMMC driver layer

    @file       nvt_sdmmc_drv.c
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
#include "../core/bus.h"


#define MMC_PROC_PATH           "nvt_info/nvt_mmc"
// used for debug
// #define NVT_MMC_TEST_ACTIVE  
// #define NVT_MMC_ENNC_TEST       


static struct nvt_mmc_config nvt_mmc_config_data = {
	.get_cd     = nvt_mmc_get_cd_polled,
	.get_ro     = nvt_mmc_get_ro_polled,
	.wires      = 4,
	.max_freq   = 96000000,
	.caps       = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
};

static struct nvt_mmc_config nvt_mmc2_config_data = {
	.get_cd     = nvt_mmc_get_cd_polled,
	.get_ro     = nvt_mmc_get_ro_polled,
	.wires      = 4,
	.max_freq   = 160000000,
	.caps       = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
};

static struct nvt_mmc_config nvt_mmc3_config_data = {
	.get_cd     = nvt_mmc_get_cd_polled,
	.get_ro     = nvt_mmc_get_ro_polled,
	.wires      = 4,
	.max_freq   = 160000000,
	.caps       = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
};

static const struct of_device_id nvt_mmcsd_of_dt_ids[] = {
	{ .compatible = "nvt,nvt_mmc", .data = &nvt_mmc_config_data },
	{ .compatible = "nvt,nvt_mmc2", .data = &nvt_mmc2_config_data },
	{ .compatible = "nvt,nvt_mmc3", .data = &nvt_mmc3_config_data },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_mmcsd_of_dt_ids);

static int nvt_sdmmc_drv_probe(struct platform_device *pdev)
//static int __init nvt_sdmmc_drv_probe(struct platform_device *pdev)
{
    return nvt_mmcsd_probe(pdev, nvt_mmcsd_of_dt_ids, MMC_PROC_PATH);
}

static int __exit nvt_sdmmc_drv_remove(struct platform_device *pdev)
{
    return nvt_mmcsd_remove(pdev);
}

#ifdef CONFIG_PM
static const struct dev_pm_ops nvt_mmcsd_pm = {
	.suspend        = nvt_mmcsd_suspend,
	.resume         = nvt_mmcsd_resume,
};

#define nvt_mmcsd_pm_ops (&nvt_mmcsd_pm)
#else
#define nvt_mmcsd_pm_ops NULL
#endif

static struct platform_driver nvt_sdmmc_driver = {
	.probe		= nvt_sdmmc_drv_probe,
	.remove		= __exit_p(nvt_sdmmc_drv_remove),
	.driver		= {
		.name	= "nvt_mmc",
		.owner	= THIS_MODULE,
		.pm	= nvt_mmcsd_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = nvt_mmcsd_of_dt_ids,
#endif
	},
};

extern struct mmc_driver nvt_mmc_test_driver;
extern void mmc_test_exit(void);

static int __init nvt_mmcsd_init(void)
{
#ifdef NVT_MMC_TEST_ACTIVE
	int ret = 0;
	ret = mmc_register_driver(&nvt_mmc_test_driver);
	if (ret != 0)
		return ret;
#endif
	printk("nvt mmc driver version is %s", MMC_MODULE_VER);
	return platform_driver_register(&nvt_sdmmc_driver);
}
module_init(nvt_mmcsd_init);

static void __exit nvt_mmcsd_exit(void)
{
#ifdef NVT_MMC_TEST_ACTIVE
	mmc_test_exit();
	mmc_unregister_driver(&nvt_mmc_test_driver);
#endif 
	platform_driver_unregister(&nvt_sdmmc_driver);
}
module_exit(nvt_mmcsd_exit);

MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MMC/SD driver for NOVATEK MMC controller");
MODULE_ALIAS("nvt_sdmmc_drv");
MODULE_VERSION(MMC_MODULE_VER);

