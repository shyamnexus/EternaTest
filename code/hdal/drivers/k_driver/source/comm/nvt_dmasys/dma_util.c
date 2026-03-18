#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/bitops.h>
#include <linux/miscdevice.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#if defined(__aarch64__)
#include <linux/soc/nvt/fmem.h>
#else
#include <mach/fmem.h>
#endif
#include <comm/dma_util.h>
#include "dma_nvt.h"

#define DRV_VER         "1.2.6"
#define DRV_NAME        "nvt_dmasys"
#define DMA_UTIL_MINOR  MISC_DYNAMIC_MINOR	//dynamic

//static DEFINE_MUTEX(ioctl_mutex);

static long dma_util_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long user_arg)
{
	dma_util_t ker_arg = {0};
	long ret = 0;
	uintptr_t src_va, dst_va;

	//mutex_lock(&ioctl_mutex);

	if (copy_from_user((void *)&ker_arg, (void *)user_arg, sizeof(dma_util_t))) {
		pr_err("copy_from_user fail\n");
		ret = -EFAULT;
		goto exit;
	}

	//must be cache line alignment for safe
	if (ker_arg.size % DMA_CACHE_LINE_SZ) {
		pr_err("%s: size(%d) not %d aligned\n",
		       __func__, ker_arg.size, DMA_CACHE_LINE_SZ);
		ret = -EFAULT;
		goto exit;
	}

	if (ker_arg.src_ddr_id >= DDR_ID_MAX) {
		pr_err("%s: Invalid src_ddr_id(%d)\n",
		       __func__, ker_arg.src_ddr_id);
		ret = -EFAULT;
		goto exit;
	}

	if (ker_arg.dst_ddr_id >= DDR_ID_MAX) {
		pr_err("%s: Invalid dst_ddr_id(%d)\n",
		       __func__, ker_arg.dst_ddr_id);
		ret = -EFAULT;
		goto exit;
	}

	switch (cmd) {
	case DMA_UTIL_DO_DMA_MEMCPY:
		if (!ker_arg.dst_addr || !ker_arg.src_addr) {
			pr_err("%s: Invalid dst_addr(0x%llx) src_addr(0x%llx)\n",
		       __func__, (u64) ker_arg.dst_addr, (u64) ker_arg.src_addr);
			ret = -EFAULT;
			goto exit;
		}

		if (ker_arg.flush == 1) {
			dst_va = nvtmem_dma_pa2va_remap(ker_arg.dst_addr, ker_arg.size);
			src_va = nvtmem_dma_pa2va_remap(ker_arg.src_addr, ker_arg.size);

			nvtmem_dma_flush_dev2mem(dst_va, ker_arg.size);
			nvtmem_dma_flush_mem2dev(src_va, ker_arg.size);

			ker_arg.dst_addr = nvtmem_dma_va2pa(dst_va);
			ker_arg.src_addr = nvtmem_dma_va2pa(src_va);

			nvtmem_dma_pa2va_unmap(dst_va, ker_arg.dst_addr);
			nvtmem_dma_pa2va_unmap(src_va, ker_arg.src_addr);
		}

		if (ker_arg.size > 0) {
			ret = nvtmem_dma_memcpy(ker_arg.dst_ddr_id, ker_arg.dst_addr,
						ker_arg.src_ddr_id, ker_arg.src_addr,
						ker_arg.size);
			if (ret < 0) {
				pr_err("%s: dma memcpy from 0x%llx to 0x%llx with "
					"size = %d fail, return %d\n", __func__,
					(u64) ker_arg.src_addr, (u64) ker_arg.dst_addr,
					ker_arg.size, (int)ret);
				ret = -EFAULT;
				goto exit;
			}
		}
		else {
			pr_err("%s,Invalid size = %d\n", __func__, ker_arg.size);
			ret = -EFAULT;
			goto exit;
		}

		break;

	case DMA_UTIL_DO_DMA_MEMSET:
		if (!ker_arg.dst_addr) {
			pr_err("%s: Invalid dst_addr(0x%llx)\n",
		       __func__, (u64) ker_arg.dst_addr);
			ret = -EFAULT;
			goto exit;
		}

		if (ker_arg.flush == 1) {
			dst_va = nvtmem_dma_pa2va_remap(ker_arg.dst_addr, ker_arg.size);

			nvtmem_dma_flush_dev2mem(dst_va, ker_arg.size);

			ker_arg.dst_addr = nvtmem_dma_va2pa(dst_va);

			nvtmem_dma_pa2va_unmap(dst_va, ker_arg.dst_addr);
		}

		ret = nvtmem_dma_memset(ker_arg.dst_ddr_id, ker_arg.dst_addr,
					ker_arg.size, ker_arg.pattern);
		if (ret < 0) {
			pr_err("%s: dma memset addr 0x%llx with size = %d fail, "
			       "return %d\n", __func__, (u64) ker_arg.dst_addr,
			       ker_arg.size, (int)ret);
			ret = -EFAULT;
			goto exit;
		}
		break;

	default:
		pr_err("%s, unknown command: 0x%x \n", __func__, cmd);
		goto exit;
		break;
	}

exit:
	//mutex_unlock(&ioctl_mutex);

	return ret;
}

static int dma_util_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dma_util_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations dma_util_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = dma_util_ioctl,
	.open = dma_util_open,
	.release = dma_util_release,
};

struct miscdevice dma_util_dev = {
	.minor = DMA_UTIL_MINOR,
	.name = "dma_util",
	.fops = &dma_util_fops,
};

int __init dma_util_init(void)
{
	int ret;

	pr_info("%s: init ver: %s\n", DRV_NAME, DRV_VER);

	ret = misc_register(&dma_util_dev);
	if (ret < 0) {
		pr_err("%s, misc_register fail\n", __func__);
		return ret;
	}

	ret = dma_proc_init(NULL);
	if (ret < 0) {
		pr_err("%s, dma_proc_init fail\n", __func__);
		return ret;
	}

	ret = frm_dma_init();

	return ret;
}

void __exit dma_util_exit(void)
{
	frm_dma_exit();

	dma_proc_deinit(NULL);

	misc_deregister(&dma_util_dev);

	pr_info("%s: exit ver: %s\n", DRV_NAME, DRV_VER);

	return;
}

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

module_init(dma_util_init);
module_exit(dma_util_exit);

MODULE_AUTHOR("Novatek Corp.");
MODULE_DESCRIPTION("Novatek DMA UTIL");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VER);
