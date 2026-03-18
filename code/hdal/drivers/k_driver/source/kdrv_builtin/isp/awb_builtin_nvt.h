#ifndef _AWB_BUILTIN_NVT_H_
#define _AWB_BUILTIN_NVT_H_

#include "kwrap/type.h"
#include "isp_builtin_int.h"

//=============================================================================
// version
//=============================================================================
#define AWB_BUILTIN_VERSION 0x00100000

//=============================================================================
// struct & enum definition
//=============================================================================
#define AWB_DEF_KRGAIN             100
#define AWB_DEF_KBGAIN             100
#define AWB_W_THRES                32

//=============================================================================
// extern functions
//=============================================================================
extern INT32 awb_builtin_init_nvt(UINT32 id);
extern INT32 awb_builtin_uninit_nvt(UINT32 id);
extern INT32 awb_builtin_trig_nvt(UINT32 id, ISP_BUILTIN_AWB_TRIG_MSG msg);
extern void awb_builtin_get_param(UINT32 id, ULONG *param);

#endif

