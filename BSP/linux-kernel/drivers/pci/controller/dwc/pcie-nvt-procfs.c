/**
    @file       pcie-nvt-procfs.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/regmap.h>
#include <linux/soc/nvt/nvt-pci.h>
#include <linux/uaccess.h>
#include <linux/bitfield.h>

#include "pcie-designware.h"
#include "pcie-nvt.h"

#ifdef CONFIG_PROC_FS
#define MAX_CMD_LENGTH 30

#define EC_G0_NUM     7
#define EC_G1_NUM     11
#define EC_G2_NUM     8
#define EC_G3_NUM     6

struct proc_dir_entry   *nvt_plat_pcie_proc_dir_root = NULL;
struct dw_pcie *glb_pci = NULL;

char g0_str[EC_G0_NUM][MAX_CMD_LENGTH] = {"EBUF Overflow", "EBUF Under-run", "Decode Error", "Running Disparity Error", "SKP OS Parity Error", "SYNC Header Error", "Rx Valid de-assertion"};
char g1_str[EC_G1_NUM][MAX_CMD_LENGTH] = {"Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Detect EI Infer", "Receiver Error", "Rx Recovery Request", "N_FTS Timeout", "Framing Error", "Deskew Error"};
char g2_str[EC_G2_NUM][MAX_CMD_LENGTH] = {"BAD TLP", "LCRC Error", "BAD DLLP", "Replay Number Rollover", "Replay Timeout", "Rx Nak DLLP", "Tx Nak DLLP", "Retry TLP"};
char g3_str[EC_G3_NUM][MAX_CMD_LENGTH] = {"FC Timeout", "Poisoned TLP", "ECRC Error", "Unsupported Request", "Completer Abort", "Completion Timeout"};

#define PCIE_TOP_BASE		(0x2F04F0000)
// PCIETOP 0x08
#define PCIETOP_STS_OFS		(0x08)
#define PCIETOP_PHY_LINK_MSK	GENMASK(11, 11)
#define PCIETOP_CTRL_LINK_MSK	GENMASK(0, 0)
#define PCIETOP_LTSSM_MSK	GENMASK(9, 4)

#define PCIE_PHY_BASE		(0x2F08B0000)
// phy reg 0xA00
#define PHY_AGC_MSK		GENMASK(3, 0)
#define PHY_CSPK_MSK		GENMASK(14, 12)
#define PHY_RSPK_MSK		GENMASK(19, 8)

// phy reg 0xA08
#define PHY_CDR_TC_MSK		GENMASK(25, 16)
#define PHY_CDR_CT_MSK		GENMASK(6, 0)

// phy reg 0xA0C
#define PHY_CDR_FT_MSK		GENMASK(6, 0)

// phy reg 0xA18
#define PHY_FREQ_VAL_TXCLK_MSK	GENMASK(29, 16)
#define PHY_FREQ_VAL_RXCLK_MSK	GENMASK(13, 0)

// phy reg 0x3018
#define PHY_DEC_ERR_MSK		GENMASK(27, 24)	// 128/130b decode err
#define PHY_EBUF_OVF_MSK	GENMASK(28, 28)	// EBUF overflow
#define PHY_EBUF_UNF_MSK	GENMASK(29, 29)	// EBUF underflow

/* All ep dev info proc */

