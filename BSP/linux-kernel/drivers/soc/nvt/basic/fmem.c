/**
    NVT memory operation handling
    To handle the flush allocate and other memory handling api
    @file       fmem.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/mmzone.h>
#include <linux/memory.h>
#include <linux/kallsyms.h>
#include <linux/nodemask.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dma-map-ops.h>
#include <linux/cma.h>
#include <linux/mutex.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/soc/nvt/fmem.h>
#include <asm/cacheflush.h>
#include <asm/cache.h>
#ifdef __arm__
#include "../../arch/arm/mm/dma.h"
#include <asm/fixmap.h>
#endif

/*
 * Local variables declaration
 */
static DEFINE_SEMAPHORE(sema_memzone);
static DEFINE_SEMAPHORE(sema_mem_info);
static DEFINE_SEMAPHORE(sema_mem_lock);
#if defined(CONFIG_NVT_DCACHE_FLUSH_ALL)
static DEFINE_SPINLOCK(clean_all_lock);
#endif /* CONFIG_NVT_DCACHE_FLUSH_ALL */
struct list_head nvt_memzone_info_list_root;

u32 nonlinear_cpuaddr_flush = 0;
u32 cpuaddr_flush = 0;
u32 va_not_64align = 0, length_not_64align = 0;
u32 debug_counter = 0;
#ifdef CONFIG_STACKPROTECTOR
unsigned int stack_dbg_value = 0;
#endif
static u32 vb_cache_flush_en = 0;
/* Used by cache flush all function */
unsigned int total_num_cpu = NR_CPUS;

static struct platform_device *pfmem_dev = NULL;

#define DEBUG_WRONG_CACHE_API   0x0
#define DEBUG_WRONG_CACHE_VA    0x1
#define DEBUG_WRONG_CACHE_LEN   0x2

#define CACHE_ALIGN_MASK  L1_CACHE_BYTES  //cache line with 64 bytes: arch/${CPU}/include/asm/cache.h
#define MAX_CMD_LENGTH    30

#define FMEM_DEV_NAME	 "nvt_fmem"

#ifdef CONFIG_CMA
struct cma {
	unsigned long   base_pfn;
	unsigned long   count;
	unsigned long   *bitmap;
	unsigned int order_per_bit; /* Order of pages represented by one bit */
	struct mutex    lock;
#ifdef CONFIG_CMA_DEBUGFS
	struct hlist_head mem_head;
	spinlock_t mem_head_lock;
#endif /* CONFIG_CMA_DEBUGFS */
};
#endif /* CONFIG_CMA */

/* Memory zone fino >>
 *	nvt_mem_info_t #0
 *					|--> nvt_fmem_mem_list_info_t #0
 *					|--> nvt_fmem_mem_list_info_t #1...
 *
 *	nvt_mem_info_t #1
 *					|--> nvt_fmem_mem_list_info_t #0
 *					|--> nvt_fmem_mem_list_info_t #1
 *					|--> nvt_fmem_mem_list_info_t #2...
 *	nvt_mem_info_t #2
 *					|--> nvt_fmem_mem_list_info_t #0
 *					|--> nvt_fmem_mem_list_info_t #1
 *					|--> nvt_fmem_mem_list_info_t #2
 *					|--> nvt_fmem_mem_list_info_t #3...
 *	....................
 *	nvt_mem_info_t #FFFFFFFF (Normal mem)
 */

/* proc function
 */
static struct proc_dir_entry *fmem_proc_root = NULL;

/*************************************************************************************************************
 * Memory manager handling
 *************************************************************************************************************/

static struct nvt_mem_info_t* fmem_memzone_get_info(unsigned int id)
{
	struct nvt_mem_info_t *curr_info, *next_info;

	down(&sema_memzone);
	list_for_each_entry_safe(curr_info, next_info, &nvt_memzone_info_list_root, list) {
        if (curr_info->id == id) {
			up(&sema_memzone);
			return curr_info;
		}
	}
	up(&sema_memzone);

	return NULL;
}

static struct nvt_mem_info_t* fmem_memzone_info_alloc(struct platform_device *pdev, unsigned int id)
{
	struct nvt_mem_info_t* memzone_info = NULL;

	if (id > MAX_CMA_AREAS && id != MEM_INFO_NORMAL_MEM) {
		dev_err(&pdev->dev, "fmem: Your cma or normal mem area 0x%08x is larger than %d or not equal 0x%08x \n", id, MAX_CMA_AREAS, MEM_INFO_NORMAL_MEM);
		return NULL;
	}

	memzone_info = fmem_memzone_get_info(id);
	if (memzone_info != NULL) {
		dev_err(&pdev->dev, "fmem: This area %u had already added\n", id);
		return NULL;
	}

	memzone_info = (struct nvt_mem_info_t*)kzalloc(sizeof(struct nvt_mem_info_t), GFP_KERNEL);
	if (memzone_info == NULL) {
		dev_err(&pdev->dev, "fmem: Your memory area %u can't be allocated\n", id);
		return NULL;
	}
	INIT_LIST_HEAD(&memzone_info->list);
	INIT_LIST_HEAD(&memzone_info->fmem_list_root);
	memzone_info->fmem_mem_cache = kmem_cache_create("fmem_mem_cache", sizeof(struct nvt_fmem_mem_list_info_t), 0, SLAB_PANIC, NULL);
	if (!memzone_info->fmem_mem_cache) {
		pr_err("%s: Allocating memory failed\n", __func__);
		return NULL;
	}
	memzone_info->id = id;
	/* This is default device name for this memory area */
	memzone_info->dev = &pdev->dev;

	return memzone_info;
}

static struct nvt_fmem_mem_list_info_t* fmem_get_mem_list_info(struct nvt_fmem_mem_list_info_t* my_fmem_mem_info, unsigned int id)
{
	struct nvt_fmem_mem_list_info_t *curr_info, *next_info;
	struct nvt_mem_info_t* memzone_info = NULL;

	if (id > MAX_CMA_AREAS && id != MEM_INFO_NORMAL_MEM) {
		pr_err("fmem: Your cma or normal mem area 0x%08x is larger than 0x%08x or not equal 0x%08x \n", id, MAX_CMA_AREAS, MEM_INFO_NORMAL_MEM);
		return NULL;
	}

	memzone_info = fmem_memzone_get_info(id);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This cma/normal mem area %u can't be used\n", __func__, id);
		return NULL;
	}

	down(&sema_mem_info);
	list_for_each_entry_safe(curr_info, next_info, &memzone_info->fmem_list_root, list) {
        if ((unsigned long)my_fmem_mem_info == (unsigned long)curr_info) {
			up(&sema_mem_info);
			if (curr_info->magic_num != NVT_FMEM_MEM_MAGIC) {
				panic("fmem/%s: Error magic number 0x%08lx\n", __func__, curr_info->magic_num);
			}
			return curr_info;
		}
	}
	up(&sema_mem_info);

	return NULL;
}

/*
 * Allocate a data structure to store mem info list from zone id
 */

