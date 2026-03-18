/**
    NVT hdal memory operation handling
    Add a wrapper to handle the fmem memory handling api and version management API
    @file       nvtmem.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_fdt.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/soc/nvt/fmem.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/soc/nvt/nvtmem.h>
#include <linux/soc/nvt/nvtmem_if.h>
#include "nvtmem_dbg.h"
#include "nvtmem_inc.h"

/* Global variable
 */
unsigned int nvt_frammap_debug_level = NVT_FRAMMAP_DBG_ERR | NVT_FRAMMAP_DBG_MSG | NVT_FRAMMAP_DBG_WRN/* | NVT_FRAMMAP_DBG_IND*/;
static struct ddr_alloc_list ddr_alloc_data[MAX_DDR_NR];
static DEFINE_SEMAPHORE(sema_ver);
static struct list_head 	ver_list_root;
static struct kmem_cache	*ver_cache;
static ddr_chip_map_t		ddr_chip_map[MAX_DDR_NR];
struct nvtmem_hdal_base		hdal_base;

/* To define /dev/nvtmemN device data structure. */
#define NVTMEM_MEM_DEV_MINOR		(0)
#define NVTMEM_MEM_DEV_COUNT		(1)
#define NVTMEM_MEM_DEV_NAME		"nvtmem"
#define NVTMEM_MEM_DRV_NAME		"nvtmem_drv"
#define NVTMEM_MEM_CLASS_NAME		"nvtmem_class"
struct nvtmem_mem_dev_t {
	dev_t nvt_mem_dev_num;
	struct cdev nvtmem_cdev;
	struct class *nvtmem_class;
	struct device *nvtmem_dev;
	struct file_operations nvtmem_mem_fops;
};
static struct nvtmem_mem_dev_t *nvtmem_mem_dev_ptr;

/* proc function
 */
static struct proc_dir_entry *nvtmem_proc_root = NULL;
static struct proc_dir_entry *nvtmem_proc_ver = NULL;
static struct proc_dir_entry *nvtmem_proc_dram_map = NULL;

void *nvtmem_alloc_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	/* Allocate from first cma area */
	return fmem_alloc_from_cma(nvt_fmem_info, 0);
}
EXPORT_SYMBOL(nvtmem_alloc_buffer);

int nvtmem_release_buffer(void *handle)
{
	/* Release first cma area */
	return fmem_release_from_cma(handle, 0);
}
EXPORT_SYMBOL(nvtmem_release_buffer);

void *nvtmem_alloc_dsp_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	/* Allocate from 2nd cma area for DSP1 */
	return fmem_alloc_from_cma(nvt_fmem_info, 1);
}
EXPORT_SYMBOL(nvtmem_alloc_dsp_buffer);

int nvtmem_release_dsp_buffer(void *handle)
{
	/* Release 2nd cma area for DSP1 */
	return fmem_release_from_cma(handle, 1);
}
EXPORT_SYMBOL(nvtmem_release_dsp_buffer);

void *nvtmem_alloc_dsp2_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	/* Allocate from 3rd cma area for DSP2 */
	return fmem_alloc_from_cma(nvt_fmem_info, 2);
}
EXPORT_SYMBOL(nvtmem_alloc_dsp2_buffer);

int nvtmem_release_dsp2_buffer(void *handle)
{
	/* Release 3rd cma area for DSP2 */
	return fmem_release_from_cma(handle, 2);
}
EXPORT_SYMBOL(nvtmem_release_dsp2_buffer);

void *nvtmem_alloc_mcu_buffer(struct nvt_fmem_mem_info_t *nvt_fmem_info)
{
	/* Allocate from 4rd cma area for mcu */
	return fmem_alloc_from_cma(nvt_fmem_info, 3);
}
EXPORT_SYMBOL(nvtmem_alloc_mcu_buffer);

int nvtmem_release_mcu_buffer(void *handle)
{
	/* Release 4rd cma area for mcu */
	return fmem_release_from_cma(handle, 3);
}
EXPORT_SYMBOL(nvtmem_release_mcu_buffer);

phys_addr_t nvtmem_va_to_pa(uintptr_t vaddr)
{
	return fmem_lookup_pa(vaddr);
}
EXPORT_SYMBOL(nvtmem_va_to_pa);

static struct nvtmem_ver_info_t *nvtmem_get_ver_info(struct platform_device *pdev, unsigned int id)
{
	struct nvtmem_ver_info_t *curr_info, *next_info;

	down(&sema_ver);
	list_for_each_entry_safe(curr_info, next_info, &ver_list_root, list) {
		if ((unsigned long)curr_info->pdev == (unsigned long)pdev && curr_info->id == id) {
			up(&sema_ver);
			return curr_info;
		}
	}
	up(&sema_ver);

	return NULL;
}

/*
 * Version control sys.
 */

static struct nvtmem_ver_info_t *nvtmem_ver_info_alloc(struct platform_device *pdev)
{
	struct nvtmem_ver_info_t *ver_info = NULL;

	/* To get cma info by id */
	ver_info = nvtmem_get_ver_info(pdev, pdev->id);
	if (ver_info != NULL) {
		pr_err("nvtmem/%s: This device %s is already added\n", __func__, pdev->name);
		return NULL;
	}

	/* Allocate a buffer to store version info */
	ver_info = (struct nvtmem_ver_info_t *)kmem_cache_zalloc(ver_cache, GFP_KERNEL);
	if (!ver_info) {
		pr_err("nvtmem/%s: This device %s can't allocate buffer\n", __func__, pdev->name);
		return NULL;
	}

	/* Basic init */
	INIT_LIST_HEAD(&ver_info->list);
	ver_info->pdev = pdev;
	ver_info->id = pdev->id;

	return ver_info;
}

