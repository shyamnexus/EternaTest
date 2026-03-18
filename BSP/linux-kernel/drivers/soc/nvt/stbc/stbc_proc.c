#include <linux/random.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "stbc_proc.h"
#include "stbc_dbg.h"
#include "stbc_main.h"
#include <plat/hardware.h>
#include <plat/stbc.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/scatterlist.h>

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

#define DBG_TEST_EN (0)

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PMODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
PSTBC_DRV_INFO pdrv_stbc_info_data;

//============================================================================
// Function define
//============================================================================

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int nvt_stbc_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\nUsage\n");
	seq_printf(sfile, "\necho [command] > efuse_op\n\n");
	seq_printf(sfile, "[command]    =>\n");
	seq_printf(sfile, "             => trim (driver's trim data)\n");
	seq_printf(sfile, "             => uniqueid (chip's unique ID)\n");
	seq_printf(sfile, "             => keyset No (key transfer to secure engine)\n");
	seq_printf(sfile, "             => nvt_write_key No (write specific key into specific key set field)\n");
	seq_printf(sfile, "             => nvt_read_key No ( read specific key from specific key set field)\n");
	seq_printf(sfile, "             => nvt_read_key_lock No (Read lock specific key set)\n");
	seq_printf(sfile, "             => encrypt_key_set No (Encrypt/decrypt via specific key set)\n");
	seq_printf(sfile, "             => version (knlPkg.a version)\n");
	return 0;
}

static int nvt_stbc_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_stbc_proc_show, NULL);
}

static ssize_t nvt_stbc_proc_cmd_read(struct file *fp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int ret = 0;
	return ret;
}

static ssize_t nvt_stbc_proc_cmd_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
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

	if (ucargc < 1) {
		pr_err("NULL command error\n");
		goto ERR_OUT;
	}
	if ((!strcmp(argv[0], "stbc"))) {
		pr_info("stbc_pmc_start\n");
		if ((ucargc == 2) && (!strcmp(argv[1], "0"))) {
			pr_info("\n stbc -> normal operation\n");
			stbc_pmc_start(0);
		} else if ((ucargc == 2) && (!strcmp(argv[1], "1"))) {
			pr_info("\n stbc -> debug operation(not necessary usb wakeup)\n");
			stbc_pmc_start(1);
		} else {
			stbc_pmc_start(1);
		}
	} else {
		pr_info("\nUsage\n");
		pr_info("\necho [command] > stbc\n\n");
		pr_info("[command]    =>\n");
		pr_info("             => stbc\n");
		pr_info("             => version (stbc version)\n");
	}
ERR_OUT:

	return size;
}

static struct proc_ops proc_stbc_fops = {
	.proc_open   = nvt_stbc_proc_help_open,
	.proc_release = single_release,
	.proc_read   = nvt_stbc_proc_cmd_read,
	.proc_lseek = seq_lseek,
	.proc_write   = nvt_stbc_proc_cmd_write
};



int nvt_stbc_proc_init(PSTBC_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_stbc_op", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}

	pdrv_info->pproc_stbc_root = pmodule_root;


	pentry = proc_create("stbc", S_IRUGO | S_IXUGO, pmodule_root, &proc_stbc_fops);

	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc otp!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_stbc_entry = pentry;

	return ret;
remove_cmd:
	proc_remove(pdrv_info->pproc_stbc_entry);
	//proc_remove(pdrv_info->pproc_avl_entry);

remove_root:
	proc_remove(pdrv_info->pproc_stbc_root);
	return ret;
}

int nvt_stbc_proc_remove(PSTBC_DRV_INFO pdrv_info)
{
	return 0;
}
