/*
    Register offset and bit definition for MSIX module

    Register offset and bit definition for MSIX module.

    @file       msix_reg.h
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#ifndef _NVT_MSIX_REG_H
#define _NVT_MSIX_REG_H

#include <linux/soc/nvt/rcw_macro.h>

/*
    @addtogroup miDrvTimer_WDT
*/
//@{

// Interrupt latch ID Register
#define INTR_LATCH_ID_OFS	 0x00
REGDEF_BEGIN(INTR_LATCH_ID)                  // --> Register "INTR_LATCH_ID" begin ---
REGDEF_BIT(SYS_INTR_ID, 7)
REGDEF_BIT(, 1)
REGDEF_BIT(LAST_SYS_INTR_ID, 7)
REGDEF_BIT(, 1)
REGDEF_BIT(MSI_INTR_ID, 7)
REGDEF_BIT(, 9)
REGDEF_END(INTR_LATCH_ID)                    // --- Register "INTR_LATCH_ID" end   <--

// AXI command ADDR[31..0]
#define AXI_CMD_ADDR_LSB_OFS  0x08
REGDEF_BEGIN(AXI_CMD_ADDR_LSB)               // --> Register "AXI_CMD_ADDR_LSB" begin ---
REGDEF_BIT(AXI_CMD_ADDR_LSB, 32)
REGDEF_END(AXI_CMD_ADDR_LSB)                 // --- Register "AXI_CMD_ADDR_LSB" end   <--


// AXI command ADDR[63..32]
#define AXI_CMD_ADDR_MSB_OFS  0x0C
REGDEF_BEGIN(AXI_CMD_ADDR_MSB)          	// --> Register "AXI_CMD_ADDR_MSB" begin ---
REGDEF_BIT(AXI_CMD_ADDR_MSB, 4)
REGDEF_BIT(, 27)
REGDEF_BIT(MSI_SEL, 1)
REGDEF_END(AXI_CMD_ADDR_MSB)            	// --- Register "AXI_CMD_ADDR_MSB" end   <--

// 2nd AXI command ADDR[31..0]
#define AXI_CMD_MAU_ADDR_LSB_OFS  0x70
REGDEF_BEGIN(AXI_CMD_MAU_ADDR_LSB)               // --> Register "AXI_CMD_ADDR_LSB" begin ---
REGDEF_BIT(AXI_CMD_ADDR_LSB, 32)
REGDEF_END(AXI_CMD_MAU_ADDR_LSB)                 // --- Register "AXI_CMD_ADDR_LSB" end   <--


// 2nd AXI command ADDR[63..32]
#define AXI_CMD_MAU_ADDR_MSB_OFS  0x74
REGDEF_BEGIN(AXI_CMD_MAU_ADDR_MSB)          	// --> Register "AXI_CMD_ADDR_MSB" begin ---
REGDEF_BIT(AXI_CMD_ADDR_MSB, 4)
REGDEF_BIT(, 27)
REGDEF_BIT(MSI_MAU_SEL, 1)
REGDEF_END(AXI_CMD_MAU_ADDR_MSB)

// AXI command ADDR[31..0]
#define AXI_CMD_MAU_DATA_OFS  	0x78
REGDEF_BEGIN(AXI_CMD_DATA)               	// --> Register "AXI_CMD_ADDR_LSB" begin ---
REGDEF_BIT(AXI_CMD_DATA, 32)
REGDEF_END(AXI_CMD_DATA)                 // --- Register "AXI_CMD_ADDR_LSB" end   <--

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

//@}

#endif
