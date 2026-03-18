/*
 * MSIX driver for Novatek RC/EP interworking
 *
 * Copyright (c) 2021 - Novatek Microelectronics Corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/bitfield.h>

#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/pci.h>
#include <linux/list.h>

#include <plat/hardware.h>

#include "irq-novatek-msix-v2.h"

// directly exported from controller driver
extern int nvt_pcie_get_rc_slot(struct pci_dev *dev);

#define DRV_VERSION		"1.00.015"

#define NR_HW_IRQS		16				/* To define the source GIC irq number which is used to expend the GIC and pcie msix interrupts */
#define NR_HW_IRQ_GROP_NUM	32
#define NR_IRQ_PER_GROP		32
#define NR_HW_IRQ_TOTAL		(NR_IRQ_PER_GROP * NR_HW_IRQ_GROP_NUM)

#define MAX_INT_PER_EP		(256)

#define MSIX_PCIE_MATCH_ADDR		(0xBF0000000ULL)	/* MSIX-TX(on EP) output pcie address */
#define MSIX_PCIE2_MATCH_ADDR		(0xFF0000000ULL)	/* MSIX-TX(on EP) output pcie address */

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
#define EP_APB_SIZE		(40*1024*1024)
#define MSIX_TABLE_DBI_OFS	(0x40300000&0x00FFFFFF)
#endif


// reg offset in DBI
#define DBI_PORT_LOGIC_BASE		0x700
#define MSIX_ADDRESS_MATCH_LOW_OFF_OFS	(DBI_PORT_LOGIC_BASE + 0x240)
#define MSIX_MATCH_EN_MSK		GENMASK(0, 0)
#define MSIX_ADDR_LOW31t2		GENMASK(31, 2)
#define MSIX_ADDRESS_MATCH_HIGH_OFF_OFS	(DBI_PORT_LOGIC_BASE + 0x244)


#define IRQ_MASK        0x4
#define IRQ_STATUS      0x8

struct nvt_ep_data {
	struct pci_dev *pci_dev;
	void __iomem *apb_va;		// RC outbound to access EP APB (VA)
	unsigned long ep_out_apb_pa;	// EP outbound to access RC APB (PA)
};

struct nvt_msix_irq_data {
	struct device		*dev;
	char			name[32];

	void __iomem		*base;
	struct irq_domain	*domain;
	struct irq_chip		irq_chip;

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	struct pci_ep_engine	fake_ep;
	phys_addr_t		pa_dbi;
	phys_addr_t		pa_pcie_top;
	phys_addr_t		pa_remote_apb;
#endif
	phys_addr_t		pa_msix;
	phys_addr_t		pa_match_addr;

	int			gic_irq_num[NR_HW_IRQS];
	unsigned int		virq_tbl[NR_HW_IRQS][NR_HW_IRQ_TOTAL / NR_HW_IRQS];	// record each hwirq's virq

	spinlock_t		lock;

	struct nvt_ep_data	ep_data[CHIP_MAX];
	int			ep_count;
};

static struct nvt_ep_data g_nvt_ep_data[CHIP_MAX];
static struct nvt_ep_data g_nvt_ep_data2[CHIP_MAX];
static unsigned long rc_inbound_len;

//static int EP_COUNT = 0;

