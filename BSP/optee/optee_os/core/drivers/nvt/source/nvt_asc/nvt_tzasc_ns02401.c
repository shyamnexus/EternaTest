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

#define DRAM_PHYS_MAX_ADDRESS           0x3FFFFFFFC
static  VK_DEFINE_SPINLOCK(my_lock);
//static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs);
static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value);
static uint32_t lpv_get_reg(int lpv_id, uint32_t ofs);
static void lpv_set_reg(int lpv_id, uint32_t ofs, uint32_t value);
void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set);
void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr);

#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)

#define _Y_LOG(fmt, args...)            printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)            printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)            printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)            printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)            printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)            printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)

#define PROT_SETREG(ofs,value)          arb_set_reg(DDR_ARB_1, (ofs),(value))
//#define PROT_GETREG(ofs)                arb_get_reg(DDR_ARB_1, (ofs))

#define PROT2_SETREG(ofs,value)         arb_set_reg(DDR_ARB_2, (ofs),(value))
//#define PROT2_GETREG(ofs)               arb_get_reg(DDR_ARB_2, (ofs))


static PROT_GP_TO_LPV_PARAM prot_gp_to_lpv[13][8] = {
	//gp0
	{{1, 0, 0, 7, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp1
	{{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 8, 15, 0x66}, {0, 0, 0, 0, 0}},
	//gp2
	{{5, 0, 0, 7, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp3
	{{10, 0, 0, 15, 0}, {4, 0, 0, 15, 0x1111}, {2, 0, 0, 15, 0x2222}, {9, 4, 24, 31, 0x33}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp4
	{{9, 0, 0, 7, 0}, {9, 0, 8, 15, 0x11}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp5
	{{10, 0, 16, 23, 0}, {10, 0, 24, 31, 0x11}, {10, 4, 0, 7, 0x22}, {10, 4, 8, 15, 0x33}, {10, 4, 16, 23, 0x44}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp6
	{{11, 0, 0, 7, 0}, {11, 0, 8, 15, 0x11}, {11, 0, 16, 23, 0x22}, {11, 0, 24, 31, 0x33}, {11, 4, 0, 7, 0x44}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp7
	{{11, 4, 8, 15, 0}, {11, 4, 16, 23, 0x11}, {11, 4, 24, 31, 0x22}, {11, 8, 0, 7, 0x33}, {11, 8, 8, 15, 0x44}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp8
	{{3, 0, 24, 31, 0}, {3, 4, 0, 7, 0x11}, {4, 0, 16, 31, 0x2222}, {3, 4, 8, 15, 0x33}, {4, 4, 0, 31, 0x44444444}, {4, 8, 0, 7, 0x55}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp9
	{{2, 0, 16, 23, 0}, {2, 0, 24, 31, 0x11}, {2, 4, 0, 7, 0x22}, {4, 4, 8, 15, 0x33}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp10
	{{0, 0, 0, 31, 0}, {0, 4, 0, 31, 0x11111111}, {0, 8, 0, 31, 0x22222222}, {0, 12, 0, 31, 0x33333333}, {1, 4, 8, 71, 0x44444444}, {9, 0, 16, 55, 0x55555555}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp11
	{{3, 0, 0, 7, 0}, {3, 0, 8, 15, 0x11}, {3, 0, 16, 23, 0x22}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
	//gp12
	{{7, 0, 0, 15, 0}, {6, 0, 0, 7, 0x11}, {6, 0, 8, 15, 0x22}, {6, 0, 16, 31, 0x3333}, {6, 4, 0, 7, 0x44}, {6, 4, 8, 23, 0x5555}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},

};

static uint32_t wp_mask[8][13] = {
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

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



/*static UINT32 prot_get_reg(DDR_ARB id, int prot_id, UINT32 offset)
{
	if (id == DDR_ARB_1) {
		if (prot_id < 6) {
			return PROT_GETREG(offset + prot_id * 0x100);
		} else {
			DBG_ERR("invalid protect set %d\r\n", prot_id);
			return 0;
		}
	} else {
		if (prot_id < 6) {
			return PROT2_GETREG(offset + prot_id * 0x100);
		} else {
			DBG_ERR("invalid protect set %d\r\n", prot_id);
			return 0;
		}

	}
}*/

static void prot_set_reg(DDR_ARB id, int prot_id, UINT32 offset, REGVALUE value)
{
	if (prot_id < 8) {
		if (id == DDR_ARB_1) {
			PROT_SETREG(offset + prot_id*0x10, value);
		} else {
			PROT2_SETREG(offset + prot_id*0x10, value);
		}
	} else {
		DBG_ERR("invalid protect set %d\r\n", prot_id);
	}
}

static void tzasc_set_ch_id(int enable, int gp, int id, int set_id_7) {
	unsigned int reg = 0;
	unsigned int mask = 0;
	unsigned int reg_off, start_bit, end_bit;
	int end_bit_tmp;


	if (enable) {

		//if (prot_gp_to_lpv[id][gp].end_bit > 31) {
			reg_off = prot_gp_to_lpv[gp][id].reg_off;
			start_bit = prot_gp_to_lpv[gp][id].start_bit;
			end_bit = 31;
			end_bit_tmp = prot_gp_to_lpv[gp][id].end_bit;

			do {
				mask = BIT(end_bit) - BIT(start_bit) + BIT(end_bit);
				reg = lpv_get_reg(0+prot_gp_to_lpv[gp][id].top_id, 0x50+reg_off);
				reg &= ~mask;
				if (set_id_7) {
					reg |= (0x77777777&mask);
				} else {
					reg |= ((prot_gp_to_lpv[gp][id].lpv_id << start_bit)&mask);
				}
				lpv_set_reg(0+prot_gp_to_lpv[gp][id].top_id, 0x50+reg_off, reg);

				reg_off +=4;
				start_bit = 0;
				end_bit_tmp -= 32;
				end_bit = (end_bit_tmp > 32) ? 31 : end_bit_tmp;
			} while (end_bit_tmp > 0);

		//} else {
		//	mask = BIT(prot_gp_to_lpv[id][gp].end_bit) - BIT(prot_gp_to_lpv[id][gp].start_bit) + BIT(prot_gp_to_lpv[id][gp].end_bit);
		//	reg = lpv_get_reg(0+prot_gp_to_lpv[id][gp].top_id, (ULONG)pmodule_info->io_addr[5+prot_gp_to_lpv[id][gp].top_id], 0x50+prot_gp_to_lpv[id][gp].reg_off);
		//	reg &= ~mask;
		//	if (set_id_7) {
		//		reg |= (0x77777777&mask);
		//	} else {
		//		reg |= (prot_gp_to_lpv[id][gp].lpv_id << prot_gp_to_lpv[id][gp].start_bit);
		//	}
		//	lpv_set_reg(0+prot_gp_to_lpv[id][gp].top_id, (ULONG)pmodule_info->io_addr[5+prot_gp_to_lpv[id][gp].top_id], 0x50+prot_gp_to_lpv[id][gp].reg_off, reg);
		//}
	} else {
			// top 1
		lpv_set_reg(0, 0x50, 0x77777777);
		lpv_set_reg(0, 0x54, 0x77777777);
		lpv_set_reg(0, 0x58, 0x77777777);
		lpv_set_reg(0, 0x5c, 0x77777777);

		// top 2
		lpv_set_reg(1, 0x50, 0x77777777);
		lpv_set_reg(1, 0x54, 0x77777777);
		lpv_set_reg(1, 0x58, 0x77777777);
		lpv_set_reg(1, 0x5c, 0x77777777);

		// top 3
		lpv_set_reg(2, 0x50, 0x77777777);
		lpv_set_reg(2, 0x54, 0x77777777);

		// top 4
		lpv_set_reg(3, 0x50, 0x77777777);
		lpv_set_reg(3, 0x54, 0x77777777);

		// top 5
		lpv_set_reg(4, 0x50, 0x77777777);
		lpv_set_reg(4, 0x54, 0x77777777);
		lpv_set_reg(4, 0x58, 0x77777777);

		// top 6
		lpv_set_reg(5, 0x50, 0x77777777);

		// top 7
		lpv_set_reg(6, 0x50, 0x77777777);
		lpv_set_reg(6, 0x54, 0x77777777);

		// top 8
		lpv_set_reg(7, 0x50, 0x77777777);

		// top 9
		//lpv_set_reg(8, (ULONG)pmodule_info->io_addr[0], 0x50, 0x00000000);
		//lpv_set_reg(8, (ULONG)pmodule_info->io_addr[0], 0x54, 0x00000000);

		// top 10
		lpv_set_reg(9, 0x50, 0x77777777);
		lpv_set_reg(9, 0x54, 0x77777777);

		// top 11
		lpv_set_reg(10, 0x50, 0x77777777);
		lpv_set_reg(10, 0x54, 0x77777777);

		// top 12
		lpv_set_reg(11, 0x50, 0x77777777);
		lpv_set_reg(11, 0x54, 0x77777777);
		lpv_set_reg(11, 0x58, 0x77777777);

	}

};

void arb_enable_wp(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr)
{
	int i = 0, group = 0, det_id = 0;
	uint32_t v_mask[DMA_CH_GROUP_CNT];
	T_TZASC_CONFIG_REG0 range_sel = {0};

	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_7) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	memset(v_mask, 0x0, sizeof(DMA_CH_MSK));
	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));
	memcpy(wp_mask[set], &p_attr->mask, sizeof(DMA_CH_MSK));
	//debug_dump_addr((UINT32)&p_attr->mask, sizeof(DMA_CH_MSK));
	
	tzasc_set_ch_id(0, 0, 0, 0); // set all channel user id to 7 if tzasc first time open

	// disable wp before setting
	prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, 0);



	det_id = 0;
	for (group = 0; group < DMA_CH_GROUP_CNT; group++) {
		for (i = 0; i < 8; i++) {
			if (v_mask[group]&(1<<i)) {
				det_id |= (1<<i);
				tzasc_set_ch_id(1, group, i, 0);
			}
		}
	}

	if (p_attr->level == DMA_WPLEL_UNWRITE) {
		// setup w protected channels

		prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, (det_id << 16) | det_id);

		if (p_attr->protect_mode == DMA_PROT_IN) {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x3;
			} else {
				range_sel.bit.range_sel_w = 0x2;
			}
		} else {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x4;
			} else {
				range_sel.bit.range_sel_w = 0x5;
			}
		}

	} else if (p_attr->level == DMA_WPLEL_DETECT) {
		// setup w detected channels
		prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, det_id);


		if (p_attr->protect_mode == DMA_PROT_IN) {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x3;
			} else {
				range_sel.bit.range_sel_w = 0x2;
			}
		} else {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x4;
			} else {
				range_sel.bit.range_sel_w = 0x5;
			}
		}

	} else if (p_attr->level == DMA_RPLEL_UNREAD) {
		// setup r protect channels
		prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, (det_id << 8) | (det_id << 24));

		if (p_attr->protect_mode == DMA_PROT_IN) {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_r = 0x3;
			} else {
				range_sel.bit.range_sel_r = 0x2;
			}
		} else {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_r = 0x4;
			} else {
				range_sel.bit.range_sel_r = 0x5;
			}
		}

	} else {
		// setup rw protected channels
		prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, (det_id << 24) | (det_id << 16) | (det_id << 8) | det_id);


		if (p_attr->protect_mode == DMA_PROT_IN) {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x3;
				range_sel.bit.range_sel_r = 0x3;
			} else {
				range_sel.bit.range_sel_w = 0x2;
				range_sel.bit.range_sel_r = 0x2;
			}
		} else {
			if (p_attr->protect_rgn_attr.starting_addr == 0x0) {
				p_attr->protect_rgn_attr.starting_addr = p_attr->protect_rgn_attr.size/2;
				range_sel.bit.range_sel_w = 0x4;
				range_sel.bit.range_sel_r = 0x4;
			} else {
				range_sel.bit.range_sel_w = 0x5;
				range_sel.bit.range_sel_r = 0x5;
			}
		}

	}
	range_sel.bit.addr0_h = (((p_attr->protect_rgn_attr.starting_addr-1) & 0xff00000000UL) >> 32);
	range_sel.bit.addr1_h = (((p_attr->protect_rgn_attr.starting_addr + p_attr->protect_rgn_attr.size-1) & 0xff00000000UL) >> 32);

	// set address
	prot_set_reg(id, (int)set, TZASC_START_ADDR_REG_OFS, (uint32_t)(p_attr->protect_rgn_attr.starting_addr-1));
	prot_set_reg(id, (int)set, TZASC_END_ADDR_REG_OFS, p_attr->protect_rgn_attr.starting_addr + p_attr->protect_rgn_attr.size-1);

	// set range
	prot_set_reg(id, (int)set, TZASC_CONFIG_REG0_OFS, range_sel.reg);

	//debug_dump_addr(IOADDR_DRAM_REG_BASE + 0x4300, 0x890);
}

