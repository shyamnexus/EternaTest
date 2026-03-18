/**
    Public header file for ISE module.

    @file       ise_eng_handle.h
    @ingroup    mIIPPISE

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _ISE_ENG_HANDLE_H_
#define _ISE_ENG_HANDLE_H_


#include "ise_eng_int_reg.h"
#include "kwrap/flag.h"

/*****************************************************************************/
typedef void (*ISE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	//SEM_HANDLE sem;
	//ID flg_id;
	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_ISE_REGISTER_STRUCT *p_ise_reg_st;
	volatile UINT8 *p_ise_reg_chg_flag;

	ULONG reg_io_base;

	UINT32 irq_id;
	ISE_ISR_CB isr_cb;
} ISE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	ISE_ENG_HANDLE *p_eng;
} ISE_ENG_CTL;


#endif //_ISE_ENG_HANDLE_H_
