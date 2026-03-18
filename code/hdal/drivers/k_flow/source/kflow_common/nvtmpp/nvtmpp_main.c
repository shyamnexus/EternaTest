#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include "kwrap/dev.h"
#include <kwrap/cpu.h>
#include "nvtmpp_init.h"
#include "nvtmpp_int.h"
#include "nvtmpp_main.h"
#include "nvtmpp_blk.h"
#include "nvtmpp_pool.h"
#include "nvtmpp_debug.h"
#include "nvtmpp_proc.h"
#include "kdrv_builtin/nvtmpp_init.h"
#include "kdrv_builtin/kdrv_builtin.h"
#include <linux/soc/nvt/fmem.h>

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
static int ddr1_mpp_size = 0x2000000;
module_param(ddr1_mpp_size, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ddr1_mpp_size, "ddr1 mpp size ");
#ifdef DEBUG
unsigned int nvtmpp_debug_level = THIS_DBGLVL;
module_param_named(nvtmpp_debug_level, nvtmpp_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(nvtmpp_debug_level, "nvtmpp debug level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id nvtmpp_match_table[] = {
	{	.compatible = "nvt,nvtmpp"},
	{}
};

extern long nvtmpp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

//=============================================================================
// function define
//=============================================================================
static int nvtmpp_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int nvtmpp_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int nvtmpp_mmap(struct file *filep, struct vm_area_struct *vma)
{
	ULONG non_cache_flag;

	if (sizeof(ULONG) == 4) {
		non_cache_flag = NONCACHE_FLAG_32;
	} else {
		non_cache_flag = NONCACHE_FLAG_64;
	}
	DBG_IND("non_cache_flag = 0x%lx\r\n", non_cache_flag);
	if (vma->vm_end < vma->vm_start) {
		DBG_ERR("vm_start 0x%lx > vm_end 0x%lx", (ULONG)vma->vm_start, (ULONG)vma->vm_end);
		return -EINVAL;
	}
	DBG_IND("vm_start 0x%lx , vm_end 0x%lx, vm_pgoff 0x%lx\r\n", (ULONG)vma->vm_start, (ULONG)vma->vm_end, (ULONG)vma->vm_pgoff);
	vma->vm_flags |= VM_IO | VM_DONTEXPAND | VM_DONTDUMP;
	if ((vma->vm_pgoff << PAGE_SHIFT) & non_cache_flag) {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		vma->vm_pgoff -= (non_cache_flag >> PAGE_SHIFT);
		DBG_IND("map write combine, vm_pgoff 0x%lx\r\n", (ULONG)vma->vm_pgoff);
	} else {
		DBG_IND("map cache\r\n");
	}
	return remap_pfn_range(vma,
						   vma->vm_start,
						   vma->vm_pgoff,
						   vma->vm_end - vma->vm_start,
						   vma->vm_page_prot);
}

struct file_operations nvtmpp_fops = {
	.owner   = THIS_MODULE,
	.open    = nvtmpp_open,
	.release = nvtmpp_release,
	.mmap    = nvtmpp_mmap,
	.unlocked_ioctl = nvtmpp_ioctl,
	.llseek  = no_llseek,
};

static int nvtmpp_probe(struct platform_device *pdev)
{
	NVTMPP_DRV_INFO *p_drv_info;
	const struct of_device_id *match;
	int ret = 0;

	DBG_IND("%s:%s\r\n", __func__, pdev->name);
	match = of_match_device(nvtmpp_match_table, &pdev->dev);
	if (!match) {
		DBG_ERR("[%s] OF not found \r\n", MODULE_NAME);
		return -EINVAL;
	}
	p_drv_info = kzalloc(sizeof(NVTMPP_DRV_INFO), GFP_KERNEL);
	if (!p_drv_info) {
		DBG_ERR("[%s]failed to allocate memory\r\n", MODULE_NAME);
		return -ENOMEM;
	}
	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&p_drv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	DBG_IND("DevID Major:%d minor:%d\n" \
			, MAJOR(p_drv_info->dev_id), MINOR(p_drv_info->dev_id));
	/* Register character device for the volume */
	cdev_init(&p_drv_info->cdev, &nvtmpp_fops);
	p_drv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&p_drv_info->cdev, p_drv_info->dev_id, 1)) {
		DBG_ERR("[%s]can't add cdev\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CDEV;
	}
	// create class
	p_drv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(p_drv_info->pmodule_class)) {
		DBG_ERR("[%s]failed in creating class.\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CDEV;
	}
	// register
	p_drv_info->pdevice = device_create(p_drv_info->pmodule_class, NULL, MKDEV(MAJOR(p_drv_info->dev_id), MINOR(p_drv_info->dev_id)), NULL, MODULE_NAME);
	if (IS_ERR(p_drv_info->pdevice)) {
		DBG_ERR("[%s]creating device fail.\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CLASS;
	}
	platform_set_drvdata(pdev, p_drv_info);

	nvtmpp_proc_init(p_drv_info);
	if (nvtmpp_init() < 0)
		goto FAIL_DEV;
	return ret;
FAIL_DEV:
	device_unregister(p_drv_info->pdevice);
FAIL_CLASS:
	class_destroy(p_drv_info->pmodule_class);
FAIL_CDEV:
	cdev_del(&p_drv_info->cdev);
	vos_unregister_chrdev_region(p_drv_info->dev_id, 1);
FAIL_FREE_REMAP:
	kfree(p_drv_info);
	p_drv_info = NULL;
	platform_set_drvdata(pdev, p_drv_info);
	DBG_ERR("probe fail\r\n");
	return ret;
}

static int nvtmpp_remove(struct platform_device *pdev)
{
	NVTMPP_DRV_INFO *p_drv_info;

	printk("%s\n", __func__);

	nvtmpp_sys_exit();
	nvtmpp_uninstall_id();
	p_drv_info = platform_get_drvdata(pdev);
	nvtmpp_proc_exit(p_drv_info);
	device_unregister(p_drv_info->pdevice);
	class_destroy(p_drv_info->pmodule_class);
	cdev_del(&p_drv_info->cdev);
	vos_unregister_chrdev_region(p_drv_info->dev_id, 1);
	//nvtmpp_exit_mmz(p_drv_info);
	if (!kdrv_builtin_is_fastboot()) {
		nvtmpp_exit_mmz();
	}
	kfree(p_drv_info);
	p_drv_info = NULL;
	platform_set_drvdata(pdev, p_drv_info);
	return 0;
}

//=============================================================================
// platform device
//=============================================================================

static struct platform_driver nvtmpp_driver = {
	.driver = {
				.name   = MODULE_NAME,
				.owner = THIS_MODULE,
				.of_match_table = nvtmpp_match_table,
		      },
	.probe      = nvtmpp_probe,
	.remove     = nvtmpp_remove,
};

#if defined(_GROUP_KO_)
#undef __init
#undef __exit
#undef module_init
#undef module_exit
#define __init
#define __exit
#define module_init(x)
#define module_exit(x)
#ifndef MODULE // in built-in case
#undef MODULE_VERSION
#undef VOS_MODULE_VERSION
#define MODULE_VERSION(x)
#define VOS_MODULE_VERSION(a1, a2, a3, a4, a5)
#endif
#endif

int __init nvtmpp_module_init(void)
{
	int ret;

	ret = platform_driver_register(&nvtmpp_driver);
	if (ret) {
		DBG_ERR(" platform_driver_register failed!\n");
		return -1;
	}
	return 0;
}

void __exit nvtmpp_module_exit(void)
{
	platform_driver_unregister(&nvtmpp_driver);
	printk(KERN_ERR "exit:\n");
}

module_init(nvtmpp_module_init);
module_exit(nvtmpp_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvtmpp driver");
MODULE_LICENSE("GPL");
