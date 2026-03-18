/**
    NVT info for the platform related tools
    This file will provide the boot stage record the timestamp function
    @file       nvt-info.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/memblock.h>
// #include <generated/nvtversion.h>

#include <plat/hardware.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt-info.h>
#include <linux/soc/nvt/fmem.h>
#ifdef CONFIG_OUTER_CACHE
#include <asm/outercache.h>
#endif

#include <linux/of_device.h>
#include <linux/of_platform.h>

#define NVT_BOOTTS_MAX_LEN          32
#define NVT_BOOTTS_NAME_MAX_LEN     16
#define NVT_BOOTTS_UBOOT_RESV_CNT   2
#define LOADER_TIME_OFFSET          0

#define NVT_TIMER_TM0_CNT       0x108
#define TM0_LIST_ONLY

#define NVT_UTS_RELEASE 	"5.10.168_"

struct bootts {
	u32 time;
	char name[NVT_BOOTTS_NAME_MAX_LEN];
	int column;
	s32 diff_time; //diff time to the same name
};

static struct bootts g_bootts[NVT_BOOTTS_MAX_LEN] = {0};
static int g_index = NVT_BOOTTS_UBOOT_RESV_CNT;
static unsigned long suspend_mode = 0;
/* To define fmem va to pa test related */
#define NVT_FMEM_LOOKUP_TEST_MAX 10
static unsigned long *nvt_fmem_test_va_pa_addr_list = NULL;
#define NVT_FMEM_LOOKUP_TEST_SIZE 0x40000000UL
/* Defined by include/linux/soc/nvt/nvt-info.h */
struct proc_dir_entry *nvt_info_dir_root = NULL;
EXPORT_SYMBOL(nvt_info_dir_root);

#ifdef CONFIG_OUTER_CACHE
static DEFINE_SPINLOCK(test_l2_lock);
#endif
static void __iomem *timer0_vaddr = NULL;

#ifdef CONFIG_NVT_GPU_AFBC_SUPPORT
/* To define Mali GPU AFBC enable control */
static unsigned long gpu_afbc_en = 0;
#endif /* CONFIG_NVT_GPU_AFBC_SUPPORT */

#ifdef CONFIG_NVT_FB_DMABUF
/* To define dma buffer on framebuffer enable control */
static unsigned long fb_dmabuf_en = 0;
#endif /* CONFIG_NVT_FB_DMABUF */

unsigned long nvt_get_time(void)
{
	if (!timer0_vaddr) {
		timer0_vaddr = ioremap(NVT_TIMER_BASE_PHYS, 4);
		if (!timer0_vaddr) {
			pr_err("%s: map timer va error\n", __func__);
			return -ENOMEM;
		}
	}

	return nvt_readl(timer0_vaddr + NVT_TIMER_TM0_CNT);
}

//NOTE: the earliest "ker" tag is set in nvt_ivot_map_io(),
//      which is located at linux-kernel/arch/arm/mach-nvt-ivot/io.c
void nvt_bootts_add_ts(char *name)
{
	unsigned char name_len = strlen(name);

	if (name_len >= NVT_BOOTTS_NAME_MAX_LEN || g_index >= NVT_BOOTTS_MAX_LEN) {
		pr_err("\n%s %s fail\n", __func__, name);
		return;
	}

	if (name != NULL)
		strncpy(g_bootts[g_index].name, name, name_len);

	g_bootts[g_index].time = nvt_get_time();
	g_index++;
}

EXPORT_SYMBOL_GPL(nvt_bootts_add_ts);

#ifdef CONFIG_PROC_FS
/* Support for	/proc/nvt_info/bootts
		/proc/nvt_info/tm0
		/proc/nvt_info/memperf
		/proc/nvt_info/memhotplug
		/proc/nvt_info/version
		/proc/nvt_info/boot_source
		/proc/nvt_info/hdal_part_num
		/proc/nvt_info/stack
		/proc/nvt_info/gpu_afbc_en
		/proc/nvt_info/fb_dmabuf_en

*/