void arb_disable_wp(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	int i = 0, group = 0;

	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_7) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	// set ch id back to 7
	for (group = 0; group < DMA_CH_GROUP_CNT; group++) {
		for (i = 0; i < 8; i++) {
			if (wp_mask[set][group]&(1<<i)) {
				tzasc_set_ch_id(1, group, i, 1);
			}
		}
	}


	// disable wp
	prot_set_reg(id, (int)set, TZASC_ID_REG_OFS, 0);
}

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

static vaddr_t arb_base_addr(DDR_ARB ddr_id)
{
	static void *va_arb[DDR_ARB_COUNT];

	if (cpu_mmu_enabled()) {
		if (!va_arb[ddr_id]) {
			if (ddr_id == DDR_ARB_1) {
				va_arb[ddr_id] = phys_to_virt(IOADDR_TZASC_REG_BASE, MEM_AREA_IO_SEC, 0x5000);
			} else {
				va_arb[ddr_id] = phys_to_virt(IOADDR_TZASC_REG_BASE, MEM_AREA_IO_SEC, 0x5000);
			}
		}
		return (vaddr_t)va_arb[ddr_id];
	} else {
		if (ddr_id == DDR_ARB_1) {
			return IOADDR_TZASC_REG_BASE;
		} else {
			return IOADDR_TZASC_REG_BASE;
		}

	}
}

