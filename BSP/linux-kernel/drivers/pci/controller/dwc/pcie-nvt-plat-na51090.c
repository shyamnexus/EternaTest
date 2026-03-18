/**
    @file       pcie-nvt-plat-na51090.c
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
#include <linux/soc/nvt/nvt-pci.h>

#include <plat/top.h>

/* TOP registers */
#define TOP_BOOT_CTRL_OFS		(0x00)
#define TOP_PCIE_REFCLK_SRC_MSK		GENMASK(10, 10)
#define TOP_PCIE_REFCLK_SRC_MPLL	(0)
#define TOP_PCIE_REFCLK_SRC_PAD		(1)
#define TOP_PCIE_BOOT_MSK		GENMASK(8, 7)
#define TOP_PCIE_BOOT_RC		(0)
#define TOP_PCIE_BOOT_EP		(1)
#define TOP_PCIE_BOOT_EP_CPU_ON		(2)

#define TOP_PCIE_CTRL_OFS		(0x28)
#define TOP_PCIE_REFCLK_MSK		GENMASK(9, 9)
#define TOP_PCIE_REFCLK_OUE_EN		(1)
#define TOP_PCIE_REFCLK_OUE_DIS		(0)
#define TOP_PCIE_PRESETN_MSK		GENMASK(4, 4)
#define TOP_PCIE_PRESETN_HIGHZ		(1)
#define TOP_PCIE_PRESETN_LOW		(0)

struct nvt_plat_pcie *nvt_plat_pcie_dev = NULL;
static u32 nvt_ep_busdev[8] = {0};

int nvt_pci_add_ep_drv_info(struct pci_dev *pcidev, struct nvt_ep_dev_info *ep_dev)
{
	if (ep_dev == NULL || nvt_plat_pcie_dev == NULL)
		return -1;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return -1;

	down(&nvt_plat_pcie_dev->sema_ep_info);
	list_add_tail(&ep_dev->list, &nvt_plat_pcie_dev->ep_info_list_root);
	up(&nvt_plat_pcie_dev->sema_ep_info);

	return 0;
}
EXPORT_SYMBOL(nvt_pci_add_ep_drv_info);

int nvt_pci_del_ep_drv_info(struct pci_dev *pcidev, struct nvt_ep_dev_info *ep_dev)
{
	if (ep_dev == NULL || nvt_plat_pcie_dev == NULL)
		return -1;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return -1;

	down(&nvt_plat_pcie_dev->sema_ep_info);
	list_del(&ep_dev->list);
	memset(&nvt_ep_busdev[0], 0, sizeof(nvt_ep_busdev));
	up(&nvt_plat_pcie_dev->sema_ep_info);

	return 0;
}
EXPORT_SYMBOL(nvt_pci_del_ep_drv_info);

unsigned int nvt_pci_list_ep_drv_info_num(void)
{
	struct nvt_ep_dev_info *curr_info, *next_info;
	unsigned int num = 0;

	if (nvt_plat_pcie_dev == NULL)
		return 0;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return 0;

	down(&nvt_plat_pcie_dev->sema_ep_info);
	if (list_empty(&nvt_plat_pcie_dev->ep_info_list_root)) {
		up(&nvt_plat_pcie_dev->sema_ep_info);
		return 0;
	}

	list_for_each_entry_safe(curr_info, next_info, &nvt_plat_pcie_dev->ep_info_list_root, list) {
		num++;
	}
	up(&nvt_plat_pcie_dev->sema_ep_info);

	return num;
}
EXPORT_SYMBOL(nvt_pci_list_ep_drv_info_num);

int nvt_pci_list_for_each_ep_drv_info(nvt_ep_func_ptr_t nvt_ep_func_ptr)
{
	struct nvt_ep_dev_info *curr_info, *next_info;
	int ret = 0;

	if (nvt_plat_pcie_dev == NULL)
		return -1;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return -1;

	down(&nvt_plat_pcie_dev->sema_ep_info);
	if (list_empty(&nvt_plat_pcie_dev->ep_info_list_root)) {
		up(&nvt_plat_pcie_dev->sema_ep_info);
		return 0;
	}

	list_for_each_entry_safe(curr_info, next_info, &nvt_plat_pcie_dev->ep_info_list_root, list) {
		ret = nvt_ep_func_ptr(curr_info->ep_drv_info);
		if (ret < 0) {
			up(&nvt_plat_pcie_dev->sema_ep_info);
			return -1;
		}
	}
	up(&nvt_plat_pcie_dev->sema_ep_info);

	return 0;
}
EXPORT_SYMBOL(nvt_pci_list_for_each_ep_drv_info);

struct pci_ep_engine* nvt_pci_get_ep_drv_info(unsigned int ep_idx)
{
	struct nvt_ep_dev_info *curr_info, *next_info;

	if (nvt_plat_pcie_dev == NULL)
		return NULL;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return NULL;

	down(&nvt_plat_pcie_dev->sema_ep_info);

	if (list_empty(&nvt_plat_pcie_dev->ep_info_list_root)) {
		up(&nvt_plat_pcie_dev->sema_ep_info);
		return NULL;
	}

	list_for_each_entry_safe(curr_info, next_info, &nvt_plat_pcie_dev->ep_info_list_root, list) {
		if (curr_info->ep_drv_info->ep_idx == ep_idx) {
			up(&nvt_plat_pcie_dev->sema_ep_info);
			return curr_info->ep_drv_info;
		}
	}

	up(&nvt_plat_pcie_dev->sema_ep_info);

	return NULL;
}
EXPORT_SYMBOL(nvt_pci_get_ep_drv_info);

