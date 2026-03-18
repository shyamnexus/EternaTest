/**
    NVT TrustZone Protection Controller
    We define the nvt 51055 based Protection Controller register here.
    @file           nvt_tzpc.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the SPDX-License-Identifier: BSD-2-Clause as
    published by the Free Software Foundation.
*/

#include <string.h>
#include <assert.h>
#include <plat/nvt_tzasc.h>
#include <io.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <stddef.h>
#include <trace.h>
#include <util.h>
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include <kwrap/error_no.h>
#include "nvt_tzasc_int.h"
#include "nvt_tzasc_reg.h"

#define DRAM_PHYS_MAX_ADDRESS           0xFFFFFFFC
static  VK_DEFINE_SPINLOCK(my_lock);
static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs);
static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value);
void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set);
void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr);

#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)


#define PROT_START_OFS(ch)				(MAU_PROTECT0_START_ADDR + ch * 8)
#define PROT_END_OFS(ch)				(MAU_PROTECT0_END_ADDR + ch * 8)
#define PROT_EN0_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS0 + ch * 8)
#define PROT_EN1_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS1 + ch * 8)


#define _Y_LOG(fmt, args...)            printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)            printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)            printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)            printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)            printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)            printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)


#define ARB_SETREG(ofs,value)           arb_set_reg(DDR_ARB_1, (ofs), (value))
#define ARB_GETREG(ofs)                 arb_get_reg(DDR_ARB_1, (ofs))

#define PROT_SETREG(ofs,value)          arb_set_reg(DDR_ARB_1, 0x4300+(ofs),(value))
#define PROT_GETREG(ofs)                arb_get_reg(DDR_ARB_1, 0x4300+(ofs))

/*
//<<----
static void debug_dump_addr(UINT64 addr, UINT32 size)
{
    UINT32 i, j;

    for (j = addr; j < (addr + size); j += 0x10) {
        printf("0x%lx:", j);
        for (i = 0; i < 4; i++) {
            printf("%08x", readl(j + 4 * i));
            printf("  ");
        }
        printf("\r\n");
    }
}
*/



/*
 * Implementation defined values used to validate inputs later.
 * Filters : max of 6 ; 0 to 5
 * Regions : max of 4 ; 0 to 3
 * Address width : Values between 32 to 64
 */
struct tzc_instance {
	uint32_t addr_width;
	uint32_t num_filters;
	uint32_t num_regions;
};

static struct tzc_instance tzc;

static vaddr_t tzasc_base_addr(DDR_ARB ddr_id)
{
	static void *va[DDR_ARB_COUNT];

	if (cpu_mmu_enabled()) {
		if (!va[ddr_id]) {
			va[ddr_id] = phys_to_virt(IOADDR_PROT_REG_BASE, MEM_AREA_IO_SEC, 0x5000);
		}
		return (vaddr_t)va[ddr_id];
	} else {
		return IOADDR_PROT_REG_BASE;
	}
}


static vaddr_t arb_base_addr(DDR_ARB ddr_id)
{
	static void *va_arb[DDR_ARB_COUNT];

	if (cpu_mmu_enabled()) {
		if (!va_arb[ddr_id]) {
			va_arb[ddr_id] = phys_to_virt(IOADDR_DDR_ARB_REG_BASE, MEM_AREA_IO_SEC, 0x5000);
		}
		return (vaddr_t)va_arb[ddr_id];
	} else {
		return IOADDR_DDR_ARB_REG_BASE;
	}
}
static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value)
{
	io_write32((arb_base_addr(ddr_id) + ofs), value);
}

static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs)
{
	return io_read32((arb_base_addr(ddr_id) + ofs));

}

static UINT32 prot_get_reg(DDR_ARB ddr_id, uint32_t ofs)
{
	return io_read32((tzasc_base_addr(ddr_id) + ofs));
}

static void prot_set_reg(DDR_ARB ddr_id, uint32_t ofs, REGVALUE value)
{
	io_write32((tzasc_base_addr(ddr_id) + ofs), value);
}

#if 0
static paddr_t nvt_tzasc_virt_to_phys(vaddr_t addr)
{
	if (cpu_mmu_enabled()) {
		return virt_to_phys((void *)addr);
	} else {
		return addr;
	}
}
#endif

