
#ifndef _VQA_CLRCST_LIB_H
#define _VQA_CLRCST_LIB_H


#define NVT_VQA_major     01
#define NVT_VQA_minor     00
#define NVT_VQA_bugfix    230531
#define NVT_VQA_ext       0

#define _VQA_STR(a,b,c)          #a"."#b"."#c
#define VQA_STR(a,b,c)           _VQA_STR(a,b,c)
#define LIB_VQA_IMPL_VERSION    VQA_STR(NVT_VQA_major, NVT_VQA_minor, NVT_VQA_bugfix)//implementation version major.minor.yymmdds


#include <stdio.h>
#include <time.h>
#include <math.h>
#include <vqa_clrcst_alg.h>

#define MAX_CA_DATA_NUM 30
#define VQA_MAX_CH_NUM		1

#define CLRCST_W_WINNUM 	32
#define CLRCST_MAX_WINNUM 	CLRCST_W_WINNUM*CLRCST_W_WINNUM

#if defined(__LINUX)
#include <sys/mman.h>
#include <pthread.h>
#endif
#if defined(__FREERTOS)
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h>
#endif

#define CLRCST_HIS_SIZE (360+1)

//#define CLRCST_HSV_BUF_SIZE (CLRCST_MAX_WINNUM+CLRCST_HIS_SIZE)*4

#include "hd_type.h"

#ifndef UINTPTR
typedef uintptr_t                       UINTPTR;
#endif

/**
    Color-Cast parameter
    Structure of parameter of Color-Cast
*/

typedef struct _CLRCST_PARM_S {
    UINT32 cl_collect_time;
    UINT32 cl_warning_max;
    UINT32 cl_diff_th;
    UINT32 cl_diff_area;
    UINT32 cl_mode;
    UINT32 cl_background_update;
    UINT32 cl_is_debug;

    UINT32 cl_max_yuv_width;
    UINT32 cl_max_yuv_height;
    UINT32 sensor_ch;
    UINT32 cl_src_type;
    UINTPTR cl_buf_va_addr;
    UINTPTR cl_buf_pa_addr;

} CLRCST_PARM_S;


typedef enum {
    CL_NORMAL_S          = 0,    ///< normal
    CL_ALARM_S           = 1,    ///< abnormal
    CL_ERROR_S           = 2,    ///< error
} CLRCST_STATUS_S;

UINT32	 NVT_VQA_CLRCST_Calc_buf_size(UINT32 width, UINT32 height);
VOID	 NVT_VQA_Set_debug(UINT32 is_debug);
UINT32	 NVT_VQA_Get_version(UINT32 ver);
INT	 NVT_VQA_CLRCST_Init(CLRCST_PARM_S *cl_parm);
UINT32	 NVT_VQA_CLRCST_Detect(UINTPTR va_addr, UINT32 yuv_width, UINT32 yuv_height);
VOID	 NVT_VQA_CLRCST_Uninit(CLRCST_PARM_S *cl_parm);

#endif
