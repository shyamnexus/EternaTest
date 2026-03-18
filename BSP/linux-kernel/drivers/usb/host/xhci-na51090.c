/*
 * Copyright (C) 2017 Novatek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <asm/io.h>
//#include <mach/hardware.h>
//#include <mach/nvt-gpio.h>
#include <linux/of_address.h>
#include <plat/efuse_protected.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include "xhci.h"

static unsigned int usb3_txen = 1;
static int g_ep_idx = 0;
// module_param_named(usb3_txen, usb3_txen, int, S_IRUGO | S_IWUSR);
// MODULE_PARM_DESC(usb3_txen, "Set usb3 ss tx default force-enable or disable. default is force-enable.");

void __iomem *U3PHY_REG;
void __iomem *U2PHY_REG;
void __iomem *U3CTRL_REG;

#define USB3PHY_SETREG(ofs,value)   writel((value), (volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))
#define USB3PHY_GETREG(ofs)         readl((volatile void __iomem *)(U3PHY_REG+((ofs)<<2)))

#define USB3U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))
#define USB3U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(U2PHY_REG+((ofs)<<2)))

static ssize_t trim_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	writel(0x0, U3CTRL_REG + 0x23fc);
	return sprintf(buf, "0x%x\n", readl(U3CTRL_REG + 0x2680));
}

static DEVICE_ATTR_RO(trim);

static struct attribute *nvt_sysfs_attributes[] = {
	&dev_attr_trim.attr,
	NULL,
};

static const struct attribute_group nvt_sysfs_attr_group = {
	.attrs = nvt_sysfs_attributes,
};

static int nvt_xhci_sysfs_init(struct device *dev)
{
	return sysfs_create_group(&dev->kobj, &nvt_sysfs_attr_group);
}

static void nvt_xhci_sysfs_remove(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &nvt_sysfs_attr_group);
}

int xhci_nvt_host_init(struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);
	struct device *dev = hcd->self.controller;
	struct resource  *res;
	void __iomem     *regs;
	struct clk       *clk;
	unsigned long    temp;
	UINT8 u3_trim_rint_sel=8,u3_trim_swctrl=4, u3_trim_sqsel=4,u3_trim_icdr=0xB;
	u32 force_hs = 0;
	u32 u32prop = 0, u32prop_array[2] = {0};
	int power_gpio[2], ep_idx;
	u32 get_tx_swing;
	int tx_swing;

	if (nvtpcie_is_common_pci()) {
		if (of_property_read_u32(dev->of_node, "chip_id", &u32prop)) {
			ep_idx = -1;
		} else {
			ep_idx = (int)(u32prop&INT_MAX) - CHIP_EP0;
		}
	} else {
		if (of_property_read_u32(dev->of_node, "ep_id", &u32prop)) {
			ep_idx = -1;
		} else {
			ep_idx = (int)(u32prop&INT_MAX);
		}
	}

	g_ep_idx = ep_idx;

	if (ep_idx > nvtpcie_get_ep_count()) {
		printk("EP not linked, skip!\n");
		return 0; //EP not linked, skip
	}

	if (of_property_read_u32(dev->of_node, "nvt_stop_ep", &xhci->nvt_stop_ep)) {
		xhci->nvt_stop_ep = 0;
	} else {
		printk("NVT stop EP patch!\n");
	}

	if(!of_property_read_u32(dev->of_node, "force-hs", &force_hs)) {
		pr_debug("force-hs!\n");
	}

	if (of_property_read_u32_array(dev->of_node, "power_gpio", u32prop_array, 2)) {
		power_gpio[0] = -1;
		power_gpio[1] = -1;
	} else {
		power_gpio[0] = u32prop_array[0];
		power_gpio[1] = u32prop_array[1];

		gpio_direction_output(power_gpio[0], !power_gpio[1]);
		mdelay(200);
	}

	if (of_property_read_u32(dev->of_node, "tx_swing", &get_tx_swing)) {
		tx_swing = -1;
	} else {
		tx_swing = get_tx_swing;
	}

	clk = clk_get(NULL, "pll20");
	if (!IS_ERR(clk)) {
		xhci->phyclk = clk;
		clk_set_rate(xhci->phyclk, 498750000);
	} else {
		xhci->phyclk = NULL;
		pr_err("%s: u3phyclk not found\n", __func__);
	}

	clk = clk_get(NULL, "pll20_ss");
	if (!IS_ERR(clk)) {
		xhci->phyclk_ss = clk;
		clk_set_phase(xhci->phyclk_ss, 0x0);
	} else {
		pr_err("%s: pll20ss not found\n", __func__);
	}

	clk = clk_get(dev, dev_name(dev));
	if (!IS_ERR(clk)) {
		clk_prepare_enable(clk);
		clk_disable_unprepare(clk);
		clk_prepare_enable(clk);
	} else
		pr_err("%s: %s not found\n", __func__, dev_name(dev));

	if (ep_idx >= 0) {
		clk = clk_get(NULL, "u3phy_ep0");
		if (!IS_ERR(clk)) {
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_err("%s: u3phy not found\n", __func__);

	} else {
		clk = clk_get(NULL, "u3phy");
		if (!IS_ERR(clk)) {
			clk_prepare_enable(clk);
			clk_disable(clk);
			clk_enable(clk);
		} else
			pr_err("%s: u3phy not found\n", __func__);
	}

#if 0
	res = platform_get_resource(dev, IORESOURCE_MEM, 1);
	if (NULL == res) {
		pr_err("%s: get res failed\n", __func__);
		return;
	}

	//if (nvtpcie_is_common_pci() && (ep_idx != -1)) {
	//	if (0 != nvtpcie_conv_resource(ep_idx + CHIP_EP0, res)) {
	//		pr_err("%s: conv res failed, ep_idx %d, res %pR\n", __func__, ep_idx, res);
	//		return;
	//	}
	//}


	if (nvtpcie_is_common_pci()) {
		regs = devm_ioremap(&dev->dev, res->start, resource_size(res));
	} else {
		regs = devm_ioremap_resource(&dev->dev, res);
	}
	if (IS_ERR(regs)) {
		pr_err("map err1\r\n");
	}
#endif
	regs = of_iomap(dev->of_node, 1);
       if (!regs) {
              pr_err("map err1\r\n");
       }

	U3PHY_REG = regs + 0x2000;
	U2PHY_REG = regs + 0x1000;
	U3CTRL_REG = regs;

	temp = readl(regs + 0x28);
	temp = 0x1;
	writel(temp, regs + 0x28);

	temp = readl(regs + 0x10);
	temp &= ~0x6; /*Set Host mode & No Force Suspend*/
	temp |= 0x1;
	if (force_hs)
		temp |= 0x8;
	else
		temp &= ~(0x1 << 11);

	temp &= ~((0x7 << 16) | (0x7 << 24) | (0x1 << 4));
	temp |= ((0x4 << 16) | (0x4 << 24));

	writel(temp, regs + 0x10);
	udelay(100);

	xhci->phy_reg = (unsigned long)regs;

	xhci->ctrl_reg = hcd->regs;

	xhci->power_gpio[0] = power_gpio[0];

	xhci->power_gpio[1] = power_gpio[1];

	USB3PHY_SETREG(0x0D, 0x01);

	if ((tx_swing >= 0) && (tx_swing <= 7)) {
		u3_trim_swctrl = tx_swing;
	}

	{
		UINT16 data=0;
		INT32 result;

		result= efuse_readParamOps(EFUSE_USB2_TRIM_DATA, &data);
		if(result >= 0) {
			u3_trim_rint_sel = data&0x1F;
			u3_trim_icdr     = (data>>5)&0xF;
		}

		//usb3_validateTrim();
		temp = USB3PHY_GETREG(0x1A0);
		if ((temp & 0xF) == 0) {
			USB3PHY_SETREG(0x1A0, 0x40+u3_trim_rint_sel);
			USB3PHY_SETREG(0x1A3, 0x60);
		}

		temp = USB3U2PHY_GETREG(0x06);
		temp &= ~(0x7<<1);
		temp |= (u3_trim_swctrl<<1);
		USB3U2PHY_SETREG(0x06, temp);

		temp = USB3U2PHY_GETREG(0x05);
		temp &= ~(0x7<<2);
		temp |= (u3_trim_sqsel<<2);
		USB3U2PHY_SETREG(0x05, temp);
	}

	/* asd_mode=1 (toggle rate) */
	USB3PHY_SETREG(0x198, 0x04);
	/* RX_ICTRL's offset = 0 */
	USB3PHY_SETREG(0x1BF, 0x40);
	/* TX_AMP_CTL=2, TX_DEC_EM_CTL=8 */
	USB3PHY_SETREG(0x014, 0x8a);
	/* TX_LFPS_AMP_CTL = 1 */
	USB3PHY_SETREG(0x034, 0xfc);//default 0xfc
	/* PHY Power Mode Change ready reponse time. (3 is 1ms.)(4 is 1.3ms.) */

	USB3PHY_SETREG(0x114, 0x0B);
	USB3PHY_SETREG(0x152, 0x2E);
	USB3PHY_SETREG(0x153, 0x01);
	USB3PHY_SETREG(0x1B0, 0xC0);
	USB3PHY_SETREG(0x1B1, 0x91);
	USB3PHY_SETREG(0x1B2, 0x00);
	USB3PHY_SETREG(0x135, 0x88);
	USB3PHY_SETREG(0x12A, 0x50);
	USB3PHY_SETREG(0x1F0, 0x80);
	USB3PHY_SETREG(0x1F5, 0x01|(u3_trim_icdr<<4));//0xB1
	USB3PHY_SETREG(0x105, 0x01);
	USB3PHY_SETREG(0x056, 0x74);//disconnect level to 0x3
	udelay(2);

	USB3PHY_SETREG(0x102, 0x20);
	udelay(10);
	USB3PHY_SETREG(0x102, 0x00);
	udelay(300);

	USB3PHY_SETREG(0x103, 0x01);
	udelay(100);

	USB3U2PHY_SETREG(0x51, 0x20);
	udelay(2);
	USB3U2PHY_SETREG(0x51, 0x00);
	udelay(2);

	if(usb3_txen) {
		temp = USB3PHY_GETREG(0x011);
		temp |= 0x1;
		USB3PHY_SETREG(0x011, temp);

		temp = USB3PHY_GETREG(0x031);
		temp |= 0x1;
		USB3PHY_SETREG(0x031, temp);
	}

	USB3PHY_SETREG(0x029, 0x1);

	if (power_gpio[0] > 0) {
		gpio_direction_output(power_gpio[0], power_gpio[1]);
		mdelay(200);
	}

	nvt_xhci_sysfs_init(dev);

	pr_info("%s with %s mode, apb 0x10= 0x%08lX, ver %s\r\n", __func__,
							force_hs ? "HS": "SS",
							temp, DRV_VERSION);
	return 0;
}

void xhci_nvt_host_uninit(struct platform_device *pdev, struct usb_hcd *hcd)
{
	struct xhci_hcd  *xhci = hcd_to_xhci(hcd);

	nvt_xhci_sysfs_remove(&pdev->dev);

	if (xhci->power_gpio[0] > 0)
		gpio_direction_output(xhci->power_gpio[0], !xhci->power_gpio[1]);

	{
		struct clk       *clk;

		/* Remove controller clk */
		clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
		if (!IS_ERR(clk)) {
			clk_disable_unprepare(clk);
			clk_put(clk);
		} else
			pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));

		/* Remove u3 phy clock*/
		if (g_ep_idx >= 0) {
			clk = clk_get(NULL, "u3phy_ep0");
			if (!IS_ERR(clk)) {
				clk_disable_unprepare(clk);
				clk_put(clk);
			} else
				pr_err("%s: u3phy not found\n", __func__);

		} else {
			clk = clk_get(NULL, "u3phy");
			if (!IS_ERR(clk)) {
				clk_disable_unprepare(clk);
				clk_put(clk);
			} else
				pr_err("%s: u3phy not found\n", __func__);
		}
	}
}

