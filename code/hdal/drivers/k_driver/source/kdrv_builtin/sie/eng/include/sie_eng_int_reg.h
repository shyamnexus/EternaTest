/*
    Internal header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_ENG_INT_REG_H_
#define _SIE_ENG_INT_REG_H_

#include "kwrap/type.h"
#include "rcw_macro.h"

/*************************************************************
 * [0000] SIE_REG_0000
 *************************************************************/
#define SIE_REG_0000_OFS    0x0000
REGDEF_BEGIN(SIE_REG_0000)
    REGDEF_BIT(SIE_SW_RST,            1)
    REGDEF_BIT(SIE_LOAD,              1)
    REGDEF_BIT(SIE_ACT_EN,            1)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(DIRECT_TO_IFE,         1)
    REGDEF_BIT(SIE_LOAD_FROM_SIEX_EN, 4)
    REGDEF_BIT(SIE_RAW_SEL,           1)
    REGDEF_BIT(VD_PHASE,              1)
    REGDEF_BIT(HD_PHASE,              1)
    REGDEF_BIT(DATA_PHASE,            1)
    REGDEF_BIT(VD_INV,                1)
    REGDEF_BIT(HD_INV,                1)
    REGDEF_BIT(DRAM_IN_START,         1)
    REGDEF_BIT(SERIAL_MAIN_IN_SEL,    3)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(PARAL_VD_SEL,          1)
    REGDEF_BIT(MAIN_IN_SEL,           2)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(SIE_DMACH_DIS,         1)
    REGDEF_BIT(SIE_DMACH_IDLE,        1)
    REGDEF_BIT(SIE_LOAD_CLR,          1)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(SIE_SERIAL_RAW_SEL,    2)
REGDEF_END(SIE_REG_0000)

/*************************************************************
 * [0004] SIE_REG_0004
 *************************************************************/
