
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/soc/nvt/fmem.h>
#include "ahci_nvt_ivot_util_fara.h"
#ifdef CONFIG_NVT_PCIE_LIB
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif
#include <linux/kconfig.h> //for IS_ENABLED
#include <linux/soc/nvt/nvt-pcie-lib.h> //for nvtpcie_xxx api
#include <linux/of_platform.h>
#include <linux/reset.h>
#include "libata.h"
#include <linux/pm_runtime.h>
#include "ahci_nvt_ivot_util_fara.h"
#include <linux/timekeeping.h>

static int nvt_ahci_skip_host_reset = 1;

static int ahci_asrt_delay = 30;
module_param(ahci_asrt_delay, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ahci_asrt_delay, "delay asrt for device stable");

static int nvt_sata_memmap_hiaddr_en = 0;
module_param(nvt_sata_memmap_hiaddr_en, int, 0444);
MODULE_PARM_DESC(nvt_sata_memmap_hiaddr_en, "nvt sata mem map to high addr enable");

/************************************
 *    Override for customization    *
 ************************************/
static void nvt_ahci_enable_ahci(void __iomem *mmio);

static void nvt_ahci_restore_initial_config(struct ata_host *host)
{
	struct ahci_host_priv *hpriv = host->private_data;
	void __iomem *mmio = hpriv->mmio;

	writel(hpriv->saved_cap, mmio + HOST_CAP);
	if (hpriv->saved_cap2)
		writel(hpriv->saved_cap2, mmio + HOST_CAP2);
	writel(hpriv->saved_port_map, mmio + HOST_PORTS_IMPL);
	(void) readl(mmio + HOST_PORTS_IMPL);	/* flush */
}

int nvt_ahci_reset_controller(struct ata_host *host)
{
	struct ahci_host_priv *hpriv = host->private_data;
	void __iomem *mmio = hpriv->mmio;
	u32 tmp;

	/* we must be in AHCI mode, before using anything
	 * AHCI-specific, such as HOST_RESET.
	 */
	nvt_ahci_enable_ahci(mmio);

	/* global controller reset */
	if (!nvt_ahci_skip_host_reset) {
		tmp = readl(mmio + HOST_CTL);
		if ((tmp & HOST_RESET) == 0) {
			writel(tmp | HOST_RESET, mmio + HOST_CTL);
			readl(mmio + HOST_CTL); /* flush */
		}

		/*
		 * to perform host reset, OS should set HOST_RESET
		 * and poll until this bit is read to be "0".
		 * reset must complete within 1 second, or
		 * the hardware should be considered fried.
		 */
		tmp = ata_wait_register(NULL, mmio + HOST_CTL, HOST_RESET,
					HOST_RESET, 10, 1000);

		if (tmp & HOST_RESET) {
			dev_err(host->dev, "controller reset failed (0x%x)\n",
				tmp);
			return -EIO;
		}

		/* turn on AHCI mode */
		nvt_ahci_enable_ahci(mmio);

		/* Some registers might be cleared on reset.  Restore
		 * initial values.
		 */
		if (!(hpriv->flags & AHCI_HFLAG_NO_WRITE_TO_RO))
			nvt_ahci_restore_initial_config(host);
	} else
		dev_info(host->dev, "skipping global host reset\n");

	return 0;
}

#ifdef CONFIG_PM_SLEEP
int nvt_ahci_platform_resume_host(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);
	int rc;

	if (dev->power.power_state.event == PM_EVENT_SUSPEND) {
		rc = nvt_ahci_reset_controller(host);
		if (rc)
			return rc;

		ahci_init_controller(host);
	}

	ata_host_resume(host);

	return 0;
}

int ahci_platform_resume(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);
	struct ahci_host_priv *hpriv = host->private_data;
	int rc;

	rc = ahci_platform_enable_resources(hpriv);
	if (rc)
		return rc;

	rc = ahci_platform_resume_host(dev);
	if (rc)
		goto disable_resources;

	/* We resumed so update PM runtime state */
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	return 0;

disable_resources:
	ahci_platform_disable_resources(hpriv);

	return rc;
}
#endif

void nvt_ata_eh_autopsy(struct ata_port *ap)
{
	struct ata_link *link;
	struct ahci_port_priv *pp = ap->private_data;


	ata_for_each_link(link, ap, EDGE)
	ata_eh_link_autopsy(link);

	/* Handle the frigging slave link.  Autopsy is done similarly
	 * but actions and flags are transferred over to the master
	 * link and handled from there.
	 */
	if (ap->slave_link) {
		struct ata_eh_context *mehc = &ap->link.eh_context;
		struct ata_eh_context *sehc = &ap->slave_link->eh_context;

		/* transfer control flags from master to slave */
		sehc->i.flags |= mehc->i.flags & ATA_EHI_TO_SLAVE_MASK;

		/* perform autopsy on the slave link */
		ata_eh_link_autopsy(ap->slave_link);

		/* transfer actions from slave to master and clear slave */
		ata_eh_about_to_do(ap->slave_link, NULL, ATA_EH_ALL_ACTIONS);
		mehc->i.action		|= sehc->i.action;
		mehc->i.dev_action[1]	|= sehc->i.dev_action[1];
		mehc->i.flags		|= sehc->i.flags;
		ata_eh_done(ap->slave_link, NULL, ATA_EH_ALL_ACTIONS);
	}

	/* Autopsy of fanout ports can affect host link autopsy.
	 * Perform host link autopsy last.
	 */
	if (sata_pmp_attached(ap)) {
		if (!pp->fbs_enabled) {
			ata_eh_link_autopsy(&ap->link);
		}
	}
}

void nvt_sata_pmp_error_handler(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	if (plat_data->ifs_occurred == false) {
		if ((((0x1 << (plat_data->sata_id - 1)) & plat_data->fbsen_map) > 0)) {
			if (ap->ops->pmp_detach) {
				ap->ops->pmp_detach(ap);
			}
		}
	}
#endif
	nvt_ata_eh_autopsy(ap);
	ata_eh_report(ap);
	sata_pmp_eh_recover(ap);
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	if ((sata_pmp_attached(ap) == true) && (((0x1 << (plat_data->sata_id - 1)) & plat_data->fbsen_map) == 0)) {
		if (ap->ops->pmp_attach) {
			ap->ops->pmp_attach(ap);
		}
	}

	plat_data->ifs_occurred = false;
#endif
	plat_data->escape_occurred = false;

	ata_eh_finish(ap);
}

void nvt_ahci_error_handler(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;

	if (!(ap->pflags & ATA_PFLAG_FROZEN)) {
		/* restart engine */
		hpriv->stop_engine(ap);
		hpriv->start_engine(ap);
	}

	nvt_sata_pmp_error_handler(ap);

	if (!ata_dev_enabled(ap->link.device))
		hpriv->stop_engine(ap);
}

static void ahci_platform_put_resources(struct device *dev, void *res)
{
	struct ahci_host_priv *hpriv = res;
	int c;

	if (hpriv->got_runtime_pm) {
		pm_runtime_put_sync(dev);
		pm_runtime_disable(dev);
	}

	for (c = 0; c < AHCI_MAX_CLKS && hpriv->clks[c]; c++)
		clk_put(hpriv->clks[c]);
	/*
	 * The regulators are tied to child node device and not to the
	 * SATA device itself. So we can't use devm for automatically
	 * releasing them. We have to do it manually here.
	 */
	for (c = 0; c < hpriv->nports; c++)
		if (hpriv->target_pwrs && hpriv->target_pwrs[c])
			regulator_put(hpriv->target_pwrs[c]);

	kfree(hpriv->target_pwrs);
}

