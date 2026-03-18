#ifndef _PPU_NT98690_H_
#define _PPU_NT98690_H_

#include "hd_type.h"

#define _PPU_REG_BASE_ADDR    0x

#define PPU_FLOW_CONTROL_REGISTER_OFS 0x000C

#define PPU_BASE_ADDR_R0 0x0020
#define PPU_BASE_ADDR_R1 0x0024
#define PPU_BASE_ADDR_R2 0x0028
#define PPU_BASE_ADDR_R3 0x002C
#define PPU_BASE_ADDR_R4 0x0030
#define PPU_BASE_ADDR_R5 0x0034
#define PPU_BASE_ADDR_R6 0x0038
#define PPU_BASE_ADDR_R7 0x003C
#define PPU_BASE_ADDR_JR0 0x0040
#define PPU_BASE_ADDR_JR1 0x0044
#define PPU_BASE_ADDR_R8 0x0048
#define PPU_BASE_ADDR_R9 0x004C
#define PPU_BASE_ADDR_R10 0x01A0
#define PPU_BASE_ADDR_R11 0x01A4
#define PPU_BASE_ADDR_R12 0x01A8
#define PPU_BASE_ADDR_R13 0x01AC
#define PPU_BASE_ADDR_RB0 0x0200
#define PPU_BASE_ADDR_RB1 0x0204
#define PPU_BASE_ADDR_RB2 0x0208
#define PPU_BASE_ADDR_RB3 0x020C
#define PPU_BASE_ADDR_RB4 0x0210
#define PPU_BASE_ADDR_RB5 0x0214
#define PPU_BASE_ADDR_RB6 0x0218
#define PPU_BASE_ADDR_RB7 0x021C

#define DMA_UB_TO_PPU_REGISTER_0_OFS 0x0050
#define DMA_UB_TO_PPU_REGISTER_1_OFS 0x0054
#define DMA_UB_TO_PPU_REGISTER_2_OFS 0x0058
#define DMA_UB_TO_PPU_REGISTER_3_OFS 0x005C
#define DMA_UB_TO_PPU_REGISTER_4_OFS 0x0060
#define DMA_UB_TO_PPU_REGISTER_5_OFS 0x0064
#define DMA_UB_TO_PPU_REGISTER_6_OFS 0x0068
#define DMA_UB_TO_PPU_REGISTER_7_OFS 0x006C
#define DMA_UB_TO_PPU_REGISTER_8_OFS 0x0070
#define DMA_UB_TO_PPU_REGISTER_9_OFS 0x0074
#define DMA_UB_TO_PPU_REGISTER_10_OFS 0x0078
#define DMA_UB_TO_PPU_REGISTER_11_OFS 0x007C
#define DMA_UB_TO_PPU_REGISTER_12_OFS 0x0080

#define DMA_TO_PPU_OFFSET_ENABLE_REGISTER_0_OFS 0x0088
#define DMA_TO_PPU_OFFSET_REGISTER_0_OFS 0x008C
#define DMA_TO_PPU_OFFSET_REGISTER_1_OFS 0x0090
#define DMA_TO_PPU_OFFSET_REGISTER_2_OFS 0x0094
#define DMA_TO_PPU_OFFSET_REGISTER_3_OFS 0x0098
#define DMA_TO_PPU_OFFSET_REGISTER_4_OFS 0x009C
#define DMA_TO_PPU_OFFSET_REGISTER_5_OFS 0x00A0

#define PPU_SIZE_REGISTER_0_OFS 0x00A4
#define PPU_SIZE_REGISTER_1_OFS 0x00A8

#define PPU_FMT_CONVERT_COEF_REGISTER_0_OFS 0x00C0
#define PPU_FMT_CONVERT_COEF_REGISTER_1_OFS 0x00C4
#define PPU_FMT_CONVERT_COEF_REGISTER_2_OFS 0x00C8
#define PPU_FMT_CONVERT_COEF_REGISTER_3_OFS 0x00CC
#define PPU_FMT_CONVERT_COEF_REGISTER_4_OFS 0x00D0
#define PPU_FMT_CONVERT_COEF_REGISTER_5_OFS 0x00D4
#define PPU_FMT_CONVERT_COEF_REGISTER_6_OFS 0x00D8
#define PPU_FMT_CONVERT_COEF_REGISTER_7_OFS 0x00DC
#define PPU_FMT_CONVERT_COEF_REGISTER_8_OFS 0x00E0