static vaddr_t lpv_base_addr(int lpv_id)
{
	static void *va_arb[DMA_CH_GROUP_CNT];

	if (cpu_mmu_enabled()) {
		if (!va_arb[lpv_id]) {
			va_arb[lpv_id] = phys_to_virt(IOADDR_LPV_REG_BASE + (0x100000 * lpv_id), MEM_AREA_IO_SEC, 0x100);
		}
		return (vaddr_t)va_arb[lpv_id];
	} else {
		return IOADDR_LPV_REG_BASE+ (0x100000 * lpv_id);

	}
}

static unsigned int lpv_get_reg(int id, unsigned int offset)
{
	return io_read32(lpv_base_addr(id) + offset);

}

static void lpv_set_reg(int id, unsigned int offset, REGVALUE value)
{
	io_write32(lpv_base_addr(id)  + offset, value);
}

static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value)
{
	io_write32((arb_base_addr(ddr_id) + ofs), value);
}

/*static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs)
{
	return io_read32((arb_base_addr(ddr_id) + ofs));

}*/

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
	if (id > DDR_ARB_1) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}
	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}
	if (set > WPSET_5) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	if (p_attr->protect_rgn_attr.starting_addr & 0x7f) {
		DBG_ERR("starting addr not 32 word aligned 0x%08x\r\n", (unsigned int)p_attr->protect_rgn_attr.starting_addr);
		return;
	}
	loc_multi_cores(lock);
	arb_enable_wp(id, set, p_attr);
	unl_multi_cores(lock);
}