static int ahci_platform_get_regulator(struct ahci_host_priv *hpriv, u32 port,
				       struct device *dev)
{
	struct regulator *target_pwr;
	int rc = 0;

	target_pwr = regulator_get_optional(dev, "target");

	if (!IS_ERR(target_pwr))
		hpriv->target_pwrs[port] = target_pwr;
	else
		rc = PTR_ERR(target_pwr);

	return rc;
}

static int ahci_platform_get_phy(struct ahci_host_priv *hpriv, u32 port,
				 struct device *dev, struct device_node *node)
{
	int rc;

	hpriv->phys[port] = devm_of_phy_get(dev, node, NULL);

	if (!IS_ERR(hpriv->phys[port]))
		return 0;

	rc = PTR_ERR(hpriv->phys[port]);
	switch (rc) {
	case -ENOSYS:
		/* No PHY support. Check if PHY is required. */
		if (of_find_property(node, "phys", NULL)) {
			dev_err(dev,
				"couldn't get PHY in node %s: ENOSYS\n",
				node->name);
			break;
		}
	/* fall through */
	case -ENODEV:
		/* continue normally */
		hpriv->phys[port] = NULL;
		rc = 0;
		break;
	case -EPROBE_DEFER:
		/* Do not complain yet */
		break;

	default:
		dev_err(dev,
			"couldn't get PHY in node %s: %d\n",
			node->name, rc);

		break;
	}

	return rc;
}

#if IS_ENABLED(CONFIG_PLAT_NOVATEK)
static void __iomem *nvt_ahci_devm_ioremap(struct platform_device *pdev,
		unsigned int type, unsigned int num)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	u32 u32prop;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL)
		return IOMEM_ERR_PTR(-EINVAL);

	if (!nvtpcie_is_common_pci()) {
		return devm_ioremap_resource(dev, res);
	}

	//the following is common pci
	if (0 == of_property_read_u32(dev->of_node, "chip_id", &u32prop)) {
		if (0 != nvtpcie_conv_resource((nvtpcie_chipid_t)u32prop, res)) {
			dev_err(dev, "conv res failed %pR\n", res);
			return IOMEM_ERR_PTR(-EINVAL);
		}
	}

	return devm_ioremap(dev, res->start, resource_size(res));
}
#endif

struct ahci_host_priv *nvt_ahci_platform_get_resources(struct platform_device *pdev,
		unsigned int flags)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	struct clk *clk;
	struct device_node *child;
	int i, enabled_ports = 0, rc = -ENOMEM, child_nodes;
	u32 mask_port_map = 0;

	if (!devres_open_group(dev, NULL, GFP_KERNEL))
		return ERR_PTR(-ENOMEM);

	hpriv = devres_alloc(ahci_platform_put_resources, sizeof(*hpriv),
			     GFP_KERNEL);
	if (!hpriv)
		goto err_out;

	devres_add(dev, hpriv);

#if IS_ENABLED(CONFIG_PLAT_NOVATEK)
	hpriv->mmio = nvt_ahci_devm_ioremap(pdev, IORESOURCE_MEM, 0);
#else
	hpriv->mmio = devm_ioremap_resource(dev,
					    platform_get_resource(pdev, IORESOURCE_MEM, 0));
#endif
	if (IS_ERR(hpriv->mmio)) {
		dev_err(dev, "no mmio space\n");
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

	if (flags & AHCI_PLATFORM_GET_RESETS) {
		hpriv->rsts = devm_reset_control_array_get_optional_shared(dev);
		if (IS_ERR(hpriv->rsts)) {
			rc = PTR_ERR(hpriv->rsts);
			goto err_out;
		}
	}

	hpriv->nports = child_nodes = of_get_child_count(dev->of_node);

	/*
	 * If no sub-node was found, we still need to set nports to
	 * one in order to be able to use the
	 * ahci_platform_[en|dis]able_[phys|regulators] functions.
	 */
	if (!child_nodes)
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
				rc = ahci_platform_get_regulator(hpriv, port,
								 &port_dev->dev);
				if (rc == -EPROBE_DEFER)
					goto err_out;
			}
#endif

			rc = ahci_platform_get_phy(hpriv, port, dev, child);
			if (rc)
				goto err_out;

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
		rc = ahci_platform_get_phy(hpriv, 0, dev, dev->of_node);
		if (rc)
			goto err_out;

		rc = ahci_platform_get_regulator(hpriv, 0, dev);
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

static unsigned int nvt_ahci_fill_sg(struct ata_queued_cmd *qc, void *cmd_tbl)
{
	struct scatterlist *sg;
	struct ahci_sg *ahci_sg = cmd_tbl + AHCI_CMD_TBL_HDR_SZ;
	unsigned int si;

	VPRINTK("ENTER\n");

	/*
	 * Next, the S/G list.
	 */
	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		dma_addr_t addr = sg_dma_address(sg);
		u32 sg_len = sg_dma_len(sg);

		ahci_sg[si].addr = cpu_to_le32(addr & 0xffffffff);
		ahci_sg[si].addr_hi = cpu_to_le32((addr >> 16) >> 16);
		ahci_sg[si].flags_size = cpu_to_le32(sg_len - 1);
	}

	return si;
}

enum ata_completion_errors nvt_ahci_qc_prep(struct ata_queued_cmd *qc)
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

static void nvt_ahci_fbs_dec_intr(struct ata_port *ap)
{
	struct ahci_port_priv *pp = ap->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 fbs = readl(port_mmio + PORT_FBS);
	int retries = 3;

	DPRINTK("ENTER\n");
	BUG_ON(!pp->fbs_enabled);

	/* time to wait for DEC is not specified by AHCI spec,
	 * add a retry loop for safety.
	 */
	writel(fbs | PORT_FBS_DEC, port_mmio + PORT_FBS);
	fbs = readl(port_mmio + PORT_FBS);
	while ((fbs & PORT_FBS_DEC) && retries--) {
		udelay(1);
		fbs = readl(port_mmio + PORT_FBS);
	}

	if (fbs & PORT_FBS_DEC)
		dev_err(ap->host->dev, "failed to clear device error\n");
}

static void nvt_ahci_error_intr(struct ata_port *ap, u32 irq_stat)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	struct ata_eh_info *host_ehi = &ap->link.eh_info;
	struct ata_link *link = NULL;
	struct ata_queued_cmd *active_qc;
	struct ata_eh_info *active_ehi;
	bool fbs_need_dec = false;
	u32 serror;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	plat_data->ifs_occurred = false;
