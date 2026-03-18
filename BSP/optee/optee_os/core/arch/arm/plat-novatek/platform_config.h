/**
   Nvt tee system init
   To config the NVT platform parameters
   @file       platform_config.h
   @ingroup
   @note
   Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause.
*/
#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <mm/generic_ram_layout.h>

/* Make stacks aligned to data cache line length */
#if (defined(PLATFORM_FLAVOR_na51090) || defined(PLATFORM_FLAVOR_na51102) || defined(PLATFORM_FLAVOR_na51103) || defined(PLATFORM_FLAVOR_ns02201) || defined(PLATFORM_FLAVOR_ns02302) || defined(PLATFORM_FLAVOR_ns02401))
#define STACK_ALIGNMENT		64
#else
#define STACK_ALIGNMENT		32
#endif /* PLATFORM_FLAVOR_na51090 || PLATFORM_FLAVOR_na51102 || PLATFORM_FLAVOR_na51103 || PLATFORM_FLAVOR_ns02201 || defined(PLATFORM_FLAVOR_ns02302) || defined(PLATFORM_FLAVOR_ns02401) */

#ifdef CFG_WITH_PAGER
#error "Pager is not supported for nvt platform"
#endif

/* This will be used to define the head info chip name */
#if defined(PLATFORM_FLAVOR_na51089)
	#define NVT_HEADINFO_CHIP_NAME "te51089"
#elif defined(PLATFORM_FLAVOR_na51090)
	#define NVT_HEADINFO_CHIP_NAME "te51090"
#elif defined(PLATFORM_FLAVOR_na51102)
	#define NVT_HEADINFO_CHIP_NAME "te51102"
#elif defined(PLATFORM_FLAVOR_na51103)
	#define NVT_HEADINFO_CHIP_NAME "te51103"
#elif defined(PLATFORM_FLAVOR_ns02201)
	#define NVT_HEADINFO_CHIP_NAME "te02201"
#elif defined(PLATFORM_FLAVOR_ns02401)
	#define NVT_HEADINFO_CHIP_NAME "te02401"
#elif defined(PLATFORM_FLAVOR_ns02301)
	#define NVT_HEADINFO_CHIP_NAME "te02301"
#elif defined(PLATFORM_FLAVOR_ns02302)
	#define NVT_HEADINFO_CHIP_NAME "te02302"
#else
	#define NVT_HEADINFO_CHIP_NAME "te51055"
#endif 

// CA9 family
#if (defined(PLATFORM_FLAVOR_na51055) || defined(PLATFORM_FLAVOR_na51084) || defined(PLATFORM_FLAVOR_na51089))
	#define SCU_BASE			0xffd00000
	/*
	 * SCU Invalidate Register
	 *
	 * Invalidate all registers
	 */
	#define SCU_INV_CTRL_INIT		0xFFFFFFFF

	/*
	 * SCU Access Register
	 * - both secure CPU access SCU
	 */
	#define SCU_SAC_CTRL_INIT		0x0000000F

	/*
	 * SCU NonSecure Access Register
	 * - both nonsec cpu access SCU, private and global timer
	 */
	#define SCU_NSAC_CTRL_INIT		0x00000FFF

	#define GIC_BASE			SCU_BASE
	#define GICC_OFFSET			0x100
	#define GICC_SIZE			0x100
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define PL310_BASE			0xffe00000
	#define PL310_SIZE			0x1000

	#define UART0_BASE			0xf0290000
	#define UART1_BASE			0xf0300000
	#define UART2_BASE			0xf0310000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#if defined(PLATFORM_FLAVOR_na51089)
	#define DRAM0_SIZE			0x08000000
	#else
	#define DRAM0_SIZE			0x20000000
	#endif

	#define RELEASE_ADDR			0xF07F8000
	//#define __OPTEEOS
	//#define _BSP_NA51055_
	// Note: moved to core/arch/arm/plat-novatek/conf.mk
	// core-platform-cppflags += -D_BSP_NA51055_
	// core-platform-cppflags += -D__OPTEEOS

	/*
	 * PL310 TAG RAM Control Register
	 *
	 * bit[10:8]:1 - 2 cycle of write accesses latency
	 * bit[6:4]:1 - 2 cycle of read accesses latency
	 * bit[2:0]:1 - 2 cycle of setup latency
	 */
	#ifndef PL310_TAG_RAM_CTRL_INIT
	#define PL310_TAG_RAM_CTRL_INIT		0x00000111
	#endif

	/*
	 * PL310 DATA RAM Control Register
	 *
	 * bit[10:8]:2 - 3 cycle of write accesses latency
	 * bit[6:4]:2 - 3 cycle of read accesses latency
	 * bit[2:0]:2 - 3 cycle of setup latency
	 */
	#ifndef PL310_DATA_RAM_CTRL_INIT
	#define PL310_DATA_RAM_CTRL_INIT	0x00000111
	#endif

	/*
	 * PL310 Auxiliary Control Register
	 *
	 * Early BRESP enable (bit31:30=2b01)
	 * I/Dcache prefetch enabled (bit29:28=2b11)
	 * NS can access interrupts (bit27=1)
	 * NS can lockown cache lines (bit26=1)
	 * Pseudo-random replacement policy (bit25=0)
	 * Force write allocated (default)
	 * Shared attribute internally ignored (bit22=1, bit13=0)
	 * Parity disabled (bit21=0)
	 * Event monitor disabled (bit20=0)
	 * Platform fmavor specific way config:
	 * - 16kb way size (bit19:17=3b001)
	 * - 8-way associciativity (bit16=0)
	 * Store buffer device limitation enabled (bit11=1)
	 * Cacheable accesses have high prio (bit10=0)
	 * Full Line Zero (FLZ) disabled (bit0=0)
	 */
	#ifndef PL310_AUX_CTRL_INIT
	#define PL310_AUX_CTRL_INIT		0x7C020000
	#endif

	/*
	 * PL310 Prefetch Control Register
	 *
	 * Double linefill disabled (bit30=0)
	 * I/D prefetch enabled (bit29:28=2b11)
	 * Prefetch drop enabled (bit24=1)
	 * Incr double linefill disable (bit23=0)
	 * Prefetch offset = 7 (bit4:0)
	 */
	#define PL310_PREFETCH_CTRL_INIT	0x70000000

	/*
	 * PL310 Power Register
	 *
	 * Dynamic clock gating enabled
	 * Standby mode enabled
	 */
	#define PL310_POWER_CTRL_INIT		0x00000000

