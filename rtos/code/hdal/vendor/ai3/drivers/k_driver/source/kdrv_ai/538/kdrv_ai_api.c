/**
    @file       kdrv_ai.c
    @ingroup    Predefined_group_name

    @brief      ai device abstraction layer

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

//#include <linux/module.h>
//#include <linux/kernel.h>
//#include <asm/io.h>
//#include <linux/uaccess.h>
//#include "mach/fmem.h"
#include "kwrap/semaphore.h"
//#include <linux/delay.h>
//#include <linux/timer.h>
//#include <linux/spinlock.h>

#include "ai_lib.h"
#include "kdrv_ai.h"
#include "kdrv_ai_version.h"
#include "kdrv_ai_int.h"
#include "kwrap/flag.h"
#include "kdrv_ai_platform.h"
#include "rou_eng.h"
#include "nue2_eng.h"
#include "conv_eng.h"
#include "jobm_eng.h"
#include "lsu_eng.h"
#include "util_eng.h"
#include "cal_eng.h"
#include "ppu_eng.h"
#include "jmisp_eng.h"
#include "pou_eng.h"

#if defined(__FREERTOS)
#include "kwrap/debug.h"
#include <string.h>
#include "ai_ioctl.h"
#include "efuse_protected.h"
#include "pll.h"
#else
#include "kdrv_ai_dbg.h"
#include <plat/efuse_protected.h>
#endif

#include "kwrap/cpu.h"
#include "ai_api.h"

#define PROF                DISABLE
#if PROF
static struct timeval tstart, tend;
#define PROF_START()    do_gettimeofday(&tstart);
#define PROF_END(msg)   do_gettimeofday(&tend);     \
	printk("%s time (us): %lu\r\n", msg,        \
		   (tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec));
#else
#define PROF_START()
#define PROF_END(msg)
#endif


static KDRV_AI_HANDLE g_kdrv_ai_handle[KDRV_AI_ENG_TOTAL] = {0};
UINT32 g_jmisp_int_en = 0xF;
unsigned int kdrv_ai_clk_gating[MODULE_CLK_NUM] = {0};
ER kdrv_ai_chk_efuse_kdrv_ai_api(VOID)
{
#if !defined(CONFIG_NVT_FPGA_EMULATION) && !defined(_NVT_FPGA_)
	if(efuse_check_available(NULL) != TRUE){
		DBG_ERR("objver mismatch in kdrv kdrv_ai_api\r\n");
		return E_CTX;
	}
#endif

	return E_OK;
}

static KDRV_AI_HANDLE* kdrv_ai_get_handle(KDRV_AI_ENG eng)
{
	if (eng == KDRV_AI_ENG_UNKNOWN || eng >= KDRV_AI_ENG_TOTAL) {
		return NULL;
	}
	
	return &g_kdrv_ai_handle[eng];
}

static int kdrv_ai_get_clk_info(KDRV_AI_CLK_INFO* clk_info)
{
	int ret = 0;
	switch (clk_info->eng) {
		case KDRV_AI_ENG_CONV1:{
			CONV_ENG_HANDLE *p_handle = conv_eng_get_handle(0, clk_info->eng - KDRV_AI_ENG_CONV1);
			clk_info->clk_rate = conv_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_LSU: {
			LSU_ENG_HANDLE *p_handle = lsu_eng_get_handle(0, 0);
			clk_info->clk_rate = lsu_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_UTIL: {
			UTIL_ENG_HANDLE *p_handle = util_eng_get_handle(0, 0);
			clk_info->clk_rate = util_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_ROU: {
			ROU_ENG_HANDLE *p_handle = rou_eng_get_handle(0, 0);
			clk_info->clk_rate = rou_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_CAL: {
			CAL_ENG_HANDLE *p_handle = cal_eng_get_handle(0, 0);
			clk_info->clk_rate = cal_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_NUE2: {
			NUE2_ENG_HANDLE *p_handle = nue2_eng_get_handle(0, 0);
			clk_info->clk_rate = nue2_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_PPU: {
			PPU_ENG_HANDLE *p_handle = ppu_eng_get_handle(0, 0);
			clk_info->clk_rate = ppu_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_POU: {
			POU_ENG_HANDLE *p_handle = pou_eng_get_handle(0, 0);
			clk_info->clk_rate = pou_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_JOBM_JLA: 
		case KDRV_AI_ENG_JOBM_JLB:
		case KDRV_AI_ENG_JOBM_JLC:
		case KDRV_AI_ENG_JOBM_JLD:
		case KDRV_AI_ENG_JOBM_JLE:{
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			clk_info->clk_rate = jobm_eng_get_clk_rate(p_handle);
		}
			break;
		case KDRV_AI_ENG_JMISP_PLA: 
		case KDRV_AI_ENG_JMISP_PLB:
		case KDRV_AI_ENG_JMISP_PLC:
		case KDRV_AI_ENG_JMISP_PLD:{
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			clk_info->clk_rate = jmisp_eng_get_clk_rate(p_handle);
		}
			break;
		default:
			DBG_ERR("unknown engine = %u\r\n",clk_info->eng);
			ret = -1;
			break;
	}
	return ret;
}

static void kdrv_ai_rou_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle;
	ROU_ENG_HANDLE *p_eng_handle = rou_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if(ll_idx==1){
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_ROU_LL1);
	}else{
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_ROU);
	}
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) { 
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = rou_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = rou_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = rou_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = rou_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = rou_eng_get_wait_cycle(p_eng_handle);
				#endif
				if(ll_idx==1)
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_ROU2, NULL);
				else
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_ROU, NULL);
			}
		}
	}
}

static void kdrv_ai_nue2_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_NUE2);
	NUE2_ENG_HANDLE *p_eng_handle = nue2_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = nue2_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = nue2_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = nue2_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = nue2_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = nue2_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_NUE2, NULL);
			}
		}
	}
}

static void kdrv_ai_conv1_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle;
	CONV_ENG_HANDLE *p_eng_handle = conv_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if(ll_idx==1){
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CONV1_LL1);
	}else{
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CONV1);
	}
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) { 
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = conv_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = conv_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = conv_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = conv_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = conv_eng_get_wait_cycle(p_eng_handle);
				#endif
				if(ll_idx==1)
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CONV2, NULL);
				else
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CONV1, NULL);
			}
		}
	}
}
/*
static void kdrv_ai_conv2_isr_cb(void* handle, UINT32 intstatus, void* data)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CONV2);
	CONV_ENG_HANDLE *p_eng_handle = conv_eng_get_handle(0, 1);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) { 
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = conv_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = conv_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = conv_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = conv_eng_get_ll_cycle(p_eng_handle);
				exec_cycle_dma = conv_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CONV2, NULL);
			}
		}
	}
}

static void kdrv_ai_conv3_isr_cb(void* handle, UINT32 intstatus, void* data)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CONV3);
	CONV_ENG_HANDLE *p_eng_handle = conv_eng_get_handle(0, 2);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = conv_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = conv_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = conv_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = conv_eng_get_ll_cycle(p_eng_handle);
				exec_cycle_dma = conv_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CONV3, NULL);
			}
		}
	}
}
static void kdrv_ai_conv4_isr_cb(void* handle, UINT32 intstatus, void* data)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CONV4);
	CONV_ENG_HANDLE *p_eng_handle = conv_eng_get_handle(0, 3);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = conv_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = conv_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = conv_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = conv_eng_get_ll_cycle(p_eng_handle);
				exec_cycle_dma = conv_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CONV4, NULL);
			}
		}
	}
}
*/

static void kdrv_ai_jobm_isr_cb(void* handle, UINT32 intstatus, void* data)
{
	KDRV_AI_HANDLE *p_handle = NULL; 
	JOBM_ENG_HANDLE *p_eng_handle = jobm_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (intstatus & JOBM_ENG_INTERRUPT_JLA_JOB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLA_DBG);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 0);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 0);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 0);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 0);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 0);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLA_DBG, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLB_JOB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLB_DBG);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 1);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 1);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 1);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 1);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 1);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLB_DBG, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLC_JOB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLC_DBG);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 2);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 2);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 2);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 2);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 2);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLC_DBG, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLD_JOB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLD_DBG);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 3);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 3);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 3);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 3);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 3);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLD_DBG, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLE_JOB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLE_DBG);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 4);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 4);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 4);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 4);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 4);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLE_DBG, NULL);
			}
		}
	}
	
	if (intstatus & JOBM_ENG_INTERRUPT_JLA_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLA);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 0);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 0);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 0);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 0);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 0);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLA, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLB_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLB);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 1);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 1);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 1);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 1);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 1);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLB, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLC_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLC);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 2);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 2);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 2);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 2);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 2);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLC, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLD_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLD);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 3);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 3);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 3);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 3);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 3);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLD, NULL);
			}
		}
	}
	if (intstatus & JOBM_ENG_INTERRUPT_JLE_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JOBM_JLE);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jobm_eng_get_joblist_cycle(p_eng_handle, 4);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = jobm_eng_get_ub_bw(p_eng_handle, 4);
				exec_cycle_dma = jobm_eng_get_dram_bw(p_eng_handle, 4);
				#else
				exec_cycle_LL  = jobm_eng_get_joblist_cycle(p_eng_handle, 4);
				exec_cycle_dma = jobm_eng_get_wait_dma_cycle(p_eng_handle, 4);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JOBM_JLE, NULL);
			}
		}
	}
	
}

static void kdrv_ai_lsu_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_LSU);
	LSU_ENG_HANDLE *p_eng_handle = lsu_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = lsu_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = lsu_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = lsu_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = lsu_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = lsu_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_LSU, NULL);
			}
		}
	}
}

static void kdrv_ai_util_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle;
	UTIL_ENG_HANDLE *p_eng_handle = util_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if(ll_idx==1){
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_UTIL_LL1);
	}else{
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_UTIL);
	}
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = util_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = util_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = util_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = util_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = util_eng_get_wait_cycle(p_eng_handle);
				#endif
				if(ll_idx==1)
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_UTIL2, NULL);
				else
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_UTIL, NULL);
			}
		}
	}
}

static void kdrv_ai_cal_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle;
	CAL_ENG_HANDLE *p_eng_handle = cal_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if(ll_idx==1){
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CAL_LL1);
	}else{
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_CAL);
	}
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = cal_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = cal_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = cal_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = cal_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = cal_eng_get_wait_cycle(p_eng_handle);
				#endif
				if(ll_idx==1)
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CAL2, NULL);
				else
					p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_CAL, NULL);
			}
		}
	}
}

static void kdrv_ai_ppu_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_PPU);
	PPU_ENG_HANDLE *p_eng_handle = ppu_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = ppu_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = ppu_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = ppu_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = ppu_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = ppu_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_PPU, NULL);
			}
		}
	}
}

