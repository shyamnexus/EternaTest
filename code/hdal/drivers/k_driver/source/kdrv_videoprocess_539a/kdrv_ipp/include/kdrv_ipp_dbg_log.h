#include "comm/util/log.h"
extern unsigned int kdrv_ipp_debug_level;


//dbg level
typedef enum  {
	KDRV_IPP_DBG_LV_NONE =  0,
	KDRV_IPP_DBG_LV_ERR  =  1,
	KDRV_IPP_DBG_LV_WRN  =  2,
	ENUM_DUMMY4WORD(KDRV_IPP_DBG_LV)
} KDRV_IPP_DBG_LV;

#if defined(__KERNEL__)
#define KDRV_IPP_LOG_BUFFER (1)
#else
#define KDRV_IPP_LOG_BUFFER (0)
#endif

#if defined(__LINUX)
#define KDRV_IPP_INT_ROOT_PATH "//mnt//sd//"
#define KDRV_IPP_INT_PATH_SPLT "//"
#elif defined(__FREERTOS)
#define KDRV_IPP_INT_ROOT_PATH "A:\\"
#define KDRV_IPP_INT_PATH_SPLT "\\"
#endif

#if KDRV_IPP_LOG_BUFFER
#define KDRV_IPP_ERR(fmt, args...)	\
	do{	\
    	printm2("[kdrv_ipp]ERR: "   fmt , ##args);	\
		if (kdrv_ipp_debug_level >= KDRV_IPP_DBG_LV_ERR) { DBG_ERR(fmt, ##args);}	\
	}while(0)
#define KDRV_IPP_WRN(fmt, args...) \
	do{	\
    	printm2("[kdrv_ipp]WRN: "   fmt , ##args);    \
		if (kdrv_ipp_debug_level >= KDRV_IPP_DBG_LV_WRN) { DBG_WRN(fmt, ##args);}}	\
	}while(0)
#define KDRV_IPP_DUMP(fmt, args...) \
	do{	\
    	printm2("[kdrv_ipp]DUMP: "   fmt , ##args);    \
    	DBG_DUMP(fmt, ##args);	\
	}while(0)
#else
#define KDRV_IPP_ERR(fmt, args...) \
	do{ if (kdrv_ipp_debug_level >= KDRV_IPP_DBG_LV_ERR) { DBG_ERR(fmt, ##args);}}while(0)
#define KDRV_IPP_WRN(fmt, args...) \
	do{ if (kdrv_ipp_debug_level >= KDRV_IPP_DBG_LV_WRN) { DBG_WRN(fmt, ##args);}}while(0)
#define KDRV_IPP_DUMP(fmt, args...) \
    do{DBG_DUMP(fmt, ##args);}while(0)
#endif