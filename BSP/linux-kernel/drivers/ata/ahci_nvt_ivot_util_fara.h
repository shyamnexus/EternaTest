#ifndef __AHCI_NVT_UTIL_H__
#define __AHCI_NVT_UTIL_H__

#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include "ahci.h"
#include <linux/gpio.h>

struct ahci_nvt_plat_data {
	void __iomem *gpio_ctrl;
	void __iomem *top_va_base;
	void __iomem *phy_va_base;
	int  gpio_num;
	u32 *temp_mem;
	u32 out_range;
	u32 skip_one;
	void __iomem *logmem;
	u32 ata_hrst;
	u32 *id_bitmap;
	u32 *sata_tune_en;
	u32 *nvt_sata_scope_gpio;
	u32 *nvt_sata_scope_event;
	struct ata_host *host;
	void (*dump_reg)(struct ata_port *ap, unsigned int tag);
	u32 dump_en;
	int ddrabt;
	u32 sata_id;
	u32 chip_id;
	u32 comm_rst;
	u32 fbsen_map;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	u32 use_fbs;
#endif
	bool ifs_occurred;
	bool escape_occurred;
	void (*cg_reset)(struct ata_link *link);
};

/************************************
 *    Override for customization    *
 ************************************/
int nvt_ahci_pmp_retry_softreset(struct ata_link *link, unsigned int *class,
				 unsigned long deadline);

int nvt_ahci_softreset(struct ata_link *link, unsigned int *class,
		       unsigned long deadline);
void nvt_ahci_pmp_attach(struct ata_port *ap);
void nvt_ahci_pmp_detach(struct ata_port *ap);
int nvt_ahci_port_resume(struct ata_port *ap);
int nvt_ahci_port_start(struct ata_port *ap);
enum ata_completion_errors nvt_ahci_qc_prep(struct ata_queued_cmd *qc);

int nvt_ahci_platform_init_host(struct platform_device *pdev,
				struct ahci_host_priv *hpriv,
				const struct ata_port_info *pi_template,
				struct scsi_host_template *sht);

struct ahci_host_priv *nvt_ahci_platform_get_resources(struct platform_device *pdev,
						   unsigned int flags);
void nvt_ahci_error_handler(struct ata_port *ap);

/*****************************
 *    Clone for reference    *
 *****************************/
int nvt_sata_set_spd_needed(struct ata_link *link);
bool nvt_ata_phys_link_offline(struct ata_link *link);
int nvt_ata_wait_ready(struct ata_link *link, unsigned long deadline, int (*check_ready)(struct ata_link *link));

#endif /* __AHCI_NVT_UTIL_H__ */
