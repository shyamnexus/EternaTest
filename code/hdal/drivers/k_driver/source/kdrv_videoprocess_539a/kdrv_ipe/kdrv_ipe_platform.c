#if defined(__LINUX)
#include <linux/slab.h>
#elif defined(__FREERTOS)
#include <malloc.h>
#endif
#include "kwrap/type.h"

#include "ipe_eng_int_platform.h"

void *kdrv_ipe_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void kdrv_ipe_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

void kdrv_ipe_eng_setreg(uintptr_t ofs, UINT32 value)
{
#if defined (__LINUX)
	iowrite32(value, (void*)(ofs));
#elif defined (__FREERTOS)
	OUTW((ofs), value);
#endif
}

UINT32 kdrv_ipe_eng_getreg(uintptr_t ofs)
{
#if defined (__LINUX)
	return ioread32((void*)(ofs));
#elif defined (__FREERTOS)
	return INW(ofs);
#endif
}