/**
    Public header file for POU module.

    @file       pou_eng_handle.h
    @ingpoup    mIIPPPOU

    @brief

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _POU_ENG_HANDLE_H_
#define _POU_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "pou_eng_int_reg.h"

/*****************************************************************************/
typedef void (*POU_ISR_CB)(void *eng, UINT32 status, void *reserve, UINT32 ll_idx);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_POU_REGISTER_STRUCT *p_pou_reg_st;
	volatile UINT8 *p_pou_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	POU_ISR_CB isr_cb;
	ID     flg_id_pou;
} POU_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	POU_ENG_HANDLE *p_eng;
} POU_ENG_CTL;

#endif //_POU_ENG_HANDLE_H_
