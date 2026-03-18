/**
    @file       pcie-nvt.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/regmap.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/version.h>
#include <linux/soc/nvt/nvt-pci.h>
#include <linux/soc/nvt/nvt-pcie-edma.h>
#include <linux/soc/nvt/nvt-info.h>

#include <linux/timekeeping.h>

#include <linux/dma-direct.h>

#include "pcie-designware.h"
#include "pcie-nvt.h"
#include "pcie-nvt-plat.h"

#include "../../pci.h"

static const struct of_device_id nvt_plat_pcie_of_match[];

static int is_probed = 0;		// record if ever probed
#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
static int is_rstn_init = 0;		// record if PCIE_RSTN asserted
#endif
static int is_phy_init_done = 0;	// record if phy ever inited
static int porn_count = 0;
#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
static int used_lane_width = 0;
#endif
static struct semaphore sem_ctrls_lock;
static DEFINE_SPINLOCK(spin_init_lock);
static DECLARE_COMPLETION(init_comp);

int nvt_pcie_get_rc_slot(struct pci_dev *dev)
{
	struct pci_dev *p_root;
	struct pci_host_bridge *bridge;
	struct pcie_port *pp;
	struct dw_pcie *pci;
	struct nvt_plat_pcie *nvt_plat_pcie;

	p_root = pcie_find_root_port(dev);
	if (p_root == NULL) {
		return -1;
	}

	if (p_root->bus == NULL) {
		return -2;
	}

	if (p_root->bus->bridge == NULL) {
		return -3;
	}
	bridge = (struct pci_host_bridge *)p_root->bus->bridge;

	if (bridge->sysdata == NULL) {
		return -4;
	}
	pp = (struct pcie_port *)bridge->sysdata;

	if (pp == NULL) {
		return -5;
	}
	pci = to_dw_pcie_from_pp(pp);

	if (pci == NULL) {
		return -6;
	}

	if (pci->dev == NULL) {
		return -7;
	}

	nvt_plat_pcie = platform_get_drvdata(to_platform_device(pci->dev));
	if (nvt_plat_pcie == NULL) {
		return -8;
	}

	return nvt_plat_pcie->slot;
}
EXPORT_SYMBOL(nvt_pcie_get_rc_slot);

static void nvt_plat_set_rc_bar0(struct dw_pcie *pci)
{
	//Note: set RESBAR_CTRL_REG_0_REG
	//RESBAR_CTRL_REG_BAR_SIZE: 0x0 (bit 13:8) = 2^0 MB
	//RESBAR_CTRL_REG_NUM_BARS: 0x1 (bit 7:5)
#if 1
	dw_pcie_writel_dbi(pci, BAR0_MASK_OFS, 0x00);
	dev_info(pci->dev, "Disable RC BAR 0\n");
#else
	dw_pcie_writel_dbi(pci, RESBAR_CTRL_REG_0_REG_OFS, 0x20);
	dev_info(pci->dev, "Set RC BAR 0 to 1 MB\n");
#endif
}

// nvt: temp disable until find how to replace rd_conf callback in nvt controller core
#if 0
static int nvt_plat_set_ep_bar0(struct dw_pcie *pci,
				void __iomem *va_cfg_base, u32 busdev)
{
	#define MASK_RESBAR_CTRL_REG_NUM_BARS   GENMASK(7, 5)
	#define MASK_RESBAR_CTRL_REG_BAR_SIZE   GENMASK(13, 8)

	const u32 max_size_shift = 9; //max_size = (1 << max_size_shift) MB
	u32 org_size_shift = 0; //org_size = (1 << org_size_shift) MB
	u32 reg;
	int idx;

	//check whether the bar0 is set or not
	for (idx = 0; idx < ARRAY_SIZE(nvt_ep_busdev); idx++) {
		if (busdev == nvt_ep_busdev[idx]) {
			//already done before, skip
			return 0;
		}
		if (0 == nvt_ep_busdev[idx]) {
			//the end of list, add to the list
			nvt_ep_busdev[idx] = busdev;
			break;
		}
		if (idx == (ARRAY_SIZE(nvt_ep_busdev) - 1)) {
			//the list is full
			panic("nvt_ep_busdev full\r\n");
		}
	}

	//read default BAR0 size
	if (PCIBIOS_SUCCESSFUL == dw_pcie_read(va_cfg_base + RESBAR_CTRL_REG_0_REG_OFS, 4, &reg)) {
		org_size_shift = FIELD_GET(MASK_RESBAR_CTRL_REG_BAR_SIZE, reg);
		dev_info(pci->dev, "Detect EP BAR 0 %d MB, busdev 0x%X\r\n", 1 << org_size_shift, busdev);
	}

	//Note: set RESBAR_CTRL_REG_0_REG
	//RESBAR_CTRL_REG_BAR_SIZE: e.g. 0x8 (bit 13:8) = 2^8 MB
	//RESBAR_CTRL_REG_NUM_BARS: e.g. 0x1 (bit 7:5) = one bar
	if (org_size_shift != 0 && org_size_shift > max_size_shift) {
		reg = FIELD_PREP(MASK_RESBAR_CTRL_REG_NUM_BARS, 0x1) |
		      FIELD_PREP(MASK_RESBAR_CTRL_REG_BAR_SIZE, max_size_shift);

		dw_pcie_write(va_cfg_base + RESBAR_CTRL_REG_0_REG_OFS, 4, reg);
		dev_info(pci->dev, "Set EP BAR 0 to %d MB, busdev 0x%X\n", 1 << max_size_shift, busdev);
	}
	return 1;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,00)
static void nvt_plat_setup_atu(struct pcie_port *pp)
{
	//Because we set up our own pp->ops->rd_other_conf,
	//dw_pcie_setup_rc will skip some ATU init.
	//This function is to compensate the missing init operation.
	//void dw_pcie_setup_rc(struct pcie_port *pp)
	//{
	//    ...
	//    if (!pp->ops->rd_other_conf) {
	//        //skipped section
	//    }

	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct resource_entry *tmp, *entry = NULL;

	/* Get last memory resource entry */
	resource_list_for_each_entry(tmp, &pp->bridge->windows)
		if (resource_type(tmp->res) == IORESOURCE_MEM)
			entry = tmp;

	dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_MEM, entry->res->start,
				  entry->res->start - entry->offset,
				  resource_size(entry->res));
	if (pci->num_viewport > 2)
		dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX2,
					  PCIE_ATU_TYPE_IO, pp->io_base,
					  pp->io_bus_addr, pp->io_size);
}
#endif

