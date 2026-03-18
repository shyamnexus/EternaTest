/**
    Public header file for CONV module.

    @file       conv_eng.h
    @ingconvp    mIIPPCONV

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _CONV_ENG_H_
#define _CONV_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "conv_eng_base.h"
#include "conv_eng_handle.h"
#include "conv_eng_dma_base.h"

#define CONV_ENG_REG_NUMS  (0x724/4+1)  // for NT98690

#define CONV_SSD_DRV_NAME "Ssdrv_conv"
#define CONV_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} CONV_ENG_REG;

typedef enum {
	CONV_ENG_INTERRUPT_FRM_END            = 0x00000001,
	CONV_ENG_INTERRUPT_LL_END             = 0x01000000,
    CONV_ENG_INTERRUPT_LL_ERR             = 0x02000000,
    CONV_ENG_INTERRUPT_JOB_END            = 0x04000000,
	CONV_ENG_INTERRUPT_ALL                = (CONV_ENG_INTERRUPT_FRM_END | CONV_ENG_INTERRUPT_LL_END | CONV_ENG_INTERRUPT_LL_ERR | CONV_ENG_INTERRUPT_JOB_END),
} CONV_ENG_INTERRUPT;

typedef void (*CONV_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 conv_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 conv_eng_release(void);

extern CONV_ENG_HANDLE* conv_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void conv_eng_reg_isr_callback(CONV_ENG_HANDLE *p_eng, CONV_ISR_CB cb);
extern INT32 conv_eng_open(CONV_ENG_HANDLE *p_eng);
extern INT32 conv_eng_close(CONV_ENG_HANDLE *p_eng);
extern INT32 conv_eng_reset(CONV_ENG_HANDLE *p_eng);
extern void conv_eng_trig_single_hw_reg(CONV_ENG_HANDLE *p_eng);
extern void conv_eng_trig_ll_hw_reg(CONV_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern INT32 conv_eng_init_resource(CONV_ENG_HANDLE *p_eng);

extern UINT32 conv_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 conv_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 conv_eng_get_eng_cycle(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_ll_cycle(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_wait_cycle(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_dram_bw(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_ub_bw(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_cycle_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL conv_eng_get_dma_idle(CONV_ENG_HANDLE *p_eng);

extern VOID conv_eng_set_reg_buf(CONV_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID conv_eng_axi_channel_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID conv_eng_dma_channel_enable_hw_reg(CONV_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL conv_ssdrv_proc_ver(void);
extern INT32 conv_eng_set_intrpt_en(CONV_ENG_HANDLE *p_eng,  UINT32 int_en);
extern void conv_eng_isr_hw_reg(CONV_ENG_HANDLE *p_eng);
extern VOID conv_eng_clr_intr_status(CONV_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID conv_eng_wait_framend_ll(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_out_chksum(CONV_ENG_HANDLE *p_eng);
extern UINT32 conv_eng_get_clk_rate(CONV_ENG_HANDLE *p_eng);

#endif //_CONV_ENG_H_
