/*
 * Proc interface for the Novatek pinmux
 *
 * Copyright (c) 2018, NOVATEK MICROELECTRONIC CORPORATION.  All rights reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <plat/efuse_protected.h>
#include <plat/dma_protected.h>
#include "na51102_pinmux.h"
#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <plat/nvt-sramctl.h>


#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6
extern void gpio_func_show(void);
extern unsigned long top_reg_addr;

typedef struct proc_pinmux {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
	struct proc_dir_entry *pproc_summary_entry;
	struct proc_dir_entry *pproc_gpio_entry;
	struct proc_dir_entry *pproc_efuse_entry;
	struct proc_dir_entry *pproc_chipid_entry;
	struct proc_dir_entry *pproc_version_entry;
	struct proc_dir_entry *pproc_i2c_pad_entry;
	struct proc_dir_entry *pproc_sram_shutdown_entry;
} proc_pinmux_t;
proc_pinmux_t proc_pinmux;

static u32 sram_nbr = 0;
static unsigned long sram_table[] = {
	SDIO_SD,          //< Shut Down SDIO SRAM
	SDIO2_SD,         //< Shut Down SDIO2 SRAM
	SDIO3_SD,         //< Shut Down SDIO3 SRAM

	SMC_SD,           //< Shut Down SMC SRAM
	TSE_SD,           //< Shut Down TSMUX SRAM

	JPG_SD,           //< Shut Down JPG SRAM
	JPG2_SD,          //< Shut Down JPG2 SRAM
	GRAPH_SD,         //< Shut Down Graphic SRAM
	GRAPH2_SD,        //< Shut Down Graphic2 SRAM

	/*0x1004*/
	GRAPH3_SD,        //< Shut Down Graphic3 SRAM
	DIS_SD,           //< Shut Down DIS SRAM
	DRE_SD,           //< Shut Down DRE SRAM
	IVE_SD,           //< Shut Down IVE SRAM
	SDE_SD,           //< Shut Down SDE SRAM
	TRKE_SD,          //< Shut Down TRKE SRAM
	CNN_SD,           //< Shut Down CNN SRAM
	CNN2_SD,          //< Shut Down CNN2 SRAM
	DCE_SD,           //< Shut Down DCE SRAM
	NUE_SD,           //< Shut Down NUE SRAM
	NUE2_SD,          //< Shut Down NUE2 SRAM
};

static char *pinmux_table[] = {
	"sdio",
	"nand",
	"eth",
	"i2c",
	"i2cII",
	"pwm",
	"pwmII",
	"ccnt",
	"sensor",
	"sensor2",
	"sensor3",
	"sensormisc",
	"sensorsync",
	"mipi_lvds",
	"audio",
	"uart",
	"uartII",
	"remote",
	"sdp",
	"spi",
	"sif",
	"misc",
	"lcd",
	"lcd2",
	"tv",
	"sel_lcd",
	"sel_lcd2",
};

static unsigned long    sram_original_value[2];
static u32  sram_ptr = 0;

/*=============================================================================
 * proc "get" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_gpio_show(struct seq_file *sfile, void *v)
{
	struct nvt_pinctrl_info *info;

	info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	if (top_reg_addr) {
		info->top_base = (void *) top_reg_addr;
		pinmux_gpio_parsing(info);
	} else {
		pr_err("invalid pinmux address\n");
		kfree(info);
		return -ENOMEM;
	}

	kfree(info);

	return 0;
}


static int nvt_pinmux_proc_gpio_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_gpio_show, NULL);
}

static struct proc_ops proc_gpio_fops = {
	.proc_open   = nvt_pinmux_proc_gpio_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};


/*=============================================================================
 * proc "Custom Command" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_pinmux_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_cmd_show, NULL);
}


static int nvt_pinmux_parse(unsigned char argc, char **pargv)
{
	unsigned long config = 0x0;
	char module[MAX_MODULE_NAME];
	PIN_GROUP_CONFIG pinmux_config[1] = {0};
	u8 count;

	if (argc != 2) {
		pr_err("wrong argument:%d", argc);
		return -EINVAL;
	}

	if (strlen(pargv[0]) <= MAX_MODULE_NAME) {
		strcpy(module, pargv[0]);
	} else {
		pr_err("invalid length of module name:%ld\n", (unsigned long)strlen(pargv[0]));
		return -EINVAL;
	}

	if (kstrtoul(pargv[1], 0, &config)) {
		pr_err("invalid config:%s\n", pargv[1]);
		return -EINVAL;
	}

	/*Find the module name and replace the configuration*/
	for (count = 0; count < PIN_FUNC_MAX; count++) {
		if (!strcmp(module, pinmux_table[count])) {
			pinmux_config->pin_function = count;
			pinmux_config->config = config;
			break;
		}
	}

	if (count == PIN_FUNC_MAX) {
		pr_err("module name invalid %s\n", module);
		return -EINVAL;
	}

	return nvt_pinmux_update(pinmux_config, 1);
}