void tzc_enable_filters(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr)
{
	unsigned long lock = 0;
	UINT32 v_mask[DMA_CH_GROUP_CNT];

	T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	T_MAU_INT_STATUS_REG sts_reg;
	T_MAU_PROTECT_MODE_REG mode;

	if (id > DDR_ARB_1) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_4) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	if (p_attr->starting_addr & 0x7f) {
		DBG_ERR("starting addr not 32 word aligned 0x%08x\r\n", (unsigned int)p_attr->starting_addr);
		return;
	}
	mode.reg = prot_get_reg(id, MAU_PROTECT_MODE_OFS);
	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));

	loc_multi_cores(lock);

	// disable wp int
	intctrl_reg.reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
	intctrl_reg.reg &= ~0xffff;
	arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

	// clr status
	sts_reg.reg = 0xffff;
	arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.reg);

	// set addr
	if (set > WPSET_4) {
		prot_set_reg(id, MAU_OUT_PROTECT_START_ADDR, p_attr->starting_addr);
		prot_set_reg(id, MAU_OUT_PROTECT_END_ADDR, p_attr->starting_addr + p_attr->size - 1);
	} else {
		prot_set_reg(id, PROT_START_OFS(set), p_attr->starting_addr);
		prot_set_reg(id, PROT_END_OFS(set), p_attr->starting_addr + p_attr->size - 1);
	}

	if (set == WPSET_0) {
		mode.bit.WP0_MODE = p_attr->level;
	} else if (set == WPSET_1) {
		mode.bit.WP1_MODE = p_attr->level;
	} else if (set == WPSET_2) {
		mode.bit.WP2_MODE = p_attr->level;
	} else if (set == WPSET_3) {
		mode.bit.WP3_MODE = p_attr->level;
	} else if (set == WPSET_4) {
		mode.bit.WP4_MODE = p_attr->level;
	} else {
		mode.bit.OUT_WP_MODE = p_attr->level;
	}
	//VIE and IVE workaround patch
	if(v_mask[1] & 0x40)
	{
		v_mask[0] |= 0x40000000;
	}


	prot_set_reg(id, MAU_PROTECT_MODE_OFS, mode.reg);

	// set en
	prot_set_reg(id, PROT_EN0_OFS(set), v_mask[0]);
	prot_set_reg(id, PROT_EN1_OFS(set), v_mask[1]);

	// enable interrupt
//	intctrl_reg.reg |= 0xffff;
//	arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

	unl_multi_cores(lock);
}

void tzc_disable_filters(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	UINT32 lock = 0;
	T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	T_MAU_INT_STATUS_REG sts_reg;

	if (id > DDR_ARB_1) {
            DBG_ERR("invalid arb ch %d\r\n", id);
            return;
    }

    if (set > WPSET_4) {
            DBG_ERR("invalid protect ch %d\r\n", set);
            return;
    }

	loc_multi_cores(lock);

	// disable wp
	arb_set_reg(id, PROT_EN0_OFS(set), 0);
	arb_set_reg(id, PROT_EN1_OFS(set), 0);

    // disable inten
    intctrl_reg.reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
    intctrl_reg.reg &= ~0xffff;
    arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

    // ensure int sts is cleared
    sts_reg.reg = 0xffff;
    arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.reg);

	unl_multi_cores(lock);
}

void nvt_tzasc_init(void)
{
	/* Save values we will use later. */
	tzc.num_filters = WPSET_COUNT;
	tzc.addr_width  = 36;
	tzc.num_regions = 1;
}


/**
    nvt_wp_configure_region is use to protected specific area of memory to avoid non secure access

    nvt_wp_configure_region is used to program regions into the TrustZone.
    Sush as ARM provided (TZASC:TrustZone Address Space Controller). There are
    6 protect set, 4 region in each set.
    @note (Region can not cross between DRAM1 & DRAM2)
    DRAM1
    +-----+========>each set N within 4 regions
    |set0 |-----+  +-------+
    +-----+     +->|region0|
    | --- |     +  +-------+
    | --- |     +->  ....
    +-----+     +    ....
    |set5 |     +  +-------+
    +-----+     +->|region3|
                   +-------+

    DRAM2
    +-----+========>each set N within 4 regions
    |set0 |-----+  +-------+
    +-----+     +->|region0|
    | --- |     +  +-------+
    | --- |     +->  ....
    +-----+     +    ....
    |set5 |     +  +-------+
    +-----+     +->|region3|
                   +-------+

    @param[in] filter_set               memory protect function set
    @param[in] region                   memory protect region in a set
    @param[in] region_base              physical start address of memory protect region of a set (need 4 words alignment)
    @param[in] region_size              size of memory protect region of a set (need 4 words alignment)
    @param[in] sec_protected_level      protect policy(un-write/un-read/un-both w/r)
    @param[in] allow_ns_device_access   1: allow REE non secure access, 0: Not allow REE non secure access
    Example:
    @code
    {
        vaddr_t addr[2] = {0};
        addr[0] = core_mmu_get_va(TZASC_BASE, MEM_AREA_IO_SEC);
        addr[1] = core_mmu_get_va(TZASC2_BASE, MEM_AREA_IO_SEC);
        //use WPSET_0 / region0 / from addr[0] ~ addr[0]+size / UR&UW / Not allow REE access
        nvt_wp_configure_region_enable(WPSET_0, DMA_PROT_RGN0, addr[0], addr[0]_size, DMA_RWPLEL_UNRW, 0);
        //use WPSET_0 / region1 / from addr[0] ~ addr[0]+size / UR&UW / Not allow REE access
        nvt_wp_configure_region_enable(WPSET_0, DMA_PROT_RGN1, addr[1], addr[1]_size, DMA_RWPLEL_UNRW, 0);

    }
    @endcode
 */
