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
#include "nue2_lib.h"
#include "kwrap/mem.h"
#include <kwrap/file.h>
#include "cnn/cnn_platform.h"
#include "nue/nue_platform.h"
#include "nue2/nue2_platform.h"
#include "kdrv_ai_version.h"
#include "nue/nue_emu.h"
#include "nue2/nue2_emu.h"
#include "cnn/cnn_emu.h"
#include "ai_ioctl.h"
#include "ai_emu.h"
#if defined(__FREERTOS)
#include <stdlib.h>
#include <string.h>
#endif
#include <linux/soc/nvt/fmem.h>

//#define DEBUG_IN_OUT nvt_dbg(ERR, "%s_%d:code_flow\r\n", __FUNCTION__, __LINE__);
#define DEBUG_IN_OUT

#if KDRV_AI_FUNC_TEST
static UINT32 kdrv_ai_trans_eng2id(KDRV_AI_ENG ai_eng)
{
	UINT32 eng_id = KDRV_AI_ENGINE_CNN;
	if (ai_eng == AI_ENG_CNN) {
		eng_id = KDRV_AI_ENGINE_CNN;
	} else if (ai_eng == AI_ENG_NUE) {
		eng_id = KDRV_AI_ENGINE_NUE;
	} else if (ai_eng == AI_ENG_NUE2) {
		eng_id = KDRV_AI_ENGINE_NUE2;
	} else if (ai_eng == AI_ENG_CNN2) {
		eng_id = KDRV_AI_ENGINE_CNN2;
	} else {
		nvt_dbg(ERR, "not support engine: %d\r\n", ai_eng);
	}
	return eng_id;
}

static int nvt_kdrv_ai_run_eng(KDRV_AI_ENG ai_eng, uintptr_t ll_cmd_addr)
{
	int er_return = 0;
	const UINT32 chip = 0, channel = 0;
	UINT32 id = 0;
	KDRV_AI_LL_INFO ll_info = {0};
	KDRV_AI_TRIG_MODE mode = AI_TRIG_MODE_LL;
	KDRV_AI_TRIGGER_PARAM trig_parm = {0};
	AI_DRV_OPENCFG cfg;

DEBUG_IN_OUT
	if (ll_cmd_addr == 0) {
		nvt_dbg(ERR, "nvt_kdrv_ai_run_eng: Error, ll_cmd_addr is 0.\r\n");
DEBUG_IN_OUT
		return -1;
	}
DEBUG_IN_OUT
	kdrv_ai_config_flow(1);
DEBUG_IN_OUT

	// opencfg
	cfg.opencfg.clock_sel = 0;
	cfg.engine = ai_eng;
	cfg.net_id = 0;
	id = KDRV_DEV_ID(chip, kdrv_ai_trans_eng2id(ai_eng), channel);
DEBUG_IN_OUT
	kdrv_ai_set(id, KDRV_AI_PARAM_OPENCFG, &cfg.opencfg);
DEBUG_IN_OUT
	// init eng
	nue_init();
	nue2_init();
	cnn_init(0);
DEBUG_IN_OUT
	
	// open eng
	er_return = kdrv_ai_open(chip, kdrv_ai_trans_eng2id(ai_eng));
	if (er_return != E_OK) {
		nvt_dbg(ERR, "nvt_kdrv_ai_run_eng: Error, kdrv_ai_open failed.\r\n");
DEBUG_IN_OUT
		goto RUN_ENG_FAIL;
	}
DEBUG_IN_OUT
	// set ll info
	ll_info.p_head = (KDRV_AI_LL_HEAD *)vmalloc(sizeof(KDRV_AI_LL_HEAD));
	if (ll_info.p_head == NULL) {
DEBUG_IN_OUT
		nvt_dbg(ERR, "nvt_kdrv_ai_run_eng: Error, malloc ll_head fail.\r\n");
		return -1;
	}
DEBUG_IN_OUT
	ll_info.head_cnt = 1;
	ll_info.p_head->parm_addr = ll_cmd_addr; //ll_cmd_addr should be physical address
	ll_info.p_head->eng = ai_eng;
	ll_info.p_head->mode = 0;
DEBUG_IN_OUT
	kdrv_ai_set(id, KDRV_AI_PARAM_MODE_INFO, &mode);
DEBUG_IN_OUT
    kdrv_ai_set(id, KDRV_AI_PARAM_LL_INFO, &ll_info);
DEBUG_IN_OUT
	
	er_return = kdrv_ai_trigger(id, &trig_parm, NULL, NULL);
	if (er_return != E_OK) {
		nvt_dbg(ERR, "nvt_kdrv_ai_run_eng: Error, kdrv_ai_trigger failed.\r\n");
DEBUG_IN_OUT
		goto RUN_ENG_FAIL;
	}
	er_return = kdrv_ai_close(chip, kdrv_ai_trans_eng2id(ai_eng));
	if (er_return != E_OK) {
		nvt_dbg(ERR, "nvt_kdrv_ai_run_eng: Error, kdrv_ai_close failed.\r\n");
DEBUG_IN_OUT
		goto RUN_ENG_FAIL;
	}
DEBUG_IN_OUT
	// uninit eng
	cnn_uninit(0);
	nue_uninit();
	nue2_uninit();
DEBUG_IN_OUT
RUN_ENG_FAIL:	
	vfree(ll_info.p_head);
DEBUG_IN_OUT
	return er_return;
}


