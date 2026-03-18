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

#include "ahci_nvt_ivot_ns02201.h"

static const struct ata_port_info ahci_port_info = {
	.flags		= AHCI_FLAG_COMMON,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_platform_ops,
};

/****************************
 *    Override Prototype    *
 ****************************/
/* Override [sata_link_resume] */
static enum ata_completion_errors nvt_ahci_qc_prep(struct ata_queued_cmd *qc);
static int nvt_ahci_hardreset(struct ata_link *link, unsigned int *class,
			  unsigned long deadline);
int nvt_sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			unsigned long deadline,
			bool *online, int (*check_ready)(struct ata_link *));
int nvt_sata_link_resume(struct ata_link *link, const unsigned long *params,
		     unsigned long deadline);
int nvt_ata_std_prereset(struct ata_link *link, unsigned long deadline);

int nvt_pmp_sata_std_hardreset(struct ata_link *link, unsigned int *class,
			      unsigned long deadline);
int nvt_pmp_sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			unsigned long deadline,
			bool *online, int (*check_ready)(struct ata_link *));
static int nvt_ahci_port_start(struct ata_port *ap);

static struct ata_port_operations nvt_ahci_ops = {
	.inherits				= &ahci_platform_ops,

	.qc_prep				= nvt_ahci_qc_prep,
	/* Override with NOVATEK solutions */
	.prereset               = nvt_ata_std_prereset,
	.hardreset              = nvt_ahci_hardreset,
	.pmp_hardreset          = nvt_pmp_sata_std_hardreset,
	.port_start				= nvt_ahci_port_start,
};

static const struct ata_port_info nvt_ahci_port_info = {
	.flags		= AHCI_FLAG_COMMON,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &nvt_ahci_ops,
};

static int ki_mode = 1;
module_param(ki_mode, int, 0444);
MODULE_PARM_DESC(ki_mode, "Enable CDR reset mechanism; the negotiation timeout is 873.8 us");

static unsigned long ki_mode_oob_delay = 10000;
module_param(ki_mode_oob_delay, ulong, 0444);
MODULE_PARM_DESC(ki_mode_oob_delay, "Set the delay(ns) for OOB");

static unsigned long ki_mode_rst_delay = 55000;
module_param(ki_mode_rst_delay, ulong, 0444);
MODULE_PARM_DESC(ki_mode_rst_delay, "Set the delay(ns) for CDR reset; the ALIGN timeout is 54.6 us)");

static int ki_dump = 0;
module_param(ki_dump, int, 0444);
MODULE_PARM_DESC(ki_dump, "Enable ki dump");

static int aeq_man_en = 1;
module_param(aeq_man_en, int, 0444);
MODULE_PARM_DESC(aeq_man_en, "Enable manual AEQ");

static unsigned long aeq_reset_delay = 5000;
module_param(aeq_reset_delay, ulong, 0444);
MODULE_PARM_DESC(aeq_reset_delay, "Set the delay(ns) for AEQ reset");

#ifdef CONFIG_AHCI_NVT_IVOT_CMB
static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ | AHCI_HFLAG_NO_FBS;
module_param_named(nvt_ahci_hflag, nvt_ahci_hflag, int, 0444);
MODULE_PARM_DESC(nvt_ahci_hflag, "nvt_ahci_hflag");
#else
static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ | AHCI_HFLAG_YES_FBS;
module_param_named(nvt_ahci_hflag, nvt_ahci_hflag, int, 0444);
MODULE_PARM_DESC(nvt_ahci_hflag, "nvt_ahci_hflag");
#endif

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	writel(val, phy_reg);
}

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 val = 0;

	val = readl(phy_reg);

	return val;
}

static void nvt_sata_phy_masked_write(u32 hi, u32 lo, u32 val, void __iomem *phy_reg)
{
	u32 mask, tmp;

	mask = GENMASK(hi, lo);
	tmp = readl(phy_reg);

	tmp &= ~(mask);
	tmp |= ((val << lo) & mask);

	writel(tmp, phy_reg);
}

const struct ata_port_info *nvt_ahci_get_port_info(
		struct platform_device *pdev, struct ahci_host_priv *hpriv)
{
	struct device *dev = &pdev->dev;
	struct nvt_ahci_plat_data *plat_data;
    struct resource *mem;
#if IS_ENABLED(CONFIG_PCI)
	u32 ep_idx;
#endif

