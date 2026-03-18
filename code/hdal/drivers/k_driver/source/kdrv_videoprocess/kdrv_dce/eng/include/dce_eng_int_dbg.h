#ifndef _DCE_ENG_INT_DBG_H_
#define _DCE_ENG_INT_DBG_H_

#define __MODULE__          dce_eng
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass

#if defined(__LINUX)
#include "linux/printk.h"
#include "kwrap/debug.h"

#elif defined(__FREERTOS)
#include <stdio.h>
#include <kwrap/debug.h>


#ifndef unlikely
#define unlikely(x) (x)
#endif

#else
//#error Not supported OS
#endif

#if defined (_NVT_EMULATION_)
#define DCE_PRINT_FREQ 0    //always print
#else
#define DCE_PRINT_FREQ 120
#endif


#define DCE_DBG_FREQ_MSG_EN (1)

#if (DCE_DBG_FREQ_MSG_EN == 1)
    #define DCE_DBG_ERR_FREQ(cnt,args...) \
    do{ \
        static UINT8 dce_i=0; \
        if(dce_i == 0){ DBG_ERR(args); if(cnt!=0)dce_i++; } \
        else if(dce_i == cnt) dce_i = 0; \
        else dce_i++; \
    }while(0)

    #define DCE_DBG_WRN_FREQ(cnt,args...) \
    do{ \
        static UINT8 dce_i=0; \
        if(dce_i == 0){ DBG_WRN(args); if(cnt!=0)dce_i++; } \
        else if(dce_i == cnt) dce_i = 0; \
        else dce_i++; \
    }while(0)
#else
        #define DCE_DBG_ERR_FREQ(cnt,args...) do{}while(0)
        #define DCE_DBG_WRN_FREQ(cnt,args...) do{}while(0)
#endif

extern unsigned int dce_eng_debug_level;

#endif //_DCE_ENG_INT_DBG_H_

