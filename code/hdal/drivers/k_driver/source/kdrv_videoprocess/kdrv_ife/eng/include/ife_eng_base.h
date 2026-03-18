/*
    Public header file for IFE module.

    This file is the header file that define the API and data type for IFE
    module.

    @file       ife_lib.h
    @ingroup    mIDrvIPP_IFE

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef _IFE_LIB_538_H
#define _IFE_LIB_538_H

#ifdef __KERNEL__
#include "kwrap/type.h"
#else
#include <kwrap/nvt_type.h>
#endif

#include "ife_eng_handle.h"

#define IFE_GET_API_EN  0


/**
    @addtogroup mIDrvIPP_IFE
*/
//@{

/*
    Define IFE function enable.
*/
//@{
//#define IFE_ROWDEFC_EN                  0x00001000  ///< enable function: row defect concealment
#define IFE_OUTL_EN                     0x00002000  ///< enable function: outlier
#define IFE_FILTER_EN                   0x00004000  ///< enable function: filter
#define IFE_CGAIN_EN                    0x00008000  ///< enable function: color gain
#define IFE_VIG_EN                      0x00010000  ///< enable function: vignette
#define IFE_GBAL_EN                     0x00020000  ///< enable function: Gbalance
#define IFE_RBFILL_EN                   0x00400000  ///< enable function: RB NR Fill (only effective in RGBIr)
#define IFE_CENTERMOD_EN                0x00800000  ///< enable function: center modify
#define IFE_NRS_EN                      0x01000000  ///< enable function: NRS
#define IFE_FCG_EN                      0x02000000  ///< enable function: f_color gain
#define IFE_FUSION_EN                   0x04000000  ///< enable function: sensor HDR
#define IFE_FCURVE_EN                   0x20000000  ///< enable function: F curve
#define IFE_MIRROR_EN                   0x40000000  ///< enable function: mirror
#define IFE_DECODE_EN                   0x80000000  ///< enable function: decode
//#define IFE_FLIP_EN                   0x00200000  ///< removed in NT96680
#define IFE_FUNC_ALL                    0xe7c3e000  ///< all func enable mask
//@}


/**
    Define IFE interrput enable.
*/
//@{
#define IFE_INTE_FRMEND                 0x00000001  ///< frame end interrupt
#define IFE_INTE_DEC1_ERR               0x00000002  ///< frame end interrupt
#define IFE_INTE_DEC2_ERR               0x00000004  ///< frame end interrupt
#define IFE_INTE_LLEND                  0x00000008  ///< LL end interrupt
#define IFE_INTE_LLERR                  0x00000010  ///< LL error  interrupt
#define IFE_INTE_LLERR2                 0x00000020  ///< LL error2 interrupt
#define IFE_INTE_LLJOBEND               0x00000040  ///< LL job end interrupt
#define IFE_INTE_BUFOVFL                0x00000080  ///< buffer overflow interrupt
#define IFE_INTE_RING_BUF_ERR           0x00000100  ///< ring buffer error interrupt
#define IFE_INTE_FRAME_ERR              0x00000200  ///< frame error interrupt (for direct mode)
#define IFE_INTE_DEC3_ERR               0x00000400  ///< path2 decode error interrupt
#define IFE_INTE_RING_BUF_ERR2          0x00000800  ///< ring buffer error 2 interrupt
#define IFE_INTE_SIE_FRM_START          0x00001000  ///< SIE frame start interrupt
#define IFE_INTE_FRM_START               0x00004000  ///< IFE frame start interrupt
#define IFE_INTE_NN_ISP_P0_SET_SLICE_READY  0x00010000  ///< IFE NN_ISP_P0_SET_SLICE_READY
#define IFE_INTE_NN_ISP_P0_SET_SLICE_CLEAR  0x00020000  ///< IFE NN_ISP_P0_SET_SLICE_CLEAR
#define IFE_INTE_NN_ISP_P1_SET_SLICE_READY  0x00040000  ///< IFE NN_ISP_P1_SET_SLICE_READY
#define IFE_INTE_NN_ISP_P1_SET_SLICE_CLEAR  0x00080000  ///< IFE NN_ISP_P1_SET_SLICE_CLEAR

#define IFE_INTE_ALL                    0x000f5fff  ///< all interrupts
//@}

/**
    Define IFE interrput status.
*/
//@{
#define IFE_INT_FRMEND                 0x00000001  ///< frame end interrupt
#define IFE_INT_DEC1_ERR               0x00000002  ///< path0 decode error interrupt
#define IFE_INT_DEC2_ERR               0x00000004  ///< path1 decode error interrupt
#define IFE_INT_LLEND                  0x00000008  ///< LL end interrupt
#define IFE_INT_LLERR                  0x00000010  ///< LL error  interrupt
#define IFE_INT_LLERR2                 0x00000020  ///< LL error2 interrupt
#define IFE_INT_LLJOBEND               0x00000040  ///< LL job end interrupt
#define IFE_INT_BUFOVFL                0x00000080  ///< buffer overflow interrupt
#define IFE_INT_RING_BUF_ERR           0x00000100  ///< ring buffer error interrupt
#define IFE_INT_FRAME_ERR              0x00000200  ///< frame error interrupt (for direct mode)
//#define IFE_INT_DEC3_ERR               0x00000400  ///< path2 decode error interrupt
//#define IFE_INT_RING_BUF_ERR2          0x00000800  ///< ring buffer error 2 interrupt
#define IFE_INT_SIE_FRM_START          0x00001000  ///< SIE frame start interrupt
#define IFE_INT_SIE2_FRM_START         0x00002000  ///< SIE2 frame start interrupt
#define IFE_INT_FRM_START              0x00004000  ///< IFE frame start interrupt
#define IFE_INT_NN_ISP_P0_SET_SLICE_READY  0x00010000  ///< IFE NN_ISP_P0_SET_SLICE_READY
#define IFE_INT_NN_ISP_P0_SET_SLICE_CLEAR  0x00020000  ///< IFE NN_ISP_P0_SET_SLICE_CLEAR
#define IFE_INT_NN_ISP_P1_SET_SLICE_READY  0x00040000  ///< IFE NN_ISP_P1_SET_SLICE_READY
#define IFE_INT_NN_ISP_P1_SET_SLICE_CLEAR  0x00080000  ///< IFE NN_ISP_P1_SET_SLICE_CLEAR




//#define IFE_INT_ROWDEFFAIL             0x00000004  ///< removed in NT96680
#define IFE_INT_ALL                    0x000f5fff  ///< all interrupts
//@}

typedef enum {
	IFE_PIPE_LEFT    = 0,
	IFE_PIPE_RIGHT   = 1,
	IFE_PIPE_UNKNOWN = 2,
}IFE_DUAL_IDX;

/**
    IFE Function Selection.

    IFE Function Selection.
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_OPMODE_D2D         = 0,        ///< D2D
	IFE_OPMODE_IPP         = 1,        ///< IPP mode
	IFE_OPMODE_ALL_DIRECT  = 2,        ///< All direct
	ENUM_DUMMY4WORD(IFE_OPMODE)
} IFE_OPMODE;

/**
    IFE RDE mode of parameters
*/
//@{
typedef enum {
	RDE_PARMS_NORMAL_MODE    = 0,       
	RDE_PARMS_AGGRESIVE_MODE    = 1,       
	ENUM_DUMMY4WORD(IFE_RDE_ADJMODE_SEL)
} IFE_RDE_ADJMODE_SEL;

/**
    IFE Input Selection.

    IFE Input Selection.
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_RAW_IN         = 0,        ///< IFE input Bayer
	IFE_YUV_IN      = 1,        ///< IFE input YUV, the path only through WDR
	ENUM_DUMMY4WORD(IFE_IN_FMT)
} IFE_IN_FMT;

/**
    IFE Output Selection.

    IFE Output Selection.
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_WDR_OUT         = 0,        ///< WDR out
	IFE_SUBISP_OUT      = 1,        ///< SUBISP out
	ENUM_DUMMY4WORD(IFE_OUT_SEL)
} IFE_OUT_SEL;


//@}

/**
    IFE Register type.

    IFE Register type.
\n  Used for ife_change_all()
*/
//@{
typedef enum {
	IFE_ALL_REG         = 0,    ///< Set all registers
	IFE_FD_LATCHED      = 1,    ///< Update VD-latched registers only
	IFE_NON_FD_LATCHED  = 2,    ///< Updat Non-VD-latched registers only
	ENUM_DUMMY4WORD(IFE_REGTYPE)
} IFE_REGTYPE;
//@}