static void* fmem_mem_info_alloc_from_memzone(struct nvt_fmem_mem_info_t *nvt_fmem_info, unsigned int id)
{
	struct nvt_mem_info_t* memzone_info = NULL;
	struct nvt_fmem_mem_list_info_t* my_fmem_meminfo = NULL;

	if (id > MAX_CMA_AREAS && id != MEM_INFO_NORMAL_MEM) {
		pr_err("fmem/%s: Your cma or normal mem area 0x%08x is larger than %d or not equal 0x%08x \n", __func__, id, MAX_CMA_AREAS, MEM_INFO_NORMAL_MEM);
		return NULL;
	}

	memzone_info = fmem_memzone_get_info(id);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This cma/normal mem area %u can't be used\n", __func__, id);
		return NULL;
	}

	/* Allocate a buffer to record memory info */
	my_fmem_meminfo = (struct nvt_fmem_mem_list_info_t *)kmem_cache_zalloc(memzone_info->fmem_mem_cache, GFP_KERNEL);
	if (!my_fmem_meminfo) {
		return NULL;
	}

	/* Copy memory info to managed data struct */
	memcpy(&my_fmem_meminfo->mem_info, nvt_fmem_info, sizeof(struct nvt_fmem_mem_info_t));
	INIT_LIST_HEAD(&my_fmem_meminfo->list);
	/* Add to list for the managed use */
	down(&sema_mem_info);
	list_add_tail(&my_fmem_meminfo->list, &memzone_info->fmem_list_root);
	up(&sema_mem_info);
	my_fmem_meminfo->magic_num = NVT_FMEM_MEM_MAGIC;

	return (void*)my_fmem_meminfo;
}

/*
 * Release a mem info list from zone id
 */

static int fmem_mem_info_release_from_memzone(struct nvt_fmem_mem_list_info_t *my_fmem_meminfo, unsigned int id)
{
	struct nvt_mem_info_t* memzone_info = NULL;
	struct nvt_fmem_mem_list_info_t* memlist_ret = NULL;

	if (id > MAX_CMA_AREAS && id != MEM_INFO_NORMAL_MEM) {
		pr_err("fmem/%s: Your cma or normal mem area 0x%08x is larger than %d or not equal 0x%08x \n", __func__, id, MAX_CMA_AREAS, MEM_INFO_NORMAL_MEM);
		return -1;
	}

	if (my_fmem_meminfo == NULL)
		return -1;

	memzone_info = fmem_memzone_get_info(id);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This cma/normal mem area %u can't be used\n", __func__, id);
		return -1;
	}
	/* To check if this info list is under memzone id */
	memlist_ret = fmem_get_mem_list_info(my_fmem_meminfo, id);
	if (memlist_ret == NULL) {
		pr_err("fmem/%s: We can't find this memory address 0x%08lx in id %u\n", __func__, (unsigned long)my_fmem_meminfo->mem_info.paddr, id);
		return -1;
	}

	if (my_fmem_meminfo->magic_num != NVT_FMEM_MEM_MAGIC) {
		pr_err("fmem/%s: Error magic number with %px id: %d\n", __func__, (void*)my_fmem_meminfo, id);
		return -1;
	}

	/* Copy memory info to managed data struct */
	memset(&my_fmem_meminfo->mem_info, 0, sizeof(struct nvt_fmem_mem_info_t));
	/* Delete list from fmem_list_root */
	down(&sema_mem_info);
	list_del(&my_fmem_meminfo->list);
	up(&sema_mem_info);
	/* Release a buffer to record memory info */
	kmem_cache_free(memzone_info->fmem_mem_cache, my_fmem_meminfo);

	return 0;
}

static void* fmem_mem_info_alloc(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	void *ret = NULL;
	ret = fmem_mem_info_alloc_from_memzone(nvt_fmem_info, MEM_INFO_NORMAL_MEM);

	return ret;
}

static int fmem_mem_info_release(struct nvt_fmem_mem_list_info_t *my_fmem_meminfo)
{
	int ret = -1;
	ret = fmem_mem_info_release_from_memzone(my_fmem_meminfo, MEM_INFO_NORMAL_MEM);

	return ret;
}

static int fmem_memzone_info_free(struct platform_device *pdev, unsigned int id)
{
	struct nvt_mem_info_t *curr_info, *next_info;

	if (id > MAX_CMA_AREAS && id != MEM_INFO_NORMAL_MEM) {
		dev_err(&pdev->dev, "fmem/%s: Your cma or normal mem area 0x%08x is larger than %d or not equal 0x%08x \n", __func__, id, MAX_CMA_AREAS, MEM_INFO_NORMAL_MEM);
		return -1;
	}

	down(&sema_memzone);
	list_for_each_entry_safe(curr_info, next_info, &nvt_memzone_info_list_root, list) {
		if (curr_info->id == id) {
			list_del(&curr_info->list);
			memset(curr_info, 0, sizeof(struct nvt_mem_info_t));
			kfree(curr_info);
			up(&sema_memzone);
			return 0;
		}
	}
	up(&sema_memzone);

	dev_err(&pdev->dev, "fmem: We can't free this id%u \n", id);
	return -1;
}

static int fmem_memzone_info_add(struct platform_device *pdev, struct nvt_mem_info_t* memzone_info)
{
	if (memzone_info == NULL) {
		dev_err(&pdev->dev, "fmem: We can't add this memory zone info because this is NULL\n");
		return -1;
	}

	down(&sema_memzone);
	list_add_tail(&memzone_info->list, &nvt_memzone_info_list_root);
	memzone_info->name = pdev->name;
	memzone_info->dev = &pdev->dev;
	up(&sema_memzone);

	return 0;
}

#ifdef CONFIG_CMA
static int nvt_cma_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct nvt_mem_info_t *memzone_info = NULL;
	unsigned int id = 0;


	/* pointer to shared-dma-pool in dts */
	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "memory reserved init failed with %d\n", ret);
		return ret;
	}

	of_property_read_u32(pdev->dev.of_node, "id",
				&id);

	memzone_info = fmem_memzone_info_alloc(pdev, id);
	if (memzone_info == NULL)
		return -1;

	memzone_info->cma = dev_get_cma_area(&pdev->dev);
	if (!memzone_info->cma) {
		dev_err(&pdev->dev, "fmem: cma id%u probe failed\n", id);
		return -1;
	} else {
		memzone_info->start = cma_get_base(memzone_info->cma);
		memzone_info->size = cma_get_size(memzone_info->cma);
	}

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
		dev_err(&pdev->dev, "To config cma region %d as coherent property error\n", id);
		return -EINVAL;
	}

	ret = fmem_memzone_info_add(pdev, memzone_info);
	if (ret < 0) {
		fmem_memzone_info_free(pdev, id);
		return -1;
	}

	dev_info(&pdev->dev, "fmem: cma area id: %u, cma base:0x%lx, size:0x%lx added\n", id, (unsigned long)memzone_info->start, (unsigned long)memzone_info->size);
	dev_info(&pdev->dev, "fmem/Version: %s\n", NVT_FMEM_VERSION);
	dev_info(&pdev->dev, "Probe successfully\n");

	return 0;
}

