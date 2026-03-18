/**
    Public header file for CONV module.

    @file       conv_eng_handle.h
    @ingconvp    mIIPPCONV

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _CONV_ENG_HANDLE_H_
#define _CONV_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "conv_eng_int_reg.h"

/*****************************************************************************/
typedef void (*CONV_ISR_CB)(void *eng, UINT32 status, void *reserve, UINT32 ll_idx);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98635_CONV_REGISTER_STRUCT *p_conv_reg_st;
	volatile UINT8 *p_conv_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	CONV_ISR_CB isr_cb;
	ID     flg_id_conv;
} CONV_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	CONV_ENG_HANDLE *p_eng;
} CONV_ENG_CTL;


#endif //_CONV_ENG_HANDLE_H_