static void kdrv_ai_pou_isr_cb(void* handle, UINT32 intstatus, void* data, UINT32 ll_idx)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_POU);
	POU_ENG_HANDLE *p_eng_handle = pou_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (p_handle == NULL || p_eng_handle == NULL) {
		DBG_ERR("kdrv_ai_get_handle fail\r\n");
	} else {
		if (intstatus & 0x1) {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = pou_eng_get_eng_cycle(p_eng_handle);
				#ifdef KDRV_AI_VP_ENV
				exec_cycle_LL  = pou_eng_get_ub_bw(p_eng_handle);
				exec_cycle_dma = pou_eng_get_dram_bw(p_eng_handle);
				#else
				exec_cycle_LL  = pou_eng_get_ll_cycle(p_eng_handle,ll_idx);
				exec_cycle_dma = pou_eng_get_wait_cycle(p_eng_handle);
				#endif
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_POU, NULL);
			}
		}
	}
}

static void kdrv_ai_jmisp_isr_cb(void* handle, UINT32 intstatus, void* data)
{
	KDRV_AI_HANDLE *p_handle = NULL; 
	JMISP_ENG_HANDLE *p_eng_handle = jmisp_eng_get_handle(0, 0);
	UINT32 exec_cycle 		= 0;
	UINT32 exec_cycle_LL 	= 0;
	UINT32 exec_cycle_dma 	= 0;
	
	if (intstatus & JMISP_ENG_INTERRUPT_PL0_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JMISP_PLA);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jmisp_eng_get_joblist_cycle(p_eng_handle, 0);
				exec_cycle_LL  = jmisp_eng_get_joblist_cycle(p_eng_handle, 0);
				exec_cycle_dma = jmisp_eng_get_wait_dma_cycle(p_eng_handle, 0);
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JMISP_PLA, NULL);
			}
		}
	}
	if (intstatus & JMISP_ENG_INTERRUPT_PL1_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JMISP_PLB);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jmisp_eng_get_joblist_cycle(p_eng_handle, 1);
				exec_cycle_LL  = jmisp_eng_get_joblist_cycle(p_eng_handle, 1);
				exec_cycle_dma = jmisp_eng_get_wait_dma_cycle(p_eng_handle, 1);
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JMISP_PLB, NULL);
			}
		}
	}
	if (intstatus & JMISP_ENG_INTERRUPT_PL2_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JMISP_PLC);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jmisp_eng_get_joblist_cycle(p_eng_handle, 2);
				exec_cycle_LL  = jmisp_eng_get_joblist_cycle(p_eng_handle, 2);
				exec_cycle_dma = jmisp_eng_get_wait_dma_cycle(p_eng_handle, 2);
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JMISP_PLC, NULL);
			}
		}
	}
	if (intstatus & JMISP_ENG_INTERRUPT_PL3_END) {
		p_handle = kdrv_ai_get_handle(KDRV_AI_ENG_JMISP_PLD);
		if (p_handle == NULL) {
			DBG_ERR("kdrv_ai_get_handle fail\r\n");
		} else {
			if (p_handle->isrcb_fp != NULL) {
				exec_cycle     = jmisp_eng_get_joblist_cycle(p_eng_handle, 3);
				exec_cycle_LL  = jmisp_eng_get_joblist_cycle(p_eng_handle, 3);
				exec_cycle_dma = jmisp_eng_get_wait_dma_cycle(p_eng_handle, 3);
				p_handle->isrcb_fp(exec_cycle, exec_cycle_LL, exec_cycle_dma, intstatus, KDRV_AI_ENG_JMISP_PLD, NULL);
			}
		}
	}
	
}

INT32 kdrv_ai_engine_reset(KDRV_AI_ENG ai_eng)
{
	ER rt = E_OK;
	CONV_ENG_HANDLE  *p_conv_handle  = NULL;
	LSU_ENG_HANDLE   *p_lsu_handle   = NULL;
	UTIL_ENG_HANDLE  *p_util_handle  = NULL;
	ROU_ENG_HANDLE   *p_rou_handle   = NULL;
	CAL_ENG_HANDLE   *p_cal_handle   = NULL;
	NUE2_ENG_HANDLE  *p_nue2_handle  = NULL;
	PPU_ENG_HANDLE   *p_ppu_handle   = NULL;
	POU_ENG_HANDLE   *p_pou_handle   = NULL;
	JOBM_ENG_HANDLE  *p_jobm_handle  = NULL;
	JMISP_ENG_HANDLE *p_jmisp_handle = NULL;

	{// Disable autogating
	#if defined (__LINUX)
		clk_set_phase(ai_clk[0], 0);
		clk_set_phase(ai_clk[1], 0);
		clk_set_phase(ai_clk[2], 0);
		clk_set_phase(ai_clk[3], 0);
		clk_set_phase(ai_clk[6], 0);
		if (nvt_get_chip_id() == CHIP_NS02402) {
		clk_set_phase(ai_clk[7], 0);
		clk_set_phase(ai_clk[8], 0); //conv autogating1
		clk_set_phase(ai_clk[9], 0); //conv autogating2
		clk_set_phase(ai_clk[10], 0);//conv autogating3
		}
	#else
		pll_clear_clk_auto_gating(CONV_M_GCLK);
		pll_clear_clk_auto_gating(NUE2_M_GCLK);
		pll_clear_clk_auto_gating(PPU_M_GCLK);
		pll_clear_clk_auto_gating(LSU_M_GCLK);
		if (nvt_get_chip_id() == CHIP_NS02402) {
		pll_clear_clk_auto_gating(POU_M_GCLK);
		pll_clear_clk_auto_gating(CONV_2_M_GCLK);
		pll_clear_clk_auto_gating(CONV_3_M_GCLK);
		pll_clear_clk_auto_gating(CONV_4_M_GCLK);
		}		
		pll_clear_clk_auto_gating(NUE30_M_GCLK);
	#endif
	}
	
	if (ai_eng == KDRV_AI_ENG_CONV1 || ai_eng == KDRV_AI_ENG_CONV1_LL1) {
		p_conv_handle = conv_eng_get_handle(0, 0);
		rt = conv_eng_reset(p_conv_handle);
	} else if (ai_eng == KDRV_AI_ENG_LSU) {
		p_lsu_handle = lsu_eng_get_handle(0, 0);
		rt = lsu_eng_reset(p_lsu_handle);
	} else if (ai_eng == KDRV_AI_ENG_UTIL || ai_eng == KDRV_AI_ENG_UTIL_LL1) {
		if (nvt_get_chip_id() == CHIP_NS02302) {
		p_util_handle = util_eng_get_handle(0, 0);
		rt = util_eng_reset(p_util_handle);
		}
	} else if (ai_eng == KDRV_AI_ENG_ROU || ai_eng == KDRV_AI_ENG_ROU_LL1) {
		p_rou_handle = rou_eng_get_handle(0, 0);
		rt = rou_eng_reset(p_rou_handle);
	} else if (ai_eng == KDRV_AI_ENG_CAL || ai_eng == KDRV_AI_ENG_CAL_LL1) {
		p_cal_handle = cal_eng_get_handle(0, 0);
		rt = cal_eng_reset(p_cal_handle);
	} else if (ai_eng == KDRV_AI_ENG_NUE2) {
		p_nue2_handle = nue2_eng_get_handle(0, 0);
		rt = nue2_eng_reset(p_nue2_handle);
	} else if (ai_eng == KDRV_AI_ENG_PPU) {
		p_ppu_handle = ppu_eng_get_handle(0, 0);
		rt = ppu_eng_reset(p_ppu_handle);
	} else if (ai_eng == KDRV_AI_ENG_POU) {
		p_pou_handle = pou_eng_get_handle(0, 0);
		rt = pou_eng_reset(p_pou_handle);
	} else if (ai_eng == KDRV_AI_ENG_JOBM_JLA || ai_eng == KDRV_AI_ENG_JOBM_JLB ||
			   ai_eng == KDRV_AI_ENG_JOBM_JLC || ai_eng == KDRV_AI_ENG_JOBM_JLD ||
			   ai_eng == KDRV_AI_ENG_JOBM_JLE || ai_eng == KDRV_AI_ENG_JMISP_PLA ||
			   ai_eng == KDRV_AI_ENG_JMISP_PLB || ai_eng == KDRV_AI_ENG_JMISP_PLC ||
			   ai_eng == KDRV_AI_ENG_JMISP_PLD) {
		
		if (ai_eng == KDRV_AI_ENG_JMISP_PLA || ai_eng == KDRV_AI_ENG_JMISP_PLB || 
			ai_eng == KDRV_AI_ENG_JMISP_PLC || ai_eng == KDRV_AI_ENG_JMISP_PLD) {
			p_jmisp_handle = jmisp_eng_get_handle(0, 0);
			rt = jmisp_eng_reset(p_jmisp_handle);
			if (rt) {
				DBG_ERR("reset jmisp fail\r\n");
				return rt;
			}
			
			p_ppu_handle = ppu_eng_get_handle(0, 0);
			rt = ppu_eng_reset(p_ppu_handle);
			if (rt) {
				DBG_ERR("reset ppu fail\r\n");
				return rt;
			}
		}
	
		p_jobm_handle = jobm_eng_get_handle(0, 0);
		rt = jobm_eng_reset(p_jobm_handle);
		if (rt) {
			DBG_ERR("reset jobm fail\r\n");
			return rt;
		}

		p_conv_handle = conv_eng_get_handle(0, 0);
		rt = conv_eng_reset(p_conv_handle);
		if (rt) {
			DBG_ERR("reset conv1 fail\r\n");
			return rt;
		}
		/*
		p_conv_handle = conv_eng_get_handle(0, KDRV_AI_ENG_CONV2);
		rt = conv_eng_reset(p_conv_handle);
		if (rt) {
			DBG_ERR("reset conv2 fail\r\n");
			return rt;
		}
		
		p_conv_handle = conv_eng_get_handle(0, KDRV_AI_ENG_CONV3);
		rt = conv_eng_reset(p_conv_handle);
		if (rt) {
			DBG_ERR("reset conv3 fail\r\n");
			return rt;
		}
		
		p_conv_handle = conv_eng_get_handle(0, KDRV_AI_ENG_CONV4);
		rt = conv_eng_reset(p_conv_handle);
		if (rt) {
			DBG_ERR("reset conv4 fail\r\n");
			return rt;
		}
		*/
		p_lsu_handle = lsu_eng_get_handle(0, 0);
		rt = lsu_eng_reset(p_lsu_handle);
		if (rt) {
			DBG_ERR("reset lsu fail\r\n");
			return rt;
		}
		
		if (nvt_get_chip_id() == CHIP_NS02302) {
		p_util_handle = util_eng_get_handle(0, 0);
		rt = util_eng_reset(p_util_handle);
		if (rt) {
			DBG_ERR("reset util fail\r\n");
			return rt;
		}
		}
		
		p_cal_handle = cal_eng_get_handle(0, 0);
		rt = cal_eng_reset(p_cal_handle);
		if (rt) {
			DBG_ERR("reset cal fail\r\n");
			return rt;
		}
		
		p_rou_handle = rou_eng_get_handle(0, 0);
		rt = rou_eng_reset(p_rou_handle);
		if (rt) {
			DBG_ERR("reset rou fail\r\n");
			return rt;
		}
	}
	
	{// Enable autogating
	#if defined (__LINUX)
		clk_set_phase(ai_clk[0], 1);
		clk_set_phase(ai_clk[1], 1);
		clk_set_phase(ai_clk[2], 1);
		clk_set_phase(ai_clk[3], 1);
		clk_set_phase(ai_clk[6], 1);
		if (nvt_get_chip_id() == CHIP_NS02402) {
		clk_set_phase(ai_clk[7], 1);
		clk_set_phase(ai_clk[8], 1); //conv autogating1
		clk_set_phase(ai_clk[9], 1); //conv autogating2
		clk_set_phase(ai_clk[10], 1);//conv autogating3
		}
	#else
		pll_set_clk_auto_gating(CONV_M_GCLK);
		pll_set_clk_auto_gating(NUE2_M_GCLK);
		pll_set_clk_auto_gating(PPU_M_GCLK);
		pll_set_clk_auto_gating(LSU_M_GCLK);
		if (nvt_get_chip_id() == CHIP_NS02402) {
		pll_set_clk_auto_gating(POU_M_GCLK);
		pll_set_clk_auto_gating(CONV_2_M_GCLK);
		pll_set_clk_auto_gating(CONV_3_M_GCLK);
		pll_set_clk_auto_gating(CONV_4_M_GCLK);
		}		
		pll_set_clk_auto_gating(NUE30_M_GCLK);
	#endif
	}
	
	return rt;
}

