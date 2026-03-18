/*
 * Novatek NA51068 Power IC driver.
 *
 * Copyright (C) 2019 Novatek MicroElectronics Corp.
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
#include <linux/pwm.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <plat/nvt_power.h>

#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6
#define NUM_PWM_CH      3

typedef struct proc_pwr {
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_cmd_entry;
} proc_pwr_t;
proc_pwr_t proc_pwr;

static struct proc_dir_entry *pmodule_root = NULL;
static int global_pwm_id = 0;
struct pwm_device *nvt_pwm_dev;

void nvt_pwr_control(int level)
{

	struct pwm_state nvt_pwm_state;

	if (level < 0 || level > 100) {
		pr_err("invalid level: %d\r\n", level);
		return;
	}
	/*reset*/
	nvt_pwm_state.period = 8400;

	nvt_pwm_state.duty_cycle = level * 84;
	nvt_pwm_state.polarity = PWM_POLARITY_NORMAL;

	nvt_pwm_state.enabled = 1;

	pwm_apply_state(nvt_pwm_dev, &nvt_pwm_state);
}
EXPORT_SYMBOL(nvt_pwr_control);

void nvt_pwr_disable(void)
{
	pwm_disable(nvt_pwm_dev);
}
EXPORT_SYMBOL(nvt_pwr_disable);

static int nvt_pwr_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_pwr_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_pwr_proc_cmd_show, NULL);
}

static ssize_t nvt_pwr_proc_cmd_write(struct file *file, const char __user *buf,
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

		nvt_pwm_dev = PDE_DATA(file_inode(file));
		nvt_pwr_control(config);

		return size;
	}
ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = nvt_pwr_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_pwr_proc_cmd_write
};

static int nvt_pwr_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *pentry = NULL;
	u32 prop;
	int pwm_id;
	char pentry_name[12];

	if (of_property_read_u32(pdev->dev.of_node, "pwm_id", &prop) == 0) {
		pwm_id = prop;
	} else {
#if IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NA51090)
		pwm_id = 0;
#elif IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NA51102)
		pwm_id = 9;
#elif IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NA51103)
		pwm_id = 1;
#elif IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02201)
		pwm_id = 12;
#elif IS_ENABLED(CONFIG_NVT_IVOT_PLAT_NS02302)
		pwm_id = 10;
#else
		pwm_id = global_pwm_id;
#endif
		dev_dbg(&pdev->dev, "get pwm_id failed, use default pwm_id(%d)\n", global_pwm_id);
	}

	/* Create proc dir */
	if (pmodule_root == NULL) {
		pmodule_root = proc_mkdir("power_ctrl", NULL);
		if (pmodule_root == NULL) {
			pr_err("failed to create power_ctrl root\n");
			return -EINVAL;
		}
	}

	proc_pwr.pproc_module_root = pmodule_root;

	/* Initial channel */
	nvt_pwm_dev = pwm_request(pwm_id, "nvt_pwm");
	if (!nvt_pwm_dev) {
		return PTR_ERR(nvt_pwm_dev);
	}

	/* Create proc node */
	sprintf(pentry_name, "pwr_ctrl%d", global_pwm_id);
	pentry = proc_create_data(pentry_name, S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops, nvt_pwm_dev);
	if (pentry == NULL) {
		pr_err("failed to create proc cmd!\n");
		return -EINVAL;
	}

	proc_pwr.pproc_cmd_entry = pentry;

	return 0;
}

static int nvt_pwr_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id nvt_pwr_match[] = {
	{ .compatible = "nvt,nvt_pwr" },
	{},
};

MODULE_DEVICE_TABLE(of, nvt_pwr_match);
#endif

static struct platform_driver nvt_pwr_driver = {
	.probe      = nvt_pwr_probe,
	.remove     = nvt_pwr_remove,
	.driver     = {
		.name   = "nvt_pwr",
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = nvt_pwr_match,
#endif
	},
};

static int __init nvt_pwr_init_driver(void)
{
	return platform_driver_register(&nvt_pwr_driver);
}
module_init(nvt_pwr_init_driver);

static void __exit nvt_pwr_exit_driver(void)
{
	platform_driver_unregister(&nvt_pwr_driver);
}
module_exit(nvt_pwr_exit_driver);


MODULE_DESCRIPTION("NVT Power IC Driver");
MODULE_AUTHOR("Novatek");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.02.003");