#define SIE_REG_0004_OFS    0x0004
REGDEF_BEGIN(SIE_REG_0004)
    REGDEF_BIT(,                1)
    REGDEF_BIT(PATGEN_EN,       1)
    REGDEF_BIT(DVI_EN,          1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(OB_BYPASS_EN,    1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(DVS_DECOMP_EN,   1)
    REGDEF_BIT(DPC_EN,          1)
    REGDEF_BIT(COLDEF_EN,       1)
    REGDEF_BIT(ACT_DATAEXT_EN,  1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(ECS_EN,          1)
    REGDEF_BIT(DGAIN_EN,        1)
    REGDEF_BIT(BS_H_EN,         1)
    REGDEF_BIT(BS_V_EN,         1)
    REGDEF_BIT(RAWENC_EN,       1)
    REGDEF_BIT(CGAIN_EN,        1)
    REGDEF_BIT(,                1)
    REGDEF_BIT(STCS_HISTO_Y_EN, 1)
    REGDEF_BIT(STCS_LA_EN,      1)
    REGDEF_BIT(STCS_CA_EN,      1)
    REGDEF_BIT(STCS_DGAIN_EN,   1)
    REGDEF_BIT(EVS_EN,          1)
    REGDEF_BIT(DRAM_OUT0_EN,    1)
    REGDEF_BIT(COMPANDING_EN,   1)
    REGDEF_BIT(BAYER_FORMAT,    2)
    REGDEF_BIT(DBG_EN,          1)
REGDEF_END(SIE_REG_0004)

/*************************************************************
 * [0008] SIE_REG_0008
 *************************************************************/
#define SIE_REG_0008_OFS    0x0008
REGDEF_BEGIN(SIE_REG_0008)
    REGDEF_BIT(INTE_VD,                       1)
    REGDEF_BIT(INTE_BP1,                      1)
    REGDEF_BIT(INTE_BP2,                      1)
    REGDEF_BIT(INTE_BP3,                      1)
    REGDEF_BIT(INTE_ACTST,                    1)
    REGDEF_BIT(INTE_CRPST,                    1)
    REGDEF_BIT(INTE_DRAM_IN_OUT_ERR,          1)
    REGDEF_BIT(INTE_DRAM_OUT0_END,            1)
    REGDEF_BIT(INTE_DRAM_OUT1_END,            1)
    REGDEF_BIT(INTE_DRAM_OUT2_END,            1)
    REGDEF_BIT(,                              1)
    REGDEF_BIT(INTE_DRAM_DBGOUT_END,          1)
    REGDEF_BIT(,                              1)
    REGDEF_BIT(INTE_DPCF,                     1)
    REGDEF_BIT(INTE_SIECLK_ERR,               1)
    REGDEF_BIT(INTE_RAWENC_OVFL,              1)
    REGDEF_BIT(INTE_ACTEND,                   1)
    REGDEF_BIT(INTE_CRPEND,                   1)
    REGDEF_BIT(INTE_SIE_BEHAVIOR_ERR,         1)
    REGDEF_BIT(,                              1)
    REGDEF_BIT(INTE_HISTO_END,                1)
    REGDEF_BIT(,                              1)
    REGDEF_BIT(,                              1)
    REGDEF_BIT(INTE_RAWENC_DATA_OVLAP,        1)
    REGDEF_BIT(INTE_RAWENC_DATA_BEHAVIOR_ERR, 1)
    REGDEF_BIT(INTE_TSEN_OOC_VD,              1)
    REGDEF_BIT(INTE_TSEN_END,                 1)
    REGDEF_BIT(INTE_TSEN_DRAM_IN3_UDFL,       1)
    REGDEF_BIT(INTE_TSEN_FIFO_OVFL,           1)
    REGDEF_BIT(INTE_TSEN_CONFIG_END,          1)
REGDEF_END(SIE_REG_0008)

/*************************************************************
 * [000C] SIE_REG_000C
 *************************************************************/
#define SIE_REG_000C_OFS    0x000C
REGDEF_BEGIN(SIE_REG_000C)
    REGDEF_BIT(INT_VD,                       1)
    REGDEF_BIT(INT_BP1,                      1)
    REGDEF_BIT(INT_BP2,                      1)
    REGDEF_BIT(INT_BP3,                      1)
    REGDEF_BIT(INT_ACTST,                    1)
    REGDEF_BIT(INT_CRPST,                    1)
    REGDEF_BIT(INT_DRAM_IN_OUT_ERR,          1)
    REGDEF_BIT(INT_DRAM_OUT0_END,            1)
    REGDEF_BIT(INT_DRAM_OUT1_END,            1)
    REGDEF_BIT(INT_DRAM_OUT2_END,            1)
    REGDEF_BIT(,                             1)
    REGDEF_BIT(INT_DRAM_DBGOUT_END,          1)
    REGDEF_BIT(,                             1)
    REGDEF_BIT(INT_DPCF,                     1)
    REGDEF_BIT(INT_SIECLK_ERR,               1)
    REGDEF_BIT(INT_RAWENC_OVFL,              1)
    REGDEF_BIT(INT_ACTEND,                   1)
    REGDEF_BIT(INT_CRPEND,                   1)
    REGDEF_BIT(INT_SIE_BEHAVIOR_ERR,         1)
    REGDEF_BIT(,                             1)
    REGDEF_BIT(INT_HISTO_END,                1)
    REGDEF_BIT(,                             1)
    REGDEF_BIT(,                             1)
    REGDEF_BIT(INT_RAWENC_DATA_OVLAP,        1)
    REGDEF_BIT(INT_RAWENC_DATA_BEHAVIOR_ERR, 1)
    REGDEF_BIT(INT_TSEN_OOC_VD,              1)
    REGDEF_BIT(INT_TSEN_END,                 1)
    REGDEF_BIT(INT_TSEN_DRAM_IN3_UDFL,       1)
    REGDEF_BIT(INT_TSEN_FIFO_OVFL,           1)
    REGDEF_BIT(INT_TSEN_TX_CONFIG_END,       1)
REGDEF_END(SIE_REG_000C)

/*************************************************************
 * [0010] SIE_REG_0010
 *************************************************************/
#define SIE_REG_0010_OFS   0x0010
REGDEF_BEGIN(SIE_REG_0010)
    REGDEF_BIT(PX_MAX_CNT,                 14)
    REGDEF_BIT(,                           2)
    REGDEF_BIT(VSYNC,                      1)
    REGDEF_BIT(HSYNC,                      1)
    REGDEF_BIT(,                           6)
    REGDEF_BIT(DRAM_IN1_UDFL,              1)
    REGDEF_BIT(DRAM_IN2_UDFL,              1)
    REGDEF_BIT(DRAM_OUT0_LINEBUF_OVFL,     1)
    REGDEF_BIT(DRAM_OUT1_LINEBUF_OVFL,     1)
    REGDEF_BIT(DRAM_OUT2_LINEBUF_OVFL,     1)
    REGDEF_BIT(,                           1)
    REGDEF_BIT(DRAM_OUT0_LINEBUF_SIZE_ERR, 1)
REGDEF_END(SIE_REG_0010)

/*************************************************************
 * [0018] SIE_REG_0018
 *************************************************************/
#define SIE_REG_0018_OFS    0x0018
REGDEF_BEGIN(SIE_REG_0018)
    REGDEF_BIT(SIE_DEBUG_PORT,       4)
    REGDEF_BIT(SIE_DEBUG_EN,         1)
    REGDEF_BIT(SIE_DEBUG_PORT_SEL,   1)
    REGDEF_BIT(,                     3)
    REGDEF_BIT(SIE_PXCNT_INIT_VALUE, 1)
    REGDEF_BIT(SIE_MCLK_INV,         1)
    REGDEF_BIT(SIE_GL_LOAD_SEL,      1)
    REGDEF_BIT(SIE_PXCLK_DETECT,     1)
    REGDEF_BIT(SIE_CLK_DETECT,       1)
    REGDEF_BIT(SIE_TX_CLK_DETECT,    1)
    REGDEF_BIT(SIE_RX_CLK_DETECT,    1)
    REGDEF_BIT(SIE_RTL_VERSION,     16)
REGDEF_END(SIE_REG_0018)

/*************************************************************
 * [001C] SIE_REG_001C
 *************************************************************/
#define SIE_REG_001C_OFS   0x001C
REGDEF_BEGIN(SIE_REG_001C)
    REGDEF_BIT(SRC_WIDTH,  16)
    REGDEF_BIT(SRC_HEIGHT, 16)
REGDEF_END(SIE_REG_001C)

/*************************************************************
 * [0020] SIE_REG_0020
 *************************************************************/
#define SIE_REG_0020_OFS    0x0020
REGDEF_BEGIN(SIE_REG_0020)
    REGDEF_BIT(BP1, 16)
    REGDEF_BIT(BP2, 16)
REGDEF_END(SIE_REG_0020)

/*************************************************************
 * [0024] SIE_REG_0024
 *************************************************************/
#define SIE_REG_0024_OFS    0x0024
REGDEF_BEGIN(SIE_REG_0024)
    REGDEF_BIT(BP3, 16)
REGDEF_END(SIE_REG_0024)

/*************************************************************
 * [0028] SIE_REG_0028
 *************************************************************/
#define SIE_REG_0028_OFS    0x0028
REGDEF_BEGIN(SIE_REG_0028)
    REGDEF_BIT(ACT_STX, 16)
    REGDEF_BIT(ACT_STY, 16)
REGDEF_END(SIE_REG_0028)

/*************************************************************
 * [002C] SIE_REG_002C
 *************************************************************/
#define SIE_REG_002C_OFS   0x002C
REGDEF_BEGIN(SIE_REG_002C)
    REGDEF_BIT(ACT_SZX, 16)
    REGDEF_BIT(ACT_SZY, 16)
REGDEF_END(SIE_REG_002C)

/*************************************************************
 * [0030] SIE_REG_0030
 *************************************************************/
#define SIE_REG_0030_OFS  0x0030
REGDEF_BEGIN(SIE_REG_0030)
    REGDEF_BIT(ACT_CFAPAT,  3)
    REGDEF_BIT(,            1)
    REGDEF_BIT(CROP_CFAPAT, 3)
    REGDEF_BIT(,            1)
    REGDEF_BIT(CA_CFAPAT,   3)
    REGDEF_BIT(,            1)
    REGDEF_BIT(DPC_CFAPAT,  3)
REGDEF_END(SIE_REG_0030)

/*************************************************************
 * [0034] SIE_REG_0034
 *************************************************************/
#define SIE_REG_0034_OFS   0x0034
REGDEF_BEGIN(SIE_REG_0034)
    REGDEF_BIT(CRP_STX, 16)
    REGDEF_BIT(CRP_STY, 16)
REGDEF_END(SIE_REG_0034)

/*************************************************************
 * [0038] SIE_REG_0038
 *************************************************************/
#define SIE_REG_0038_OFS   0x0038
REGDEF_BEGIN(SIE_REG_0038)
    REGDEF_BIT(CRP_SZX, 16)
    REGDEF_BIT(CRP_SZY, 16)
REGDEF_END(SIE_REG_0038)

/*************************************************************
 * [003C] SIE_REG_003C
 *************************************************************/
#define SIE_REG_003C_OFS    0x003C
REGDEF_BEGIN(SIE_REG_003C)
    REGDEF_BIT(VD_DELAY,    2)
    REGDEF_BIT(HD_DELAY,    2)
    REGDEF_BIT(DELAY_CHAIN, 5)
REGDEF_END(SIE_REG_003C)

/*************************************************************
 * [0040] SIE_REG_0040
 *************************************************************/
#define SIE_REG_0040_OFS   0x0040
REGDEF_BEGIN(SIE_REG_0040)
    REGDEF_BIT(VD_CURRENT_CNT,     16)
    REGDEF_BIT(CRPEND_CURRENT_CNT, 16)
REGDEF_END(SIE_REG_0040)

/*************************************************************
 * [0044] SIE_REG_0044
 *************************************************************/
#define SIE_REG_0044_OFS    0x0044
REGDEF_BEGIN(SIE_REG_0044)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DRAM_IN1_SAI, 30)
REGDEF_END(SIE_REG_0044)

/*************************************************************
 * [0048] SIE_REG_0048
 *************************************************************/
#define SIE_REG_0048_OFS    0x0048
REGDEF_BEGIN(SIE_REG_0048)
    REGDEF_BIT(DRAM_IN1_SAI_2, 4)
REGDEF_END(SIE_REG_0048)

/*************************************************************
 * [004C] SIE_REG_004C
 *************************************************************/
#define SIE_REG_004C_OFS    0x004C
REGDEF_BEGIN(SIE_REG_004C)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(DRAM_IN1_OFSO,      14)
    REGDEF_BIT(DRAM_IN1_PACK_BUS ,  2)
REGDEF_END(SIE_REG_004C)

/*************************************************************
 * [0050] SIE_REG_0050
 *************************************************************/
#define SIE_REG_0050_OFS    0x0050
REGDEF_BEGIN(SIE_REG_0050)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DRAM_IN2_SAI, 30)
REGDEF_END(SIE_REG_0050)

/*************************************************************
 * [0054] SIE_REG_0054
 *************************************************************/
#define SIE_REG_0054_OFS    0x0054
REGDEF_BEGIN(SIE_REG_0054)
    REGDEF_BIT(DRAM_IN2_SAI_2, 4)
REGDEF_END(SIE_REG_0054)

/*************************************************************
 * [0058] SIE_REG_0058
 *************************************************************/
#define SIE_REG_0058_OFS    0x0058
REGDEF_BEGIN(SIE_REG_0058)
    REGDEF_BIT(,              2)
    REGDEF_BIT(DRAM_OUT0_SAO, 30)
REGDEF_END(SIE_REG_0058)

/*************************************************************
 * [005C] SIE_REG_005C
 *************************************************************/
#define SIE_REG_005C_OFS    0x005C
REGDEF_BEGIN(SIE_REG_005C)
    REGDEF_BIT(DRAM_OUT0_SAO_2, 4)
REGDEF_END(SIE_REG_005C)

/*************************************************************
 * [0060] SIE_REG_0060
 *************************************************************/
#define SIE_REG_0060_OFS    0x0060
REGDEF_BEGIN(SIE_REG_0060)
    REGDEF_BIT(,                           2)
    REGDEF_BIT(DRAM_OUT0_OFSO,            14)
    REGDEF_BIT(DRAM_OUT0_PACK_BUS,         2)
    REGDEF_BIT(DRAM_OUT0_HFLIP,            1)
    REGDEF_BIT(DRAM_OUT0_VFLIP,            1)
    REGDEF_BIT(DRAM_OUT0_RINGBUF_EN,       1)
    REGDEF_BIT(,                           7)
    REGDEF_BIT(DRAM_OUT0_BITDEPTH_SEL,     2)
    REGDEF_BIT(DRAM_OUT0_PACK_FORMAT,      1)
    REGDEF_BIT(DRAM_OUT0_BITDEPTH_SEL_MSB, 1)
REGDEF_END(SIE_REG_0060)

/*************************************************************
 * [0064] SIE_REG_0064
 *************************************************************/
#define SIE_REG_0064_OFS    0x0064
REGDEF_BEGIN(SIE_REG_0064)
    REGDEF_BIT(DRAM_OUT0_RINGBUF_LEN, 12)
REGDEF_END(SIE_REG_0064)

/*************************************************************
 * [0068] SIE_REG_0068
 *************************************************************/
#define SIE_REG_0068_OFS    0x0068
REGDEF_BEGIN(SIE_REG_0068)
    REGDEF_BIT(,              2)
    REGDEF_BIT(DRAM_OUT1_SAO, 30)
REGDEF_END(SIE_REG_0068)

/*************************************************************
 * [006C] SIE_REG_006C
 *************************************************************/
#define SIE_REG_006C_OFS    0x006C
REGDEF_BEGIN(SIE_REG_006C)
    REGDEF_BIT(DRAM_OUT1_SAO_2, 4)
REGDEF_END(SIE_REG_006C)

/*************************************************************
 * [0070] SIE_REG_0070
 *************************************************************/
#define SIE_REG_0070_OFS    0x0070
REGDEF_BEGIN(SIE_REG_0070)
    REGDEF_BIT(,                 2)
    REGDEF_BIT(DRAM_OUT1_OFSO,  14)
    REGDEF_BIT(,                 3)
    REGDEF_BIT(DRAM_OUT1_VFLIP,  1)
REGDEF_END(SIE_REG_0070)

/*************************************************************
 * [0074] SIE_REG_0074
 *************************************************************/
#define SIE_REG_0074_OFS    0x0074
REGDEF_BEGIN(SIE_REG_0074)
    REGDEF_BIT(,              2)
    REGDEF_BIT(DRAM_OUT2_SAO, 30)
REGDEF_END(SIE_REG_0074)

/*************************************************************
 * [0078] SIE_REG_0078
 *************************************************************/
#define SIE_REG_0078_OFS    0x0078
REGDEF_BEGIN(SIE_REG_0078)
    REGDEF_BIT(DRAM_OUT2_SAO_2, 4)
REGDEF_END(SIE_REG_0078)

/*************************************************************
 * [0088] SIE_REG_0088
 *************************************************************/
#define SIE_REG_0088_OFS    0x0088
REGDEF_BEGIN(SIE_REG_0088)
    REGDEF_BIT(,                2)
    REGDEF_BIT(DRAM_DBGOUT_SAO, 30)
REGDEF_END(SIE_REG_0088)

/*************************************************************
 * [008C] SIE_REG_008C
 *************************************************************/
#define SIE_REG_008C_OFS    0x008C
REGDEF_BEGIN(SIE_REG_008C)
    REGDEF_BIT(DRAM_DBGOUT_SAO_2, 4)
REGDEF_END(SIE_REG_008C)

/*************************************************************
 * [0090] SIE_REG_0090
 *************************************************************/
#define SIE_REG_0090_OFS    0x0090
REGDEF_BEGIN(SIE_REG_0090)
    REGDEF_BIT(OUT0_CH_EN,               1)
    REGDEF_BIT(OUT1_CH_EN,               1)
    REGDEF_BIT(OUT2_CH_EN,               1)
    REGDEF_BIT(,                         2)
    REGDEF_BIT(IN1_CH_EN,                1)
    REGDEF_BIT(IN2_CH_EN,                1)
    REGDEF_BIT(IN3_CH_EN,                1)
    REGDEF_BIT(OUT0_LOCK_DISABLE,        1)
    REGDEF_BIT(OUT1_LOCK_DISABLE,        1)
    REGDEF_BIT(OUT2_LOCK_DISABLE,        1)
    REGDEF_BIT(,                         2)
    REGDEF_BIT(IN1_LOCK_DISABLE,         1)
    REGDEF_BIT(IN2_LOCK_DISABLE,         1)
    REGDEF_BIT(IN3_LOCK_DISABLE,         1)
    REGDEF_BIT(WRITE_CH_OUTSTANDING_NUM, 8)
    REGDEF_BIT(READ_CH_OUTSTANDING_NUM,  8)
REGDEF_END(SIE_REG_0090)

/*************************************************************
 * [0094] SIE_REG_0094
 *************************************************************/
#define SIE_REG_0094_OFS    0x0094
REGDEF_BEGIN(SIE_REG_0094)
    REGDEF_BIT(AXI_BUS_DISABLE,     1)
    REGDEF_BIT(,                    15)
    REGDEF_BIT(AXI_HIGH_BUS_IDLE,   1)
    REGDEF_BIT(AXI_NORMAL_BUS_IDLE, 1)
REGDEF_END(SIE_REG_0094)

/*************************************************************
 * [0098] SIE_REG_0098
 *************************************************************/
#define SIE_REG_0098_OFS    0x0098
REGDEF_BEGIN(SIE_REG_0098)
    REGDEF_BIT(AXI_HIGH_STATUS, 32)
REGDEF_END(SIE_REG_0098)

/*************************************************************
 * [009C] SIE_REG_009C
 *************************************************************/
#define SIE_REG_009C_OFS    0x009C
REGDEF_BEGIN(SIE_REG_009C)
    REGDEF_BIT(AXI_NORMAL_STATUS, 32)
REGDEF_END(SIE_REG_009C)

/*************************************************************
 * [00A0] SIE_REG_00A0
 *************************************************************/
#define SIE_REG_00A0_OFS    0x00A0
REGDEF_BEGIN(SIE_REG_00A0)
    REGDEF_BIT(DRAM_OUT0_MODE,    1)
    REGDEF_BIT(DRAM_OUT1_MODE,    1)
    REGDEF_BIT(DRAM_OUT2_MODE,    1)
    REGDEF_BIT(,                  1)
    REGDEF_BIT(DRAM_DBGOUT_MODE,  1)
REGDEF_END(SIE_REG_00A0)

/*************************************************************
 * [00A4] SIE_REG_00A4
 *************************************************************/
#define SIE_REG_00A4_OFS    0x00A4
REGDEF_BEGIN(SIE_REG_00A4)
    REGDEF_BIT(DRAM_OUT0_SINGLE_EN,    1)
    REGDEF_BIT(DRAM_OUT1_SINGLE_EN,    1)
    REGDEF_BIT(DRAM_OUT2_SINGLE_EN,    1)
    REGDEF_BIT(,                       1)
    REGDEF_BIT(DRAM_DBGOUT_SINGLE_EN,  1)
REGDEF_END(SIE_REG_00A4)

/*************************************************************
 * [00A8] SIE_REG_00A8
 *************************************************************/
#define SIE_REG_00A8_OFS    0x00A8
REGDEF_BEGIN(SIE_REG_00A8)
    REGDEF_BIT(DRAM_BURST_LENGTH_OUT0, 2)
    REGDEF_BIT(,                       2)
    REGDEF_BIT(DRAM_BURST_LENGTH_OUT1, 2)
    REGDEF_BIT(DRAM_BURST_LENGTH_OUT2, 2)
    REGDEF_BIT(,                       8)
    REGDEF_BIT(DRAM_BURST_LENGTH_IN1,  2)
    REGDEF_BIT(DRAM_BURST_LENGTH_IN2,  2)
    REGDEF_BIT(DRAM_BURST_LENGTH_IN3,  2)
REGDEF_END(SIE_REG_00A8)

/*************************************************************
 * [00AC] SIE_REG_00AC
 *************************************************************/
#define SIE_REG_00AC_OFS    0x00AC
REGDEF_BEGIN(SIE_REG_00AC)
    REGDEF_BIT(,                6)
    REGDEF_BIT(OUT_SWAP,        2)
    REGDEF_BIT(OUT_SPLIT,       1)
    REGDEF_BIT(OUT_YUV420,      1)
    REGDEF_BIT(YUV_IN_FMT,      1)
    REGDEF_BIT(OUT_YUV420_SWAP, 2)
REGDEF_END(SIE_REG_00AC)

/*************************************************************
 * [00B0] SIE_REG_00B0
 *************************************************************/
#define SIE_REG_00B0_OFS    0x00B0
REGDEF_BEGIN(SIE_REG_00B0)
    REGDEF_BIT(PATGEN_MODE,            3)
    REGDEF_BIT(,                       1)
    REGDEF_BIT(PATGEN_SYNC_MODE,       1)
    REGDEF_BIT(,                       3)
    REGDEF_BIT(PATGEN_SYNC_SRC,        4)
    REGDEF_BIT(PATGEN_SYNC_DATA_DELAY, 2)
    REGDEF_BIT(,                       2)
    REGDEF_BIT(PATGEN_VAL,             12)
    REGDEF_BIT(,                       3)
    REGDEF_BIT(PATGEN_SYNC_GO,         1)
REGDEF_END(SIE_REG_00B0)

/*************************************************************
 * [00BC] SIE_REG_00BC
 *************************************************************/
#define SIE_REG_00BC_OFS    0x00BC
REGDEF_BEGIN(SIE_REG_00BC)
    REGDEF_BIT(OB_OFS,    16)
REGDEF_END(SIE_REG_00BC)

/*************************************************************
 * [00C8] SIE_REG_00C8
 *************************************************************/
#define SIE_REG_00C8_OFS    0x00C8
REGDEF_BEGIN(SIE_REG_00C8)
    REGDEF_BIT(CURR_DP_X, 16)
    REGDEF_BIT(CURR_DP_Y, 16)
REGDEF_END(SIE_REG_00C8)

/*************************************************************
 * [00CC] SIE_REG_00CC
 *************************************************************/
#define SIE_REG_00CC_OFS    0x00CC
REGDEF_BEGIN(SIE_REG_00CC)
    REGDEF_BIT(DEF_FACT,             2)
    REGDEF_BIT(,                     2)
    REGDEF_BIT(DEF_MODE,             2)
    REGDEF_BIT(,                     2)
    REGDEF_BIT(DEF_DBG_VAL,         12)
    REGDEF_BIT(DEF_SAME_CH_ONLY_EN,  1)
REGDEF_END(SIE_REG_00CC)

/*************************************************************
 * [00D0] SIE_REG_00D0
 *************************************************************/
#define SIE_REG_00D0_OFS    0x00D0
REGDEF_BEGIN(SIE_REG_00D0)
    REGDEF_BIT(DEF_COL0,    14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL1,    14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00D0)

/*************************************************************
 * [00D4] SIE_REG_00D4
 *************************************************************/
#define SIE_REG_00D4_OFS    0x00D4
REGDEF_BEGIN(SIE_REG_00D4)
    REGDEF_BIT(DEF_COL2,    14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL3,    14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00D4)

/*************************************************************
 * [00D8] SIE_REG_00D8
 *************************************************************/
#define SIE_REG_00D8_OFS    0x00D8
REGDEF_BEGIN(SIE_REG_00D8)
    REGDEF_BIT(DEF_COL4,    14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL5,    14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00D8)

/*************************************************************
 * [00DC] SIE_REG_00DC
 *************************************************************/
#define SIE_REG_00DC_OFS    0x00DC
REGDEF_BEGIN(SIE_REG_00DC)
    REGDEF_BIT(DEF_COL6,    14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL7,    14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00DC)

/*************************************************************
 * [00E0] SIE_REG_00E0
 *************************************************************/
#define SIE_REG_00E0_OFS    0x00E0
REGDEF_BEGIN(SIE_REG_00E0)
    REGDEF_BIT(DEF_COL8,    14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL9,    14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00E0)

/*************************************************************
 * [00E4] SIE_REG_00E4
 *************************************************************/
#define SIE_REG_00E4_OFS    0x00E4
REGDEF_BEGIN(SIE_REG_00E4)
    REGDEF_BIT(DEF_COL10,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL11,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00E4)

/*************************************************************
 * [00E8] SIE_REG_00E8
 *************************************************************/
#define SIE_REG_00E8_OFS    0x00E8
REGDEF_BEGIN(SIE_REG_00E8)
    REGDEF_BIT(DEF_COL12,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL13,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00E8)

/*************************************************************
 * [00EC] SIE_REG_00EC
 *************************************************************/
#define SIE_REG_00EC_OFS    0x00EC
REGDEF_BEGIN(SIE_REG_00EC)
    REGDEF_BIT(DEF_COL14,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL15,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00EC)

/*************************************************************
 * [00F0] SIE_REG_00F0
 *************************************************************/
#define SIE_REG_00F0_OFS    0x00F0
REGDEF_BEGIN(SIE_REG_00F0)
    REGDEF_BIT(DEF_COL16,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL17,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00F0)

/*************************************************************
 * [00F4] SIE_REG_00F4
 *************************************************************/
#define SIE_REG_00F4_OFS    0x00F4
REGDEF_BEGIN(SIE_REG_00F4)
    REGDEF_BIT(DEF_COL18,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL19,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00F4)

/*************************************************************
 * [00F8] SIE_REG_00F8
 *************************************************************/
#define SIE_REG_00F8_OFS    0x00F8
REGDEF_BEGIN(SIE_REG_00F8)
    REGDEF_BIT(DEF_COL20,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL21,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00F8)

/*************************************************************
 * [00FC] SIE_REG_00FC
 *************************************************************/
#define SIE_REG_00FC_OFS    0x00FC
REGDEF_BEGIN(SIE_REG_00FC)
    REGDEF_BIT(DEF_COL22,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL23,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_00FC)

/*************************************************************
 * [0100] SIE_REG_0100
 *************************************************************/
#define SIE_REG_0100_OFS    0x0100
REGDEF_BEGIN(SIE_REG_0100)
    REGDEF_BIT(DEF_COL24,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL25,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_0100)

/*************************************************************
 * [0104] SIE_REG_0104
 *************************************************************/
#define SIE_REG_0104_OFS    0x0104
REGDEF_BEGIN(SIE_REG_0104)
    REGDEF_BIT(DEF_COL26,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL27,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_0104)

/*************************************************************
 * [0108] SIE_REG_0108
 *************************************************************/
#define SIE_REG_0108_OFS    0x0108
REGDEF_BEGIN(SIE_REG_0108)
    REGDEF_BIT(DEF_COL28,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL29,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_0108)

/*************************************************************
 * [010C] SIE_REG_010C
 *************************************************************/
#define SIE_REG_010C_OFS    0x010C
REGDEF_BEGIN(SIE_REG_010C)
    REGDEF_BIT(DEF_COL30,   14)
    REGDEF_BIT(,             2)
    REGDEF_BIT(DEF_COL31,   14)
    REGDEF_BIT(,             2)
REGDEF_END(SIE_REG_010C)

/*************************************************************
 * [0120] SIE_REG_0120
 *************************************************************/
#define SIE_REG_0120_OFS    0x0120
REGDEF_BEGIN(SIE_REG_0120)
    REGDEF_BIT(ACT_DATAEXT_ORDER,         1)
    REGDEF_BIT(ACT_DATAEXT_INI_LINE_OP,   1)
    REGDEF_BIT(,                          2)
    REGDEF_BIT(ACT_DATAEXT_SKIPLEN,       8)
    REGDEF_BIT(ACT_DATAEXT_LEN,           8)
    REGDEF_BIT(ACT_DATAEXT_INI_LINE_LEN, 12)
REGDEF_END(SIE_REG_0120)

/*************************************************************
 * [0154] SIE_REG_0154
 *************************************************************/
#define SIE_REG_0154_OFS    0x0154
REGDEF_BEGIN(SIE_REG_0154)
    REGDEF_BIT(MAP_SIZESEL,        2)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(MAP_DTHR_EN,        1)
    REGDEF_BIT(MAP_DTHR_RST,       1)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(MAP_DTHR_LEVEL,     3)
    REGDEF_BIT(,                   5)
    REGDEF_BIT(MAP_SHIFT,          4)
    REGDEF_BIT(ECS_BAYER_MODE,     1)
REGDEF_END(SIE_REG_0154)

/*************************************************************
 * [0158] SIE_REG_0158
 *************************************************************/
#define SIE_REG_0158_OFS    0x0158
REGDEF_BEGIN(SIE_REG_0158)
    REGDEF_BIT(MAP_HSCL, 16)
    REGDEF_BIT(MAP_VSCL, 16)
REGDEF_END(SIE_REG_0158)

/*************************************************************
 * [015C] SIE_REG_015C
 *************************************************************/
#define SIE_REG_015C_OFS    0x015C
REGDEF_BEGIN(SIE_REG_015C)
    REGDEF_BIT(DGAIN_GAIN, 16)
REGDEF_END(SIE_REG_015C)

/*************************************************************
 * [0160] SIE_REG_0160
 *************************************************************/
#define SIE_REG_0160_OFS    0x0160
REGDEF_BEGIN(SIE_REG_0160)
    REGDEF_BIT(BS_H_IV,     21)
    REGDEF_BIT(,            10)
    REGDEF_BIT(BS_SRC_INTP, 1)
REGDEF_END(SIE_REG_0160)

/*************************************************************
 * [0164] SIE_REG_0164
 *************************************************************/
#define SIE_REG_0164_OFS    0x0164
REGDEF_BEGIN(SIE_REG_0164)
    REGDEF_BIT(BS_H_SV, 20)
REGDEF_END(SIE_REG_0164)

/*************************************************************
 * [0168] SIE_REG_0168
 *************************************************************/
#define SIE_REG_0168_OFS    0x0168
REGDEF_BEGIN(SIE_REG_0168)
    REGDEF_BIT(BS_H_BV_R, 21)
REGDEF_END(SIE_REG_0168)

/*************************************************************
 * [016C] SIE_REG_016C
 *************************************************************/
#define SIE_REG_016C_OFS    0x016C
REGDEF_BEGIN(SIE_REG_016C)
    REGDEF_BIT(BS_H_BV_B, 21)
REGDEF_END(SIE_REG_016C)

/*************************************************************
 * [0170] SIE_REG_0170
 *************************************************************/
#define SIE_REG_0170_OFS    0x0170
REGDEF_BEGIN(SIE_REG_0170)
    REGDEF_BIT(BS_H_OUTSIZE,    14)
    REGDEF_BIT(,                2)
    REGDEF_BIT(BS_H_ACC_DIV_M0, 10)
    REGDEF_BIT(,                2)
    REGDEF_BIT(BS_H_ACC_DIV_S0, 3)
REGDEF_END(SIE_REG_0170)

/*************************************************************
 * [0174] SIE_REG_0174
 *************************************************************/
#define SIE_REG_0174_OFS    0x0174
REGDEF_BEGIN(SIE_REG_0174)
    REGDEF_BIT(BS_V_IV, 20)
REGDEF_END(SIE_REG_0174)

/*************************************************************
 * [0178] SIE_REG_0178
 *************************************************************/
#define SIE_REG_0178_OFS    0x0178
REGDEF_BEGIN(SIE_REG_0178)
    REGDEF_BIT(BS_V_SV, 19)
REGDEF_END(SIE_REG_0178)

/*************************************************************
 * [017C] SIE_REG_017C
 *************************************************************/
#define SIE_REG_017C_OFS    0x017C
REGDEF_BEGIN(SIE_REG_017C)
    REGDEF_BIT(BS_V_BV_R, 21)
REGDEF_END(SIE_REG_017C)

/*************************************************************
 * [0180] SIE_REG_0180
 *************************************************************/
#define SIE_REG_0180_OFS    0x0180
REGDEF_BEGIN(SIE_REG_0180)
    REGDEF_BIT(BS_V_BV_B, 21)
REGDEF_END(SIE_REG_0180)

/*************************************************************
 * [0184] SIE_REG_0184
 *************************************************************/
#define SIE_REG_0184_OFS    0x0184
REGDEF_BEGIN(SIE_REG_0184)
    REGDEF_BIT(BS_V_OUTSIZE,    13)
    REGDEF_BIT(,                1)
    REGDEF_BIT(BS_V_ACC_IN_S,   2)
    REGDEF_BIT(BS_V_ACC_DIV_M0, 10)
    REGDEF_BIT(,                2)
    REGDEF_BIT(BS_V_ACC_DIV_S0, 3)
REGDEF_END(SIE_REG_0184)

/*************************************************************
 * [0188] SIE_REG_0188
 *************************************************************/
#define SIE_REG_0188_OFS    0x0188
REGDEF_BEGIN(SIE_REG_0188)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB0, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB0, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB1, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB1, 4)
REGDEF_END(SIE_REG_0188)

