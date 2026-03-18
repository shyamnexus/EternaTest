/*#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>*/
#include "kdrv_ai_dbg.h"
#include "ai_proc.h"
#include "ai_main.h"
#include "ai_api.h"
#include "kwrap/mem.h"
#include <kwrap/file.h>
#include "../../../include/kdrv_ai_version.h"
#include "ai_emu.h"
#include "kdrv_ai.h"
#include "kwrap/error_no.h"
#include "jmisp_eng.h"

#if defined(__FREERTOS)
#include <stdlib.h>
#include <string.h>
#include "efuse_protected.h"
#else
#include <plat/efuse_protected.h>
#endif
#define EMUF_CEILING(a, n)              (((a) + ((n)-1)) & (~((n)-1)))

#define KDRV_AI_ECHO_TEST 0

KDRV_AI_JMISP_PATH_INFO g_path_info = {0};

ER kdrv_ai_chk_efuse_ai_api(VOID)
{
#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_)
	if(efuse_check_available(NULL) != TRUE){
		DBG_ERR("objver mismatch in kdrv ai_api\r\n");
		return E_CTX;
	}
#endif

	return E_OK;
}

#if !defined(CONFIG_NVT_SMALL_HDAL)
int nvt_ai_api_read_reg(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr = 0;

	if (argc != 1) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}

	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
#if defined(__FREERTOS)
	if ((reg_addr = strtoul(pargv[0], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
#else
	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
#endif

	nvt_ai_drv_read_reg(pmodule_info, reg_addr);

	return 0;
}
#endif

#if KDRV_AI_ECHO_TEST
#include "kwrap/cpu.h"

static int load_data(char* path, uintptr_t addr)
{
	VOS_FILE fd;
	int len = 0;
	struct vos_stat f_stat;
	//struct file *filp  = NULL;
 
	if (1) {

		fd = vos_file_open(path, O_RDONLY, 0);
		if ((VOS_FILE)(-1) == fd) {
			//nvt_dbg(ERR, "failed in file open:%s\r\n", path);
			return 0;
		}
			
		if (vos_file_fstat(fd, &f_stat) != 0) {
			DBG_ERR("fstat error\r\n");
			vos_file_close(fd);
			return 0;
		}
		len = vos_file_read(fd, (void *)addr, f_stat.st_size);
		if (len != (int)f_stat.st_size) {
			DBG_ERR("read file error\r\n");
			vos_file_close(fd);
			return 0;
		}
		vos_file_close(fd);
	}
	return len;
}

INT32 test_rou_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_rou_cb LL0 occur\n");
	return 0;
}

INT32 test_rou_cb_LL1(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_rou_cb LL1 occur\n");
	return 0;
}

INT32 test_nue2_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_nue2_cb occur\n");
	return 0;
}

INT32 test_conv_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_conv_cb LL0 occur\n");
	return 0;
}

INT32 test_conv_cb_LL1(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_conv_cb LL1 occur\n");
	return 0;
}

INT32 test_jobm_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_jobm_cb occur\n");
	return 0;
}

INT32 test_lsu_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_lsu_cb occur\n");
	return 0;
}

INT32 test_util_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_util_cb LL0 occur\n");
	return 0;
}

INT32 test_util_cb_LL1(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_util_cb LL1 occur\n");
	return 0;
}

INT32 test_cal_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_cal_cb LL0 occur\n");
	return 0;
}

INT32 test_cal_cb_LL1(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_cal_cb LL1 occur\n");
	return 0;
}

INT32 test_ppu_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_ppu_cb occur\n");
	return 0;
}

INT32 test_pou_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_pou_cb occur\n");
	return 0;
}

INT32 test_jmisp_cb(UINT32 a, UINT32 b, UINT32 c, UINT32 d, UINT32 e, void * f)
{
	printk("test_jmisp_cb occur\n");
	return 0;
}

