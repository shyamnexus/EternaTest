/**
    Public header file for JOBM module.

    @file       jobm_eng_handle.h
    @ingjobmp    mIIPPJOBM

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/


#ifndef _JOBM_ENG_HANDLE_H_
#define _JOBM_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "jobm_eng_int_reg.h"

/*****************************************************************************/
typedef void (*JOBM_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98690_JOBM_REGISTER_STRUCT *p_jobm_reg_st;
	volatile UINT8 *p_jobm_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	JOBM_ISR_CB isr_cb;
	ID     flg_id_jobm;
} JOBM_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	JOBM_ENG_HANDLE *p_eng;
} JOBM_ENG_CTL;


#endif //_JOBM_ENG_HANDLE_H_
