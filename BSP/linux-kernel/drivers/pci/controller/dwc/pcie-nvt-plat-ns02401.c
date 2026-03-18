/**
    @file       pcie-nvt-plat-ns02201.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/pci.h>

#include "pcie-designware.h"
#include "pcie-nvt.h"
#include "pcie-nvt-plat.h"

#include <plat/top.h>

/* TOP registers */
#define TOP_BOOT_CTRL_OFS		(0x00)
#define TOP_PCIE_REFCLK_SRC_MSK		GENMASK(16, 16)
#define TOP_PCIE_REFCLK_SRC_MPLL	(0)
#define TOP_PCIE_REFCLK_SRC_PAD		(1)
#define TOP_PCIE_LANEMODE_MSK		GENMASK(15, 14)
#define TOP_PCIE_LANEMODE_ONE_x4	(0)
#define TOP_PCIE_LANEMODE_TWO_x2	(1)
#define TOP_PCIE_LANEMODE_FOUR_x1	(2)
#define TOP_PCIE_LANEMODE_ONEx2_TWOx1	(3)
#define TOP_PCIE_BOOT_MSK		GENMASK(13, 13)
#define TOP_PCIE_BOOT_RC		(0)
#define TOP_PCIE_BOOT_EP		(1)

#define TOP_PCIE_CTRL_OFS		(0x60)
#define TOP_PCIE_REFSRC_MSK		GENMASK(8, 8)
#define TOP_PCIE_REFCLK_MSK		GENMASK(9, 9)
#define TOP_PCIE_REFCLK_OUE_EN		(1)
#define TOP_PCIE_REFCLK_OUE_DIS		(0)
#define TOP_PCIE_PRESETN_MSK		GENMASK(4, 4)
#define TOP_PCIE_PRESETN_HIGHZ		(1)
#define TOP_PCIE_PRESETN_LOW		(0)
#define TOP_PCIE_CTRL2_MODE_MSK		GENMASK(1, 1)
#define TOP_PCIE_MODE_EP		(1)
#define TOP_PCIE_MODE_RC		(0)
#define TOP_PCIE_CTRL3_MODE_MSK		GENMASK(2, 2)
#define TOP_PCIE_CTRL4_MODE_MSK		GENMASK(3, 3)

static int is_preset_init_done = 0;            // record if PCIE_RSTN asserted

