#ifndef _DRE_ENG_INT_H
#define _DRE_ENG_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
#include "kwrap/type.h"
//#include <mach/rcw_macro.h>
#include "linux/soc/nvt/rcw_macro.h"
//#include "kwrap/error_no.h"
#elif defined(__FREERTOS)
#include "kwrap/type.h"
#include "rcw_macro.h"
#include "kwrap/error_no.h"
#else
#endif

#include "dre_eng_int_reg.h"//header for DRE: reg struct

#define DRE_DMA_CACHE_HANDLE    (1)
#define DRE_DRV_REL_EN          0
#define DRE_PRINT_DBG_MSG_EN    1
#define DRE_SAVE_DBG_DATA_EN    0

//#define LAYER_MAX_NUM       PROC_MAX_NUM
#define DRE_CLK_VAL_192     192
#define DRE_CLK_VAL_240     240
#define DRE_CLK_VAL_480     480 //SC
#define DRE_CLK_VAL_PLL6    300
#define DRE_CLK_VAL_PLL13   250

#define REG_MAX(bit)        ((1<<(bit))-1)
#define CEIL(a, n)          ((((a)+((1<<(n))-1))>>(n))<<(n))
#define GET_OFST_FACTOR(isz, osz, ScOfst)    ((((isz-1)<<16)-ScOfst)/(osz-1))
#define HALF_SIZE(isz)      ((isz+1)>>1)

extern volatile NT98690_DRE_ENG_REG_STRUCT *p_int_dre_reg;
extern volatile NT98690_DRE_ENG_REG_STRUCT *p_dre_reg;
extern volatile UINT8 *p_dre_reg_chg_flag;


/*
    Layer number ID
*/
typedef enum {
	L0 = 0,
	L1 = 1,
	L2 = 2,
	L3 = 3,
	L4 = 4,
	L5 = 5,
	L6 = 6,
	L7 = 7,
	L8 = 8,
	L9 = 9
} DRE_LAYER_ID;

#ifdef __cplusplus
}
#endif

#endif//_DRE_INT_H
