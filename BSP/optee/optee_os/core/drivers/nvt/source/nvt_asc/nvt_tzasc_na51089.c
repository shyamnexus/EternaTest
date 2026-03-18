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

static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)

#define DRAM2_STARTING_PHYS_ADDRESS		0x40000000

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_STARTADDR0_REG0_OFS+(ch)*8)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_STOPADDR0_REG0_OFS+(ch)*8)
#define PROTECT_CH_MSK0_OFS(ch)         (DMA_PROTECT_RANGE0_MSK0_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK1_OFS(ch)         (DMA_PROTECT_RANGE0_MSK1_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK2_OFS(ch)         (DMA_PROTECT_RANGE0_MSK2_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK3_OFS(ch)         (DMA_PROTECT_RANGE0_MSK3_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK4_OFS(ch)         (DMA_PROTECT_RANGE0_MSK4_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK5_OFS(ch)         (DMA_PROTECT_RANGE0_MSK5_REG_OFS+(ch)*32)

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
			if (ddr_id == DDR_ARB_1) {
				va[ddr_id] = phys_to_virt(IOADDR_DDR_ARB_PROT_REG_BASE, MEM_AREA_IO_SEC,0x5000);

			} else {
				va[ddr_id] = phys_to_virt(IOADDR_DDR_ARB2_PROT_REG_BASE, MEM_AREA_IO_SEC,0x5000);
			}
		}
		return (vaddr_t)va[ddr_id];
	} else {
		if (ddr_id == DDR_ARB_1) {
			return IOADDR_DDR_ARB_PROT_REG_BASE;
		} else {
			return IOADDR_DDR_ARB2_PROT_REG_BASE;
		}

	}
}

static vaddr_t arb_base_addr(DDR_ARB ddr_id)
{
	static void *va_arb[DDR_ARB_COUNT];

	if (cpu_mmu_enabled()) {
		if (!va_arb[ddr_id]) {
			if (ddr_id == DDR_ARB_1) {
				va_arb[ddr_id] = phys_to_virt(IOADDR_DDR_ARB_REG_BASE, MEM_AREA_IO_SEC,0x5000);
			} else {
				va_arb[ddr_id] = phys_to_virt(IOADDR_DDR_ARB2_REG_BASE, MEM_AREA_IO_SEC,0x5000);
			}
		}
		return (vaddr_t)va_arb[ddr_id];
	} else {
		if (ddr_id == DDR_ARB_1) {
			return IOADDR_DDR_ARB_PROT_REG_BASE;
		} else {
			return IOADDR_DDR_ARB2_PROT_REG_BASE;
		}

	}
}


static void prot_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value)
{
	io_write32((tzasc_base_addr(ddr_id) + ofs), value);
}

static UINT32 prot_get_reg(DDR_ARB id, UINT32 ofs)
{
	return io_read32((tzasc_base_addr(id) + ofs));
}

static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value)
{
	io_write32((arb_base_addr(ddr_id) + ofs), value);
}

static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs)
{
	return io_read32((arb_base_addr(ddr_id) + ofs));

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
	UINT32 i = 0;
	unsigned long lock = 0;
	UINT32 v_mask[DMA_CH_GROUP_CNT];
	T_DMA_PROTECT_INTSTS_REG sts_reg = {0};
	T_DMA_PROTECT_INTCTRL_REG intctrl_reg = {0};
	T_DMA_PROTECT_CTRL_REG wp_ctrl;
	T_DMA_PROTECT_REGION_EN_REG0 region_en0 = {0};
	T_DMA_PROTECT_REGION_EN_REG1 region_en1 = {0};

	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			if (((UINT32)p_attr->protect_rgn_attr[i].starting_addr) & 0xF) {
				DBG_WRN("start addr 0x%x NOT 4 word align\r\n",
						(unsigned int)p_attr->protect_rgn_attr[i].starting_addr);
			}
			if (((UINT32)p_attr->protect_rgn_attr[i].size) & 0xF) {
				DBG_WRN("size 0x%x NOT 4 word align\r\n",
						(unsigned int)p_attr->protect_rgn_attr[i].size);
			}
		}
	}

	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));

	loc_multi_cores(lock);
	// disable wp before setting
	wp_ctrl.reg = prot_get_reg(id, DMA_PROTECT_CTRL_REG_OFS);
	wp_ctrl.reg &= ~(1 << set);
	prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.reg);

	// disable ch inten before setting
	intctrl_reg.reg = arb_get_reg(id, DMA_PROTECT_INTCTRL_REG_OFS);
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			intctrl_reg.reg &= ~(1 << (set * 4 + i));
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);

	// ensure int sts is cleared
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			sts_reg.reg |= 1 << (set * 4 + i);
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTSTS_REG_OFS, sts_reg.reg);

	// setup protected channels
	prot_set_reg(id, PROTECT_CH_MSK0_OFS(set), v_mask[DMA_CH_GROUP0]);
	prot_set_reg(id, PROTECT_CH_MSK1_OFS(set), v_mask[DMA_CH_GROUP1]);
	prot_set_reg(id, PROTECT_CH_MSK2_OFS(set), v_mask[DMA_CH_GROUP2]);
	prot_set_reg(id, PROTECT_CH_MSK3_OFS(set), v_mask[DMA_CH_GROUP3]);
	prot_set_reg(id, PROTECT_CH_MSK4_OFS(set), v_mask[DMA_CH_GROUP4]);
	prot_set_reg(id, PROTECT_CH_MSK5_OFS(set), v_mask[DMA_CH_GROUP5]);

	// setup range
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			prot_set_reg(id, PROTECT_START_ADDR_OFS(set * 4 + i), p_attr->protect_rgn_attr[i].starting_addr);
			prot_set_reg(id, PROTECT_END_ADDR_OFS(set * 4 + i),
						 p_attr->protect_rgn_attr[i].starting_addr + p_attr->protect_rgn_attr[i].size - 1);
		}
	}

	// enable write protect
	wp_ctrl.reg |= 1 << set;

	// set mode
	wp_ctrl.reg &= ~(1 << (set + 8));
	wp_ctrl.reg |= (p_attr->protect_mode << (set + 8));

	// set level
	wp_ctrl.reg &= ~(0x3 << (set * 2 + 16));
	wp_ctrl.reg |= (p_attr->level << (set * 2 + 16));

	prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.reg);

	// enable region
	if (set > WPSET_3) {
		region_en1.reg = prot_get_reg(id, DMA_PROTECT_REGION_EN_REG1_OFS);
		for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
			if (p_attr->protect_rgn_attr[i].en) {
				region_en1.reg |= (1 << ((set - 4) * 8 + i));
			}
		}
		prot_set_reg(id, DMA_PROTECT_REGION_EN_REG1_OFS, region_en1.reg);
	} else {
		region_en0.reg = prot_get_reg(id, DMA_PROTECT_REGION_EN_REG0_OFS);
		for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
			if (p_attr->protect_rgn_attr[i].en) {
				region_en0.reg |= 1 << (set * 8 + i);
			}
		}
		prot_set_reg(id, DMA_PROTECT_REGION_EN_REG0_OFS, region_en0.reg);
	}
