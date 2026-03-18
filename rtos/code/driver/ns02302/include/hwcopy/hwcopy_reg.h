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

#include <rcw_macro.h>


/**
     Hwcopy General Controller Register.

     Control all Fire, Interrupt and Debug.
     @note
*/

//This is a template for following operation register
REGDEF_BEGIN(HWCOPY_OPER_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Trigger Operation
REGDEF_BIT(LL_FIRE, 1)                    // LL Trigger Operation
REGDEF_BIT(SWRESET, 1)                    // Software Reset
REGDEF_BIT(, 29)                          // Reserved
REGDEF_END(HWCOPY_OPER_REG)


//0x00 CH0 Operation Register
#define HWCOPY0_OPER_REG_OFS                  0x00
REGDEF_BEGIN(HWCOPY0_OPER_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Channel 0 Trigger Operation
REGDEF_BIT(LL_FIRE, 1)                    // Channel 0 LL Trigger Operation
REGDEF_BIT(SWRESET, 1)                    // Channel 0 Software Reset
REGDEF_BIT(, 29)                          // Reserved
REGDEF_END(HWCOPY0_OPER_REG)

//0x04 CH1 Operation Register
#define HWCOPY1_OPER_REG_OFS                  0x04
REGDEF_BEGIN(HWCOPY1_OPER_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Channel 1 Trigger Operation
REGDEF_BIT(LL_FIRE, 1)                    // Channel 1 LL Trigger Operation
REGDEF_BIT(SWRESET, 1)                    // Channel 1 Software Reset
REGDEF_BIT(, 29)                          // Reserved
REGDEF_END(HWCOPY1_OPER_REG)

//0x08 CH2 Operation Register
#define HWCOPY2_OPER_REG_OFS                  0x08
REGDEF_BEGIN(HWCOPY2_OPER_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Channel 2 Trigger Operation
REGDEF_BIT(LL_FIRE, 1)                    // Channel 2 LL Trigger Operation
REGDEF_BIT(SWRESET, 1)                    // Channel 2 Software Reset
REGDEF_BIT(, 29)                          // Reserved
REGDEF_END(HWCOPY2_OPER_REG)

//0x0C CH3 Operation Register
#define HWCOPY3_OPER_REG_OFS                  0x0C
REGDEF_BEGIN(HWCOPY3_OPER_REG)
REGDEF_BIT(TRIG_OP, 1)                    // Channel 3 Trigger Operation
REGDEF_BIT(LL_FIRE, 1)                    // Channel 3 LL Trigger Operation
REGDEF_BIT(SWRESET, 1)                    // Channel 3 Software Reset
REGDEF_BIT(, 29)                          // Reserved
REGDEF_END(HWCOPY3_OPER_REG)

//0x10 Hardware Copy Overall Operation Register
#define HWCOPY_ALL_OPER_REG_OFS               0x10
REGDEF_BEGIN(HWCOPY_ALL_OPER_REG)
REGDEF_BIT(SWRESET, 1)                    // All Channel Software Reset
REGDEF_BIT(, 15)                          // Reserved
REGDEF_BIT(VERSION, 16)                   // HWCopy Version Code
REGDEF_END(HWCOPY_ALL_OPER_REG)

//0x14 Interrupt Enable Register (Cannot access by linked list)
#define HWCOPY_INTEN_REG_OFS                 0x14
REGDEF_BEGIN(HWCOPY_INTEN_REG)
REGDEF_BIT(HWCOPY_INT_EN, 4)              // HWCopy Int Enable
REGDEF_BIT(LLDONE_INT_EN, 4)              // LLDONE Int Enable
REGDEF_BIT(LLERROR_INT_EN, 4)             // LLERROR Int Enable
REGDEF_BIT(LLJOBEND_INT_EN, 4)            // LLJOBEND Int Enable
REGDEF_BIT(, 16)                          // Reserved
REGDEF_END(HWCOPY_INTEN_REG)

//0x18 Interrupt Statue Register (Cannot access by linked list)
#define HWCOPY_INT_REG_OFS                   0x18
REGDEF_BEGIN(HWCOPY_INT_REG)
REGDEF_BIT(HWCOPY_INT, 4)                 // HWCopy Int State
REGDEF_BIT(LLDONE_INT, 4)                 // LLDONE Int State
REGDEF_BIT(LLERROR_INT, 4)                // LLERROR Int State
REGDEF_BIT(LLJOBEND_INT, 4)               // LLJOBEND Int State
REGDEF_BIT(, 16)                          // Reserved
REGDEF_END(HWCOPY_INT_REG)

//0x1C DMA General Setting (Cannot access by linked list)
#define HWCOPY_DMA_REG_OFS                   0x1C
REGDEF_BEGIN(HWCOPY_DMA_REG)
REGDEF_BIT(, 1)                           // Reserved
REGDEF_BIT(DMA_DISABLE, 1)             
REGDEF_BIT(, 4)                           // Reserved    
REGDEF_BIT(DMA_READ_OUTSTANDING_MAX, 8)    
REGDEF_BIT(DMA_WRITE_OUTSTANDING_MAX, 8)   
REGDEF_BIT(DMA_IDLE_STATUS, 1)             
REGDEF_BIT(, 9)                           // Reserved
REGDEF_END(HWCOPY_DMA_REG)

//0x20 HWCOPY Debug Select (Cannot access by linked list)
#define HWCOPY_DBG_SEL_REG_OFS               0x20
REGDEF_BEGIN(HWCOPY_DBG_SEL_REG)
REGDEF_BIT(DEBUG_SEL, 12)                    
REGDEF_BIT(, 20)                           // Reserved
REGDEF_END(HWCOPY_DBG_SEL_REG)

//0x24 HWCOPY AXI STATUS (Cannot access by linked list)
#define HWCOPY_AXI_STA_REG_OFS               0x24
REGDEF_BEGIN(HWCOPY_AXI_STA_REG)
REGDEF_BIT(AXI_DBG_INFO, 32)                    
REGDEF_END(HWCOPY_AXI_STA_REG)

/**
     Hwcopy Channel Register.

     Control HWCOPY Function.
     @note
*/
//0x00 Operation Configuration Register
#define HWCOPY_CONFIG_REG_OFS                 0x00
REGDEF_BEGIN(HWCOPY_CONFIG_REG)
REGDEF_BIT(HWCOPY_MODE, 2)                // Operation Type: linear/block set,linear/block copy
REGDEF_BIT(DMA_BURST_LENGTH, 1)           // BURST LENGTH: 0 is 64 words, 1 is 32 words
REGDEF_BIT(DMA_ALWAYS_CHKB, 1)            // Check Bus Ack
REGDEF_BIT(, 28)                          // Reserved
REGDEF_END(HWCOPY_CONFIG_REG)

//0x04 Memory Source Address Register
#define HWCOPY_SRC_ADDR_REG_OFS               0x04
REGDEF_BEGIN(HWCOPY_SRC_ADDR_REG)
REGDEF_BIT(SRC_ADDR, 32)                  // Memory Source Address(byte align)
REGDEF_END(HWCOPY_SRC_ADDR_REG)

//0x08 Memory Source Address Register MSB
#define HWCOPY_SRC_ADDR_MSB_REG_OFS           0x08
REGDEF_BEGIN(HWCOPY_SRC_ADDR_MSB_REG)
REGDEF_BIT(SRC_ADDR_MSB, 4)               // Memory Source Address MSB
REGDEF_BIT(, 28)                          // Reserved
REGDEF_END(HWCOPY_SRC_ADDR_MSB_REG)

//0x14 Memory Destination Address Register
#define HWCOPY_DST_ADDR_REG_OFS               0x14
REGDEF_BEGIN(HWCOPY_DST_ADDR_REG)
REGDEF_BIT(DST_ADDR, 32)                  // Memory Destination Address(byte align)
REGDEF_END(HWCOPY_DST_ADDR_REG)

//0x08 Memory Destination Address Register MSB
#define HWCOPY_DST_ADDR_MSB_REG_OFS           0x18
REGDEF_BEGIN(HWCOPY_DST_ADDR_MSB_REG)
REGDEF_BIT(DST_ADDR_MSB, 4)               // Memory Destination Address MSB
REGDEF_BIT(, 28)                          // Reserved
REGDEF_END(HWCOPY_DST_ADDR_MSB_REG)

//0x20 Memory Source Line Offset Register
#define HWCOPY_SRC_LOFF_REG_OFS              0x20
REGDEF_BEGIN(HWCOPY_SRC_LOFF_REG)
REGDEF_BIT(SRC_LOFF, 17)                  // Memory Source Line Offset
REGDEF_BIT(, 15)                          // Reserved
REGDEF_END(HWCOPY_SRC_LOFF_REG)

//0x24 Memory Active Height Register
#define HWCOPY_LCNT_REG_OFS                   0x24
REGDEF_BEGIN(HWCOPY_LCNT_REG)
REGDEF_BIT(ACT_HEIGHT, 14)                // Active Height
REGDEF_BIT(, 18)                          // Reserved
REGDEF_END(HWCOPY_LCNT_REG)

//0x28 Memory Active Width Register
#define HWCOPY_XRGN_REG_OFS                   0x28
REGDEF_BEGIN(HWCOPY_XRGN_REG)
REGDEF_BIT(ACT_WIDTH, 14)                 // Active Width
REGDEF_BIT(, 18)                          // Reserved
REGDEF_END(HWCOPY_XRGN_REG)

//0x2C Memory Destination Line Offset Register
#define HWCOPY_DST_LOFF_REG_OFS              0x2C
REGDEF_BEGIN(HWCOPY_DST_LOFF_REG)
REGDEF_BIT(DST_LOFF, 17)                  // Memory Destination Line Offset
REGDEF_BIT(, 15)                          // Reserved
REGDEF_END(HWCOPY_DST_LOFF_REG)

//0x30 Data Length Register
#define HWCOPY_DATA_LENGTH_REG_OFS            0x30
REGDEF_BEGIN(HWCOPY_DATA_LENGTH_REG)
REGDEF_BIT(DATA_LENGTH, 26)               // Data Length
REGDEF_BIT(, 6)                           // Reserved
REGDEF_END(HWCOPY_DATA_LENGTH_REG)

//0x34 Constant Value Register
#define HWCOPY_CONST_REG_OFS                  0x34
REGDEF_BEGIN(HWCOPY_CONST_REG)
REGDEF_BIT(CTEX, 32)                      // Constant Value
REGDEF_END(HWCOPY_CONST_REG)

/**
     Linked List Register.

     As Title.
     @note
*/
//0x50 Linked List Control Register
#define LL_CTRL_REG_OFS                       0x50
REGDEF_BEGIN(LL_CTRL_REG)
REGDEF_BIT(LL_TERMINATE, 1)              // Terminate Linked List(LL will not stop until job done)
REGDEF_BIT(LL_CKSUM_ENABLE, 1)           
REGDEF_BIT(, 30)                         // Reserved
REGDEF_END(LL_CTRL_REG)

//0x54 Linked List Start Address Register
#define LL_START_ADDR_REG_OFS                 0x54
REGDEF_BEGIN(LL_START_ADDR_REG)
REGDEF_BIT(LL_START_ADDR, 32)            // Start Linked List in Specification DRAM Address
REGDEF_END(LL_START_ADDR_REG)

//0x58 Linked List Start Address Register MSB
#define LL_START_ADDR_MSB_REG_OFS             0x58
REGDEF_BEGIN(LL_START_ADDR_MSB_REG)
REGDEF_BIT(LL_START_ADDR_MSB, 4)         // Linked List Start Address MSB
REGDEF_BIT(, 28)                         // Reserved
REGDEF_END(LL_START_ADDR_MSB_REG)

//0x60 Linked List Debug Register1
#define LL_DBG1_REG_OFS                       0x60
REGDEF_BEGIN(LL_DBG1_REG)
REGDEF_BIT(LL_CMD_START_ADDR_INFO, 32)        
REGDEF_END(LL_DBG1_REG)

//0x64 Linked List Debug Register2
#define LL_DBG2_REG_OFS                       0x64
REGDEF_BEGIN(LL_DBG2_REG)
REGDEF_BIT(LL_CMDPRS_CNT, 32)        
REGDEF_END(LL_DBG2_REG)

//0x68 Linked List Table Index Register 0
#define LL_TABLE0_REG_OFS                     0x68
REGDEF_BEGIN(LL_TABLE0_REG)
REGDEF_BIT(LL_TABLE_INDEX0, 8)
REGDEF_BIT(LL_TABLE_INDEX1, 8)  
REGDEF_BIT(LL_TABLE_INDEX2, 8)  
REGDEF_BIT(LL_TABLE_INDEX3, 8)          
REGDEF_END(LL_TABLE0_REG)

//0x6C Linked List Table Index Register 1
#define LL_TABLE1_REG_OFS                     0x6C
REGDEF_BEGIN(LL_TABLE1_REG)
REGDEF_BIT(LL_TABLE_INDEX4, 8)
REGDEF_BIT(LL_TABLE_INDEX5, 8)  
REGDEF_BIT(LL_TABLE_INDEX6, 8)  
REGDEF_BIT(LL_TABLE_INDEX7, 8)          
REGDEF_END(LL_TABLE1_REG)

//0x70 Linked List Table Index Register 2
#define LL_TABLE2_REG_OFS                     0x70
REGDEF_BEGIN(LL_TABLE2_REG)
REGDEF_BIT(LL_TABLE_INDEX8, 8)
REGDEF_BIT(LL_TABLE_INDEX9, 8)  
REGDEF_BIT(LL_TABLE_INDEX10, 8)  
REGDEF_BIT(LL_TABLE_INDEX11, 8)          
REGDEF_END(LL_TABLE2_REG)

//0x74 Linked List Table Index Register 3
#define LL_TABLE3_REG_OFS                     0x74
REGDEF_BEGIN(LL_TABLE3_REG)
REGDEF_BIT(LL_TABLE_INDEX12, 8)
REGDEF_BIT(LL_TABLE_INDEX13, 8)  
REGDEF_BIT(LL_TABLE_INDEX14, 8)  
REGDEF_BIT(LL_TABLE_INDEX15, 8)          
REGDEF_END(LL_TABLE3_REG)

//0x78 Linked List Check Sum Register
#define LL_CHKSUM_REG_OFS                     0x74
REGDEF_BEGIN(LL_CHKSUM_REG)
REGDEF_BIT(LL_CKSUM, 32)    
REGDEF_END(LL_CHKSUM_REG)

#endif
