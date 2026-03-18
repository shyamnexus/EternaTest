/**
    Public header file for ROU module.

    @file       rou_eng.h
    @ingroup    mIIPPROU

    @brief

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#ifndef _ROU_ENG_H_
#define _ROU_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "rou_eng_base.h"
#include "rou_eng_handle.h"
#include "rou_eng_dma_base.h"

#define ROU_ENG_REG_NUMS        ((0x4FC>>2)+1)

#define ROU_SSD_DRV_NAME "Ssdrv_rou"
#define ROU_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

// interrupt flag
#define FLGPTN_ROU_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_ROU_LL0END       FLGPTN_BIT(24)
#define FLGPTN_ROU_LL0ERROR    	FLGPTN_BIT(25)
#define FLGPTN_ROU_LL1END       FLGPTN_BIT(28)
#define FLGPTN_ROU_LL1ERROR    	FLGPTN_BIT(29)


/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} ROU_ENG_REG;

typedef enum {
	ROU_ENG_INTERRUPT_FRM_END            = 0x00000001,
	ROU_ENG_INTERRUPT_LL0_END            = 0x01000000,
    ROU_ENG_INTERRUPT_LL0_ERR            = 0X02000000,
    ROU_ENG_INTERRUPT_JOB0_END           = 0X04000000,
    ROU_ENG_INTERRUPT_LL1_END            = 0x10000000,
    ROU_ENG_INTERRUPT_LL1_ERR            = 0X20000000,
    ROU_ENG_INTERRUPT_JOB1_END           = 0X40000000,
	ROU_ENG_INTERRUPT_ALL                = (ROU_ENG_INTERRUPT_FRM_END | ROU_ENG_INTERRUPT_LL0_END | ROU_ENG_INTERRUPT_LL0_ERR | ROU_ENG_INTERRUPT_JOB0_END | ROU_ENG_INTERRUPT_LL1_END | ROU_ENG_INTERRUPT_LL1_ERR | ROU_ENG_INTERRUPT_JOB1_END),
} ROU_ENG_INTERRUPT;

typedef struct {
    UINT32 unlock_cycle;
    UINT32 priority_mode;
    UINT32 qos_tot_time;
    UINT32 qos_ocpy_time;
    UINT32 sta_period_time;
} ROU_LL_SETTING;


typedef VOID (*ROU_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve, UINT32 ll_idx);

extern INT32 rou_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 rou_eng_release(VOID);

extern ROU_ENG_HANDLE* rou_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID rou_eng_reg_isr_callback(ROU_ENG_HANDLE *p_eng, ROU_ISR_CB cb);
extern INT32 rou_eng_open(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_close(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_reset(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_trig_single_hw_reg(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_trig_ll_hw_reg(ROU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb, UINT32 ll_idx);
extern INT32 rou_eng_init_resource(ROU_ENG_HANDLE *p_eng);
extern INT32 rou_eng_set_intrpt_en(ROU_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID rou_eng_clr_intr_status(ROU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID rou_eng_isr_hw_reg(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_wait_framend_ll(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx);

extern UINT32 rou_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 rou_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 rou_eng_get_eng_cycle(ROU_ENG_HANDLE *p_eng);
extern UINT32 rou_eng_get_ll_cycle(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern UINT32 rou_eng_get_wait_cycle(ROU_ENG_HANDLE *p_eng);
extern UINT32 rou_eng_get_dram_bw(ROU_ENG_HANDLE *p_eng);
extern UINT32 rou_eng_get_ub_bw(ROU_ENG_HANDLE *p_eng);
extern BOOL rou_eng_get_dma_idle(ROU_ENG_HANDLE *p_eng);
extern UINT32 rou_eng_get_ll_sta_acc_cycle(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern UINT32 rou_eng_get_clk_rate(ROU_ENG_HANDLE *p_eng);

extern INT32 rou_eng_set_ll_setting(ROU_ENG_HANDLE *p_eng, ROU_LL_SETTING *ll_param);
extern UINT32 rou_eng_get_qos_setting(ROU_ENG_HANDLE *p_eng);
extern VOID rou_eng_set_reg_buf(ROU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID rou_eng_dma_channel_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID rou_eng_axi_channel_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID rou_eng_cycle_enable_hw_reg(ROU_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL rou_ssdrv_proc_ver(void);

extern UINT32 rou_eng_get_busy_count(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx);
extern VOID rou_eng_clr_busy_count(ROU_ENG_HANDLE *p_eng, UINT32 ll_idx);

#endif //_ROU_ENG_H_
