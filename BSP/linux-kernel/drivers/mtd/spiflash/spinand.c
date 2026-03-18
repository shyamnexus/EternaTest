/*
 * drivers/mtd/spiflash/spinand.c
 *
 * Copyright © 2005 Intel Corporation
 * Copyright © 2006 Marvell International Ltd.
 * Copyright © 2016 Novatek Microelectronics Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/interrupt.h>

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/platnand.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/version.h>
#include <plat/nvt-gpio.h>
#include <plat/nvt-sramctl.h>
#include <plat/pad.h>

#include "../nvt_flash_spi/nvt_flash_spi_reg.h"
#include "../nvt_flash_spi/nvt_flash_spi_int.h"

#ifdef CONFIG_OF
#include <linux/of_device.h>
#endif
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/div64.h>
#include <asm/io.h>
#include "../mtdcore.h"
#ifdef CONFIG_PM
#include <linux/suspend.h>
#include <linux/soc/nvt/nvt-info.h>
#endif

#define NAND_VERSION "1.00.024"

static int enable_erase_cnt = 0;
static u32 erase_count[4096] = {};
static u32 total_block_cnt = 0;

typedef struct nand_2plane_id {
	char    *name;
	int     mfr_id;
	int     dev_id;
} NAND_2PLANE_ID;

static NAND_2PLANE_ID nand_2plane_list[] = {
	// flash name / Manufacturer ID / Device ID
	{ "MXIC_MX35LF2G14AC",		0xC2,	0x20	},
	{ "MICRON_MT29F2G01AB",		0x2C,	0x24	}
};
#define NUM_OF_2PLANE_ID (sizeof(nand_2plane_list) / sizeof(NAND_2PLANE_ID))

#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6
#define READ_REG(addr)            ioread32((void *)addr)
#define WRITE_REG(value, addr)    iowrite32(value, (void *)addr)

typedef struct proc_nand {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_static_entry;
	struct proc_dir_entry *pproc_switch_entry;
	struct proc_dir_entry *pproc_mtd_ec_info_entry;
	struct proc_dir_entry *pproc_peb_mtd_ec_info_entry;
} proc_nand_static_t;
proc_nand_static_t proc_nand;

static int drv_nand_read_page(struct drv_nand_dev_info *info, int column, int page_addr);

static int nvt_nand_proc_mtd_peb_ec_info_show(struct seq_file *sfile, void *v)
{
	struct mtd_info *mtd;
	int i = 0;
	u64 block_cnt = 0, block_end = 0, block_begin = 0;

	mtd_for_each_device(mtd) {
		seq_printf(sfile, "mtd%d: %8.8llx %8.8llx \"%s\"\n",
			   mtd->index, block_begin, \
			   (unsigned long long)mtd->size + block_begin, mtd->name);

		block_begin += mtd->size;
		block_cnt = mtd->size;
		do_div(block_cnt, mtd->erasesize);

		block_end += block_cnt;

		for (;i < block_end; i++) {
			if (((i % 10) == 0) && (i != 0))
				seq_printf(sfile, "\n");

			seq_printf(sfile, "%-4d: %-6d", i, erase_count[i]);
		}
		seq_printf(sfile, "\n");
	}

	return 0;
}


static int nvt_nand_proc_mtd_peb_ec_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_mtd_peb_ec_info_show, NULL);
}


static struct proc_ops proc_mtd_peb_ec_info_fops = {
	.proc_open    = nvt_nand_proc_mtd_peb_ec_info_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};


static int nvt_nand_proc_mtd_ec_info_show(struct seq_file *sfile, void *v)
{
	struct mtd_info *mtd;
	int i = 0;
	u64 block_cnt = 0, block_end = 0, block_begin = 0;
	u32 total_cnt = 0;

	mtd_for_each_device(mtd) {
		seq_printf(sfile, "mtd%d: %8.8llx %8.8llx \"%s\"\n",
			   mtd->index, block_begin, \
			   (unsigned long long)mtd->size + block_begin, mtd->name);

		block_begin += mtd->size;
		block_cnt = mtd->size;
		do_div(block_cnt, mtd->erasesize);

		block_end += block_cnt;

		for (;i < block_end; i++) {
			total_cnt += erase_count[i];
		}
		seq_printf(sfile, "Total erase count %d\n", total_cnt);
		total_cnt = 0;
	}

	return 0;
}


static int nvt_nand_proc_mtd_ec_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_mtd_ec_info_show, NULL);
}


static struct proc_ops proc_mtd_ec_info_fops = {
	.proc_open    = nvt_nand_proc_mtd_ec_info_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};


static int nvt_nand_proc_static_show(struct seq_file *sfile, void *v)
{
	int i;

	for (i = 0; i < total_block_cnt; i++) {
		if ((i % 10) == 0)
			seq_printf(sfile, "\n");
		seq_printf(sfile, "%-4d: %-6d", i, erase_count[i]);
	}
	seq_printf(sfile, "\n");
	return 0;
}


static int nvt_nand_proc_static_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_static_show, NULL);
}


static struct proc_ops proc_static_fops = {
	.proc_open    = nvt_nand_proc_static_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};

static int nvt_nand_proc_switch_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho $command > enable_static\n\n");
	seq_printf(sfile, "enable           : enable erase static\n");
	seq_printf(sfile, "disable          : disable erase static\n");
	seq_printf(sfile, "clean            : clean static data\n");
	return 0;
}


static int nvt_nand_proc_switch_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_nand_proc_switch_show, NULL);
}

static ssize_t nvt_nand_proc_switch_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	/*check command length*/
	if ((!len) || (len > (MAX_CMD_LENGTH - 1))) {
		pr_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	/*copy command string from user space*/
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (!strcmp(argv[0], "enable")) {
		enable_erase_cnt = 1;
		return size;
	} else if (!strcmp(argv[0], "disable")) {
		enable_erase_cnt = 0;
		return size;
	} else if (!strcmp(argv[0], "clean")) {
		int i;
		for (i = 0; i < 4096; i++)
			erase_count[i] = 0x0;

		return size;
	}

ERR_OUT:
	return -1;
}

static int nvt_ecc_init(struct device *dev, struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(nand);
	u8 maf_id = info->flash_info->chip_id & 0xFF;

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
		nand->ecc.strength = 8;
		mtd->bitflip_threshold = 6;

		nand->ecc.size = 512;

		nand_host_set_spare_size_sel(info, info->flash_info->spare_size);
		nand_host_set_spare_align_sel(info, info->flash_info->spare_align);
		nand_host_set_spare_eccfree(info, info->flash_info->spare_protect);

		dev_info(&info->pdev->dev, "use BCH ecc, section size(512+%d)\n", \
			16*(1+info->flash_info->spare_size));

	} else {
		// on-die ecc
		if (maf_id == MFG_ID_WINBOND) {
			if (info->flash_info->chip_id == WINBOND_W25N02KV) {
				nand->ecc.strength = 8;
				mtd->bitflip_threshold = 6;
			} else {
				nand->ecc.strength = 1;
				mtd->bitflip_threshold = 1;
			}
		} else if ((maf_id == MFG_ID_MXIC) || (maf_id == MFG_ID_MICRON)) {
			nand->ecc.strength = 4;
			mtd->bitflip_threshold = 3;
		} else if (maf_id == MFG_ID_TOSHIBA) {
			if (info->flash_info->page_size == 4096) {
				nand->ecc.strength = 8;
				mtd->bitflip_threshold = 6;
			} else {
				nand->ecc.strength = 8;
				mtd->bitflip_threshold = 3;
			}
		} else if (maf_id == MFG_ID_XTX) {
			nand->ecc.strength = 8;
			mtd->bitflip_threshold = 6;
		} else if (maf_id == MFG_ID_GD) {
			nand->ecc.strength = 4;
			mtd->bitflip_threshold = 3;
		} else {
			nand->ecc.strength = 1;
			mtd->bitflip_threshold = 1;
		}

		nand->ecc.size = 512;

		dev_info(&info->pdev->dev, "use flash on-die ecc \n");
	}

	dev_info(dev, "ecc.step_size[%d] step[%d] strength[%d] threshold[%d]\n",
		 nand->ecc.size, nand->ecc.steps, nand->ecc.strength, mtd->bitflip_threshold);

#if 0
	pr_err("=== dump nand flash info === \n");
	pr_err("  Total size: 0x%llx         \n", mtd->size);
	pr_err("  Erase size: 0x%x           \n", mtd->erasesize);
	pr_err("  Write size: 0x%x           \n", mtd->writesize);
	pr_err("  Write buf size: 0x%x       \n", mtd->writebufsize);
	pr_err("  oob size: 0x%x             \n", mtd->oobsize);
	pr_err("  oobavail: 0x%x             \n", mtd->oobavail);

	pr_err("  threshold:     %d          \n", mtd->bitflip_threshold);
	pr_err("  ecc step size: %d          \n", mtd->ecc_step_size);
	pr_err("  ecc strength:  %d          \n", mtd->ecc_strength);
