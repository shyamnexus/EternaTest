#ifndef __ISF_VDOENC_PLATFORM_H
#define __ISF_VDOENC_PLATFORM_H
#include <kwrap/type.h>

void* vdoenc_alloc(int size);
void vdoenc_free(void *buf);
void vdoenc_delay_m_sec(UINT32 m_sec);
UINT32 vdoenc_read_dtsi_value(CHAR *node, CHAR *tag);
INT32 vdoenc_read_dtsi_array(CHAR *node, CHAR *tag, UINT32 *array, UINT32 num);
BOOL vdoenc_copy_from_user(void *p_dest, void *p_src, UINT32 size);
#endif


