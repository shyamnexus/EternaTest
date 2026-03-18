/**
    @file       pcie-nvt-util.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/version.h>

#include "pcie-designware.h"
#include "pcie-nvt.h"


static int nvt_plat_rd_peer_cfg(struct pcie_port *pp,
				u32 devfn, int where, int size, u32 *val)
{
	//This function is almost the same as dw_pcie_rd_other_conf,
	//but adding nvt_plat_set_ep_bar0 to set ep bar 0 before bus scan
	int ret, type;
	u32 busdev;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);

	if (!dw_pcie_link_up(pci))
		return 0xffffffff;

	busdev = PCIE_ATU_BUS(0) | PCIE_ATU_DEV(PCI_SLOT(devfn)) |
		 PCIE_ATU_FUNC(PCI_FUNC(devfn));

	type = PCIE_ATU_TYPE_CFG0;

	dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX_CFG, type, pp->cfg0_base, busdev, pp->cfg0_size);

	ret = dw_pcie_read(pp->va_cfg0_base + where, size, val);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,00)
	if (pci->num_viewport <= 2)
		dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX1,
					  PCIE_ATU_TYPE_IO, pp->io_base,
					  pp->io_bus_addr, pp->io_size);
#else
	if (!ret && pci->io_cfg_atu_shared)
		dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX_IO, PCIE_ATU_TYPE_IO, pp->io_base,
					  pp->io_bus_addr, pp->io_size);
#endif

	return ret;
}

static int __nvt_pci_bus_find_cap_start(struct pcie_port *pp,
					unsigned int devfn)
{
	u32 status;
	u32 hdr_type;

	nvt_plat_rd_peer_cfg(pp, devfn, PCI_STATUS, 2, &status);
	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;

	nvt_plat_rd_peer_cfg(pp, devfn, PCI_HEADER_TYPE, 1, &hdr_type);

	switch (hdr_type) {
	case PCI_HEADER_TYPE_NORMAL:
	case PCI_HEADER_TYPE_BRIDGE:
		return PCI_CAPABILITY_LIST;
	case PCI_HEADER_TYPE_CARDBUS:
		return PCI_CB_CAPABILITY_LIST;
	}

	return 0;
}

static int __nvt_pci_find_next_cap_ttl(struct pcie_port *pp, unsigned int devfn,
				       u8 pos, int cap, int *ttl)
{
	u32 id;
	u32 ent;
	u32 val;

	nvt_plat_rd_peer_cfg(pp, devfn, pos, 1, &val);
	pos = val;

	while ((*ttl)--) {
		if (pos < 0x40)
			break;
		pos &= ~3;
		nvt_plat_rd_peer_cfg(pp, devfn, pos, 2, &ent);

		id = ent & 0xff;
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pos = (ent >> 8);
	}
	return 0;
}

static int __nvt_pci_find_next_cap(struct pcie_port *pp, unsigned int devfn,
				   u8 pos, int cap)
{
	int ttl = 48;
//	int ttl = PCI_FIND_CAP_TTL;

	return __nvt_pci_find_next_cap_ttl(pp, devfn, pos, cap, &ttl);
}

static int nvt_pci_find_capability(struct pcie_port *pp, int cap)
{
	int pos;

	pos = __nvt_pci_bus_find_cap_start(pp, 0);
	if (pos)
		pos = __nvt_pci_find_next_cap(pp, 0, pos, cap);

	return pos;
}

static bool nvt_pcie_capability_reg_implemented(struct pcie_port *pp, int pos)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);

	switch (pos) {
	case PCI_EXP_FLAGS:
		return true;
	case PCI_EXP_DEVCAP:
	case PCI_EXP_DEVCTL:
	case PCI_EXP_DEVSTA:
		return true;
	case PCI_EXP_LNKCAP:
	case PCI_EXP_LNKCTL:
	case PCI_EXP_LNKSTA:
		return true;
#if 0	// assume nvt application does not need these
	case PCI_EXP_SLTCAP:
	case PCI_EXP_SLTCTL:
	case PCI_EXP_SLTSTA:
		return pcie_cap_has_sltctl(dev);
	case PCI_EXP_RTCTL:
	case PCI_EXP_RTCAP:
	case PCI_EXP_RTSTA:
		return pcie_cap_has_rtctl(dev);
#endif
	case PCI_EXP_DEVCAP2:
	case PCI_EXP_DEVCTL2:
	case PCI_EXP_LNKCAP2:
	case PCI_EXP_LNKCTL2:
	case PCI_EXP_LNKSTA2: {
		u32 val;

		nvt_plat_rd_peer_cfg(pp, 0, nvt_pcie->pcie_cap + PCI_EXP_FLAGS, 2, &val);
		val &= PCI_EXP_FLAGS_VERS;
		return val > 1;
	}
	default:
		return false;
	}
}

static int nvt_pcie_capability_read_dword(struct pcie_port *pp, int pos, u32 *val)
{
	int ret = 0;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);

	*val = 0;
	if (pos & 3)
		return -EINVAL;

	if (nvt_pcie_capability_reg_implemented(pp, pos)) {
		ret = nvt_plat_rd_peer_cfg(pp, 0, nvt_pcie->pcie_cap + pos, 4, val);
		/*
		 * Reset *val to 0 if pci_read_config_dword() fails, it may
		 * have been written as 0xFFFFFFFF if hardware error happens
		 * during pci_read_config_dword().
		 */
		if (ret)
			*val = 0;
		return ret;
	}

	return 0;
}