static int nvt_plat_pcie_setup_ib_reg(struct dw_pcie *pci,
				      struct of_pci_range *range, unsigned int region_id, u8 *ib_reg_mask)
{
	u64 cpu_addr = range->cpu_addr;
	u64 pcie_addr = range->pci_addr;
	u64 len = range->size;
	if (region_id < IATU_INBOUND_TOTAL_SEG_SIZE) {
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LWR_BASE_ADDR_OFF_INBOUND_0, (u32)(pcie_addr & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_BASE_ADDR_OFF_INBOUND_0, (u32)(pcie_addr >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LIMIT_ADDR_OFF_INBOUND_0, (u32)((pcie_addr + len) & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_LIMIT_ADDR_OFF_INBOUND_0, (u32)((pcie_addr + len) >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LWR_TARGET_ADDR_OFF_INBOUND_0, (u32)(cpu_addr & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_TARGET_ADDR_OFF_INBOUND_0, (u32)(cpu_addr >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_REGION_CTRL_1_OFF_INBOUND_0, 0x00000000 | (1 << 13));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_REGION_CTRL_2_OFF_INBOUND_0, (1 << 31));
	} else {
		dev_err(pci->dev, "Inbound config region %d is larger than total size %d \n", region_id, IATU_INBOUND_TOTAL_SEG_SIZE);
		return -1;
	}

	return 0;
}

static int nvt_plat_pcie_setup_ib_exclusive(struct dw_pcie *pci,
				      struct of_pci_range *range, unsigned int region_id, u8 *ib_reg_mask)
{
	u64 cpu_addr = range->cpu_addr;
	u64 pcie_addr = range->pci_addr;
	if (region_id < IATU_INBOUND_TOTAL_SEG_SIZE) {
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LWR_BASE_ADDR_OFF_INBOUND_0, (u32)(pcie_addr & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_BASE_ADDR_OFF_INBOUND_0, (u32)(pcie_addr >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LIMIT_ADDR_OFF_INBOUND_0, (u32)((pcie_addr + 0xFFFFFFFFFULL) & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_LIMIT_ADDR_OFF_INBOUND_0, (u32)((pcie_addr + 0xFFFFFFFFFULL) >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_LWR_TARGET_ADDR_OFF_INBOUND_0, (u32)(cpu_addr & 0xFFFFFFFF));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_UPPER_TARGET_ADDR_OFF_INBOUND_0, (u32)(cpu_addr >> 32));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_REGION_CTRL_1_OFF_INBOUND_0, 0x00000000 | (1 << 13));
		dw_pcie_writel_ib_unroll(pci, region_id, PCIE_ATU_UNR_REGION_CTRL_2_OFF_INBOUND_0, (1 << 31)| (0<<29) | (0x1<<24) | (0<<23));
	} else {
		dev_err(pci->dev, "Inbound config region %d is larger than total size %d \n", region_id, IATU_INBOUND_TOTAL_SEG_SIZE);
		return -1;
	}

	return 0;
}

static int nvt_plat_pcie_parse_map_dma_ranges(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);
	struct device_node *np = pci->dev->of_node;
	struct of_pci_range range, range_mau;
	struct of_pci_range_parser parser;
	struct device *dev = pci->dev;
	unsigned int region_id = 0;
	int ret = 0;
	u8 ib_reg_mask = 0;

	if (of_pci_dma_range_parser_init(&parser, np)) {
		dev_err(dev, "missing dma-ranges property\n");
		return -EINVAL;
	}

	/* Get the dma-ranges from DT */
	for_each_of_pci_range(&parser, &range) {
		u64 end = range.cpu_addr + range.size - 1;

		dev_info(dev, "Region: %d 0x%08x 0x%016llx..0x%016llx -> 0x%016llx\n",
			 region_id, range.flags, range.cpu_addr, end, range.pci_addr);
		ret = nvt_plat_pcie_setup_ib_reg(pci, &range, region_id, &ib_reg_mask);
		if (ret < 0)
			return -1;
		if (region_id == 0) memcpy(&range_mau, &range, sizeof(range));

		if ((end - range.cpu_addr) > UINT_MAX && (end - range.cpu_addr) <= ULONG_MAX)
			region_id += 2;
		else if ((end - range.cpu_addr) > ULONG_MAX) {
			dev_err(dev, "The config range is larger than %lx\n", ULONG_MAX);
			return -1;
		} else
			region_id++;
	}

	if (nvt_pcie && nvt_pcie->exclude_ib) {
		dev_info(dev, "Region: %d 0x%08x 0x%016llx.. -> 0x%016llx\n",
			 region_id, range_mau.flags, range_mau.cpu_addr, range_mau.pci_addr);
		nvt_plat_pcie_setup_ib_exclusive(pci, &range_mau, region_id, &ib_reg_mask);
	}

	dev_info(dev, "RC inbound setup successfully\n");
	return 0;
}

#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
static int manual_aeq(struct nvt_plat_pcie_link_eq *eq, u32 preset, u32 r_c_idx)
{
	//use manuel aeq and sw eq voting
	u32 FOM = 0;
	u32 reg;
	u32 R, Q;
	u32 i;
	const u32 EQ_COUNT = 10;
	const u32 TH = 35;

	// AEQ R frz = 0
	reg = readl(eq->ptr_phy + 0x0214);
	reg |= 1<<5;
	writel(reg, eq->ptr_phy + 0x0214);

	// Manual AEQ enable
	reg = readl(eq->ptr_phy + 0x120C);
	reg |= 1<<2;
	writel(reg, eq->ptr_phy + 0x120C);

	Q = 0;

	for(i = 0; i < EQ_COUNT; i++) {
		// Manual R on
		reg = readl(eq->ptr_phy + 0x1000);
		reg |= 1<<30;
		writel(reg, eq->ptr_phy + 0x1000);

		udelay(65);

		// Manual R off
		reg = readl(eq->ptr_phy + 0x1000);
		reg &= ~(1<<30);
		writel(reg, eq->ptr_phy + 0x1000);

		reg = readl(eq->ptr_phy + 0x0A00);
		R = (reg>>8) & 0x7;
		Q += R;
	}

	eq->v_history_R[r_c_idx] = Q;
	if (Q > TH) {
		FOM = 180 - Q;
	} else {
		if (preset == 4) {
			FOM = 255;
		} else {
			FOM = 180 + Q;
		}
	}

	return FOM;
}

static void restore_aeq(struct nvt_plat_pcie_link_eq *eq)
{
	u32 reg;

	reg = readl(eq->ptr_phy + 0x091C); // enable AEQ R/C split
	reg |= (1<<16);
	writel(reg, eq->ptr_phy + 0x091C);

	reg = readl(eq->ptr_phy + 0x0214); // AEQ frz = 1
	reg &= ~(1<<5);
	writel(reg, eq->ptr_phy + 0x0214);

	reg = readl(eq->ptr_phy + 0x120C); // auto AEQ
	reg &= ~(1<<2);
	writel(reg, eq->ptr_phy + 0x120C);
}

static void do_aeq_rc(struct nvt_plat_pcie_link_eq *eq)
{
	u32 reg;

	// AEQ R/C frz = 0
	reg = readl(eq->ptr_phy + 0x0214);
	reg |= 1<<5;
	reg |= 1<<6;
	writel(reg, eq->ptr_phy + 0x0214);

	// Manual AEQ enable
	reg = readl(eq->ptr_phy + 0x120C);
	reg |= 1<<2;
	writel(reg, eq->ptr_phy + 0x120C);

	// Manual R on
	reg = readl(eq->ptr_phy + 0x1000);
	reg |= 1<<30;
	writel(reg, eq->ptr_phy + 0x1000);

	udelay(65);

	// Manual R off
	reg = readl(eq->ptr_phy + 0x1000);
	reg &= ~(1<<30);
	writel(reg, eq->ptr_phy + 0x1000);

	// Manual C on
	reg = readl(eq->ptr_phy + 0x1000);
	reg |= 1<<0;
	writel(reg, eq->ptr_phy + 0x1000);

	udelay(65);

	// Manual C off
	reg = readl(eq->ptr_phy + 0x1000);
	reg &= ~(1<<0);
	writel(reg, eq->ptr_phy + 0x1000);

	// AEQ frz = 1
	reg = readl(eq->ptr_phy + 0x0214);
	reg &= ~(1<<5);
	reg &= ~(1<<6);
	writel(reg, eq->ptr_phy + 0x0214);

	// auto AEQ
	reg = readl(eq->ptr_phy + 0x120C);
	reg &= ~(1<<2);
	writel(reg, eq->ptr_phy + 0x120C);
}

static void fom_cal_sw_trigger(struct nvt_plat_pcie_link_eq *eq)
{
	u32 reg;

	//cfg_fom_cal_sw_trg

	//clear
	reg = readl(eq->ptr_phy + 0x102C);
	reg &= ~(1<<17);
	writel(reg, eq->ptr_phy + 0x102C);

	//set
	reg = readl(eq->ptr_phy + 0x102C);
	reg |= (1<<17);
	writel(reg, eq->ptr_phy + 0x102C);

	//clear
	reg = readl(eq->ptr_phy + 0x102C);
	reg &= ~(1<<17);
	writel(reg, eq->ptr_phy + 0x102C);
}
#endif

#define DETECT_QUIET	0x00
#define RCVRY_LOCK		0x0D
#define RCVRY_SPEED		0x0E
#define L0				0x11
#define RCVRY_EQ0		0x20
#define RCVRY_EQ1		0x21
#define RCVRY_EQ2		0x22
#define RCVRY_EQ3		0x23
#define HOT_RESET		0x1F

static int nvt_pcie_link_up(struct dw_pcie *pci);
static int wait_speed_change(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);
	struct nvt_plat_pcie_link_eq nvt_link_eq;
	u64 start_t;
	u64 curr_t;
	u64 diff_t;
	u64 state_start_t = 0;
	u32 val;
	u32 last_ltssm = DETECT_QUIET;
	int ret = 0;
	int	is_reset_done = 0;
#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
	int is_preset_handled = 0;
#endif

	start_t = ktime_get_real_ns();
	nvt_link_eq.ptr_phy = nvt_pcie->ptr_phy;

	while (1) {
#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
		u32 reg;
#endif
		u32 curr_ltssm;

		val = readl(nvt_pcie->top_base + 0x08);
		curr_ltssm = (val >> 4) & 0x3F;

		if (curr_ltssm == L0) break;

		if (last_ltssm != curr_ltssm) {
			state_start_t = ktime_get_real_ns();
			is_reset_done = 0;
			//printk("%s: 0x%x->0x%x\r\n", __func__, last_ltssm, curr_ltssm);
			last_ltssm = curr_ltssm;
		}

		switch (curr_ltssm) {
		case L0:

		    break;
		case RCVRY_EQ0:

		    break;
		case RCVRY_EQ1:
			if ((ktime_get_real_ns() - state_start_t) > 4000000) {	// > 1ms
				if (is_reset_done == 0) {
					is_reset_done = 1;

					phy_set_mode(nvt_pcie->phy, PHY_MODE_INVALID);
					printk("%s: reset mux\r\n", __func__);
				}
			}
			break;
		case RCVRY_EQ3:
#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
		    if(nvt_pcie->auto_preset &&(is_preset_handled == 0)) {
				u32 local_fom = 0;
				u32 max_fom_value = 0;
				u32 max_fom_p_idx = 0;
				u32 i;

				is_preset_handled = 1;

				for(i = 0; i < PRESET_COUNT; i++) {
					nvt_link_eq.v_fom_history[i] = 0;
					nvt_link_eq.v_history_R[i] = 0;
					nvt_link_eq.v_history_C[i] = 0;
		        }

		        // #1 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 4, 0);	// initial preset (P4)
				nvt_link_eq.v_fom_history[0] = local_fom;

		        reg = readl(nvt_link_eq.ptr_phy + 0x1338);
				reg &= ~(0xFF<<0);
				reg |= local_fom<<0;
		        writel(reg, nvt_link_eq.ptr_phy + 0x1338);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				max_fom_value = local_fom;
				max_fom_p_idx = 4;

		        // #2 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 0, 1);	// P0
				nvt_link_eq.v_fom_history[1] = local_fom;

				reg = readl(nvt_link_eq.ptr_phy + 0x1338);
				reg &= ~(0xFF<<8);
				reg |= local_fom<<8;
				writel(reg, nvt_link_eq.ptr_phy + 0x1338);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 0;
				}

		        // #3 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 1, 2);	// P1
				nvt_link_eq.v_fom_history[2] = local_fom;

		        reg = readl(nvt_link_eq.ptr_phy + 0x1338);
				reg &= ~(0xFF<<16);
				reg |= local_fom<<16;
				writel(reg, nvt_link_eq.ptr_phy + 0x1338);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 1;
				}
		        // #4 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 2, 3);	// P2
				nvt_link_eq.v_fom_history[3] = local_fom;

		        reg = readl(nvt_link_eq.ptr_phy + 0x1338);
				reg &= ~(0xFF<<24);
				reg |= local_fom<<24;
				writel(reg, nvt_link_eq.ptr_phy + 0x1338);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 2;
				}
		        // #5 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 3, 4);	// P3
				nvt_link_eq.v_fom_history[4] = local_fom;

				reg = readl(nvt_link_eq.ptr_phy + 0x133C);
				reg &= ~(0xFF<<0);
				reg |= local_fom<<0;
				writel(reg, nvt_link_eq.ptr_phy + 0x133C);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 3;
				}
		        // #6 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, 4, 5);	// P4
				nvt_link_eq.v_fom_history[5] = local_fom;

				reg = readl(nvt_link_eq.ptr_phy + 0x133C);
				reg &= ~(0xFF<<8);
				reg |= local_fom<<8;
				writel(reg, nvt_link_eq.ptr_phy + 0x133C);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 4;
				}
		        // #7 TX preset
		        local_fom = manual_aeq(&nvt_link_eq, max_fom_p_idx, 6); // final preset
				nvt_link_eq.v_fom_history[6] = local_fom;

		        reg = readl(nvt_link_eq.ptr_phy + 0x133C);
				reg &= ~(0xFF<<16);
				reg |= local_fom<<16;
				writel(reg, nvt_link_eq.ptr_phy + 0x133C);
				udelay(2);

		        fom_cal_sw_trigger(&nvt_link_eq);

		        restore_aeq(&nvt_link_eq);
				do_aeq_rc(&nvt_link_eq);

				printk(KERN_CONT "pick preset %d\r\nFOM: ", max_fom_p_idx);
				for (i = 0; i < PRESET_COUNT; i++) {
					printk(KERN_CONT "%d ", nvt_link_eq.v_fom_history[i]);
				}
		        printk(KERN_CONT "\r\nR: ");
				for (i = 0; i < PRESET_COUNT; i++) {
					printk(KERN_CONT "%d.", nvt_link_eq.v_history_R[i]);
				}
		        printk(KERN_CONT "\r\n");

		        reg = readl(nvt_link_eq.ptr_phy + 0x0A00);
				printk("Final R 0x%x\r\n", (reg>>8) & 0x7);
				printk("Final C 0x%x\r\n", (reg>>12) & 0x7);
		    }
#endif
		    break;
		case RCVRY_LOCK:

