/**
    NVT PCIE cascade driver internal header
    Provide driver's internal prototypes
    @file nvt-pcie-int.h
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

#include "nvt-msix.h"
#include "pcie-drv-wrap.h"

#ifndef __NVT_PCIE_INT__
#define __NVT_PCIE_INT__

//#define NVT_PCIE_LOOPBACK	1

#ifdef NVT_PCIE_LOOPBACK
#define MSIX_PCIE_MATCH_ADDR		(0x600000000ULL)	/* MSIX-TX(on EP) output pcie address */
#else
#define MSIX_PCIE_MATCH_ADDR		(0xF00000000ULL)	/* MSIX-TX(on EP) output pcie address */
#endif

#define NR_HW_IRQS		32				/* To define the source GIC irq number which is used to expend the GIC and pcie msix interrupts */
#define NR_HW_IRQ_GROP_NUM	32
#define NR_HW_IRQ_TOTAL		(NR_HW_IRQS * NR_HW_IRQ_GROP_NUM)

// edma interrupt mode
#define PCIE_EDMA_INT_EN  1

// described in DTS
#define MEM_MSIX		(0)
#define MEM_EP_MSIX_CTRL	(1)
#define MEM_PCIE_TOP		(2)
#define MEM_EP_PCIE_TOP		(3)
#define MEM_PCIE_DBI		(4)
#define MEM_EP_DBI		(5)

// described in private array
#define MEM_EP_MSIX_TBL		(6)
#define MEM_EP_ATU		(7)
#define MEM_EP_CFG		(8)

#define MEM_DTS_COUNT		(6)	// expect how many region from dts
#define MEM_PRIVATE_COUNT	(3)
#define MEM_COUNT		(MEM_DTS_COUNT+MEM_PRIVATE_COUNT)


#define PCIE_SETREG(chip, ofs, value)	writel((value), (chip)->PCIE_REG_BASE[MEM_PCIE_TOP] + (ofs))
#define PCIE_GETREG(chip, ofs)		readl((chip)->PCIE_REG_BASE[MEM_PCIE_TOP] + (ofs))

#define PCIE_SETDBI(chip, ofs, value)	writel((value), (chip)->PCIE_REG_BASE[MEM_PCIE_DBI] + (ofs))
#define PCIE_GETDBI(chip, ofs)		readl((chip)->PCIE_REG_BASE[MEM_PCIE_DBI] + (ofs))

#define PCIE_EP_SETMSIX(chip, ofs, value)	writel((value), (chip)->PCIE_REG_BASE[MEM_EP_ATU] + (ofs) + 0x20000)
#define PCIE_EP_GETMSIX(chip, ofs)		readl((chip)->PCIE_REG_BASE[MEM_EP_ATU] + (ofs) + 0x20000)

#define PCIE_EP_SETDBI(chip, ofs, value)	writel((value), (chip)->PCIE_REG_BASE[MEM_EP_PCIE_TOP] + (ofs) + 0x0D000000)
#define PCIE_EP_GETDBI(chip, ofs)		readl((chip)->PCIE_REG_BASE[MEM_EP_PCIE_TOP] + (ofs) + 0x0D000000)

#define PCIE_EP_SETCFG(chip, ofs, value)	writel((value), (chip)->PCIE_REG_BASE[MEM_EP_CFG] + (ofs))
#define PCIE_EP_GETCFG(chip, ofs)		readl((chip)->PCIE_REG_BASE[MEM_EP_CFG] + (ofs))

struct nvt_pcie_region {
	const char		*name;
	phys_addr_t		addr;
	size_t			size;
};


#define NVT_PCIE_RC		(0)
#define NVT_PCIE_EP		(1)

enum {
	EOM_SM_IDLE,
	EOM_SM_SCAN_ONGOING,
	EOM_SM_SCAN_DONE,
};

struct nvt_pci_ctrl {
	phys_addr_t dbi_base;		// DBI base (physical address)
	phys_addr_t atu_base;		// ATU base (physical address)
	phys_addr_t cfg_base;		// CFG base (physical address)

	void __iomem *dbi_ptr;		// DBI base (virtual address)
	void __iomem *atu_ptr;		// ATU base (virtual address)
	void __iomem *cfg_ptr;		// CFG base (virtual address)
	void __iomem *msix_ctrl_ptr;	// MSIX controller base (virtual address)
	void __iomem *msix_tbl_ptr;	// MSIX table base (virtual address)
//	struct pci_region regions[MAX_PCI_REGIONS];
//	int region_count;
	int mode;	// NVT_PCIE_RC or NVT_PCIE_EP
	int last_atu;	// record last accessed ATU type (for EP multiplex) (DBI offset 0x30_0000 is shared by ATU/DMA and MSIX table)

	// If ctrl is RC, phys_map stands for PCIE address accessed by EP.
	// If ctrl is EP, phys_map stands for PCIE address accessed by RC.
	nvt_pcie_phy_map phys_map[ATU_MAP_COUNT];

	struct nvt_pcie_chip *chip;