static ER kdrv_ai_set_isr_cb(KDRV_AI_ENG eng, void *data)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(eng);

	if (p_handle == NULL) {
		DBG_ERR("unknown eng : %d\r\n", (int)eng);
		return E_ID;
	}
	if (data == NULL) {
		DBG_ERR("eng = %d, data param is NULL!\r\n", (int)eng);
		return E_PAR;
	}

	p_handle->isrcb_fp = (KDRV_AI_ISRCB)data;
	
	return E_OK;
}

static ER kdrv_ai_get_isr_cb(KDRV_AI_ENG eng, void *data)
{
	KDRV_AI_HANDLE *p_handle = kdrv_ai_get_handle(eng);

	if (p_handle == NULL) {
		DBG_ERR("unknown eng : %d\r\n", (int)eng);
		return E_ID;
	}

	*(KDRV_AI_ISRCB *)data = p_handle->isrcb_fp;
	
	return E_OK;
}

INT32 kdrv_ai_set(KDRV_AI_PARAM_ID param_id, VOID *p_param)
{
	ER rt = E_OK;
	UINT32 i = 0;
	
	if (p_param == NULL) {
		return -1;
	}

	switch (param_id) {
		case KDRV_AI_PARAM_JOBM_JLA_PARAM:
		case KDRV_AI_PARAM_JOBM_JLB_PARAM:
		case KDRV_AI_PARAM_JOBM_JLC_PARAM:
		case KDRV_AI_PARAM_JOBM_JLD_PARAM:
		case KDRV_AI_PARAM_JOBM_JLE_PARAM: {
			KDRV_AI_JOBM_PARAM* jm_param = (KDRV_AI_JOBM_PARAM*)p_param;
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			
			jobm_eng_set_priority(p_handle, param_id-KDRV_AI_PARAM_JOBM_JLA_PARAM, 
								jm_param->jl_no_dispatch_cnt, jm_param->jl_priority);
			jobm_eng_set_net_id(p_handle, param_id-KDRV_AI_PARAM_JOBM_JLA_PARAM, jm_param->net_id);
		}
			break;
		case KDRV_AI_PARAM_CBFUNC: {
			KDRV_AI_CBFUNC_PARAM* p_cb_parm = (KDRV_AI_CBFUNC_PARAM*)p_param;
			kdrv_ai_set_isr_cb(p_cb_parm->eng, p_cb_parm->isrcb_fp);
		}
			break;
		case KDRV_AI_PARAM_BASE_ADDR: {
			KDRV_AI_BASE_ADDR_PARAM* p_base_addr_parm = (KDRV_AI_BASE_ADDR_PARAM*)p_param;
			
			if (p_base_addr_parm->eng == KDRV_AI_ENG_ROU) {
				ROU_ENG_HANDLE *p_handle = rou_eng_get_handle(0, 0);
				rou_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_ROU_LL1) {
				ROU_ENG_HANDLE *p_handle = rou_eng_get_handle(0, 0);
				rou_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,1);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_NUE2) {
				NUE2_ENG_HANDLE *p_handle = nue2_eng_get_handle(0, 0);
				nue2_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_CONV1) {
				CONV_ENG_HANDLE *p_handle = conv_eng_get_handle(0, 0);
				conv_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_CONV1_LL1) {
				CONV_ENG_HANDLE *p_handle = conv_eng_get_handle(0, 0);
				conv_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,1);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_JOBM_JLA || p_base_addr_parm->eng == KDRV_AI_ENG_JOBM_JLB || 
					   p_base_addr_parm->eng == KDRV_AI_ENG_JOBM_JLC || p_base_addr_parm->eng == KDRV_AI_ENG_JOBM_JLD || 
					   p_base_addr_parm->eng == KDRV_AI_ENG_JOBM_JLE) {
				JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
				jobm_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF, p_base_addr_parm->pa & 0xFFFFFFFF, 
											p_base_addr_parm->base_id, p_base_addr_parm->eng - KDRV_AI_ENG_JOBM_JLA);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_LSU) {
				LSU_ENG_HANDLE *p_handle = lsu_eng_get_handle(0, 0);
				lsu_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_UTIL) {
				if (nvt_get_chip_id() == CHIP_NS02302) {
				UTIL_ENG_HANDLE *p_handle = util_eng_get_handle(0, 0);
				util_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
				}
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_UTIL_LL1) {
				if (nvt_get_chip_id() == CHIP_NS02302) {
				UTIL_ENG_HANDLE *p_handle = util_eng_get_handle(0, 0);
				util_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,1);
				}
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_CAL) {
				CAL_ENG_HANDLE *p_handle = cal_eng_get_handle(0, 0);
				cal_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_CAL_LL1) {
				CAL_ENG_HANDLE *p_handle = cal_eng_get_handle(0, 0);
				cal_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,1);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_PPU) {
				PPU_ENG_HANDLE *p_handle = ppu_eng_get_handle(0, 0);
				ppu_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_POU) {
				POU_ENG_HANDLE *p_handle = pou_eng_get_handle(0, 0);
				pou_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF,
											p_base_addr_parm->pa & 0xFFFFFFFF, p_base_addr_parm->base_id,0);
			} else if (p_base_addr_parm->eng == KDRV_AI_ENG_JMISP_PLA || p_base_addr_parm->eng == KDRV_AI_ENG_JMISP_PLB || 
					   p_base_addr_parm->eng == KDRV_AI_ENG_JMISP_PLC || p_base_addr_parm->eng == KDRV_AI_ENG_JMISP_PLD) {
				JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
				jmisp_eng_set_dma_base_addr(p_handle, (p_base_addr_parm->pa >> 32) & 0xF, p_base_addr_parm->pa & 0xFFFFFFFF, 
											p_base_addr_parm->base_id, p_base_addr_parm->eng - KDRV_AI_ENG_JMISP_PLA);
			}
		}
			break;
		case KDRV_AI_PARAM_JOBM_DBG: {
			KDRV_AI_JOBM_DBG_PARAM* dbg_param = (KDRV_AI_JOBM_DBG_PARAM*)p_param;
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			UINT32 interrupt_en = JOBM_ENG_INTERRUPT_JLA_JOB_END;
			
			// initial as close
			for (i = 0; i < 5; i++) {
				jobm_eng_set_single_dbg_en(p_handle, 0, i);
			}
			if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_ARB) {
				jobm_eng_set_arb_dbg_en(p_handle, 0, 0);
			} else if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_ARB2) {
				jobm_eng_set_arb_dbg_en(p_handle, 0, 1);
			} else {
				jobm_eng_set_arb_dbg_en(p_handle, 0, 0);
				jobm_eng_set_arb_dbg_en(p_handle, 0, 1);
			}
			interrupt_en |= JOBM_ENG_INTERRUPT_JLB_JOB_END;
			interrupt_en |= JOBM_ENG_INTERRUPT_JLC_JOB_END;
			interrupt_en |= JOBM_ENG_INTERRUPT_JLD_JOB_END;
			interrupt_en |= JOBM_ENG_INTERRUPT_JLE_JOB_END;
			jobm_eng_set_intrpt_dis(p_handle, interrupt_en);
			
			if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_NONE) {
				// do nothing
			} else if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_SINGLE) {
				jobm_eng_set_single_dbg_en(p_handle, 1, dbg_param->single_jl_id);
			} else if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_ARB) {
				jobm_eng_set_arb_dbg_en(p_handle, dbg_param->arb_eng, 0);
				jobm_eng_set_intrpt_en(p_handle, interrupt_en);
			}  else if (dbg_param->dbg_mode == KDRV_AI_JM_DBG_MODE_ARB2) {
				jobm_eng_set_arb_dbg_en(p_handle, dbg_param->arb_eng, 1);
				jobm_eng_set_intrpt_en(p_handle, interrupt_en);
			} 
		}
			break;
		case KDRV_AI_PARAM_JMISP_DBG: {
			KDRV_AI_JMISP_DBG_PARAM* dbg_param = (KDRV_AI_JMISP_DBG_PARAM*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			// initial as close
			for (i = 0; i < 4; i++) {
				jmisp_eng_set_single_dbg_en(p_handle, 0, i);
			}
			jmisp_eng_set_arb_dbg_en(p_handle, 0);
			
			if (dbg_param->dbg_mode == KDRV_AI_JMISP_DBG_MODE_NONE) {
				// do nothing
			} else if (dbg_param->dbg_mode == KDRV_AI_JMISP_DBG_MODE_SINGLE) {
				jmisp_eng_set_single_dbg_en(p_handle, 1, dbg_param->single_pl_id);
			} else if (dbg_param->dbg_mode == KDRV_AI_JMISP_DBG_MODE_ARB) {
				jmisp_eng_set_arb_dbg_en(p_handle, dbg_param->arb_eng);
				if(dbg_param->arb_eng == KDRV_AI_JMISP_DBG_ENG_PPU) {
                    PPU_ENG_HANDLE *p_ppu_handle = ppu_eng_get_handle(0, 0);
					ppu_eng_set_intrpt_en(p_ppu_handle, PPU_ENG_INTERRUPT_LL_END);
                } else {
                    POU_ENG_HANDLE *p_pou_handle = pou_eng_get_handle(0, 0);
                    pou_eng_set_intrpt_en(p_pou_handle, POU_ENG_INTERRUPT_LL_END);
                }
			}
		}
			break;
		case KDRV_AI_PARAM_JMISP_PARAM: {
			KDRV_AI_JMISP_PATH_INFO* jmisp_param = (KDRV_AI_JMISP_PATH_INFO*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_set_path_info(p_handle, (JMISP_ENG_PATH_INFO*)jmisp_param);
		}
			break;
		case KDRV_AI_PARAM_JMISP_DBG_RUN_SLICE: {
			UINT32* jmisp_param = (UINT32*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_run_cpu_slice(p_handle, jmisp_param[0]);
		}
			break;
		case KDRV_AI_PARAM_JMISP_ADDR_PARAM: {
			KDRV_AI_JMISP_FRAME_ADDR_INFO* jmisp_param = (KDRV_AI_JMISP_FRAME_ADDR_INFO*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_set_frame_addr(p_handle, (JMISP_ENG_FRAME_ADDR_INFO*)jmisp_param);
		}
			break;
		case KDRV_AI_PARAM_JOBM_BUSY_CNT: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			
			rt = jobm_eng_set_cnt_clr(p_handle, KDRV_AI_JM_DBG_ENG_ALL);
		}
			break;
		case KDRV_AI_PARAM_JMISP_INT: {
			KDRV_AI_JMISP_INT_INFO* jmisp_int_info = (KDRV_AI_JMISP_INT_INFO*)p_param;
			
			g_jmisp_int_en = 0;
			for (i = 0; i < KDRV_AI_JMISP_MAX_PATH_NUM; i++) {
				g_jmisp_int_en |= ((1-jmisp_int_info->pl_disable[i]) << i);
			}
		}
			break;
		case KDRV_AI_PARAM_LL_SETTING: {
			KDRV_AI_LL_SETTING_PARAM* p_ll_setting_param = (KDRV_AI_LL_SETTING_PARAM*)p_param;
			
			if(p_ll_setting_param->eng == KDRV_AI_ENG_CONV1 || p_ll_setting_param->eng == KDRV_AI_ENG_CONV1_LL1){
				CONV_ENG_HANDLE *p_handle = conv_eng_get_handle(0, 0);
				CONV_LL_SETTING ll_param;
				ll_param.unlock_cycle = p_ll_setting_param->unlock_cycle;
				ll_param.priority_mode = p_ll_setting_param->priority_mode;
				ll_param.qos_tot_time = p_ll_setting_param->qos_tot_time;
				ll_param.qos_ocpy_time = p_ll_setting_param->qos_ocpy_time;
				ll_param.sta_period_time = p_ll_setting_param->sta_period_time;
				conv_eng_set_ll_setting(p_handle,&ll_param);
			}else if(p_ll_setting_param->eng == KDRV_AI_ENG_UTIL || p_ll_setting_param->eng == KDRV_AI_ENG_UTIL_LL1){
				if (nvt_get_chip_id() == CHIP_NS02302) {
				UTIL_ENG_HANDLE *p_handle = util_eng_get_handle(0, 0);
				UTIL_LL_SETTING ll_param;
				ll_param.unlock_cycle = p_ll_setting_param->unlock_cycle;
				ll_param.priority_mode = p_ll_setting_param->priority_mode;
				ll_param.qos_tot_time = p_ll_setting_param->qos_tot_time;
				ll_param.qos_ocpy_time = p_ll_setting_param->qos_ocpy_time;
				ll_param.sta_period_time = p_ll_setting_param->sta_period_time;
				util_eng_set_ll_setting(p_handle,&ll_param);
				}
			}else if(p_ll_setting_param->eng == KDRV_AI_ENG_ROU || p_ll_setting_param->eng == KDRV_AI_ENG_ROU_LL1){
				ROU_ENG_HANDLE *p_handle = rou_eng_get_handle(0, 0);
				ROU_LL_SETTING ll_param;
				ll_param.unlock_cycle = p_ll_setting_param->unlock_cycle;
				ll_param.priority_mode = p_ll_setting_param->priority_mode;
				ll_param.qos_tot_time = p_ll_setting_param->qos_tot_time;
				ll_param.qos_ocpy_time = p_ll_setting_param->qos_ocpy_time;
				ll_param.sta_period_time = p_ll_setting_param->sta_period_time;
				rou_eng_set_ll_setting(p_handle,&ll_param);
			}else if(p_ll_setting_param->eng == KDRV_AI_ENG_CAL || p_ll_setting_param->eng == KDRV_AI_ENG_CAL_LL1){
				CAL_ENG_HANDLE *p_handle = cal_eng_get_handle(0, 0);
				CAL_LL_SETTING ll_param;
				ll_param.unlock_cycle = p_ll_setting_param->unlock_cycle;
				ll_param.priority_mode = p_ll_setting_param->priority_mode;
				ll_param.qos_tot_time = p_ll_setting_param->qos_tot_time;
				ll_param.qos_ocpy_time = p_ll_setting_param->qos_ocpy_time;
				ll_param.sta_period_time = p_ll_setting_param->sta_period_time;
				cal_eng_set_ll_setting(p_handle,&ll_param);
			}
		}
			break;	
		case KDRV_AI_PARAM_JOBM_ENG_BUSY_CNT: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JM_DBG_ENG *p_eng = (KDRV_AI_JM_DBG_ENG*)p_param;
			rt = jobm_eng_set_cnt_clr(p_handle, p_eng[0]);
		}
			break;
		case KDRV_AI_PARAM_JMISP_FRM_UPD_PARAM: {
			KDRV_AI_JMISP_FRM_UPD_PATH_INFO* jmisp_param = (KDRV_AI_JMISP_FRM_UPD_PATH_INFO*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_set_frm_upd_path_info(p_handle, (JMISP_ENG_FRM_UPD_PATH_INFO*)jmisp_param);
		}
			break;
		case KDRV_AI_PARAM_GATING: {
			KDRV_AI_GATING_INFO* gating_param = (KDRV_AI_GATING_INFO*)p_param;
			UINT32 flag,engine_index;
			
			flag = gating_param->flag;
			engine_index = gating_param->engine_index;
			#if defined (__LINUX)
				if(ai_get_clk[engine_index]){
					kdrv_ai_clk_gating[engine_index] = flag;

					if (kdrv_ai_clk_gating[engine_index] == 1) {
						clk_set_phase(ai_clk[engine_index], 1);
					} else {
						clk_set_phase(ai_clk[engine_index], 0);
					}
				}else{
					DBG_ERR("[kdrv_ai_set_gating] Did not get clock source.\n");
				}
			#else
				kdrv_ai_clk_gating[engine_index] = flag;
				if (kdrv_ai_clk_gating[engine_index] == 1) {
					switch(engine_index){
						case 0:
							pll_set_clk_auto_gating(CONV_M_GCLK);
							break;
						case 1:
							pll_set_clk_auto_gating(NUE2_M_GCLK);
							break;
						case 2:
							pll_set_clk_auto_gating(PPU_M_GCLK);
							break;
						case 3:
							pll_set_clk_auto_gating(LSU_M_GCLK);
							break;
						case 6:
							if (nvt_get_chip_id() == CHIP_NS02302) {
								pll_set_clk_auto_gating(NUE30_M_GCLK);
							} else if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_set_clk_auto_gating(POU_M_GCLK);
							}							
							break;
						case 7:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_set_clk_auto_gating(NUE30_M_GCLK);
							}
							break;
						case 8:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_set_clk_auto_gating(CONV_2_M_GCLK);
							}
							break;
						case 9:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_set_clk_auto_gating(CONV_3_M_GCLK);
							}
							break;
						case 10:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_set_clk_auto_gating(CONV_4_M_GCLK);
							}
							break;
						default:
							break;						
					}
				} else {
					switch(engine_index){
						case 0:
							pll_clear_clk_auto_gating(CONV_M_GCLK);
							break;
						case 1:
							pll_clear_clk_auto_gating(NUE2_M_GCLK);
							break;
						case 2:
							pll_clear_clk_auto_gating(PPU_M_GCLK);
							break;
						case 3:
							pll_clear_clk_auto_gating(LSU_M_GCLK);
							break;
						case 6:
							if (nvt_get_chip_id() == CHIP_NS02302) {
								pll_clear_clk_auto_gating(NUE30_M_GCLK);
							} else if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_clear_clk_auto_gating(POU_M_GCLK);
							}
							break;
						case 7:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_clear_clk_auto_gating(NUE30_M_GCLK);
							}
							break;
						case 8:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_clear_clk_auto_gating(CONV_2_M_GCLK);
							}
							break;
						case 9:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_clear_clk_auto_gating(CONV_3_M_GCLK);
							}
							break;
						case 10:
							if (nvt_get_chip_id() == CHIP_NS02402) {
								pll_clear_clk_auto_gating(CONV_4_M_GCLK);
							}
							break;
						default:
							break;						
					}
				}
			#endif
		}
			break;
		default:
			DBG_ERR("unknown param_id : %d\r\n", (int)param_id);
			break;
	}
	
	return rt;
}

