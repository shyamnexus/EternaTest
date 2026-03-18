/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_ENG_LIMIT_H_
#define _SIE_ENG_LIMIT_H_

/***********************************************************************************************
 * SIE Master Clock Max Frequency Definition
 ***********************************************************************************************/
#define SIE1_MAX_CLK_FREQ                       400000000   ///< 400MHz, 2-pixel base, BCC_2P
#define SIE2_MAX_CLK_FREQ                       400000000   ///< 400MHz, 2-pixel base,         BCC_SIE1(Shared)
#define SIE3_MAX_CLK_FREQ                       400000000   ///< 400MHz, 2-pixel base, BCC_2P
#define SIE4_MAX_CLK_FREQ                       400000000   ///< 400MHz, 2-pixel base,         BCC_SIE3(Shared)
#define SIE5_MAX_CLK_FREQ                       400000000   ///< 400MHz, 2-pixel base,

#define SIE1_MAX_CLK_FREQ_539A                  480000000   ///< 480MHz, 2-pixel base, BCC_2P
#define SIE2_MAX_CLK_FREQ_539A                  480000000   ///< 480MHz, 2-pixel base,         BCC_SIE1(Shared)
#define SIE1_MAX_CLK_FREQ_539A_OC               600000000   ///< 600MHz, 2-pixel base, BCC_2P
#define SIE2_MAX_CLK_FREQ_539A_OC               600000000   ///< 600MHz, 2-pixel base,         BCC_SIE1(Shared)

/***********************************************************************************************
 * SIE BCC Clock Max Frequency Definition
 ***********************************************************************************************/
#define SIE_BCC_MAX_CLK_FREQ                    400000000   ///< BCC clock the same as SIE clock

/***********************************************************************************************
 * SIE Sensor MClock Max Frequency Definition
 ***********************************************************************************************/
#define SIE_MCLK_MAX_NUM                        4           ///< SENSOR_MCLK1~4
#define SIE_MCLK_MAX_CLK_FREQ                   150000000   ///< 150MHz

/***********************************************************************************************
 * SIE Global Hardware Limitation Definition
 ***********************************************************************************************/
#define SIE_MAX_ENG_NUM                         5           ///< SIE1~5

#define SIE_MAX_OUTPUT_NUM                      3           ///< OUT0:IMG/DATAEXT, OUT1:CA/UV, OUT2:LA/DBG
#define SIE_MAX_INPUT_NUM                       2           ///< IN1 :DPC/IMG,     IN2 :ECS    IN3 :OOC(T-Sensor), only SIE5

#define SIE_CA_MAX_WIN_NUM                      32
#define SIE_LA_MAX_WIN_NUM                      32
#define SIE_LA_HISTO_BIN_NUM                    128

#define SIE_DPC_COLDEF_MAX_COL                  32          ///< only SIE5
#define SIE_DPC_COLDEF_MAX_X                    0x3fff      ///< only SIE5

#define SIE_PATGEN_SRC_WIN_W_ALIGN              2
#define SIE_PATGEN_SRC_WIN_H_ALIGN              1
#define SIE_PATGEN_SRC_WIN_MIN_W                16
#define SIE_PATGEN_SRC_WIN_MIN_H                16
#define SIE_PATGEN_SRC_WIN_MAX_W                65534
#define SIE_PATGEN_SRC_WIN_MAX_H                65535

#define SIE_OB_WIN_W_ALIGN                      2           ///< OB not supported in this platform
#define SIE_OB_WIN_H_ALIGN                      2
#define SIE_OB_WIN_X_ALIGN                      1
#define SIE_OB_WIN_Y_ALIGN                      1
#define SIE_OB_WIN_MAX_X                        65535
#define SIE_OB_WIN_MAX_Y                        65535
#define SIE_OB_WIN_MAX_W                        65534
#define SIE_OB_WIN_MAX_H                        14

#define SIE_ACT_ECS_WIN_MIN_W                   256         ///< for ECS table size 65x65
#define SIE_ACT_ECS_WIN_MIN_H                   256         ///< for ECS table size 65x65