static int nvtmem_add_ver_info(struct platform_device *pdev, struct nvtmem_ver_info_t *ver_info, char *ver)
{
	int ret = 0;
	if (ver_info == NULL) {
		dev_err(&pdev->dev, "nvtmem/%s: We can't add this ver_info because this is NULL\n", __func__);
		return -1;
	}

	down(&sema_ver);
	/* To transform version to info struct */
	ret = sscanf(ver, "%u.%u.%u.%u", &ver_info->version,
		     &ver_info->patch_level,
		     &ver_info->sub_level,
		     &ver_info->extra_ver);
	if (ret != 4) {
		kmem_cache_free(ver_cache, ver_info);
		up(&sema_ver);
		return -1;
	}

	list_add_tail(&ver_info->list, &ver_list_root);
	up(&sema_ver);

	return 0;
}

static int nvtmem_ver_info_free(struct platform_device *pdev)
{
	struct nvtmem_ver_info_t *curr_info, *next_info;

	down(&sema_ver);
	list_for_each_entry_safe(curr_info, next_info, &ver_list_root, list) {
		if ((unsigned long)curr_info->pdev == (unsigned long)pdev && curr_info->id == pdev->id) {
			list_del(&curr_info->list);
			memset(curr_info, 0, sizeof(struct nvtmem_ver_info_t));
			kmem_cache_free(ver_cache, curr_info);
			up(&sema_ver);
			return 0;
		}
	}
	up(&sema_ver);

	dev_err(&pdev->dev, "nvtmem/%s: We can't free this device %s \n", __func__, pdev->name);
	return -1;
}

int nvtmem_version_register(struct platform_device *pdev, char *version)
{
	int ret = 0;
	struct nvtmem_ver_info_t *ver_info = NULL;

	if (version == NULL || pdev == NULL)
		return -1;

	ver_info = nvtmem_ver_info_alloc(pdev);
	if (ver_info == NULL)
		return -ENOMEM;

	/* It will release this buffer if add failed */
	ret = nvtmem_add_ver_info(pdev, ver_info, version);
	if (ret < 0) {
		dev_err(&pdev->dev, "nvtmem/%s: Error to add version with %s\n", __func__, version);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(nvtmem_version_register);

int nvtmem_version_unregister(struct platform_device *pdev)
{
	int ret = 0;

	if (pdev == NULL)
		return -1;

	ret = nvtmem_ver_info_free(pdev);
	if (ret < 0)
		return -1;

	return 0;
}
EXPORT_SYMBOL(nvtmem_version_unregister);

static int nvtmem_ver_sys_init(void)
{
	INIT_LIST_HEAD(&ver_list_root);
	ver_cache = kmem_cache_create("nvtmem_ver", sizeof(struct nvtmem_ver_info_t), 0, SLAB_PANIC, NULL);
	if (!ver_cache) {
		pr_err("%s: Cache create memory failed\n", __func__);
		return -1;
	}

	return 0;
}

static void nvtmem_ver_sys_deinit(void)
{
	kmem_cache_destroy(ver_cache);

	return;
}

/*
 *	proc init
 */

static int version_proc_show(struct seq_file *sfile, void *v)
{
	struct nvtmem_ver_info_t *curr_info, *next_info;
	seq_printf(sfile, "Version: %s\n", NVT_HDAL_MEM_VERSION);

	down(&sema_ver);
	list_for_each_entry_safe(curr_info, next_info, &ver_list_root, list) {
		seq_printf(sfile, "Device %s id %d: %u.%u.%u.%u\n", curr_info->pdev->name,
			   curr_info->id,
			   curr_info->version,
			   curr_info->patch_level,
			   curr_info->sub_level,
			   curr_info->extra_ver);
	}
	up(&sema_ver);
	return 0;
}
static int version_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, version_proc_show, NULL);
}

static const struct proc_ops version_proc_fops = {
	.proc_open	= version_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

static int dram_map_proc_show(struct seq_file *sfile, void *v)
{
	int i = 0;

	seq_printf(sfile, "DRAM mapping relationship:\n");

	for (i = 0; i < MAX_DDR_NR; i ++) {
		if (!ddr_chip_map[i].active)
			continue;
		seq_printf(sfile, "\r\tCHIP id: 0x%08x DRAM ID: 0x%08x addr: 0x%08lx size: 0x%08lx active: 0x%08x\n",
			ddr_chip_map[i].chipid,
			ddr_chip_map[i].loc_ddrid,
			(unsigned long)ddr_chip_map[i].start,
			(unsigned long)ddr_chip_map[i].size,
			ddr_chip_map[i].active);

		seq_printf(sfile, "\r\tHDAL memory => DRAM ID: 0x%lx addr: 0x%lx size: 0x%lx\n",
			(unsigned long)hdal_base.ddr_id[i],
			(unsigned long)hdal_base.base[i],
			hdal_base.size[i]);
	}

	return 0;
}

static int dram_map_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, dram_map_proc_show, NULL);
}

