/**
    NVT PCIE cascade driver
    To handle NVT PCIE cascade driver: RC/EP connection and isr handling
    @file pcie-drv-wrap.c
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
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/msi.h>

#include "nvt_pcie_int.h"
#include "pcie-drv-wrap.h"

static struct nvt_pcie_info priv_pcie_info = {0};

struct nvt_pcie_info *nvt_pcie_get_info(void)
{
	// if someone invoke nvt_pcie_get_info() before pcie driver is probed
	// Use RC link layer status to represent EP0 link.
	if ((is_nvt_pcie_init() != 0) || (is_nvt_ep_init() != 0)) {
		// if either one is true, do nothing
		// fall throuhg to return priv_pcie_info
	} else {
		const phys_addr_t TOP_BASE = 0x2F0010000;
		const size_t TOP_REG_SIZE = 0x100;
		const u32 PCIE_BOOT_SEL_SFT = 7;
		const u32 PCIE_BOOT_SEL_MSK = 0x3;
		const u32 PCIE_BOOT_SEL_RC = 0x0;
		void __iomem *ptr;
		u32 reg;
		int is_link_up;

		ptr = ioremap(TOP_BASE, TOP_REG_SIZE);
		if (unlikely(ptr == 0)) {
			return 0;
		}

		reg = (readl(ptr) >> PCIE_BOOT_SEL_SFT) & PCIE_BOOT_SEL_MSK;

		iounmap(ptr);

		is_link_up = check_pcie_link();
		if (reg == PCIE_BOOT_SEL_RC) {
			priv_pcie_info.link_masks = is_link_up<<CHIP_EP0;
		} else {
			priv_pcie_info.link_masks = is_link_up<<CHIP_RC;
		}
	}

	return &priv_pcie_info;
}

