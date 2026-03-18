#ifndef _CAL_NT98690_H_
#define _CAL_NT98690_H_

#include "hd_type.h"

#define _CAL_REG_BASE_ADDR    0x


#define CAL_FLOW_CONTROL_REGISTER_0_OFS 0x000C
#define CAL_FLOW_CONTROL_REGISTER_1_OFS 0x0010
#define CAL_FLOW_CONTROL_REGISTER_2_OFS 0x0014
#define CAL_FLOW_CONTROL_REGISTER_3_OFS 0x0018
#define CAL_FLOW_CONTROL_REGISTER_4_OFS 0x001C

#define DMA_UB_TO_CAL_REGISTER_0_OFS 0x0050
#define DMA_UB_TO_CAL_REGISTER_1_OFS 0x0054
#define DMA_UB_TO_CAL_REGISTER_2_OFS 0x0058
#define DMA_UB_TO_CAL_REGISTER_3_OFS 0x005C
#define DMA_UB_TO_CAL_REGISTER_4_OFS 0x0060
#define DMA_UB_TO_CAL_REGISTER_5_OFS 0x0064
#define DMA_UB_TO_CAL_REGISTER_14_OFS 0x0088
#define DMA_UB_TO_CAL_REGISTER_15_OFS 0x008C
#define DMA_UB_TO_CAL_REGISTER_16_OFS 0x0090
#define DMA_UB_TO_CAL_REGISTER_17_OFS 0x0094

#define DMA_TO_CAL_OFFSET_ENABLE_REGISTER_0_OFS 0x00A4
#define DMA_TO_CAL_OFFSET_REGISTER_0_OFS 0x00A8
#define DMA_TO_CAL_OFFSET_REGISTER_1_OFS 0x00AC
#define DMA_TO_CAL_OFFSET_REGISTER_2_OFS 0x00B0
#define DMA_TO_CAL_OFFSET_REGISTER_3_OFS 0x00B4
#define DMA_TO_CAL_OFFSET_REGISTER_4_OFS 0x00B8
#define DMA_TO_CAL_OFFSET_REGISTER_5_OFS 0x00BC
#define DMA_TO_CAL_OFFSET_REGISTER_6_OFS 0x00C0
#define DMA_TO_CAL_OFFSET_REGISTER_7_OFS 0x00C4
#define DMA_TO_CAL_OFFSET_REGISTER_8_OFS 0x00C8
#define DMA_TO_CAL_OFFSET_REGISTER_9_OFS 0x00CC

#define CAL_SIZE_REGISTER_0_OFS 0x00E0
#define CAL_SIZE_REGISTER_1_OFS 0x00E4
#define CAL_SIZE_REGISTER_2_OFS 0x00E8
#define CAL_SIZE_REGISTER_3_OFS 0x00EC
#define CAL_SIZE_REGISTER_4_OFS 0x00F0
#define CAL_SIZE_REGISTER_5_OFS 0x00F4
#define CAL_SIZE_REGISTER_6_OFS 0x00F8
#define CAL_SIZE_REGISTER_7_OFS 0x00FC
#define CAL_SIZE_REGISTER_8_OFS 0x0100

#define CAL_GROUP0_SIZE0_REGISTER_0_OFS 0x0104
#define CAL_GROUP0_SIZE0_REGISTER_1_OFS 0x0108
#define CAL_GROUP0_SIZE0_REGISTER_2_OFS 0x010C
#define CAL_GROUP1_SIZE0_REGISTER_0_OFS 0x0110
#define CAL_GROUP1_SIZE0_REGISTER_1_OFS 0x0114
#define CAL_GROUP1_SIZE0_REGISTER_2_OFS 0x0118
#define CAL_GROUP2_SIZE0_REGISTER_0_OFS 0x011C
#define CAL_GROUP2_SIZE0_REGISTER_1_OFS 0x0120
#define CAL_GROUP2_SIZE0_REGISTER_2_OFS 0x0124
#define CAL_GROUP3_SIZE0_REGISTER_0_OFS 0x0128
#define CAL_GROUP3_SIZE0_REGISTER_1_OFS 0x012C
#define CAL_GROUP3_SIZE0_REGISTER_2_OFS 0x0130
#define CAL_GROUP0_SIZE1_REGISTER_0_OFS 0x0134
#define CAL_GROUP0_SIZE1_REGISTER_1_OFS 0x0138
#define CAL_GROUP0_SIZE1_REGISTER_2_OFS 0x013C

#define CAL_DATA_TYPE_REGISTER_0_OFS 0x0164
#define CAL_DATA_TYPE_REGISTER_1_OFS 0x0168

