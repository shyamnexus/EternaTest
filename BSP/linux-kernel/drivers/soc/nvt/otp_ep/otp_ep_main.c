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
#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
#include <linux/soc/nvt/nvt-pcie-lib.h>
#endif
#include <asm/signal.h>
#include "otp_ep_drv.h"
#include "otp_ep_main.h"
#include "otp_ep_proc.h"
#include "otp_ep_dbg.h"
#if defined (__KERNEL__)
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
#define IOREMAP(m, n)	ioremap(m, n)
#else
#define IOREMAP(m, n)	ioremap_nocache(m, n)
#endif
#endif


//=============================================================================
//Module parameter : Set module parameters when insert the module
//=============================================================================
#ifdef DEBUG
unsigned int otp_debug_level = NVT_DBG_IND;//NVT_DBG_ERR;
module_param_named(otp_debug_level, otp_debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(otp_debug_level, "Debug message level");
#endif

//=============================================================================
// Global variable
//=============================================================================
static struct of_device_id otp_match_table[] = {
	{	.compatible = "nvt,nvt_otp_ep0"},
	{}
};

//=============================================================================
// function declaration
//=============================================================================
//static int nvt_ddr_arb_open(struct inode *inode, struct file *file);
//static int nvt_ddr_arb_release(struct inode *inode, struct file *file);
//static long nvt_ddr_arb_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int nvt_otp_ep_probe(struct platform_device *pdev);
static int nvt_otp_ep_suspend(struct platform_device *pdev, pm_message_t state);
static int nvt_otp_ep_resume(struct platform_device *pdev);
static int nvt_otp_ep_remove(struct platform_device *pdev);
int __init nvt_otp_ep_module_init(void);
void __exit nvt_otp_ep_module_exit(void);

//=============================================================================
// function define
//=============================================================================
static int nvt_otp_open(struct inode *inode, struct file *file)
{
	OTP_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, OTP_DRV_INFO, cdev);

	pdrv_info = container_of(inode->i_cdev, OTP_DRV_INFO, cdev);
	file->private_data = pdrv_info;

	if (nvt_otp_ep_drv_open(&pdrv_info->module_info[0], MINOR(inode->i_rdev))) {
		nvt_dbg(ERR, "failed to open driver\n");
		return -1;
	}

	return 0;
}

static int nvt_otp_release(struct inode *inode, struct file *file)
{
	OTP_DRV_INFO *pdrv_info;

	pdrv_info = container_of(inode->i_cdev, OTP_DRV_INFO, cdev);
	nvt_otp_ep_drv_release(&pdrv_info->module_info[0], MINOR(inode->i_rdev));
	return 0;
}

static long nvt_otp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	POTP_DRV_INFO pdrv;

	inode = file_inode(filp);
	pdrv = filp->private_data;

	return nvt_otp_ep_drv_ioctl(MINOR(inode->i_rdev), &pdrv->module_info[0], cmd, arg);
}

struct file_operations nvt_otp_ep_fops = {
	.owner   = THIS_MODULE,
	.open    = nvt_otp_open,
	.release = nvt_otp_release,
	.unlocked_ioctl = nvt_otp_ioctl,
	.llseek  = no_llseek,
};


static int nvt_otp_ep_probe(struct platform_device *pdev)
{
	OTP_DRV_INFO *pdrv_info;
	const struct of_device_id *match;
	int ret = 0;
	int loop;

#if IS_ENABLED(CONFIG_PCI)
#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	u32 prop;
	nvtpcie_chipid_t ep_chipid;
#endif
#endif

	nvt_dbg(IND, "%s\n", pdev->name);

	match = of_match_device(otp_match_table, &pdev->dev);
	if (!match) {
		nvt_dbg(ERR, "Platform device not found \n");
		return -EINVAL;
	}

	pdrv_info = kzalloc(sizeof(OTP_DRV_INFO), GFP_KERNEL);
	if (!pdrv_info) {
		nvt_dbg(ERR, "failed to allocate memory\n");
		return -ENOMEM;
	}

#if IS_ENABLED(CONFIG_PCI)
#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	/* EP info */
	if (of_property_read_u32(pdev->dev.of_node, "chip_id", &prop)) {
		dev_err(&pdev->dev, "get ep prop failed\n");
		return -EINVAL;
	}

	if (prop > nvtpcie_get_ep_count()) {
		dev_info(&pdev->dev, "skip ep[%d] ep count [%d]\n", prop - CHIP_EP0, nvtpcie_get_ep_count());
		return 0;
	} else {
		//dev_info(&pdev->dev, "start ep[%d]\n", prop - CHIP_EP0);
		nvt_dbg(IND, "start ep[%d]\n", prop - CHIP_EP0);
	}
	ep_chipid = (nvtpcie_chipid_t)prop;
	nvt_dbg(IND, "ep_chipid:%d\n", ep_chipid);
#endif
#endif
	/* resuorce - memory */



	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		pdrv_info->presource[loop] = platform_get_resource(pdev, IORESOURCE_MEM, loop);
		if (pdrv_info->presource[loop] == NULL) {
			nvt_dbg(ERR, "No IO memory resource defined:%d -> %d\n", loop, MODULE_REG_NUM);
			ret = -ENODEV;
			goto FAIL_FREE_BUF;
		}
#if IS_ENABLED(CONFIG_PCI)
#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
		if (-1 == nvtpcie_conv_resource(ep_chipid, pdrv_info->presource[loop])) {
			dev_err(&pdev->dev, "conv ep mem failed ep-id[%d]\n", ep_chipid);
			return -EINVAL;
		} else {
			nvt_dbg(IND, "conv ep mem success ep_chipid:%d\n", ep_chipid);
		}
#endif
#endif
	}