static ssize_t nvt_pinmux_proc_cmd_write(struct file *file, const char __user *buf,
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

	printk("CMD:%s\n", cmd_line);

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (nvt_pinmux_parse(ucargc, &argv[0])) {
		goto ERR_OUT;
	} else {
		return size;
	}

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = nvt_pinmux_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_pinmux_proc_cmd_write,
};

/*=============================================================================
 * proc "get" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_summary_show(struct seq_file *sfile, void *v)
{
	struct nvt_pinctrl_info *info;
	u8 count;

	info = kmalloc(sizeof(struct nvt_pinctrl_info), GFP_KERNEL);
	if (!info) {
		pr_err("nvt pinmux mem alloc fail\n");
		return -ENOMEM;
	}

	if (top_reg_addr) {
		info->top_base = (void *) top_reg_addr;
		pinmux_parsing(info);

		seq_printf(sfile, "\n&top {\n");
		for (count = 0; count < PIN_FUNC_MAX; count++) {
			seq_printf(sfile, "%s{pinmux=<0x%x>;};\n", pinmux_table[count], info->top_pinmux[count].config);
		}

		seq_printf(sfile, "};\n");
	} else {
		pr_err("invalid pinmux address\n");
		kfree(info);
		return -ENOMEM;
	}

	kfree(info);

	return 0;
}

static int nvt_pinmux_proc_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_summary_show, NULL);
}

static struct proc_ops proc_summary_fops = {
	.proc_open   = nvt_pinmux_proc_summary_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};


/*=============================================================================
 * proc "help" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho module pinmux_config > pinmux_set\n\n");
	seq_printf(sfile, "module           : name of modules\n");
	seq_printf(sfile, "                   example: sdio, nand, spi\n");
	seq_printf(sfile, "pinmux_config    : pinmux configuration\n");
	seq_printf(sfile, "                   example: 0x25, please refer modelext table for value\n");
	seq_printf(sfile, "Example          : echo spi 0x370 > /proc/nvt_info/nvt_pinmux/pinmux_set\n");
	return 0;
}

static int nvt_pinmux_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   = nvt_pinmux_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

/*=============================================================================
 * proc "parsing_i2c" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_parsing_i2c_show(struct seq_file *sfile, void *v)
{
	PIN_GROUP_CONFIG pinmux_config[2];
	int ret;
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	pinmux_config[0].config = 0x0;
	pinmux_config[1].pin_function = PIN_FUNC_I2CII;
	pinmux_config[1].config = 0x0;
	ret = nvt_pinmux_capture(pinmux_config, 2);
	if (ret) {
		pr_info("I2C capture err\n");
	} else {
		if (pinmux_parsing_i2c((uint32_t)pinmux_config[0].config) > 0) {
			pr_info(" I2C pad check pull none Fail(force pull none)\n");
		} else {
			pr_info(" I2C pad check pull none PASS\n");
		}

		if (pinmux_parsing_i2cII((uint32_t)pinmux_config[1].config) > 0) {
			pr_info("I2C2 pad check pull none Fail(force pull none)\n");
		} else {
			pr_info("I2C2 pad check pull none PASS\n");
		}
	}
	return 0;
}

static int nvt_pinmux_proc_i2c_pad_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_parsing_i2c_show, NULL);
}

static struct proc_ops proc_parsing_i2c_fops = {
	.proc_open   = nvt_pinmux_proc_i2c_pad_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

/*=============================================================================
 * proc "sram_shutdown" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_sram_shutdown_show(struct seq_file *sfile, void *v)
{
#if 0
	PIN_GROUP_CONFIG pinmux_config[2];
	int ret;
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	pinmux_config[0].config = 0x0;
	pinmux_config[1].pin_function = PIN_FUNC_I2CII;
	pinmux_config[1].config = 0x0;
	ret = nvt_pinmux_capture(pinmux_config, 2);
	if (ret) {
		pr_info("I2C capture err\n");
	} else {
		if (pinmux_parsing_i2c((uint32_t)pinmux_config[0].config) > 0) {
			pr_info(" I2C pad check pull none Fail(force pull none)\n");
		} else {
			pr_info(" I2C pad check pull none PASS\n");
		}

		if (pinmux_parsing_i2cII((uint32_t)pinmux_config[1].config) > 0) {
			pr_info("I2C2 pad check pull none Fail(force pull none)\n");
		} else {
			pr_info("I2C2 pad check pull none PASS\n");
		}
	}
#endif
	return 0;
}

#define WRITE_REG(VALUE, ADDR)  iowrite32(VALUE, ADDR)
#define READ_REG(ADDR)          ioread32(ADDR)

static ssize_t nvt_pinmux_proc_sram_shutdown_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{

	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	u32 sram_idx;

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

	printk("CMD:%s\n", cmd_line);

	/*parse command string*/
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ucargc < 1) {
		pr_err("NULL command error\n");
		goto ERR_OUT;
	}

	if (!strcmp(argv[0], "sram_init")) {
		sram_ptr = 0;
		sram_nbr = sizeof(sram_table) / sizeof(unsigned long);
		sram_original_value[0] = READ_REG((void __iomem *)top_reg_addr + 0x1000);
		sram_original_value[1] = READ_REG((void __iomem *)top_reg_addr + 0x1004);
		pr_info("0x1000[0x%08x][0x%08x][Init][SUCCESS][%d]\r\n", (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), sram_nbr);
	} else if (!strcmp(argv[0], "sram_enable")) {
		pr_cont("[%2d]:enum[%2d] dis[%d]->en\r\n", (int)sram_ptr, (int)sram_table[sram_ptr], (int)((READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) >> (sram_table[sram_idx] % 32)) & 0x1)) ;
		nvt_enable_sram_shutdown(sram_table[sram_ptr]);
		pr_info("0x1000[0x%08x][0x%08x]\r\n", (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), (int)READ_REG((void __iomem *)top_reg_addr + 0x1004));
	} else if (!strcmp(argv[0], "sram_disable")) {
		pr_cont("[%2d] enum[%2d]  en[%d]->dis", (int)sram_ptr, (int)sram_table[sram_ptr], (int)((READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) >> (sram_table[sram_idx] % 32)) & 0x1)) ;
		nvt_disable_sram_shutdown(sram_table[sram_ptr]);
		pr_info("0x1000[0x%08x][0x%08x]\r\n", (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), (int)READ_REG((void __iomem *)top_reg_addr + 0x1004));
	} else if (!strcmp(argv[0], "sram_next")) {
		if (sram_ptr < (sram_nbr - 1)) {
			sram_ptr++;
			pr_cont("[SUCCESS][%d][%d]\r\n", sram_ptr, (sram_nbr - 1)) ;
		} else {
			WRITE_REG((unsigned long)sram_original_value[0], (void __iomem *)(top_reg_addr + 0x1000));
			WRITE_REG((unsigned long)sram_original_value[1], (void __iomem *)(top_reg_addr + 0x1004));
			pr_info("0x1000[0x%08x][0x%08x]\r\n", (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), (int)READ_REG((void __iomem *)top_reg_addr + 0x1004));
			pr_cont("[FINISH]\r\n") ;
		}
	} else if (!strcmp(argv[0], "sram_self_test")) {
		sram_ptr = 0;
		sram_original_value[0] = READ_REG((void __iomem *)top_reg_addr + 0x1000);
		sram_original_value[1] = READ_REG((void __iomem *)top_reg_addr + 0x1004);
		pr_info("0x1000[0x%08x][0x%08x]\r\n", (int)sram_original_value[0], (int)sram_original_value[1]);

		sram_nbr = sizeof(sram_table) / sizeof(unsigned long);
		pr_info("Test item[%2d]\r\n", (int)sram_nbr);
		for (sram_idx = 0; sram_idx < sram_nbr; sram_idx++) {
			pr_cont("[%2d]:enum[%2d] dis[%d]->en", (int)sram_idx, (int)sram_table[sram_idx], (int)((READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) >> (sram_table[sram_idx] % 32)) & 0x1)) ;
			nvt_enable_sram_shutdown(sram_table[sram_idx]);
//			pr_info("0x1000[0x%08x][0x%08x]\r\n", (int)READ_REG((void __iomem *)top_reg_addr + 0x1000), (int)READ_REG((void __iomem *)top_reg_addr + 0x1004));
//			pr_info("sram_original_value[sram_table[sram_idx]/32]->[0x%08x] => [0x%08x]\r\n", (int)sram_original_value[sram_table[sram_idx]/32], (1<<(sram_table[sram_idx]%32)));
			if ((READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) & (1 << (sram_table[sram_idx] % 32)))) {
				pr_cont("  => [O]<SUCCESS>\r\n");
			} else {
				pr_cont("  => [X]<FAIL>\r\n");
				continue;
			}

			nvt_disable_sram_shutdown(sram_table[sram_idx]);
			pr_cont("[%2d] enum[%2d]  en[%d]->dis", (int)sram_idx, (int)sram_table[sram_idx], (int)((READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) >> (sram_table[sram_idx] % 32)) & 0x1)) ;
			if (!(READ_REG((void __iomem *)(top_reg_addr + 0x1000 + (sram_table[sram_idx] / 32 * 4))) & (1 << (sram_table[sram_idx] % 32)))) {
				pr_cont(" => [O]<SUCCESS>\r\n");
			} else {
				pr_cont(" => [X]<FAIL>\r\n");
				continue;
			}
		}
		WRITE_REG((unsigned long)sram_original_value[0], (void __iomem *)(top_reg_addr + 0x1000));
		WRITE_REG((unsigned long)sram_original_value[1], (void __iomem *)(top_reg_addr + 0x1004));
	}
	return size;