void tzc_disable_filters(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	UINT32 lock = 0;
	//T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	//T_MAU_INT_STATUS_REG sts_reg;

	if (id > DDR_ARB_2) {
		DBG_ERR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_5) {
		DBG_ERR("invalid protect ch %d\r\n", set);
		return;
	}

	loc_multi_cores(lock);
	arb_disable_wp(id, set);
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
	protect_attr.protect_mode = DMA_PROT_IN;
	protect_attr.protect_rgn_attr.en = ENABLE;
	protect_attr.protect_rgn_attr.size = region_size;
	protect_attr.protect_rgn_attr.starting_addr = region_base;
	/*if (protect_attr.protect_rgn_attr.starting_addr >= DRAM2_STARTING_PHYS_ADDRESS) {
		id = DDR_ARB_2;
		protect_attr.protect_rgn_attr.starting_addr -= DRAM2_STARTING_PHYS_ADDRESS;
	} else {    // starting_addr @ DDR1 but size exceed DDR1*/
		if ((protect_attr.protect_rgn_attr.starting_addr + protect_attr.protect_rgn_attr.size) >= DRAM_PHYS_MAX_ADDRESS) {
			DBG_ERR("Address cross DDR1 & DDR2 physical addr[0x%08x] ~ addr[0x%08x]\r\n", (int)protect_attr.protect_rgn_attr.starting_addr, (int)(protect_attr.protect_rgn_attr.starting_addr + protect_attr.protect_rgn_attr.size));
			return;
		}
		id = DDR_ARB_1;
	//}

	//protect_attr.protect_rgn_attr[region].en = ~allow_ns_device_access;
	if (allow_ns_device_access == 0) { //enable WP
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



