/**
    NVT TrustZone Protection Controller
    We define the nvt 51055 based Protection Controller register here.
    @file           nvt_tzpc.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the SPDX-License-Identifier: BSD-2-Clause as
    published by the Free Software Foundation.
*/

#include <assert.h>
#include <plat/nvt_tzpc.h>
#include <io.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <stddef.h>
#include <trace.h>
#include <util.h>
#include <kwrap/debug.h>

#define TZPC_LOG    0

#if (TZPC_LOG)
#define _G_LOG(fmt, args...)            DBG_DUMP(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#else
#define _G_LOG(fmt, args...)
#endif

static     void nvt_tzpc_write_apb_region_attr(vaddr_t base, uint32_t region, uint32_t attr);
static uint32_t nvt_tzpc_read_apb_region_attr(vaddr_t base, uint32_t region);


void nvt_tzpc_config_access_right(CPU_TZPC_MEMORY_MAP eng, CPU_TZPC_ACS_RIGHT acs_right)
{
	uint32_t    address_ofs, address_bit;
	uint32_t    reg;
#if (TZPC_LOG)
	uint32_t    tzpc_address, module_address = 0;
#endif

	if (eng >= CPU_TZPC_PARTIAL_START) {
		address_ofs = 9;
		address_bit = eng - CPU_TZPC_PARTIAL_START;
	} else {
		address_ofs = eng;
		address_bit = address_ofs % 32;
		address_ofs = (address_ofs / 32);
#if (TZPC_LOG)
		module_address = (IOADDR_BASE | (eng << 16));
#endif
	}

	if (cpu_mmu_enabled()) {
		reg = nvt_tzpc_read_apb_region_attr((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100), address_ofs);
#if (TZPC_LOG)
		tzpc_address = (vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100);
#endif
	} else {
		reg = nvt_tzpc_read_apb_region_attr((paddr_t)NVT_TZPC_REG_BASE, address_ofs);
#if (TZPC_LOG)
		tzpc_address = (paddr_t)NVT_TZPC_REG_BASE;
#endif
	}

	_G_LOG("engine[0x%08x], tzpc addr[0x%08x][0x%02x] bit[%d]\r\n", module_address, tzpc_address, address_ofs, address_bit);

	_G_LOG("reg original = [0x%08x] write [0x%08x]\r\n", reg, (reg | (acs_right << address_bit)));

	reg |= (acs_right << address_bit);

	if (cpu_mmu_enabled()) {
		nvt_tzpc_write_apb_region_attr((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100), address_ofs, reg);
	} else {
		nvt_tzpc_write_apb_region_attr((paddr_t)NVT_TZPC_REG_BASE, address_ofs, reg);
	}
#if (TZPC_LOG)
	reg = nvt_tzpc_read_apb_region_attr((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100), address_ofs);
	_G_LOG("engine[0x%08x], tzpc addr[0x%08x] = [0x%08x]\r\n", (int)module_address, (int)((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC) + address_ofs * 4), (int)reg);
#endif
}

static void nvt_tzpc_write_apb_region_attr(vaddr_t base, uint32_t region, uint32_t attr)
{
	return io_write32(base + NVT_APB_ACS_REGION_TO_OFFSET(region), attr);
}

static uint32_t nvt_tzpc_read_apb_region_attr(vaddr_t base, uint32_t region)
{
	return io_read32(base + NVT_APB_ACS_REGION_TO_OFFSET(region));
}

void nvt_tzpc_init(void)
{
	uint32_t n;

	for (n = 0; n <= NVT_APB_ACS_RIGHT_REGION_MAX; n++) {
		/* Enable all apb bus access right in normal and secure world */
		if (cpu_mmu_enabled()) {
			nvt_tzpc_write_apb_region_attr((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100), n, 0);
		} else {
			nvt_tzpc_write_apb_region_attr((paddr_t)NVT_TZPC_REG_BASE, n, 0);
		}
	}
	//nvt_tzpc_config_access_right(CPU_TZPC_SCE_CTRL, CPU_TZPC_ACS_RIGHT_SEC_ONLY);
	//nvt_tzpc_config_access_right(CPU_TZPC_EFUSE_CTRL, CPU_TZPC_ACS_RIGHT_SEC_ONLY);
	DMSG("init successfully\n");
}

#if TRACE_LEVEL >= TRACE_DEBUG
void nvt_tzpc_dump(void)
{
	uint32_t n;
	uint32_t temp_32reg;

	DMSG("enter");
	for (n = 0; n <= NVT_APB_ACS_RIGHT_REGION_MAX; n++) {
		DMSG("");
		DMSG("APB region %d", n);
		temp_32reg = nvt_tzpc_read_apb_region_attr((vaddr_t)phys_to_virt(NVT_TZPC_REG_BASE, MEM_AREA_IO_SEC, 0x100), n);
		DMSG("attr: 0x%08x", temp_32reg);
	}
	DMSG("exit");

	nvt_tzpc_config_access_right(CPU_TZPC_SCE_CTRL, CPU_TZPC_ACS_RIGHT_SEC_ONLY);
}

#endif /* CFG_TRACE_LEVEL >= TRACE_DEBUG */