static const struct proc_ops dram_map_proc_fops = {
	.proc_open	= dram_map_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

static int __init nvtmem_proc_init(void)
{
	struct proc_dir_entry *p;

	p = proc_mkdir("nvtmem", NULL);
	if (p == NULL) {
		pr_err("%s, fail! \n", __func__);
		return -1;
	}

	nvtmem_proc_root = p;

	/* Create version proc node
	*/
	nvtmem_proc_ver = proc_create("version", S_IRUGO, nvtmem_proc_root, &version_proc_fops);
	if (nvtmem_proc_ver == NULL)
		panic("nvtmem/%s: Fail to create version proc node!\n", __func__);

	/* Create dram mapping proc node
	*/
	nvtmem_proc_dram_map = proc_create("dram_map", S_IRUGO, nvtmem_proc_root, &dram_map_proc_fops);
	if (nvtmem_proc_dram_map == NULL)
		panic("nvtmem/%s: Fail to create dram_map proc node!\n", __func__);

	return 0;
}

static int nvtmem_proc_deinit(void)
{
	/* Remove version proc node
	*/
	if (nvtmem_proc_ver != NULL)
		proc_remove(nvtmem_proc_ver);

	if (nvtmem_proc_root != NULL)
		remove_proc_entry("nvtmem", NULL);
	return 0;
}

static int __init nvtmem_dram_mapping_init(void)
{
	struct device_node *node;
	struct property *prop;
	int reg_count;
	u32 *array = NULL;
	int	i, idx = 0;
	int ret = 0, nr_addr, nr_size;

	memset(&ddr_chip_map[0], 0x0, sizeof(ddr_chip_map));

	node = of_find_node_by_name(NULL, "nvt_chip_mapping");
	if (node == NULL) {
		pr_info("nvt_chip_mapping doesn't exist in dts! Create default table \n");
		ddr_chip_map[0].loc_ddrid = 0;
		ddr_chip_map[0].chipid = 0;
		ddr_chip_map[0].active = 1;
		return -1;
	}

	ret = of_property_read_u32(node, "#address-cells", &nr_addr);
	if (ret)
		return -EINVAL;

	ret = of_property_read_u32(node, "#size-cells", &nr_size);
	if (ret)
		return -EINVAL;

#if defined(__aarch64__)
	if (nr_size != 2 || nr_addr != 4) {
		nvt_dbg(ERR, "%s: Error config address cell %d or size cell %d\n", __func__, nr_addr, nr_size);
		return -EINVAL;
	}
#else
	if (nr_size != 1 || nr_addr != 3) {
		nvt_dbg(ERR, "%s: Error config address cell %d or size cell %d \n", __func__, nr_addr, nr_size);
		return -EINVAL;
	}
#endif

	node = of_find_node_by_name(node, "dram_map");
	if (node == NULL) {
		nvt_dbg(ERR, "Failed to get nvt_chip_mapping/dram_map device node\n");
		return -EINVAL;
	}

	prop = of_find_property(node, "reg", NULL);
	if (!prop) {
		nvt_dbg(ERR, "Failed to get nvt_chip_mapping/dram_map/reg device node\n");
		return -EINVAL;
	}

	array = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (array == NULL) {
		pr_info("%s, no memory! \n", __func__);
		return -1;
	}

	/* Parsing PCIE dram mapping relationship */
	reg_count = of_property_count_u32_elems(node, "reg");
	ret = of_property_read_u32_array(node, "reg", &array[0], reg_count);
	if (ret != 0) {
		pr_err("%s, node \"reg\" not found\n", __func__);
		goto ERR_OUT;
	}

	/* active: It's used to indicate whether the dram is mapped or not */
#if defined(__aarch64__)
	for (i = 0; i < reg_count; i += 6) {
		/* [DDRID][CHIPID] [ADDR-H][ADDR-L] [SIZE-H][SIZE-L] */
		ddr_chip_map[idx].loc_ddrid = array[i];
		ddr_chip_map[idx].chipid = array[i + 1];
		ddr_chip_map[idx].start = array[i + 2];
		ddr_chip_map[idx].start = (ddr_chip_map[idx].start << 32) | array[i + 3];
		ddr_chip_map[idx].size = array[i + 4];
		ddr_chip_map[idx].size = (ddr_chip_map[idx].size << 32) | array[i + 5];
#else
	for (i = 0; i < reg_count; i += 4) {
		/* [DDRID] [CHIPID] [ADDR] [SIZE] */
		ddr_chip_map[idx].loc_ddrid = array[i];
		ddr_chip_map[idx].chipid = array[i + 1];
		ddr_chip_map[idx].start = array[i + 2];
		ddr_chip_map[idx].size = array[i + 3];
#endif
		ddr_chip_map[idx].active = 1;

		nvt_dbg(MSG, "ddr_id: 0x%08x chipid: 0x%08x addr: 0x%08lx size: 0x%08lx active: 0x%08x",
			ddr_chip_map[idx].loc_ddrid,
			ddr_chip_map[idx].chipid,
			(unsigned long)ddr_chip_map[idx].start,
			(unsigned long)ddr_chip_map[idx].size,
			ddr_chip_map[idx].active);
		idx ++;
	}

ERR_OUT:
	kfree(array);
	return ret;
}

static int nvtmem_dram_mapping_deinit(void)
{
	memset(&ddr_chip_map[0], 0x0, sizeof(ddr_chip_map));
	return 0;
}

static void nvtmem_dump_version(void)
{
	struct nvtmem_ver_info_t *curr_info, *next_info;

	pr_info("NVTMEM: %s\n", NVT_HDAL_MEM_VERSION);
	down(&sema_ver);
	list_for_each_entry_safe(curr_info, next_info, &ver_list_root, list) {
		pr_info("Device %s id %d: %u.%u.%u.%u\n", curr_info->pdev->name,
			curr_info->id,
			curr_info->version,
			curr_info->patch_level,
			curr_info->sub_level,
			curr_info->extra_ver);
	}
	up(&sema_ver);
}

static struct ap_open_info_t *nvtmem_get_open_info(struct ddr_alloc_list *ddr_alloc_list_data, void *filep)
{
	struct ap_open_info_t  *openinfo = NULL;
	struct list_head *node;
	int bFound = 0;

	if (list_empty(&ddr_alloc_list_data->open_list_head)) {
		nvt_dbg(ERR, "open_list is empty\n");
		return NULL;
	}

	list_for_each(node, &ddr_alloc_list_data->open_list_head) {
		openinfo = list_entry(node, struct ap_open_info_t, open_list);
		if ((uintptr_t)openinfo->filep != (uintptr_t)filep)
			continue;

		bFound = 1;
		break;
	}

	nvt_dbg(IND, "open:%p\n", openinfo->filep);

	return bFound ? openinfo : NULL;
}

static int nvtmem_dev_to_arr_idx(int minor)
{
	int desc_idx;
	int array_id = -EINVAL;


	down(&sema_ver);
	for (desc_idx = 0; desc_idx < MAX_DDR_NR; desc_idx ++) {

		nvt_dbg(IND, "ddr_alloc_data[desc_idx].minor_id:%d minor:%d\n", ddr_alloc_data[desc_idx].minor_id, minor);

		if (minor != ddr_alloc_data[desc_idx].minor_id)
			continue;

		array_id = desc_idx;
		break;
	}

	up(&sema_ver);

	return array_id;
}

/*
 * @This function shows memory layout for whole system
 *
 * @function void frm_get_ddrinfo(frammap_ddrinfo_t *info)
 * @input param: None
 * @output param: the layout of all DDRs
 * @return value: None.
 */
void nvtmem_get_ddrinfo(nvtmem_ddrinfo_t *info)
{
	int i;

	memset(info, 0, sizeof(nvtmem_ddrinfo_t));

	for (i = 0; i < MAX_DDR_NR; i ++) {
		if (!ddr_chip_map[i].active)
			continue;
		info->bank[info->nr_banks].start = ddr_chip_map[i].start;
		info->bank[info->nr_banks].size = ddr_chip_map[i].size;
		info->bank[info->nr_banks].ddrid = ddr_chip_map[i].loc_ddrid;
		info->bank[info->nr_banks].chip = ddr_chip_map[i].chipid;
		info->nr_banks ++;
	}
}
EXPORT_SYMBOL(nvtmem_get_ddrinfo);

/*
	 * arm32: u32, arm64: u64
	 * 1: addr 2: ddr size
	 * We suppose the maximum of the memory segments is NVTMEM_OS_MEM_SEGMENTS
	 */
#define NVTMEM_OS_MEM_SEGMENTS		3
#if defined(__aarch64__)
unsigned long long nvtmem_os_mem_cfg[NVTMEM_OS_MEM_SEGMENTS * 2] = {0};
#else
unsigned int nvtmem_os_mem_cfg[NVTMEM_OS_MEM_SEGMENTS * 2] = {0};
#endif

static int nvtmem_get_os_memory(void)
{
	struct device_node *node;
	struct property *prop;
	int reg_count;
	int nr_addr, nr_size;
	int length;
	int ret = 0;

	memset(nvtmem_os_mem_cfg, 0, sizeof(nvtmem_os_mem_cfg));

	node = of_find_node_by_name(NULL, "memory");
	if (node == NULL) {
		nvt_dbg(ERR, "Failed to get memory device node\n");
		return -EINVAL;
	}

	nr_addr = of_n_addr_cells(node);
	nr_size = of_n_size_cells(node);

#if defined(__aarch64__)
	if (nr_size != 2 || nr_addr != 2)
		return -EINVAL;
#else
	if (nr_size != 1 || nr_addr != 1)
		return -EINVAL;
#endif

	prop = of_find_property(node, "reg", &length);
	if (!prop) {
		nvt_dbg(ERR, "Failed to get /memory/reg device node\n");
		return -EINVAL;
	}

	/* only parsing RC's DDR controller */
#if defined(__aarch64__)
	reg_count = of_property_count_u64_elems(node, "reg");
	ret = of_property_read_u64_array(node, "reg", &nvtmem_os_mem_cfg[0], reg_count);
#else
	reg_count = of_property_count_u32_elems(node, "reg");
	ret = of_property_read_u32_array(node, "reg", &nvtmem_os_mem_cfg[0], reg_count);
#endif

	if (ret != 0) {
		pr_err("%s, node \"reg\" not found\n", __func__);
		return -1;
	}

	return 0;
}

static bool nvtmem_ep_is_enabled(void)
{
	u32 m_epboot = 0x0;
	int ret = 0;
	u32 is_daughter = 0x0;
	struct device_node *of_node = of_find_node_by_path("/ep_info");

	if (of_node) {
		ret = of_property_read_u32(of_node, "enable", &m_epboot);
		if (ret < 0)
			return false;
		else {
			if (m_epboot == 1) {
				nvt_dbg(MSG, "EP cpu is booted\n");
				if (of_property_read_u32(of_node, "daughter", &is_daughter) < 0) {
					return true;
				} else if (is_daughter == 0){
					return true;
				}
			}
		}
	}

	return false;
}

static int nvtmem_load_hdal_base(struct nvtmem_hdal_base *hdal_base)
{
	struct device_node *node;
	struct property *prop;
	int reg_count;
	int nr_addr, nr_size, err;
	int i = 0, j = 0;

	/*
	 * arm32: u32, arm64: u64
	 * 1: ddr id, 2: ddr addr, 3: ddr size
	 */
#if defined(__aarch64__)
	unsigned long long array[MAX_DDR_NR * 3] = {0};
#else
	unsigned int array[MAX_DDR_NR * 3] = {0};
#endif
	int length;
	int ret = 0;

	if (nvtmem_ep_is_enabled() == true)
		return 0;

	ret = nvtmem_get_os_memory();
	if (ret < 0) {
		return -EINVAL;
	}
	memset(hdal_base, 0, sizeof(struct nvtmem_hdal_base));
	node = of_find_node_by_name(NULL, "hdal-memory");
	if (node == NULL) {
		nvt_dbg(ERR, "Failed to get hdal-memory device node\n");
		return -EINVAL;
	}

	err = of_property_read_u32(node, "#address-cells", &nr_addr);
	if (err)
		return -EINVAL;

	err = of_property_read_u32(node, "#size-cells", &nr_size);
	if (err)
		return -EINVAL;

#if defined(__aarch64__)
	if (nr_size != 2 || nr_addr != 4)
		return -EINVAL;
#else
	if (nr_size != 1 || nr_addr != 2)
		return -EINVAL;
#endif
	node = of_find_node_by_name(node, "media");
	if (node == NULL) {
		nvt_dbg(ERR, "Failed to get hdal-memory/media device node\n");
		return -EINVAL;
	}

	prop = of_find_property(node, "reg", &length);
	if (!prop) {
		nvt_dbg(ERR, "Failed to get hdal-memory/media/reg device node\n");
		return -EINVAL;
	}

	/* only parsing RC's DDR controller */
#if defined(__aarch64__)
	reg_count = of_property_count_u64_elems(node, "reg");
	ret = of_property_read_u64_array(node, "reg", &array[0], reg_count);
#else
	reg_count = of_property_count_u32_elems(node, "reg");
	ret = of_property_read_u32_array(node, "reg", &array[0], reg_count);
#endif

	if (ret != 0) {
		pr_err("%s, node \"reg\" not found\n", __func__);
		goto ERR_OUT;
	}

	/* We suppose that the maximum of the dram controller is MAX_DDR_NR */
	for (i = 0; i < MAX_DDR_NR ; i++) {
		if (array[(3 * i) + 2] != 0) {
			hdal_base->ddr_id[i] = (uintptr_t)array[3 * i];
			hdal_base->base[i] = (uintptr_t)array[(3 * i) + 1];
			hdal_base->size[i] = array[(3 * i) + 2];

			nvt_dbg(MSG, "Parsing HDAL mem: ID: 0x%08lx Addr: 0x%08lx Size: 0x%08lx\n",
				(unsigned long)hdal_base->ddr_id[i],
				(unsigned long)hdal_base->base[i],
				(unsigned long)hdal_base->size[i]);
		}
	}

	/* Check for memory tbl config is valid in RC */
	for (i = 0; i < NVTMEM_OS_MEM_SEGMENTS; i++) {
		for (j = 0; j < MAX_DDR_NR; j++) {
			/* to check os and hdal mem region if the ddr id 0 only
			 * os mem will have multi segments, we should check for each segment.
			 * dram 0 is often used as Linux os memory region,  */
			if (nvtmem_os_mem_cfg[2 * i] != 0 && nvtmem_os_mem_cfg[2 * i + 1] != 0) {
				if (hdal_base->ddr_id[j] == 0 && hdal_base->base[j] != 0 && hdal_base->size[j] != 0) {
					if (nvtmem_os_mem_cfg[2 * i] > hdal_base->base[j]) {
						if ((hdal_base->base[j] + hdal_base->size[j]) > nvtmem_os_mem_cfg[2 * i])
							ret = -1;
					} else {
						if ((nvtmem_os_mem_cfg[2 * i] + nvtmem_os_mem_cfg[2 * i + 1]) > hdal_base->base[j])
							ret = -1;
					}
					if (ret < 0) {
						nvt_dbg(ERR, "=====>>> hdal and os mem config is overlapped, os: 0x%08lx@0x%08lx, hdal: 0x%08lx@0x%08lx\n",
							(unsigned long)nvtmem_os_mem_cfg[2 * i + 1],
							(unsigned long)nvtmem_os_mem_cfg[2 * i],
							hdal_base->size[j],
							hdal_base->base[j]);
						panic("HDAL is overlaped with Linux OS mem\n");
					}
				}
			}
		}
	}

ERR_OUT:
	return ret;
}

/*
 * misc device init.
 */
static long nvtmem_mem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int array_id;
	struct ddr_alloc_list *cookie;
	struct ap_open_info_t *openinfo;
	struct inode *inode = NULL;

	if (filp == NULL) {
		nvt_dbg(ERR, "file is NULL pointer\n");
		return -EINVAL;
	}
	inode = filp->f_mapping->host;

	array_id = nvtmem_dev_to_arr_idx(iminor(inode));
	if (array_id < 0) {
		nvt_dbg(ERR, "invalid dev:%d \n", array_id);
		return -1;
	}

	cookie = &ddr_alloc_data[array_id];
	if (list_empty(&cookie->open_list_head)) {
		nvt_dbg(ERR, "This dev%d is not opened! \n", iminor(inode));
		return -EFAULT;
	}

	openinfo = nvtmem_get_open_info(cookie, filp);
	if (openinfo == NULL) {
		nvt_dbg(ERR, "Gets unknown filp:0x%p! \n", filp);
		return -EFAULT;
	}

	switch (cmd) {
	case NVTMEM_ALLOC_TYPE: {
		nvt_fmem_alloc_type_t alloc_type;
		unsigned long  retVal;

		retVal = copy_from_user((void *)&alloc_type, (void *)arg, sizeof(int));
		if (retVal) {
			nvt_dbg(ERR, "TYPE copy_from_user fail! \n");
			ret = -EFAULT;
			break;
		}

		openinfo->alloc_type = alloc_type;
		nvt_dbg(IND, "set memory allocate type:0x%x for filp: 0x%p \n", openinfo->alloc_type, openinfo->filep);

		break;
	}
	case NVTMEM_ADDR_V2P: {
		unsigned long retVal;
		uintptr_t vaddr;
		phys_addr_t paddr;

		ret = -EFAULT;

		retVal = copy_from_user((void *)&vaddr, (void *)arg, sizeof(uintptr_t));
		if (retVal) {
			nvt_dbg(ERR, "V2P copy_from_user fail! \n");
			break;
		}
		paddr = fmem_lookup_pa(vaddr);
		if (paddr != -1) {
			if (copy_to_user((void *)arg, (void *)&paddr, sizeof(phys_addr_t)))
				break;

			ret = 0;
		}
		break;
	}
	case NVTMEM_MEMORY_SYNC: {
		unsigned long retVal;
		struct nvtmem_vaddr_info vaddr;

		retVal = copy_from_user((void *)&vaddr, (void *)arg, sizeof(struct nvtmem_vaddr_info));

		if (retVal)
			break;

		fmem_dcache_sync((void *)vaddr.vaddr, vaddr.length, vaddr.dir);
		break;
	}
	case NVTMEM_GET_DDR_MAP_ALL: {
		struct nvtmem_ddr_all 	ddr_all;
		nvtmem_ddrinfo_t		ddr_info;
		int	i;

		memset(&ddr_all, 0, sizeof(struct nvtmem_ddr_all));
		nvtmem_get_ddrinfo(&ddr_info);

		for (i = 0; i < ddr_info.nr_banks; i ++) {
			ddr_all.bank[i].start = (uintptr_t)ddr_info.bank[i].start;
			ddr_all.bank[i].size = ddr_info.bank[i].size;
			ddr_all.bank[i].ddrid = ddr_info.bank[i].ddrid;
		}
		ddr_all.nr_banks = ddr_info.nr_banks;
		break;
	}
	case NVTMEM_GET_DTS_HDAL_BASE: {
		if (copy_to_user((void *)arg, (void *)&hdal_base, sizeof(struct nvtmem_hdal_base)))
			ret = -EFAULT;
		break;
	}
	default:
		pr_err("%s, unknown cmd: 0x%x \n", __func__, cmd);
		ret = -1;
		break;
	}

	return (long)ret;
}

static int nvtmem_mem_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	int array_id;
	struct ap_open_info_t *openinfo;
	struct ddr_alloc_list *cookie;

	if (inode == NULL || filp == NULL) {
		nvt_dbg(ERR, "invalid point found inode:0x%p filp:0x%p\n", inode, filp);
		ret = -EINVAL;
		goto ERR_EXIT;
	}

	nvt_dbg(IND, "minor:%d\n", iminor(inode));
	array_id = nvtmem_dev_to_arr_idx(iminor(inode));
	if (array_id < 0) {
		nvt_dbg(ERR, "invalid dev:%d \n", array_id);
		ret = -EINVAL;
		goto ERR_EXIT;
	}

	cookie = &ddr_alloc_data[array_id];
	down(&sema_ver);

	nvt_dbg(IND, "AP%d open file: 0x%p \n", iminor(inode), filp);

	/* create one node */
	openinfo = kzalloc(sizeof(struct ap_open_info_t), GFP_KERNEL);
	if (openinfo == NULL) {
		ret = -ENOMEM;
		goto ERR_EXIT;
	}

	INIT_LIST_HEAD(&openinfo->alloc_list_head);
	openinfo->filep = filp;
//	openinfo->alloc_cnt = 0;
	openinfo->alloc_type = ALLOC_NONCACHABLE;  //default type
	filp->private_data = NULL;
	list_add_tail(&openinfo->open_list, &cookie->open_list_head);
	cookie->open_cnt++;

	/* inc the module reference counter */
	try_module_get(THIS_MODULE);

ERR_EXIT:
	up(&sema_ver);

	return ret;
}

