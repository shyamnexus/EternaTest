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

#include "ahci_nvt_ivot_util.h"

/*************************
 *    Clone Implement    *
 *************************/

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

void nvt_ahci_platform_put_resources(struct device *dev, void *res)
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

int nvt_ahci_platform_get_phy(struct ahci_host_priv *hpriv, u32 port,
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
				"couldn't get PHY in node %pOFn: ENOSYS\n",
				node);
			break;
		}
		fallthrough;
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
			"couldn't get PHY in node %pOFn: %d\n",
			node, rc);

		break;
	}

	return rc;
}

int nvt_ahci_platform_get_regulator(struct ahci_host_priv *hpriv, u32 port,
				struct device *dev)
{
	struct regulator *target_pwr;
	int rc = 0;

	target_pwr = regulator_get(dev, "target");

	if (!IS_ERR(target_pwr))
		hpriv->target_pwrs[port] = target_pwr;
	else
		rc = PTR_ERR(target_pwr);

	return rc;
}