typedef KDRV_AI_TEST_RESULT(*CONSTRUCT_LL_FP)(uintptr_t, uintptr_t, uintptr_t, UINT32, UINT32);
typedef KDRV_AI_TEST_RESULT(*GET_LL_BUF_SZ_FP)(UINT32*);
typedef KDRV_AI_TEST_RESULT(*CHECK_RST_FP)(uintptr_t, uintptr_t, uintptr_t, UINT32, UINT32);
#define KDRV_AI_TEST_ITEM_CNN       {cnn_construct_ll_cmd, cnn_get_ll_cmd_buf_size, cnn_check_result},
#define KDRV_AI_TEST_ITEM_NUE       {nue_construct_ll_cmd, nue_get_ll_cmd_buf_size, nue_check_result},
#define KDRV_AI_TEST_ITEM_NUE2      {nue2_construct_ll_cmd, nue2_get_ll_cmd_buf_size, nue2_check_result},
#define KDRV_AI_TEST_ITEM_CNN2       {cnn_construct_ll_cmd, cnn_get_ll_cmd_buf_size, cnn_check_result},
#define KDRV_AI_TEST_ITEM_CNN3       {cnn_construct_ll_cmd, cnn_get_ll_cmd_buf_size, cnn_check_result},

typedef struct {
	CONSTRUCT_LL_FP  p_func_construct_ll_cmd;
	GET_LL_BUF_SZ_FP p_func_get_ll_cmd_buf_size;
	CHECK_RST_FP     p_func_check_result;
} AITEST_LIST, *PAITEST_LIST;

static AITEST_LIST AI_TEST_LIST[] = {
	KDRV_AI_TEST_ITEM_CNN
	KDRV_AI_TEST_ITEM_NUE
	KDRV_AI_TEST_ITEM_NUE2
	KDRV_AI_TEST_ITEM_CNN2
	KDRV_AI_TEST_ITEM_CNN3
};

#define EMUF_CEILING(a, n)  (((a) + ((n)-1)) & (~((n)-1)))
#define EMUF_64_BYTE_ALIGN_CEILING(a)  EMUF_CEILING(a,64)

#define EMUF_MEM_PHYSICAL_START_ADDR 0x108800000
#define EMUF_MAX_ALLOC_BUF_NUM 10

uintptr_t g_buf_pa;
uintptr_t g_buf_va;
uintptr_t g_pa_addr = EMUF_MEM_PHYSICAL_START_ADDR;
uintptr_t g_emu_alloc_tb_va[EMUF_MAX_ALLOC_BUF_NUM] = {0};
uintptr_t g_emu_alloc_tb_pa[EMUF_MAX_ALLOC_BUF_NUM] = {0};
uintptr_t g_emu_alloc_idx = 0;

