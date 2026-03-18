/**
    Public header file for IVE module.

    @file       ive_eng_handle.h
    @ingroup    mIIPPIVE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _IVE_ENG_HANDLE_H_
#define _IVE_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "ive_eng_int_reg.h"

/*****************************************************************************/
typedef void (*IVE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_IVE_REGISTER_STRUCT *p_ive_reg_st;
	volatile UINT8 *p_ive_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	IVE_ISR_CB isr_cb;
	ID     flg_id_ive;
} IVE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	IVE_ENG_HANDLE *p_eng;
} IVE_ENG_CTL;


#endif //_IVE_ENG_HANDLE_H_