#define SIE_ACT_WIN_W_ALIGN                     2           ///< RGB=> align to 2, RGBIR=> align to 4
#define SIE_ACT_WIN_H_ALIGN                     2           ///< RGB=> align to 2, RGBIR=> align to 4
#define SIE_ACT_WIN_X_ALIGN                     1
#define SIE_ACT_WIN_Y_ALIGN                     1
#define SIE_ACT_WIN_MAX_X                       65535
#define SIE_ACT_WIN_MAX_Y                       65535
#define SIE_ACT_WIN_MIN_W                       16
#define SIE_ACT_WIN_MIN_H                       2
#define SIE_ACT_WIN_MAX_W                       65534
#define SIE_ACT_WIN_MAX_H                       65534

#define SIE_ACT2_WIN_W_ALIGN                    2           ///< ACT2 not supported in this platform
#define SIE_ACT2_WIN_H_ALIGN                    2
#define SIE_ACT2_WIN_X_ALIGN                    1
#define SIE_ACT2_WIN_Y_ALIGN                    1
#define SIE_ACT2_WIN_MAX_X                      16383
#define SIE_ACT2_WIN_MAX_Y                      16383
#define SIE_ACT2_WIN_MIN_W                      16
#define SIE_ACT2_WIN_MIN_H                      2
#define SIE_ACT2_WIN_MAX_W_8BIT                 8192
#define SIE_ACT2_WIN_MAX_W_10BIT                6552
#define SIE_ACT2_WIN_MAX_W_12BIT                5460
#define SIE_ACT2_WIN_MAX_W_16BIT                4096
#define SIE_ACT2_WIN_MAX_H                      16383

#define SIE_CROP_WIN_W_ALIGN                    2           ///< RGB=> align to 2, RGBIR=> align to 4
#define SIE_CROP_WIN_H_ALIGN                    2           ///< RGB=> align to 2, RGBIR=> align to 4
#define SIE_CROP_WIN_X_ALIGN                    1
#define SIE_CROP_WIN_Y_ALIGN                    1
#define SIE_CROP_WIN_MAX_X                      65535
#define SIE_CROP_WIN_MAX_Y                      65535
#define SIE_CROP_WIN_MIN_W                      16
#define SIE_CROP_WIN_MIN_H                      2
#define SIE_CROP_WIN_MAX_W                      65534
#define SIE_CROP_WIN_MAX_H                      65534

#define SIE_CROP_BS_WIN_H_ALIGN                 2           ///< reserve for backward compatiable, will be remove
#define SIE_CROP_BS_IN_WIN_W_ALIGN              2           ///< when bayer-scaling horizontial on, crop width  should be 2x
#define SIE_CROP_BS_IN_WIN_H_ALIGN              2           ///< when bayer-scaling vertical    on, crop height should be 2x

#define SIE_CROP_WIN_MAX_W_8BIT                 8192        ///< when bcc disable, output max width for  8bit output is 65536bit / 8bit
#define SIE_CROP_WIN_MAX_W_10BIT                6552        ///< when bcc disable, output max width for 10bit output is 65536bit / 10bit
#define SIE_CROP_WIN_MAX_W_12BIT                5460        ///< when bcc disable, output max width for 12bit output is 65536bit / 12bit
#define SIE_CROP_WIN_MAX_W_16BIT                4096        ///< when bcc disable, output max width for 16bit output is 65536bit / 16bit

#define SIE_CROP_WIN_MAX_W_8BIT_EX              SIE_CROP_WIN_MAX_W_8BIT
#define SIE_CROP_WIN_MAX_W_10BIT_EX             SIE_CROP_WIN_MAX_W_10BIT
#define SIE_CROP_WIN_MAX_W_12BIT_EX             SIE_CROP_WIN_MAX_W_12BIT
#define SIE_CROP_WIN_MAX_W_16BIT_EX             SIE_CROP_WIN_MAX_W_16BIT