int nvt_plat_top_init(struct device *dev, struct nvt_plat_pcie *nvt_pcie, enum dw_pcie_device_mode mode)
{
	u32 val;
	int refclk_from_pad = 0;
	int bs_lane_mode = 0;
	int ret = 0;
#if 0
	PIN_GROUP_CONFIG pinmux_config[2];
#endif
	void __iomem *ptr_top;

	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 256);

	if (is_preset_init_done == 0) {
		is_preset_init_done = 1;

		val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
		val &= ~TOP_PCIE_PRESETN_MSK;
		val |= FIELD_PREP(TOP_PCIE_PRESETN_MSK, TOP_PCIE_PRESETN_LOW);
		writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

#if 0
		pinmux_config[0].pin_function = PIN_FUNC_PCIE;
		pinmux_config[1].pin_function = PIN_FUNC_SEL_PCIE;
		nvt_pinmux_capture(pinmux_config, 2);
		pinmux_config[0].config |= PIN_PCIE_CFG_RSTN;
		pinmux_config[1].config = PINMUX_PCIEMUX_SEL_RSTN | PINMUX_PCIEMUX_RSTN_OUT_LOW;
		ret = nvt_pinmux_update(pinmux_config, 2);
		if (ret)
			pr_err("Enable PCIE_RSTN pinmux fail\n");
#endif
	}

	val = readl(ptr_top + TOP_BOOT_CTRL_OFS);
	refclk_from_pad = FIELD_GET(TOP_PCIE_REFCLK_SRC_MSK, val);
	bs_lane_mode = FIELD_GET(TOP_PCIE_LANEMODE_MSK, val);
	if (nvt_pcie->slot == 0) {
		// CTRL1
		switch (FIELD_GET(TOP_PCIE_BOOT_MSK, val)) {
		case TOP_PCIE_BOOT_RC:
			if (mode != DW_PCIE_RC_TYPE) {
				dev_err(dev, "Bootstrap[13] is configured to RC (0x%lx), but DTS is NOT configured to RC\n",
					FIELD_GET(TOP_PCIE_BOOT_MSK, val));
				ret = -EPERM;
				goto exit;
			}
			break;
		case TOP_PCIE_BOOT_EP:
			if (mode != DW_PCIE_EP_TYPE) {
				dev_err(dev, "Bootstrap[13] is configured to EP (0x%lx), but DTS is NOT configured to EP\n",
					FIELD_GET(TOP_PCIE_BOOT_MSK, val));
				ret = -EPERM;
				goto exit;
			}
			break;
		default:
			dev_err(dev, "Bootstrap[13] = 0x%lx is invalid setting\n",
				FIELD_GET(TOP_PCIE_BOOT_MSK, val));
			ret = -EPERM;
			goto exit;
		}

		if (bs_lane_mode == TOP_PCIE_LANEMODE_ONE_x4) {
			nvt_pcie->bs_lane_width = 4;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_TWO_x2) {
			nvt_pcie->bs_lane_width = 2;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_FOUR_x1) {
			nvt_pcie->bs_lane_width = 1;
		} else {	// one x2 and two x1
			nvt_pcie->bs_lane_width = 2;
		}
		nvt_pcie->bs_lane_mode = bs_lane_mode;
	} else if (nvt_pcie->slot == 1) {
		// CTRL2
		val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
		val &= ~TOP_PCIE_CTRL2_MODE_MSK;
		if (mode == DW_PCIE_RC_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL2_MODE_MSK, TOP_PCIE_MODE_RC);
		} else if (mode == DW_PCIE_EP_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL2_MODE_MSK, TOP_PCIE_MODE_EP);
		}
		writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

		if (bs_lane_mode == TOP_PCIE_LANEMODE_ONE_x4) {
			dev_err(dev, "Bootstrap[15..14] is ONE x4 => PCIE controller2 can ONLY allocate 0 lane\n");
			nvt_pcie->bs_lane_width = 0;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_TWO_x2) {
			dev_err(dev, "Bootstrap[15..14] is TWO x2 => PCIE controller2 can ONLY allocate 0 lane\n");
			nvt_pcie->bs_lane_width = 0;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_FOUR_x1) {
			nvt_pcie->bs_lane_width = 1;
		} else {	// one x2 and two x1
			nvt_pcie->bs_lane_width = 0;
		}
		nvt_pcie->bs_lane_mode = bs_lane_mode;
	} else if (nvt_pcie->slot == 2) {
		// CTRL3
		val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
		val &= ~TOP_PCIE_CTRL3_MODE_MSK;
		if (mode == DW_PCIE_RC_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL3_MODE_MSK, TOP_PCIE_MODE_RC);
		} else if (mode == DW_PCIE_EP_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL3_MODE_MSK, TOP_PCIE_MODE_EP);
		}
		writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

		if (bs_lane_mode == TOP_PCIE_LANEMODE_ONE_x4) {
			dev_err(dev, "Bootstrap[15..14] is ONE x4 => PCIE controller2 can ONLY allocate 0 lane\n");
			nvt_pcie->bs_lane_width = 0;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_TWO_x2) {
			nvt_pcie->bs_lane_width = 2;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_FOUR_x1) {
			nvt_pcie->bs_lane_width = 1;
		} else {	// one x2 and two x1
			nvt_pcie->bs_lane_width = 1;
		}
		nvt_pcie->bs_lane_mode = bs_lane_mode;
	} else if (nvt_pcie->slot == 3) {
		// CTRL4
		val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
		val &= ~TOP_PCIE_CTRL4_MODE_MSK;
		if (mode == DW_PCIE_RC_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL4_MODE_MSK, TOP_PCIE_MODE_RC);
		} else if (mode == DW_PCIE_EP_TYPE) {
			val |= FIELD_PREP(TOP_PCIE_CTRL4_MODE_MSK, TOP_PCIE_MODE_EP);
		}
		writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

		if (bs_lane_mode == TOP_PCIE_LANEMODE_ONE_x4) {
			dev_err(dev, "Bootstrap[15..14] is ONE x4 => PCIE controller2 can ONLY allocate 0 lane\n");
			nvt_pcie->bs_lane_width = 0;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_TWO_x2) {
			dev_err(dev, "Bootstrap[15..14] is TWO x2 => PCIE controller2 can ONLY allocate 0 lane\n");
			nvt_pcie->bs_lane_width = 0;
		} else if (bs_lane_mode == TOP_PCIE_LANEMODE_FOUR_x1) {
			nvt_pcie->bs_lane_width = 1;
		} else {	// one x2 and two x1
			nvt_pcie->bs_lane_width = 1;
		}
		nvt_pcie->bs_lane_mode = bs_lane_mode;
	} else {
		printk("%s: unsupported slot id %d\r\n", __func__, nvt_pcie->slot);
		return -1;
	}

	nvt_pcie->refclk_from_pad = refclk_from_pad;
