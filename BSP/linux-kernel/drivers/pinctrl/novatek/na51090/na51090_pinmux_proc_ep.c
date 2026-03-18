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
#include <linux/version.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "na51090_pinmux.h"
#include <linux/soc/nvt/nvt-pcie-lib.h>

#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

static char *pinmux_table[] = {
	"uart",
	"i2c",
	"sdio",
	"spi",
	"extclk",
	"ssp",
	"lcd",
	"remote",
	"vcap",
	"eth",
	"misc",
	"pwm",
	"mipi"
};

static int nvt_pinmux_parse(unsigned char argc, char **pargv, int ep_ch)
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
		pr_err("invalid length of module name:%ld\n", strlen(pargv[0]));
		return -EINVAL;
	}

	if (kstrtoul(pargv[1], 0, &config)) {
		pr_err("invalid config:%s\n", pargv[1]);
		return -EINVAL;
	}

	if (ep_ch + CHIP_EP0 > nvtpcie_get_ep_count(PCI_DEV_ID_NA51090) || ep_ch + CHIP_EP0 > CHIP_MAX) {
		pr_err("invalid ep id %d \r\n", ep_ch + CHIP_EP0);
		return -EINVAL;;
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

	return nvt_pinmux_update_ep(pinmux_config, 1, ep_ch);
}

static int nvt_pinmux_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_pinmux_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_cmd_show, NULL);
}

/*=============================================================================
 * proc "get" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_gpio_show_ep(struct seq_file *sfile, void *v)
{
	struct nvt_pinctrl_info *info = PDE_DATA(file_inode(sfile->file));

	if (!info) {
		pr_err("invalid pinctrl address\n");
		return -EINVAL;
	}

	if (info->top_base) {
		pinmux_gpio_parsing(info);
	} else {
		pr_err("invalid pinmux address\n");
		return -EINVAL;
	}

	return 0;
}


static int nvt_pinmux_proc_gpio_open_ep(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_gpio_show_ep, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_gpio_fops_ep = {
	.proc_open   = nvt_pinmux_proc_gpio_open_ep,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#else
static struct file_operations proc_gpio_fops_ep = {
	.owner  = THIS_MODULE,
	.open   = nvt_pinmux_proc_gpio_open_ep,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#endif

/*=============================================================================
 * proc "Custom Command" file operation functions
 *=============================================================================
 */