static int nvtmem_mem_release(struct inode *inode, struct file *filp)
{
	struct ddr_alloc_list *cookie;
	struct frm_vm_struct *node, *ne;
	struct ap_open_info_t *openinfo;
	int ret = 0, array;

	if (IS_ERR_OR_NULL(inode) || IS_ERR_OR_NULL(filp)) {
		nvt_dbg(ERR, "invalid point found inode:0x%p filp:0x%p\n", inode, filp);
		return -EINVAL;
	}

	array = nvtmem_dev_to_arr_idx(iminor(inode));
	if (array < 0) {
		nvt_dbg(ERR, "invalid minor:%d \n", iminor(inode));
		return array;
	}

	nvt_dbg(IND, "Minor:%d is called for release\n", iminor(inode));

	down(&sema_ver);

	cookie = &ddr_alloc_data[array];
	openinfo = nvtmem_get_open_info(cookie, filp);
	if (unlikely(!openinfo)) {
		ret = -EINVAL;
		goto ERR_OUT;
	}

	list_del(&openinfo->open_list);
	cookie->open_cnt--;

	nvt_dbg(IND, "release filp:0x%p\n", openinfo->filep);

	list_for_each_entry_safe(node, ne, &openinfo->alloc_list_head, alloc_list) {

		list_del_init(&node->alloc_list);

		/* from cma */
		if (node->handle != NULL) {
			nvt_dbg(IND, "release memory for handle: 0x%p \n", node->handle);
			if (fmem_release_from_cma(node->handle, 0) < 0) {
				nvt_dbg(ERR, "fmem release from cma fail\n");
			}
		}

		kfree(node);
//		openinfo->alloc_cnt--;
	}

	nvt_dbg(IND, "release\n");
/*
	if (openinfo->alloc_cnt != 0) {
		nvt_dbg(ERR, "Mismatch release count: %d \n", openinfo->alloc_cnt);
	}
*/

	module_put(THIS_MODULE);

	kfree(openinfo);

ERR_OUT:
	up(&sema_ver);

	return ret;
}

