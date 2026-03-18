/**
    NVT PCIe Library Shared Block
    To handle the flush allocate and other memory handling api header file
    @file      nvt-pcie-lib-shmblk.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_PCIE_LIB_SHMBLK_H__
#define __NVT_PCIE_LIB_SHMBLK_H__
#include <linux/types.h>

#define SHMBLK_NAME_SIZE    32
#define SHMBLK_DATA_SIZE    84
#define SHMBLK_NUM          32

typedef struct {
	unsigned int head_tag;
	unsigned int size;
	char name[SHMBLK_NAME_SIZE];
	unsigned char buf[SHMBLK_DATA_SIZE];
	unsigned int tail_tag;
} nvtpcie_shmblk_t; //aligned ?

#endif
