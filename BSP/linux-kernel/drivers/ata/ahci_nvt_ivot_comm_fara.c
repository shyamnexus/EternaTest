/*
 * AHCI SATA platform driver for NOVATECH IVOT SOCs
 *
 * Copyright (C) 2021 NOVATECH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/acpi.h>
#include <linux/pci_ids.h>
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
#include "ahci_nvt_ivot_na51090.h"
#endif
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102)
#include "ahci_nvt_ivot_na51102.h"
#endif
#if defined(CONFIG_NVT_IVOT_PLAT_NA51103)
#include "ahci_nvt_ivot_na51103.h"
#endif

static struct scsi_host_template ahci_platform_sht = {
        AHCI_SHT(DRV_NAME),
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		.can_queue = 32,
#endif
};

static int ahci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	int rc;
	static int first = 1;
	int val = 0;

    of_property_read_s32(dev->of_node,"device-id", &val);
    pdev->id = val;

    rc = ahci_nvt_sata100_init(pdev);
    if (rc) {
        return rc;
    }

	hpriv = nvt_ahci_platform_get_resources(pdev, 0);
	if (IS_ERR(hpriv))
		return PTR_ERR(hpriv);

	rc = ahci_platform_enable_resources(hpriv);
	if (rc)
		return rc;

	of_property_read_u32(dev->of_node,"ports-implemented", &hpriv->force_port_map);

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
#if defined(CONFIG_NVT_IVOT_PLAT_NA51090)
	if (nvt_get_chip_ver(NVT_PERIPHERAL_PHYS_BASE) == CHIPVER_A) {
		ahci_platform_sht.can_queue = 2;
	}
#endif
#endif

	rc = nvt_ahci_platform_init_host(pdev, hpriv, ahci_nvt_get_port_info(pdev, hpriv),
				     &ahci_platform_sht);
	if (rc)
		goto disable_resources;

	ahci_nvt_sata100_post_init(pdev, hpriv);

	if (first) {
		nvt_sata_proc_init();
		first = 0;
	}

	return 0;
disable_resources:
	ahci_platform_disable_resources(hpriv);
	return rc;
}

static int ahci_remove(struct platform_device *pdev)
{
	nvt_sata_proc_exit();

    ata_platform_remove_one(pdev);

	ahci_nvt_sata100_exit(pdev);

    return 0;
}

static SIMPLE_DEV_PM_OPS(ahci_pm_ops, ahci_platform_suspend,
			 ahci_platform_resume);

static const struct of_device_id ahci_of_match[] = {
    { .compatible = "novatech,nvt_sata100", },
    { .compatible = "nvt,nvt_sata100", },
	{},
};
MODULE_DEVICE_TABLE(of, ahci_of_match);

static struct platform_driver ahci_driver = {
	.probe = ahci_probe,
	.remove = ahci_remove,
	.driver = {
		.name = "nvt_sata100", /*"ahci",*/
		.owner = THIS_MODULE,
		.of_match_table = ahci_of_match,
		.pm = &ahci_pm_ops,
	},
};
module_platform_driver(ahci_driver);

MODULE_AUTHOR("Arvin Hsu");
MODULE_DESCRIPTION("NOVATEK AHCI SATA driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ahci");
MODULE_VERSION(DRV_VERSION);