#endif

	return 0;
}

static struct proc_ops proc_switch_fops = {
	.proc_open    = nvt_nand_proc_switch_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_write   = nvt_nand_proc_switch_write
};

int nvt_nand_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_info/nvt_flash", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_module_root = pmodule_root;

	pentry = proc_create("static_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_static_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_static_entry = pentry;

	pentry = proc_create("enable_static", S_IRUGO | S_IXUGO, pmodule_root, &proc_switch_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_switch_entry = pentry;

	pentry = proc_create("mtd_ec_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_mtd_ec_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_mtd_ec_info_entry = pentry;

	pentry = proc_create("mtd_peb_ec_info", S_IRUGO | S_IXUGO, pmodule_root, &proc_mtd_peb_ec_info_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc static!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_nand.pproc_peb_mtd_ec_info_entry = pentry;

remove_proc:
	return ret;
}

/* error code and state */
enum {
	ERR_NONE	= 0,
	ERR_DMABUSERR	= -1,
	ERR_SENDCMD	= -2,
	ERR_DBERR	= -3,
	ERR_BBERR	= -4,
	ERR_ECC_FAIL	= -5,
	ERR_ECC_UNCLEAN = -6,
};

enum {
	STATE_READY = 0,
	STATE_CMD_HANDLE,
	STATE_DMA_READING,
	STATE_DMA_WRITING,
	STATE_DMA_DONE,
	STATE_PIO_READING,
	STATE_PIO_WRITING,
};


static struct nand_flash_dev spinand_flash_ids[] = {
/*
* Some incompatible NAND chips share device ID's and so must be
* listed by full ID. We list them first so that we can easily identify
* the most specific match.
*/
	// Micron
	{"MT29F1G01AAADD 1G 3.3V",
		{ .id = {MFG_ID_MICRON, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// Micron
	{"MT29F1G01ABAF 1G 3.3V",
		{ .id = {MFG_ID_MICRON, 0x14} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	{"MT29F2G01AB 2G 3.3V",
		{ .id = {MFG_ID_MICRON, 0x24} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// Winbond
	{"W25N01GV 1G 3.3V",
		{ .id = {MFG_ID_WINBOND, 0xAA, 0x21} },
		SZ_2K, SZ_128, SZ_128K, 0, 3, 64 },
	// Winbond new 1G
	{"W25N01KV 1G 3.3V",
		{ .id = {MFG_ID_WINBOND, 0xAE, 0x21} },
		SZ_2K, SZ_128, SZ_128K, 0, 3, 64 },
	// Winbond new 2G
	{"W25N02KV 2G 3.3V",
		{ .id = {MFG_ID_WINBOND, 0xAA, 0x22} },
		SZ_2K, SZ_256, SZ_128K, 0, 3, 64 },
	// Winbond new 4G
	{"W25N04KV 4G 3.3V",
		{ .id = {MFG_ID_WINBOND, 0xAA, 0x23} },
		SZ_2K, SZ_512, SZ_128K, 0, 3, 64 },
	// ESMT
	{"F250L512M 512M 3.3V",
		{ .id = {MFG_ID_ESMT, 0x20} },
		SZ_2K, SZ_64, SZ_128K, 0, 2, 64 },
	// ESMT
	{"F250L512M 1GiB 3.3V",
		{ .id = {MFG_ID_ESMT, 0x01} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice
	{"GD5F1GQ4UBYIG 1GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD1} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice
	{"GD5F1GQ4UEYIH 1GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD9} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice 2G
	{"GD5F2GQ4UB9IG 2GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD2} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// GigaDevice 2G
	{"GD5F2GQ5UEYIH 2GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x32} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// GigaDevice 2G
	{"GD5F2GQ5UEYIG 2GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x52} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// GigaDevice 4G
	{"GD5F4GQ6UEYIH 4GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x35} },
		SZ_2K, SZ_512, SZ_128K, 0, 2, 64 },
	// MXIC
	{"MX35LF1GE4AB 1GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// MXIC
	{"MX35LF2G14AC 2GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x20} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// ETRON
	{"EM73C044 1GiB 3.3V",
		{ .id = {MFG_ID_ETRON, 0x11} },
		SZ_2K, SZ_128, SZ_64K, 0, 2, 64 },
	// ATO
	{"ATO25D1GA 1GiB 3.3V",
		{ .id = {MFG_ID_ATO, 0x12} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// TOSHIBA
	{"TC58CVG0S3Hx 1GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xC2} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// TOSHIBA
	{"TC58CVG1S3Hx 2GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xEB, 0x40} },
		SZ_2K, SZ_256, SZ_128K, 0, 3, 64 },
	// TOSHIBA
	{"TC58CVG1S3HRAIG 2GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xCB} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 64 },
	// TOSHIBA
	{"TC58CVG2S0Hx 4GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xED} },
		SZ_4K, SZ_512, SZ_256K, 0, 2, 128 },
	// TOSHIBA
	{"TC58CVG2S0HRAIG 4GiB 3.3V",
		{ .id = {MFG_ID_TOSHIBA, 0xCD} },
		SZ_4K, SZ_512, SZ_256K, 0, 2, 128 },
	// Doscilicon
	{"DS35Q1GA 1GiB 3.3V",
		{ .id = {MFG_ID_DOSILICON, 0x71} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 64 },
	// XTX
	{"XT26G04A 4GiB 3.3V",
		{ .id = {MFG_ID_XTX, 0xE3} },
		SZ_2K, SZ_512, SZ_256K, 0, 2, 64 },
	// GigaDevice 4G
	{"GD5F4GQ4UAYIG 4GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xF4} },
		SZ_2K, SZ_512, SZ_128K, 0, 2, 64 },
	// MXIC
	{"MX35LF4GE4AD 4GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x37} },
		SZ_4K, SZ_512, SZ_256K, 0, 2, 128 },
	// GigaDevice 4G
	{"GD5F4GQ4U 4GiB 3.3V",
		{ .id = {MFG_ID_GD, 0xD4} },
		SZ_4K, SZ_512, SZ_256K, 0, 2, 256 },
	// MXIC
	{"MX35LF2GE4AD 2GiB 3.3V",
		{ .id = {MFG_ID_MXIC, 0x26} },
		SZ_2K, SZ_256, SZ_128K, 0, 2, 128 },
	// GigaDevice 1G
	{"GD5F1GM7UE 1GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x91} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice 1G
	{"GD5F1GM7RE 1GiB 1.8V",
		{ .id = {MFG_ID_GD, 0x81} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	// GigaDevice 4G, 2k page
	{"GD5F4GM8UE 4GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x95} },
		SZ_2K, SZ_512, SZ_128K, 0, 2, 128 },
	// GigaDevice 1G
	{"GD5F1GQ5UE 1GiB 3.3V",
		{ .id = {MFG_ID_GD, 0x51} },
		SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
};

#if 1 //def CONFIG_MTD_SPINAND_HWECC
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
#else
static int spinand_oob_64_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{
	if (section > 3)
		return -ERANGE;

	if (!section) {
		oobregion->offset = 8;
		oobregion->length = 8;
	} else if (section == 1) {
		oobregion->offset = 24;
		oobregion->length = 8;
	} else if (section == 2) {
		oobregion->offset = 40;
		oobregion->length = 8;
	} else {
		oobregion->offset = 56;
		oobregion->length = 8;
	}

	return 0;
}

static int spinand_oob_64_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	if (section > 2)
		return -ERANGE;

	if (!section) {
		oobregion->offset = 16;
		oobregion->length = 4;
	} else if (section == 1) {
		oobregion->offset = 32;
		oobregion->length = 4;
	} else {
		oobregion->offset = 48;
		oobregion->length = 4;
	}

	return 0;
}

const struct mtd_ooblayout_ops spinand_oob_64 = {
	.ecc = spinand_oob_64_ooblayout_ecc,
	.free = spinand_oob_64_ooblayout_free,
};

static int spinand_oob_etron_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{

	if (section > 0)
		return -ERANGE;

	oobregion->offset = 4;
	oobregion->length = 60;

	return 0;
}

static int spinand_oob_etron_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	if (section > 0)
		return -ERANGE;

	oobregion->offset = 2;
	oobregion->length = 2;

	return 0;
}

const struct mtd_ooblayout_ops spinand_oob_etron = {
	.ecc = spinand_oob_etron_ooblayout_ecc,
	.free = spinand_oob_etron_ooblayout_free,
};

static int spinand_oob_gd_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{
	if (section > 0)
		return -ERANGE;

#ifdef CONFIG_JFFS2_FS
	oobregion->offset = 64;
	oobregion->length = 64;
#else
	oobregion->offset = 0;
	oobregion->length = 0;
#endif
	return 0;
}

static int spinand_oob_gd_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
#ifdef CONFIG_JFFS2_FS
	if (section > 2)
		return -ERANGE;

	if (!section) {
		oobregion->offset = 16;
		oobregion->length = 4;
	} else if (section == 1) {
		oobregion->offset = 32;
		oobregion->length = 4;
	} else {
		oobregion->offset = 48;
		oobregion->length = 4;
	}
#else
	if (section > 0)
		return -ERANGE;

	oobregion->offset = 2;
	oobregion->length = 62;
#endif
	return 0;
}

const struct mtd_ooblayout_ops spinand_oob_gd = {
	.ecc = spinand_oob_gd_ooblayout_ecc,
	.free = spinand_oob_gd_ooblayout_free,
};

static int spinand_oob_xtx_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{

	if (section > 0)
		return -ERANGE;

	oobregion->offset = 8;
	oobregion->length = 56;

	return 0;
}

static int spinand_oob_xtx_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	if (section > 0)
		return -ERANGE;

	oobregion->offset = 2;
	oobregion->length = 6;

	return 0;
}

const struct mtd_ooblayout_ops spinand_oob_xtx = {
	.ecc = spinand_oob_xtx_ooblayout_ecc,
	.free = spinand_oob_xtx_ooblayout_free,
};

static int spinand_oob_nvt_8bit_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);

	if (section > 0)
		return -ERANGE;

	if (info->flash_info->spare_size) {
		oobregion->offset = 0;
		oobregion->length = 0;
	} else {
#ifdef CONFIG_JFFS2_FS
		oobregion->offset = 0;
		oobregion->length = 0;
#else
		if (info->flash_info->page_size == 4096) {
			oobregion->offset = 24;
			oobregion->length = 104;
		} else {
			oobregion->offset = 12;
			oobregion->length = 52;
		}
#endif
	}

	return 0;
}

static int spinand_oob_nvt_8bit_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);

	if (section > 0)
		return -ERANGE;

	if (info->flash_info->spare_size) {
		oobregion->offset = 2;
		oobregion->length = 62;

	} else {
		if (info->flash_info->page_size == 4096) {
			oobregion->offset = 2;
			oobregion->length = 22;
		} else {
			oobregion->offset = 2;
			oobregion->length = 10;
		}
	}

	return 0;
}

const struct mtd_ooblayout_ops spinand_oob_nvt_8bit = {
	.ecc = spinand_oob_nvt_8bit_ooblayout_ecc,
	.free = spinand_oob_nvt_8bit_ooblayout_free,
};
#endif
#endif

//static int use_dma = 1;

static int drv_nand_reset(struct drv_nand_dev_info *info, struct platform_device *pdev)
{
	int ret = 0;

#ifdef CONFIG_PM
	if (pm_suspend_target_state == 0) {
		info->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	}
#else
	info->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
#endif

	if (!IS_ERR(info->clk)) {
		clk_prepare(info->clk);
		clk_enable(info->clk);
	} else {
		pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));
		return -ENXIO;
	}

	NAND_SETREG(info, NAND_TIME0_REG_OFS, 0x06002222);
	NAND_SETREG(info, NAND_TIME1_REG_OFS, 0x7f0f);

#ifdef CONFIG_NVT_FPGA_EMULATION
	pr_err("in FPGA, fix clk to 12MHz\r\n");
	info->flash_freq = 12000000;
#endif

	clk_set_rate(info->clk, info->flash_freq);

	/* Release SRAM */
#if defined(CONFIG_NVT_IVOT_PLAT_NS02201) || \
	defined(CONFIG_NVT_IVOT_PLAT_NS02302) || \
	defined(CONFIG_NVT_IVOT_PLAT_NA51102) || \
	defined(CONFIG_NVT_IVOT_PLAT_NA51103) || \
	defined(CONFIG_NVT_IVOT_PLAT_NA51090) || \
	defined(CONFIG_NVT_IVOT_PLAT_NS02401)
	nvt_disable_sram_shutdown(SMC_SD);
#else
	nvt_disable_sram_shutdown(NAND_SD);
#endif

#if defined(CONFIG_NVT_IVOT_PLAT_NA51102) || defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	if (info->flash_freq < 24000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_4);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_4);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_4);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_4);	// D3
		pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_4);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO9,  PAD_DRIVINGSINK_LEVEL_4);	// CS0
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_4);	// CS1
	} else if (info->flash_freq < 48000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_6);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_6);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_6);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_6);	// D3
		pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_6);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO9,  PAD_DRIVINGSINK_LEVEL_6);	// CS0
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_6);	// CS1
	} else {	// freq > 48MHz
		pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_8);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_8);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_8);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_8);	// D3
		pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_8);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO9,  PAD_DRIVINGSINK_LEVEL_8);	// CS0
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_8);	// CS1
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51055) || defined(CONFIG_NVT_IVOT_PLAT_NA51089)
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_10MA);
		pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_10MA);
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_10MA);
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_10MA);
		pad_set_drivingsink(PAD_DS_CGPIO8, PAD_DRIVINGSINK_16MA);
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_10MA);
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301)
	if (info->flash_freq < 48000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_0);
		pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_0);	// CS1
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_0);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO12, PAD_DRIVINGSINK_LEVEL_0);	// CS0
	} else if (info->flash_freq <= 96000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_1);	// CS1
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_1);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO12, PAD_DRIVINGSINK_LEVEL_1);	// CS0
	} else {	// flash_freq > 96000000
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_1);
		pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_1);	// CS1
		pad_set_drivingsink(PAD_DS_CGPIO10, PAD_DRIVINGSINK_LEVEL_2);	// CLK
		pad_set_drivingsink(PAD_DS_CGPIO12, PAD_DRIVINGSINK_LEVEL_1);	// CS0
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02302)
	if (nvt_get_chip_id() == CHIP_NS02302) {
		if (info->flash_freq < 48000000) {
			pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_0);
			pad_set_drivingsink(PAD_DS_CGPIO8, PAD_DRIVINGSINK_LEVEL_0);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_0);	// CS1
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_0);	// CS0
		} else if (info->flash_freq <= 96000000) {
			pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO8, PAD_DRIVINGSINK_LEVEL_1);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_1);	// CS1
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_1);	// CS0
		} else {	// flash_freq > 96000000
			pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO6, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO7, PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO8, PAD_DRIVINGSINK_LEVEL_2);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO9, PAD_DRIVINGSINK_LEVEL_1);	// CS1
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_1);	// CS0
		}
	} else {
		// CHIP_NS02402
		if (info->flash_freq >= 120000000) {
			pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_4);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_2);	// CS0
		} else if (info->flash_freq >= 48000000) {
			pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_2);
			pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_3);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_2);	// CS0
		} else {
			pad_set_drivingsink(PAD_DS_CGPIO0,  PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO1,  PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO2,  PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO3,  PAD_DRIVINGSINK_LEVEL_1);
			pad_set_drivingsink(PAD_DS_CGPIO8,  PAD_DRIVINGSINK_LEVEL_2);	// CLK
			pad_set_drivingsink(PAD_DS_CGPIO11, PAD_DRIVINGSINK_LEVEL_1);	// CS0
		}

		if (info->flash_info->chip_sel == 1) {
			if (info->flash_freq >= 48000000) {
				pad_set_drivingsink(PAD_DS_CGPIO9,  PAD_DRIVINGSINK_LEVEL_2);	// CS1
			} else {
				pad_set_drivingsink(PAD_DS_CGPIO9,  PAD_DRIVINGSINK_LEVEL_1);	// CS1
			}
		}
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51090)
	pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_0);	// cs0
	if (info->flash_freq <= 24000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);	// clk
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);	// D3
	} else if (info->flash_freq <= 60000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_2);	// clk
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_2);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_2);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_2);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_2);	// D3
	} else {	// freq > 60MHz
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_3);	// clk
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_2);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_2);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_2);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_2);	// D3
	}
