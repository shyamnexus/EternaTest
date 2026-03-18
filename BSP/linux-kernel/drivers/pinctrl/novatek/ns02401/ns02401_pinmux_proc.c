/*
    Pinmux module driver.

    This file is the driver of Piumux module.

    @file       ns02201_pinmux_proc.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
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
#include "ns02401_pinmux.h"
#include <linux/scatterlist.h>
#include <linux/crypto.h>

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
} proc_pinmux_t;
proc_pinmux_t proc_pinmux;

static char *pinmux_table[] = {
	"sdio",
	"nand",
	"eth",
	"i2c",
	"pwm",
	"audio",
	"uart",
	"remote",
	"sdp",
	"spi",
	"misc",
	"lcd",
	"tv",
	"sel_lcd",
	"pcie",
	"pcie_sel",
};

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
	.proc_write   = nvt_pinmux_proc_cmd_write
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
	return single_open(file,nvt_pinmux_proc_pinmux_show, NULL);
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

remove_proc:
	return ret;
}
