#if defined(__LINUX)
#include <linux/slab.h>
#elif defined(__FREERTOS)
#include <malloc.h>
#endif
#include "kwrap/type.h"

#include "pre_eng_int_platform.h"


void *kdrv_pre_os_malloc_wrap(UINT32 want_size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(want_size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(want_size);
#endif

	return p_buf;
}

void kdrv_pre_os_mfree_wrap(void *p_buf)
{
#if defined(__LINUX)
	kfree(p_buf);
#elif defined(__FREERTOS)
	free(p_buf);
#endif
}

UINT32 kdrv_pre_do_div(UINT64 n, UINT64 base)
{
	UINT32 rt=0;
#if defined(__LINUX)
	do_div(n, base);
#elif defined(__FREERTOS)
	n = n / base;
#endif

	rt = (UINT32) n;

	return rt;
}

void kdrv_pre_eng_setreg(uintptr_t ofs, UINT32 value)
{
#if defined (__LINUX)
	iowrite32(value, (void*)(ofs));
#elif defined (__FREERTOS)
	OUTW((ofs), value);
#endif
}

UINT32 kdrv_pre_eng_getreg(uintptr_t ofs)
{
#if defined (__LINUX)
	return ioread32((void*)(ofs));
#elif defined (__FREERTOS)
	return INW(ofs);
#endif
}