INT32 test_jm_single(UINT32 step) 
{
	uintptr_t jl_pa = 0, jl_va = 0;
	static KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
	
	if (step == 0) {
		// test function (JOBM)
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		uintptr_t conv_ll_pa = 0;
		uintptr_t rou_ll_pa = 0;
		
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		kdrv_ai_init();
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// prepare CONV part
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		conv_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_conv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//CONV//convg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		
		printk("[kdrv_ai] start update cmd\n");
		
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare ROU part
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		rou_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ll_cmd_rou.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UNIT//ROU//roug10000//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare JOBM part
		jl_pa = usable_pa;
		jl_va = usable_va;
		cur_size = 0;
		// set cmd
		{
			UINT32* p_jl = (UINT32*)jl_va;
			UINT32 jl_idx = 0;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 1 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 2 << 8;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 3 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 4 << 8;
			// set NULL
			p_jl[jl_idx++] = 0;
			cur_size = jl_idx*4;
			vos_cpu_dcache_sync(jl_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		}
		
		// set debug
		jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_SINGLE;
		jm_dbg.single_jl_id = KDRV_AI_JL_E;
		kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
		
		// set job end callback
		{
			KDRV_AI_CBFUNC_PARAM cb_parm;
			KDRV_AI_ISRCB test_cb_func = &test_jobm_cb;
			
			cb_parm.eng = KDRV_AI_ENG_JOBM_JLE_DBG;
			cb_parm.isrcb_fp = test_cb_func;
			kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		}
				
		// run JOBM
		kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLE_DBG, jl_pa);
		printk("JOBM 1st process done\n");
	} else {
		kdrv_ai_jm_dbg_run_next(jm_dbg);
		printk("JOBM next process done\n");
	}
	
	return 0;
}

