/**
    NVT dma drivers

    @file       dma_proc.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sizes.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/seq_file.h>

#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <kwrap/debug.h>
#include <kwrap/mem.h>

#include "dma_nvt.h"

#define IS_PROC_ENABLE      0 //0: default, 1: for debug

#define PROC_ROOT_DIR       "nvt_dmasys"
#define PROC_CMD_LENGTH     64
#define PROC_ARG_NUM        6

#if IS_PROC_ENABLE //---------------------------------------------

typedef struct proc_cmd {
	char cmd[PROC_CMD_LENGTH];
	int (*pfunc)(int argc, char** argv);
} PROC_CMD;

static struct proc_dir_entry *gp_proc_root = NULL;

static int _proc_cmd_test(int argc, char** argv)
{
	struct vos_mem_info_t src_mem = {0};
	struct vos_mem_info_t dst_mem = {0};
	VOS_MEM_CMA_HDL src_hdl = NULL;
	VOS_MEM_CMA_HDL dst_hdl = NULL;

	nvtpcie_ddrid_t src_ddrid = CHIP_RC;
	nvtpcie_ddrid_t dst_ddrid = CHIP_RC;

	size_t copy_size = 128;
	int ret;

	//prepare src buffer
	if (0 != vos_mem_init_info(&src_mem, VOS_MEM_CMA_TYPE_CACHE, SZ_1M, NULL)) {
		pr_err("init src_mem failed\r\n");
		ret = -1;
		goto dma_exit;
	}

	src_hdl = vos_mem_alloc_contiguous(&src_mem);
	if (NULL == src_hdl) {
		pr_err("alloc src failed\r\n");
		ret = -1;
		goto dma_exit;
	}

	pr_info("src_buf.paddr 0x%lx\r\n", src_mem.paddr);
	pr_info("src_buf.size 0x%lx\r\n", src_mem.size);
	memset((void *)src_mem.vaddr, 0xAA, src_mem.size);
	pr_info("memset src_buf to 0x%x\r\n", *(unsigned char *)src_mem.vaddr);

	//prepare dst buffer
	if (0 != vos_mem_init_info(&dst_mem, VOS_MEM_TYPE_NONCACHE, SZ_1M, NULL)) {
		pr_err("init dst_mem failed\r\n");
		ret = -1;
		goto dma_exit;
	}

	dst_hdl = vos_mem_alloc_contiguous(&dst_mem);
	if (NULL == dst_hdl) {
		pr_err("alloc src failed\r\n");
		ret = -1;
		goto dma_exit;
	}
	pr_info("dst_buf.paddr 0x%lx\r\n", dst_mem.paddr);
	pr_info("dst_buf.size 0x%lx\r\n", dst_mem.size);
	memset((void *)dst_mem.vaddr, 0x55, dst_mem.size);
	pr_info("memset dst_buf to 0x%x\r\n", *(unsigned char *)dst_mem.vaddr);

#if IS_ENABLED(CONFIG_NVT_PCIE_LIB)
	{//get ddr id
		nvtpcie_chipid_t loc_chipid = nvtpcie_get_my_chipid();

		src_ddrid = nvtpcie_get_ddrid(loc_chipid, src_mem.paddr);
		if (src_ddrid >= DDR_ID_MAX) {
			pr_err("get ddrid of src pa(0x%lx) failed\r\n", src_mem.paddr);
			ret = -1;
			goto dma_exit;
		}

		dst_ddrid = nvtpcie_get_ddrid(loc_chipid, dst_mem.paddr);
		if (src_ddrid >= DDR_ID_MAX) {
			pr_err("get ddrid of dst pa(0x%lx) failed\r\n", dst_mem.paddr);
			ret = -1;
			goto dma_exit;
		}
	}
#endif

	//call dma copy
	if (0 != nvtmem_dma_memcpy(dst_ddrid, dst_mem.paddr, src_ddrid, src_mem.paddr, copy_size)) {
		pr_err("dma cpy failed\r\n");
		ret = -1;
		goto dma_exit;
	}

	//check results
	pr_info("dst_buf after dma cpy 0x%x\r\n", *(unsigned char *)dst_mem.vaddr);

	//call dma set
	if (0 != nvtmem_dma_memset(dst_ddrid, dst_mem.paddr + copy_size, copy_size, 0x11)) {
		pr_err("dma set failed\r\n");
		ret = -1;
		goto dma_exit;
	}

	//check results
	pr_info("dst_buf after dma set 0x%x\r\n", *(unsigned char *)(dst_mem.vaddr + copy_size));

dma_exit:
	if (src_hdl) {
		if (0 != vos_mem_release_contiguous(src_hdl)) {
			pr_err("release src_hdl failed\r\n");
		}
	}

	if (dst_hdl) {
		if (0 != vos_mem_release_contiguous(dst_hdl)) {
			pr_err("release dst_hdl failed\r\n");
		}
	}

	return ret;
}

static int _proc_cmd_version(int argc, char** argv)
{
	return 0;
}

static PROC_CMD g_proc_cmd_list[] = {
	{"test", _proc_cmd_test},
	{"version", _proc_cmd_version},
};

static void _proc_cmd_usage(void)
{
	unsigned int cmd_idx;

	pr_info("Usage: echo xxx > cmd\n");
	pr_info("---------------------\n");
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		pr_info("%s\n", g_proc_cmd_list[cmd_idx].cmd);
	}
}

static int _proc_cmd_show(struct seq_file *sfile, void *v)
{
	_proc_cmd_usage();
	return 0;
}

static int _proc_cmd_open(struct inode *inode, struct file *file)
{
	return single_open(file, _proc_cmd_show, NULL);
}

static ssize_t _proc_cmd_write(struct file *file, const char __user *buf,
                                  size_t size, loff_t *off)
{
	char cmd_line[PROC_CMD_LENGTH];
	const char delimiters[] = {' ', '\r', '\n', '\0'};
	char *argv[PROC_ARG_NUM] = {0};
	unsigned char argc = 0;

	char *cmdstr = cmd_line;
	unsigned int cmd_idx;
	int ret = -EINVAL;

	// Note: In fact, size will never be smaller than 1 in proc cmd
	//       to prevent coverity warning, we still check the size < 1
	// e.g. cmd [abcde], size = 6
	// e.g. cmd [], size = 1
	if (size > PROC_CMD_LENGTH || size < 1) {
		pr_err("Invalid cmd size %ld\n", (unsigned long)size);
		goto ERR_OUT;
	}

	// copy command string from user space
	if (copy_from_user(cmd_line, buf, size))
		goto ERR_OUT;

	cmd_line[size - 1] = '\0';

	pr_info("PROC_CMD: [%s], size %ld\n", cmd_line, (unsigned long)size);

	// parse command string
	for (argc = 0; argc < PROC_ARG_NUM; argc++) {
		argv[argc] = strsep(&cmdstr, delimiters);

		if (argv[argc] == NULL)
			break;
	}

	// dispatch command handler
	for (cmd_idx = 0 ; cmd_idx < ARRAY_SIZE(g_proc_cmd_list); cmd_idx++) {
		if (strncmp(argv[0], g_proc_cmd_list[cmd_idx].cmd, PROC_CMD_LENGTH) == 0) {
			ret = g_proc_cmd_list[cmd_idx].pfunc(argc - 1, &argv[1]);
			break;
		}
	}
	if (cmd_idx >= ARRAY_SIZE(g_proc_cmd_list))
		goto ERR_INVALID_CMD;

	return size;

ERR_INVALID_CMD:
	pr_err("Invalid CMD [%s]\n", cmd_line);
	_proc_cmd_usage();

ERR_OUT:
	return -EINVAL;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations _proc_cmd_fops = {
	.owner   = THIS_MODULE,
	.open    = _proc_cmd_open,
	.read    = seq_read,
	.write   = _proc_cmd_write,
	.llseek  = seq_lseek,
	.release = single_release,
};
#else
static struct proc_ops _proc_cmd_fops = {
	.proc_open    = _proc_cmd_open,
	.proc_read    = seq_read,
	.proc_write   = _proc_cmd_write,
	.proc_lseek  = seq_lseek,
	.proc_release = single_release,
};
#endif

int dma_proc_init(void *data_ptr)
{
	struct proc_dir_entry *entry;

	gp_proc_root = proc_mkdir(PROC_ROOT_DIR, NULL);
	if (!gp_proc_root) {
		return -ENOMEM;
	}

	entry = proc_create_data("cmd", 0664, gp_proc_root,
					    &_proc_cmd_fops, data_ptr);
	if (!entry) {
		pr_err("%s: Create cmd node error \n", __func__);
		return -ENOMEM;
	}

	pr_info("%s: Init successfully\n", __func__);

	return 0;
}

int dma_proc_deinit(void *data_ptr)
{
	proc_remove(gp_proc_root);
	gp_proc_root = NULL;

	return 0;
}

#else //#if IS_PROC_ENABLE //---------------------------------------------
int dma_proc_init(void *data_ptr)
{
	return 0;
}
int dma_proc_deinit(void *data_ptr)
{
	return 0;
}
#endif //#if IS_PROC_ENABLE