int nvt_pcie_get_speed_cap(struct pcie_port *pp)
{
	int pcie_cap;
	u32 lnkcap2, lnkcap;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);

	pcie_cap = nvt_pci_find_capability(pp, PCI_CAP_ID_EXP);
	nvt_pcie->pcie_cap = pcie_cap;

	/*
	 * Link Capabilities 2 was added in PCIe r3.0, sec 7.8.18.  The
	 * implementation note there recommends using the Supported Link
	 * Speeds Vector in Link Capabilities 2 when supported.
	 *
	 * Without Link Capabilities 2, i.e., prior to PCIe r3.0, software
	 * should use the Supported Link Speeds field in Link Capabilities,
	 * where only 2.5 GT/s and 5.0 GT/s speeds were defined.
	 */
	nvt_pcie_capability_read_dword(pp, PCI_EXP_LNKCAP2, &lnkcap2);
	if (lnkcap2) { /* PCIe r3.0-compliant */
		u32 lnkctrl2;

		nvt_pcie_capability_read_dword(pp, PCI_EXP_LNKCTL2, &lnkctrl2);
		lnkctrl2 &= PCI_EXP_LNKCTL2_TLS;

		if ((lnkcap2 & PCI_EXP_LNKCAP2_SLS_16_0GB) && (lnkctrl2 >= PCI_EXP_LNKCTL2_TLS_16_0GT))
			return PCIE_SPEED_16_0GT;
		else if ((lnkcap2 & PCI_EXP_LNKCAP2_SLS_8_0GB) && (lnkctrl2 >= PCI_EXP_LNKCTL2_TLS_8_0GT))
			return PCIE_SPEED_8_0GT;
		else if ((lnkcap2 & PCI_EXP_LNKCAP2_SLS_5_0GB) && (lnkctrl2 >= PCI_EXP_LNKCTL2_TLS_5_0GT))
			return PCIE_SPEED_5_0GT;
		else if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_2_5GB)
			return PCIE_SPEED_2_5GT;
		return PCI_SPEED_UNKNOWN;
	}

	nvt_pcie_capability_read_dword(pp, PCI_EXP_LNKCAP, &lnkcap);
	if ((lnkcap & PCI_EXP_LNKCAP_SLS) == PCI_EXP_LNKCAP_SLS_5_0GB)
		return PCIE_SPEED_5_0GT;
	else if ((lnkcap & PCI_EXP_LNKCAP_SLS) == PCI_EXP_LNKCAP_SLS_2_5GB)
		return PCIE_SPEED_2_5GT;

	return PCI_SPEED_UNKNOWN;
}