static int nvtmem_mem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct ddr_alloc_list *cookie;
	struct ap_open_info_t *openinfo;
	struct inode *inode = NULL;
	struct frm_vm_struct *pfrm_alloc = NULL;
	unsigned long size;
	unsigned long off;    /* offset from the buffer start point */
	unsigned long pfn;          /* which page number is the start page*/
	int ret = 0, array_id, i = 0;

	if (IS_ERR_OR_NULL(filp)) {
		nvt_dbg(ERR, "file pointer is invalid\n");
		return -1;
	}

	if (IS_ERR_OR_NULL(vma)) {
		nvt_dbg(ERR, "vma is invalid\n");
		return -1;
	}

	inode = filp->f_mapping->host;

	off = vma->vm_pgoff << PAGE_SHIFT;
	size = vma->vm_end - vma->vm_start;

	array_id = nvtmem_dev_to_arr_idx(iminor(inode));
	if (array_id < 0) {
		nvt_dbg(ERR, "invalid minor:%d \n", iminor(inode));
		return -EINVAL;
	}

	cookie = &ddr_alloc_data[array_id];

	openinfo = nvtmem_get_open_info(cookie, filp);
	if (unlikely(!openinfo)) {
		nvt_dbg(ERR, "unknown filp: 0x%p \n", filp);
		return -EFAULT;
	}

	nvt_dbg(IND, "Alloc memory for filp: 0x%p \n", openinfo->filep);

	if (filp->private_data == NULL) {
		pfrm_alloc = kzalloc(sizeof(struct frm_vm_struct), GFP_KERNEL);
		if (unlikely(!pfrm_alloc)) {
			ret = -ENOMEM;
			goto ERR_EXIT;
		}
	} else {
		pfrm_alloc = filp->private_data;
	}

	switch (openinfo->alloc_type & ALLOC_MMU_TYPE_MASK) {
	case ALLOC_NONCACHABLE:
		pfrm_alloc->nvt_fmem_mem_info.type = NVT_FMEM_ALLOC_NONCACHE;
		nvt_dbg(IND, "Alloc non-cache\n");
		break;

	case ALLOC_BUFFERABLE:
		pfrm_alloc->nvt_fmem_mem_info.type = NVT_FMEM_ALLOC_BUFFER;
		nvt_dbg(IND, "Alloc bufferable\n");
		break;

	case ALLOC_CACHEABLE:
		pfrm_alloc->nvt_fmem_mem_info.type = NVT_FMEM_ALLOC_CACHE;
		nvt_dbg(IND, "Alloc cacheable\n");
		break;

	default:
		pfrm_alloc->nvt_fmem_mem_info.type = NVT_FMEM_ALLOC_CACHE;
		nvt_dbg(IND, "Alloc cacheable\n");
		break;
	}

	for (i = 0; i < MAX_DDR_NR ; i++) {
		if (hdal_base.base[i] != 0 && hdal_base.size[i] != 0) {
			nvt_dbg(IND, "off:0x%lx\n", off);
			nvt_dbg(IND, "vm_start:0x%p, size:0x%lx, ddr id: 0x%lx, hdal_base:0x%lx, hdal_size:0x%lx\n",
				(void *)vma->vm_start,
				size,
				hdal_base.ddr_id[i],
				hdal_base.base[i],
				hdal_base.size[i]);
		}
	}

	/* map only is used to do mapping all area (including RC/EP).
	 * Non-OS address region: non-linux os area will use mapping only (Including IO and memory)
	 * OS memory region: allocate from cma
	 * ALLOC_MAP_ONLY: this flag will map any physical address only (Bypass above config setting)
	 */
	if (((off >= hdal_base.base[0]) && (off < hdal_base.base[0] + hdal_base.size[0])) ||
	    (openinfo->alloc_type & ALLOC_MAP_ONLY)) {
		/* from hdal or mapping directly */
		pfrm_alloc->nvt_fmem_mem_info.size =  size;
		pfrm_alloc->nvt_fmem_mem_info.vaddr = 0;
		pfrm_alloc->nvt_fmem_mem_info.paddr = off;
		pfrm_alloc->nvt_fmem_mem_info.page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	} else {
		pfrm_alloc->nvt_fmem_mem_info.size = size;
		pfrm_alloc->nvt_fmem_mem_info.vaddr = 0;
		pfrm_alloc->nvt_fmem_mem_info.page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
		pfrm_alloc->handle = fmem_alloc_from_cma(&pfrm_alloc->nvt_fmem_mem_info, 0);
		if (pfrm_alloc->handle == NULL) {
			nvt_dbg(ERR, "Failed to alloc mem from fmem:0x%p\n", pfrm_alloc->handle);
			ret = -EFAULT;
			goto ERR_EXIT;
		}
		nvt_dbg(IND, " Allocate from CMA memory\n");
	}
	nvt_dbg(IND, "Alloc va:%p pa:%lx size:%lx pages:0x%lx\n", pfrm_alloc->nvt_fmem_mem_info.vaddr, (unsigned long)pfrm_alloc->nvt_fmem_mem_info.paddr,
		(unsigned long)pfrm_alloc->nvt_fmem_mem_info.size, (unsigned long)pfrm_alloc->nvt_fmem_mem_info.page_count);

	if (filp->private_data == NULL) {
		INIT_LIST_HEAD(&pfrm_alloc->alloc_list);
		list_add_tail(&pfrm_alloc->alloc_list, &openinfo->alloc_list_head);
	}

	/* Get the memory success, start to do mmap	*/
	if ((openinfo->alloc_type & ALLOC_MMU_TYPE_MASK) == ALLOC_CACHEABLE) {
		vma->vm_page_prot = PAGE_SHARED;
		nvt_dbg(IND, " vm page prot config with cacheable\n");
	} else if ((openinfo->alloc_type & ALLOC_MMU_TYPE_MASK) == ALLOC_BUFFERABLE) {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		nvt_dbg(IND, " vm page prot config with bufferable\n");
	} else {
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		nvt_dbg(IND, " vm page prot config with non-cacheable\n");
	}

	/* We must mapping linux os area from cma if no map only
	 * The paddr of the mapping only address is not from cma, it's from user space off to assign
	 * */
	if (((off < hdal_base.base[0]) || (off >= hdal_base.base[0] + hdal_base.size[0])) &&
	    !(openinfo->alloc_type & ALLOC_MAP_ONLY)) {
		off = pfrm_alloc->nvt_fmem_mem_info.paddr;     //points to the offset from node->buf_info.phy_addr
		nvt_dbg(IND, "To assign cma offset to user space offset\n");
	}
	pfn = off >> PAGE_SHIFT;    //which page number is the start page

	ret = remap_pfn_range(vma, vma->vm_start, pfn, pfrm_alloc->nvt_fmem_mem_info.size, vma->vm_page_prot);

	if (filp->private_data == NULL)
		filp->private_data = pfrm_alloc;

	nvt_dbg(IND, "AP%d, off:%lx, p:0x%lx, v:0x%lx, size:0x%lx, pfn:0x%lx, alloc_type:%d \n", iminor(inode),
		off, (unsigned long)pfrm_alloc->nvt_fmem_mem_info.paddr, (unsigned long)vma->vm_start, size, pfn,
		openinfo->alloc_type);

	//openinfo->alloc_cnt++;

	return ret;