static int nvt_plat_pcie_proc_ep_info_show(struct seq_file *m, void *v)
{
	struct nvt_ep_dev_info *curr_info, *next_info;
	struct dw_pcie *pci = (struct dw_pcie *)m->private;
	struct nvt_plat_pcie *nvt_pcie = to_nvt_plat_pcie(pci);
	int i;

	down(&nvt_pcie->sema_ep_info);
	seq_printf(m, "The overall of the EP device info: \n");

	list_for_each_entry_safe(curr_info, next_info, &nvt_pcie->ep_info_list_root, list) {
		seq_printf(m, "\rEP IDX: %d \n", curr_info->ep_drv_info->ep_idx);
		seq_printf(m, "\rRC Outbound >>\n \
					\r\tmmio: 0x%08lx@0x%08lx (va: 0x%08lx), bus:0x%08lx \n \
					\r\t apb: 0x%08lx@0x%08lx (va: 0x%08lx), bus:0x%08lx \n \
					\r\t atu: 0x%08lx@0x%08lx (va: 0x%08lx) \n",
			   curr_info->ep_drv_info->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->rc_outb_info.mmio[NVT_PCIE_ADDR_SYS_START],
			   (unsigned long)curr_info->ep_drv_info->rc_outb_info.mmio_va,
			   curr_info->ep_drv_info->rc_outb_info.mmio[NVT_PCIE_ADDR_BUS],
			   curr_info->ep_drv_info->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->rc_outb_info.apb[NVT_PCIE_ADDR_SYS_START],
			   (unsigned long)curr_info->ep_drv_info->rc_outb_info.apb_va,
			   curr_info->ep_drv_info->rc_outb_info.apb[NVT_PCIE_ADDR_BUS],
			   curr_info->ep_drv_info->atu_info.atu[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->atu_info.atu[NVT_PCIE_ADDR_SYS_START],
			   (unsigned long)curr_info->ep_drv_info->atu_info.atu_va);
		seq_printf(m, "\rEP Inbound >>\n");
		for (i = 0; i < NVT_PCIE_INBOUND_TOTAL_SEG_SIZE; i++) {
			if (curr_info->ep_drv_info->ep_inb_info.mmio[i][NVT_PCIE_ADDR_SYS_LEN] > 0) {
				seq_printf(m, "\r\tmmio%d: 0x%08lx@0x%08lx, bus:0x%08lx \n",
						i,
						curr_info->ep_drv_info->ep_inb_info.mmio[i][NVT_PCIE_ADDR_SYS_LEN],
						curr_info->ep_drv_info->ep_inb_info.mmio[i][NVT_PCIE_ADDR_SYS_START],
						curr_info->ep_drv_info->ep_inb_info.mmio[i][NVT_PCIE_ADDR_BUS]);
			}
		}
		seq_printf(m,"\r\t  apb: 0x%08lx@0x%08lx, bus:0x%08lx  \n \
				\r\t  atu: 0x%08lx@0x%08lx  \n",
				curr_info->ep_drv_info->ep_inb_info.apb[NVT_PCIE_ADDR_SYS_LEN],
				curr_info->ep_drv_info->ep_inb_info.apb[NVT_PCIE_ADDR_SYS_START],
				curr_info->ep_drv_info->ep_inb_info.apb[NVT_PCIE_ADDR_BUS],
				curr_info->ep_drv_info->atu_info.atu[NVT_PCIE_ADDR_SYS_LEN],
				curr_info->ep_drv_info->atu_info.atu[NVT_PCIE_ADDR_SYS_START]);
		seq_printf(m, "\rRC Inbound >>\n \
					\r\tmmio: 0x%08lx@0x%08lx \n \
					\r\t apb: 0x%08lx@0x%08lx \n ",
			   curr_info->ep_drv_info->rc_inb_info.mmio[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->rc_inb_info.mmio[NVT_PCIE_ADDR_SYS_START],
			   curr_info->ep_drv_info->rc_inb_info.apb[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->rc_inb_info.apb[NVT_PCIE_ADDR_SYS_START]);
		seq_printf(m, "\rEP Outbound >>\n \
					\r\tmmio: 0x%08lx@0x%08lx \n \
					\r\t apb: 0x%08lx@0x%08lx  \n\n",
			   curr_info->ep_drv_info->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->ep_outb_info.mmio[NVT_PCIE_ADDR_SYS_START],
			   curr_info->ep_drv_info->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_LEN],
			   curr_info->ep_drv_info->ep_outb_info.apb[NVT_PCIE_ADDR_SYS_START]);
	}
	up(&nvt_pcie->sema_ep_info);

	return 0;
}

static int nvt_plat_pcie_proc_ep_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_ep_info_show, PDE_DATA(inode));
}

static const struct proc_ops nvt_plat_pcie_proc_ep_info_fops = {
	.proc_open = nvt_plat_pcie_proc_ep_info_open,
	.proc_read = seq_read,
};