#endif

	/* determine active link with error */
	if (pp->fbs_enabled) {
		void __iomem *port_mmio = ahci_port_base(ap);
		u32 fbs = readl(port_mmio + PORT_FBS);
		int pmp = fbs >> PORT_FBS_DWE_OFFSET;

		if ((fbs & PORT_FBS_SDE) && (pmp < ap->nr_pmp_links)) {
			link = &ap->pmp_link[pmp];
			fbs_need_dec = true;
		}

	} else
		ata_for_each_link(link, ap, EDGE)
		if (ata_link_active(link))
			break;

	if (!link)
		link = &ap->link;

	active_qc = ata_qc_from_tag(ap, link->active_tag);
	active_ehi = &link->eh_info;

	/* record irq stat */
	ata_ehi_clear_desc(host_ehi);
	ata_ehi_push_desc(host_ehi, "irq_stat 0x%08x", irq_stat);

	/* AHCI needs SError cleared; otherwise, it might lock up */
	ap->ops->scr_read(&ap->link, SCR_ERROR, &serror);
	//ahci_scr_read(&ap->link, SCR_ERROR, &serror);
	ap->ops->scr_write(&ap->link, SCR_ERROR, serror);
	//ahci_scr_write(&ap->link, SCR_ERROR, serror);
	host_ehi->serror |= serror;

	/* some controllers set IRQ_IF_ERR on device errors, ignore it */
	if (hpriv->flags & AHCI_HFLAG_IGN_IRQ_IF_ERR)
		irq_stat &= ~PORT_IRQ_IF_ERR;

	if (irq_stat & PORT_IRQ_TF_ERR) {
		/* If qc is active, charge it; otherwise, the active
		 * link.  There's no active qc on NCQ errors.  It will
		 * be determined by EH by reading log page 10h.
		 */
		if (active_qc)
			active_qc->err_mask |= AC_ERR_DEV;
		else
			active_ehi->err_mask |= AC_ERR_DEV;

		if (hpriv->flags & AHCI_HFLAG_IGN_SERR_INTERNAL)
			host_ehi->serror &= ~SERR_INTERNAL;
	}

	if (irq_stat & PORT_IRQ_UNK_FIS) {
		u32 *unk = pp->rx_fis + RX_FIS_UNK;

		active_ehi->err_mask |= AC_ERR_HSM;
		active_ehi->action |= ATA_EH_RESET;
		ata_ehi_push_desc(active_ehi,
				  "unknown FIS %08x %08x %08x %08x",
				  unk[0], unk[1], unk[2], unk[3]);
	}

	if (sata_pmp_attached(ap) && (irq_stat & PORT_IRQ_BAD_PMP)) {
		active_ehi->err_mask |= AC_ERR_HSM;
		active_ehi->action |= ATA_EH_RESET;
		ata_ehi_push_desc(active_ehi, "incorrect PMP");
	}

	if (irq_stat & (PORT_IRQ_HBUS_ERR | PORT_IRQ_HBUS_DATA_ERR)) {
		host_ehi->err_mask |= AC_ERR_HOST_BUS;
		host_ehi->action |= ATA_EH_RESET;
		ata_ehi_push_desc(host_ehi, "host bus error");
	}

	if (irq_stat & PORT_IRQ_IF_ERR) {
		if (fbs_need_dec)
			active_ehi->err_mask |= AC_ERR_DEV;
		else {
			host_ehi->err_mask |= AC_ERR_ATA_BUS;
			host_ehi->action |= ATA_EH_RESET;
		}

		ata_ehi_push_desc(host_ehi, "interface fatal error");
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		plat_data->ifs_occurred = true;
#endif
	}

	if (irq_stat & (PORT_IRQ_CONNECT | PORT_IRQ_PHYRDY)) {
		ata_ehi_hotplugged(host_ehi);
		ata_ehi_push_desc(host_ehi, "%s",
				  irq_stat & PORT_IRQ_CONNECT ?
				  "connection status changed" : "PHY RDY changed");
	}

	/* okay, let's hand over to EH */

	if (irq_stat & PORT_IRQ_FREEZE)
		ata_port_freeze(ap);
	else if (fbs_need_dec) {
		ata_link_abort(link);
		nvt_ahci_fbs_dec_intr(ap);
	} else
		ata_port_abort(ap);
}

static void nvt_ahci_handle_port_interrupt(struct ata_port *ap,
		void __iomem *port_mmio, u32 status)
{
	struct ata_eh_info *ehi = &ap->link.eh_info;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	int resetting = !!(ap->pflags & ATA_PFLAG_RESETTING);
	u32 qc_active = 0;
	int rc;

	/* ignore BAD_PMP while resetting */
	if (unlikely(resetting))
		status &= ~PORT_IRQ_BAD_PMP;

	if (sata_lpm_ignore_phy_events(&ap->link)) {
		status &= ~PORT_IRQ_PHYRDY;
		ap->ops->scr_write(&ap->link, SCR_ERROR, SERR_PHYRDY_CHG);
		//ahci_scr_write(&ap->link, SCR_ERROR, SERR_PHYRDY_CHG);
	}

	if (unlikely(status & PORT_IRQ_ERROR)) {
		nvt_ahci_error_intr(ap, status);
		return;
	}

	if (status & PORT_IRQ_SDB_FIS) {
		/* If SNotification is available, leave notification
		 * handling to sata_async_notification().  If not,
		 * emulate it by snooping SDB FIS RX area.
		 *
		 * Snooping FIS RX area is probably cheaper than
		 * poking SNotification but some constrollers which
		 * implement SNotification, ICH9 for example, don't
		 * store AN SDB FIS into receive area.
		 */
		if (hpriv->cap & HOST_CAP_SNTF) {
				sata_async_notification(ap);
		} else {
			/* If the 'N' bit in word 0 of the FIS is set,
			 * we just received asynchronous notification.
			 * Tell libata about it.
			 *
			 * Lack of SNotification should not appear in
			 * ahci 1.2, so the workaround is unnecessary
			 * when FBS is enabled.
			 */
			if (pp->fbs_enabled)
				WARN_ON_ONCE(1);
			else {
				const __le32 *f = pp->rx_fis + RX_FIS_SDB;
				u32 f0 = le32_to_cpu(f[0]);
				if (f0 & (1 << 15))
					sata_async_notification(ap);
			}
		}
	}

	/* pp->active_link is not reliable once FBS is enabled, both
	 * PORT_SCR_ACT and PORT_CMD_ISSUE should be checked because
	 * NCQ and non-NCQ commands may be in flight at the same time.
	 */
	if (pp->fbs_enabled) {
		if (ap->qc_active) {
			qc_active = readl(port_mmio + PORT_SCR_ACT);
			qc_active |= readl(port_mmio + PORT_CMD_ISSUE);
		}
	} else {
		/* pp->active_link is valid iff any command is in flight */
		if (ap->qc_active && pp->active_link->sactive)
			qc_active = readl(port_mmio + PORT_SCR_ACT);
		else
			qc_active = readl(port_mmio + PORT_CMD_ISSUE);
	}


	rc = ata_qc_complete_multiple(ap, qc_active);

	/* while resetting, invalid completions are expected */
	if (unlikely(rc < 0 && !resetting)) {
		ehi->err_mask |= AC_ERR_HSM;
		ehi->action |= ATA_EH_RESET;
		ata_port_freeze(ap);
	}
}

static void nvt_ahci_port_intr(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 status;
#ifdef CONFIG_PLAT_NOVATEK
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
#endif

	status = readl(port_mmio + PORT_IRQ_STAT);

	writel(status, port_mmio + PORT_IRQ_STAT);

#ifdef CONFIG_PLAT_NOVATEK
	if ((*plat_data->sata_tune_en != 0) && (*plat_data->nvt_sata_scope_gpio < 0x80) && ((status & ((u32) *plat_data->nvt_sata_scope_event)) != 0x0)) {
		gpio_direction_output(((u32) *plat_data->nvt_sata_scope_gpio), 0);
	}
#endif

	nvt_ahci_handle_port_interrupt(ap, port_mmio, status);

#ifdef CONFIG_PLAT_NOVATEK
	if ((*plat_data->sata_tune_en != 0) && (*plat_data->nvt_sata_scope_gpio < 0x80) && ((status & ((u32) *plat_data->nvt_sata_scope_event)) != 0x0)) {
		gpio_direction_output(((u32) *plat_data->nvt_sata_scope_gpio), 1);
	}
#endif
}

u32 nvt_ahci_handle_port_intr(struct ata_host *host, u32 irq_masked)
{
	unsigned int i, handled = 0;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap;

		if (!(irq_masked & (1 << i)))
			continue;

		ap = host->ports[i];
		if (ap) {
			nvt_ahci_port_intr(ap);
			VPRINTK("port %u\n", i);
		} else {
			VPRINTK("port %u (no irq)\n", i);
			if (ata_ratelimit())
				dev_warn(host->dev,
					 "interrupt on disabled port %u\n", i);
		}

		handled = 1;
	}

	return handled;
}