/**
    IFE Function Control.

    Set IFE Function Control
\n  Used for ife_set_mode()
*/
//@{
/*
typedef struct _IFE_CONTROL {

    BOOL            b_decode_en;      ///< RDE decode enable

    BOOL            b_nrs_en;          ///< enable of NRS enable
    BOOL            b_fcurve_en;       ///< enable of Fcurve enable
    BOOL            b_fcgain_en;       ///< enable of F_color gain enable
    BOOL            b_fusion_en;       ///< enable of Fusion (sensor HDR) enable
    BOOL            b_scompression_en; ///< enable of S compression enable

    BOOL            b_outl_en;        ///< enable of outlier filter
    BOOL            b_filter_en;      ///< enable of IFE filter
    BOOL            b_cen_mod_en;      ///< enable of center modify
    BOOL            b_rb_fill_en;      ///< enable RB_NRFill (only effective in RGBIr)
    BOOL            b_cgain_en;       ///< enable of color gain
    BOOL            b_vig_en;         ///< enable of vignette
    BOOL            b_gbal_en;        ///< enable of Gbalance

    BOOL            b_mirror_en;      ///< enable Mirror
} IFE_CONTROL;
*/
//@}

#if 0//removed in NT96680
/**
    IFE Filter mode Selection.

    Set IFE filter mode
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_NORMAL_9x9 = 0,           ///< standard filter in 9x9 area
	IFE_SONLY_9x9  = 1,           ///< spatial filter only in 9x9 area
	ENUM_DUMMY4WORD(IFE_FILTMODE)
} IFE_FILTMODE;
//@}
#endif

/**
    IFE binning Selection.

    binning level selection
*/
//@{
typedef enum {
	IFE_BINNING_DISABLE = 0,        ///< Disable binning
	IFE_BINNINGX2       = 1,        ///< binning x2
	IFE_BINNINGX4       = 2,        ///< binning x4
	IFE_BINNINGX8       = 3,        ///< binning x8
	IFE_BINNINGX16      = 4,        ///< binning x16
	IFE_BINNINGX32      = 5,        ///< binning x32
	IFE_BINNINGX64      = 6,        ///< binning x64
	IFE_BINNINGX128     = 7,        ///< binning x128
	ENUM_DUMMY4WORD(IFE_BINNSEL)
} IFE_BINNSEL;
//@}

/**
    IFE CFA Pattern Selection.

    Set IFE start pixel of the CFA pattern
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_SONLY_3x3 = 0,           ///< spatial filter only in 3x3 area
	IFE_SONLY_5x5 = 1,           ///< spatial filter only in 5x5 area
	IFE_SONLY_7x7 = 2,           ///< spatial filter only in 7x7 area
	IFE_SONLY_9x9 = 3,           ///< spatial filter only in 9x9 area
	ENUM_DUMMY4WORD(IFE_SONLYLEN)
} IFE_SONLYLEN;
//@}


/**
    IFE Wait Mode Selection.

    IFE Wait Mode Selection.
\n  Used for ife_set_mode()
*/
//@{
typedef enum {
	IFE_WAIT    = 0,                ///< Wait for frame end
	IFE_NOWAIT  = 1,                ///< Do not wait for frame end
	ENUM_DUMMY4WORD(IFE_WAITMODE)
} IFE_WAITMODE;
//@}


/**
    IFE Bit Depth.

    Set IFE input/output bitdepth
*/
//@{
typedef enum {
	IFE_8BIT    = 0,                ///<  8bit data
	IFE_10BIT   = 1,                ///< 10bit data
	IFE_12BIT   = 2,                ///< 12bit data
	IFE_16BIT   = 3,                ///< 16bit data
	ENUM_DUMMY4WORD(IFE_BITDEPTH)
} IFE_BITDEPTH;
//@}

/**
    IFE color gain range selection

    Select IFE color gain range
*/
//@{
typedef enum {
	IFE_CGRANGE_2_8 = 0,    ///< 8bit decimal, set 256 for 1x
	IFE_CGRANGE_3_7 = 1,    ///< 7bit decimal, set 128 for 1x
	ENUM_DUMMY4WORD(IFE_CGRANGE)
} IFE_CGRANGE;
//@}

/**
    IFE fusion color gain range selection

    Select IFE fusion color gain range
*/
//@{
typedef enum {
	IFE_FCGRANGE_8_8 = 0,    ///< 8bit decimal, set 256 for 1x
	IFE_FCGRANGE_9_7 = 1,    ///< 7bit decimal, set 128 for 1x
	ENUM_DUMMY4WORD(IFE_FCGRANGE)
} IFE_FCGRANGE;
//@}


/**
    IFE input Bayer data start CFA selection

    Select intput Bayer raw data start channel - R, Gr, Gb, B.
*/
//@{
typedef enum {
	IFE_PAT0  = 0,  ///<
	IFE_PAT1  = 1,  ///<
	IFE_PAT2  = 2,  ///<
	IFE_PAT3  = 3,  ///<
	IFE_PAT4  = 4,  ///<
	IFE_PAT5  = 5,  ///<
	IFE_PAT6  = 6,  ///<
	IFE_PAT7  = 7,  ///<
	ENUM_DUMMY4WORD(IFE_CFASEL)
} IFE_CFASEL;
//@}

/**
    IFE input Bayer format selection

    Select intput Bayer format
*/
//@{
typedef enum {
	IFE_BAYER_RGGB  = 0, ///< Traditional RGGB format
	IFE_BAYER_RGBIR = 1, ///< RGBIr format
	ENUM_DUMMY4WORD(IFE_BAYERFMTSEL)
} IFE_BAYERFMTSEL;
//@}

/**
    IFE burst length define
*/
//@{
typedef enum {
	IFE_IN_BURST_64W    = 0,       ///< burst length 64 word
	IFE_IN_BURST_32W    = 1,       ///< burst length 32 word
	ENUM_DUMMY4WORD(IFE_IN_BURST_SEL)
} IFE_IN_BURST_SEL;

/**
    IFE output burst length define
*/
//@{
typedef enum {
	IFE_OUT_BURST_32W    = 0,       ///< burst length 32 word
	IFE_OUT_BURST_16W    = 1,       //removed in NT96680
	ENUM_DUMMY4WORD(IFE_OUT_BURST_SEL)
} IFE_OUT_BURST_SEL;


/**
    IFE SET ENUM
*/
//@{
typedef enum {
	IFE_SET_INFO,
	IFE_SET_RING_BUF,
	IFE_SET_MIRROR,
	IFE_SET_RDE,
	IFE_SET_NRS,
	IFE_SET_FUSION,
	IFE_SET_FCURVE,
	IFE_SET_OUTL,
	IFE_SET_FILTER,
	IFE_SET_CGAIN,
	IFE_SET_VIG,
	IFE_SET_VIG_CENTER,
	IFE_SET_GBAL,
	IFE_SET_ALL,
	ENUM_DUMMY4WORD(IFE_UPDATE_SEL)
} IFE_SET_SEL;

typedef struct _IFE_ENG_FPN_PARAM {
	BOOL   fpn_en;
	UINT8  fpn_cgain_range;
	UINT16  fpn_cgain_r[2];
	UINT16  fpn_cgain_gr[2];
	UINT16  fpn_cgain_gb[2];
	UINT16  fpn_cgain_b[2];
	UINT16  fpn_cgain_ir[2];
	UINT16  fpn_cofs_r[2];
	UINT16  fpn_cofs_gr[2];
	UINT16  fpn_cofs_gb[2];
	UINT16  fpn_cofs_b[2];
	UINT16  fpn_cofs_ir[2];
} IFE_ENG_FPN_PARAM;

typedef struct _IFE_SIE_HANDSHAKE {
	BOOL   input_combine_en;
	UINT8 src_clk_sel_fifo0;
	UINT8 src_clk_sel_fifo1;
	UINT16 in_buf_write_width0;
	UINT16 in_buf_write_width1;
	UINT16 in_buf_crop_st0;
	UINT16 in_buf_crop_st1;
	UINT16 in_buf_crop_width0;
	UINT16 in_buf_crop_width1;
} IFE_SIE_HANDSHAKE;

typedef struct _IFE_ENG_DUAL_PARAM {
	UINT16  start_offset;
	UINT16  left_overlap;
	UINT16  right_overlap;
} IFE_ENG_DUAL_PARAM;

typedef struct _IFE_ENG_SINGLE_OUT_PARAM {
	BOOL  single_out_0_en;
	BOOL  single_out_1_en;
	BOOL  single_out_2_en;
	BOOL  single_out_indep_va_en;	
	BOOL  out_mode;
} IFE_ENG_SIGLE_OUT_PARAM;




typedef struct _IFE_BURST_LENGTH {
	IFE_IN_BURST_SEL   burst_len_input;  ///< Input burst length
	IFE_OUT_BURST_SEL   burst_len_output; ///< Output burst length
} IFE_BURST_LENGTH;
//@}

/**
    Struct IFE open object.

    ISR callback function
*/
//@{
typedef struct _IFE_OPENOBJ {
	void (*FP_IFEISR_CB)(UINT32 ui_int_status); ///< isr callback function
	UINT32 ui_ife_clock_sel; ///< TBD(240 is tested by now)
} IFE_OPENOBJ;
//@}

/**
    Struct IFE stripe

    Parameters of stripe
*/
//@{
typedef struct _IFE_STRIPE {
	//UINT32 hn;            ///< range filter threshold
	//UINT32 hl;           ///< range filter threshold adjustment
	//UINT32 hm;
	UINT32 ife_stripe_w[8];
	UINT32 ife_stripe_num;
	UINT32 ife_stripe_overlap;
	UINT32 input_format;
} IFE_STRIPE;
//@}


