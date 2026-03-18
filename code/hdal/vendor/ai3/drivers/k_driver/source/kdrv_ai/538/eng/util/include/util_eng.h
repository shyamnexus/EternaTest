/**
    Public header file for UTIL module.

    @file       util_eng.h
    @ingutilp    mIIPPUTIL

    @brief

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _UTIL_ENG_H_
#define _UTIL_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "util_eng_base.h"
#include "util_eng_handle.h"
#include "util_eng_dma_base.h"

#define UTIL_ENG_REG_NUMS  ((0x4fc>>2)+1)  // for NT98538

#define UTIL_SSD_DRV_NAME "Ssdrv_util"
#define UTIL_SSD_DRV_MODULE_VERSION "1.00.Beta.01"


// interrupt flag
#define FLGPTN_UTIL_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_UTIL_LL0END       FLGPTN_BIT(24)
#define FLGPTN_UTIL_LL0ERROR     FLGPTN_BIT(25)
#define FLGPTN_UTIL_LL1END       FLGPTN_BIT(28)
#define FLGPTN_UTIL_LL1ERROR     FLGPTN_BIT(29)
/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} UTIL_ENG_REG;

typedef enum {
	UTIL_ENG_INTERRUPT_FRM_END            = 0x00000001,
	UTIL_ENG_INTERRUPT_LL0_END            = 0x01000000,
    	UTIL_ENG_INTERRUPT_LL0_ERR            = 0X02000000,
    	UTIL_ENG_INTERRUPT_JOB0_END           = 0X04000000,
	UTIL_ENG_INTERRUPT_LL1_END            = 0x10000000,
    	UTIL_ENG_INTERRUPT_LL1_ERR            = 0X20000000,
    	UTIL_ENG_INTERRUPT_JOB1_END           = 0X40000000,
	UTIL_ENG_INTERRUPT_ALL                = (UTIL_ENG_INTERRUPT_FRM_END | UTIL_ENG_INTERRUPT_LL0_END | UTIL_ENG_INTERRUPT_LL0_ERR | UTIL_ENG_INTERRUPT_JOB0_END | UTIL_ENG_INTERRUPT_LL1_END | UTIL_ENG_INTERRUPT_LL1_ERR | UTIL_ENG_INTERRUPT_JOB1_END),
} UTIL_ENG_INTERRUPT;


typedef struct {
    UINT32 unlock_cycle;
    UINT32 priority_mode;
    UINT32 qos_tot_time;
    UINT32 qos_ocpy_time;
    UINT32 sta_period_time;
} UTIL_LL_SETTING;

typedef VOID (*UTIL_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve, UINT32 ll_idx);

extern INT32 util_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 util_eng_release(VOID);

extern UTIL_ENG_HANDLE* util_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID util_eng_reg_isr_callback(UTIL_ENG_HANDLE *p_eng, UTIL_ISR_CB cb);
extern INT32 util_eng_open(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_close(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_reset(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_trig_single_hw_reg(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_trig_ll_hw_reg(UTIL_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx);
extern INT32 util_eng_init_resource(UTIL_ENG_HANDLE *p_eng);

extern INT32 util_eng_set_ll_setting(UTIL_ENG_HANDLE *p_eng, UTIL_LL_SETTING *ll_param);
extern UINT32 util_eng_get_qos_setting(UTIL_ENG_HANDLE *p_eng);
extern INT32 util_eng_set_intrpt_en(UTIL_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID  util_eng_clr_intr_status(UTIL_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);

extern VOID util_eng_isr_hw_reg(UTIL_ENG_HANDLE *p_eng);
extern VOID util_eng_wait_framend_ll(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern BOOL util_eng_get_dma_idle(UTIL_ENG_HANDLE *p_eng);

extern UINT32 util_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 util_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 util_eng_get_eng_cycle(UTIL_ENG_HANDLE *p_eng);
extern UINT32 util_eng_get_ll_cycle(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern UINT32 util_eng_get_ll_sta_acc_cycle(UTIL_ENG_HANDLE *p_eng, UINT32 ll_idx);

extern UINT32 util_eng_get_wait_cycle(UTIL_ENG_HANDLE *p_eng);
extern UINT32 util_eng_get_dram_bw(UTIL_ENG_HANDLE *p_eng);
extern UINT32 util_eng_get_ub_bw(UTIL_ENG_HANDLE *p_eng);

extern VOID util_eng_set_reg_buf(UTIL_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID util_eng_dma_channel_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID util_eng_axi_channel_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID util_eng_cycle_enable_hw_reg(UTIL_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL util_ssdrv_proc_ver(VOID);
extern UINT32 util_eng_get_clk_rate(UTIL_ENG_HANDLE *p_eng);

#endif //_UTIL_ENG_H_