#define SIE_CROP_WIN_W_ALIGN_8BIT_LEGACY420     48          ///< 8bit legacy420, odd line => UYY, even line => VYY, not supported in this platform
#define SIE_CROP_WIN_MIN_W_8BIT_LEGACY420       48
#define SIE_CROP_WIN_MAX_W_8BIT_LEGACY420       8160
#define SIE_CROP_WIN_MAX_W_8BIT_EX_LEGACY420    12288

#define SIE_ACT_DATAEXT_SKIPLEN_MIN             1
#define SIE_ACT_DATAEXT_LEN_MIN                 1
#define SIE_ACT_DATAEXT_INILEN_MIN              0
#define SIE_ACT_DATAEXT_SKIPLEN_MAX             255
#define SIE_ACT_DATAEXT_LEN_MAX                 255
#define SIE_ACT_DATAEXT_INILEN_MAX              4095

#define SIE_ACT2_DATAEXT_SKIPLEN_MIN            1           ///< ACT2 DATAEXT not supported in this platform
#define SIE_ACT2_DATAEXT_LEN_MIN                1
#define SIE_ACT2_DATAEXT_INILEN_MIN             0
#define SIE_ACT2_DATAEXT_SKIPLEN_MAX            255
#define SIE_ACT2_DATAEXT_LEN_MAX                255
#define SIE_ACT2_DATAEXT_INILEN_MAX             255

#define SIE_ROI_ACC_WIN_NUM                     25          ///< 5x5 window accumulation result, not supported in this platform
#define SIE_ROI_ACC_WIN_W_ALIGN                 2
#define SIE_ROI_ACC_WIN_H_ALIGN                 2
#define SIE_ROI_ACC_WIN_X_ALIGN                 1
#define SIE_ROI_ACC_WIN_Y_ALIGN                 1
#define SIE_ROI_ACC_WIN_MAX_X                   16383
#define SIE_ROI_ACC_WIN_MAX_Y                   16383
#define SIE_ROI_ACC_WIN_MAX_W                   16382
#define SIE_ROI_ACC_WIN_MAX_H                   16382
#define SIE_ROI_ACC_WIN_MAX_SIZE                1048575     ///< G channel accumulate, (ROI_ACC_WIN_W/2)*ROI_ACC_WIN_H < 2^20 to prevent result counter(32bit) overflow of each window

#define SIE_SCALE_WIN_W_ALIGN                   2
#define SIE_SCALE_WIN_H_ALIGN                   2
#define SIE_SCALE_INPUT_WIN_MAX_W               5120        ///< if Bayer scaling source interpolation enable
#define SIE_SCALE_INPUT_WIN_MAX_H               65520
#define SIE_SCALE_OUTPUT_WIN_MAX_W              4096
#define SIE_SCALE_OUTPUT_WIN_MAX_H              8190
#define SIE_SCALE_W_RATIO                       8
#define SIE_SCALE_H_RATIO                       8

#define SIE_CA_CROP_WIN_W_ALIGN                 2
#define SIE_CA_CROP_WIN_H_ALIGN                 2
#define SIE_CA_CROP_WIN_X_ALIGN                 1
#define SIE_CA_CROP_WIN_Y_ALIGN                 1
#define SIE_CA_CROP_WIN_MAX_X                   65535
#define SIE_CA_CROP_WIN_MAX_Y                   65535
#define SIE_CA_CROP_WIN_MAX_W                   20416       ///< (32767*(640/2-1) / 1024 + 1 )*2 = 20416
#define SIE_CA_CROP_WIN_MAX_H                   24514       ///< (32767*(768/2-1) / 1024 + 1 )*2 = 24514
#define SIE_CA_CROP_WIN_MIN_W                   128
#define SIE_CA_CROP_WIN_MIN_H                   128