INT32 kdrv_ai_get(KDRV_AI_PARAM_ID param_id, VOID *p_param)
{
	ER rt = E_OK;
	UINT32 i = 0;
	
	if (p_param == NULL) {
		DBG_ERR("invalid p_param\r\n");
		return -1;
	}

	switch (param_id) {
		case KDRV_AI_PARAM_JOBM_JLA_PARAM:
		case KDRV_AI_PARAM_JOBM_JLB_PARAM:
		case KDRV_AI_PARAM_JOBM_JLC_PARAM:
		case KDRV_AI_PARAM_JOBM_JLD_PARAM:
		case KDRV_AI_PARAM_JOBM_JLE_PARAM:
		case KDRV_AI_PARAM_JOBM_DBG:
			break;
		case KDRV_AI_PARAM_CBFUNC:{
			KDRV_AI_CBFUNC_PARAM* p_cb_parm = (KDRV_AI_CBFUNC_PARAM*)p_param;
			kdrv_ai_get_isr_cb(p_cb_parm->eng, p_cb_parm->isrcb_fp);
		}
			break;
		case KDRV_AI_PARAM_BASE_ADDR:
			//TODO: get engine base addr
			break;
		case KDRV_AI_PARAM_JMISP_PARAM: {
			KDRV_AI_JMISP_PATH_INFO* jmisp_param = (KDRV_AI_JMISP_PATH_INFO*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_get_path_info(p_handle, (JMISP_ENG_PATH_INFO*)jmisp_param);
		}
			break;
		case KDRV_AI_PARAM_JOBM_BUSY_CNT: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			CONV_ENG_HANDLE* p_conv_handle = conv_eng_get_handle(0, 0);
			LSU_ENG_HANDLE  *p_lsu_handle   = lsu_eng_get_handle(0, 0);
			UTIL_ENG_HANDLE* p_util_handle  = NULL;
			CAL_ENG_HANDLE* p_cal_handle = cal_eng_get_handle(0, 0);
			ROU_ENG_HANDLE* p_rou_handle = rou_eng_get_handle(0, 0);
			KDRV_AI_JOBM_BUSY_CNT_INFO* cnt_param = (KDRV_AI_JOBM_BUSY_CNT_INFO*)p_param;
			
			if(conv_eng_get_qos_setting(p_conv_handle) == 0){
				cnt_param->busy_cnt[KDRV_AI_ENG_CONV1] = (UINT32)((UINT64)jobm_eng_get_conv0_busy_cnt(p_handle, 0)*conv_eng_get_clk_rate(p_conv_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_CONV1_LL1] = (UINT32)((UINT64)jobm_eng_get_conv0_busy_cnt(p_handle, 1)*conv_eng_get_clk_rate(p_conv_handle)/jobm_eng_get_clk_rate(p_handle));
			}else{
				cnt_param->busy_cnt[KDRV_AI_ENG_CONV1] = (UINT32)((UINT64)conv_eng_get_ll_sta_acc_cycle(p_conv_handle, 0)*conv_eng_get_clk_rate(p_conv_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_CONV1_LL1] = (UINT32)((UINT64)conv_eng_get_ll_sta_acc_cycle(p_conv_handle, 1)*conv_eng_get_clk_rate(p_conv_handle)/jobm_eng_get_clk_rate(p_handle));
			}
			cnt_param->busy_cnt[KDRV_AI_ENG_LSU]   = (UINT32)((UINT64)jobm_eng_get_lsu_busy_cnt(p_handle)*lsu_eng_get_clk_rate(p_lsu_handle)/jobm_eng_get_clk_rate(p_handle));
			if (nvt_get_chip_id() == CHIP_NS02302) {
				p_util_handle = util_eng_get_handle(0, 0);
				if(util_eng_get_qos_setting(p_util_handle) == 0){
					cnt_param->busy_cnt[KDRV_AI_ENG_UTIL]  = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*util_eng_get_clk_rate(p_util_handle)/jobm_eng_get_clk_rate(p_handle));
					cnt_param->busy_cnt[KDRV_AI_ENG_UTIL_LL1]  = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*util_eng_get_clk_rate(p_util_handle)/jobm_eng_get_clk_rate(p_handle));
				}else{
					cnt_param->busy_cnt[KDRV_AI_ENG_UTIL] = (UINT32)((UINT64)util_eng_get_ll_sta_acc_cycle(p_util_handle, 0)*util_eng_get_clk_rate(p_util_handle)/jobm_eng_get_clk_rate(p_handle));
					cnt_param->busy_cnt[KDRV_AI_ENG_UTIL_LL1] = (UINT32)((UINT64)util_eng_get_ll_sta_acc_cycle(p_util_handle, 1)*util_eng_get_clk_rate(p_util_handle)/jobm_eng_get_clk_rate(p_handle));
				}
			}
			if(rou_eng_get_qos_setting(p_rou_handle) == 0){
				cnt_param->busy_cnt[KDRV_AI_ENG_ROU]   = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*rou_eng_get_clk_rate(p_rou_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_ROU_LL1] = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*rou_eng_get_clk_rate(p_rou_handle)/jobm_eng_get_clk_rate(p_handle));
			}else{
				cnt_param->busy_cnt[KDRV_AI_ENG_ROU] = (UINT32)((UINT64)rou_eng_get_ll_sta_acc_cycle(p_rou_handle, 0)*rou_eng_get_clk_rate(p_rou_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_ROU_LL1] = (UINT32)((UINT64)rou_eng_get_ll_sta_acc_cycle(p_rou_handle, 1)*rou_eng_get_clk_rate(p_rou_handle)/jobm_eng_get_clk_rate(p_handle));
			}
			if(cal_eng_get_qos_setting(p_cal_handle) == 0){
				cnt_param->busy_cnt[KDRV_AI_ENG_CAL]   = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*cal_eng_get_clk_rate(p_cal_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_CAL_LL1] = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*cal_eng_get_clk_rate(p_cal_handle)/jobm_eng_get_clk_rate(p_handle));
			}else{
				cnt_param->busy_cnt[KDRV_AI_ENG_CAL] = (UINT32)((UINT64)cal_eng_get_ll_sta_acc_cycle(p_cal_handle, 0)*cal_eng_get_clk_rate(p_cal_handle)/jobm_eng_get_clk_rate(p_handle));
				cnt_param->busy_cnt[KDRV_AI_ENG_CAL_LL1] = (UINT32)((UINT64)cal_eng_get_ll_sta_acc_cycle(p_cal_handle, 1)*cal_eng_get_clk_rate(p_cal_handle)/jobm_eng_get_clk_rate(p_handle));
			}			
		}
			break;
		case KDRV_AI_PARAM_JMISP_INT: {
			KDRV_AI_JMISP_INT_INFO* jmisp_int_info = (KDRV_AI_JMISP_INT_INFO*)p_param;
			
			for (i = 0; i < KDRV_AI_JMISP_MAX_PATH_NUM; i++) {
				jmisp_int_info->pl_disable[i] = 1 - ((g_jmisp_int_en >> i) & 0x1);
			}
		}
			break;
		case KDRV_AI_PARAM_JOBM_ENG_BUSY_CNT: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			CONV_ENG_HANDLE *p_conv_handle = conv_eng_get_handle(0, 0);
			LSU_ENG_HANDLE  *p_lsu_handle   = lsu_eng_get_handle(0, 0);
			UTIL_ENG_HANDLE *p_util_handle  = util_eng_get_handle(0, 0);
			ROU_ENG_HANDLE  *p_rou_handle   = rou_eng_get_handle(0, 0);
			CAL_ENG_HANDLE  *p_cal_handle   = cal_eng_get_handle(0, 0);
			KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO *p_info = (KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO*)p_param;
			if (p_info->eng == KDRV_AI_JM_DBG_ENG_CONV0) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_conv0_busy_cnt(p_handle, 0)*conv_eng_get_clk_rate(p_conv_handle)/jobm_eng_get_clk_rate(p_handle));
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_LSU) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_lsu_busy_cnt(p_handle)*lsu_eng_get_clk_rate(p_lsu_handle)/jobm_eng_get_clk_rate(p_handle));
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_UTIL) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*util_eng_get_clk_rate(p_util_handle)/jobm_eng_get_clk_rate(p_handle));
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_ROU) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*rou_eng_get_clk_rate(p_rou_handle)/jobm_eng_get_clk_rate(p_handle));
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_CAL) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 0)*cal_eng_get_clk_rate(p_cal_handle)/jobm_eng_get_clk_rate(p_handle));
			} else {
				DBG_ERR("unknown engine : %d\r\n", (int)p_info->eng);
				p_info->busy_cnt = 0;
			}
		}
			break;
		case KDRV_AI_PARAM_JOBM_ENG_BUSY_CNT_ARB2: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO *p_info = (KDRV_AI_JOBM_SINGLE_BUSY_CNT_INFO*)p_param;
			if (p_info->eng == KDRV_AI_JM_DBG_ENG_CONV0) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_conv0_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_CONV1) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_conv1_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_CONV2) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_conv2_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_CONV3) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_conv3_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_LSU) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_lsu_busy_cnt(p_handle)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_UTIL) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_ROU) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*600/240);
			} else if (p_info->eng == KDRV_AI_JM_DBG_ENG_CAL) {
				p_info->busy_cnt = (UINT32)((UINT64)jobm_eng_get_util_rou_cal_busy_cnt(p_handle, 1)*600/240);
			} else {
				DBG_ERR("unknown engine : %d\r\n", (int)p_info->eng);
				p_info->busy_cnt = 0;
			}
		}
			break;
		case KDRV_AI_PARAM_JOBM_ARB_STATUS: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_STATUS *p_info = (KDRV_AI_JOBM_ARB_STATUS*)p_param;
			UINT32 arb_status = jobm_eng_get_arb_status(p_handle, 0);
			if (nvt_get_chip_id() == CHIP_NS02302) {
				p_info->arb_status[KDRV_AI_ENG_CONV1] = arb_status & 0x1;
				p_info->arb_status[KDRV_AI_ENG_LSU]   = (arb_status >> 4) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_UTIL]  = (arb_status >> 5) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_ROU]   = (arb_status >> 6) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_CAL]   = (arb_status >> 7) & 0x1;
			} else if (nvt_get_chip_id() == CHIP_NS02402) {
				p_info->arb_status[KDRV_AI_ENG_CONV1] = arb_status & 0x1;
				p_info->arb_status[KDRV_AI_ENG_LSU]   = (arb_status >> 4) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_ROU]   = (arb_status >> 6) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_CAL]   = (arb_status >> 7) & 0x1;
			}				
		}
			break;
		case KDRV_AI_PARAM_JOBM_ARB_INFO: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_INFO *p_info = (KDRV_AI_JOBM_ARB_INFO*)p_param;
			
			if (p_info->eng < KDRV_AI_ENG_CONV1 || p_info->eng > KDRV_AI_ENG_CAL) {
				DBG_ERR("unknown engine : %d\r\n", (int)p_info->eng);
				return rt;
			}
			
			jobm_eng_get_eng_arb_info(p_handle, p_info->eng, &p_info->arb_jl_idx, &p_info->arb_dispatch_sel, &p_info->arb_net_id, &p_info->arb_job_addr, 0);		
		}
			break;
		case KDRV_AI_PARAM_JOBM_ARB2_STATUS: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_STATUS *p_info = (KDRV_AI_JOBM_ARB_STATUS*)p_param;
			UINT32 arb_status = jobm_eng_get_arb_status(p_handle, 1);
			if (nvt_get_chip_id() == CHIP_NS02302) {
				p_info->arb_status[KDRV_AI_ENG_CONV1] = arb_status & 0x1;
				p_info->arb_status[KDRV_AI_ENG_LSU]   = (arb_status >> 4) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_UTIL]  = (arb_status >> 5) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_ROU]   = (arb_status >> 6) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_CAL]   = (arb_status >> 7) & 0x1;
			} else if (nvt_get_chip_id() == CHIP_NS02402) {
				p_info->arb_status[KDRV_AI_ENG_CONV1] = arb_status & 0x1;
				p_info->arb_status[KDRV_AI_ENG_LSU]   = (arb_status >> 4) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_ROU]   = (arb_status >> 6) & 0x1;
				p_info->arb_status[KDRV_AI_ENG_CAL]   = (arb_status >> 7) & 0x1;
			}				
		}
			break;
		case KDRV_AI_PARAM_JOBM_ARB2_INFO: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_INFO *p_info = (KDRV_AI_JOBM_ARB_INFO*)p_param;
			
			if (p_info->eng < KDRV_AI_ENG_CONV1 || p_info->eng > KDRV_AI_ENG_CAL) {
				DBG_ERR("unknown engine : %d\r\n", (int)p_info->eng);
				return rt;
			}
			
			jobm_eng_get_eng_arb_info(p_handle, p_info->eng, &p_info->arb_jl_idx, &p_info->arb_dispatch_sel, &p_info->arb_net_id, &p_info->arb_job_addr, 1);		
		}
			break;
		case KDRV_AI_PARAM_JMISP_FRM_UPD_PARAM: {
			KDRV_AI_JMISP_FRM_UPD_PATH_INFO* jmisp_param = (KDRV_AI_JMISP_FRM_UPD_PATH_INFO*)p_param;
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			
			rt = jmisp_eng_get_frm_upd_path_info(p_handle, (JMISP_ENG_FRM_UPD_PATH_INFO*)jmisp_param);
		}
			break;
		case KDRV_AI_PARAM_JMISP_ARB_STATUS: {
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_STATUS *p_info = (KDRV_AI_JOBM_ARB_STATUS*)p_param;
			UINT32 arb_status = jmisp_eng_get_arb_status(p_handle);
			p_info->arb_status[KDRV_AI_ENG_PPU] = arb_status & 0x1;
			
		}
			break;
		case KDRV_AI_PARAM_JMISP_ARB_INFO: {
			JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_INFO *p_info = (KDRV_AI_JOBM_ARB_INFO*)p_param;
			
			if (p_info->eng != KDRV_AI_ENG_PPU && p_info->eng != KDRV_AI_ENG_POU) {
				DBG_ERR("unknown engine : %d\r\n", (int)p_info->eng);
				return rt;
			}
			
			jmisp_eng_get_eng_arb_info(p_handle, p_info->eng, &p_info->arb_jl_idx, &p_info->arb_dispatch_sel, 
										&p_info->arb_net_id, &p_info->arb_job_addr);		
		}
			break;
		case KDRV_AI_PARAM_JOBM_ARB_DBG_CYCLE: {
			JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
			KDRV_AI_JOBM_ARB_CYCLE *p_info = (KDRV_AI_JOBM_ARB_CYCLE*)p_param;
				
			jobm_eng_get_eng_arb_cycle(p_handle, p_info->eng, p_info->eng_cycle, p_info->arb_id);		
		}
			break;
		case KDRV_AI_PARAM_CLK_INFO:
			rt = kdrv_ai_get_clk_info((KDRV_AI_CLK_INFO*)p_param);
			break;	
		default:
			DBG_ERR("unknown param_id : %d\r\n", (int)param_id);
			break;
	}
	
	return rt;
}