#elif defined(CONFIG_NVT_IVOT_PLAT_NA51103)
	pad_set_drivingsink(PAD_DS_CGPIO1, PAD_DRIVINGSINK_LEVEL_0);	// cs0
	if (info->flash_freq <= 48000000) {
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_0);	// clk
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_0);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_0);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_0);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_0);	// D3
	} else {	// freq > 48MHz
		pad_set_drivingsink(PAD_DS_CGPIO0, PAD_DRIVINGSINK_LEVEL_1);	// clk
		pad_set_drivingsink(PAD_DS_CGPIO2, PAD_DRIVINGSINK_LEVEL_1);	// D0
		pad_set_drivingsink(PAD_DS_CGPIO3, PAD_DRIVINGSINK_LEVEL_1);	// D1
		pad_set_drivingsink(PAD_DS_CGPIO4, PAD_DRIVINGSINK_LEVEL_1);	// D2
		pad_set_drivingsink(PAD_DS_CGPIO5, PAD_DRIVINGSINK_LEVEL_1);	// D3
	}
#else
	dev_info(&pdev->dev, "wrn: Pls check pad driving\n");
#endif
	info->flash_info->config_nand_type = NANDCTRL_SPI_NAND_TYPE;
	nand_host_set_nand_type(info, NANDCTRL_SPI_NAND_TYPE);

	nand_host_settiming2(info, NVT_SPI_TIMING);

	ret = nand_cmd_reset(info);

	return ret;
}