#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
static void __iomem *msix_map_region(struct pci_dev *dev, unsigned nr_entries)
{
	resource_size_t phys_addr;
	u32 table_offset;
	unsigned long flags;
	u8 bir;

	pci_read_config_dword(dev, dev->msix_cap + PCI_MSIX_TABLE,
			      &table_offset);
	bir = (u8)(table_offset & PCI_MSIX_TABLE_BIR);
	flags = pci_resource_flags(dev, bir);
	if (!flags || (flags & IORESOURCE_UNSET))
		return NULL;

	table_offset &= PCI_MSIX_TABLE_OFFSET;
	phys_addr = pci_resource_start(dev, bir) + table_offset;

	return ioremap(phys_addr, nr_entries * PCI_MSIX_ENTRY_SIZE);
}
#endif

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
static void init_msix_table(struct nvt_msix_irq_data *data, struct pci_dev *pci_dev, int ep_id)
{
	int i;
	u32 val;
	u16 ctrl;
	const int MAX_INTRS = MAX_INT_PER_EP;
	const phys_addr_t MSIX_PCIE_RC_RX_ADDR = (data->pa_msix-NVT_PERIPHERAL_PHYS_BASE) + g_nvt_ep_data[ep_id+CHIP_EP0].ep_out_apb_pa;
	void __iomem *p_tbl;
	void __iomem *p_pcie_top = ioremap(data->pa_pcie_top, 0x1000);
	void __iomem *p_ep_dbi = g_nvt_ep_data[ep_id+CHIP_EP0].apb_va + (data->pa_dbi-NVT_PERIPHERAL_PHYS_BASE);

	p_tbl = ioremap(data->pa_dbi+MSIX_TABLE_DBI_OFS, MAX_INTRS*PCI_MSIX_ENTRY_SIZE);

	if (p_tbl == NULL) {
		printk("%s: map MSIX TBL fail\r\n", __func__);
		return;
	}

	// switch to MSI-X table
	val = readl(p_pcie_top + 0x30C);
	val &= ~((1<<22) - 1);
	val |= (0x40300000>>22) << 22;
	writel(val, p_pcie_top + 0x30C);

	//
	// step 1. populate the MSI-X table
	//
	for (i=0; i<MAX_INTRS; i++) {
		const int INT_BASE = MAX_INTRS * ep_id;
		u64 target_addr = MSIX_PCIE_RC_RX_ADDR + ((i+INT_BASE)/32)*4 + 0x300;
		u32 data_mask = 1 << (i%32);

		writel(lower_32_bits(target_addr), p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_LOWER_ADDR);
		writel(upper_32_bits(target_addr), p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_UPPER_ADDR);
		writel(data_mask, p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_DATA);
		writel(0, p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_VECTOR_CTRL);	// clear mask => enable interrupt
	}

	// switch to normal DBI
	val = readl(p_pcie_top + 0x30C);
	val &= (1<<22) - 1;
	writel(val, p_pcie_top + 0x30C);

	printk("%s: ep apb va %p, offset 0x%llx\r\n", __func__, g_nvt_ep_data[ep_id+CHIP_EP0].apb_va, (data->pa_dbi-NVT_PERIPHERAL_PHYS_BASE));
	//
	// step2. setup MSIX_ADDRESS_MATCH
	//
	writel(upper_32_bits(data->pa_match_addr), p_ep_dbi+MSIX_ADDRESS_MATCH_HIGH_OFF_OFS);
	val = lower_32_bits(data->pa_match_addr);
	val |= FIELD_PREP(MSIX_MATCH_EN_MSK, 1);
	writel(val, p_ep_dbi+MSIX_ADDRESS_MATCH_LOW_OFF_OFS);

	//
	// step 3. enable MSIX function mask
	//
	ctrl = readw(p_ep_dbi + 0xB0 + PCI_MSIX_FLAGS);
	ctrl &= ~PCI_MSIX_FLAGS_MASKALL;
	ctrl |= PCI_MSIX_FLAGS_ENABLE;
	writew(ctrl, p_ep_dbi + 0xB0 + PCI_MSIX_FLAGS);

	iounmap(p_tbl);
	iounmap(p_pcie_top);
}
#else
static void init_msix_table(struct nvt_msix_irq_data *data, struct pci_dev *pci_dev, int ep_id)
{
	int i;
	u32 val;
	u16 ctrl;
	const int MAX_INTRS = MAX_INT_PER_EP;
	const phys_addr_t MSIX_PCIE_RC_RX_ADDR = (NVT_MSI_BASE_PHYS-NVT_PERIPHERAL_PHYS_BASE) + data->ep_data[ep_id+CHIP_EP0].ep_out_apb_pa;
	void __iomem *p_tbl = msix_map_region(pci_dev, MAX_INTRS);

	if (p_tbl == NULL) {
		printk("%s: map MSIX TBL fail\r\n", __func__);
		return;
	}

	//
	// step 1. populate the MSI-X table
	//
	for (i=0; i<MAX_INTRS; i++) {
		const int INT_BASE = MAX_INTRS * ep_id;
		u64 target_addr = MSIX_PCIE_RC_RX_ADDR + ((i+INT_BASE)/32)*4 + 0x300;
		u32 data_mask = 1 << (i%32);

		writel(lower_32_bits(target_addr), p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_LOWER_ADDR);
		writel(upper_32_bits(target_addr), p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_UPPER_ADDR);
		writel(data_mask, p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_DATA);
		writel(0, p_tbl + i*PCI_MSIX_ENTRY_SIZE + PCI_MSIX_ENTRY_VECTOR_CTRL);	// clear mask => enable interrupt
	}

	//
	// step2. setup MSIX_ADDRESS_MATCH
	//
	pci_write_config_dword(pci_dev, MSIX_ADDRESS_MATCH_HIGH_OFF_OFS, upper_32_bits(data->pa_match_addr));
//	writel(upper_32_bits(data->pa_match_addr), p_ep_dbi+MSIX_ADDRESS_MATCH_HIGH_OFF_OFS);
	val = lower_32_bits(data->pa_match_addr);
	val |= FIELD_PREP(MSIX_MATCH_EN_MSK, 1);
	pci_write_config_dword(pci_dev, MSIX_ADDRESS_MATCH_LOW_OFF_OFS, val);
//	writel(val, p_ep_dbi+MSIX_ADDRESS_MATCH_LOW_OFF_OFS);

	//
	// step 3. enable MSIX function mask
	//
	pci_read_config_word(pci_dev, pci_dev->msix_cap + PCI_MSIX_FLAGS, &ctrl);
	ctrl &= ~PCI_MSIX_FLAGS_MASKALL;
	ctrl |= PCI_MSIX_FLAGS_ENABLE;
	pci_write_config_word(pci_dev, pci_dev->msix_cap + PCI_MSIX_FLAGS, ctrl);

	iounmap(p_tbl);
}
#endif