/* ERR_STAT proc */
u32 pcie_aer_get_uncorr(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, AER_UNCORR_STATUS_OFS);
	dw_pcie_writel_dbi(pci, AER_UNCORR_STATUS_OFS, val);

	return val;
}

u32 pcie_aer_get_corr(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, AER_CORR_STATUS_OFS);
	dw_pcie_writel_dbi(pci, AER_CORR_STATUS_OFS, val);

	return val;
}


void pcie_ras_ec_set(struct dw_pcie *pci, u32  ec_sel)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, EVENT_COUNTER_CONTROL_OFS);
	val &= 0x0000FFFF;
	dw_pcie_writel_dbi(pci, EVENT_COUNTER_CONTROL_OFS, val | (ec_sel << 16));
}

//[sel]
//0x1 (PER_EVENT_OFF): per event off
//0x3 (PER_EVENT_ON): per event on
//0x5 (ALL_OFF): all off
//0x7 (ALL_ON): all on
void pcie_ras_ec_start(struct dw_pcie *pci, u32 sel)
{
	u32 val;

	if (sel > 7)
		sel = 1;

	if ((sel == 2) || (sel == 4) || (sel == 6))
		sel = 1;

	val = dw_pcie_readl_dbi(pci, EVENT_COUNTER_CONTROL_OFS);
	val &= 0xFFFFFFE3;
	dw_pcie_writel_dbi(pci, EVENT_COUNTER_CONTROL_OFS, val | (sel << 2));
}

//[sel]
//0x0 (NO_CHANGE): no change
//0x1 (PER_CLEAR): per clear
//0x2 (NO_CHANGE_2): no change
//0x3 (ALL_CLEAR): all clear
void pcie_ras_ec_stop(struct dw_pcie *pci, u32 sel)
{
	u32 val;

	if (sel > 3)
		sel = 0;

	val = dw_pcie_readl_dbi(pci, EVENT_COUNTER_CONTROL_OFS);
	val &= 0xFFFFFFFC;
	dw_pcie_writel_dbi(pci, EVENT_COUNTER_CONTROL_OFS, val | sel);
}

u32 pcie_ras_ec_data(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, EVENT_COUNTER_DATA_OFS);

	return val;
}

u32 glb_pcie_ras_ec(u32 ec_sel)
{
	u32 val;
	struct nvt_plat_pcie *nvt_pcie_chip_ptr = to_nvt_plat_pcie(glb_pci);

	spin_lock(&nvt_pcie_chip_ptr->lock_ras);

	val = dw_pcie_readl_dbi(glb_pci, EVENT_COUNTER_CONTROL_OFS);
	val &= 0x0000FFFF;
	dw_pcie_writel_dbi(glb_pci, EVENT_COUNTER_CONTROL_OFS, val | (ec_sel << 16));

	val = dw_pcie_readl_dbi(glb_pci, EVENT_COUNTER_DATA_OFS);
	spin_unlock(&nvt_pcie_chip_ptr->lock_ras);

	return val;
}
EXPORT_SYMBOL(glb_pcie_ras_ec);