//static int spinand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
//				uint8_t *buf, int oob_required, int page)
static int spinand_read_page_hwecc(struct nand_chip *chip, uint8_t *buf,
				int oob_required, int page)
{
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	struct mtd_info *mtd = nand_to_mtd(chip);
	u8 status = 0, chip_id = info->flash_info->chip_id & 0xFF;
	u32 full_id = info->flash_info->chip_id;
	int ret = 0, count = 0, i = 0;
	const u32 column = 0;

	//nand_read_page_op(chip, page, 0, buf, mtd->writesize);

	if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
		dev_err(&info->pdev->dev,
			"NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

	info->buf_start = column;
	info->buf_count = mtd->writesize + mtd->oobsize;
	mutex_lock(info->lock);
	drv_nand_read_page(info, column, page);
	mutex_unlock(info->lock);

	memcpy((uint32_t *)buf, (uint32_t *)info->data_buff, mtd->writesize);

	if (oob_required) {
		memcpy(chip->oob_poi, info->data_buff + mtd->writesize, mtd->oobsize);
	}

	mutex_lock(info->lock);

	if (info->retcode == ERR_ECC_UNCLEAN) {
		mtd->ecc_stats.failed++;
	} else if (info->retcode == ECC_CORRECTED) {
		if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC || info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
				mtd->ecc_stats.corrected += \
					nand_cmd_read_ecc_corrected(info);

				NAND_DEBUG_MSG("ecc_stats.corrected: %d\n", mtd->ecc_stats.corrected);
				ret = status;
		} else {
			if (chip_id == MFG_ID_MXIC) {
				ret = nand_cmd_read_flash_ecc_corrected(info);
				mtd->ecc_stats.corrected += ret;
			} else if ((chip_id == MFG_ID_TOSHIBA) || \
					(full_id == WINBOND_W25N02KV)) {
				status = nand_cmd_read_status(info, \
					NAND_SPI_STS_FEATURE_4);

				for (i = 0; i < 8; i++)
					count += ((status >> i) & 0x1) ? 1 : 0;

				mtd->ecc_stats.corrected += count;
				ret = count;
			} else if (chip_id == MFG_ID_XTX) {
				status = nand_cmd_read_status(info, \
					NAND_SPI_STS_FEATURE_3);

				count = (status >> 2) & 0xF;
				mtd->ecc_stats.corrected += count;
				ret = count;
			} else {
				mtd->ecc_stats.corrected++;
				ret = 1;
			}
		}
	}

	mutex_unlock(info->lock);

	return ret;

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
static int spinand_write_page_hwecc(struct nand_chip *chip, const uint8_t *buf,
				int oob_required, int page)
#else
static int spinand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required)
#endif
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);

	nand_prog_page_begin_op(chip, page, 0, buf, mtd->writesize);
	if (oob_required) {
		//struct nand_chip *chip = mtd_to_nand(mtd);

		memcpy(info->data_buff + info->buf_start, buf, mtd->oobsize);

		info->buf_start += mtd->oobsize;
		//chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	}

	//pr_err("spinand_write_page_hwecc \r\n");
	return nand_prog_page_end_op(chip);
}

static int drv_nand_read_id(struct drv_nand_dev_info *info, uint32_t *id)
{
	uint8_t  card_id[4];
	uint8_t  id_count;
	uint32_t i;
	int ret = E_OK;

	mutex_lock(info->lock);
	if (nand_cmd_read_id(card_id, info) != E_OK) {
		pr_err("NAND cmd timeout\r\n");
		ret = -EIO;
	} else {
		pr_err("id =  0x%02x 0x%02x 0x%02x 0x%02x\n",
			card_id[0], card_id[1], card_id[2], card_id[3]);

		for (i = 0; i < sizeof(spinand_flash_ids); i++) {
			if ((card_id[0] == spinand_flash_ids[i].mfr_id) && \
			(card_id[1] == spinand_flash_ids[i].dev_id)) {
				id_count = spinand_flash_ids[i].id_len;
				memset(card_id + id_count, 0, 4 - id_count);
				info->flash_info->page_size = spinand_flash_ids[i].pagesize;
				info->flash_info->device_size = spinand_flash_ids[i].chipsize;
				info->flash_info->block_size = spinand_flash_ids[i].erasesize;
				info->flash_info->page_per_block = \
					(spinand_flash_ids[i].erasesize / spinand_flash_ids[i].pagesize);

				// check flash oob size
				if (info->flash_info->spare_size == 1) {
					if ((spinand_flash_ids[i].pagesize == 0x1000 && spinand_flash_ids[i].oobsize != 0x100) || \
						(spinand_flash_ids[i].pagesize == 0x0800 && spinand_flash_ids[i].oobsize != 0x080))
						pr_err("warning oob size: 0x%x, Pls check flash can support section(512+32) size \r\n", spinand_flash_ids[i].oobsize);
				}

				// fixed oob size
				if (spinand_flash_ids[i].pagesize == 0x1000)
					spinand_flash_ids[i].oobsize = 0x80;
				else
					spinand_flash_ids[i].oobsize = 0x40;

				break;
			}
		}
		*id = card_id[0] | (card_id[1] << 8) | (card_id[2] << 16) | \
			(card_id[3] << 24);

		ret = E_OK;
	}

	mutex_unlock(info->lock);

	return ret;
}

static int spinand_read_oob(struct nand_chip *chip, int page)
{
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	struct mtd_info *mtd = nand_to_mtd(chip);
	uint8_t *ptr;

	info->buf_count = mtd->writesize + mtd->oobsize;
	info->buf_start = mtd->writesize;

	//pr_err(" %s: page: 0x%x, data_buff: %p, buf_start: 0x%x\r\n", __func__, page, info->data_buff, info->buf_start);

	ptr = info->data_buff + info->buf_start;
	//pr_err(" ptr: %p\r\n", ptr);

	if (info->buf_start != info->flash_info->page_size) {
		dev_err(&info->pdev->dev,
		"info->buf_start = %d, != 0\n", info->buf_start);
	}
	mutex_lock(info->lock);
	nand_cmd_read_page_spare_data(info, ptr,
			info->flash_info->page_size * page);
	mutex_unlock(info->lock);

	memcpy(chip->oob_poi, ptr, mtd->oobsize);

	return 0;
}

static int spinand_write_oob(struct nand_chip *chip, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	u32 column = mtd->writesize;

	info->buf_count = mtd->writesize + mtd->oobsize;

	//pr_err("chip->oob_poi: 0x%p , buf_start: 0x%x, column: 0x%x\r\n", chip->oob_poi, info->buf_start, column);
	//WARN(1, "write oob\n");

	memcpy(info->data_buff + column, chip->oob_poi, mtd->oobsize);

	info->buf_start += mtd->oobsize;

	nand_cmd_write_spare_sram(info);

	return nand_cmd_write_operation_single(info, \
			page * info->flash_info->page_size, column);
}

static int spinand_wait(struct nand_chip *chip)
{
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	int ret = info->retcode;

	info->retcode = ERR_NONE;

	if (ret < 0)
		return NAND_STATUS_FAIL;
	else
		return E_OK;
}

static int drv_nand_read_page(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	return nand_cmd_read_operation(info, info->data_buff,
			page_addr * info->flash_info->page_size, 1);
}

static int drv_nand_write_page(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
		struct nand_chip *chip = &info->nand_chip;
		struct mtd_info *mtd = nand_to_mtd(chip);
		uint32_t oob_start = info->flash_info->page_size;

		if (column != info->flash_info->page_size) {
			// BCH ecc: update spare data to dram
			memcpy(info->data_buff + oob_start, chip->oob_poi, mtd->oobsize);
			nand_cmd_write_spare_sram(info);
		}

	} else {	// on-die ecc
		if (column != info->flash_info->page_size)
			nand_cmd_write_spare_sram(info);
	}

	return nand_cmd_write_operation_single(info, \
			page_addr * info->flash_info->page_size, column);
}

