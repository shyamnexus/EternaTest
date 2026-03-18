#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "ctl_sen_proc.h"
#include "ctl_sen_dbg.h"
#include "ctl_sen_main.h"
#include "ctl_sen_api.h"
#include "ctl_sen_int.h"
#include "ctl_sen_debug_infor_int.h"

static struct seq_file *g_seq_file;

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH 100
#define MAX_ARG_NUM     30

//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(PCTL_SEN_MODULE_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static PCTL_SEN_DRV_INFO pdrv_info_data;
static int nvt_ctl_sen_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_ctl_sen_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_cmd_show, &pdrv_info_data->module_info);
}

static ssize_t nvt_ctl_sen_proc_cmd(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		ctl_sen_dbg_err("Command length is too long!\n");
		goto ERR_OUT;
	} else if (len < 1) {
		ctl_sen_dbg_err("Command length is too short!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	ctl_sen_dbg_ind("CMD:%s\n", cmd_line);

	argv[0] = CTL_SEN_PROC_NAME;
	// parse command string
	for (ucargc = 1; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	if (ctl_sen_cmd_execute(ucargc, &argv[0]) == 0) {
		return size;
	}

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	.proc_open    	= nvt_ctl_sen_proc_cmd_open,
	.proc_read    	= seq_read,
	.proc_lseek  	= seq_lseek,
	.proc_release 	= single_release,
	.proc_write   	= nvt_ctl_sen_proc_cmd
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_ctl_sen_proc_help_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_ctl_sen_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	.proc_open   	= nvt_ctl_sen_proc_help_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
};

//=============================================================================
// proc "info" file operation functions
//=============================================================================

static int ctl_sen_seq_printf(const char *fmtstr, ...)
{
	char    buf[512];
	int     len;

	va_list marker;

	/* Initialize variable arguments. */
	va_start(marker, fmtstr);

	len = vsnprintf(buf, sizeof(buf), fmtstr, marker);
	va_end(marker);
	seq_printf(g_seq_file, buf);
	return 0;
}

static int nvt_ctl_sen_proc_info_show(struct seq_file *sfile, void *v)
{
	vos_sem_wait(ctl_sen_sem_proc);
	g_seq_file = sfile;
	ctl_sen_dbg_dump_info(ctl_sen_seq_printf);
	ctl_sen_dbg_dump_process(ctl_sen_seq_printf);
	vos_sem_sig(ctl_sen_sem_proc);

	return 0;
}

static int nvt_ctl_sen_proc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_info_show, NULL);
}

static struct proc_ops proc_info_fops = {
	.proc_open   	= nvt_ctl_sen_proc_info_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
};


//=============================================================================
// proc "signal" file operation functions
//=============================================================================
static int nvt_ctl_sen_proc_signal_show(struct seq_file *sfile, void *v)
{
	UINT32 i;
	CTL_SEN_CHGMODE_INFO chgmode_info;
	INT32 rt;

	vos_sem_wait(ctl_sen_sem_proc);
	g_seq_file = sfile;
	for (i = CTL_SEN_MIN_SEN_ID; i <= CTL_SEN_MAX_SEN_ID; i++) {
		if (ctl_sen_status(i) & CTL_SEN_STATUS_OPEN) {
			rt = ctl_sen_senid_info(i, NULL, NULL, &chgmode_info);
			if ((rt == CTL_SEN_E_OK) && (chgmode_info.output_dest != CTL_SEN_MAP_NULL)) {
				if (chgmode_info.output_dest >= ctl_sen_output_vie_base) {
					ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_VD_TO_VIE0_CH0 + (chgmode_info.output_dest - ctl_sen_output_vie_base));
					ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_FMD_TO_VIE0_CH0 + (chgmode_info.output_dest - ctl_sen_output_vie_base));
				} else {
					ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_VD_TO_SIE0 + chgmode_info.output_dest);
					ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_FMD_TO_SIE0 + chgmode_info.output_dest);
				}
			} else {
				ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_VD_TO_SIE0);
				ctl_sen_dbg_dump_signal(ctl_sen_seq_printf, i, CTL_SEN_INTE_FMD_TO_SIE0);
			}
		}
	}
	vos_sem_sig(ctl_sen_sem_proc);

	return 0;
}