	/* Enable/Disable FBS */
	hpriv->flags |= nvt_ahci_hflag;

	plat_data = devm_kzalloc(dev, sizeof(*plat_data), GFP_KERNEL);
	if (!plat_data)
		return &ahci_port_info;

#if IS_ENABLED(CONFIG_PCI)
	/* Get EP idx */
	if (of_property_read_u32(pdev->dev.of_node, "ep_id", &ep_idx)) {
		dev_err(&pdev->dev, "Can't get ep prop\n");
		ep_idx = CHIP_RC;
	}

	if (ep_idx > nvtpcie_get_ep_count(PCI_DEV_ID_ALL)){
		dev_info(&pdev->dev, "skip ep[%d]\n", ep_idx - CHIP_EP0);
		return &ahci_port_info;
	}

	plat_data->ep_idx = ep_idx;
#endif

	/* Get sideband base */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!mem) {
        return &ahci_port_info;
    }

#if IS_ENABLED(CONFIG_PCI)
	if (CHIP_RC != ep_idx) {
		if (nvtpcie_conv_resource(ep_idx, mem) < 0) {
			dev_err(&pdev->dev, "conv ep resource failed\n");
			return &ahci_port_info;
		}
	}
#endif

	plat_data->top_va_base= devm_ioremap(dev, mem->start, resource_size(mem));
	if (IS_ERR(plat_data->top_va_base))
		return &ahci_port_info;

	/* Get phy base */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if (!mem) {
        return &ahci_port_info;
    }

#if IS_ENABLED(CONFIG_PCI)
	if (CHIP_RC != ep_idx) {
		if (nvtpcie_conv_resource(ep_idx, mem) < 0) {
			dev_err(&pdev->dev, "conv ep resource failed\n");
			return &ahci_port_info;
		}
	}
#endif

	plat_data->phy_va_base= devm_ioremap(dev, mem->start, resource_size(mem));
	if (IS_ERR(plat_data->phy_va_base))
		return &ahci_port_info;

	hpriv->em_loc = 0;
	hpriv->em_buf_sz = 4;
	hpriv->em_msg_type = EM_MSG_TYPE_LED;
	hpriv->plat_data = plat_data;

	return &nvt_ahci_port_info;
}
EXPORT_SYMBOL(nvt_ahci_get_port_info);

/****************************
 *    Override Implement    *
 ****************************/
/* Override [sata_link_resume] */
static int nvt_ahci_hardreset(struct ata_link *link, unsigned int *class,
		      unsigned long deadline)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	struct ata_taskfile tf;
	bool online;
	int rc;

	DPRINTK("ENTER\n");

	hpriv->stop_engine(ap);

	/* clear D2H reception area to properly wait for D2H FIS */
	ata_tf_init(link->device, &tf);
	tf.command = ATA_BUSY;
	ata_tf_to_fis(&tf, 0, 0, d2h_fis);

	rc = nvt_sata_link_hardreset(link, timing, deadline, &online,
				 ahci_check_ready);

	hpriv->start_engine(ap);

	if (online)
		*class = ahci_dev_classify(ap);

	DPRINTK("EXIT, rc=%d, class=%u\n", rc, *class);
	return rc;
}