static irqreturn_t spinand_irq(int irq, void *devid)
{
	struct drv_nand_dev_info *info = devid;

	info->nand_int_status = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

	if (info->nand_int_status) {
		NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, info->nand_int_status);
		complete(&info->cmd_complete);
		return IRQ_HANDLED;
	} else
		return IRQ_NONE;


}

static inline int is_buf_blank(uint8_t *buf, size_t len)
{
	for (; len > 0; len--)
		if (*buf++ != 0xff)
			return 0;
	return 1;
}

static int nvt_attach_chip(struct nand_chip *chip)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct device *dev = mtd->dev.parent;

	if (nvt_ecc_init(dev, mtd))
		pr_err(" nvt ecc init fail\n");

	return 0;
}

static int nvt_setup_interface(struct nand_chip *chip, int csline,
				   const struct nand_interface_config *conf)
{
	//pr_err(" %s: \n", __func__);
	return 0;
}

#define NAND_EXEC_OP_DBG 0
#define  NAND_EXEC_OP_DBG_MSG(fmt, args...) \
	if(NAND_EXEC_OP_DBG) pr_err(fmt, ##args)
static int nvt_exec_instr(struct nand_chip *chip,
			      const struct nand_op_instr *instr)
{
	unsigned int i;
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	static u32 nand_cmd_op, nand_cmd_addr, nand_cmd_h_addr, nand_cmd_cnt;
	struct mtd_info *mtd = nand_to_mtd(chip);
	int ret;

	//pr_err("dbg exec_op: \r\n");
	//pr_err("  type: %d       \r\n", instr->type);

	switch (instr->type) {
	case NAND_OP_CMD_INSTR:
		NAND_EXEC_OP_DBG_MSG("  cmd:  0x%x     \r\n", instr->ctx.cmd.opcode);

		switch (instr->ctx.cmd.opcode) {
		case NAND_CMD_RESET:
			break;

		case NAND_CMD_READID:
			nand_cmd_op = NAND_CMD_READID;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;
			break;

		case NAND_CMD_STATUS:
			nand_cmd_op = NAND_CMD_STATUS;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;
			break;

		case NAND_CMD_ERASE1:
			nand_cmd_op = NAND_CMD_ERASE1;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;
			break;

		case NAND_CMD_ERASE2:
			break;

		case NAND_CMD_SEQIN:
			nand_cmd_op = NAND_CMD_SEQIN;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;
			break;

		case NAND_CMD_PAGEPROG:
			nand_cmd_op = 0;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;
			if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
				dev_err(&info->pdev->dev, "not CACHE_LINE_SIZE alignment!\n");

			mutex_lock(info->lock);
			ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
			mutex_unlock(info->lock);

			if (ret)
				info->retcode = ERR_SENDCMD;

			break;

		case NAND_CMD_READ0:
		//case NAND_CMD_READ1:
			nand_cmd_op = NAND_CMD_READ0;
			nand_cmd_addr = 0;
			nand_cmd_h_addr = 0;
			nand_cmd_cnt = 0;

			break;

		case NAND_CMD_READSTART:
			nand_cmd_op = NAND_CMD_READSTART;
			break;

		default:
			pr_debug(KERN_ERR"non-supported command[0x%x].\n", instr->ctx.cmd.opcode);
			return -EINVAL;

		}
		return 0;

	case NAND_OP_ADDR_INSTR:
		for (i = 0; i < instr->ctx.addr.naddrs; i++) {
			u8 addr = instr->ctx.addr.addrs[i];
			NAND_EXEC_OP_DBG_MSG("    addr 0x%x\n", addr);
			//u8 *buf = &addr;
			if (nand_cmd_op == NAND_CMD_ERASE1) {
				nand_cmd_addr |= addr<<(0x8*nand_cmd_cnt);
				nand_cmd_cnt++;
				if (nand_cmd_cnt == instr->ctx.addr.naddrs) {
					int ret;
					mutex_lock(info->lock);
					ret = nand_cmd_erase_block(info, nand_cmd_addr);
					mutex_unlock(info->lock);

					nand_cmd_op = 0;
					nand_cmd_addr = 0;
					nand_cmd_cnt = 0;
					nand_cmd_h_addr = 0;

					if (ret)
						info->retcode = ERR_BBERR;

					if (enable_erase_cnt) {
						u32 block_number = nand_cmd_addr/info->flash_info->page_per_block;
						erase_count[block_number] += 1;
					}
				}
			} else if (nand_cmd_op == NAND_CMD_SEQIN || nand_cmd_op == NAND_CMD_READ0) {
				if (i < 4)
					nand_cmd_addr |= addr<<(0x8*nand_cmd_cnt);
				else
					nand_cmd_h_addr |= addr<<(0x8 * (4 - nand_cmd_cnt));
				nand_cmd_cnt++;
			} else if (nand_cmd_op == NAND_CMD_READID) {
			}
		}
		return 0;

	case NAND_OP_DATA_IN_INSTR:
		NAND_EXEC_OP_DBG_MSG("      data_in[%p] len[%d] force[%d] \r\n", instr->ctx.data.buf.in, \
			instr->ctx.data.len, instr->ctx.data.force_8bit);

		if (nand_cmd_op == NAND_CMD_READSTART) {
			int ret, column, page;

			column = nand_cmd_addr & 0xFFFF;
			page = (nand_cmd_addr>>16) & 0xFFFF;
			page |= (nand_cmd_h_addr<<16);
			NAND_EXEC_OP_DBG_MSG("      addr: 0x%x\r\n", nand_cmd_addr);

			if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
				dev_err(&info->pdev->dev,
					"NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

			info->buf_start = column;
			info->buf_count = mtd->writesize + mtd->oobsize;
			mutex_lock(info->lock);
			ret = drv_nand_read_page(info, column, page);
			mutex_unlock(info->lock);

			memcpy((uint32_t *)instr->ctx.data.buf.in, (uint32_t *)info->data_buff, instr->ctx.data.len);

			if (ret == E_CTX)
				info->retcode = ERR_ECC_UNCLEAN;
			else if (ret < 0)
				info->retcode = ERR_SENDCMD;
			else if (ret == ECC_CORRECTED)
				info->retcode = ECC_CORRECTED;
			else
				info->retcode = ERR_NONE;
		} else if (nand_cmd_op == NAND_CMD_READID) {
			int ret;
			info->buf_start = 0;
			info->buf_count = 4;
			if(info->flash_info->chip_id) {
				memcpy((uint32_t *)instr->ctx.data.buf.in, &info->flash_info->chip_id, 4);
				//memcpy((uint32_t *)info->data_buff, &info->flash_info->chip_id, 16);
			} else {
				ret = drv_nand_read_id(info, (uint32_t *)info->data_buff);
				if (ret)
					info->retcode = ERR_SENDCMD;
			}
		} else if (nand_cmd_op == NAND_CMD_STATUS) {
			info->buf_start = 0;
			info->buf_count = 1;
			mutex_lock(info->lock);
			nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_3);
			mutex_unlock(info->lock);

			NAND_EXEC_OP_DBG_MSG("get sts: 0x%x \r\n", info->data_buff[0]);
			*(u8*)instr->ctx.data.buf.in = info->data_buff[0];
		}

		return 0;

	case NAND_OP_DATA_OUT_INSTR:
		NAND_EXEC_OP_DBG_MSG("      data_out[%p] len[%d] force[%d] \r\n", instr->ctx.data.buf.out, \
			instr->ctx.data.len, instr->ctx.data.force_8bit);

		if (nand_cmd_op == NAND_CMD_SEQIN) {
			NAND_EXEC_OP_DBG_MSG("      adr: 0x%x \r\n", nand_cmd_addr);
			info->buf_start = nand_cmd_addr&0xFFFF;	// column
			//info->buf_start = 0;
			info->buf_count = mtd->writesize + mtd->oobsize;

			memset(info->data_buff, 0xff, info->buf_count);

			memcpy(info->data_buff, instr->ctx.data.buf.out, instr->ctx.data.len);

			/* save column/page_addr for next CMD_PAGEPROG */
			info->seqin_column = nand_cmd_addr&0xFFFF;
			info->seqin_page_addr = (nand_cmd_addr>>16) & 0xFFFF;
			info->seqin_page_addr |= (nand_cmd_h_addr<<16);
		}

		return 0;

	case NAND_OP_WAITRDY_INSTR:
		return spinand_wait(chip);
	default:
		break;
	}



	return -EINVAL;
}

static int nvt_exec_op(struct nand_chip *chip, const struct nand_operation *op,
			   bool check_only)
{
	unsigned int i;
	int ret = 0;

	if (check_only)
		return 0;

	// TODO: cs target select

	for (i = 0; i < op->ninstrs; i++) {
		ret = nvt_exec_instr(chip, &op->instrs[i]);
		if (ret)
			break;
	}

	return ret;
}

static const struct nand_controller_ops nvt_controller_ops = {
	.attach_chip = nvt_attach_chip,
	.setup_interface = nvt_setup_interface,
	.exec_op = nvt_exec_op,
};

#if 0
static void spinand_cmdfunc(struct mtd_info *mtd, unsigned command,
		int column, int page_addr)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	int ret;
	uint8_t *ptr;

	info->use_dma = (use_dma) ? 1 : 0;
	info->data_size = 0;
	info->state = STATE_READY;
	info->retcode = ERR_NONE;

	switch (command) {
	case NAND_CMD_READOOB:
		info->buf_count = mtd->writesize + mtd->oobsize;
		info->buf_start = mtd->writesize + column;

		ptr = info->data_buff + info->buf_start;
		if (info->buf_start != info->flash_info->page_size) {
			dev_err(&info->pdev->dev,
			"info->buf_start = %d, != 0\n", info->buf_start);
		}
		mutex_lock(info->lock);
		nand_cmd_read_page_spare_data(info, ptr,
				info->flash_info->page_size * page_addr);
		mutex_unlock(info->lock);
		/* We only are OOB, so if the data has error, does not matter */
	break;

	case NAND_CMD_READ0:
	case NAND_CMD_READ1:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			dev_err(&info->pdev->dev,
				"NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		mutex_lock(info->lock);
		ret = drv_nand_read_page(info, column, page_addr);
		mutex_unlock(info->lock);
		if (ret == E_CTX)
			info->retcode = ERR_ECC_UNCLEAN;
		else if (ret < 0)
			info->retcode = ERR_SENDCMD;
		else if (ret == ECC_CORRECTED)
			info->retcode = ECC_CORRECTED;
		else
			info->retcode = ERR_NONE;
	break;

	case NAND_CMD_SEQIN:
		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		memset(info->data_buff, 0xff, info->buf_count);

		/* save column/page_addr for next CMD_PAGEPROG */
		info->seqin_column = column;
		info->seqin_page_addr = page_addr;
	break;

	case NAND_CMD_PAGEPROG:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			dev_err(&info->pdev->dev,
				"not CACHE_LINE_SIZE alignment!\n");
		mutex_lock(info->lock);
		ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
		mutex_unlock(info->lock);

		if (ret)
			info->retcode = ERR_SENDCMD;
	break;

	case NAND_CMD_ERASE1:
		mutex_lock(info->lock);
		ret = nand_cmd_erase_block(info, page_addr);
		mutex_unlock(info->lock);

		if (ret)
			info->retcode = ERR_BBERR;

		if (enable_erase_cnt) {
			u32 block_number = page_addr/info->flash_info->page_per_block;
			erase_count[block_number] += 1;
		}
	break;

	case NAND_CMD_ERASE2:
	break;

	case NAND_CMD_READID:
		info->buf_start = 0;
		info->buf_count = 16;
		if(info->flash_info->chip_id)
			memcpy((uint32_t *)info->data_buff, &info->flash_info->chip_id, 16);
		else {
			ret = drv_nand_read_id(info, (uint32_t *)info->data_buff);
			if (ret)
				info->retcode = ERR_SENDCMD;
		}
	break;
	case NAND_CMD_STATUS:
		info->buf_start = 0;
		info->buf_count = 1;
		mutex_lock(info->lock);
		nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);
		mutex_unlock(info->lock);
		if (!(info->data_buff[0] & 0x80))
			info->data_buff[0] = 0x80;
	break;

	case NAND_CMD_RESET:
	break;

	default:
		pr_debug(KERN_ERR "non-supported command.\n");
	break;
	}
}