static void add_uboot_time(void)
{
	char *pstr = NULL;
	char *psep = NULL;
	char symbol;
	unsigned long uboot_ts = 0;
	int bootts_idx = 0;
	int ret = 0;

	pstr = strstr(saved_command_line, "bootts=");
	if (pstr) {
		pstr += strlen("bootts=");
	}
	while (pstr) {
		psep = strpbrk(pstr, ", "); //find ',' or ' ', or '\0'
		if (NULL == psep) {
			break;
		}

		symbol = *psep;
		*psep = '\0';
		uboot_ts = 0;
		ret = kstrtoul(pstr, 10, &uboot_ts);
		if (ret < 0) {
			pr_info("Error parsing uboot time\n");
			return;
		}
		*psep = symbol;

		if (bootts_idx < NVT_BOOTTS_UBOOT_RESV_CNT) {
			strncpy(g_bootts[bootts_idx].name, "uboot", sizeof(g_bootts[bootts_idx].name)-1);
			g_bootts[bootts_idx].time = uboot_ts;
			bootts_idx++;
		}

		if (' ' == symbol) {
			break;
		}

		pstr = psep + 1;
	}
}

#ifndef TM0_LIST_ONLY
static void bootts_dump_linux_tm0(struct seq_file *m)
{
	int i;

	add_uboot_time();

	seq_puts(m, "Name\t\tDiff(us)\tTM0\n");
	for (i = 0; i < g_index; i++) {
		seq_printf(m, "%-8.8s\t%-8.8u\t%-8.8u\n",
				g_bootts[i].name,
				(i-1 < 0) ? (g_bootts[i].time - 0) :
				(g_bootts[i].time - g_bootts[i-1].time),
				g_bootts[i].time
				);
	}
	seq_printf(m, "%-8.8s\t%-8.8u\t\n",
			"Total",
			(g_bootts[g_index - 1].time - g_bootts[0].time)
			);
}
#else //TM0_LIST_ONLY
static void bootts_dump_linux(struct seq_file *m)
{
	int item_idx, item_idx2;
	int column_cur, column_max;
	int row_idx;
	int max_diff_time;

	if (g_index < 1) {
		seq_printf(m, "no data");
		return;
	}

	add_uboot_time();

	//reset the column to recount the newest column
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		g_bootts[item_idx].column = 0;
	}

	//setup the column of each item
	column_cur = 0;
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		if (0 != g_bootts[item_idx].column) {
			continue;
		}
		column_cur++;
		g_bootts[item_idx].column = column_cur;
		for (item_idx2 = item_idx; item_idx2 < g_index; item_idx2++) {
			if (!strcmp(g_bootts[item_idx2].name, g_bootts[item_idx].name)) {
				g_bootts[item_idx2].column = column_cur;
				g_bootts[item_idx2].diff_time = g_bootts[item_idx2].time - g_bootts[item_idx].time;
			}
		}
	}
	column_max = column_cur;

#if 0 //for debug
	pr_info("g_index %d, column_max %d\r\n", g_index, column_max);
	for (item_idx = 0; item_idx < g_index; item_idx++) {
		pr_info("item[%d], column %d, %u, %s, %d\r\n",
			item_idx,
			g_bootts[item_idx].column,
			g_bootts[item_idx].time,
			g_bootts[item_idx].name,
			g_bootts[item_idx].diff_time);
	}
#endif

	//print column name
	seq_printf(m, "Name");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		for (item_idx = 0; item_idx < g_index; item_idx++) {
			if (column_cur == g_bootts[item_idx].column) {
				seq_printf(m, "%12s", g_bootts[item_idx].name);
				break;
			}
		}
	}
	seq_printf(m, "\n");

	//print item data by column
	for (row_idx = 0; row_idx < g_index; row_idx++) {
		seq_printf(m, "[%02d]", row_idx);
		for (column_cur = 1; column_cur <= column_max; column_cur++) {
			if (column_cur == g_bootts[row_idx].column) {
				seq_printf(m, "%12u", g_bootts[row_idx].time + LOADER_TIME_OFFSET);
			} else {
				seq_printf(m, "%12c", '-');
			}
		}
		seq_printf(m, "\n");
	}

	//print diff time
	seq_puts(m, "-----------------------------------------------\n");
	seq_printf(m, "Diff");
	for (column_cur = 1; column_cur <= column_max; column_cur++) {
		max_diff_time = 0;
		for (item_idx = 0; item_idx < g_index; item_idx++) {
			if (g_bootts[item_idx].column == column_cur &&
				g_bootts[item_idx].diff_time > max_diff_time) {
				max_diff_time = g_bootts[item_idx].diff_time;
			}
		}
		seq_printf(m, "%12d", max_diff_time);
	}
	seq_printf(m, "\n");
}
#endif //TM0_LIST_ONLY

static int nvt_bootts_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "============== Boot time results ==============\n");
	/* seq_printf(m, "TM0 initial value: %llu\n", g_bootts.time[0]); */
#ifndef TM0_LIST_ONLY
	bootts_dump_linux_tm0(m);
#else
	bootts_dump_linux(m);
#endif
	seq_puts(m, "================================================\n");
	return 0;
}


