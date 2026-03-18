/**
    Public header file for PPU module.

    @file       ppu_eng.h
    @ingroup    mIIPPPPU

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _PPU_ENG_H_
#define _PPU_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kdrv_type.h"
#include "ppu_eng_base.h"
#include "ppu_eng_handle.h"
#include "ppu_eng_dma_base.h"

#define PPU_ENG_REG_NUMS  ((0x21C>>2)+1)  // for NT98690

#define PPU_SSD_DRV_NAME "Ssdrv_ppu"
#define PPU_SSD_DRV_MODULE_VERSION "1.00.Beta.01"

// interrupt flag
#define FLGPTN_PPU_FRAMEND      FLGPTN_BIT(0)
#define FLGPTN_PPU_LLEND        FLGPTN_BIT(24)
#define FLGPTN_PPU_LLERROR     	FLGPTN_BIT(25)

/*****************************************************************************/
typedef struct {
	UINT32 ofs;
	UINT32 val;
} PPU_ENG_REG;

typedef enum {
	PPU_ENG_INTERRUPT_FRM_END            = 0x00000001,
	PPU_ENG_INTERRUPT_LL_END             = 0x01000000,
    PPU_ENG_INTERRUPT_LL_ERR             = 0X02000000,
    PPU_ENG_INTERRUPT_JOB_END            = 0X04000000,
	PPU_ENG_INTERRUPT_ALL                = (PPU_ENG_INTERRUPT_FRM_END | PPU_ENG_INTERRUPT_LL_END | PPU_ENG_INTERRUPT_LL_ERR | PPU_ENG_INTERRUPT_JOB_END),
} PPU_ENG_INTERRUPT;

typedef VOID (*PPU_ISR_CB)(VOID *eng, UINT32 status, VOID *reserve);

extern INT32 ppu_eng_init(UINT32 chip_num, UINT32 eng_num);
extern INT32 ppu_eng_release(VOID);

extern PPU_ENG_HANDLE* ppu_eng_get_handle(UINT32 chip_id, UINT32 eng_id);
extern VOID ppu_eng_reg_isr_callback(PPU_ENG_HANDLE *p_eng, PPU_ISR_CB cb);
extern INT32 ppu_eng_open(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_close(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_reset(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_trig_single_hw_reg(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_trig_ll_hw_reg(PPU_ENG_HANDLE *p_eng, uintptr_t ll_addr_msb, uintptr_t ll_addr_lsb);
extern INT32 ppu_eng_init_resource(PPU_ENG_HANDLE *p_eng);
extern INT32 ppu_eng_set_intrpt_en(PPU_ENG_HANDLE *p_eng, UINT32 int_en);
extern VOID ppu_eng_clr_intr_status(PPU_ENG_HANDLE *p_eng, UINT32 uiIntrStatus);
extern VOID ppu_eng_isr_hw_reg(PPU_ENG_HANDLE *p_eng);
extern VOID ppu_eng_wait_framend_ll(PPU_ENG_HANDLE *p_eng);

extern UINT32 ppu_eng_get_reg_base_buf_size(UINT32 eng_id);
extern UINT32 ppu_eng_get_reg_flag_buf_size(UINT32 eng_id);

extern UINT32 ppu_eng_get_eng_cycle(PPU_ENG_HANDLE *p_eng);
extern UINT32 ppu_eng_get_ll_cycle(PPU_ENG_HANDLE *p_eng);
extern UINT32 ppu_eng_get_wait_cycle(PPU_ENG_HANDLE *p_eng);
extern UINT32 ppu_eng_get_dram_bw(PPU_ENG_HANDLE *p_eng);
extern UINT32 ppu_eng_get_ub_bw(PPU_ENG_HANDLE *p_eng);
extern BOOL ppu_eng_get_dma_idle(PPU_ENG_HANDLE *p_eng);

extern VOID ppu_eng_set_reg_buf(PPU_ENG_HANDLE *p_eng, uintptr_t reg_base_addr, uintptr_t reg_flag_addr);
extern VOID ppu_eng_dma_channel_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID ppu_eng_axi_channel_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en);
extern VOID ppu_eng_cycle_enable_hw_reg(PPU_ENG_HANDLE *p_eng, BOOL set_en);
extern BOOL ppu_ssdrv_proc_ver(VOID);
extern UINT32 ppu_eng_get_clk_rate(PPU_ENG_HANDLE *p_eng);

#endif //_PPU_ENG_H_