static irqreturn_t nvt_ahci_single_level_irq_intr(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	struct ahci_host_priv *hpriv;
	unsigned int rc = 0;
	void __iomem *mmio;
	u32 irq_stat, irq_masked;

	VPRINTK("ENTER\n");

	hpriv = host->private_data;
	mmio = hpriv->mmio;

	/* sigh.  0xffffffff is a valid return from h/w */
	irq_stat = readl(mmio + HOST_IRQ_STAT);
	if (!irq_stat)
		return IRQ_NONE;

	irq_masked = irq_stat & hpriv->port_map;

	spin_lock(&host->lock);

	rc = nvt_ahci_handle_port_intr(host, irq_masked);

	/* HOST_IRQ_STAT behaves as level triggered latch meaning that
	 * it should be cleared after all the port events are cleared;
	 * otherwise, it will raise a spurious interrupt after each
	 * valid one.  Please read section 10.6.2 of ahci 1.1 for more
	 * information.
	 *
	 * Also, use the unmasked value to clear interrupt as spurious
	 * pending event on a dummy port might cause screaming IRQ.
	 */
	writel(irq_stat, mmio + HOST_IRQ_STAT);

	spin_unlock(&host->lock);

	VPRINTK("EXIT\n");

	return IRQ_RETVAL(rc);
}

static void nvt_ahci_sw_activity_blink(struct timer_list *t)
{
	struct ahci_em_priv *emp = from_timer(emp, t, timer);
	struct ata_link *link = emp->link;
	struct ata_port *ap = link->ap;

	unsigned long led_message = emp->led_state;
	u32 activity_led_state;
	unsigned long flags;

	led_message &= EM_MSG_LED_VALUE;
	led_message |= ap->port_no | (link->pmp << 8);

	/* check to see if we've had activity.  If so,
	 * toggle state of LED and reset timer.  If not,
	 * turn LED to desired idle state.
	 */
	spin_lock_irqsave(ap->lock, flags);
	if (emp->saved_activity != emp->activity) {
		emp->saved_activity = emp->activity;
		/* get the current LED state */
		activity_led_state = led_message & EM_MSG_LED_VALUE_ON;

		if (activity_led_state)
			activity_led_state = 0;
		else
			activity_led_state = 1;

		/* clear old state */
		led_message &= ~EM_MSG_LED_VALUE_ACTIVITY;

		/* toggle state */
		led_message |= (activity_led_state << 16);
		mod_timer(&emp->timer, jiffies + msecs_to_jiffies(100));
	} else {
		/* switch to idle */
		led_message &= ~EM_MSG_LED_VALUE_ACTIVITY;
		if (emp->blink_policy == BLINK_OFF)
			led_message |= (1 << 16);
	}
	spin_unlock_irqrestore(ap->lock, flags);
	ap->ops->transmit_led_message(ap, led_message, 4);
}

static void nvt_ahci_init_sw_activity(struct ata_link *link)
{
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_em_priv *emp = &pp->em_priv[link->pmp];

	/* init activity stats, setup timer */
	emp->saved_activity = emp->activity = 0;
	emp->link = link;
	timer_setup(&emp->timer, nvt_ahci_sw_activity_blink, 0);

	/* check our blink policy and set flag for link if it's enabled */
	if (emp->blink_policy)
		link->flags |= ATA_LFLAG_SW_ACTIVITY;
}

static int nvt_ahci_rpm_get_port(struct ata_port *ap)
{
	return pm_runtime_get_sync(ap->dev);
}

static void nvt_ahci_power_up(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 cmd;

	cmd = readl(port_mmio + PORT_CMD) & ~PORT_CMD_ICC_MASK;

	/* spin up device */
	if (hpriv->cap & HOST_CAP_SSS) {
		cmd |= PORT_CMD_SPIN_UP;
		writel(cmd, port_mmio + PORT_CMD);
	}

	/* wake up link */
	writel(cmd | PORT_CMD_ICC_ACTIVE, port_mmio + PORT_CMD);
}

void nvt_ahci_start_fis_rx(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ahci_port_priv *pp = ap->private_data;
	u32 tmp;

#ifndef CONFIG_PLAT_NOVATEK
	struct ahci_host_priv *hpriv = ap->host->private_data;

	/* set FIS registers */
	if (hpriv->cap & HOST_CAP_64)
		writel((pp->cmd_slot_dma >> 16) >> 16,
		       port_mmio + PORT_LST_ADDR_HI);
#endif
	writel(pp->cmd_slot_dma & 0xffffffff, port_mmio + PORT_LST_ADDR);

#ifndef CONFIG_PLAT_NOVATEK
	if (hpriv->cap & HOST_CAP_64)
		writel((pp->rx_fis_dma >> 16) >> 16,
		       port_mmio + PORT_FIS_ADDR_HI);
#endif
	writel(pp->rx_fis_dma & 0xffffffff, port_mmio + PORT_FIS_ADDR);

	/* enable FIS reception */
	tmp = readl(port_mmio + PORT_CMD);
	tmp |= PORT_CMD_FIS_RX;
	writel(tmp, port_mmio + PORT_CMD);

	/* flush */
	readl(port_mmio + PORT_CMD);
}

static int nvt_ahci_check_axi_bus(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u32 val;
	u32 val_2;
	int wait_tout = 1000;

	/* select internal FSM */
	val = readl(hpriv->mmio + 0x174);
	val &= ~(0x1 << 31);
	val |= (0x1 << 31);
	writel(val, hpriv->mmio + 0x174);

	do {
		/* check DMA idle */
		val = readl(hpriv->mmio + 0x17C);
		val_2 = readl(hpriv->mmio + 0x15C);
		if (!(val & (0x7 << 24))) {
			break;
		}

		msleep(1);
		wait_tout--;
	} while (wait_tout > 0);

	ata_link_dbg(&ap->link, "0x17C val: 0x%x, 0x15C val: 0x%x\r\n", val, val_2);

	if (wait_tout <= 0) {
		ata_link_dbg(&ap->link, "SATA wait for DMA idle timeout\r\n");
	}

	return 0;
}

static int nvt_ahci_stop_fis_rx(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 tmp;

	/* disable FIS reception */
	tmp = readl(port_mmio + PORT_CMD);
	tmp &= ~PORT_CMD_FIS_RX;
	writel(tmp, port_mmio + PORT_CMD);

	/* ensure the response of apb */
	msleep(10);

	/* wait for completion, spec says 500ms, give it 1000 */
	tmp = ata_wait_register(ap, port_mmio + PORT_CMD, PORT_CMD_FIS_ON,
				PORT_CMD_FIS_ON, 10, 1000);
	if (tmp & PORT_CMD_FIS_ON) {
		u32 val_fsm, val_evt, val_pxcmd;

		/* select internal FSM */
		val_fsm = readl(port_mmio + 0x74);
		val_fsm &= ~(0x1 << 31);
		val_fsm |= (0x1 << 31);
		writel(val_fsm, port_mmio + 0x74);

		/* check FSM */
		val_fsm = readl(port_mmio + 0x7C);
		val_evt = readl(port_mmio + 0x5C);
		val_pxcmd = readl(port_mmio + PORT_CMD);

		dev_err(ap->host->dev, "%s: FR is still running, it may hang! [0x17c(%x), 0x15c(%x), 0x118(%x)]\n", __func__, val_fsm, val_evt, val_pxcmd);
		return -EBUSY;
	}

	/* PORT_CMD_FIS_ON doesn't guarantee data transfer is complete, we should check dma idle either */
	nvt_ahci_check_axi_bus(ap);

	return 0;
}