static int nvt_bootts_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_bootts_proc_show, NULL);
}

static ssize_t nvt_bootts_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EINVAL;
	if (count >= NVT_BOOTTS_NAME_MAX_LEN)
		goto out;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	buf[count-1] = '\0';

	nvt_bootts_add_ts(buf);
	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}
static const struct proc_ops nvt_bootts_fops = {
	.proc_open = nvt_bootts_open,
	.proc_read = seq_read,
	.proc_write = nvt_bootts_write,
	.proc_release = single_release,
};

static int nvt_timer_tm0_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%lu\n", nvt_get_time());

	return 0;
}

static int nvt_timer_tm0_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_timer_tm0_proc_show, NULL);
}

static const struct proc_ops nvt_timer_tm0_fops = {
	.proc_open = nvt_timer_tm0_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static unsigned long nvt_test_memcpy_perf(struct seq_file *m, size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	char *ptr_buf2 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;

	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);
	ptr_buf2 = (char*)kzalloc(bufsize, GFP_KERNEL);

	time_before = nvt_get_time();
	for (i = 0; i < iters; ++i) {
		memcpy(ptr_buf1, ptr_buf2, bufsize);
	}
	time_after = nvt_get_time();
	kfree(ptr_buf1);
	kfree(ptr_buf2);
	return ((bufsize * iters) / (time_after - time_before));
}

static unsigned long nvt_test_memset_perf(struct seq_file *m, size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;

	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);

	time_before = nvt_get_time();
	for (i = 0; i < iters; ++i) {
		memset(ptr_buf1, i, bufsize);
	}
	time_after = nvt_get_time();
	kfree(ptr_buf1);
	return ((bufsize * iters) / (time_after - time_before));
}

static unsigned long nvt_test_memcmp_perf(struct seq_file *m, size_t bufsize, unsigned long iters)
{
	unsigned int i = 0;
	char *ptr_buf1 = NULL;
	char *ptr_buf2 = NULL;
	unsigned long time_before = 0;
	unsigned long time_after = 0;
	int ret = 0;
	
	ptr_buf1 = (char*)kzalloc(bufsize, GFP_KERNEL);
	ptr_buf2 = (char*)kzalloc(bufsize, GFP_KERNEL);
	memset(ptr_buf1, 0x55aa55aa, bufsize);
	memset(ptr_buf2, 0x55aa55aa, bufsize);

	time_before = nvt_get_time();
	for (i = 0; i < iters; ++i) {
		ret = memcmp(ptr_buf1, ptr_buf2, bufsize);
		if (ret != 0)
			break;
	}
	time_after = nvt_get_time();
	if (ret == 0)
		seq_printf(m, "\tCompare result: Same\n");
	else
		seq_printf(m, "\tCompare result: Different\n");

	kfree(ptr_buf1);
	kfree(ptr_buf2);
	return ((bufsize * iters) / (time_after - time_before));
}

