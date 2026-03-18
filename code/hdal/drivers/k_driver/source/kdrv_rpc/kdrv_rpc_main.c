#include <linux/version.h>
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
#include "kdrv_rpc_int.h"
#include "kdrv_rpc_main.h"
#include "kdrv_rpc_debug.h"
#include "kdrv_rpc_proc.h"
#include "kdrv_rpc.h"
#ifdef CONFIG_PM
#include <linux/soc/nvt/nvt-info.h>
#endif


//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
unsigned int kdrv_rpc_debug_level = THIS_DBGLVL;
module_param_named(kdrv_rpc_debug_level, kdrv_rpc_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kdrv_rpc_debug_level, "kdrv_rpc debug level");

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id kdrv_rpc_match_table[] = {
	{	.compatible = "kdrv_rpc"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================

//=============================================================================
// function define
//=============================================================================
static int kdrv_rpc_open_p(struct inode *inode, struct file *file)
{
	return 0;
}

static int kdrv_rpc_release_p(struct inode *inode, struct file *file)
{
	return 0;
}



struct file_operations kdrv_rpc_fops = {
	.owner   = THIS_MODULE,
	.open    = kdrv_rpc_open_p,
	.release = kdrv_rpc_release_p,
	.llseek  = no_llseek,
};





static int kdrv_rpc_probe_p(struct platform_device *pdev)
{
	KDRV_RPC_DRV_INFO *p_drv_info;
	const struct of_device_id *match;
	int ret = 0;
	//unsigned char ucloop;

	//printk(KERN_INFO "%s:%s\r\n", __func__, pdev->name);
	match = of_match_device(kdrv_rpc_match_table, &pdev->dev);
	if (!match) {
		DBG_ERR("[%s] OF not found \r\n", MODULE_NAME);
		return -EINVAL;
	}
	p_drv_info = kzalloc(sizeof(KDRV_RPC_DRV_INFO), GFP_KERNEL);
	if (!p_drv_info) {
		DBG_ERR("[%s]failed to allocate memory\r\n", MODULE_NAME);
		return -ENOMEM;
	}
	#if 0
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		p_drv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
		if (p_drv_info->presource[ucloop] == NULL) {
			DBG_ERR("No IO memory resource defined:%d\n", ucloop);
		ret = -ENODEV;
		goto FAIL_FREE_BUF;
		}
	}
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		nvt_dbg(IND, "%d. resource:0x%lx size:0x%lx\n", ucloop, (ULONG)p_drv_info->presource[ucloop]->start, (ULONG)resource_size(p_drv_info->presource[ucloop]));
		if (!request_mem_region(p_drv_info->presource[ucloop]->start, resource_size(p_drv_info->presource[ucloop]), pdev->name)) {
			DBG_ERR("failed to request memory resource%d\n", ucloop);
			/*
			for (; ucloop > 0 ;) {
				ucloop -= 1;
				release_mem_region(p_drv_info->presource[ucloop]->start, resource_size(p_drv_info->presource[ucloop]));
			}
			*/
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		p_drv_info->module_info.io_addr[ucloop] = ioremap_nocache(p_drv_info->presource[ucloop]->start, resource_size(p_drv_info->presource[ucloop]));
		if (p_drv_info->module_info.io_addr[ucloop] == NULL) {
			DBG_ERR("ioremap() failed in module%d\n", ucloop);
			/*
			for (; ucloop > 0 ;) {
				ucloop -= 1;
			    iounmap(p_drv_info->module_info.io_addr[ucloop]);
			}
			*/
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}
	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		p_drv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		DBG_IND("IRQ %d. ID%d\n", ucloop, p_drv_info->module_info.iinterrupt_id[ucloop]);
		if (p_drv_info->module_info.iinterrupt_id[ucloop] < 0) {
			DBG_ERR("No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}
	#endif
	//Dynamic to allocate Device ID
	if (alloc_chrdev_region(&p_drv_info->dev_id, MODULE_MINOR_ID, MODULE_MINOR_COUNT, MODULE_NAME)) {
		DBG_ERR("Can't get device ID\n");
		ret = -ENODEV;
		//goto FAIL_FREE_REMAP;
		goto FAIL_FREE_BUF;
	}

	DBG_IND("DevID Major:%d minor:%d\n" \
			, MAJOR(p_drv_info->dev_id), MINOR(p_drv_info->dev_id));
	/* Register character device for the volume */
	cdev_init(&p_drv_info->cdev, &kdrv_rpc_fops);
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
	p_drv_info->pdevice = device_create(p_drv_info->pmodule_class, NULL, MKDEV(MAJOR(p_drv_info->dev_id), 0), NULL, MODULE_NAME);
	if (IS_ERR(p_drv_info->pdevice)) {
		DBG_ERR("[%s]creating device fail.\r\n", MODULE_NAME);
		ret = -ENODEV;
		goto FAIL_CLASS;
	}
	// proc init
	if (kdrv_rpc_proc_init(p_drv_info) < 0) {
		ret = -ENODEV;
		goto FAIL_DEV;

	}
	platform_set_drvdata(pdev, p_drv_info);
	if (kdrv_rpc_drv_init(&p_drv_info->module_info) < 0) {
		ret = -ENODEV;
		goto FAIL_DEV;

	}
	return ret;

FAIL_DEV:
	device_unregister(p_drv_info->pdevice);
FAIL_CLASS:
	class_destroy(p_drv_info->pmodule_class);
FAIL_CDEV:
	cdev_del(&p_drv_info->cdev);
	unregister_chrdev_region(p_drv_info->dev_id, 1);
#if 0
FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(p_drv_info->module_info.io_addr[ucloop]);
FAIL_FREE_RES:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(p_drv_info->presource[ucloop]->start, resource_size(p_drv_info->presource[ucloop]));
#endif
FAIL_FREE_BUF:
	kfree(p_drv_info);
	p_drv_info = NULL;
	platform_set_drvdata(pdev, p_drv_info);
	DBG_ERR("probe fail\r\n");
	return ret;
}

static int kdrv_rpc_remove_p(struct platform_device *pdev)
{
	KDRV_RPC_DRV_INFO *p_drv_info;
//	unsigned char ucloop;

	printk("%s\n", __func__);
	p_drv_info = platform_get_drvdata(pdev);
	kdrv_rpc_drv_exit(&p_drv_info->module_info);
	kdrv_rpc_proc_exit(p_drv_info);
	device_unregister(p_drv_info->pdevice);
	class_destroy(p_drv_info->pmodule_class);
	cdev_del(&p_drv_info->cdev);
	unregister_chrdev_region(p_drv_info->dev_id, 1);
#if 0
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(p_drv_info->module_info.io_addr[ucloop]);
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(p_drv_info->presource[ucloop]->start, resource_size(p_drv_info->presource[ucloop]));
#endif
	kfree(p_drv_info);
	p_drv_info = NULL;
	platform_set_drvdata(pdev, p_drv_info);
	return 0;
}

#ifdef CONFIG_PM
static int kdrv_rpc_module_suspend(struct device *dev)
{
	// check power down mode
	if (nvt_get_suspend_mode()) { /* 0: not power off, 1: power off */
		kdrv_rpc_drv_suspend();
	}
	return 0;
}

static int kdrv_rpc_module_resume(struct device *dev)
{
	// check power down mode
	if (nvt_get_suspend_mode()) {
		kdrv_rpc_drv_resume();
	}
	return 0;
}

static const struct dev_pm_ops kdrv_rpc_pm_ops = {
	.suspend = kdrv_rpc_module_suspend,
	.resume = kdrv_rpc_module_resume,
};
#endif

//=============================================================================
// platform device
//=============================================================================

static struct platform_driver kdrv_rpc_driver = {
	.driver = {
				.name   = MODULE_NAME,
				.owner = THIS_MODULE,
				.of_match_table = kdrv_rpc_match_table,
#ifdef CONFIG_PM
				.pm         = &kdrv_rpc_pm_ops,
#endif
		      },
	.probe      = kdrv_rpc_probe_p,
	.remove     = kdrv_rpc_remove_p,
};


int __init kdrv_rpc_module_init(void)
{
	int ret;

	//printk(KERN_ERR "init:\n");
	ret = platform_driver_register(&kdrv_rpc_driver);
	if (ret) {
		DBG_ERR(" platform_driver_register failed!\n");
		return -1;
	}
	return 0;
}

void __exit kdrv_rpc_module_exit(void)
{
	platform_driver_unregister(&kdrv_rpc_driver);
	printk(KERN_ERR "exit:\n");
}

module_init(kdrv_rpc_module_init);
module_exit(kdrv_rpc_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("kdrv_rpc driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00.001");