ERR_EXIT:

	kfree(pfrm_alloc);
	return ret;
}

static int __init nvtmem_mem_init(void)
{
	int	ret = 0;
	int index;
	dev_t curr_dev;
	nvtmem_mem_dev_ptr = kzalloc(sizeof(struct nvtmem_mem_dev_t), GFP_KERNEL);

	/* Only support NVTMEM_MEM_DEV_COUNT is 1 case currently */
	ret = alloc_chrdev_region(&nvtmem_mem_dev_ptr->nvt_mem_dev_num, NVTMEM_MEM_DEV_MINOR,
				  NVTMEM_MEM_DEV_COUNT, NVTMEM_MEM_DRV_NAME);
	if (ret < 0) {
		pr_err("%s: failed to allocate char drv region\n", __func__);
		goto err_alloc_chrdev_fail;
	}

	nvtmem_mem_dev_ptr->nvtmem_class = class_create(THIS_MODULE, NVTMEM_MEM_CLASS_NAME);
	if (IS_ERR(nvtmem_mem_dev_ptr->nvtmem_class)) {
		ret = -1;
		pr_err("%s: Error to create class: %s\n", __func__, NVTMEM_MEM_CLASS_NAME);
		goto err_class_fail;
	}

	nvtmem_mem_dev_ptr->nvtmem_mem_fops.owner		= THIS_MODULE;
	nvtmem_mem_dev_ptr->nvtmem_mem_fops.unlocked_ioctl	= nvtmem_mem_ioctl;
	nvtmem_mem_dev_ptr->nvtmem_mem_fops.mmap		= nvtmem_mem_mmap;
	nvtmem_mem_dev_ptr->nvtmem_mem_fops.open		= nvtmem_mem_open;
	nvtmem_mem_dev_ptr->nvtmem_mem_fops.release		= nvtmem_mem_release;

	cdev_init(&nvtmem_mem_dev_ptr->nvtmem_cdev, &nvtmem_mem_dev_ptr->nvtmem_mem_fops);
	nvtmem_mem_dev_ptr->nvtmem_cdev.owner = THIS_MODULE;
	/* Build up the current device number. To be used further */
	curr_dev = MKDEV(MAJOR(nvtmem_mem_dev_ptr->nvt_mem_dev_num), MINOR(nvtmem_mem_dev_ptr->nvt_mem_dev_num));

	/* /dev/nvtmem0, /dev/nvtmem1...etc, we only support nvtmem0 currently */
	nvtmem_mem_dev_ptr->nvtmem_dev = device_create(nvtmem_mem_dev_ptr->nvtmem_class, NULL, curr_dev, NULL, NVTMEM_MEM_DEV_NAME"%d", 0);
	if (IS_ERR(nvtmem_mem_dev_ptr->nvtmem_dev)) {
		ret = -1;
		pr_err("%s: Error to create device: %s\n", __func__, NVTMEM_MEM_DEV_NAME);
		goto err_device_fail;
	}

	ret = cdev_add(&nvtmem_mem_dev_ptr->nvtmem_cdev, curr_dev, NVTMEM_MEM_DEV_COUNT);
	if (ret) {
		pr_err("%s: Failed to add nvtmem cdev: %d\n", __func__, ret);
		goto err_cdev_add_fail;
	}

	ddr_alloc_data[0].minor_id = NVTMEM_MEM_DEV_MINOR;
	pr_info("nvtmem: register char device successfully! \n");

	for (index = 0 ; index < MAX_DDR_NR ; index++) {
		ddr_alloc_data[index].open_cnt = 0;
		INIT_LIST_HEAD(&ddr_alloc_data[index].open_list_head);
	}

	return 0;

err_cdev_add_fail:
	device_destroy(nvtmem_mem_dev_ptr->nvtmem_class, curr_dev);

err_device_fail:
	class_destroy(nvtmem_mem_dev_ptr->nvtmem_class);

err_class_fail:
	unregister_chrdev_region(curr_dev, NVTMEM_MEM_DEV_COUNT);

err_alloc_chrdev_fail:
	kfree(nvtmem_mem_dev_ptr);

	return ret;
}

