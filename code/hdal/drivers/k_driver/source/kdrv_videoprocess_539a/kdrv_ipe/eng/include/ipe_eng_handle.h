#ifndef __IPE_ENG_HANDLE_H_
#define __IPE_ENG_HANDLE_H_



#include "kdrv_type.h"
#include "ipe_eng_int_reg.h"


typedef void (*IPE_ISR_CB)(void *eng, UINT32 status, void *reserve);


typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;         // linux struct clk*, not used in freertos 

	ULONG reg_io_base;
	UINT32 irq_id;

	IPE_ISR_CB isr_cb;

    volatile NT98538_IPE_REGISTER_STRUCT *p_ipe_reg_st;
    volatile UINT8 *p_ipe_reg_chg_flag;

	
} IPE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	IPE_ENG_HANDLE *p_eng;
} IPE_ENG_CTL;



#endif
