
#ifndef _NVT_SMC_UTIL_H
#define _NVT_SMC_UTIL_H
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))   ///<
#include <types_ext.h>
uintptr_t nvt_smc_pa_to_va(uintptr_t pa, uintptr_t size);

#endif // _NVT_SMC_UTIL_H
