#ifndef _AI_LIB_H_
#define _AI_LIB_H_

#ifdef __KERNEL__

//#include "mach/fmem.h"
//#include <mach/rcw_macro.h>
#include <linux/string.h>

#include <linux/clk.h>
#include <linux/hrtimer.h>

#else


#endif

#include <kwrap/semaphore.h>
//#include "kwrap/flag.h"
#include "kdrv_ai.h"

#define KDRV_AI_HANDLE_MAX_NUM (1)

#define KDRV_AI_LOCK            FLGPTN_BIT(0)  //(ofs bit << id)
//#define KDRV_AI_FMD             FLGPTN_BIT(8)
#define KDRV_AI_CNN_FMD         FLGPTN_BIT(9)
#define KDRV_AI_NUE_FMD         FLGPTN_BIT(10)
#define KDRV_AI_TIMEOUT         FLGPTN_BIT(12)
#define KDRV_AI_HDL_UNLOCK      FLGPTN_BIT(13)
#define KDRV_AI_RESET         	FLGPTN_BIT(14)
#define KDRV_AI_NUE2_FMD        FLGPTN_BIT(15)
#define KDRV_AI_CNN2_FMD        FLGPTN_BIT(16)
#define KDRV_AI_LOCK_BITS       ((UINT32)0x000000ff)
#define KDRV_AI_INIT_BITS       (KDRV_AI_LOCK_BITS | KDRV_AI_HDL_UNLOCK)

#if defined(__FREERTOS)
extern ID     	  FLG_ID_KDRV_AI[KDRV_AI_ENG_TOTAL];
#else
extern FLGPTN     FLG_ID_KDRV_AI[KDRV_AI_ENG_TOTAL];
#endif

extern SEM_HANDLE SEMID_KDRV_AI[KDRV_AI_ENG_TOTAL];

typedef ER(*KDRV_AI_SET_FP)(UINT32, void *);
typedef ER(*KDRV_AI_GET_FP)(UINT32, void *);


extern BOOL g_ai_isr_trig;
/**
    AI KDRV handle structure
*/
#define KDRV_AI_HANDLE_LOCK    0x00000001

typedef struct _KDRV_AI_HANDLE {
	UINT32 entry_id;
	UINT32 flag_id;
	UINT32 lock_bit;
	UINT32 sts;
	KDRV_AI_ISRCB isrcb_fp;
} KDRV_AI_HANDLE;

/**
    AI KDRV structure
*/
typedef struct {
	//KDRV_AI_TRIG_MODE mode;
	//UINT32 app_parm_cnt;
	//KDRV_AI_APP_HEAD app_parm;
	UINT32 ll_parm_cnt;
	//KDRV_AI_LL_HEAD ll_parm;
} KDRV_AI_PRAM, *pKDRV_AI_PRAM;

#if defined(__FREERTOS)
UINT32 kdrv_ai_drv_get_clock_freq(UINT8 clk_idx);
#endif

VOID kdrv_ai_set_init_status(UINT32 status);
UINT32 kdrv_ai_get_init_status(VOID);
VOID kdrv_ai_set_conv_clk_rate(UINT32 clk_rate);
UINT32 kdrv_ai_get_conv_clk_rate(VOID);

#endif //_KDRV_AI_ID_H