#elif defined(PLATFORM_FLAVOR_na51103)
        #define GIC_BASE                        0xfff00000
        #define GICD_OFFSET                     0x1000
        #define GICD_SIZE                       0x1000
        #define GICC_OFFSET                     0x2000
        #define GICC_SIZE                       0x2000

        #define GICC_BASE                       (GIC_BASE + GICC_OFFSET)
        #define GICD_BASE                       (GIC_BASE + GICD_OFFSET)

        #define UART0_BASE                      0xf0280000
        #define UART1_BASE                      0xf0290000
        #define UART2_BASE                      0xf02a0000

        #define CONSOLE_UART_BASE               UART0_BASE
        #define CONSOLE_BAUDRATE                115200
        #define CONSOLE_UART_CLK_IN_HZ          24000000

        #define DRAM0_BASE                      0x00000000
        #define DRAM0_SIZE                      0x20000000

        #define RELEASE_ADDR                    0xF0011200

// CA53 family for ARM64
#elif defined(PLATFORM_FLAVOR_na51090)
	#define GIC_BASE			0x2fff00000
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x2000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0x2f0280000
	#define UART1_BASE			0x2f0290000
	#define UART2_BASE			0x2f02a0000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x20000000
#elif defined(PLATFORM_FLAVOR_na51102)
	#define GIC_BASE			0x2fff00000
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x2000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0x2f0290000
	#define UART1_BASE			0x2f0300000
	#define UART2_BASE			0x2f0310000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x20000000
#elif defined(PLATFORM_FLAVOR_ns02201)
	#define GIC_BASE			0x2fff00000
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x2000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0x2f0130000
	#define UART1_BASE			0x2f0131000
	#define UART2_BASE			0x2f0132000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x20000000
#elif defined(PLATFORM_FLAVOR_ns02401)
	#define GIC_BASE			0x4ff800000
	#define GICD_OFFSET			0
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x40000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0x4f0130000
	#define UART1_BASE			0x4f0131000
	#define UART2_BASE			0x4f0132000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x20000000
#elif defined(PLATFORM_FLAVOR_ns02301)
	#define GIC_BASE			0xfff00000
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x2000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0xf0290000
	#define UART1_BASE			0xf0300000
	#define UART2_BASE			0xf0310000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x08000000
	#define RELEASE_ADDR			0xF01E0100
#elif defined(PLATFORM_FLAVOR_ns02302)
	#define GIC_BASE			0x2fff00000
	#define GICD_OFFSET			0x1000
	#define GICD_SIZE			0x1000
	#define GICC_OFFSET			0x2000
	#define GICC_SIZE			0x2000

	#define GICC_BASE			(GIC_BASE + GICC_OFFSET)
	#define GICD_BASE			(GIC_BASE + GICD_OFFSET)

	#define UART0_BASE			0x2f0110000
	#define UART1_BASE			0x2f0111000
	#define UART2_BASE			0x2f0112000

	#define CONSOLE_UART_BASE		UART0_BASE
	#define CONSOLE_BAUDRATE		115200
	#define CONSOLE_UART_CLK_IN_HZ		24000000

	#define DRAM0_BASE			0x00000000
	#define DRAM0_SIZE			0x20000000
#else
	#error "Unknown platform flavor"
#endif /* PLATFORM_FLAVOR_na51055 || PLATFORM_FLAVOR_na51084 || PLATFORM_FLAVOR_na51089 */
#endif /*PLATFORM_CONFIG_H*/