#if 0
	// enable interrupt
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_attr->protect_rgn_attr[i].en) {
			intctrl_reg.reg |= 1 << (set * 4 + i);
		}
	}
	arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);
#endif
	unl_multi_cores(lock);
}

void tzc_disable_filters(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	UINT32 lock = 0;
	T_DMA_PROTECT_INTSTS_REG sts_reg = {0};
	T_DMA_PROTECT_INTCTRL_REG intctrl_reg = {0};
	T_DMA_PROTECT_CTRL_REG wp_ctrl;

	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	loc_multi_cores(lock);

	// disable wp
	wp_ctrl.reg = prot_get_reg(id, DMA_PROTECT_CTRL_REG_OFS);
	wp_ctrl.reg &= ~(1 << set);
	prot_set_reg(id, DMA_PROTECT_CTRL_REG_OFS, wp_ctrl.reg);

	// disable ch inten
	intctrl_reg.reg = arb_get_reg(id, DMA_PROTECT_INTCTRL_REG_OFS);
	intctrl_reg.reg &= ~(0xf << (set * 4));
	arb_set_reg(id, DMA_PROTECT_INTCTRL_REG_OFS, intctrl_reg.reg);

	// ensure int sts is cleared
	sts_reg.reg = 0xf << (set * 4);
	arb_set_reg(id, DMA_PROTECT_INTSTS_REG_OFS, sts_reg.reg);

	unl_multi_cores(lock);
}

void nvt_tzasc_init(void)
{
	/* Save values we will use later. */
	tzc.num_filters = WPSET_COUNT;
	tzc.addr_width  = 32;
	tzc.num_regions = DMA_PROT_RGN_TOTAL;
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
    +-----+	    +->|region0|
    | --- |  	+  +-------+
    | --- |  	+->  ....
    +-----+  	+    ....
    |set5 |	 	+  +-------+
    +-----+	 	+->|region3|
                   +-------+

    DRAM2
    +-----+========>each set N within 4 regions
    |set0 |-----+  +-------+
    +-----+	    +->|region0|
    | --- |  	+  +-------+
    | --- |  	+->  ....
    +-----+  	+    ....
    |set5 |	 	+  +-------+
    +-----+	 	+->|region3|
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
									DMA_PROT_REGION  region,
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
	protect_attr.mask.bCPU_NS = ~allow_ns_device_access;
	protect_attr.level = sec_protected_level;
	protect_attr.protect_rgn_attr[region].starting_addr = region_base;
	protect_attr.protect_rgn_attr[region].size = region_size;

	if(protect_attr.protect_rgn_attr[region].starting_addr >= DRAM2_STARTING_PHYS_ADDRESS) {
		id = DDR_ARB_2;
	} else {	// starting_addr @ DDR1 but size exceed DDR1
		if((protect_attr.protect_rgn_attr[region].starting_addr + protect_attr.protect_rgn_attr[region].size) >= DRAM2_STARTING_PHYS_ADDRESS) {
			DBG_ERR("Address cross DDR1 & DDR2 physical addr[0x%08x] ~ addr[0x%08x]\r\n", (int)protect_attr.protect_rgn_attr[region].starting_addr, (int)(protect_attr.protect_rgn_attr[region].starting_addr+protect_attr.protect_rgn_attr[region].size));
			return;
		}
		id = DDR_ARB_1;
	}

	protect_attr.protect_rgn_attr[region].en = ~allow_ns_device_access;

 	if(allow_ns_device_access == 0)	{//enable WP
		tzc_enable_filters(id, filter_set, &protect_attr);
	} else {
		tzc_disable_filters(id, filter_set);
	}

}

void nvt_tzasc_config(void)
{
	IMSG_RAW("TZASC config:\n");
	/* size@address   permission[Read only(R/Y), R/W(RW/Y), No R/W(RW/N)]     Tee/Ree  */
	IMSG_RAW("\r\t0x%08x@0x%08x \t(RW/N)\tRee\n", CFG_TZDRAM_SIZE, CFG_TZDRAM_START);
	nvt_wp_configure_region_enable(WPSET_5, DMA_PROT_RGN0, CFG_TZDRAM_START, CFG_TZDRAM_SIZE, DMA_RWPLEL_UNRW, 0);
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



