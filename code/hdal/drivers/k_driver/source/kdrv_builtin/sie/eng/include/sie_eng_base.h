/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _SIE_ENG_BASE_H_
#define _SIE_ENG_BASE_H_

#include "kwrap/type.h"
#include "sie_eng_handle.h"

typedef enum {
    FUNC_PATGEN_EN        = (1<<1),
    FUNC_DVI_EN           = (1<<2),
    FUNC_OB_AVG_EN        = (1<<3),     ///< not supported
    FUNC_OB_SUB_SEL       = (1<<4),     ///< not supported
    FUNC_OB_BYPASS_EN     = (1<<5),
    FUNC_MASK0_EN         = (1<<6),     ///< not supported
    FUNC_MASK1_EN         = (1<<7),     ///< not supported
    FUNC_MASK2_EN         = (1<<8),     ///< not supported
    FUNC_MASK3_EN         = (1<<9),     ///< not supported
    FUNC_DVS_DECOMP_EN    = (1<<10),    ///< only SIE5
    FUNC_DPC_EN           = (1<<11),
    FUNC_COLDEF_EN        = (1<<12),    ///< only SIE5, dpc column defect, DPC must enable, bayer_fmt must RGB
    FUNC_ACT_DATAEXT_EN   = (1<<13),
    FUNC_ACT2_DATAEXT_EN  = (1<<14),    ///< not supported
    FUNC_ECS_EN           = (1<<15),
    FUNC_DGAIN_EN         = (1<<16),
    FUNC_BS_H_EN          = (1<<17),
    FUNC_BS_V_EN          = (1<<18),
    FUNC_RAWENC_EN        = (1<<19),
    FUNC_CGAIN_EN         = (1<<20),
    FUNC_ROI_ACC_EN       = (1<<21),    ///< not supported
    FUNC_STCS_HISTO_Y_EN  = (1<<22),
    FUNC_STCS_LA_EN       = (1<<23),
    FUNC_STCS_CA_EN       = (1<<24),
    FUNC_STCS_DGAIN_EN    = (1<<25),
    FUNC_EVS_EN           = (1<<26),    ///< only SIE5
    FUNC_DRAM_OUT0_EN     = (1<<27),
    FUNC_COMPANDING_EN    = (1<<28),
    FUNC_DBG_EN           = (1<<31)
} SIE_FUNCTION_SEL;

typedef enum {
    MAIN_IN_PARA_MSTR_SNR = 0,  ///< Parallel Master Sensor
    MAIN_IN_SELF_PATGEN,        ///< Self Pattern-Generator(1-pixel), not supported
    MAIN_IN_CSI_1,              ///< Serial Sensor from CSI-1
    MAIN_IN_CSI_2,              ///< Serial Sensor from CSI-2
    MAIN_IN_CSI_3,              ///< Serial Sensor from CSI-3
    MAIN_IN_CSI_4,              ///< Serial Sensor from CSI-4
    MAIN_IN_CSI_5,              ///< Serial Sensor from CSI-5       , not supported
    MAIN_IN_CSI_6,              ///< Serial Sensor from CSI-6       , not supported
    MAIN_IN_CSI_7,              ///< Serial Sensor from CSI-7       , not supported
    MAIN_IN_CSI_8,              ///< Serial Sensor from CSI-8       , not supported
    MAIN_IN_SLVS_EC_1,          ///< Serial Sensor from SLVS_EC-1   , not supported
    MAIN_IN_SLVS_EC_2,          ///< Serial Sensor from SLVS_EC-2   , not supported
    MAIN_IN_2PPATGEN,           ///< Self Pattern-Generator(2-pixel)
    MAIN_IN_4PPATGEN,           ///< Self Pattern-Generator(4-pixel), not supported
    MAIN_IN_DRAM_IN,            ///< from DRAM IN1
    MAIN_IN_PARA_THERMAL_SNR,   ///< Parallel Thermal Sensor        , only SIE5
    MAIN_IN_MAX,
    ENUM_DUMMY4WORD(SIE_MAIN_INPUT_SEL)
} SIE_MAIN_INPUT_SEL;

typedef enum {
    DATA_PHASE_RISING  = 0,     ///< rising  edge latch/trigger
    DATA_PHASE_FALLING,         ///< falling edge latch/trigger
    DATA_PHASE_MAX,
    ENUM_DUMMY4WORD(SIE_PHASE_SEL)
} SIE_PHASE_SEL;

typedef enum {
    /* for RGGB 2x2 */
    CFAPAT_R  = 0,              ///< start pixel is R
    CFAPAT_Gr = 1,              ///< start pixel is GR
    CFAPAT_Gb = 2,              ///< start pixel is GB
    CFAPAT_B  = 3,              ///< start pixel is B

    /* for RGBIR 4x4 */
    CFAPAT_RG_GI = 0,
    CFAPAT_GB_IG = 1,
    CFAPAT_GI_BG = 2,
    CFAPAT_IG_GR = 3,
    CFAPAT_BG_GI = 4,
    CFAPAT_GR_IG = 5,
    CFAPAT_GI_RG = 6,
    CFAPAT_IG_GB = 7,

    CFAPAT_MAX,
    ENUM_DUMMY4WORD(SIE_CFAPAT_SEL)
} SIE_CFAPAT_SEL;

typedef enum {
    PATGEN_MODE_COLORBAR_V = 1, ///< Color bar, Vertical
    PATGEN_MODE_RANDOM,         ///< Random
    PATGEN_MODE_FIXED,          ///< Fixed
    PATGEN_MODE_HINCREASE,      ///< 1D Increment
    PATGEN_MODE_HVINCREASE,     ///< 2D increment
    PATGEN_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_PATGEN_MODE)
} SIE_PATGEN_MODE;

typedef enum {
    PATGEN_SYNC_MODE_NORMAL = 0,
    PATGEN_SYNC_MODE_SYNC,
    PATGEN_SYNC_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_PATGEN_SYNC_MODE)
} SIE_PATGEN_SYNC_MODE;

typedef enum {
    PATGEN_SYNC_SRC_DISABLE = 0,
    PATGEN_SYNC_SRC_SIE1,
    PATGEN_SYNC_SRC_SIE2,
    PATGEN_SYNC_SRC_SIE3,
    PATGEN_SYNC_SRC_SIE4,
    PATGEN_SYNC_SRC_SIE5,
    PATGEN_SYNC_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_PATGEN_SYNC_SRC)
} SIE_PATGEN_SYNC_SRC;

typedef enum {
    PATGEN_SYNC_DELAY_0T = 0,
    PATGEN_SYNC_DELAY_4T,
    PATGEN_SYNC_DELAY_8T,
    PATGEN_SYNC_DELAY_16T,
    PATGEN_SYNC_DELAY_MAX,
    ENUM_DUMMY4WORD(SIE_PATGEN_SYNC_DELAY)
} SIE_PATGEN_SYNC_DELAY;

typedef enum {
    DATA_PACKBUS_8 = 0,         ///<  8 bits packing
    DATA_PACKBUS_10,            ///< 10 bits packing
    DATA_PACKBUS_12,            ///< 12 bits packing
    DATA_PACKBUS_16,            ///< 16 bits packing
    DATA_PACKBUS_MAX,
    ENUM_DUMMY4WORD(SIE_DATA_PACKBUS_SEL)
} SIE_DATA_PACKBUS_SEL;

typedef enum {
    IN3_DATA_PACKBUS_6 = 0,     ///< 6 bits packing
    IN3_DATA_PACKBUS_7,         ///< 7 bits packing
    IN3_DATA_PACKBUS_8,         ///< 8 bits packing
    IN3_DATA_PACKBUS_MAX,
    ENUM_DUMMY4WORD(SIE_IN3_DATA_PACKBUS_SEL)
} SIE_IN3_DATA_PACKBUS_SEL;

typedef enum {
    PARAL_RAW_BIT_12 = 0,       ///< 12 bits input
    PARAL_RAW_BIT_10,           ///< 10 bits input
    PARAL_RAW_BIT_MAX,
    ENUM_DUMMY4WORD(SIE_PARAL_RAW_BITDEPTH)
} SIE_PARAL_RAW_BITDEPTH;       ///< for parallel interface

typedef enum {
    SERIAL_RAW_BIT_12 = 0,      ///< 12 bits input
    SERIAL_RAW_BIT_14,          ///< 14 bits input
    SERIAL_RAW_BIT_16,          ///< 16 bits input
    SERIAL_RAW_BIT_MAX,
    ENUM_DUMMY4WORD(SIE_SERIAL_RAW_BITDEPTH)
} SIE_SERIAL_RAW_BITDEPTH;      ///< for serial interface, MIPI/LVDS/PATGEN

typedef enum {
    DRAM_OUT_MODE_NORMAL = 0,
    DRAM_OUT_MODE_SINGLE,
    DRAM_OUT_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_OUT_MODE)
} SIE_DRAM_OUT_MODE;

typedef enum {
    STCS_OUT_MODE_NORMAL = 0,
    STCS_OUT_MODE_SINGLE,
    STCS_OUT_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_OUT_MODE)
} SIE_STCS_OUT_MODE;