static const struct of_device_id nvt_cma_dt_match[] = {
	{.compatible = "nvt,nvt_cma"},
	{},
};
MODULE_DEVICE_TABLE(of, nvt_cma_dt_match);

static struct platform_driver nvt_cma_driver = {
	.probe = nvt_cma_probe,
	.driver = {
		.name = "nvt_cma",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nvt_cma_dt_match),
	},
};

static int __init nvt_cma_early_init(void)
{
	int ret = 0;

	INIT_LIST_HEAD(&nvt_memzone_info_list_root);
	ret = platform_driver_register(&nvt_cma_driver);
	if (ret < 0) {
		pr_err("%s: init failed\n", __func__);
	}

	return 0;
}
core_initcall(nvt_cma_early_init);
#endif /* CONFIG_CMA */

static int nvt_os_mem_probe(struct platform_device *pdev)
{
	struct nvt_mem_info_t *normal_mem_info = NULL;
	int ret = 0;

	normal_mem_info = fmem_memzone_info_alloc(pdev, MEM_INFO_NORMAL_MEM);
	if (normal_mem_info == NULL)
		return -1;

	ret = fmem_memzone_info_add(pdev, normal_mem_info);
	if (ret < 0) {
		fmem_memzone_info_free(pdev, MEM_INFO_NORMAL_MEM);
		return -1;
	}

	dev_info(&pdev->dev, "fmem/Version: %s\n", NVT_FMEM_VERSION);
	dev_info(&pdev->dev, "Probe successfully\n");

	return 0;
}

static const struct of_device_id nvt_os_mem_dt_match[] = {
	{.compatible = "nvt,nvt_os_mem"},
	{},
};
MODULE_DEVICE_TABLE(of, nvt_os_mem_dt_match);

static struct platform_driver nvt_os_mem_driver = {
	.probe = nvt_os_mem_probe,
	.driver = {
		.name = "nvt_os_mem",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nvt_os_mem_dt_match),
	},
};

static int __init nvt_os_mem_early_init(void)
{
	int ret = 0;

#ifndef CONFIG_CMA
	// This will do init if the cma is not enabled.
	INIT_LIST_HEAD(&nvt_memzone_info_list_root);
#endif
	ret = platform_driver_register(&nvt_os_mem_driver);
	if (ret < 0) {
		pr_err("%s: init failed\n", __func__);
	}

	return 0;
}
core_initcall(nvt_os_mem_early_init);

bool fmem_cma_is_valid(unsigned int index)
{
	struct nvt_mem_info_t *memzone_info = NULL;
	bool ret = false;

	down(&sema_mem_lock);
	memzone_info = fmem_memzone_get_info(index);
	if (memzone_info == NULL)
		ret = false;
	else
		ret = true;
	up(&sema_mem_lock);

	return ret;
}

/* CMA memory parts */

void* fmem_alloc_from_cma(struct nvt_fmem_mem_info_t *nvt_fmem_info, unsigned int index)
{
#ifdef CONFIG_CMA
	struct page *pages;
	void *vaddr = NULL;
	unsigned page_count = ALIGN(nvt_fmem_info->size, PAGE_SIZE)/PAGE_SIZE;
	void *handle = NULL;
	struct nvt_mem_info_t *memzone_info = NULL;
	struct device * dev = NULL;

	down(&sema_mem_lock);
	/* To get cma info by id */
	memzone_info = fmem_memzone_get_info(index);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This cma area %u can't be used\n", __func__, index);
		goto alloc_cma_exit;
	}
	dev = memzone_info->dev;

	if (!dev)
		goto alloc_cma_exit;

	if (page_count == 0)
		goto alloc_cma_exit;

	if (nvt_fmem_info->type == NVT_FMEM_ALLOC_CACHE) {
		pages = dma_alloc_from_contiguous(dev, page_count, 0, __GFP_NOWARN);
		if (pages != NULL) {
			nvt_fmem_info->paddr = __pfn_to_phys(page_to_pfn(pages));
			vaddr = __va(nvt_fmem_info->paddr);
		}
	} else if (nvt_fmem_info->type == NVT_FMEM_ALLOC_NONCACHE) {
		vaddr = dma_alloc_coherent(dev, nvt_fmem_info->size, &nvt_fmem_info->paddr,  GFP_KERNEL);
		page_count = 0;
		pages = NULL;
	} else {
		vaddr = dma_alloc_coherent(dev, nvt_fmem_info->size, &nvt_fmem_info->paddr,  GFP_KERNEL);
		page_count = 0;
		pages = NULL;
	}

	if (vaddr == NULL) {
		pr_err("%s:%d Can't to create this buffer with CMA area %d size:0x%lx\n", __func__, __LINE__, index, (unsigned long)nvt_fmem_info->size);
		goto alloc_cma_exit;
	}

	nvt_fmem_info->vaddr = vaddr;
	nvt_fmem_info->page_count = page_count;
	nvt_fmem_info->pages = pages;
	nvt_fmem_info->size = ALIGN(nvt_fmem_info->size, PAGE_SIZE);

	handle = fmem_mem_info_alloc_from_memzone(nvt_fmem_info, index);
	if (handle == NULL) {
		if (nvt_fmem_info->type == NVT_FMEM_ALLOC_CACHE) {
			dma_release_from_contiguous(dev, pages, page_count);
		} else if (nvt_fmem_info->type == NVT_FMEM_ALLOC_NONCACHE) {
			dma_free_coherent(dev, nvt_fmem_info->size, vaddr, nvt_fmem_info->paddr);
		} else {
			dma_free_coherent(dev, nvt_fmem_info->size, vaddr, nvt_fmem_info->paddr);
		}
		goto alloc_cma_exit;
	}

	up(&sema_mem_lock);
	return handle;

alloc_cma_exit:
	up(&sema_mem_lock);
	return NULL;
#else
	return NULL;
#endif /* CONFIG_CMA */
}

int fmem_release_from_cma(void *handle, unsigned int index)
{
#ifdef CONFIG_CMA
	struct nvt_fmem_mem_list_info_t* my_fmem_meminfo = NULL;
	int ret = 0;
	struct nvt_mem_info_t *memzone_info = NULL;
	struct device * dev = NULL;

	down(&sema_mem_lock);
	/* To get cma info by id */
	memzone_info = fmem_memzone_get_info(index);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This cma area %u can't be used\n", __func__, index);
		goto release_cma_exit;
	}
	dev = memzone_info->dev;

	if (!dev)
		goto release_cma_exit;

	my_fmem_meminfo = fmem_get_mem_list_info((struct nvt_fmem_mem_list_info_t*)handle, index);
	if (my_fmem_meminfo == NULL) {
		pr_err("fmem/%s: Error to release buffer with this handle %px\n", __func__, (void*)handle);
		goto release_cma_exit;
	}

	if (my_fmem_meminfo->mem_info.type == NVT_FMEM_ALLOC_CACHE) {
		dma_release_from_contiguous(dev, my_fmem_meminfo->mem_info.pages, my_fmem_meminfo->mem_info.page_count);
	} else if (my_fmem_meminfo->mem_info.type == NVT_FMEM_ALLOC_NONCACHE) {
		dma_free_coherent(dev, my_fmem_meminfo->mem_info.size, my_fmem_meminfo->mem_info.vaddr, my_fmem_meminfo->mem_info.paddr);
	} else {
		dma_free_coherent(dev, my_fmem_meminfo->mem_info.size, my_fmem_meminfo->mem_info.vaddr, my_fmem_meminfo->mem_info.paddr);
	}

	ret = fmem_mem_info_release_from_memzone(my_fmem_meminfo, index);
	if (ret < 0) {
		pr_err("fmem/%s: We can't release this memory info with id %u\n", __func__, index);
		goto release_cma_exit;
	}

	up(&sema_mem_lock);
	return 0;

