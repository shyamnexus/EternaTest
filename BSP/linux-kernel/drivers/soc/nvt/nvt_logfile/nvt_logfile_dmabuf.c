/**
    NVT logfile-dam-buf function
    This file will provide the logfile shared buffer object
    @file logfile-dma-buf.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt_logfile.h>
#include "nvt_logfile_internal.h"

typedef struct {
	struct miscdevice miscdev;
	int ep_idx;
} logfile_dev_t;

static struct dma_buf *g_logfile_dmabuf = NULL;

static void* _shmblk_kzalloc(size_t size)
{
	nvtpcie_chipid_t my_chipid;
	int ep_count;
	void *va = NULL;
	logfile_shmblk_t *p_shm;
	int ep_idx;

	my_chipid = nvtpcie_get_my_chipid();
	ep_count = nvtpcie_get_ep_count();

	if (CHIP_RC == my_chipid) {
		size_t total_size = LOGFILE_BUFFER_SIZE * (1 + ep_count);

		va = kzalloc(total_size, GFP_KERNEL);
		if (NULL == va) {
			DBG_ERR("alloc buffer failed, total_size 0x%llx\r\n", (u64)total_size);
			return NULL;
		}

		if (0 == ep_count) {
			return va; //if no EP, we are done
		}
	}

	p_shm = nvtpcie_shmblk_get(NVT_LOGFILE_NAME, sizeof(logfile_shmblk_t));
	if (NULL == p_shm) {
		DBG_ERR("shmblk_get failed\r\n");
		goto kfree_err_exit;
	}

	if (CHIP_RC == my_chipid) { //I am RC. Prepare buffer for EP
		void *ep_va;

		p_shm->size = LOGFILE_BUFFER_SIZE;

		for (ep_idx = 0; ep_idx < ep_count; ep_idx++) {
			ep_va = va + LOGFILE_BUFFER_SIZE * (1 + ep_idx);
			p_shm->rc_buf_pa[ep_idx] = fmem_lookup_pa((uintptr_t)ep_va);
			DBG_INFO("shmblk(ep%d) rc_buf_pa 0x%llx, size 0x%lx\r\n",
				ep_idx, (u64)p_shm->rc_buf_pa[ep_idx], p_shm->size);
		}
	} else { //I am EP. Get buffer from RC
		phys_addr_t mapped_pa;

		ep_idx = my_chipid - CHIP_EP0;

		DBG_INFO("shmblk(ep%d) rc_buf_pa 0x%llx, size 0x%lx\r\n",
			ep_idx, (u64)p_shm->rc_buf_pa[ep_idx], p_shm->size);

		if (0 == p_shm->size) {
			DBG_ERR("Please insert RC side nvt_logfile first\r\n");
			return NULL;
		}

		mapped_pa = nvtpcie_get_upstream_pa(my_chipid, p_shm->rc_buf_pa[ep_idx]);
		if (NVTPCIE_INVALID_PA == mapped_pa) {
			DBG_ERR("map to rc failed\r\n");
			return NULL;
		}

		va = ioremap(mapped_pa, p_shm->size);
		if (NULL == va) {
			DBG_ERR("ioremap rc_buf_pa failed\r\n");
		}

		p_shm->flag[ep_idx] |= LOGFILE_FLAG_USING;
	}

	return va;

kfree_err_exit:
	if (va) {
		kfree(va);
	}
	return NULL;
}

static void _shmblk_kfree(void *va)
{
	nvtpcie_chipid_t my_chipid;

	my_chipid = nvtpcie_get_my_chipid();

	//only release
	if (CHIP_RC == my_chipid) {
		nvtpcie_shmblk_release(NVT_LOGFILE_NAME);
		kfree(va);
	}
}

struct dma_buf* logfile_dmabuf_get_obj(void)
{
	return g_logfile_dmabuf;
}
EXPORT_SYMBOL(logfile_dmabuf_get_obj);

static int logfile_dmabuf_attach(struct dma_buf *dmabuf, struct dma_buf_attachment *attachment)
{
	return 0;
}

static void logfile_dmabuf_detach(struct dma_buf *dmabuf, struct dma_buf_attachment *attachment)
{
}

static struct sg_table *logfile_dmabuf_map_dma_buf(struct dma_buf_attachment *attachment,
					 							enum dma_data_direction dir)
{
	return NULL;
}

static void logfile_dmabuf_unmap_dma_buf(struct dma_buf_attachment *attachment,
										struct sg_table *table,
										enum dma_data_direction dir)
{
}

static void logfile_dmabuf_release(struct dma_buf *dmabuf)
{
	_shmblk_kfree(dmabuf->priv);
	g_logfile_dmabuf = NULL;
}

static int logfile_dmabuf_begin_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
	return 0;
}

static int logfile_dmabuf_end_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
	return 0;
}

static void *logfile_dmabuf_map(struct dma_buf *dmabuf, unsigned long num)
{
	return dmabuf->priv;
}

static void logfile_dmabuf_unmap(struct dma_buf *dmabuf, unsigned long num, void *p)
{
}

static int logfile_dmabuf_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	size_t dmabuf_size = LOGFILE_BUFFER_SIZE;
	void *vaddr = dmabuf->priv;
	return remap_pfn_range(vma, vma->vm_start, virt_to_pfn(vaddr), dmabuf_size, vma->vm_page_prot);
}

static void *logfile_dmabuf_vmap(struct dma_buf *dmabuf)
{
	return dmabuf->priv;
}

static void logfile_dmabuf_vunmap(struct dma_buf *dmabuf, void *vaddr)
{
}

static const struct dma_buf_ops logfile_dmabuf_ops = {
	.attach = logfile_dmabuf_attach,
	.detach = logfile_dmabuf_detach,

	.map_dma_buf = logfile_dmabuf_map_dma_buf,
	.unmap_dma_buf = logfile_dmabuf_unmap_dma_buf,

	.release = logfile_dmabuf_release,

	.begin_cpu_access = logfile_dmabuf_begin_cpu_access,
	.end_cpu_access = logfile_dmabuf_end_cpu_access,

	.map = logfile_dmabuf_map,
	.unmap = logfile_dmabuf_unmap,

	.mmap = logfile_dmabuf_mmap,

	.vmap = logfile_dmabuf_vmap,
	.vunmap = logfile_dmabuf_vunmap,
};

static struct dma_buf *logfile_dmabuf_alloc(size_t size)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf;
	int flags;
	void *vaddr;

	vaddr = _shmblk_kzalloc(size);
	if (!vaddr)
		return NULL;

	DBG_INFO("ver %s, local pa 0x%llx, size 0x%llx\n", NVT_LOGFILE_VERSION,
		fmem_lookup_pa((uintptr_t)vaddr), (u64)size);

	/*
	 * Open for read/write and enable the close-on-exec flag
	 * for the new file descriptor.
	 */
	flags = O_RDWR | O_CLOEXEC;

	exp_info.ops = &logfile_dmabuf_ops;
	exp_info.size = size;
	exp_info.flags = flags;
	exp_info.priv = vaddr;

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		_shmblk_kfree(vaddr);
		return NULL;
	}

	return dmabuf;
}

