/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_MMAP_H
#define NOVATEK_MMAP_H

/*#define BOOT_AFTER_UBOOT    1*/

#define BL31_BASE       _BOARD_BL31_ADDR_
#define BL31_SIZE       _BOARD_BL31_SIZE_
#define BL31_LIMIT      (BL31_BASE + BL31_SIZE)

#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_STACK_SIZE		(0x1000 / PLATFORM_CORE_COUNT)

/* IP BASE */
#define UART0_BASE  0x2f0280000
#define GICD_BASE   0x2fff01000
#define GICC_BASE   0x2fff02000
#define TZPC_BASE   0x2f0050000
#define WDT_BASE    0x2f0110000
#define CG_BASE     0x2f0020000
#define DDR_ARB_REG_BASE     0x2f0080000
#define DDR_ARB2_REG_BASE    0x2f0090000
#define RELEASE_ADDR    0x2f0160000
#define DEVICE_START_ADDR   0x2f0000000
#define DEVICE_END_ADDR     0x2ffff0000

/* UART configuration */
#define UART0_BAUDRATE      115200

#if defined(_NVT_FPGA_EMULATION_ON_)
#define UART0_CLK_IN_HZ     24000000
#define OSC_CLK_IN_HZ       24000000
#else
#define UART0_CLK_IN_HZ     48000000
#define OSC_CLK_IN_HZ       12000000
#endif

#define REG_ADDR_REMAP      0xFFE41014
#define REG_RELEASE_ADDR    0x2F016010C

/* WDT related*/
#define ATF_CG_ENABLE_OFS 0x7C
#define ATF_CG_RESET_OFS 0x9C
#define ATF_WDT_RST 12
#define ATF_WDT_POS 4
#define MAN_RST_OFS 0xC
#endif /* NOVATEK_MMAP_H */