release_cma_exit:
	up(&sema_mem_lock);
	return -1;
#else
	return -1;
#endif /* CONFIG_CMA */
}

/* Normal memory parts */

void* fmem_alloc_from_ker(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	struct page *pages;
	void *vaddr = NULL;
	void *handle = NULL;
	unsigned page_count = ALIGN(nvt_fmem_info->size, PAGE_SIZE)/PAGE_SIZE;
	struct device * dev = NULL;
	struct nvt_mem_info_t *memzone_info = NULL;
	size_t alloc_size = 0;

	down(&sema_mem_lock);
	/* To get cma info  */
	if (nvt_fmem_info->dev)
		dev = nvt_fmem_info->dev;
	else {
		memzone_info = fmem_memzone_get_info(MEM_INFO_NORMAL_MEM);
		if (memzone_info == NULL) {
			pr_err("fmem/%s: This noraml mem area can't be used\n", __func__);
			goto alloc_from_ker_exit;
		}

		dev = memzone_info->dev;
	}

	if (page_count == 0)
		goto alloc_from_ker_exit;

	if (nvt_fmem_info->type == NVT_FMEM_ALLOC_CACHE) {
		alloc_size = nvt_fmem_info->size;
		vaddr = kzalloc(alloc_size, GFP_KERNEL | GFP_DMA);
		if (!vaddr)
			goto alloc_from_ker_exit;
		else
			nvt_fmem_info->paddr = virt_to_phys(vaddr);
	} else if (nvt_fmem_info->type == NVT_FMEM_ALLOC_NONCACHE) {
		alloc_size = ALIGN(nvt_fmem_info->size, PAGE_SIZE);
		vaddr = dma_alloc_coherent(dev, alloc_size, &nvt_fmem_info->paddr,  GFP_KERNEL);
		page_count = 0;
		pages = NULL;
	} else {
		alloc_size = ALIGN(nvt_fmem_info->size, PAGE_SIZE);
		vaddr = dma_alloc_coherent(dev, alloc_size, &nvt_fmem_info->paddr,  GFP_KERNEL);
		page_count = 0;
		pages = NULL;
	}

	if (vaddr == NULL) {
		pr_err("%s:%d Can't to create this buffer with normal mem area\n", __func__, __LINE__);
		goto alloc_from_ker_exit;
	}

	nvt_fmem_info->vaddr = vaddr;
	nvt_fmem_info->page_count = page_count;
	nvt_fmem_info->pages = pages;
	nvt_fmem_info->size = alloc_size;

	handle = fmem_mem_info_alloc(nvt_fmem_info);
	if (handle == NULL) {
		if (nvt_fmem_info->type == NVT_FMEM_ALLOC_CACHE) {
			dma_release_from_contiguous(dev, pages, page_count);
		} else if (nvt_fmem_info->type == NVT_FMEM_ALLOC_NONCACHE) {
			dma_free_coherent(dev, nvt_fmem_info->size, vaddr, nvt_fmem_info->paddr);
		} else {
			dma_free_coherent(dev, nvt_fmem_info->size, vaddr, nvt_fmem_info->paddr);
		}
		goto alloc_from_ker_exit;
	}

	up(&sema_mem_lock);
	return handle;

alloc_from_ker_exit:
	up(&sema_mem_lock);
	return NULL;
}

int fmem_release_from_ker(void *handle)
{
	struct nvt_fmem_mem_list_info_t* my_fmem_meminfo = NULL;
	int ret = 0;
	struct nvt_mem_info_t *memzone_info = NULL;
	struct device * dev = NULL;

	down(&sema_mem_lock);
	/* To get cma info by id */
	memzone_info = fmem_memzone_get_info(MEM_INFO_NORMAL_MEM);
	if (memzone_info == NULL) {
		pr_err("fmem/%s: This noraml mem area can't be used\n", __func__);
		goto release_from_ker_exit;
	}
	dev = memzone_info->dev;

	if (!dev)
		goto release_from_ker_exit;

	my_fmem_meminfo = fmem_get_mem_list_info((struct nvt_fmem_mem_list_info_t*)handle, MEM_INFO_NORMAL_MEM);
	if (my_fmem_meminfo == NULL) {
		pr_err("fmem/%s: Error to release buffer with this handle %px\n", __func__, (void*)handle);
		goto release_from_ker_exit;
	}

	if (my_fmem_meminfo->mem_info.type == NVT_FMEM_ALLOC_CACHE) {
		kfree(my_fmem_meminfo->mem_info.vaddr);
	} else if (my_fmem_meminfo->mem_info.type == NVT_FMEM_ALLOC_NONCACHE) {
		dma_free_coherent(dev, my_fmem_meminfo->mem_info.size, my_fmem_meminfo->mem_info.vaddr, my_fmem_meminfo->mem_info.paddr);
	} else {
		dma_free_coherent(dev, my_fmem_meminfo->mem_info.size, my_fmem_meminfo->mem_info.vaddr, my_fmem_meminfo->mem_info.paddr);
	}

	ret = fmem_mem_info_release(my_fmem_meminfo);
	if (ret < 0) {
		pr_err("fmem/%s: We can't release this cma memory info\n", __func__);
		goto release_from_ker_exit;
	}

	up(&sema_mem_lock);
	return 0;

release_from_ker_exit:
	up(&sema_mem_lock);
	return -1;
}

/*************************************************************************************************************
 * fmem proc handling
 *************************************************************************************************************/

/* counter info
 */
static int proc_show_dbgcounter(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "\n");

	seq_printf(sfile, "L1 Cache line size: %d Bytes\n", CACHE_ALIGN_MASK);
	seq_printf(sfile, "linear cpu_addr flush: %d \n", cpuaddr_flush);
	seq_printf(sfile, "non-linear cpu_addr flush: %d \n", nonlinear_cpuaddr_flush);
	seq_printf(sfile, "vaddr not cache alignment: %d \n", va_not_64align);
	seq_printf(sfile, "len not cache alignment: %d \n", length_not_64align);
	seq_printf(sfile, "debug_counter = 0x%x \n\n", debug_counter);

	seq_printf(sfile, "DEBUG_WRONG_CACHE_API = 0x%x \n", DEBUG_WRONG_CACHE_API);
	seq_printf(sfile, "DEBUG_WRONG_CACHE_VA = 0x%x \n", DEBUG_WRONG_CACHE_VA);
	seq_printf(sfile, "DEBUG_WRONG_CACHE_LEN = 0x%x \n\n", DEBUG_WRONG_CACHE_LEN);

	seq_printf(sfile, "\rUsage:\n");
	seq_printf(sfile, "\r\tEnable: CACHE_API/CACHE_VA/CACHE_LEN \
			\t=> echo 7 > /proc/fmem/dbg_info\n");
	seq_printf(sfile, "\r\tEnable: CACHE_API/CACHE_VA \
			\t\t=> echo 3 > /proc/fmem/dbg_info\n");
	seq_printf(sfile, "\r\tEnable: CACHE_VA/CACHE_LEN \
			\t\t=> echo 6 > /proc/fmem/dbg_info\n");

	return 0;
}