void nvt_wp_configure_region_enable(DMA_WRITEPROT_SET filter_set,
									paddr_t     region_base,
									uint32_t    region_size,
									DMA_WRITEPROT_LEVEL sec_protected_level,
									uint32_t allow_ns_device_access)
{
	DDR_ARB id;
	DMA_WRITEPROT_ATTR  protect_attr = {0};
	/* Do range checks on filters and regions. */
	assert((filters < tzc.num_filters) && (region < tzc.num_regions));
	memset(&(protect_attr.mask), 0x0, sizeof(protect_attr.mask));
	protect_attr.mask.CPU_NS = ~allow_ns_device_access;
	protect_attr.level = sec_protected_level;
	protect_attr.starting_addr = region_base;
	protect_attr.size = region_size;

	if ((protect_attr.starting_addr + protect_attr.size) >= DRAM_PHYS_MAX_ADDRESS) {
		DBG_ERR("Address exceed DDR physical addr[0x%08x] ~ addr[0x%08x]\r\n", (int)protect_attr.starting_addr, (int)(protect_attr.starting_addr + protect_attr.size));
		return;
	}
	id = DDR_ARB_1;

	//protect_attr.protect_rgn_attr[region].en = ~allow_ns_device_access;

 	if(allow_ns_device_access == 0)	{//enable WP
		tzc_enable_filters(id, filter_set, &protect_attr);
	} else {
		tzc_disable_filters(id, filter_set);
	}

}

void nvt_tzasc_config(void)
{
	IMSG_RAW("TZASC config:\n");
	IMSG_RAW("\r\t0x%08x@0x%08x \t(RW/N)\tRee\n", CFG_TZDRAM_SIZE, CFG_TZDRAM_START);
	nvt_wp_configure_region_enable(WPSET_0, CFG_TZDRAM_START, CFG_TZDRAM_SIZE, DMA_WP, 0);
	nvt_wp_configure_region_enable(WPSET_1, CFG_TZDRAM_START, CFG_TZDRAM_SIZE, DMA_RP, 0);
}

#if TRACE_LEVEL >= TRACE_DEBUG
#if 0
static uint32_t tzc_read_region_attributes(vaddr_t base, uint32_t region)
{
	return io_read32(base + REGION_ATTRIBUTES_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_base_low(vaddr_t base, uint32_t region)
{
	return io_read32(base + REGION_BASE_LOW_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_base_high(vaddr_t base, uint32_t region)
{
	return io_read32(base + REGION_BASE_HIGH_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_top_low(vaddr_t base, uint32_t region)
{
	return io_read32(base + REGION_TOP_LOW_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_top_high(vaddr_t base, uint32_t region)
{
	return io_read32(base + REGION_TOP_HIGH_OFF + REGION_NUM_OFF(region));
}

#define REGION_MAX      8
static const __maybe_unused char *const tzc_attr_msg[] = {
	"TZC_REGION_S_NONE",
	"TZC_REGION_S_RD",
	"TZC_REGION_S_WR",
	"TZC_REGION_S_RDWR"
};

void tzc_dump_state(void)
{
	uint32_t n;
	uint32_t temp_32reg, temp_32reg_h;

	DMSG("enter");
	for (n = 0; n <= REGION_MAX; n++) {
		temp_32reg = tzc_read_region_attributes(tzc.base, n);
		if (!(temp_32reg & REG_ATTR_F_EN_MASK)) {
			continue;
		}

		DMSG("\n");
		DMSG("region %d", n);
		temp_32reg = tzc_read_region_base_low(tzc.base, n);
		temp_32reg_h = tzc_read_region_base_high(tzc.base, n);
		DMSG("region_base: 0x%08x%08x", temp_32reg_h, temp_32reg);
		temp_32reg = tzc_read_region_top_low(tzc.base, n);
		temp_32reg_h = tzc_read_region_top_high(tzc.base, n);
		DMSG("region_top: 0x%08x%08x", temp_32reg_h, temp_32reg);
		temp_32reg = tzc_read_region_attributes(tzc.base, n);
		DMSG("secure rw: %s",
			 tzc_attr_msg[temp_32reg >> REG_ATTR_SEC_SHIFT]);
		if (temp_32reg & (1 << 0)) {
			DMSG("filter 0 enable");
		}
		if (temp_32reg & (1 << 1)) {
			DMSG("filter 1 enable");
		}
		if (temp_32reg & (1 << 2)) {
			DMSG("filter 2 enable");
		}
		if (temp_32reg & (1 << 3)) {
			DMSG("filter 3 enable");
		}
	}
	DMSG("exit");
}
#endif
#endif /* CFG_TRACE_LEVEL >= TRACE_DEBUG */