		    break;
		case HOT_RESET:

		    break;
		default:
			break;
	    }
	}

	curr_t = ktime_get_real_ns();

	diff_t = curr_t - start_t;
	printk("%s: elps %lld ns, resolution %d ns (start %lld, end %lld)\r\n", __func__, diff_t, ktime_get_resolution_ns(), start_t, curr_t);

	val = dw_pcie_readl_dbi(pci, LINK_CONTROL_LINK_STATUS_REG);
	printk("%s: link sts 0x%x\r\n", __func__, val);

	return ret;
}

int nvt_pcie_wait_speed_change(struct pci_dev *dev)
{

	struct pci_dev *p_root;
	struct pcie_port *pp;
	struct pci_host_bridge *bridge;

	p_root = pcie_find_root_port(dev);

	bridge = (struct pci_host_bridge *)p_root->bus->bridge;

	if (bridge->sysdata == NULL) {
		return -4;
	}
	pp = (struct pcie_port *)bridge->sysdata;

	if(pp != NULL) {
		wait_speed_change(pp);
	}

	return 0;
}
EXPORT_SYMBOL(nvt_pcie_wait_speed_change);

static int nvt_pcie_power_on(struct nvt_plat_pcie *nvt_pcie);

static int nvt_plat_pcie_host_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);
	u32 val;
	int ret = 0;
	int count = 0;
	int retry = 10;

	// Can't do init retry because phy is shared by 2 controller in 690.
	for (retry = 1; retry >= 1; retry--) {
		int pos;
		int peer_cap;

		count++;

		ret = nvt_pcie_power_on(nvt_pcie);
		if (ret)
			return ret;

		//
		// start ltssm (link layer)
		//

		// enable link training
		val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);
		val |= FIELD_PREP(NVT_PCIE_LINK_TRAIN_MSK, NVT_PCIE_LINK_TRAIN_EN);
		writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);

		ret = dw_pcie_wait_for_link(pci);
		if (ret < 0) {
			nvt_pcie->link_stat = PCIE_LINK_STAT_ERR;
			return ret;
		}

		//
		// setup RC controller
		//
		dw_pcie_setup_rc(pp);
		dw_pcie_wait_for_link(pci);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,00)
		nvt_plat_setup_atu(pp);
#else
		dw_pcie_iatu_detect(pci);
