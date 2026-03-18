/**
    NVT TrustZone Protection Controller
    We define the nvt 51055 based Protection Controller register here.
    @file           nvt_51055_tzpc.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the SPDX-License-Identifier: BSD-2-Clause as
    published by the Free Software Foundation.
*/

#ifndef __DRIVERS_NVT_TZPC_H
#define __DRIVERS_NVT_TZPC_H

#include <stdint.h>
#include <trace_levels.h>
#include <types_ext.h>
#include <util.h>
#include <plat/io_address.h>

#define NVT_TZPC_REG_BASE       IOADDR_TZPC_REG_BASE
#define NVT_TZPC_REG_SIZE       0x30

/* Register offset */
#define AXI_MASTER_ACS_RIGHT    0x20
#define APB_SLAVE_NS_ACS_DIS    0x24
#define NVT_COUNT_CTRL          0x30

typedef enum _axi_master_acs_right {
	AXI_ETH_MASTER_ACS_RIGHT = 0,
	AXI_USB_MASTER_ACS_RIGHT = 1,
	AXI_MASTER_ACS_RIGHT_MAX = 2,
} axi_master_acs_right;

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
typedef enum _nvt_apb_pc_acs_right {
	NVT_APB_ACS_RIGHT_REGION_0 = 0,
	NVT_APB_ACS_RIGHT_REGION_1,
	NVT_APB_ACS_RIGHT_REGION_2,
	NVT_APB_ACS_RIGHT_REGION_3,
	NVT_APB_ACS_RIGHT_REGION_4,
	NVT_APB_ACS_RIGHT_REGION_5,
	NVT_APB_ACS_RIGHT_REGION_6,
	NVT_APB_ACS_RIGHT_REGION_7,
	NVT_APB_ACS_RIGHT_REGION_MAX = 7,
} nvt_apb_pc_acs_right;

#define NVT_APB_ACS_REGION_TO_OFFSET(n) (n * 0x4)

/**
     Trust zone protected control

     Each enum represent specific controller will allow or not allow non secure access
*/
typedef enum {
#if defined(_BSP_NA51103_)
	CPU_TZPC_SCE_CTRL       = 0x64,
	CPU_TZPC_EFUSE_CTRL     = 0x63,
	CPU_TZPC_RSA_CTRL       = 0x66,
	CPU_TZPC_HASH_CTRL      = 0x65,
	CPU_TZPC_TZPC_CTRL      = 0x05,
	CPU_TZPC_PARTIAL_START  = 0x100,
	CPU_TZPC_CLK_PARTIAL    = CPU_TZPC_PARTIAL_START,
	CPU_TZPC_TRNG_PARTIAL	= CPU_TZPC_PARTIAL_START + 2,
#elif defined(_BSP_NA51102_)
	CPU_TZPC_SCE_CTRL       = 0x62,
	CPU_TZPC_EFUSE_CTRL     = 0x66,
	CPU_TZPC_RSA_CTRL       = 0x6A,
	CPU_TZPC_HASH_CTRL      = 0x67,
	CPU_TZPC_TZPC_CTRL      = 0x0C,
	CPU_TZPC_PARTIAL_START  = 0x100,
	CPU_TZPC_CLK_PARTIAL    = CPU_TZPC_PARTIAL_START,
	CPU_TZPC_TRNG_PARTIAL	= CPU_TZPC_PARTIAL_START + 2,
#else
	CPU_TZPC_SCE_CTRL       = 0x95,
	CPU_TZPC_EFUSE_CTRL     = 0xA9,
	CPU_TZPC_RSA_CTRL       = 0xA8,
	CPU_TZPC_HASH_CTRL      = 0x96,
	CPU_TZPC_TZPC_CTRL      = 0x05,
	CPU_TZPC_PARTIAL_START  = 0x100,
	CPU_TZPC_CLK_PARTIAL    = CPU_TZPC_PARTIAL_START,
	CPU_TZPC_TRNG_PARTIAL,
#endif



	CPU_TZPC_MAX,

} CPU_TZPC_MEMORY_MAP;

/**
     Access right enumation

*/
typedef enum {
	CPU_TZPC_ACS_RIGHT_BOTH = 0x0,      ///< allow non secure & secure access
	CPU_TZPC_ACS_RIGHT_SEC_ONLY,        ///< allow secure access only

	CPU_TZPC_ACS_TYOE,

} CPU_TZPC_ACS_RIGHT;


void nvt_tzpc_init(void);
void nvt_tzpc_config_access_right(CPU_TZPC_MEMORY_MAP eng, CPU_TZPC_ACS_RIGHT acs_right);

void nvt_tzpc_apb_config_region(nvt_apb_pc_acs_right region, vaddr_t addr_base, uint32_t attr);
void nvt_tzpc_region_enable(nvt_apb_pc_acs_right region);

#if TRACE_LEVEL >= TRACE_DEBUG
void nvt_tzpc_dump(void);
#else
static inline void nvt_tzpc_dump(void)
{
}
#endif

#endif /* __DRIVERS_NVT_TZPC_H */