exit:
	iounmap(ptr_top);

	return ret;
}

int nvt_plat_set_preset(struct nvt_plat_pcie *nvt_pcie, int is_assert)
{
	int ret = 0;
#if 0
	PIN_GROUP_CONFIG pinmux_config[1];
#endif
	u32 val;
	void __iomem *ptr_top;

	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 256);

	val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
	val &= ~TOP_PCIE_PRESETN_MSK;
	if (is_assert) {
		val |= FIELD_PREP(TOP_PCIE_PRESETN_MSK, TOP_PCIE_PRESETN_LOW);
	} else {
		val |= FIELD_PREP(TOP_PCIE_PRESETN_MSK, TOP_PCIE_PRESETN_HIGHZ);
	}
	writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

	iounmap(ptr_top);
#if 0
	pinmux_config[0].pin_function = PIN_FUNC_SEL_PCIE;
	pinmux_config[0].config = PINMUX_PCIEMUX_SEL_RSTN;
	if (is_assert) {
		pinmux_config[0].config |= PINMUX_PCIEMUX_RSTN_OUT_LOW;
	} else {
		pinmux_config[0].config |= PINMUX_PCIEMUX_RSTN_HIGHZ;
	}
	ret = nvt_pinmux_update(pinmux_config, 1);
	if (ret)
		pr_err("Set RSTN pinmux fail\n");
#endif

	return ret;
}

int nvt_plat_refclkout(struct nvt_plat_pcie *nvt_pcie, int output_en)
{
	int ret = 0;
#if 0
	PIN_GROUP_CONFIG pinmux_config[1];
#endif
	u32 val;
	void __iomem *ptr_top;

	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 256);

	val = readl(ptr_top + TOP_PCIE_CTRL_OFS);
	val &= ~TOP_PCIE_REFCLK_MSK;
	val &= ~TOP_PCIE_REFSRC_MSK;
	if (output_en) {
		val &= ~TOP_PCIE_REFSRC_MSK;
		val |= FIELD_PREP(TOP_PCIE_REFCLK_MSK, TOP_PCIE_REFCLK_OUE_EN);
	} else {
		val |= TOP_PCIE_REFSRC_MSK;
		val |= FIELD_PREP(TOP_PCIE_REFCLK_MSK, TOP_PCIE_REFCLK_OUE_DIS);
	}
	writel(val, ptr_top + TOP_PCIE_CTRL_OFS);

	iounmap(ptr_top);
#if 0
	pinmux_config[0].pin_function = PIN_FUNC_PCIE;
	nvt_pinmux_capture(pinmux_config, 1);
	if (output_en) {
		pinmux_config[0].config |= PIN_PCIE_CFG_REFCLK_OUTEN;
	} else {
		pinmux_config[0].config &= ~PIN_PCIE_CFG_REFCLK_OUTEN;
	}
	ret = nvt_pinmux_update(pinmux_config, 1);
	if (ret)
		pr_err("Set REFCLK pinmux fail\n");
#endif
	return ret;
}