/*************************************************************
 * [018C] SIE_REG_018C
 *************************************************************/
#define SIE_REG_018C_OFS    0x018C
REGDEF_BEGIN(SIE_REG_018C)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB2, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB2, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB3, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB4, 4)
REGDEF_END(SIE_REG_018C)

/*************************************************************
 * [0190] SIE_REG_0190
 *************************************************************/
#define SIE_REG_0190_OFS    0x0190
REGDEF_BEGIN(SIE_REG_0190)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB4, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB4, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB5, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB6, 4)
REGDEF_END(SIE_REG_0190)

/*************************************************************
 * [0194] SIE_REG_0194
 *************************************************************/
#define SIE_REG_0194_OFS    0x0194
REGDEF_BEGIN(SIE_REG_0194)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB6, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB6, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB7, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB7, 4)
REGDEF_END(SIE_REG_0194)

/*************************************************************
 * [0198] SIE_REG_0198
 *************************************************************/
#define SIE_REG_0198_OFS    0x0198
REGDEF_BEGIN(SIE_REG_0198)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB8, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB8, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB9, 10)
    REGDEF_BIT(,                   2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB9, 4)
REGDEF_END(SIE_REG_0198)

/*************************************************************
 * [019C] SIE_REG_019C
 *************************************************************/
#define SIE_REG_019C_OFS    0x019C
REGDEF_BEGIN(SIE_REG_019C)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB10, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB10, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB11, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB11, 4)
REGDEF_END(SIE_REG_019C)

/*************************************************************
 * [01A0] SIE_REG_01A0
 *************************************************************/
#define SIE_REG_01A0_OFS    0x01A0
REGDEF_BEGIN(SIE_REG_01A0)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB12, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB12, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB13, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB13, 4)
REGDEF_END(SIE_REG_01A0)

/*************************************************************
 * [01A4] SIE_REG_01A4
 *************************************************************/
#define SIE_REG_01A4_OFS    0x01A4
REGDEF_BEGIN(SIE_REG_01A4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB14, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB14, 4)
    REGDEF_BIT(BS_V_ACC_DIV_M_RB15, 10)
    REGDEF_BIT(,                    2)
    REGDEF_BIT(BS_V_ACC_DIV_S_RB15, 4)
REGDEF_END(SIE_REG_01A4)

/*************************************************************
 * [01A8] SIE_REG_01A8
 *************************************************************/
#define SIE_REG_01A8_OFS    0x01A8
REGDEF_BEGIN(SIE_REG_01A8)
    REGDEF_BIT(BS_H_ACC_DIV_M1, 10)
    REGDEF_BIT(,                2)
    REGDEF_BIT(BS_H_ACC_DIV_S1, 3)
    REGDEF_BIT(,                1)
    REGDEF_BIT(BS_V_ACC_DIV_M1, 10)
    REGDEF_BIT(,                2)
    REGDEF_BIT(BS_V_ACC_DIV_S1, 3)
REGDEF_END(SIE_REG_01A8)

/*************************************************************
 * [01B4] SIE_REG_01B4
 *************************************************************/
#define SIE_REG_01B4_OFS    0x01B4
REGDEF_BEGIN(SIE_REG_01B4)
    REGDEF_BIT(CGAIN_RGAIN,  10)
    REGDEF_BIT(,             6)
    REGDEF_BIT(CGAIN_GRGAIN, 10)
REGDEF_END(SIE_REG_01B4)

/*************************************************************
 * [01B8] SIE_REG_01B8
 *************************************************************/
#define SIE_REG_01B8_OFS    0x01B8
REGDEF_BEGIN(SIE_REG_01B8)
    REGDEF_BIT(CGAIN_GBGAIN, 10)
    REGDEF_BIT(,             6)
    REGDEF_BIT(CGAIN_BGAIN,  10)
REGDEF_END(SIE_REG_01B8)

/*************************************************************
 * [01BC] SIE_REG_01BC
 *************************************************************/
#define SIE_REG_01BC_OFS    0x01BC
REGDEF_BEGIN(SIE_REG_01BC)
    REGDEF_BIT(CGAIN_IRGAIN,    10)
    REGDEF_BIT(,                6)
    REGDEF_BIT(CGAIN_LEVEL_SEL, 1)
REGDEF_END(SIE_REG_01BC)

/*************************************************************
 * [01C4] SIE_REG_01C4
 *************************************************************/
#define SIE_REG_01C4_OFS    0x01C4
REGDEF_BEGIN(SIE_REG_01C4)
    REGDEF_BIT(DVS_POS_CODE,   8)
    REGDEF_BIT(DVS_NEG_CODE,   8)
    REGDEF_BIT(DVS_NOCHG_CODE, 8)
REGDEF_END(SIE_REG_01C4)

/*************************************************************
 * [01C8] SIE_REG_01C8
 *************************************************************/
#define SIE_REG_01C8_OFS    0x01C8
REGDEF_BEGIN(SIE_REG_01C8)
    REGDEF_BIT(DVS_TIMESTAMP, 32)
REGDEF_END(SIE_REG_01C8)

/*************************************************************
 * [01CC] SIE_REG_01CC
 *************************************************************/
#define SIE_REG_01CC_OFS    0x01CC
REGDEF_BEGIN(SIE_REG_01CC)
    REGDEF_BIT(MOT_OUT_SZ, 32)
REGDEF_END(SIE_REG_01CC)

/*************************************************************
 * [01D0] SIE_REG_01D0
 *************************************************************/
#define SIE_REG_01D0_OFS    0x01D0
REGDEF_BEGIN(SIE_REG_01D0)
    REGDEF_BIT(EVS_PAD_VAL, 16)
REGDEF_END(SIE_REG_01D0)

/*************************************************************
 * [01E0] SIE_REG_01E0
 *************************************************************/
#define SIE_REG_01E0_OFS    0x01E0
REGDEF_BEGIN(SIE_REG_01E0)
    REGDEF_BIT(EVS_TOTAL_CNT, 32)
REGDEF_END(SIE_REG_01E0)

/*************************************************************
 * [01E8] SIE_REG_01E8
 *************************************************************/
#define SIE_REG_01E8_OFS    0x01E8
REGDEF_BEGIN(SIE_REG_01E8)
    REGDEF_BIT(,                      2)
    REGDEF_BIT(STCS_LA_CG_EN,         1)
    REGDEF_BIT(STCS_GAMMA_1_EN,       1)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(STCS_HISTO_Y_SEL,      1)
    REGDEF_BIT(STCS_CA_TH_EN,         1)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(STCS_LA_RGB2Y_SEL,     1)
    REGDEF_BIT(STCS_LA_RGB2Y1_MOD,    1)
    REGDEF_BIT(STCS_LA_RGB2Y2_MOD,    1)
    REGDEF_BIT(,                      1)
    REGDEF_BIT(STCS_CA_ACCM_SRC,      1)
    REGDEF_BIT(,                      3)
    REGDEF_BIT(STCS_COMPANDING_SHIFT, 4)
    REGDEF_BIT(STCS_RANG_SHIFT,       4)
REGDEF_END(SIE_REG_01E8)

/*************************************************************
 * [01EC] SIE_REG_01EC
 *************************************************************/
#define SIE_REG_01EC_OFS    0x01EC
REGDEF_BEGIN(SIE_REG_01EC)
    REGDEF_BIT(LA_CG_RGAIN, 10)
    REGDEF_BIT(,            6)
    REGDEF_BIT(LA_CG_GGAIN, 10)
REGDEF_END(SIE_REG_01EC)

/*************************************************************
 * [01F0] SIE_REG_01F0
 *************************************************************/
#define SIE_REG_01F0_OFS    0x01F0
REGDEF_BEGIN(SIE_REG_01F0)
    REGDEF_BIT(LA_CG_BGAIN,  10)
REGDEF_END(SIE_REG_01F0)

/*************************************************************
 * [01F4] SIE_REG_01F4
 *************************************************************/
#define SIE_REG_01F4_OFS    0x01F4
REGDEF_BEGIN(SIE_REG_01F4)
    REGDEF_BIT(LA_IRSUB_RWET, 8)
    REGDEF_BIT(LA_IRSUB_GWET, 8)
    REGDEF_BIT(LA_IRSUB_BWET, 8)
REGDEF_END(SIE_REG_01F4)

/*************************************************************
 * [0220] SIE_REG_0220
 *************************************************************/
#define SIE_REG_0220_OFS    0x0220
REGDEF_BEGIN(SIE_REG_0220)
    REGDEF_BIT(CA_CROP_STX, 16)
    REGDEF_BIT(CA_CROP_STY, 16)
REGDEF_END(SIE_REG_0220)

/*************************************************************
 * [0224] SIE_REG_0224
 *************************************************************/
#define SIE_REG_0224_OFS    0x0224
REGDEF_BEGIN(SIE_REG_0224)
    REGDEF_BIT(SMPL_X_FACT, 15)
    REGDEF_BIT(,            1)
    REGDEF_BIT(SMPL_Y_FACT, 15)
REGDEF_END(SIE_REG_0224)

/*************************************************************
 * [0228] SIE_REG_0228
 *************************************************************/
#define SIE_REG_0228_OFS    0x0228
REGDEF_BEGIN(SIE_REG_0228)
    REGDEF_BIT(CA_CROP_SZX, 16)
    REGDEF_BIT(CA_CROP_SZY, 16)
REGDEF_END(SIE_REG_0228)

/*************************************************************
 * [022C] SIE_REG_022C
 *************************************************************/
#define SIE_REG_022C_OFS    0x022c
REGDEF_BEGIN(SIE_REG_022C)
    REGDEF_BIT(CA_TH_G_LOWER, 12)
    REGDEF_BIT(,              4)
    REGDEF_BIT(CA_TH_G_UPPER, 12)
REGDEF_END(SIE_REG_022C)

/*************************************************************
 * [0230] SIE_REG_0230
 *************************************************************/
#define SIE_REG_0230_OFS    0x0230
REGDEF_BEGIN(SIE_REG_0230)
    REGDEF_BIT(CA_TH_RG_LOWER, 12)
    REGDEF_BIT(,               4)
    REGDEF_BIT(CA_TH_RG_UPPER, 12)
REGDEF_END(SIE_REG_0230)

/*************************************************************
 * [0234] SIE_REG_0234
 *************************************************************/
#define SIE_REG_0234_OFS    0x0234
REGDEF_BEGIN(SIE_REG_0234)
    REGDEF_BIT(CA_TH_BG_LOWER, 12)
    REGDEF_BIT(,               4)
    REGDEF_BIT(CA_TH_BG_UPPER, 12)
REGDEF_END(SIE_REG_0234)

/*************************************************************
 * [0238] SIE_REG_0238
 *************************************************************/
#define SIE_REG_0238_OFS    0x0238
REGDEF_BEGIN(SIE_REG_0238)
    REGDEF_BIT(CA_TH_PG_LOWER, 12)
    REGDEF_BIT(,               4)
    REGDEF_BIT(CA_TH_PG_UPPER, 12)
REGDEF_END(SIE_REG_0238)

/*************************************************************
 * [023C] SIE_REG_023C
 *************************************************************/
#define SIE_REG_023C_OFS    0x023C
REGDEF_BEGIN(SIE_REG_023C)
    REGDEF_BIT(CA_WIN_SZX, 8)
    REGDEF_BIT(CA_WIN_SZY, 8)
REGDEF_END(SIE_REG_023C)

/*************************************************************
 * [0240] SIE_REG_0240
 *************************************************************/
#define SIE_REG_0240_OFS    0x0240
REGDEF_BEGIN(SIE_REG_0240)
    REGDEF_BIT(CA_WIN_NUMX, 5)
    REGDEF_BIT(,            3)
    REGDEF_BIT(CA_WIN_NUMY, 5)
REGDEF_END(SIE_REG_0240)

/*************************************************************
 * [0244] SIE_REG_0244
 *************************************************************/
#define SIE_REG_0244_OFS    0x0244
REGDEF_BEGIN(SIE_REG_0244)
    REGDEF_BIT(CA_IRSUB_RWET, 8)
    REGDEF_BIT(CA_IRSUB_GWET, 8)
    REGDEF_BIT(CA_IRSUB_BWET, 8)
REGDEF_END(SIE_REG_0244)

/*************************************************************
 * [0248] SIE_REG_0248
 *************************************************************/
#define SIE_REG_0248_OFS    0x0248
REGDEF_BEGIN(SIE_REG_0248)
    REGDEF_BIT(LA_CROP_STX, 11)
    REGDEF_BIT(,            5)
    REGDEF_BIT(LA_CROP_STY, 11)
REGDEF_END(SIE_REG_0248)

/*************************************************************
 * [024C] SIE_REG_024C
 *************************************************************/
#define SIE_REG_024C_OFS    0x024C
REGDEF_BEGIN(SIE_REG_024C)
    REGDEF_BIT(LA_CROP_SZX, 11)
    REGDEF_BIT(,            5)
    REGDEF_BIT(LA_CROP_SZY, 11)
REGDEF_END(SIE_REG_024C)

/*************************************************************
 * [0250] SIE_REG_0250
 *************************************************************/
#define SIE_REG_0250_OFS    0x0250
REGDEF_BEGIN(SIE_REG_0250)
    REGDEF_BIT(STCS_GAMMA_1_TBL0, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL1, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL2, 10)
REGDEF_END(SIE_REG_0250)

/*************************************************************
 * [0254] SIE_REG_0254
 *************************************************************/
#define SIE_REG_0254_OFS    0x0254
REGDEF_BEGIN(SIE_REG_0254)
    REGDEF_BIT(STCS_GAMMA_1_TBL3, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL4, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL5, 10)
REGDEF_END(SIE_REG_0254)

/*************************************************************
 * [0258] SIE_REG_0258
 *************************************************************/
#define SIE_REG_0258_OFS    0x0258
REGDEF_BEGIN(SIE_REG_0258)
    REGDEF_BIT(STCS_GAMMA_1_TBL6, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL7, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL8, 10)
REGDEF_END(SIE_REG_0258)

/*************************************************************
 * [025C] SIE_REG_025C
 *************************************************************/
#define SIE_REG_025C_OFS    0x025C
REGDEF_BEGIN(SIE_REG_025C)
    REGDEF_BIT(STCS_GAMMA_1_TBL9,  10)
    REGDEF_BIT(STCS_GAMMA_1_TBL10, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL11, 10)
REGDEF_END(SIE_REG_025C)

/*************************************************************
 * [0260] SIE_REG_0260
 *************************************************************/
#define SIE_REG_0260_OFS    0x0260
REGDEF_BEGIN(SIE_REG_0260)
    REGDEF_BIT(STCS_GAMMA_1_TBL12, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL13, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL14, 10)
REGDEF_END(SIE_REG_0260)

/*************************************************************
 * [0264] SIE_REG_0264
 *************************************************************/
#define SIE_REG_0264_OFS    0x0264
REGDEF_BEGIN(SIE_REG_0264)
    REGDEF_BIT(STCS_GAMMA_1_TBL15, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL16, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL17, 10)
REGDEF_END(SIE_REG_0264)

/*************************************************************
 * [0268] SIE_REG_0268
 *************************************************************/
#define SIE_REG_0268_OFS    0x0268
REGDEF_BEGIN(SIE_REG_0268)
    REGDEF_BIT(STCS_GAMMA_1_TBL18, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL19, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL20, 10)
REGDEF_END(SIE_REG_0268)

/*************************************************************
 * [026C] SIE_REG_026C
 *************************************************************/
#define SIE_REG_026C_OFS    0x026C
REGDEF_BEGIN(SIE_REG_026C)
    REGDEF_BIT(STCS_GAMMA_1_TBL21, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL22, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL23, 10)
REGDEF_END(SIE_REG_026C)

/*************************************************************
 * [0270] SIE_REG_0270
 *************************************************************/
#define SIE_REG_0270_OFS    0x0270
REGDEF_BEGIN(SIE_REG_0270)
    REGDEF_BIT(STCS_GAMMA_1_TBL24, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL25, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL26, 10)
REGDEF_END(SIE_REG_0270)

/*************************************************************
 * [0274] SIE_REG_0274
 *************************************************************/
#define SIE_REG_0274_OFS    0x0274
REGDEF_BEGIN(SIE_REG_0274)
    REGDEF_BIT(STCS_GAMMA_1_TBL27, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL28, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL29, 10)
REGDEF_END(SIE_REG_0274)

/*************************************************************
 * [0278] SIE_REG_0278
 *************************************************************/
#define SIE_REG_0278_OFS    0x0278
REGDEF_BEGIN(SIE_REG_0278)
    REGDEF_BIT(STCS_GAMMA_1_TBL30, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL31, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL32, 10)
REGDEF_END(SIE_REG_0278)

/*************************************************************
 * [027C] SIE_REG_027C
 *************************************************************/
#define SIE_REG_027C_OFS    0x027C
REGDEF_BEGIN(SIE_REG_027C)
    REGDEF_BIT(STCS_GAMMA_1_TBL33, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL34, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL35, 10)
REGDEF_END(SIE_REG_027C)

/*************************************************************
 * [0280] SIE_REG_0280
 *************************************************************/
#define SIE_REG_0280_OFS    0x0280
REGDEF_BEGIN(SIE_REG_0280)
    REGDEF_BIT(STCS_GAMMA_1_TBL36, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL37, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL38, 10)
REGDEF_END(SIE_REG_0280)

/*************************************************************
 * [0284] SIE_REG_0284
 *************************************************************/
#define SIE_REG_0284_OFS    0x0284
REGDEF_BEGIN(SIE_REG_0284)
    REGDEF_BIT(STCS_GAMMA_1_TBL39, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL40, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL41, 10)
REGDEF_END(SIE_REG_0284)

/*************************************************************
 * [0288] SIE_REG_0288
 *************************************************************/
#define SIE_REG_0288_OFS    0x0288
REGDEF_BEGIN(SIE_REG_0288)
    REGDEF_BIT(STCS_GAMMA_1_TBL42, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL43, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL44, 10)
REGDEF_END(SIE_REG_0288)

/*************************************************************
 * [028C] SIE_REG_028C
 *************************************************************/
#define SIE_REG_028C_OFS    0x028C
REGDEF_BEGIN(SIE_REG_028C)
    REGDEF_BIT(STCS_GAMMA_1_TBL45, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL46, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL47, 10)
REGDEF_END(SIE_REG_028C)

/*************************************************************
 * [0290] SIE_REG_0290
 *************************************************************/
#define SIE_REG_0290_OFS    0x0290
REGDEF_BEGIN(SIE_REG_0290)
    REGDEF_BIT(STCS_GAMMA_1_TBL48, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL49, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL50, 10)
