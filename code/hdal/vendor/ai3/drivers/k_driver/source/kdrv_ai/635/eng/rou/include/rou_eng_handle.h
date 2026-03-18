/**
    Public header file for ROU module.

    @file       rou_eng_handle.h
    @ingroup    mIIPPROU

    @brief

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/
#ifndef _ROU_ENG_HANDLE_H_
#define _ROU_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "rou_eng_int_reg.h"

/*****************************************************************************/
typedef void (*ROU_ISR_CB)(void *eng, UINT32 status, void *reserve, UINT32 ll_idx);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98635_ROU_REGISTER_STRUCT *p_rou_reg_st;
	volatile UINT8 *p_rou_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	ROU_ISR_CB isr_cb;
	ID     flg_id_rou;
} ROU_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	ROU_ENG_HANDLE *p_eng;
} ROU_ENG_CTL;


#endif //_ROU_ENG_HANDLE_H_
