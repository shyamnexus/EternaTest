/*
	Internal header file for THERMAL

	This file is the internal header file that defines the variables,
	structures and function prototypes of RTC module.

	@file       thermal_int.h

	Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _THERMAL_INT_H
#define _THERMAL_INT_H

#if IS_BUILTIN(CONFIG_NVTIVOT_THERMAL)
extern int nvt_thermal_get_temp(int *temp);
#else
#define nvt_thermal_get_temp(temp)
#endif

#endif