REGDEF_END(SIE_REG_0290)

/*************************************************************
 * [0294] SIE_REG_0294
 *************************************************************/
#define SIE_REG_0294_OFS    0x0294
REGDEF_BEGIN(SIE_REG_0294)
    REGDEF_BIT(STCS_GAMMA_1_TBL51, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL52, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL53, 10)
REGDEF_END(SIE_REG_0294)

/*************************************************************
 * [0298] SIE_REG_0298
 *************************************************************/
#define SIE_REG_0298_OFS    0x0298
REGDEF_BEGIN(SIE_REG_0298)
    REGDEF_BIT(STCS_GAMMA_1_TBL54, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL55, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL56, 10)
REGDEF_END(SIE_REG_0298)

/*************************************************************
 * [029C] SIE_REG_029C
 *************************************************************/
#define SIE_REG_029C_OFS    0x029C
REGDEF_BEGIN(SIE_REG_029C)
    REGDEF_BIT(STCS_GAMMA_1_TBL57, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL58, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL59, 10)
REGDEF_END(SIE_REG_029C)

/*************************************************************
 * [02A0] SIE_REG_02A0
 *************************************************************/
#define SIE_REG_02A0_OFS    0x02A0
REGDEF_BEGIN(SIE_REG_02A0)
    REGDEF_BIT(STCS_GAMMA_1_TBL60, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL61, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL62, 10)
REGDEF_END(SIE_REG_02A0)

/*************************************************************
 * [02A4] SIE_REG_02A4
 *************************************************************/
#define SIE_REG_02A4_OFS    0x02A4
REGDEF_BEGIN(SIE_REG_02A4)
    REGDEF_BIT(STCS_GAMMA_1_TBL63, 10)
    REGDEF_BIT(STCS_GAMMA_1_TBL64, 10)
REGDEF_END(SIE_REG_02A4)

/*************************************************************
 * [02A8] SIE_REG_02A8
 *************************************************************/
#define SIE_REG_02A8_OFS    0x02A8
REGDEF_BEGIN(SIE_REG_02A8)
    REGDEF_BIT(CA_OB_OFS, 12)
    REGDEF_BIT(LA_OB_OFS, 12)
REGDEF_END(SIE_REG_02A8)

/*************************************************************
 * [02AC] SIE_REG_02AC
 *************************************************************/
#define SIE_REG_02AC_OFS    0x02AC
REGDEF_BEGIN(SIE_REG_02AC)
    REGDEF_BIT(LA_WIN_SZX, 8)
    REGDEF_BIT(LA_WIN_SZY, 8)
REGDEF_END(SIE_REG_02AC)

/*************************************************************
 * [02B0] SIE_REG_02B0
 *************************************************************/
#define SIE_REG_02B0_OFS    0x02B0
REGDEF_BEGIN(SIE_REG_02B0)
    REGDEF_BIT(LA_WIN_NUMX, 5)
    REGDEF_BIT(,            3)
    REGDEF_BIT(LA_WIN_NUMY, 5)
    REGDEF_BIT(,            1)
    REGDEF_BIT(LA1_WIN_SUM, 18)
REGDEF_END(SIE_REG_02B0)

/*************************************************************
 * [0300] SIE_REG_0300
 *************************************************************/
#define SIE_REG_0300_OFS    0x0300
REGDEF_BEGIN(SIE_REG_0300)
    REGDEF_BIT(HISTO_Y_RSLT_0, 16)
    REGDEF_BIT(HISTO_Y_RSLT_1, 16)
REGDEF_END(SIE_REG_0300)

/*************************************************************
 * [0304] SIE_REG_0304
 *************************************************************/
#define SIE_REG_0304_OFS    0x0304
REGDEF_BEGIN(SIE_REG_0304)
    REGDEF_BIT(HISTO_Y_RSLT_2, 16)
    REGDEF_BIT(HISTO_Y_RSLT_3, 16)
REGDEF_END(SIE_REG_0304)

/*************************************************************
 * [0308] SIE_REG_0308
 *************************************************************/
#define SIE_REG_0308_OFS    0x0308
REGDEF_BEGIN(SIE_REG_0308)
    REGDEF_BIT(HISTO_Y_RSLT_4, 16)
    REGDEF_BIT(HISTO_Y_RSLT_5, 16)
REGDEF_END(SIE_REG_0308)

/*************************************************************
 * [030C] SIE_REG_030C
 *************************************************************/
#define SIE_REG_030C_OFS    0x030C
REGDEF_BEGIN(SIE_REG_030C)
    REGDEF_BIT(HISTO_Y_RSLT_6, 16)
    REGDEF_BIT(HISTO_Y_RSLT_7, 16)
REGDEF_END(SIE_REG_030C)

/*************************************************************
 * [0310] SIE_REG_0310
 *************************************************************/
#define SIE_REG_0310_OFS    0x0310
REGDEF_BEGIN(SIE_REG_0310)
    REGDEF_BIT(HISTO_Y_RSLT_8, 16)
    REGDEF_BIT(HISTO_Y_RSLT_9, 16)
REGDEF_END(SIE_REG_0310)

/*************************************************************
 * [0314] SIE_REG_0314
 *************************************************************/
#define SIE_REG_0314_OFS    0x0314
REGDEF_BEGIN(SIE_REG_0314)
    REGDEF_BIT(HISTO_Y_RSLT_10, 16)
    REGDEF_BIT(HISTO_Y_RSLT_11, 16)
REGDEF_END(SIE_REG_0314)

/*************************************************************
 * [0318] SIE_REG_0318
 *************************************************************/
#define SIE_REG_0318_OFS    0x0318
REGDEF_BEGIN(SIE_REG_0318)
    REGDEF_BIT(HISTO_Y_RSLT_12, 16)
    REGDEF_BIT(HISTO_Y_RSLT_13, 16)
REGDEF_END(SIE_REG_0318)

/*************************************************************
 * [031C] SIE_REG_031C
 *************************************************************/
#define SIE_REG_031C_OFS    0x031C
REGDEF_BEGIN(SIE_REG_031C)
    REGDEF_BIT(HISTO_Y_RSLT_14, 16)
    REGDEF_BIT(HISTO_Y_RSLT_15, 16)
REGDEF_END(SIE_REG_031C)

/*************************************************************
 * [0320] SIE_REG_0320
 *************************************************************/
#define SIE_REG_0320_OFS    0x0320
REGDEF_BEGIN(SIE_REG_0320)
    REGDEF_BIT(HISTO_Y_RSLT_16, 16)
    REGDEF_BIT(HISTO_Y_RSLT_17, 16)
REGDEF_END(SIE_REG_0320)

/*************************************************************
 * [0324] SIE_REG_0324
 *************************************************************/
#define SIE_REG_0324_OFS    0x0324
REGDEF_BEGIN(SIE_REG_0324)
    REGDEF_BIT(HISTO_Y_RSLT_18, 16)
    REGDEF_BIT(HISTO_Y_RSLT_19, 16)
REGDEF_END(SIE_REG_0324)

/*************************************************************
 * [0328] SIE_REG_0328
 *************************************************************/
#define SIE_REG_0328_OFS    0x0328
REGDEF_BEGIN(SIE_REG_0328)
    REGDEF_BIT(HISTO_Y_RSLT_20, 16)
    REGDEF_BIT(HISTO_Y_RSLT_21, 16)
REGDEF_END(SIE_REG_0328)

/*************************************************************
 * [032C] SIE_REG_032C
 *************************************************************/
#define SIE_REG_032C_OFS    0x032C
REGDEF_BEGIN(SIE_REG_032C)
    REGDEF_BIT(HISTO_Y_RSLT_22, 16)
    REGDEF_BIT(HISTO_Y_RSLT_23, 16)
REGDEF_END(SIE_REG_032C)

/*************************************************************
 * [0330] SIE_REG_0330
 *************************************************************/
#define SIE_REG_0330_OFS    0x0330
REGDEF_BEGIN(SIE_REG_0330)
    REGDEF_BIT(HISTO_Y_RSLT_24, 16)
    REGDEF_BIT(HISTO_Y_RSLT_25, 16)
REGDEF_END(SIE_REG_0330)

/*************************************************************
 * [0334] SIE_REG_0334
 *************************************************************/
#define SIE_REG_0334_OFS    0x0334
REGDEF_BEGIN(SIE_REG_0334)
    REGDEF_BIT(HISTO_Y_RSLT_26, 16)
    REGDEF_BIT(HISTO_Y_RSLT_27, 16)
REGDEF_END(SIE_REG_0334)

/*************************************************************
 * [0338] SIE_REG_0338
 *************************************************************/
#define SIE_REG_0338_OFS    0x0338
REGDEF_BEGIN(SIE_REG_0338)
    REGDEF_BIT(HISTO_Y_RSLT_28, 16)
    REGDEF_BIT(HISTO_Y_RSLT_29, 16)
REGDEF_END(SIE_REG_0338)

/*************************************************************
 * [033C] SIE_REG_033C
 *************************************************************/
#define SIE_REG_033C_OFS    0x033C
REGDEF_BEGIN(SIE_REG_033C)
    REGDEF_BIT(HISTO_Y_RSLT_30, 16)
    REGDEF_BIT(HISTO_Y_RSLT_31, 16)
REGDEF_END(SIE_REG_033C)

/*************************************************************
 * [0340] SIE_REG_0340
 *************************************************************/
#define SIE_REG_0340_OFS    0x0340
REGDEF_BEGIN(SIE_REG_0340)
    REGDEF_BIT(HISTO_Y_RSLT_32, 16)
    REGDEF_BIT(HISTO_Y_RSLT_33, 16)
REGDEF_END(SIE_REG_0340)

/*************************************************************
 * [0344] SIE_REG_0344
 *************************************************************/
#define SIE_REG_0344_OFS    0x0344
REGDEF_BEGIN(SIE_REG_0344)
    REGDEF_BIT(HISTO_Y_RSLT_34, 16)
    REGDEF_BIT(HISTO_Y_RSLT_35, 16)
REGDEF_END(SIE_REG_0344)

/*************************************************************
 * [0348] SIE_REG_0348
 *************************************************************/
#define SIE_REG_0348_OFS    0x0348
REGDEF_BEGIN(SIE_REG_0348)
    REGDEF_BIT(HISTO_Y_RSLT_36, 16)
    REGDEF_BIT(HISTO_Y_RSLT_37, 16)
REGDEF_END(SIE_REG_0348)

/*************************************************************
 * [034C] SIE_REG_034C
 *************************************************************/
#define SIE_REG_034C_OFS    0x034C
REGDEF_BEGIN(SIE_REG_034C)
    REGDEF_BIT(HISTO_Y_RSLT_38, 16)
    REGDEF_BIT(HISTO_Y_RSLT_39, 16)
REGDEF_END(SIE_REG_034C)

/*************************************************************
 * [0350] SIE_REG_0350
 *************************************************************/
#define SIE_REG_0350_OFS    0x0350
REGDEF_BEGIN(SIE_REG_0350)
    REGDEF_BIT(HISTO_Y_RSLT_40, 16)
    REGDEF_BIT(HISTO_Y_RSLT_41, 16)
REGDEF_END(SIE_REG_0350)

/*************************************************************
 * [0354] SIE_REG_0354
 *************************************************************/
#define SIE_REG_0354_OFS    0x0354
REGDEF_BEGIN(SIE_REG_0354)
    REGDEF_BIT(HISTO_Y_RSLT_42, 16)
    REGDEF_BIT(HISTO_Y_RSLT_43, 16)
REGDEF_END(SIE_REG_0354)

/*************************************************************
 * [0358] SIE_REG_0358
 *************************************************************/
#define SIE_REG_0358_OFS    0x0358
REGDEF_BEGIN(SIE_REG_0358)
    REGDEF_BIT(HISTO_Y_RSLT_44, 16)
    REGDEF_BIT(HISTO_Y_RSLT_45, 16)
REGDEF_END(SIE_REG_0358)

/*************************************************************
 * [035C] SIE_REG_035C
 *************************************************************/
#define SIE_REG_035C_OFS    0x035C
REGDEF_BEGIN(SIE_REG_035C)
    REGDEF_BIT(HISTO_Y_RSLT_46, 16)
    REGDEF_BIT(HISTO_Y_RSLT_47, 16)
REGDEF_END(SIE_REG_035C)

/*************************************************************
 * [0360] SIE_REG_0360
 *************************************************************/
#define SIE_REG_0360_OFS    0x0360
REGDEF_BEGIN(SIE_REG_0360)
    REGDEF_BIT(HISTO_Y_RSLT_48, 16)
    REGDEF_BIT(HISTO_Y_RSLT_49, 16)
REGDEF_END(SIE_REG_0360)

/*************************************************************
 * [0364] SIE_REG_0364
 *************************************************************/
#define SIE_REG_0364_OFS    0x0364
REGDEF_BEGIN(SIE_REG_0364)
    REGDEF_BIT(HISTO_Y_RSLT_50, 16)
    REGDEF_BIT(HISTO_Y_RSLT_51, 16)
REGDEF_END(SIE_REG_0364)

/*************************************************************
 * [0368] SIE_REG_0368
 *************************************************************/
#define SIE_REG_0368_OFS    0x0368
REGDEF_BEGIN(SIE_REG_0368)
    REGDEF_BIT(HISTO_Y_RSLT_52, 16)
    REGDEF_BIT(HISTO_Y_RSLT_53, 16)
REGDEF_END(SIE_REG_0368)

/*************************************************************
 * [036C] SIE_REG_036C
 *************************************************************/
#define SIE_REG_036C_OFS    0x036C
REGDEF_BEGIN(SIE_REG_036C)
    REGDEF_BIT(HISTO_Y_RSLT_54, 16)
    REGDEF_BIT(HISTO_Y_RSLT_55, 16)
REGDEF_END(SIE_REG_036C)

/*************************************************************
 * [0370] SIE_REG_0370
 *************************************************************/
#define SIE_REG_0370_OFS    0x0370
REGDEF_BEGIN(SIE_REG_0370)
    REGDEF_BIT(HISTO_Y_RSLT_56, 16)
    REGDEF_BIT(HISTO_Y_RSLT_57, 16)
REGDEF_END(SIE_REG_0370)

/*************************************************************
 * [0374] SIE_REG_0374
 *************************************************************/
#define SIE_REG_0374_OFS    0x0374
REGDEF_BEGIN(SIE_REG_0374)
    REGDEF_BIT(HISTO_Y_RSLT_58, 16)
    REGDEF_BIT(HISTO_Y_RSLT_59, 16)
REGDEF_END(SIE_REG_0374)

/*************************************************************
 * [0378] SIE_REG_0378
 *************************************************************/
#define SIE_REG_0378_OFS    0x0378
REGDEF_BEGIN(SIE_REG_0378)
    REGDEF_BIT(HISTO_Y_RSLT_60, 16)
    REGDEF_BIT(HISTO_Y_RSLT_61, 16)
REGDEF_END(SIE_REG_0378)

/*************************************************************
 * [037C] SIE_REG_037C
 *************************************************************/
#define SIE_REG_037C_OFS    0x037C
REGDEF_BEGIN(SIE_REG_037C)
    REGDEF_BIT(HISTO_Y_RSLT_62, 16)
    REGDEF_BIT(HISTO_Y_RSLT_63, 16)
REGDEF_END(SIE_REG_037C)

/*************************************************************
 * [0380] SIE_REG_0380
 *************************************************************/
#define SIE_REG_0380_OFS    0x0380
REGDEF_BEGIN(SIE_REG_0380)
    REGDEF_BIT(HISTO_Y_RSLT_64, 16)
    REGDEF_BIT(HISTO_Y_RSLT_65, 16)
REGDEF_END(SIE_REG_0380)

/*************************************************************
 * [0384] SIE_REG_0384
 *************************************************************/
#define SIE_REG_0384_OFS    0x0384
REGDEF_BEGIN(SIE_REG_0384)
    REGDEF_BIT(HISTO_Y_RSLT_66, 16)
    REGDEF_BIT(HISTO_Y_RSLT_67, 16)
REGDEF_END(SIE_REG_0384)

/*************************************************************
 * [0388] SIE_REG_0388
 *************************************************************/
#define SIE_REG_0388_OFS    0x0388
REGDEF_BEGIN(SIE_REG_0388)
    REGDEF_BIT(HISTO_Y_RSLT_68, 16)
    REGDEF_BIT(HISTO_Y_RSLT_69, 16)
REGDEF_END(SIE_REG_0388)

/*************************************************************
 * [038C] SIE_REG_038C
 *************************************************************/
#define SIE_REG_038C_OFS    0x038C
REGDEF_BEGIN(SIE_REG_038C)
    REGDEF_BIT(HISTO_Y_RSLT_70, 16)
    REGDEF_BIT(HISTO_Y_RSLT_71, 16)
REGDEF_END(SIE_REG_038C)

/*************************************************************
 * [0390] SIE_REG_0390
 *************************************************************/
#define SIE_REG_0390_OFS    0x0390
REGDEF_BEGIN(SIE_REG_0390)
    REGDEF_BIT(HISTO_Y_RSLT_72, 16)
    REGDEF_BIT(HISTO_Y_RSLT_73, 16)
REGDEF_END(SIE_REG_0390)

/*************************************************************
 * [0394] SIE_REG_0394
 *************************************************************/
#define SIE_REG_0394_OFS    0x0394
REGDEF_BEGIN(SIE_REG_0394)
    REGDEF_BIT(HISTO_Y_RSLT_74, 16)
    REGDEF_BIT(HISTO_Y_RSLT_75, 16)
REGDEF_END(SIE_REG_0394)

/*************************************************************
 * [0398] SIE_REG_0398
 *************************************************************/
#define SIE_REG_0398_OFS    0x0398
REGDEF_BEGIN(SIE_REG_0398)
    REGDEF_BIT(HISTO_Y_RSLT_76, 16)
    REGDEF_BIT(HISTO_Y_RSLT_77, 16)
REGDEF_END(SIE_REG_0398)

/*************************************************************
 * [039C] SIE_REG_039C
 *************************************************************/
#define SIE_REG_039C_OFS    0x039C
REGDEF_BEGIN(SIE_REG_039C)
    REGDEF_BIT(HISTO_Y_RSLT_78, 16)
    REGDEF_BIT(HISTO_Y_RSLT_79, 16)
REGDEF_END(SIE_REG_039C)

/*************************************************************
 * [03A0] SIE_REG_03A0
 *************************************************************/