/**
    Struct IFE stripe limit

    Parameters of stripe
*/
//@{
typedef struct _IFE_STRIPE_LIMIT {
	UINT32 stripe_align;
	UINT32 stripe_overlap;
	UINT32 stripe_min;
} IFE_STRIPE_LIMIT;
//@}


/**
    Struct IFE stripe

    Parameters of stripe
*/
//@{
typedef struct _IFE_RING_BUF {
	BOOL   dmaloop_ctrl;    ///< dma loop control
	BOOL   dmaloop_en;      ///< dma loop enable
	UINT32 dmaloop_line;    ///< dma loop dram line number
} IFE_RING_BUF;

/**
    Struct IFE sync_dich

    Parameters of stripe
*/
//@{
typedef struct _IFE_DICH_SYNC_BUF {
	UINT32 dich_line_ctrl;
	BOOL   dma_sync_dich_line_dis;
	BOOL   dma1_wait_sie2_start_dis;
	BOOL   dma2_wait_sie3_start_dis;
} IFE_DICH_SYNC_BUF;


//@}

/**
    Struct IFE RDE.

    Parameters of RDE
*/
//@{
typedef struct _IFE_RDESET {
	UINT8           encode_rate;               ///<  RDE encode rate of bitstream per segment
#if (defined(_NVT_EMULATION_) == ON)
	BOOL            b_degamma_en;               ///<  RDE Degamma enable
	BOOL            b_dithering_en;             ///<  RDE Dithering enable
	BOOL            dither_reset;              ///<  RDE Dithering reset initial random seed

	UINT8           *p_dct_qtable_inx;        ///<  RDE the index for DCT decode Q table
	//UINT16          *p_degamma_table;         ///<  RDE degamma table
	UINT8           rand1_init1;              ///<  RDE Random sequence init setting
	UINT16          rand1_init2;              ///<  RDE Random sequence init setting
	UINT8           rand2_init1;              ///<  RDE Random sequence init setting
	UINT16          rand2_init2;              ///<  RDE Random sequence init setting
	//UINT8           rand3_init1;              ///<  RDE Random sequence init setting
	//UINT16          rand3_init2;              ///<  RDE Random sequence init setting
	//UINT8           *p_dpcm_table_lbn;
	//UINT8           *p_dpcm_table_vlc;
	//BOOL            b_index_en;
#endif
} IFE_RDESET;
//@}


/**
    Struct IFE NRS.

    Parameters of NRS
*/
//@{
typedef struct _IFE_NRSSET {
	UINT16          *ife_f_nrs0_str;              ///<  NRS0 strength
	//UINT16          *ife_f_nrs1_str;              ///<  NRS1 strength
} IFE_NRSSET;
//@}

/**
    Struct IFE Fcurve.

    Parameters of Fcurve
*/
//@{
typedef struct _IFE_FCURVESET {
	UINT8            y_mean_select;     ///<  Fcurve Y Mean Method selection
	UINT8            yv_weight;         ///<  Fcurve Y mean and V weight
	UINT8            ev_format;         ///<  Fcurve ev format, 4->4EV format. 3->3 EV format ... so on
	UINT8           *p_y_weight_lut;    ///<  Fcurve Y weight
	UINT32          *p_left_lut;        ///<  Fcurve left part, IFE_F_FCURVE_L
	UINT32          *p_right_lut;       ///<  Fcurve right part, IFE_F_FCURVE_R
	//UINT32          *p_end_lut;         ///<  Fcurve end part, IFE_F_FCURVE_END
} IFE_FCURVESET;
//@}

/**
    Struct IFE Fusion.

    Parameters of Fusion
*/
//@{
typedef struct _IFE_FUSIONSET {
	UINT16           debug_mode;
	UINT8            y_mean_sel;
	UINT8            nor_blendcur_sel;
	UINT8            diff_blendcur_sel;
	UINT8            mode;
	UINT16           ev_ratio;
	//UINT16           ev_ratio_1;

	UINT16           mc_lum_th;
	UINT8            mc_diff_ratio;
	UINT8            *p_mc_diff_w;
	//UINT8            *p_mc_pos_diff_w;
	//UINT8            *p_mc_neg_diff_w;
	UINT8            mc_diff_lumth_diff_w;


	UINT16           long_nor_blendcur_knee;
	UINT16           long_nor_blendcur_range;
	UINT16           long_nor_blendcur_slope;
	UINT16           short_nor_blendcur_knee;
	UINT16           short_nor_blendcur_range;
	UINT16           short_nor_blendcur_slope;

	//UINT16           very_short_nor_blendcur_knee;
	//UINT16           very_short_nor_blendcur_range;
	//UINT16           very_short_nor_blendcur_slope;

	UINT16           long_diff_blendcur_knee;
	UINT16           long_diff_blendcur_range;
	UINT16           long_diff_blendcur_slope;
	UINT16           short_diff_blendcur_knee;
	UINT16           short_diff_blendcur_range;
	UINT16           short_diff_blendcur_slope;

	//UINT16           very_short_diff_blendcur_knee;
	//UINT16           very_short_diff_blendcur_range;
	//UINT16           very_short_diff_blendcur_slope;

	UINT16           *p_dark_sat_reduce_th;
	UINT8            *p_dark_sat_reduce_step;
	UINT8            *p_dark_sat_reduce_lowbound;

} IFE_FUSIONSET;
//@}


/**
    Struct IFE Range Filter Setting A.

    Parameters of range filter
*/
//@{
typedef struct _IFE_RANGESETA {
	UINT16 *p_rngth_c0;            ///< range filter threshold
	UINT16 *p_rnglut_c0;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c1;            ///< range filter threshold
	UINT16 *p_rnglut_c1;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c2;            ///< range filter threshold
	UINT16 *p_rnglut_c2;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c3;            ///< range filter threshold
	UINT16 *p_rnglut_c3;           ///< range filter threshold adjustment
} IFE_RANGESETA;
//@}

/**
    Struct IFE Range Filter Advanced Parameter.

    Parameters of range filter
*/
//@{
typedef struct _IFE_CENMODSET {
	UINT32 bilat_cen_sel;  ///< 0: DPC (52x algothrim), 1: Bilateral, bypass center pixel
	UINT32 bilat_th1;      ///< 3x3 bilateral filter threshold 1
	UINT32 bilat_th2;      ///< 3x3 bilateral filter threshold 2
} IFE_CENMODSET;
//@}

/**
    Struct IFE Range Filter Setting B.

    Parameters of range filter
*/
//@{
typedef struct _IFE_RANGESETB {
	UINT16 *p_rngth_c0;            ///< range filter threshold
	UINT16 *p_rnglut_c0;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c1;            ///< range filter threshold
	UINT16 *p_rnglut_c1;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c2;            ///< range filter threshold
	UINT16 *p_rnglut_c2;           ///< range filter threshold adjustment
	UINT16 *p_rngth_c3;            ///< range filter threshold
	UINT16 *p_rnglut_c3;           ///< range filter threshold adjustment
} IFE_RANGESETB;
//@}

/**
    Struct IFE Clamp and Weighting.

    Parameters of clamp and weight
*/
//@{
typedef struct _IFE_CLAMPSET {
	UINT16 clamp_th;             ///< clamp threshold
	UINT8  clamp_mul;            ///< weighting multiplier
	UINT16 clamp_dlt;            ///< clamp threshold adjustment
} IFE_CLAMPSET;
//@}

/**
    Struct IFE Digital Gain.

    Parameters of digatal gain
*/
//@{
typedef struct _IFE_DGAINSET {
	UINT32      dgain;          ///< digatal gain, for all channnel
} IFE_DGAINSET;


/**
    Struct IFE Color Gain.

    Parameters of color gain
*/
//@{
typedef struct _IFE_CGAINSET {
	BOOL        b_cgain_inv;         ///< color gain invert
	BOOL        b_cgain_hinv;        ///< color gain H-invert
	IFE_CGRANGE cgain_range;         ///< select color gain decimal range
	UINT32      cgain_mask;          ///< Cloro gain mask
	UINT32      p_cgain[5];          ///< color gain, always R/Gr/Gb/B order
	UINT32      p_cofs[5];           ///< color offset, always R/Gr/Gb/B order
} IFE_CGAINSET;
//@}

/**
    Struct IFE Fusion Color Gain.

    Parameters of fusion color gain
*/
//@{
typedef struct _IFE_FCGAINSET {
	IFE_FCGRANGE fcgain_range;                   ///< select Fusion color gain decimal range
	UINT16      p_fusion_cgain_path0[5];          ///< color gain, always R/Gr/Gb/B order
	UINT16      p_fusion_cgain_path1[5];          ///< color gain, always R/Gr/Gb/B order
	//UINT16      p_fusion_cgain_path2[5];          ///< color gain, always R/Gr/Gb/B order
	UINT16      p_fusion_cofs_path0[5];           ///< color offset, always R/Gr/Gb/B order
	UINT16      p_fusion_cofs_path1[5];           ///< color offset, always R/Gr/Gb/B order
	//UINT16      p_fusion_cofs_path2[5];           ///< color offset, always R/Gr/Gb/B order
} IFE_FCGAINSET;
//@}