/* debug counter */
static ssize_t proc_write_dbgcounter(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char value[30];

	if (copy_from_user(value, buffer, count))
		return 0;

	if (sscanf(value, "%x\n", &debug_counter) != 1)
		return 0;

	if (debug_counter > 7) {
		pr_err("%s: Your dbgcounter setting is wrong\n", __func__);
		return 0;
	}

	pr_info("debug counter: 0x%x \n", debug_counter);

	return count;
}

static int dbgcounter_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_dbgcounter, NULL);
}

static const struct proc_ops dbgcounter_proc_fops = {
	.proc_open		= dbgcounter_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release		= single_release,
	.proc_write		= proc_write_dbgcounter,
};

#ifdef CONFIG_STACKPROTECTOR
/* stack dbg */
static ssize_t proc_write_stack_dbg(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char value[30];

	if (copy_from_user(value, buffer, count))
		return 0;

	sscanf(value, "%d\n", &stack_dbg_value);
	pr_info("stack debug value: %d \n", stack_dbg_value);

	return count;
}

static int proc_show_stack_dbg(struct seq_file *m, void *v)
{
	return 0;
}

static int stack_dbg_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_stack_dbg, NULL);
}

static const struct proc_ops stack_proc_fops = {
	.proc_open		= stack_dbg_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
	.proc_write		= proc_write_stack_dbg,
};
#endif

static ssize_t proc_resolve_pa(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	unsigned char value[30];
	unsigned long vaddr;
	phys_addr_t paddr;

	if (copy_from_user(value, buffer, count))
		return 0;

	sscanf(value, "%lx\n", &vaddr);
	paddr = fmem_lookup_pa(vaddr);

	pr_info("Resolve vaddr: 0x%lx ---> paddr: 0x%lx \n", vaddr, (unsigned long)paddr);

	return count;
}

static int resolve_proc_show(struct seq_file *m, void *v)
{
	return 0;
}
static int resolve_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, resolve_proc_show, NULL);
}

static const struct proc_ops resolve_proc_fops = {
	.proc_open		= resolve_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release		= single_release,
	.proc_write		= proc_resolve_pa,
};

struct nvtcma_proc_list_cmainfo_t {
	struct seq_file *sfile;
	phys_addr_t		base;
	size_t			size;
};

#ifdef CONFIG_CMA
static int nvtcma_list_all_cma_info(struct cma *cma, void*data)
{
	struct nvtcma_proc_list_cmainfo_t *cmainfo = (struct nvtcma_proc_list_cmainfo_t *)data;
	unsigned long used = 0;
	u64 val = 0;

	if (cma_get_base(cma) == cmainfo->base) {
		mutex_lock(&cma->lock);
		/* pages counter is smaller than sizeof(int) */
		used = bitmap_weight(cma->bitmap, (int)cma->count);
		mutex_unlock(&cma->lock);
		val = (u64)used << cma->order_per_bit;
		if (cmainfo->size != cma_get_size(cma))
			seq_printf(cmainfo->sfile, "You used the dma api directly, not from nvt fmem api!!!\n");
		seq_printf(cmainfo->sfile, "CMA managed >> Used/Total size: %lu(bytes)/%lu(bytes), %llu(pages)\n", (used * PAGE_SIZE), cma_get_size(cma), val);
	}

	return 0;
}
#endif /* CONFIG_CMA */

static int nvt_memzone_info_proc_show(struct seq_file *sfile, void *v)
{
	struct nvt_mem_info_t *curr_info, *next_info;
	struct nvt_fmem_mem_list_info_t *curr_mem_info, *next_mem_info;
	#ifdef CONFIG_CMA
	struct nvtcma_proc_list_cmainfo_t proc_cmainfo;
	#endif /* CONFIG_CMA */
	size_t total_used_size = 0;

	seq_printf(sfile, "================= NVT MEM INFO =================\n");
	down(&sema_memzone);
	list_for_each_entry_safe(curr_info, next_info, &nvt_memzone_info_list_root, list) {
		if (curr_info->size != 0 && curr_info->start != 0) {
			seq_printf(sfile, "\r\tArea:%d Name: %s Total size: %ld\n", curr_info->id, curr_info->name, (unsigned long)curr_info->size);
			seq_printf(sfile, "\r\tphysical address: 0x%08lx@0x%08lx \n", (unsigned long)curr_info->size, (unsigned long)curr_info->start);
		} else
			seq_printf(sfile, "\r\tArea:%d Name: %s \n", curr_info->id, curr_info->name);

		down(&sema_mem_info);
		list_for_each_entry_safe(curr_mem_info, next_mem_info, &curr_info->fmem_list_root, list) {
			seq_printf(sfile, "\r\t\t Device_Name: %s, Type: %s, Physical addr:0x%lx, Virtual addr: 0x%lx, size: %ld\n",
															(curr_mem_info->mem_info.dev != NULL)?curr_mem_info->mem_info.dev->init_name:"",
															(curr_mem_info->mem_info.type == NVT_FMEM_ALLOC_CACHE)?"NVT_FMEM_ALLOC_CACHE":
															(curr_mem_info->mem_info.type == NVT_FMEM_ALLOC_NONCACHE)?"NVT_FMEM_ALLOC_NONCACHE":
															(curr_mem_info->mem_info.type == NVT_FMEM_ALLOC_BUFFER)?"NVT_FMEM_ALLOC_BUFFER":"ERROR Type",
															(unsigned long)curr_mem_info->mem_info.paddr,
															(unsigned long)curr_mem_info->mem_info.vaddr,
															(unsigned long)curr_mem_info->mem_info.size);
			total_used_size += curr_mem_info->mem_info.size;
		}
		#ifdef CONFIG_CMA
		proc_cmainfo.sfile = sfile;
		proc_cmainfo.base = curr_info->start;
		proc_cmainfo.size = curr_info->size;
		cma_for_each_area(nvtcma_list_all_cma_info, (void*)&proc_cmainfo);
		#endif /* CONFIG_CMA */
		/* Only list cma with addr and size are not equal 0 */
		if (curr_info->size != 0 && curr_info->start != 0)
			seq_printf(sfile, "Fmem managed >> Used/Total size: %ld(bytes)/%ld(bytes)\n\n", (unsigned long)total_used_size, (unsigned long)curr_info->size);
		else
			seq_printf(sfile, "Fmem managed >> Used size: %ld(bytes)\n\n", (unsigned long)total_used_size);

		total_used_size = 0;
		up(&sema_mem_info);
	}
	up(&sema_memzone);

	return 0;
}
static int nvt_memzone_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_memzone_info_proc_show, NULL);
}