/* functions for local (RC) msix recive control */
static u32 local_msix_get_pending_irq(struct nvt_msix_irq_data *chip_data, int group)
{
	u32 val;

	val = readl(chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS + (group<<2));
	return val;
}

static u32 local_msix_get_enabled_irq(struct nvt_msix_irq_data *chip_data, int group)
{
	u32 val;

	val = readl(chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2));
	return val;
}

static void local_msix_enable_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS + (group<<2));
}

static void local_msix_clr_pending_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS + (group<<2));
}

static void local_msix_disable_irq(struct nvt_msix_irq_data *chip_data, int group, int number)
{
	writel(number, chip_data->base + MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_EN_OFS + (group<<2));
}

static u32 local_msix_get_pending_grps(struct nvt_msix_irq_data *chip_data)
{
	u32 val;

	val = readl(chip_data->base + INTR_GROUP_PENDING_OFS);

	return val;
}

/* functions for remote (EP) msix transmit control */

static void ep_msix_enable_irq(struct nvt_msix_irq_data *chip_data, int ep, int number)
{
	int	num_except_spi;
	unsigned int bank;
	void __iomem *p_ep_msix = chip_data->ep_data[ep+CHIP_EP0].apb_va + (chip_data->pa_msix-NVT_PERIPHERAL_PHYS_BASE);

	number %= MAX_INT_PER_EP;
	num_except_spi = number;

	bank = num_except_spi>>5;
	number   = number & 0x1f;
	number   = 1 << number;
//	printk("%s: reg offset 0x%x, value 0x%x\r\n", __func__, MSIx_MSIx_INT_SET_EN0_OFS + (bank<<2), number);
	writel(number, p_ep_msix + MSIx_MSIx_INT_SET_EN0_OFS + (bank<<2));
}

static void ep_msix_disable_irq(struct nvt_msix_irq_data *chip_data, int ep, int number)
{
	int	num_except_spi;
	unsigned int bank;
	void __iomem *p_ep_msix = chip_data->ep_data[ep+CHIP_EP0].apb_va + (chip_data->pa_msix-NVT_PERIPHERAL_PHYS_BASE);

	number %= MAX_INT_PER_EP;
	num_except_spi = number;
	bank =  num_except_spi>>5;
	number   = number & 0x1f;
	number   = 1 << number;
	writel(number, p_ep_msix + MSIx_MSIx_INT_SET_CLR0_OFS + (bank<<2));
}