int nvt_sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			unsigned long deadline,
			bool *online, int (*check_ready)(struct ata_link *))
{
	u32 scontrol;
	int rc;
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;

	DPRINTK("ENTER\n");

	ahci_platform_disable_phys(hpriv);
	ahci_platform_enable_phys(hpriv);

	if (online)
		*online = false;

	if (nvt_sata_set_spd_needed(link)) {
		/* SATA spec says nothing about how to reconfigure
		 * spd.  To be on the safe side, turn off phy during
		 * reconfiguration.  This works for at least ICH7 AHCI
		 * and Sil3124.
		 */
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			goto out;

		scontrol = (scontrol & 0x0f0) | 0x304;

		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
			goto out;

		sata_set_spd(link);
	}

	/* issue phy wake/reset */
	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		goto out;

	scontrol = (scontrol & 0x0f0) | 0x301;

	if ((rc = sata_scr_write_flush(link, SCR_CONTROL, scontrol)))
		goto out;

	/* Couldn't find anything in SATA I/II specs, but AHCI-1.1
	 * 10.4.2 says at least 1 ms.
	 */
	ata_msleep(link->ap, 1);

	/* bring link back */
	rc = nvt_sata_link_resume(link, timing, deadline);
	if (rc)
		goto out;
	/* if link is offline nothing more to do */
	if (nvt_ata_phys_link_offline(link))
		goto out;

	/* Link is online.  From this point, -ENODEV too is an error. */
	if (online)
		*online = true;

	if (sata_pmp_supported(link->ap) && ata_is_host_link(link)) {
		/* If PMP is supported, we have to do follow-up SRST.
		 * Some PMPs don't send D2H Reg FIS after hardreset if
		 * the first port is empty.  Wait only for
		 * ATA_TMOUT_PMP_SRST_WAIT.
		 */
		if (check_ready) {
			unsigned long pmp_deadline;

			pmp_deadline = ata_deadline(jiffies,
						    ATA_TMOUT_PMP_SRST_WAIT);
			if (time_after(pmp_deadline, deadline))
				pmp_deadline = deadline;
			nvt_ata_wait_ready(link, pmp_deadline, check_ready);
		}
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	if (check_ready)
		rc = nvt_ata_wait_ready(link, deadline, check_ready);
 out:
	if (rc && rc != -EAGAIN) {
		/* online is set iff link is online && reset succeeded */
		if (online)
			*online = false;
		ata_link_err(link, "COMRESET failed (errno=%d)\n", rc);
	}
	DPRINTK("EXIT, rc=%d\n", rc);
	return rc;
}

#define KI_MEM_SIZE  1000
u32 ki_mem[2][KI_MEM_SIZE] = {{0}, {0}};
u64 wake_t = 0;

static DEFINE_SPINLOCK(link_resume_lock);
int nvt_sata_link_resume(struct ata_link *link, const unsigned long *params,
		     unsigned long deadline)
{
	int tries = ATA_LINK_RESUME_TRIES;
	u32 scontrol, serror;
	int rc;
	unsigned long flags;

	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		return rc;

	/*
	 * Writes to SControl sometimes get ignored under certain
	 * controllers (ata_piix SIDPR).  Make sure DET actually is
	 * cleared.
	 */
	do {
		if (ki_mode && ata_is_host_link(link)) {
			/* Lock for atomic */
			spin_lock_irqsave(&link_resume_lock, flags);
		}

		scontrol = (scontrol & 0x0f0) | 0x300;
		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol))) {
			if (ki_mode && ata_is_host_link(link)) {
				/* Unlock */
				spin_unlock_irqrestore(&link_resume_lock, flags);
			}
			return rc;
		}

		if (ki_mode) {
			struct ata_port *ap = link->ap;
			struct ahci_host_priv *hpriv = ap->host->private_data;
			struct nvt_ahci_plat_data *plat_data = hpriv->plat_data;
			int ki_idx = 0;
			u32 val = 0;
			u32 round = 0;
			u32 sstatus;
			u64 curr_t = 0;
			u64 rc_t = 0;
			u32 cal_count = 0;
			u32 r = 0;
			u32 c[2] = {0, 0};
			u32 rc_idx = 0;
			bool cdr_reset_done = false;
			bool aeq_reset_done = false;

			if (ata_is_host_link(link)) {
				/* The delay for OOB */
				curr_t = ktime_get_real_ns();
				rc_t = curr_t;
				writel(0x1 << 18, (hpriv->mmio + 0x130));

				while ((ktime_get_real_ns() - curr_t) < ki_mode_oob_delay) {
					if ((ktime_get_real_ns() - rc_t) >= 1000){
						rc_t = ktime_get_real_ns();
						val = readl(hpriv->mmio + 0x130);

						if (val & (0x1 << 18)) {
							wake_t = ktime_get_real_ns() - curr_t;
							break;
						}
					}
				}

				do {
					/* The delay for CDR reset */
					curr_t = ktime_get_real_ns();

					while ((ktime_get_real_ns() - curr_t) < ki_mode_rst_delay) {
						if (round != 0) {
							if (!cdr_reset_done) {
								/* CDR reset */
								nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x3fc));

								val = nvt_sata_phy_read((plat_data->phy_va_base + 0x164));
								val &= ~(0xFF);
								nvt_sata_phy_write(val,(plat_data->phy_va_base + 0x164));

								val = nvt_sata_phy_read((plat_data->phy_va_base + 0x164));
								val |= (0x80);
								nvt_sata_phy_write(val,(plat_data->phy_va_base + 0x164));

								cdr_reset_done = true;
							}

							if (aeq_man_en) {
								u64 aeq_t = ktime_get_real_ns() - curr_t;

								if (aeq_t >= aeq_reset_delay && !aeq_reset_done) {
									/* AEQ reset */
									nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x3fc));

									nvt_sata_phy_write(0x00, (plat_data->phy_va_base + 0x0a4));
									nvt_sata_phy_write(0x08, (plat_data->phy_va_base + 0x0a4));

									nvt_sata_phy_write(0x02, (plat_data->phy_va_base + 0x0a4));
									nvt_sata_phy_write(0x0a, (plat_data->phy_va_base + 0x0a4));

									aeq_reset_done = true;
								}
							}
						}

						nvt_sata_phy_write(0x02, (plat_data->phy_va_base + 0x3fc));
						val = readl(plat_data->phy_va_base + 0x088);
						if (val) {
							if (ki_idx < KI_MEM_SIZE) {
								/* Record ki variation */
								if (ki_idx == 0 || (val != ki_mem[0][ki_idx - 1]) || round != ki_mem[1][ki_idx - 1]) {
									ki_mem[0][ki_idx] = val;
									ki_mem[1][ki_idx] = round;
									ki_idx++;
								}
							}
						}
						cal_count++;
					}

					cdr_reset_done = false;
					if (aeq_man_en) {
						aeq_reset_done = false;
					}

					round++;

					if (sata_scr_read(link, SCR_STATUS, &sstatus))
						break;

					/* Check link */
					if ((sstatus & 0x103) == 0x103)
						break;
				} while (round < 15);
			}

			/* Enable ABJ */
			nvt_sata_phy_write(0x01, (plat_data->phy_va_base + 0x3fc));
			nvt_sata_phy_masked_write(5, 4, 0x0, (plat_data->phy_va_base + 0x168));

			if (ata_is_host_link(link)) {
				/* Unlock */
				spin_unlock_irqrestore(&link_resume_lock, flags);
			}

			if (ki_dump) {
				int j = 0;
				for (j = 0; j < min(ki_idx, KI_MEM_SIZE); j++) {
					ata_link_err(link, "[KI_DUMP] sata phy KI[%d]: 0x%08x, round: %u\r\n", j, ki_mem[0][j], ki_mem[1][j]);
				}
				ata_link_err(link, "[KI_DUMP] cal_count: %u, oob_delay: %llu , rst_delay: %lu\r\n", cal_count, wake_t, ki_mode_rst_delay);

				for(rc_idx = 0; rc_idx < 3; rc_idx++) {
					nvt_sata_phy_write(0x02, (plat_data->phy_va_base + 0x3fc));

					/* Read resistor value */
					r = readl(plat_data->phy_va_base + 0x01c);

					/* Read capacitance value */
					c[0] = readl(plat_data->phy_va_base + 0x004);
					c[1] = readl(plat_data->phy_va_base + 0x008);

					ata_link_err(link, "[KI_DUMP] rc_idx: %d, 0x201c: 0x%x(r[0][1]), 0x2004: 0x%x(c[0]), 0x2008: 0x%x(c[1])\r\n", rc_idx, r, c[0], c[1]);
				}
			}
		}

		/*
		 * Some PHYs react badly if SStatus is pounded
		 * immediately after resuming.  Delay 200ms before
		 * debouncing.
		 */
		if (!(link->flags & ATA_LFLAG_NO_DB_DELAY))
			ata_msleep(link->ap, 200);

		/* is SControl restored correctly? */
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			return rc;
	} while ((scontrol & 0xf0f) != 0x300 && --tries);

	if ((scontrol & 0xf0f) != 0x300) {
		ata_link_warn(link, "failed to resume link (SControl %X)\n",
			     scontrol);
		return 0;
	}

	if (tries < ATA_LINK_RESUME_TRIES)
		ata_link_warn(link, "link resume succeeded after %d retries\n",
			      ATA_LINK_RESUME_TRIES - tries);

	if ((rc = sata_link_debounce(link, params, deadline)))
		return rc;

	/* clear SError, some PHYs require this even for SRST to work */
	if (!(rc = sata_scr_read(link, SCR_ERROR, &serror)))
		rc = sata_scr_write(link, SCR_ERROR, serror);

	return rc != -EINVAL ? rc : 0;
}

