#ifndef __AHCI_NVT_NS02201_H__
#define __AHCI_NVT_NS02201_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kconfig.h>  //for IS_ENABLED
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reset.h>

/* NOVATEK headers */
#include "ahci_nvt_ivot_util.h"
#if IS_ENABLED(CONFIG_PCI)
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif
#include <linux/soc/nvt/fmem.h>
#include <plat/hardware.h>

#define DRV_NAME	"ahci-nvt-ivot"
#define DRV_VERSION	"1.00.07"

const struct ata_port_info *nvt_ahci_get_port_info(
		struct platform_device *pdev, struct ahci_host_priv *hpriv);
struct ahci_host_priv *nvt_ahci_platform_get_resources(struct platform_device *pdev,
						   unsigned int flags);
#endif /* __AHCI_NVT_NS02201_H__ */