static void ep_msix_clear_tx_irq(struct nvt_msix_irq_data *chip_data, int number)
{
	int ep_id;
	unsigned int bank;
	void __iomem *p_ep_msix;

	ep_id = number / MAX_INT_PER_EP;
	number %= MAX_INT_PER_EP;

	p_ep_msix = chip_data->ep_data[ep_id+CHIP_EP0].apb_va + (chip_data->pa_msix-NVT_PERIPHERAL_PHYS_BASE);

	bank = number>>5;
	number   = number & 0x1f;
	number   = 1 << number;
	writel(number, p_ep_msix + MSIx_MSIx_INT_STS0_OFS + (bank<<2));
}


/* callbacks registered to kernel */

static void nvt_msix_int_get_status(struct nvt_msix_irq_data *chip_data, unsigned long *pending_int)
{
	int n;

	for (n = 0; n < NR_HW_IRQS; n++) {
		pending_int[n] = local_msix_get_pending_irq(chip_data, n) & local_msix_get_enabled_irq(chip_data, n);
	}

	return;
}

static int nvt_msix_chk_pending_dummy_data(struct nvt_msix_irq_data *chip_data, int number)
{
	return 1;
}



static void nvt_msix_summary_irq_handler(struct irq_desc *desc)
{
	unsigned int irq = irq_desc_get_irq(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct nvt_msix_irq_data *chip_data = irq_get_handler_data(irq);
	unsigned int virq = 0, bit = 0, n = 0;
	unsigned long pending_int[NR_HW_IRQS];
	u32 pending_grps;
	const unsigned int gic_offset = irq - chip_data->gic_irq_num[0];

	pending_grps = local_msix_get_pending_grps(chip_data);
//	printk("%s: enter, parent irq %d, data irq %d, base irq %d, pending group 0x%x\r\n", __func__, desc->parent_irq, irq, chip_data->gic_irq_num[0], pending_grps);
	if ((pending_grps & (1<<gic_offset)) || (pending_grps & (1<<(gic_offset+NR_HW_IRQS)))) {
//		printk("%s: found in pending grps\r\n", __func__);
	} else {
		printk("%s: NOT found in pending grps\r\n", __func__);
	}

	/* To read the interrupt status to find which pin is trigger: msi_int_status */
	nvt_msix_int_get_status(chip_data, pending_int);

	/* mack irq */
	chained_irq_enter(chip, desc);

	/* To find PCIE MSIX which pin is triggered */
	for (n = 0, bit = gic_offset; n < 2; n++, bit += NR_HW_IRQS)
	{
		u32 pendings;

//		printk("%s: HW GROUP %d, pending 0x%lx\r\n", __func__, n, pending_int[n]);
		if (pending_grps & (1<<bit)) {
			pendings = local_msix_get_pending_irq(chip_data, bit);

			while (pendings) {
				u32 pending_bit;

				pending_bit = __builtin_ctz(pendings);

				/* To find the virtual irq number */
				virq = irq_find_mapping(chip_data->domain, bit * NR_IRQ_PER_GROP + pending_bit);

				/* Assign virq to handle irq isr */
				generic_handle_irq(virq);

				/* Clear Interrupt Source */
				ep_msix_clear_tx_irq(chip_data, bit * NR_IRQ_PER_GROP + pending_bit);

				pendings &= ~(1<<pending_bit);
			}
//			printk("%s: HW GROUP %d, bit %d\r\n", __func__, n, bit);
			nvt_msix_chk_pending_dummy_data(chip_data, bit + (n * NR_HW_IRQ_GROP_NUM));
			/* To find the virtual irq number */
			virq = irq_find_mapping(chip_data->domain, bit + (n * NR_HW_IRQ_GROP_NUM));
//			printk("%s: virq %d found\r\n", __func__, virq);
			/* Assign virq to handle irq isr */
			generic_handle_irq(virq);
			/* Clear Interrupt Source */
			ep_msix_clear_tx_irq(chip_data, bit + (n * NR_HW_IRQ_GROP_NUM));
		}
	}

	/* unmask irq */
	chained_irq_exit(chip, desc);
}



static void msix_unmask_irq(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

	spin_lock_irqsave(&chip_data->lock, flags);

	// unmask remote EP irq
	ep_msix_enable_irq(chip_data, ep_id, data->hwirq);

	// unmask local msix-rx irq
	local_msix_enable_irq(chip_data, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}





static void msix_mask_irq(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

	spin_lock_irqsave(&chip_data->lock, flags);

	// mask remote EP irq
	ep_msix_disable_irq(chip_data, ep_id, data->hwirq);

	// mask local msix-rx irq
	local_msix_disable_irq(chip_data, data->hwirq/NR_HW_IRQ_GROP_NUM, 1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}



static void msix_irq_ack(struct irq_data *data)
{
	int ep_id;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);
	unsigned long flags;

	ep_id = data->hwirq / MAX_INT_PER_EP;

//	printk("%s: ep %d hwirq %ld\r\n", __func__, ep_id, data->hwirq);
//	printk("%s: HW GROUP %lu, bit %lu\r\n", __func__, data->hwirq/NR_HW_IRQ_GROP_NUM, data->hwirq%NR_HW_IRQ_GROP_NUM);

	spin_lock_irqsave(&chip_data->lock, flags);

	local_msix_clr_pending_irq(chip_data,
		data->hwirq/NR_HW_IRQ_GROP_NUM,
		1<<(data->hwirq%NR_HW_IRQ_GROP_NUM));

	spin_unlock_irqrestore(&chip_data->lock, flags);
}

static int msix_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
	int i;
	unsigned int cpu;
	struct nvt_msix_irq_data *chip_data = irq_data_get_irq_chip_data(data);

	if (force)
		cpu = cpumask_first(dest);
	else
		cpu = cpumask_any_and(dest, cpu_online_mask);

	if (cpu >= nr_cpu_ids)
		return -EINVAL;

	for (i = 0; i < (NR_HW_IRQ_TOTAL / NR_HW_IRQS); i++) {
		int each_virq = chip_data->virq_tbl[data->hwirq % NR_HW_IRQS][i];
		struct irq_data *pdata;
		struct irq_desc *desc;

		if (each_virq == 0) continue;
		if (each_virq == data->irq) continue;

		pdata = irq_get_irq_data(each_virq);
		desc = irq_data_to_desc(pdata);
		if (pdata) {
			desc = irq_data_to_desc(pdata);

			if (desc) {
				raw_spin_lock(&desc->lock);

				cpumask_copy(desc->irq_common_data.affinity, dest);
				irqd_mark_affinity_was_set(pdata);

				raw_spin_unlock(&desc->lock);
			}
		}
	}
	irq_data_update_effective_affinity(data, cpumask_of(cpu));
//	irq_set_affinity(chip_data->gic_irq_num[data->hwirq%NR_HW_IRQ_GROP_NUM], dest);

	return 0;
}

static struct irq_chip nvt_msix_irq_chip = {
	.name = "Novatek PCIe MSIX",
	.irq_enable = msix_unmask_irq,
	.irq_disable = msix_mask_irq,
	.irq_mask = msix_mask_irq,
	.irq_unmask = msix_unmask_irq,
	.irq_ack = msix_irq_ack,
	.irq_set_affinity = msix_set_affinity,
};

static int nvt_msix_intx_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	struct nvt_msix_irq_data *data = domain->host_data;

	if (hwirq > NR_HW_IRQ_TOTAL) {
		printk("%s: input hwirq %ld > max %d, invalid\r\n", __func__, hwirq, NR_HW_IRQ_TOTAL);
		return -EPERM;
	}

	irq_set_chip_and_handler_name(irq, &nvt_msix_irq_chip, handle_edge_irq, data->name);
	irq_set_chip_data(irq, domain->host_data);

	data->virq_tbl[hwirq % NR_HW_IRQS][hwirq / NR_HW_IRQS] = irq;
	return 0;
}

