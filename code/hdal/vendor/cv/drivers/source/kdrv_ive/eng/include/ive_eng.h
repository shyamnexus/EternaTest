/**
    Public header file for IVE module.

    @file       ive_eng.h
    @ingroup    mIIPPIVE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _IVE_ENG_H_
#define _IVE_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "ive_eng_base.h"
#include "ive_eng_handle.h"
#include "ive_eng_dma_base.h"

#define IVE_ENG_REG_NUMS  116  // for NT98690

#define IVE_SSD_DRV_NAME "Ssdrv_ive"
#define IVE_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} IVE_ENG_REG;

typedef enum {
	IVE_ENG_INTERRUPT_FRM_END            = 0x00000001,
	IVE_ENG_INTERRUPT_LL_END             = 0x00000100,
    IVE_ENG_INTERRUPT_LL_ERR             = 0X00000200,
    IVE_ENG_INTERRUPT_JOB_END            = 0X00000400,	
	IVE_ENG_INTERRUPT_ALL                = (IVE_ENG_INTERRUPT_FRM_END | IVE_ENG_INTERRUPT_LL_END |IVE_ENG_INTERRUPT_LL_ERR | IVE_ENG_INTERRUPT_JOB_END  ),
} IVE_ENG_INTERRUPT;

typedef void (*IVE_ISR_CB)(void *eng, UINT32 status, void *reserve);

extern INT32 ive_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 ive_eng_release(void);

extern IVE_ENG_HANDLE* ive_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern void ive_eng_reg_isr_callback(IVE_ENG_HANDLE *p_eng, IVE_ISR_CB cb);
extern INT32 ive_eng_open(IVE_ENG_HANDLE *p_eng);
extern INT32 ive_eng_close(IVE_ENG_HANDLE *p_eng);
extern void ive_eng_trig_single_hw_reg(IVE_ENG_HANDLE *p_eng);
extern void ive_eng_trig_ll_hw_reg(IVE_ENG_HANDLE *p_eng, ULONG ll_addr);
extern INT32 ive_eng_init_resource(IVE_ENG_HANDLE *p_eng);
extern UINT32 ive_eng_wait_flg(IVE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg);
extern UINT32 ive_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 ive_eng_get_reg_flag_buf_size(UINT32 eng_id);
extern VOID ive_eng_clr_intr_status(IVE_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID ive_eng_set_reg_buf(IVE_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID ive_eng_dma_channel_enable_hw_reg(IVE_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL ive_ssdrv_proc_ver(void);
extern INT32 ive_eng_set_intrpt_en(IVE_ENG_HANDLE *p_eng,  UINT32 int_en);
extern void ive_eng_isr_hw_reg(IVE_ENG_HANDLE *p_eng);
extern void ive_eng_wait_framend_ll(IVE_ENG_HANDLE *p_eng);
#endif //_IVE_ENG_H_