#define SIE_CA_SMPL_MAX_W                       640         ///< max horizontal subsample output, line buffer 640x12bit
#define SIE_CA_SMPL_MAX_H                       768         ///< max vertical   subsample output, limited to 768 to prevent CA/LA accumlator(19bit) overflow, CA hw limitation is 8191, LA limitation is 2047(LA_CROP_STY + LA_CROP_SZY <= 2047, counter only 11bit)

#define SIE_CA_WIN_W_ALIGN                      2
#define SIE_CA_WIN_H_ALIGN                      2
#define SIE_CA_WIN_MAX_W                        254
#define SIE_CA_WIN_MAX_H                        254

#define SIE_LA_CROP_WIN_W_ALIGN                 2
#define SIE_LA_CROP_WIN_H_ALIGN                 2
#define SIE_LA_CROP_WIN_X_ALIGN                 1
#define SIE_LA_CROP_WIN_Y_ALIGN                 1
#define SIE_LA_CROP_WIN_MAX_X                   320         ///< 640/2
#define SIE_LA_CROP_WIN_MAX_Y                   384         ///< 768/2
#define SIE_LA_CROP_WIN_MAX_W                   320         ///< scale down to 640, split to rgb, transfer to Y only
#define SIE_LA_CROP_WIN_MAX_H                   384         ///< scale down to 384, split to rgb, transfer to Y only
#define SIE_LA_CROP_WIN_MIN_W                   64
#define SIE_LA_CROP_WIN_MIN_H                   64

#define SIE_LA_WIN_W_ALIGN                      2
#define SIE_LA_WIN_H_ALIGN                      2
#define SIE_LA_WIN_MAX_W                        254
#define SIE_LA_WIN_MAX_H                        254

#define SIE_MASK_X_ALIGN                        1           ///< Mask not supported in this platform
#define SIE_MASK_Y_ALIGN                        1
#define SIE_MASK_MIN_X                          0
#define SIE_MASK_MIN_Y                          0
#define SIE_MASK_MAX_X                          16383
#define SIE_MASK_MAX_Y                          16383

#define SIE_IN1_LINEOFFSET_ALIGN                4           ///< word align
#define SIE_IN3_LINEOFFSET_ALIGN                4           ///< word align
#define SIE_OUT0_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT1_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT2_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT3_LINEOFFSET_ALIGN               4           ///< word align

#define SIE_IN1_LINEOFFSET_MAX_SIZE             65532       ///< Byte
#define SIE_IN3_LINEOFFSET_MAX_SIZE             65532       ///< Byte
#define SIE_OUT0_LINEOFFSET_MAX_SIZE            65532       ///< Byte
#define SIE_OUT1_LINEOFFSET_MAX_SIZE            65532       ///< Byte
#define SIE_OUT2_LINEOFFSET_MAX_SIZE            65532       ///< Byte
#define SIE_OUT3_LINEOFFSET_MAX_SIZE            65532       ///< Byte

#define SIE_RING_BUFFER_MAX_LEN                 4095        ///< line count, ring buffer as line base

#define SIE_DBGOUT_DATA_MAX_SIZE                80          ///< Byte, 20 word

#define SIE_DVS_WIN_W_ALIGN                     8           ///< dvs 1 group is 8 pixel
#define SIE_DVS_WIN_H_ALIGN                     1
#define SIE_DVS_WIN_MIN_W                       8
#define SIE_DVS_WIN_MIN_H                       1
#define SIE_DVS_WIN_MAX_W                       16376       ///< 16383 align to 8
#define SIE_DVS_WIN_MAX_H                       8191

#define SIE_EVS_MOT_OUT_ALIGN                   4           ///< 538  align to 4
#define SIE_EVS_MOT_OUT_ALIGN_539A              2           ///< 539A align to 2
#define SIE_EVS_MOT_OUT_MIN_SIZE                4
#define SIE_EVS_MOT_OUT_MAX_SIZE                0xfffffffc

#define SIE_OB_OFS_MAX                          0x7ff       ///< 11bits
#define SIE_OB_OFS_MAX_539A                     0xffff      ///< 16bits

