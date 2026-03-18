/**
    NVT PCIE cascade driver
    PCIE controller related API
    @file nvt-pcie-ctrl.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/msi.h>

#include "nvt_pcie_int.h"
#include "pcie_reg.h"

// debug start
#define DBG_ERR(fmt, args...) pr_err("ERR:%s() "fmt, __func__, ##args)
#define DBG_WRN(fmt, args...) pr_err("WRN:%s() "fmt, __func__, ##args)
#define DBG_DUMP(fmt, args...) printk(fmt, ##args)
// debug end ------




void init_msix_table(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
#ifdef NVT_PCIE_LOOPBACK
	T_PCI_TOP_CTL3_REG top_ctl3;
	T_MSIX_ADDRESS_MATCH_LOW_OFF addr_match_low = {0};
	T_PCI_MSIX_CAP_ID_NEXT_CTRL_REG msix_cap_reg;
	UINT32 i;

	//
	// step 1. populate the MSI-X table
	//

	// switch to MSI-X table
	top_ctl3.reg = PCIE_GETREG(nvt_pcie_chip_ptr, PCI_TOP_CTL3_REG_OFS);
	top_ctl3.bit.DBI_ADDR_32to22 = DBI_ADDR_SEL_MSIX;
	PCIE_SETREG(nvt_pcie_chip_ptr, PCI_TOP_CTL3_REG_OFS, top_ctl3.reg);

	DBG_WRN(" table0: 0x%8x 0x%8x 0x%8x 0x%8x\r\n",
		PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS),
		PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x04),
		PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x08),
		PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x0C));

	for (i=0; i<INT_ID_MAX; i++) {
		UINT64 target_addr = MSIX_PCIE_RC_RX_ADDR + (i/32)*4 + 0x300;
		UINT32 data_mask = 1 << (i%32);

		// set address to 0x6_0000_0000
		PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x00+i*0x10, target_addr&0xFFFFFFFF);// low address
		PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x04+i*0x10, target_addr>>32);	// high address
		PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x08+i*0x10, data_mask);	// data
		PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x0C+i*0x10, 0x00);	// enable this MSIX vector (per vector mask)

#if 0
		DBG_WRN("table %d (0x%p): 0x%8x 0x%8x 0x%8x 0x%8x\r\n",
			i,
			nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI],
			PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x00+i*0x10),
			PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x04+i*0x10),
			PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x08+i*0x10),
			PCIE_GETDBI(nvt_pcie_chip_ptr, MSIX_TABLE_OFS+0x0C+i*0x10));
#endif
	}

	// switch to normal DBI
	top_ctl3.reg = PCIE_GETREG(nvt_pcie_chip_ptr, PCI_TOP_CTL3_REG_OFS);
	top_ctl3.bit.DBI_ADDR_32to22 = 0;
	PCIE_SETREG(nvt_pcie_chip_ptr, PCI_TOP_CTL3_REG_OFS, top_ctl3.reg);

	//
	// step2. setup MSIX_ADDRESS_MATCH
	//
	PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_ADDRESS_MATCH_HIGH_OFF_OFS, MSIX_PCIE_MATCH_ADDR>>32);
	addr_match_low.reg = MSIX_PCIE_MATCH_ADDR&0xFFFFFFFF;
	addr_match_low.bit.MATCH_EN = 1;
	PCIE_SETDBI(nvt_pcie_chip_ptr, MSIX_ADDRESS_MATCH_LOW_OFF_OFS, addr_match_low.reg);

	//
	// step 8. enable MSIX function mask
	//
	msix_cap_reg.reg = PCIE_GETDBI(nvt_pcie_chip_ptr, PCI_MSIX_CAP_ID_NEXT_CTRL_REG_OFS);
	msix_cap_reg.bit.PCI_MSIX_FUNCTION_MASK = 0;
	msix_cap_reg.bit.PCI_MSIX_ENABLE = 1;
	PCIE_SETDBI(nvt_pcie_chip_ptr, PCI_MSIX_CAP_ID_NEXT_CTRL_REG_OFS, msix_cap_reg.reg);
#else
	T_MSIX_ADDRESS_MATCH_LOW_OFF addr_match_low = {0};
	T_PCI_MSIX_CAP_ID_NEXT_CTRL_REG msix_cap_reg;
	UINT32 i;

	//
	// step 1. populate the MSI-X table
	//

	DBG_WRN(" table0: 0x%8x 0x%8x 0x%8x 0x%8x\r\n",
		nvt_pcie_chip_ptr->ep.read_msix_tbl(&nvt_pcie_chip_ptr->ep, 0x0),
		nvt_pcie_chip_ptr->ep.read_msix_tbl(&nvt_pcie_chip_ptr->ep, 0x04),
		nvt_pcie_chip_ptr->ep.read_msix_tbl(&nvt_pcie_chip_ptr->ep, 0x08),
		nvt_pcie_chip_ptr->ep.read_msix_tbl(&nvt_pcie_chip_ptr->ep, 0x0C));

	for (i=0; i<INT_ID_MAX; i++) {
		UINT64 target_addr = MSIX_PCIE_RC_RX_ADDR + (i/32)*4 + 0x300;
		UINT32 data_mask = 1 << (i%32);

		// set address to 0x6_0000_0000
		nvt_pcie_chip_ptr->ep.write_msix_tbl(&nvt_pcie_chip_ptr->ep,
					0x00+i*0x10, lower_32_bits(target_addr));	// low address
		nvt_pcie_chip_ptr->ep.write_msix_tbl(&nvt_pcie_chip_ptr->ep,
					0x04+i*0x10, upper_32_bits(target_addr));	// high address
		nvt_pcie_chip_ptr->ep.write_msix_tbl(&nvt_pcie_chip_ptr->ep,
					0x08+i*0x10, data_mask);	// data
		nvt_pcie_chip_ptr->ep.write_msix_tbl(&nvt_pcie_chip_ptr->ep,
					0x0C+i*0x10, 0x00);	// enable this MSIX vector (per vector mask)
	}


	//
	// step2. setup MSIX_ADDRESS_MATCH
	//
	nvt_pcie_chip_ptr->ep.write_dbi(&nvt_pcie_chip_ptr->ep, MSIX_ADDRESS_MATCH_HIGH_OFF_OFS, upper_32_bits(MSIX_PCIE_MATCH_ADDR));
	addr_match_low.reg = lower_32_bits(MSIX_PCIE_MATCH_ADDR);
	addr_match_low.bit.MATCH_EN = 1;
	nvt_pcie_chip_ptr->ep.write_dbi(&nvt_pcie_chip_ptr->ep, MSIX_ADDRESS_MATCH_LOW_OFF_OFS, addr_match_low.reg);

	//
	// step 8. enable MSIX function mask
	//
	msix_cap_reg.reg = nvt_pcie_chip_ptr->ep.read_cfg(&nvt_pcie_chip_ptr->ep, PCI_MSIX_CAP_ID_NEXT_CTRL_REG_OFS);
	msix_cap_reg.bit.PCI_MSIX_FUNCTION_MASK = 0;
	msix_cap_reg.bit.PCI_MSIX_ENABLE = 1;
	nvt_pcie_chip_ptr->ep.write_cfg(&nvt_pcie_chip_ptr->ep, PCI_MSIX_CAP_ID_NEXT_CTRL_REG_OFS, msix_cap_reg.reg);
#endif

	DBG_WRN("config MSIX table done\r\n");
}

static u32 rc_read_dbi(struct nvt_pci_ctrl *ctrl, int reg)
{
	T_PCI_TOP_CTL3_REG top_ctl3;

	top_ctl3.reg = PCIE_GETREG(ctrl->chip, PCI_TOP_CTL3_REG_OFS);
	if (reg >= DBI_SHADOW_SIZE) {
		top_ctl3.bit.DBI_ADDR_32to22 = reg >> 22;
	} else {
		top_ctl3.bit.DBI_ADDR_32to22 = 0;
	}
	PCIE_SETREG(ctrl->chip, PCI_TOP_CTL3_REG_OFS, top_ctl3.reg);

	return readl(ctrl->dbi_ptr + reg);
}

static int rc_write_dbi(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	T_PCI_TOP_CTL3_REG top_ctl3;

	top_ctl3.reg = PCIE_GETREG(ctrl->chip, PCI_TOP_CTL3_REG_OFS);
	if (reg >= DBI_SHADOW_SIZE) {
		top_ctl3.bit.DBI_ADDR_32to22 = reg >> 22;
	} else {
		top_ctl3.bit.DBI_ADDR_32to22 = 0;
	}
	PCIE_SETREG(ctrl->chip, PCI_TOP_CTL3_REG_OFS, top_ctl3.reg);

	writel(data, ctrl->dbi_ptr + reg);

	return 0;
}

static u32 rc_read_cfg(struct nvt_pci_ctrl *ctrl, int reg)
{
	// In RC, CFG is on DBI
	return rc_read_dbi(ctrl, reg);
}

static int rc_write_cfg(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	// In RC, CFG is on DBI
	return rc_write_dbi(ctrl, reg, data);
}

static u32 rc_read_atu(struct nvt_pci_ctrl *ctrl, int reg)
{
	// In RC, ATU is on DBI
	return rc_read_dbi(ctrl, reg + DBI_ATU_BASE);
}

static int rc_write_atu(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	// In RC, ATU is on DBI
	return rc_write_dbi(ctrl, reg + DBI_ATU_BASE, data);
}

static u32 rc_read_msix_ctrl(struct nvt_pci_ctrl *ctrl, int reg)
{
	return readl(ctrl->msix_ctrl_ptr + reg);
}

static int rc_write_msix_ctrl(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	writel(data, ctrl->msix_ctrl_ptr + reg);

	return 0;
}

static u32 rc_read_msix_tbl(struct nvt_pci_ctrl *ctrl, int reg)
{
	// In RC, MSIX table is on DBI
	return rc_read_dbi(ctrl, reg + MSIX_TABLE_OFS);
}

static int rc_write_msix_tbl(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	// In RC, MSIX table is on DBI
	return rc_write_dbi(ctrl, reg + MSIX_TABLE_OFS, data);
}

static void rc_config_oATU(struct nvt_pci_ctrl *ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en)
{
	printk("%s: still NOT implement\r\n", __func__);
}

static void rc_config_iATU(struct nvt_pci_ctrl *ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en)
{
	printk("%s: still NOT implement\r\n", __func__);
}

static int setup_controller_rc(struct nvt_pcie_chip *chip, struct nvt_pci_ctrl *ctrl)
{
	ctrl->mode = NVT_PCIE_RC;
	ctrl->chip = chip;
	ctrl->dbi_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI];
	ctrl->atu_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI] + DBI_ATU_BASE;
	ctrl->cfg_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI];
	ctrl->msix_ctrl_ptr = chip->PCIE_REG_BASE[MEM_MSIX];
	ctrl->msix_tbl_ptr = chip->PCIE_REG_BASE[MEM_PCIE_DBI] + MSIX_TABLE_OFS;

	ctrl->read_dbi = rc_read_dbi;
	ctrl->write_dbi = rc_write_dbi;
	ctrl->read_cfg = rc_read_cfg;
	ctrl->write_cfg = rc_write_cfg;
	ctrl->read_atu = rc_read_atu;
	ctrl->write_atu = rc_write_atu;
	ctrl->read_msix_ctrl = rc_read_msix_ctrl;
	ctrl->write_msix_ctrl = rc_write_msix_ctrl;
	ctrl->read_msix_tbl = rc_read_msix_tbl;
	ctrl->write_msix_tbl = rc_write_msix_tbl;

	ctrl->config_oATU = rc_config_oATU;
	ctrl->config_iATU = rc_config_iATU;

	return 0;
}

static u32 ep_read_dbi(struct nvt_pci_ctrl *ctrl, int reg)
{
	return readl(ctrl->dbi_ptr + reg);
}

static int ep_write_dbi(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	writel(data, ctrl->dbi_ptr + reg);

	return 0;
}

static u32 ep_read_cfg(struct nvt_pci_ctrl *ctrl, int reg)
{
	return readl(ctrl->cfg_ptr + reg);
}

static int ep_write_cfg(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	writel(data, ctrl->cfg_ptr + reg);

	return 0;
}

static u32 ep_read_atu(struct nvt_pci_ctrl *ctrl, int reg)
{
	printk("%s: still NOT implement\r\n", __func__);

	return 0;
}

static int ep_write_atu(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	printk("%s: still NOT implement\r\n", __func__);

	return -1;
}

static u32 ep_read_msix_ctrl(struct nvt_pci_ctrl *ctrl, int reg)
{
	return readl(ctrl->msix_ctrl_ptr + reg);
}

static int ep_write_msix_ctrl(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	writel(data, ctrl->msix_ctrl_ptr + reg);

	return 0;
}

static u32 ep_read_msix_tbl(struct nvt_pci_ctrl *ctrl, int reg)
{
	return readl(ctrl->msix_tbl_ptr + reg);
}

static int ep_write_msix_tbl(struct nvt_pci_ctrl *ctrl, int reg, u32 data)
{
	writel(data, ctrl->msix_tbl_ptr + reg);

	return 0;
}

static void ep_config_oATU(struct nvt_pci_ctrl *ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en)
{
	printk("%s: still NOT implement\r\n", __func__);
}

static void ep_config_iATU(struct nvt_pci_ctrl *ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en)
{
	printk("%s: still NOT implement\r\n", __func__);
}

static int setup_controller_ep(struct nvt_pcie_chip *chip, struct nvt_pci_ctrl *ctrl)
{
	ctrl->mode = NVT_PCIE_EP;
	ctrl->chip = chip;
	ctrl->dbi_ptr = chip->PCIE_REG_BASE[MEM_EP_DBI];
	ctrl->atu_ptr = chip->PCIE_REG_BASE[MEM_EP_ATU];
	ctrl->cfg_ptr = chip->PCIE_REG_BASE[MEM_EP_CFG];
	ctrl->msix_ctrl_ptr = chip->PCIE_REG_BASE[MEM_EP_MSIX_CTRL];
	ctrl->msix_tbl_ptr = chip->PCIE_REG_BASE[MEM_EP_MSIX_TBL];

	ctrl->read_dbi = ep_read_dbi;
	ctrl->write_dbi = ep_write_dbi;
	ctrl->read_cfg = ep_read_cfg;
	ctrl->write_cfg = ep_write_cfg;
	ctrl->read_atu = ep_read_atu;
	ctrl->write_atu = ep_write_atu;
	ctrl->read_msix_ctrl = ep_read_msix_ctrl;
	ctrl->write_msix_ctrl = ep_write_msix_ctrl;
	ctrl->read_msix_tbl = ep_read_msix_tbl;
	ctrl->write_msix_tbl = ep_write_msix_tbl;

	ctrl->config_oATU = ep_config_oATU;
	ctrl->config_iATU = ep_config_iATU;

	return 0;
}

int setup_controller_hdl(struct nvt_pcie_chip *chip)
{
	// Fill RC
	setup_controller_rc(chip, &chip->rc);

	// Fill EP(1)
	setup_controller_ep(chip, &chip->ep);

	return 0;
}

// use before module init
int check_pcie_link(void)
{
	const phys_addr_t PCIE_TOP_BASE = 0x2F04F0000;
	const size_t PCIE_TOP_REG_SIZE = 0x100;
	void __iomem *ptr;
	T_PCI_TOP_DMC_STATUS0_REG sts_reg;

	ptr = ioremap(PCIE_TOP_BASE, PCIE_TOP_REG_SIZE);
	if (unlikely(ptr == 0)) {
		return 0;
	}

	sts_reg.reg = readl(ptr + PCI_TOP_DMC_STATUS0_REG_OFS);

	iounmap(ptr);

	return sts_reg.bit.rdlh_link_up;
}

void dump_pcie_dbi(struct nvt_pcie_chip *nvt_pcie_chip_ptr) {
	int i;

	for (i=0; i<256; i+=16) {
		printk("0x%X:\t0x%8X\t0x%8X\t0x%8X\t0x%8X\r\n",
			i,
			PCIE_GETDBI(nvt_pcie_chip_ptr, i),
			PCIE_GETDBI(nvt_pcie_chip_ptr, i+4),
			PCIE_GETDBI(nvt_pcie_chip_ptr, i+8),
			PCIE_GETDBI(nvt_pcie_chip_ptr, i+12));
	}
}

void dump_pcie_msix_tbl(struct nvt_pcie_chip *chip) {
	int i;

	for (i=0; i<1024; i+=16) {
		printk("0x%X:\t0x%8X\t0x%8X\t0x%8X\t0x%8X\r\n",
			i,
			chip->ep.read_msix_tbl(&chip->ep, i),
			chip->ep.read_msix_tbl(&chip->ep, i+4),
			chip->ep.read_msix_tbl(&chip->ep, i+8),
			chip->ep.read_msix_tbl(&chip->ep, i+12));
	}
}


