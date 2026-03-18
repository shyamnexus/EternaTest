/**
    NVT PCIE Common ep driver wrapping header
    Provide driver's prototypes to nvt-pcie-lib
    @file pcie-drv-wrap.h
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
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/soc/nvt/nvt_type.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>

#ifndef __NVT_PCIE_WRAP__
#define __NVT_PCIE_WRAP__

typedef enum {
	ATU_MAP_MAU = 0,
	ATU_MAP_APB = 1,
	ATU_MAP_COUNT
} nvtpcie_map_type_t;

typedef struct __nvt_pcie_phy_map__ {
	nvtpcie_map_type_t	type;
	phys_addr_t		addr;
	size_t			size;
} nvt_pcie_phy_map;

// structure shared by pcie lib/driver
struct nvt_pcie_info {
	unsigned int link_masks; // each bit represents link of nvtpcie_chipid_t
	void __iomem *dbi_ptr; // DBI base (virtual address)

	nvt_pcie_phy_map	pcie_chip_map[CHIP_MAX][ATU_MAP_COUNT];
};

#if 1
extern struct nvt_pcie_info *nvt_pcie_get_info(void);

#else
static inline struct nvt_pcie_info *nvt_pcie_get_info(void)
{
	return NULL;
}

#endif

#endif
