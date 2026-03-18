#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "kdrv_ipp_int_dbg.h"
#include "kdrv_ipp_int_proc.h"
#include "kdrv_ipp_int_main.h"
#include "kdrv_ipp_int_api.h"
#include "kdrv_ipp_int.h"

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
	int (*execute)(PKDRV_IPP_DRV_INFO pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static PKDRV_IPP_DRV_INFO pdrv_info_data;

//============================================================================
// Function define
//============================================================================


//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static int nvt_kdrv_ipp_proc_cmd_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int nvt_kdrv_ipp_proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_kdrv_ipp_proc_cmd_show, NULL);
}

static ssize_t nvt_kdrv_ipp_proc_cmd_write(struct file *file, const char __user *buf,
									  size_t size, loff_t *off)
{
	int len = size;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char ucargc = 0;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		DBG_ERR("Command length is too long!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len) != 0) {
		goto ERR_OUT;
	}

	if (len > 0) {
		cmd_line[len - 1] = '\0';
	}

	// parse command string
	for (ucargc = 0; ucargc < MAX_ARG_NUM; ucargc++) {
		argv[ucargc] = strsep(&cmdstr, delimiters);

		if (argv[ucargc] == NULL) {
			break;
		}
	}

	// dispatch command handler
	if (strncmp(argv[0], "r", 2) == 0) {
		kdrv_ipp_cmd_execute(ucargc - 1, &argv[1]);
	} else if (strncmp(argv[0], "w", 2) == 0)  {
		kdrv_ipp_cmd_execute(ucargc - 1, &argv[1]);
	} else {
		kdrv_ipp_cmd_execute(ucargc, &argv[0]);
	}

	return size;

ERR_OUT:
	return -1;
}

static struct proc_ops proc_cmd_fops = {
	//.owner   = THIS_MODULE,
	.proc_open    = nvt_kdrv_ipp_proc_cmd_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
	.proc_write   = nvt_kdrv_ipp_proc_cmd_write
};

//=============================================================================
// proc "help" file operation functions
//=============================================================================
static int nvt_kdrv_ipp_proc_help_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "=====================================================================\n");
	seq_printf(sfile, " Add message here\n");
	seq_printf(sfile, "=====================================================================\n");
	return 0;
}

static int nvt_kdrv_ipp_proc_help_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_kdrv_ipp_proc_help_show, NULL);
}

static struct proc_ops proc_help_fops = {
	//.owner  = THIS_MODULE,
	.proc_open   = nvt_kdrv_ipp_proc_help_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

//util info
static int nvt_kdrv_ipp_proc_util_show(struct seq_file *sfile, void *v)
{
	kdrv_ipp_util_dump(sfile);
	return 0;
}

static int nvt_kdrv_ipp_proc_util_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_kdrv_ipp_proc_util_show, NULL);
}

static struct proc_ops proc_util_fops = {
	//.owner  = THIS_MODULE,
	.proc_open   = nvt_kdrv_ipp_proc_util_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

//dbglevel
static int nvt_kdrv_ipp_proc_dbglevel_show(struct seq_file *sfile, void *v)
{
	DBG_DUMP("0: fatal, 1: err, 2: wrn, 3: msg, 4: ind, 5: func\r\n");
	DBG_DUMP("kdrv_vprc dbglevel = %d\r\n", kdrv_ipp_debug_level);
	return 0;
}

static int nvt_kdrv_ipp_proc_dbglevel_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_kdrv_ipp_proc_dbglevel_show, NULL);
}

static struct proc_ops proc_dbglevel_fops = {
	//.owner  = THIS_MODULE,
	.proc_open   = nvt_kdrv_ipp_proc_dbglevel_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

//gating_en
static int nvt_kdrv_ipp_proc_gating_en_show(struct seq_file *sfile, void *v)
{
	DBG_DUMP("0: disable, 1: enable\r\n");
	DBG_DUMP("gating_en = 1\r\n");
	return 0;
}

static int nvt_kdrv_ipp_proc_gating_en_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_kdrv_ipp_proc_gating_en_show, NULL);
}

static struct proc_ops proc_gating_en_fops = {
	//.owner  = THIS_MODULE,
	.proc_open   = nvt_kdrv_ipp_proc_gating_en_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};

int nvt_kdrv_ipp_proc_init(PKDRV_IPP_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pise_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pise_root = proc_mkdir("kdrv_ipp", NULL);
	if (pise_root == NULL) {
		DBG_ERR("failed to create Module root\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_kdrv_ipp_root = pise_root;


	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pise_root, &proc_cmd_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc cmd!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_cmd_entry = pentry;

	pentry = proc_create("help", S_IRUGO | S_IXUGO, pise_root, &proc_help_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc help!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_help_entry = pentry;

	pentry = proc_create("utilization", S_IRUGO | S_IXUGO, pise_root, &proc_util_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc util!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_util_entry = pentry;

	pentry = proc_create("dbglevel", S_IRUGO | S_IXUGO, pise_root, &proc_dbglevel_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc dbglevel!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_dbglevel_entry = pentry;

	pentry = proc_create("gating_en", S_IRUGO | S_IXUGO, pise_root, &proc_gating_en_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc gating_en!\n");
		ret = -EINVAL;
		goto remove_cmd;
	}
	pdrv_info->pproc_gating_en_entry = pentry;


	pdrv_info_data = pdrv_info;

	return ret;

remove_cmd:
	proc_remove(pdrv_info->pproc_cmd_entry);

remove_root:
	proc_remove(pdrv_info->pproc_kdrv_ipp_root);
	return ret;
}

int nvt_kdrv_ipp_proc_remove(PKDRV_IPP_DRV_INFO pdrv_info)
{
	proc_remove(pdrv_info->pproc_help_entry);
	proc_remove(pdrv_info->pproc_cmd_entry);
	proc_remove(pdrv_info->pproc_kdrv_ipp_root);
	return 0;
}

void kdrv_ipp_proc_seq_printf(void *sfile, const char *fmt, ...)
{
#define MAX_MSG_SIZE 256
	va_list args;
	char msg[MAX_MSG_SIZE];

	if (sfile == 0) {
		va_start(args, fmt);
		vsnprintf(msg, MAX_MSG_SIZE, fmt, args);
		va_end(args);
		nvt_ipp_dbg_log("%s", &msg[0]);
	} else {
		va_start(args, fmt);
		seq_vprintf(sfile, fmt, args);
		va_end(args);
	}
}