static void spinand_select_chip(struct mtd_info *mtd, int dev)
{

}

static uint8_t spinand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	char retval = 0xFF;

	if (info->buf_start < info->buf_count)
		/* Has just send a new command? */
		retval = info->data_buff[info->buf_start++];

	return retval;
}
#endif

#if 0
//static void spinand_read_buf(struct nand_chip *chip, uint8_t *buf, int len)
static void spinand_read_buf(struct drv_nand_dev_info *info, void *buf, int len)
{
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);

	memcpy(buf, info->data_buff + info->buf_start, len);

	info->buf_start += len;
}
#endif

#if 0
//static void spinand_write_buf(struct nand_chip *chip, const uint8_t *buf, int len)
static void spinand_write_buf(struct drv_nand_dev_info *info, const void *buf, int len)
{
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);

	memcpy(info->data_buff + info->buf_start, buf, len);

	info->buf_start += len;
}
#endif

#if 1 //def CONFIG_MTD_SPINAND_HWECC
static void drv_nand_ecc_hwctl(struct nand_chip *chip, int mode)
{
	//struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return;
}

static int drv_nand_ecc_calculate(struct nand_chip *chip,
	const uint8_t *dat, uint8_t *ecc_code)
{
	//struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return 0;
}

static int drv_nand_ecc_correct(struct nand_chip *chip,
	uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
	//struct nand_chip *chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_get_controller_data(chip);
	dev_info(&info->pdev->dev, "%s retcode\n", __func__);
	return 0;
}
#endif

static int nvt_nand_scan_2plane_id(struct drv_nand_dev_info *info)
{
	int32_t i, read_id, flash_id;

	read_id = (info->flash_info->chip_id & 0x0000FFFF);

	for (i = 0 ; i < NUM_OF_2PLANE_ID; i++) {
		flash_id = (nand_2plane_list[i].mfr_id + (nand_2plane_list[i].dev_id << 8));
		if (read_id == flash_id) {
			dev_info(&info->pdev->dev, "%s is 2-plane flash\n", nand_2plane_list[i].name);
			return 1;
		}
    }
	return 0;
}

static int drv_nand_identify(struct drv_nand_dev_info *info)
{
	uint32_t id;
	uint32_t reg;
	u8 status;

	if(drv_nand_read_id(info, &id) != E_OK)
		return -EIO;

	info->flash_info->chip_id = id;
	//info->flash_info->chip_sel = 0;

	if (info->flash_info->page_size == 4096) {
		info->flash_info->oob_size = 128;
		info->flash_info->phy_page_ratio= 8;
		info->flash_info->module_config = NAND_PAGE4K | NAND_2COL_ADDR;
		nand_host_setup_page_size(info, NAND_PAGE_SIZE_4096);
	} else {
		info->flash_info->oob_size = 64;
		info->flash_info->phy_page_ratio= 4;
		info->flash_info->module_config = NAND_PAGE2K | NAND_2COL_ADDR;
		nand_host_setup_page_size(info, NAND_PAGE_SIZE_2048);
	}

	total_block_cnt = info->flash_info->num_blocks;

	if (nvt_nand_scan_2plane_id(info) == 1) {
		info->flash_info->plane_flags = 1;
	} else {
		info->flash_info->plane_flags = 0;
	}

	mutex_lock(info->lock);
	status = nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);

	if (status & SPINAND_CONFIG_ECCEN) {
		info->use_ecc = NANDCTRL_SPIFLASH_USE_ONDIE_ECC;
		info->flash_info->module_config |= NAND_PRI_ECC_SPI_ON_DIE;
		NAND_SETREG(info, NAND_INTMASK_REG_OFS, NAND_COMP_INTREN | \
			NAND_STS_FAIL_INTREN | NAND_TOUT_INTREN);
	} else {
#if 1	// BCH 8-bits ecc
		info->use_ecc = NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC;
		info->flash_info->module_config |= NAND_PRI_ECC_BCH;
		NAND_SETREG(info, NAND_INTMASK_REG_OFS, NAND_COMP_INTREN | \
			NAND_PRI_ECC_INTREN | NAND_STS_FAIL_INTREN | NAND_TOUT_INTREN);

#else	// RS ecc
		info->use_ecc = NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC;
		info->flash_info->module_config |= NAND_PRI_ECC_RS;
		NAND_SETREG(info, NAND_INTMASK_REG_OFS, NAND_COMP_INTREN | \
			NAND_PRI_ECC_INTREN | NAND_STS_FAIL_INTREN | NAND_TOUT_INTREN);
#endif
	}

	reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	reg &= ~(0x7FFFF);
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg | info->flash_info->module_config);
	mutex_unlock(info->lock);

	flash_copy_info(info);

	return 0;
}


/* the maximum possible buffer size for large page with OOB data
 * is: 2048 + 64 = 2112 bytes, allocate a page here for both the
 * data buffer and the DMA descriptor
 */
#define MAX_BUFF_SIZE	(4096 + 256) // (4096 + 128)

