/**
    Public header file for CAL module.

    @file       cal_eng_handle.h
    @ingroup    mIIPPCAL

    @brief

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/


#ifndef _CAL_ENG_HANDLE_H_
#define _CAL_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "cal_eng_int_reg.h"

/*****************************************************************************/
typedef void (*CAL_ISR_CB)(void *eng, UINT32 status, void *reserve, UINT32 ll_idx);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_CAL_REGISTER_STRUCT *p_cal_reg_st;
	volatile UINT8 *p_cal_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	CAL_ISR_CB isr_cb;
	ID     flg_id_cal;
} CAL_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	CAL_ENG_HANDLE *p_eng;
} CAL_ENG_CTL;


#endif //_CAL_ENG_HANDLE_H_
