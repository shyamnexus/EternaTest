/**
    NVT utilities for command customization

    @file       nvt_ivot_soc_utils.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_IVOT_SOC_UTILS_H__
#define __NVT_IVOT_SOC_UTILS_H__

int nvt_boot_rtos_bin_auto(void);
int nvt_runfw_bin_chk_valid(ulong addr);
int gz_uncompress(unsigned char *in, unsigned char *out, unsigned int insize, unsigned int outsize);
UINT32 MemCheck_CalcCheckSum16Bit(ulong uiAddr,UINT32 uiLen);

#endif /* __NVT_IVOT_SOC_UTILS_H__ */