static int nvt_test_L2_cache(struct seq_file *m, size_t size)
{
	void __iomem *mem_base_cache, *mem_base_noncache, *mem_base_wt;
	phys_addr_t phy_base_cache, phy_base_nocache, phy_base_cache_random;
	size_t hdal_size = 0;
	unsigned long time_before = 0;
	unsigned long time_after = 0;
	#ifdef CONFIG_OUTER_CACHE
	unsigned long flags;
	#endif
	struct device_node *node;
	struct property *prop;
	unsigned int *array;
	int length, i;

	node = of_find_node_by_name(NULL, "hdal-memory");
	if (node == NULL) {
		pr_err("Failed to get hdal-memory device node\n");
		return -EINVAL;
	}

	node = of_find_node_by_name(node, "media");
	if (node == NULL) {
		pr_err("Failed to get hdal-memory/media device node\n");
		return -EINVAL;
	}

	prop = of_find_property(node, "reg", &length);
	if (!prop) {
		pr_err("Failed to get hdal-memory/media/reg device node\n");
	}

	/* get one line elements (Bytes) */
	array = kmalloc(length, GFP_KERNEL);
	/* read hdal memory node and store it into array with length/4 (U32 unit) */
	of_property_read_u32_array(node, "reg", (u32 *)array, length/4);

	#ifdef CONFIG_ARM
	phy_base_cache = array[1];
	hdal_size = array[2];
	#else
	phy_base_cache = array[2];
	phy_base_cache = phy_base_cache << 32;
	phy_base_cache |= array[3];
	hdal_size = array[4];
	hdal_size = hdal_size << 32;
	hdal_size = array[5];
	#endif /* !CONFIG_ARM */
	/* The test buffer size is larger than hdal size. */
	if (size > hdal_size || size < SZ_1M * 100)
		return -1;

	phy_base_nocache = phy_base_cache;
	mem_base_cache = ioremap_cache(phy_base_cache, size);
	mem_base_noncache = ioremap(phy_base_nocache, size);
	mem_base_wt = ioremap_wt(phy_base_cache, size);

	phy_base_cache = fmem_lookup_pa((phys_addr_t)mem_base_cache);
	phy_base_nocache = fmem_lookup_pa((phys_addr_t)mem_base_noncache);
	phy_base_cache_random = (phys_addr_t)mem_base_cache;
	phy_base_cache_random += 0x2622d4;
	phy_base_cache_random = fmem_lookup_pa(phy_base_cache_random);

	seq_printf(m, "To test cacheable/non-cacheable/write-through performance\n");
	#ifdef CONFIG_ARM64
	seq_printf(m, "\tCACHE: phys: 0x%08llx virt: 0x%px\n", phy_base_cache, mem_base_cache);
	seq_printf(m, "\tCACHE (Random offset): phys: 0x%08llx virt: 0x%llx\n", phy_base_cache_random, (phys_addr_t)mem_base_cache + 0x2622d4);
	seq_printf(m, "\tNon-CACHE: phys: 0x%08llx virt: 0x%px\n", phy_base_nocache, mem_base_noncache);
	#else
	seq_printf(m, "\tCACHE: phys: 0x%08x virt: 0x%px\n", phy_base_cache, mem_base_cache);
	seq_printf(m, "\tNon-CACHE: phys: 0x%08x virt: 0x%px\n", phy_base_nocache, mem_base_noncache);
	#endif
	for (i=0;i<(size/8);i+=8)
	{
		*(unsigned long*)(mem_base_cache+i) = 0x55aa55aa;
	}

	/* To test different ioremap mapping types throughput */
	time_before = nvt_get_time();
	memcpy(mem_base_wt + SZ_16M, mem_base_wt, SZ_16M);
	time_after = nvt_get_time();
	seq_printf(m, "\tmemcpy wt non-cacheable memory (Data: 16M): %lu (MB/s)\n", SZ_16M/(time_after - time_before));
	time_before = nvt_get_time();
	memcpy(mem_base_cache + SZ_16M, mem_base_cache, SZ_16M);
	time_after = nvt_get_time();
	seq_printf(m, "\tmemcpy cacheable memory (Data: 16M): %lu (MB/s)\n", SZ_16M/(time_after - time_before));
	time_before = nvt_get_time();
	memcpy(mem_base_noncache + SZ_16M, mem_base_noncache, SZ_16M);
	time_after = nvt_get_time();
	seq_printf(m, "\tmemcpy wb non-cacheable memory (Data: 16M): %lu (MB/s)\n", SZ_16M/(time_after - time_before));

	#ifdef CONFIG_OUTER_CACHE
	time_before = nvt_get_time();
	spin_lock_irqsave(&test_l2_lock, flags);
	outer_flush_all();
	spin_unlock_irqrestore(&test_l2_lock, flags);
	time_after = nvt_get_time();
	seq_printf(m, "\tFlush all time (10M): %lu (us)\n", time_after - time_before);
	time_before = nvt_get_time();
	outer_flush_range(phy_base_cache, phy_base_cache + (SZ_1M * 10));
	time_after = nvt_get_time();
	seq_printf(m, "\tflush range time (10M): %lu (us)\n", (time_after - time_before));
	time_before = nvt_get_time();
	outer_clean_range(phy_base_cache, phy_base_cache + (SZ_1M * 10));
	time_after = nvt_get_time();
	seq_printf(m, "\tClean range time (10M): %lu (us)\n", (time_after - time_before));
	time_before = nvt_get_time();
	outer_inv_range(phy_base_cache, phy_base_cache + (SZ_1M * 10));
	time_after = nvt_get_time();
	seq_printf(m, "\tInv range time (10M): %lu (us)\n", (time_after - time_before));
	#endif

	iounmap(mem_base_cache);
	iounmap(mem_base_wt);
	iounmap(mem_base_noncache);
	kfree(array);
	return 0;
}

static int nvt_memperf_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "Memory performance testing results\n");
	seq_printf(m, "\tmemcpy: %lu MB/Sec. (Data 20MB)\n", nvt_test_memcpy_perf(m, 2000000, 1000));
	seq_printf(m, "\tmemset: %lu MB/Sec. (Data 20MB)\n", nvt_test_memset_perf(m, 2000000, 1000));
	seq_printf(m, "\tmemcmp: %lu MB/Sec. (Data 20MB)\n", nvt_test_memcmp_perf(m, 2000000, 1000));

	nvt_test_L2_cache(m, SZ_1M * 100);

	return 0;
}