#define SIE_REG_03A0_OFS    0x03A0
REGDEF_BEGIN(SIE_REG_03A0)
    REGDEF_BIT(HISTO_Y_RSLT_80, 16)
    REGDEF_BIT(HISTO_Y_RSLT_81, 16)
REGDEF_END(SIE_REG_03A0)

/*************************************************************
 * [03A4] SIE_REG_03A4
 *************************************************************/
#define SIE_REG_03A4_OFS    0x03A4
REGDEF_BEGIN(SIE_REG_03A4)
    REGDEF_BIT(HISTO_Y_RSLT_82, 16)
    REGDEF_BIT(HISTO_Y_RSLT_83, 16)
REGDEF_END(SIE_REG_03A4)

/*************************************************************
 * [03A8] SIE_REG_03A8
 *************************************************************/
#define SIE_REG_03A8_OFS    0x03A8
REGDEF_BEGIN(SIE_REG_03A8)
    REGDEF_BIT(HISTO_Y_RSLT_84, 16)
    REGDEF_BIT(HISTO_Y_RSLT_85, 16)
REGDEF_END(SIE_REG_03A8)

/*************************************************************
 * [03AC] SIE_REG_03AC
 *************************************************************/
#define SIE_REG_03AC_OFS    0x03AC
REGDEF_BEGIN(SIE_REG_03AC)
    REGDEF_BIT(HISTO_Y_RSLT_86, 16)
    REGDEF_BIT(HISTO_Y_RSLT_87, 16)
REGDEF_END(SIE_REG_03AC)

/*************************************************************
 * [03B0] SIE_REG_03B0
 *************************************************************/
#define SIE_REG_03B0_OFS    0x03B0
REGDEF_BEGIN(SIE_REG_03B0)
    REGDEF_BIT(HISTO_Y_RSLT_88, 16)
    REGDEF_BIT(HISTO_Y_RSLT_89, 16)
REGDEF_END(SIE_REG_03B0)

/*************************************************************
 * [03B4] SIE_REG_03B4
 *************************************************************/
#define SIE_REG_03B4_OFS    0x03B4
REGDEF_BEGIN(SIE_REG_03B4)
    REGDEF_BIT(HISTO_Y_RSLT_90, 16)
    REGDEF_BIT(HISTO_Y_RSLT_91, 16)
REGDEF_END(SIE_REG_03B4)

/*************************************************************
 * [03B8] SIE_REG_03B8
 *************************************************************/
#define SIE_REG_03B8_OFS    0x03B8
REGDEF_BEGIN(SIE_REG_03B8)
    REGDEF_BIT(HISTO_Y_RSLT_92, 16)
    REGDEF_BIT(HISTO_Y_RSLT_93, 16)
REGDEF_END(SIE_REG_03B8)

/*************************************************************
 * [03BC] SIE_REG_03BC
 *************************************************************/
#define SIE_REG_03BC_OFS    0x03BC
REGDEF_BEGIN(SIE_REG_03BC)
    REGDEF_BIT(HISTO_Y_RSLT_94, 16)
    REGDEF_BIT(HISTO_Y_RSLT_95, 16)
REGDEF_END(SIE_REG_03BC)

/*************************************************************
 * [03C0] SIE_REG_03C0
 *************************************************************/
#define SIE_REG_03C0_OFS    0x03C0
REGDEF_BEGIN(SIE_REG_03C0)
    REGDEF_BIT(HISTO_Y_RSLT_96, 16)
    REGDEF_BIT(HISTO_Y_RSLT_97, 16)
REGDEF_END(SIE_REG_03C0)

/*************************************************************
 * [03C4] SIE_REG_03C4
 *************************************************************/
#define SIE_REG_03C4_OFS    0x03C4
REGDEF_BEGIN(SIE_REG_03C4)
    REGDEF_BIT(HISTO_Y_RSLT_98, 16)
    REGDEF_BIT(HISTO_Y_RSLT_99, 16)
REGDEF_END(SIE_REG_03C4)

/*************************************************************
 * [03C8] SIE_REG_03C8
 *************************************************************/
#define SIE_REG_03C8_OFS    0x03C8
REGDEF_BEGIN(SIE_REG_03C8)
    REGDEF_BIT(HISTO_Y_RSLT_100, 16)
    REGDEF_BIT(HISTO_Y_RSLT_101, 16)
REGDEF_END(SIE_REG_03C8)

/*************************************************************
 * [03CC] SIE_REG_03CC
 *************************************************************/
#define SIE_REG_03CC_OFS    0x03CC
REGDEF_BEGIN(SIE_REG_03CC)
    REGDEF_BIT(HISTO_Y_RSLT_102, 16)
    REGDEF_BIT(HISTO_Y_RSLT_103, 16)
REGDEF_END(SIE_REG_03CC)

/*************************************************************
 * [03D0] SIE_REG_03D0
 *************************************************************/
#define SIE_REG_03D0_OFS    0x03D0
REGDEF_BEGIN(SIE_REG_03D0)
    REGDEF_BIT(HISTO_Y_RSLT_104, 16)
    REGDEF_BIT(HISTO_Y_RSLT_105, 16)
REGDEF_END(SIE_REG_03D0)

/*************************************************************
 * [03D4] SIE_REG_03D4
 *************************************************************/
#define SIE_REG_03D4_OFS    0x03D4
REGDEF_BEGIN(SIE_REG_03D4)
    REGDEF_BIT(HISTO_Y_RSLT_106, 16)
    REGDEF_BIT(HISTO_Y_RSLT_107, 16)
REGDEF_END(SIE_REG_03D4)

/*************************************************************
 * [03D8] SIE_REG_03D8
 *************************************************************/
#define SIE_REG_03D8_OFS    0x03D8
REGDEF_BEGIN(SIE_REG_03D8)
    REGDEF_BIT(HISTO_Y_RSLT_108, 16)
    REGDEF_BIT(HISTO_Y_RSLT_109, 16)
REGDEF_END(SIE_REG_03D8)

/*************************************************************
 * [03DC] SIE_REG_03DC
 *************************************************************/
#define SIE_REG_03DC_OFS    0x03DC
REGDEF_BEGIN(SIE_REG_03DC)
    REGDEF_BIT(HISTO_Y_RSLT_110, 16)
    REGDEF_BIT(HISTO_Y_RSLT_111, 16)
REGDEF_END(SIE_REG_03DC)

/*************************************************************
 * [03E0] SIE_REG_03E0
 *************************************************************/
#define SIE_REG_03E0_OFS    0x03E0
REGDEF_BEGIN(SIE_REG_03E0)
    REGDEF_BIT(HISTO_Y_RSLT_112, 16)
    REGDEF_BIT(HISTO_Y_RSLT_113, 16)
REGDEF_END(SIE_REG_03E0)

/*************************************************************
 * [03E4] SIE_REG_03E4
 *************************************************************/
#define SIE_REG_03E4_OFS    0x03E4
REGDEF_BEGIN(SIE_REG_03E4)
    REGDEF_BIT(HISTO_Y_RSLT_114, 16)
    REGDEF_BIT(HISTO_Y_RSLT_115, 16)
REGDEF_END(SIE_REG_03E4)

/*************************************************************
 * [03E8] SIE_REG_03E8
 *************************************************************/
#define SIE_REG_03E8_OFS    0x03E8
REGDEF_BEGIN(SIE_REG_03E8)
    REGDEF_BIT(HISTO_Y_RSLT_116, 16)
    REGDEF_BIT(HISTO_Y_RSLT_117, 16)
REGDEF_END(SIE_REG_03E8)

/*************************************************************
 * [03EC] SIE_REG_03EC
 *************************************************************/
#define SIE_REG_03EC_OFS    0x03EC
REGDEF_BEGIN(SIE_REG_03EC)
    REGDEF_BIT(HISTO_Y_RSLT_118, 16)
    REGDEF_BIT(HISTO_Y_RSLT_119, 16)
REGDEF_END(SIE_REG_03EC)

/*************************************************************
 * [03F0] SIE_REG_03F0
 *************************************************************/
#define SIE_REG_03F0_OFS    0x03F0
REGDEF_BEGIN(SIE_REG_03F0)
    REGDEF_BIT(HISTO_Y_RSLT_120, 16)
    REGDEF_BIT(HISTO_Y_RSLT_121, 16)
REGDEF_END(SIE_REG_03F0)

/*************************************************************
 * [03F4] SIE_REG_03F4
 *************************************************************/
#define SIE_REG_03F4_OFS    0x03F4
REGDEF_BEGIN(SIE_REG_03F4)
    REGDEF_BIT(HISTO_Y_RSLT_122, 16)
    REGDEF_BIT(HISTO_Y_RSLT_123, 16)
REGDEF_END(SIE_REG_03F4)

/*************************************************************
 * [03F8] SIE_REG_03F8
 *************************************************************/
#define SIE_REG_03F8_OFS    0x03F8
REGDEF_BEGIN(SIE_REG_03F8)
    REGDEF_BIT(HISTO_Y_RSLT_124, 16)
    REGDEF_BIT(HISTO_Y_RSLT_125, 16)
REGDEF_END(SIE_REG_03F8)

/*************************************************************
 * [03FC] SIE_REG_03FC
 *************************************************************/
#define SIE_REG_03FC_OFS    0x03FC
REGDEF_BEGIN(SIE_REG_03FC)
    REGDEF_BIT(HISTO_Y_RSLT_126, 16)
    REGDEF_BIT(HISTO_Y_RSLT_127, 16)
REGDEF_END(SIE_REG_03FC)

/*************************************************************
 * [0500] SIE_REG_0500
 *************************************************************/
#define SIE_REG_0500_OFS    0x0500
REGDEF_BEGIN(SIE_REG_0500)
    REGDEF_BIT(DECOMP_KPX0, 12)
    REGDEF_BIT(DECOMP_KPY0, 20)
REGDEF_END(SIE_REG_0500)

/*************************************************************
 * [0504] SIE_REG_0504
 *************************************************************/
#define SIE_REG_0504_OFS    0x0504
REGDEF_BEGIN(SIE_REG_0504)
    REGDEF_BIT(DECOMP_KPX1, 12)
    REGDEF_BIT(DECOMP_KPY1, 20)
REGDEF_END(SIE_REG_0504)

/*************************************************************
 * [0508] SIE_REG_0508
 *************************************************************/
#define SIE_REG_0508_OFS    0x0508
REGDEF_BEGIN(SIE_REG_0508)
    REGDEF_BIT(DECOMP_KPX2, 12)
    REGDEF_BIT(DECOMP_KPY2, 20)
REGDEF_END(SIE_REG_0508)

/*************************************************************
 * [050C] SIE_REG_050C
 *************************************************************/
#define SIE_REG_050C_OFS    0x050C
REGDEF_BEGIN(SIE_REG_050C)
    REGDEF_BIT(DECOMP_KPX3, 12)
    REGDEF_BIT(DECOMP_KPY3, 20)
REGDEF_END(SIE_REG_050C)

/*************************************************************
 * [0510] SIE_REG_0510
 *************************************************************/
#define SIE_REG_0510_OFS    0x0510
REGDEF_BEGIN(SIE_REG_0510)
    REGDEF_BIT(DECOMP_KPX4, 12)
    REGDEF_BIT(DECOMP_KPY4, 20)
REGDEF_END(SIE_REG_0510)

/*************************************************************
 * [0514] SIE_REG_0514
 *************************************************************/
#define SIE_REG_0514_OFS    0x0514
REGDEF_BEGIN(SIE_REG_0514)
    REGDEF_BIT(DECOMP_KPX5, 12)
    REGDEF_BIT(DECOMP_KPY5, 20)
REGDEF_END(SIE_REG_0514)

/*************************************************************
 * [0518] SIE_REG_0518
 *************************************************************/
#define SIE_REG_0518_OFS    0x0518
REGDEF_BEGIN(SIE_REG_0518)
    REGDEF_BIT(DECOMP_KPX6, 12)
    REGDEF_BIT(DECOMP_KPY6, 20)
REGDEF_END(SIE_REG_0518)

/*************************************************************
 * [051C] SIE_REG_051C
 *************************************************************/
#define SIE_REG_051C_OFS    0x051C
REGDEF_BEGIN(SIE_REG_051C)
    REGDEF_BIT(DECOMP_KPX7, 12)
    REGDEF_BIT(DECOMP_KPY7, 20)
REGDEF_END(SIE_REG_051C)

/*************************************************************
 * [0520] SIE_REG_0520
 *************************************************************/
#define SIE_REG_0520_OFS    0x0520
REGDEF_BEGIN(SIE_REG_0520)
    REGDEF_BIT(DECOMP_KPX8, 12)
    REGDEF_BIT(DECOMP_KPY8, 20)
REGDEF_END(SIE_REG_0520)

/*************************************************************
 * [0524] SIE_REG_0524
 *************************************************************/
#define SIE_REG_0524_OFS    0x0524
REGDEF_BEGIN(SIE_REG_0524)
    REGDEF_BIT(DECOMP_KPX9, 12)
    REGDEF_BIT(DECOMP_KPY9, 20)
REGDEF_END(SIE_REG_0524)

/*************************************************************
 * [0528] SIE_REG_0528
 *************************************************************/
#define SIE_REG_0528_OFS    0x0528
REGDEF_BEGIN(SIE_REG_0528)
    REGDEF_BIT(DECOMP_KPX10, 12)
    REGDEF_BIT(DECOMP_KPY10, 20)
REGDEF_END(SIE_REG_0528)

/*************************************************************
 * [052C] SIE_REG_052C
 *************************************************************/
#define SIE_REG_052C_OFS    0x052C
REGDEF_BEGIN(SIE_REG_052C)
    REGDEF_BIT(DECOMP_KPX11, 12)
    REGDEF_BIT(DECOMP_KPY11, 20)
REGDEF_END(SIE_REG_052C)

/*************************************************************
 * [0530] SIE_REG_0530
 *************************************************************/
#define SIE_REG_0530_OFS    0x0530
REGDEF_BEGIN(SIE_REG_0530)
    REGDEF_BIT(DECOMP_KPX12, 12)
    REGDEF_BIT(DECOMP_KPY12, 20)
REGDEF_END(SIE_REG_0530)

/*************************************************************
 * [0534] SIE_REG_0534
 *************************************************************/
#define SIE_REG_0534_OFS    0x0534
REGDEF_BEGIN(SIE_REG_0534)
    REGDEF_BIT(DECOMP_KPX13, 12)
    REGDEF_BIT(DECOMP_KPY13, 20)
REGDEF_END(SIE_REG_0534)

/*************************************************************
 * [0538] SIE_REG_0538
 *************************************************************/
#define SIE_REG_0538_OFS    0x0538
REGDEF_BEGIN(SIE_REG_0538)
    REGDEF_BIT(DECOMP_KPX14, 12)
    REGDEF_BIT(DECOMP_KPY14, 20)
REGDEF_END(SIE_REG_0538)

/*************************************************************
 * [053C] SIE_REG_053C
 *************************************************************/
#define SIE_REG_053C_OFS    0x053C
REGDEF_BEGIN(SIE_REG_053C)
    REGDEF_BIT(DECOMP_KPX15, 12)
    REGDEF_BIT(DECOMP_KPY15, 20)
REGDEF_END(SIE_REG_053C)

/*************************************************************
 * [0540] SIE_REG_0540
 *************************************************************/
#define SIE_REG_0540_OFS    0x0540
REGDEF_BEGIN(SIE_REG_0540)
    REGDEF_BIT(DECOMP_KPX16, 12)
    REGDEF_BIT(DECOMP_KPY16, 20)
REGDEF_END(SIE_REG_0540)

/*************************************************************
 * [0544] SIE_REG_0544
 *************************************************************/
#define SIE_REG_0544_OFS    0x0544
REGDEF_BEGIN(SIE_REG_0544)
    REGDEF_BIT(DECOMP_KPX17, 12)
    REGDEF_BIT(DECOMP_KPY17, 20)
REGDEF_END(SIE_REG_0544)

/*************************************************************
 * [0548] SIE_REG_0548
 *************************************************************/
#define SIE_REG_0548_OFS    0x0548
REGDEF_BEGIN(SIE_REG_0548)
    REGDEF_BIT(DECOMP_KPX18, 12)
    REGDEF_BIT(DECOMP_KPY18, 20)
REGDEF_END(SIE_REG_0548)

/*************************************************************
 * [054C] SIE_REG_054C
 *************************************************************/
#define SIE_REG_054C_OFS    0x054C
REGDEF_BEGIN(SIE_REG_054C)
    REGDEF_BIT(DECOMP_KPX19, 12)
    REGDEF_BIT(DECOMP_KPY19, 20)
REGDEF_END(SIE_REG_054C)

/*************************************************************
 * [0550] SIE_REG_0550
 *************************************************************/
#define SIE_REG_0550_OFS    0x0550
REGDEF_BEGIN(SIE_REG_0550)
    REGDEF_BIT(DECOMP_KPX20, 12)
    REGDEF_BIT(DECOMP_KPY20, 20)
REGDEF_END(SIE_REG_0550)

/*************************************************************
 * [0554] SIE_REG_0554
 *************************************************************/
#define SIE_REG_0554_OFS    0x0554
REGDEF_BEGIN(SIE_REG_0554)
    REGDEF_BIT(DECOMP_KPX21, 12)
    REGDEF_BIT(DECOMP_KPY21, 20)
REGDEF_END(SIE_REG_0554)

/*************************************************************
 * [0558] SIE_REG_0558
 *************************************************************/
#define SIE_REG_0558_OFS    0x0558
REGDEF_BEGIN(SIE_REG_0558)
    REGDEF_BIT(DECOMP_KPX22, 12)
    REGDEF_BIT(DECOMP_KPY22, 20)
REGDEF_END(SIE_REG_0558)

/*************************************************************
 * [055C] SIE_REG_055C
 *************************************************************/
#define SIE_REG_055C_OFS    0x055C
REGDEF_BEGIN(SIE_REG_055C)
    REGDEF_BIT(DECOMP_KPX23, 12)
    REGDEF_BIT(DECOMP_KPY23, 20)
REGDEF_END(SIE_REG_055C)

/*************************************************************
 * [0560] SIE_REG_0560
 *************************************************************/
#define SIE_REG_0560_OFS    0x0560
REGDEF_BEGIN(SIE_REG_0560)
    REGDEF_BIT(DECOMP_KPX24, 12)
    REGDEF_BIT(DECOMP_KPY24, 20)
REGDEF_END(SIE_REG_0560)

/*************************************************************
 * [0564] SIE_REG_0564
 *************************************************************/