INT32 test_kdrv_ai_func(VOID)
{
	uintptr_t jl_pa = 0, jl_va = 0;
	
	kdrv_ai_init();
	kdrv_ai_uninit();
	kdrv_ai_init();
	
	{ // CONV LL0
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_conv_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test conv set base addr\n");
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV1;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_CONV1;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);		
	}
	
	{ // CONV LL1
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_conv_cb_LL1;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test conv ll1 set base addr\n");
		for (i = 0; i < 5; i++) {
			base_addr_parm.eng = KDRV_AI_ENG_CONV2;
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A+i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_CONV2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);		
	}
	
	{ // NUE2
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_nue2_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		
		printk("test nue2 set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_NUE2;
		base_addr_parm.pa = (uintptr_t)0x5A5A5A5A | ((uintptr_t)0x2 << 32);
		base_addr_parm.base_id = 0;
		kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_NUE2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // PPU
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_ppu_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test ppu set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_PPU;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_PPU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // POU
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_pou_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test pou set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_POU;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_POU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}

	{ // LSU
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_lsu_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test lsu set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_LSU;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_LSU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // JM
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_jobm_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLA;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLB;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLC;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLD;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		base_addr_parm.eng = KDRV_AI_ENG_JOBM_JLE;
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLA;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLB;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLC;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLD;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
		
		cb_parm.eng = KDRV_AI_ENG_JOBM_JLE;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	/*
	{ // UTIL LL0
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_util_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test util set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_UTIL;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_UTIL;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // UTIL LL1
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_util_cb_LL1;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test util set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_UTIL2;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_UTIL2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}	
	*/
	{ // ROU LL0
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_rou_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_ROU;
		base_addr_parm.pa = (uintptr_t)0x5A5A5A5A | ((uintptr_t)0x2 << 32);
		base_addr_parm.base_id = 0;
		kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_ROU;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // ROU LL1
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_rou_cb_LL1;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		
		printk("test set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_ROU2;
		base_addr_parm.pa = (uintptr_t)0x5A5A5A5A | ((uintptr_t)0x2 << 32);
		base_addr_parm.base_id = 0;
		kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_ROU2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // CAL LL0
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_cal_cb;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test cal set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_CAL;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_CAL;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
	
	{ // CAL LL1
		// test set base addr
		KDRV_AI_BASE_ADDR_PARAM base_addr_parm;
		KDRV_AI_ISRCB test_cb_func = &test_cal_cb_LL1;
		KDRV_AI_CBFUNC_PARAM cb_parm;
		int i = 0;
		
		printk("test cal set base addr\n");
		base_addr_parm.eng = KDRV_AI_ENG_CAL2;	
		for (i = 0; i < 5; i++) {
			base_addr_parm.pa = (uintptr_t)(0x5A5A5A5A-i) | ((uintptr_t)0x2 << 32);
			base_addr_parm.base_id = i;
			kdrv_ai_set(KDRV_AI_PARAM_BASE_ADDR, &base_addr_parm);
		}
		printk("test set base addr done\n");
		
		// test set cb
		cb_parm.eng = KDRV_AI_ENG_CAL2;
		cb_parm.isrcb_fp = test_cb_func;
		kdrv_ai_set(KDRV_AI_PARAM_CBFUNC, &cb_parm);
	}
		
	// test function (CONV)
	if (1) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CONV//convg6//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CONV//convg6//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		if(0){			
			kdrv_ai_trigger(KDRV_AI_ENG_CONV2, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_CONV2)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_CONV2, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			dma_out_pa += cur_size;
			printk("CONV1 LL1 process done\n");
		}else{
			kdrv_ai_trigger(KDRV_AI_ENG_CONV1, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_CONV1)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_CONV1, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			dma_out_pa += cur_size;
			printk("CONV1 process done\n");
		}
	}
	
	// test function (NUE2)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//NUE2//ll_cmd_nue2.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//NUE2//DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x08) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x18) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_NUE2, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_NUE2)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_NUE2, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("NUE2 process done\n");
		
	}
	
	// test function (PPU)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//PPU//ppu_1//ll_cmd_ppu_kdrv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//PPU//ppu_1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_PPU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_PPU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_PPU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("PPU process done\n");
	}
	
	// test function (POU)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//PPU//ppu_1//ll_cmd_ppu_kdrv.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//PPU//ppu_1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_POU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_POU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_POU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("POU process done\n");
	}
	
	// test function (LSU)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//LSU//lsug657//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//LSU//lsug657//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x58) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		kdrv_ai_trigger(KDRV_AI_ENG_LSU, ll_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_LSU)) {
			printk("reset fail\n");
		}
		kdrv_ai_trigger(KDRV_AI_ENG_LSU, ll_pa);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		printk("LSU process done\n");
		
	}
	
	// test function (JOBM)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		uintptr_t conv_ll_pa = 0;
		uintptr_t rou_ll_pa = 0;
		
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// prepare CONV part
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		conv_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CONV//convg1//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CONV//convg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		
		printk("[kdrv_ai] start update cmd\n");
		
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x120) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare ROU part
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		rou_ll_pa = ll_pa;
		snprintf(io_path, 64, "//mnt//sd//Pattern//ROU//roug1//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//ROU//roug1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		// prepare JOBM part
		jl_pa = usable_pa;
		jl_va = usable_va;
		cur_size = 0;
		// set cmd
		{
			UINT32* p_jl = (UINT32*)jl_va;
			UINT32 jl_idx = 0;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 1 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 2 << 8;
			// trigger CONV
			p_jl[jl_idx++] = 4 | (0<<4) | (1<<8) | (1<<16);
			p_jl[jl_idx++] = conv_ll_pa;
			p_jl[jl_idx++] = 3 << 8;
			// trigger ROU
			p_jl[jl_idx++] = 4 | (3<<4) | (2<<8) | (1<<16);
			p_jl[jl_idx++] = rou_ll_pa;
			p_jl[jl_idx++] = 4 << 8;
			// set NULL
			p_jl[jl_idx++] = 0;
			cur_size = jl_idx*4;
			vos_cpu_dcache_sync(jl_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		}
		
		// test busy count
		{
			KDRV_AI_JOBM_BUSY_CNT_INFO cnt_info = {0};
			kdrv_ai_set(KDRV_AI_PARAM_JOBM_BUSY_CNT, &cnt_info);
		}
		
		// run JOBM
		/*kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jl_pa);
		if (kdrv_ai_engine_reset(KDRV_AI_ENG_JOBM_JLA)) {
			printk("reset fail\n");
		}*/
		kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jl_pa);
		printk("JOBM process done\n");
		
		// test busy count
		{
			KDRV_AI_JOBM_BUSY_CNT_INFO cnt_info = {0};
			int i = 0;
			kdrv_ai_get(KDRV_AI_PARAM_JOBM_BUSY_CNT, &cnt_info);
			for (i = 0; i < 8; i++) printk("cnt = %d\n", cnt_info.busy_cnt[i]);
		}
	}
	
	// test function (UTIL)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UTIL//utilg1471//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//UTIL//utilg1471//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x90) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		if(1){			
			kdrv_ai_trigger(KDRV_AI_ENG_UTIL2, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_UTIL2)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_UTIL2, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("UTIL LL1 process done\n");
		}else{
			kdrv_ai_trigger(KDRV_AI_ENG_UTIL, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_UTIL)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_UTIL, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("UTIL process done\n");
		}
	}
	
	// test function (ROU) 
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//ROU//roug1//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//ROU//roug1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x68) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		if(0){			
			kdrv_ai_trigger(KDRV_AI_ENG_ROU2, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_ROU2)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_ROU2, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("ROU LL1 process done\n");
		}else{
			kdrv_ai_trigger(KDRV_AI_ENG_ROU, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_ROU)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_ROU, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("ROU process done\n");
		}
	}
	
	// test function (CAL)
	if (0) {
		struct vos_mem_info_t buf_info = {0};
		int buf_size = 1024*1024;
		VOS_MEM_HDL vos_mem_id = 0;
		uintptr_t buffer_pa = 0;
		uintptr_t buffer_va = 0;
		uintptr_t usable_pa = 0;
		uintptr_t usable_va = 0;
		uintptr_t dma_in_pa = 0, dma_in_va = 0;
		uintptr_t dma_out_pa = 0, dma_out_va = 0;
		uintptr_t ll_pa = 0, ll_va = 0;
		UINT32 ll_size = 0;
		UINT32 cur_size = 0;
		char io_path[64];
		
		printk("[kdrv_ai] start alloc buffer\n");
		if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
			printk("nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
			return -1;
		} else {
			vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
			if (0 == vos_mem_id) {
				nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
				return -1;
			}
		}
		printk("[kdrv_ai]   end alloc buffer\n");
		buffer_pa = (uintptr_t)buf_info.paddr;
		buffer_va = (uintptr_t)buf_info.vaddr;
		
		usable_pa = buffer_pa;
		usable_va = buffer_va;
		
		// step 1. load cmd
		printk("[kdrv_ai] start load cmd\n");
		ll_pa = usable_pa; 
		ll_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CAL//calg1//ll_cmd.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		ll_size = cur_size;
		printk("[kdrv_ai] end load cmd\\n");
		
		// step 2. load dma in/out
		dma_in_pa = usable_pa; 
		dma_in_va = usable_va;
		snprintf(io_path, 64, "//mnt//sd//Pattern//CAL//calg1//LL0_DI0.bin");
		cur_size = load_data(io_path, usable_va);
		usable_pa += cur_size;
		usable_va += cur_size;
		vos_cpu_dcache_sync(dma_in_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		dma_out_pa = usable_pa; 
		dma_out_va = usable_va;
		usable_pa += cur_size;
		usable_va += cur_size;
		memset((VOID*)dma_out_va, 0, cur_size);
		vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
		
		printk("[kdrv_ai] start update cmd\n");
		// step 3. update cmd
		{
			UINT64 *p_ll_cmd = (UINT64*)ll_va;
			UINT32 ll_idx = 0;
			while (1) {
				if ((p_ll_cmd[ll_idx] & 0xF) == 0) {
					break;
				} 
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x50) {
					// update input address
					p_ll_cmd[ll_idx] |= (dma_in_pa << 32);
				}
				
				if (((p_ll_cmd[ll_idx] >> 8) & 0xFFF) == 0x88) {
					// update output address
					p_ll_cmd[ll_idx] |= (dma_out_pa << 32);
				}
				ll_idx++;
			
			}
			
			vos_cpu_dcache_sync(ll_va, ll_size, VOS_DMA_BIDIRECTIONAL);
		}
		printk("[kdrv_ai] end update cmd\n");
		
		if(1){			
			kdrv_ai_trigger(KDRV_AI_ENG_CAL2, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_CAL2)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_CAL2, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("CAL LL1 process done\n");
		}else{
			kdrv_ai_trigger(KDRV_AI_ENG_CAL, ll_pa);
			if (kdrv_ai_engine_reset(KDRV_AI_ENG_CAL)) {
				printk("reset fail\n");
			}
			kdrv_ai_trigger(KDRV_AI_ENG_CAL, ll_pa);
			vos_cpu_dcache_sync(dma_out_va, cur_size, VOS_DMA_BIDIRECTIONAL);
			printk("CAL process done\n");
		}
	}
			
	kdrv_ai_uninit();
	return jl_pa;
}



