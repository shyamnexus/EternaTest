/**
    Public header file for POU module.

    @file       pou_eng.h
    @ingpoup    mIIPPPOU

    @brief

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#ifndef _POU_ENG_H_
#define _POU_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "pou_eng_base.h"
#include "pou_eng_handle.h"
#include "pou_eng_dma_base.h"

#define POU_ENG_REG_NUMS  ((0x504>>2)+1)  // for NT98539A

#define POU_SSD_DRV_NAME "Ssdrv_pou"
#define POU_SSD_DRV_MODULE_VERSION "1.00.Beta.01"


// interrupt flag
#define FLGPTN_POU_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_POU_LLEND        FLGPTN_BIT(24)
#define FLGPTN_POU_LLERROR      FLGPTN_BIT(25)
/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} POU_ENG_REG;

typedef enum {
	POU_ENG_INTERRUPT_FRM_END             = 0x00000001,
	POU_ENG_INTERRUPT_LL_END              = 0x01000000,
    POU_ENG_INTERRUPT_LL_ERR              = 0X02000000,
    POU_ENG_INTERRUPT_JOB_END             = 0X04000000,
	POU_ENG_INTERRUPT_ALL                 = (POU_ENG_INTERRUPT_FRM_END | POU_ENG_INTERRUPT_LL_END | POU_ENG_INTERRUPT_LL_ERR | POU_ENG_INTERRUPT_JOB_END),
} POU_ENG_INTERRUPT;

typedef VOID (*POU_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve, UINT32 ll_idx);

extern INT32 pou_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 pou_eng_release(VOID);

extern POU_ENG_HANDLE* pou_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID pou_eng_reg_isr_callback(POU_ENG_HANDLE *p_eng, POU_ISR_CB cb);
extern INT32 pou_eng_open(POU_ENG_HANDLE *p_eng);
extern INT32 pou_eng_close(POU_ENG_HANDLE *p_eng);
extern INT32 pou_eng_reset(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_trig_single_hw_reg(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_trig_ll_hw_reg(POU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx);
extern INT32 pou_eng_init_resource(POU_ENG_HANDLE *p_eng);

extern INT32 pou_eng_set_intrpt_en(POU_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID pou_eng_clr_intr_status(POU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID pou_eng_isr_hw_reg(POU_ENG_HANDLE *p_eng);
extern VOID pou_eng_wait_framend_ll(POU_ENG_HANDLE *p_eng, UINT32 ll_idx);

extern UINT32 pou_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 pou_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 pou_eng_get_eng_cycle(POU_ENG_HANDLE *p_eng);
extern UINT32 pou_eng_get_ll_cycle(POU_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern UINT32 pou_eng_get_wait_cycle(POU_ENG_HANDLE *p_eng);
extern UINT32 pou_eng_get_dram_bw(POU_ENG_HANDLE *p_eng);
extern UINT32 pou_eng_get_ub_bw(POU_ENG_HANDLE *p_eng);
extern BOOL pou_eng_get_dma_idle(POU_ENG_HANDLE *p_eng);

extern VOID pou_eng_set_reg_buf(POU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID pou_eng_dma_channel_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID pou_eng_axi_channel_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID pou_eng_cycle_enable_hw_reg(POU_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL pou_ssdrv_proc_ver(VOID);
extern UINT32 pou_eng_get_clk_rate(POU_ENG_HANDLE *p_eng);

#endif //_POU_ENG_H_
