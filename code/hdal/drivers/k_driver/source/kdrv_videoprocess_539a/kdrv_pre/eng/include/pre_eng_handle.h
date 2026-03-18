#ifndef _PRE_ENG_HANDLE_H_
#define _PRE_ENG_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __KERNEL__
#include "kwrap/type.h"
//#include <mach/rcw_macro.h> tmp change
//#include "linux/soc/nvt/rcw_macro.h"
//#include "kwrap/error_no.h"
#elif defined(__FREERTOS)
#include "kwrap/type.h"
//#include "rcw_macro.h"
//#include "kwrap/error_no.h"
#else
#endif

#include "pre_eng_int_reg.h"

typedef void (*PRE_ISR_CB)(void *eng, UINT32 status, void *reserve);

typedef struct {
	UINT32 chip_id;
	UINT32 eng_id;
	CHAR name[16];

	UINT32 clock_rate;
	void *pclk;         /* linux struct clk*, not used in freertos */

	volatile NT98539A_PRE_REGISTER_STRUCT *p_pre_reg_st;
	volatile UINT8 *p_pre_reg_chg_flag;

	uintptr_t reg_io_base;
	UINT32 irq_id;

	PRE_ISR_CB isr_cb;
} PRE_ENG_HANDLE;

typedef struct {
	UINT32 chip_num;
	UINT32 eng_num;
	UINT32 total_ch;
	PRE_ENG_HANDLE *p_eng;
} PRE_ENG_CTL;



#endif