static int nvt_plat_pcie_proc_err_stat_show(struct seq_file *m, void *v)
{
	struct dw_pcie *pci = (struct dw_pcie *)m->private;
    struct nvt_plat_pcie *nvt_pcie_chip_ptr = to_nvt_plat_pcie(pci);

	u32 Sel, data, i, j;

	seq_printf(m, "\r\n Test RAS D.E.S --- event count analysis function \r\n\n");

	Sel = 0x0000;

    spin_lock(&nvt_pcie_chip_ptr->lock_ras);

	i = 0;
	seq_printf(m, "=== Physical Layer 0 === (Grp %d)\n\n\t", i);
	for (j = 0; j < EC_G0_NUM; j++) {
		pcie_ras_ec_set(pci, Sel | j);
		data = pcie_ras_ec_data(pci);
		seq_printf(m, "%d : [%-25s]: 0x%08x", j, g0_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(m, "\n\t");
		else
			seq_printf(m, " ;\t ");
	}
	seq_printf(m, "\r\n");
	Sel += 0x100;

	i = 1;
	seq_printf(m, "\r\n=== Physical Layer 1 === (Grp %d)\n\n\t", i);
	for (j = 0; j < EC_G1_NUM; j++) {
		pcie_ras_ec_set(pci, Sel | j);
		data = pcie_ras_ec_data(pci);
		seq_printf(m, "%d : [%-25s]: 0x%08x", j, g1_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(m, "\n\t");
		else
			seq_printf(m, " ;\t ");
	}
	seq_printf(m, "\r\n");
	Sel += 0x100;

	i = 2;
	seq_printf(m, "\r\n=== Data Link Layer === (Grp %d)\n\n\t", i);
	for (j = 0; j < EC_G2_NUM; j++) {
		pcie_ras_ec_set(pci, Sel | j);
		data = pcie_ras_ec_data(pci);
		seq_printf(m, "%d : [%-25s]: 0x%08x", j, g2_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(m, "\n\t");
		else
			seq_printf(m, " ;\t ");
	}
	seq_printf(m, "\r\n");
	Sel += 0x100;

	i = 3;
	seq_printf(m, "\r\n=== Transaction Layer === (Grp %d)\n\n\t", i);
	for (j = 0; j < EC_G3_NUM; j++) {
		pcie_ras_ec_set(pci, Sel | j);
		data = pcie_ras_ec_data(pci);
		seq_printf(m, "%d : [%-25s]: 0x%08x", j, g3_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(m, "\n\t");
		else
			seq_printf(m, " ;\t ");
	}
	seq_printf(m, "\r\n");

    spin_unlock(&nvt_pcie_chip_ptr->lock_ras);

	seq_printf(m, "\r\n ----------------------------------------------------\r\n");

	seq_printf(m, "AER uncorr status :\n\t");
	data = pcie_aer_get_uncorr(pci);
	seq_printf(m, "0x%08x, ", data);
	seq_printf(m, "\r\n\n");

	seq_printf(m, "AER corr status :\n\t");
	data = pcie_aer_get_corr(pci);
	seq_printf(m, "0x%08x, ", data);
	seq_printf(m, "\r\n");

	return 0;
}

static int nvt_plat_pcie_proc_err_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_err_stat_show, PDE_DATA(inode));
}

static const struct proc_ops nvt_plat_pcie_proc_err_stat_fops = {
	.proc_open = nvt_plat_pcie_proc_err_stat_open,
	.proc_read = seq_read,
};

static int cursor2preset(int cursor)
{
	switch (cursor) {
	case PCIE_EQ_CURSOR_P0:
		return 0;
	case PCIE_EQ_CURSOR_P1:
		return 1;
	case PCIE_EQ_CURSOR_P2:
		return 2;
	case PCIE_EQ_CURSOR_P3:
		return 3;
	case PCIE_EQ_CURSOR_P4:
		return 4;
	case PCIE_EQ_CURSOR_P5:
		return 5;
	case PCIE_EQ_CURSOR_P6:
		return 6;
	case PCIE_EQ_CURSOR_P7:
		return 7;
	case PCIE_EQ_CURSOR_P8:
		return 8;
	case PCIE_EQ_CURSOR_P9:
		return 9;
	case PCIE_EQ_CURSOR_P10:
		return 10;
	default:
		return -1;
	}
}

static int nvt_plat_pcie_proc_host_stat_show(struct seq_file *m, void *v)
{
	struct dw_pcie *pci = (struct dw_pcie *)m->private;
	u32 val;

	seq_printf(m, "Host status :\n");

	val = dw_pcie_readl_dbi(pci, EQ_STATUS2_REG_OFS) & PCIE_EQ_STATUS_CURSOR_MSK;
	seq_printf(m, "\tLocal EQ preset: P%d\n", cursor2preset(val));

	val = dw_pcie_readl_dbi(pci, EQ_STATUS3_REG_OFS) & PCIE_EQ_STATUS_CURSOR_MSK;
	seq_printf(m, "\tRemote EQ preset: P%d\n", cursor2preset(val));

	return 0;
}

static int nvt_plat_pcie_proc_host_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_host_stat_show, PDE_DATA(inode));
}

static const struct proc_ops nvt_plat_pcie_proc_host_stat_fops = {
	.proc_open = nvt_plat_pcie_proc_host_stat_open,
	.proc_read = seq_read,
};

