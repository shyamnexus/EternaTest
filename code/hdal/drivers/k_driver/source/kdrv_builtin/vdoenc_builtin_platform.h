#ifndef __VDOENC_BUILTIN_PLATFORM_H
#define __VDOENC_BUILTIN_PLATFORM_H
#include <kwrap/type.h>
#include "kwrap/debug.h"
#include <kwrap/error_no.h>
#include "kdrv_builtin_debug.h" //must behind include kwarp/debug.h

void* vdoenc_builtin_alloc(int size);
void vdoenc_builtin_free(void *buf);
ULONG vdoenc_builtin_ioremap(ULONG addr, UINT32 size);
void vdoenc_builtin_iounmap(void *addr);
int vdoenc_plat_get_nodeoffset(CHAR *node, int *nodeoffset);
void vdoenc_plat_get_nodeoffset_list(int nodeoffset, int *nodeoffset_list);
INT32 vdoenc_plat_read_dtsi(int nodeoffset, CHAR *tag, UINT32 *value);

#endif
