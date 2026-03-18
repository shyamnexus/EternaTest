/*
	Register definition header file for HRtimer module.

	This file is the header file that define the address offset
	definition of registers of HRtimer module.

	@file       hrtimer-reg.h

	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _HRTIMER_REG_H
#define _HRTIMER_REG_H

#include <linux/bitfield.h>
#include <linux/bits.h>

#define DISABLE 0x0
#define ENABLE 0x1
#define CLEAN 0x1

#define TMR_EN BIT(0)
#define TMR_MODE BIT(1)
#define TMR_INIT_COUNT_INIT BIT(2)

#define TMR_FREE_RUN BIT(1)
#define TMR_ONE_SHOT ~TMR_FREE_RUN

#define TMR_MAX_HZ 120000000
#define TMR_MAX_VALUE GENMASK_ULL(63,0)
#define TMR_MAX_DIV_BIT GENMASK_ULL(7,0)

#define TMR_STS_CPU 0x0
#define TMR_INTE_CPU 0x4
#define TMR_CTRL 0x8
#define TMR_TVAL_L 0xC
#define TMR_TVAL_H 0x10
#define TMR_CNT_L 0x34
#define TMR_CNT_H 0x38
#define TMR_RLD 0x1C
#define TMR_CLK_DIV 0x20
#define TMR_INIT_TVAL_L 0x24
#define TMR_INIT_TVAL_H 0x28
#define TMR_LATCH_APB 0x30
#define TMR_LATCH_STS 0x0
#define TMR_LATCH_STS_ERRCNT 0x4000
#define TMR_LATCH_STS_CLEAN BIT(1)

#endif /* _HRTIMER_REG_H */
