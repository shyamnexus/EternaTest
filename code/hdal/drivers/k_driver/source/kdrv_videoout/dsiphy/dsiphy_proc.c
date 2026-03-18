#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include "dsiphy_proc.h"
#include "dsiphy_dbg.h"
#include "dsiphy_main.h"
#include "dsiphy_api.h"


//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 30
#define MAX_ARG_NUM     6

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PDSIPHY_MODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
PDSIPHY_DRV_INFO pdsiphy_drv_info_data;

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_read_list[] = {
	// keyword          function name
	{ "reg",            nvt_dsiphy_api_read_reg           },
};

#define NUM_OF_READ_CMD (sizeof(cmd_read_list) / sizeof(PROC_CMD))

static PROC_CMD cmd_write_list[] = {
	// keyword          function name
	{ "reg",            nvt_dsiphy_api_write_reg          }
//	{ "pattern",        nvt_dsiphy_api_write_pattern      }
};

#define NUM_OF_WRITE_CMD (sizeof(cmd_write_list) / sizeof(PROC_CMD))

static int nvt_dsiphy_proc_cmd_show(struct seq_file *sfile, void *v)
{
	nvt_dbg(IND, "\n");

	nvt_dbg(IND, "%s\n", __func__);

	return 0;
}

static int nvt_dsiphy_proc_cmd_open(struct inode *inode, struct file *file)
{
	nvt_dbg(IND, "\n");

	nvt_dbg(IND, "%s\n", __func__);

	return single_open(file, nvt_dsiphy_proc_cmd_show, &pdsiphy_drv_info_data->module_info);
}

static ssize_t nvt_dsiphy_proc_cmd_write(struct file *file, const char __user *buf,
									   size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;
	unsigned char loop;

	nvt_dbg(IND, "%s\n", __func__);

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	if (len == 0) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	nvt_dbg(IND, "CMD:%s\n", cmd_line);

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	if (strncmp(argv[0], "r", 2) == 0) {
		for (loop = 0 ; loop < NUM_OF_READ_CMD; loop++) {
			if (strncmp(argv[1], cmd_read_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_read_list[loop].execute(&pdsiphy_drv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}
		if (loop >= NUM_OF_READ_CMD) {
			goto ERR_INVALID_CMD;
		}

	} else if (strncmp(argv[0], "w", 2) == 0)  {

		for (loop = 0 ; loop < NUM_OF_WRITE_CMD ; loop++) {
			if (strncmp(argv[1], cmd_write_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
				ret = cmd_write_list[loop].execute(&pdsiphy_drv_info_data->module_info, ucargc - 2, &argv[2]);
				break;
			}
		}

		if (loop >= NUM_OF_WRITE_CMD) {
			goto ERR_INVALID_CMD;
		}

	} else {
		goto ERR_INVALID_CMD;
	}

	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD \"%s\"\n", cmd_line);

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    = nvt_dsiphy_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_dsiphy_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_dsiphy_proc_help_show(struct seq_file *sfile, void *v)
{
	int reg;

	nvt_dbg(IND, "%s\n", __func__);

	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, "DSIPHY REG (0x%08lX)::\n", (uintptr_t)pdsiphy_drv_info_data->module_info.io_addr[0]);
	for (reg = 0x0; reg < 0x80; reg += 16) {
		seq_printf(sfile, "0x%02X: 0x%08X 0x%08X 0x%08X 0x%08X\n", reg, nvt_dsiphy_drv_read_reg(&(pdsiphy_drv_info_data->module_info), reg)
			, nvt_dsiphy_drv_read_reg(&(pdsiphy_drv_info_data->module_info), reg+4), nvt_dsiphy_drv_read_reg(&(pdsiphy_drv_info_data->module_info), reg+8),
			nvt_dsiphy_drv_read_reg(&(pdsiphy_drv_info_data->module_info), reg+12));
	}

	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int nvt_dsiphy_proc_help_open(struct inode *inode, struct file *file)
{
	nvt_dbg(IND, "%s\n", __func__);

	return single_open(file, nvt_dsiphy_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open    = nvt_dsiphy_proc_help_open,
	.proc_release = single_release,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
};

int nvt_dsiphy_proc_init(PDSIPHY_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	nvt_dbg(IND, "%s\n", __func__);

	pmodule_root = proc_mkdir("dsiphy", NULL);
	if (pmodule_root == NULL) {
		nvt_dbg(ERR, "failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;


	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		nvt_dbg(ERR, "failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_help_entry = pentry;


	pdsiphy_drv_info_data = pdrv_info;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_dsiphy_proc_remove(PDSIPHY_DRV_INFO pdrv_info)
{
	nvt_dbg(IND, "%s\n", __func__);

	proc_remove(pdrv_info->pproc_help_entry);
	proc_remove(pdrv_info->pproc_cmd_entry);
	proc_remove(pdrv_info->pproc_module_root);
	return 0;
}