int nvt_ata_std_prereset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct ata_eh_context *ehc = &link->eh_context;
	const unsigned long *timing = sata_ehc_deb_timing(ehc);
	int rc;

	/* if we're about to do hardreset, nothing more to do */
	if (ehc->i.action & ATA_EH_HARDRESET)
		return 0;

	/* if SATA, resume link */
	if (ap->flags & ATA_FLAG_SATA) {
		rc = nvt_sata_link_resume(link, timing, deadline);
		/* whine about phy resume failure but proceed */
		if (rc && rc != -EOPNOTSUPP)
			ata_link_warn(link,
				      "failed to resume link for reset (errno=%d)\n",
				      rc);
	}

	/* no point in trying softreset on offline link */
	if (nvt_ata_phys_link_offline(link))
		ehc->i.action &= ~ATA_EH_SOFTRESET;

	return 0;
}

int nvt_pmp_sata_std_hardreset(struct ata_link *link, unsigned int *class,
		       unsigned long deadline)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	bool online;
	int rc;

	/* do hardreset */
	rc = nvt_pmp_sata_link_hardreset(link, timing, deadline, &online, NULL);
	return online ? -EAGAIN : rc;
}

int nvt_pmp_sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			unsigned long deadline,
			bool *online, int (*check_ready)(struct ata_link *))
{
	u32 scontrol;
	int rc;

	DPRINTK("ENTER\n");

	if (online)
		*online = false;

	if (nvt_sata_set_spd_needed(link)) {
		/* SATA spec says nothing about how to reconfigure
		 * spd.  To be on the safe side, turn off phy during
		 * reconfiguration.  This works for at least ICH7 AHCI
		 * and Sil3124.
		 */
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			goto out;

		scontrol = (scontrol & 0x0f0) | 0x304;

		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
			goto out;

		sata_set_spd(link);
	}

	/* issue phy wake/reset */
	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		goto out;

	scontrol = (scontrol & 0x0f0) | 0x301;

	if ((rc = sata_scr_write_flush(link, SCR_CONTROL, scontrol)))
		goto out;

	/* Couldn't find anything in SATA I/II specs, but AHCI-1.1
	 * 10.4.2 says at least 1 ms.
	 */
	ata_msleep(link->ap, 1);

	/* bring link back */
	rc = sata_link_resume(link, timing, deadline);
	if (rc)
		goto out;

	/* Synopsys controller patch:
	 * Wait for second [D->H] ACK from [H->D Write(0xE8)] to avoid pmp chaotically sending a single X_RDY;
	 * this causes Synopsys controller to issue a fatal error and freeze that port.
	 */
	ata_msleep(link->ap, 1);

	/* if link is offline nothing more to do */
	if (nvt_ata_phys_link_offline(link))
		goto out;

	/* Link is online.  From this point, -ENODEV too is an error. */
	if (online)
		*online = true;

	if (sata_pmp_supported(link->ap) && ata_is_host_link(link)) {
		/* If PMP is supported, we have to do follow-up SRST.
		 * Some PMPs don't send D2H Reg FIS after hardreset if
		 * the first port is empty.  Wait only for
		 * ATA_TMOUT_PMP_SRST_WAIT.
		 */
		if (check_ready) {
			unsigned long pmp_deadline;

			pmp_deadline = ata_deadline(jiffies,
						    ATA_TMOUT_PMP_SRST_WAIT);
			if (time_after(pmp_deadline, deadline))
				pmp_deadline = deadline;
			nvt_ata_wait_ready(link, pmp_deadline, check_ready);
		}
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	if (check_ready)
		rc = nvt_ata_wait_ready(link, deadline, check_ready);
 out:
	if (rc && rc != -EAGAIN) {
		/* online is set iff link is online && reset succeeded */
		if (online)
			*online = false;
		ata_link_err(link, "COMRESET failed (errno=%d)\n", rc);
	}
	DPRINTK("EXIT, rc=%d\n", rc);
	return rc;
}

static unsigned int nvt_ahci_fill_sg(struct ata_queued_cmd *qc, void *cmd_tbl)
{
	struct scatterlist *sg;
	struct ahci_sg *ahci_sg = cmd_tbl + AHCI_CMD_TBL_HDR_SZ;
	unsigned int si;
#if IS_ENABLED(CONFIG_PCI)
	struct ata_port *ap = qc->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct nvt_ahci_plat_data *plat_data = hpriv->plat_data;
#endif

	VPRINTK("ENTER\n");

	/*
	 * Next, the S/G list.
	 */
	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		dma_addr_t addr = sg_dma_address(sg);
		u32 sg_len = sg_dma_len(sg);

#if IS_ENABLED(CONFIG_PCI)
	if (CHIP_RC != plat_data->ep_idx) {
		if (NVTPCIE_INVALID_PA == nvtpcie_get_upstream_pa(plat_data->ep_idx, addr)) {
			dev_err(ap->host->dev, "conv ep resource failed\n");
			return -ENOMEM;
		} else {
			addr = nvtpcie_get_upstream_pa(plat_data->ep_idx, addr);
		}
	}
#endif

		ahci_sg[si].addr = cpu_to_le32(addr & 0xffffffff);
		ahci_sg[si].addr_hi = cpu_to_le32((addr >> 16) >> 16);
		ahci_sg[si].flags_size = cpu_to_le32(sg_len - 1);
	}

	return si;
}

