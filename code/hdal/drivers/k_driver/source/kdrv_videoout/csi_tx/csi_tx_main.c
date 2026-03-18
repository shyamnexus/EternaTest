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

#include "csi_tx_drv.h"
#include "csi_tx_reg.h"
#include "csi_tx_main.h"
#include "csi_tx_proc.h"
#include "csi_tx_dbg.h"
#include "dsiphy.h"

//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int csi_tx_debug_level = NVT_DBG_ERR;//NVT_DBG_IND;
module_param_named(csi_tx_debug_level, csi_tx_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(csi_tx_debug_level, "Debug message level");
#endif


//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id csi_tx_match_table[] = {
	{	.compatible = "nvt,nvt_csi_tx"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
static int nvt_csi_tx_open(struct inode *inode, struct file *file);
static int nvt_csi_tx_release(struct inode *inode, struct file *file);
static long nvt_csi_tx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_csi_tx_probe(struct platform_device *pdev);
static int nvt_csi_tx_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_csi_tx_resume(struct platform_device *pdev);
static int nvt_csi_tx_remove(struct platform_device *pdev);
int __init nvt_csi_tx_module_init(void);
void __exit nvt_csi_tx_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_csi_tx_open(struct inode *inode, struct file *file)
{
	CSI_TX_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, CSI_TX_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, CSI_TX_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_csi_tx_drv_open(&pdrv_info->module_info, MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_csi_tx_release(struct inode *inode, struct file *file)
{
	CSI_TX_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, CSI_TX_DRV_INFO, cdev);
	nvt_csi_tx_drv_release(&pdrv_info->module_info, MINOR(inode->i_rdev));
	return 0;
}

static long nvt_csi_tx_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	PCSI_TX_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_csi_tx_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info, cmd, arg);
}

struct file_operations nvt_csi_tx_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_csi_tx_open,
	.release = nvt_csi_tx_release,
	.unlocked_ioctl = nvt_csi_tx_ioctl,
	.llseek  = no_llseek,
};

static int nvt_csi_tx_probe(struct platform_device *pdev)
{
	CSI_TX_DRV_INFO *pdrv_info;
	const struct of_device_id *match;
	int ret = 0;
	int index;
	unsigned char ucloop;

	nvt_dbg(IND, "%s\n", pdev->name);

	match = of_match_device(csi_tx_match_table, &pdev->dev);
	if (!match) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(CSI_TX_DRV_INFO), GFP_KERNEL);
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
		nvt_dbg(IND, "%d. resource:0x%llx size:0x%llx\n", ucloop, pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (!request_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]), pdev->name)) {
			nvt_dbg(ERR, "failed to request memory resource%d\n", ucloop);
			for (index = 0; index < ucloop; index++) {
				release_mem_region(pdrv_info->presource[index]->start, resource_size(pdrv_info->presource[index]));
			}
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
	}
	

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++) {
		pdrv_info->module_info.io_addr[ucloop] = ioremap(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));
		if (pdrv_info->module_info.io_addr[ucloop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", ucloop);
			for (index = 0; index < ucloop; index++) {
			    iounmap(pdrv_info->module_info.io_addr[index]);
			}
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		}
	}