/**
    Struct IFE Outlier Filter.

    Parameters of outlier filter
*/
//@{
typedef struct _IFE_OUTLSET {
	UINT32          *p_bri_th;    ///< outlier threshold for bright points
	UINT32          *p_dark_th;   ///< outlier threshold for dark points
	UINT32          *p_outl_cnt;      ///< outlier counter
	UINT32          outl_w;     ///< outlier weighting
	UINT32          dark_ofs;    ///< outlier dark offset
	UINT32          bright_ofs;  ///< outlier bright offset

	UINT8           ord_range_bri;
	UINT8           ord_range_dark;
	UINT16          ord_protect_th;
	UINT8           ord_blend_w;
	UINT8           outl_comp_mode;
	//BOOL            outl_edge_smooth_en;
	//UINT8           outl_extre_idx;
	UINT8           *p_ord_bri_w;
	UINT8           *p_ord_dark_w;

	UINT8           outl_rgbir_rb_w;
	UINT8           ord_rgbir_rb_w;

	//BOOL            OutlCrsChanEn;  ///< remove in NT 98520
	//IFE_OUTL_SEL    OutlOp;         ///< outlier option for neighbour condition

} IFE_OUTLSET;
//@}

/**
    IFE structure - IFE RB Fill.
*/
//@{
typedef struct _IFE_RBFill_PARAM {
	UINT8 *p_rbfill_rbluma;  ///<
	UINT8 *p_rbfill_rbratio; ///<
	UINT8  rbfill_ratio_mode;
} IFE_RBFill_PARAM;
//@}

/**
    IFE structure - IFE VIG.
*/
//@{
typedef struct _IFE_VIG_PARAM {
	INT32 *p_vig_x;        ///< Legal range : -8192~8191,VIG x center of 4 channel
	INT32 *p_vig_y;        ///< Legal range : -8192~8191,VIG y center of 4 channel
	UINT32 vig_dist_th;       ///< Legal range : 0~1023,VIG disabled area
	UINT8  vig_tab_gain; ///< Legal range : 0~3,VIG gain factor
	BOOL   b_vig_dither_en; ///< VIG dithering enable
	BOOL   b_vig_dither_rst;///< VIG dithering reset, HW clear
	UINT32 vig_xdiv;    ///< Legal range : 0~4095,VIG distance gain factor in x direction. unsigned 6.6
	UINT32 vig_ydiv;    ///< Legal range : 0~4095,VIG distance gain factor in y direction. unsigned 6.6
	UINT16 *p_vig_lut_c0; ///< Legal range : 0~1023,VIG LUT of channel 0
	UINT16 *p_vig_lut_c1; ///< Legal range : 0~1023,VIG LUT of channel 1
	UINT16 *p_vig_lut_c2; ///< Legal range : 0~1023,VIG LUT of channel 2
	UINT16 *p_vig_lut_c3; ///< Legal range : 0~1023,VIG LUT of channel 3

	BOOL    vig_fisheye_gain_en;
	UINT8   vig_fisheye_slope;
	UINT32  vig_fisheye_radius;
} IFE_VIG_PARAM;
//@}

/**
    IFE structure - IFE Gbal.
*/
//@{
typedef struct _IFE_GBAL_PARAM {
	BOOL     b_protect_en;
	UINT16   diff_th_str;
	UINT8    diff_w_max;
	UINT16   edge_protect_th1;
	UINT16   edge_protect_th0;
	UINT8    edge_w_max;
	UINT8    edge_w_min;
	UINT8    *p_gbal_ofs;

	UINT16    str_luma_low_bnd;
	UINT16    edge_luma_low_bnd;
} IFE_GBAL_PARAM;
//@}
#if 0

/**
    IFE structure - IFE NlmKer.
*/
//@{
typedef struct _IFE_NLM_KER {
	BOOL  b_nlm_ker_en;    ///< enable bilateral/NLM kernel modification
	UINT8 ui_ker_slope0;  ///< legal range : 0~127, kernel slope1 = ui_ker_slope0/32
	UINT8 ui_ker_slope1;  ///< legal range : 0~127, kernel slope1 = ui_ker_slope1/4
	BOOL  b_locw_en;      ///< enable bilateral/NLM local weighting modification
	UINT8 ui_bilat_w_d1;   ///< legal range : 0~15, bilateral weighting 1
	UINT8 ui_bilat_w_d2;   ///< legal range : 0~15, bilateral weighting 2
	UINT8 *pui_ker_radius;///< kernel radius
} IFE_NLM_KER;
//@}

/**
    IFE structure - IFE NlmKer Weighting LUT.
*/
//@{
typedef struct _IFE_NLM_LUT {
	UINT8 *pui_mwth;     ///<
	UINT8 *pui_bilat_wa;  ///< legal range : 0~15, weighting table A
	UINT8 *pui_bilat_wb;  ///< legal range : 0~15, weighting table B
	UINT8 *pui_bilat_wc;  ///< legal range : 0~15, weighting table C
	UINT8 *pui_bilat_wbl; ///< weighting table BL
	UINT8 *pui_bilat_wbm; ///< weighting table BM
	UINT8 *pui_bilat_wbh; ///< weighting table BH
} IFE_NLM_LUT;
//@}

/**
    Struct IFE Filter Parameters.

    IFE filter detailed parameters.
*/
//@{
typedef struct _IFE_FILT_PARA {
	IFE_CFASEL      cfa_pat;         ///< CFA pattern if RAW format selected
	//IFE_FILTMODE    filt_mode;       ///< filter mode select
	//BOOL            b_row_def_en;      ///< enable of row defect concealment
	BOOL            b_outl_en;        ///< enable of outlier filter
	BOOL            b_filter_en;      ///< enable of IFE filter
	//BOOL            b_crv_map_en;      ///< enable of curve mapping
	BOOL            b_cgain_en;       ///< enable of color gain
	BOOL            b_vig_en;         ///< enable of vignette
	BOOL            b_gbal_en;        ///< enable of gbalance
	BOOL            b_cen_mod_en;      ///< enable of center modify

	UINT32          ui_bayer_format;  ///< 0: bayer, 1: RGBIr, 2: RGBIr
	//BOOL            b_flip_en;        ///< enable of flip
	UINT32          *p_spatial_weight;    ///< spatial weighting
	IFE_RANGESETA   rng_thA;         ///< range filter threshold setting for filterA
	IFE_RANGESETB   rng_thB;         ///< range filter threshold setting for filterB
	IFE_CENMODSET   center_mod_set;

	//IFE_NLM_KER     nlm_ker_set;      ///< NLM kernel setting
	//IFE_NLM_LUT     nlm_lut_set;      ///< NLM kernel look up tables
	//UINT32          ui_bilatW;       ///< range filter A and B weighting
	UINT32          ui_rthW;         ///< range filter and spatial filter weighting
	IFE_BINNSEL     binn;           ///< image binning
	BOOL            bSOnly_en;       ///< spatial only enable
	IFE_SONLYLEN    SOnly_len;       ///< filter length of spatial only
	IFE_CLAMPSET    clamp_set;     ///< clamp and weighting setting
	IFE_CGAINSET    cgain_set;       ///< color gain setting
	IFE_OUTLSET     outl_set;        ///< outlier filter setting
	IFE_RBFill_PARAM rbfill_set;     ///< R-B filled setting
	//IFE_ROWDEF_FACT row_def_fact;     //removed in NT96680
	UINT32          ui_row_def_st_idx;  ///< start index of row defect table
	UINT32          ui_row_defVIni;   ///< vertical start of cropped window
	UINT32          *pui_row_def_tbl;  ///< table of row defect concealment
	IFE_VIG_PARAM   vig_set;         ///< vignette setting
	IFE_GBAL_PARAM  gbal_set;        ///< gbalance setting
	//UINT8           ui_bit_dither;    ///< select bit dithering while truncation
	UINT32          ui_repeat;       ///< repeat times
} IFE_FILT_PARA;
#endif
//@}

/**
    Struct IFE size Parameters.

    IFE Size related parameters.
*/
//@{
typedef struct _IFE_SIZE_PARA {
	UINT32 width;               ///< image width
	UINT32 height;              ///< image height
	UINT32 ui_ofsi_1;              ///< image input lineoffset
	UINT32 ui_ofsi_2;              ///< image input lineoffset
	UINT32 ui_ofsi_3;              ///< image input lineoffset
	UINT32 ui_ofso;                ///< image output lineoffset
} IFE_SIZE_PARA;
//@}

/**
    Struct IFE ROI Parameters.

    IFE Address, Cropping detailed parameters.
*/
//@{
typedef struct _IFE_ROI_PARA {
	IFE_SIZE_PARA   size;            ///< input/output size para.
	UINT32          ui_in_addr_1;      ///< input starting address
	UINT32          ui_in_addr_2;      ///< input starting address
	UINT32          ui_in_addr_3;      ///< input starting address
	UINT32          ui_out_addr;       ///< output starting address
	UINT32          crop_width;     ///< image crop width
	UINT32          crop_height;    ///< image crop height
	UINT32          crop_hpos;      ///< image horizontal crop start pixel
	UINT32          crop_vpos;      ///< image virtical crop start pixel
	IFE_BITDEPTH    in_bit;           ///< input bit select
	IFE_BITDEPTH    out_bit;          ///< out bit select
	//BOOL            b_flip_en;        ///< enable of flip
} IFE_ROI_PARA;
//@}

