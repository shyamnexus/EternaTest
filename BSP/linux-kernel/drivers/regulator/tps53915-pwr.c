/*
 * Novatek NA51039 Regulator driver.
 *
 * Copyright (C) 2018 Novatek MicroElectronics Corp.
 *
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

struct regulator *nvt_regulator_core;
struct regulator *nvt_regulator_cnn;
struct regulator *nvt_regulator_cpu;
struct regulator *nvt_regulator_dsp;
struct regulator *nvt_regulator_mp8870_main;
struct regulator *nvt_regulator_mp8870_preroll;

#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

typedef struct proc_regulator {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_cmd_entry_core;
	struct proc_dir_entry *pproc_cmd_entry_cnn;
	struct proc_dir_entry *pproc_cmd_entry_cpu;
	struct proc_dir_entry *pproc_cmd_entry_dsp;
	struct proc_dir_entry *pproc_cmd_entry_mp8870_main;
	struct proc_dir_entry *pproc_cmd_entry_mp8870_preroll;
} proc_regulator_t;

proc_regulator_t proc_regulator;


void nvt_regulator_mp8870_preroll_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_mp8870_preroll = regulator_get(0, "tpsMP8870_vout_preroll");

		if (!nvt_regulator_mp8870_preroll) {
			pr_err("failed to get regulator tpsMP8870_vout_preroll\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_mp8870_preroll, voltage, voltage);
}


static int nvt_regulator_mp8870_preroll_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_mp8870_preroll_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_mp8870_preroll_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_mp8870_preroll_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_mp8870_preroll_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_mp8870_preroll_fops = {
	.proc_open    = nvt_regulator_mp8870_preroll_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_mp8870_preroll_proc_cmd_write
};

void nvt_regulator_mp8870_main_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_mp8870_main = regulator_get(0, "tpsMP8870_vout_main");

		if (!nvt_regulator_mp8870_main) {
			pr_err("failed to get regulator tpsMP8870_vout_main\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_mp8870_main, voltage, voltage);
}


static int nvt_regulator_mp8870_main_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_mp8870_main_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_mp8870_main_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_mp8870_main_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_mp8870_main_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_mp8870_main_fops = {
	.proc_open    = nvt_regulator_mp8870_main_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_mp8870_main_proc_cmd_write
};

void nvt_regulator_dsp_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_dsp = regulator_get(0, "tps53915_vout_dsp");

		if (!nvt_regulator_dsp) {
			pr_err("failed to get regulator tps53915_vout_dsp\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_dsp, voltage, voltage);
}


static int nvt_regulator_dsp_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_dsp_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_dsp_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_dsp_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_dsp_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_dsp_fops = {
	.proc_open    = nvt_regulator_dsp_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_dsp_proc_cmd_write
};

void nvt_regulator_cpu_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_cpu = regulator_get(0, "tps53915_vout_cpu");

		if (!nvt_regulator_cpu) {
			pr_err("failed to get regulator tps53915_vout_cpu\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_cpu, voltage, voltage);
}


static int nvt_regulator_cpu_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_cpu_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_cpu_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_cpu_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_cpu_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}
static struct proc_ops proc_cmd_cpu_fops = {
	.proc_open    = nvt_regulator_cpu_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_cpu_proc_cmd_write
};

void nvt_regulator_cnn_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_cnn = regulator_get(0, "tps53915_vout_cnn");

		if (!nvt_regulator_cnn) {
			pr_err("failed to get regulator tps53915_vout_cnn\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_cnn, voltage, voltage);
}


static int nvt_regulator_cnn_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_cnn_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_cnn_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_cnn_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_cnn_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_cnn_fops = {
	.proc_open    = nvt_regulator_cnn_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_cnn_proc_cmd_write
};

void nvt_regulator_core_control(int voltage)
{
	static int count = 1;

	if (count) {
		nvt_regulator_core = regulator_get(0, "tps53915_vout_core");

		if (!nvt_regulator_core) {
			pr_err("failed to get regulator tps53915_vout_core\n");
			return;
		}

		count = 0;
	}

	regulator_set_voltage(nvt_regulator_core, voltage, voltage);
}


static int nvt_regulator_core_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_regulator_core_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_regulator_core_proc_cmd_show, NULL);
}

static ssize_t nvt_regulator_core_proc_cmd_write(struct file *file, const char __user *buf,
		size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned long config = 0x0;

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

	if (ucargc > 1) {
		goto ERR_OUT;
	} else {
		if (kstrtoul(argv[0], 0, &config)) {
			pr_err("invalid config:%s\n", argv[0]);
			goto ERR_OUT;
		}

		nvt_regulator_core_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_core_fops = {
	.proc_open    = nvt_regulator_core_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_regulator_core_proc_cmd_write
};


static int nvt_regulator_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("power_ctrl", NULL);
	if (pmodule_root == NULL) {
		pr_err("failed to create Module root\n");
		return -EINVAL;
	}
	proc_regulator.pproc_module_root = pmodule_root;

	pentry = proc_create("regulator_ctrl_core", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_core_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_core = pentry;

	pentry = proc_create("regulator_ctrl_cnn", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_cnn_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_cnn = pentry;

	pentry = proc_create("regulator_ctrl_cpu", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_cpu_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_cpu = pentry;

	pentry = proc_create("regulator_ctrl_dsp", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_dsp_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_dsp = pentry;

	pentry = proc_create("regulator_ctrl_mp8870_main", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_mp8870_main_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_mp8870_main = pentry;
	
	pentry = proc_create("regulator_ctrl_mp8870_preroll", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_mp8870_preroll_fops);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}
	proc_regulator.pproc_cmd_entry_mp8870_preroll = pentry;

	return 0;
}

static int nvt_regulator_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id nvt_regulator_match[] = {
	{ .compatible = "nvt,nvt_regulator" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_regulator_match);
#endif

static struct platform_driver nvt_regulator_driver = {
	.probe      = nvt_regulator_probe,
	.remove     = nvt_regulator_remove,
	.driver     = {
		.name   = "nvt_regulator",
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = nvt_regulator_match,
#endif
	},
};

static int __init nvt_regulator_init_driver(void)
{
	return platform_driver_register(&nvt_regulator_driver);
}
module_init(nvt_regulator_init_driver);

static void __exit nvt_regulator_exit_driver(void)
{
	platform_driver_unregister(&nvt_regulator_driver);
}
module_exit(nvt_regulator_exit_driver);


MODULE_DESCRIPTION("NVT Regulator Driver");
MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.01.002");