int nvt_pci_list_register_notifier(struct nvt_ep_dev_info_notifier *p_notifier)
{
	if (nvt_plat_pcie_dev == NULL)
		return -1;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return -1;

	down(&nvt_plat_pcie_dev->sema_ep_info);

	list_add(&p_notifier->list, &nvt_plat_pcie_dev->ep_info_notifier_head);

	up(&nvt_plat_pcie_dev->sema_ep_info);

	return 0;
}
EXPORT_SYMBOL(nvt_pci_list_register_notifier);

int nvt_pci_list_call_notifier(void)
{
	struct nvt_ep_dev_info *curr_info, *next_info;
	struct nvt_ep_dev_info_notifier *curr_notify, *next_notify;
	int ret = 0;

	if (nvt_plat_pcie_dev == NULL)
		return -1;

	if (nvt_plat_pcie_dev->link_stat != PCIE_LINK_STAT_OK)
		return -1;

	down(&nvt_plat_pcie_dev->sema_ep_info);

	if (list_empty(&nvt_plat_pcie_dev->ep_info_list_root) ||
			list_empty(&nvt_plat_pcie_dev->ep_info_notifier_head)) {
		goto exit;
	}

	list_for_each_entry_safe(curr_notify, next_notify, &nvt_plat_pcie_dev->ep_info_notifier_head, list) {
		list_for_each_entry_safe(curr_info, next_info, &nvt_plat_pcie_dev->ep_info_list_root, list) {
			ret = curr_notify->p_func(curr_info->ep_drv_info);
			if (ret < 0) {
				break;
			}
		}
	}

exit:
	up(&nvt_plat_pcie_dev->sema_ep_info);
	return 0;
}
EXPORT_SYMBOL(nvt_pci_list_call_notifier);

static int is_preset_init_done = 0;            // record if PCIE_RSTN asserted

int nvt_plat_top_init(struct device *dev, struct nvt_plat_pcie *nvt_pcie, enum dw_pcie_device_mode mode)
{
	u32 val;
	int refclk_from_pad = 0;
//	int bs_lane_mode = 0;
	int ret = 0;
	void __iomem *ptr_top;

	nvt_plat_pcie_dev = nvt_pcie;

	if (is_preset_init_done == 0) {
		PIN_GROUP_CONFIG pinmux_config[1];

		is_preset_init_done = 1;

		pinmux_config[0].pin_function = PIN_FUNC_MISC;
		nvt_pinmux_capture(pinmux_config, 1);
		pinmux_config[0].config |= PIN_MISC_CFG_PCIE_RSTN;	// only PIN_MISC_CFG_PCIE_RSTN and without PIN_MISC_CFG_RSTN_HIGHZ => RSTN output LOW
		ret = nvt_pinmux_update(pinmux_config, 1);
		if (ret)
			pr_err("Enable PCIE_RSTN pinmux fail\n");

	}

	ptr_top = ioremap(NVT_TOP_BASE_PHYS, 256);

	val = readl(ptr_top + TOP_BOOT_CTRL_OFS);
	refclk_from_pad = FIELD_GET(TOP_PCIE_REFCLK_SRC_MSK, val);
//	bs_lane_mode = FIELD_GET(TOP_PCIE_LANEMODE_MSK, val);
	if (nvt_pcie->slot == 0) {
		// CTRL1
		switch (FIELD_GET(TOP_PCIE_BOOT_MSK, val)) {
		case TOP_PCIE_BOOT_RC:
			if (mode != DW_PCIE_RC_TYPE) {
				dev_err(dev, "Bootstrap[8..7] is configured to RC (0x%lx), but DTS is NOT configured to RC\n",
					FIELD_GET(TOP_PCIE_BOOT_MSK, val));
				ret = -EPERM;
				goto exit;
			}
			break;
		case TOP_PCIE_BOOT_EP:
		case TOP_PCIE_BOOT_EP_CPU_ON:
			if (mode != DW_PCIE_EP_TYPE) {
				dev_err(dev, "Bootstrap[8..7] is configured to EP (0x%lx), but DTS is NOT configured to EP\n",
					FIELD_GET(TOP_PCIE_BOOT_MSK, val));
				ret = -EPERM;
				goto exit;
			}
			break;
		default:
			dev_err(dev, "Bootstrap[8..7] = 0x%lx is invalid setting\n",
				FIELD_GET(TOP_PCIE_BOOT_MSK, val));
			ret = -EPERM;
			goto exit;
		}

		nvt_pcie->bs_lane_width = 1;
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
	PIN_GROUP_CONFIG pinmux_config[1];

	pinmux_config[0].pin_function = PIN_FUNC_MISC;
	nvt_pinmux_capture(pinmux_config, 1);
	if (is_assert) {
		pinmux_config[0].config &= ~PIN_MISC_CFG_RSTN_HIGHZ;
	} else {
		pinmux_config[0].config |= PIN_MISC_CFG_RSTN_HIGHZ;
	}
	ret = nvt_pinmux_update(pinmux_config, 1);
	if (ret)
		pr_err("Set RSTN pinmux fail\n");

	return 0;
}

int nvt_plat_refclkout(struct nvt_plat_pcie *nvt_pcie, int output_en)
{
	int ret = 0;
	PIN_GROUP_CONFIG pinmux_config[1];

	pinmux_config[0].pin_function = PIN_FUNC_MISC;
	nvt_pinmux_capture(pinmux_config, 1);
	if (output_en) {
		pinmux_config[0].config |= PIN_MISC_CFG_PCIE_REFCLK_OUTEN;
	} else {
		pinmux_config[0].config &= ~PIN_MISC_CFG_PCIE_REFCLK_OUTEN;
	}
	ret = nvt_pinmux_update(pinmux_config, 1);
	if (ret)
		pr_err("Set REFCLK pinmux fail\n");

	return 0;
}