typedef struct {
	UINT32 h_fact;    // horizontal factor
	UINT32 v_fact;    // vertical factor
} IFE_ENG_HV_FACTOR;

typedef struct {
	UINT32 wdr_subimg_hori_div; //horizontal division ratio
	UINT32 wdr_subimg_vert_div; //vertical  division ratio
} IFE_ENG_WDR_SUBIMG_DIV;

typedef struct {
	BOOL    fbc_en;//forward brigness compensation
	UINT8   fbc_ratio;
	UINT16  fbc_th[6];
} IFE_ENG_WDR_FBC;

typedef struct {
	BOOL subimg_limit_en;
	UINT8 subimg_st_blk;
	UINT8 subimg_end_blk;
} IFE_ENG_WDR_DUAL;

typedef struct {
	UINT32 wdr_subimg_width;        // Wdr subimg width
	UINT32 wdr_subimg_height;       // Wdr subimg height
	IFE_ENG_HV_FACTOR subin_scal_fact;  // Wdr subimg scaling factor
	IFE_ENG_WDR_SUBIMG_DIV  wdr_subimg_div; //wdr subimage division ratio
} IFE_ENG_WDR_SUBIMG;

typedef enum {
	IFE_ENG_WDR_ROUNDING        = 0,  // rounding
	IFE_ENG_WDR_HALFTONING      = 1,  // halftone rounding
	IFE_ENG_WDR_RANDOMLSB       = 2,  // random rounding
} IFE_ENG_WDR_DITHER_MODE;

typedef struct {
	BOOL wdr_dithering_en;//WDR output dithering switch
	BOOL wdr_rand_rst;
	IFE_ENG_WDR_DITHER_MODE wdr_rand_sel;
} IFE_ENG_WDR_DITHER;

typedef enum {
	IFE_ENG_INPUT_BLD_3x3_Y  = 0,   // use Y from bayer 3x3 as the input blending source selection
	IFE_ENG_INPUT_BLD_GMEAN  = 1,   // use Gmean from bayer 3x3 as the input blending source selection
	IFE_ENG_INPUT_BLD_BAYER  = 2,   // use Bayer as the input blending source selection
} IFE_ENG_WDR_INPUT_BLDSEL;

typedef struct {
	IFE_ENG_WDR_INPUT_BLDSEL wdr_bld_sel;
	//UINT32 wdr_bld_wt;
	UINT32 wdr_input_yv_bld_lut[9];
	UINT8 *p_inblend_lut;   // 17 entries, IFE_ENG_WDR_IN_BLEND_NUM
} IFE_ENG_WDR_IN_BLD;

typedef struct {
	INT16  wdr_coeff[4];		// 4 entries, IFE_ENG_WDR_COEFF_NUM
	UINT32 strength;
	//UINT32 contrast;//alex, removed in 530
	BOOL   wdr_mode;
	BOOL   wdr_anti_halo_opt;
	UINT8  wdr_halo_ratio;
	UINT8  wdr_halo_slope;
	UINT16 wdr_b2p_var;
} IFE_ENG_WDR_STRENGTH;

typedef struct {
	BOOL gainctrl_en;
	UINT32 max_gain;
	UINT32 min_gain;
} IFE_ENG_WDR_GAINCTRL;

typedef struct {
	UINT32 sat_th;
	UINT32 sat_wt_low;
	UINT32 sat_delta;
} IFE_ENG_WDR_SAT_REDUCT;

typedef enum {
	IFE_ENG_HIST_BEFORE_WDR      = 0,    // histogram statistics before wdr algorithm
	IFE_ENG_HIST_AFTER_WDR       = 1,    // histogram statistics after wdr algorithm
} IFE_ENG_WDR_HIST_SEL;

typedef struct {
	BOOL histogram_enable;
	IFE_ENG_WDR_HIST_SEL hist_sel;
	UINT32 hist_step_h;
	UINT32 hist_step_v;
	//UINT8  hist_shift;
	//UINT8 hist_init_hit;
} IFE_ENG_WDR_HIST;

typedef struct {
	IFE_ENG_WDR_DITHER dither;//
	IFE_ENG_WDR_IN_BLD wdr_input_bld;//
	IFE_ENG_WDR_STRENGTH wdr_strength;
	IFE_ENG_WDR_GAINCTRL wdr_gainctrl;//
	IFE_ENG_WDR_SAT_REDUCT wdr_sat_reduct;//
	IFE_ENG_WDR_FBC  wdr_fbc; //
	UINT32 wdr_gain_port_str;
} IFE_ENG_WDR_PARAM;

typedef struct {
	BOOL tone_curve_enable;
	UINT16 *p_tone_r_buf;
	UINT16 *p_tone_l_buf;
} IFE_ENG_TONE_CUR_PARAM;

typedef struct {
	BOOL outbld_enable;
	UINT16 *p_outbld_r_buf;
	UINT16 *p_outbld_l_buf;
} IFE_ENG_WDR_OUT_BLD_PARAM;

typedef struct {
	UINT16 *p_hist_stcs;    // 128 entries, IFE_ENG_WDR_HIST_BIN_NUM
} IFE_ENG_WDR_HIST_RESULT;

typedef enum {
	/* 3 channel */
	IFE_ENG_FMT_Y_PACK_UV422 = 0,   // 422 format y planar UV pack UVUVUVUVUV.....
	IFE_ENG_FMT_Y_PACK_UV420,       // 420 format y planar UV pack UVUVUVUVUV.....
	IFE_ENG_FMT_Y_8BIT,         // 444 format y planar UV pack UVUVUVUVUV.....
	IFE_ENG_FMT_Y_UNKNOWN,      // unknown
} IFE_YUV_IN_FMT;

typedef enum {
	/* 3 channel */
	IFE_OUT_YUV422 = 0,
	IFE_OUT_YUV420,
} IFE_YUV_OUT_FMT;

typedef enum {
	IFE_ENG_FULL_YUV        = 0,    // Full range YUV input
	IFE_ENG_BT601_YUV       = 1,    // BT601 YUV input
	IFE_ENG_BT709_YUV       = 2,    // BT709 YUV input
	IFE_ENG_UNKNOWN_YUV     = 3,    // unknown
} IFE_ENG_YUV2RGB_FMT;

typedef struct {
	IFE_YUV_IN_FMT yuv_in_fmt;
	BOOL yuv2rgb_enable;
	IFE_ENG_YUV2RGB_FMT yuv2rgb_fmt;
	BOOL yuv_in_random_lsb_enable;
	BOOL yuv_in_random_seed_rst;
	BOOL yuv_in_uv_lpf_enable;
	BOOL yuv_in_io_stop;
} IFE_ENG_YUV_IN_PARAM;

typedef enum {
	IFE_VA_FLTR_MIRROR  = 0,   ///<
	IFE_VA_FLTR_INVERSE = 1,   ///<
	ENUM_DUMMY4WORD(IFE_VA_FLTR_SYMM_SEL)
} IFE_VA_FLTR_SYMM_SEL;

typedef enum {
	IFE_VA_FLTR_SIZE_1  = 0,   ///<
	IFE_VA_FLTR_SIZE_3  = 1,   ///<
	IFE_VA_FLTR_SIZE_5  = 2,   ///<
	IFE_VA_FLTR_SIZE_7  = 3,   ///<
	ENUM_DUMMY4WORD(IFE_VA_FLTR_SIZE_SEL)
} IFE_VA_FLTR_SIZE_SEL;


typedef struct {
	INT16                   tap_a;     ///<
	INT16                   tap_b;      ///<
	INT16                   tap_c;      ///<
	INT16                   tap_d;      ///<
	INT16                   tap_iir1_e;      ///<
	INT16                   tap_iir1_f;      ///<

	INT16                   tap_iir2_a;      ///<
	INT16                   tap_iir2_b;      ///<
	INT16                   tap_iir2_e;      ///<
	INT16                   tap_iir2_f;      ///<

	INT16                   tap_iir3_a;      ///<
	INT16                   tap_iir3_b;      ///<
	INT16                   tap_iir3_e;      ///<
	INT16                   tap_iir3_f;      ///<

	IFE_VA_FLTR_SYMM_SEL    filt_symm;   ///<
	IFE_VA_FLTR_SYMM_SEL    filt_symm_iir2;   ///<
	IFE_VA_FLTR_SYMM_SEL    filt_symm_iir3;   ///<
	IFE_VA_FLTR_SIZE_SEL    fltr_size;   ///<
	UINT8                   div;      ///<
	UINT16                  th_low;      ///<
	UINT16                  th_high;      ///<
} IFE_VA_FLTR_PARAM;


typedef struct {
	IFE_VA_FLTR_PARAM       filt_h;
	IFE_VA_FLTR_PARAM       filt_v;
	BOOL                    linemax_en;   ///<
	BOOL                    cnt_en;       ///<
	BOOL                    h_iir2_en;       ///<
	BOOL                    h_iir3_en;       ///<
} IFE_VA_FLTR_GROUP_PARAM;