int nvt_ahci_stop_engine(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u32 tmp;

	/*
	 * On some controllers, stopping a port's DMA engine while the port
	 * is in ALPM state (partial or slumber) results in failures on
	 * subsequent DMA engine starts.  For those controllers, put the
	 * port back in active state before stopping its DMA engine.
	 */
	if ((hpriv->flags & AHCI_HFLAG_WAKE_BEFORE_STOP) &&
	    (ap->link.lpm_policy > ATA_LPM_MAX_POWER) &&
	    ap->ops->set_lpm(&ap->link, ATA_LPM_MAX_POWER, ATA_LPM_WAKE_ONLY)) {
		dev_err(ap->host->dev, "Failed to wake up port before engine stop\n");
		return -EIO;
	}

	tmp = readl(port_mmio + PORT_CMD);

	/* check if the HBA is idle */
	if ((tmp & (PORT_CMD_START | PORT_CMD_LIST_ON)) == 0)
		return 0;

	/*
	 * Don't try to issue commands but return with ENODEV if the
	 * AHCI controller not available anymore (e.g. due to PCIe hot
	 * unplugging). Otherwise a 500ms delay for each port is added.
	 */
	if (tmp == 0xffffffff) {
		dev_err(ap->host->dev, "AHCI controller unavailable!\n");
		return -ENODEV;
	}

	/* disable FIS_RX */
	nvt_ahci_stop_fis_rx(ap);

	/* setting HBA to idle */
	tmp = readl(port_mmio + PORT_CMD);
	tmp &= ~PORT_CMD_START;
	writel(tmp, port_mmio + PORT_CMD);

	/* wait for engine to stop. This could be as long as 500 msec */
	tmp = ata_wait_register(ap, port_mmio + PORT_CMD,
				PORT_CMD_LIST_ON, PORT_CMD_LIST_ON, 1, 500);
	if (tmp & PORT_CMD_LIST_ON)
		return -EIO;

	/* wait for packets to be drained */
	msleep(10);

	/* wait for completion, spec says 500ms, give it 1000 */
	tmp = ata_wait_register(ap, port_mmio + PORT_CMD, PORT_CMD_FIS_ON,
				PORT_CMD_FIS_ON, 10, 1000);
	if (tmp & PORT_CMD_FIS_ON) {
		u32 val_fsm, val_evt, val_pxcmd;

		/* select internal FSM */
		val_fsm = readl(port_mmio + 0x74);
		val_fsm &= ~(0x1 << 31);
		val_fsm |= (0x1 << 31);
		writel(val_fsm, port_mmio + 0x74);

		/* check FSM */
		val_fsm = readl(port_mmio + 0x7C);
		val_evt = readl(port_mmio + 0x5C);
		val_pxcmd = readl(port_mmio + PORT_CMD);

		dev_err(ap->host->dev, "%s: FR is still running, it may hang! [0x17c(%x), 0x15c(%x), 0x118(%x)]\n", __func__, val_fsm, val_evt, val_pxcmd);
	}

	/* enable FIS_RX */
	nvt_ahci_start_fis_rx(ap);

	return 0;
}

static void nvt_ahci_start_port(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	struct ata_link *link;
	struct ahci_em_priv *emp;
	ssize_t rc;
	int i;

	/* enable FIS reception */
	nvt_ahci_start_fis_rx(ap);

	/* enable DMA */
	if (!(hpriv->flags & AHCI_HFLAG_DELAY_ENGINE))
		hpriv->start_engine(ap);

	/* turn on LEDs */
	if (ap->flags & ATA_FLAG_EM) {
		ata_for_each_link(link, ap, EDGE) {
			emp = &pp->em_priv[link->pmp];

			/* EM Transmit bit maybe busy during init */
			for (i = 0; i < EM_MAX_RETRY; i++) {
				rc = ap->ops->transmit_led_message(ap,
								   emp->led_state,
								   4);
				/*
				 * If busy, give a breather but do not
				 * release EH ownership by using msleep()
				 * instead of ata_msleep().  EM Transmit
				 * bit is busy for the whole host and
				 * releasing ownership will cause other
				 * ports to fail the same way.
				 */
				if (rc == -EBUSY)
					msleep(1);
				else
					break;
			}
		}
	}

	if (ap->flags & ATA_FLAG_SW_ACTIVITY)
		ata_for_each_link(link, ap, EDGE)
		nvt_ahci_init_sw_activity(link);

}
static void nvt_ahci_enable_fbs(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 fbs;
	int rc;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
#endif

	if (!pp->fbs_supported)
		return;

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	if (!plat_data->use_fbs)
		return;
#endif

	fbs = readl(port_mmio + PORT_FBS);
	if (fbs & PORT_FBS_EN) {
		pp->fbs_enabled = true;
		pp->fbs_last_dev = -1; /* initialization */
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		plat_data->fbsen_map |= (0x1 << (plat_data->sata_id - 1));
#endif
		return;
	}

	rc = hpriv->stop_engine(ap);
	if (rc)
		return;

	writel(fbs | PORT_FBS_EN, port_mmio + PORT_FBS);
	fbs = readl(port_mmio + PORT_FBS);
	if (fbs & PORT_FBS_EN) {
		dev_info(ap->host->dev, "FBS is enabled\n");
		pp->fbs_enabled = true;
		pp->fbs_last_dev = -1; /* initialization */
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		plat_data->fbsen_map |= (0x1 << (plat_data->sata_id - 1));
#endif
	} else
		dev_err(ap->host->dev, "Failed to enable FBS\n");

	hpriv->start_engine(ap);
}

static void nvt_ahci_disable_fbs(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u32 fbs;
	int rc;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
#endif

	if (!pp->fbs_supported)
		return;

#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	if (!plat_data->use_fbs)
		return;
#endif

	fbs = readl(port_mmio + PORT_FBS);
	if (fbs & PORT_FBS_DEV_MASK)
		fbs &= ~PORT_FBS_DEV_MASK;		//PxFBS.DEV bits are still referenced even if PxFBS.EN=0. Clear it to make sure FBS-off works.
	if ((fbs & PORT_FBS_EN) == 0) {
		pp->fbs_enabled = false;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		plat_data->fbsen_map &= ~(0x1 << (plat_data->sata_id - 1));
#endif
		return;
	}

	rc = hpriv->stop_engine(ap);
	if (rc)
		return;

	writel(fbs & ~PORT_FBS_EN, port_mmio + PORT_FBS);
	fbs = readl(port_mmio + PORT_FBS);
	if (fbs & PORT_FBS_EN)
		dev_err(ap->host->dev, "Failed to disable FBS\n");
	else {
		dev_info(ap->host->dev, "FBS is disabled\n");
		pp->fbs_enabled = false;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
		plat_data->fbsen_map &= ~(0x1 << (plat_data->sata_id - 1));
#endif
	}

	hpriv->start_engine(ap);
}

void nvt_ahci_pmp_attach(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ahci_port_priv *pp = ap->private_data;
	u32 cmd;

	cmd = readl(port_mmio + PORT_CMD);
	cmd |= PORT_CMD_PMP;
	writel(cmd, port_mmio + PORT_CMD);

	nvt_ahci_enable_fbs(ap);

	pp->intr_mask |= PORT_IRQ_BAD_PMP;

	/*
	 * We must not change the port interrupt mask register if the
	 * port is marked frozen, the value in pp->intr_mask will be
	 * restored later when the port is thawed.
	 *
	 * Note that during initialization, the port is marked as
	 * frozen since the irq handler is not yet registered.
	 */
	if (!(ap->pflags & ATA_PFLAG_FROZEN))
		writel(pp->intr_mask, port_mmio + PORT_IRQ_MASK);
}