#endif

		pos = nvt_pcie_get_speed_cap(pp);
		switch (pos) {
		case PCIE_SPEED_2_5GT:
			peer_cap = PCI_EXP_LNKCTL2_TLS_2_5GT;
			break;
		case PCIE_SPEED_5_0GT:
			peer_cap = PCI_EXP_LNKCTL2_TLS_5_0GT;
			break;
		case PCIE_SPEED_8_0GT:
			peer_cap = PCI_EXP_LNKCTL2_TLS_8_0GT;
			break;
		case PCIE_SPEED_16_0GT:
			peer_cap = PCI_EXP_LNKCTL2_TLS_16_0GT;
			break;
		default:
			peer_cap = 0;
			break;
		}

		// prepare higher speed
		if (nvt_pcie->link_gen != PCI_EXP_LNKCTL2_TLS_2_5GT) {
			// setup target speed, and will take effect after dw_pcie_setup_rc()
			val = dw_pcie_readl_dbi(pci, LINK_CONTROL2_LINK_STATUS2_REG);
			val &= ~PCI_EXP_LNKCTL2_TLS;
			val |= nvt_pcie->link_gen;
			dw_pcie_writel_dbi(nvt_pcie->pci, LINK_CONTROL2_LINK_STATUS2_REG, val);
		} else {
			val = dw_pcie_readl_dbi(pci, LINK_CONTROL2_LINK_STATUS2_REG);
			val &= ~PCI_EXP_LNKCTL2_TLS;
			val |= PCI_EXP_LNKCTL2_TLS_2_5GT;
			dw_pcie_writel_dbi(nvt_pcie->pci, LINK_CONTROL2_LINK_STATUS2_REG, val);// comment to support EP issue speed change
		}

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
		break;
#else
		wait_speed_change(pp);
		dw_pcie_wait_for_link(pci);

		val = dw_pcie_readl_dbi(pci, LINK_CONTROL_LINK_STATUS_REG);
		val = (val >> 16) & 0x0F;
		if (peer_cap > nvt_pcie->link_gen) peer_cap = nvt_pcie->link_gen;

		if (val == peer_cap) break;
#endif
	}

	printk("%s: speed change retry count %d\r\n", __func__, count);

	if (retry < 0) {
		printk("%s: switch to higher speed timeout\r\n", __func__);
	}

	ret = nvt_plat_pcie_parse_map_dma_ranges(pp);
	if (ret < 0) {
		nvt_pcie->link_stat = PCIE_LINK_STAT_RC_INB_ERR;
		return -1;
	}

	nvt_plat_set_rc_bar0(pci);

	// nvt: linux 5-15 invokes dw_pcie_msi_init in dw_pcie_setup_rc() at pcie-designware-host.c
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,00)
	dw_pcie_msi_init(pp);
#endif

	nvt_pcie->link_stat = PCIE_LINK_STAT_OK;
	return 0;
}

/* Chained MSI interrupt service routine */
static void nvt_chained_msi_isr(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
//	struct pcie_port *pp;
//	struct dw_pcie *pci;
	struct nvt_plat_pcie *nvt_pcie;
	unsigned int virq = 0;
	u32 val;

	nvt_pcie = irq_desc_get_handler_data(desc);
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_STS_OFS);

	chained_irq_enter(chip, desc);

	//EDMA
	if (val & NVT_PCIE_TOP_INT_EDMA_MSK) {
#ifdef CONFIG_NVT_IVOT_PLAT_NA51090
		struct dw_pcie *pci = nvt_pcie->pci;
		struct pcie_port *pp = &pci->pp;
		nvt_pcie_drv_isr(pp->msi_irq, pci);
#else
		virq = irq_find_mapping(nvt_pcie->top_irq_domain, 22);
		generic_handle_irq(virq);
#endif
	}

	//MSI
	if (val & NVT_PCIE_TOP_INT_MSI_MSK) {
		virq = irq_find_mapping(nvt_pcie->top_irq_domain, 26);
		generic_handle_irq(virq);
	}

	chained_irq_exit(chip, desc);

	// link_down_rst_req
	if (val & NVT_PCIE_TOP_INT_LINK_DONW_MSK) {
		printk("PCIE TOP ISR(%s): link down\r\n", dev_name(nvt_pcie->pci->dev));

		// Disable link_down_rst_req INTEN
		val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
		val |= NVT_PCIE_TOP_INT_LINK_DONW_MSK;
		writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);

		// trigger bottom half
		nvt_pcie->linkdown_hdl_data.footprint = 0;
		schedule_delayed_work(&nvt_pcie->deffered_linkdown, 1);
	}
}

/*static void nvt_plat_set_num_vectors(struct pcie_port *pp)
{
	pp->num_vectors = MAX_MSI_IRQS;
}*/


// nvt: .rd_other_conf is remvoed from struct dw_pcie_host_ops in 5.15
#if 0
int nvt_plat_rd_other_conf(struct pcie_port *pp, struct pci_bus *bus,
			   u32 devfn, int where, int size, u32 *val)
{
	//This function is almost the same as dw_pcie_rd_other_conf,
	//but adding nvt_plat_set_ep_bar0 to set ep bar 0 before bus scan
	int ret;
	u32 busdev;
	struct pcie_port *pp = bus->sysdata;
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);

	busdev = PCIE_ATU_BUS(bus->number) | PCIE_ATU_DEV(PCI_SLOT(devfn)) |
		 PCIE_ATU_FUNC(PCI_FUNC(devfn));

	ret = pci_generic_config_read(bus, devfn, where, size, val);

	//add some code here (relatively to dw_pcie_rd_other_conf)
	if ((PCI_VENDOR_ID == where) &&
	    (PCI_VENDOR_ID_NOVATEK == PCI_GET_VENDOR_ID(*val))) {
		nvt_plat_set_ep_bar0(pci, pp->va_cfg0_base, busdev);
	}

	if (!ret && pci->io_cfg_atu_shared)
		dw_pcie_prog_outbound_atu(pci, 0, PCIE_ATU_TYPE_IO, pp->io_base,
					  pp->io_bus_addr, pp->io_size);

	return ret;
}
#endif

static const struct dw_pcie_host_ops nvt_plat_pcie_host_ops = {
	.host_init = nvt_plat_pcie_host_init,
};

static int nvt_plat_pcie_establish_link(struct dw_pcie *pci)
{
	return 0;
}

static int nvt_pcie_link_up(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, LINK_CONTROL_LINK_STATUS_REG);

	return (val & PCIE_CAP_DLL_ACTIVE_BIT);
}

static const struct dw_pcie_ops nvt_pcie_ops = {
	.start_link = nvt_plat_pcie_establish_link,
	.link_up = nvt_pcie_link_up,
};

static u32 nvt_elbi_read(struct pci_dev *dev)
{
	u32 val;
	u32 reg;

	pci_read_config_dword(dev, ELBI_BASE_OFS + 0x10, &val);
	val |= 1<<16;		// enable auto mode
	pci_write_config_dword(dev, ELBI_BASE_OFS + 0x10, val);

	// setup address MSB
	pci_read_config_dword(dev, ELBI_BASE_OFS + 0x00, &val);
	val &= ~(0xF<<16);
	val |= upper_32_bits(NVT_TOP_BASE_PHYS)<<16;
	pci_write_config_dword(dev, ELBI_BASE_OFS + 0x00, val);

	// setup address LSB
	val= lower_32_bits(NVT_TOP_BASE_PHYS);
	pci_write_config_dword(dev, ELBI_BASE_OFS + 0x08, val);

	// read
	pci_read_config_dword(dev, ELBI_BASE_OFS + 0x04, &reg);

	pci_read_config_dword(dev, ELBI_BASE_OFS + 0x10, &val);
	val &= ~(1<<16);	// close ELBI
	pci_write_config_dword(dev, ELBI_BASE_OFS + 0x10, val);

	return reg;
}

