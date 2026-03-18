#ifndef _NUE2_NT98690_H_
#define _NUE2_NT98690_H_

#include "hd_type.h"

#define _NUE2_REG_BASE_ADDR    0x


#define NUE2_FUNCTION_ENABLE_REGISTER0_OFS 0x0004
#define DMA_TO_NUE2_REGISTER0_OFS 0x0008
#define DMA_TO_NUE2_REGISTER1_OFS 0x000c
#define DMA_TO_NUE2_REGISTER2_OFS 0x0010
#define NUE2_TO_DMA_RESULT_REGISTER0_OFS 0x0018
#define NUE2_TO_DMA_RESULT_REGISTER1_OFS 0x001c
#define NUE2_TO_DMA_RESULT_REGISTER2_OFS 0x0020
#define NUE2_INPUT_LINE_OFFSET_REGISTER0_OFS 0x0024
#define NUE2_INPUT_LINE_OFFSET_REGISTER1_OFS 0x0028
#define NUE2_INPUT_LINE_OFFSET_REGISTER2_OFS 0x002c
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER0_OFS 0x0030
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER1_OFS 0x0034
#define NUE2_OUTPUT_LINE_OFFSET_REGISTER2_OFS 0x0038
#define NUE2_INPUT_SIZE_REGISTER0_OFS 0x0044
#define NUE2_SCALING_RATE_REGISTER0_OFS 0x0048
#define NUE2_SCALING_RATE_REGISTER1_OFS 0x004c
#define NUE2_SCALING_RATE_REGISTER2_OFS 0x0050
#define NUE2_SCALING_RATE_REGISTER3_OFS 0x0054
#define NUE2_SCALING_OUTPUT_SIZE_REGISTER0_OFS 0x0058
#define NUE2_MEAN_SUBTRACTION_REGISTER0_OFS 0x005c
#define NUE2_MEAN_SUBTRACTION_REGISTER1_OFS 0x0060
#define NUE2_PADDING_REGISTER0_OFS 0x0064
#define NUE2_PADDING_REGISTER1_OFS 0x0068
#define NUE2_PADDING_REGISTER2_OFS 0x006c
#define NUE2_PADDING_REGISTER3_OFS 0x0070
#define NUE2_PADDING_REGISTER4_OFS 0x0074
#define NUE2_MEAN_SHIFT_REGISTER0_OFS 0x007c

#define NUE2_YUV2RGB_COEF_REGISTER0_OFS 0x0118
#define NUE2_YUV2RGB_COEF_REGISTER1_OFS 0x011c
#define NUE2_YUV2RGB_COEF_REGISTER2_OFS 0x0120
#define NUE2_YUV2RGB_COEF_REGISTER3_OFS 0x0124
#define NUE2_YUV2RGB_COEF_REGISTER4_OFS 0x0128
#define NUE2_YUV2RGB_COEF_REGISTER5_OFS 0x012c
#define NUE2_YUV2RGB_COEF_REGISTER6_OFS 0x0130
#define NUE2_YUV2RGB_COEF_REGISTER7_OFS 0x0134
#define NUE2_YUV2RGB_COEF_REGISTER8_OFS 0x0138
#define NUE2_YUV2RGB_BIAS_REGISTER0_OFS 0x013c
#define NUE2_YUV2RGB_BIAS_REGISTER1_OFS 0x0140
#define NUE2_YUV2RGB_BIAS_REGISTER2_OFS 0x0144

#define DMA_TO_NUE2_MSB_REGISTER0_OFS 0x0148
#define DMA_TO_NUE2_MSB_REGISTER1_OFS 0x014c
#define DMA_TO_NUE2_MSB_REGISTER2_OFS 0x0150
#define NUE2_TO_DMA_RESULT_MSB_REGISTER0_OFS 0x0158
#define NUE2_TO_DMA_RESULT_MSB_REGISTER1_OFS 0x015c
#define NUE2_TO_DMA_RESULT_MSB_REGISTER2_OFS 0x0160

#define NUE2_MEAN_SHIFT_REGISTER1_OFS 0x0168
#define NUE2_MEAN_SHIFT_REGISTER2_OFS 0x016c
#define NUE2_MEAN_SHIFT_REGISTER3_OFS 0x0170


