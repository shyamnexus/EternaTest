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
//#include <kwrap/dev.h>

//#include "otp_platform.h"
#include "stbc_drv.h"
//#include "ddr_arb_reg.h"
#include "stbc_main.h"
#include "stbc_proc.h"
#include "stbc_dbg.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int otp_debug_level = NVT_DBG_ERR;
module_param_named(otp_debug_level, otp_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(otp_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id stbc_match_table[] = {
	{   .compatible = "nvt,nvt_stbc"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_stbc_probe(struct platform_device *pdev);
 int __init nvt_stbc_module_init(void);
void __exit nvt_stbc_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_stbc_open(struct inode *inode, struct file *file)
{
	STBC_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, STBC_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, STBC_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_stbc_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_stbc_release(struct inode *inode, struct file *file)
{
	STBC_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, STBC_DRV_INFO, cdev);
	nvt_stbc_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
	return 0;
}

#if 0
static long nvt_stbc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PSTBC_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_stbc_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}
#endif

struct file_operations nvt_stbc_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_stbc_open,
	.release = nvt_stbc_release,
#if 0
	.unlocked_ioctl = nvt_stbc_ioctl,
#endif
	.llseek  = no_llseek,
};


static int nvt_stbc_probe(struct platform_device *pdev)
{
	STBC_DRV_INFO *pdrv_info;
	const struct of_device_id *match;
	int ret = 0;
	int loop;

	nvt_dbg(IND, "%s\n", pdev->name);

	match = of_match_device(stbc_match_table, &pdev->dev);
	if (!match) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(STBC_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		nvt_dbg(ERR, "failed to allocate memory\n");
		return -ENOMEM;
	}


	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		pdrv_info->presource[loop] = platform_get_resource(pdev, IORESOURCE_MEM, loop);
		if (pdrv_info->presource[loop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d -> %d\n", loop, MODULE_REG_NUM);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
//		nvt_dbg(IND, "%d. resource:0x%llx size:0x%x\n", (int)loop, (uintptr_t)pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
		if (!request_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", loop);
#if (MODULE_REG_NUM > 1)
			for (; loop > 0 ;) {
				loop -= 1;
				release_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}

	nvt_dbg(ERR, "    STBC addr[0x%08lx]", (unsigned long)pdrv_info->presource[IOADDR_STBC_REG_BASE_ENUM]->start);
	//nvt_dbg(IND, "STBC CKG addr[0x%08lx]", pdrv_info->presource[IOADDR_STBC_CG_REG_BASE_ENUM]->start);
	//nvt_dbg(IND, "    Core addr[0x%08lx]", pdrv_info->presource[IOADDR_CORE_REG_BASE_ENUM]->start);


	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		pdrv_info->module_info.io_addr[loop] = ioremap(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
		if (pdrv_info->module_info.io_addr[loop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", loop);
#if (MODULE_REG_NUM > 1)
			for (; loop > 0 ;) {
				loop -= 1;
				iounmap(pdrv_info->module_info.io_addr[loop]);
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}

	//Get clock source
	for (loop = 0 ; loop < MODULE_CLK_NUM; loop++) {
//		char  pstr1[16];

//		snprintf(pstr1, 16, "%08llx.grph", pdrv_info->presource[ucloop]->start);

		pdrv_info->module_info.pclk[loop] = clk_get(&pdev->dev, NULL);
	}

	ret = nvt_stbc_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}
	ret = nvt_stbc_drv_init(&pdrv_info->module_info);

	platform_set_drvdata(pdev, pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating drvdata.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;
FAIL_DRV_INIT:
	nvt_stbc_proc_remove(pdrv_info);

FAIL_DEV:
	for (loop = 0 ; loop < (MODULE_MINOR_COUNT) ; loop++) {
		device_unregister(pdrv_info->pdevice[loop]);
	}
#if 0
FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);


FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		iounmap(pdrv_info->module_info.io_addr[loop]);
	}
#endif

FAIL_FREE_RES:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		release_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
	}

FAIL_FREE_BUF:
	kfree(pdrv_info);
	pdrv_info = NULL;
	return ret;

	nvt_dbg(IND, "%s\n", pdev->name);
	return 0;

}

static int nvt_stbc_remove(struct platform_device *pdev)
{
#if 0
	PXXX_DRV_INFO pdrv_info;
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_ddr_arb_drv_remove(&pdrv_info->module_info);

	nvt_ddr_arb_proc_remove(pdrv_info);

	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		clk_put(pdrv_info->module_info.pclk[ucloop]);
	}

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		device_unregister(pdrv_info->pdevice[ucloop]);
	}

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		iounmap(pdrv_info->module_info.io_addr[ucloop]);
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
	}

	kfree(pdrv_info);
	pdrv_info = NULL;
#endif
	return 0;
}

static int nvt_stbc_suspend(struct platform_device *pdev, pm_message_t state)
{
#if 0
	PXXX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_ddr_arb_drv_suspend(&pdrv_info->module_info);
#endif
	nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_stbc_resume(struct platform_device *pdev)
{
#if 0
	PXXX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_ddr_arb_drv_resume(&pdrv_info->module_info);
#endif
	nvt_dbg(IND, "finished\n");
	return 0;
}

static struct platform_driver nvt_stbc_driver = {
	.driver = {
		.name   = "nvt_stbc",
		.owner = THIS_MODULE,
		.of_match_table = stbc_match_table,
	},
	.probe      = nvt_stbc_probe,
	.remove     = nvt_stbc_remove,
	.suspend    = nvt_stbc_suspend,
	.resume     = nvt_stbc_resume
};

int __init nvt_stbc_module_init(void)
{
	int ret;

	nvt_dbg(WRN, "\n");

	printk("nvt_stbc_module_init\r\n");

	ret = platform_driver_register(&nvt_stbc_driver);

	return 0;
}

void __exit nvt_stbc_module_exit(void)
{
	nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_stbc_driver);
}


arch_initcall(nvt_stbc_module_init);
//module_init(nvt_otp_module_init);
module_exit(nvt_stbc_module_exit);
MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt stbc driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00.004");