typedef enum {
    TIMING_DELAY_0T = 0,
    TIMING_DELAY_4T,
    TIMING_DELAY_8T,
    TIMING_DELAY_16T,
    TIMING_DELAY_MAX,
    ENUM_DUMMY4WORD(SIE_TIMING_DELAY)
} SIE_TIMING_DELAY;

typedef enum {
    PXCNT_INIT_VAL_FFFF = 0,
    PXCNT_INIT_VAL_0000,
    PXCNT_INIT_VAL_MAX,
    ENUM_DUMMY4WORD(SIE_PXCNT_INIT_VAL)
} SIE_PXCNT_INIT_VAL;

typedef enum {
    SIE_GLOBAL_LOAD_SEL_RST_LOAD = 0,   ///< send load signal on sw_reset 1->0 or set_load=1 when VD comes
    SIE_GLOBAL_LOAD_SEL_LOAD_ONLY,      ///< send load signal on set_load=1 when VD comes
    SIE_GLOBAL_LOAD_SEL_MAX,
    ENUM_DUMMY4WORD(SIE_GLOBAL_LOAD_SEL)
} SIE_GLOBAL_LOAD_SEL;

typedef enum {
    SIE_LOAD_DISABLE = 0,
    SIE_LOAD_FROM_SIE1,
    SIE_LOAD_FROM_SIE2,
    SIE_LOAD_FROM_SIE3,
    SIE_LOAD_FROM_SIE4,
    SIE_LOAD_FROM_SIE5,
    SIE_LOAD_FROM_MAX,
    ENUM_DUMMY4WORD(SIE_LOAD_SRC)
} SIE_LOAD_SRC;

typedef enum {
    SIE_OUTBIT_12 = 0,          ///< 12 output bits
    SIE_OUTBIT_10,              ///< 10 output bits
    SIE_OUTBIT_8,               ///< 8  output bits
    SIE_OUTBIT_14,              ///< 14 output bits, support when MAIN_IN = MAIN_IN_PARA_THERMAL_SNR, 538 only SIE5 supported, 539A All SIE supported
    SIE_OUTBIT_16,              ///< 16 output bits, 538 not supported, 539A All SIE supported
    SIE_OUTBIT_MAX,
    ENUM_DUMMY4WORD(SIE_OUTBIT_SEL)
} SIE_OUTBIT_SEL;

typedef enum {
    SIE_PACK_AT_MSB  = 0,       ///< data output at MSB
    SIE_PACK_AT_LSB,            ///< data output at LSB
    SIE_PACK_AT_MAX,
    ENUM_DUMMY4WORD(SIE_PACK_FORMAT)
} SIE_PACK_FORMAT;

typedef enum {
    OUT3_SOURCE_ACT2 = 0,       ///< SIE Dram output channel 3 source = variation accumulation
    OUT3_SOURCE_CCIR_UV,        ///< SIE Dram output channel 3 source = cropped and scaled image
    OUT3_SOURCE_MAX,
    ENUM_DUMMY4WORD(SIE_OUT3_SRC_SEL)
} SIE_OUT3_SRC_SEL;             ///< not supported

typedef enum {
    SIE_FMT_BAYER = 0,          ///< Bayer  2x2 format
    SIE_FMT_RGBIR,              ///< RGB-IR 4x4 format
    SIE_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_BAYER_FMT)
} SIE_BAYER_FMT;

typedef enum {
    ECS_BAYER_3CH_10BIT = 0,    ///< Bayer  2x2 format
    ECS_BAYER_4CH_8BIT,         ///< RGB-IR 4x4 format
    ECS_BAYER_MAX,
    ENUM_DUMMY4WORD(SIE_ECS_BAYER_FMT)
} SIE_ECS_BAYER_FMT;

typedef enum {
    SIE_OB_SUB_OFS = 0,         ///< ob sub use ofs
    SIE_OB_SUB_OBAVG,           ///< ob sub use ob avg result
    SIE_OB_SUB_MAX,
    ENUM_DUMMY4WORD(SIE_OB_SUB_MODE)
} SIE_OB_SUB_MODE;              ///< not supported

typedef enum {
    ECS_MAPSIZE_65X65  = 0,     ///< 65x65
    ECS_MAPSIZE_49X49,          ///< 49x49
    ECS_MAPSIZE_33X33,          ///< 33x33
    ECS_MAPSIZE_MAX,
    ENUM_DUMMY4WORD(SIE_ECS_MSIZE_SEL)
} SIE_ECS_MSIZE_SEL;

typedef enum {
    STCS_HISTO_POST_GAMMA = 0,
    STCS_HISTO_PRE_GAMMA,
    STCS_HISTO_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_HISTO_SRC_SEL)
} SIE_HISTO_SRC_SEL;

typedef enum {
    STCS_CA_SRC_AFTER_IRSUB = 0,
    STCS_CA_SRC_BEFORE_OB,
    STCS_CA_SRC_SEL_MAX,
    ENUM_DUMMY4WORD(SIE_CA_SRC_SEL)
} SIE_CA_SRC_SEL;

typedef enum {
    STCS_LA1_POST_CG = 0,       ///< LA1 source from data right after  color-gain
    STCS_LA1_PRE_CG,            ///< LA1 source from data right before color-gain
    STCS_LA1_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA1_SRC_SEL)
} SIE_STCS_LA1_SRC_SEL;

typedef enum {
    STCS_LA1_RGB = 0,
    STCS_LA1_G,
    STCS_LA1_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA_RGB2Y1_MOD_SEL)
} SIE_STCS_LA_RGB2Y1_MOD_SEL;

typedef enum {
    STCS_LA2_RGB = 0,
    STCS_LA2_G,
    STCS_LA2_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA_RGB2Y2_MOD_SEL)
} SIE_STCS_LA_RGB2Y2_MOD_SEL;

typedef enum {
    DEF_FACT_50 = 0,            ///< defect concealment cross-channel weighting = 50%
    DEF_FACT_25,                ///< defect concealment cross-channel weighting = 25%; recommanded
    DEF_FACT_12,                ///< defect concealment cross-channel weighting = 12.5%
    DEF_FACT_06,                ///< defect concealment cross-channel weighting = 6.25%
    DEF_FACT_MAX,
    ENUM_DUMMY4WORD(SIE_DEF_FACT_SEL)
} SIE_DEF_FACT_SEL;

typedef enum {
    DEF_MODE_NORMAL = 0,        ///< normal mode for DPC
    DEF_MODE_DEBUG,             ///< debug  mode for DPC data replace
    DEF_MODE_PROGRESSIVE,       ///< progressive mode for DPC, only SIE5
    DEF_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_DEF_MODE_SEL)
} SIE_DEF_MODE_SEL;

typedef enum {
    GAIN_FIELD_2_8 = 0,         ///< 2bits decimal, 8bits fraction
    GAIN_FIELD_3_7,             ///< 3bits decimal, 7bits fraction
    GAIN_FIELD_MAX,
    ENUM_DUMMY4WORD(SIE_GAIN_FIELD)
} SIE_GAIN_FIELD;

typedef enum {
    EV_FMT_16BIT = 0,           ///< 16bit mode
    EV_FMT_20BIT,               ///< 20bit mode
    EV_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_COMPANDING_EV_FMT_SEL)
} SIE_COMPANDING_EV_FMT_SEL;

typedef enum {
    MD_MASK_COL_ROW = 0,
    MD_MASK_8X8,
    MD_MASK_MAX,
    ENUM_DUMMY4WORD(SIE_MD_MASK_MODE)
} SIE_MD_MASK_MODE;             ///< not supported

typedef enum {
    MD_SRC_PREGAMMA = 0,
    MD_SRC_POSTGAMMA,
    MD_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_MD_SRC)
} SIE_MD_SRC;                   ///< not supported

typedef enum {
    DATAEXT_ORDER_EXT_SKIP = 0,
    DATAEXT_ORDER_SKIP_EXT,
    DATAEXT_ORDER_MAX,
    ENUM_DUMMY4WORD(SIE_DATAEXT_ORDER)
} SIE_DATAEXT_ORDER;

typedef enum {
    DATAEXT_INI_OP_DROP_INI_LINE = 0,
    DATAEXT_INI_OP_KEEP_INI_LINE,
    DATAEXT_INI_OP_MAX,
    ENUM_DUMMY4WORD(SIE_DATAEXT_INI_OP)
} SIE_DATAEXT_INI_OP;

typedef enum {
    DVI_IN_FMT_YUV422 = 0,
    DVI_IN_FMT_YUV420,                      ///< Legacy-YUV420
    DVI_IN_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_IN_FMT_SEL)
} SIE_DVI_IN_FMT_SEL;

typedef enum {
    DVI_OUT_SWAP_YUYV = 0,
    DVI_OUT_SWAP_YVYU,
    DVI_OUT_SWAP_UYVY,
    DVI_OUT_SWAP_VYUY,
    DVI_OUT_SWAP_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_OUT_SWAP_SEL)
} SIE_DVI_OUT_SWAP_SEL;                     ///< output swap for DVI_IN_FMT_YUV422

