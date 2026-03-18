/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_SPARSEMEM_H
#define __ASM_SPARSEMEM_H

#ifdef CONFIG_SPARSEMEM
#define MAX_PHYSMEM_BITS	CONFIG_ARM64_PA_BITS
#if (defined(CONFIG_MEMORY_HOTPLUG_SPARSE) && defined(CONFIG_PLAT_NOVATEK))
#define SECTION_SIZE_BITS	22
#else
#define SECTION_SIZE_BITS	30
#endif
#endif

#endif