	u32 (*read_dbi)(struct nvt_pci_ctrl*, int reg);
	int (*write_dbi)(struct nvt_pci_ctrl*, int reg, u32);

	u32 (*read_cfg)(struct nvt_pci_ctrl*, int reg);
	int (*write_cfg)(struct nvt_pci_ctrl*, int reg, u32);

	u32 (*read_atu)(struct nvt_pci_ctrl*, int reg);
	int (*write_atu)(struct nvt_pci_ctrl*, int reg, u32);

	u32 (*read_msix_ctrl)(struct nvt_pci_ctrl*, int reg);
	int (*write_msix_ctrl)(struct nvt_pci_ctrl*, int reg, u32);

	u32 (*read_msix_tbl)(struct nvt_pci_ctrl*, int reg);
	int (*write_msix_tbl)(struct nvt_pci_ctrl*, int reg, u32);

	void (*config_oATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en);
	void (*config_iATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en);
};

struct nvt_pcie_chip {
	struct device		*dev;
	struct device_node	*node;
	struct irq_domain	*domain;
	struct resource 	*res[MEM_COUNT];
	void __iomem		*PCIE_REG_BASE[MEM_COUNT];
	unsigned int		gic_irq_num[NR_HW_IRQS];			/* Shared IRQ: GIC to PCIE MISX, 32 will expend to 32*32 */
	unsigned long		msi_int_status[NR_HW_IRQS];			/* Interrupt status: 1 GIC is mapped to 32 MISX, we have 32 GIC SPIINT */
	unsigned int		irq_summary;				/* Total IRQ numbers */
	int			irq;					/* pcie controller irq to GIC */
	spinlock_t		lock;
	spinlock_t		lock_eom;				/* protect member state_eom */
	spinlock_t		lock_ras;				/* protect ras function */

	dma_addr_t		dma_msix_dummy_addr;
	void			*va_msix_dummy_addr;

	bool			is_rc;		/* true: ourself is RC */

	struct nvt_pci_ctrl	rc;
	struct nvt_pci_ctrl	ep;

	struct proc_dir_entry	*pproc_module_root;
	struct proc_dir_entry	*pproc_help_entry;
	struct proc_dir_entry	*pproc_cmd_msix;

	struct delayed_work	eom_work;			/* work queue for scan rx eye */
	int			state_eom;			/* sw state machine for scan rx eye */
	int			eom_result[64][128];
	unsigned long		eom_start_jiffies;
	unsigned long		eom_end_jiffies;
};

/**
    Interrupt module ID

    Interrupt module ID for int_getIRQId() and int_getDummyId().
*/
#define INT_GIC_SPI_START_ID    32

typedef enum {
	INT_ID_TIMER = INT_GIC_SPI_START_ID,
	INT_ID_VPELITE,
	INT_ID_DMACP,
	INT_ID_VPE,

	INT_ID_VCAP,
	INT_ID_VCAP2,
	INT_ID_DEI,
	INT_ID_VENC,
	INT_ID_VENC2,
	INT_ID_VDEC,
	INT_ID_OSG,
	INT_ID_ETHERNET2,
	INT_ID_ETHERNET,
	INT_ID_SATA0,
	INT_ID_SATA1,
	INT_ID_DAI,
	INT_ID_AGE,
	INT_ID_JPEG,
	INT_ID_LCD,
	INT_ID_LCD2,
	INT_ID_LCD3,
	INT_ID_DEI2,
	INT_ID_GPENC,
	INT_ID_GPENC2,
	INT_ID_GPIO,
	INT_ID_REMOTE,
	INT_ID_PWM,
	INT_ID_USB,
	INT_ID_USB3,
	INT_ID_NAND,
	INT_ID_SDIO,
	INT_ID_PCIE,

	INT_ID_ETHERNET_TX,
	INT_ID_MAU,
	INT_ID_DAI2,
	INT_ID_DAI3,
	INT_ID_DAI4,
	INT_ID_DAI5,
	INT_ID_UART4,
	INT_ID_ETHERNET_RX,
	INT_ID_ETHERNET2_RX,
	INT_ID_I2C,
	INT_ID_I2C2,
	INT_ID_UART,
	INT_ID_UART2,
	INT_ID_UART3,
	INT_ID_HASH,
	INT_ID_UART5,
	INT_ID_RSA,
	INT_ID_ETHERNET2_TX,
	INT_ID_I2C4,
	INT_ID_TVE100,
	INT_ID_HDMI,
	INT_ID_MAU2,
	INT_ID_CSI,
	INT_ID_CSI2,
	INT_ID_RTC,
	INT_ID_WDT,
	INT_ID_CG,
	INT_ID_CSI3,
	INT_ID_I2C3,
	INT_ID_SDIO2,
	INT_ID_GPIO2,
	INT_ID_GPIO3,

	INT_ID_CSI4,
	INT_ID_CSI5,
	INT_ID_CSI6,
	INT_ID_CSI7,
	INT_ID_CSI8,
	INT_ID_SATA3,
	INT_ID_HWCP,
	INT_ID_IVE,
	INT_ID_NUE,
	INT_ID_NUE2,
	INT_ID_SSCA,
	INT_ID_CNN,
	INT_ID_CNN2,
	INT_ID_CNN3,
	INT_ID_USB2,
	INT_ID_SCE,
	INT_ID_GPIO4,
	INT_ID_CC,
	INT_ID_CC1,
	INT_ID_CC2,
	INT_ID_CC3,

	INT_ID_RSV0,
	INT_ID_RSV1,
	INT_ID_RSV2,
	INT_ID_RSV3,
	INT_ID_RSV4,
	INT_ID_RSV5,
	INT_ID_RSV6,
	INT_ID_RSV7,
	INT_ID_RSV8,
	INT_ID_RSV9,
	INT_ID_SUDO_INT_MSIx,	//95

	INT_ID_MSIx_0,
	INT_ID_MSIx_1,
	INT_ID_MSIx_2,
	INT_ID_MSIx_3,

	INT_ID_MSIx_4,
	INT_ID_MSIx_5,
	INT_ID_MSIx_6,
	INT_ID_MSIx_7,

	INT_ID_MSIx_8,
	INT_ID_MSIx_9,
	INT_ID_MSIx_10,
	INT_ID_MSIx_11,

	INT_ID_MSIx_12,
	INT_ID_MSIx_13,
	INT_ID_MSIx_14,
	INT_ID_MSIx_15,

	INT_ID_MSIx_16,
	INT_ID_MSIx_17,
	INT_ID_MSIx_18,
	INT_ID_MSIx_19,

	INT_ID_MSIx_20,
	INT_ID_MSIx_21,
	INT_ID_MSIx_22,
	INT_ID_MSIx_23,

	INT_ID_MSIx_24,
	INT_ID_MSIx_25,
	INT_ID_MSIx_26,
	INT_ID_MSIx_27,

	INT_ID_MSIx_28,
	INT_ID_MSIx_29,
	INT_ID_MSIx_30,
	INT_ID_MSIx_31,
	//
	//
	//

	INT_ID_CNT,
	INT_ID_MAX = INT_ID_CNT - INT_GIC_SPI_START_ID, //94


	INT_ID_WFI = 229,
	INT_ID_GIC_TOTAL = 256,
	ENUM_DUMMY4WORD(INT_ID)
} INT_ID;