static void logfile_dmabuf_free(void)
{
/*
	This module can not be removed, because dma_buf_export will cause usage count + 1
	If the usage count > 1, module_exit will be skipped.
	To reduce the usage count, dma_buf_put should be called before module_exit.
	if (g_logfile_dmabuf) {
		dma_buf_put(g_logfile_dmabuf);
	}
*/
}

static ssize_t logfile_mdev_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct dma_buf* dmabuf;
	void *va;
	struct miscdevice *p_miscdev = filp->private_data;
	logfile_dev_t *p_logfile_dev = container_of(p_miscdev, logfile_dev_t, miscdev);
	unsigned int offset = 0;
	ssize_t ret_bytes;

	if (p_logfile_dev->ep_idx >= 0) {
		offset = (p_logfile_dev->ep_idx + 1) * LOGFILE_BUFFER_SIZE;
	}

	dmabuf = g_logfile_dmabuf;
	if (!dmabuf) {
		return -EFAULT;
	}

	va = dma_buf_kmap(dmabuf, 0);
	if (!va) {
		return -EFAULT;
	}

	ret_bytes = simple_read_from_buffer(ubuf, count, ppos, va + offset, dmabuf->size);

	dma_buf_kunmap(dmabuf, 0, va);

	return ret_bytes;
}