void nvt_ahci_pmp_detach(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ahci_port_priv *pp = ap->private_data;
	u32 cmd;
#ifdef CONFIG_AHCI_NVT_IVOT_FBS
	ahci_kick_engine(ap);
#endif
	nvt_ahci_disable_fbs(ap);

	cmd = readl(port_mmio + PORT_CMD);
	cmd &= ~PORT_CMD_PMP;
	writel(cmd, port_mmio + PORT_CMD);

	pp->intr_mask &= ~PORT_IRQ_BAD_PMP;

	/* see comment above in ahci_pmp_attach() */
	if (!(ap->pflags & ATA_PFLAG_FROZEN))
		writel(pp->intr_mask, port_mmio + PORT_IRQ_MASK);
}

int nvt_ahci_port_resume(struct ata_port *ap)
{
	nvt_ahci_rpm_get_port(ap);

	nvt_ahci_power_up(ap);
	nvt_ahci_start_port(ap);

	if (sata_pmp_attached(ap))
		nvt_ahci_pmp_attach(ap);
	else
		nvt_ahci_pmp_detach(ap);

	return 0;
}

int nvt_ahci_port_start(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct device *dev = ap->host->dev;
	struct ahci_port_priv *pp;
	void *mem;
	dma_addr_t mem_dma;
	size_t dma_sz, rx_fis_sz;

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

	memset(mem, 0, dma_sz);

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
	return nvt_ahci_port_resume(ap);
}

static void nvt_ahci_enable_ahci(void __iomem *mmio)
{
	int i;
	u32 tmp;

	/* turn on AHCI_EN */
	tmp = readl(mmio + HOST_CTL);
	if (tmp & HOST_AHCI_EN)
		return;

	/* Some controllers need AHCI_EN to be written multiple times.
	 * Try a few times before giving up.
	 */
	for (i = 0; i < 5; i++) {
		tmp |= HOST_AHCI_EN;
		writel(tmp, mmio + HOST_CTL);
		tmp = readl(mmio + HOST_CTL);	/* flush && sanity check */
		if (tmp & HOST_AHCI_EN)
			return;
		msleep(10);
	}

	WARN_ON(1);
}

void nvt_ahci_save_initial_config(struct device *dev, struct ahci_host_priv *hpriv)
{
	void __iomem *mmio = hpriv->mmio;
	u32 cap, cap2, vers, port_map;
	int i;

	/* make sure AHCI mode is enabled before accessing CAP */
	nvt_ahci_enable_ahci(mmio);

	/* Values prefixed with saved_ are written back to host after
	 * reset.  Values without are used for driver operation.
	 */
	hpriv->saved_cap = cap = readl(mmio + HOST_CAP);
	hpriv->saved_port_map = port_map = readl(mmio + HOST_PORTS_IMPL);

	/* CAP2 register is only defined for AHCI 1.2 and later */
	vers = readl(mmio + HOST_VERSION);
	if ((vers >> 16) > 1 ||
	    ((vers >> 16) == 1 && (vers & 0xFFFF) >= 0x200))
		hpriv->saved_cap2 = cap2 = readl(mmio + HOST_CAP2);
	else
		hpriv->saved_cap2 = cap2 = 0;

#ifdef CONFIG_PLAT_NOVATEK
	cap |= HOST_CAP_64;
#else
	/* some chips have errata preventing 64bit use */
	if ((cap & HOST_CAP_64) && (hpriv->flags & AHCI_HFLAG_32BIT_ONLY)) {
		dev_info(dev, "controller can't do 64bit DMA, forcing 32bit\n");
		cap &= ~HOST_CAP_64;
	}
#endif

	if ((cap & HOST_CAP_NCQ) && (hpriv->flags & AHCI_HFLAG_NO_NCQ)) {
		dev_info(dev, "controller can't do NCQ, turning off CAP_NCQ\n");
		cap &= ~HOST_CAP_NCQ;
	}

	if (!(cap & HOST_CAP_NCQ) && (hpriv->flags & AHCI_HFLAG_YES_NCQ)) {
		dev_info(dev, "controller can do NCQ, turning on CAP_NCQ\n");
		cap |= HOST_CAP_NCQ;
	}

	if ((cap & HOST_CAP_PMP) && (hpriv->flags & AHCI_HFLAG_NO_PMP)) {
		dev_info(dev, "controller can't do PMP, turning off CAP_PMP\n");
		cap &= ~HOST_CAP_PMP;
	}

	if ((cap & HOST_CAP_SNTF) && (hpriv->flags & AHCI_HFLAG_NO_SNTF)) {
		dev_info(dev,
			 "controller can't do SNTF, turning off CAP_SNTF\n");
		cap &= ~HOST_CAP_SNTF;
	}

	if ((cap2 & HOST_CAP2_SDS) && (hpriv->flags & AHCI_HFLAG_NO_DEVSLP)) {
		dev_info(dev,
			 "controller can't do DEVSLP, turning off\n");
		cap2 &= ~HOST_CAP2_SDS;
		cap2 &= ~HOST_CAP2_SADM;
	}

	if (!(cap & HOST_CAP_FBS) && (hpriv->flags & AHCI_HFLAG_YES_FBS)) {
		dev_info(dev, "controller can do FBS, turning on CAP_FBS\n");
		cap |= HOST_CAP_FBS;
	}

	if ((cap & HOST_CAP_FBS) && (hpriv->flags & AHCI_HFLAG_NO_FBS)) {
		dev_info(dev, "controller can't do FBS, turning off CAP_FBS\n");
		cap &= ~HOST_CAP_FBS;
	}

	if (!(cap & HOST_CAP_ALPM) && (hpriv->flags & AHCI_HFLAG_YES_ALPM)) {
		dev_info(dev, "controller can do ALPM, turning on CAP_ALPM\n");
		cap |= HOST_CAP_ALPM;
	}

	if (hpriv->force_port_map && port_map != hpriv->force_port_map) {
		dev_info(dev, "forcing port_map 0x%x -> 0x%x\n",
			 port_map, hpriv->force_port_map);
		port_map = hpriv->force_port_map;
		hpriv->saved_port_map = port_map;
	}

	if (hpriv->mask_port_map) {
		dev_warn(dev, "masking port_map 0x%x -> 0x%x\n",
			 port_map,
			 port_map & hpriv->mask_port_map);
		port_map &= hpriv->mask_port_map;
	}

	/* cross check port_map and cap.n_ports */
	if (port_map) {
		int map_ports = 0;

		for (i = 0; i < AHCI_MAX_PORTS; i++)
			if (port_map & (1 << i))
				map_ports++;

		/* If PI has more ports than n_ports, whine, clear
		 * port_map and let it be generated from n_ports.
		 */
		if (map_ports > ahci_nr_ports(cap)) {
			dev_warn(dev,
				 "implemented port map (0x%x) contains more ports than nr_ports (%u), using nr_ports\n",
				 port_map, ahci_nr_ports(cap));
			port_map = 0;
		}
	}

	/* fabricate port_map from cap.nr_ports for < AHCI 1.3 */
	if (!port_map && vers < 0x10300) {
		port_map = (1 << ahci_nr_ports(cap)) - 1;
		dev_warn(dev, "forcing PORTS_IMPL to 0x%x\n", port_map);

		/* write the fixed up value to the PI register */
		hpriv->saved_port_map = port_map;
	}

	/* record values to use during operation */
	hpriv->cap = cap;
	hpriv->cap2 = cap2;
	hpriv->version = readl(mmio + HOST_VERSION);
	hpriv->port_map = port_map;

	if (!hpriv->start_engine)
		hpriv->start_engine = ahci_start_engine;

	if (!hpriv->stop_engine)
		hpriv->stop_engine = nvt_ahci_stop_engine;

	if (!hpriv->irq_handler)
		hpriv->irq_handler = nvt_ahci_single_level_irq_intr;
}