//
// Exported from nvt-pcie-proc.c
//
extern int	nvt_pcie_proc_init(struct nvt_pcie_chip *nvt_pcie_chip_ptr);
extern int	nvt_pcie_proc_remove(struct nvt_pcie_chip *nvt_pcie_chip_ptr);

//
// Exported from nvt_pcie_ctrl.c
//
extern void	init_msix_table(struct nvt_pcie_chip *nvt_pcie_chip_ptr);
extern int 	setup_controller_hdl(struct nvt_pcie_chip *chip);
extern int 	check_pcie_link(void);
extern void	dump_pcie_dbi(struct nvt_pcie_chip *nvt_pcie_chip_ptr);
extern void 	dump_pcie_msix_tbl(struct nvt_pcie_chip *chip);

//edma
extern int nvt_pcie_edma_init(struct nvt_pcie_chip *chip_ptr,void *pcie_dbi_vbase, int pcie_id);

//tba
extern int nvt_pcie_tba_init(struct nvt_pcie_chip *chip_ptr,void *pcie_dbi_vbase, int pcie_id);

//
// Exported from nvt_msix.c
//
extern ER	msix_open(struct nvt_pcie_chip *nvt_pcie_chip_ptr, MSIx_SEL_TYPE msix_type, UINT32 lsb, UINT32 msb);
extern ER	msix_set_config(struct nvt_pcie_chip *nvt_pcie_chip_ptr, MSIx_CONFIG_ID cfg_id, u64 ui_config);
extern void	msix_enable_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number);
extern void	msix_disable_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number);
extern void	msix_clear_msix_tx_irq_status(struct nvt_pcie_chip *nvt_pcie_chip_ptr, int number);
extern void	msix_recv_cmd_set_en_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number);
extern void	msix_recv_cmd_clr_en_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number);
extern void	msix_recv_cmd_set_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number);
extern UINT32	msix_recv_cmd_get_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group);
extern UINT32 	msix_recv_cmd_chk_pending_dummy_data(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 number);
extern UINT32	msix_recv_cmd_get_enabled_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group);
extern void	msix_recv_cmd_clr_pending_irq(struct nvt_pcie_chip *nvt_pcie_chip_ptr, UINT32 group, UINT32 number);

extern void	msix_proc_clear_pending(struct nvt_pcie_chip *nvt_pcie_chip_ptr);
extern void	msix_proc_list_pending(struct seq_file *seq, struct nvt_pcie_chip *nvt_pcie_chip_ptr);


#if PCIE_EDMA_INT_EN
extern irqreturn_t nvt_pcie_drv_isr(int irq, void *devid);
#endif

//
// Exported to pcie-drv-wrap
//
extern int is_nvt_pcie_init(void);
extern int is_nvt_ep_init(void);

#endif


