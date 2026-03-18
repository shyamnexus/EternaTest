// #include <linux/kernel.h>
// #include <linux/slab.h>
// #include <linux/fs.h>
// #include <asm/uaccess.h>
// #include "hwcopy_api.h"
// #include "hwcopy_drv.h"
// #include "hwcopy_dbg.h"
// //#include "frammap/frammap_if.h" linux build fail.
// #include <linux/soc/nvt/nvtmem.h>
// #include "emu_hwcopy.h"
// #include <linux/dma-mapping.h>
// #include "kwrap/cpu.h"
// #include "kwrap/mem.h"



// int nvt_hwcopy_api_auto_test(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
// {


// 	struct vos_mem_cma_info_t  buf_info = {0};
//     void *handle= NULL;
// 	int ret = 0;

// 	//show_ddr_info();
//     memset(&buf_info, 0, sizeof(struct vos_mem_cma_info_t));

// 	buf_info.size = 0x100000;

// 	//buf_info.size = 0x100000;
// 	//buf_info.align = 64;      ///< address alignment
// 	//buf_info.name = "nvtmpp";
// 	buf_info.type = NVT_FMEM_ALLOC_CACHE;

// 	//ret = frm_get_buf_ddr(DDR_ID0, &buf_info);

//     ret = vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_CACHE, buf_info.size);
// 	if (ret >= 0) {
// 		handle = vos_mem_alloc_from_cma(&buf_info);
// 	} else {
// 		nvt_dbg(WRN, "frm allocate mem fail!!\r\n");
// 		return ret;
// 	}

//     //mem_dcache_sync(fbi->gui_buf[i], ge_info.size, DMA_BIDIRECTIONAL);

//     nvt_dbg(IND, "base: 0x%lx, id: 0x%d\n", (unsigned long) *(pmodule_info->io_addr), pmodule_info->chip_idx);
// 	nvt_dbg(IND, "VA: 0x%lx, PA: 0x%lx\n", (unsigned long) buf_info.vaddr,(unsigned long) buf_info.paddr);

//     emu_hwcopy_auto(buf_info.vaddr, buf_info.size,pmodule_info->chip_idx);
// 	nvt_dbg(IND, "%s: done\n", __func__);

// 	//frm_free_buf_ddr(buf_info.va_addr);

// 	return 0;
// }

// int nvt_hwcopy_api_write_reg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
// {
// 	unsigned long reg_addr = 0, reg_value = 0;

// 	if (argc != 2) {
// 		nvt_dbg(ERR, "wrong argument:%d", argc);
// 		return -EINVAL;
// 	}

// 	if (kstrtoul (pargv[0], 0, &reg_addr)) {
// 		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
// 		return -EINVAL;
// 	}

// 	if (kstrtoul (pargv[1], 0, &reg_value)) {
// 		nvt_dbg(ERR, "invalid rag value:%s\n", pargv[1]);
// 		return -EINVAL;

// 	}

// 	nvt_dbg(IND, "W REG 0x%lx to 0x%lx\n", reg_value, reg_addr);

// 	nvt_hwcopy_drv_write_reg(pmodule_info, reg_addr, reg_value);
// 	return 0;
// }

// int nvt_hwcopy_api_write_pattern(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
// {
// 	mm_segment_t old_fs;
// 	struct file *fp;
// 	int len = 0;
// 	//unsigned char *pbuffer;

// 	if (argc != 1) {
// 		nvt_dbg(ERR, "wrong argument:%d", argc);
// 		return -EINVAL;
// 	}

// 	fp = filp_open(pargv[0], O_RDONLY, 0);
// 	if (IS_ERR_OR_NULL(fp)) {
// 	    nvt_dbg(ERR, "failed in file open:%s\n", pargv[0]);
// 		return -EFAULT;
// 	}

// 	//pbuffer = kmalloc(256, GFP_KERNEL);
// 	//if (pbuffer == NULL) {
// 		//filp_close(fp, NULL);
// 		//return -ENOMEM;
// 	//}

// 	old_fs = get_fs();
// 	set_fs(get_ds());

// 	//len = vfs_read(fp, pbuffer, 256, &fp->f_pos); unknown symbol

// 	/* Do something after get data from file */

// 	//kfree(pbuffer);
// 	//pbuffer = NULL;
// 	filp_close(fp, NULL);
// 	set_fs(old_fs);

// 	return len;
// }

// int nvt_hwcopy_api_read_reg(PMODULE_INFO pmodule_info, unsigned char argc, char **pargv)
// {
// 	unsigned long reg_addr = 0;
// 	unsigned long value;

// 	if (argc != 1) {
// 		nvt_dbg(ERR, "wrong argument:%d", argc);
// 		return -EINVAL;
// 	}

// 	if (kstrtoul (pargv[0], 0, &reg_addr)) {
// 		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
// 		return -EINVAL;
// 	}

// 	nvt_dbg(IND, "R REG 0x%lx\n", reg_addr);
// 	value = nvt_hwcopy_drv_read_reg(pmodule_info, reg_addr);

// 	nvt_dbg(ERR, "REG 0x%lx = 0x%lx\n", reg_addr, value);
// 	return 0;
// }