static int nvt_memperf_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_memperf_proc_show, NULL);
}

static const struct proc_ops nvt_memperf_fops = {
	.proc_open = nvt_memperf_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static ssize_t nvt_memhotplug_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	char* const delim = "@";
	char *token = NULL;
	unsigned long mem_address = 0;
	unsigned long mem_size = 0;
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	res = -EFAULT;
	while ((token = strsep(&buf, delim)) != NULL) {
		if (mem_size == 0) {
			if (kstrtoul(token, 16, &mem_size) < 0)
				goto out;
		} else {
			if (kstrtoul(token, 16, &mem_address) < 0)
				goto out;
			break;
		}
	}

	res = memblock_add(mem_address, mem_size);
	if (res != 0) {
		pr_info("Got failures during add memory region 0x%08lx@0x%08lx\n", mem_size, mem_address);
		res = -1;
	} else {
		pr_info("Add memory region 0x%08lx@0x%08lx\n", mem_size, mem_address);
		res = count;
	}

out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_memhotplug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "Memory hotplug usage\n");
	seq_puts(m, "\tAdd memory region:\n");
	seq_puts(m, "\t$ echo Phys_size@Phys_addr > /proc/nvt_info/memhotplug\n");
	seq_puts(m, "\tPrint memory region:\n");
	seq_puts(m, "\t$ cat /sys/kernel/debug/memblock/memory\n");

	return 0;
}

static int nvt_memhotplug_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_memhotplug_proc_show, NULL);
}

static const struct proc_ops nvt_memhotplug_fops = {
	.proc_open = nvt_memhotplug_open,
	.proc_read = seq_read,
	.proc_write = nvt_memhotplug_write,
	.proc_release = single_release,
};

static int nvt_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Version: %s \n", NVT_UTS_RELEASE);

	return 0;
}

static int nvt_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_version_show, NULL);
}