int nvt_ahci_platform_init_host(struct platform_device *pdev,
				struct ahci_host_priv *hpriv,
				const struct ata_port_info *pi_template,
				struct scsi_host_template *sht)
{
	struct device *dev = &pdev->dev;
	struct ata_port_info pi = *pi_template;
	const struct ata_port_info *ppi[] = { &pi, NULL };
	struct ata_host *host;
	int i, irq, n_ports, rc;
#ifdef CONFIG_PLAT_NOVATEK
	struct ahci_nvt_plat_data *nvt_plat_data = hpriv->plat_data;
#endif

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		if (irq != -EPROBE_DEFER)
			dev_err(dev, "no irq\n");
		return irq;
	}

	hpriv->irq = irq;

	/* prepare host */
	pi.private_data = (void *)(unsigned long)hpriv->flags;

	nvt_ahci_save_initial_config(dev, hpriv);

	if (hpriv->cap & HOST_CAP_NCQ)
		pi.flags |= ATA_FLAG_NCQ;

	if (hpriv->cap & HOST_CAP_PMP)
		pi.flags |= ATA_FLAG_PMP;

	ahci_set_em_messages(hpriv, &pi);

	/* CAP.NP sometimes indicate the index of the last enabled
	 * port, at other times, that of the last possible port, so
	 * determining the maximum port number requires looking at
	 * both CAP.NP and port_map.
	 */
	n_ports = max(ahci_nr_ports(hpriv->cap), fls(hpriv->port_map));

	host = ata_host_alloc_pinfo(dev, ppi, n_ports);
	if (!host)
		return -ENOMEM;

	host->private_data = hpriv;
#ifdef CONFIG_PLAT_NOVATEK
	nvt_plat_data->host = host;
#endif
	if (!(hpriv->cap & HOST_CAP_SSS) || ahci_ignore_sss)
		host->flags |= ATA_HOST_PARALLEL_SCAN;
	else
		dev_info(dev, "SSS flag set, parallel bus scan disabled\n");

	if (pi.flags & ATA_FLAG_EM)
		ahci_reset_em(host);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		ata_port_desc(ap, "mmio %pR",
			      platform_get_resource(pdev, IORESOURCE_MEM, 0));
		ata_port_desc(ap, "port 0x%x", 0x100 + ap->port_no * 0x80);

		/* set enclosure management message type */
		if (ap->flags & ATA_FLAG_EM)
			ap->em_message_type = hpriv->em_msg_type;

		/* disabled/not-implemented port */
		if (!(hpriv->port_map & (1 << i)))
			ap->ops = &ata_dummy_port_ops;
	}
#ifdef CONFIG_PLAT_NOVATEK
	rc = dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(32));
	if (rc) {
		dev_err(dev, "Failed to enable 32-bit DMA.\n");
		return rc;
	}
#else
	if (hpriv->cap & HOST_CAP_64) {
		rc = dma_coerce_mask_and_coherent(dev, DMA_BIT_MASK(64));
		if (rc) {
			rc = dma_coerce_mask_and_coherent(dev,
							  DMA_BIT_MASK(32));
			if (rc) {
				dev_err(dev, "Failed to enable 64-bit DMA.\n");
				return rc;
			}
			dev_warn(dev, "Enable 32-bit DMA instead of 64-bit.\n");
		}
	}
#endif
	rc = nvt_ahci_reset_controller(host);
	if (rc)
		return rc;

	ahci_init_controller(host);
	ahci_print_info(host, "platform");

	return ahci_host_activate(host, sht);
}

static int nvt_ahci_bad_pmp_check_ready(struct ata_link *link)
{
	void __iomem *port_mmio = ahci_port_base(link->ap);
	u8 status = readl(port_mmio + PORT_TFDATA) & 0xFF;
	u32 irq_status = readl(port_mmio + PORT_IRQ_STAT);
	u32 cmd = readl(port_mmio + PORT_CMD);

	/*
	 * There is no need to check TFDATA if BAD PMP is found due to HW bug,
	 * which can save timeout delay.
	 */
	if ((cmd & PORT_CMD_PMP) && (irq_status & PORT_IRQ_BAD_PMP))
		return -EIO;

	return ata_check_ready(status);
}

static int nvt_ahci_exec_polled_cmd(struct ata_port *ap, int pmp,
				    struct ata_taskfile *tf, int is_cmd, u16 flags,
				    unsigned long timeout_msec)
{
	const u32 cmd_fis_len = 5; /* five dwords */
	struct ahci_port_priv *pp = ap->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u8 *fis = pp->cmd_tbl;
	u32 tmp;

	/* prep the command */
	ata_tf_to_fis(tf, pmp, is_cmd, fis);
	ahci_fill_cmd_slot(pp, 0, cmd_fis_len | flags | (pmp << 12));

	/* set port value for softreset of Port Multiplier */
	if (pp->fbs_enabled && pp->fbs_last_dev != pmp) {
		tmp = readl(port_mmio + PORT_FBS);
		tmp &= ~(PORT_FBS_DEV_MASK | PORT_FBS_DEC);
		tmp |= pmp << PORT_FBS_DEV_OFFSET;
		writel(tmp, port_mmio + PORT_FBS);
		pp->fbs_last_dev = pmp;
	}

	/* issue & wait */
	writel(1, port_mmio + PORT_CMD_ISSUE);

	if (timeout_msec) {
		tmp = ata_wait_register(ap, port_mmio + PORT_CMD_ISSUE,
					0x1, 0x1, 1, timeout_msec);
		if (tmp & 0x1) {
			ahci_kick_engine(ap);
			return -EBUSY;
		}
	} else
		readl(port_mmio + PORT_CMD_ISSUE);	/* flush */

	return 0;
}

