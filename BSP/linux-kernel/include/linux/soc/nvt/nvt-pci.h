/**
    NVT PCIE header

    @file       nvt-pci.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __SOC_NVT_NVT_PCI_H
#define __SOC_NVT_NVT_PCI_H
#include <linux/pci.h>
#include <linux/list.h>

#define PCI_VENDOR_ID_NOVATEK           0x1e09
#define PCI_DEVICE_ID_NOVATEK_NA51090   0x0005

#define NVT_PCIE_OUTBOUND_TOTAL_SEG_SIZE    16
#define NVT_PCIE_INBOUND_TOTAL_SEG_SIZE     8

typedef enum {
	NVT_PCIE_ADDR_SYS_START = 0,
	NVT_PCIE_ADDR_SYS_FLAGS = 1,
	NVT_PCIE_ADDR_SYS_LEN = 2,
	NVT_PCIE_ADDR_BUS = 3,
	NVT_PCIE_ADDR_MAX = 4
} NVT_PCIE_EP_ADDR_INFO;

struct pci_ep_addr_inbound {
	unsigned long mmio[NVT_PCIE_INBOUND_TOTAL_SEG_SIZE][NVT_PCIE_ADDR_MAX];
	unsigned long apb[NVT_PCIE_ADDR_MAX];
};

struct pci_ep_addr_outbound {
	unsigned long mmio[NVT_PCIE_ADDR_MAX];
	unsigned long apb[NVT_PCIE_ADDR_MAX];
};

struct pci_rc_addr_inbound {
	unsigned long mmio[NVT_PCIE_ADDR_MAX];
	unsigned long apb[NVT_PCIE_ADDR_MAX];
};

struct pci_rc_addr_outbound {
	void __iomem *mmio_va;
	unsigned long mmio[NVT_PCIE_ADDR_MAX];
	void __iomem *apb_va;
	unsigned long apb[NVT_PCIE_ADDR_MAX];
};

struct pci_ep_atu {
	void __iomem *atu_va;
	unsigned long atu[NVT_PCIE_ADDR_MAX];
	unsigned long inb_bitmap;
};

struct pci_ep_engine {
	struct pci_dev *pci_dev;
	struct pci_ep_addr_inbound ep_inb_info;
	struct pci_ep_addr_outbound ep_outb_info;
	struct pci_rc_addr_inbound rc_inb_info;
	struct pci_rc_addr_outbound rc_outb_info;
	struct pci_ep_atu atu_info;
	unsigned int ep_idx;
	spinlock_t ep_drv_atu_lock;
	struct semaphore ep_dyn_atu_sem;
	void *p_pcie_ep_ctrl;
};

struct nvt_ep_dev_info {
	struct pci_ep_engine	*ep_drv_info;
	struct list_head		list;
};

typedef int (*nvt_ep_func_ptr_t)(struct pci_ep_engine *ep_info_t);

struct nvt_ep_dev_info_notifier {
	nvt_ep_func_ptr_t p_func;
	struct list_head list;
};

/**
 * @brief Add EP device info to host bus driver list
 *
 *
 * @parm pcidev
 *		 ep_dev: ep list data struct
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pci_add_ep_drv_info(struct pci_dev *pcidev, struct nvt_ep_dev_info *ep_dev);

/**
 * @brief Remove EP device info from host bus driver list
 *
 *
 * @parm pcidev
 *		 ep_dev: ep list data struct
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pci_del_ep_drv_info(struct pci_dev *pcidev, struct nvt_ep_dev_info *ep_dev);

/**
 * @brief This function will return ep_drv_info by the specific ep_idx
 *
 *
 * @parm ep_idx: ep index
 *
 * @return a valid pointer for success, NULL for fail
 */
struct pci_ep_engine* nvt_pci_get_ep_drv_info(unsigned int ep_idx);

/**
 * @brief Get the total number of the ep devices
 *
 *
 * @parm
 *
 * @return  >= 0 for success, < 0 for fail
 */
unsigned int nvt_pci_list_ep_drv_info_num(void);

/**
 * @brief This function will call the function pointer to get every ep device info
 *
 *
 * @parm nvt_ep_func_ptr: Used to get ep device info
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pci_list_for_each_ep_drv_info(nvt_ep_func_ptr_t nvt_ep_func_ptr);

/**
 * @brief Register a notifier to get update when ep device info is changed
 *
 *
 * @parm p_notifier: Used to register a notify
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pci_list_register_notifier(struct nvt_ep_dev_info_notifier *p_notifier);

/**
 * @brief Notify all registered functions about ep device info
 *
 *
 * @parm void
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pci_list_call_notifier(void);

/**
 * @brief Wait until NVT RC speed change done
 *
 *
 * @parm void
 *
 * @return  >= 0 for success, < 0 for fail
 */
int nvt_pcie_wait_speed_change(struct pci_dev *dev);

/**
 * @brief set RAS sel then read RAS data
 *
 *
 * @parm ec_sle: Event Counter Data Select.
 *
 * @return data
 */

u32 glb_pcie_ras_ec(u32 ec_sel);

#endif /* __SOC_NVT_NVT_PCI_H */