static int nvt_kdrv_ai_alloc_buffer(UINT32 buf_size, uintptr_t *buf_pa, uintptr_t *buf_va, VOS_MEM_HDL *vos_mem_id) 
{
#if 1
	struct vos_mem_info_t buf_info = {0};
	
	if (0 != vos_mem_init_info(&buf_info, VOS_MEM_TYPE_CACHE, buf_size, NULL)) {
		nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
		return -1;
	} else {
		*vos_mem_id = vos_mem_alloc_cma(&buf_info, 0);
		if (NULL == *vos_mem_id) {
			nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
			return -1;
		}
	}
	
	*buf_pa = g_buf_pa = (uintptr_t)buf_info.paddr;
	*buf_va = g_buf_va = (uintptr_t)buf_info.vaddr;
	g_pa_addr = g_buf_pa;
	
#else
#if 0
	struct vos_mem_cma_info_t buf_info = {0};
	
	if (0 != vos_mem_init_cma_info(&buf_info, VOS_MEM_CMA_TYPE_CACHE, buf_size)) {
		nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: init buffer fail\r\n");
		return -1;
	} else {
		*vos_mem_id = vos_mem_alloc_from_cma(&buf_info);
		if (NULL == *vos_mem_id) {
			nvt_dbg(ERR, "nvt_kdrv_ai_alloc_buffer: get buffer fail\n");
			return -1;
		}
	}
	*buf_pa = g_buf_pa = (uintptr_t)buf_info.paddr;
	*buf_va = g_buf_va = (uintptr_t)buf_info.vaddr;
#endif

	*buf_pa = g_buf_pa = g_pa_addr;
	*buf_va = g_buf_va = nue_platform_pa2va_remap(g_pa_addr, buf_size, 2);
#endif
	if (g_emu_alloc_idx < EMUF_MAX_ALLOC_BUF_NUM) {
		g_emu_alloc_tb_va[g_emu_alloc_idx] = g_buf_va;
		g_emu_alloc_tb_pa[g_emu_alloc_idx] = g_buf_pa;
		g_emu_alloc_idx++;
	} else {
		nvt_dbg(ERR, "%s_%d: Error to allocate buffer.\r\n", __FUNCTION__, __LINE__);
		return -1;
	}

	g_pa_addr += buf_size;
	g_pa_addr = EMUF_64_BYTE_ALIGN_CEILING(g_pa_addr);
	
	
	return 0;
}

static int nvt_kdrv_ai_release_buffer(VOS_MEM_CMA_HDL vos_mem_id)
{
#if 0
	return vos_mem_release_from_cma(vos_mem_id);
#endif
	UINT32 i;

	for (i = 0; i < EMUF_MAX_ALLOC_BUF_NUM; i++) {
		if ((g_emu_alloc_idx >= 0) && (g_emu_alloc_tb_va[g_emu_alloc_idx] != 0)) {
			nue_platform_pa2va_unmap(g_emu_alloc_tb_va[g_emu_alloc_idx], g_emu_alloc_tb_pa[g_emu_alloc_idx]);
			g_emu_alloc_idx--;
		}
	}

	return 0;
}