/* Version proc */

static int nvt_plat_pcie_proc_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Version: %s \n", NVT_PCIE_CONTROLLER_VERSION);

	return 0;
}

static int nvt_plat_pcie_proc_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_version_show, PDE_DATA(inode));
}

static const struct proc_ops nvt_plat_pcie_proc_version_fops = {
	.proc_open = nvt_plat_pcie_proc_version_open,
	.proc_read = seq_read,
};

static int nvt_plat_pcie_proc_eom_show(struct seq_file *sfile, void *v)
{

    struct dw_pcie *pci = (struct dw_pcie *)sfile->private;
    struct nvt_plat_pcie *chip = to_nvt_plat_pcie(pci);
	//struct nvt_pcie_chip *chip = (struct nvt_pcie_chip *)sfile->private;

	spin_lock(&chip->lock_eom);
	if (chip->state_eom == EOM_SM_SCAN_ONGOING) {
		unsigned long elps = jiffies - chip->eom_start_jiffies;
		printk("%s: eom scan is still on-going (elps %d s), please cat after scanning is done\r\n",
			__func__,
			jiffies_to_msecs(elps)/1000);
	} else if (chip->state_eom == EOM_SM_IDLE) {
		printk("%s: eom scan is still not triggered, plz write scan_rx_eye to trigger\r\n", __func__);
	} else {
		int i, j;
		unsigned long elps = chip->eom_end_jiffies - chip->eom_start_jiffies;

		seq_printf(sfile, "\r\nEOM scan result (elps %d s) :\r\n", jiffies_to_msecs(elps)/1000);
		for (i=63; i>=0; i--) {
			seq_printf(sfile, "  ");
			for (j=0; j<64; j++) {
				seq_printf(sfile, "%3d ", chip->eom_result[j][i]);
			}
			seq_printf(sfile, "\r\n");
		}
		for (i=0; i<64; i++) {
			seq_printf(sfile, "  ");
			for (j=0; j<64; j++) {
				seq_printf(sfile, "%3d ", chip->eom_result[j][i+64]);
			}
			seq_printf(sfile, "\r\n");
		}
	}
	spin_unlock(&chip->lock_eom);

	return 0;
}

static int nvt_plat_pcie_proc_eom_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_eom_show, PDE_DATA(inode));
}

static ssize_t nvt_plat_pcie_proc_eom_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	//struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;
    struct dw_pcie *pci = ((struct seq_file *)(file->private_data))->private;
    struct nvt_plat_pcie *nvt_pcie_chip_ptr = to_nvt_plat_pcie(pci);

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	spin_lock(&nvt_pcie_chip_ptr->lock_eom);
	if (nvt_pcie_chip_ptr->state_eom == EOM_SM_SCAN_ONGOING) {
		printk("%s: previous scan is still on-going\r\n", __func__);
	} else {
		printk("%s: start scanning...\r\n", __func__);
		nvt_pcie_chip_ptr->state_eom = EOM_SM_SCAN_ONGOING;
		nvt_pcie_chip_ptr->eom_start_jiffies = jiffies;
		schedule_delayed_work(&nvt_pcie_chip_ptr->eom_work, 1);
	}
	spin_unlock(&nvt_pcie_chip_ptr->lock_eom);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops nvt_plat_pcie_proc_eom_fops = {
        .proc_open    = nvt_plat_pcie_proc_eom_open,
        .proc_read    = seq_read,
        .proc_lseek  = seq_lseek,
        .proc_release = single_release,
        .proc_write   = nvt_plat_pcie_proc_eom_write
};

static int nvt_plat_pcie_proc_affinity_show(struct seq_file *sfile, void *v)
{
	struct dw_pcie *pci = (struct dw_pcie *)sfile->private;
	struct irq_data *p_data;
	struct irq_desc *p_desc;
	const struct cpumask *mask;

	p_data = irq_get_irq_data(pci->pp.irq);
	p_desc = irq_data_to_desc(p_data);

	mask = p_desc->irq_common_data.affinity;
	seq_printf(sfile, "%*pb\n", cpumask_pr_args(mask));

	return 0;
}

