#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include "kdrv_rpc_proc.h"
#include "kdrv_rpc_debug.h"
#include "kdrv_rpc_debug_cmd.h"


//static struct seq_file *g_seq_file;

//============================================================================
// Define
//============================================================================
#define MAX_CMD_LENGTH  64
#define MAX_ARG_NUM     20
//============================================================================
// Declaration
//============================================================================
typedef struct proc_cmd {
	char cmd[MAX_CMD_LENGTH];
	int (*execute)(KDRV_RPC_DRV_INFO *pdrv, unsigned char argc, char **argv);
} PROC_CMD, *PPROC_CMD;

//============================================================================
// Global variable
//============================================================================
static KDRV_RPC_DRV_INFO *pdrv_info_data;

//============================================================================
// Function define
//============================================================================

int kdrv_rpc_proc_cmd(KDRV_RPC_DRV_INFO *p_drv, unsigned char argc, char **argv)
{
	kdrv_rpc_cmd_execute(argc, argv);
	return 0;
}

//=============================================================================
// proc "Custom Command" file operation functions
//=============================================================================
static PROC_CMD cmd_list[] = {
	// keyword          function name
	{ "rpc",			kdrv_rpc_proc_cmd	},
};

#define NUM_OF_CMD (sizeof(cmd_list) / sizeof(PROC_CMD))


//=============================================================================
// proc "test1" file operation functions
//=============================================================================
static ssize_t kdrv_rpc_cmd_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	int len = size;
	int ret = -EINVAL;
	char cmd_line[MAX_CMD_LENGTH];
	char *cmdstr = cmd_line;
	const char delimiters[] = {' ', 0x0A, 0x0D, '\0'};
	unsigned char argc = 0;
	char *argv[MAX_ARG_NUM] = {0};
	unsigned char loop;

	// check command length
	if (len > (MAX_CMD_LENGTH - 1)) {
		nvt_dbg(ERR, "Command length is too long!\n");
		goto ERR_OUT;
	}
	if (len == 0) {
		nvt_dbg(ERR, "Command length is 0!\n");
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, len)) {
		goto ERR_OUT;
	}

	cmd_line[len - 1] = '\0';

	DBG_IND("CMD:%s\n", cmd_line);

	// parse command string
	for (argc = 0; argc < MAX_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);
		if (argv[argc] == NULL) {
			break;
		}
	}
	if (argc < 2) {
		goto ERR_INVALID_CMD;
	}

	// dispatch command handler
	for (loop = 0 ; loop < NUM_OF_CMD; loop++) {
		if (strncmp(argv[0], cmd_list[loop].cmd, MAX_CMD_LENGTH) == 0) {
			ret = cmd_list[loop].execute(pdrv_info_data, argc , &argv[0]);
			break;
		}
	}
	if (loop >= NUM_OF_CMD) {
		goto ERR_INVALID_CMD;
	}
	return size;

ERR_INVALID_CMD:
	nvt_dbg(ERR, "Invalid CMD !!\r\n");
ERR_OUT:
	return -1;
}

static int kdrv_rpc_cmd_proc_show(struct seq_file *sfile, void *v)
{
	return 0;
}

static int kdrv_rpc_cmd_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, kdrv_rpc_cmd_proc_show, PDE_DATA(inode));
}

// plug file-system
static const struct proc_ops kdrv_rpc_cmd_proc_fops = {
	.proc_open  = kdrv_rpc_cmd_proc_open,
	.proc_read  = seq_read,
	.proc_write   = kdrv_rpc_cmd_proc_write,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};


int kdrv_rpc_proc_init(PKDRV_RPC_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("rpc", NULL);
	if (pmodule_root == NULL) {
		DBG_ERR("failed to create Module root\r\n");
		ret = -EINVAL;
		return ret;
	}
	pdrv_info->pproc_module_root = pmodule_root;
	pentry = proc_create("cmd", S_IRUGO | S_IXUGO, pmodule_root, &kdrv_rpc_cmd_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create proc cmd!\r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_debug_entry = pentry;
	pdrv_info_data = pdrv_info;
	kdrv_rpc_cmd_init();
	return ret;

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

void kdrv_rpc_proc_exit(PKDRV_RPC_DRV_INFO pdrv_info)
{
	kdrv_rpc_cmd_exit();
	if (pdrv_info_data) {
		proc_remove(pdrv_info->pproc_debug_entry);
		proc_remove(pdrv_info->pproc_module_root);
		pdrv_info_data = NULL;
	}
}