/***********************************************************************************************
 * SIE T-Sensor Limitation Definition (only SIE5)
 ***********************************************************************************************/
#define SIE_TSEN_TX_CFG_MIN_LEN                 4
#define SIE_TSEN_TX_CFG_MAX_LEN                 512         ///< 538  max up to 512
#define SIE_TSEN_TX_CFG_MAX_LEN_539A            640         ///< 539A max up to 640

#define SIE_TSEN_TX_FS_CODE_MIN_LEN             4
#define SIE_TSEN_TX_FS_CODE_MAX_LEN             32
#define SIE_TSEN_TX_LS_CODE_MIN_LEN             4
#define SIE_TSEN_TX_LS_CODE_MAX_LEN             32

#define SIE_TSEN_TX_OOC_WIN_W_ALIGN             1
#define SIE_TSEN_TX_OOC_WIN_H_ALIGN             1
#define SIE_TSEN_TX_OOC_WIN_MAX_W               4095
#define SIE_TSEN_TX_OOC_WIN_MAX_H               4095

#define SIE_TSEN_TX_OOC_WIN_MAX_W_6BIT_PACK     2730        ///< tx line buffer (16384bit)/6 = 2730 pixel
#define SIE_TSEN_TX_OOC_WIN_MAX_W_7BIT_PACK     2340        ///< tx line buffer (16384bit)/7 = 2340 pixel
#define SIE_TSEN_TX_OOC_WIN_MAX_W_8BIT_PACK     2048        ///< tx line buffer (16384bit)/8 = 2048 pixel

#define SIE_TSEN_TX_OOC_DUMMY_TOP_MAX_SIZE      255
#define SIE_TSEN_TX_OOC_DUMMY_BOT_MAX_SIZE      255

#define SIE_TSEN_TX_VBLANK_MAX_SIZE             65535
#define SIE_TSEN_TX_HBLANK_MAX_SIZE             65535

/***********************************************************************************************
 * SIE1 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE1_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE1_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE1_CROP_WIN_MAX_W_8BIT_LEGACY420      SIE_CROP_WIN_MAX_W_8BIT_LEGACY420
#define SIE1_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE1_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE1_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE1_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE1_BCC_PIXEL_RATE                     2           ///< 2-pixel base
#define SIE1_BCC_HW_ABILITY                     1           ///< support BCC hardware function
#define SIE1_BCC_SHARE_SRC                      0           ///< SIE1
#define SIE1_BCC_SHARE_INPUT_WIN_MIN_W          64          ///< width >= 64, input source from crop or bayer scaling output

/***********************************************************************************************
 * SIE2 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE2_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE2_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE2_CROP_WIN_MAX_W_8BIT_LEGACY420      SIE_CROP_WIN_MAX_W_8BIT_LEGACY420
#define SIE2_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE2_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE2_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE2_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE2_BCC_PIXEL_RATE                     0           ///< not support BCC hardware function
#define SIE2_BCC_HW_ABILITY                     0           ///< not support BCC hardware function, share with SIE1
#define SIE2_BCC_SHARE_SRC                      0           ///< SIE1
#define SIE2_BCC_SHARE_INPUT_WIN_MIN_W          64          ///< width >= 64, input source from crop or bayer scaling output

/***********************************************************************************************
 * SIE3 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE3_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE3_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE3_CROP_WIN_MAX_W_8BIT_LEGACY420      SIE_CROP_WIN_MAX_W_8BIT_LEGACY420
#define SIE3_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE3_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE3_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE3_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE3_BCC_PIXEL_RATE                     2           ///< 2-pixel base
#define SIE3_BCC_HW_ABILITY                     1           ///< support BCC hardware function
#define SIE3_BCC_SHARE_SRC                      2           ///< SIE3
#define SIE3_BCC_SHARE_INPUT_WIN_MIN_W          64          ///< width >= 64, input source from crop or bayer scaling output

/***********************************************************************************************
 * SIE4 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE4_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE4_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE4_CROP_WIN_MAX_W_8BIT_LEGACY420      SIE_CROP_WIN_MAX_W_8BIT_LEGACY420
#define SIE4_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE4_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE4_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE4_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE4_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE4_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE4_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE4_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE4_BCC_PIXEL_RATE                     0           ///< not support BCC hardware function
#define SIE4_BCC_HW_ABILITY                     0           ///< not support BCC hardware function, share with SIE3
#define SIE4_BCC_SHARE_SRC                      2           ///< SIE3
#define SIE4_BCC_SHARE_INPUT_WIN_MIN_W          64          ///< width >= 64, input source from crop or bayer scaling output

/***********************************************************************************************
 * SIE5 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE5_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE5_MAX_INPUT_NUM                      3

#define SIE5_CROP_WIN_MAX_W_8BIT_LEGACY420      SIE_CROP_WIN_MAX_W_8BIT_LEGACY420
#define SIE5_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE5_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE5_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE5_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE5_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE5_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE5_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE5_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE5_BCC_PIXEL_RATE                     0           ///< not support BCC hardware function
#define SIE5_BCC_HW_ABILITY                     0           ///< not support BCC hardware function
#define SIE5_BCC_SHARE_SRC                      3           ///< SIE4
#define SIE5_BCC_SHARE_INPUT_WIN_MIN_W          64          ///< width >= 64, input source from crop or bayer scaling output

/***********************************************************************************************
 * SIE Function Definition
 ***********************************************************************************************/