static int emu_run_ai_test(AITEST_LIST* p_run_list, UINT32 start_idx, UINT32 end_idx, KDRV_AI_ENG ai_eng, uintptr_t io_buf_pa, uintptr_t io_buf_va)
{
	UINT32 i = 0;
	INT32 ret = 0;
	UINT32 buf_size = 0;
	uintptr_t ll_buf_va = 0, ll_buf_pa = 0;
	VOS_MEM_CMA_HDL vos_mem_id;

	DBG_ERR("KDRV_AI_DEBUG: ai_eng=%d\r\n", ai_eng);
	
	// alloc ll buffer
	p_run_list[ai_eng].p_func_get_ll_cmd_buf_size(&buf_size);	
	ret = nvt_kdrv_ai_alloc_buffer(buf_size, &ll_buf_pa, &ll_buf_va, &vos_mem_id);
	if (ret == -1) {
		nvt_dbg(ERR, "emu_run_ai_test: failed in alloc buffer\n");
		return -1;
	}
	printk("ll buf pa/va = 0x%lx/0x%lx\n", (uintptr_t)ll_buf_pa, (uintptr_t)ll_buf_va);
	printk("io buf pa/va = 0x%lx/0x%lx\n", (uintptr_t)io_buf_pa, (uintptr_t)io_buf_va);
	

	// run pattern
	for (i = start_idx; i <= end_idx; i++) {

DEBUG_IN_OUT
		// construct ll cmd buffer
		ret = p_run_list[ai_eng].p_func_construct_ll_cmd(ll_buf_va, io_buf_pa, io_buf_va, i, i);
		if (ret == AI_RUN_FAIL) {
			nvt_dbg(ERR, "emu_run_ai_test: construct ll cmd fail\n");
DEBUG_IN_OUT
			goto RUN_EMU_FAIL; 
		} else if (ret == AI_RUN_SKIP) {
			nvt_dbg(WRN, "emu_run_ai_test: skip proc idx %d\n", (int)i);
DEBUG_IN_OUT
			continue; 
		}
DEBUG_IN_OUT
		// run engine through kdrv_ai flow
		ret = nvt_kdrv_ai_run_eng(ai_eng, ll_buf_va);
		if (ret != 0) {
			nvt_dbg(ERR, "emu_run_ai_test: nvt_kdrv_ai_run_eng fail\n");
DEBUG_IN_OUT
			goto RUN_EMU_FAIL; 
		}

DEBUG_IN_OUT
		
		// check output result
		ret = p_run_list[ai_eng].p_func_check_result(ll_buf_va, io_buf_pa, io_buf_va, i, i);
		if (ret != 0) {
			nvt_dbg(ERR, "emu_run_ai_test: check result fail\n");
DEBUG_IN_OUT
			goto RUN_EMU_FAIL; 
		}
		
	}

DEBUG_IN_OUT

	printk("pattern %d~%d compare OK ,ret = %d\n", (int)start_idx, (int)end_idx, ret);
RUN_EMU_FAIL:
	if (nvt_kdrv_ai_release_buffer(vos_mem_id) != 0) {
		nvt_dbg(ERR, "emu_run_ai_test: failed in release buffer\n");
DEBUG_IN_OUT
		return -1;
	}
DEBUG_IN_OUT
	
	return ret;
}

int emu_ai(char **pargv)
{
	KDRV_AI_ENG ai_eng;
	UINT32 index_start, index_end;
	VOS_MEM_CMA_HDL vos_mem_id;
	uintptr_t io_buf_pa = 0, io_buf_va = 0;
	int ret = 0;
	
	// check run engine
	if (strcmp(pargv[0], "cnn") == 0) {
		ai_eng = AI_ENG_CNN;
	} else if (strcmp(pargv[0], "nue") == 0) {
		ai_eng = AI_ENG_NUE;
	} else if (strcmp(pargv[0], "nue2") == 0) {
		ai_eng = AI_ENG_NUE2;
	} else if (strcmp(pargv[0], "cnn2") == 0) {
		ai_eng = AI_ENG_CNN2;
	} else if (strcmp(pargv[0], "cnn3") == 0) {
		ai_eng = AI_ENG_CNN3;
	}
	// check pattern index
	/*if (kstrtouint (pargv[1], 10, &index_start)) {
        nvt_dbg(ERR, "invalid debug_layer value:%s\n", pargv[1]);
        return -1;
    }
	if (kstrtouint (pargv[2], 10, &index_end)) {
        nvt_dbg(ERR, "invalid debug_layer value:%s\n", pargv[2]);
        return -1;
    }
	nvt_dbg(ERR, "index_start=%d, index_end=%d\r\n", index_start, index_end);*/

	ret = kstrtoint(pargv[1], 0, &index_start);
	ret = kstrtoint(pargv[2], 0, &index_end);
	g_pa_addr = EMUF_MEM_PHYSICAL_START_ADDR;

	// prepare io buffer for emulation
	ret = nvt_kdrv_ai_alloc_buffer(16*1024*1024, &io_buf_pa, &io_buf_va, &vos_mem_id);
	if (ret == -1) {
		nvt_dbg(ERR, "emu_ai: failed in alloc buffer\n");
		return ret;
	}
	
	ret = emu_run_ai_test(AI_TEST_LIST, index_start, index_end, ai_eng, io_buf_pa, io_buf_va);
	if (ret != 0) {
		nvt_dbg(ERR, "emu_ai: failed in emu_run_ai_test\n");
		goto RUN_FUNC_FAIL;
	}

RUN_FUNC_FAIL:	
	if (nvt_kdrv_ai_release_buffer(vos_mem_id) != 0) {
		nvt_dbg(ERR, "emu_ai: failed in release buffer\n");
		return -1;
	}

	g_pa_addr = EMUF_MEM_PHYSICAL_START_ADDR;
	
	return ret;
}
#endif
