/**
    @file       pcie-nvt-ep-proc.c
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
#include <linux/uaccess.h>
#include <linux/bitfield.h>

#include "nvt_pcie_int.h"

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30

//============================================================================
// Declaration
//============================================================================

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

// DBI reg
#define AER_UNCORR_STATUS_OFS           0x104
#define AER_CORR_STATUS_OFS             0x110

#define EVENT_COUNTER_CONTROL_OFS       0x1a4
#define EVENT_COUNTER_DATA_OFS          0x1a8

#define MAX_CMD_LENGTH 30

#define EC_G0_NUM     7
#define EC_G1_NUM     11
#define EC_G2_NUM     8
#define EC_G3_NUM     6

typedef enum {
	PCIE_EC_EN_PEROFF = 0x1,
	PCIE_EC_EN_PERON = 0x3,
	PCIE_EC_EN_ALLOFF = 0x5,
	PCIE_EC_EN_ALLON = 0x7,
	PCIE_EC_EN_COUNT = 4
} PCIE_EC_EN_SEL;

static char g0_str[EC_G0_NUM][MAX_CMD_LENGTH] = {"EBUF Overflow", "EBUF Under-run", "Decode Error", "Running Disparity Error", "SKP OS Parity Error", "SYNC Header Error", "Rx Valid de-assertion"};
static char g1_str[EC_G1_NUM][MAX_CMD_LENGTH] = {"Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Detect EI Infer", "Receiver Error", "Rx Recovery Request", "N_FTS Timeout", "Framing Error", "Deskew Error"};
static char g2_str[EC_G2_NUM][MAX_CMD_LENGTH] = {"BAD TLP", "LCRC Error", "BAD DLLP", "Replay Number Rollover", "Replay Timeout", "Rx Nak DLLP", "Tx Nak DLLP", "Retry TLP"};
static char g3_str[EC_G3_NUM][MAX_CMD_LENGTH] = {"FC Timeout", "Poisoned TLP", "ECRC Error", "Unsupported Request", "Completer Abort", "Completion Timeout"};

static int proc_ep_eom_show(struct seq_file *sfile, void *v)
{
	struct nvt_pcie_chip *chip = (struct nvt_pcie_chip *)sfile->private;

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

static int proc_ep_eom_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_ep_eom_show, PDE_DATA(inode));
}

static ssize_t proc_ep_eom_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	int val;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *p_arg;
	struct nvt_pcie_chip *nvt_pcie_chip_ptr = ((struct seq_file *)(file->private_data))->private;

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

static struct proc_ops pcie_ep_proc_eom_fops = {
        .proc_open    = proc_ep_eom_open,
        .proc_read    = seq_read,
        .proc_lseek  = seq_lseek,
        .proc_release = single_release,
        .proc_write   = proc_ep_eom_write
};

static UINT32 eom_scan_y(void __iomem *ptr_phy, int idx_y)
{
	int i;
	UINT32 val;

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
	struct nvt_pcie_chip *chip =
		container_of(delayed_work, struct nvt_pcie_chip, eom_work);
	void __iomem		*ptr_phy;
	UINT32   val;
	int idx_x;

//	ptr_phy = chip->PCIE_REG_BASE[MEM_PCIE_PHY];
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
			UINT64 a;

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
			UINT64 a;

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

#if 1

/* ERR_STAT proc */
static u32 pcie_ep_aer_get_uncorr(void __iomem *ptr_dbi)
{
	u32 val;

	val = readl(ptr_dbi + AER_UNCORR_STATUS_OFS);
	writel(val, ptr_dbi + AER_UNCORR_STATUS_OFS);

	return val;
}

static u32 pcie_ep_aer_get_corr(void __iomem *ptr_dbi)
{
	u32 val;

	val = readl(ptr_dbi + AER_CORR_STATUS_OFS);
	writel(val, ptr_dbi + AER_CORR_STATUS_OFS);

	return val;
}


static void pcie_ep_ras_ec_set(void __iomem *ptr_dbi, u32 ec_sel)
{
	u32 val;

	val = readl(ptr_dbi + EVENT_COUNTER_CONTROL_OFS);
	val &= 0x0000FFFF;
	writel(val | (ec_sel << 16), ptr_dbi + EVENT_COUNTER_CONTROL_OFS);
}

//[sel]
//0x1 (PER_EVENT_OFF): per event off
//0x3 (PER_EVENT_ON): per event on
//0x5 (ALL_OFF): all off
//0x7 (ALL_ON): all on
static void pcie_ep_ras_ec_start(void __iomem *ptr_dbi, u32 sel)
{
	u32 val;

	if (sel > 7)
		sel = 1;

	if ((sel == 2) || (sel == 4) || (sel == 6))
		sel = 1;

	val = readl(ptr_dbi + EVENT_COUNTER_CONTROL_OFS);
	val &= 0xFFFFFFE3;
	writel(val | (sel << 2), ptr_dbi + EVENT_COUNTER_CONTROL_OFS);
}