typedef enum {
    DVI_YUV420_OUT_SWAP_YYU_YYV = 0,
    DVI_YUV420_OUT_SWAP_YUY_YVY,
    DVI_YUV420_OUT_SWAP_UYY_VYY,
    DVI_YUV420_OUT_SWAP_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_YUV420_OUT_SWAP_SEL)
} SIE_DVI_YUV420_OUT_SWAP_SEL;              ///< output swap for DVI_IN_FMT_YUV420

typedef enum {
	ENC_ADJ_MODE_NORMAL = 0,    ///< rawenc parameters for normal    mode
	ENC_ADJ_MODE_AGGRESIVE,     ///< rawenc parameters for aggresive mode
	ENC_ADJ_MODE_MAX,
	ENUM_DUMMY4WORD(SIE_RAWENC_ADJ_MODE_SEL)
} SIE_RAWENC_ADJ_MODE_SEL;

typedef enum {
    ENC_RATE_50 = 0,
    ENC_RATE_58,
    ENC_RATE_66,            ///< not supported
    ENC_RATE_41,            ///< not supported
    ENC_RATE_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_RATE_SEL)
} SIE_RAWENC_RATE_SEL;

typedef enum {
    ENC_SHARE_NONE = 0,
    ENC_SHARE_SHARED,
    ENC_SHARE_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_SHARE_SEL)
} SIE_RAWENC_SHARE_SEL;

typedef enum {
    ENC_STCS_ID_0 = 0,      ///< SIE1/3
    ENC_STCS_ID_1,          ///< SIE2/4
    ENC_STCS_ID_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_STCS_ID_SEL)
} SIE_RAWENC_STCS_ID_SEL;

typedef enum {
    ENC_FRM_RST_ID_0 = 0,
    ENC_FRM_RST_ID_1,
    ENC_FRM_RST_ID_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_FRM_RST_ID_SEL)
} SIE_RAWENC_FRM_RST_ID_SEL;                ///< only supported for SIE1/3

typedef struct {
    UINT8                   tbl_idx[8];     ///< Raw encoder - Quality DCT Table
    UINT8                   lvl_thres[4];   ///< Raw encoder - Quality DCT Level Threshold
} SIE_RAWENC_DCT_PARAM;

typedef struct {
    UINT8                   tbl_lbn[16];    ///< Raw encoder - Quality DPCM Table
    UINT8                   tbl_vlc[16];    ///< Raw encoder - Quality DPCM Table
} SIE_RAWENC_DPCM_PARAM;                    ///< not supported

typedef struct {
    SIE_RAWENC_ADJ_MODE_SEL adj_mode;       ///< Raw encoder - Table parameter adjust mode
    SIE_RAWENC_RATE_SEL     rate;           ///< Raw encoder - Target rate of encoded bitstream per segment
    BOOL                    gamma_en;       ///< Raw encoder - Gamma
    SIE_RAWENC_DCT_PARAM    dct;            ///< Raw encoder - DCT  Param
    SIE_RAWENC_DPCM_PARAM   dpcm;           ///< Raw encoder - DPCM Param, not supported
} SIE_RAWENC_PARAM;

typedef struct {
    SIE_PHASE_SEL           vd_phase;       ///< VD phase
    SIE_PHASE_SEL           hd_phase;       ///< HD phase
    SIE_PHASE_SEL           data_phase;     ///< Data phase
    BOOL                    vd_inv;         ///< VD Invert
    BOOL                    hd_inv;         ///< HD Invert
    SIE_PARAL_RAW_BITDEPTH  raw_bitdepsel;
} SIE_PARAL_SIGNAL_PARAM;                   ///< for parallel interface

typedef struct {
    SIE_SERIAL_RAW_BITDEPTH raw_bitdepsel;
} SIE_SERIAL_SIGNAL_PARAM;                  ///< for serial interface, MIPI/LVDS/PATGEN, only 539A supported

typedef struct {
    SIE_PATGEN_MODE         patgen_mode;
    UINT32                  patgen_val;
} SIE_PATGEN_PARAM;

typedef struct {
    SIE_PATGEN_SYNC_MODE    sync_mode;
    SIE_PATGEN_SYNC_SRC     sync_src;
    SIE_PATGEN_SYNC_DELAY   sync_dly;
} SIE_PATGEN_SYNC_PARAM;

typedef struct {
    UINT32  sz_x;               ///< horizontal size
    UINT32  sz_y;               ///< vertical size
} SIE_SRC_WIN_PARAM;

typedef struct {
    UINT16          st_x;       ///< horizontal start
    UINT16          st_y;       ///< vertical start
    UINT32          sz_x;       ///< horizontal size
    UINT32          sz_y;       ///< vertical size
    SIE_CFAPAT_SEL  cfa_pat;    ///< CFA start pattern
} SIE_ACT_WIN_PARAM;

typedef struct {
    UINT16          st_x;       ///< horizontal start
    UINT16          st_y;       ///< vertical start
    UINT32          sz_x;       ///< horizontal size
    UINT32          sz_y;       ///< vertical size
} SIE_ACT2_WIN_PARAM;           ///< not supported

typedef struct {
    UINT16          st_x;       ///< horizontal start
    UINT16          st_y;       ///< vertical start
    UINT32          sz_x;       ///< horizontal size
    UINT32          sz_y;       ///< vertical size
    SIE_CFAPAT_SEL  cfa_pat;    ///< CFA start pattern
} SIE_CRP_WIN_PARAM;

typedef struct {
    SIE_TIMING_DELAY vd_delay;
    SIE_TIMING_DELAY hd_delay;
} SIE_VDHD_DELAY_PARAM;

typedef enum {
    DRAM_BURST_LEN_64 = 0,
    DRAM_BURST_LEN_48,
    DRAM_BURST_LEN_32,
    DRAM_BURST_LEN_16,
    DRAM_BURST_LEN_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_BURST_LEN_SEL)
} SIE_DRAM_BURST_LEN_SEL;

typedef struct {
    SIE_DRAM_BURST_LEN_SEL out0;
    SIE_DRAM_BURST_LEN_SEL out1;
    SIE_DRAM_BURST_LEN_SEL out2;
    SIE_DRAM_BURST_LEN_SEL in1;
    SIE_DRAM_BURST_LEN_SEL in2;
    SIE_DRAM_BURST_LEN_SEL in3;             ///< only SIE5
} SIE_DRAM_BURST_LEN_PARAM;

typedef struct {
    UINT16   write_num;                     ///< 0 ~ 255, default is 15
    UINT16   read_num;                      ///< 0 ~ 255, default is 15
} SIE_DRAM_OUTSTD_PARAM;

typedef struct {
    ULONG   addr_pa;
} SIE_DRAM_ADDR;

typedef struct {
    UINT64  addr_pa;
} SIE_DRAM_ADDR64;

typedef struct {
    UINT32                lofs;             ///< line-offset
    SIE_DATA_PACKBUS_SEL  pack_bus;         ///< pack bus bit number
} SIE_DRAM_IN1_PARAM;

typedef struct {
    UINT32                   lofs;          ///< line-offset
    SIE_IN3_DATA_PACKBUS_SEL pack_bus;      ///< pack bus bit number
} SIE_DRAM_IN3_PARAM;

typedef struct {
    UINT32                lofs;             ///< line-offset
    SIE_DATA_PACKBUS_SEL  pack_bus;         ///< pack bus bit number
    BOOL                  hflip;            ///< mirror
    BOOL                  vflip;            ///< flip
    BOOL                  ringbuf_en;
    UINT16                ringbuf_len;
    SIE_OUTBIT_SEL        out_bitdepth;
    SIE_PACK_FORMAT       pack_fmt;
} SIE_DRAM_OUT0_PARAM;

typedef struct {
    UINT32                lofs;             ///< line-offset
    BOOL                  vflip;            ///< H flip
    BOOL                  hflip;            ///< V flip, not supported, will apply from OUT0_HFLIP if OUT0 have line buffer
} SIE_DRAM_OUT1_PARAM;

typedef struct {
    UINT32                lofs;             ///< line-offset
    SIE_DATA_PACKBUS_SEL  pack_bus;         ///< pack bus bit number
    BOOL                  hflip;            ///< H flip
    BOOL                  vflip;            ///< V flip
    SIE_OUTBIT_SEL        out_bitdepth;
    SIE_PACK_FORMAT       pack_fmt;
    SIE_OUT3_SRC_SEL      out_src;          ///< 0: act2, 1:ccir-uv
} SIE_DRAM_OUT3_PARAM;                      ///< not supported

typedef struct {
    UINT8   pos_code;                       ///< positive code value
    UINT8   neg_code;                       ///< negative code value
    UINT8   nochg_code;                     ///< no changed code value
} SIE_DVS_CODE_PARAM;

typedef struct {
    UINT32  sz_x;
    UINT32  sz_y;
} SIE_DVS_WIN_PARAM;

typedef struct {
    UINT32  mot_out_sz;             ///< must > 0 and align 4
} SIE_EVS_OUT_PARAM;

typedef struct {
    UINT16  padding_val;            ///< padding value when output size not 4 byte align
} SIE_EVS_PADDING_PARAM;