static void nvt_pcie_fixup_class(struct pci_dev *dev)
{
	u32 val;
	u32 ep_width_config;
	u32 ep_boot_source = 0;
	u32 width;
	u32 bar_sz;

	pci_read_config_dword(dev, RESBAR_CTRL_REG_0_REG_OFS, &val);
	bar_sz = (val>>8) & 0x3F;
	if (bar_sz == 0x0D) {
		printk("%s: default bar size 8GB, shrink to 32MB\r\n", __func__);
		bar_sz = 0x05;	// If default 8GB, set it to 32 MB
	}

	/* read link capbility */
	pci_read_config_dword(dev, dev->pcie_cap + PCI_EXP_LNKCTL, &val);
	val >>= 16;
	width = (val&PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
	pci_read_config_dword(dev, PORT_LINK_CTRL_OFF_OFS, &val);
	ep_width_config = FIELD_GET(LINK_CAPABLE_MSK, val);
	printk("%s: PORT_LINK_CTRL_OFF 0x%x, width mask %d\r\n", __func__, val, ep_width_config);
	val = nvt_elbi_read(dev);
	ep_boot_source = val & 0xF;

	// if EP cap is x2 and EP boot source is pcie(no flash)
	if ((ep_width_config==0x3) && (ep_boot_source==0xD)) {

		// EP bootstrap is x2, but get x1 result
		if (width == 1) {
			printk("%s: EP BST[3..0] = 0x%x, BST[16] is x2, but curr width %d\r\n", __func__, ep_boot_source, width);

			pci_write_config_dword(dev, SD_CONTROL1_REG_OFS, FORCE_DETECT_LANE_EN_MSK|FORCE_DETECT_LANE_X2_MSK);
			pci_read_config_dword(dev, SD_CONTROL1_REG_OFS, &val);
			printk("%s: apply x2 patch and reset...\r\n", __func__);

			pci_reset_bus(dev);

			msleep(100);

			pci_read_config_dword(dev, PCI_VENDOR_ID, &val);
			val &= 0xFFFF;
			printk("%s: Done. Vendor id 0x%x\r\n", __func__, val);
			if (val != PCI_VENDOR_ID_NOVATEK) {
				printk("%s: 0x%x not NOVATEK vendor ID, reset fail\r\n", __func__, val);
				return;
			}
		}
	}

	pci_read_config_dword(dev, RESBAR_CTRL_REG_0_REG_OFS, &val);
	val = (val&0xFF)|(bar_sz<<8);	// modify BAR0 size
	pci_write_config_dword(dev, RESBAR_CTRL_REG_0_REG_OFS, val);

}
DECLARE_PCI_FIXUP_CLASS_EARLY(PCI_VENDOR_ID_NOVATEK, PCI_DEVICE_ID_NOVATEK_98690, 0xff00, 8, nvt_pcie_fixup_class);

static void nvt_pcie_fixup_class_336(struct pci_dev *dev)
{
	u32 val;
	u32 ep_width_config;
	u32 bar_sz;

	pci_read_config_dword(dev, RESBAR_CTRL636_REG_0_REG_OFS, &val);
	bar_sz = (val>>8) & 0x3F;
	if (bar_sz == 0x0D) {
		printk("%s: default bar size 8GB, shrink to 32MB\r\n", __func__);
		bar_sz = 0x05;	// If default 8GB, set it to 32 MB
	}

	/* read link capbility */
	pci_read_config_dword(dev, PORT_LINK_CTRL_OFF_OFS, &val);
	ep_width_config = FIELD_GET(LINK_CAPABLE_MSK, val);
	printk("%s: PORT_LINK_CTRL_OFF 0x%x, width mask %d\r\n", __func__, val, ep_width_config);

	pci_read_config_dword(dev, RESBAR_CTRL636_REG_0_REG_OFS, &val);
	val = (val&0xFF)|(bar_sz<<8);	// modify BAR0 size
	pci_write_config_dword(dev, RESBAR_CTRL636_REG_0_REG_OFS, val);

}
DECLARE_PCI_FIXUP_CLASS_EARLY(PCI_VENDOR_ID_NOVATEK, PCI_DEVICE_ID_NOVATEK_98336, 0xff00, 8, nvt_pcie_fixup_class_336);

static void nvt_pcie_fixup_class_635(struct pci_dev *dev)
{
	u32 val;
	u32 ep_width_config;
	u32 bar_sz;

	pci_read_config_dword(dev, RESBAR_CTRL635_REG_0_REG_OFS, &val);
	bar_sz = (val>>8) & 0x3F;
	if (bar_sz == 0x0D) {
		printk("%s: default bar size 8GB, shrink to 32MB\r\n", __func__);
		bar_sz = 0x05;	// If default 8GB, set it to 32 MB
	}

	/* read link capbility */
	pci_read_config_dword(dev, PORT_LINK_CTRL_OFF_OFS, &val);
	ep_width_config = FIELD_GET(LINK_CAPABLE_MSK, val);
	printk("%s: PORT_LINK_CTRL_OFF 0x%x, width mask %d\r\n", __func__, val, ep_width_config);

	pci_read_config_dword(dev, RESBAR_CTRL635_REG_0_REG_OFS, &val);
	val = (val&0xFF)|(bar_sz<<8);	// modify BAR0 size
	pci_write_config_dword(dev, RESBAR_CTRL635_REG_0_REG_OFS, val);

}
DECLARE_PCI_FIXUP_CLASS_EARLY(PCI_VENDOR_ID_NOVATEK, PCI_DEVICE_ID_NOVATEK_98635, 0xff00, 8, nvt_pcie_fixup_class_635);

static void quirk_disable_aspm_l0s(struct pci_dev *dev)
{
	pci_info(dev, "Disabling ASPM L0s\n");
	pci_disable_link_state(dev, PCIE_LINK_STATE_L0S);
}

DECLARE_PCI_FIXUP_CLASS_FINAL(PCI_VENDOR_ID_NOVATEK, PCI_DEVICE_ID_NOVATEK_98690, 0xff00, 8, quirk_disable_aspm_l0s);

static void nvt_plat_pcie_ep_init(struct dw_pcie_ep *ep)
{
	struct dw_pcie *pci = to_dw_pcie_from_ep(ep);
//	struct pci_epc *epc = ep->epc;
	enum pci_barno bar;

	for (bar = BAR_0; bar <= BAR_5; bar++)
		dw_pcie_ep_reset_bar(pci, bar);

	// nvt: EPC_FEATURE_NO_LINKUP_NOTIFIER is removed from 5.15
//	epc->features |= EPC_FEATURE_NO_LINKUP_NOTIFIER;
//	epc->features |= EPC_FEATURE_MSIX_AVAILABLE;
}

static int nvt_plat_pcie_ep_raise_irq(struct dw_pcie_ep *ep, u8 func_no,
				      enum pci_epc_irq_type type,
				      u16 interrupt_num)
{
	struct dw_pcie *pci = to_dw_pcie_from_ep(ep);

	switch (type) {
	case PCI_EPC_IRQ_LEGACY:
		return dw_pcie_ep_raise_legacy_irq(ep, func_no);
	case PCI_EPC_IRQ_MSI:
		return dw_pcie_ep_raise_msi_irq(ep, func_no, interrupt_num);
	case PCI_EPC_IRQ_MSIX:
		return dw_pcie_ep_raise_msix_irq(ep, func_no, interrupt_num);
	default:
		dev_err(pci->dev, "UNKNOWN IRQ type\n");
	}

	return 0;
}

static struct dw_pcie_ep_ops pcie_ep_ops = {
	.ep_init = nvt_plat_pcie_ep_init,
	.raise_irq = nvt_plat_pcie_ep_raise_irq,
};

static int nvt_plat_add_pcie_port(struct nvt_plat_pcie *nvt_plat_pcie,
				  struct platform_device *pdev)
{
	struct dw_pcie *pci = nvt_plat_pcie->pci;
	struct pcie_port *pp = &pci->pp;
	struct device *dev = &pdev->dev;
	int ret;

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = platform_get_irq(pdev, 0);
		if (pp->msi_irq < 0) {
			dev_err(dev, "Can't find irq, plz check dts setting interrupt-parent\r\n");
			return pp->msi_irq;
		}
	}

	pp->ops = &nvt_plat_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "Failed to initialize host\n");
		return ret;
	}

	// inbound iATU would be corrupted in dw_pcie_host_init. Redo it again.
	ret = nvt_plat_pcie_parse_map_dma_ranges(pp);

	return ret;
}

static int nvt_plat_add_pcie_ep(struct nvt_plat_pcie *nvt_plat_pcie,
				struct platform_device *pdev)
{
	int ret;
	struct dw_pcie_ep *ep;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct dw_pcie *pci = nvt_plat_pcie->pci;

	ep = &pci->ep;
	ep->ops = &pcie_ep_ops;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi2");
	pci->dbi_base2 = devm_ioremap_resource(dev, res);
	if (IS_ERR(pci->dbi_base2))
		return PTR_ERR(pci->dbi_base2);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "addr_space");
	if (!res)
		return -EINVAL;

	ep->phys_base = res->start;
	ep->addr_size = resource_size(res);

	ret = dw_pcie_ep_init(ep);
	if (ret) {
		dev_err(dev, "Failed to initialize endpoint\n");
		return ret;
	}
	return 0;
}