static u32 pcie_ep_ras_ec_data(void __iomem *ptr_dbi)
{
	u32 val;

	val = readl(ptr_dbi + EVENT_COUNTER_DATA_OFS);

	return val;
}

static int pcie_ep_proc_err_stat_show(struct seq_file *m, void *v)
{
	struct nvt_pcie_chip *chip = (struct nvt_pcie_chip *)m->private;
	u32 Sel, data, i, j;
	u64 time_start;

	time_start = ktime_get_real_ns();

	seq_printf(m, "\r\n Test RAS D.E.S --- event count analysis function (Capture Time [sec.usec]: %lld.%06lld) \r\n\n", time_start / 1000000000, time_start / 1000);

	Sel = 0x0000;

	spin_lock(&chip->lock);

	i = 0;
	seq_printf(m, "=== Physical Layer 0 === (Grp %d)\n\n\t", i);
	for (j = 0; j < EC_G0_NUM; j++) {
		pcie_ep_ras_ec_set(chip->PCIE_REG_BASE[MEM_PCIE_DBI], Sel | j);
		data = pcie_ep_ras_ec_data(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
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
		pcie_ep_ras_ec_set(chip->PCIE_REG_BASE[MEM_PCIE_DBI], Sel | j);
		data = pcie_ep_ras_ec_data(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
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
		pcie_ep_ras_ec_set(chip->PCIE_REG_BASE[MEM_PCIE_DBI], Sel | j);
		data = pcie_ep_ras_ec_data(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
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
		pcie_ep_ras_ec_set(chip->PCIE_REG_BASE[MEM_PCIE_DBI], Sel | j);
		data = pcie_ep_ras_ec_data(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
		seq_printf(m, "%d : [%-25s]: 0x%08x", j, g3_str[j], data);
		if ((j + 1) % 3 == 0)
			seq_printf(m, "\n\t");
		else
			seq_printf(m, " ;\t ");
	}
	seq_printf(m, "\r\n");

	spin_unlock(&chip->lock);

	seq_printf(m, "\r\n ----------------------------------------------------\r\n");

	seq_printf(m, "AER uncorr status :\n\t");
	data = pcie_ep_aer_get_uncorr(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
	seq_printf(m, "0x%08x, ", data);
	seq_printf(m, "\r\n\n");

	seq_printf(m, "AER corr status :\n\t");
	data = pcie_ep_aer_get_corr(chip->PCIE_REG_BASE[MEM_PCIE_DBI]);
	seq_printf(m, "0x%08x, ", data);
	seq_printf(m, "\r\n");

	return 0;
}

static int pcie_ep_proc_err_stat_open(struct inode *inode, struct file *file)
{
	return single_open(file, pcie_ep_proc_err_stat_show, PDE_DATA(inode));
}

static const struct proc_ops pcie_ep_proc_err_stat_fops = {
	.proc_open = pcie_ep_proc_err_stat_open,
	.proc_read = seq_read,
};
#endif

/* PCIE ep proc init */

int nvt_pcie_ep_proc_init(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{
	int ret = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *pmodule_root = NULL;

	memset(nvt_pcie_chip_ptr->eom_result, 0, sizeof(nvt_pcie_chip_ptr->eom_result));
	spin_lock_init(&nvt_pcie_chip_ptr->lock);
	spin_lock_init(&nvt_pcie_chip_ptr->lock_eom);
	nvt_pcie_chip_ptr->state_eom = EOM_SM_IDLE;
	INIT_DELAYED_WORK(&nvt_pcie_chip_ptr->eom_work, eom_scan_rx_work);

	pmodule_root = proc_mkdir("nvt_pcie_ep", NULL);
	if (pmodule_root == NULL) {
		printk(KERN_ERR "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	nvt_pcie_chip_ptr->pproc_module_root = pmodule_root;

	entry = proc_create_data("scan_rx_eye", 0664, pmodule_root, &pcie_ep_proc_eom_fops, (void*)nvt_pcie_chip_ptr);
	if (!entry) {
		printk(KERN_ERR "failed to create proc scan_rx_eye!\n");
		return -ENOMEM;
	}

	/* RAS log proc node init */
	entry = proc_create_data("err_stat", 0664, pmodule_root, &pcie_ep_proc_err_stat_fops, (void *)nvt_pcie_chip_ptr);
	if (!entry)
		return -ENOMEM;

	/* Enable RAS log */
	pcie_ep_ras_ec_start(nvt_pcie_chip_ptr->PCIE_REG_BASE[MEM_PCIE_DBI], PCIE_EC_EN_ALLON);

	return ret;

remove_root:
	nvt_pcie_ep_proc_remove(nvt_pcie_chip_ptr);
	//proc_remove(nvt_pcie_chip_ptr->pproc_module_root);
	return ret;

}

int nvt_pcie_ep_proc_remove(struct nvt_pcie_chip *nvt_pcie_chip_ptr)
{

	proc_remove(nvt_pcie_chip_ptr->pproc_module_root);

	cancel_delayed_work_sync(&nvt_pcie_chip_ptr->eom_work);
	return 0;
}
