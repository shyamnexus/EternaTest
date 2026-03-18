/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
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
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_STACK_SIZE		(0x1000 / PLATFORM_CORE_COUNT)

#define _BSP_NA51102_

/* Enable suspend to ram function */
#define LINUX_S3_SUPPORT

/* IP BASE */
#define UART0_BASE  0x2f0290000
#define GICD_BASE   0x2fff01000
#define GICC_BASE   0x2fff02000
#define TZPC_BASE   0x2f00c0000
#define WDT_BASE    0x2f0050000
#define CG_BASE     0x2f0020000
#define STBC_BASE   0x2f0580000
#define RTC_BASE    0x2f0060000
#define PWBC_BASE   0x2f00f0000

#define DDR_ARB_REG_BASE     0x2f00d0000
#define RELEASE_ADDR    0x2f0090000
#define DEVICE_START_ADDR   0x2f0000000
#define DEVICE_END_ADDR     0x2ffff0000
#define SRAM_BASE   0xf0e00000 /* use IFE */
#define SRAM_A64_BASE   0x2f0e00000 /* use IFE */
#define SRAM_SIZE   0x32000
#define S3_MAGIC_NUMBER       0x3939889
#define S3_MAGIC_ADDR         0x2f0e1f000
#define S3_DATA_PTR_ADDR      0x2f0e1f008
#define S3_MODE_ADDR          0x2f0e1f010
#define POWER_DOWN_MODE               0x1
#define POWER_KEEP_MODE               0x0
#define S3_ATF_DATA_PTR_ADDR  0x1fffff0


#define S3_CPU_CORE0_DATA_OFFSET    0x1000
#define S3_CPU_CORE1_DATA_OFFSET    0x2000
#define S3_CPU_CORE2_DATA_OFFSET    0x3000
#define S3_CPU_CORE3_DATA_OFFSET    0x4000

#define S3_DDR_DATA_OFFSET			0x5000

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
#define REG_RELEASE_ADDR    0x2F009010C

/* WDT related*/
#define ATF_CG_ENABLE_OFS 0x140
#define ATF_CG_RESET_OFS 0x9C
#define ATF_WDT_RST 4
#define ATF_WDT_POS 16
#define MAN_RST_OFS 0xC
#endif /* NOVATEK_MMAP_H */