static const struct proc_ops nvt_version_fops = {
	.proc_open = nvt_version_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

static ssize_t nvt_fmem_va_to_pa_test_write(struct file *file, const char __user *buffer,
						size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	char* const delim = " ";
	char *token = NULL;
	int res = 0;
	unsigned int num = 0;

	if (nvt_fmem_test_va_pa_addr_list == NULL)
		nvt_fmem_test_va_pa_addr_list = kzalloc(sizeof(ulong) * NVT_FMEM_LOOKUP_TEST_MAX, GFP_KERNEL);

	if (!nvt_fmem_test_va_pa_addr_list || !buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	res = -EFAULT;
	while ((token = strsep(&buf, delim)) != NULL) {
		if (kstrtoul(token, 16, (nvt_fmem_test_va_pa_addr_list+num)) < 0)
			goto out;
		num++;
		if (num > NVT_FMEM_LOOKUP_TEST_MAX)
			pr_err("Your test address list is wrong (num is %d)\n", num);
	}

	res = count;

out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_fmem_va_to_pa_test_show(struct seq_file *m, void *v)
{
	int i = 0;
	void __iomem *va = NULL;

	if (nvt_fmem_test_va_pa_addr_list == NULL)
		return 0;

	for (i=0;i<NVT_FMEM_LOOKUP_TEST_MAX; i++)
	{
		if (*(nvt_fmem_test_va_pa_addr_list + i) > 0) {
			seq_printf(m, "Test Physical Address: 0x%08lx \n", *(nvt_fmem_test_va_pa_addr_list + i));
			// To map with 4B(A32)/8B(A64) space.
			//va = ioremap(*(nvt_fmem_test_va_pa_addr_list + i), sizeof(*(nvt_fmem_test_va_pa_addr_list + i)));
			// To map with 1024MB space.
			va = ioremap(*(nvt_fmem_test_va_pa_addr_list + i), NVT_FMEM_LOOKUP_TEST_SIZE);
			memset(va, 0x5a, NVT_FMEM_LOOKUP_TEST_SIZE);
			seq_printf(m, "\r\tVirtual address (non-cacheable): 0x%08lx@0x%08lx\n", NVT_FMEM_LOOKUP_TEST_SIZE, (unsigned long)va);
			seq_printf(m, "\r\tfmem lookup pa (non-cacheable): 0x%08lx@0x%08lx\n", NVT_FMEM_LOOKUP_TEST_SIZE, (ulong)fmem_lookup_pa((phys_addr_t)va));
			iounmap(va);
			// To map with 4B(A32)/8B(A64) space.
			//va = ioremap_cache(*(nvt_fmem_test_va_pa_addr_list + i), sizeof(*(nvt_fmem_test_va_pa_addr_list + i)));
			// To map with 1024MB space.
			va = ioremap_cache(*(nvt_fmem_test_va_pa_addr_list + i), NVT_FMEM_LOOKUP_TEST_SIZE);
			memset(va, 0xa5, NVT_FMEM_LOOKUP_TEST_SIZE);
			seq_printf(m, "\r\tVirtual address (cacheable): 0x%08lx@0x%08lx\n", NVT_FMEM_LOOKUP_TEST_SIZE, (unsigned long)va);
			seq_printf(m, "\r\tfmem lookup pa (cacheable): 0x%08lx@0x%08lx\n", NVT_FMEM_LOOKUP_TEST_SIZE, (ulong)fmem_lookup_pa((phys_addr_t)va));
			iounmap(va);
		}
	}

	return 0;
}

static int nvt_fmem_va_to_pa_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_fmem_va_to_pa_test_show, NULL);
}

static const struct proc_ops nvt_fmem_va_to_pa_test_fops = {
	.proc_open = nvt_fmem_va_to_pa_test_open,
	.proc_read = seq_read,
	.proc_release = single_release,
	.proc_write = nvt_fmem_va_to_pa_test_write,
};

static int nvt_boot_source_show(struct seq_file *m, void *v)
{
	char *path = "/nvt_info";
	struct device_node *dt_node;
	const u32 *property;
	int len=0;

	dt_node = of_find_node_by_path(path);
	if (!dt_node) {
		seq_printf(m, "error Failed to find device-tree node: %s\n", path);
		return -ENODEV;
	}

	property = of_get_property(dt_node, "EMBMEM", &len);
	//  seq_printf(m, "(I) len=%d\n", len);
	seq_printf(m, "%s\n", (char *)property);

	return 0;
}

static int nvt_boot_source_open(struct inode *inode, struct file *file){
	return single_open(file, nvt_boot_source_show, NULL);
};

static const struct proc_ops nvt_boot_source_fops = {
	.proc_open = nvt_boot_source_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

static int nvt_hdal_mtd_num_show(struct seq_file *m, void *v){
	char path[64] = {0};
	struct device_node *dt_node = NULL;
	struct device_node *child = NULL;
	char id_name[4]={0};
	int i=0;
	const u32 *property;
	int len=0;
	int check_rootfs1=0;

	sprintf(path,"/nand@%lx", (unsigned long)NVT_NAND_BASE_PHYS);
	dt_node = of_find_node_by_path(path);
	if (!dt_node) {
		seq_printf(m, "error Failed to find device-tree node: %s\n", path);
		return 0;
	}

	dt_node = of_get_child_by_name(dt_node, "nvtpack");

	if(dt_node == NULL){
		seq_printf(m, "nvtpack node not found\n");
		return 0;
	}
	dt_node = of_get_child_by_name(dt_node, "index");
	if(dt_node == NULL){
		seq_printf(m, "index node not found\n");
		return 0;
	}

	while(1){
		sprintf(id_name,"id%d",i);
		child = of_get_child_by_name(dt_node, id_name);
		if(child == NULL){
			//seq_printf(m, "%s node not found\n",id_name);
			break;
		}
		property = of_get_property(child, "partition_name", &len);
		//  seq_printf(m, "(I) len=%d\n", len);
		//seq_printf(m, "%s\n", (char *)property);
		if(strcmp((char *)property,"rootfs1") == 0){    // rootfs1 is user partition
			check_rootfs1=1;
			break;
		}
		//check user partition, now user partition name 
		//seq_printf(m, "find %s \n",id_name);
		i++;
	}
	if(check_rootfs1 == 1){
		seq_printf(m,"%d\n",i);
	}
	else{
		seq_printf(m,"can not find rootfs1 (user partition),can not support mount user partition\n");
	}
	return 0;
}

static int nvt_hdal_mtd_num_open(struct inode *inode, struct file *file){

	return single_open(file, nvt_hdal_mtd_num_show, NULL);

}

static const struct proc_ops nvt_hdal_mtd_num_fops = {
	.proc_open = nvt_hdal_mtd_num_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

#ifdef CONFIG_NVT_STACK_CHECK

extern unsigned long stack_warn;
extern int remaining_min;
extern unsigned long remaining_stack;

static ssize_t nvt_stack_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	int res = -EINVAL;

	if (!buf) {
		pr_err("get free page fail\n");
		return -ENOMEM;
	}

	if (count < 1) {
		pr_err("count < 1\n");
		goto out;
	}

	if (copy_from_user(buf, buffer, count)) {
		pr_err("copy_from_user fail\n");
		goto out;
	}

	buf[count-1] = '\0';

	if (kstrtoul(buf, 10, &stack_warn) < 0) {
		pr_err("kstrtoul fail\n");
		goto out;
	}

	if ((stack_warn <= 0) || (stack_warn > THREAD_SIZE)) {
		pr_err("\nCommand error, value = 0x%lx\n", stack_warn);
		goto out;
	}
	pr_info("\nNew stack check value = %ld\n", stack_warn);
	remaining_min = THREAD_SIZE;

	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_stack_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "====== Stack Detect ======\n");
	seq_printf(m, "current stack usage %ld\n", THREAD_SIZE - remaining_stack);
	seq_printf(m, "total stack size %ld, reserve %ld\n", THREAD_SIZE, stack_warn);
	seq_printf(m, "Max stack usage %ld\n", THREAD_SIZE - remaining_min);

	return 0;
}

static int nvt_stack_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_stack_proc_show, NULL);
}