#if 0 // 538 not support
	if (dsiphy_check_boostrap() < 0) {
		nvt_dbg(ERR, "DSI_PROT_EN BST[7] is 0, CSI-TX init fail\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}
#endif
	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		pdrv_info->module_info.iinterrupt_id[ucloop] = platform_get_irq(pdev, ucloop);
		nvt_dbg(IND, "IRQ %d. ID%d\n", ucloop, pdrv_info->module_info.iinterrupt_id[ucloop]);
		if (pdrv_info->module_info.iinterrupt_id[ucloop] < 0) {
			nvt_dbg(ERR, "No IRQ resource defined\n");
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
	}
	
	//Get clock source
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		char  pstr1[16];

		if (ucloop == 0) {
			sprintf(pstr1, "clk_csi_tx");
		} else {
			sprintf(pstr1, "clk_csi_tx2");
		}
		pdrv_info->module_info.pclk[ucloop] = clk_get(&pdev->dev, pstr1);

		if (IS_ERR(pdrv_info->module_info.pclk[ucloop])) {
			nvt_dbg(ERR, "faile to get csi_tx clock[%d] source (%s)\n", ucloop, pstr1);
			ret = -ENODEV;
			goto FAIL_FREE_REMAP;
		}
		nvt_dbg(IND, "CLK %d. Name %s\n", ucloop, pstr1);
	}


	//Dynamic to allocate Device ID
	if (vos_alloc_chrdev_region(&pdrv_info->dev_id, MODULE_MINOR_COUNT, MODULE_NAME)) {
		nvt_dbg(ERR, "Can't get device ID\n");
		ret = -ENODEV;
		goto FAIL_FREE_REMAP;
	}

	nvt_dbg(IND, "DevID Major:%d minor:%d\n", MAJOR(pdrv_info->dev_id), MINOR(pdrv_info->dev_id));

	/* Register character device for the volume */
	cdev_init(&pdrv_info->cdev, &nvt_csi_tx_fops);
	pdrv_info->cdev.owner = THIS_MODULE;

	if (cdev_add(&pdrv_info->cdev, pdrv_info->dev_id, MODULE_MINOR_COUNT)) {
		nvt_dbg(ERR, "Can't add cdev\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	pdrv_info->pmodule_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(pdrv_info->pmodule_class)) {
		nvt_dbg(ERR, "failed in creating class.\n");
		ret = -ENODEV;
		goto FAIL_CDEV;
	}

	/* register your own device in sysfs, and this will cause udev to create corresponding device node */
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++) {
		pdrv_info->pdevice[ucloop] = device_create(pdrv_info->pmodule_class, NULL
			, MKDEV(MAJOR(pdrv_info->dev_id), (ucloop + MINOR(pdrv_info->dev_id))), NULL
			, MODULE_NAME"%d", ucloop);

		if (IS_ERR(pdrv_info->pdevice[ucloop])) {
			nvt_dbg(ERR, "failed in creating device%d.\n", ucloop);
			#if 0
			if( ucloop > 0 ){
				for (index = 0; index < ucloop; index++)
					device_unregister(pdrv_info->pdevice[index]);
			}
			#endif
			ret = -ENODEV;
			goto FAIL_CLASS;
		}
	}

#if !defined(CONFIG_NVT_SMALL_HDAL)
	ret = nvt_csi_tx_proc_init(pdrv_info);

	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}
#endif

	ret = nvt_csi_tx_drv_init(&pdrv_info->module_info);

	platform_set_drvdata(pdev, pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DRV_INIT;
	}

	return ret;

FAIL_DRV_INIT:
#if !defined(CONFIG_NVT_SMALL_HDAL)
	nvt_csi_tx_proc_remove(pdrv_info);

FAIL_DEV:
#endif
	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
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

static int nvt_csi_tx_remove(struct platform_device *pdev)
{
	PCSI_TX_DRV_INFO pdrv_info;
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	pdrv_info = platform_get_drvdata(pdev);

	nvt_csi_tx_drv_remove(&pdrv_info->module_info);

#if !defined(CONFIG_NVT_SMALL_HDAL)
	nvt_csi_tx_proc_remove(pdrv_info);
#endif

	for (ucloop = 0 ; ucloop < (MODULE_MINOR_COUNT) ; ucloop++)
		device_unregister(pdrv_info->pdevice[ucloop]);

	class_destroy(pdrv_info->pmodule_class);
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++)
		clk_put(pdrv_info->module_info.pclk[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
	    iounmap(pdrv_info->module_info.io_addr[ucloop]);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM ; ucloop++)
		release_mem_region(pdrv_info->presource[ucloop]->start, resource_size(pdrv_info->presource[ucloop]));

	kfree(pdrv_info);
	return 0;
}

static int nvt_csi_tx_suspend(struct platform_device *pdev, pm_message_t state)
{
	PCSI_TX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_csi_tx_drv_suspend(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}


static int nvt_csi_tx_resume(struct platform_device *pdev)
{
	PCSI_TX_DRV_INFO pdrv_info;;

	nvt_dbg(IND, "start\n");

	pdrv_info = platform_get_drvdata(pdev);
	nvt_csi_tx_drv_resume(&pdrv_info->module_info);

	nvt_dbg(IND, "finished\n");
	return 0;
}

static struct platform_driver nvt_csi_tx_driver = {
	.driver = {
				.name   = "nvt_csi_tx",
				.owner = THIS_MODULE,
				.of_match_table = csi_tx_match_table,
				},
	.probe      = nvt_csi_tx_probe,
	.remove     = nvt_csi_tx_remove,
	.suspend = nvt_csi_tx_suspend,
	.resume = nvt_csi_tx_resume
};

int __init nvt_csi_tx_module_init(void)
{
	int ret;

	nvt_dbg(WRN, "\n");
	ret = platform_driver_register(&nvt_csi_tx_driver);

	return 0;
}

void __exit nvt_csi_tx_module_exit(void)
{
	nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_csi_tx_driver);
}

module_init(nvt_csi_tx_module_init);
module_exit(nvt_csi_tx_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("csi_tx driver");
MODULE_VERSION("1.00.002");
MODULE_LICENSE("GPL");