typedef struct {
	UINT32              win_stx;    ///< Variation accumulation - horizontal starting point
	UINT32              win_sty;    ///< Variation accumulation - vertical starting point
	UINT32              win_numx;    ///< Variation accumulation - horizontal window number
	UINT32              win_numy;    ///< Variation accumulation - vertical window number
	UINT32              win_szx;    ///< Variation accumulation - horizontal window size
	UINT32              win_szy;    ///< Variation accumulation - vertical window size
	UINT32              win_spx;    ///< Variation accumulation - horizontal window spacing
	UINT32              win_spy;    ///< Variation accumulation - vertical window spacing
} IFE_VA_WIN_PARAM;

typedef struct {
	BOOL                indep_va_en;
	UINT32              win_stx;    ///< horizontal window start
	UINT32              win_sty;    ///< vertical window start
	UINT32              win_szx;    ///< horizontal window size
	UINT32              win_szy;    ///< vertical window size
	BOOL                linemax_g1_en;
	BOOL                linemax_g2_en;
} IFE_INDEP_VA_PARAM;

typedef struct {
	UINT32*              p_va_g1_h;
	UINT32*              p_va_g1_v;
	UINT32*              p_va_g2_h;
	UINT32*              p_va_g2_v;
	UINT32*              p_va_cnt_g1_h;
	UINT32*              p_va_cnt_g1_v;
	UINT32*              p_va_cnt_g2_h;
	UINT32*              p_va_cnt_g2_v;
} IFE_INDEP_VA_WIN_MERGE;

typedef struct {
	UINT16*              p_wdr_hist;
} IFE_WDR_HIST_MERGE;



typedef enum {
	IFE_VA_OUT_GROUP1  = 0,   ///<
	IFE_VA_OUT_BOTH    = 1,   ///<
	ENUM_DUMMY4WORD(IFE_VA_OUTSEL)
} IFE_VA_OUTSEL;

typedef struct {
	UINT32 *p_luma;
	UINT32 *p_g1_h;
	UINT32 *p_g1_v;
	UINT32 *p_g2_h;
	UINT32 *p_g2_v;
	UINT32 *p_g1_h_cnt;
	UINT32 *p_g1_v_cnt;
	UINT32 *p_g2_h_cnt;
	UINT32 *p_g2_v_cnt;
} IFE_VA_RSLT;


typedef struct {
	BOOL va_en;
	IFE_VA_OUTSEL outsel;
	BOOL va_force_write_out;
	ULONG address_pipe1;
	ULONG address_pipe2;
	ULONG lineoffset;
	UINT32 dual_cut_pos;
} IFE_VA_SETTING;


typedef struct {
	UINT32              va_luma;
	UINT32              va_g1_h;
	UINT32              va_g1_v;
	UINT32              va_g2_h;
	UINT32              va_g2_v;
	UINT32              va_cnt_g1_h;
	UINT32              va_cnt_g1_v;
	UINT32              va_cnt_g2_h;
	UINT32              va_cnt_g2_v;
} IFE_INDEP_VA_WIN_RSLT;


typedef struct {
	//UINT8               va_LDG_en;
	UINT8               ldg_low_th;
	UINT8               ldg_high_th;
	UINT8               ldg_low_gain;
	UINT8               ldg_high_gain;
	UINT8               ldg_low_slope;
	UINT8               ldg_high_slope;
} IFE_VA_LDG_PARAM;


typedef enum {
	IFE_VA_ENERGY_COUNT     = 0,   ///<
	IFE_VA_HIGH_LUMA_COUNT  = 1,   ///<
	ENUM_DUMMY4WORD(IFE_VA_CNT_OUTSEL)
} IFE_VA_CNT_OUTSEL;

typedef enum {
	IFE_NO_PRE_FILTER     = 0,   ///<
	IFE_PRE_FILTER_3x3_1  = 1,   ///<
	IFE_PRE_FILTER_3x3_2  = 2,   ///<
	ENUM_DUMMY4WORD(IFE_VA_PRE_FILTER_MODE)
} IFE_VA_PRE_FILTER_MODE;

typedef struct {
	IFE_VA_PRE_FILTER_MODE pre_filter_mode;
	IFE_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 blending_w;
} IFE_VA_COMMON_PARAM;

typedef struct {
	BOOL en;
	UINT16 *p_lut;
} IFE_VA_GAMMA;

typedef struct {
	UINT8 vdetgh1_filter_sel;
	UINT8 vdetgh1_iir_input_sel;
	UINT8 vdetgh2_filter_sel;
	UINT8 vdetgh2_iir_input_sel;
} IFE_VA_FILTER_SEL;

typedef struct {
	UINT16 vdetgh1_iir_shift_bit;
	UINT16 vdetgh1_iir2_shift_bit;
	UINT16 vdetgh1_iir3_shift_bit;
	UINT16 vdetgh2_iir_shift_bit;
	UINT16 vdetgh2_iir2_shift_bit;
	UINT16 vdetgh2_iir3_shift_bit;
} IFE_VA_SHIFT_BIT;

typedef struct {

	IFE_VA_FLTR_GROUP_PARAM filt_group_para_g1;
	IFE_VA_FLTR_GROUP_PARAM filt_group_para_g2;
	IFE_VA_WIN_PARAM        va_win_para;
	IFE_INDEP_VA_PARAM      idp_va_para[5];
	IFE_VA_OUTSEL           out_sel;
	IFE_VA_SETTING          va_ctrl;

	IFE_VA_LDG_PARAM        ldg;
	IFE_VA_CNT_OUTSEL       cnt_outsel;
	IFE_VA_PRE_FILTER_MODE  pre_filt;
	IFE_VA_COMMON_PARAM     com_para;
	IFE_VA_GAMMA            gamma;
	IFE_VA_FILTER_SEL       filt_sel;
	IFE_VA_SHIFT_BIT        shift;

	BOOL ldg_en;

	IFE_INDEP_VA_WIN_RSLT   idp_va_win_result; // read only
	IFE_VA_RSLT             va_rslt;
} IFE_ENG_VA_PARAM;

typedef struct {
	UINT16 crop_width;
	UINT16 crop_height;
} IFE_ENG_SUBISP_CROP;

typedef struct {
	BOOL gamma_en;
	UINT8 *p_gamma_lut;
} IFE_ENG_SUBISP_GAMMA;

typedef struct {
	BOOL cst_en;
	//INT16 *p_cst_coef;
} IFE_ENG_SUBISP_CST;

typedef struct {
	INT16 *p_ccm_coef;
} IFE_ENG_SUBISP_CCM;


typedef struct {
	UINT8 sample_rate_x;
	UINT8 sample_rate_y;
} IFE_ENG_SUBISP_SUBSAMPLE;

typedef struct {
	UINT16 sub_ipp_para1;
	UINT16 sub_ipp_para2;
	UINT8  sub_ipp_smpl_hit_init;
} IFE_ENG_SUBISP_DUAL;


typedef struct {
	IFE_ENG_SUBISP_CROP subisp_crop;//
	IFE_ENG_SUBISP_GAMMA subisp_gamma;//
	IFE_ENG_SUBISP_CST subisp_cst;
	IFE_ENG_SUBISP_CCM subisp_ccm;
	IFE_ENG_SUBISP_SUBSAMPLE subisp_scale;//
	IFE_ENG_SUBISP_DUAL  subisp_dual;
	UINT32 subisp_yuv_out_fmt;
	BOOL subisp_cfa_en;
} IFE_ENG_SUBISP_PARAM;