static int nvt_msix_intx_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	struct nvt_msix_irq_data *data = d->host_data;

	if (is_of_node(fwspec->fwnode)) {
		int ep_id;

		// compatible with GIC 3-tupples description
		if (fwspec->param_count < 3)
			return -EINVAL;

		if (fwspec->param[0] < CHIP_EP0) {
			dev_err(data->dev,"dts EP number %d < CHIP_EP0\r\n", fwspec->param[0]);
			return -EINVAL;
		}
		if (fwspec->param[0] > data->ep_count) {
			dev_err(data->dev, "dts EP number CHIP_EP%d, but total ep count is %d\r\n",
					fwspec->param[0]-CHIP_EP0,
					data->ep_count);
			return -EINVAL;
		}
		if (fwspec->param[0] > CHIP_EP7) {
			dev_err(data->dev, "dts EP number CHIP_EP%d, but MSIX irqchip only support 8 EPs\r\n",
					fwspec->param[0]-CHIP_EP0);
			return -EINVAL;
		}

		ep_id = fwspec->param[0] - CHIP_EP0;
		*hwirq = fwspec->param[1] + ep_id*MAX_INT_PER_EP;
		*type = IRQ_TYPE_NONE;
	}

	return 0;
}

static const struct irq_domain_ops nvt_msix_domain_ops = {
	.map = nvt_msix_intx_map,
	.translate = nvt_msix_intx_translate,
};