static const struct proc_ops nvt_memzone_info_proc_fops = {
	.proc_open		= nvt_memzone_info_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};

static int version_proc_show(struct seq_file *sfile, void *v)
{
	seq_printf(sfile, "Version: %s\n", NVT_FMEM_VERSION);

	return 0;
}
static int version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, version_proc_show, NULL);
}

static const struct proc_ops version_proc_fops = {
	.proc_open		= version_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};

static int __init fmem_proc_init(void)
{
	struct proc_dir_entry *p;
	static struct proc_dir_entry *myproc = NULL;

	p = proc_mkdir("fmem", NULL);
	if (p == NULL) {
		pr_err("%s, fail! \n", __func__);
		return -1;
	}

	fmem_proc_root = p;

	/* resolve, va->pa
	*/
	myproc = proc_create("resolve", S_IRUGO, fmem_proc_root, &resolve_proc_fops);
	if (myproc == NULL)
		panic("FMEM: Fail to create proc resolve_proc!\n");

	/* CMA info */
	myproc = proc_create("memzone_info", S_IRUGO, fmem_proc_root, &nvt_memzone_info_proc_fops);
	if (myproc == NULL)
		panic("FMEM: Fail to create proc cma info!\n");

	/* Version info */
	myproc = proc_create("version", S_IRUGO, fmem_proc_root, &version_proc_fops);
	if (myproc == NULL)
		panic("FMEM: Fail to create proc version info!\n");

	/* counter */
	myproc = proc_create("dbg_info", S_IRUGO, fmem_proc_root, &dbgcounter_proc_fops);
	if (myproc == NULL)
		panic("FMEM: Fail to create proc dbg_info!\n");

#ifdef CONFIG_STACKPROTECTOR
	/* stack msg */
	myproc = proc_create("stack_msg", S_IRUGO, fmem_proc_root, &stack_proc_fops);
	if (myproc == NULL)
		panic("FMEM: Fail to create proc stack_msg!\n");
#endif

	return 0;
}

static const struct of_device_id fmem_dt_match[] = {
	{.compatible = "nvt,nvt_fmem" },
	{},
};

static int fmem_drv_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;

	match = of_match_device(fmem_dt_match, &pdev->dev);
	if (!match) {
		pr_err("FMEM: fmem platform device not found \n");
		return -EINVAL;
	}

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
		pr_err("FMEM: No suitable DMA available\n");
		return -EINVAL;
	}

	pfmem_dev = pdev;

	return 0;
}

static struct platform_driver fmem_platform_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = FMEM_DEV_NAME,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(fmem_dt_match),
#endif
	},
	.probe	= fmem_drv_probe,
};

static int __init nvt_fmem_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&fmem_platform_driver);
	if (ret < 0) {
		pr_err("%s: register failed\n", __func__);
		goto ERR_OUT;
	}

	ret = fmem_proc_init();
	if (ret < 0) {
		pr_err("%s: proc failed\n", __func__);
		goto ERR_OUT;
	}

ERR_OUT:
	return ret;
}

#if defined(CONFIG_PLAT_NOVATEK)
/**
 * @brief to resolve the virtual address (including direct mapping, ioremap or user space address to
 *      its real physical address.
 * @note This API is used by ptdump to convert va of a process depending on its mm_struct pointer.
 * @parm vaddr indicates any virtual address
 * @parm mm is the process mm
 * @parm ret_pa is returned pa if succeeded
 *
 * @return On error, -1UL is returned, or a physical address is returned.
 */
phys_addr_t fmem_ptdump_v2p(uintptr_t vaddr, struct mm_struct *mm)
{
	pgd_t *pgdp;
	p4d_t *p4dp;
	pmd_t *pmdp;
	pud_t *pudp;
	pte_t *ptep;
	phys_addr_t paddr = -1UL;
	struct vm_struct *area = NULL;
	unsigned long vaddr_off = 0;

	/* for speed up */
	if (virt_addr_valid(vaddr)) {
		return (phys_addr_t)__pa(vaddr);
	}

	/* find static mapping address */
	if (vaddr >= FIXADDR_START && vaddr < FIXADDR_TOP) {
		return (phys_addr_t)virt_to_fix(vaddr);
	}

        /* for speed up */
	if (virt_addr_valid(vaddr)) {
		return (phys_addr_t)__pa(vaddr);
	}

	/*
	* The pgd is never bad, and a pmd always exists(as it's folded into the pgd entry)
	*/
	if (vaddr >= TASK_SIZE) { /* use init_mm as mmu to look up in kernel mode */
		pgdp = pgd_offset_k(vaddr);
	} else if (mm){
		pgdp = pgd_offset(mm, vaddr);
	} else {
		pr_err("Invalid va 0x%lx\r\n", (unsigned long)vaddr);
		dump_stack();
		return paddr;
	}

	if (pgd_none((*pgdp))) {
		pr_err("fmem: 0x%lx not mapped in pgd table! \n", (unsigned long)vaddr);
		goto err;
	}

	/* Using 5-level MMU translation table formula, Here we are in order to meet generic Linux
	* version, pud is listed here.
	*/
	p4dp = p4d_offset(pgdp, vaddr);
	if (p4d_none((*p4dp))) {
		pr_err("fmem: 0x%lx not mapped in p4d table! \n", (unsigned long)vaddr);
		goto err;
	}

	pudp = pud_offset(p4dp, vaddr);
	if (pud_none((*pudp))) {
		pr_err("fmem: 0x%lx not mapped in pud table! \n", (unsigned long)vaddr);
		goto err;
	}

	pmdp = pmd_offset(pudp, vaddr);
	/* A pmdp virtual address checking needs to be added here, some cases will get pmdp invalid address. */
	if (virt_addr_valid((unsigned long)pmdp) && pmd_none((*pmdp))) {
		pr_err("fmem: 0x%lx not mapped in pmd 0x%lx! \n", (unsigned long)vaddr, (unsigned long)pmdp);
		goto err;
	}

	if (!pmd_bad(*pmdp)) {
		/* page mapping */
		ptep = pte_offset_kernel(pmdp, vaddr);
		if (pte_none(*ptep)) {
			//pr_err("fmem: 0x%lx not mapped in pte! \n", (unsigned long)vaddr);
			//skip error messages, to prevent ptdump messages chaos
			goto err;
		}
		paddr = (pte_pfn(*ptep) << PAGE_SHIFT) + (vaddr & (PAGE_SIZE - 1));
	} else {
		/* This is to handle the ioremap va to pa translation */
		if (in_irq()) {
			pr_err("fmem: You could not use this api %s in interrupt context!!!!!!\n", __func__);
			dump_stack();
			goto err;
		}

		/******************************
		 * VM window: 	--------- phys_addr
		 * 		|	|
		 * 		|	|
		 * 		---------
		 *
		 * paddr = window_phys_addr + va_offset
		 * Here: PMD_SIZE is the va offset calculated minimum size. (arch/arm64/include/asm/pgtable-hwdef.h)
		 *
		 * *****************************/
		area = find_vm_area((void*)vaddr);
		if (!area) {
			pr_err("fmem: can't find this mapping with %lx\n", (unsigned long)vaddr);
			goto err;
		}

		vaddr_off = vaddr - (unsigned long)area->addr;
		if (area->size < vaddr_off) {
			pr_err("fmem: Your va 0x%08lx is larger than the translation window size\n", (unsigned long)vaddr);
			goto err;
		}
		paddr = area->phys_addr + (vaddr_off & PMD_MASK) + (vaddr & ~PMD_MASK);
	}
err:
	return paddr;
}
#endif //#if defined(CONFIG_PLAT_NOVATEK)

