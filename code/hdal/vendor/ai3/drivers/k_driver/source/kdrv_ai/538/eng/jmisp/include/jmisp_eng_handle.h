/**
    Public header file for JMISP module.

    @file       jmisp_eng_handle.h
    @ingjmispp    mIIPPJMISP

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _JMISP_ENG_HANDLE_H_
#define _JMISP_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "jmisp_eng_int_reg.h"

/*****************************************************************************/
typedef void (*JMISP_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_JMISP_REGISTER_STRUCT *p_jmisp_reg_st;
	volatile UINT8 *p_jmisp_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	JMISP_ISR_CB isr_cb;
	ID     flg_id_jmisp;
} JMISP_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	JMISP_ENG_HANDLE *p_eng;
} JMISP_ENG_CTL;


#endif //_JMISP_ENG_HANDLE_H_