static int nvt_ahci_do_softreset(struct ata_link *link, unsigned int *class,
				 int pmp, unsigned long deadline,
				 int (*check_ready)(struct ata_link *link))
{
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	const char *reason = NULL;
	unsigned long now, msecs;
	struct ata_taskfile tf;
	bool fbs_disabled = false;
	int rc;
#ifdef CONFIG_PLAT_NOVATEK
	struct ahci_nvt_plat_data *plat_data = hpriv->plat_data;
	u32 val_fsm;
#endif

	DPRINTK("ENTER\n");

	/* prepare for SRST (AHCI-1.1 10.4.1) */
	rc = ahci_kick_engine(ap);
	if (rc && rc != -EOPNOTSUPP)
		ata_link_warn(link, "failed to reset engine (errno=%d)\n", rc);

	/*
	 * According to AHCI-1.2 9.3.9: if FBS is enable, software shall
	 * clear PxFBS.EN to '0' prior to issuing software reset to devices
	 * that is attached to port multiplier.
	 */
	if (pp->fbs_enabled) {
		nvt_ahci_disable_fbs(ap);
		fbs_disabled = true;
	}

	ata_tf_init(link->device, &tf);

	/* issue the first H2D Register FIS */
	msecs = 0;
	now = jiffies;

	if (time_after(deadline, now))
		msecs = jiffies_to_msecs(deadline - now);

#ifdef CONFIG_PLAT_NOVATEK
	if (plat_data->ata_hrst == 0) {
		tf.ctl |= ATA_SRST;
	}
#else
	tf.ctl |= ATA_SRST;
#endif

	if (nvt_ahci_exec_polled_cmd(ap, pmp, &tf, 0,
				     AHCI_CMD_RESET | AHCI_CMD_CLR_BUSY, msecs)) {
		rc = -EIO;
		reason = "1st FIS failed";
		goto fail;
	}

	/* spec says at least 5us, but be generous and sleep for 1ms */
#ifdef CONFIG_PLAT_NOVATEK
	ata_msleep(ap, ahci_asrt_delay);
#else
	ata_msleep(ap, 1);
#endif
	/* issue the second H2D Register FIS */
	tf.ctl &= ~ATA_SRST;

	nvt_ahci_exec_polled_cmd(ap, pmp, &tf, 0, 0, 0);

	/* wait for link to become ready */
	rc = ata_wait_after_reset(link, deadline, check_ready);
	if (rc == -EBUSY && hpriv->flags & AHCI_HFLAG_SRST_TOUT_IS_OFFLINE) {
		/*
		 * Workaround for cases where link online status can't
		 * be trusted.  Treat device readiness timeout as link
		 * offline.
		 */
		ata_link_info(link, "device not ready, treating as offline\n");
		*class = ATA_DEV_NONE;
	} else if (rc) {
		/* link occupied, -ENODEV too is an error */
		reason = "device not ready";
		goto fail;
	} else
		*class = ahci_dev_classify(ap);

	/* re-enable FBS if disabled before */
	if (fbs_disabled)
		nvt_ahci_enable_fbs(ap);

	DPRINTK("EXIT, class=%u\n", *class);
	return 0;

fail:
#ifdef CONFIG_PLAT_NOVATEK
	if (plat_data->ata_hrst == 1) {
		plat_data->ata_hrst = 0;
	} else {
		ata_link_err(link, "softreset failed (%s)\n", reason);
#ifdef CONFIG_PLAT_NOVATEK
		if (plat_data->cg_reset) {
			/* select internal FSM */
			val_fsm = readl(hpriv->mmio + 0x174);
			val_fsm &= ~(0x1 << 31);
			val_fsm |= (0x1 << 31);
			writel(val_fsm, hpriv->mmio + 0x174);

			/* check FSM */
			val_fsm = readl(hpriv->mmio + 0x17C);

			if ((val_fsm & 0x5000c00) == 0x5000c00) {
				ata_link_err(link, "[AXI_PATCH] trigger cg reset at softreset, [0x17c(%x), 0x15c(%x), 0x118(%x)]\n", val_fsm, readl(hpriv->mmio + 0x15C), readl(hpriv->mmio + 0x118));
				plat_data->cg_reset(link);
			}
		}
#endif
	}
#else
	ata_link_err(link, "softreset failed (%s)\n", reason);
#endif
	return rc;
}

int nvt_ahci_pmp_retry_softreset(struct ata_link *link, unsigned int *class,
				 unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	void __iomem *port_mmio = ahci_port_base(ap);
	int pmp = sata_srst_pmp(link);
	int rc;
	u32 irq_sts;

	DPRINTK("ENTER\n");

	rc = nvt_ahci_do_softreset(link, class, pmp, deadline,
				   nvt_ahci_bad_pmp_check_ready);

	/*
	 * Soft reset fails with IPMS set when PMP is enabled but
	 * SATA HDD/ODD is connected to SATA port, do soft reset
	 * again to port 0.
	 */
	if (rc == -EIO) {
		irq_sts = readl(port_mmio + PORT_IRQ_STAT);
		if (irq_sts & PORT_IRQ_BAD_PMP) {
			ata_link_warn(link,
				      "applying PMP SRST workaround "
				      "and retrying\n");
			rc = nvt_ahci_do_softreset(link, class, 0, deadline,
						   ahci_check_ready);
		}
	}

	return rc;
}

int nvt_ahci_softreset(struct ata_link *link, unsigned int *class,
		       unsigned long deadline)
{
	int pmp = sata_srst_pmp(link);

	DPRINTK("ENTER\n");

	return nvt_ahci_do_softreset(link, class, pmp, deadline, ahci_check_ready);
}

/*****************************
 *    Clone for reference    *
 *****************************/
static int __nvt_sata_set_spd_needed(struct ata_link *link, u32 *scontrol)
{
	struct ata_link *host_link = &link->ap->link;
	u32 limit, target, spd;

	limit = link->sata_spd_limit;

	/* Don't configure downstream link faster than upstream link.
	 * It doesn't speed up anything and some PMPs choke on such
	 * configuration.
	 */
	if (!ata_is_host_link(link) && host_link->sata_spd)
		limit &= (1 << host_link->sata_spd) - 1;

	if (limit == UINT_MAX)
		target = 0;
	else
		target = fls(limit);

	spd = (*scontrol >> 4) & 0xf;
	*scontrol = (*scontrol & ~0xf0) | ((target & 0xf) << 4);

	return spd != target;
}

int nvt_sata_set_spd_needed(struct ata_link *link)
{
	u32 scontrol;

	if (sata_scr_read(link, SCR_CONTROL, &scontrol))
		return 1;

	return __nvt_sata_set_spd_needed(link, &scontrol);
}

static bool nvt_ata_sstatus_online(u32 sstatus)
{
	return (sstatus & 0xf) == 0x3;
}

bool nvt_ata_phys_link_offline(struct ata_link *link)
{
	u32 sstatus;

	if (sata_scr_read(link, SCR_STATUS, &sstatus) == 0 &&
	    !nvt_ata_sstatus_online(sstatus))
		return true;
	return false;
}

int nvt_ata_wait_ready(struct ata_link *link, unsigned long deadline,
			      int (*check_ready)(struct ata_link *link))
{
	unsigned long start = jiffies;
	unsigned long nodev_deadline;
	int warned = 0;

	/* choose which 0xff timeout to use, read comment in libata.h */
	if (link->ap->host->flags & ATA_HOST_PARALLEL_SCAN)
		nodev_deadline = ata_deadline(start, ATA_TMOUT_FF_WAIT_LONG);
	else
		nodev_deadline = ata_deadline(start, ATA_TMOUT_FF_WAIT);

	/* Slave readiness can't be tested separately from master.  On
	 * M/S emulation configuration, this function should be called
	 * only on the master and it will handle both master and slave.
	 */
	WARN_ON(link == link->ap->slave_link);

	if (time_after(nodev_deadline, deadline))
		nodev_deadline = deadline;

	while (1) {
		unsigned long now = jiffies;
		int ready, tmp;

		ready = tmp = check_ready(link);
		if (ready > 0)
			return 0;

		/*
		 * -ENODEV could be transient.  Ignore -ENODEV if link
		 * is online.  Also, some SATA devices take a long
		 * time to clear 0xff after reset.  Wait for
		 * ATA_TMOUT_FF_WAIT[_LONG] on -ENODEV if link isn't
		 * offline.
		 *
		 * Note that some PATA controllers (pata_ali) explode
		 * if status register is read more than once when
		 * there's no device attached.
		 */
		if (ready == -ENODEV) {
			if (ata_link_online(link))
				ready = 0;
			else if ((link->ap->flags & ATA_FLAG_SATA) &&
				 !ata_link_offline(link) &&
				 time_before(now, nodev_deadline))
				ready = 0;
		}

		if (ready)
			return ready;
		if (time_after(now, deadline))
			return -EBUSY;

		if (!warned && time_after(now, start + 5 * HZ) &&
		    (deadline - now > 3 * HZ)) {
			ata_link_warn(link,
				      "link is slow to respond, please be patient "
				      "(ready=%d)\n", tmp);
			warned = 1;
		}

		ata_msleep(link->ap, 50);
	}
}