typedef struct {
    UINT16          r_gain;         ///< Color Gain, R  Gain
    UINT16          gr_gain;        ///< Color Gain, Gr Gain
    UINT16          gb_gain;        ///< Color Gain, Gb Gain
    UINT16          b_gain;         ///< Color Gain, B  Gain
    UINT16          ir_gain;        ///< Color Gain, B  Gain
    SIE_GAIN_FIELD  gain_sel;       ///< gain filed selection
} SIE_CGAIN_PARAM;

typedef struct {
    UINT32  gain_8p8bit;            ///< digital gain in 8.8 bits format
} SIE_DGAIN_PARAM;

typedef struct {
    UINT32 decomp_kpx[32];          ///< 0 ~ 0xfff
    UINT32 decomp_kpy[32];          ///< 0 ~ 0xfff
    UINT32 decomp_gain[32];         ///< 0 ~ 0x3ff
    UINT32 decomp_sb[32];           ///< 0 ~ 0x1f
} SIE_DECOMPANDING_PARAM;

typedef struct {
    UINT32                      comp_fcurve_l[65];      ///< 0 ~ 0xfff
    UINT32                      comp_fcurve_m[17];      ///< 0 ~ 0xfff
    UINT32                      comp_fcurve_r[16];      ///< 0 ~ 0xfff
    SIE_COMPANDING_EV_FMT_SEL   comp_ev_fmt;
} SIE_COMPANDING_PARAM;

typedef struct {
    BOOL                        comp_fcurve_y_enb;      ///< bayer scaling must disable when fcurve_y enable
    UINT32                      comp_y_gain_max;        ///< 0 ~ 0xfff
    UINT32                      comp_y_gain_shift;      ///< 0 ~ 0xf
} SIE_COMPANDING_Y_PARAM;

typedef struct {
    UINT8                       comp_yweight_lut[17];   ///< 0 ~ 0xff
} SIE_COMPANDING_Y_WEIGHT_PARAM;

typedef struct {
    UINT32  st_x;       ///< window horizontal start
    UINT32  st_y;       ///< window vertical start
    UINT32  sz_x;       ///< window horizontal size
    UINT32  sz_y;       ///< window vertical size
    UINT32  div_x;      ///< window horizontal subsample
    UINT32  thres;      ///< threshold
    UINT16  sub_ratio;  ///< OB Detect Result Subtraction Ratio
} SIE_OB_AVG_PARAM;

typedef struct {
    UINT32  avg;
    UINT32  cnt;
} SIE_OB_DT_RSLT;

typedef struct {
    UINT16  ofs;
} SIE_OB_OFS_PARAM;

typedef struct {
    BOOL                dither_en;      ///< dithering enable
    BOOL                dither_rst;     ///< dithering reset
    UINT32              dither_lvl;     ///< dithering level; 0x0=bit[1:0], 0x7=bit[8:7]
    SIE_ECS_MSIZE_SEL   map_size_sel;   ///< ecs table size
    UINT32              map_shift;      ///< multiplication shift bit number
} SIE_ECS_PARAM;

typedef struct {
    UINT32  hscl_fctr;                  ///< map horizontal scaling factor
    UINT32  vscl_fctr;                  ///< map vertical scaling factor
} SIE_ECS_SCAL_PARAM;

typedef struct {
    UINT16  x0;
    UINT16  y0;
    UINT16  x1;
    UINT16  y1;
    UINT16  color_r;
    UINT16  color_g;
    UINT16  color_b;
    UINT16  color_ir;
} SIE_MASK_PARAM;       ///< not supported

typedef struct {
    SIE_DEF_FACT_SEL    fact;
    SIE_CFAPAT_SEL      cfa_pat;
    BOOL                def_same_ch_enb;///< deffect cross channel weighting, 538 not supported
} SIE_DPC_PARAM;

typedef struct {
    SIE_DEF_MODE_SEL    mode;
    UINT16              dbg_value;      ///< DPC replace value when mode = DEF_MODE_DEBUG
} SIE_DPC_MODE_PARAM;

typedef struct {
    UINT16              def_col[32];    ///< 0 ~ 0x3fff
} SIE_DPC_COLDEF_PARAM;

typedef struct {
    SIE_DATAEXT_ORDER   ext_order;
    SIE_DATAEXT_INI_OP  ini_line_op;
    UINT8               ext_data_skip;
    UINT8               ext_data_len;
    UINT16              ini_line_len;
} SIE_DATAEXT_PARAM;

typedef struct {
    BOOL    src_intpV;      ///< vertical interpolation, only for input width<=8188 or 5504
    UINT32  iv;             ///< bayer scaling - integration value
    UINT32  sv;             ///< bayer scaling - skipping value
    UINT32  bvR;            ///< bayer scaling - begin value for R/Gb channel
    UINT32  bvB;            ///< bayer scaling - begin value for B/Gr channel
    UINT32  out_sz;         ///< bayer scaling - output size
    UINT32  divM[2];        ///< bayer scaling - division parameter M
    UINT32  divS[2];        ///< bayer scaling - division parameter S
} SIE_BS_H_PARAM;

typedef struct {
    UINT32  iv;             ///< bayer scaling - integration value
    UINT32  sv;             ///< bayer scaling - skipping value
    UINT32  bvR;            ///< bayer scaling - begin value for R/Gb channel
    UINT32  bvB;            ///< bayer scaling - begin value for B/Gr channel
    UINT32  out_sz;         ///< bayer scaling - output size
    UINT32  divM[2];        ///< bayer scaling - division parameter M
    UINT32  divS[2];        ///< bayer scaling - division parameter S
    UINT32  in_shift;       ///< bayer scaling - input shift
    UINT32  divMRb[16];     ///< bayer scaling - division parameter M for R/B
    UINT32  divSRb[16];     ///< bayer scaling - division parameter S for R/B
} SIE_BS_V_PARAM;

typedef struct {
    UINT32  in_sz_x;        ///< input  horizontal size
    UINT32  in_sz_y;        ///< input  vertical size
    UINT32  out_sz_x;       ///< output horizontal size
    UINT32  out_sz_y;       ///< output vertical size
    UINT32  lpf;            ///< low-pass valumn, 0~100, (suggestion: 50)
    UINT32  bin_pwr;        ///< binning, 0~2, (suggestion: 0)
    BOOL    adaptivelpf_en; ///< 0: LPF is absolute value; 1: LPF is adaptive value (suggestion: 1)
} SIE_BS_PARAM;

typedef struct {
    BOOL                       la_cg_en;
    BOOL                       la_gamma_en;
    BOOL                       la_th_en;            ///< not supported
    BOOL                       ca_th_en;
    UINT16                     companding_shift;    ///< must setup when companding enable and roi_acc or ca/la enable
    SIE_HISTO_SRC_SEL          histo_src_sel;
    SIE_STCS_LA1_SRC_SEL       la1_src_sel;
    SIE_STCS_LA_RGB2Y1_MOD_SEL la_rgb2y1_mod;
    SIE_STCS_LA_RGB2Y2_MOD_SEL la_rgb2y2_mod;
    SIE_CA_SRC_SEL             ca_accm_src;
    UINT16                     rang_shift;          ///< right shift bit when companding is disable, 0~8, only 539A supported
} SIE_STCS_PATH_PARAM;

typedef struct {
    SIE_CFAPAT_SEL  ca_cfa_pat;
    UINT32          ca_roi_stx;
    UINT32          ca_roi_sty;
    UINT32          ca_roi_szx;
    UINT32          ca_roi_szy;
    UINT32          ca_win_numx;
    UINT32          ca_win_numy;
    UINT32          la_roi_stx;
    UINT32          la_roi_sty;
    UINT32          la_roi_szx;
    UINT32          la_roi_szy;
    UINT32          la_win_numx;
    UINT32          la_win_numy;
} SIE_STCS_CALASIZE_INFO;

typedef struct {
    UINT16  ca_ob_ofs;          ///< OB ofs for CA
    UINT16  la_ob_ofs;          ///< OB ofs for LA
} SIE_STCS_OB_PARAM;

typedef struct {
    UINT32  fctr_x;             ///< horizontal scaling factor
    UINT32  fctr_y;             ///< vertical   scaling factor
} SIE_CA_SCALE_PARAM;

typedef struct {
    UINT32  ca_irsub_rwet;      ///< IR weight for subtraction, R channel
    UINT32  ca_irsub_gwet;      ///< IR weight for subtraction, G channel
    UINT32  ca_irsub_bwet;      ///< IR weight for subtraction, B channel
} SIE_CA_IRSUB_PARAM;

typedef struct {
    UINT32  la_irsub_rwet;      ///< IR weight for subtraction, R channel
    UINT32  la_irsub_gwet;      ///< IR weight for subtraction, G channel
    UINT32  la_irsub_bwet;      ///< IR weight for subtraction, B channel
} SIE_LA_IRSUB_PARAM;

typedef struct {
    UINT32         st_x;        ///< horizontal start
    UINT32         st_y;        ///< vertical start
    UINT32         sz_x;        ///< horizontal size
    UINT32         sz_y;        ///< vertical size
    SIE_CFAPAT_SEL cfa_pat;     ///< CFA start pattern
} SIE_CA_CROP_PARAM;

