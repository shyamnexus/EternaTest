/**
    To setup sparse memory section size
    @file      memory.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __ASM_ARCH_NVT_IVOT_MEMORY_H
#define __ASM_ARCH_NVT_IVOT_MEMORY_H

#if (defined(CONFIG_MEMORY_HOTPLUG_SPARSE) && defined(CONFIG_PLAT_NOVATEK))
#define MAX_PHYSMEM_BITS	31
#define SECTION_SIZE_BITS	23
#else
#define MAX_PHYSMEM_BITS	32
#define SECTION_SIZE_BITS	24
#endif

#endif /* __ASM_ARCH_NVT_IVOT_MEMORY_H */
