/**
	@brief Header file of extend engine CPU.

	@file vendor_ai_cpu.h

	@ingroup vendor_ai_cpu

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _VENDOR_AI_CPU_H_
#define _VENDOR_AI_CPU_H_

#include "vendor_ai_plugin.h"
#include "vendor_ai_net/nn_net.h"

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
extern void* vendor_ai_cpu1_get_engine(void);

extern void vendor_ai_cpu_thread_setbind(UINT32 proc_id, int cpu_id);
extern int vendor_ai_cpu_thread_getbind(UINT32 proc_id);

extern HD_RESULT vendor_ai_cpu_thread_init(UINT32 proc_id);
extern HD_RESULT vendor_ai_cpu_thread_exit(UINT32 proc_id);
extern HD_RESULT vendor_ai_cpu_thread_reg_cb(VENDOR_AI_ENG_CB fp);
extern HD_RESULT vendor_ai_cpu_thread_init_task(VOID);
extern HD_RESULT vendor_ai_cpu_thread_uninit_task(VOID);

extern VOID* _vendor_ai_net_get_usr_info(UINT32 proc_id);

extern HD_RESULT vendor_ai_cpu_util_float2fixed(FLOAT *in_data, FLOAT in_scale_ratio, VOID *out_data, HD_VIDEO_PXLFMT out_fmt, INT32 data_size, INT32 zero_point);
extern HD_RESULT vendor_ai_cpu_util_fixed2float (VOID *in_data, HD_VIDEO_PXLFMT in_fmt, FLOAT *out_data, FLOAT out_scale_ratio, INT32 data_size, INT32 zero_point);

#ifdef __cplusplus
}
#endif
#endif  /* _VENDOR_AI_CPU_H_ */