typedef struct {
    UINT32  st_x;               ///< horizontal start
    UINT32  st_y;               ///< vertical start
    UINT32  sz_x;               ///< horizontal size
    UINT32  sz_y;               ///< vertical size
} SIE_LA_CROP_PARAM;

typedef struct {
    UINT32  win_numx;           ///< color accumulation - horizontal window number
    UINT32  win_numy;           ///< color accumulation - vertical window number
    UINT32  win_szx;            ///< color accumulation - horizontal window size
    UINT32  win_szy;            ///< color accumulation - vertical window size
} SIE_CA_WIN_PARAM;

typedef struct {
    UINT32  win_numx;           ///< color accumulation - horizontal window number
    UINT32  win_numy;           ///< color accumulation - vertical window number
    UINT32  win_szx;            ///< color accumulation - horizontal window size
    UINT32  win_szy;            ///< color accumulation - vertical window size
    UINT32  dgain;
} SIE_LA_WIN_PARAM;

typedef struct {
    UINT32  la_thy1_lower;
    UINT32  la_thy1_upper;
    UINT32  la_thy2_lower;
    UINT32  la_thy2_upper;
} SIE_LA_TH_PARAM;              ///< not supported

typedef struct {
    UINT16  r_gain;             ///< R  Gain
    UINT16  g_gain;             ///< G  Gain
    UINT16  b_gain;             ///< B  Gain
} SIE_LA_CG_PARAM;

typedef struct {
    UINT16  *buf_la1;
    UINT16  *buf_la2;
} SIE_LA_RSLT;

typedef struct {
    UINT16  *buf_r;
    UINT16  *buf_g;
    UINT16  *buf_b;
    UINT16  *buf_ir;
    UINT16  *acc_cnt;
} SIE_CA_RSLT;

typedef struct {
    UINT16  g_th_lower;
    UINT16  g_th_upper;
    UINT16  rg_th_lower;
    UINT16  rg_th_upper;
    UINT16  bg_th_lower;
    UINT16  bg_th_upper;
    UINT16  pg_th_lower;
    UINT16  pg_th_upper;
} SIE_CA_TH_PARAM;

typedef struct {
    SIE_CA_WIN_PARAM  ca_win;
    SIE_STCS_OB_PARAM stcs_ob;
    SIE_CGAIN_PARAM   cgain;
    SIE_BAYER_FMT     bayer_fmt;
    UINT32            dgain;
} SIE_CA_RSLT_PARAMS;

typedef struct {
    UINT16  *buf_hist;
} SIE_HIST_RSLT;

typedef struct {
    SIE_MD_SRC        md_src;
    UINT8             sum_frms;
    SIE_MD_MASK_MODE  mask_mode;
    UINT32            mask0;
    UINT32            mask1;
    UINT32            blkdiff_thr;
    UINT32            total_blkdiff_thr;
    UINT16            blkdiff_cnt_thr;
} SIE_MD_PARAM;         ///< not supported

typedef struct {
    UINT8   *buf_md_thr_Rslt;
    UINT16   blkdiff_cnt;
    UINT32   total_blkdiff;
} SIE_MD_RESULT_INFO;   ///< not supported

typedef struct {
    SIE_DVI_IN_FMT_SEL          in_fmt;             ///< DVI input format, YUV422 or Legacy YUV420
    SIE_DVI_YUV420_OUT_SWAP_SEL yuv420_out_swap;    ///< swap control for DVI_IN_FMT_YUV420

    SIE_DVI_OUT_SWAP_SEL        out_swap;           ///< swap control for DVI_IN_FMT_YUV422
    BOOL                        out_split;          ///< must enable when DVI_IN_FMT_YUV420 or out_yuv420_en = 1
    BOOL                        out_yuv420_en;      ///< YUV422 to YUV420 control for DVI_IN_FMT_YUV422
} SIE_DVI_PARAM;

typedef struct {
    UINT16  st_x;               ///< horizontal start
    UINT16  st_y;               ///< vertical start
    UINT16  sz_x;               ///< horizontal size
    UINT16  sz_y;               ///< vertical size
} SIE_ROI_ACC_WIN_PARAM;

typedef struct {
    UINT32  *buf_acc_rslt;      ///< pointer to result buffer, 5x5 window accumulation result, 32bit counter for each window
} SIE_ROI_ACC_RSLT;

typedef struct {
    BOOL    pxclk_in;           ///< pixel clock in,  for parallel sensor debug only
    BOOL    sieclk_in;          ///< sie module clock in
    UINT32  cur_line_cnt;       ///< current line count
    UINT32  max_line_cnt;       ///< max line count
    UINT32  max_pixel_cnt;      ///< Max pixel count
    UINT32  csi_hd_cnt;         ///< Hd count from CSI
    UINT32  csi_line_end_cnt;   ///< Line end count from CSI
    UINT32  csi_pixel_cnt;      ///< Pixel count of each line from CSI
    UINT32  vd_pxclk_cnt;       ///< VD time, count by apb clock
    UINT32  hd_pxclk_cnt;       ///< HD time, count by apb clock
    UINT32  dvs_timestamp;      ///< timestamp of DVS
    UINT32  vd_cnt;             ///< VD count, count start from SW_RST from 1 to 0
    UINT32  crp_end_cnt;        ///< Crop-end count
} SIE_SYS_DEBUG_INFO;

typedef struct {
    UINT32  act;                ///< active window checksum
    UINT32  ecs;                ///< ecs checksum
    UINT32  crop;               ///< crop window checksum
    UINT32  bcc_out;            ///< bcc output checksum
    UINT32  ch0_buf_in;         ///< ch0 line buffer input checksum
    UINT32  dpc_in;             ///< dcp dram in checksum
    UINT32  ecs_in;             ///< ecs dram in checksum
} SIE_CHECKSUM_INFO;

/*************************************************************************************
 *  Thermal Sensor RX definition (only SIE5)
 *************************************************************************************/

typedef enum {
    TSEN_RX_DECODE_MODE_DVP_8BIT = 0,               ///< PCLK + VD + HD + D0~7,  SDR mode
    TSEN_RX_DECODE_MODE_DVP_14BIT,                  ///< PCLK + VD + HD + D0~13, SDR mode
    TSEN_RX_DECODE_MODE_SYNC_CODE,                  ///< base on sync code setting, PCLK + D0~3
    TSEN_RX_DECODE_MODE_TX_FS_LS_4BIT,              ///< base on OOC TX FS/LS loopback signal with FS/LS_Delay setting, PCLK + D0~3
    TSEN_RX_DECODE_MODE_TX_FS_LS_14BIT,             ///< base on OOC TX FS/LS loopback signal with FS/LS_Delay setting, PCLK + D0~13
    TSEN_RX_DECODE_MODE_DVP_8BIT_DDR,               ///< PCLK + VD + HD + D0~7,  DDR mode
    TSEN_RX_DECODE_MODE_DVP_14BIT_DDR,              ///< PCLK + VD + HD + D0~13  DDR mode
    TSEN_RX_DECODE_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_RX_DECODE_MODE_SEL)
} SIE_TSEN_RX_DECODE_MODE_SEL;

#define TSEN_RX_DECODE_MODE_TX_FS_LS                TSEN_RX_DECODE_MODE_TX_FS_LS_4BIT   ///< for backward compatible

typedef enum {
    TSEN_RX_OUT_FMT_MSB = 0,                        ///< output data for raw 14bit from buffer[15:0] MSB => [15:2]
    TSEN_RX_OUT_FMT_LSB,                            ///< output data for raw 14bit from buffer[15:0] LSB => [13:0]
    TSEN_RX_OUT_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_RX_OUT_FMT_SEL)
} SIE_TSEN_RX_OUT_FMT_SEL;

typedef struct {
    SIE_TSEN_RX_DECODE_MODE_SEL mode;               ///< receive frame data mode
    SIE_TSEN_RX_OUT_FMT_SEL     out_fmt;            ///< receive frame data output format

    BOOL    hi_byte_inv;                            ///< [15:8][7:0] => [8:15][7:0]
    BOOL    lo_byte_inv;                            ///< [15:8][7:0] => [15:8][0:7]

    UINT8   hi_byte_r_shift;                        ///< [15:8][7:0] => 2bit shift, [15:10][0][0][7:0]
    UINT8   lo_byte_r_shift;                        ///< [15:8][7:0] => 2bit shift, [15 :8][7:2][0][0]

    BOOL    two_byte_swap;                          ///< [15:8][7:0] => [7:0][15:8]
    BOOL    two_byte_inv;                           ///< [15:0]      => [0:15]
    UINT8   two_byte_r_shift;                       ///< [15:0]      => 3bit shift, [15:3][0][0][0]
} SIE_TSEN_RX_DATA_PARAM;

typedef struct {
    UINT16 fs_code;                                 ///< frame start detect code
    UINT16 ls_code;                                 ///< line  start detect code
} SIE_TSEN_RX_SYNC_CODE_PARAM;

typedef struct {
    UINT16 fs_delay;                                ///< frame start delay cycle when rx_mode=TSEN_RX_DECODE_MODE_TX_FS_LS
    UINT16 ls_delay;                                ///< line  start delay cycle when rx_mode=TSEN_RX_DECODE_MODE_TX_FS_LS
} SIE_TSEN_RX_DELAY_PARAM;

