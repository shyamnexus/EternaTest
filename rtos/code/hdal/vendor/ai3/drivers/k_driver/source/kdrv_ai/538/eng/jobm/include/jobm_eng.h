/**
    Public header file for JOBM module.

    @file       jobm_eng.h
    @ingjobmp    mIIPPJOBM

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _JOBM_ENG_H_
#define _JOBM_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "jobm_eng_base.h"
#include "jobm_eng_handle.h"
#include "jobm_eng_dma_base.h"

#define JOBM_ENG_REG_NUMS  (0xF2C/4+1)  // for NT98538

#define JOBM_SSD_DRV_NAME "Ssdrv_jobm"
#define JOBM_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

#define FLGPTN_JOBM_JLA_END     	FLGPTN_BIT(0)
#define FLGPTN_JOBM_JLB_END     	FLGPTN_BIT(1)
#define FLGPTN_JOBM_JLC_END     	FLGPTN_BIT(2)
#define FLGPTN_JOBM_JLD_END     	FLGPTN_BIT(3)
#define FLGPTN_JOBM_JLE_END     	FLGPTN_BIT(4)

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} JOBM_ENG_REG;

typedef enum {
	JOBM_ENG_INTERRUPT_JLA_END            = 0x00000001,
	JOBM_ENG_INTERRUPT_JLB_END            = 0x00000002,
	JOBM_ENG_INTERRUPT_JLC_END            = 0x00000004,
	JOBM_ENG_INTERRUPT_JLD_END            = 0x00000008,
	JOBM_ENG_INTERRUPT_JLE_END            = 0x00000010,
	
	JOBM_ENG_INTERRUPT_JLA_ERR            = 0x00000100,
	JOBM_ENG_INTERRUPT_JLB_ERR            = 0x00000200,
	JOBM_ENG_INTERRUPT_JLC_ERR            = 0x00000400,
	JOBM_ENG_INTERRUPT_JLD_ERR            = 0x00000800,
	JOBM_ENG_INTERRUPT_JLE_ERR            = 0x00001000,
	
	JOBM_ENG_INTERRUPT_JLA_JOB_END        = 0x00010000,
	JOBM_ENG_INTERRUPT_JLB_JOB_END        = 0x00020000,
	JOBM_ENG_INTERRUPT_JLC_JOB_END        = 0x00040000,
	JOBM_ENG_INTERRUPT_JLD_JOB_END        = 0x00080000,
	JOBM_ENG_INTERRUPT_JLE_JOB_END        = 0x00100000,

	JOBM_ENG_INTERRUPT_JLA_JOB_ERR        = 0x01000000,
	JOBM_ENG_INTERRUPT_JLB_JOB_ERR        = 0x02000000,
	JOBM_ENG_INTERRUPT_JLC_JOB_ERR        = 0x04000000,
	JOBM_ENG_INTERRUPT_JLD_JOB_ERR        = 0x08000000,
	JOBM_ENG_INTERRUPT_JLE_JOB_ERR        = 0x10000000,
	
	JOBM_ENG_INTERRUPT_KDRV_JLA           = 0x01000101,
	JOBM_ENG_INTERRUPT_KDRV               = 0x1F001F1F,
	JOBM_ENG_INTERRUPT_ALL                = 0x1F1F1F1F,
} JOBM_ENG_INTERRUPT;

typedef void (*JOBM_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 jobm_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 jobm_eng_release(void);

extern JOBM_ENG_HANDLE* jobm_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void jobm_eng_reg_isr_callback(JOBM_ENG_HANDLE *p_eng, JOBM_ISR_CB cb);
extern INT32 jobm_eng_open(JOBM_ENG_HANDLE *p_eng);
extern INT32 jobm_eng_close(JOBM_ENG_HANDLE *p_eng);
//extern void jobm_eng_trig_single_hw_reg(JOBM_ENG_HANDLE *p_eng);
//extern void jobm_eng_trig_ll_hw_reg(JOBM_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern void jobm_eng_trig_jl_hw_reg(JOBM_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 jl_id);
extern INT32 jobm_eng_init_resource(JOBM_ENG_HANDLE *p_eng);

extern UINT32 jobm_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 jobm_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern VOID jobm_eng_set_reg_buf(JOBM_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID jobm_eng_dma_channel_enable_hw_reg(JOBM_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID jobm_eng_axi_channel_enable_hw_reg(JOBM_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL jobm_ssdrv_proc_ver(void);
extern INT32 jobm_eng_set_intrpt_en(JOBM_ENG_HANDLE *p_eng,  UINT32 int_en);
extern INT32 jobm_eng_set_intrpt_dis(JOBM_ENG_HANDLE *p_eng, UINT32 int_dis);
extern void jobm_eng_isr_hw_reg(JOBM_ENG_HANDLE *p_eng);

extern INT32 jobm_eng_set_single_dbg_en(JOBM_ENG_HANDLE *p_eng, UINT32 enable, UINT32 jl_id);
extern INT32 jobm_eng_run_single_dbg(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);
extern INT32 jobm_eng_set_arb_dbg_en(JOBM_ENG_HANDLE *p_eng, UINT32 enable_eng, UINT32 arb_id);
extern INT32 jobm_eng_run_arb_dbg(JOBM_ENG_HANDLE *p_eng, UINT32 enable_eng, UINT32 arb_id);

extern INT32 jobm_eng_set_priority(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id, UINT32 no_dispatch_cnt, UINT32 priority);
extern void jobm_eng_wait_framend(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);

extern INT32 jobm_eng_reset(JOBM_ENG_HANDLE *p_eng);
extern BOOL jobm_eng_chk_dma_channel_idle_reg(JOBM_ENG_HANDLE *p_eng);

extern UINT32 jobm_eng_get_joblist_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);
extern UINT32 jobm_eng_get_wait_dma_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);
extern UINT32 jobm_eng_get_wait_unit_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id); 
extern UINT32 jobm_eng_get_dram_bw(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);
extern UINT32 jobm_eng_get_ub_bw(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id);

extern UINT32 jobm_eng_set_cnt_clr(JOBM_ENG_HANDLE *p_eng, UINT32 clr_eng);
extern UINT32 jobm_eng_get_conv0_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_conv1_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_conv2_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_conv3_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_lsu_busy_cnt(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_util_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_rou_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_cal_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_util_rou_cal_busy_cnt(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);

extern INT32 jobm_eng_set_unlock_idle_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 cycle, UINT32 jl_id);
extern INT32 jobm_eng_set_preload(JOBM_ENG_HANDLE *p_eng, UINT32 enable);

extern INT32 jobm_eng_set_tcm_measure_enable(JOBM_ENG_HANDLE *p_eng, UINT32 enable);
extern INT32 jobm_eng_set_tcm_measure_time(JOBM_ENG_HANDLE *p_eng, UINT32 cycle);
extern UINT32 jobm_eng_get_tcm0_write_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm1_write_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm2_write_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm3_write_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm0_read_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm1_read_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm2_read_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_tcm3_read_count(JOBM_ENG_HANDLE *p_eng);
extern UINT32 jobm_eng_get_arb_status(JOBM_ENG_HANDLE *p_eng, UINT32 arb_id);
extern UINT32 jobm_eng_get_eng_arb_info(JOBM_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *jl_idx, UINT32 *dispatch_sel, UINT32 *net_id, UINT64 *job_addr, UINT32 arb_id);
extern UINT32 jobm_eng_get_eng_arb_cycle(JOBM_ENG_HANDLE *p_eng, UINT32 eng, UINT32 *eng_cycle, UINT32 arb_id);
extern INT32 jobm_eng_set_net_id(JOBM_ENG_HANDLE *p_eng, UINT32 jl_id, UINT32 net_id);
extern UINT32 jobm_eng_get_clk_rate(JOBM_ENG_HANDLE *p_eng);
#endif //_JOBM_ENG_H_