ERR_OUT:
	return -1;
}

static int nvt_pinmux_proc_sram_shutdown_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_sram_shutdown_show, NULL);
}

static struct proc_ops proc_sram_shutdown_fops = {
	.proc_open   = nvt_pinmux_proc_sram_shutdown_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write  = nvt_pinmux_proc_sram_shutdown_write,
};

/*=============================================================================
 * proc "chip_id" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_chipid_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "0x%x\n", nvt_get_chip_id());
	return 0;
}

static int nvt_pinmux_proc_chipid_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_chipid_show, NULL);
}

static struct proc_ops proc_chipid_fops = {
	.proc_open   = nvt_pinmux_proc_chipid_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
/*=============================================================================
 * proc "pinmux" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_pinmux_show(struct seq_file *sfile, void *v)
{
	gpio_func_show();
	return 0;
}
static int nvt_pinmux_proc_show_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_pinmux_show, NULL);
}


static struct proc_ops proc_pinmux_fops = {
	.proc_open   = nvt_pinmux_proc_show_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
/*=============================================================================
 * proc "version" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_version_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "%s\n", DRV_VERSION);
	return 0;
}

static int nvt_pinmux_proc_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_version_show, NULL);
}

static struct proc_ops proc_version_fops = {
	.proc_open   = nvt_pinmux_proc_version_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

int nvt_pinmux_proc_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_info/nvt_pinmux", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_module_root = pmodule_root;

	pentry = proc_create("pinmux_set", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_cmd_entry = pentry;

	pentry = proc_create("pinmux_summary", S_IRUGO | S_IXUGO, pmodule_root, &proc_summary_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_summary_entry = pentry;

	pentry = proc_create("gpio_summary", S_IRUGO | S_IXUGO, pmodule_root, &proc_gpio_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc gpio!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_gpio_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_help_entry = pentry;

	pentry = proc_create("chip_id", S_IRUGO | S_IXUGO, pmodule_root, &proc_chipid_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc efuse!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_chipid_entry = pentry;

	pentry = proc_create("pinmux_show", S_IRUGO | S_IXUGO, pmodule_root, &proc_pinmux_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc efuse!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_chipid_entry = pentry;

	pentry = proc_create("version", S_IRUGO | S_IXUGO, pmodule_root, &proc_version_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc efuse!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_version_entry = pentry;

	pentry = proc_create("i2c_pad_chk", S_IRUGO | S_IXUGO, pmodule_root, &proc_parsing_i2c_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc i2c!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_i2c_pad_entry = pentry;

	pentry = proc_create("sram_shutdown", S_IRUGO | S_IXUGO, pmodule_root, &proc_sram_shutdown_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc sram shutdown!\n");
		ret = -EINVAL;
		goto remove_proc;
	}
	proc_pinmux.pproc_sram_shutdown_entry = pentry;

remove_proc:
	return ret;
}
