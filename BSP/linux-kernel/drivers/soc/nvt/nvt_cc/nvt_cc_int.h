/**

    CC module internal header file

    CC module internal header file

    @file       cc_int.h
    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _CC_INT_H
#define _CC_INT_H

#ifdef CONFIG_NVT_IVOT_PLAT_NS02201
#define CC_SUPPORT_7_CORES
#endif


#define CC_UNIT_TEST                    1

extern void     nvt_cc_config(CC_DEVICE  *cc_device);

extern void     nvt_cc_isr(void);

extern void     nvt_cc_ist(void);

extern CC_ER    nvt_cc_open(void);

extern CC_ER    nvt_cc_close(void);

#endif
