/**
    Public header file for MD module.

    @file       md_eng_handle.h
    @ingroup    mIIPPMD

    @brief

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _MD_ENG_HANDLE_H_
#define _MD_ENG_HANDLE_H_

#include "kdrv_type.h"
#include "md_eng_int_reg.h"

/*****************************************************************************/
typedef void (*MD_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;			/* linux struct clk*, not used in freertos */

	volatile NT98538_MDBC_REG_STRUCT *p_md_reg_st;
	volatile UINT8 *p_md_reg_chg_flag;

	uintptr_t reg_io_base;

	UINT32 irq_id;
	MD_ISR_CB isr_cb;
	ID     flg_id_md;
} MD_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	MD_ENG_HANDLE *p_eng;
} MD_ENG_CTL;

#endif //_MDBC_ENG_HANDLE_H_
