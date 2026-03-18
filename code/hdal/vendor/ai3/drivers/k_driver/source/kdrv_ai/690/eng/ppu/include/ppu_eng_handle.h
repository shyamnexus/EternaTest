/**
    Public header file for PPU module.

    @file       ppu_eng_handle.h
    @ingroup    mIIPPPPU

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _PPU_ENG_HANDLE_H_
#define _PPU_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "ppu_eng_int_reg.h"

/*****************************************************************************/
typedef void (*PPU_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98690_PPU_REGISTER_STRUCT *p_ppu_reg_st;
	volatile UINT8 *p_ppu_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	PPU_ISR_CB isr_cb;
	ID     flg_id_ppu;
} PPU_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	PPU_ENG_HANDLE *p_eng;
} PPU_ENG_CTL;


#endif //_PPU_ENG_HANDLE_H_
