/**
    @file       pcie-nvt-plat.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _PCIE_NVT_PLAT_H
#define _PCIE_NVT_PLAT_H

#include <plat/hardware.h>

#include <linux/kernel.h>
#include <linux/pci.h>

#include "pcie-designware.h"
#include "pcie-nvt.h"

#if IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02201)
#define NVT_SHARED_LANE_WIDTH		(2)
#elif IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02401)
#define NVT_SHARED_LANE_WIDTH		(4)
#else
#define NVT_SHARED_LANE_WIDTH		(1)
#endif

int nvt_plat_top_init(struct device *dev, struct nvt_plat_pcie *nvt_pcie, enum dw_pcie_device_mode mode);
int nvt_plat_set_preset(struct nvt_plat_pcie *nvt_pcie, int is_assert);
int nvt_plat_refclkout(struct nvt_plat_pcie *nvt_pcie, int output_en);

#endif