static const struct proc_ops nvt_stack_fops = {
	.proc_open = nvt_stack_open,
	.proc_read = seq_read,
	.proc_write = nvt_stack_write,
	.proc_release = single_release,
};
#endif /* CONFIG_NVT_STACK_CHECK */

#ifdef CONFIG_NVT_GPU_AFBC_SUPPORT
static ssize_t nvt_gpu_afbc_en_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	buf[count-1] = '\0';

	if (kstrtoul(buf, 10, &gpu_afbc_en) < 0) {
		pr_err("kstrtoul fail\n");
		goto out;
	}

	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_gpu_afbc_en_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%ld\n", gpu_afbc_en);

	return 0;
}

static int nvt_gpu_afbc_en_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_gpu_afbc_en_proc_show, NULL);
}

static const struct proc_ops nvt_gpu_afbc_en_fops = {
	.proc_open = nvt_gpu_afbc_en_open,
	.proc_read = seq_read,
	.proc_write = nvt_gpu_afbc_en_write,
	.proc_release = single_release,
};
#endif /* CONFIG_NVT_GPU_AFBC_SUPPORT */

#ifdef CONFIG_NVT_FB_DMABUF
static ssize_t nvt_fb_dmabuf_en_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *pos)
{
	char *buf = (char *) __get_free_page(GFP_USER);
	int res = 0;

	if (!buf)
		return -ENOMEM;

	res = -EFAULT;
	if (copy_from_user(buf, buffer, count))
		goto out;

	buf[count-1] = '\0';

	if (kstrtoul(buf, 10, &fb_dmabuf_en) < 0) {
		pr_err("kstrtoul fail\n");
		goto out;
	}

	res = count;
out:
	free_page((unsigned long)buf);
	return res;
}

static int nvt_fb_dmabuf_en_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%ld\n", fb_dmabuf_en);

	return 0;
}

static int nvt_fb_dmabuf_en_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_fb_dmabuf_en_proc_show, NULL);
}

static const struct proc_ops nvt_fb_dmabuf_en_fops = {
	.proc_open = nvt_fb_dmabuf_en_open,
	.proc_read = seq_read,
	.proc_write = nvt_fb_dmabuf_en_write,
	.proc_release = single_release,
};
#endif /* CONFIG_NVT_FB_DMABUF */

#ifdef CONFIG_NVT_AMP
extern void nvt_gic_target_parsing(struct seq_file *m);

static int nvt_gic_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "================ INT target ================\n");
	nvt_gic_target_parsing(m);

	return 0;
}

static int nvt_gic_open(struct inode *inode, struct file *file)
{
	return single_open(file, nvt_gic_proc_show, NULL);
}

static const struct proc_ops nvt_gic_fops = {
	.proc_open = nvt_gic_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};
#endif /* CONFIG_NVT_AMP */

#endif /* CONFIG_PROC_FS */

static int nvt_bootts_init(void)
{
	memset(g_bootts, 0, sizeof(g_bootts));
	g_index = NVT_BOOTTS_UBOOT_RESV_CNT;

	nvt_bootts_add_ts("ker");

	pr_info("NVTBOOTTS: %s initial success\n", __func__);

	return 0;
}

/**
 * nvt_get_suspend_mode - get suspend mode if chip is power on or power off.
 * return 0: chip is power on, 1: chip is power off
 */