static int nvt_pcie_phy_init(struct nvt_plat_pcie *nvt_pcie)
{
	struct device *dev = nvt_pcie->pci->dev;
	int err;

	printk("%s: phy_init\r\n", __func__);

	err = phy_set_mode_ext(nvt_pcie->phy ,PHY_MODE_PCIE ,nvt_pcie->bs_lane_width);
	if (err) {
                dev_err(dev, "failed to set lane mode\n");
                //return -EIO;
        }

	err = phy_init(nvt_pcie->phy);
	if (err) {
		dev_err(dev, "failed to initialize phy\n");
		return -EIO;
	}

	err = phy_power_on(nvt_pcie->phy);
	if (err) {
		dev_err(dev, "failed to power on phy\n");
		return -EIO;
	}

	return 0;
}

#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
static int nvt_pcie_phy_init_aeq(void)
{
	u32 reg;
	void __iomem *ptr_pcie_phy = ioremap(NVT_PCIE_PHY_BASE_PHYS, 0x1000);;

	// disable AEQ R/C split
	reg = readl(ptr_pcie_phy + 0x091C);
	reg &= ~(1<<16);
	writel(reg, ptr_pcie_phy + 0x091C);

	// tune R
	reg = readl(ptr_pcie_phy + 0x00AC);
	reg &= ~(1<<22);
	writel(reg, ptr_pcie_phy + 0x00AC);

	// manual fix C
	reg = readl(ptr_pcie_phy + 0x00AC);
	reg |= (1<<23);
	writel(reg, ptr_pcie_phy + 0x00AC);

	// fix C = 4
	writel(reg, ptr_pcie_phy + 0x00A8);
	reg &= ~(0x7<<25);
	reg |= (0x4<<25);
	writel(reg, ptr_pcie_phy + 0x00A8);

	iounmap(ptr_pcie_phy);

	return 0;
}
#endif

static int nvt_pcie_power_on(struct nvt_plat_pcie *nvt_pcie)
{
	int ret = 0;
	u32 val;

	// enable AXI-M non-bufferable
#if !IS_ENABLED(CONFIG_PCIE_TLP_ORDER_WEAK)
	val = readl(nvt_pcie->top_base + NVT_PCIE_SOC_CTRL0_OFS);
	val |= NVT_PCIE_AXIM_NON_BUFF_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_SOC_CTRL0_OFS);
#endif

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_DEBUG_PIPE_CTL1_REG_OFS);
	val |= NVT_PCIE_TOP_FAKE_PHY_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_DEBUG_PIPE_CTL1_REG_OFS);
#endif

	// assert PCIe controller reset signals
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val &= ~NVT_PCIE_TOP_RESET_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// disable train_en while reset asserted
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);
	val &= ~NVT_PCIE_LINK_TRAIN_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);

#if 0	// ALL RC port shares the same PRESET, don't affects others
	// assert PRESET
	nvt_plat_set_preset(nvt_pcie, true);
#endif

	// release PCIe phy register reset
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val |= NVT_PCIE_TOP_PHY_APBRST_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// release shared (by N controller) PHY_PORn
	spin_lock(&spin_init_lock);
	porn_count++;
	spin_unlock(&spin_init_lock);
	printk("%s: PORn count %d, dts count %d\r\n", __func__, porn_count, nvt_pcie->dts_count);

	// output refclk (if specified)
	if (nvt_pcie->refclk_out) {
		nvt_plat_refclkout(nvt_pcie, true);

		val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_LOCAL_CLK_OFS);
		val |= FIELD_PREP(NVT_PCIE_REFCLK_MSK, NVT_PCIE_REFCLK_OUT_EN);
		writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_LOCAL_CLK_OFS);

		usleep_range(100, 200);
	}

	if (is_phy_init_done == 0) {
		is_phy_init_done = 1;

		down(&sem_ctrls_lock);

		// apply phy setting while PHY PORn
		nvt_pcie_phy_init(nvt_pcie);

		if (!IS_ERR(nvt_pcie->p_clk_phy_porn)) {
			clk_prepare_enable(nvt_pcie->p_clk_phy_porn);
		}

		up(&sem_ctrls_lock);
	}

	// release PCIe controller reset signals
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);
	val |= NVT_PCIE_TOP_RESET_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL0_OFS);

	// Force EQ preset
	val = dw_pcie_readl_dbi(nvt_pcie->pci, GEN3_EQ_CONTROL_OFS);
	val &= ~GEN3_EQ_PSET_REQ_VEC_MSK;
	if (nvt_pcie->auto_preset) {
		val |= FIELD_PREP(GEN3_EQ_PSET_REQ_VEC_MSK, GEN3_EQ_AUTO_PSET_REQ_VEC_NVT);
	} else {
		val |= FIELD_PREP(GEN3_EQ_PSET_REQ_VEC_MSK, GEN3_EQ_PSET_REQ_VEC_NVT);
		val &= ~GEN3_EQ_FOM_INC_INITIAL_MSK;
	}
	dw_pcie_writel_dbi(nvt_pcie->pci, GEN3_EQ_CONTROL_OFS, val);

#if (!IS_ENABLED(CONFIG_NVT_FPGA_EMULATION))
	//init aeq
	if (nvt_pcie->auto_preset) {
		nvt_pcie_phy_init_aeq();
	}
#endif

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	val = dw_pcie_readl_dbi(nvt_pcie->pci, PIPE_LOOPBACK_CONTROL_OFF_OFS);
	val |= PIPE_LOOPBACK_MSK;
	dw_pcie_writel_dbi(nvt_pcie->pci, PIPE_LOOPBACK_CONTROL_OFF_OFS, val);

	val = dw_pcie_readl_dbi(nvt_pcie->pci, PORT_LINK_CTRL_OFF_OFS);
	val |= LOOPBACK_ENABLE_MSK;
	dw_pcie_writel_dbi(nvt_pcie->pci, PORT_LINK_CTRL_OFF_OFS, val);
#endif

	val = dw_pcie_readl_dbi(nvt_pcie->pci, PORT_FORCE_OFF_OFS);
	val |= SUPPORT_PART_LANES_MSK;
	dw_pcie_writel_dbi(nvt_pcie->pci, PORT_FORCE_OFF_OFS, val);

	// CTS 4.2.6.7.2. L1.Idle
	val = dw_pcie_readl_dbi(nvt_pcie->pci, GEN3_RELATED_OFS);
	val &= ~GEN3_ZRXDC_NONCOMPL_MSK;
	dw_pcie_writel_dbi(nvt_pcie->pci, GEN3_RELATED_OFS, val);

	/* Only support ASPM L1 */
	dw_pcie_dbi_ro_wr_en(nvt_pcie->pci);
	val = dw_pcie_readl_dbi(nvt_pcie->pci, LINK_CAPABILITIES_REG);
	val &= ~PCI_EXP_LNKCAP_ASPMS;
	val |= PCI_EXP_LNKCAP_ASPM_L1;
	dw_pcie_writel_dbi(nvt_pcie->pci, LINK_CAPABILITIES_REG, val);
	dw_pcie_dbi_ro_wr_dis(nvt_pcie->pci);

	// Force detect
	if (nvt_pcie->bs_lane_width >= 2) {
		dw_pcie_dbi_ro_wr_en(nvt_pcie->pci);

		dw_pcie_writel_dbi(nvt_pcie->pci, SD_CONTROL1_REG_OFS, FORCE_DETECT_LANE_EN_MSK|FORCE_DETECT_LANE_X2_MSK);
		val = dw_pcie_readl_dbi(nvt_pcie->pci, SD_CONTROL1_REG_OFS);

		dw_pcie_dbi_ro_wr_dis(nvt_pcie->pci);
	}

	// release PRESET
	nvt_plat_set_preset(nvt_pcie, false);

	msleep(200);

	return ret;
}

