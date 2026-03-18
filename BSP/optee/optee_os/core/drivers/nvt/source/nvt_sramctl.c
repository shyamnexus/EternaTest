/**
    nvt-opt key manager
    This file will Enable and disable SRAM shutdown
    @file       nvt-otp.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <plat/nvt-sramctl.h>
#include "trace.h"
#include "io.h"
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <efuse_protected.h>
#include <plat/io_address.h>
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include <kwrap/error_no.h>
#include <initcall.h>

static vaddr_t sram_ctl_base_mmu;
static vaddr_t sram_ctl_base_nommu;

static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)

#ifndef CHKPNT
#define CHKPNT    EMSG("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   EMSG("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   EMSG("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) EMSG(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  EMSG("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  EMSG("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

static void SRAMCTL_SETREG(uint32_t ofs, uint32_t value)
{
	if (cpu_mmu_enabled()) {
		return io_write32(sram_ctl_base_mmu + ofs, value);
	} else {
		return io_write32(sram_ctl_base_nommu + ofs, value);
	}
}

static uint32_t SRAMCTL_GETREG(uint32_t ofs)
{
	if (cpu_mmu_enabled()) {
		return io_read32(sram_ctl_base_mmu + ofs);
	} else {
		return io_read32(sram_ctl_base_nommu + ofs);
	}
}

#define SRAM_OFS 0x1000

void nvt_disable_sram_shutdown(SRAM_SD id)
{
	UINT32 reg_data, reg_ofs;
	unsigned long      flags;

	reg_ofs = (id >> 5) << 2;

	loc_multi_cores(flags);

	reg_data = SRAMCTL_GETREG(SRAM_OFS + reg_ofs);

	reg_data &= ~(1 << (id & 0x1F));

	SRAMCTL_SETREG(SRAM_OFS + reg_ofs, reg_data);

	unl_multi_cores(flags);
}

void nvt_enable_sram_shutdown(SRAM_SD id)
{
	UINT32 reg_data, reg_ofs;
	unsigned long      flags;

	reg_ofs = (id >> 5) << 2;

	loc_multi_cores(flags);

	reg_data = SRAMCTL_GETREG(SRAM_OFS + reg_ofs);

	reg_data |= (1 << (id & 0x1F));

	SRAMCTL_SETREG(SRAM_OFS + reg_ofs, reg_data);

	unl_multi_cores(flags);
}

//
//0xF0 Version Code Register
//
#define TOP_VERSION_REG_OFS                 0xF0
union TOP_VERSION_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int ROM_CODE_VERSION:4; // ROM Code Version
	unsigned int ECO_VERSION:4;      // ECO Version
	unsigned int CHIP_ID:16;         // Chip ID
	} bit;
};


UINT32 nvt_get_chip_id(void)
{
	union TOP_VERSION_REG top_version;

	top_version.reg = SRAMCTL_GETREG(TOP_VERSION_REG_OFS);
	return top_version.bit.CHIP_ID;
}

static TEE_Result nvt_sramctl_init(void)
{
	sram_ctl_base_mmu = (vaddr_t)phys_to_virt(IOADDR_TOP_REG_BASE, MEM_AREA_IO_SEC, 0x10000);
	sram_ctl_base_nommu = ((paddr_t)IOADDR_TOP_REG_BASE);

	DMSG("nvt_sramctl_init done\n");
	return TEE_SUCCESS;
}

service_init(nvt_sramctl_init);
