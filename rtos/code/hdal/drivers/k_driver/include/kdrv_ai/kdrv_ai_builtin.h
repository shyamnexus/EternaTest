#ifndef _KDRV_AI_BUILTIN_H_
#define _KDRV_AI_BUILTIN_H_

#include "kwrap/platform.h"
#include "kdrv_type.h"

#define KDRV_AI_MINI_FOR_FASTBOOT    1

#if (KDRV_AI_MINI_FOR_FASTBOOT == 2 || KDRV_AI_MINI_FOR_FASTBOOT == 1)
#define KDRV_AI_MINI_NO_LOCK         0  //0:semaphore enable
#else
#define KDRV_AI_MINI_NO_LOCK         0  //0:semaphore enable
#endif


extern INT32 kdrv_ai_builtin_trigger(UINT32 a, VOID *b, VOID *c, VOID *d);
extern void kdrv_ai_builtin_config_flow(UINT32 a);
extern INT32 kdrv_ai_builtin_set(UINT32 a, UINT32 b, void *c);
extern INT32 kdrv_ai_builtin_open(UINT32 a, UINT32 b);
extern INT32 kdrv_ai_builtin_close(UINT32 a, UINT32 b);
#endif