typedef enum {
    SIE_SSDRV_FUNC_NONE          = 0x00000000,      ///< none

    SIE_SSDRV_FUNC_DIRECT        = 0x00000001,      ///< direct to ipp
    SIE_SSDRV_FUNC_PATGEN        = 0x00000002,      ///< pattern gen
    SIE_SSDRV_FUNC_DVI           = 0x00000004,      ///< YUV sensor, support 8bit_YUV422, 8bit_LEGACY_YUV420
    SIE_SSDRV_FUNC_OB_AVG        = 0x00000008,      ///< ob average

    SIE_SSDRV_FUNC_PDAF          = 0x00000010,      ///< data extraction
    SIE_SSDRV_FUNC_OB_BYPASS     = 0x00000020,      ///< ob bypass
    SIE_SSDRV_FUNC_SINGLE_OUT    = 0x00000040,      ///< single out
    SIE_SSDRV_FUNC_RING_BUF      = 0x00000080,      ///< ring buffer

    SIE_SSDRV_FUNC_MASK          = 0x00000100,      ///< mask0~3
    SIE_SSDRV_FUNC_CGAIN         = 0x00000200,      ///< color gain
    SIE_SSDRV_FUNC_DVS           = 0x00000400,      ///< DVS data parser
    SIE_SSDRV_FUNC_DPC           = 0x00000800,      ///< defect pixel compensation

    SIE_SSDRV_FUNC_MD            = 0x00001000,      ///< motion detection
    SIE_SSDRV_FUNC_ROI_ACC       = 0x00002000,      ///< ROI accumulation
    SIE_SSDRV_FUNC_COLDEF        = 0x00004000,      ///< Defect column concealment
    SIE_SSDRV_FUNC_ECS           = 0x00008000,      ///< ECS

    SIE_SSDRV_FUNC_DGAIN         = 0x00010000,      ///< digital gain
    SIE_SSDRV_FUNC_BS_H          = 0x00020000,      ///< horizontal bayer scale
    SIE_SSDRV_FUNC_BS_V          = 0x00040000,      ///< vertical   bayer scale
    SIE_SSDRV_FUNC_RAWENC        = 0x00080000,      ///< raw compression(BCC)

    SIE_SSDRV_FUNC_FLIP_H        = 0x00100000,      ///< mirror
    SIE_SSDRV_FUNC_LA_HISTO      = 0x00200000,      ///< luminance accumulation histogram output
    SIE_SSDRV_FUNC_FLIP_V        = 0x00400000,      ///< flip
    SIE_SSDRV_FUNC_LA            = 0x00800000,      ///< luminance accumulation output

    SIE_SSDRV_FUNC_CA            = 0x01000000,      ///< color accumulation output
    SIE_SSDRV_FUNC_STCS_DGAIN    = 0x02000000,      ///< digital gain for statistics data
    SIE_SSDRV_FUNC_DBG           = 0x04000000,      ///< debug info
    SIE_SSDRV_FUNC_VDHD_DLY      = 0x08000000,      ///< vdhd delay

    SIE_SSDRV_FUNC_COMPANDING    = 0x10000000,      ///< companding/decompanding function
    SIE_SSDRV_FUNC_RGGB_FMT_SEL  = 0x20000000,      ///< RGGB  2x2 format selection
    SIE_SSDRV_FUNC_RGBIR_FMT_SEL = 0x40000000,      ///< RGBIR 4x4 format selection
    SIE_SSDRV_FUNC_EVS           = 0x80000000,      ///< EVS data parser

    SIE_SSDRV_FUNC_TSEN          = 0x100000000ULL,  ///< TSEN tx/rx function
} SIE_SSDRV_FUNC_SUPPORT;

