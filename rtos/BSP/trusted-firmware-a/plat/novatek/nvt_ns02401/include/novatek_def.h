/*
 * Copyright (c) 2024, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_MMAP_H
#define NOVATEK_MMAP_H

#define BL31_BASE       _BOARD_BL31_ADDR_
#define BL31_SIZE       _BOARD_BL31_SIZE_
#define BL31_LIMIT      (BL31_BASE + BL31_SIZE)

#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	8
#define PLATFORM_STACK_SIZE		0x800

#define _BSP_NS02401_

/* IP BASE */
#define UART0_BASE  0x4f0130000
#define GICD_BASE   0x4ff800000
#define GICR_BASE   (GICD_BASE + 0x40000)
//#define TZPC_BASE   0x2f0166000
#define WDT_BASE    0x4f0060000
#define CG_BASE     0x4f0020000
//#define STBC_BASE   0x2f0230000
//#define DDR_ARB_REG_BASE     0x2f0220000 //tzasc
#define TZASC_REG_BASE     0x4f02d0000 //tzasc
#define LPV_REG_BASE	   0x4f01f0000
//#define DDR_ARB2_REG_BASE	 0x2f00d0000//no ARB2
#define RELEASE_ADDR    0x4f0090000
#define DEVICE_START_ADDR   0x4f0000000
#define DEVICE_END_ADDR     0x4ffff0000
//#define RTC_BASE    0x2f0a70000

/* UART configuration */
#define UART0_BAUDRATE      115200

#if defined(_NVT_FPGA_EMULATION_ON_)
#define UART0_CLK_IN_HZ     24000000
#define OSC_CLK_IN_HZ       24000000
#else
#define UART0_CLK_IN_HZ     24000000
#define OSC_CLK_IN_HZ       12000000
#endif

#define REG_ADDR_REMAP      0xFFE41014
#define REG_RELEASE_ADDR    0x4F009010C

/* WDT related*/
#define ATF_CG_ENABLE_OFS 0x70
#define ATF_CG_RESET_OFS 0x90
#define ATF_WDT_RST 2
#define ATF_WDT_POS 2
#define MAN_RST_OFS 0xC
#endif /* NOVATEK_MMAP_H */