int nvt_get_suspend_mode(void)
{
	return (int)suspend_mode;
}
EXPORT_SYMBOL(nvt_get_suspend_mode);

static int __init nvt_suspend_mode(void)
{
	char *pstr = NULL;
	char *psep = NULL;
	char symbol;
	int ret = 0;

	pstr = strstr(saved_command_line, "suspend_mode=");
	if (pstr) {
		pstr += strlen("suspend_mode=");
	}
	while (pstr) {
		psep = strpbrk(pstr, " "); //find ',' or ' ', or '\0'
		if (NULL == psep) {
			break;
		}

		symbol = *psep;
		*psep = '\0';
		suspend_mode = 0;
		ret = kstrtoul(pstr, 10, &suspend_mode);
		if (ret < 0) {
			pr_info("Error parsing power setting\n");
			return -1;
		}
		*psep = symbol;

		if (' ' == symbol) {
			break;
		}

		pstr = psep + 1;
	}
#ifdef CONFIG_PM
	pr_info("suspend mode = %ld\n", suspend_mode);
#endif
	return 0;
}
early_initcall(nvt_suspend_mode);

#ifdef CONFIG_PROC_FS
static int __init nvt_bootts_proc_init(void)
{
	struct proc_dir_entry *entry = NULL;

	nvt_info_dir_root = proc_mkdir("nvt_info", NULL);
	if (!nvt_info_dir_root)
		return -ENOMEM;

	entry = proc_create("bootts", 0664, nvt_info_dir_root, &nvt_bootts_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("tm0", 0664, nvt_info_dir_root, &nvt_timer_tm0_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("memperf", 0664, nvt_info_dir_root, &nvt_memperf_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("memhotplug", 0664, nvt_info_dir_root, &nvt_memhotplug_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("version", 0664, nvt_info_dir_root, &nvt_version_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("fmem_va_to_pa_test", 0664, nvt_info_dir_root, &nvt_fmem_va_to_pa_test_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("boot_source", 0664, nvt_info_dir_root,&nvt_boot_source_fops);
	if (!entry)
		return -ENOMEM;

	entry = proc_create("hdal_part_num", 0664, nvt_info_dir_root,&nvt_hdal_mtd_num_fops);
	if (!entry)
		return -ENOMEM;

#ifdef CONFIG_NVT_STACK_CHECK
	entry = proc_create("stack", 0664, nvt_info_dir_root,&nvt_stack_fops);
	if (!entry)
		return -ENOMEM;
#endif

#ifdef CONFIG_NVT_GPU_AFBC_SUPPORT
	entry = proc_create("gpu_afbc_en", 0664, nvt_info_dir_root,&nvt_gpu_afbc_en_fops);
	if (!entry)
		return -ENOMEM;
#endif /* CONFIG_NVT_GPU_AFBC_SUPPORT */

#ifdef CONFIG_NVT_FB_DMABUF
	entry = proc_create("fb_dmabuf_en", 0664, nvt_info_dir_root,&nvt_fb_dmabuf_en_fops);
	if (!entry)
		return -ENOMEM;
#endif /* CONFIG_NVT_FB_DMABUF */

#ifdef CONFIG_NVT_AMP
	entry = proc_create("nvt_int_target", 0664, nvt_info_dir_root,&nvt_gic_fops);
	if (!entry)
		return -ENOMEM;
#endif /* CONFIG_NVT_AMP */

	nvt_bootts_init();
	pr_info("NVTBOOTTS: %s initial success\n", __func__);

	return 0;
}

early_initcall(nvt_bootts_proc_init);
#endif

#if (!IS_ENABLED(CONFIG_PCI) && !IS_ENABLED(CONFIG_NVT_NT98690_PCI_EP_EVB) && !IS_ENABLED(CONFIG_NVT_NT98336_PCI_EP_EVB))
//Provide basic APIs for rHDAL if pcie driver not exist
//CONFIG_PCI: RC with common PCIe, x86 rc driver
//CONFIG_NVT_NT98690_PCI_EP_EVB: EP daughter board, x86 ep driver
int nvt_pciedev_get_my_busid(void)
{
	return 0; //RC bus id is 0
}
EXPORT_SYMBOL(nvt_pciedev_get_my_busid);

int nvt_pciedev_get_ep_info(void *ret_info)
{
	return -1;
}
EXPORT_SYMBOL(nvt_pciedev_get_ep_info);

int nvt_pciedev_get_ep_resource(int ep_idx, void *ret_resource)
{
	return -1;
}
EXPORT_SYMBOL(nvt_pciedev_get_ep_resource);
#endif
