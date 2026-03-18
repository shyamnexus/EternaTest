#ifndef __VPE_ENG_HANDLE_H_
#define __VPE_ENG_HANDLE_H_
#include "vpe_eng_int_reg.h"
#include "vpe_eng_int_column_cal.h"
#include "kwrap/flag.h"

typedef void (*VPE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];
	UINT32 clock_rate;
	void *mclk;			/* linux struct clk*, not used in freertos */
	uintptr_t reg_io_base;
    volatile NT98538_VPE_REGISTER_STRUCT *p_vpe_reg_st;
    volatile UINT8 *p_vpe_reg_chg_flag;
	UINT32 irq_id;
	VPE_ISR_CB isr_cb;
	ID     flg_id_vpe;
	VPE_GLO_PARAM *vpe_glo_param;
} VPE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	VPE_ENG_HANDLE *p_eng;
} VPE_ENG_CTL;


#endif

