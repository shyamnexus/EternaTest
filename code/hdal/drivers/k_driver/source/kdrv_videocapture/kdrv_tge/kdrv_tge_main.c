#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/kdev_t.h>
#include <linux/clk.h>
#include <asm/signal.h>
#include <kwrap/dev.h>
#include <linux/proc_fs.h>
#include "kdrv_tge_int_drv.h"
#include "kdrv_tge_int_main.h"
#include "kdrv_tge_int_proc.h"
#include "kdrv_tge_int_dbg.h"
#include "kdrv_tge.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#if 0
unsigned int kdrv_tge_debug_level = (NVT_DBG_IND | NVT_DBG_WRN | NVT_DBG_ERR);
module_param_named(kdrv_tge_debug_level, kdrv_tge_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(kdrv_tge_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id tge_match_table[] = {
	{	.compatible = "nvt,kdrv_tge"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_tge_open(struct inode *inode, struct file *file);
static int nvt_tge_release(struct inode *inode, struct file *file);
static long nvt_tge_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_tge_probe(struct platform_device * pdev);
static int nvt_tge_suspend( struct platform_device * pdev, pm_message_t state);
static int nvt_tge_resume( struct platform_device * pdev);
static int nvt_tge_remove(struct platform_device *pdev);
int __init nvt_tge_module_init(void);
void __exit nvt_tge_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_tge_open(struct inode *inode, struct file *file)
{
	TGE_DRV_INFO* pdrv_info;

	pdrv_info = container_of(inode->i_cdev, TGE_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, TGE_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_tge_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

    return 0;
}

static int nvt_tge_release(struct inode *inode, struct file *file)
{
	TGE_DRV_INFO* pdrv_info;
	pdrv_info = container_of(inode->i_cdev, TGE_DRV_INFO, cdev);

	nvt_tge_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
    return 0;
}

static long nvt_tge_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PTGE_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_tge_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

static struct file_operations nvt_tge_fops = {
    .owner   = THIS_MODULE,
    .open    = nvt_tge_open,
    .release = nvt_tge_release,
    .unlocked_ioctl = nvt_tge_ioctl,
    .llseek  = no_llseek,
};

static int nvt_tge_probe(struct platform_device * pdev)
{
    TGE_DRV_INFO* pdrv_info;
	const struct of_device_id *match;
    int ret = 0;
	unsigned char ucloop;

	match = of_match_device(tge_match_table, &pdev->dev);
	if (!match){
        nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

    pdrv_info = kzalloc(sizeof(TGE_DRV_INFO), GFP_KERNEL);
    if (!pdrv_info) {
        nvt_dbg(ERR, "failed to allocate memory\n");
        return -ENOMEM;
    }

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->presource[ucloop] = platform_get_resource(pdev, IORESOURCE_MEM, ucloop);
		if (pdrv_info->presource[ucloop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d\n", ucloop);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		nvt_dbg(IND, "%d. resource:0x%lx size:0x%lx\n", ucloop, (ULONG)pdrv_info->presource[ucloop]->start, (ULONG)resource_size(pdrv_info->presource[ucloop]));
		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", ucloop);
#if (MODULE_REG_NUM > 1)
			for (; ucloop > 0 ;) {
				ucloop -= 1;
				release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
#if (MODULE_REG_NUM > 1)
			for (; ucloop > 0 ;) {
				ucloop -= 1;
	    		iounmap(pdrv_info->module_info.io_addr[ucloop]);
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		nvt_dbg(IND, "IRQ %d. ID%d\n", ucloop,pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}

	//Get clock source
	pdrv_info->module_info.tge_clk = clk_get(&pdev->dev, "2f0330000.tge");
	if (IS_ERR(pdrv_info->module_info.tge_clk)) {
		nvt_dbg(ERR, "fail to get tge clock\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	pdrv_info->module_info.flash_clk = clk_get(&pdev->dev, "clk_tgeflashA");
	if (IS_ERR(pdrv_info->module_info.flash_clk)) {
		nvt_dbg(ERR, "fail to get tge flash clock\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	pdrv_info->module_info.mshA_clk = clk_get(&pdev->dev, "clk_tgemshA");
	if (IS_ERR(pdrv_info->module_info.mshA_clk)) {
		nvt_dbg(ERR, "fail to get tge MshA clock\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	pdrv_info->module_info.pclk = clk_get(&pdev->dev, "tge_pclk");
	if (IS_ERR(pdrv_info->module_info.pclk)) {
		nvt_dbg(ERR, "fail to get tge pclk\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	nvt_dbg(IND, "DevID Major:%d minor:%d\n" \
		        , MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_tge_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

    pdrv_info->pmodule_class = class_create(THIS_MODULE, "kdrv_tge");
	if(IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
			, MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
			, MODULE_NAME"%d", ucloop);

		if(IS_ERR(pdrv_info->pdevice[ucloop])) {
			nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

	ret = nvt_tge_proc_init(pdrv_info);
	if(ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}

	ret = nvt_tge_drv_init(&pdrv_info->module_info);

    platform_set_drvdata(pdev, pdrv_info);
	if(ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;

FAIL_DRV_INIT:
	nvt_tge_proc_remove(pdrv_info);

FAIL_DEV:
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT ) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);

FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

FAIL_FREE_RES:
	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

FAIL_FREE_BUF:
	kfree(pdrv_info);
    return ret;
}

static int nvt_tge_remove(struct platform_device *pdev)
{
	PTGE_DRV_INFO pdrv_info;
	unsigned char ucloop;

    nvt_dbg(IND, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_tge_drv_remove(&pdrv_info->module_info);

	nvt_tge_proc_remove(pdrv_info);

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

	kfree(pdrv_info);
	return 0;
}

static int nvt_tge_suspend( struct platform_device * pdev, pm_message_t state)
{
	PTGE_DRV_INFO pdrv_info;;

    nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_tge_drv_suspend(&pdrv_info->module_info);

    nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_tge_resume( struct platform_device * pdev)
{
	PTGE_DRV_INFO pdrv_info;;

    nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_tge_drv_resume(&pdrv_info->module_info);

    nvt_dbg(IND, "finished\n");
	return 0;
}

static struct platform_driver nvt_tge_driver = {
    .driver = {
		        .name   = "kdrv_tge",
				.owner = THIS_MODULE,
				.of_match_table = tge_match_table,
		      },
    .probe      = nvt_tge_probe,
    .remove     = nvt_tge_remove,
	.suspend = nvt_tge_suspend,
	.resume = nvt_tge_resume
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
int __init nvt_tge_module_init(void)
{
	int ret;

    nvt_dbg(WRN, "\n");
	ret = platform_driver_register(&nvt_tge_driver);

	#if KDRV_TGE_LOG_BUFFER
	if (register_printout_notifier(kdrv_tge_module_printout_handler) < 0) {
		 KDRV_TGE_ERR("register_printout_notifier fail\n");
    }
	#endif

	return 0;
}

void __exit nvt_tge_module_exit(void)
{
	#if KDRV_TGE_LOG_BUFFER
	if(unregister_printout_notifier(kdrv_tge_module_printout_handler) < 0) {
		KDRV_TGE_ERR("unregister_printout_notifier fail\n");
	}
	#endif
    nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_tge_driver);
}

module_init(nvt_tge_module_init);
module_exit(nvt_tge_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("TGE driver");
MODULE_LICENSE("GPL");