UINT32 rand(VOID) 
{
	UINT32 output = 0;
	get_random_bytes(&output, sizeof(UINT32));
	
	return output;
}

#endif

int nvt_ai_api_read_version(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{

	CHAR* kdrv_ai_version;
	kdrv_ai_version = KDRV_AI_IMPL_VERSION;
	DBG_DUMP("kdrv_ai_version:%s\n", kdrv_ai_version);
#if KDRV_AI_ECHO_TEST	
	test_kdrv_ai_func();
	if (0) {
		//static int jm_single_step = 0;
		//test_jm_single(jm_single_step++); 
	}
	if (0) {
	//test_kdrv_ai_func();
	//test_jmisp_func();
	}
	if (0) {
		static int jm_dbg_cnt = 0;
		static uintptr_t jm_jl_pa = 0;
		
		if (jm_dbg_cnt == 0) {
			jm_jl_pa = test_kdrv_ai_func();
		} else {
			if (jm_dbg_cnt < 6) {
				// set debug
				KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
				jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_SINGLE;
				jm_dbg.single_jl_id = KDRV_AI_JL_A;
				if (jm_dbg_cnt == 1) {
					kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
					kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jm_jl_pa);
				} else {
					kdrv_ai_jm_dbg_run_next(jm_dbg);
				}
				
			} else {
				// set debug
				KDRV_AI_JOBM_DBG_PARAM jm_dbg = {0};
				jm_dbg.dbg_mode = KDRV_AI_JM_DBG_MODE_ARB;
				jm_dbg.arb_eng = KDRV_AI_JM_DBG_ENG_CONV0 | KDRV_AI_JM_DBG_ENG_ROU;
				if (jm_dbg_cnt == 6) {
					kdrv_ai_set(KDRV_AI_PARAM_JOBM_DBG, &jm_dbg);
					kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, jm_jl_pa);
				} else {
					kdrv_ai_jm_dbg_run_next(jm_dbg);
				}
			}
		}
		jm_dbg_cnt++;
	}