static int nvt_pcie_install_irq_chip(struct nvt_msix_irq_data *data)
{
	int n = 0;

	/* To create PCIE MSIX IRQ domain with total expend irq number */
	data->domain = irq_domain_add_tree(data->dev->of_node,
						  &nvt_msix_domain_ops, data);
	if (!data->domain)
		return -ENOMEM;

	/*
	 * Assign handler to PCIE MSIX hw irq which is used to connect ARM GIC
	 * Once HWIRQ is triggered by PCIE MSIX, this handler will be called.
	 */
	for (n = 0; n < NR_HW_IRQS; n++) {
//		printk("%s: install %d isr\r\n", __func__, data->gic_irq_num[n]);
		irq_set_chained_handler_and_data(data->gic_irq_num[n], nvt_msix_summary_irq_handler, data);
	}

	return 0;
}

static int _get_ep_info(struct pci_ep_info *ep)
{
//	struct pci_dev * p_root;
	int slot;

	if ((ep->ep_idx+CHIP_EP0) > CHIP_EP8) {
		printk("%s: EP id %d exceed max supported %d\r\n", __func__, ep->ep_idx, CHIP_EP8-CHIP_EP0);
		return -1;
	}

	slot = nvt_pcie_get_rc_slot(ep->pci_dev);
	if (slot < 0) return slot;

	if ((ep->dev_id == PCI_DEV_ID_NS02201) && (slot == 0)) {
		g_nvt_ep_data[ep->ep_idx+CHIP_EP0].apb_va = ep->apb_va;
		g_nvt_ep_data[ep->ep_idx+CHIP_EP0].pci_dev = ep->pci_dev;
		// hard code
//		g_nvt_ep_data[ep->ep_idx+CHIP_EP0].ep_out_apb_pa = 0xB00000000;
		g_nvt_ep_data[ep->ep_idx+CHIP_EP0].ep_out_apb_pa = ep->ep_out_apb_pa;
		rc_inbound_len = ep->ep_out_apb_len;
	}

	return 0;
}

static int _get_ep_info2(struct pci_ep_info *ep)
{
//	struct pci_dev * p_root;
	int slot;

	if ((ep->ep_idx+CHIP_EP0) > CHIP_EP8) {
		printk("%s: EP id %d exceed max supported %d\r\n", __func__, ep->ep_idx, CHIP_EP8-CHIP_EP0);
		return -1;
	}

	slot = nvt_pcie_get_rc_slot(ep->pci_dev);
	if (slot < 0) return slot;

	if ((ep->dev_id == PCI_DEV_ID_NS02201) && (slot == 1)) {
		g_nvt_ep_data2[ep->ep_idx+CHIP_EP0].apb_va = ep->apb_va;
		g_nvt_ep_data2[ep->ep_idx+CHIP_EP0].pci_dev = ep->pci_dev;
		// hard code
//		g_nvt_ep_data2[ep->ep_idx+CHIP_EP0].ep_out_apb_pa = 0xF00000000;
		g_nvt_ep_data2[ep->ep_idx+CHIP_EP0].ep_out_apb_pa = ep->ep_out_apb_pa;
		rc_inbound_len = ep->ep_out_apb_len;
	}

	return 0;
}

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
static int build_fake_ep_info(struct nvt_msix_irq_data *p_data)
{
	u32 val;
	void __iomem *p_ep_apb;

	p_ep_apb = ioremap(NVT_PERIPHERAL_PHYS_BASE, EP_APB_SIZE);
	if (IS_ERR(p_ep_apb)) {
		return PTR_ERR(p_ep_apb);
	}
	val = readl(p_ep_apb+0x10000);
	p_data->fake_ep.rc_outb_info.apb_va = p_ep_apb;
	p_data->fake_ep.pci_dev = NULL;
	p_data->fake_ep.ep_outb_info.apb[NVT_PCIE_ADDR_SYS_START] = p_data->pa_remote_apb;
	p_data->fake_ep.rc_inb_info.apb[NVT_PCIE_ADDR_SYS_LEN] = EP_APB_SIZE;

	return _get_ep_info(&p_data->fake_ep);
}
#endif

