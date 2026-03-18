/**
    To setup for uncompress debug uart function
 
    @file       uncompress.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <plat/hardware.h>

#define SERIAL_THR	0x00
#define SERIAL_LSR	0x14
#define SERIAL_LSR_THRE	0x20

#define readl(a)	(*(volatile unsigned int *)(a))
#define writel(v,a)	(*(volatile unsigned int *)(a) = (v))

/*
 * This does not append a newline
 */
static inline void putc(int c)
{
	unsigned long base = NVT_UART1_BASE_PHYS;

	while ((readl(base + SERIAL_LSR) & SERIAL_LSR_THRE) == 0)
		barrier();

	writel(c, base + SERIAL_THR);
}

static inline void flush(void)
{
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