#endif	
	return 0;
}

#if !defined(CONFIG_NVT_SMALL_HDAL)
int nvt_ai_api_write_reg(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	unsigned long reg_addr = 0, reg_value = 0;

	if (argc != 2) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}
	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}

#if defined(__FREERTOS)
	if ((reg_addr = strtoul(pargv[0], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
	if ((reg_value = strtoul(pargv[1], NULL, 16)) == 0) {
		nvt_dbg(ERR, "invalid reg value:%s\n", pargv[1]);
		return -1;

	}
#else
	if (kstrtoul(pargv[0], 0, &reg_addr)) {
		nvt_dbg(ERR, "invalid reg addr:%s\n", pargv[0]);
		return -1;
	}
	if (kstrtoul(pargv[1], 0, &reg_value)) {
		nvt_dbg(ERR, "invalid reg value:%s\n", pargv[1]);
		return -1;

	}
#endif

	nvt_dbg(IND, "W REG 0x%lx to 0x%lx\n", reg_value, reg_addr);

	nvt_ai_drv_write_reg(pmodule_info, reg_addr, reg_value);
	return 0;
}
#endif

int nvt_ai_api_write_pattern(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	VOS_FILE fd;
	int len = 0;
	//unsigned char *pbuffer;
	struct vos_mem_cma_info_t buf_info = {0};
	int ret = 0;
	VOS_MEM_CMA_HDL buf_info_id;

	if (argc != 1) {
		nvt_dbg(ERR, "wrong argument:%d", argc);
		return -1;
	}

	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
	
	fd = vos_file_open(pargv[0], O_RDONLY, 0);
	if ((VOS_FILE)-1 == fd) {
		nvt_dbg(ERR, "failed in file open:%s\r\n", pargv[0]);
		return -1;
	}

	//Allocate memory
	if (0 != vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_CACHE, 0x600000)) {
        nvt_dbg(ERR, "vos_mem_init_cma_info: init buffer fail. \r\n");
		vos_file_close(fd);
        return -1;
    } else {
        buf_info_id = vos_mem_alloc_from_cma(&buf_info);
		if (NULL == buf_info_id) {
            DBG_ERR("get buffer fail\n");
			nvt_dbg(ERR, "get buffer fail\n");
			vos_file_close(fd);
            return -1;
        }
    }

	len = vos_file_read(fd, (void *)buf_info.vaddr, 1152 * 64);
	/* Do something after get data from file */
	ret = vos_mem_release_from_cma(buf_info_id);
    if (ret != 0) {
        nvt_dbg(ERR, "failed in release buffer\n");
		vos_file_close(fd);
        return -1;
    }

	vos_file_close(fd);

	return len;
}

int nvt_kdrv_ai_api_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
	if (pargv == NULL) {
		nvt_dbg(ERR, "invalid reg = null\n");
		return -1;
	}
	return 0;
}

int nvt_kdrv_ai_func_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv)
{
#if (KDRV_AI_FUNC_TEST == 1)
	emu_ai(pargv);
#else
	nvt_dbg(ERR, "KDRV_AI: Error, please enable #define KDRV_AI_FUNC_TEST 1 in ai_emu.h and copy Makefile.kdrv_vfy to Makefile.\r\n");
#endif
	return 0;
}