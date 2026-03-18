#ifndef __AHCI_NVT_UTIL_H__
#define __AHCI_NVT_UTIL_H__

#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include "ahci.h"

struct nvt_ahci_plat_data {
	void __iomem *top_va_base;
	void __iomem *phy_va_base;
#if IS_ENABLED(CONFIG_PCI)
	u32			 ep_idx;
#endif
};

/*************************
 *    Clone Prototype    *
 *************************/
int nvt_sata_set_spd_needed(struct ata_link *link);
bool nvt_ata_phys_link_offline(struct ata_link *link);
int nvt_ata_wait_ready(struct ata_link *link, unsigned long deadline,
		   int (*check_ready)(struct ata_link *link));
void nvt_ahci_platform_put_resources(struct device *dev, void *res);
int nvt_ahci_platform_get_phy(struct ahci_host_priv *hpriv, u32 port,
				struct device *dev, struct device_node *node);
int nvt_ahci_platform_get_regulator(struct ahci_host_priv *hpriv, u32 port,
				struct device *dev);
#endif /* __AHCI_NVT_UTIL_H__ */