/***********************************************************************************************
 * SIE Hardware Support Function Support Definition
 ***********************************************************************************************/
#define SIE_SUPPORT_FUNC    (SIE_SSDRV_FUNC_PATGEN        | \
                             SIE_SSDRV_FUNC_DVI           | \
                             SIE_SSDRV_FUNC_PDAF          | \
                             SIE_SSDRV_FUNC_OB_BYPASS     | \
                             SIE_SSDRV_FUNC_SINGLE_OUT    | \
                             SIE_SSDRV_FUNC_RING_BUF      | \
                             SIE_SSDRV_FUNC_CGAIN         | \
                             SIE_SSDRV_FUNC_DPC           | \
                             SIE_SSDRV_FUNC_ECS           | \
                             SIE_SSDRV_FUNC_DGAIN         | \
                             SIE_SSDRV_FUNC_FLIP_H        | \
                             SIE_SSDRV_FUNC_FLIP_V        | \
                             SIE_SSDRV_FUNC_LA_HISTO      | \
                             SIE_SSDRV_FUNC_LA            | \
                             SIE_SSDRV_FUNC_CA            | \
                             SIE_SSDRV_FUNC_STCS_DGAIN    | \
                             SIE_SSDRV_FUNC_DBG           | \
                             SIE_SSDRV_FUNC_VDHD_DLY      | \
                             SIE_SSDRV_FUNC_COMPANDING    | \
                             SIE_SSDRV_FUNC_RGGB_FMT_SEL  | \
                             SIE_SSDRV_FUNC_RGBIR_FMT_SEL)

#define SIE1_SUPPORT_FUNC   (SIE_SUPPORT_FUNC | SIE_SSDRV_FUNC_RAWENC | SIE_SSDRV_FUNC_BS_H | SIE_SSDRV_FUNC_BS_V | SIE_SSDRV_FUNC_DIRECT)
#define SIE2_SUPPORT_FUNC   (SIE_SUPPORT_FUNC | SIE_SSDRV_FUNC_RAWENC | SIE_SSDRV_FUNC_BS_H | SIE_SSDRV_FUNC_BS_V)
#define SIE3_SUPPORT_FUNC   (SIE_SUPPORT_FUNC | SIE_SSDRV_FUNC_RAWENC)
#define SIE4_SUPPORT_FUNC   (SIE_SUPPORT_FUNC | SIE_SSDRV_FUNC_RAWENC)
#define SIE5_SUPPORT_FUNC   (SIE_SUPPORT_FUNC | SIE_SSDRV_FUNC_DVS | SIE_SSDRV_FUNC_EVS | SIE_SSDRV_FUNC_COLDEF | SIE_SSDRV_FUNC_TSEN)

#endif  /* _SIE_ENG_LIMIT_H_ */
