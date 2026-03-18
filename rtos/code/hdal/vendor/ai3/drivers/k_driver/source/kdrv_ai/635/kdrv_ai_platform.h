/**
	@brief Header file of internal definition of kdrv_ai.

	@file kdrv_ai_platform.h

	@ingroup kdrv_ai_platform

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KDRV_AI_PLATFORM_H_
#define _KDRV_AI_PLATFORM_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
uintptr_t kdrv_ai_pa2va_remap(uintptr_t pa, UINT32 sz);
uintptr_t kdrv_ai_pa2va_remap_wo_sync(uintptr_t pa, UINT32 sz);
VOID kdrv_ai_pa2va_unmap(uintptr_t va, uintptr_t pa);
void *kdrv_ai_alloc_mem(UINT32 size);
void kdrv_ai_free_mem(void *p_buf);

#endif  /* _KDRV_AI_PLATFORM_H_ */