static enum ata_completion_errors nvt_ahci_qc_prep(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ahci_port_priv *pp = ap->private_data;
	int is_atapi = ata_is_atapi(qc->tf.protocol);
	void *cmd_tbl;
	u32 opts;
	const u32 cmd_fis_len = 5; /* five dwords */
	unsigned int n_elem;

	/*
	 * Fill in command table information.  First, the header,
	 * a SATA Register - Host to Device command FIS.
	 */
	cmd_tbl = pp->cmd_tbl + qc->hw_tag * AHCI_CMD_TBL_SZ;

	ata_tf_to_fis(&qc->tf, qc->dev->link->pmp, 1, cmd_tbl);
	if (is_atapi) {
		memset(cmd_tbl + AHCI_CMD_TBL_CDB, 0, 32);
		memcpy(cmd_tbl + AHCI_CMD_TBL_CDB, qc->cdb, qc->dev->cdb_len);
	}

	n_elem = 0;
	if (qc->flags & ATA_QCFLAG_DMAMAP)
		n_elem = nvt_ahci_fill_sg(qc, cmd_tbl);

	/*
	 * Fill in command slot information.
	 */
	opts = cmd_fis_len | n_elem << 16 | (qc->dev->link->pmp << 12);
	if (qc->tf.flags & ATA_TFLAG_WRITE)
		opts |= AHCI_CMD_WRITE;
	if (is_atapi)
		opts |= AHCI_CMD_ATAPI | AHCI_CMD_PREFETCH;

	ahci_fill_cmd_slot(pp, qc->hw_tag, opts);

	return AC_ERR_OK;
}