typedef struct {

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_EN                   : 1;		// bits : 0
	  unsigned NUE2_SUB_EN                       : 1;		// bits : 1
	  unsigned NUE2_PAD_EN                       : 1;		// bits : 2
	  unsigned                                   : 1;
	  unsigned NUE2_ROTATE_EN                    : 1;		// bits : 4
	  unsigned                                   : 7;
	  unsigned NUE2_IN_FMT                       : 2;		// bits : 13_12
	  unsigned NUE2_OUT_SIGNEDNESS               : 1;		// bits : 14
	  unsigned NUE2_SUB_MODE                     : 1;		// bits : 15
	  unsigned                                   : 1;
	  unsigned NUE2_ROTATE_MODE                  : 2;		// bits : 18_17
	  unsigned NUE2_FLIP_MODE                    : 2;		// bits : 20_19
	  unsigned NUE2_YUV_MODE                     : 3;		// bits : 23_21
	  unsigned NUE2_SCALE_H_MODE                 : 1;		// bits : 24
	  unsigned NUE2_SCALE_V_MODE                 : 1;		// bits : 25
	  unsigned NUE2_MEAN_SCALE_SHIFT_MODE        : 1;		// bits : 26
	} Bit;
	UINT32 Word;
  } NUE2_Register_1; // 0x0004

  union
  {
	struct
	{
	  unsigned DRAM_SAI0        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_2; // 0x0008

  union
  {
	struct
	{
	  unsigned DRAM_SAI1        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_3; // 0x000c

  union
  {
	struct
	{
	  unsigned DRAM_SAI2        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_4; // 0x0010

  union
  {
	struct
	{
	  unsigned DRAM_SAO0        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_6; // 0x0018

  union
  {
	struct
	{
	  unsigned DRAM_SAO1        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_7; // 0x001c

  union
  {
	struct
	{
	  unsigned DRAM_SAO2        : 32;		// bits : 31_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_8; // 0x0020

  union
  {
	struct
	{
	  unsigned DRAM_OFSI0        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_9; // 0x0024

  union
  {
	struct
	{
	  unsigned DRAM_OFSI1        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_10; // 0x0028

  union
  {
	struct
	{
	  unsigned DRAM_OFSI2        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_11; // 0x002c

  union
  {
	struct
	{
	  unsigned DRAM_OFSO0        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_12; // 0x0030

  union
  {
	struct
	{
	  unsigned DRAM_OFSO1        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_13; // 0x0034

  union
  {
	struct
	{
	  unsigned DRAM_OFSO2        : 17;		// bits : 16_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_14; // 0x0038

  union
  {
	struct
	{
	  unsigned NUE2_IN_WIDTH         : 13;		// bits : 12_0
	  unsigned                       : 3;
	  unsigned NUE2_IN_HEIGHT        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_17; // 0x0044

  union
  {
	struct
	{
	  unsigned NUE2_H_DNRATE          : 7;		// bits : 6_0
	  unsigned                        : 1;
	  unsigned NUE2_V_DNRATE          : 7;		// bits : 14_8
	  unsigned                        : 1;
	  unsigned NUE2_H_FILTMODE        : 1;		// bits : 16
	  unsigned NUE2_H_FILTCOEF        : 6;		// bits : 22_17
	  unsigned                        : 1;
	  unsigned NUE2_V_FILTMODE        : 1;		// bits : 24
	  unsigned NUE2_V_FILTCOEF        : 6;		// bits : 30_25
	} Bit;
	UINT32 Word;
  } NUE2_Register_18; // 0x0048

  union
  {
	struct
	{
	  unsigned NUE2_H_SFACT        : 16;		// bits : 15_0
	  unsigned NUE2_V_SFACT        : 16;		// bits : 31_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_19; // 0x004c

  union
  {
	struct
	{
	  unsigned NUE2_INI_H_DNRATE        : 7;		// bits : 6_0
	  unsigned                          : 8;
	  unsigned NUE2_INI_H_SFACT         : 17;		// bits : 31_15
	} Bit;
	UINT32 Word;
  } NUE2_Register_20; // 0x0050

  union
  {
	struct
	{
	  unsigned NUE2_FINAL_H_DNRATE        : 7;		// bits : 6_0
	  unsigned                            : 8;
	  unsigned NUE2_FINAL_H_SFACT         : 17;		// bits : 31_15
	} Bit;
	UINT32 Word;
  } NUE2_Register_21; // 0x0054

  union
  {
	struct
	{
	  unsigned NUE2_H_SCL_SIZE        : 13;		// bits : 12_0
	  unsigned                        : 3;
	  unsigned NUE2_V_SCL_SIZE        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_22; // 0x0058

  union
  {
	struct
	{
	  unsigned NUE2_SUB_IN_WIDTH         : 13;		// bits : 12_0
	  unsigned                           : 3;
	  unsigned NUE2_SUB_IN_HEIGHT        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_23; // 0x005c

  union
  {
	struct
	{
	  unsigned NUE2_SUB_COEF_0        : 8;		// bits : 7_0
	  unsigned NUE2_SUB_COEF_1        : 8;		// bits : 15_8
	  unsigned NUE2_SUB_COEF_2        : 8;		// bits : 23_16
	  unsigned NUE2_SUB_DUP           : 2;		// bits : 25_24
	} Bit;
	UINT32 Word;
  } NUE2_Register_24; // 0x0060

  union
  {
	struct
	{
	  unsigned NUE2_PAD_CROP_X        : 13;		// bits : 12_0
	  unsigned                        : 3;
	  unsigned NUE2_PAD_CROP_Y        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_25; // 0x0064

  union
  {
	struct
	{
	  unsigned NUE2_PAD_CROP_WIDTH         : 13;		// bits : 12_0
	  unsigned                             : 3;
	  unsigned NUE2_PAD_CROP_HEIGHT        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_26; // 0x0068

  union
  {
	struct
	{
	  unsigned NUE2_PAD_OUT_X        : 13;		// bits : 12_0
	  unsigned                       : 3;
	  unsigned NUE2_PAD_OUT_Y        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_27; // 0x006c

  union
  {
	struct
	{
	  unsigned NUE2_PAD_OUT_WIDTH         : 13;		// bits : 12_0
	  unsigned                            : 3;
	  unsigned NUE2_PAD_OUT_HEIGHT        : 13;		// bits : 28_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_28; // 0x0070

  union
  {
	struct
	{
	  unsigned NUE2_PAD_VAL_0        : 8;		// bits : 7_0
	  unsigned NUE2_PAD_VAL_1        : 8;		// bits : 15_8
	  unsigned NUE2_PAD_VAL_2        : 8;		// bits : 23_16
	} Bit;
	UINT32 Word;
  } NUE2_Register_29; // 0x0074

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
  } NUE2_Register_31; // 0x007c

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF0        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_70; // 0x0118

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF1        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_71; // 0x011c

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF2        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_72; // 0x0120

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF3        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_73; // 0x0124

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF4        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_74; // 0x0128

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF5        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_75; // 0x012c

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF6        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_76; // 0x0130

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF7        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_77; // 0x0134

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_COEF8        : 15;		// bits : 14_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_78; // 0x0138

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_BIAS0        : 9;		// bits : 8_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_79; // 0x013c

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_BIAS1        : 9;		// bits : 8_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_80; // 0x0140

  union
  {
	struct
	{
	  unsigned NUE2_YUV2RGB_BIAS2        : 9;		// bits : 8_0
	} Bit;
	UINT32 Word;
  } NUE2_Register_81; // 0x0144

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAI0         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAI0         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAI0        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_82; // 0x0148

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAI1         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAI1         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAI1        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_83; // 0x014c

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAI2         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAI2         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAI2        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_84; // 0x0150

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAO0         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAO0         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAO0        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_86; // 0x0158

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAO1         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAO1         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAO1        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_87; // 0x015c

  union
  {
	struct
	{
#if defined(_BSP_NS02401_)
	  unsigned DRAM_MSB_SAO2         : 8;		// bits : 7_0
	  unsigned                       : 20;
#else
	  unsigned DRAM_MSB_SAO2         : 4;		// bits : 3_0
	  unsigned                       : 24;
#endif
	  unsigned DRAM_MODE_SAO2        : 4;		// bits : 31_28
	} Bit;
	UINT32 Word;
  } NUE2_Register_88; // 0x0160

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
  } NUE2_Register_90; // 0x0168

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
  } NUE2_Register_91; // 0x016c

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
  } NUE2_Register_92; // 0x0170

} NT98690_NUE2_REG_STRUCT;


#endif