static int nvt_ctl_sen_proc_signal_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_signal_show, NULL);
}

static struct proc_ops proc_signal_fops = {
	.proc_open   	= nvt_ctl_sen_proc_signal_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_lseek 	= seq_lseek,
};

//=============================================================================
// proc "dbglevel" file operation functions
//=============================================================================
static int nvt_ctl_sen_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " 0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\n");
	seq_printf(sfile, " kflow_sen dbglevel = %u\n", ctl_sen_dbg_level);
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int nvt_ctl_sen_proc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ctl_sen_proc_dbglevel_show, NULL);
}

static ssize_t nvt_ctl_sen_proc_dbglevel_write(struct file *file, const char __user *buf,
											size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];

	// check command length
	if (len == 0 || len > (MAX_CMD_LENGTH - 1)) {
		ctl_sen_dbg_err("Command length is too long or 0!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len))
	    goto ERR_OUT;

	cmd_line[len - 1] = '\0';

	ctl_sen_dbg_ind("CMD:%s\n", cmd_line);

	sscanf(cmd_line, "%u", &ctl_sen_dbg_level);

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_dbglevel_fops = {
	.proc_open   	= nvt_ctl_sen_proc_dbglevel_open,
	.proc_release 	= single_release,
	.proc_read   	= seq_read,
	.proc_write		= nvt_ctl_sen_proc_dbglevel_write,
	.proc_lseek 	= seq_lseek,
};

//=============================================================================
// proc init
//=============================================================================
int nvt_ctl_sen_proc_init(PCTL_SEN_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("kflow_sen", NULL);
	if (pmodule_root == NULL) {
		ctl_sen_dbg_err("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_module_root = pmodule_root;

	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &proc_cmd_fops);
	if (pentry == NULL) {
		ctl_sen_dbg_err("failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pmodule_root, &proc_help_fops);
	if (pentry == NULL) {
		ctl_sen_dbg_err("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_help;
	}
	pdrv_info->pproc_help_entry = pentry;

	pentry = proc_create("info", S_IRUGO | S_IXUGO, pmodule_root, &proc_info_fops);
	if (pentry == NULL) {
		ctl_sen_dbg_err("failed to create proc info!\n");
		ret = -EINVAL;
		goto remove_info;
	}
	pdrv_info->pproc_info_entry = pentry;

	pentry = proc_create("signal", S_IRUGO | S_IXUGO, pmodule_root, &proc_signal_fops);
	if (pentry == NULL) {
		ctl_sen_dbg_err("failed to create proc signal!\n");
		ret = -EINVAL;
		goto remove_signal;
	}
	pdrv_info->pproc_signal_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, pmodule_root, &proc_dbglevel_fops);
	if (pentry == NULL) {
		ctl_sen_dbg_err("failed to create proc dbglevel!\n");
		ret = -EINVAL;
		goto remove_dbglevel;
	}
	pdrv_info->pproc_dbglevel_entry = pentry;

	pdrv_info_data = pdrv_info;

	return ret;


remove_dbglevel:
	proc_remove(pdrv_info->pproc_dbglevel_entry);
remove_signal:
	proc_remove(pdrv_info->pproc_signal_entry);
remove_info:
	proc_remove(pdrv_info->pproc_info_entry);
remove_help:
	proc_remove(pdrv_info->pproc_help_entry);
remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);
remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

int nvt_ctl_sen_proc_remove(PCTL_SEN_DRV_INFO pdrv_info)
{
	if (pdrv_info_data) {
		proc_remove(pdrv_info->pproc_dbglevel_entry);
		proc_remove(pdrv_info->pproc_signal_entry);
		proc_remove(pdrv_info->pproc_info_entry);
		proc_remove(pdrv_info->pproc_help_entry);
		proc_remove(pdrv_info->pproc_cmd_entry);
		proc_remove(pdrv_info->pproc_module_root);
	}
	return 0;
}