static int drv_nand_init_buff(struct drv_nand_dev_info *info)
{
	struct device *dev = &info->pdev->dev;

#if 1 //def CONFIG_NVT_IVOT_PLAT_NA51103
	#if defined(CONFIG_ZONE_DMA32)
	info->data_buff = kzalloc(MAX_BUFF_SIZE, GFP_DMA32 | GFP_KERNEL);
	#else
	info->data_buff = kzalloc(MAX_BUFF_SIZE, GFP_KERNEL);
	#endif

	info->data_desc_addr = virt_to_phys(info->data_buff);

	dev_info(dev, "dma buffer va: %p, pa: %pad \n", info->data_buff, &info->data_desc_addr);

	if ((uintptr_t)info->data_desc_addr % CACHE_LINE_SIZE) {
		dev_err(dev, "CACHE_LINE_SIZE is not alignment\n");
	}

#else
	info->data_buff = dma_alloc_coherent(dev, MAX_BUFF_SIZE, &info->data_desc_addr, GFP_DMA | GFP_KERNEL);

	if (info->data_buff == NULL) {
		dev_info(dev, "failed to allocate dma buffer\n");
		//kfree(info->data_buff);
		dma_free_coherent(dev, MAX_BUFF_SIZE, info->data_buff, info->data_desc_addr);
		return -ENOMEM;
	}

	dev_info(dev, "dma buffer va: %p, pa: 0x%llx\n", info->data_buff, info->data_desc_addr);
#endif

	info->data_buff_size = SZ_4K;

	//if ((uintptr_t)info->data_buff % CACHE_LINE_SIZE) {
	//	info->data_buff = (uint8_t *)((((uintptr_t)info->data_buff + CACHE_LINE_SIZE - 1)) & 0xFFFFFFC0);
	//	dev_info(dev, "CACHE_LINE_SIZE is not alignment, new va: %p\n", info->data_buff);
	//}

	return E_OK;
}

static void spinand_init_mtd(struct drv_nand_dev_info *info)
{
	struct device *dev = &info->pdev->dev;
	struct device_node *np = dev->of_node;
	struct nand_chip *chip = &info->nand_chip;
	struct mtd_info *mtd = nand_to_mtd(chip);
	const struct nvt_nand_flash *f = info->flash_info;
	u8 chip_id = f->chip_id & 0xFF;

	chip->ecc.engine_type = NAND_ECC_ENGINE_TYPE_ON_HOST;
	chip->ecc.size	= 0x200;
	chip->ecc.bytes	= 0x8;
	//chip->ecc.steps	= 0x4;
	chip->ecc.steps = info->flash_info->phy_page_ratio;

	mtd->owner = THIS_MODULE;
	mtd->dev.parent = dev;

	chip->ecc.strength = 1;
	chip->ecc.total	= chip->ecc.steps * chip->ecc.bytes;

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
		mtd_set_ooblayout(mtd, &spinand_oob_nvt_8bit);
	} else if (chip_id == MFG_ID_ETRON) {
		mtd_set_ooblayout(mtd, &spinand_oob_etron);
	} else if (chip_id == MFG_ID_GD) {
		mtd_set_ooblayout(mtd, &spinand_oob_gd);
	} else {
		mtd_set_ooblayout(mtd, &spinand_oob_64);
	}

	//nand = &chip->nand;
	chip->controller = &info->controller;

	nand_set_flash_node(chip, np);
	nand_set_controller_data(chip, info);

	chip->ecc.read_page = spinand_read_page_hwecc;
	chip->ecc.read_oob = spinand_read_oob;
	//chip->ecc.read_subpage	// TODO
	//chip->ecc.read_page_raw
	//chip->ecc.read_oob_raw

	chip->ecc.write_page = spinand_write_page_hwecc;
	chip->ecc.write_oob = spinand_write_oob;
	//chip->ecc.write_subpage	// TODO
	//chip->ecc.write_page_raw
	//chip->ecc.write_oob_raw

	//chip->legacy.select_chip = nvt_select_chip;

	chip->ecc.hwctl = drv_nand_ecc_hwctl;
	chip->ecc.calculate = drv_nand_ecc_calculate;
	chip->ecc.correct = drv_nand_ecc_correct;
	chip->options |= NAND_NO_SUBPAGE_WRITE | NAND_BROKEN_XD;

	chip->priv	= info;

	//chip->read_byte	= spinand_read_byte;
	//chip->cmdfunc	= spinand_cmdfunc;
	//chip->waitfunc	= spinand_wait;
	//chip->select_chip = spinand_select_chip;
	//chip->set_features = nand_get_set_features_notsupp;
	//chip->get_features = nand_get_set_features_notsupp;
	info->flash_info->spi_nand_status.block_unlocked = FALSE;
	info->flash_info->spi_nand_status.quad_en = FALSE;
	info->flash_info->spi_nand_status.quad_program = FALSE;
	info->flash_info->spi_nand_status.timer_record = FALSE;
}

