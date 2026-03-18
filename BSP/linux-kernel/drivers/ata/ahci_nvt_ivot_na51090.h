#ifndef __AHCI_NVT_NA51090_H__
#define __AHCI_NVT_NA51090_H__
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include "ahci.h"
#include "ahci_nvt_ivot_util_fara.h"
#include <plat/hardware.h>

#define DRV_NAME	"NVT_SATA100_AHCI"
#define DRV_VERSION	"1.00.41"

void nvt_sata_proc_exit(void);
int nvt_sata_proc_init(void);

void nvt_sata100_clk_enable(void);
int ahci_nvt_sata100_init(struct platform_device *pdev);
int ahci_nvt_sata100_post_init(struct platform_device *pdev, struct ahci_host_priv *hpriv);

const struct ata_port_info *ahci_nvt_get_port_info(struct platform_device *pdev, struct ahci_host_priv *hpriv);
int ahci_nvt_sata100_exit(struct platform_device *pdev);

/************************************
 *    Override for customization    *
 ************************************/
int nvt_sata_link_hardreset(struct ata_link *link, const unsigned long *timing, unsigned long deadline, bool *online, int (*check_ready)(struct ata_link *));

#endif /* __AHCI_NVT_NA51090_H__ */