/*************************************************************************************
 *  Thermal Sensor TX definition (only SIE5)
 *************************************************************************************/
typedef enum {
    TSEN_TX_STAGE_CONFIG = 0,                       ///< CFG commond mode
    TSEN_TX_STAGE_STREAM,                           ///< OOC stream  mode
    TSEN_TX_STAGE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_STAGE_SEL)
} SIE_TSEN_TX_STAGE_SEL;

typedef enum {
    TSEN_TX_CFG_MODE_SD0_M1 = 0,                    ///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~511
    TSEN_TX_CFG_MODE_SD0_1,                         ///< output port 2-bit, SD0 => TX_CONFIG_DATA bit0~255, SD1 => TX_CONFIG_DATA bit256~511
    TSEN_TX_CFG_MODE_SD0_2,                         ///< output port 3-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383
    TSEN_TX_CFG_MODE_SD0_3,                         ///< output port 4-bit, SD0 => TX_CONFIG_DATA bit0~127, SD1 => TX_CONFIG_DATA bit128~255, SD2 => TX_CONFIG_DATA bit256~383, SD3 => TX_CONFIG_DATA bit384~511
    TSEN_TX_CFG_MODE_SD0_5_M1,                      ///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~95,  SD1 => TX_CONFIG_DATA bit96~191,  SD2 => TX_CONFIG_DATA bit192~287, SD3 => TX_CONFIG_DATA bit288~383, SD4 => TX_CONFIG_DATA bit384~479, SD5 => TX_CONFIG_DATA bit480~575
    TSEN_TX_CFG_MODE_SD0_M2,                        ///< output port 1-bit, SD0 => TX_CONFIG_DATA bit0~639
    TSEN_TX_CFG_MODE_SD0_5_M2,                      ///< output port 6-bit, SD0 => TX_CONFIG_DATA bit0~319, SD1 => TX_CONFIG_DATA bit320~383, SD2 => TX_CONFIG_DATA bit384~447, SD3 => TX_CONFIG_DATA bit448~511, SD4 => TX_CONFIG_DATA bit512~575, SD5 => TX_CONFIG_DATA bit576~639
    TSEN_TX_CFG_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_CFG_MODE_SEL)
} SIE_TSEN_TX_CFG_MODE_SEL;

#define TSEN_TX_CFG_MODE_SD0                        TSEN_TX_CFG_MODE_SD0_M1     ///< for backward compatible
#define TSEN_TX_CFG_MODE_SD0_5                      TSEN_TX_CFG_MODE_SD0_5_M1   ///< for backward compatible

typedef enum {
    TSEN_TX_SYNC_CODE_MODE_SD0 = 0,                 ///< output port 1-bit
    TSEN_TX_SYNC_CODE_MODE_SD0_1,                   ///< output port 2-bit
    TSEN_TX_SYNC_CODE_MODE_SD0_2,                   ///< output port 3-bit
    TSEN_TX_SYNC_CODE_MODE_SD0_6,                   ///< output port 7-bit
    TSEN_TX_SYNC_CODE_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_SYNC_CODE_MODE_SEL)
} SIE_TSEN_TX_SYNC_CODE_MODE_SEL;

typedef enum {
    TSEN_TX_OOC_MODE_SD0 = 0,                       ///< output port 1-bit
    TSEN_TX_OOC_MODE_SD0_1_2PXL_M1,                 ///< output port 2-bit, 2-pixel mode-1, SD0=>Pixel(N,0), SD1=>Pixel(N,1)
    TSEN_TX_OOC_MODE_SD0_1_2PXL_M2,                 ///< output port 2-bit, 2-pixel mode-2, SD0=>Pixel(N,1), SD1=>Pixel(N,0)
    TSEN_TX_OOC_MODE_SD0_1,                         ///< output port 2-bit, 1-pixel mode
    TSEN_TX_OOC_MODE_SD0_2_M1,                      ///< output port 3-bit, 1-pixel mode-1, SD[0 1 2]=>T0:[0 1 2] T1:[3 4 5]
    TSEN_TX_OOC_MODE_SD0_6,                         ///< output port 7-bit
    TSEN_TX_OOC_MODE_SD0_2_M2,                      ///< output port 3-bit, 1-pixel mode-2, SD[0 1 2]=>T0:[2 1 0] T1:[5 4 3]
    TSEN_TX_OOC_MODE_SD0_2_M3,                      ///< output port 3-bit, 1-pixel mode-2, SD[0 1 2]=>T0:[1 3 5] T1:[0 2 4]
    TSEN_TX_OOC_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_OOC_MODE_SEL)
} SIE_TSEN_TX_OOC_MODE_SEL;

#define TSEN_TX_OOC_MODE_SD0_2                      TSEN_TX_OOC_MODE_SD0_2_M1   ///< for backward compatible

typedef enum {
    TSEN_TX_OOC_BITDEPTH_6 = 0,
    TSEN_TX_OOC_BITDEPTH_7,
    TSEN_TX_OOC_BITDEPTH_8,
    TSEN_TX_OOC_BITDEPTH_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_OOC_BITDEPTH_SEL)
} SIE_TSEN_TX_OOC_BITDEPTH_SEL;                     ///< apply when TX_OOC_MODE = SD0_1_2PXL_M1 or SD0_1_2PXL_M2

typedef enum {
    TSEN_TX_LEVEL_LOW = 0,                          ///< unused data output level low
    TSEN_TX_LEVEL_HIGH,                             ///< unused data output level high
    TSEN_TX_LEVEL_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_LEVEL_SEL)
} SIE_TSEN_TX_LEVEL_SEL;

typedef enum {
    SIE_TSEN_TX_FS_IDLE_MODE_HILO_END = 0,          ///< frame sync signal go to idle when tx high/low clock cycle end
    SIE_TSEN_TX_FS_IDLE_MODE_DATA_END,              ///< frame sync signal go to idle when tx config/stream data end
    SIE_TSEN_TX_FS_IDLE_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_FS_IDLE_MODE_SEL)
} SIE_TSEN_TX_FS_IDLE_MODE_SEL;                     ///< only 539A supported

typedef enum {
    SIE_TSEN_TX_FS_INV_NONE = 0,                    ///< frame sync signal invert disable
    SIE_TSEN_TX_FS_INV_OUT,                         ///< frame sync signal invert output(high/low/delay/idle)
    SIE_TSEN_TX_FS_INV_HILO,                        ///< frame sync signal invert high/low only, delay/idle keep low
    SIE_TSEN_TX_FS_INV_HILO_OUT,                    ///< frame sync signal invert high/low + output
    SIE_TSEN_TX_FS_INV_MAX,
    ENUM_DUMMY4WORD(SIE_TSEN_TX_FS_INV_SEL)
} SIE_TSEN_TX_FS_INV_SEL;                           ///< only 539A supported

typedef struct {
    SIE_TSEN_TX_LEVEL_SEL          sd_lvl[7];       ///< SD#0~6 output level
} SIE_TSEN_TX_LEVEL_PARAM;

typedef struct {
    SIE_TSEN_TX_CFG_MODE_SEL       mode;            ///< config data output port mode
    UINT16                         tx_len;          ///< from 4 ~ 640 clock cycle
    UINT32                         tx_data[20];     ///< config data, 538 max up to 512bit, 539A max up to 640bit
    UINT32                         tx_period;       ///< config data output period, clock cycle, 539A only, from 0 ~ 31 => means 1~32 clock cycle output 1 data
    UINT32                         tx_blanking[2];  ///< blanking clock cycle for tx config,     539A only, | blanking[0] | tx_config | blanking[1] |, from 0 ~ 0xffff
} SIE_TSEN_TX_CFG_DATA_PARAM;

typedef struct {
    SIE_TSEN_TX_OOC_MODE_SEL       mode;            ///< OOC data output port mode
    SIE_TSEN_TX_OOC_BITDEPTH_SEL   bitdepth;        ///< support on TX_OOC_TRANS_MODE_SD0_1_2PXL_M1 and TX_OOC_TRANS_MODE_SD0_1_2PXL_M2
    BOOL                           data_swap;       ///< data swap, none(MSB->LSB), do-swap(LSB->MSB)
    UINT8                          data_r_shift;    ///< data right-shift 0 ~ 7
    UINT8                          err_det_code;    ///< OOC illegal code detection
    UINT8                          err_rep_code;    ///< OOC replace code for illegal data
} SIE_TSEN_TX_OOC_DATA_PARAM;

typedef struct {
    SIE_TSEN_TX_SYNC_CODE_MODE_SEL mode;            ///< sync_code data output port mode
    UINT16                         fs_code_len;     ///< FS code length, 4 ~ 32 clock cycle
    UINT16                         ls_code_len;     ///< LS code length, 4 ~ 32 clock cycle
    UINT32                         fs_code_sd[8];   ///< FS code data
    UINT32                         ls_code_sd[8];   ///< LS code data
} SIE_TSEN_TX_SYNC_CODE_PARAM;