static void nvt_plat_defferred_linkdown(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct nvt_plat_pcie *nvt_pcie =
		container_of(delayed_work, struct nvt_plat_pcie, deffered_linkdown);
	struct dw_pcie *pci = nvt_pcie->pci;
	u32 val;
	u64 start_t;
	u64 curr_t;
	unsigned long flags;

	start_t = ktime_get_real_ns();

	if (nvt_pcie->linkdown_hdl_data.footprint == 0) {
		nvt_pcie->linkdown_hdl_data.footprint = 1;

		phy_reset(nvt_pcie->phy);

		// assert link_down_rst_ack, let it goto POLLING state
		val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);
		val |= FIELD_PREP(NVT_PCIE_LINK_DOWN_RST_ACK_MSK, 1);
		writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);
	}

	if ((readl(nvt_pcie->top_base + NVT_PCIE_TOP_DMC_STS0_OFS) & NVT_PCIE_TOP_DMC_LINK_MSK) == 0) {
		// If link is still down, check it at next round.
		schedule_delayed_work(&nvt_pcie->deffered_linkdown, 1);
		return;
	}

	// ensure not been disrupted by interrupt
	spin_lock_irqsave(&nvt_pcie->linkdown_hdl_data.lock, flags);

	// clear link_down_rst_ack
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);
	val &= ~NVT_PCIE_LINK_DOWN_RST_ACK_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_CTRL1_OFS);

	// Re-enable link_down_rst_req INTEN
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	val &= ~NVT_PCIE_TOP_INT_LINK_DONW_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);

	start_t = ktime_get_real_ns();

	// prepare higher speed
	if (nvt_pcie->link_gen != PCI_EXP_LNKCTL2_TLS_2_5GT) {
		val = dw_pcie_readl_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL);
		val |= PORT_LOGIC_SPEED_CHANGE;
		dw_pcie_writel_dbi(nvt_pcie->pci, PCIE_LINK_WIDTH_SPEED_CONTROL, val);

		wait_speed_change(&nvt_pcie->pci->pp);
	}

#ifdef CONFIG_NVT_IVOT_PLAT_NS02201
	nvt_edma_reinit();
#endif

	spin_unlock_irqrestore(&nvt_pcie->linkdown_hdl_data.lock, flags);

	curr_t = ktime_get_real_ns();
//	printk("%s: elps %lld ns\r\n", __func__, curr_t-start_t);

	// end of polling and patch (It will be triggered from ISR next time)
}

#if 1

static void pcie_top_mask_irq(struct irq_data *data)
{
	u32 val;
	struct nvt_plat_pcie *nvt_pcie = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	spin_lock_irqsave(&nvt_pcie->lock_int, flags);

	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	val |= 1<<data->hwirq;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);

	spin_unlock_irqrestore(&nvt_pcie->lock_int, flags);
}

static void pcie_top_unmask_irq(struct irq_data *data)
{
	u32 val;
	struct nvt_plat_pcie *nvt_pcie = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	spin_lock_irqsave(&nvt_pcie->lock_int, flags);

	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	val &= ~(1<<data->hwirq);
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);

	spin_unlock_irqrestore(&nvt_pcie->lock_int, flags);
}

static void pcie_top_ack(struct irq_data *data)
{
	return;
}

static struct irq_chip pcie_top_irq_chip = {
	.name = "Novatek PCIe MSIX",
	.irq_enable = pcie_top_unmask_irq,
	.irq_disable = pcie_top_mask_irq,
	.irq_mask = pcie_top_mask_irq,
	.irq_unmask = pcie_top_unmask_irq,
	.irq_ack = pcie_top_ack,
};

static int pcie_top_intx_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &pcie_top_irq_chip, handle_edge_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static const struct irq_domain_ops pcie_top_domain_ops = {
	.map = pcie_top_intx_map,
};

static int nvt_plat_pcie_irq_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct device_node *np = pci->dev->of_node;
	struct device_node *np_intc;
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);
	int ret = 0;
	u32 val = 0;

	/* Disable non-msi isr, enable msi isr only (nvt top pcie) */
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	val = NVT_PCIE_TOP_INT_ALL_MSK;
	val &= ~NVT_PCIE_TOP_INT_LINK_DONW_MSK;
	val &= ~NVT_PCIE_TOP_INT_MSI_MSK;
	val &= ~NVT_PCIE_TOP_INT_EDMA_MSK;
	writel(val, nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	val = readl(nvt_pcie->top_base + NVT_PCIE_TOP_INT_MASK_OFS);
	dev_info(pci->dev, "NVT PCIE TOP INT MASK: 0x%08x \n", val);

	np_intc = of_get_child_by_name(np, "pcie-top-intc");
	if (!np_intc) {
		dev_err(pci->dev, "Failed to get pcie-top-intc node\n");
		return -EINVAL;
	}

	pp->irq = irq_of_parse_and_map(np_intc, 0);
	if (!pp->irq) {
		dev_err(pci->dev, "Failed to get an IRQ entry in pcie-top-intc\n");
		ret = -EINVAL;
		goto out_put_node;
	}

	nvt_pcie->top_irq_domain = irq_domain_add_linear(np_intc, 32,
						  &pcie_top_domain_ops, nvt_pcie);
	if (!nvt_pcie->top_irq_domain) {
		ret = -ENOMEM;
		goto out_put_node;
	}

	irq_set_chained_handler_and_data(pp->irq, nvt_chained_msi_isr, nvt_pcie);

out_put_node:
	of_node_put(np_intc);
	return ret;
}
#endif

static int nvt_plat_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dw_pcie *pci;
	struct resource *res;  /* Resource from DT */
	int ret;
#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	int num_lanes;
#endif
	int refclk_out = 0;
	int exclude_ib = 0;
	int dtscount = 0;
	u32 slot;
	struct nvt_plat_pcie *nvt_plat_pcie = NULL;
	const struct of_device_id *match;
	const struct nvt_plat_pcie_of_data *data;
	enum dw_pcie_device_mode mode;
	struct device_node *node;

	for_each_compatible_node(node, NULL, "nvt,nvt-dwc-pcie") {
		int ret;
		const char *str_status = NULL;

		ret = of_property_read_string(node, "status", &str_status);
		if (ret || (str_status == NULL) ||
			(strncmp(str_status, "disabled", strlen("disabled")) != 0)) {
	                dtscount++;
		}
	}

	printk("%s: dts count %d\r\n", __func__, dtscount);
	printk("%s: pdev->dev 0x%px\r\n", __func__, dev);
	printk("%s: dev dma range map 0x%px\r\n", __func__, dev->dma_range_map);

	spin_lock(&spin_init_lock);

	if (is_probed == 0) {
		init_completion(&init_comp);
		sema_init(&sem_ctrls_lock, 1);
		is_probed = 1;
	}

	spin_unlock(&spin_init_lock);

	match = of_match_device(nvt_plat_pcie_of_match, dev);
	if (!match)
		return -EINVAL;

	/* Get from dts private data */
	data = (struct nvt_plat_pcie_of_data *)match->data;
	/* Get from dts private data to know if it's the ep or rc mode */
	mode = (enum dw_pcie_device_mode)data->mode;

	nvt_plat_pcie = devm_kzalloc(dev, sizeof(*nvt_plat_pcie), GFP_KERNEL);
	if (!nvt_plat_pcie)
		return -ENOMEM;

	pci = devm_kzalloc(dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;

	slot = of_get_pci_domain_nr(dev->of_node);	/* get dts linux,pci-domain */
	nvt_plat_pcie->slot = slot;
	printk("%s: get slot %d\r\n", __func__, slot);

	down(&sem_ctrls_lock);
	// Check if dts setting matches with bootstrap
	ret = nvt_plat_top_init(dev, nvt_plat_pcie, mode);
	if (ret < 0) {
		return ret;
	}
	up(&sem_ctrls_lock);

	INIT_LIST_HEAD(&nvt_plat_pcie->ep_info_list_root);
	INIT_LIST_HEAD(&nvt_plat_pcie->ep_info_notifier_head);
	sema_init(&nvt_plat_pcie->sema_ep_info, 1);
	pci->dev = dev;
	pci->ops = &nvt_pcie_ops;
	pci->version = 0x600A;

	nvt_plat_pcie->pci = pci;
	nvt_plat_pcie->mode = mode;
	nvt_plat_pcie->dts_count = dtscount;

	// nvt: 5.15 dw_pcie_host_init() add parsing dbi resource
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	pci->dbi_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pci->dbi_base))
		return PTR_ERR(pci->dbi_base);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nvt-pcie-top");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	nvt_plat_pcie->top_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(nvt_plat_pcie->top_base))
		return PTR_ERR(nvt_plat_pcie->top_base);

	nvt_plat_pcie->ptr_phy = ioremap(NVT_PCIE_PHY_BASE_PHYS, 0x4000);
	if (IS_ERR(nvt_plat_pcie->ptr_phy))
		return PTR_ERR(nvt_plat_pcie->ptr_phy);

	if (!of_property_read_u32(pdev->dev.of_node, "refclk-out", &refclk_out)) {
		printk("%s: find refclk-out %d\r\n", __func__, refclk_out);
		nvt_plat_pcie->refclk_out = refclk_out;

		if (refclk_out && nvt_plat_pcie->refclk_from_pad) {
			dev_err(dev, "Bootstrap[10] = 1 means REFCLK from pad(i.e. input), but your DTS enables refclk-out\n");
			return -ETXTBSY;
		}
	}

	if (!of_property_read_u32(pdev->dev.of_node, "exclude-ib", &exclude_ib)) {
		printk("%s: find exclude-ib %d\r\n", __func__, exclude_ib);
		nvt_plat_pcie->exclude_ib = exclude_ib;
	}

	/* Limit link speed */
	nvt_plat_pcie->link_gen = PCI_EXP_LNKCTL2_TLS_2_5GT;	// defualt gen1
	if (!of_property_read_u32(pdev->dev.of_node, "max-link-speed", &nvt_plat_pcie->link_gen)) {
		if (nvt_plat_pcie->link_gen < PCI_EXP_LNKCTL2_TLS_2_5GT) {
			printk("%s: max-link-speed %d not supported, force to gen1\r\n", __func__, nvt_plat_pcie->link_gen);
			nvt_plat_pcie->link_gen = PCI_EXP_LNKCTL2_TLS_2_5GT;
		} else if (nvt_plat_pcie->link_gen > PCI_EXP_LNKCTL2_TLS_8_0GT) {
			printk("%s: max-link-speed %d not supported, force to gen3\r\n", __func__, nvt_plat_pcie->link_gen);
			nvt_plat_pcie->link_gen = PCI_EXP_LNKCTL2_TLS_8_0GT;
		}
	}
	printk("%s: confirm max-link-speed %d\r\n", __func__, nvt_plat_pcie->link_gen);