#define PPU_FMT_CONVERT_BIAS_REGISTER_0_OFS 0x00E4
#define PPU_FMT_CONVERT_BIAS_REGISTER_1_OFS 0x00E8
#define PPU_FMT_CONVERT_BIAS_REGISTER_2_OFS 0x00EC

#define PPU_MEAN_SUBTRACTION_REGISTER_0_OFS 0x00F8
#define PPU_MEAN_SUBTRACTION_REGISTER_1_OFS 0x00FC
#define PPU_MEAN_SUBTRACTION_REGISTER_2_OFS 0x0100

#define PPU_MEAN_SHIFT_REGISTER_0_OFS 0x0104
#define PPU_MEAN_SHIFT_REGISTER_1_OFS 0x0108
#define PPU_MEAN_SHIFT_REGISTER_2_OFS 0x010C
#define PPU_MEAN_SHIFT_REGISTER_3_OFS 0x0110

#define PPU_PADDING_REGISTER_0_OFS 0x0118
#define PPU_PADDING_REGISTER_1_OFS 0x011C
#define PPU_PADDING_REGISTER_2_OFS 0x0120
#define PPU_PADDING_REGISTER_3_OFS 0x0124
#define PPU_PADDING_REGISTER_4_OFS 0x0128

#define PPU_CROP_REGISTER_0_OFS 0x012C
#define PPU_CROP_REGISTER_1_OFS 0x0130

#if !defined(_BSP_NS02201_)
#define PPU_QUAN_SCALE_SHIFT_REGISTER_0_OFS 0x0220
#define PPU_QUAN_SCALE_SHIFT_REGISTER_1_OFS 0x0224
#define PPU_QUAN_SCALE_SHIFT_REGISTER_2_OFS 0x0228
#define PPU_QUAN_SCALE_SHIFT_REGISTER_3_OFS 0x022c
#define PPU_QUAN_SCALE_SHIFT_REGISTER_4_OFS 0x0230
#define PPU_QUAN_SCALE_SHIFT_REGISTER_5_OFS 0x0234

#define PPU_QUAN_OFFSET_REGISTER_0_OFS 0x0238
#define PPU_QUAN_OFFSET_REGISTER_1_OFS 0x023c
#define PPU_QUAN_OFFSET_REGISTER_2_OFS 0x0240
#define PPU_QUAN_OFFSET_REGISTER_3_OFS 0x0244
#define PPU_QUAN_OFFSET_REGISTER_4_OFS 0x0248
#define PPU_QUAN_OFFSET_REGISTER_5_OFS 0x024c
#endif