#define CAL_FRACTION_BIT_REGISTER_0_OFS 0x016C

#define CAL_SCALE_SHIFT_REGISTER_0_OFS 0x0170
#define CAL_SCALE_SHIFT_REGISTER_1_OFS 0x0174
#define CAL_SCALE_SHIFT_REGISTER_2_OFS 0x0178
#define CAL_SCALE_SHIFT_REGISTER_3_OFS 0x017C

#define CAL_OFFSET_REGISTER_1_OFS 0x0184
#define CAL_OFFSET_REGISTER_2_OFS 0x0188
#define CAL_OFFSET_REGISTER_3_OFS 0x018C
#define CAL_OFFSET_REGISTER_4_OFS 0x0190

#define CAL_SOURCE_CONTROL_REGISTER_0_OFS 0x0194
#define CAL_BROADCASTING_REGISTER_0_OFS 0x0198
#define CAL_REDUCE_AXIS_REGISTER_0_OFS 0x019C

#define CAL_OPERATION_ATTRIBUTE_REGISTER_0_OFS 0x01A0
#define CAL_OPERATION_ATTRIBUTE_REGISTER_1_OFS 0x01A4
#define CAL_SCALAR_REGISTER_0_OFS 0x01A8
#define CAL_SCALAR_REGISTER_1_OFS 0x01AC
#define CAL_SCALAR_REGISTER_2_OFS 0x01B0
#define CAL_SCALAR_REGISTER_3_OFS 0x01B4
#define CAL_OPERATION_ATTRIBUTE_REGISTER_2_OFS 0x01B8
#define CAL_OPERATION_ATTRIBUTE_REGISTER_3_OFS 0x01BC
#define CAL_OPERATION_ATTRIBUTE_REGISTER_4_OFS 0x01C0
#define CAL_OPERATION_ATTRIBUTE_REGISTER_5_OFS 0x01C4
#define CAL_OPERATION_ATTRIBUTE_REGISTER_6_OFS 0x01C8


