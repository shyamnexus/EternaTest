#if defined(__LINUX)
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#endif

#include <csi_tx/csi_tx_internal.h>
#include <csi_tx/csi_tx_open.h>

///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          csi_tx_open
#define __DBGLVL__          NVT_DBG_WRN
#include "kwrap/debug.h"
unsigned int csi_tx_open_debug_level = __DBGLVL__;
///////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX)

void csi_tx_memset(void *buf, unsigned char val, int len)
{
	memset(buf, val, len);
}

void csi_tx_memcpy(void *buf, void *src, int len)
{
	memcpy(buf, src, len);
}

int csi_tx_copy_from_user(void *dst, void *src, int len)
{
	return copy_from_user(dst, (void __user *)src, len);
}

int csi_tx_copy_to_user(void *dst, void *src, int len)
{
	return copy_to_user((void __user *)dst, src, len);
}

static int proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "nothing!\n");
	return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, proc_show, NULL);
}

static long proc_ioctl(struct file* f, unsigned int cmd, unsigned long arg)
{
	return nvt_csi_tx_ioctl(-1, cmd, (void*)arg);
}

static struct proc_dir_entry *proc_file_entry = NULL;
static const struct proc_ops proc_file_fops = {
	.proc_open    = proc_open,
	.proc_ioctl   = proc_ioctl,
	.proc_release = single_release,
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_csi_tx_probe(struct platform_device *pdev);
static int nvt_csi_tx_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_csi_tx_resume(struct platform_device *pdev);
static int nvt_csi_tx_remove(struct platform_device *pdev);
int __init nvt_csi_tx_module_init(void);
void __exit nvt_csi_tx_module_exit(void);

//=============================================================================
// function define
//=============================================================================

static int nvt_csi_tx_probe(struct platform_device *pdev)
{
	return 0;
}

static int nvt_csi_tx_remove(struct platform_device *pdev)
{
	return 0;
}

static int nvt_csi_tx_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int nvt_csi_tx_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver nvt_csi_tx_driver = {
	.driver = {
		.name   = "kflow_csi_tx",
		.owner  = THIS_MODULE,
	},
	.probe      = nvt_csi_tx_probe,
	.remove     = nvt_csi_tx_remove,
	.suspend    = nvt_csi_tx_suspend,
	.resume     = nvt_csi_tx_resume
};

int __init nvt_csi_tx_module_init(void)
{
	int                      ret;

	if(nvt_csi_tx_init()){
		DBG_ERR("nvt_gfx_init() fail\n");
		return -1;
	}

	ret = platform_driver_register(&nvt_csi_tx_driver);

	proc_file_entry = proc_create("nvt_csi_tx", 0, NULL, &proc_file_fops);
	if(proc_file_entry == NULL){
		DBG_ERR("fail to create /proc/nvt_csi_tx\n");
		return -ENOMEM;
	}

	return 0;
}

void __exit nvt_csi_tx_module_exit(void)
{
	nvt_csi_tx_exit();

	if(proc_file_entry){
		proc_file_entry = NULL;
		remove_proc_entry("nvt_csi_tx", NULL);
	}

	platform_driver_unregister(&nvt_csi_tx_driver);
}

module_init(nvt_csi_tx_module_init);
module_exit(nvt_csi_tx_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("csi_tx driver");
MODULE_VERSION("1.06.000");
MODULE_LICENSE("GPL");
#endif