#define SIE_REG_0564_OFS    0x0564
REGDEF_BEGIN(SIE_REG_0564)
    REGDEF_BIT(DECOMP_KPX25, 12)
    REGDEF_BIT(DECOMP_KPY25, 20)
REGDEF_END(SIE_REG_0564)

/*************************************************************
 * [0568] SIE_REG_0568
 *************************************************************/
#define SIE_REG_0568_OFS    0x0568
REGDEF_BEGIN(SIE_REG_0568)
    REGDEF_BIT(DECOMP_KPX26, 12)
    REGDEF_BIT(DECOMP_KPY26, 20)
REGDEF_END(SIE_REG_0568)

/*************************************************************
 * [056C] SIE_REG_056C
 *************************************************************/
#define SIE_REG_056C_OFS    0x056C
REGDEF_BEGIN(SIE_REG_056C)
    REGDEF_BIT(DECOMP_KPX27, 12)
    REGDEF_BIT(DECOMP_KPY27, 20)
REGDEF_END(SIE_REG_056C)

/*************************************************************
 * [0570] SIE_REG_0570
 *************************************************************/
#define SIE_REG_0570_OFS    0x0570
REGDEF_BEGIN(SIE_REG_0570)
    REGDEF_BIT(DECOMP_KPX28, 12)
    REGDEF_BIT(DECOMP_KPY28, 20)
REGDEF_END(SIE_REG_0570)

/*************************************************************
 * [0574] SIE_REG_0574
 *************************************************************/
#define SIE_REG_0574_OFS    0x0574
REGDEF_BEGIN(SIE_REG_0574)
    REGDEF_BIT(DECOMP_KPX29, 12)
    REGDEF_BIT(DECOMP_KPY29, 20)
REGDEF_END(SIE_REG_0574)

/*************************************************************
 * [0578] SIE_REG_0578
 *************************************************************/
#define SIE_REG_0578_OFS    0x0578
REGDEF_BEGIN(SIE_REG_0578)
    REGDEF_BIT(DECOMP_KPX30, 12)
    REGDEF_BIT(DECOMP_KPY30, 20)
REGDEF_END(SIE_REG_0578)

/*************************************************************
 * [057C] SIE_REG_057C
 *************************************************************/
#define SIE_REG_057C_OFS    0x057C
REGDEF_BEGIN(SIE_REG_057C)
    REGDEF_BIT(DECOMP_KPX31, 12)
    REGDEF_BIT(DECOMP_KPY31, 20)
REGDEF_END(SIE_REG_057C)

/*************************************************************
 * [05A0] SIE_REG_05A0
 *************************************************************/
#define SIE_REG_05A0_OFS    0x05A0
REGDEF_BEGIN(SIE_REG_05A0)
    REGDEF_BIT(DECOMP_GAIN0 , 10)
    REGDEF_BIT(DECOMP_SB0   , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN1 , 10)
    REGDEF_BIT(DECOMP_SB1   , 5)
REGDEF_END(SIE_REG_05A0)

/*************************************************************
 * [05A4] SIE_REG_05A4
 *************************************************************/
#define SIE_REG_05A4_OFS    0x05A4
REGDEF_BEGIN(SIE_REG_05A4)
    REGDEF_BIT(DECOMP_GAIN2 , 10)
    REGDEF_BIT(DECOMP_SB2   , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN3 , 10)
    REGDEF_BIT(DECOMP_SB3   , 5)
REGDEF_END(SIE_REG_05A4)

/*************************************************************
 * [05A8] SIE_REG_05A8
 *************************************************************/
#define SIE_REG_05A8_OFS    0x05A8
REGDEF_BEGIN(SIE_REG_05A8)
    REGDEF_BIT(DECOMP_GAIN4 , 10)
    REGDEF_BIT(DECOMP_SB4   , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN5 , 10)
    REGDEF_BIT(DECOMP_SB5   , 5)
REGDEF_END(SIE_REG_05A8)

/*************************************************************
 * [05AC] SIE_REG_05AC
 *************************************************************/
#define SIE_REG_05AC_OFS    0x05AC
REGDEF_BEGIN(SIE_REG_05AC)
    REGDEF_BIT(DECOMP_GAIN6 , 10)
    REGDEF_BIT(DECOMP_SB6   , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN7 , 10)
    REGDEF_BIT(DECOMP_SB7   , 5)
REGDEF_END(SIE_REG_05AC)

/*************************************************************
 * [05B0] SIE_REG_05B0
 *************************************************************/
#define SIE_REG_05B0_OFS    0x05B0
REGDEF_BEGIN(SIE_REG_05B0)
    REGDEF_BIT(DECOMP_GAIN8 , 10)
    REGDEF_BIT(DECOMP_SB8   , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN9 , 10)
    REGDEF_BIT(DECOMP_SB9   , 5)
REGDEF_END(SIE_REG_05B0)

/*************************************************************
 * [05B4] SIE_REG_05B4
 *************************************************************/
#define SIE_REG_05B4_OFS    0x05B4
REGDEF_BEGIN(SIE_REG_05B4)
    REGDEF_BIT(DECOMP_GAIN10, 10)
    REGDEF_BIT(DECOMP_SB10  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN11, 10)
    REGDEF_BIT(DECOMP_SB11  , 5)
REGDEF_END(SIE_REG_05B4)

/*************************************************************
 * [05B8] SIE_REG_05B8
 *************************************************************/
#define SIE_REG_05B8_OFS    0x05B8
REGDEF_BEGIN(SIE_REG_05B8)
    REGDEF_BIT(DECOMP_GAIN12, 10)
    REGDEF_BIT(DECOMP_SB12  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN13, 10)
    REGDEF_BIT(DECOMP_SB13  , 5)
REGDEF_END(SIE_REG_05B8)

/*************************************************************
 * [05BC] SIE_REG_05BC
 *************************************************************/
#define SIE_REG_05BC_OFS    0x05BC
REGDEF_BEGIN(SIE_REG_05BC)
    REGDEF_BIT(DECOMP_GAIN14, 10)
    REGDEF_BIT(DECOMP_SB14  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN15, 10)
    REGDEF_BIT(DECOMP_SB15  , 5)
REGDEF_END(SIE_REG_05BC)

/*************************************************************
 * [05C0] SIE_REG_05C0
 *************************************************************/
#define SIE_REG_05C0_OFS    0x05C0
REGDEF_BEGIN(SIE_REG_05C0)
    REGDEF_BIT(DECOMP_GAIN16, 10)
    REGDEF_BIT(DECOMP_SB16  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN17, 10)
    REGDEF_BIT(DECOMP_SB17  , 5)
REGDEF_END(SIE_REG_05C0)

/*************************************************************
 * [05C4] SIE_REG_05C4
 *************************************************************/
#define SIE_REG_05C4_OFS    0x05C4
REGDEF_BEGIN(SIE_REG_05C4)
    REGDEF_BIT(DECOMP_GAIN18, 10)
    REGDEF_BIT(DECOMP_SB18  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN19, 10)
    REGDEF_BIT(DECOMP_SB19  , 5)
REGDEF_END(SIE_REG_05C4)

/*************************************************************
 * [05C8] SIE_REG_05C8
 *************************************************************/
#define SIE_REG_05C8_OFS    0x05C8
REGDEF_BEGIN(SIE_REG_05C8)
    REGDEF_BIT(DECOMP_GAIN20, 10)
    REGDEF_BIT(DECOMP_SB20  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN21, 10)
    REGDEF_BIT(DECOMP_SB21  , 5)
REGDEF_END(SIE_REG_05C8)

/*************************************************************
 * [05CC] SIE_REG_05CC
 *************************************************************/
#define SIE_REG_05CC_OFS    0x05CC
REGDEF_BEGIN(SIE_REG_05CC)
    REGDEF_BIT(DECOMP_GAIN22, 10)
    REGDEF_BIT(DECOMP_SB22  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN23, 10)
    REGDEF_BIT(DECOMP_SB23  , 5)
REGDEF_END(SIE_REG_05CC)

/*************************************************************
 * [05D0] SIE_REG_05D0
 *************************************************************/
#define SIE_REG_05D0_OFS    0x05D0
REGDEF_BEGIN(SIE_REG_05D0)
    REGDEF_BIT(DECOMP_GAIN24, 10)
    REGDEF_BIT(DECOMP_SB24  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN25, 10)
    REGDEF_BIT(DECOMP_SB25  , 5)
REGDEF_END(SIE_REG_05D0)

/*************************************************************
 * [05D4] SIE_REG_05D4
 *************************************************************/
#define SIE_REG_05D4_OFS    0x05D4
REGDEF_BEGIN(SIE_REG_05D4)
    REGDEF_BIT(DECOMP_GAIN26, 10)
    REGDEF_BIT(DECOMP_SB26  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN27, 10)
    REGDEF_BIT(DECOMP_SB27  , 5)
REGDEF_END(SIE_REG_05D4)

/*************************************************************
 * [05D8] SIE_REG_05D8
 *************************************************************/
#define SIE_REG_05D8_OFS    0x05D8
REGDEF_BEGIN(SIE_REG_05D8)
    REGDEF_BIT(DECOMP_GAIN28, 10)
    REGDEF_BIT(DECOMP_SB28  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN29, 10)
    REGDEF_BIT(DECOMP_SB29  , 5)
REGDEF_END(SIE_REG_05D8)

/*************************************************************
 * [05DC] SIE_REG_05DC
 *************************************************************/
#define SIE_REG_05DC_OFS    0x05DC
REGDEF_BEGIN(SIE_REG_05DC)
    REGDEF_BIT(DECOMP_GAIN30, 10)
    REGDEF_BIT(DECOMP_SB30  , 5)
    REGDEF_BIT(             , 1)
    REGDEF_BIT(DECOMP_GAIN31, 10)
    REGDEF_BIT(DECOMP_SB31  , 5)
REGDEF_END(SIE_REG_05DC)

/*************************************************************
 * [0600] SIE_REG_0600
 *************************************************************/
#define SIE_REG_0600_OFS    0x0600
REGDEF_BEGIN(SIE_REG_0600)
    REGDEF_BIT(COMP_FCURVE_L0 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L1 , 12)
REGDEF_END(SIE_REG_0600)

/*************************************************************
 * [0604] SIE_REG_0604
 *************************************************************/
#define SIE_REG_0604_OFS    0x0604
REGDEF_BEGIN(SIE_REG_0604)
    REGDEF_BIT(COMP_FCURVE_L2 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L3 , 12)
REGDEF_END(SIE_REG_0604)

/*************************************************************
 * [0608] SIE_REG_0608
 *************************************************************/
#define SIE_REG_0608_OFS    0x0608
REGDEF_BEGIN(SIE_REG_0608)
    REGDEF_BIT(COMP_FCURVE_L4 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L5 , 12)
REGDEF_END(SIE_REG_0608)

/*************************************************************
 * [060C] SIE_REG_060C
 *************************************************************/
#define SIE_REG_060C_OFS    0x060C
REGDEF_BEGIN(SIE_REG_060C)
    REGDEF_BIT(COMP_FCURVE_L6 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L7 , 12)
REGDEF_END(SIE_REG_060C)

/*************************************************************
 * [0610] SIE_REG_0610
 *************************************************************/
#define SIE_REG_0610_OFS    0x0610
REGDEF_BEGIN(SIE_REG_0610)
    REGDEF_BIT(COMP_FCURVE_L8 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L9 , 12)
REGDEF_END(SIE_REG_0610)

/*************************************************************
 * [0614] SIE_REG_0614
 *************************************************************/
#define SIE_REG_0614_OFS    0x0614
REGDEF_BEGIN(SIE_REG_0614)
    REGDEF_BIT(COMP_FCURVE_L10, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L11, 12)
REGDEF_END(SIE_REG_0614)

/*************************************************************
 * [0618] SIE_REG_0618
 *************************************************************/
#define SIE_REG_0618_OFS    0x0618
REGDEF_BEGIN(SIE_REG_0618)
    REGDEF_BIT(COMP_FCURVE_L12, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L13, 12)
REGDEF_END(SIE_REG_0618)

/*************************************************************
 * [061C] SIE_REG_061C
 *************************************************************/
#define SIE_REG_061C_OFS    0x061C
REGDEF_BEGIN(SIE_REG_061C)
    REGDEF_BIT(COMP_FCURVE_L14, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L15, 12)
REGDEF_END(SIE_REG_061C)

/*************************************************************
 * [0620] SIE_REG_0620
 *************************************************************/
#define SIE_REG_0620_OFS    0x0620
REGDEF_BEGIN(SIE_REG_0620)
    REGDEF_BIT(COMP_FCURVE_L16, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L17, 12)
REGDEF_END(SIE_REG_0620)

/*************************************************************
 * [0624] SIE_REG_0624
 *************************************************************/
#define SIE_REG_0624_OFS    0x0624
REGDEF_BEGIN(SIE_REG_0624)
    REGDEF_BIT(COMP_FCURVE_L18, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L19, 12)
REGDEF_END(SIE_REG_0624)

/*************************************************************
 * [0628] SIE_REG_0628
 *************************************************************/
#define SIE_REG_0628_OFS    0x0628
REGDEF_BEGIN(SIE_REG_0628)
    REGDEF_BIT(COMP_FCURVE_L20, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L21, 12)
REGDEF_END(SIE_REG_0628)

/*************************************************************
 * [062C] SIE_REG_062C
 *************************************************************/
#define SIE_REG_062C_OFS    0x062C
REGDEF_BEGIN(SIE_REG_062C)
    REGDEF_BIT(COMP_FCURVE_L22, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L23, 12)
REGDEF_END(SIE_REG_062C)

/*************************************************************
 * [0630] SIE_REG_0630
 *************************************************************/
#define SIE_REG_0630_OFS    0x0630
REGDEF_BEGIN(SIE_REG_0630)
    REGDEF_BIT(COMP_FCURVE_L24, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L25, 12)
REGDEF_END(SIE_REG_0630)

/*************************************************************
 * [0634] SIE_REG_0634
 *************************************************************/
#define SIE_REG_0634_OFS    0x0634
REGDEF_BEGIN(SIE_REG_0634)
    REGDEF_BIT(COMP_FCURVE_L26, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L27, 12)
REGDEF_END(SIE_REG_0634)

/*************************************************************
 * [0638] SIE_REG_0638
 *************************************************************/
#define SIE_REG_0638_OFS    0x0638
REGDEF_BEGIN(SIE_REG_0638)
    REGDEF_BIT(COMP_FCURVE_L28, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L29, 12)
REGDEF_END(SIE_REG_0638)

/*************************************************************
 * [063C] SIE_REG_063C
 *************************************************************/
#define SIE_REG_063C_OFS    0x063C
REGDEF_BEGIN(SIE_REG_063C)
    REGDEF_BIT(COMP_FCURVE_L30, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L31, 12)
REGDEF_END(SIE_REG_063C)

/*************************************************************
 * [0640] SIE_REG_0640
 *************************************************************/
#define SIE_REG_0640_OFS    0x0640
REGDEF_BEGIN(SIE_REG_0640)
    REGDEF_BIT(COMP_FCURVE_L32, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L33, 12)
REGDEF_END(SIE_REG_0640)

/*************************************************************
 * [0644] SIE_REG_0644
 *************************************************************/
#define SIE_REG_0644_OFS    0x0644
REGDEF_BEGIN(SIE_REG_0644)
    REGDEF_BIT(COMP_FCURVE_L34, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L35, 12)
REGDEF_END(SIE_REG_0644)

/*************************************************************
 * [0648] SIE_REG_0648
 *************************************************************/
#define SIE_REG_0648_OFS    0x0648
REGDEF_BEGIN(SIE_REG_0648)
    REGDEF_BIT(COMP_FCURVE_L36, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L37, 12)
REGDEF_END(SIE_REG_0648)

/*************************************************************
 * [064C] SIE_REG_064C
 *************************************************************/
#define SIE_REG_064C_OFS    0x064C
REGDEF_BEGIN(SIE_REG_064C)
    REGDEF_BIT(COMP_FCURVE_L38, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L39, 12)
REGDEF_END(SIE_REG_064C)

/*************************************************************
 * [0650] SIE_REG_0650
 *************************************************************/
#define SIE_REG_0650_OFS    0x0650
REGDEF_BEGIN(SIE_REG_0650)
    REGDEF_BIT(COMP_FCURVE_L40, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L41, 12)
REGDEF_END(SIE_REG_0650)

/*************************************************************
 * [0654] SIE_REG_0654
 *************************************************************/
#define SIE_REG_0654_OFS    0x0654
REGDEF_BEGIN(SIE_REG_0654)
    REGDEF_BIT(COMP_FCURVE_L42, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L43, 12)
REGDEF_END(SIE_REG_0654)

/*************************************************************
 * [0658] SIE_REG_0658
 *************************************************************/
#define SIE_REG_0658_OFS    0x0658
REGDEF_BEGIN(SIE_REG_0658)
    REGDEF_BIT(COMP_FCURVE_L44, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L45, 12)
REGDEF_END(SIE_REG_0658)

/*************************************************************
 * [065C] SIE_REG_065C
 *************************************************************/
#define SIE_REG_065C_OFS    0x065C
REGDEF_BEGIN(SIE_REG_065C)
    REGDEF_BIT(COMP_FCURVE_L46, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L47, 12)
REGDEF_END(SIE_REG_065C)

/*************************************************************
 * [0660] SIE_REG_0660
 *************************************************************/
#define SIE_REG_0660_OFS    0x0660
REGDEF_BEGIN(SIE_REG_0660)
    REGDEF_BIT(COMP_FCURVE_L48, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L49, 12)
REGDEF_END(SIE_REG_0660)

/*************************************************************
 * [0664] SIE_REG_0664
 *************************************************************/
#define SIE_REG_0664_OFS    0x0664
REGDEF_BEGIN(SIE_REG_0664)
    REGDEF_BIT(COMP_FCURVE_L50, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L51, 12)
REGDEF_END(SIE_REG_0664)

/*************************************************************
 * [0668] SIE_REG_0668
 *************************************************************/
#define SIE_REG_0668_OFS    0x0668
REGDEF_BEGIN(SIE_REG_0668)
    REGDEF_BIT(COMP_FCURVE_L52, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L53, 12)
REGDEF_END(SIE_REG_0668)

/*************************************************************
 * [066C] SIE_REG_066C
 *************************************************************/
#define SIE_REG_066C_OFS    0x066C
REGDEF_BEGIN(SIE_REG_066C)
    REGDEF_BIT(COMP_FCURVE_L54, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L55, 12)
REGDEF_END(SIE_REG_066C)

/*************************************************************
 * [0670] SIE_REG_0670
 *************************************************************/