static long logfile_mdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case LOGFILE_IOCTL_FD:
	{
		int fd;
		int flags;

		/*
		 * Open for read/write and enable the close-on-exec flag
		 * for the new file descriptor.
		 */
		flags = O_RDWR | O_CLOEXEC;
		fd = dma_buf_fd(g_logfile_dmabuf, flags);
		if (fd < 0)
			return fd;

		if (copy_to_user((void __user *)arg, &fd, sizeof(fd)))
			return -EFAULT;

		break;
	}
	default:
		return -EFAULT;
	}
	return 0;
}

static int logfile_mdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* Setup up a userspace mmap with the given vma. */
	return dma_buf_mmap(g_logfile_dmabuf, vma, 0);
}

static const struct file_operations logfile_mdev_fops = {
	.open = simple_open,
	.read = logfile_mdev_read,
	.owner = THIS_MODULE,
	.unlocked_ioctl	= logfile_mdev_ioctl,
	.mmap = logfile_mdev_mmap,
};

static logfile_dev_t g_logfile_dev[CHIP_MAX] = {0};

static int logfile_misc_register(void)
{
	logfile_dev_t *p_logfile_dev;
	int ret;

	//create local device
	p_logfile_dev = &g_logfile_dev[0];
	p_logfile_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	p_logfile_dev->miscdev.name = NVT_LOGFILE_NAME;
	p_logfile_dev->miscdev.fops = &logfile_mdev_fops;
	p_logfile_dev->ep_idx = -1;

	ret = misc_register(&p_logfile_dev->miscdev);
	if (ret < 0) {
		DBG_ERR("misc_register fail\n");
		return ret;
	}

	//create remote ep device (only on RC side)
	if (CHIP_RC == nvtpcie_get_my_chipid()) {
		int ep_count;
		int ep_idx;
		char devname[32];

		ep_count = nvtpcie_get_ep_count();
		for (ep_idx = 0; ep_idx < ep_count; ep_idx++) {
			p_logfile_dev = &g_logfile_dev[ep_idx + 1];
			snprintf(devname, sizeof(devname), "%s_ep%d", NVT_LOGFILE_NAME, ep_idx);

			p_logfile_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
			p_logfile_dev->miscdev.name = devname;
			p_logfile_dev->miscdev.fops = &logfile_mdev_fops;
			p_logfile_dev->ep_idx = ep_idx;

			ret = misc_register(&p_logfile_dev->miscdev);
			if (ret < 0) {
				DBG_ERR("misc_register fail, ep_idx %d\n", ep_idx);
				return ret;
			}
		}
	}

	return 0;
}

static void logfile_misc_deregister(void)
{
	logfile_dev_t *p_logfile_dev;

	//destroy remote ep device (only on RC side)
	if (CHIP_RC == nvtpcie_get_my_chipid()) {
		int ep_count;
		int ep_idx;

		ep_count = nvtpcie_get_ep_count();
		for (ep_idx = 0; ep_idx < ep_count; ep_idx++) {
			p_logfile_dev = &g_logfile_dev[ep_idx + 1];
			misc_deregister(&p_logfile_dev->miscdev);
		}
	}

	//destroy local device
	p_logfile_dev = &g_logfile_dev[0];
	misc_deregister(&p_logfile_dev->miscdev);
}


static int __init logfile_mdev_init(void)
{
	int ret;

	/*
	 * Create the logfile dma_buf object here,
	 * and return error if allocate failed to
	 * avoid userspace application using invalid mdev.
	 */
	g_logfile_dmabuf = logfile_dmabuf_alloc(LOGFILE_BUFFER_SIZE);
	if (!g_logfile_dmabuf)
		return -ENOMEM;

	ret = logfile_misc_register();

	return ret;
}

static void __exit logfile_mdev_exit(void)
{
	logfile_misc_deregister();

	logfile_dmabuf_free();
}

module_init(logfile_mdev_init);
module_exit(logfile_mdev_exit);

MODULE_AUTHOR("Novatek Microelectronics Corp.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NVT_LOGFILE_DMABUF");
MODULE_VERSION(NVT_LOGFILE_VERSION);