static int nvtmem_mem_deinit(void)
{
	dev_t curr_dev;

	curr_dev = MKDEV(MAJOR(nvtmem_mem_dev_ptr->nvt_mem_dev_num), MINOR(nvtmem_mem_dev_ptr->nvt_mem_dev_num));
	cdev_del(&nvtmem_mem_dev_ptr->nvtmem_cdev);
	device_destroy(nvtmem_mem_dev_ptr->nvtmem_class, curr_dev);
	class_destroy(nvtmem_mem_dev_ptr->nvtmem_class);
	unregister_chrdev_region(curr_dev, NVTMEM_MEM_DEV_COUNT);
	kfree(nvtmem_mem_dev_ptr);

	return 0;
}

static int __init nvtmem_init(void)
{
	int ret = 0;

	ret = nvtmem_ver_sys_init();
	if (ret < 0)
		return -1;

	ret = nvtmem_proc_init();
	if (ret < 0)
		return -1;

	ret = nvtmem_dram_mapping_init();
	if (ret < 0)
		return -1;

	ret = nvtmem_load_hdal_base(&hdal_base);
	if (ret < 0)
		return -1;

	ret = nvtmem_mem_init();
	if (ret < 0)
		return -1;

	nvtmem_dump_version();
	return 0;
}

static void __exit nvtmem_exit(void)
{
	nvtmem_ver_sys_deinit();
	nvtmem_proc_deinit();
	nvtmem_dram_mapping_deinit();
	nvtmem_mem_deinit();
	return;
}

core_initcall(nvtmem_init);

MODULE_AUTHOR("Novatek");
MODULE_DESCRIPTION("NVT Memory Library");
MODULE_LICENSE("GPL");
MODULE_VERSION(NVT_HDAL_MEM_VERSION);
