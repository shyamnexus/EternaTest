/**

    The header file of dmacopy ioctl operations.

    @file       kflow_copy.h
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#ifndef __KFLOW_TSE_H
#define __KFLOW_TSE_H

#include "kwrap/type.h"
#include "kwrap/ioctl.h"

#define KFLOW_TSE_IOC_MAGIC 'T'

typedef struct {
	uintptr_t       p_src;                                 ///< [in] source buffer's physical address
	uintptr_t       p_dst;                                 ///< [in] destination buffer's physical address
	UINT32          size;                                  ///< [in] how many bytes to copy
} TSE_IOC_HWCOPY_INFO_S;

#define KFLOW_TSE_IOC_HWCOPY  _VOS_IOWR(KFLOW_TSE_IOC_MAGIC, 0, TSE_IOC_HWCOPY_INFO_S)

#if defined(__FREERTOS)
int kflow_tse_ioctl (int fd, unsigned int cmd, void *p_arg);
#endif

#endif
