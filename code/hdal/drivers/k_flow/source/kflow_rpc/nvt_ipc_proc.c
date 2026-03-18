#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include "nvt_ipc_proc.h"
#include "nvt_ipc_debug.h"


//============================================================================
// Global variable
//============================================================================
static NVT_IPC_DRV_INFO *pdrv_info_data;

//============================================================================
// Function define
//============================================================================

//=============================================================================
// proc "info" file operation functions
//=============================================================================
static int nvt_ipc_info_proc_show(struct seq_file *sfile, void *v)
{
	nvt_ipc_drv_dump_debug();
	return 0;
}

static int nvt_ipc_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_ipc_info_proc_show, NULL);
}

static struct proc_ops nvt_ipc_info_proc_fops = {
	.proc_open   = nvt_ipc_info_proc_open,
	.proc_release = single_release,
	.proc_read   = seq_read,
	.proc_lseek = seq_lseek,
};


int nvt_ipc_proc_init(PNVT_IPC_DRV_INFO pdrv_info)
{
	int ret = 0;
	struct proc_dir_entry *pmodule_root = NULL;
	struct proc_dir_entry *pentry = NULL;

	pmodule_root = proc_mkdir("nvt_ipc", NULL);
	if (pmodule_root == NULL) {
		DBG_ERR("failed to create Module root\r\n");
		ret = -EINVAL;
		return ret;
	}
	pdrv_info->pproc_module_root = pmodule_root;
	pentry = proc_create("info", S_IRUGO | S_IXUGO, pmodule_root, &nvt_ipc_info_proc_fops);
	if (pentry == NULL) {
		DBG_ERR("failed to create info cmd!\r\n");
		ret = -EINVAL;
		goto remove_root;
	}
	pdrv_info->pproc_info_entry = pentry;
	pdrv_info_data = pdrv_info;
	return ret;

remove_root:
	proc_remove(pdrv_info->pproc_module_root);
	return ret;
}

void nvt_ipc_proc_exit(PNVT_IPC_DRV_INFO pdrv_info)
{
	if (pdrv_info_data) {
		proc_remove(pdrv_info->pproc_info_entry);
		proc_remove(pdrv_info->pproc_module_root);
		pdrv_info_data = NULL;
	}
}