static int nvt_ahci_port_start(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct device *dev = ap->host->dev;
	struct ahci_port_priv *pp;
	void *mem;
	dma_addr_t mem_dma;
	size_t dma_sz, rx_fis_sz;
#if IS_ENABLED(CONFIG_PCI)
	struct nvt_ahci_plat_data *plat_data = hpriv->plat_data;
#endif

	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	if (ap->host->n_ports > 1) {
		pp->irq_desc = devm_kzalloc(dev, 8, GFP_KERNEL);
		if (!pp->irq_desc) {
			devm_kfree(dev, pp);
			return -ENOMEM;
		}
		snprintf(pp->irq_desc, 8,
			 "%s%d", dev_driver_string(dev), ap->port_no);
	}

	/* check FBS capability */
	if ((hpriv->cap & HOST_CAP_FBS) && sata_pmp_supported(ap)) {
		void __iomem *port_mmio = ahci_port_base(ap);
		u32 cmd = readl(port_mmio + PORT_CMD);
		if (cmd & PORT_CMD_FBSCP)
			pp->fbs_supported = true;
		else if (hpriv->flags & AHCI_HFLAG_YES_FBS) {
			dev_info(dev, "port %d can do FBS, forcing FBSCP\n",
				 ap->port_no);
			pp->fbs_supported = true;
		} else
			dev_warn(dev, "port %d is not capable of FBS\n",
				 ap->port_no);
	}

	if (pp->fbs_supported) {
		dma_sz = AHCI_PORT_PRIV_FBS_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ * 16;
	} else {
		dma_sz = AHCI_PORT_PRIV_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ;
	}

	mem = dmam_alloc_coherent(dev, dma_sz, &mem_dma, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;

#if IS_ENABLED(CONFIG_PCI)
	if (CHIP_RC != plat_data->ep_idx) {
		if (NVTPCIE_INVALID_PA == nvtpcie_get_upstream_pa(plat_data->ep_idx, (phys_addr_t) mem_dma)) {
			dev_err(ap->host->dev, "conv ep resource failed\n");
			return -ENOMEM;
		} else {
			mem_dma = nvtpcie_get_upstream_pa(plat_data->ep_idx, (phys_addr_t) mem_dma);
		}
	}
#endif

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot = mem;
	pp->cmd_slot_dma = mem_dma;

	mem += AHCI_CMD_SLOT_SZ;
	mem_dma += AHCI_CMD_SLOT_SZ;

	/*
	 * Second item: Received-FIS area
	 */
	pp->rx_fis = mem;
	pp->rx_fis_dma = mem_dma;

	mem += rx_fis_sz;
	mem_dma += rx_fis_sz;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = mem;
	pp->cmd_tbl_dma = mem_dma;

	/*
	 * Save off initial list of interrupts to be enabled.
	 * This could be changed later
	 */
	pp->intr_mask = DEF_PORT_IRQ;

	/*
	 * Switch to per-port locking in case each port has its own MSI vector.
	 */
	if (hpriv->flags & AHCI_HFLAG_MULTI_MSI) {
		spin_lock_init(&pp->lock);
		ap->lock = &pp->lock;
	}

	ap->private_data = pp;

	/* engage engines, captain */
	return ahci_port_resume(ap);
}

/**
 * ahci_platform_get_resources - Get platform resources
 * @pdev: platform device to get resources for
 * @flags: bitmap representing the resource to get
 *
 * This function allocates an ahci_host_priv struct, and gets the following
 * resources, storing a reference to them inside the returned struct:
 *
 * 1) mmio registers (IORESOURCE_MEM 0, mandatory)
 * 2) regulator for controlling the targets power (optional)
 *    regulator for controlling the AHCI controller (optional)
 * 3) 0 - AHCI_MAX_CLKS clocks, as specified in the devs devicetree node,
 *    or for non devicetree enabled platforms a single clock
 * 4) resets, if flags has AHCI_PLATFORM_GET_RESETS (optional)
 * 5) phys (optional)
 *
 * RETURNS:
 * The allocated ahci_host_priv on success, otherwise an ERR_PTR value
 */
struct ahci_host_priv *nvt_ahci_platform_get_resources(struct platform_device *pdev,
						   unsigned int flags)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	struct clk *clk;
	struct device_node *child;
	int i, enabled_ports = 0, rc = -ENOMEM, child_nodes;
	u32 mask_port_map = 0;
#if IS_ENABLED(CONFIG_PCI)
	struct resource *res;
	u32 ep_idx;
#endif

	if (!devres_open_group(dev, NULL, GFP_KERNEL))
		return ERR_PTR(-ENOMEM);

	hpriv = devres_alloc(nvt_ahci_platform_put_resources, sizeof(*hpriv),
			     GFP_KERNEL);
	if (!hpriv)
		goto err_out;

	devres_add(dev, hpriv);

#if IS_ENABLED(CONFIG_PCI)
	if (of_property_read_u32(pdev->dev.of_node, "ep_id", &ep_idx)) {
		dev_err(&pdev->dev, "Can't get ep prop\n");
		ep_idx = CHIP_RC;
	}

	if (ep_idx > nvtpcie_get_ep_count(PCI_DEV_ID_ALL)){
		dev_info(&pdev->dev, "skip ep[%d]\n", ep_idx - CHIP_EP0);
		goto err_out;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (CHIP_RC != ep_idx) {
		if (nvtpcie_conv_resource(ep_idx, res) < 0) {
			dev_err(&pdev->dev, "conv ep resource failed\n");
			goto err_out;
		}
	}

	hpriv->mmio = devm_ioremap(dev, res->start, resource_size(res));
#else
	hpriv->mmio = devm_ioremap_resource(dev,
			      platform_get_resource(pdev, IORESOURCE_MEM, 0));
#endif

	if (IS_ERR(hpriv->mmio)) {
		rc = PTR_ERR(hpriv->mmio);
		goto err_out;
	}

	for (i = 0; i < AHCI_MAX_CLKS; i++) {
		/*
		 * For now we must use clk_get(dev, NULL) for the first clock,
		 * because some platforms (da850, spear13xx) are not yet
		 * converted to use devicetree for clocks.  For new platforms
		 * this is equivalent to of_clk_get(dev->of_node, 0).
		 */
		if (i == 0)
			clk = clk_get(dev, NULL);
		else
			clk = of_clk_get(dev->of_node, i);

		if (IS_ERR(clk)) {
			rc = PTR_ERR(clk);
			if (rc == -EPROBE_DEFER)
				goto err_out;
			break;
		}
		hpriv->clks[i] = clk;
	}

	hpriv->ahci_regulator = devm_regulator_get(dev, "ahci");
	if (IS_ERR(hpriv->ahci_regulator)) {
		rc = PTR_ERR(hpriv->ahci_regulator);
		if (rc != 0)
			goto err_out;
	}

	hpriv->phy_regulator = devm_regulator_get(dev, "phy");
	if (IS_ERR(hpriv->phy_regulator)) {
		rc = PTR_ERR(hpriv->phy_regulator);
		goto err_out;
	}

	if (flags & AHCI_PLATFORM_GET_RESETS) {
		hpriv->rsts = devm_reset_control_array_get_optional_shared(dev);
		if (IS_ERR(hpriv->rsts)) {
			rc = PTR_ERR(hpriv->rsts);
			goto err_out;
		}
	}

	/*
	 * Too many sub-nodes most likely means having something wrong with
	 * the firmware.
	 */
	child_nodes = of_get_child_count(dev->of_node);
	if (child_nodes > AHCI_MAX_PORTS) {
		rc = -EINVAL;
		goto err_out;
	}

	/*
	 * If no sub-node was found, we still need to set nports to
	 * one in order to be able to use the
	 * ahci_platform_[en|dis]able_[phys|regulators] functions.
	 */
	if (child_nodes)
		hpriv->nports = child_nodes;
	else
		hpriv->nports = 1;

	hpriv->phys = devm_kcalloc(dev, hpriv->nports, sizeof(*hpriv->phys), GFP_KERNEL);
	if (!hpriv->phys) {
		rc = -ENOMEM;
		goto err_out;
	}
	/*
	 * We cannot use devm_ here, since ahci_platform_put_resources() uses
	 * target_pwrs after devm_ have freed memory
	 */
	hpriv->target_pwrs = kcalloc(hpriv->nports, sizeof(*hpriv->target_pwrs), GFP_KERNEL);
	if (!hpriv->target_pwrs) {
		rc = -ENOMEM;
		goto err_out;
	}

	if (child_nodes) {
		for_each_child_of_node(dev->of_node, child) {
			u32 port;
			struct platform_device *port_dev __maybe_unused;

			if (!of_device_is_available(child))
				continue;

			if (of_property_read_u32(child, "reg", &port)) {
				rc = -EINVAL;
				of_node_put(child);
				goto err_out;
			}

			if (port >= hpriv->nports) {
				dev_warn(dev, "invalid port number %d\n", port);
				continue;
			}
			mask_port_map |= BIT(port);

#ifdef CONFIG_OF_ADDRESS
			of_platform_device_create(child, NULL, NULL);

			port_dev = of_find_device_by_node(child);

			if (port_dev) {
				rc = nvt_ahci_platform_get_regulator(hpriv, port,
								&port_dev->dev);
				if (rc == -EPROBE_DEFER) {
					of_node_put(child);
					goto err_out;
				}
			}
#endif

			rc = nvt_ahci_platform_get_phy(hpriv, port, dev, child);
			if (rc) {
				of_node_put(child);
				goto err_out;
			}

			enabled_ports++;
		}
		if (!enabled_ports) {
			dev_warn(dev, "No port enabled\n");
			rc = -ENODEV;
			goto err_out;
		}

		if (!hpriv->mask_port_map)
			hpriv->mask_port_map = mask_port_map;
	} else {
		/*
		 * If no sub-node was found, keep this for device tree
		 * compatibility
		 */
		rc = nvt_ahci_platform_get_phy(hpriv, 0, dev, dev->of_node);
		if (rc)
			goto err_out;

		rc = nvt_ahci_platform_get_regulator(hpriv, 0, dev);
		if (rc == -EPROBE_DEFER)
			goto err_out;
	}
	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);
	hpriv->got_runtime_pm = true;

	devres_remove_group(dev, NULL);
	return hpriv;

err_out:
	devres_release_group(dev, NULL);
	return ERR_PTR(rc);
}