typedef struct {
    UINT16  width;                                  ///< OOC frame width
    UINT16  height;                                 ///< OOC frame height
    UINT8   dummy_top_cnt;                          ///< OOC frame dummy top    line count
    UINT8   dummy_bot_cnt;                          ///< OOC frame dummy bottom line count
} SIE_TSEN_TX_OOC_WIN_PARAM;

typedef struct {
    UINT16  vblanking[3];                           ///< OOC frame vertical   blanking0,1,2, unit: clock cycle
    UINT16  hblanking[3];                           ///< OOC frame horizontal blanking0,1,2, unit: clock cycle
} SIE_TSEN_TX_OOC_BLANKING_PARAM;

typedef struct {
    UINT32  tx_dly_cnt;                             ///< clock count of delay for frame sync signal,      from 0 ~ 0xffff
    UINT32  tx_high_cnt;                            ///< clock count of level high for frame sync signal, from 1 ~ 0xffffffff
    UINT32  tx_low_cnt;                             ///< clock count of level low  for frame sync signal, from 1 ~ 0xffffffff
} SIE_TSEN_TX_FS_CLKCNT;                            ///< only 539A supported, SD6 as FS output pin when enable tsen_tx_fs

/*********************************************************************************************************
  T-Sensor TX OOC Timing

  |----------------------------------------------------|   TSEN_START = 1
  |                       VB0                          |
  |----------------------------------------------------|
  | FS  |                                              |
  |----------------------------------------------------|
  |                       VB1                          |
  |----------------------------------------------------|
  | HB0 | LS | HB1 |      Dummy                  | HB2 |
  |----------------------------------------------------|
  | HB0 | LS | HB1 |      OOC Data               | HB2 |
  |----------------------------------------------------|
  | HB0 | LS | HB1 |      OOC Data               | HB2 |
  |----------------------------------------------------|
  :                        :                           :
  :                        :                           :
  |----------------------------------------------------|
  | HB0 | LS | HB1 |      OOC Data               | HB2 |
  |----------------------------------------------------|
  | HB0 | LS | HB1 |      Dummy                  | HB2 |
  |----------------------------------------------------|
  |                       VB2                          |
  |----------------------------------------------------|

 *********************************************************************************************************/

/*************************************************************************************
 *  Public Set Function Prototype [Direct apply]
 *************************************************************************************/
int sie_eng_set_soft_reset_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_checksum_en_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_interrupt_enable_hw_reg(SIE_ENG_HANDLE *p_eng, UINT32 int_en);
int sie_eng_set_axi_enable_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enable);
int sie_eng_set_load_hw_reg(SIE_ENG_HANDLE *p_eng);
int sie_eng_set_load_clear_hw_reg(SIE_ENG_HANDLE *p_eng);
int sie_eng_set_dramin_start_hw_reg(SIE_ENG_HANDLE *p_eng);
int sie_eng_set_main_input_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_MAIN_INPUT_SEL mainInInfo);
int sie_eng_set_vdhd_delay_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_VDHD_DELAY_PARAM *p_dly_info);
int sie_eng_set_global_load_src_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_LOAD_SRC src);
int sie_eng_set_global_load_sel_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_GLOBAL_LOAD_SEL sel);
int sie_eng_set_parallel_sig_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_PARAL_SIGNAL_PARAM *p_paral_info);
int sie_eng_set_serial_sig_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_SERIAL_SIGNAL_PARAM *p_serial_info);
int sie_eng_set_dvs_code_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_DVS_CODE_PARAM *p_dvs);
int sie_eng_set_patgen_sync_param_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_PATGEN_SYNC_PARAM *p_param);
int sie_eng_set_patgen_sync_go_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_evs_padding_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_EVS_PADDING_PARAM *p_param);
int sie_eng_set_pxcnt_init_val_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_PXCNT_INIT_VAL val);

int sie_eng_set_dpc_mode_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_DPC_MODE_PARAM *p_dpc);
int sie_eng_set_dpc_coldef_param_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_DPC_COLDEF_PARAM *p_param);

int sie_eng_set_companding_y_weight_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_COMPANDING_Y_WEIGHT_PARAM *p_companding);
int sie_eng_set_companding_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_COMPANDING_PARAM *p_companding);
int sie_eng_set_decompanding_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_DECOMPANDING_PARAM *p_decompanding);

int sie_eng_set_la_gamma_hw_reg(SIE_ENG_HANDLE *p_eng, UINT16 *p_gam_tbl);

int sie_eng_set_mask0_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_MASK_PARAM *p_mask0);
int sie_eng_set_mask1_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_MASK_PARAM *p_mask1);
int sie_eng_set_mask2_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_MASK_PARAM *p_mask2);
int sie_eng_set_mask3_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_MASK_PARAM *p_mask3);

int sie_eng_set_rawenc_share_sel_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_SHARE_SEL share);
int sie_eng_set_rawenc_stcs_id_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_STCS_ID_SEL stcs_id);
int sie_eng_set_rawenc_frm_rst_id_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_FRM_RST_ID_SEL frm_rst_id);

int sie_eng_set_dma_ch_disable_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL disable);

int sie_eng_set_dram_outstanding_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUTSTD_PARAM *p_outstd);

int sie_eng_set_dramin1_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramin2_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramin3_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout0_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout1_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout2_axi_ch_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);

int sie_eng_set_dramin1_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);
int sie_eng_set_dramin2_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);
int sie_eng_set_dramin3_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);
int sie_eng_set_dramout0_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);
int sie_eng_set_dramout1_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);
int sie_eng_set_dramout2_axi_ch_lock_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL lock);

int sie_eng_set_tsen_tx_start_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_tsen_tx_stage_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_STAGE_SEL stage);
int sie_eng_set_tsen_tx_ooc_load_hw_reg(SIE_ENG_HANDLE *p_eng);
int sie_eng_set_tsen_tx_level_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_LEVEL_PARAM *param);
int sie_eng_set_tsen_tx_cfg_data_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_CFG_DATA_PARAM *param);
int sie_eng_set_tsen_tx_ooc_data_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_OOC_DATA_PARAM *param);
int sie_eng_set_tsen_tx_sync_code_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_SYNC_CODE_PARAM *param);
int sie_eng_set_tsen_tx_ooc_blanking_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_OOC_BLANKING_PARAM *param);
int sie_eng_set_tsen_tx_ooc_dummy_value_hw_reg(SIE_ENG_HANDLE *p_eng, UINT8 value);
int sie_eng_set_tsen_tx_ooc_trans_invert_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_tsen_tx_combine_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_tsen_tx_fs_enb_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_tsen_tx_fs_idle_mode_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_FS_IDLE_MODE_SEL mode);
int sie_eng_set_tsen_tx_fs_inv_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_FS_INV_SEL inv);
int sie_eng_set_tsen_tx_fs_clkcnt_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_FS_CLKCNT *p_clkcnt);

int sie_eng_set_tsen_rx_data_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_RX_DATA_PARAM *param);
int sie_eng_set_tsen_rx_sync_code_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_RX_SYNC_CODE_PARAM *param);
int sie_eng_set_tsen_rx_delay_hw_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_RX_DELAY_PARAM *param);
int sie_eng_set_tsen_mclk_invert_hw_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);

/*************************************************************************************
 *  Public Set Function Prototype [FS latched]
 *************************************************************************************/
int sie_eng_set_tsen_tx_ooc_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_TSEN_TX_OOC_WIN_PARAM *param);

/*************************************************************************************
 *  Public Set Function Prototype [VD latched]
 *************************************************************************************/
int sie_eng_set_params_to_reg(SIE_ENG_HANDLE *p_eng);

int sie_eng_set_act_enable_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_ipp_ctrl_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL direct2ipp_en);
int sie_eng_set_function_enable_buf_reg(SIE_ENG_HANDLE *p_eng, UINT32 func_en);
int sie_eng_set_single_function_enable_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_FUNCTION_SEL func_item, BOOL enb);
int sie_eng_set_bayer_fmt_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BAYER_FMT fmt);
int sie_eng_set_ecs_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ECS_BAYER_FMT mode);
int sie_eng_set_dvi_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DVI_PARAM *p_dvi);
int sie_eng_set_patgen_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_PATGEN_PARAM *p_param);

int sie_eng_set_rawenc_rate_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_RATE_SEL rate);
int sie_eng_set_rawenc_gamma_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_rawenc_dct_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_DCT_PARAM *p_dct);
int sie_eng_set_rawenc_dpcm_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_DPCM_PARAM *p_dpcm);
int sie_eng_set_rawenc_param_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_RAWENC_ADJ_MODE_SEL mode, SIE_RAWENC_RATE_SEL rate);

int sie_eng_set_src_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_SRC_WIN_PARAM *p_src_win);
int sie_eng_set_act_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ACT_WIN_PARAM *p_act_win);
int sie_eng_set_act2_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ACT2_WIN_PARAM *p_act2_win);
int sie_eng_set_crop_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CRP_WIN_PARAM *p_crp_win);
int sie_eng_set_roi_acc_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ROI_ACC_WIN_PARAM *p_roi_acc_win);