static int nvt_plat_pcie_proc_affinity_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_plat_pcie_proc_affinity_show, PDE_DATA(inode));
}

static ssize_t nvt_plat_pcie_proc_affinity_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	int err;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	cpumask_var_t new_value;
	struct dw_pcie *pci = ((struct seq_file *)(file->private_data))->private;

	if (!alloc_cpumask_var(&new_value, GFP_KERNEL))
		return -ENOMEM;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		printk(KERN_ERR "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0)
		cmd_line[0] = '\0';
	else
		cmd_line[len - 1] = '\0';

	p_arg = strsep(&cmdstr, delimiters);
	sscanf(p_arg, "%x", &val);

	err = cpumask_parse_user(buf, len, new_value);
	if (err) {
		printk("%s: cpumask parse fail\r\n", __func__);
		goto ERR_OUT;
	}

	__irq_set_affinity(pci->pp.irq, new_value, false);

	free_cpumask_var(new_value);

	return size;

ERR_OUT:
	free_cpumask_var(new_value);

	return -1;
}

static struct proc_ops nvt_plat_pcie_proc_affinity_fops = {
        .proc_open    = nvt_plat_pcie_proc_affinity_open,
        .proc_read    = seq_read,
        .proc_lseek  = seq_lseek,
        .proc_release = single_release,
        .proc_write   = nvt_plat_pcie_proc_affinity_write
};

static u32 eom_scan_y(void __iomem *ptr_phy, int idx_y)
{
	int i;
	u32 val;

	// change y phase
	val = readl(ptr_phy+0x1048);
	val &= ~GENMASK(13, 8);
	val |= FIELD_PREP(GENMASK(13, 8), idx_y);
	writel(val, ptr_phy+0x1048);

	val = readl(ptr_phy+0x1214);
	val &= ~GENMASK(18, 18);
	writel(val, ptr_phy+0x1214);
//	usleep_range(10, 20);
	val |= FIELD_PREP(GENMASK(18, 18), 1);
	writel(val, ptr_phy+0x1214);

	for (i=0; i<1000; i++) {
		val = readl(ptr_phy+0x115C);
		if (FIELD_GET(GENMASK(1, 1), val)) break;

		usleep_range(10, 20);
	}
	if (i >= 1000) printk("%s: loop timeout\r\n", __func__);

	val = readl(ptr_phy+0x1160);
	return val;
}

static void eom_scan_rx_work(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct nvt_plat_pcie *chip =
		container_of(delayed_work, struct nvt_plat_pcie, eom_work);
	void __iomem		*ptr_phy;
	u32   val;
	int idx_x;

	ptr_phy = ioremap(PCIE_PHY_BASE, 16384);

	// EOM manual
	val = readl(ptr_phy+0x1034);
	val |= FIELD_PREP(GENMASK(30, 29), 3);
	writel(val, ptr_phy+0x1034);

	// set EOM period
	val = readl(ptr_phy+0x1214);
	val |= FIELD_PREP(GENMASK(30, 19), 0xFFF);
	writel(val, ptr_phy+0x1214);

	for (idx_x=0; idx_x<64; idx_x++) {
		int idx_y;

		// change x phase
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(7, 2);
		val |= FIELD_PREP(GENMASK(7, 2), idx_x);
		writel(val, ptr_phy+0x1048);

		// scan upper eye
		val = readl(ptr_phy+0x1048);
		val |= FIELD_PREP(GENMASK(14, 14), 1);
		writel(val, ptr_phy+0x1048);
		val = readl(ptr_phy+0x1214);
		val |= FIELD_PREP(GENMASK(6, 6), 1);
		writel(val, ptr_phy+0x1214);
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(0, 0);
		writel(val, ptr_phy+0x1048);

		for (idx_y=63; idx_y>=0; idx_y--) {
			u64 a;

			a = eom_scan_y(ptr_phy, idx_y);
			a = (a*100 + 0x7FF80/2) / 0x7FF80;
			if (a > 100) a = 100;

			chip->eom_result[idx_x][idx_y] = a;
		}

		// scan lower eye
		val = readl(ptr_phy+0x1048);
		val &= ~GENMASK(14, 14);
		writel(val, ptr_phy+0x1048);
		val = readl(ptr_phy+0x1214);
		val &= ~GENMASK(6, 6);
		writel(val, ptr_phy+0x1214);
		val = readl(ptr_phy+0x1048);
		val |= FIELD_PREP(GENMASK(0, 0), 1);
		writel(val, ptr_phy+0x1048);

		for (idx_y=0; idx_y<64; idx_y++) {
			u64 a;

			a = eom_scan_y(ptr_phy, idx_y);
			a = (a*100 + 0x7FF80/2) / 0x7FF80;
			if (a > 100) a = 100;

			chip->eom_result[idx_x][64+idx_y] = a;
		}
	}

	iounmap(ptr_phy);

	chip->eom_end_jiffies = jiffies;

	spin_lock(&chip->lock_eom);
	chip->state_eom = EOM_SM_SCAN_DONE;
	spin_unlock(&chip->lock_eom);
}

