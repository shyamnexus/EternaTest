/*
 * MSIX header for Novatek RC/EP interworking
 *
 * Copyright (c) 2021 - Novatek Microelectronics Corp.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __IRQ_NOVATEK_MSIX_V1_H
#define __IRQ_NOVATEK_MSIX_V1_H

/* MSIX controller registers */

typedef enum {

	MSIx_SEL_TYPE_PULSE_GEN = 0x0,
	MSIx_SEL_TYPE_AXICMD,

	MSIx_SEL_TYPE_CNT,
} MSIx_SEL_TYPE;

// AXI command ADDR[31..0]
#define AXI_CMD_ADDR_LSB_OFS  0x08

// AXI command ADDR[63..32]
#define AXI_CMD_ADDR_MSB_OFS  0x0C
#define AXI_CMD_ADDR_MSB_MSK	GENMASK(3, 0)
#define MSI_SEL_MSK		GENMASK(31, 31)

// 2nd AXI command ADDR[31..0]
#define AXI_CMD_MAU_ADDR_LSB_OFS  0x70

// 2nd AXI command ADDR[63..32]
#define AXI_CMD_MAU_ADDR_MSB_OFS  0x74

// AXI command ADDR[31..0]
#define AXI_CMD_MAU_DATA_OFS  	0x78

#define MSIx_SYS_INT_SET_EN0_OFS  	0x10
#define MSIx_SYS_INT_SET_EN1_OFS  	0x14
#define MSIx_SYS_INT_SET_EN2_OFS  	0x18
#define MSIx_SYS_INT_SET_EN3_OFS  	0x1C

#define MSIx_SYS_INT_SET_CLR0_OFS  	0x20
#define MSIx_SYS_INT_SET_CLR1_OFS  	0x24
#define MSIx_SYS_INT_SET_CLR2_OFS  	0x28
#define MSIx_SYS_INT_SET_CLR3_OFS  	0x2C

#define MSIx_MSIx_INT_SET_EN0_OFS  	0x40
#define MSIx_MSIx_INT_SET_EN1_OFS  	0x44
#define MSIx_MSIx_INT_SET_EN2_OFS  	0x48
#define MSIx_MSIx_INT_SET_EN3_OFS  	0x4C

#define MSIx_MSIx_INT_SET_CLR0_OFS  0x50
#define MSIx_MSIx_INT_SET_CLR1_OFS  0x54
#define MSIx_MSIx_INT_SET_CLR2_OFS  0x58
#define MSIx_MSIx_INT_SET_CLR3_OFS  0x5C

#define MSIx_MSIx_INT_STS0_OFS  0x60
#define MSIx_MSIx_INT_STS1_OFS  0x64
#define MSIx_MSIx_INT_STS2_OFS  0x68
#define MSIx_MSIx_INT_STS3_OFS  0x6C

#define MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_EN_OFS		0x100
#define MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_EN_OFS		0x200
#define MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_SET_PENDING_OFS	0x300
#define MSIx_RCV_AXI_CMD_Group_0_BIT_0_31_CLR_PENDING_OFS	0x400


#endif