typedef struct {

  union
  {
    struct
    {
      unsigned CAL_OP_FLOW_SEL        : 1;		// bits : 0
      unsigned                        : 7;
      unsigned CAL_IN1_EN             : 1;		// bits : 8
      unsigned CAL_IN2_EN             : 1;		// bits : 9
      unsigned                        : 6;
      unsigned CAL_STAGE0_EN          : 1;		// bits : 16
      unsigned CAL_STAGE1_EN          : 1;		// bits : 17
      unsigned CAL_STAGE2_EN          : 1;		// bits : 18
      unsigned CAL_STAGE3_EN          : 1;		// bits : 19
    } Bit;
    UINT32 Word;
  } CAL_Register_3; // 0x000c

  union
  {
    struct
    {
      unsigned CAL_STAGE0_GROUP_IDX        : 2;		// bits : 1_0
      unsigned                             : 2;
      unsigned CAL_STAGE1_GROUP_IDX        : 2;		// bits : 5_4
      unsigned                             : 2;
      unsigned CAL_STAGE2_GROUP_IDX        : 2;		// bits : 9_8
      unsigned                             : 2;
      unsigned CAL_STAGE3_GROUP_IDX        : 2;		// bits : 13_12
    } Bit;
    UINT32 Word;
  } CAL_Register_4; // 0x0010

  union
  {
    struct
    {
      unsigned CAL_GROUP0_OP_MODE        : 5;		// bits : 4_0
      unsigned                           : 3;
      unsigned CAL_GROUP1_OP_MODE        : 5;		// bits : 12_8
      unsigned                           : 3;
      unsigned CAL_GROUP2_OP_MODE        : 5;		// bits : 20_16
      unsigned                           : 3;
      unsigned CAL_GROUP3_OP_MODE        : 5;		// bits : 28_24
    } Bit;
    UINT32 Word;
  } CAL_Register_5; // 0x0014

  union
  {
    struct
    {
      unsigned CAL_STANDALONE_OP_MODE        : 3;		// bits : 2_0
    } Bit;
    UINT32 Word;
  } CAL_Register_6; // 0x0018

  union
  {
    struct
    {
      unsigned CAL_IN1_TO_STAGE_IDX           : 2;		// bits : 1_0
      unsigned                                : 2;
      unsigned CAL_IN2_TO_STAGE_IDX           : 2;		// bits : 5_4
      unsigned                                : 10;
      unsigned CAL_STAGE0_SWAP_ROLE_EN        : 1;		// bits : 16
      unsigned CAL_STAGE1_SWAP_ROLE_EN        : 1;		// bits : 17
      unsigned CAL_STAGE2_SWAP_ROLE_EN        : 1;		// bits : 18
      unsigned CAL_STAGE3_SWAP_ROLE_EN        : 1;		// bits : 19
    } Bit;
    UINT32 Word;
  } CAL_Register_7; // 0x001c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_20; // 0x0050

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI0         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI0         : 5;		// bits : 4_0
      unsigned                         : 23;
#endif
      unsigned DRAMUB_MODE_SAI0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CAL_Register_21; // 0x0054

  union
  {
    struct
    {
      unsigned DRAMUB_SAI1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_22; // 0x0058

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI1         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI1         : 5;		// bits : 4_0
      unsigned                         : 23;
#endif
      unsigned DRAMUB_MODE_SAI1        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CAL_Register_23; // 0x005c

  union
  {
    struct
    {
      unsigned DRAMUB_SAI2        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_24; // 0x0060

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAI2         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAI2         : 5;		// bits : 4_0
      unsigned                         : 23;
#endif
      unsigned DRAMUB_MODE_SAI2        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CAL_Register_25; // 0x0064

  union
  {
    struct
    {
      unsigned DRAMUB_SAO0        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_34; // 0x0088

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAO0         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAO0         : 5;		// bits : 4_0
      unsigned                         : 23;
#endif
      unsigned DRAMUB_MODE_SAO0        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CAL_Register_35; // 0x008c

  union
  {
    struct
    {
      unsigned DRAMUB_SAO1        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_36; // 0x0090

  union
  {
    struct
    {
#if defined(_BSP_NS02401_)
      unsigned DRAMUB_MSB_SAO1         : 8;		// bits : 7_0
      unsigned                         : 20;
#else
      unsigned DRAMUB_MSB_SAO1         : 5;		// bits : 4_0
      unsigned                         : 23;
#endif
      unsigned DRAMUB_MODE_SAO1        : 4;		// bits : 31_28
    } Bit;
    UINT32 Word;
  } CAL_Register_37; // 0x0094

  union
  {
    struct
    {
      unsigned CAL_IN0_LOFS_EN          : 1;		// bits : 0
      unsigned CAL_IN0_CHOFS_EN         : 1;		// bits : 1
      unsigned                          : 2;
      unsigned CAL_IN1_LOFS_EN          : 1;		// bits : 4
      unsigned CAL_IN1_CHOFS_EN         : 1;		// bits : 5
      unsigned                          : 2;
      unsigned CAL_IN2_LOFS_EN          : 1;		// bits : 8
      unsigned CAL_IN2_CHOFS_EN         : 1;		// bits : 9
      unsigned                          : 2;
      unsigned CAL_OUT0_LOFS_EN         : 1;		// bits : 12
      unsigned CAL_OUT0_CHOFS_EN        : 1;		// bits : 13
      unsigned                          : 2;
      unsigned CAL_OUT1_LOFS_EN         : 1;		// bits : 16
      unsigned CAL_OUT1_CHOFS_EN        : 1;		// bits : 17
    } Bit;
    UINT32 Word;
  } CAL_Register_41; // 0x00a4

  union
  {
    struct
    {
      unsigned CAL_IN0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_42; // 0x00a8

  union
  {
    struct
    {
      unsigned CAL_IN0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_43; // 0x00ac

  union
  {
    struct
    {
      unsigned CAL_IN1_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_44; // 0x00b0

  union
  {
    struct
    {
      unsigned CAL_IN1_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_45; // 0x00b4

  union
  {
    struct
    {
      unsigned CAL_IN2_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_46; // 0x00b8

  union
  {
    struct
    {
      unsigned CAL_IN2_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_47; // 0x00bc

  union
  {
    struct
    {
      unsigned CAL_OUT0_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_48; // 0x00c0

  union
  {
    struct
    {
      unsigned CAL_OUT0_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_49; // 0x00c4

  union
  {
    struct
    {
      unsigned CAL_OUT1_LOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_50; // 0x00c8

  union
  {
    struct
    {
      unsigned CAL_OUT1_CHOFS        : 29;		// bits : 28_0
    } Bit;
    UINT32 Word;
  } CAL_Register_51; // 0x00cc

  union
  {
    struct
    {
      unsigned CAL_IN0_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_56; // 0x00e0

  union
  {
    struct
    {
      unsigned CAL_IN0_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_57; // 0x00e4

  union
  {
    struct
    {
      unsigned CAL_IN0_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_58; // 0x00e8

  union
  {
    struct
    {
      unsigned CAL_IN1_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_59; // 0x00ec

  union
  {
    struct
    {
      unsigned CAL_IN1_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_60; // 0x00f0

  union
  {
    struct
    {
      unsigned CAL_IN1_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_61; // 0x00f4

  union
  {
    struct
    {
      unsigned CAL_IN2_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_62; // 0x00f8

  union
  {
    struct
    {
      unsigned CAL_IN2_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_63; // 0x00fc

  union
  {
    struct
    {
      unsigned CAL_IN2_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_64; // 0x0100

  union
  {
    struct
    {
      unsigned CAL_GP0_IN0_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_65; // 0x0104

  union
  {
    struct
    {
      unsigned CAL_GP0_IN0_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_66; // 0x0108

  union
  {
    struct
    {
      unsigned CAL_GP0_IN0_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_67; // 0x010c

  union
  {
    struct
    {
      unsigned CAL_GP1_IN0_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_68; // 0x0110

  union
  {
    struct
    {
      unsigned CAL_GP1_IN0_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_69; // 0x0114

  union
  {
    struct
    {
      unsigned CAL_GP1_IN0_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_70; // 0x0118

  union
  {
    struct
    {
      unsigned CAL_GP2_IN0_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_71; // 0x011c

  union
  {
    struct
    {
      unsigned CAL_GP2_IN0_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_72; // 0x0120

  union
  {
    struct
    {
      unsigned CAL_GP2_IN0_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_73; // 0x0124

  union
  {
    struct
    {
      unsigned CAL_GP3_IN0_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_74; // 0x0128

  union
  {
    struct
    {
      unsigned CAL_GP3_IN0_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_75; // 0x012c

  union
  {
    struct
    {
      unsigned CAL_GP3_IN0_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_76; // 0x0130

  union
  {
    struct
    {
      unsigned CAL_GP0_IN1_WIDTH        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_77; // 0x0134

  union
  {
    struct
    {
      unsigned CAL_GP0_IN1_HEIGHT        : 17;		// bits : 16_0
    } Bit;
    UINT32 Word;
  } CAL_Register_78; // 0x0138

  union
  {
    struct
    {
      unsigned CAL_GP0_IN1_CHANNEL        : 12;		// bits : 11_0
    } Bit;
    UINT32 Word;
  } CAL_Register_79; // 0x013c

  union
  {
    struct
    {
      unsigned CAL_IN0_BIT_DEPTH        : 3;		// bits : 2_0
      unsigned                          : 1;
      unsigned CAL_IN0_DATA_FMT         : 2;		// bits : 5_4
      unsigned                          : 2;
      unsigned CAL_IN1_BIT_DEPTH        : 3;		// bits : 10_8
      unsigned                          : 1;
      unsigned CAL_IN1_DATA_FMT         : 2;		// bits : 13_12
      unsigned                          : 2;
      unsigned CAL_IN2_BIT_DEPTH        : 3;		// bits : 18_16
      unsigned                          : 1;
      unsigned CAL_IN2_DATA_FMT         : 2;		// bits : 21_20
    } Bit;
    UINT32 Word;
  } CAL_Register_89; // 0x0164

  union
  {
    struct
    {
      unsigned CAL_OUT0_BIT_DEPTH        : 3;		// bits : 2_0
      unsigned                           : 1;
      unsigned CAL_OUT0_DATA_FMT         : 2;		// bits : 5_4
    } Bit;
    UINT32 Word;
  } CAL_Register_90; // 0x0168

  union
  {
    struct
    {
      unsigned CAL_FRAC_BIT_IN0         : 6;		// bits : 5_0
      unsigned                          : 2;
      unsigned CAL_FRAC_BIT_IN1         : 6;		// bits : 13_8
      unsigned                          : 2;
      unsigned CAL_FRAC_BIT_IN2         : 6;		// bits : 21_16
      unsigned                          : 2;
      unsigned CAL_FRAC_BIT_OUT0        : 6;		// bits : 29_24
    } Bit;
    UINT32 Word;
  } CAL_Register_91; // 0x016c

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN0_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned CAL_QUAN_IN0_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned CAL_QUAN_IN0_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CAL_Register_92; // 0x0170

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN1_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned CAL_QUAN_IN1_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned CAL_QUAN_IN1_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CAL_Register_93; // 0x0174

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN2_SHIFT_DIR        : 1;		// bits : 0
      unsigned                               : 3;
      unsigned CAL_QUAN_IN2_SHIFT            : 5;		// bits : 8_4
      unsigned                               : 3;
      unsigned CAL_QUAN_IN2_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CAL_Register_94; // 0x0178

  union
  {
    struct
    {
      unsigned CAL_QUAN_OUT0_SHIFT_DIR        : 1;		// bits : 0
      unsigned                                : 3;
      unsigned CAL_QUAN_OUT0_SHIFT            : 5;		// bits : 8_4
      unsigned                                : 3;
      unsigned CAL_QUAN_OUT0_SCALE            : 16;		// bits : 27_12
    } Bit;
    UINT32 Word;
  } CAL_Register_95; // 0x017c

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN0_OFS        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } CAL_Register_97; // 0x0184

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN1_OFS        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } CAL_Register_98; // 0x0188

  union
  {
    struct
    {
      unsigned CAL_QUAN_IN2_OFS        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } CAL_Register_99; // 0x018c

  union
  {
    struct
    {
      unsigned CAL_QUAN_OUT0_OFS        : 16;		// bits : 15_0
    } Bit;
    UINT32 Word;
  } CAL_Register_100; // 0x0190

  union
  {
    struct
    {
      unsigned CAL_IN0_REMAINSRC_EN        : 1;		// bits : 0
    } Bit;
    UINT32 Word;
  } CAL_Register_101; // 0x0194

  union
  {
    struct
    {
      unsigned CAL_IN0_BROADCAST_MODE        : 2;		// bits : 1_0
      unsigned                               : 2;
      unsigned CAL_IN1_BROADCAST_MODE        : 2;		// bits : 5_4
      unsigned                               : 2;
      unsigned CAL_IN2_BROADCAST_MODE        : 2;		// bits : 9_8
    } Bit;
    UINT32 Word;
  } CAL_Register_102; // 0x0198

  union
  {
    struct
    {
      unsigned                               : 4;
      unsigned CAL_GROUP1_REDUCE_AXIS        : 2;		// bits : 5_4
      unsigned                               : 2;
      unsigned CAL_GROUP2_REDUCE_AXIS        : 2;		// bits : 9_8
      unsigned                               : 2;
      unsigned CAL_GROUP3_REDUCE_AXIS        : 2;		// bits : 13_12
    } Bit;
    UINT32 Word;
  } CAL_Register_103; // 0x019c

  union
  {
    struct
    {
      unsigned CAL_ROOT_REFINE_ITER        : 3;		// bits : 2_0
      unsigned                             : 1;
      unsigned CAL_SORT_INDICES_EN         : 1;		// bits : 4
      unsigned CAL_SORT_MODE_SEL           : 2;		// bits : 6_5
      unsigned                             : 1;
      unsigned CAL_SORT_TOP_N              : 8;		// bits : 15_8
      unsigned                             : 8;
      unsigned CAL_MAX_INDICES_EN          : 1;		// bits : 24
      unsigned CAL_MIN_INDICES_EN          : 1;		// bits : 25
      unsigned                             : 2;
      unsigned CAL_BITSHIFT_DIR            : 1;		// bits : 28
    } Bit;
    UINT32 Word;
  } CAL_Register_104; // 0x01a0

  union
  {
    struct
    {
      unsigned CAL_GRIDSAMPLE_ALIGN_CORNER_EN        : 1;		// bits : 0
      unsigned CAL_GRIDSAMPLE_PADDING_MODE           : 1;		// bits : 1
    } Bit;
    UINT32 Word;
  } CAL_Register_105; // 0x01a4

  union
  {
    struct
    {
      unsigned CAL_GP0_SCALAR_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_106; // 0x01a8

  union
  {
    struct
    {
      unsigned CAL_GP1_SCALAR_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_107; // 0x01ac

  union
  {
    struct
    {
      unsigned CAL_GP2_SCALAR_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_108; // 0x01b0

  union
  {
    struct
    {
      unsigned CAL_GP3_SCALAR_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_109; // 0x01b4

  union
  {
    struct
    {
      unsigned CAL_LOG_BASE_ALPHA        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_110; // 0x01b8

  union
  {
    struct
    {
      unsigned CAL_THRES_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_111; // 0x01bc

  union
  {
    struct
    {
      unsigned CAL_THRES_REPLACE_VAL        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_112; // 0x01c0

  union
  {
    struct
    {
      unsigned CAL_CLAMP_LOW_BOUND        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_113; // 0x01c4

  union
  {
    struct
    {
      unsigned CAL_CLAMP_HIGH_BOUND        : 32;		// bits : 31_0
    } Bit;
    UINT32 Word;
  } CAL_Register_114; // 0x01c8

} NT98690_CAL_REG_STRUCT;


#endif