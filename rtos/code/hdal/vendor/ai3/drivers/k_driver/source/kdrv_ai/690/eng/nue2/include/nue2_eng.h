/**
    Public header file for NUE2 module.

    @file       nue2_eng.h
    @ingnue2p    mIIPPNUE2

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _NUE2_ENG_H_
#define _NUE2_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "nue2_eng_base.h"
#include "nue2_eng_handle.h"
#include "nue2_eng_dma_base.h"

#define NUE2_ENG_REG_NUMS  (0x190/4+1)  // for NT98690

#define NUE2_SSD_DRV_NAME "Ssdrv_nue2"
#define NUE2_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} NUE2_ENG_REG;

typedef enum {
	NUE2_ENG_INTERRUPT_FRM_END            = 0x00000001,
	NUE2_ENG_INTERRUPT_LL_END             = 0x00000100,
    NUE2_ENG_INTERRUPT_LL_ERR             = 0x00000200,
    NUE2_ENG_INTERRUPT_JOB_END            = 0x00000400,
	NUE2_ENG_INTERRUPT_ALL                = (NUE2_ENG_INTERRUPT_FRM_END | NUE2_ENG_INTERRUPT_LL_END | NUE2_ENG_INTERRUPT_LL_ERR | NUE2_ENG_INTERRUPT_JOB_END),
} NUE2_ENG_INTERRUPT;

typedef void (*NUE2_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 nue2_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 nue2_eng_release(void);

extern NUE2_ENG_HANDLE* nue2_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void nue2_eng_reg_isr_callback(NUE2_ENG_HANDLE *p_eng, NUE2_ISR_CB cb);
extern INT32 nue2_eng_open(NUE2_ENG_HANDLE *p_eng);
extern INT32 nue2_eng_close(NUE2_ENG_HANDLE *p_eng);
extern INT32 nue2_eng_reset(NUE2_ENG_HANDLE *p_eng);
extern void nue2_eng_trig_single_hw_reg(NUE2_ENG_HANDLE *p_eng);
extern void nue2_eng_trig_ll_hw_reg(NUE2_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern INT32 nue2_eng_init_resource(NUE2_ENG_HANDLE *p_eng);

extern UINT32 nue2_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 nue2_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 nue2_eng_get_eng_cycle(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_get_ll_cycle(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_get_wait_cycle(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_get_dram_bw(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_get_ub_bw(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_cycle_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL nue2_eng_get_dma_idle(NUE2_ENG_HANDLE *p_eng);

extern VOID nue2_eng_set_reg_buf(NUE2_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID nue2_eng_dma_channel_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID nue2_eng_axi_channel_enable_hw_reg(NUE2_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL nue2_ssdrv_proc_ver(void);
extern INT32 nue2_eng_set_intrpt_en(NUE2_ENG_HANDLE *p_eng,  UINT32 int_en);
extern void nue2_eng_isr_hw_reg(NUE2_ENG_HANDLE *p_eng);
extern VOID nue2_eng_clr_intr_status(NUE2_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID nue2_eng_wait_framend_ll(NUE2_ENG_HANDLE *p_eng);
extern UINT32 nue2_eng_get_clk_rate(NUE2_ENG_HANDLE *p_eng);

#endif //_NUE2_ENG_H_