int sie_eng_set_bp1_buf_reg(SIE_ENG_HANDLE *p_eng, UINT32 bp);
int sie_eng_set_bp2_buf_reg(SIE_ENG_HANDLE *p_eng, UINT32 bp);
int sie_eng_set_bp3_buf_reg(SIE_ENG_HANDLE *p_eng, UINT32 bp);

int sie_eng_set_companding_y_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_COMPANDING_Y_PARAM *p_companding);

int sie_eng_set_cgain_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CGAIN_PARAM *p_cgain);
int sie_eng_set_dgain_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DGAIN_PARAM *p_dgain);

int sie_eng_set_ecs_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ECS_PARAM *p_ecs, SIE_ACT_WIN_PARAM *p_actwin);
int sie_eng_set_ecs_scale_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ECS_SCAL_PARAM *p_ecs_scal);

int sie_eng_set_dpc_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DPC_PARAM *p_dpc);

int sie_eng_set_ob_ofs_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_OB_OFS_PARAM *p_ob_ofs);
int sie_eng_set_ob_avg_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_OB_AVG_PARAM *p_ob_avg);

int sie_eng_set_dataext_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DATAEXT_PARAM *p_dataext);
int sie_eng_set_dataext2_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DATAEXT_PARAM *p_dataext2);

int sie_eng_set_bs_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BS_PARAM *p_bs);
int sie_eng_set_bsh_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BS_H_PARAM *p_bsh);
int sie_eng_set_bsv_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BS_V_PARAM *p_bsv);

int sie_eng_set_stcs_path_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_STCS_PATH_PARAM *p_stcs_path);
int sie_eng_set_stcs_ob_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_STCS_OB_PARAM *p_stcs_ob);

int sie_eng_set_ca_crop_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_CROP_PARAM *p_ca_crop);
int sie_eng_set_ca_scale_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_SCALE_PARAM *p_ca_scal);
int sie_eng_set_ca_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_WIN_PARAM *p_la_win);
int sie_eng_set_ca_th_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_TH_PARAM *p_ca_th);
int sie_eng_set_ca_irsub_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_IRSUB_PARAM *p_ca_irsub);

int sie_eng_set_la_crop_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_CROP_PARAM *p_la_crop);
int sie_eng_set_la_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_WIN_PARAM *p_la_win);
int sie_eng_set_la_th_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_TH_PARAM *p_la_th);
int sie_eng_set_la_irsub_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_IRSUB_PARAM *p_la_irsub);
int sie_eng_set_la_cg_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_CG_PARAM *p_la_cg);

int sie_eng_set_cala_size_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_STCS_CALASIZE_INFO *p_sz_info, SIE_LA_WIN_PARAM *p_win_param);

int sie_eng_set_md_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_MD_PARAM *p_md);

int sie_eng_set_dvs_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DVS_WIN_PARAM *p_dvs_win);
int sie_eng_set_evs_out_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_EVS_OUT_PARAM *p_evs_out);

int sie_eng_set_dramin1_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_in1);
int sie_eng_set_dramin2_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_in2);
int sie_eng_set_dramin3_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_in3);
int sie_eng_set_dramout0_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_dout0);
int sie_eng_set_dramout1_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_dout1);
int sie_eng_set_dramout2_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_dout2);
int sie_eng_set_dramout3_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_dout3);
int sie_eng_set_dram_dbgout_addr_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR *p_dbgout);

int sie_eng_set_dramin1_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_IN1_PARAM *p_din1);
int sie_eng_set_dramin3_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_IN3_PARAM *p_din3);
int sie_eng_set_dramout0_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT0_PARAM *p_dout0);
int sie_eng_set_dramout1_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT1_PARAM *p_dout1);
int sie_eng_set_dramout3_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT3_PARAM *p_dout3);

int sie_eng_set_dramout0_singleout_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dramout1_singleout_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dramout2_singleout_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dramout3_singleout_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dram_dbgout_singleout_mode_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT_MODE mode);

int sie_eng_set_dramout0_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout1_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout2_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dramout3_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);
int sie_eng_set_dram_dbgout_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL enb);

int sie_eng_set_dramin1_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_in1);
int sie_eng_set_dramin2_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_in2);
int sie_eng_set_dramin3_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_in3);
int sie_eng_set_dramout0_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_dout0);
int sie_eng_set_dramout1_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_dout1);
int sie_eng_set_dramout2_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_dout2);
int sie_eng_set_dramout3_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_dout3);
int sie_eng_set_dram_dbgout_addr64_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_ADDR64 *p_dbgout);

int sie_eng_set_dram_burst_len_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_BURST_LEN_PARAM *p_burst_len);

/*************************************************************************************
 *  Public Get Function Prototype
 *************************************************************************************/
int sie_eng_get_dramout0_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL *enb);
int sie_eng_get_dramout1_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL *enb);
int sie_eng_get_dramout2_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL *enb);
int sie_eng_get_dramout3_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL *enb);
int sie_eng_get_dram_dbgout_singleout_en_buf_reg(SIE_ENG_HANDLE *p_eng, BOOL *enb);

int sie_eng_get_dramin1_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_IN1_PARAM *p_din1);
int sie_eng_get_dramin3_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_IN3_PARAM *p_din3);
int sie_eng_get_dramout0_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT0_PARAM *p_dout0);
int sie_eng_get_dramout1_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT1_PARAM *p_dout1);
int sie_eng_get_dramout3_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DRAM_OUT3_PARAM *p_dout3);

int sie_eng_get_sysdbg_info(SIE_ENG_HANDLE *p_eng, SIE_SYS_DEBUG_INFO *p_dbg_Info);
int sie_eng_get_checksum_info(SIE_ENG_HANDLE *p_eng, SIE_CHECKSUM_INFO *p_Info);

int sie_eng_get_bayer_fmt_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BAYER_FMT *p_fmt);

int sie_eng_get_cgain_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CGAIN_PARAM *p_cgain);
int sie_eng_get_dgain_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DGAIN_PARAM *p_dgain);

int sie_eng_get_obdt_rslt(SIE_ENG_HANDLE *p_eng, SIE_OB_DT_RSLT *p_obdt_rslt);
int sie_eng_get_ca_rslt(SIE_ENG_HANDLE *p_eng, ULONG addr_va, SIE_CA_RSLT *p_ca_rslt, SIE_CA_RSLT_PARAMS *p_ca_rslt_info);
int sie_eng_get_la_rslt(SIE_ENG_HANDLE *p_eng, ULONG addr_va, SIE_LA_RSLT *p_la_rslt, SIE_LA_WIN_PARAM *p_la_win);
int sie_eng_get_md_rslt(SIE_ENG_HANDLE *p_eng, SIE_MD_RESULT_INFO *p_md_Info);
int sie_eng_get_hist(SIE_ENG_HANDLE *p_eng, SIE_HIST_RSLT *p_hist_rslt);
int sie_eng_get_roi_acc_rslt(SIE_ENG_HANDLE *p_eng, SIE_ROI_ACC_RSLT *p_roi_acc_rslt);

int sie_eng_get_ir_level(SIE_ENG_HANDLE *p_eng, UINT32 *level);
int sie_eng_get_sat_gain_info(SIE_ENG_HANDLE *p_eng, UINT32 *gain);

int sie_eng_get_function_enable_buf_reg(SIE_ENG_HANDLE *p_eng, UINT32 *func_en);
int sie_eng_get_act_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ACT_WIN_PARAM *p_act_win);
int sie_eng_get_act2_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_ACT2_WIN_PARAM *p_act2_win);
int sie_eng_get_crop_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CRP_WIN_PARAM *p_crp_win);
int sie_eng_get_dvi_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_DVI_PARAM *p_dvi);

int sie_eng_get_bsh_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BS_H_PARAM *p_bsh);
int sie_eng_get_bsv_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_BS_V_PARAM *p_bsv);

int sie_eng_get_stcs_path_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_STCS_PATH_PARAM *p_stcs_path);
int sie_eng_get_stcs_ob_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_STCS_OB_PARAM *p_stcs_ob);

int sie_eng_get_ca_crop_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_CROP_PARAM *p_ca_crop);
int sie_eng_get_la_crop_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_CROP_PARAM *p_la_crop);
int sie_eng_get_ca_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_CA_WIN_PARAM *p_ca_win);
int sie_eng_get_la_win_buf_reg(SIE_ENG_HANDLE *p_eng, SIE_LA_WIN_PARAM *p_la_win);

int sie_eng_get_evs_total_cnt_hw_reg(SIE_ENG_HANDLE *p_eng, UINT32 *p_cnt);

int sie_eng_calc_ir_level(SIE_ENG_HANDLE *p_eng, SIE_CA_RSLT *ca_rst, SIE_CA_WIN_PARAM *p_ca_win, SIE_CA_IRSUB_PARAM *p_ca_ir_sub);

UINT32 sie_eng_calc_rawenc_sie_clk_max_rate(SIE_ENG_HANDLE *p_eng, UINT32 bcc_clk_rate, BOOL bcc_share);
UINT32 sie_eng_calc_rawenc_sie_clk_min_rate(SIE_ENG_HANDLE *p_eng, UINT32 bcc_clk_rate);

#endif  /* _SIE_ENG_BASE_H_ */