#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	/* Check lane width configuration */
	of_property_read_u32(pdev->dev.of_node, "num-lanes", &num_lanes);
	if (!num_lanes) {
		dev_warn(dev, "num-lanes not found, auto detect with bootstrap\n");
		num_lanes = nvt_plat_pcie->bs_lane_width;
	} else {
		if (num_lanes != nvt_plat_pcie->bs_lane_width) {
			dev_err(dev, "dts lane width %d, but allocated bootstrap lane width %d. Plz check bootstrap and DTS\n", num_lanes, nvt_plat_pcie->bs_lane_width);
			return -EINVAL;
		}
	}
	if (num_lanes == 0) {
		dev_err(dev, "lane width 0 is invalid\n");
		return -EINVAL;
	}
	spin_lock(&spin_init_lock);
	used_lane_width += num_lanes;
	spin_unlock(&spin_init_lock);
	if (used_lane_width > NVT_SHARED_LANE_WIDTH) {
		dev_err(dev, "Max lane width %d, but you used %d\n", NVT_SHARED_LANE_WIDTH, used_lane_width);
		return -EINVAL;
	}

	down(&sem_ctrls_lock);
	if (is_rstn_init == 0) {
		is_rstn_init = 1;

		// assert PRESET
		nvt_plat_set_preset(nvt_plat_pcie, true);
	}
	up(&sem_ctrls_lock);
#endif

	if (!of_property_read_u32(pdev->dev.of_node, "auto-preset", &nvt_plat_pcie->auto_preset)) {
		printk("%s: support auto_preset %d\r\n", __func__, nvt_plat_pcie->auto_preset);
	}

	nvt_plat_pcie->phy = devm_phy_get(dev, "pcie-phy");
	if (IS_ERR(nvt_plat_pcie->phy)) {
		dev_err(dev, "DTS node pcie-phy is NOT found, PCIe can not work\n");
		if (PTR_ERR(nvt_plat_pcie->phy) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		nvt_plat_pcie->phy = NULL;
	}

	nvt_plat_pcie->p_clk_phy_porn = devm_clk_get(dev, "PHY_PORN");
	if (IS_ERR(nvt_plat_pcie->p_clk_phy_porn)) {
		dev_warn(dev, "clk PHY_PORN not found\n");
	}

	nvt_plat_pcie->p_clk_ref = devm_clk_get(dev, "pcie_refclk");
	if (IS_ERR(nvt_plat_pcie->p_clk_ref)) {
		dev_warn(dev, "clk pcie_refclk not found\n");
	} else {
		printk("%s: enable refclk\r\n", __func__);
		clk_prepare_enable(nvt_plat_pcie->p_clk_ref);
	}

	nvt_plat_pcie->p_clk_top = devm_clk_get(dev, "pcie_clk");
	if (IS_ERR(nvt_plat_pcie->p_clk_top)) {
		dev_warn(dev, "clk pcie_clk not found\n");
	} else {
		printk("%s: enable pcie top clk\r\n", __func__);
		clk_prepare_enable(nvt_plat_pcie->p_clk_top);
	}

	spin_lock_init(&nvt_plat_pcie->lock_int);
	spin_lock_init(&nvt_plat_pcie->linkdown_hdl_data.lock);
	INIT_DELAYED_WORK(&nvt_plat_pcie->deffered_linkdown, nvt_plat_defferred_linkdown);

	platform_set_drvdata(pdev, nvt_plat_pcie);

	ret = nvt_plat_pcie_irq_init(&pci->pp);
	if (ret < 0) {
		printk("%s: irq init fail\r\n", __func__);
		return ret;
	}

	switch (nvt_plat_pcie->mode) {
	case DW_PCIE_RC_TYPE:
		if (!IS_ENABLED(CONFIG_PCIE_DW_PLAT_HOST))
			return -ENODEV;

		ret = nvt_plat_add_pcie_port(nvt_plat_pcie, pdev);
		if (ret < 0)
			goto err_out_link_failed;
		break;
	case DW_PCIE_EP_TYPE:
		if (!IS_ENABLED(CONFIG_PCIE_DW_PLAT_EP))
			return -ENODEV;

		ret = nvt_plat_add_pcie_ep(nvt_plat_pcie, pdev);
		if (ret < 0)
			goto err_out_link_failed;
		break;
	default:
		dev_err(dev, "INVALID device type %d\n", nvt_plat_pcie->mode);
		goto err_out_link_failed;
	}

#ifdef CONFIG_PROC_FS
	/* Proc init */
	ret = nvt_plat_pcie_proc_init(dev, pci);
	if (ret < 0)
		return ret;
#endif /* CONFIG_PROC_FS */

	dev_info(dev, "NVT pcie controller driver version: %s\n", NVT_PCIE_CONTROLLER_VERSION);
	dev_info(dev, "NVT pcie controller driver init successfully\n");

	return 0;

err_out_link_failed:
	irq_domain_remove(nvt_plat_pcie->top_irq_domain);

	return ret;

}

#ifdef CONFIG_PM
static int __maybe_unused pcie_suspend_noirq(struct device *dev)
{
	printk("%s: enter\r\n", __func__);
	if (nvt_get_suspend_mode()) {
		printk("%s: S3\r\n", __func__);
	}
	return 0;
}

static int __maybe_unused pcie_resume_noirq(struct device *dev)
{
	printk("%s: enter\r\n", __func__);
	return 0;
}

static const struct dev_pm_ops pcie_pm_ops = {
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(pcie_suspend_noirq,
				      pcie_resume_noirq)
};
#endif

static const struct nvt_plat_pcie_of_data nvt_plat_pcie_rc_of_data = {
	.mode = DW_PCIE_RC_TYPE,
};

static const struct nvt_plat_pcie_of_data nvt_plat_pcie_ep_of_data = {
	.mode = DW_PCIE_EP_TYPE,
};

static const struct of_device_id nvt_plat_pcie_of_match[] = {
	{
		.compatible = "nvt,nvt-dwc-pcie",
		.data = &nvt_plat_pcie_rc_of_data,
	},
	{
		.compatible = "nvt,nvt-dwc-pcie-ep",
		.data = &nvt_plat_pcie_ep_of_data,
	},
	{},
};

static struct platform_driver nvt_plat_pcie_driver = {
	.driver = {
		.name	= "nvt-pcie",
		.of_match_table = nvt_plat_pcie_of_match,
		.suppress_bind_attrs = true,
#ifdef CONFIG_PM
		.pm = &pcie_pm_ops,
#endif
	},
	.probe = nvt_plat_pcie_probe,
};

builtin_platform_driver(nvt_plat_pcie_driver);