phys_addr_t __fmem_lookup_pa_vm_area(uintptr_t vaddr)
{
	phys_addr_t paddr = -1UL;
	struct vm_struct *area = NULL;
	unsigned long vaddr_off = 0;

	/* This is to handle the ioremap va to pa translation */
	if (in_irq()) {
		pr_err("fmem: You could not use this api %s in interrupt context!!!!!!\n", __func__);
		dump_stack();
		return paddr;
	}

	/******************************
	 * VM window: 	--------- phys_addr
	 * 		|	|
	 * 		|	|
	 * 		---------
	 *
	 * paddr = window_phys_addr + va_offset
	 * Here: PMD_SIZE is the va offset calculated minimum size. (arch/arm64/include/asm/pgtable-hwdef.h)
	 *
	 * *****************************/
	area = find_vm_area((void*)vaddr);
	if (!area) {
		pr_err("fmem: can't find this mapping with %lx\n", (unsigned long)vaddr);
		return paddr;
	}

	vaddr_off = vaddr - (unsigned long)area->addr;
	if (area->size < vaddr_off) {
		pr_err("fmem: Your va 0x%08lx is larger than the translation window size\n", (unsigned long)vaddr);
		return paddr;
	}
	paddr = area->phys_addr + (vaddr_off & PMD_MASK) + (vaddr & ~PMD_MASK);
	return paddr;
}

/**
 * @brief to resolve the virtual address (including direct mapping, ioremap or user space address to
 *      its real physical address.
 *
 * @parm vaddr indicates any virtual address
 *
 * @return  >= 0 for success, -1UL(a32: 0xffffffff, a64: 0xffffffff_ffffffff) for fail
 */
phys_addr_t fmem_lookup_pa(uintptr_t vaddr)
{
	pgd_t *pgdp;
	p4d_t *p4dp;
	pmd_t *pmdp;
	pud_t *pudp;
	pte_t *ptep;
	phys_addr_t paddr = -1UL;

	#if defined(__arm__)
	/* check if va is module space global variable */
	if (vaddr >= TASK_SIZE && vaddr < PAGE_OFFSET) {
		pr_err("Invalid va 0x%lx, TASK_SIZE = 0x%lx, PAGE_OFFSET = 0x%lx\r\n", (unsigned long)vaddr, (unsigned long)TASK_SIZE, (unsigned long)PAGE_OFFSET);
		dump_stack();
		return paddr;
	}
	#endif /* __arm__ */
	/* for speed up */
	if (virt_addr_valid(vaddr)) {
		return (phys_addr_t)__pa(vaddr);
	}

	/*
	* The pgd is never bad, and a pmd always exists(as it's folded into the pgd entry)
	*/
	if (vaddr >= TASK_SIZE) { /* use init_mm as mmu to look up in kernel mode */
		pgdp = pgd_offset_k(vaddr);
	} else if (current && current->mm){
		pgdp = pgd_offset(current->mm, vaddr);
	} else {
		pr_err("Invalid va 0x%lx\r\n", (unsigned long)vaddr);
		dump_stack();
		return paddr;
	}

	if (pgd_none((*pgdp))) {
		pr_err("fmem: 0x%lx not mapped in pgd table! \n", (unsigned long)vaddr);
		goto err;
	}

	/* Using 5-level MMU translation table formula, Here we are in order to meet generic Linux
	* version, pud is listed here.
	*/
	p4dp = p4d_offset(pgdp, vaddr);
	if (p4d_none((*p4dp))) {
		pr_err("fmem: 0x%lx not mapped in p4d table! \n", (unsigned long)vaddr);
		goto err;
	}

	pudp = pud_offset(p4dp, vaddr);
	if (pud_none((*pudp))) {
		pr_err("fmem: 0x%lx not mapped in pud table! \n", (unsigned long)vaddr);
		goto err;
	}

	pmdp = pmd_offset(pudp, vaddr);
	/* A pmdp virtual address checking needs to be added here, some cases will get pmdp invalid address. */
	if (!virt_addr_valid((unsigned long)pmdp)) {
		paddr = __fmem_lookup_pa_vm_area(vaddr);
	} else if (virt_addr_valid((unsigned long)pmdp) && pmd_none((*pmdp))) {
		pr_err("fmem: 0x%lx not mapped in pmd 0x%lx! \n", (unsigned long)vaddr, (unsigned long)pmdp);
		goto err;
	} else if (!pmd_bad(*pmdp)) {
		/* page mapping */
		ptep = pte_offset_kernel(pmdp, vaddr);
		if (pte_none(*ptep)) {
			pr_err("fmem: 0x%lx not mapped in pte! \n", (unsigned long)vaddr);
			goto err;
		}
		paddr = (pte_pfn(*ptep) << PAGE_SHIFT) + (vaddr & (PAGE_SIZE - 1));
	} else {
		paddr = __fmem_lookup_pa_vm_area(vaddr);
	}
err:
	return paddr;
}

/* @this function is a data cache operation function,
 * @parm: vaddr: any virtual address
 * @parm: dir will be:
 * DMA_BIDIRECTIONAL = 0, it means flush operation.
 * DMA_TO_DEVICE = 1, it means clean operation.
 * DMA_FROM_DEVICE = 2, it means invalidate operation.
 * DMA_NONE = 3,
 */