#if !IS_ENABLED(CONFIG_PCI)
#if !IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		nvt_dbg(IND, "%d. resource:0x%llx size:0x%llx\n", loop, pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
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
#endif
#endif
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
#if IS_ENABLED(CONFIG_PCI)
#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
		pdrv_info->module_info[loop].io_addr[loop] = devm_ioremap(&pdev->dev, pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
#endif
#else
		pdrv_info->module_info[loop].io_addr[loop] = IOREMAP(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));
#endif
		if (pdrv_info->module_info[loop].io_addr[loop] == NULL) {
			nvt_dbg(ERR, "ioremap() failed in module%d\n", loop);
#if (MODULE_REG_NUM > 1)
			for (; loop > 0 ;) {
				loop -= 1;
				iounmap(pdrv_info->module_info[loop].io_addr[loop]);
			}
#endif
			ret = -ENODEV;
			goto FAIL_FREE_RES;
		} else {
			//printk("loop[%d] address[0x%llx]\r\n", loop, (UINT64)pdrv_info->module_info[loop].io_addr[loop]);
			nvt_dbg(IND, "loop[%d] address[0x%llx]\r\n", loop, (UINT64)pdrv_info->module_info[loop].io_addr[loop]);
		}
	}

	ret = nvt_otp_ep_proc_init(pdrv_info);
	if (ret) {
		nvt_dbg(ERR, "failed in creating proc.\n");
		goto FAIL_DEV;
	}

	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++) {
		ret = nvt_otp_ep_drv_init(&pdrv_info->module_info[loop], loop);
		if (ret) {
			nvt_dbg(ERR, "failed in creating drvdata.\n");
			goto FAIL_DRV_INIT;
		}
	}

	platform_set_drvdata(pdev, pdrv_info);


	return ret;
FAIL_DRV_INIT:
	nvt_otp_ep_proc_remove(pdrv_info);

FAIL_DEV:
	for (loop = 0 ; loop < (MODULE_MINOR_COUNT) ; loop++)
		device_unregister(pdrv_info->pdevice[loop]);
#if 0
FAIL_CLASS:
	class_destroy(pdrv_info->pmodule_class);


FAIL_CDEV:
	cdev_del(&pdrv_info->cdev);
	vos_unregister_chrdev_region(pdrv_info->dev_id, MODULE_MINOR_COUNT);

FAIL_FREE_REMAP:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++)
		iounmap(pdrv_info->module_info.io_addr[loop]);
#endif

FAIL_FREE_RES:
	for (loop = 0 ; loop < MODULE_REG_NUM ; loop++)
		release_mem_region(pdrv_info->presource[loop]->start, resource_size(pdrv_info->presource[loop]));

FAIL_FREE_BUF:
	kfree(pdrv_info);
	pdrv_info = NULL;
	return ret;

	nvt_dbg(IND, "%s\n", pdev->name);
	return 0;

}

static int nvt_otp_ep_remove(struct platform_device *pdev)
{
	return 0;
}

static int nvt_otp_ep_suspend(struct platform_device *pdev, pm_message_t state)
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


static int nvt_otp_ep_resume(struct platform_device *pdev)
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

static struct platform_driver nvt_otp_driver = {
	.driver = {
		.name   = "nvt_otp_ep0",
		.owner = THIS_MODULE,
		.of_match_table = otp_match_table,
	},
	.probe      = nvt_otp_ep_probe,
	.remove     = nvt_otp_ep_remove,
	.suspend = nvt_otp_ep_suspend,
	.resume = nvt_otp_ep_resume
};

int __init nvt_otp_ep_module_init(void)
{
	int ret;

	nvt_dbg(WRN, "\n");

	printk("nvt_otp_ep_module_init\r\n");

	ret = platform_driver_register(&nvt_otp_driver);

	return 0;
}

void __exit nvt_otp_ep_module_exit(void)
{
	nvt_dbg(WRN, "\n");
	platform_driver_unregister(&nvt_otp_driver);
}

arch_initcall(nvt_otp_ep_module_init);
module_exit(nvt_otp_ep_module_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("nvt otp ep driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.00.010");