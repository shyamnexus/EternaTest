/**
    Public header file for CAL module

    @file       cal_eng.h
    @ingroup    mIIPPCAL

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _CAL_ENG_H_
#define _CAL_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "cal_eng_base.h"
#include "cal_eng_handle.h"
#include "cal_eng_dma_base.h"

#define CAL_ENG_REG_NUMS  ((0x240>>2)+1)  // for NT98690

#define CAL_SSD_DRV_NAME "Ssdrv_cal"
#define CAL_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

// interrupt flag
#define FLGPTN_CAL_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_CAL_LLEND        FLGPTN_BIT(24)
#define FLGPTN_CAL_LLERROR     	FLGPTN_BIT(25)


/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} CAL_ENG_REG;

typedef enum {
	CAL_ENG_INTERRUPT_FRM_END            = 0x00000001,
	CAL_ENG_INTERRUPT_LL_END             = 0x01000000,
    CAL_ENG_INTERRUPT_LL_ERR             = 0X02000000,
    CAL_ENG_INTERRUPT_JOB_END            = 0X04000000,
	CAL_ENG_INTERRUPT_ALL                = (CAL_ENG_INTERRUPT_FRM_END | CAL_ENG_INTERRUPT_LL_END | CAL_ENG_INTERRUPT_LL_ERR | CAL_ENG_INTERRUPT_JOB_END),
} CAL_ENG_INTERRUPT;

typedef VOID (*CAL_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve);

extern INT32 cal_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 cal_eng_release(VOID);

extern CAL_ENG_HANDLE* cal_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID cal_eng_reg_isr_callback(CAL_ENG_HANDLE *p_eng, CAL_ISR_CB cb);
extern INT32 cal_eng_open(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_close(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_reset(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_trig_single_hw_reg(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_trig_ll_hw_reg(CAL_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern INT32 cal_eng_init_resource(CAL_ENG_HANDLE *p_eng);
extern INT32 cal_eng_set_intrpt_en(CAL_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID cal_eng_clr_intr_status(CAL_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID cal_eng_isr_hw_reg(CAL_ENG_HANDLE *p_eng);
extern VOID cal_eng_wait_framend_ll(CAL_ENG_HANDLE *p_eng);

extern UINT32 cal_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 cal_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 cal_eng_get_eng_cycle(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_get_ll_cycle(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_get_wait_cycle(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_get_dram_bw(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_get_ub_bw(CAL_ENG_HANDLE *p_eng);
extern BOOL cal_eng_get_dma_idle(CAL_ENG_HANDLE *p_eng);
extern UINT32 cal_eng_get_clk_rate(CAL_ENG_HANDLE *p_eng);

extern VOID cal_eng_set_reg_buf(CAL_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID cal_eng_dma_channel_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID cal_eng_axi_channel_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID cal_eng_cycle_enable_hw_reg(CAL_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL cal_ssdrv_proc_ver(void);


#endif //_CAL_ENG_H_
