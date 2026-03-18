/**
    Public header file for NUE2 module.

    @file       nue2_eng_handle.h
    @ingnue2p    mIIPPNUE2

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _NUE2_ENG_HANDLE_H_
#define _NUE2_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "nue2_eng_int_reg.h"

/*****************************************************************************/
typedef void (*NUE2_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98690_NUE2_REGISTER_STRUCT *p_nue2_reg_st;
	volatile UINT8 *p_nue2_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	NUE2_ISR_CB isr_cb;
	ID     flg_id_nue2;
} NUE2_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	NUE2_ENG_HANDLE *p_eng;
} NUE2_ENG_CTL;


#endif //_NUE2_ENG_HANDLE_H_