/**
    Struct IFE Information.

    IFE parameter and function selection

\n  Used for ife_set_mode()
*/
//@{
typedef struct _IFE_PARAM {

	IFE_OPMODE      mode;           ///< operation mode
	IFE_CFASEL      cfa_pat;         ///< CFA pattern if RAW format selected
	IFE_CFASEL      cfa_pat_2;         ///< CFA pattern if RAW format selected	
	IFE_BITDEPTH    in_bit;          ///< input bit select
	IFE_BITDEPTH    out_bit;         ///< out bit select

	UINT8    input_format;    ///< raw or yuv
	UINT8    output_sel;      ///< wdr out or sub isp out

	//IFE_CONTROL     ife_ctrl;
	BOOL            ife_set_sel[IFE_SET_ALL];

	UINT32          bayer_format;  ///< 0: Bayer, 1: RGBIr, 2: RGBIr2
	IFE_STRIPE      ife_stripe_info;
	IFE_RING_BUF    ife_ringbuf_info;

	UINT8           filt_mode;
	UINT32          *p_spatial_weight;    ///< spatial weighting
	IFE_RANGESETA   rng_th_a;         ///< range filter threshold setting for filterA
	IFE_RANGESETB   rng_th_b;         ///< range filter threshold setting for filterB
	IFE_CENMODSET   center_mod_set;

	UINT32          bilat_w;       ///< range filter A and B weighting
	UINT32          rth_w;         ///< range filter and spatial filter weighting
	IFE_BINNSEL     binn;           ///< image binning
	//BOOL            bSOnlyEn;       ///< spatial only enable
	//IFE_SONLYLEN    SOnlyLen;       ///< filter length of spatial only
	IFE_CLAMPSET    clamp_set;       ///< clamp and weighting setting
	IFE_CGAINSET    cgain_set;       ///< color gain setting
	IFE_DGAINSET    dgain_set;       ///< digital gain setting
	IFE_OUTLSET     outl_set;        ///< outlier filter setting
	IFE_RBFill_PARAM rb_fill_set;     ///< R-B Filled setting

	IFE_RDESET      rde_set;
	IFE_NRSSET      nrs_set;         ///< nrs setting
	IFE_FCURVESET   fcurve_set;      ///< fcurve setting
	IFE_FUSIONSET   fusion_set;      ///< fusion setting
	IFE_FCGAINSET   fcgain_set;      ///< fusion color gain setting

	IFE_VIG_PARAM   vig_set;         ///< vignette setting
	IFE_GBAL_PARAM  gbal_set;        ///< gbalance setting
	//UINT8           uiBitDither;   ///< Select bit dithering while truncation
	UINT32          in_addr0;        ///< input starting address 0
	UINT32          in_addr1;        ///< input starting address 1
	UINT32          subimg_in_addr;  ///< sub img in
	UINT32          out_addr;        ///< output starting address
	UINT32          out_addr_uv;     ///< output uv starting address
	UINT32          subimg_out_addr; ///< output subimg starting address
	UINT32          va_out_addr;     ///< output va starting address
	UINT32          in_addr0_msb;        ///< input starting address 0
	UINT32          in_addr1_msb;        ///< input starting address 1
	UINT32          subimg_in_addr_msb;  ///< sub img in
	UINT32          out_addr_msb;        ///< output starting address
	UINT32          out_addr_uv_msb;     ///< output uv starting address
	UINT32          subimg_out_addr_msb; ///< output subimg starting address
	UINT32          va_out_addr_msb;     ///< output va starting address

	UINT32          width;           ///< image width
	UINT32          height;          ///< image height
	UINT32          subimg_width;    ///< image width
	UINT32          subimg_height;   ///< image height
	UINT32          crop_width;      ///< crop image width
	UINT32          crop_height;     ///< crop image height
	UINT32          crop_hpos;       ///< crop image horizontal start position
	UINT32          crop_vpos;       ///< crop image vertical start position
	UINT32          in_ofs0;         ///< image input lineoffset0
	UINT32          in_ofs1;         ///< image input lineoffset1
	UINT32          subimg_in_ofs;   ///< image input lineoffset2
	UINT32          out_ofs;         ///< image output lineoffset
	UINT32          out_ofs_uv;      ///< image output uv lineoffset
	UINT32          subimg_out_ofs;  ///< image output lineoffset
	UINT32          va_out_ofs;      ///< image output lineoffset
	UINT32          intr_en;         ///< interrupt enable
	UINT8           fusion_number;   ///< Input path

	UINT8           wdr_en;          ///< wdr enable
	UINT8           sub_out_en;      ///< wdr sub img enable
	UINT8           hist_en;         ///< hist_en
	UINT8           tone_curve_en;   ///< tone_curve_en

	UINT8           fpn_en;          ///< fpn_en

	IFE_ENG_SUBISP_PARAM subisp_set;

	IFE_ENG_WDR_PARAM wdr_set;
	IFE_ENG_WDR_HIST wdr_hist_set;
	IFE_ENG_TONE_CUR_PARAM tone_cur_set;
	IFE_ENG_YUV_IN_PARAM   yuv_in_set;
	IFE_ENG_WDR_OUT_BLD_PARAM out_bld_set;
	IFE_ENG_WDR_SUBIMG sub_img_set;
	UINT8 lpf_coef[3];

	IFE_ENG_VA_PARAM va_set;

	IFE_ENG_FPN_PARAM fpn_set;

	IFE_ENG_SIGLE_OUT_PARAM single_out_set;

	BOOL axi_disable;
	UINT32 axi_channel_en;
	UINT32 axi_lock_dis;
	UINT32 axi_r_ostd_num;
	UINT32 axi_w_ostd_num;

	BOOL dma_out_en;

	UINT32 ife_dram_hist_add;
	UINT32 ife_dram_indep_add;

	UINT32 ife_nn_isp_p0_en;
	UINT32 ife_nn_isp_p1_en;	


} IFE_PARAM;
//@}


//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
extern void ife_eng_set_int_burst_length_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_BURST_LENGTH *p_burst_len);

extern void ife_eng_set_enable_int_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 intr);

extern void ife_eng_set_global_en_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL ife_global_load);

extern void ife_eng_set_op_mode_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_OPMODE mode);

extern void ife_eng_set_outlier_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 outlier_enable);

extern void ife_eng_set_filter_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 filter_enable);

extern void ife_eng_set_cen_mod_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 outlier_enable);

extern void ife_eng_set_rb_fill_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 rb_fill_enable);

extern void ife_eng_set_cgain_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 cgain_enable);

extern void ife_eng_set_dgain_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 dgain_enable);

extern void ife_eng_set_vig_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 vig_enable);

extern void ife_eng_set_gbal_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 gbal_enable);

extern void ife_eng_set_mirror_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 mirror_enable);

extern void ife_eng_set_thermal_mode_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 thermal_mode);

extern void ife_eng_set_decode_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 decode_enable);

extern void ife_eng_set_decode2_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 decode_enable);

extern void ife_eng_set_nrs0_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 nrs0_enable);

extern void ife_eng_set_fcurve_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 fcurve_enable);

extern void ife_eng_set_fcgain_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 fcgain_enable);

extern void ife_eng_set_fusion_en_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 fusion_enable);

extern void ife_eng_set_fnum_buf_reg(IFE_ENG_HANDLE *p_eng, UINT8 fusion_num);

extern void ife_eng_set_stripe_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_STRIPE stripe_info);

extern void ife_eng_set_cfa_pat_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_CFASEL cfa_pat);

extern void ife_eng_set_cfa_pat_2_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_CFASEL cfa_pat);

extern void ife_eng_set_bayer_fmt_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_BAYERFMTSEL bayer_fmt_sel);

extern void ife_eng_set_bitsel_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_BITDEPTH in_bit_depth, IFE_BITDEPTH out_bit_depth);

extern void ife_eng_set_16bit_fmt_sel_reg(IFE_ENG_HANDLE *p_eng, BOOL in_16bit_fmt_sel);

extern void ife_eng_set_ring_buf_ctrl_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RING_BUF *ring_info);

extern void ife_eng_set_ring_buf_chk_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RING_BUF *ring_info);

extern void ife_eng_set_dmaloop_line_count_clear_buf_reg(IFE_ENG_HANDLE *p_eng);

extern void ife_eng_set_dmaloop_sie2_line_count_reload_buf_reg(IFE_ENG_HANDLE *p_eng);

extern void ife_eng_set_dich_line_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_DICH_SYNC_BUF *dich_sync_info);

extern void ife_eng_set_sync_dich_disable_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_DICH_SYNC_BUF *dich_sync_info);

extern void ife_eng_set_dma_wait_sie2_start_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_DICH_SYNC_BUF *dich_sync_info);

extern void ife_eng_set_2dnr_filter_mode_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 filt_mode);

extern void ife_eng_set_2dnr_spatial_w_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 *p_spatial_weight);

extern void ife_eng_set_binning_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_BINNSEL binn_sel);

extern void ife_eng_set_rde_default(IFE_ENG_HANDLE *p_eng, IFE_RDESET *p_rde, IFE_RDE_ADJMODE_SEL rde_mode);

extern void ife_eng_set_wdr_enable_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL wdr_enable);

extern void ife_eng_set_wdr_subout_enable_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL wdr_subout_enable);




#if (defined(_NVT_EMULATION_) == ON)

extern void ife_eng_set_rde_ctrl_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RDESET *p_rde);

extern void ife_eng_set_rde_rate_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RDESET *p_rde);

extern void ife_eng_set_rde_qtable_inx_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RDESET *p_rde);

extern void ife_eng_set_rde_rand_init_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RDESET *p_rde);

#endif


extern void ife_eng_set_2dnr_nlm_parameter_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RANGESETA *p_range_set);

extern void ife_eng_set_2dnr_range_weight_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 rth_w);

extern void ife_eng_set_2dnr_blend_weight_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 bilat_w);

extern void ife_eng_set_2dnr_bilat_parameter_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RANGESETB *p_range_set);

extern void ife_eng_set_2dnr_center_modify_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_CENMODSET *p_center_mod_set);

extern void ife_eng_set_dma_in_addr_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 in_addr, UINT32 in_addr_msb, UINT32 line_ofs);

extern void ife_eng_set_dma_in_addr_2_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 in_addr, UINT32 in_addr_msb, UINT32 line_ofs);

extern void ife_eng_set_hshift_buf_reg(IFE_ENG_HANDLE *p_eng, UINT8 h_shift);

extern void ife_eng_set_dma_out_addr_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 out_addr, UINT32 out_addr_msb, UINT32 line_ofs);