#define SIE_REG_0670_OFS    0x0670
REGDEF_BEGIN(SIE_REG_0670)
    REGDEF_BIT(COMP_FCURVE_L56, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L57, 12)
REGDEF_END(SIE_REG_0670)

/*************************************************************
 * [0674] SIE_REG_0674
 *************************************************************/
#define SIE_REG_0674_OFS    0x0674
REGDEF_BEGIN(SIE_REG_0674)
    REGDEF_BIT(COMP_FCURVE_L58, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L59, 12)
REGDEF_END(SIE_REG_0674)

/*************************************************************
 * [0678] SIE_REG_0678
 *************************************************************/
#define SIE_REG_0678_OFS    0x0678
REGDEF_BEGIN(SIE_REG_0678)
    REGDEF_BIT(COMP_FCURVE_L60, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L61, 12)
REGDEF_END(SIE_REG_0678)

/*************************************************************
 * [067C] SIE_REG_067C
 *************************************************************/
#define SIE_REG_067C_OFS    0x067C
REGDEF_BEGIN(SIE_REG_067C)
    REGDEF_BIT(COMP_FCURVE_L62, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_L63, 12)
REGDEF_END(SIE_REG_067C)

/*************************************************************
 * [0680] SIE_REG_0680
 *************************************************************/
#define SIE_REG_0680_OFS    0x0680
REGDEF_BEGIN(SIE_REG_0680)
    REGDEF_BIT(COMP_FCURVE_L64, 12)
REGDEF_END(SIE_REG_0680)

/*************************************************************
 * [0690] SIE_REG_0690
 *************************************************************/
#define SIE_REG_0690_OFS    0x0690
REGDEF_BEGIN(SIE_REG_0690)
    REGDEF_BIT(COMP_FCURVE_M0 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M1 , 12)
REGDEF_END(SIE_REG_0690)

/*************************************************************
 * [0694] SIE_REG_0694
 *************************************************************/
#define SIE_REG_0694_OFS    0x0694
REGDEF_BEGIN(SIE_REG_0694)
    REGDEF_BIT(COMP_FCURVE_M2 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M3 , 12)
REGDEF_END(SIE_REG_0694)

/*************************************************************
 * [0698] SIE_REG_0698
 *************************************************************/
#define SIE_REG_0698_OFS    0x0698
REGDEF_BEGIN(SIE_REG_0698)
    REGDEF_BIT(COMP_FCURVE_M4 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M5 , 12)
REGDEF_END(SIE_REG_0698)

/*************************************************************
 * [069C] SIE_REG_069C
 *************************************************************/
#define SIE_REG_069C_OFS    0x069C
REGDEF_BEGIN(SIE_REG_069C)
    REGDEF_BIT(COMP_FCURVE_M6 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M7 , 12)
REGDEF_END(SIE_REG_069C)

/*************************************************************
 * [06A0] SIE_REG_06A0
 *************************************************************/
#define SIE_REG_06A0_OFS    0x06A0
REGDEF_BEGIN(SIE_REG_06A0)
    REGDEF_BIT(COMP_FCURVE_M8 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M9 , 12)
REGDEF_END(SIE_REG_06A0)

/*************************************************************
 * [06A4] SIE_REG_06A4
 *************************************************************/
#define SIE_REG_06A4_OFS    0x06A4
REGDEF_BEGIN(SIE_REG_06A4)
    REGDEF_BIT(COMP_FCURVE_M10, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M11, 12)
REGDEF_END(SIE_REG_06A4)

/*************************************************************
 * [06A8] SIE_REG_06A8
 *************************************************************/
#define SIE_REG_06A8_OFS    0x06A8
REGDEF_BEGIN(SIE_REG_06A8)
    REGDEF_BIT(COMP_FCURVE_M12, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M13, 12)
REGDEF_END(SIE_REG_06A8)

/*************************************************************
 * [06AC] SIE_REG_06AC
 *************************************************************/
#define SIE_REG_06AC_OFS    0x06AC
REGDEF_BEGIN(SIE_REG_06AC)
    REGDEF_BIT(COMP_FCURVE_M14, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_M15, 12)
REGDEF_END(SIE_REG_06AC)

/*************************************************************
 * [06B0] SIE_REG_06B0
 *************************************************************/
#define SIE_REG_06B0_OFS    0x06B0
REGDEF_BEGIN(SIE_REG_06B0)
    REGDEF_BIT(COMP_FCURVE_M16, 12)
REGDEF_END(SIE_REG_06B0)

/*************************************************************
 * [06C0] SIE_REG_06C0
 *************************************************************/
#define SIE_REG_06C0_OFS    0x06C0
REGDEF_BEGIN(SIE_REG_06C0)
    REGDEF_BIT(COMP_FCURVE_R0 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R1 , 12)
REGDEF_END(SIE_REG_06C0)

/*************************************************************
 * [06C4] SIE_REG_06C4
 *************************************************************/
#define SIE_REG_06C4_OFS    0x06C4
REGDEF_BEGIN(SIE_REG_06C4)
    REGDEF_BIT(COMP_FCURVE_R2 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R3 , 12)
REGDEF_END(SIE_REG_06C4)

/*************************************************************
 * [06C8] SIE_REG_06C8
 *************************************************************/
#define SIE_REG_06C8_OFS    0x06C8
REGDEF_BEGIN(SIE_REG_06C8)
    REGDEF_BIT(COMP_FCURVE_R4 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R5 , 12)
REGDEF_END(SIE_REG_06C8)

/*************************************************************
 * [06CC] SIE_REG_06CC
 *************************************************************/
#define SIE_REG_06CC_OFS    0x06CC
REGDEF_BEGIN(SIE_REG_06CC)
    REGDEF_BIT(COMP_FCURVE_R6 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R7 , 12)
REGDEF_END(SIE_REG_06CC)

/*************************************************************
 * [06D0] SIE_REG_06D0
 *************************************************************/
#define SIE_REG_06D0_OFS    0x06D0
REGDEF_BEGIN(SIE_REG_06D0)
    REGDEF_BIT(COMP_FCURVE_R8 , 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R9 , 12)
REGDEF_END(SIE_REG_06D0)

/*************************************************************
 * [06D4] SIE_REG_06D4
 *************************************************************/
#define SIE_REG_06D4_OFS    0x06D4
REGDEF_BEGIN(SIE_REG_06D4)
    REGDEF_BIT(COMP_FCURVE_R10, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R11, 12)
REGDEF_END(SIE_REG_06D4)

/*************************************************************
 * [06D8] SIE_REG_06D8
 *************************************************************/
#define SIE_REG_06D8_OFS    0x06D8
REGDEF_BEGIN(SIE_REG_06D8)
    REGDEF_BIT(COMP_FCURVE_R12, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R13, 12)
REGDEF_END(SIE_REG_06D8)

/*************************************************************
 * [06DC] SIE_REG_06DC
 *************************************************************/
#define SIE_REG_06DC_OFS    0x06DC
REGDEF_BEGIN(SIE_REG_06DC)
    REGDEF_BIT(COMP_FCURVE_R14, 12)
    REGDEF_BIT(               , 4)
    REGDEF_BIT(COMP_FCURVE_R15, 12)
REGDEF_END(SIE_REG_06DC)

/*************************************************************
 * [06E0] SIE_REG_06E0
 *************************************************************/
#define SIE_REG_06E0_OFS    0x06E0
REGDEF_BEGIN(SIE_REG_06E0)
    REGDEF_BIT(,                   16)
    REGDEF_BIT(COMP_FCURVE_EV_FMT, 1)
    REGDEF_BIT(,                   3)
    REGDEF_BIT(COMP_FCURVE_Y_EN,   1)
REGDEF_END(SIE_REG_06E0)

/*************************************************************
 * [06E4] SIE_REG_06E4
 *************************************************************/
#define SIE_REG_06E4_OFS   0x06E4
REGDEF_BEGIN(SIE_REG_06E4)
    REGDEF_BIT(COMP_YWEIGHT_LUT0, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT1, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT2, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT3, 8)
REGDEF_END(SIE_REG_06E4)

/*************************************************************
 * [06E8] SIE_REG_06E8
 *************************************************************/
#define SIE_REG_06E8_OFS    0x06E8
REGDEF_BEGIN(SIE_REG_06E8)
    REGDEF_BIT(COMP_YWEIGHT_LUT4, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT5, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT6, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT7, 8)
REGDEF_END(SIE_REG_06E8)

/*************************************************************
 * [06EC] SIE_REG_06EC
 *************************************************************/
#define SIE_REG_06EC_OFS    0x06EC
REGDEF_BEGIN(SIE_REG_06EC)
    REGDEF_BIT(COMP_YWEIGHT_LUT8 , 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT9 , 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT10, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT11, 8)
REGDEF_END(SIE_REG_06EC)

/*************************************************************
 * [06F0] SIE_REG_06F0
 *************************************************************/
#define SIE_REG_06F0_OFS    0x06F0
REGDEF_BEGIN(SIE_REG_06F0)
    REGDEF_BIT(COMP_YWEIGHT_LUT12, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT13, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT14, 8)
    REGDEF_BIT(COMP_YWEIGHT_LUT15, 8)
REGDEF_END(SIE_REG_06F0)

/*************************************************************
 * [06F4] SIE_REG_06F4
 *************************************************************/
#define SIE_REG_06F4_OFS 0x06F4
REGDEF_BEGIN(SIE_REG_06F4)
    REGDEF_BIT(COMP_YWEIGHT_LUT16, 8)
REGDEF_END(SIE_REG_06F4)

/*************************************************************
 * [06FC] SIE_REG_06FC
 *************************************************************/
#define SIE_REG_06FC_OFS 0x06FC
REGDEF_BEGIN(SIE_REG_06FC)
    REGDEF_BIT(COMP_Y_GAIN_MAX,   12)
    REGDEF_BIT(COMP_Y_GAIN_SHIFT,  4)
REGDEF_END(SIE_REG_06FC)

/*************************************************************
 * [07C0] SIE_REG_07C0
 *************************************************************/
#define SIE_REG_07C0_OFS    0x07C0
REGDEF_BEGIN(SIE_REG_07C0)
    REGDEF_BIT(LN_CNT    , 16)
    REGDEF_BIT(LN_CNT_MAX, 16)
REGDEF_END(SIE_REG_07C0)

/*************************************************************
 * [07C4] SIE_REG_07C4
 *************************************************************/
#define SIE_REG_07C4_OFS    0x07C4
REGDEF_BEGIN(SIE_REG_07C4)
    REGDEF_BIT(CHECKSUM_ACT, 32)
REGDEF_END(SIE_REG_07C4)

/*************************************************************
 * [07C8] SIE_REG_07C8
 *************************************************************/
#define SIE_REG_07C8_OFS    0x07C8
REGDEF_BEGIN(SIE_REG_07C8)
    REGDEF_BIT(CHECKSUM_ECS, 32)
REGDEF_END(SIE_REG_07C8)

/*************************************************************
 * [07CC] SIE_REG_07CC
 *************************************************************/
#define SIE_REG_07CC_OFS    0x07CC
REGDEF_BEGIN(SIE_REG_07CC)
    REGDEF_BIT(CHECKSUM_CROP, 32)
REGDEF_END(SIE_REG_07CC)

/*************************************************************
 * [07D0] SIE_REG_07D0
 *************************************************************/
#define SIE_REG_07D0_OFS    0x07D0
REGDEF_BEGIN(SIE_REG_07D0)
    REGDEF_BIT(CHECKSUM_BCC_OUT, 32)
REGDEF_END(SIE_REG_07D0)

/*************************************************************
 * [07D4] SIE_REG_07D4
 *************************************************************/
#define SIE_REG_07D4_OFS    0x07D4
REGDEF_BEGIN(SIE_REG_07D4)
    REGDEF_BIT(CHECKSUM_OCH0_BUF_IN, 32)
REGDEF_END(SIE_REG_07D4)

/*************************************************************
 * [07DC] SIE_REG_07DC
 *************************************************************/
#define SIE_REG_07DC_OFS    0x07DC
REGDEF_BEGIN(SIE_REG_07DC)
    REGDEF_BIT(CHECKSUM_DPC_DRAM_IN, 32)
REGDEF_END(SIE_REG_07DC)

/*************************************************************
 * [07E0] SIE_REG_07E0
 *************************************************************/
#define SIE_REG_07E0_OFS    0x07E0
REGDEF_BEGIN(SIE_REG_07E0)
    REGDEF_BIT(CHECKSUM_ECS_DRAM_IN, 32)
REGDEF_END(SIE_REG_07E0)

/*************************************************************
 * [07A4] SIE_REG_07E4
 *************************************************************/
#define SIE_REG_07E4_OFS    0x07E4
REGDEF_BEGIN(SIE_REG_07E4)
    REGDEF_BIT(DEBUG_HD_CNT      , 16)
    REGDEF_BIT(DEBUG_LINE_END_CNT, 16)
REGDEF_END(SIE_REG_07E4)

/*************************************************************
 * [07E8] SIE_REG_07E8
 *************************************************************/
#define SIE_REG_07E8_OFS    0x07E8
REGDEF_BEGIN(SIE_REG_07E8)
    REGDEF_BIT(DEBUG_INPUT_PXL_CNT, 16)
REGDEF_END(SIE_REG_07E8)

/*************************************************************
 * [07F0] SIE_REG_07F0
 *************************************************************/
#define SIE_REG_07F0_OFS    0x07F0
REGDEF_BEGIN(SIE_REG_07F0)
    REGDEF_BIT(SIE_VD_TIME, 32)
REGDEF_END(SIE_REG_07F0)

/*************************************************************
 * [07F4] SIE_REG_07F4
 *************************************************************/
#define SIE_REG_07F4_OFS    0x07F4
REGDEF_BEGIN(SIE_REG_07F4)
    REGDEF_BIT(SIE_HD_TIME, 32)
REGDEF_END(SIE_REG_07F4)

/*************************************************************
 * [07F8] SIE_REG_07F8
 *************************************************************/
#define SIE_REG_07F8_OFS    0x07F8
REGDEF_BEGIN(SIE_REG_07F8)
    REGDEF_BIT(CROP_WIN_TIME, 32)
REGDEF_END(SIE_REG_07F8)

/*************************************************************
 * [07FC] SIE_REG_07FC
 *************************************************************/
#define SIE_REG_07FC_OFS    0x07FC
REGDEF_BEGIN(SIE_REG_07FC)
    REGDEF_BIT(CROPEND_VD_TIME, 32)
REGDEF_END(SIE_REG_07FC)

/*************************************************************
 * [0A00] SIE_REG_0A00
 *************************************************************/
#define SIE_REG_0A00_OFS    0x0A00
REGDEF_BEGIN(SIE_REG_0A00)
    REGDEF_BIT(BRC_SEGBITNO,     3)
    REGDEF_BIT(,                 1)
    REGDEF_BIT(BCC_SEL,          1)
    REGDEF_BIT(,                 3)
    REGDEF_BIT(BCC_STCS_ID_SEL,  2)
    REGDEF_BIT(,                 2)
    REGDEF_BIT(BCC_FRM_RST_ID,   2)
REGDEF_END(SIE_REG_0A00)

/*************************************************************
 * [0A04] SIE_REG_0A04
 *************************************************************/
#define SIE_REG_0A04_OFS    0x0A04
REGDEF_BEGIN(SIE_REG_0A04)
    REGDEF_BIT(        , 4)
    REGDEF_BIT(GAMMA_EN, 1)
REGDEF_END(SIE_REG_0A04)

/*************************************************************
 * [0A10] SIE_REG_0A10
 *************************************************************/
#define SIE_REG_0A10_OFS    0x0A10
REGDEF_BEGIN(SIE_REG_0A10)
    REGDEF_BIT(DCT_QTBL0_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL1_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL2_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL3_IDX, 5)
REGDEF_END(SIE_REG_0A10)

/*************************************************************
 * [0A14] SIE_REG_0A14
 *************************************************************/
#define SIE_REG_0A14_OFS    0x0A14
REGDEF_BEGIN(SIE_REG_0A14)
    REGDEF_BIT(DCT_QTBL4_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL5_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL6_IDX, 5)
    REGDEF_BIT(             , 3)
    REGDEF_BIT(DCT_QTBL7_IDX, 5)
REGDEF_END(SIE_REG_0A14)

/*************************************************************
 * [0A18] SIE_REG_0A18
 *************************************************************/
#define SIE_REG_0A18_OFS    0x0A18
REGDEF_BEGIN(SIE_REG_0A18)
    REGDEF_BIT(DCT_LEVEL_TH0, 8)
    REGDEF_BIT(DCT_LEVEL_TH1, 8)
    REGDEF_BIT(DCT_LEVEL_TH2, 8)
    REGDEF_BIT(DCT_LEVEL_TH3, 8)
REGDEF_END(SIE_REG_0A18)

/*************************************************************
 * [0B00] SIE_REG_0B00
 *************************************************************/
#define SIE_REG_0B00_OFS    0x0B00
REGDEF_BEGIN(SIE_REG_0B00)
    REGDEF_BIT(TSEN_TX_START,        1)
    REGDEF_BIT(TSEN_TX_STAGE,        1)
    REGDEF_BIT(TSEN_TX_OOC_LOAD,     1)
    REGDEF_BIT(TSEN_TX_COMBINE_EN,   1)
    REGDEF_BIT(TSEN_TX_FS_EN,        1)
    REGDEF_BIT(TSEN_TX_FS_OUT_INV,   1)
    REGDEF_BIT(TSEN_TX_FS_HILO_INV,  1)
    REGDEF_BIT(TSEN_TX_FS_IDLE_MODE, 1)
REGDEF_END(SIE_REG_0B00)

/*************************************************************
 * [0B04] SIE_REG_0B04
 *************************************************************/
#define SIE_REG_0B04_OFS    0x0B04
REGDEF_BEGIN(SIE_REG_0B04)
    REGDEF_BIT(TSEN_TX_CONFIG_MODE,       3)
    REGDEF_BIT(,                          1)
    REGDEF_BIT(TSEN_TX_CONFIG_TRANS_LEN, 10)
    REGDEF_BIT(,                          2)
    REGDEF_BIT(TSEN_TX_CONFIG_PRD,        5)
REGDEF_END(SIE_REG_0B04)

/*************************************************************
 * [0B08] SIE_REG_0B08
 *************************************************************/
