/*
 *   @file   vg_memcpy.h
 *
 *   @brief  vg memcpy header file.
 *
 *   A rewrite version of memory opertion function for vg.
 *
 *   Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.
 */

#ifndef _VG_MEMORY_H_
#define _VG_MEMORY_H_

void vg_mem_init(unsigned int debug_size);
int vg_memcpy(int dst_ddr_id, uintptr_t dst_pa,
	      int src_ddr_id, uintptr_t src_pa,
	      int size);
void vg_memset(int ddr_id, uintptr_t addr_pa, unsigned int pattern, int size);
int vg_ddr_to_chip(int ddr_id);

#endif
