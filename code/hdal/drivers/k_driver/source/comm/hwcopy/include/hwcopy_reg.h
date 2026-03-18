/**
    HwCopy module register header

    HwCopy module register header

    @file       hwcopy_reg.h
    @ingroup    mIDrvIPP_Hwcopy
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef  _HWCOPY_REG_H
#define  _HWCOPY_REG_H

#include "hwcopy_platform.h"

/**
     Hwcopy mode.

     Hwcopy mode.
     @note
*/
typedef enum {
	HWCOPY_MODE_SET_LINEAR,
	HWCOPY_MODE_SET_BLOCK,
	HWCOPY_MODE_COPY_LINEAR,
	HWCOPY_MODE_COPY_BLOCK,

	ENUM_DUMMY4WORD(HWCOPY_MODE_ENUM)
} HWCOPY_MODE_ENUM;


//0x00 Operation Control Register
#define HWCOPY_CTRL_REG_OFS                   0x00
REGDEF_BEGIN(HWCOPY_CTRL_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Trigger Operation
REGDEF_BIT(, 31)                          // Reserved
REGDEF_END(HWCOPY_CTRL_REG)

//0x04 Operation Configuration Register
#define HWCOPY_CONFIG_REG_OFS                 0x04
REGDEF_BEGIN(HWCOPY_CONFIG_REG)
REGDEF_BIT(HWCOPY_MODE, 2)                // Operation Type: linear/block set,linear/block copy
REGDEF_BIT(, 2)				  // Reserved
REGDEF_BIT(DMA_BURST_LEN, 1)		  // BURST LENGTH
REGDEF_BIT(DMA_CHKB, 1)			  // check bus aclk
REGDEF_BIT(DMA_DIS, 1) 			  // DMA disable
REGDEF_BIT(,1);				  // Reserved
REGDEF_BIT(DMA_READ_EN, 1)		  // DMA READ
REGDEF_BIT(DMA_WRITE_EN, 1)               // DMA WRITE
REGDEF_BIT(DMA_READ_LOCK_DIS, 1)	  // DMA READ LOCK DISABLE
REGDEF_BIT(DMA_WRITE_LOCK_DIS, 1)	  // DMA WRITE LOCK DISABLE
REGDEF_BIT(DMA_READ_OUTSTD, 8)		  // DMA read outstanding max
REGDEF_BIT(DMA_WRITE_OUTSTD, 8)		  // DMA write outstanding max
REGDEF_BIT(, 4)                          // Reserved
REGDEF_END(HWCOPY_CONFIG_REG)

//0x08 Interrupt Enable Register
#define HWCOPY_INTEN_REG_OFS                  0x08
REGDEF_BEGIN(HWCOPY_INTEN_REG)
REGDEF_BIT(HWCOPY_INT_EN, 1)              // HwCopy Interrupt Enable
REGDEF_BIT(, 31)                          // Reserved
REGDEF_END(HWCOPY_INTEN_REG)

//0x0C Interrupt Status Register
#define HWCOPY_INTSTS_REG_OFS                 0x0C
REGDEF_BEGIN(HWCOPY_INTSTS_REG)
REGDEF_BIT(HWCOPY_INT, 1)                 // HwCopy Interrupt Status
REGDEF_BIT(, 31)                          // Reserved
REGDEF_END(HWCOPY_INTSTS_REG)

/*
//0x10 Memory Buffer-A Address Register
#define HWCOPY_MEMA_ADDR_REG_OFS              0x10
REGDEF_BEGIN(HWCOPY_MEMA_ADDR_REG)
REGDEF_BIT(MEMA_SADDR, 31)                 // Memory Buffer-A Address(byte align)
REGDEF_BIT(, 1)                            // Reserved
REGDEF_END(HWCOPY_MEMA_ADDR_REG)
*/

//0x14 Memory Buffer-A Line Offset Register
#define HWCOPY_MEMA_LOFF_REG_OFS              0x14
REGDEF_BEGIN(HWCOPY_MEMA_LOFF_REG)
REGDEF_BIT(MEMA_LOFF, 17)                 // Memory Buffer-A Line Offset
REGDEF_BIT(, 15)                          // Reserved
REGDEF_END(HWCOPY_MEMA_LOFF_REG)


//0x18 Memory Active Height Register
#define HWCOPY_LCNT_REG_OFS                   0x18
REGDEF_BEGIN(HWCOPY_LCNT_REG)
REGDEF_BIT(ACT_HEIGHT, 14)                // Active Height
REGDEF_BIT(, 18)                          // Reserved
REGDEF_END(HWCOPY_LCNT_REG)

//0x1C Memory Active Width Register
#define HWCOPY_XRGN_REG_OFS                   0x1C
REGDEF_BEGIN(HWCOPY_XRGN_REG)
REGDEF_BIT(ACT_WIDTH, 14)                 // Active Width
REGDEF_BIT(, 18)                          // Reserved
REGDEF_END(HWCOPY_XRGN_REG)

/*
//0x20 Memory Buffer-B Address Register
#define HWCOPY_MEMB_ADDR_REG_OFS              0x20
REGDEF_BEGIN(HWCOPY_MEMB_ADDR_REG)
REGDEF_BIT(MEMB_SADDR, 31)                // Memory Buffer-B Address(byte align)
REGDEF_BIT(, 1)                           // Reserved
REGDEF_END(HWCOPY_MEMB_ADDR_REG)
*/

//0x24 Memory Buffer-B Line Offset Register
#define HWCOPY_MEMB_LOFF_REG_OFS              0x24
REGDEF_BEGIN(HWCOPY_MEMB_LOFF_REG)
REGDEF_BIT(MEMB_LOFF, 17)                 // Memory Buffer-B Line Offset
REGDEF_BIT(, 15)                          // Reserved
REGDEF_END(HWCOPY_MEMB_LOFF_REG)

//0x28 Data Length Register
#define HWCOPY_DATA_LENGTH_REG_OFS            0x28
REGDEF_BEGIN(HWCOPY_DATA_LENGTH_REG)
REGDEF_BIT(DATA_LENGTH, 26)               // Data Length
REGDEF_BIT(, 6)                           // Reserved
REGDEF_END(HWCOPY_DATA_LENGTH_REG)

//0x2C Constant Value Register
#define HWCOPY_CONST_REG_OFS                  0x2C
REGDEF_BEGIN(HWCOPY_CONST_REG)
REGDEF_BIT(CTEX, 32)                     // Constant Value
REGDEF_END(HWCOPY_CONST_REG)

//0x40  Memory Buffer-A Address LSB Register Register
#define HWCOPY_MEMA_ADDR_LSB_REG_OFS                  0x40
REGDEF_BEGIN(HWCOPY_MEMA_ADDR_LSB_REG)
REGDEF_BIT(MEMA_SADDR_LSB, 32)                     // Memory Buffer-A LSB Address(byte align)
REGDEF_END(HWCOPY_MEMA_ADDR_LSB_REG)

//0x44  Memory Buffer-A Address LSB Register Register
#define HWCOPY_MEMA_ADDR_MSB_REG_OFS                  0x44
REGDEF_BEGIN(HWCOPY_MEMA_ADDR_MSB_REG)
REGDEF_BIT(MEMA_SDDR_MSB, 4)                     // Constant Value
REGDEF_BIT(, 28)                                 // Reserved
REGDEF_END(HWCOPY_MEMA_ADDR_MSB_REG)

//0x48  Memory Buffer-B Address LSB Register Register
#define HWCOPY_MEMB_ADDR_LSB_REG_OFS                  0x48
REGDEF_BEGIN(HWCOPY_MEMB_ADDR_LSB_REG)
REGDEF_BIT(MEMB_SADDR_LSB, 32)                     // Memory Buffer-A LSB Address(byte align)
REGDEF_END(HWCOPY_MEMB_ADDR_LSB_REG)

//0x44  Memory Buffer-B Address LSB Register Register
#define HWCOPY_MEMB_ADDR_MSB_REG_OFS                  0x4C
REGDEF_BEGIN(HWCOPY_MEMB_ADDR_MSB_REG)
REGDEF_BIT(MEMB_SDDR_MSB, 4)                     // Constant Value
REGDEF_BIT(, 28)                                 // Reserved
REGDEF_END(HWCOPY_MEMB_ADDR_MSB_REG)

#endif