typedef struct {
  union
  {
    struct
    {
      unsigned PPU_RST              : 1;		// bits : 0
      unsigned PPU_START            : 1;		// bits : 1
      unsigned                      : 26;
      unsigned JOB_START            : 1;		// bits : 28
      unsigned JOB_TERMINATE        : 1;		// bits : 29
    } Bit;
    UINT32 Word;
  } PPU_Register_0; // 0x0000

  union
  {
    struct
    {
      unsigned INTE_FRM_END          : 1;		// bits : 0
      unsigned                       : 23;
      unsigned INTE_JOB_END          : 1;		// bits : 24
      unsigned INTE_JOB_ERR          : 1;		// bits : 25
      unsigned INTE_LL_WR_END        : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } PPU_Register_1; // 0x0004

  union
  {
    struct
    {
      unsigned INTS_FRM_END          : 1;		// bits : 0
      unsigned                       : 23;
      unsigned INTS_JOB_END          : 1;		// bits : 24
      unsigned INTS_JOB_ERR          : 1;		// bits : 25
      unsigned INTS_LL_WR_END        : 1;		// bits : 26
    } Bit;
    UINT32 Word;
  } PPU_Register_2; // 0x0008

  union
  {
    struct
    {
      unsigned PPU_IN_FMT                       : 3;		// bits : 2_0
      unsigned                                  : 1;
      unsigned PPU_OUT_FMT                      : 3;		// bits : 6_4
      unsigned                                  : 1;
      unsigned PPU_MEANSUB_EN                   : 1;		// bits : 8
      unsigned                                  : 3;
      unsigned PPU_MEAN_SCALE_SHIFT_MODE        : 1;		// bits : 12
      unsigned                                  : 3;
      unsigned PPU_PAD_CROP_MODE                : 1;		// bits : 16
      unsigned                                  : 3;
      unsigned PPU_PAD_MODE                        : 1;		// bits : 20
      unsigned                                     : 3;
      unsigned PPU_OUT_SIGNEDNESS                  : 1;		// bits : 24
#if defined(_BSP_NS02302_) || defined(_BSP_NS02401_)
      unsigned                                     : 3;
      unsigned PPU_PAD_EDGE_MODE_HORIZONTAL        : 1;		// bits : 28
      unsigned PPU_PAD_EDGE_MODE_VERTICAL          : 1;		// bits : 29
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned BASE_ADDR0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_8; // 0x0020

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR0        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR0        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_9; // 0x0024

  union
  {
    struct
    {
      unsigned BASE_ADDR1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_10; // 0x0028

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR1        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR1        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_11; // 0x002c

  union
  {
    struct
    {
      unsigned BASE_ADDR2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_12; // 0x0030

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR2        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR2        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_13; // 0x0034

  union
  {
    struct
    {
      unsigned BASE_ADDR3        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_14; // 0x0038

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR3        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR3        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_15; // 0x003c

  union
  {
    struct
    {
      unsigned DRAMUB_SAIJOB        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_16; // 0x0040

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAIJOB        : 8;		// bits : 7_0
#else
      unsigned DRAMUB_MSB_SAIJOB        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_17; // 0x0044

  union
  {
    struct
    {
      unsigned BASE_ADDR4        : 32;		// bits : 3_0
    } Bit;
    UINT32 Word;
  } PPU_Register_18; // 0x0048

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR4        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR4        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_19; // 0x004c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_20; // 0x0050

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI0         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI0         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAI0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_21; // 0x0054

  union
  {
    struct
    {
      unsigned DRAMUB_SAI1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_22; // 0x0058

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI1         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI1         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAI1        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_23; // 0x005c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_24; // 0x0060

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI2         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI2         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAI2        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_25; // 0x0064

  union
  {
    struct
    {
      unsigned DRAMUB_SAO0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_26; // 0x0068

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAO0         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAO0         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAO0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_27; // 0x006c

  union
  {
    struct
    {
      unsigned DRAMUB_SAO1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_28; // 0x0070

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAO1         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAO1         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAO1        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_29; // 0x0074

  union
  {
    struct
    {
      unsigned DRAMUB_SAO2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_30; // 0x0078

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAO2         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAO2         : 4;		// bits : 3_0
      unsigned                         : 24;
#endif
      unsigned DRAMUB_MODE_SAO2        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } PPU_Register_31; // 0x007c

  union
  {
    struct
    {
      unsigned DRAMUB_INPUT_RINGBUF_EN        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } PPU_Register_32; // 0x0080

  union
  {
    struct
    {
      unsigned PPU_IN0_LOFS_EN         : 1;		// bits : 0
      unsigned PPU_IN1_LOFS_EN         : 1;		// bits : 1
      unsigned PPU_IN2_LOFS_EN         : 1;		// bits : 2
      unsigned                         : 1;
      unsigned PPU_OUT0_LOFS_EN        : 1;		// bits : 4
      unsigned PPU_OUT1_LOFS_EN        : 1;		// bits : 5
      unsigned PPU_OUT2_LOFS_EN        : 1;		// bits : 6
    } Bit;
    UINT32 Word;
  } PPU_Register_34; // 0x0088

  union
  {
    struct
    {
      unsigned PPU_IN0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_35; // 0x008c

  union
  {
    struct
    {
      unsigned PPU_IN1_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_36; // 0x0090

  union
  {
    struct
    {
      unsigned PPU_IN2_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_37; // 0x0094

  union
  {
    struct
    {
      unsigned PPU_OUT0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_38; // 0x0098

  union
  {
    struct
    {
      unsigned PPU_OUT1_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_39; // 0x009c

  union
  {
    struct
    {
      unsigned PPU_OUT2_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } PPU_Register_40; // 0x00a0

  union
  {
    struct
    {
      unsigned PPU_IN0_WIDTH        : 11;		// bits : 10_0
    } Bit;
    UINT32 Word;
  } PPU_Register_41; // 0x00a4

  union
  {
    struct
    {
      unsigned PPU_IN0_HEIGHT        : 11;		// bits : 10_0
    } Bit;
    UINT32 Word;
  } PPU_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF0        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_48; // 0x00c0

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF1        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_49; // 0x00c4

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF2        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_50; // 0x00c8

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF3        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_51; // 0x00cc

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF4        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_52; // 0x00d0

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF5        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_53; // 0x00d4

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF6        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_54; // 0x00d8

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF7        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_55; // 0x00dc

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_COEF8        : 15;		// bits : 14_0
    } Bit;
    UINT32 Word;
  } PPU_Register_56; // 0x00e0

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_BIAS0        : 9;		// bits : 8_0
    } Bit;
    UINT32 Word;
  } PPU_Register_57; // 0x00e4

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_BIAS1        : 9;		// bits : 8_0
    } Bit;
    UINT32 Word;
  } PPU_Register_58; // 0x00e8

  union
  {
    struct
    {
      unsigned PPU_FMT_CONVERT_BIAS2        : 9;		// bits : 8_0
    } Bit;
    UINT32 Word;
  } PPU_Register_59; // 0x00ec

  union
  {
    struct
    {
      unsigned PPU_MEANSUB_COEF_0        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_62; // 0x00f8

  union
  {
    struct
    {
      unsigned PPU_MEANSUB_COEF_1        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_63; // 0x00fc

  union
  {
    struct
    {
      unsigned PPU_MEANSUB_COEF_2        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_64; // 0x0100

  union
  {
    struct
    {
      unsigned MEAN_SHIFT_DIR        : 1;		// bits : 0
      unsigned                       : 3;
      unsigned MEAN_SHIFT            : 5;		// bits : 8_4
      unsigned                       : 3;
      unsigned MEAN_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_65; // 0x0104

  union
  {
    struct
    {
      unsigned MEAN_CH_SHIFT_DIR0        : 1;		// bits : 0
      unsigned                           : 3;
      unsigned MEAN_CH_SHIFT0            : 5;		// bits : 8_4
      unsigned                           : 3;
      unsigned MEAN_CH_SCALE0            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_66; // 0x0108

  union
  {
    struct
    {
      unsigned MEAN_CH_SHIFT_DIR1        : 1;		// bits : 0
      unsigned                           : 3;
      unsigned MEAN_CH_SHIFT1            : 5;		// bits : 8_4
      unsigned                           : 3;
      unsigned MEAN_CH_SCALE1            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_67; // 0x010c

  union
  {
    struct
    {
      unsigned MEAN_CH_SHIFT_DIR2        : 1;		// bits : 0
      unsigned                           : 3;
      unsigned MEAN_CH_SHIFT2            : 5;		// bits : 8_4
      unsigned                           : 3;
      unsigned MEAN_CH_SCALE2            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_68; // 0x0110

  union
  {
    struct
    {
      unsigned PPU_PAD_LEFT_NUM         : 10;		// bits : 9_0
      unsigned                          : 6;
      unsigned PPU_PAD_RIGHT_NUM        : 10;		// bits : 25_16
    } Bit;
    UINT32 Word;
  } PPU_Register_70; // 0x0118

  union
  {
    struct
    {
      unsigned PPU_PAD_TOP_NUM           : 10;		// bits : 9_0
      unsigned                           : 6;
      unsigned PPU_PAD_BOTTOM_NUM        : 10;		// bits : 25_16
    } Bit;
    UINT32 Word;
  } PPU_Register_71; // 0x011c

  union
  {
    struct
    {
      unsigned PPU_PAD_VAL_0        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_72; // 0x0120

  union
  {
    struct
    {
      unsigned PPU_PAD_VAL_1        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_73; // 0x0124

  union
  {
    struct
    {
      unsigned PPU_PAD_VAL_2        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } PPU_Register_74; // 0x0128

  union
  {
    struct
    {
      unsigned PPU_CROP_START_X        : 10;		// bits : 9_0
      unsigned                         : 6;
      unsigned PPU_CROP_START_Y        : 10;		// bits : 25_16
    } Bit;
    UINT32 Word;
  } PPU_Register_75; // 0x012c

  union
  {
    struct
    {
      unsigned PPU_CROP_OUT_WIDTH         : 11;		// bits : 10_0
      unsigned                            : 5;
      unsigned PPU_CROP_OUT_HEIGHT        : 11;		// bits : 26_16
    } Bit;
    UINT32 Word;
  } PPU_Register_76; // 0x0130

  union
  {
    struct
    {
      unsigned BASE_ADDR5        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_104; // 0x01A0

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR5        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR5        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_105; // 0x01A4

  union
  {
    struct
    {
      unsigned BASE_ADDR6        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_106; // 0x01A8

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned BASE_MSB_ADDR6        : 8;		// bits : 7_0
#else
      unsigned BASE_MSB_ADDR6        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_107; // 0x01AC

  union
  {
    struct
    {
      unsigned RINGBUF_START_ADDR0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_128; // 0x0200

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned RINGBUF_MSB_START_ADDR0        : 8;		// bits : 7_0
#else
      unsigned RINGBUF_MSB_START_ADDR0        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_129; // 0x0204

  union
  {
    struct
    {
      unsigned RINGBUF_END_ADDR0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_130; // 0x0208

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned RINGBUF_MSB_END_ADDR0        : 8;		// bits : 7_0
#else
      unsigned RINGBUF_MSB_END_ADDR0        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_131; // 0x020c

  union
  {
    struct
    {
      unsigned RINGBUF_START_ADDR1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_132; // 0x0210

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned RINGBUF_MSB_START_ADDR1        : 8;		// bits : 7_0
#else
      unsigned RINGBUF_MSB_START_ADDR1        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_133; // 0x0214

  union
  {
    struct
    {
      unsigned RINGBUF_END_ADDR1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } PPU_Register_134; // 0x0218

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned RINGBUF_MSB_END_ADDR1        : 8;		// bits : 7_0
#else
      unsigned RINGBUF_MSB_END_ADDR1        : 4;		// bits : 3_0
#endif
    } Bit;
    UINT32 Word;
  } PPU_Register_135; // 0x021c
#if !defined(_BSP_NS02201_)
  union
  {
    struct
    {
      unsigned PPU_QUAN_IN0_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned PPU_QUAN_IN0_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned PPU_QUAN_IN0_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_136; // 0x0220

  union
  {
    struct
    {
      unsigned PPU_QUAN_IN1_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned PPU_QUAN_IN1_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned PPU_QUAN_IN1_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_137; // 0x0224

  union
  {
    struct
    {
      unsigned PPU_QUAN_IN2_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned PPU_QUAN_IN2_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned PPU_QUAN_IN2_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_138; // 0x0228

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT0_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT0_SHIFT            : 5;		// bits : 8_4
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT0_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_139; // 0x022c

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT1_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT1_SHIFT            : 5;		// bits : 8_4
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT1_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_140; // 0x0230

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT2_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT2_SHIFT            : 5;		// bits : 8_4
      unsigned                                : 3;
      unsigned PPU_QUAN_OUT2_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } PPU_Register_141; // 0x0234

  union
  {
    struct
    {
      unsigned PPU_QUAN_IN0_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_142; // 0x0238

  union
  {
    struct
    {
      unsigned PPU_QUAN_IN1_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_143; // 0x023c

  union
  {
    struct
    {
      unsigned PPU_QUAN_IN2_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_144; // 0x0240

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT0_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_145; // 0x0244

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT1_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_146; // 0x0248

  union
  {
    struct
    {
      unsigned PPU_QUAN_OUT2_OFFSET_VAL        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } PPU_Register_147; // 0x024c
#endif
} NT98690_PPU_REG_STRUCT;

#endif