INT32 kdrv_ai_init(VOID)
{
	ROU_ENG_HANDLE* p_rou_handle = NULL;
	UINT8* p_rou_handle_reg_base = NULL; 
	UINT8* p_rou_handle_reg_flag = NULL; 
	NUE2_ENG_HANDLE* p_nue2_handle = NULL;
	UINT8* p_nue2_handle_reg_base = NULL; 
	UINT8* p_nue2_handle_reg_flag = NULL;
	CONV_ENG_HANDLE* p_conv_handle = NULL;
	UINT8* p_conv_handle_reg_base = NULL; 
	UINT8* p_conv_handle_reg_flag = NULL;
	JOBM_ENG_HANDLE* p_jobm_handle = NULL;
	UINT8* p_jobm_handle_reg_base = NULL; 
	UINT8* p_jobm_handle_reg_flag = NULL;
	LSU_ENG_HANDLE* p_lsu_handle = NULL;
	UINT8* p_lsu_handle_reg_base = NULL; 
	UINT8* p_lsu_handle_reg_flag = NULL;
	UTIL_ENG_HANDLE* p_util_handle = NULL;
	UINT8* p_util_handle_reg_base = NULL; 
	UINT8* p_util_handle_reg_flag = NULL;
	CAL_ENG_HANDLE* p_cal_handle = NULL;
	UINT8* p_cal_handle_reg_base = NULL; 
	UINT8* p_cal_handle_reg_flag = NULL;
	PPU_ENG_HANDLE* p_ppu_handle = NULL;
	UINT8* p_ppu_handle_reg_base = NULL; 
	UINT8* p_ppu_handle_reg_flag = NULL;
	JMISP_ENG_HANDLE* p_jmisp_handle = NULL;
	UINT8* p_jmisp_handle_reg_base   = NULL; 
	UINT8* p_jmisp_handle_reg_flag   = NULL;
	POU_ENG_HANDLE* p_pou_handle = NULL;
	UINT8* p_pou_handle_reg_base = NULL; 
	UINT8* p_pou_handle_reg_flag = NULL;
	//int i = 0;
	
	// init ssdrv
	conv_eng_init(1, 1);
	//for (i = 0; i < 4; i++) {
		p_conv_handle = conv_eng_get_handle(0, 0);
		if (p_conv_handle == NULL) {
			DBG_ERR("CONV init fail\r\n");
			return -1;
		}
		p_conv_handle->chip_id = 0;
		p_conv_handle->eng_id  = 0;
		p_conv_handle->pclk    = ai_clk[0];
		//printk("CONV ai_eng_io_addr[0]\r\n");
		p_conv_handle->reg_io_base = ai_eng_io_addr[0];
		p_conv_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
		//if (i == 0) {
			p_conv_handle->isr_cb = kdrv_ai_conv1_isr_cb;
		//} else if (i == 1) {
		//	p_conv_handle->isr_cb = kdrv_ai_conv2_isr_cb;
		//} else if (i == 2) {
		//	p_conv_handle->isr_cb = kdrv_ai_conv3_isr_cb;
		//} else if (i == 3) {
		//	p_conv_handle->isr_cb = kdrv_ai_conv4_isr_cb;
		//}
		p_conv_handle->clock_rate = 600;
		snprintf(&p_conv_handle->name[0], sizeof(p_conv_handle->name), "conv");
		conv_eng_init_resource(p_conv_handle);
		p_conv_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(conv_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
		p_conv_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(conv_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
		conv_eng_set_reg_buf(p_conv_handle, (uintptr_t)p_conv_handle_reg_base, (uintptr_t)p_conv_handle_reg_flag);
		if (conv_eng_open(p_conv_handle)) {
			return -1;
		}
		conv_eng_reg_isr_callback(p_conv_handle, p_conv_handle->isr_cb);
	//}	
	
	nue2_eng_init(1, 1);
	p_nue2_handle = nue2_eng_get_handle(0, 0);
	if (p_nue2_handle == NULL) {
		DBG_ERR("NUE2 init fail\r\n");
		return -1;
	}
	p_nue2_handle->chip_id = 0;
	p_nue2_handle->eng_id  = 0;
	p_nue2_handle->pclk    = ai_clk[1];
	//printk("NUE2 ai_eng_io_addr[1]\r\n");
	p_nue2_handle->reg_io_base = ai_eng_io_addr[1];
	p_nue2_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_nue2_handle->isr_cb = kdrv_ai_nue2_isr_cb;
	p_nue2_handle->clock_rate = 600;
	snprintf(&p_nue2_handle->name[0], sizeof(p_nue2_handle->name), "nue2");
	nue2_eng_init_resource(p_nue2_handle);
	p_nue2_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(nue2_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_nue2_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(nue2_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	nue2_eng_set_reg_buf(p_nue2_handle, (uintptr_t)p_nue2_handle_reg_base, (uintptr_t)p_nue2_handle_reg_flag);
	if (nue2_eng_open(p_nue2_handle)) {
		return -1;
	}
	nue2_eng_reg_isr_callback(p_nue2_handle, p_nue2_handle->isr_cb);
	
	ppu_eng_init(1, 1);
	p_ppu_handle = ppu_eng_get_handle(0, 0);
	if (p_ppu_handle == NULL) {
		DBG_ERR("PPU init fail\r\n");
		return -1;
	}
	p_ppu_handle->chip_id = 0;
	p_ppu_handle->eng_id  = 0;
	p_ppu_handle->pclk    = ai_clk[2];
	//printk("PPU ai_eng_io_addr[2]\r\n");
	p_ppu_handle->reg_io_base = ai_eng_io_addr[2];
	p_ppu_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_ppu_handle->isr_cb = kdrv_ai_ppu_isr_cb;
	p_ppu_handle->clock_rate = 600;
	snprintf(&p_ppu_handle->name[0], sizeof(p_ppu_handle->name), "ppu");
	ppu_eng_init_resource(p_ppu_handle);
	p_ppu_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(ppu_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_ppu_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(ppu_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	ppu_eng_set_reg_buf(p_ppu_handle, (uintptr_t)p_ppu_handle_reg_base, (uintptr_t)p_ppu_handle_reg_flag);
	if (ppu_eng_open(p_ppu_handle)) {
		return -1;
	}
	ppu_eng_reg_isr_callback(p_ppu_handle, p_ppu_handle->isr_cb);
	
	lsu_eng_init(1, 1);
	p_lsu_handle = lsu_eng_get_handle(0, 0);
	if (p_lsu_handle == NULL) {
		DBG_ERR("LSU init fail\r\n");
		return -1;
	}
	p_lsu_handle->chip_id = 0;
	p_lsu_handle->eng_id  = 0;
	p_lsu_handle->pclk    = ai_clk[3];
	//printk("LSU ai_eng_io_addr[3]\r\n");
	p_lsu_handle->reg_io_base = ai_eng_io_addr[3];
	p_lsu_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_lsu_handle->isr_cb = kdrv_ai_lsu_isr_cb;
	p_lsu_handle->clock_rate = 600;
	snprintf(&p_lsu_handle->name[0], sizeof(p_lsu_handle->name), "lsu");
	lsu_eng_init_resource(p_lsu_handle);
	p_lsu_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(lsu_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_lsu_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(lsu_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	lsu_eng_set_reg_buf(p_lsu_handle, (uintptr_t)p_lsu_handle_reg_base, (uintptr_t)p_lsu_handle_reg_flag);
	if (lsu_eng_open(p_lsu_handle)) {
		return -1;
	}
	lsu_eng_reg_isr_callback(p_lsu_handle, p_lsu_handle->isr_cb);
	
	jobm_eng_init(1, 1);
	p_jobm_handle = jobm_eng_get_handle(0, 0);
	if (p_jobm_handle == NULL) {
		DBG_ERR("JOBM init fail\r\n");
		return -1;
	}
	p_jobm_handle->chip_id = 0;
	p_jobm_handle->eng_id  = 0;
	p_jobm_handle->pclk    = 0;//ai_clk[1];
	//printk("JM ai_eng_io_addr[4]\r\n");
	p_jobm_handle->reg_io_base = ai_eng_io_addr[4];
	p_jobm_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_jobm_handle->isr_cb = kdrv_ai_jobm_isr_cb;
	p_jobm_handle->clock_rate = 150;
	snprintf(&p_jobm_handle->name[0], sizeof(p_jobm_handle->name), "jobm");
	jobm_eng_init_resource(p_jobm_handle);
	p_jobm_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(jobm_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_jobm_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(jobm_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	jobm_eng_set_reg_buf(p_jobm_handle, (uintptr_t)p_jobm_handle_reg_base, (uintptr_t)p_jobm_handle_reg_flag);
	if (jobm_eng_open(p_jobm_handle)) {
		return -1;
	}
	jobm_eng_reg_isr_callback(p_jobm_handle, p_jobm_handle->isr_cb);
		
	jmisp_eng_init(1, 1);
	p_jmisp_handle = jmisp_eng_get_handle(0, 0);
	if (p_jmisp_handle == NULL) {
		DBG_ERR("JMISP init fail\r\n");
		return -1;
	}
	p_jmisp_handle->chip_id = 0;
	p_jmisp_handle->eng_id  = 0;
	p_jmisp_handle->pclk    = 0;//ai_clk[1];
	//printk("JMISP ai_eng_io_addr[5]\r\n");
	p_jmisp_handle->reg_io_base = ai_eng_io_addr[5];
	p_jmisp_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_jmisp_handle->isr_cb = kdrv_ai_jmisp_isr_cb;
	p_jmisp_handle->clock_rate = 150;
	snprintf(&p_jmisp_handle->name[0], sizeof(p_jmisp_handle->name), "jmisp");
	jmisp_eng_init_resource(p_jmisp_handle);
	p_jmisp_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(jmisp_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_jmisp_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(jmisp_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	jmisp_eng_set_reg_buf(p_jmisp_handle, (uintptr_t)p_jmisp_handle_reg_base, (uintptr_t)p_jmisp_handle_reg_flag);
	if (jmisp_eng_open(p_jmisp_handle)) {
		return -1;
	}
	jmisp_eng_reg_isr_callback(p_jmisp_handle, p_jmisp_handle->isr_cb);
	
	if (nvt_get_chip_id() == CHIP_NS02302) {
		// 538 driver/emulation flow
		util_eng_init(1, 1);
		p_util_handle = util_eng_get_handle(0, 0);
		if (p_util_handle == NULL) {
			DBG_ERR("UTIL init fail\r\n");
			return -1;
		}
		p_util_handle->chip_id = 0;
		p_util_handle->eng_id  = 0;
		p_util_handle->pclk    = ai_clk[6];
		//printk("UTIL ai_eng_io_addr[6]\r\n");
		p_util_handle->reg_io_base = ai_eng_io_addr[6];
		p_util_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
		p_util_handle->isr_cb = kdrv_ai_util_isr_cb;
		p_util_handle->clock_rate = 600;
		snprintf(&p_util_handle->name[0], sizeof(p_util_handle->name), "util");
		util_eng_init_resource(p_util_handle);
		p_util_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(util_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
		p_util_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(util_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
		util_eng_set_reg_buf(p_util_handle, (uintptr_t)p_util_handle_reg_base, (uintptr_t)p_util_handle_reg_flag);
		if (util_eng_open(p_util_handle)) {
			return -1;
		}
		util_eng_reg_isr_callback(p_util_handle, p_util_handle->isr_cb);
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
		// 539A driver/emulation flow
		pou_eng_init(1, 1);
		p_pou_handle = pou_eng_get_handle(0, 0);
		if (p_pou_handle == NULL) {
			DBG_ERR("POU init fail\r\n");
			return -1;
		}
		p_pou_handle->chip_id = 0;
		p_pou_handle->eng_id  = 0;
		p_pou_handle->pclk    = ai_clk[6];
		//printk("POU ai_eng_io_addr[6]\r\n");
		p_pou_handle->reg_io_base = ai_eng_io_addr[6];
		p_pou_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
		p_pou_handle->isr_cb = kdrv_ai_pou_isr_cb;
		p_pou_handle->clock_rate = 600;
		snprintf(&p_pou_handle->name[0], sizeof(p_pou_handle->name), "pou");
		pou_eng_init_resource(p_pou_handle); 
		p_pou_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(pou_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
		p_pou_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(pou_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
		pou_eng_set_reg_buf(p_pou_handle, (uintptr_t)p_pou_handle_reg_base, (uintptr_t)p_pou_handle_reg_flag);
		if (pou_eng_open(p_pou_handle)) {
			return -1;
		}
		pou_eng_reg_isr_callback(p_pou_handle, p_pou_handle->isr_cb);
	}
	
	rou_eng_init(1, 1);
	p_rou_handle = rou_eng_get_handle(0, 0);
	if (p_rou_handle == NULL) {
		DBG_ERR("ROU init fail\r\n");
		return -1;
	}
	p_rou_handle->chip_id = 0;
	p_rou_handle->eng_id  = 0;
	if (nvt_get_chip_id() == CHIP_NS02302) {
	p_rou_handle->pclk    = ai_clk[6];
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	p_rou_handle->pclk    = ai_clk[7];
	}
	//printk("ROU ai_eng_io_addr[7]\r\n");
	p_rou_handle->reg_io_base = ai_eng_io_addr[7];
	p_rou_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_rou_handle->isr_cb = kdrv_ai_rou_isr_cb;
	p_rou_handle->clock_rate = 600;
	snprintf(&p_rou_handle->name[0], sizeof(p_rou_handle->name), "rou");
	rou_eng_init_resource(p_rou_handle);
	p_rou_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(rou_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_rou_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(rou_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	rou_eng_set_reg_buf(p_rou_handle, (uintptr_t)p_rou_handle_reg_base, (uintptr_t)p_rou_handle_reg_flag);
	if (rou_eng_open(p_rou_handle)) {
		return -1;
	}
	rou_eng_reg_isr_callback(p_rou_handle, p_rou_handle->isr_cb);
	
	cal_eng_init(1, 1);
	p_cal_handle = cal_eng_get_handle(0, 0);
	if (p_cal_handle == NULL) {
		DBG_ERR("CAL init fail\r\n");
		return -1;
	}
	p_cal_handle->chip_id = 0;
	p_cal_handle->eng_id  = 0;
	if (nvt_get_chip_id() == CHIP_NS02302) {
	p_cal_handle->pclk    = ai_clk[6];
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
	p_cal_handle->pclk    = ai_clk[7];
	}	
	//printk("CAL ai_eng_io_addr[8]\r\n");
	p_cal_handle->reg_io_base = ai_eng_io_addr[8];
	p_cal_handle->irq_id = 0;//pive_drv_eng_info[i].irq_no;
	p_cal_handle->isr_cb = kdrv_ai_cal_isr_cb;
	p_cal_handle->clock_rate = 600;
	snprintf(&p_cal_handle->name[0], sizeof(p_cal_handle->name), "cal");
	cal_eng_init_resource(p_cal_handle);
	p_cal_handle_reg_base = (UINT8*)kdrv_ai_alloc_mem(cal_eng_get_reg_base_buf_size(0)*sizeof(UINT8)); 
	p_cal_handle_reg_flag = (UINT8*)kdrv_ai_alloc_mem(cal_eng_get_reg_flag_buf_size(0)*sizeof(UINT8)); 
	cal_eng_set_reg_buf(p_cal_handle, (uintptr_t)p_cal_handle_reg_base, (uintptr_t)p_cal_handle_reg_flag);
	if (cal_eng_open(p_cal_handle)) {
		return -1;
	}
	cal_eng_reg_isr_callback(p_cal_handle, p_cal_handle->isr_cb);
	
	return 0;
}

INT32 kdrv_ai_uninit(VOID)
{
	ROU_ENG_HANDLE* p_rou_handle = NULL;
	NUE2_ENG_HANDLE* p_nue2_handle = NULL;
	CONV_ENG_HANDLE* p_conv_handle = NULL;
	JOBM_ENG_HANDLE* p_jobm_handle = NULL;
	LSU_ENG_HANDLE* p_lsu_handle = NULL;
	UTIL_ENG_HANDLE* p_util_handle = NULL;
	CAL_ENG_HANDLE* p_cal_handle = NULL;
	PPU_ENG_HANDLE* p_ppu_handle = NULL;
	JMISP_ENG_HANDLE* p_jmisp_handle = NULL;	
	POU_ENG_HANDLE* p_pou_handle = NULL;
	//int i = 0;

	//for (i = 0; i < 4; i++) {
		p_conv_handle = conv_eng_get_handle(0, 0);
		if (p_conv_handle) {
			conv_eng_close(p_conv_handle);
			kdrv_ai_free_mem((VOID*)p_conv_handle->p_conv_reg_st);
			kdrv_ai_free_mem((VOID*)p_conv_handle->p_conv_reg_chg_flag);
		}
	//}
	conv_eng_release();
	
	p_nue2_handle = nue2_eng_get_handle(0, 0);
	if (p_nue2_handle) {
		nue2_eng_close(p_nue2_handle);
		kdrv_ai_free_mem((VOID*)p_nue2_handle->p_nue2_reg_st);
		kdrv_ai_free_mem((VOID*)p_nue2_handle->p_nue2_reg_chg_flag);
		nue2_eng_release();
	}




	p_ppu_handle = ppu_eng_get_handle(0, 0);
	if (p_ppu_handle) {
		ppu_eng_close(p_ppu_handle);
		kdrv_ai_free_mem((VOID*)p_ppu_handle->p_ppu_reg_st);
		kdrv_ai_free_mem((VOID*)p_ppu_handle->p_ppu_reg_chg_flag);
		ppu_eng_release();
	}
	
	p_lsu_handle = lsu_eng_get_handle(0, 0);
	if (p_lsu_handle) {
		lsu_eng_close(p_lsu_handle);
		kdrv_ai_free_mem((VOID*)p_lsu_handle->p_lsu_reg_st);
		kdrv_ai_free_mem((VOID*)p_lsu_handle->p_lsu_reg_chg_flag);
		lsu_eng_release();
	}
	
	p_jobm_handle = jobm_eng_get_handle(0, 0);
	if (p_jobm_handle) {
		jobm_eng_close(p_jobm_handle);
		kdrv_ai_free_mem((VOID*)p_jobm_handle->p_jobm_reg_st);
		kdrv_ai_free_mem((VOID*)p_jobm_handle->p_jobm_reg_chg_flag);
		jobm_eng_release();
	}
	
	p_jmisp_handle = jmisp_eng_get_handle(0, 0);
	if (p_jmisp_handle) {
		jmisp_eng_close(p_jmisp_handle);
		kdrv_ai_free_mem((VOID*)p_jmisp_handle->p_jmisp_reg_st);
		kdrv_ai_free_mem((VOID*)p_jmisp_handle->p_jmisp_reg_chg_flag);
		jmisp_eng_release();
	}
	
	if (nvt_get_chip_id() == CHIP_NS02302) {	
		p_util_handle = util_eng_get_handle(0, 0);
		if (p_util_handle) {
			util_eng_close(p_util_handle);
			kdrv_ai_free_mem((VOID*)p_util_handle->p_util_reg_st);
			kdrv_ai_free_mem((VOID*)p_util_handle->p_util_reg_chg_flag);
			util_eng_release();
		}
	} else if (nvt_get_chip_id() == CHIP_NS02402) {
		p_pou_handle = pou_eng_get_handle(0, 0);
		if (p_pou_handle) {
			pou_eng_close(p_pou_handle);
			kdrv_ai_free_mem((VOID*)p_pou_handle->p_pou_reg_st);
			kdrv_ai_free_mem((VOID*)p_pou_handle->p_pou_reg_chg_flag);
			pou_eng_release();
		}
	}
	
	p_rou_handle = rou_eng_get_handle(0, 0);
	if (p_rou_handle) {
		rou_eng_close(p_rou_handle);
		kdrv_ai_free_mem((VOID*)p_rou_handle->p_rou_reg_st);
		kdrv_ai_free_mem((VOID*)p_rou_handle->p_rou_reg_chg_flag);
		rou_eng_release();
	}
	
	p_cal_handle = cal_eng_get_handle(0, 0);
	if (p_cal_handle) {
		cal_eng_close(p_cal_handle);
		kdrv_ai_free_mem((VOID*)p_cal_handle->p_cal_reg_st);
		kdrv_ai_free_mem((VOID*)p_cal_handle->p_cal_reg_chg_flag);
		cal_eng_release();
	}
		
	return 0;
}

INT32 kdrv_ai_trigger(KDRV_AI_ENG eng, uintptr_t pa)
{
	if (pa == 0) {
		return -1;
	}
	
	if (eng == KDRV_AI_ENG_ROU) {
		ROU_ENG_HANDLE* p_rou_handle = rou_eng_get_handle(0, 0);
		rou_eng_set_intrpt_en(p_rou_handle, 0x1000000);
		rou_eng_trig_ll_hw_reg(p_rou_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_ROU_LL1) {
		ROU_ENG_HANDLE* p_rou_handle = rou_eng_get_handle(0, 0);
		rou_eng_set_intrpt_en(p_rou_handle, 0x1000000);
		rou_eng_trig_ll_hw_reg(p_rou_handle, pa >> 32, pa & 0xFFFFFFFF,1);
	} else if (eng == KDRV_AI_ENG_NUE2) {
		NUE2_ENG_HANDLE* p_nue2_handle = nue2_eng_get_handle(0, 0);
		nue2_eng_set_intrpt_en(p_nue2_handle, 0x100);
		nue2_eng_trig_ll_hw_reg(p_nue2_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_CONV1) {
		CONV_ENG_HANDLE* p_conv_handle = conv_eng_get_handle(0, 0);
		conv_eng_set_intrpt_en(p_conv_handle, 0x1000000);
		conv_eng_trig_ll_hw_reg(p_conv_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_CONV1_LL1) {
		CONV_ENG_HANDLE* p_conv_handle = conv_eng_get_handle(0, 0);
		conv_eng_set_intrpt_en(p_conv_handle, 0x1000000);
		conv_eng_trig_ll_hw_reg(p_conv_handle, pa >> 32, pa & 0xFFFFFFFF,1);
	} else if (eng == KDRV_AI_ENG_JOBM_JLA || eng == KDRV_AI_ENG_JOBM_JLB || 
			   eng == KDRV_AI_ENG_JOBM_JLC || eng == KDRV_AI_ENG_JOBM_JLD || eng == KDRV_AI_ENG_JOBM_JLE) {
		JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
		UINT32 interrupt_en = JOBM_ENG_INTERRUPT_KDRV_JLA;
		jobm_eng_set_intrpt_en(p_handle, interrupt_en << (eng-KDRV_AI_ENG_JOBM_JLA));
		jobm_eng_trig_jl_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF, eng - KDRV_AI_ENG_JOBM_JLA);
	} else if (eng == KDRV_AI_ENG_LSU) {
		LSU_ENG_HANDLE* p_handle = lsu_eng_get_handle(0, 0);
		lsu_eng_set_intrpt_en(p_handle, 0x1000000);
		lsu_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_UTIL) {
		if (nvt_get_chip_id() == CHIP_NS02302) {
		UTIL_ENG_HANDLE* p_handle = util_eng_get_handle(0, 0);
		util_eng_set_intrpt_en(p_handle, 0x1000000);
		util_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,0);
		}
	} else if (eng == KDRV_AI_ENG_UTIL_LL1) {
		if (nvt_get_chip_id() == CHIP_NS02302) {
		UTIL_ENG_HANDLE* p_handle = util_eng_get_handle(0, 0);
		util_eng_set_intrpt_en(p_handle, 0x1000000);
		util_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,1);
		}
	} else if (eng == KDRV_AI_ENG_CAL) {
		CAL_ENG_HANDLE* p_handle = cal_eng_get_handle(0, 0);
		cal_eng_set_intrpt_en(p_handle, 0x1000000);
		cal_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_CAL_LL1) {
		CAL_ENG_HANDLE* p_handle = cal_eng_get_handle(0, 0);
		cal_eng_set_intrpt_en(p_handle, 0x1000000);
		cal_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,1);
	} else if (eng == KDRV_AI_ENG_PPU) {
		PPU_ENG_HANDLE* p_handle = ppu_eng_get_handle(0, 0);
		ppu_eng_set_intrpt_en(p_handle, 0x1000000);
		ppu_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_POU) {
		POU_ENG_HANDLE* p_handle = pou_eng_get_handle(0, 0);
		pou_eng_set_intrpt_en(p_handle, 0x1000000);
		pou_eng_trig_ll_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF,0);
	} else if (eng == KDRV_AI_ENG_JMISP_PLA || eng == KDRV_AI_ENG_JMISP_PLB || 
			   eng == KDRV_AI_ENG_JMISP_PLC || eng == KDRV_AI_ENG_JMISP_PLD) {
		JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
		JOBM_ENG_HANDLE  *p_jm_handle = jobm_eng_get_handle(0, 0);
		UINT32 jmisp_int_en = JMISP_ENG_INTERRUPT_KDRV;
		// disable correspoding JM interrupt enable for preventing dummy isr
		jobm_eng_set_intrpt_dis(p_jm_handle, 1 << (eng-KDRV_AI_ENG_JMISP_PLA));
		jmisp_int_en &= 0xFFFFFFF0;
		jmisp_int_en |= g_jmisp_int_en;
		jmisp_eng_set_intrpt_en(p_handle, jmisp_int_en, eng - KDRV_AI_ENG_JMISP_PLA);
		jmisp_eng_trig_pl_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF, eng - KDRV_AI_ENG_JMISP_PLA);
	} else if (eng == KDRV_AI_ENG_JOBM_JLA_DBG || eng == KDRV_AI_ENG_JOBM_JLB_DBG || 
			   eng == KDRV_AI_ENG_JOBM_JLC_DBG || eng == KDRV_AI_ENG_JOBM_JLD_DBG || eng == KDRV_AI_ENG_JOBM_JLE_DBG) {
		JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
		UINT32 interrupt_en = JOBM_ENG_INTERRUPT_KDRV_JLA | JOBM_ENG_INTERRUPT_JLA_JOB_END;
		jobm_eng_set_intrpt_en(p_handle, interrupt_en << (eng-KDRV_AI_ENG_JOBM_JLA_DBG));
		jobm_eng_trig_jl_hw_reg(p_handle, pa >> 32, pa & 0xFFFFFFFF, eng - KDRV_AI_ENG_JOBM_JLA_DBG);
	}
	
	return 0;
}

INT32 kdrv_ai_jm_dbg_run_next(KDRV_AI_JOBM_DBG_PARAM jm_dbg_param)
{
	JOBM_ENG_HANDLE *p_handle = jobm_eng_get_handle(0, 0);
	
	if (jm_dbg_param.dbg_mode == KDRV_AI_JM_DBG_MODE_NONE) {
		// do nothing
	} else if (jm_dbg_param.dbg_mode == KDRV_AI_JM_DBG_MODE_SINGLE) {	
		jobm_eng_run_single_dbg(p_handle, jm_dbg_param.single_jl_id);
	} else if (jm_dbg_param.dbg_mode == KDRV_AI_JM_DBG_MODE_ARB) {
		jobm_eng_run_arb_dbg(p_handle, jm_dbg_param.arb_eng, 0);
	} else if (jm_dbg_param.dbg_mode == KDRV_AI_JM_DBG_MODE_ARB2) {
		jobm_eng_run_arb_dbg(p_handle, jm_dbg_param.arb_eng, 1);
	} 
	
	return 0;
}

INT32 kdrv_ai_jmisp_dbg_run_next(KDRV_AI_JMISP_DBG_PARAM jmisp_dbg_param)
{
	JMISP_ENG_HANDLE *p_handle = jmisp_eng_get_handle(0, 0);
	
	if (jmisp_dbg_param.dbg_mode == KDRV_AI_JMISP_DBG_MODE_NONE) {
		// do nothing
	} else if (jmisp_dbg_param.dbg_mode == KDRV_AI_JMISP_DBG_MODE_SINGLE) {	
		jmisp_eng_run_single_dbg(p_handle, jmisp_dbg_param.single_pl_id);
	} else if (jmisp_dbg_param.dbg_mode == KDRV_AI_JMISP_DBG_MODE_ARB) {
		jmisp_eng_run_arb_dbg(p_handle, jmisp_dbg_param.arb_eng);
	} 
	
	return 0;
}

UINT32 kdrv_ai_get_eng_caps(KDRV_AI_ENG eng)
{
	UINT32 is_valid = 1;
	if(eng==KDRV_AI_ENG_CONV3 || eng==KDRV_AI_ENG_CONV4)
		is_valid = 0;
	return is_valid;
}

INT32 kdrv_ai_dma_abort(KDRV_AI_ENG eng)
{
	ER er_code = E_OK;
	// TODO
	return er_code;
}

const CHAR *kdrv_ai_get_version(VOID)
{
	return KDRV_AI_IMPL_VERSION;
}

#ifdef __KERNEL__
EXPORT_SYMBOL(kdrv_ai_trigger);
//EXPORT_SYMBOL(kdrv_ai_trigger_isr);
//EXPORT_SYMBOL(kdrv_ai_reset);
EXPORT_SYMBOL(kdrv_ai_init);
EXPORT_SYMBOL(kdrv_ai_uninit);
EXPORT_SYMBOL(kdrv_ai_set);
EXPORT_SYMBOL(kdrv_ai_get);
EXPORT_SYMBOL(kdrv_ai_jm_dbg_run_next);
EXPORT_SYMBOL(kdrv_ai_jmisp_dbg_run_next);
EXPORT_SYMBOL(kdrv_ai_get_eng_caps);
//EXPORT_SYMBOL(kdrv_ai_reset_status);
EXPORT_SYMBOL(kdrv_ai_dma_abort);
EXPORT_SYMBOL(kdrv_ai_engine_reset);
EXPORT_SYMBOL(kdrv_ai_get_version);  
#endif
