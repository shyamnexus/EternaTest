/**
    Public header file for TRKE module.

    @file       trke_eng_handle.h
    @ingroup    mIIPPTRKE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _TRKE_ENG_HANDLE_H_
#define _TRKE_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "trke_eng_int_reg.h"

/*****************************************************************************/
typedef void (*TRKE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_TRKE_REGISTER_STRUCT *p_trke_reg_st;
	volatile UINT8 *p_trke_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	TRKE_ISR_CB isr_cb;
	ID     flg_id_trke;
} TRKE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	TRKE_ENG_HANDLE *p_eng;
} TRKE_ENG_CTL;


#endif //_TRKE_ENG_HANDLE_H_
