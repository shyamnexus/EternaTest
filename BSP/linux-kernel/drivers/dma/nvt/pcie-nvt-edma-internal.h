/*
   NVT PCIE eDMA register header

   @file      pcie-nvt-edma-internal.h 
   @ingroup
   @note
   Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.
*/

#define REGVALUE                uint32_t

#define UBITFIELD		unsigned int 	/* Unsigned bit field */
#define BITFIELD 		signed int	/* Signed bit field */
/**
    @name Assert macros
*/
//@{
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)
#if defined(__COUNTER__)

#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __COUNTER__) = 1/(expr) }

#else

// This might cause compile error when writing STATIC_ASSERT at the same line
// in two (or more) files and one file include another one.
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __LINE__) = 1/(expr) }

#endif

//
// Macros for Register Cache Word (RCW) type definition
//
// Each RCW type should be exactly the same size with REGVALUE type
// For example, to declare a Register Cache Word type:
//
//     #define rcwname_OFS   0x00   /* the name of RCW corresponding register address offset
//                                     should be in specific format with "_OFS" appended */
//     REGDEF_BEGIN(rcwname)
//         REGDEF_BIT(field1, 8)    /* declare field1 as 8 bits width */
//         REGDEF_BIT(field2, 8)    /* declare field1 as 8 bits width */
//         REGDEF_BIT(, 16)         /* pad reserved (not-used) bits to fill RCW type same as REGVALUE size */
//     REGDEF_END(rcwname)
//
// Register Cache Word type defintion header
#define REGDEF_BEGIN(name)      \
typedef union                   \
{                               \
    REGVALUE    reg;            \
    struct                      \
    {

// Register Cache Word bit defintion
#define REGDEF_BIT(field, bits) \
    UBITFIELD   field : bits;

// Register Cache Word type defintion trailer
#define REGDEF_END(name)        \
    } bit;                      \
} T_##name;                     \
STATIC_ASSERT(sizeof(T_##name) == sizeof(REGVALUE));

// Macro to define register offset
#define REGDEF_OFFSET(name, ofs)        static const unsigned long name##_OFS=(ofs);

//
// Macros for prerequisite stuff initialization for Register Cache Word
// operations
//
// Macro to set register base address for current module
// One must call this macro somewhere to set register I/O base address for
// current module, either globally or locally before any RCW operations is
// invoked
// What this macro set is only effective in current C file
#define REGDEF_SETIOBASE(base)          static const unsigned long _REGIOBASE=(base)

// Macro to set register repeat group offset address in byte
// This macro is optional and only used where register with repeat group exist,
// only one repeat group is allowed in one file
// If registers with repeat group exist, then one only have to define the RCW type
// for the first group, respective group of registers can be accessed through
// RCW_LD2, RCW_ST2, RCW_LD2OF macros
// What this macro set is only effective in current C file
#define REGDEF_SETGRPOFS(grpofs)        static const unsigned long _REGGRPOFS=(grpofs)

//
// Macros for Register Cache Word (RCW) operations
//
// These RCW operations are most likely to be used inside C functions
// It is not recommended to use RCW_DEF(RCW) in global scope which
// declare RCW as global variable
// RCW_LD/RCW_ST/RCW_LDOF pair is for normal single register access while
// RCW_LD2/RCW_ST2/RCW_LD2OF pair is for repeat group register access
// Beware that the RCW_LD/RCW_ST/RCW_LDOF and RCW_LD2/RCW_ST2/RCW_LD2OF
// must not be mixed used. e.g. use RCW_LD then RCW_ST2 or use RCW_LD2 then RCW_ST
//
// Register Cache Word declaration
#define RCW_DEF(RCW)            T##_##RCW t##RCW
// Register Cache Word read from I/O
#define RCW_LD(RCW)             t##RCW.reg = INW(_REGIOBASE+RCW##_OFS)
// Register Cache Word repeat from I/O for group read
#define RCW_LD2(RCW, grpidx)	t##RCW.reg = INW(_REGIOBASE+RCW##_OFS+(grpidx)*_REGGRPOFS)
// Register Cache Word bit reference
#define RCW_OF(RCW)             t##RCW.bit
// Register Cache Word reference
#define RCW_VAL(RCW)            t##RCW.reg
// Register Cache Word write to I/O
#define RCW_ST(RCW)             OUTW(_REGIOBASE+RCW##_OFS, t##RCW.reg)
// Register Cache Word write to I/O for repeat group
#define RCW_ST2(RCW, grpidx)	OUTW(_REGIOBASE+RCW##_OFS+(grpidx)*_REGGRPOFS, t##RCW.reg)
// Register Cache Word read and reference to register cache word bit
// !!!need to check for compiler if support for this syntax
#define RCW_LDOF(RCW)           ((T##_##RCW)(RCW_LD(RCW))).bit
// Register Cache Word read and reference to register cache word bit for repeat group
// !!!need to check for compiler if support for this syntax
#define RCW_LD2OF(RCW, grpidx)	((T##_##RCW)(RCW_LD2(RCW, grpidx))).bit

#define rcw_get_phy_addr(addr)	((addr) & 0x1FFFFFFF)





#define DBI_DMA_BASE			0x380000

#define DMA_WRITE_ENGINE_EN_OFF_OFS	(DBI_DMA_BASE + 0x0C)
REGDEF_BEGIN(DMA_WRITE_ENGINE_EN_OFF)
REGDEF_BIT(DMA_WRITE_ENGINE, 1)
REGDEF_BIT(, 31)
REGDEF_END(DMA_WRITE_ENGINE_EN_OFF)

#define DMA_WRITE_DOORBELL_OFF_OFS	(DBI_DMA_BASE + 0x10)
REGDEF_BEGIN(DMA_WRITE_DOORBELL_OFF)
REGDEF_BIT(WR_DOORBELL_NUM, 3)
REGDEF_BIT(, 28)
REGDEF_BIT(WR_STOP, 1)
REGDEF_END(DMA_WRITE_DOORBELL_OFF)

#define DMA_WRITE_CHAN_WEIGHT_LOW_OFF_OFS	(DBI_DMA_BASE + 0x18)
REGDEF_BEGIN(DMA_WRITE_CHAN_WEIGHT_LOW_OFF)
REGDEF_BIT(WR_CHAN_WEIGHT, 20)
REGDEF_BIT(, 12)
REGDEF_END(DMA_WRITE_CHAN_WEIGHT_LOW_OFF)

#define DMA_WRITE_CHAN_WEIGHT_HIGH_OFF_OFS	(DBI_DMA_BASE + 0x1C)
REGDEF_BEGIN(DMA_WRITE_CHAN_WEIGHT_HIGH_OFF)
REGDEF_BIT(WR_CHAN_WEIGHT, 20)
REGDEF_BIT(, 12)
REGDEF_END(DMA_WRITE_CHAN_WEIGHT_HIGH_OFF)

#define DMA_READ_ENGINE_EN_OFF_OFS	(DBI_DMA_BASE + 0x2C)
REGDEF_BEGIN(DMA_READ_ENGINE_EN_OFF)
REGDEF_BIT(DMA_READ_ENGINE, 1)
REGDEF_BIT(, 31)
REGDEF_END(DMA_READ_ENGINE_EN_OFF)

#define DMA_READ_DOORBELL_OFF_OFS	(DBI_DMA_BASE + 0x30)
REGDEF_BEGIN(DMA_READ_DOORBELL_OFF)
REGDEF_BIT(RD_DOORBELL_NUM, 3)
REGDEF_BIT(, 28)
REGDEF_BIT(RD_STOP, 1)
REGDEF_END(DMA_READ_DOORBELL_OFF)

#define DMA_READ_CHAN_WEIGHT_LOW_OFF_OFS	(DBI_DMA_BASE + 0x38)
REGDEF_BEGIN(DMA_READ_CHAN_WEIGHT_LOW_OFF)
REGDEF_BIT(RD_CHAN_WEIGHT, 20)
REGDEF_BIT(, 12)
REGDEF_END(DMA_READ_CHAN_WEIGHT_LOW_OFF)

#define DMA_READ_CHAN_WEIGHT_HIGH_OFF_OFS	(DBI_DMA_BASE + 0x3C)
REGDEF_BEGIN(DMA_READ_CHAN_WEIGHT_HIGH_OFF)
REGDEF_BIT(RD_CHAN_WEIGHT, 20)
REGDEF_BIT(, 12)
REGDEF_END(DMA_READ_CHAN_WEIGHT_HIGH_OFF)

#define DMA_WRITE_INT_STATUS_OFF_OFS	(DBI_DMA_BASE + 0x4C)
REGDEF_BEGIN(DMA_WRITE_INT_STATUS_OFF)
REGDEF_BIT(WR_DONE_INT_STATUS, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(WR_ABORT_INT_STATUS, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_WRITE_INT_STATUS_OFF)

#define DMA_WRITE_INT_MASK_OFF_OFS	(DBI_DMA_BASE + 0x54)
REGDEF_BEGIN(DMA_WRITE_INT_MASK_OFF)
REGDEF_BIT(WR_DONE_INT_MASK, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(WR_ABORT_INT_MASK, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_WRITE_INT_MASK_OFF)

#define DMA_WRITE_INT_CLEAR_OFF_OFS	(DBI_DMA_BASE + 0x58)
REGDEF_BEGIN(DMA_WRITE_INT_CLEAR_OFF)
REGDEF_BIT(WR_DONE_INT_CLEAR, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(WR_ABORT_INT_CLEAR, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_WRITE_INT_CLEAR_OFF)

#define DMA_WRITE_DONE_IMWR_LOW_OFF_OFS	(DBI_DMA_BASE + 0x60)
REGDEF_BEGIN(DMA_WRITE_DONE_IMWR_LOW_OFF)
REGDEF_BIT(DMA_WRITE_DONE_LOW_REG, 32)
REGDEF_END(DMA_WRITE_DONE_IMWR_LOW_OFF)

#define DMA_WRITE_DONE_IMWR_HIGH_OFF_OFS (DBI_DMA_BASE + 0x64)
REGDEF_BEGIN(DMA_WRITE_DONE_IMWR_HIGH_OFF)
REGDEF_BIT(DMA_WRITE_DONE_HIGH_REG, 32)
REGDEF_END(DMA_WRITE_DONE_IMWR_HIGH_OFF)

#define DMA_WRITE_CH01_IMWR_DATA_OFF_OFS	(DBI_DMA_BASE + 0x70)
REGDEF_BEGIN(DMA_WRITE_CH01_IMWR_DATA_OFF)
REGDEF_BIT(WR_CHANNEL_0_DATA, 16)
REGDEF_BIT(WR_CHANNEL_1_DATA, 16)
REGDEF_END(DMA_WRITE_CH01_IMWR_DATA_OFF)

#define DMA_WRITE_CH23_IMWR_DATA_OFF_OFS	(DBI_DMA_BASE + 0x74)
REGDEF_BEGIN(DMA_WRITE_CH23_IMWR_DATA_OFF)
REGDEF_BIT(WR_CHANNEL_2_DATA, 16)
REGDEF_BIT(WR_CHANNEL_3_DATA, 16)
REGDEF_END(DMA_WRITE_CH23_IMWR_DATA_OFF)

#define DMA_WRITE_CH45_IMWR_DATA_OFF_OFS	(DBI_DMA_BASE + 0x78)
REGDEF_BEGIN(DMA_WRITE_CH45_IMWR_DATA_OFF)
REGDEF_BIT(WR_CHANNEL_4_DATA, 16)
REGDEF_BIT(WR_CHANNEL_5_DATA, 16)
REGDEF_END(DMA_WRITE_CH45_IMWR_DATA_OFF)

#define DMA_WRITE_CH67_IMWR_DATA_OFF_OFS	(DBI_DMA_BASE + 0x7C)
REGDEF_BEGIN(DMA_WRITE_CH67_IMWR_DATA_OFF)
REGDEF_BIT(WR_CHANNEL_6_DATA, 16)
REGDEF_BIT(WR_CHANNEL_7_DATA, 16)
REGDEF_END(DMA_WRITE_CH67_IMWR_DATA_OFF)

#define DMA_READ_INT_STATUS_OFF_OFS	(DBI_DMA_BASE + 0xA0)
REGDEF_BEGIN(DMA_READ_INT_STATUS_OFF)
REGDEF_BIT(RD_DONE_INT_STATUS, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(RD_ABORT_INT_STATUS, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_READ_INT_STATUS_OFF)

#define DMA_READ_INT_MASK_OFF_OFS	(DBI_DMA_BASE + 0xA8)
REGDEF_BEGIN(DMA_READ_INT_MASK_OFF)
REGDEF_BIT(RD_DONE_INT_MASK, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(RD_ABORT_INT_MASK, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_READ_INT_MASK_OFF)

#define DMA_READ_INT_CLEAR_OFF_OFS	(DBI_DMA_BASE + 0xAC)
REGDEF_BEGIN(DMA_READ_INT_CLEAR_OFF)
REGDEF_BIT(RD_DONE_INT_CLEAR, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(RD_ABORT_INT_CLEAR, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_READ_INT_CLEAR_OFF)

#define DMA_CH_CONTROL1_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x200 + (i)*0x200)
REGDEF_BEGIN(DMA_CH_CONTROL1_OFF_WRCH)
REGDEF_BIT(CB, 1)
REGDEF_BIT(TCB, 1)
REGDEF_BIT(LLP, 1)
REGDEF_BIT(LIE, 1)
REGDEF_BIT(RIE, 1)
REGDEF_BIT(CS, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(CCS, 1)
REGDEF_BIT(LLE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_FUNC_NUM, 5)
REGDEF_BIT(, 5)
REGDEF_BIT(DMA_MEM_TYPE, 1)
REGDEF_BIT(DMA_NS_DST, 1)
REGDEF_BIT(DMA_NS_SRC, 1)
REGDEF_BIT(DMA_RO, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(DMA_TC, 3)
REGDEF_BIT(DMA_AT, 2)
REGDEF_END(DMA_CH_CONTROL1_OFF_WRCH)

#define DMA_TRANSFER_SIZE_OFF_WRCH_OFS(i)	(DBI_DMA_BASE + 0x208 + (i)*0x200)
REGDEF_BEGIN(DMA_TRANSFER_SIZE_OFF_WRCH)
REGDEF_BIT(DMA_TRANSFER_SIZE, 32)
REGDEF_END(DMA_TRANSFER_SIZE_OFF_WRCH)

#define DMA_SAR_LOW_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x20C + (i)*0x200)
REGDEF_BEGIN(DMA_SAR_LOW_OFF_WRCH)
REGDEF_BIT(SRC_ADDR_REG_LOW, 32)
REGDEF_END(DMA_SAR_LOW_OFF_WRCH)

#define DMA_SAR_HIGH_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x210 + (i)*0x200)
REGDEF_BEGIN(DMA_SAR_HIGH_OFF_WRCH)
REGDEF_BIT(SRC_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_SAR_HIGH_OFF_WRCH)

#define DMA_DAR_LOW_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x214 + (i)*0x200)
REGDEF_BEGIN(DMA_DAR_LOW_OFF_WRCH)
REGDEF_BIT(DST_ADDR_REG_LOW, 32)
REGDEF_END(DMA_DAR_LOW_OFF_WRCH)

#define DMA_DAR_HIGH_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x218 + (i)*0x200)
REGDEF_BEGIN(DMA_DAR_HIGH_OFF_WRCH)
REGDEF_BIT(DST_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_DAR_HIGH_OFF_WRCH)

#define DMA_LLP_LOW_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x21c + (i)*0x200)
REGDEF_BEGIN(DMA_LLP_LOW_OFF_WRCH)
REGDEF_BIT(LLP_ADDR_REG_LOW, 32)
REGDEF_END(DMA_LLP_LOW_OFF_WRCH)

#define DMA_LLP_HIGH_OFF_WRCH_OFS(i)		(DBI_DMA_BASE + 0x220 + (i)*0x200)
REGDEF_BEGIN(DMA_LLP_HIGH_OFF_WRCH)
REGDEF_BIT(LLP_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_LLP_HIGH_OFF_WRCH)

#define DMA_CH_CONTROL1_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x300 + (i)*0x200)
REGDEF_BEGIN(DMA_CH_CONTROL1_OFF_RDCH)
REGDEF_BIT(CB, 1)
REGDEF_BIT(TCB, 1)
REGDEF_BIT(LLP, 1)
REGDEF_BIT(LIE, 1)
REGDEF_BIT(RIE, 1)
REGDEF_BIT(CS, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(CCS, 1)
REGDEF_BIT(LLE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_FUNC_NUM, 5)
REGDEF_BIT(, 5)
REGDEF_BIT(DMA_MEM_TYPE, 1)
REGDEF_BIT(DMA_NS_DST, 1)
REGDEF_BIT(DMA_NS_SRC, 1)
REGDEF_BIT(DMA_RO, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(DMA_TC, 3)
REGDEF_BIT(DMA_AT, 2)
REGDEF_END(DMA_CH_CONTROL1_OFF_RDCH)

#define DMA_TRANSFER_SIZE_OFF_RDCH_OFS(i)	(DBI_DMA_BASE + 0x308 + (i)*0x200)
REGDEF_BEGIN(DMA_TRANSFER_SIZE_OFF_RDCH)
REGDEF_BIT(DMA_TRANSFER_SIZE, 32)
REGDEF_END(DMA_TRANSFER_SIZE_OFF_RDCH)

#define DMA_SAR_LOW_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x30C + (i)*0x200)
REGDEF_BEGIN(DMA_SAR_LOW_OFF_RDCH)
REGDEF_BIT(SRC_ADDR_REG_LOW, 32)
REGDEF_END(DMA_SAR_LOW_OFF_RDCH)

#define DMA_SAR_HIGH_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x310 + (i)*0x200)
REGDEF_BEGIN(DMA_SAR_HIGH_OFF_RDCH)
REGDEF_BIT(SRC_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_SAR_HIGH_OFF_RDCH)

#define DMA_DAR_LOW_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x314 + (i)*0x200)
REGDEF_BEGIN(DMA_DAR_LOW_OFF_RDCH)
REGDEF_BIT(DST_ADDR_REG_LOW, 32)
REGDEF_END(DMA_DAR_LOW_OFF_RDCH)

#define DMA_DAR_HIGH_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x318 + (i)*0x200)
REGDEF_BEGIN(DMA_DAR_HIGH_OFF_RDCH)
REGDEF_BIT(DST_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_DAR_HIGH_OFF_RDCH)

#define DMA_LLP_LOW_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x31c + (i)*0x200)
REGDEF_BEGIN(DMA_LLP_LOW_OFF_RDCH)
REGDEF_BIT(LLP_ADDR_REG_LOW, 32)
REGDEF_END(DMA_LLP_LOW_OFF_RDCH)

#define DMA_LLP_HIGH_OFF_RDCH_OFS(i)		(DBI_DMA_BASE + 0x320 + (i)*0x200)
REGDEF_BEGIN(DMA_LLP_HIGH_OFF_RDCH)
REGDEF_BIT(LLP_ADDR_REG_HIGH, 32)
REGDEF_END(DMA_LLP_HIGH_OFF_RDCH)