#define SIE_REG_0B08_OFS    0x0B08
REGDEF_BEGIN(SIE_REG_0B08)
    REGDEF_BIT(TSEN_TX_SYNC_CODE_MODE,    2)
    REGDEF_BIT(,                          2)
    REGDEF_BIT(TSEN_TX_FS_CODE_TRANS_LEN, 6)
    REGDEF_BIT(,                          2)
    REGDEF_BIT(TSEN_TX_LS_CODE_TRANS_LEN, 6)
    REGDEF_BIT(,                          2)
    REGDEF_BIT(TSEN_TX_LEVEL_SD0,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD1,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD2,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD3,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD4,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD5,         1)
    REGDEF_BIT(TSEN_TX_LEVEL_SD6,         1)
REGDEF_END(SIE_REG_0B08)

/*************************************************************
 * [0B0C] SIE_REG_0B0C
 *************************************************************/
#define SIE_REG_0B0C_OFS    0x0B0C
REGDEF_BEGIN(SIE_REG_0B0C)
    REGDEF_BIT(TSEN_TX_OOC_TRANS_MODE,     3)
    REGDEF_BIT(TSEN_TX_OOC_SWAP,           1)
    REGDEF_BIT(TSEN_TX_OOC_RSFT,           3)
    REGDEF_BIT(,                           1)
    REGDEF_BIT(TSEN_TX_OOC_TRANS_BITDEPTH, 2)
REGDEF_END(SIE_REG_0B0C)

/*************************************************************
 * [0B10] SIE_REG_0B10
 *************************************************************/
#define SIE_REG_0B10_OFS    0x0B10
REGDEF_BEGIN(SIE_REG_0B10)
    REGDEF_BIT(TSEN_TX_OOC_WIDTH,  12)
    REGDEF_BIT(TSEN_TX_OOC_HEIGHT, 12)
REGDEF_END(SIE_REG_0B10)

/*************************************************************
 * [0B14] SIE_REG_0B14
 *************************************************************/
#define SIE_REG_0B14_OFS    0x0B14
REGDEF_BEGIN(SIE_REG_0B14)
    REGDEF_BIT(TSEN_TX_OOC_DUMMY_TOP,    8)
    REGDEF_BIT(TSEN_TX_OOC_DUMMY_BOTTOM, 8)
REGDEF_END(SIE_REG_0B14)

/*************************************************************
 * [0B18] SIE_REG_0B18
 *************************************************************/
#define SIE_REG_0B18_OFS    0x0B18
REGDEF_BEGIN(SIE_REG_0B18)
    REGDEF_BIT(TSEN_TX_OOC_DUMMY_VAL,    8)
    REGDEF_BIT(TSEN_TX_OOC_ERR_CODE,     8)
    REGDEF_BIT(TSEN_TX_OOC_ERR_CODE_VAL, 8)
REGDEF_END(SIE_REG_0B18)

/*************************************************************
 * [0B20] SIE_REG_0B20
 *************************************************************/
#define SIE_REG_0B20_OFS    0x0B20
REGDEF_BEGIN(SIE_REG_0B20)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD0, 32)
REGDEF_END(SIE_REG_0B20)

/*************************************************************
 * [0B24] SIE_REG_0B24
 *************************************************************/
#define SIE_REG_0B24_OFS    0x0B24
REGDEF_BEGIN(SIE_REG_0B24)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD1, 32)
REGDEF_END(SIE_REG_0B24)

/*************************************************************
 * [0B28] SIE_REG_0B28
 *************************************************************/
#define SIE_REG_0B28_OFS    0x0B28
REGDEF_BEGIN(SIE_REG_0B28)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD2, 32)
REGDEF_END(SIE_REG_0B28)

/*************************************************************
 * [0B2C] SIE_REG_0B2C
 *************************************************************/
#define SIE_REG_0B2C_OFS    0x0B2C
REGDEF_BEGIN(SIE_REG_0B2C)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD3, 32)
REGDEF_END(SIE_REG_0B2C)

/*************************************************************
 * [0B30] SIE_REG_0B30
 *************************************************************/
#define SIE_REG_0B30_OFS    0x0B30
REGDEF_BEGIN(SIE_REG_0B30)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD4, 32)
REGDEF_END(SIE_REG_0B30)

/*************************************************************
 * [0B34] SIE_REG_0B34
 *************************************************************/
#define SIE_REG_0B34_OFS    0x0B34
REGDEF_BEGIN(SIE_REG_0B34)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD5, 32)
REGDEF_END(SIE_REG_0B34)

/*************************************************************
 * [0B38] SIE_REG_0B38
 *************************************************************/
#define SIE_REG_0B38_OFS    0x0B38
REGDEF_BEGIN(SIE_REG_0B38)
    REGDEF_BIT(TSEN_TX_FS_CODE_SD6, 32)
REGDEF_END(SIE_REG_0B38)

/*************************************************************
 * [0B40] SIE_REG_0B40
 *************************************************************/
#define SIE_REG_0B40_OFS    0x0B40
REGDEF_BEGIN(SIE_REG_0B40)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD0, 32)
REGDEF_END(SIE_REG_0B40)

/*************************************************************
 * [0B44] SIE_REG_0B44
 *************************************************************/
#define SIE_REG_0B44_OFS    0x0B44
REGDEF_BEGIN(SIE_REG_0B44)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD1, 32)
REGDEF_END(SIE_REG_0B44)

/*************************************************************
 * [0B48] SIE_REG_0B48
 *************************************************************/
#define SIE_REG_0B48_OFS    0x0B48
REGDEF_BEGIN(SIE_REG_0B48)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD2, 32)
REGDEF_END(SIE_REG_0B48)

/*************************************************************
 * [0B4C] SIE_REG_0B4C
 *************************************************************/
#define SIE_REG_0B4C_OFS    0x0B4C
REGDEF_BEGIN(SIE_REG_0B4C)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD3, 32)
REGDEF_END(SIE_REG_0B4C)

/*************************************************************
 * [0B50] SIE_REG_0B50
 *************************************************************/
#define SIE_REG_0B50_OFS    0x0B50
REGDEF_BEGIN(SIE_REG_0B50)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD4, 32)
REGDEF_END(SIE_REG_0B50)

/*************************************************************
 * [0B54] SIE_REG_0B54
 *************************************************************/
#define SIE_REG_0B54_OFS    0x0B54
REGDEF_BEGIN(SIE_REG_0B54)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD5, 32)
REGDEF_END(SIE_REG_0B54)

/*************************************************************
 * [0B58] SIE_REG_0B58
 *************************************************************/
#define SIE_REG_0B58_OFS    0x0B58
REGDEF_BEGIN(SIE_REG_0B58)
    REGDEF_BIT(TSEN_TX_LS_CODE_SD6, 32)
REGDEF_END(SIE_REG_0B58)

/*************************************************************
 * [0B60] SIE_REG_0B60
 *************************************************************/
#define SIE_REG_0B60_OFS    0x0B60
REGDEF_BEGIN(SIE_REG_0B60)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA0, 32)
REGDEF_END(SIE_REG_0B60)

/*************************************************************
 * [0B64] SIE_REG_0B64
 *************************************************************/
#define SIE_REG_0B64_OFS    0x0B64
REGDEF_BEGIN(SIE_REG_0B64)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA1, 32)
REGDEF_END(SIE_REG_0B64)

/*************************************************************
 * [0B68] SIE_REG_0B68
 *************************************************************/
#define SIE_REG_0B68_OFS    0x0B68
REGDEF_BEGIN(SIE_REG_0B68)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA2, 32)
REGDEF_END(SIE_REG_0B68)

/*************************************************************
 * [0B6C] SIE_REG_0B6C
 *************************************************************/
#define SIE_REG_0B6C_OFS    0x0B6C
REGDEF_BEGIN(SIE_REG_0B6C)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA3, 32)
REGDEF_END(SIE_REG_0B6C)

/*************************************************************
 * [0B70] SIE_REG_0B70
 *************************************************************/
#define SIE_REG_0B70_OFS    0x0B70
REGDEF_BEGIN(SIE_REG_0B70)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA4, 32)
REGDEF_END(SIE_REG_0B70)

/*************************************************************
 * [0B74] SIE_REG_0B74
 *************************************************************/
#define SIE_REG_0B74_OFS    0x0B74
REGDEF_BEGIN(SIE_REG_0B74)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA5, 32)
REGDEF_END(SIE_REG_0B74)

/*************************************************************
 * [0B78] SIE_REG_0B78
 *************************************************************/
#define SIE_REG_0B78_OFS    0x0B78
REGDEF_BEGIN(SIE_REG_0B78)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA6, 32)
REGDEF_END(SIE_REG_0B78)

/*************************************************************
 * [0B7C] SIE_REG_0B7C
 *************************************************************/
#define SIE_REG_0B7C_OFS    0x0B7C
REGDEF_BEGIN(SIE_REG_0B7C)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA7, 32)
REGDEF_END(SIE_REG_0B7C)

/*************************************************************
 * [0B80] SIE_REG_0B80
 *************************************************************/
#define SIE_REG_0B80_OFS    0x0B80
REGDEF_BEGIN(SIE_REG_0B80)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA8, 32)
REGDEF_END(SIE_REG_0B80)

/*************************************************************
 * [0B84] SIE_REG0B84
 *************************************************************/
#define SIE_REG_0B84_OFS    0x0B84
REGDEF_BEGIN(SIE_REG_0B84)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA9, 32)
REGDEF_END(SIE_REG_0B84)

/*************************************************************
 * [0B88] SIE_REG0B88
 *************************************************************/
#define SIE_REG_0B88_OFS    0x0B88
REGDEF_BEGIN(SIE_REG_0B88)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA10, 32)
REGDEF_END(SIE_REG_0B88)

/*************************************************************
 * [0B8C] SIE_REG0B8C
 *************************************************************/
#define SIE_REG_0B8C_OFS    0x0B8C
REGDEF_BEGIN(SIE_REG_0B8C)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA11, 32)
REGDEF_END(SIE_REG_0B8C)

/*************************************************************
 * [0B90] SIE_REG0B90
 *************************************************************/
#define SIE_REG_0B90_OFS    0x0B90
REGDEF_BEGIN(SIE_REG_0B90)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA12, 32)
REGDEF_END(SIE_REG_0B90)

/*************************************************************
 * [0B94] SIE_REG0B94
 *************************************************************/
#define SIE_REG_0B94_OFS    0x0B94
REGDEF_BEGIN(SIE_REG_0B94)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA13, 32)
REGDEF_END(SIE_REG_0B94)

/*************************************************************
 * [0B98] SIE_REG0B98
 *************************************************************/
#define SIE_REG_0B98_OFS    0x0B98
REGDEF_BEGIN(SIE_REG_0B98)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA14, 32)
REGDEF_END(SIE_REG_0B98)

/*************************************************************
 * [0B9C] SIE_REG0B9C
 *************************************************************/
#define SIE_REG_0B9C_OFS    0x0B9C
REGDEF_BEGIN(SIE_REG_0B9C)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA15, 32)
REGDEF_END(SIE_REG_0B9C)

/*************************************************************
 * [0BA0] SIE_REG0BA0
 *************************************************************/
#define SIE_REG_0BA0_OFS    0x0BA0
REGDEF_BEGIN(SIE_REG_0BA0)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA16, 32)
REGDEF_END(SIE_REG_0BA0)

/*************************************************************
 * [0BA4] SIE_REG0BA4
 *************************************************************/
#define SIE_REG_0BA4_OFS    0x0BA4
REGDEF_BEGIN(SIE_REG_0BA4)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA17, 32)
REGDEF_END(SIE_REG_0BA4)

/*************************************************************
 * [0BA8] SIE_REG0BA8
 *************************************************************/
#define SIE_REG_0BA8_OFS    0x0BA8
REGDEF_BEGIN(SIE_REG_0BA8)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA18, 32)
REGDEF_END(SIE_REG_0BA8)

/*************************************************************
 * [0BAC] SIE_REG0BAC
 *************************************************************/
#define SIE_REG_0BAC_OFS    0x0BAC
REGDEF_BEGIN(SIE_REG_0BAC)
    REGDEF_BIT(TSEN_TX_CONFIG_DATA19, 32)
REGDEF_END(SIE_REG_0BAC)

/*************************************************************
 * [0BB0] SIE_REG0BB0
 *************************************************************/
#define SIE_REG_0BB0_OFS    0x0BB0
REGDEF_BEGIN(SIE_REG_0BB0)
    REGDEF_BIT(,              2)
    REGDEF_BIT(DRAM_IN3_SAI, 30)
REGDEF_END(SIE_REG_0BB0)

/*************************************************************
 * [0BB4] SIE_REG0BB4
 *************************************************************/
#define SIE_REG_0BB4_OFS    0x0BB4
REGDEF_BEGIN(SIE_REG_0BB4)
    REGDEF_BIT(DRAM_IN3_SAI_2, 4)
REGDEF_END(SIE_REG_0BB4)

/*************************************************************
 * [0BB8] SIE_REG0BB8
 *************************************************************/
#define SIE_REG_0BB8_OFS    0x0BB8
REGDEF_BEGIN(SIE_REG_0BB8)
    REGDEF_BIT(,                  2)
    REGDEF_BIT(DRAM_IN3_OFSO,    14)
    REGDEF_BIT(DRAM_IN3_PACK_BUS, 2)
REGDEF_END(SIE_REG_0BB8)

/*************************************************************
 * [0BC0] SIE_REG0BC0
 *************************************************************/
#define SIE_REG_0BC0_OFS    0x0BC0
REGDEF_BEGIN(SIE_REG_0BC0)
    REGDEF_BIT(TSEN_TX_VB0, 16)
    REGDEF_BIT(TSEN_TX_VB1, 16)
REGDEF_END(SIE_REG_0BC0)

/*************************************************************
 * [0BC4] SIE_REG0BC4
 *************************************************************/
#define SIE_REG_0BC4_OFS    0x0BC4
REGDEF_BEGIN(SIE_REG_0BC4)
    REGDEF_BIT(TSEN_TX_VB2, 16)
REGDEF_END(SIE_REG_0BC4)

/*************************************************************
 * [0BC8] SIE_REG0BC8
 *************************************************************/
#define SIE_REG_0BC8_OFS    0x0BC8
REGDEF_BEGIN(SIE_REG_0BC8)
    REGDEF_BIT(TSEN_TX_HB0, 16)
    REGDEF_BIT(TSEN_TX_HB1, 16)
REGDEF_END(SIE_REG_0BC8)

/*************************************************************
 * [0BCC] SIE_REG0BCC
 *************************************************************/
#define SIE_REG_0BCC_OFS    0x0BCC
REGDEF_BEGIN(SIE_REG_0BCC)
    REGDEF_BIT(TSEN_TX_HB2, 16)
REGDEF_END(SIE_REG_0BCC)

/*************************************************************
 * [0BD0] SIE_REG0BD0
 *************************************************************/
#define SIE_REG_0BD0_OFS    0x0BD0
REGDEF_BEGIN(SIE_REG_0BD0)
    REGDEF_BIT(TSEN_TX_CONFIG_CB0, 16)
    REGDEF_BIT(TSEN_TX_CONFIG_CB1, 16)
REGDEF_END(SIE_REG_0BD0)

/*************************************************************
 * [0BD4] SIE_REG0BD4
 *************************************************************/
#define SIE_REG_0BD4_OFS    0x0BD4
REGDEF_BEGIN(SIE_REG_0BD4)
    REGDEF_BIT(TSEN_TX_FS_H, 32)
REGDEF_END(SIE_REG_0BD4)

/*************************************************************
 * [0BD8] SIE_REG0BD8
 *************************************************************/
#define SIE_REG_0BD8_OFS    0x0BD8
REGDEF_BEGIN(SIE_REG_0BD8)
    REGDEF_BIT(TSEN_TX_FS_L, 32)
REGDEF_END(SIE_REG_0BD8)

/*************************************************************
 * [0BDC] SIE_REG0BDC
 *************************************************************/
#define SIE_REG_0BDC_OFS    0x0BDC
REGDEF_BEGIN(SIE_REG_0BDC)
    REGDEF_BIT(TSEN_TX_FS_DELAY,      16)
    REGDEF_BIT(,                      15)
    REGDEF_BIT(TSEN_TX_OOC_TRANS_INV,  1)
REGDEF_END(SIE_REG_0BDC)

/*************************************************************
 * [0BE0] SIE_REG0BE0
 *************************************************************/
#define SIE_REG_0BE0_OFS    0x0BE0
REGDEF_BEGIN(SIE_REG_0BE0)
    REGDEF_BIT(TSEN_RX_DECODE_MODE,     3)
    REGDEF_BIT(,                        1)
    REGDEF_BIT(TSEN_RX_HI_BYTE_INV,     1)
    REGDEF_BIT(TSEN_RX_LO_BYTE_INV,     1)
    REGDEF_BIT(TSEN_RX_HI_BYTE_RSFT,    3)
    REGDEF_BIT(TSEN_RX_LO_BYTE_RSFT,    3)
    REGDEF_BIT(TSEN_RX_HI_LO_BYTE_SWAP, 1)
    REGDEF_BIT(TSEN_RX_2BYTE_INV,       1)
    REGDEF_BIT(TSEN_RX_2BYTE_RSFT,      3)
    REGDEF_BIT(TSEN_RX_OUT_FMT,         1)
REGDEF_END(SIE_REG_0BE0)

/*************************************************************
 * [0BE4] SIE_REG0BE4
 *************************************************************/
#define SIE_REG_0BE4_OFS    0x0BE4
REGDEF_BEGIN(SIE_REG_0BE4)
    REGDEF_BIT(TSEN_RX_FS_CODE, 16)
REGDEF_END(SIE_REG_0BE4)

/*************************************************************
 * [0BE8] SIE_REG0BE8
 *************************************************************/
#define SIE_REG_0BE8_OFS    0x0BE8
REGDEF_BEGIN(SIE_REG_0BE8)
    REGDEF_BIT(TSEN_RX_LS_CODE, 16)
REGDEF_END(SIE_REG_0BE8)

/*************************************************************
 * [0BEC] SIE_REG0BEC
 *************************************************************/
#define SIE_REG_0BEC_OFS    0x0BEC
REGDEF_BEGIN(SIE_REG_0BEC)
    REGDEF_BIT(TSEN_RX_FS_DELAY, 16)
    REGDEF_BIT(TSEN_RX_LS_DELAY, 16)
REGDEF_END(SIE_REG_0BEC)

/********************************************************************
 * [RSVD] SIE_REG_RSVD
 ********************************************************************/
REGDEF_BEGIN(SIE_REG_RSVD)
    REGDEF_BIT(RESERVED, 32)
REGDEF_END(SIE_REG_RSVD)

#endif  /* _SIE_ENG_INT_REG_H_ */