static int nvt_msix_probe(struct platform_device *pdev)
{
	struct nvt_msix_irq_data *data;
	struct resource *res;
	int i;
	int ep_count;

	dev_info(&pdev->dev, "NVT MSIX Probing...\r\n");

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dev = &pdev->dev;
	rc_inbound_len = 0;

#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	printk("%s: loopback en\r\n", __func__);
#else
	printk("%s: loopback DIS\r\n", __func__);
#endif
	printk("%s: EP count %d\r\n", __func__, nvtpcie_get_ep_count(PCI_DEV_ID_NS02201));

#if (!IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	printk("%s: no pipe loopback\r\n", __func__);
#else
	printk("%s: HAS pipe loopback\r\n", __func__);
#endif


	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	data->pa_msix = res->start;
	data->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	// get each EP APB VA
#if (IS_ENABLED(CONFIG_PCIE_PIPE_LOOPBACK))
	if (res->start == NVT_MSI_BASE_PHYS) {
		printk("%s: MSI ctrl 1\r\n", __func__);
		data->pa_dbi = NVT_PCIE_DBI_BASE_PHYS;
		data->pa_pcie_top = NVT_PCIE_BASE_PHYS;
		data->pa_remote_apb = 0xB00000000;
	} else {
		printk("%s: MSI ctrl 2\r\n", __func__);
		data->pa_dbi = NVT_PCIE2_DBI_BASE_PHYS;
		data->pa_pcie_top = NVT_PCIE2_BASE_PHYS;
		data->pa_remote_apb = 0xF00000000;
	}
	printk("MSI BASE 0x%llx, PCIE TOP BASE 0x%llx, PCIE DBI BASE 0x%llx\r\n", data->pa_msix, data->pa_pcie_top, data->pa_dbi);
	build_fake_ep_info(data);
	ep_count = 1;
#else
//	nvt_pci_list_for_each_ep_drv_info(_get_ep_info);
	ep_count = nvtpcie_get_ep_count(PCI_DEV_ID_NS02201);	// this will get total EP count on both RC port0 and port1
#endif
	if (res->start == NVT_MSI_BASE_PHYS) {
		memset(g_nvt_ep_data, 0, sizeof(g_nvt_ep_data));
		nvt_pci_list_for_each_ep_drv_info(_get_ep_info);
		memcpy(data->ep_data, g_nvt_ep_data, sizeof(g_nvt_ep_data));
		strncpy(data->name, "CTRL1", sizeof(data->name));
		data->pa_match_addr = MSIX_PCIE_MATCH_ADDR;
	} else {
		memset(g_nvt_ep_data2, 0, sizeof(g_nvt_ep_data2));
		nvt_pci_list_for_each_ep_drv_info(_get_ep_info2);
		memcpy(data->ep_data, g_nvt_ep_data2, sizeof(g_nvt_ep_data2));
		strncpy(data->name, "CTRL2", sizeof(data->name));
		data->pa_match_addr = MSIX_PCIE2_MATCH_ADDR;
	}
	data->ep_count = ep_count;

//	printk("MSI BASE MACRO 0x%lx, PHY BASE 0x%lx\r\n", NVT_MSI_BASE_PHYS, NVT_PERIPHERAL_PHYS_BASE);
	if (rc_inbound_len < (data->pa_msix-NVT_PERIPHERAL_PHYS_BASE)) {
		dev_err(&pdev->dev, "RC inbound size 0x%lx < 0x%llx\n",
				rc_inbound_len, data->pa_msix-NVT_PERIPHERAL_PHYS_BASE);
		dev_err(&pdev->dev, "Please check dma-ranges setting in PCIe controller dts node\n");
		return -EINVAL;
	}

	spin_lock_init(&data->lock);

	for (i = 0; i < NR_HW_IRQS; i++) {
		data->gic_irq_num[i] = platform_get_irq(pdev, i);

		if (data->gic_irq_num[i] < 0) {
			dev_err(&pdev->dev, "failed to get parent IRQ\n");
			return -EINVAL;
		}
	}

	printk("%s: ep count %d\r\n", __func__, ep_count);
	for (i=0; i<ep_count; i++) {
		u32 val;
		u32 addr_msb = 0;

		if (data->ep_data[i+CHIP_EP0].apb_va == NULL) {
			printk("%s: CTRL base 0x%llx: loop %d: EP data is NULL, skip\r\n",
				__func__, res->start, i);
			continue;
		}

		// PLL enable (dirty code in 1st version)
		val = readl(data->ep_data[i+CHIP_EP0].apb_va + (NVT_CG_BASE_PHYS-NVT_PERIPHERAL_PHYS_BASE));
		val |= GENMASK(2, 2);		// enable PLL2 (AXI1)
		val |= GENMASK(13, 13);		// enable PLL13 (AXI2)
		writel(val, data->ep_data[i+CHIP_EP0].apb_va + (NVT_CG_BASE_PHYS-NVT_PERIPHERAL_PHYS_BASE));

		// MSIX clk enable (dirty code in 1st version)
		val = readl(data->ep_data[i+CHIP_EP0].apb_va + (NVT_CG_BASE_PHYS-NVT_PERIPHERAL_PHYS_BASE) + 0x8C);
		val |= GENMASK(19, 19);		// MSI_CLKEN
		val |= GENMASK(20, 20);		// MSI2_CLKEN
		writel(val, data->ep_data[i+CHIP_EP0].apb_va + (NVT_CG_BASE_PHYS-NVT_PERIPHERAL_PHYS_BASE) + 0x8C);

		/* Initialize controllers */
		init_msix_table(data, data->ep_data[i+CHIP_EP0].pci_dev, i);	// init PCIE MSIX table on each EP

		writel(lower_32_bits(data->pa_match_addr),
			data->ep_data[i+CHIP_EP0].apb_va + (data->pa_msix-NVT_PERIPHERAL_PHYS_BASE) + AXI_CMD_ADDR_LSB_OFS);
		addr_msb = FIELD_PREP(AXI_CMD_ADDR_MSB_MSK, upper_32_bits(data->pa_match_addr));
		addr_msb |= FIELD_PREP(MSI_SEL_MSK, MSIx_SEL_TYPE_AXICMD);
		writel(addr_msb, data->ep_data[i+CHIP_EP0].apb_va + (data->pa_msix-NVT_PERIPHERAL_PHYS_BASE) + AXI_CMD_ADDR_MSB_OFS);
	}

	/* To install PCIE IRQ chip */
	nvt_pcie_install_irq_chip(data);

	platform_set_drvdata(pdev, data);

	dev_info(&pdev->dev, "NVT MSIX probe done\r\n");

	return 0;
}

static int nvt_msix_remove(struct platform_device *pdev)
{
	struct nvt_msix_irq_data *data = platform_get_drvdata(pdev);

	irq_domain_remove(data->domain);

	return 0;
}

static const struct of_device_id nvt_msix_of_match[] = {
	{ .compatible = "nvt,pcie-msix-v2", },
	{},
};
MODULE_DEVICE_TABLE(of, nvt_msix_of_match);

static struct platform_driver nvt_msix_driver = {
	.probe  = nvt_msix_probe,
	.remove = nvt_msix_remove,
	.driver = {
		.name = "nvt-pcie-msix-irqchip",
		.of_match_table = nvt_msix_of_match,
	},
};
module_platform_driver(nvt_msix_driver);

MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");
