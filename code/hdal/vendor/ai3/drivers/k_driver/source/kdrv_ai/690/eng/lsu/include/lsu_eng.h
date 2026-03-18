/**
    Public header file for LSU module.

    @file       lsu_eng.h
    @inglsup    mIIPPLSU

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _LSU_ENG_H_
#define _LSU_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "lsu_eng_base.h"
#include "lsu_eng_handle.h"
#include "lsu_eng_dma_base.h"

#define LSU_ENG_REG_NUMS  ((0x11c>>2)+1)  // for NT98690

#define LSU_SSD_DRV_NAME "Ssdrv_lsu"
#define LSU_SSD_DRV_MODULE_VERSION "1.00.Beta.01"


// interrupt flag
#define FLGPTN_LSU_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_LSU_LENERROR     FLGPTN_BIT(4)
#define FLGPTN_LSU_LLEND        FLGPTN_BIT(24)
#define FLGPTN_LSU_LLERROR      FLGPTN_BIT(25)
/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} LSU_ENG_REG;

typedef enum {
	LSU_ENG_INTERRUPT_FRM_END             = 0x00000001,
	LSU_ENG_INTERRUPT_LEN_ERR             = 0x00000010,
	LSU_ENG_INTERRUPT_LL_END              = 0x01000000,
    LSU_ENG_INTERRUPT_LL_ERR              = 0X02000000,
    LSU_ENG_INTERRUPT_JOB_END             = 0X04000000,
	LSU_ENG_INTERRUPT_ALL                 = (LSU_ENG_INTERRUPT_FRM_END | LSU_ENG_INTERRUPT_LEN_ERR | LSU_ENG_INTERRUPT_LL_END | LSU_ENG_INTERRUPT_LL_ERR | LSU_ENG_INTERRUPT_JOB_END),
} LSU_ENG_INTERRUPT;

typedef VOID (*LSU_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve);

extern INT32 lsu_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 lsu_eng_release(VOID);

extern LSU_ENG_HANDLE* lsu_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID lsu_eng_reg_isr_callback(LSU_ENG_HANDLE *p_eng, LSU_ISR_CB cb);
extern INT32 lsu_eng_open(LSU_ENG_HANDLE *p_eng);
extern INT32 lsu_eng_close(LSU_ENG_HANDLE *p_eng);
extern INT32 lsu_eng_reset(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_trig_single_hw_reg(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_trig_ll_hw_reg(LSU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern INT32 lsu_eng_init_resource(LSU_ENG_HANDLE *p_eng);

extern INT32 lsu_eng_set_intrpt_en(LSU_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID lsu_eng_clr_intr_status(LSU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID lsu_eng_isr_hw_reg(LSU_ENG_HANDLE *p_eng);
extern VOID lsu_eng_wait_framend_ll(LSU_ENG_HANDLE *p_eng);

extern UINT32 lsu_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 lsu_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 lsu_eng_get_eng_cycle(LSU_ENG_HANDLE *p_eng);
extern UINT32 lsu_eng_get_ll_cycle(LSU_ENG_HANDLE *p_eng);
extern UINT32 lsu_eng_get_wait_cycle(LSU_ENG_HANDLE *p_eng);
extern UINT32 lsu_eng_get_dram_bw(LSU_ENG_HANDLE *p_eng);
extern UINT32 lsu_eng_get_ub_bw(LSU_ENG_HANDLE *p_eng);
extern BOOL lsu_eng_get_dma_idle(LSU_ENG_HANDLE *p_eng);

extern VOID lsu_eng_set_reg_buf(LSU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID lsu_eng_dma_channel_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID lsu_eng_axi_channel_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID lsu_eng_cycle_enable_hw_reg(LSU_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL lsu_ssdrv_proc_ver(VOID);
extern UINT32 lsu_eng_get_clk_rate(LSU_ENG_HANDLE *p_eng);

#endif //_LSU_ENG_H_
