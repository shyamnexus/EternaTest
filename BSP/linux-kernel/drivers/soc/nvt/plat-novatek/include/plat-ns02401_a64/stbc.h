/*
    Novatek protected header file of NS02302 driver.

    The header file for Novatek protected APIs of NS02302's driver.

    @file       stbc.h
    @ingroup    mIDriver
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _NVT_STBC_PUBLIC_H
#define _NVT_STBC_PUBLIC_H

#include <linux/soc/nvt/nvt_type.h>

/*
     stbc_version

     stbc_version
*/
extern void stbc_version(void);

/*
     stbc_pmc_start

     stbc_pmc_start : start stbc operation
*/
extern void stbc_pmc_start(UINT32 dbg_en);

#endif

