/*
 * AHCI SATA platform driver for NOVATEK IVOT SOCs
 *
 * Copyright (C) 2023 NOVATEK
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
#include <linux/kconfig.h>  //for IS_ENABLED
#if IS_ENABLED(CONFIG_PHY_NS02201_SATA)
#include "ahci_nvt_ivot_ns02201.h"
#endif

static struct scsi_host_template ahci_platform_sht = {
	AHCI_SHT(DRV_NAME),
};

static int ahci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	int rc;

	printk(DRV_NAME " driver version " DRV_VERSION "\r\n");

	hpriv = nvt_ahci_platform_get_resources(pdev,
					    AHCI_PLATFORM_GET_RESETS);
	if (IS_ERR(hpriv))
		return PTR_ERR(hpriv);

	rc = ahci_platform_enable_resources(hpriv);
	if (rc)
		return rc;

	of_property_read_u32(dev->of_node,
			     "ports-implemented", &hpriv->force_port_map);

	/* NOVATEK Enable sss, pi, fbscp, qphyinit before initializing the host */
	writel(readl(hpriv->mmio + HOST_CAP) | HOST_CAP_SSS, hpriv->mmio + HOST_CAP);
	writel(readl(hpriv->mmio + HOST_PORTS_IMPL) | 0x1, hpriv->mmio + HOST_PORTS_IMPL);
	writel(readl(hpriv->mmio + 0x100 + PORT_CMD) | PORT_CMD_FBSCP, hpriv->mmio + 0x100 + PORT_CMD);
	writel(readl(hpriv->mmio + 0xa4) | (1 << 14), hpriv->mmio + 0xa4);

	rc = ahci_platform_init_host(pdev, hpriv,
				     nvt_ahci_get_port_info(pdev, hpriv),
				     &ahci_platform_sht);
	if (rc)
		goto disable_resources;

	return 0;

disable_resources:
	ahci_platform_disable_resources(hpriv);
	return rc;
}

static SIMPLE_DEV_PM_OPS(ahci_pm_ops, ahci_platform_suspend,
			 ahci_platform_resume);

static const struct of_device_id ahci_of_match[] = {
	{ .compatible = "snps,dwc-ahci", },
	{},
};
MODULE_DEVICE_TABLE(of, ahci_of_match);

static struct platform_driver ahci_driver = {
	.probe = ahci_probe,
	.remove = ata_platform_remove_one,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = ahci_of_match,
		.pm = &ahci_pm_ops,
	},
};
module_platform_driver(ahci_driver);

MODULE_AUTHOR("NOVATEK");
MODULE_DESCRIPTION("NOVATEK AHCI SATA driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ahci");
MODULE_VERSION(DRV_VERSION);
