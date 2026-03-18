#ifndef __VPE_ENG_INT_DBG_H_
#define __VPE_ENG_INT_DBG_H_

#ifdef __cplusplus
extern "C" {
#endif



#if defined(__LINUX)
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


#define VPE_DBG_FREQ_MSG_EN (1)

#if (VPE_DBG_FREQ_MSG_EN == 1)
#define VPE_DBG_ERR_FREQ(cnt,args...) \
	do{ \
		static UINT8 i=0; \
		if(i == 0){ DBG_ERR(args); if(cnt!=0)i++; } \
		else if(i == cnt) i = 0; \
		else i++; \
	}while(0)

#define VPE_DBG_WRN_FREQ(cnt,args...) \
	do{ \
		static UINT8 i=0; \
		if(i == 0){ DBG_WRN(args); if(cnt!=0)i++; } \
		else if(i == cnt) i = 0; \
		else i++; \
	}while(0)
#else
#define VPE_DBG_ERR_FREQ(cnt,args...) do{}while(0)
#define VPE_DBG_WRN_FREQ(cnt,args...) do{}while(0)
#endif




#ifdef __cplusplus
}
#endif


#endif