/* PCIE proc init */

int nvt_plat_pcie_proc_init(struct device *dev, struct dw_pcie *pci)
{
	struct proc_dir_entry *entry = NULL;
	struct nvt_plat_pcie *nvt_pcie_chip_ptr = to_nvt_plat_pcie(pci);
	char str_slot[80];
	struct proc_dir_entry *p_slot_dir;

	memset(nvt_pcie_chip_ptr->eom_result, 0, sizeof(nvt_pcie_chip_ptr->eom_result));
	spin_lock_init(&nvt_pcie_chip_ptr->lock_eom);
	spin_lock_init(&nvt_pcie_chip_ptr->lock_ras);
	nvt_pcie_chip_ptr->state_eom = EOM_SM_IDLE;
	INIT_DELAYED_WORK(&nvt_pcie_chip_ptr->eom_work, eom_scan_rx_work);

	snprintf(str_slot, sizeof(str_slot), "rootport%d", nvt_pcie_chip_ptr->slot);
	glb_pci = pci;

	if (nvt_plat_pcie_proc_dir_root == NULL) {
		nvt_plat_pcie_proc_dir_root = proc_mkdir("nvt_pcie", NULL);
		if (!nvt_plat_pcie_proc_dir_root)
			return -ENOMEM;
	}

	p_slot_dir = proc_mkdir(str_slot, nvt_plat_pcie_proc_dir_root);
	if (!p_slot_dir) {
		return -ENOMEM;
	}

	entry = proc_create_data("version", 0664, p_slot_dir, &nvt_plat_pcie_proc_version_fops, (void *)pci);
	if (!entry)
		return -ENOMEM;

	/* RAS log proc node init */
	entry = proc_create_data("err_stat", 0664, p_slot_dir, &nvt_plat_pcie_proc_err_stat_fops, (void *)pci);
	if (!entry)
		return -ENOMEM;

	/* Host log proc node init */
	entry = proc_create_data("host_stat", 0664, p_slot_dir, &nvt_plat_pcie_proc_host_stat_fops, (void *)pci);
	if (!entry)
		return -ENOMEM;

	/* ep_info log proc node init */
	entry = proc_create_data("ep_info", 0664, p_slot_dir, &nvt_plat_pcie_proc_ep_info_fops, (void *)pci);
	if (!entry)
		return -ENOMEM;

	entry = proc_create_data("scan_rx_eye", 0664, p_slot_dir, &nvt_plat_pcie_proc_eom_fops, (void*)pci);
	if (!entry) {
		printk(KERN_ERR "failed to create proc scan_rx_eye!\n");
		return -ENOMEM;
	}

	entry = proc_create_data("affinity", 0664, p_slot_dir, &nvt_plat_pcie_proc_affinity_fops, (void*)pci);
	if (!entry) {
		printk(KERN_ERR "failed to create proc affinity!\n");
		return -ENOMEM;
	}

	/* Enable RAS log */
	pcie_ras_ec_start(pci, PCIE_EC_EN_ALLON);

	dev_info(dev, "Proc fs create successfully\n");

	return 0;
}
#endif /* CONFIG_PROC_FS */
