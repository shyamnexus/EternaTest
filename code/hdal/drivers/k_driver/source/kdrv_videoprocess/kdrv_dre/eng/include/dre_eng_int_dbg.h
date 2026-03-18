#ifndef _DRE_ENG_INT_DBG_H_
#define _DRE_ENG_INT_DBG_H_

#define __MODULE__          ssdrv_dre
#define __DBGLVL__          8 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
#define __DBGFLT__          "*" // *=All, [mark]=CustomClass

#if defined(__LINUX)
//#include "linux/printk.h"
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

extern unsigned int ssdrv_dre_debug_level;

#define DRE_DBG_FREQ_MSG_EN (1)

#if (DRE_DBG_FREQ_MSG_EN == 1)
    #define DRE_DBG_ERR_FREQ(cnt,args...) \
    do{ \
        static UINT8 i=0; \
        if(i == 0){ DBG_ERR(args); if(cnt!=0)i++; } \
        else if(i == cnt) i = 0; \
        else i++; \
    }while(0)

    #define DRE_DBG_WRN_FREQ(cnt,args...) \
    do{ \
        static UINT8 i=0; \
        if(i == 0){ DBG_WRN(args); if(cnt!=0)i++; } \
        else if(i == cnt) i = 0; \
        else i++; \
    }while(0)
#else
        #define DRE_DBG_ERR_FREQ(cnt,args...) do{}while(0)
        #define DRE_DBG_WRN_FREQ(cnt,args...) do{}while(0)
#endif



#endif //_DRE_ENG_INT_DBG_H_