extern void ife_eng_set_dma_out_uv_addr_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 out_addr, UINT32 out_addr_msb, UINT32 line_ofs);

extern void ife_eng_set_in_size_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 hsize, UINT32 vsize);

extern void ife_eng_set_crop_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 crop_hsize, UINT32 crop_vsize, UINT32 crop_hstart, UINT32 crop_vstart);

extern void ife_eng_set_clamp_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_CLAMPSET *p_clamp_weight);

extern void ife_eng_set_digital_gain_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_DGAINSET *p_dgain);

extern void ife_eng_set_color_gain_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_CGAINSET *p_cgain, IFE_CFASEL cfa_pat);

extern void ife_eng_set_outlier_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_OUTLSET *p_outlier);

extern void ife_eng_set_nrs0_str_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NRSSET *p_nrs);

extern void ife_eng_set_nrs1_str_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NRSSET *p_nrs);

extern void ife_eng_set_fcurve_ctrl_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_yweight_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_l_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_r_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_end_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

/*  NT98530 Changed
extern void ife_eng_set_fcurve_index_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_split_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);

extern void ife_eng_set_fcurve_value_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCURVESET *p_fcurve);
*/
extern void ife_eng_set_fusion_debug_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FUSIONSET *p_fusion);

extern void ife_eng_set_fusion_ctrl_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FUSIONSET *p_fusion);

extern void ife_eng_set_fusion_blend_curve(IFE_ENG_HANDLE *p_eng, IFE_FUSIONSET *p_fusion);

extern void ife_eng_set_fusion_diff_weight_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FUSIONSET *p_fusion);

extern void ife_eng_set_fusion_dark_saturation_reduction_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FUSIONSET *p_fusion);

/*NT98530 Changed*/
//extern void ife_eng_set_fusion_short_exposure_compress_buf_reg(IFE_ENG_HANDLE *p_eng,IFE_FUSIONSET *p_fusion);

extern void ife_eng_set_fusion_color_gain_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_FCGAINSET *p_fcgain);

extern void ife_eng_set_2dnr_rbfill_ratio_mode_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RBFill_PARAM *p_rbfill_set);

extern void ife_eng_set_2dnr_rbfill_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_RBFill_PARAM *p_rbfill_set);

extern void ife_eng_set_vignette_center_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VIG_PARAM *p_vig_set);

extern void ife_eng_set_vignette_param_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VIG_PARAM *p_vig_set);

extern void ife_eng_set_gbal_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_GBAL_PARAM *p_gbal_set);

extern void ife_eng_set_wdr_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_WDR_PARAM *p_wdr_param);

extern void ife_eng_set_wdr_in_blend_ratio_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_WDR_PARAM *p_wdr_param);

extern void ife_eng_set_wdr_histo_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_WDR_HIST *p_wdr_hist);

extern void ife_eng_set_wdr_sub_in_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 subin_addr, UINT32 subin_addr_msb, UINT32 subin_lofs);

extern void ife_eng_set_wdr_sub_out_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 subout_addr, UINT32 subout_addr_msb, UINT32 subout_lofs);

extern void ife_eng_set_wdr_subimg_param_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_WDR_SUBIMG *p_wdr_subimg);

extern void ife_eng_set_wdr_subimg_lpf_coeff_buf_reg(IFE_ENG_HANDLE *p_eng, UINT8 *p_lpf_coef);

extern void ife_eng_set_wdr_tone_curve_enable_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL tone_curve_enable);

extern void ife_eng_set_wdr_tone_curve_buf_r_reg(IFE_ENG_HANDLE *p_eng, UINT16 *p_tone_r_buf);

extern void ife_eng_set_wdr_tone_curve_buf_l_reg(IFE_ENG_HANDLE *p_eng, UINT16 *p_tone_l_buf);

extern void ife_eng_set_wdr_output_blend_enable_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL outbld_enable);

extern void ife_eng_set_wdr_output_blend_buf_r_reg(IFE_ENG_HANDLE *p_eng, UINT16 *p_outbld_r_buf);

extern void ife_eng_set_wdr_output_blend_buf_l_reg(IFE_ENG_HANDLE *p_eng, UINT16 *p_outbld_l_buf);

extern void ife_eng_set_yuv_d2d_in_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_YUV_IN_PARAM *p_yuv_in_param);

extern void ife_eng_set_yuv_out_fmt_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_YUV_OUT_FMT yuv_out_fmt);

extern void ife_eng_set_subisp_crop_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_SUBISP_CROP crop_param);

extern void ife_eng_set_subisp_subsample_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_SUBISP_SUBSAMPLE subsam_param);

extern void ife_eng_set_subisp_cst_buf_reg(IFE_ENG_HANDLE *p_eng,  IFE_ENG_SUBISP_CST cst_param);

extern void ife_eng_set_subisp_ccm_buf_reg(IFE_ENG_HANDLE *p_eng,  IFE_ENG_SUBISP_CCM ccm_param);

extern void ife_eng_set_subisp_gamma_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_SUBISP_GAMMA gamma_param);

extern void ife_eng_set_subisp_cfa_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL cfa_en);

extern void ife_eng_set_in_fmt_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_IN_FMT fmt);

extern void ife_eng_set_out_sel_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_OUT_SEL sel);

extern void ife_eng_set_va_output_addr_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 va_out_addr, UINT32 va_out_addr_msb, UINT32 va_out_lofs);

extern void ife_eng_set_va_gamma_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_GAMMA *set_gamma);

extern void ife_eng_set_va_filter_sel_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_FILTER_SEL *filter_sel);

extern void ife_eng_set_va_shift_bit_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_SHIFT_BIT *shift_sel);

extern void ife_eng_set_vacc_enable_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void ife_eng_set_force_enable_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void ife_eng_set_vacc_win_enable_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 set0_en, UINT32 set1_en, UINT32 set2_en, UINT32 set3_en, UINT32 set4_en);

extern void ife_eng_set_va_out_sel_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL en);

extern void ife_eng_set_va_filter_g1_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_FLTR_GROUP_PARAM *p_va_fltr_g1);

extern void ife_eng_set_va_filter_g2_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);

extern void ife_eng_set_va_mode_enable_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_FLTR_GROUP_PARAM *p_va_fltr_g1, IFE_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);

extern void ife_eng_set_va_win_info_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_WIN_PARAM *p_va_win);

extern void ife_eng_set_va_indep_win_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_INDEP_VA_PARAM  *p_indep_va_win_info, UINT32 win_idx);

extern void ife_eng_set_va_common_info_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_COMMON_PARAM *lpPara);

extern void ife_eng_set_va_ldg_enable_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void ife_eng_set_va_ldg_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_VA_LDG_PARAM *lpPara);

extern void ife_eng_set_dma_in_addr_ofs_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 in_addr_ofs);

extern void ife_eng_set_dma_in_addr_2_ofs_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 in_addr_ofs);

extern void ife_eng_set_dma_out_addr_ofs_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 out_addr_ofs);

extern void ife_eng_set_dma_out_uv_addr_ofs_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 out_addr_ofs);

extern void ife_eng_set_va_out_addr_ofs_buf_reg(IFE_ENG_HANDLE *p_eng, UINT32 out_addr_ofs);

extern void ife_eng_set_dma_out_en_buf_reg(IFE_ENG_HANDLE *p_eng, BOOL enable);

extern void ife_eng_set_fpn_para_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_FPN_PARAM *p_fpn);

extern void ife_eng_get_stripe_limit(UINT32 input_format, IFE_STRIPE_LIMIT* stp_limit);

extern void ife_eng_set_single_out_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_ENG_SIGLE_OUT_PARAM p_single);

//-------------------------------------------------------

typedef enum _IFE_NN_ISP_PATH_ {
	IFE_NN_ISP_PATH0_NR_HDR = 0,   ///< NR / HDR
	IFE_NN_ISP_PATH1_WDR    = 1,   ///< WDR
	ENUM_DUMMY4WORD(IFE_NN_ISP_PATH_SEL)
} IFE_NN_ISP_PATH_SEL;


extern VOID ife_eng_set_nn_isp_enable_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 set_en);

extern VOID ife_eng_set_nn_isp_fw_handshake_enable_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 set_en);

extern VOID ife_eng_set_nn_isp_slice_info_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 ring_buf_height, UINT32 slice_height, UINT32 slice_ovlap);

extern VOID ife_eng_set_nn_isp_output_height_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 buf_height);

extern VOID ife_eng_set_nn_isp_lineoffset_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 lofs);

extern VOID ife_eng_set_nn_isp_ring_buf_addr_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern VOID ife_eng_set_nn_isp_out_buf_addr_buf_reg(IFE_ENG_HANDLE *p_eng, IFE_NN_ISP_PATH_SEL path_sel, UINT32 addr_lsb, UINT32 addr_msb);

extern UINT32 ife_eng_cal_nn_isp_buf_output_lines(UINT32 height, UINT32 slice_base_height, UINT32 slice_ovlap, UINT32 *p_buf_out_lines);

extern UINT32 ife_eng_cal_nn_isp_spectial_case_enable(UINT32 height, UINT32 slice_height, UINT32 slice_overlap);
#endif

//@}