void fmem_dcache_sync(void *vaddr, size_t len, enum dma_data_direction dir)
{
	struct device *dev = &pfmem_dev->dev;
	#ifdef CONFIG_OUTER_CACHE
	phys_addr_t paddr = fmem_lookup_pa((uintptr_t)vaddr);
	#endif

	if (!valid_dma_direction(dir))
		panic("%s, invalid direction: %d \n", __func__, dir);

	/* kernel buffer may not cache line alignment, it only can use both clean/inv
	* for safe. Others we regard them as warning cases in coding.
	*/
	if (dir != DMA_BIDIRECTIONAL) {
		if (!IS_ALIGNED((uintptr_t)vaddr, CACHE_ALIGN_MASK)) {
			va_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_VA)) {
				pr_err("%s, warning, vaddr: 0x%lx not cache alignment! \n", __func__, (unsigned long)vaddr);
				dump_stack();
			}
		}

		if (!IS_ALIGNED(len, CACHE_ALIGN_MASK)) {
			length_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_LEN)) {
				pr_err("%s, warning, len: %ld not cache alignment! \n", __func__, (unsigned long)len);
				dump_stack();
			}
		}
	}


	if (virt_addr_valid(vaddr) && virt_addr_valid(vaddr + len - 1)) {
		/* Notice:
		* The following code only for the direct mapping VA
		*/

		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_TO_DEVICE:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}
		cpuaddr_flush ++;
	} else {
		/* Notice:
		* We must process outer cache if have. The code is not implement yet!
		*/
		switch (dir) {
		case DMA_BIDIRECTIONAL:
#ifdef __aarch64__
			__dma_flush_area(vaddr, len);
#else
			dmac_flush_range(vaddr, vaddr + len);
			#ifdef CONFIG_OUTER_CACHE
			outer_flush_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_TO_DEVICE:
#ifdef __aarch64__
			__dma_map_area(vaddr, len, DMA_TO_DEVICE);
#else
			dmac_map_area(vaddr, len, DMA_TO_DEVICE);
			#ifdef CONFIG_OUTER_CACHE
			outer_clean_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_FROM_DEVICE:
#ifdef __aarch64__
			__dma_unmap_area(vaddr, len, DMA_FROM_DEVICE);
#else
			dmac_map_area(vaddr, len, DMA_FROM_DEVICE);
			#ifdef CONFIG_OUTER_CACHE
			outer_inv_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_NONE:
			break;
		}
		nonlinear_cpuaddr_flush ++;
		if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_API)) {
			pr_err("%s, warning, memory addr 0x%lx not direct mapped! \n", __func__, (unsigned long)vaddr);
			dump_stack();
		}
	}
}


void fmem_dcache_sync_vb(void *vaddr, size_t len, enum dma_data_direction dir)
{
	struct device *dev = &pfmem_dev->dev;
	#ifdef CONFIG_OUTER_CACHE
	phys_addr_t paddr = fmem_lookup_pa((uintptr_t)vaddr);
	#endif

	if (0 == vb_cache_flush_en){
		return;
	}
	if (!valid_dma_direction(dir))
		panic("%s, invalid direction: %d \n", __func__, dir);

	/* kernel buffer may not cache line alignment, it only can use both clean/inv
	* for safe. Others we regard them as warning cases in coding.
	*/
	if (dir != DMA_BIDIRECTIONAL) {
		if (!IS_ALIGNED((uintptr_t)vaddr, CACHE_ALIGN_MASK)) {
			va_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_VA)) {
				pr_err("%s, warning, vaddr: 0x%lx not cache alignment! \n", __func__, (unsigned long)vaddr);
				dump_stack();
			}
		}

		if (!IS_ALIGNED(len, CACHE_ALIGN_MASK)) {
			length_not_64align ++;
			if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_LEN)) {
				pr_err("%s, warning, len: %ld not cache alignment! \n", __func__, (unsigned long)len);
				dump_stack();
			}
		}
	}

	if (virt_addr_valid(vaddr) && virt_addr_valid(vaddr + len - 1)) {
		/* Notice:
		* The following code only for the direct mapping VA
		*/
		switch (dir) {
		case DMA_BIDIRECTIONAL:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_TO_DEVICE:
			dma_map_single(dev, vaddr, len, DMA_TO_DEVICE);
			break;
		case DMA_FROM_DEVICE:
			/* v7_dma_unmap_area is only valid for DMA_FROM_DEVICE */
			dma_unmap_single(dev, __pa(vaddr), len, DMA_FROM_DEVICE);
			break;
		case DMA_NONE:
			break;
		}
		cpuaddr_flush ++;
	} else {
		/* Notice:
		* We must process outer cache if have. The code is not implement yet!
		*/
		switch (dir) {
		case DMA_BIDIRECTIONAL:
#ifdef __aarch64__
			__dma_flush_area(vaddr, len);
#else
			dmac_flush_range(vaddr, vaddr + len);
			#ifdef CONFIG_OUTER_CACHE
			outer_flush_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_TO_DEVICE:
#ifdef __aarch64__
			__dma_map_area(vaddr, len, DMA_TO_DEVICE);
#else
			dmac_map_area(vaddr, len, DMA_TO_DEVICE);
			#ifdef CONFIG_OUTER_CACHE
			outer_clean_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_FROM_DEVICE:
#ifdef __aarch64__
			__dma_unmap_area(vaddr, len, DMA_FROM_DEVICE);
#else
			dmac_map_area(vaddr, len, DMA_FROM_DEVICE);
			#ifdef CONFIG_OUTER_CACHE
			outer_inv_range(paddr, paddr + len);
			#endif
#endif
			break;
		case DMA_NONE:
			break;
		}
		nonlinear_cpuaddr_flush ++;
		if (debug_counter & (0x1 << DEBUG_WRONG_CACHE_API)) {
			pr_err("%s, warning, memory addr 0x%lx not direct mapped! \n", __func__, (unsigned long)vaddr);
			dump_stack();
		}
	}
}

core_initcall(nvt_fmem_init);

extern void v7_flush_kern_cache_all(void);

void fmem_cache_flush_all(unsigned int cache_op_cpu_id)
{
#if defined(CONFIG_NVT_DCACHE_FLUSH_ALL)
	unsigned int cpu_id;
	unsigned long flags;

	/* We should check the cache operation id firstly. */
	if (cache_op_cpu_id > total_num_cpu) {
		dev_err(&pfmem_dev->dev, "%s: error to use clean all function, your cache op cpu id %d \
				is larger than total cpu cores %d\n", \
				__func__, cache_op_cpu_id, total_num_cpu);
		return;
	}

	/* The Cache clean all function needs to be called in the same cpu cores,
	 * We add a checking here to avoid you have wrong cache operation */
	cpu_id = smp_processor_id();
	if (cpu_id != cache_op_cpu_id) {
		dev_err(&pfmem_dev->dev, "%s: error to use clean all function, cpu id %d, cache op cpu id is %d\n",
							__func__, cpu_id, cache_op_cpu_id);
		return;
	}
	spin_lock_irqsave(&clean_all_lock, flags);
	#if defined(CONFIG_ARM64)
	__dma_flush_all();
	#elif defined(CONFIG_ARM)
	v7_flush_kern_cache_all();
	#else
	dev_err(&pfmem_dev->dev, "fmem: Not support this function\n");
	#endif /* CONFIG_ARM64 */
	spin_unlock_irqrestore(&clean_all_lock, flags);
#else
	dev_err(&pfmem_dev->dev, "fmem: Not support this function\n");
#endif /* CONFIG_NVT_DCACHE_FLUSH_ALL */
	return;
}

EXPORT_SYMBOL(fmem_alloc_from_cma);
EXPORT_SYMBOL(fmem_release_from_cma);
EXPORT_SYMBOL(fmem_lookup_pa);
EXPORT_SYMBOL(fmem_dcache_sync);
EXPORT_SYMBOL(fmem_dcache_sync_vb);
EXPORT_SYMBOL(fmem_cma_is_valid);
EXPORT_SYMBOL(fmem_alloc_from_ker);
EXPORT_SYMBOL(fmem_release_from_ker);
EXPORT_SYMBOL(fmem_cache_flush_all);