static ssize_t nvt_pinmux_proc_cmd_write_ep(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	struct nvt_pinctrl_info *info = PDE_DATA(file_inode(file));

	if (!info) {
		pr_err("invalid pinctrl address\n");
		return -EINVAL;
	}

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

	if (nvt_pinmux_parse(ucargc, &argv[0], info->ep_ch)) {
		goto ERR_OUT;
	} else {
		return size;
	}
ERR_OUT:
	return -1;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_cmd_fops_ep = {
	.proc_open    = nvt_pinmux_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_pinmux_proc_cmd_write_ep
};
#else
static struct file_operations proc_cmd_fops_ep = {
	.owner   = THIS_MODULE,
	.open    = nvt_pinmux_proc_cmd_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = nvt_pinmux_proc_cmd_write_ep
};
#endif

/*=============================================================================
 * proc "get" file operation functions
 *=============================================================================
 */

static int nvt_pinmux_proc_summary_show_ep(struct seq_file *sfile, void *v)
{
	u8 count;
	struct nvt_pinctrl_info *info = PDE_DATA(file_inode(sfile->file));

	if (!info) {
		pr_err("invalid pinctrl address\n");
		return -EINVAL;
	}

	if (info->top_base) {
		pinmux_parsing_ep(info);

		seq_printf(sfile, "\n&top {\n");
		for (count = 0; count < PIN_FUNC_MAX; count++) {
			seq_printf(sfile, "%s{pinmux=<0x%x>;};\n", pinmux_table[count], info->top_pinmux[count].config);
		}

		seq_printf(sfile, "};\n");
	} else {
		pr_err("invalid pinmux address\n");
		return -EINVAL;
	}

	return 0;
}

static int nvt_pinmux_proc_summary_open_ep(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_summary_show_ep, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_summary_fops_ep = {
	.proc_open   = nvt_pinmux_proc_summary_open_ep,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#else
static struct file_operations proc_summary_fops_ep = {
	.owner  = THIS_MODULE,
	.open   = nvt_pinmux_proc_summary_open_ep,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#endif

/*=============================================================================
 * proc "parsing_i2c" file operation functions
 *=============================================================================
 */
static int nvt_pinmux_proc_parsing_i2c_show(struct seq_file *sfile, void *v)
{
	struct nvt_pinctrl_info *info = PDE_DATA(file_inode(sfile->file));
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret = 0;

	if (!info) {
		pr_err("invalid pinctrl address\n");
		return -EINVAL;
	}

	pinmux_config->pin_function = PIN_FUNC_I2C;
	pinmux_config->config = 0x0;
	ret = nvt_pinmux_capture_ep(pinmux_config, 1, info->ep_ch);
	if (ret) {
    	pr_info("I2C capture err\n");
	} else {
		ret = pinmux_parsing_i2c_ep((uint32_t)pinmux_config[0].config, info->ep_ch);
		if(ret > 0) {
			pr_info(" I2C pad check pull none Fail(force pull none)\n");
		} else {
			pr_info(" I2C pad check pull none PASS\n");
		}
	}
	return 0;
}

static int nvt_pinmux_proc_i2c_pad_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pinmux_proc_parsing_i2c_show, NULL);
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_parsing_i2c_fops_ep = {
	.proc_open   = nvt_pinmux_proc_i2c_pad_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#else
static struct file_operations proc_parsing_i2c_fops_ep = {
	.owner  = THIS_MODULE,
	.open   = nvt_pinmux_proc_i2c_pad_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#endif

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

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_help_fops = {
	.proc_open   = nvt_pinmux_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#else
static struct file_operations proc_help_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_pinmux_proc_help_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#endif

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

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
static struct proc_ops proc_version_fops = {
	.proc_open   = nvt_pinmux_proc_version_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};
#else
static struct file_operations proc_version_fops = {
	.owner  = THIS_MODULE,
	.open   = nvt_pinmux_proc_version_open,
	.release = single_release,
	.read   = seq_read,
	.llseek = seq_lseek,
};
#endif

int nvt_pinmux_proc_init_ep(struct nvt_pinctrl_info *info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;
	char path[128];

	if (!info) {
		pr_err("invalid pinctrl address\n");
		return -EINVAL;
	}

	snprintf(path, sizeof(path), "nvt_info/nvt_pinmux/ep%d", info->ep_ch);

	pmodule_root = proc_mkdir(path, NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_proc;
	}

	if (info->ep_ch + CHIP_EP0 > nvtpcie_get_ep_count(PCI_DEV_ID_NA51090) || info->ep_ch + CHIP_EP0 > CHIP_MAX) {
		pr_err("invalid ep id %d \r\n", info->ep_ch + CHIP_EP0);
		return -EINVAL;;
	} else {
		pentry = proc_create_data("pinmux_set", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops_ep, info);
		if (pentry == NULL) {
			pr_err("failed to create proc cmd!\n");
			ret = -EINVAL;
			goto remove_proc;
		}

		pentry = proc_create_data("pinmux_summary", S_IRUGO | S_IXUGO, pmodule_root, &proc_summary_fops_ep, info);
		if (pentry == NULL) {
			pr_err("failed to create proc pinmux_summary!\n");
			ret = -EINVAL;
			goto remove_proc;
		}

		pentry = proc_create_data("gpio_summary", S_IRUGO | S_IXUGO, pmodule_root, &proc_gpio_fops_ep, info);
		if (pentry == NULL) {
			pr_err("failed to create proc gpio!\n");
			ret = -EINVAL;
			goto remove_proc;
		}
		pentry = proc_create_data("i2c_pad_chk", S_IRUGO | S_IXUGO, pmodule_root, &proc_parsing_i2c_fops_ep, info);
		if (pentry == NULL) {
			pr_err("failed to create proc i2c!\n");
			ret = -EINVAL;
			goto remove_proc;
		}
		pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
		if (pentry == NULL) {
			pr_err("failed to create proc help!\n");
			ret = -EINVAL;
			goto remove_proc;
		}

		pentry = proc_create("version", S_IRUGO | S_IXUGO, pmodule_root, &proc_version_fops);
		if (pentry == NULL) {
			pr_err("failed to create proc efuse!\n");
			ret = -EINVAL;
			goto remove_proc;
		}
	}

remove_proc:
	return ret;
}
EXPORT_SYMBOL(nvt_pinmux_proc_init_ep);
