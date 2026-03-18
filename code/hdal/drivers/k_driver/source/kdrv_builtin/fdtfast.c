/* vi: set sw=4 ts=4: */
/*
 * Small lzma deflate implementation for embedded systems (https://github.com/j-d-r/unlzma_tiny)
 * Copyright (C) 2018  Julien Dusser <julien.dusser@free.fr>
 *
 * Based on decompress_unlzma.c from busybox
 * Copyright (C) 2006  Aurelien Jacobs <aurel@gnuage.org>
 *
 * Based on LzmaDecode.c from the LZMA SDK 4.22 (http://www.7-zip.org/)
 * Copyright (C) 1999-2005  Igor Pavlov
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#if defined(__KERNEL__)
#include <asm/io.h>
#include <asm/div64.h>
#include <asm/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include "bridge.h"
#else
#include <stdio.h>
#include <compiler.h>
#include <plat/rtosfdt.h>
#endif
#include <libfdt.h>
#include <kwrap/nvt_type.h>
#include <kwrap/debug.h>
#include <fdtfast.h>

static unsigned char *mp_fdtfast = NULL;

#if defined(__KERNEL__)
const void *fdtfast_get_base(void)
{
	int fdt_len;
	unsigned char *p_fdt = (unsigned char *)kdrv_fdt_get_fdt();

	if (mp_fdtfast) {
		return mp_fdtfast;
	}

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return NULL;
	}

	fdt_len = ALIGN_CEIL(fdt_totalsize(p_fdt), 64);
	mp_fdtfast = (unsigned char *)(p_fdt + fdt_len);
	if (fdt_check_header(mp_fdtfast) != 0) {
		DBG_WRN("no fdtfast\n");
		return NULL;
	}
	return mp_fdtfast;
}
#endif
#include <kwrap/task.h>
#if defined(__FREERTOS)
const void *fdtfast_get_base(void)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	if (mp_fdtfast) {
		return mp_fdtfast;
	}

	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL. \n");
		return NULL;
	}

	int fdt_len = ALIGN_CEIL(fdt_totalsize(p_fdt), 64);
	unsigned char *mp_fdtfast = (unsigned char *)(p_fdt + fdt_len);
	if (fdt_check_header(mp_fdtfast) != 0) {
		DBG_WRN("no fdtfast\n");
		return NULL;
	}
	return mp_fdtfast;
}
#endif

#if defined(__KERNEL__)
EXPORT_SYMBOL(fdtfast_get_base);
#endif