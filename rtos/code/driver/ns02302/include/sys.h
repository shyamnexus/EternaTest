/**
    Header file for SYS module.

    This file is the header file that define the API and data type
    for SYS module.

    @file       sys.h
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _SYS_H
#define _SYS_H

#include <kwrap/type.h>
#include <kwrap/nvt_type.h>
#include <kwrap/platform.h>
#include <comm/driver.h>

typedef union _sys_pa_ {
	uint64_t addr64;
	struct {
		unsigned int low:32;
		unsigned int high:32;
	} addr32;
} SYS_PA;

extern SYS_PA sys_va2pa(ULONG va);

#endif