static int spinand_probe(struct platform_device *pdev)
{
	struct drv_nand_dev_info *info;
	struct mtd_part_parser_data ppdata;
	struct platform_nand_data *pplatdata;
	struct nand_chip *this;
	struct mtd_info *mtd;
	struct resource *res;
	int ret = 0;
	u8 chip_id;
	u32 eccmode;
#if defined(CONFIG_NVT_IVOT_PLAT_NA51102)|| defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	u32 loader_clk = 0;
#endif

	dev_info(&pdev->dev, "nand version: %s \n", NAND_VERSION);

	info = devm_kzalloc(&pdev->dev, sizeof(struct drv_nand_dev_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->pdev = pdev;
	info->controller.ops = &nvt_controller_ops;
	mutex_init(&info->controller.lock);

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(36))) {
		dev_err(&pdev->dev, "set mask to 36bit fail!\n");
	}

	info->flash_info = kmalloc(sizeof(struct nvt_nand_flash), GFP_KERNEL);
	if (!info->flash_info) {
		dev_err(&pdev->dev, "failed to allocate nvt_nand_flash\n");
		return -ENOMEM;
	}

	pplatdata = NULL;
	if (pdev->dev.platform_data != NULL)
	{
		pplatdata = pdev->dev.platform_data;
	}

	platform_set_drvdata(pdev, info);
	this = &info->nand_chip;
	mtd = nand_to_mtd(this);

	/* Get mtd device resource of base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (!res) {
		dev_err(&pdev->dev, "failed to get resource\n");
		goto fail_free_mtd;
	}

	info->mmio_base = ioremap(res->start, (res->end - res->start + 1));

	if (IS_ERR(info->mmio_base)) {
		ret = PTR_ERR(info->mmio_base);
		goto fail_free_mtd;
	}
    dev_err(&pdev->dev, "mmio_base: 0x%lx\n", (long unsigned int)info->mmio_base);

	info->irq = platform_get_irq(pdev, 0);
	if (info->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource defined\n");
		ret = -ENXIO;
		goto fail_free_mtd;
	}

	ret = drv_nand_init_buff(info);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate buffer\n");
		goto fail_free_buf;
	}

	info->clk = clk_get(&pdev->dev, dev_name(&pdev->dev));
	if (!IS_ERR(info->clk)) {
		clk_prepare(info->clk);
		clk_enable(info->clk);
	} else {
		pr_err("%s: %s not found\n", __func__, dev_name(&pdev->dev));
		return -ENXIO;
	}

#if defined(CONFIG_NVT_IVOT_PLAT_NA51102)|| defined(CONFIG_NVT_IVOT_PLAT_NS02201)
	loader_clk = clk_get_rate(info->clk);
	if((loader_clk == 120000000) || (loader_clk == 96000000)) {
		info->flash_freq = loader_clk;
	} else
#elif defined(CONFIG_NVT_IVOT_PLAT_NS02301) || defined(CONFIG_NVT_IVOT_PLAT_NS02302) || \
		defined(CONFIG_NVT_IVOT_PLAT_NS02401)
	of_property_read_u32(pdev->dev.of_node, "clock-frequency", &info->flash_freq);
	if (0)
#endif
	{
		of_property_read_u32(pdev->dev.of_node, "clock-frequency", &info->flash_freq);
		if (info->flash_freq > 80000000) {
			dev_info(&pdev->dev, "freq %d out of range, fixed to 80MHz\n", info->flash_freq);
			info->flash_freq = 80000000;
		}
	}

	info->flash_info->chip_sel = 0;
	of_property_read_u32(pdev->dev.of_node, "chip-select",
		&info->flash_info->chip_sel);

	dev_info(&pdev->dev, "chip_select %d \n", info->flash_info->chip_sel);

	if (of_property_read_bool(pdev->dev.of_node, "ecc-mode"))
		of_property_read_u32(pdev->dev.of_node, "ecc-mode", &eccmode);
	else
		eccmode = 0;

	dev_info(&pdev->dev, "ecc-mode: %d \n", eccmode);

	if (eccmode) {
		info->flash_info->spare_align = 1;
		info->flash_info->spare_protect = 1;
		if (eccmode == 2)
			info->flash_info->spare_size = 1;
		else
			info->flash_info->spare_size = 0;
	} else {
		info->flash_info->spare_size = 0;
		info->flash_info->spare_align = 0;
		info->flash_info->spare_protect = 0;
	}

	if (of_property_read_bool(pdev->dev.of_node, "indly")) {
		of_property_read_u32(pdev->dev.of_node, "indly", &info->flash_info->indly);
		dev_info(&pdev->dev, "dtsi indly = %d \n", info->flash_info->indly);
	} else
		info->flash_info->indly = 0;

#if (0)
	ret = request_irq(info->irq, spinand_irq, IRQF_SHARED, pdev->name, info);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		goto fail_free_buf;
	}
#else
	ret = request_common_irq(info->irq, spinand_irq, IRQF_SHARED, \
		pdev->name, info, info->flash_info->chip_sel);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		goto fail_free_buf;
	}
#endif

	info->lock = mutex_common_init();

	init_completion(&info->cmd_complete);

	mutex_lock(info->lock);
	drv_nand_reset(info, pdev);
	nand_phy_config(info);
	nand_dll_reset(info);
	mutex_unlock(info->lock);
	/*Delay 1ms for spinand characteristic*/
	mdelay(1);

	ret = drv_nand_identify(info);

	if (ret) {
		dev_err(&pdev->dev, "failed to identify flash\n");
		ret = -ENODEV;
		goto fail_free_irq;
	}

	spinand_init_mtd(info);

	nand_set_flash_node(&info->nand_chip, pdev->dev.of_node);

#ifdef CONFIG_OF
	of_property_read_string_index(pdev->dev.of_node, "nvt-devname", 0, &mtd->name);
#else
	mtd->name = dev_name(&pdev->dev);
#endif
	mtd->owner = THIS_MODULE;
	mtd->oobsize = 64;

	chip_id = info->flash_info->chip_id & 0xFF;

#ifndef CONFIG_FLASH_ONLY_DUAL
	if (((info->flash_info->chip_id & 0xFFFF) != TOSHIBA_TC58CVG) && \
		((info->flash_info->chip_id & 0xFFFF) != TOSHIBA_TC58CVG3) && \
		((info->flash_info->chip_id & 0xFFFF) != TOSHIBA_TC58CVG2S0HRAIG) && \
		((info->flash_info->chip_id & 0xFFFF) != TOSHIBA_TC58CVG2))
		info->flash_info->spi_nand_status.quad_program = TRUE;
#endif

	ret = nand_scan_with_ids(this, 1, spinand_flash_ids);
	if (ret) {
		dev_err(&pdev->dev, "Identify nand fail\n");
		goto fail_free_irq;
	}

	dev_info(&pdev->dev, "%d-bit mode @ %d Hz\n",
		info->flash_info->spi_nand_status.quad_program ? 4 : 1,
		info->flash_freq);

#ifdef CONFIG_NVT_IVOT_PLAT_NA51103
	if (info->flash_info->spi_nand_status.quad_program == 0) {
		PIN_GROUP_CONFIG pin_group = {PIN_FUNC_SPI, 0};
		int ret = 0;

		ret = nvt_pinmux_capture(&pin_group, 1);
		if (ret) {
			dev_err(&pdev->dev, "get pinmux config failed\n");
		} else if (pin_group.config & PIN_SPI_CFG_CH_BUS_WIDTH) {
			pin_group.config &= ~PIN_SPI_CFG_CH_BUS_WIDTH;
			ret = nvt_pinmux_update(&pin_group, 1);
			if (ret)
				dev_err(&pdev->dev, "spi-flash pinmux update failed\n");

			if ((gpio_request(C_GPIO(4), "spi_wp")) || (gpio_request(C_GPIO(5), "spi_hold"))) {
				dev_err(&pdev->dev, "failed to request C_GPIO(4-5)\n");
			} else {
				// Set spi_wp & spi_hold output high
				gpio_direction_output(C_GPIO(4), 1);
				gpio_direction_output(C_GPIO(5), 1);

				gpio_set_value(C_GPIO(4), 1);
				gpio_set_value(C_GPIO(5), 1);
				dev_info(&pdev->dev, "(331) single/dual mode: WP/HOLD force output high\n");
			}
		}
	}
#endif

	ret = nvt_nand_proc_init();
	if (ret) {
		dev_err(&pdev->dev, "create nand static proc failed\n");
	}

	//ppdata.of_node = pdev->dev.of_node;
	mtd->dev.of_node = pdev->dev.of_node;;
	if (pplatdata == NULL)
		return mtd_device_register(mtd, NULL, 0);
	else
		return mtd_device_parse_register(mtd, NULL, &ppdata,
				pplatdata->chip.partitions,
				pplatdata->chip.nr_partitions);

fail_free_irq:
	free_irq(info->irq, info);

fail_free_buf:
	kfree(info->data_buff);
	//dma_free_coherent(&pdev->dev, MAX_BUFF_SIZE, info->data_buff, info->data_desc_addr);

fail_free_mtd:
	kfree(mtd);
	return ret;
}

static int spinand_remove(struct platform_device *pdev)
{
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct drv_nand_dev_info *info = mtd->priv;
    struct nand_chip *chip = mtd_to_nand(mtd);

	platform_set_drvdata(pdev, NULL);

	/* Unregister device */
	WARN_ON(mtd_device_unregister(mtd));

	/* Release resources */
	nand_cleanup(chip);

	free_irq(info->irq, info);

	dma_free_coherent(&pdev->dev, MAX_BUFF_SIZE, info->data_buff, info->data_desc_addr);

	kfree(mtd);
	return E_OK;
}

#ifdef CONFIG_PM
static int spinand_suspend(struct device *dev)
{
	struct drv_nand_dev_info *info = dev_get_drvdata(dev);

	if (!IS_ERR(info->clk))
		clk_disable(info->clk);

	return 0;
}

static int spinand_resume(struct device *dev)
{
	struct drv_nand_dev_info *info = dev_get_drvdata(dev);
	int ret;
	int id;
	unsigned char status;

	clk_enable(info->clk);

        if (nvt_get_suspend_mode() == 0)
		return 0;

	drv_nand_reset(info, info->pdev);

	nand_phy_config(info);

	nand_dll_reset(info);

	ret = drv_nand_identify(info);
	if (ret < 0)
		return -ENODEV;

	status = nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
		status &= ~SPINAND_CONFIG_ECCEN;
	}
#if NVT_BCH_ECC_SUPPORT
	else if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
		status &= ~SPINAND_CONFIG_ECCEN;
	}
#endif
	else {
		status |= SPINAND_CONFIG_ECCEN;
	}

	id = info->flash_info->chip_id;
#ifndef CONFIG_FLASH_ONLY_DUAL
	if (((id & 0xFF) == MFG_ID_MXIC) || ((id & 0xFF) == MFG_ID_GD) || \
			((id & 0xFF) == MFG_ID_DOSILICON) || ((id & 0xFF) == MFG_ID_XTX)) {
		status |= SPINAND_CONFIG_QUADEN;
	}
#endif

	nand_cmd_write_status(info, NAND_SPI_STS_FEATURE_2, status);

	return 0;
}

static const struct dev_pm_ops spinand_pmops = {
	.suspend	= spinand_suspend,
	.resume		= spinand_resume,
};
#define SPINAND_PMOPS &spinand_pmops
#else
#define SPINAND_PMOPS NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_OF
static const struct of_device_id nvt_spinand_of_dt_ids[] = {
    { .compatible = "nvt,nvt_spinand" },
    {},
};

MODULE_DEVICE_TABLE(of, nvt_spinand_of_dt_ids);
#endif

static struct platform_driver nvt_nand_driver = {
	.driver = {
		.name = "spi_nand",
		.owner = THIS_MODULE,
		.pm = SPINAND_PMOPS,
#ifdef CONFIG_OF
		.of_match_table = nvt_spinand_of_dt_ids,
#endif
	},
	.probe	    = spinand_probe,
	.remove     = spinand_remove,
};

static int __init spinand_init(void)
{
	return platform_driver_register(&nvt_nand_driver);
}

static void __exit spinand_exit(void)
{
	platform_driver_unregister(&nvt_nand_driver);
}

module_init(spinand_init);
module_exit(spinand_exit);

MODULE_DESCRIPTION("SPI NAND driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(NAND_VERSION);

