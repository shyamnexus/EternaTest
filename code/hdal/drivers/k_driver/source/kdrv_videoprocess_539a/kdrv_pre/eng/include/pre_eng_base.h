/*
    Public header file for PRE module.

    This file is the header file that define the API and data type for PRE
    module.

    @file       pre_lib.h
    @ingroup    mIDrvIPP_PRE

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#ifndef _PRE_LIB_539A_H
#define _PRE_LIB_539A_H

#ifdef __KERNEL__
#include "kwrap/type.h"
#else
#include <kwrap/nvt_type.h>
#endif

#include "pre_eng_handle.h"

#define PRE_GET_API_EN  0


/**
    @addtogroup mIDrvIPP_PRE
*/
//@{

/*
    Define PRE function enable.
*/
//@{
//#define PRE_ROWDEFC_EN                  0x00001000  ///< enable function: row defect concealment
#define PRE_OUTL_EN                     0x00002000  ///< enable function: outlier
#define PRE_NRS_EN                      0x01000000  ///< enable function: NRS
#define PRE_FCG_EN                      0x02000000  ///< enable function: f_color gain
#define PRE_FUSION_EN                   0x04000000  ///< enable function: sensor HDR
#define PRE_FCURVE_EN                   0x20000000  ///< enable function: F curve
#define PRE_MIRROR_EN                   0x40000000  ///< enable function: mirror
#define PRE_DECODE_EN                   0x80000000  ///< enable function: decode
//#define PRE_FLIP_EN                   0x00200000  ///< removed in NT96680
#define PRE_FUNC_ALL                    0xe7c3e000  ///< all func enable mask
//@}


/**
    Define PRE interrput enable.
*/
//@{
#define PRE_INTE_FRMEND                 0x00000001  ///< frame end interrupt
#define PRE_INTE_DEC1_ERR               0x00000002  ///< frame end interrupt
#define PRE_INTE_DEC2_ERR               0x00000004  ///< frame end interrupt
#define PRE_INTE_LLEND                  0x00000008  ///< LL end interrupt
#define PRE_INTE_LLERR                  0x00000010  ///< LL error  interrupt
#define PRE_INTE_LLERR2                 0x00000020  ///< LL error2 interrupt
#define PRE_INTE_LLJOBEND               0x00000040  ///< LL job end interrupt
#define PRE_INTE_BUFOVFL                0x00000080  ///< buffer overflow interrupt
#define PRE_INTE_RING_BUF_ERR           0x00000100  ///< ring buffer error interrupt
#define PRE_INTE_FRAME_ERR              0x00000200  ///< frame error interrupt (for direct mode)
#define PRE_INTE_DEC3_ERR               0x00000400  ///< path2 decode error interrupt
#define PRE_INTE_RING_BUF_ERR2          0x00000800  ///< ring buffer error 2 interrupt
#define PRE_INTE_SIE_FRM_START          0x00001000  ///< SIE frame start interrupt
#define PRE_INTE_FRM_START               0x00004000  ///< PRE frame start interrupt
#define PRE_INTE_ALL                    0x000f5fff  ///< all interrupts
//@}

/**
    Define PRE interrput status.
*/
//@{
#define PRE_INT_FRMEND                 0x00000001  ///< frame end interrupt
#define PRE_INT_DEC1_ERR               0x00000002  ///< path0 decode error interrupt
#define PRE_INT_DEC2_ERR               0x00000004  ///< path1 decode error interrupt
#define PRE_INT_LLEND                  0x00000008  ///< LL end interrupt
#define PRE_INT_LLERR                  0x00000010  ///< LL error  interrupt
#define PRE_INT_LLERR2                 0x00000020  ///< LL error2 interrupt
#define PRE_INT_LLJOBEND               0x00000040  ///< LL job end interrupt
#define PRE_INT_BUFOVFL                0x00000080  ///< buffer overflow interrupt
#define PRE_INT_RING_BUF_ERR           0x00000100  ///< ring buffer error interrupt
#define PRE_INT_FRAME_ERR              0x00000200  ///< frame error interrupt (for direct mode)
//#define PRE_INT_DEC3_ERR               0x00000400  ///< path2 decode error interrupt
//#define PRE_INT_RING_BUF_ERR2          0x00000800  ///< ring buffer error 2 interrupt
#define PRE_INT_SIE_FRM_START          0x00001000  ///< SIE frame start interrupt
#define PRE_INT_SIE2_FRM_START         0x00002000  ///< SIE2 frame start interrupt
#define PRE_INT_FRM_START              0x00004000  ///< PRE frame start interrupt




//#define PRE_INT_ROWDEFFAIL             0x00000004  ///< removed in NT96680
#define PRE_INT_ALL                    0x000f5fff  ///< all interrupts
//@}


/**
    PRE Function Selection.

    PRE Function Selection.
\n  Used for pre_set_mode()
*/
//@{
typedef enum {
	PRE_OPMODE_D2D         = 0,        ///< D2D
	PRE_OPMODE_IPP         = 1,        ///< IPP mode
	PRE_OPMODE_ALL_DIRECT  = 2,        ///< All direct
	ENUM_DUMMY4WORD(PRE_OPMODE)
} PRE_OPMODE;

/**
    PRE Output Selection.

    PRE Output Selection.
\n  Used for pre_set_out_sel()
*/
//@{
typedef enum {
	PRE_OUT_TO_DRAM        = 0,        ///< Not output to IFE
	PRE_OUT_TO_IFE         = 1,        ///< Output to IFE
	ENUM_DUMMY4WORD(PRE_OUT_SEL)
} PRE_OUT_SEL;


/**
    PRE RDE mode of parameters
*/
//@{
typedef enum {
	PRE_RDE_PARMS_NORMAL_MODE    = 0,
	PRE_RDE_PARMS_AGGRESIVE_MODE    = 1,
	ENUM_DUMMY4WORD(PRE_RDE_ADJMODE_SEL)
} PRE_RDE_ADJMODE_SEL;
//@}

/**
    PRE Register type.

    PRE Register type.
\n  Used for pre_change_all()
*/
//@{
typedef enum {
	PRE_ALL_REG         = 0,    ///< Set all registers
	PRE_FD_LATCHED      = 1,    ///< Update VD-latched registers only
	PRE_NON_FD_LATCHED  = 2,    ///< Updat Non-VD-latched registers only
	ENUM_DUMMY4WORD(PRE_REGTYPE)
} PRE_REGTYPE;
//@}



/**
    PRE Wait Mode Selection.

    PRE Wait Mode Selection.
\n  Used for pre_set_mode()
*/
//@{
typedef enum {
	PRE_WAIT    = 0,                ///< Wait for frame end
	PRE_NOWAIT  = 1,                ///< Do not wait for frame end
	ENUM_DUMMY4WORD(PRE_WAITMODE)
} PRE_WAITMODE;
//@}


/**
    PRE Bit Depth.

    Set PRE input/output bitdepth
*/
//@{
typedef enum {
	PRE_8BIT    = 0,                ///<  8bit data
	PRE_10BIT   = 1,                ///< 10bit data
	PRE_12BIT   = 2,                ///< 12bit data
	PRE_16BIT   = 3,                ///< 16bit data
	ENUM_DUMMY4WORD(PRE_BITDEPTH)
} PRE_BITDEPTH;
//@}

/**
    PRE color gain range selection

    Select PRE color gain range
*/
//@{
typedef enum {
	PRE_CGRANGE_2_8 = 0,    ///< 8bit decimal, set 256 for 1x
	PRE_CGRANGE_3_7 = 1,    ///< 7bit decimal, set 128 for 1x
	ENUM_DUMMY4WORD(PRE_CGRANGE)
} PRE_CGRANGE;
//@}

/**
    PRE fusion color gain range selection

    Select PRE fusion color gain range
*/
//@{
typedef enum {
	PRE_FCGRANGE_8_8 = 0,    ///< 8bit decimal, set 256 for 1x
	PRE_FCGRANGE_9_7 = 1,    ///< 7bit decimal, set 128 for 1x
	ENUM_DUMMY4WORD(PRE_FCGRANGE)
} PRE_FCGRANGE;
//@}


/**
    PRE input Bayer data start CFA selection

    Select intput Bayer raw data start channel - R, Gr, Gb, B.
*/
//@{
typedef enum {
	PRE_PAT0  = 0,  ///<
	PRE_PAT1  = 1,  ///<
	PRE_PAT2  = 2,  ///<
	PRE_PAT3  = 3,  ///<
	PRE_PAT4  = 4,  ///<
	PRE_PAT5  = 5,  ///<
	PRE_PAT6  = 6,  ///<
	PRE_PAT7  = 7,  ///<
	ENUM_DUMMY4WORD(PRE_CFASEL)
} PRE_CFASEL;
//@}

/**
    PRE input Bayer format selection

    Select intput Bayer format
*/
//@{
typedef enum {
	PRE_BAYER_RGGB  = 0, ///< Traditional RGGB format
	PRE_BAYER_RGBIR = 1, ///< RGBIr format
	ENUM_DUMMY4WORD(PRE_BAYERFMTSEL)
} PRE_BAYERFMTSEL;
//@}

/**
    PRE burst length define
*/
//@{
typedef enum {
	PRE_IN_BURST_64W    = 0,       ///< burst length 64 word
	PRE_IN_BURST_32W    = 1,       ///< burst length 32 word
	ENUM_DUMMY4WORD(PRE_IN_BURST_SEL)
} PRE_IN_BURST_SEL;

/**
    PRE output burst length define
*/
//@{
typedef enum {
	PRE_OUT_BURST_32W    = 0,       ///< burst length 32 word
	PRE_OUT_BURST_16W    = 1,       //removed in NT96680
	ENUM_DUMMY4WORD(PRE_OUT_BURST_SEL)
} PRE_OUT_BURST_SEL;


/**
    PRE decode mode of parameters
*/
//@{
typedef enum {
	PRE_BNR_DECODE_PARMS_NORMAL_MODE    = 0,
	PRE_BNR_DECODE_PARMS_AGGRESIVE_MODE = 1,
	ENUM_DUMMY4WORD(PRE_BNR_DECODE_ADJMODE_SEL)
} PRE_BNR_DECODE_ADJMODE_SEL;
//@}


/**
    PRE SET ENUM
*/
//@{
typedef enum {
	PRE_SET_INFO,
	PRE_SET_RING_BUF,
	PRE_SET_MIRROR,
	PRE_SET_RDE,
	PRE_SET_NRS,
	PRE_SET_FUSION,
	PRE_SET_FCURVE,
	PRE_SET_OUTL,
	PRE_SET_ALL,
	ENUM_DUMMY4WORD(PRE_UPDATE_SEL)
} PRE_SET_SEL;

typedef struct _PRE_ENG_FPN_PARAM {
	BOOL    fpn_en;
	UINT8   fpn_cgain_range;
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
} PRE_ENG_FPN_PARAM;

typedef struct _PRE_SIE_HANDSHAKE {
	BOOL   input_combine_en;
	UINT8 src_clk_sel_fifo0;
	UINT8 src_clk_sel_fifo1;
	UINT16 in_buf_write_width0;
	UINT16 in_buf_write_width1;
	UINT16 in_buf_crop_st0;
	UINT16 in_buf_crop_st1;
	UINT16 in_buf_crop_width0;
	UINT16 in_buf_crop_width1;
} PRE_SIE_HANDSHAKE;

typedef struct _PRE_ENG_SINGLE_OUT_PARAM {
	BOOL  single_out_0_en;
	BOOL  single_out_2_en;
	BOOL  single_out_ref_en;
	BOOL  single_out_sigma_en;
	BOOL  single_out_sta_en;
	BOOL  single_out_gamma_en;
	BOOL  single_out_fusion_weight_en;
	BOOL  out_mode;
} PRE_ENG_SIGLE_OUT_PARAM;




typedef struct _PRE_BURST_LENGTH {
	PRE_IN_BURST_SEL   burst_len_input;  ///< Input burst length
	PRE_OUT_BURST_SEL   burst_len_output; ///< Output burst length
} PRE_BURST_LENGTH;
//@}

/**
    Struct PRE open object.

    ISR callback function
*/
//@{
typedef struct _PRE_OPENOBJ {
	void (*FP_PREISR_CB)(UINT32 ui_int_status); ///< isr callback function
	UINT32 ui_pre_clock_sel; ///< TBD(240 is tested by now)
} PRE_OPENOBJ;
//@}

/**
    Struct PRE stripe

    Parameters of stripe
*/
//@{
typedef struct _PRE_STRIPE {
	//UINT32 hn;            ///< range filter threshold
	//UINT32 hl;           ///< range filter threshold adjustment
	//UINT32 hm;
	UINT32 pre_stripe_w[8];
	UINT32 pre_stripe_num;
	UINT32 pre_stripe_overlap;
	UINT32 input_format;
} PRE_STRIPE;
//@}


/**
    Struct PRE stripe limit

    Parameters of stripe
*/
//@{
typedef struct _PRE_STRIPE_LIMIT {
	UINT32 stripe_align;
	UINT32 stripe_overlap;
	UINT32 stripe_min;
	UINT32 last_stripe_align;
	UINT32 last_stripe_min;
} PRE_STRIPE_LIMIT;
//@}


/**
    Struct PRE stripe

    Parameters of stripe
*/
//@{
typedef struct _PRE_RING_BUF {
	BOOL   dmaloop_ctrl;    ///< dma loop control
	BOOL   dmaloop_en;      ///< dma loop enable
	UINT32 dmaloop_line;    ///< dma loop dram line number
} PRE_RING_BUF;

/**
    Struct PRE sync_dich

    Parameters of stripe
*/
//@{
typedef struct _PRE_DICH_SYNC_BUF {
	UINT32 dich_line_ctrl;
	BOOL   dma_sync_dich_line_dis;
	BOOL   dma1_wait_sie2_start_dis;
	BOOL   dma2_wait_sie3_start_dis;
} PRE_DICH_SYNC_BUF;


//@}

/**
    Struct PRE RDE.

    Parameters of RDE
*/
//@{
typedef struct _PRE_RDESET {
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
#endif
} PRE_RDESET;
//@}

/**
    Struct PRE Color Gain.

    Parameters of color gain
*/
//@{
typedef struct _PRE_CGAINSET {
	PRE_CGRANGE cgain_range;         ///< select color gain decimal range
	UINT32      p_cgain[4];          ///< color gain, always R/Gr/Gb/B order
	UINT32      p_cofs[4];           ///< color offset, always R/Gr/Gb/B order
} PRE_CGAINSET;
//@}


/**
    Struct PRE NRS.

    Parameters of NRS
*/
//@{
typedef struct _PRE_NRSSET {
	UINT16          *pre_f_nrs0_str;              ///<  NRS0 strength
	//UINT16          *pre_f_nrs1_str;              ///<  NRS1 strength
} PRE_NRSSET;
//@}

/**
    Struct PRE Fcurve.

    Parameters of Fcurve
*/
//@{
typedef struct _PRE_FCURVESET {
	UINT8            y_mean_select;     ///<  Fcurve Y Mean Method selection
	UINT8            yv_weight;         ///<  Fcurve Y mean and V weight
	UINT8            ev_format;         ///<  Fcurve ev format, 4->4EV format. 3->3 EV format ... so on
	UINT8           *p_y_weight_lut;    ///<  Fcurve Y weight
	UINT32          *p_left_lut;        ///<  Fcurve left part, PRE_F_FCURVE_L
	UINT32          *p_right_lut;       ///<  Fcurve right part, PRE_F_FCURVE_R
	UINT32          *p_end_lut;         ///<  Fcurve end part, PRE_F_FCURVE_END
} PRE_FCURVESET;
//@}

/**
    Struct PRE Fusion.

    Parameters of Fusion
*/
//@{
typedef struct _PRE_FUSIONSET {
	UINT16           debug_mode;
	UINT8            y_mean_sel;
	UINT8            nor_blendcur_sel;
	UINT8            diff_blendcur_sel;
	UINT8            mode;
	UINT16           ev_ratio;

	UINT16           mc_lum_th;
	UINT8            mc_diff_ratio;
	UINT8            *p_mc_diff_w;
	UINT8            mc_diff_lumth_diff_w;


	UINT16           long_nor_blendcur_knee;
	UINT16           long_nor_blendcur_range;
	UINT16           long_nor_blendcur_slope;
	UINT16           short_nor_blendcur_knee;
	UINT16           short_nor_blendcur_range;
	UINT16           short_nor_blendcur_slope;

	UINT16           long_diff_blendcur_knee;
	UINT16           long_diff_blendcur_range;
	UINT16           long_diff_blendcur_slope;
	UINT16           short_diff_blendcur_knee;
	UINT16           short_diff_blendcur_range;
	UINT16           short_diff_blendcur_slope;

	UINT16           *p_dark_sat_reduce_th;
	UINT8            *p_dark_sat_reduce_step;
	UINT8            *p_dark_sat_reduce_lowbound;

	BOOL             fu_weight_en;
	BOOL             fu_weight_wait_en;

} PRE_FUSIONSET;
//@}

typedef struct _PRE_FUSION_WEIGHT_BUF_INFO {
	UINT32 get_fusion_weight_lofs;
	UINT32 get_fusion_weight_size;
} PRE_FUSION_WEIGHT_BUF_INFO;

/**
    Struct PRE Fusion Color Gain.

    Parameters of fusion color gain
*/
//@{
typedef struct _PRE_FCGAINSET {
	PRE_FCGRANGE fcgain_range;                   ///< select Fusion color gain decimal range
	UINT16      p_fusion_cgain_path0[5];          ///< color gain, always R/Gr/Gb/B order
	UINT16      p_fusion_cgain_path1[5];          ///< color gain, always R/Gr/Gb/B order
	UINT16      p_fusion_cofs_path0[5];           ///< color offset, always R/Gr/Gb/B order
	UINT16      p_fusion_cofs_path1[5];           ///< color offset, always R/Gr/Gb/B order

} PRE_FCGAINSET;
//@}

/**
    Struct PRE Outlier Filter.

    Parameters of outlier filter
*/
//@{
typedef struct _PRE_OUTLSET {
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
	//PRE_OUTL_SEL    OutlOp;         ///< outlier option for neighbour condition

} PRE_OUTLSET;
//@}


/**
    Struct PRE size Parameters.

    PRE Size related parameters.
*/
//@{
typedef struct _PRE_SIZE_PARA {
	UINT32 width;               ///< image width
	UINT32 height;              ///< image height
	UINT32 ui_ofsi_1;              ///< image input lineoffset
	UINT32 ui_ofsi_2;              ///< image input lineoffset
	UINT32 ui_ofsi_3;              ///< image input lineoffset
	UINT32 ui_ofso;                ///< image output lineoffset
} PRE_SIZE_PARA;
//@}

/**
    Struct PRE ROI Parameters.

    PRE Address, Cropping detailed parameters.
*/
//@{
typedef struct _PRE_ROI_PARA {
	PRE_SIZE_PARA   size;            ///< input/output size para.
	UINT32          ui_in_addr_1;      ///< input starting address
	UINT32          ui_in_addr_2;      ///< input starting address
	UINT32          ui_in_addr_3;      ///< input starting address
	UINT32          ui_out_addr;       ///< output starting address
	UINT32          crop_width;     ///< image crop width
	UINT32          crop_height;    ///< image crop height
	UINT32          crop_hpos;      ///< image horizontal crop start pixel
	UINT32          crop_vpos;      ///< image virtical crop start pixel
	PRE_BITDEPTH    in_bit;           ///< input bit select
	PRE_BITDEPTH    out_bit;          ///< out bit select
	//BOOL            b_flip_en;        ///< enable of flip
} PRE_ROI_PARA;
//@}

typedef enum {
	PRE_VA_PATH0     = 0,   ///<
	PRE_VA_PATH1     = 1,   ///<
	PRE_VA_AFTER_HDR = 2,   ///<
	ENUM_DUMMY4WORD(PRE_VA_IN_SEL)
} PRE_VA_IN_SEL;

typedef enum {
	PRE_VA_FLTR_MIRROR  = 0,   ///<
	PRE_VA_FLTR_INVERSE = 1,   ///<
	ENUM_DUMMY4WORD(PRE_VA_FLTR_SYMM_SEL)
} PRE_VA_FLTR_SYMM_SEL;

typedef enum {
	PRE_VA_FLTR_SIZE_1  = 0,   ///<
	PRE_VA_FLTR_SIZE_3  = 1,   ///<
	PRE_VA_FLTR_SIZE_5  = 2,   ///<
	PRE_VA_FLTR_SIZE_7  = 3,   ///<
	ENUM_DUMMY4WORD(PRE_VA_FLTR_SIZE_SEL)
} PRE_VA_FLTR_SIZE_SEL;


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

	PRE_VA_FLTR_SYMM_SEL    filt_symm;   ///<
	PRE_VA_FLTR_SYMM_SEL    filt_symm_iir2;   ///<
	PRE_VA_FLTR_SYMM_SEL    filt_symm_iir3;   ///<
	PRE_VA_FLTR_SIZE_SEL    fltr_size;   ///<
	UINT8                   div;      ///<
	UINT16                  th_low;      ///<
	UINT16                  th_high;      ///<
} PRE_VA_FLTR_PARAM;


typedef struct {
	PRE_VA_FLTR_PARAM       filt_h;
	PRE_VA_FLTR_PARAM       filt_v;
	BOOL                    linemax_en;   ///<
	BOOL                    cnt_en;       ///<
	BOOL                    h_iir2_en;       ///<
	BOOL                    h_iir3_en;       ///<
} PRE_VA_FLTR_GROUP_PARAM;


typedef struct {
	UINT32              win_stx;    ///< Variation accumulation - horizontal starting point
	UINT32              win_sty;    ///< Variation accumulation - vertical starting point
	UINT32              win_numx;    ///< Variation accumulation - horizontal window number
	UINT32              win_numy;    ///< Variation accumulation - vertical window number
	UINT32              win_szx;    ///< Variation accumulation - horizontal window size
	UINT32              win_szy;    ///< Variation accumulation - vertical window size
	UINT32              win_spx;    ///< Variation accumulation - horizontal window spacing
	UINT32              win_spy;    ///< Variation accumulation - vertical window spacing
} PRE_VA_WIN_PARAM;

typedef struct {
	BOOL                indep_va_en;
	UINT32              win_stx;    ///< horizontal window start
	UINT32              win_sty;    ///< vertical window start
	UINT32              win_szx;    ///< horizontal window size
	UINT32              win_szy;    ///< vertical window size
	BOOL                linemax_g1_en;
	BOOL                linemax_g2_en;
} PRE_INDEP_VA_PARAM;

typedef struct {
	UINT32*              p_va_g1_h;
	UINT32*              p_va_g1_v;
	UINT32*              p_va_g2_h;
	UINT32*              p_va_g2_v;
	UINT32*              p_va_cnt_g1_h;
	UINT32*              p_va_cnt_g1_v;
	UINT32*              p_va_cnt_g2_h;
	UINT32*              p_va_cnt_g2_v;
} PRE_INDEP_VA_WIN_MERGE;


typedef enum {
	PRE_VA_OUT_GROUP1  = 0,   ///<
	PRE_VA_OUT_BOTH    = 1,   ///<
	ENUM_DUMMY4WORD(PRE_VA_OUTSEL)
} PRE_VA_OUTSEL;

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
} PRE_VA_RSLT;


typedef struct {
	BOOL va_en;
	PRE_VA_OUTSEL outsel;
	BOOL va_force_write_out;
	ULONG address_pipe1;
	ULONG address_pipe2;
	ULONG lineoffset;
	UINT32 dual_cut_pos;

	PRE_VA_IN_SEL va_in_sel;
	BOOL  va_vertical_fir_en;

} PRE_VA_SETTING;


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
} PRE_INDEP_VA_WIN_RSLT;


typedef struct {
	//UINT8               va_LDG_en;
	UINT8               ldg_low_th;
	UINT8               ldg_high_th;
	UINT8               ldg_low_gain;
	UINT8               ldg_high_gain;
	UINT8               ldg_low_slope;
	UINT8               ldg_high_slope;
} PRE_VA_LDG_PARAM;


typedef enum {
	PRE_VA_ENERGY_COUNT     = 0,   ///<
	PRE_VA_HIGH_LUMA_COUNT  = 1,   ///<
	ENUM_DUMMY4WORD(PRE_VA_CNT_OUTSEL)
} PRE_VA_CNT_OUTSEL;

typedef enum {
	PRE_NO_PRE_FILTER     = 0,   ///<
	PRE_PRE_FILTER_3x3_1  = 1,   ///<
	PRE_PRE_FILTER_3x3_2  = 2,   ///<
	ENUM_DUMMY4WORD(PRE_VA_PRE_FILTER_MODE)
} PRE_VA_PRE_FILTER_MODE;

typedef struct {
	PRE_VA_PRE_FILTER_MODE pre_filter_mode;
	PRE_VA_CNT_OUTSEL win_cnt_out_sel;
	UINT8 high_luma_th;
	UINT8 blending_w;
} PRE_VA_COMMON_PARAM;

typedef struct {
	BOOL en;
	UINT16 *p_lut;
} PRE_VA_GAMMA;

typedef struct {
	UINT8 vdetgh1_filter_sel;
	UINT8 vdetgh1_iir_input_sel;
	UINT8 vdetgh2_filter_sel;
	UINT8 vdetgh2_iir_input_sel;
} PRE_VA_FILTER_SEL;

typedef struct {
	UINT16 vdetgh1_iir_shift_bit;
	UINT16 vdetgh1_iir2_shift_bit;
	UINT16 vdetgh1_iir3_shift_bit;
	UINT16 vdetgh2_iir_shift_bit;
	UINT16 vdetgh2_iir2_shift_bit;
	UINT16 vdetgh2_iir3_shift_bit;
} PRE_VA_SHIFT_BIT;

typedef struct {

	PRE_VA_FLTR_GROUP_PARAM filt_group_para_g1;
	PRE_VA_FLTR_GROUP_PARAM filt_group_para_g2;
	PRE_VA_WIN_PARAM        va_win_para;
	PRE_INDEP_VA_PARAM      idp_va_para[5];
	PRE_VA_OUTSEL           out_sel;
	PRE_VA_SETTING          va_ctrl;

	PRE_VA_LDG_PARAM        ldg;
	PRE_VA_CNT_OUTSEL       cnt_outsel;
	PRE_VA_PRE_FILTER_MODE  pre_filt;
	PRE_VA_COMMON_PARAM     com_para;
	PRE_VA_GAMMA            gamma;
	PRE_VA_FILTER_SEL       filt_sel;
	PRE_VA_SHIFT_BIT        shift;

	BOOL ldg_en;

	PRE_INDEP_VA_WIN_RSLT   idp_va_win_result; // read only
	PRE_VA_RSLT             va_rslt;
} PRE_ENG_VA_PARAM;


typedef struct {
	BOOL           bnr_en;          ///< bnr_en
	BOOL           bnr_sta_en;
	BOOL           bnr_sigma_en;

	BOOL           bnr_ref_out_en;
	BOOL           bnr_ref_dec_en;
	BOOL           bnr_ref_enc_en;

	BOOL		   bnr_gamma_out_en;
	BOOL		   bnr_gamma_out_wait_en;

	BOOL           bnr_err_comp_en;

	BOOL           bnr_aided_map_direct_en;
}PRE_BNR_CTRL;

typedef struct {
	UINT16          *md_th_l;   ///<
	UINT16          *md_th_r;   ///<
	UINT16          md_base;
	UINT16          md_k1;
	UINT16          md_k2;
}PRE_BNR_MD_PARAM;

typedef struct {
	UINT16          *err_comp_l;   ///<
	UINT16          *err_comp_r;   ///<
	UINT8            err_sft;
}PRE_BNR_ERR_COMP;

typedef struct {
	UINT16           coef_a;   ///<
	UINT16           coef_b;   ///<
}PRE_BNR_NP_COEF;

typedef struct {
	UINT8            downsample_th1;   ///<
	UINT8            downsample_th2;   ///<
}PRE_BNR_PS_DS_TH;

typedef struct {
	UINT16            static_region;   ///<
	UINT16            transition_region;   ///<
	UINT16            motion_region;
}PRE_BNR_FUSION_STR;

typedef struct {
	UINT32 get_motion_lofs;           ///< get motion buffer lineoffset
	UINT32 get_motion_size;           ///< get motion buffer size

	UINT32 get_sigma_lofs;            ///< get sigma buffer lineoffset
	UINT32 get_sigma_size;            ///< get sigma status buffer size

	UINT32 get_sta_lofs;              ///< get statistic data buffer lineoffset
	UINT32 get_sta_size;              ///< get statistic data buffer size

	UINT32 get_gamma_map_lofs;        ///< get gamma map buffer lineoffset
	UINT32 get_gamma_map_size;        ///< get gamma map buffer size
} PRE_BNR_BUF_SIZE_INFO;



/**
    Struct PRE BNR reference codec.

    Parameters of PRE BNR refference codec
*/
//@{
typedef struct _PRE_BNR_REF_CODEC_SET {
	UINT8           encode_rate;        ///<  BNR codec encode rate of bitstream per segment
#if (defined(_NVT_EMULATION_) == ON)
	BOOL            b_dec_degamma_en;   ///<  BNR decode degamma enable
	BOOL            b_enc_gamma_en;     ///<  BNR encode gamma enable
	BOOL            b_dec_dithering_en; ///<  BNR decode dithering enable
	BOOL            dec_dither_reset;   ///<  BNR decode dithering reset initial random seed

	UINT8           *p_dct_qtable_inx;  ///<  BNR codec the index for DCT decode Q table

	UINT8           *p_enc_dct_level_th;///<  BNR encode the index for DCT decode Q table

	UINT8           rand1_init1;        ///<  BNR decode random sequence init setting
	UINT16          rand1_init2;        ///<  BNR decode random sequence init setting
#endif
} PRE_BNR_REF_CODEC_SET;
//@}

typedef struct {
	PRE_BNR_CTRL          bnr_ctrl;
	PRE_BNR_MD_PARAM      bnr_md_para;
	PRE_BNR_ERR_COMP      bnr_err_comp;
	PRE_BNR_NP_COEF       bnr_np_coef;
	PRE_BNR_PS_DS_TH      bnr_ps_ds_th;
	PRE_BNR_FUSION_STR    bnr_fu_str;
	PRE_BNR_BUF_SIZE_INFO bnr_buf_size_info;
    PRE_BNR_REF_CODEC_SET bnr_ref_codec;
	UINT32 sigma_opt;
	UINT32 prefilter_str;
	UINT32 dbg_mode;
	UINT32 residue_th;

} PRE_ENG_BNR_PARAM;


/**
    Struct PRE Information.

    PRE parameter and function selection

\n  Used for pre_set_mode()
*/
//@{
typedef struct _PRE_PARAM {

	PRE_OPMODE      mode;           ///< operation mode
	PRE_CFASEL      cfa_pat;        ///< CFA pattern if RAW format selected
	PRE_CFASEL      cfa_pat_2;      ///< CFA pattern if RAW format selected
	PRE_BITDEPTH    in_bit;         ///< input bit select
	PRE_BITDEPTH    out_bit;        ///< out bit select

	UINT8           pre_output_sel;      ///< output to dram or IFE

	BOOL            pre_set_sel[PRE_SET_ALL];

	UINT32          bayer_format;  ///< 0: Bayer, 1: RGBIr
	PRE_STRIPE      pre_stripe_info;
	PRE_RING_BUF    pre_ringbuf_info;

	UINT8           filt_mode;

	PRE_OUTLSET     outl_set;        ///< outlier filter setting

	PRE_RDESET      rde_set;
	PRE_NRSSET      nrs_set;         ///< nrs setting
	PRE_FCURVESET   fcurve_set;      ///< fcurve setting
	PRE_FUSIONSET   fusion_set;      ///< fusion setting
	PRE_FCGAINSET   fcgain_set;      ///< fusion color gain setting


	UINT32          in_addr0;        ///< input starting address 0
	UINT32          in_addr1;        ///< input starting address 1

	UINT32          ref_in_addr;
	UINT32          ref_out_addr;
	UINT32          motion_in_addr;
	UINT32          motion_out_addr;
	UINT32          sigma_in_addr;
	UINT32          sigma_out_addr;
	UINT32          sta_out_addr;
	UINT32          gamma_out_addr;
	UINT32          fusion_weight_addr;

	UINT32          ref_in_addr_msb;
	UINT32          ref_out_addr_msb;
	UINT32          motion_in_addr_msb;
	UINT32          motion_out_addr_msb;
	UINT32          sigma_in_addr_msb;
	UINT32          sigma_out_addr_msb;
	UINT32          sta_out_addr_msb;
	UINT32          gamma_out_addr_msb;
	UINT32          fusion_weight_addr_msb;


	UINT32          subimg_in_addr;  ///< sub img in
	UINT32          out_addr;        ///< output starting address


	UINT32          va_out_addr;     ///< output va starting address
	UINT32          in_addr0_msb;        ///< input starting address 0
	UINT32          in_addr1_msb;        ///< input starting address 1

	UINT32          out_addr_msb;        ///< output starting address


	UINT32          va_out_addr_msb;     ///< output va starting address

	UINT32          width;           ///< image width
	UINT32          height;          ///< image height


	UINT32          crop_width;      ///< crop image width
	UINT32          crop_height;     ///< crop image height
	UINT32          crop_hpos;       ///< crop image horizontal start position
	UINT32          crop_vpos;       ///< crop image vertical start position
	UINT32          in_ofs0;         ///< image input lineoffset0
	UINT32          in_ofs1;         ///< image input lineoffset1

	UINT32          out_ofs;         ///< image output lineoffset


	UINT32          va_out_ofs;      ///< image output lineoffset

	UINT32			ref_in_ofs;
	UINT32          ref_out_ofs;
	UINT32			motion_in_ofs;
	UINT32          motion_out_ofs;
	UINT32			sigma_in_ofs;
	UINT32          sigma_out_ofs;

	UINT32			sta_out_ofs;
	UINT32          gamma_out_ofs;
	UINT32          fusion_weight_ofs;

	UINT32          intr_en;         ///< interrupt enable
	UINT8           fusion_number;   ///< Input path

	UINT8           tone_curve_en;   ///< tone_curve_en


	UINT8 lpf_coef[3];

	PRE_ENG_BNR_PARAM bnr_set;

	PRE_ENG_VA_PARAM va_set;

	PRE_ENG_FPN_PARAM fpn_set;

	PRE_CGAINSET pre_cgain;

	PRE_ENG_SIGLE_OUT_PARAM single_out_set;

	BOOL axi_disable;
	UINT32 axi_channel_en;
	UINT32 axi_lock_dis;
	UINT32 axi_r_ostd_num;
	UINT32 axi_w_ostd_num;

	BOOL dma_out_en;

	UINT32 pre_dram_indep_add;


} PRE_PARAM;
//@}


//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
extern void pre_eng_set_int_burst_length_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BURST_LENGTH *p_burst_len);

extern void pre_eng_set_enable_int_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 intr);

extern void pre_eng_set_global_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL pre_global_load);

extern void pre_eng_set_op_mode_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_OPMODE mode);

extern void pre_eng_set_out_sel_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_OUT_SEL out_sel);

extern void pre_eng_set_outlier_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 outlier_enable);

extern void pre_eng_set_mirror_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 mirror_enable);

extern void pre_eng_set_flip0_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 flip0_enable);

extern void pre_eng_set_flip1_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 flip1_enable);

extern void pre_eng_set_thermal_mode_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 thermal_mode);

extern void pre_eng_set_decode_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 decode_enable);

extern void pre_eng_set_decode2_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 decode_enable);

extern void pre_eng_set_nrs0_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 nrs0_enable);

extern void pre_eng_set_bnr_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_enable);

extern void pre_eng_set_bnr_ref_out_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL bnr_ref_out_enable);

extern void pre_eng_set_bnr_sta_out_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL bnr_sta_out_enable);

extern void pre_eng_set_bnr_sigma_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL bnr_sigma_enable);

extern void pre_eng_set_bnr_gamma_map_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL bnr_gamma_map_enable);

extern void pre_eng_set_bnr_ref_enc_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_bnr_ref_dec_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_bnr_err_comp_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);



extern void pre_eng_set_fcurve_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 fcurve_enable);

extern void pre_eng_set_fcgain_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 fcgain_enable);

extern void pre_eng_set_fusion_en_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 fusion_enable);

extern void pre_eng_set_fnum_buf_reg(PRE_ENG_HANDLE *p_eng, UINT8 fusion_num);

extern void pre_eng_set_stripe_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_STRIPE stripe_info);

extern void pre_eng_set_cfa_pat_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_CFASEL cfa_pat);

extern void pre_eng_set_cfa_pat_2_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_CFASEL cfa_pat);

extern void pre_eng_set_bayer_fmt_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BAYERFMTSEL bayer_fmt_sel);

extern void pre_eng_set_bitsel_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BITDEPTH in_bit_depth, PRE_BITDEPTH out_bit_depth);

extern void pre_eng_set_16bit_fmt_sel_reg(PRE_ENG_HANDLE *p_eng, BOOL in_16bit_fmt_sel);

extern void pre_eng_set_ring_buf_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RING_BUF *ring_info);

extern void pre_eng_set_ring_buf_chk_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RING_BUF *ring_info);

extern void pre_eng_set_dmaloop_line_count_clear_buf_reg(PRE_ENG_HANDLE *p_eng);

extern void pre_eng_set_dmaloop_sie2_line_count_reload_buf_reg(PRE_ENG_HANDLE *p_eng);

extern void pre_eng_set_dich_line_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_DICH_SYNC_BUF *dich_sync_info);

extern void pre_eng_set_sync_dich_disable_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_DICH_SYNC_BUF *dich_sync_info);

extern void pre_eng_set_dma_wait_sie2_start_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_DICH_SYNC_BUF *dich_sync_info);

extern void pre_eng_set_rde_default(PRE_ENG_HANDLE *p_eng, PRE_RDESET *p_rde, PRE_RDE_ADJMODE_SEL rde_mode);

#if (defined(_NVT_EMULATION_) == ON)

extern void pre_eng_set_rde_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RDESET *p_rde);

extern void pre_eng_set_rde_rate_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RDESET *p_rde);

extern void pre_eng_set_rde_qtable_inx_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RDESET *p_rde);

extern void pre_eng_set_rde_rand_init_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_RDESET *p_rde);

#endif

extern void pre_eng_set_dma_in_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 in_addr, UINT32 in_addr_msb, UINT32 line_ofs);

extern void pre_eng_set_dma_in_addr_2_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 in_addr, UINT32 in_addr_msb, UINT32 line_ofs);

extern void pre_eng_set_dma_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 out_addr, UINT32 out_addr_msb, UINT32 line_ofs);

extern void pre_eng_set_in_size_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 hsize, UINT32 vsize);

extern void pre_eng_set_crop_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 crop_hsize, UINT32 crop_vsize, UINT32 crop_hstart, UINT32 crop_vstart);

extern void pre_eng_set_outlier_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_OUTLSET *p_outlier);

extern void pre_eng_set_nrs0_str_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_NRSSET *p_nrs);

extern void pre_eng_set_nrs1_str_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_NRSSET *p_nrs);

extern void pre_eng_set_fcurve_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fcurve_yweight_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fcurve_l_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fcurve_r_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fcurve_end_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fcurve_end_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCURVESET *p_fcurve);

extern void pre_eng_set_fusion_debug_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FUSIONSET *p_fusion);

extern void pre_eng_set_fusion_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FUSIONSET *p_fusion);

extern void pre_eng_set_fusion_blend_curve(PRE_ENG_HANDLE *p_eng, PRE_FUSIONSET *p_fusion);

extern void pre_eng_set_fusion_diff_weight_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FUSIONSET *p_fusion);

extern void pre_eng_set_fusion_dark_saturation_reduction_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FUSIONSET *p_fusion);

extern void pre_eng_set_fusion_color_gain_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_FCGAINSET *p_fcgain);

extern void pre_eng_set_fusion_weight_out_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 fusion_weight_enable, UINT32 fusion_weight_wait_en);

extern void pre_eng_set_fusion_weight_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 fu_wet_out_addr, UINT32 fu_wet_out_addr_msb, UINT32 fu_wet_lofs);

extern void pre_eng_set_va_output_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 va_out_addr, UINT32 va_out_addr_msb, UINT32 va_out_lofs);

extern void pre_eng_set_va_gamma_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_GAMMA *set_gamma);

extern void pre_eng_set_va_filter_sel_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_FILTER_SEL *filter_sel);

extern void pre_eng_set_va_shift_bit_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_SHIFT_BIT *shift_sel);

extern void pre_eng_set_vacc_enable_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void pre_eng_set_force_enable_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void pre_eng_set_vacc_win_enable_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 set0_en, UINT32 set1_en, UINT32 set2_en, UINT32 set3_en, UINT32 set4_en);

extern void pre_eng_set_va_out_sel_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL en);

extern void pre_eng_set_va_in_sel_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_IN_SEL sel);

extern void pre_eng_set_va_vertical_fir_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL en);

extern void pre_eng_set_va_filter_g1_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_FLTR_GROUP_PARAM *p_va_fltr_g1);

extern void pre_eng_set_va_filter_g2_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);

extern void pre_eng_set_va_mode_enable_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_FLTR_GROUP_PARAM *p_va_fltr_g1, PRE_VA_FLTR_GROUP_PARAM *p_va_fltr_g2);

extern void pre_eng_set_va_win_info_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_WIN_PARAM *p_va_win);

extern void pre_eng_set_va_indep_win_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_INDEP_VA_PARAM  *p_indep_va_win_info, UINT32 win_idx);

extern void pre_eng_set_va_common_info_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_COMMON_PARAM *lpPara);

extern void pre_eng_set_va_ldg_enable_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 set_en);

extern void pre_eng_set_va_ldg_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_VA_LDG_PARAM *lpPara);

extern void pre_eng_set_dma_in_addr_ofs_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 in_addr_ofs);

extern void pre_eng_set_dma_in_addr_2_ofs_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 in_addr_ofs);

extern void pre_eng_set_dma_out_addr_ofs_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 out_addr_ofs);

extern void pre_eng_set_va_out_addr_ofs_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 out_addr_ofs);

extern void pre_eng_set_dma_out_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_fpn_para_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_ENG_FPN_PARAM *p_fpn);

extern ER pre_eng_get_size_limit(UINT32 bnr_en, UINT32 crop_width, UINT32 crop_height);

extern void pre_eng_get_stripe_limit(UINT32 pre_mode, UINT32 bnr_en, PRE_STRIPE_LIMIT* stp_limit);

extern void pre_eng_set_single_out_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_ENG_SIGLE_OUT_PARAM p_single);

extern void pre_eng_set_color_gain_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_CGAINSET *p_cgain);

extern void pre_eng_set_bnr_codec_default(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec, PRE_BNR_DECODE_ADJMODE_SEL rde_mode);

#if (defined(_NVT_EMULATION_) == ON)

extern void pre_eng_set_bnr_ref_enc_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

extern void pre_eng_set_bnr_ref_dec_ctrl_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

extern void pre_eng_set_bnr_codec_rate_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

extern void pre_eng_set_bnr_codec_qtable_inx_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

extern void pre_eng_set_bnr_decode_rand_init_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

extern void pre_eng_set_bnr_enc_dct_level_th_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_REF_CODEC_SET *p_codec);

#endif

extern void pre_eng_set_bnr_ref_in_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_ref_in_addr, UINT32 bnr_ref_in_msb, UINT32 bnr_ref_in_lofs);

extern void pre_eng_set_bnr_motion_bit_in_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_mot_bit_in_addr, UINT32 bnr_mot_bit_in_msb, UINT32 bnr_mot_bit_in_lofs);

extern void pre_eng_set_bnr_sigma_in_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_sigma_in_addr, UINT32 bnr_sigma_in_msb, UINT32 bnr_sigma_in_lofs);

extern void pre_eng_set_bnr_ref_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_ref_out_addr, UINT32 bnr_ref_out_msb, UINT32 bnr_ref_out_lofs);

extern void pre_eng_set_bnr_motion_bit_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_mot_bit_out_addr, UINT32 bnr_mot_bit_out_msb, UINT32 bnr_mot_bit_out_lofs);

extern void pre_eng_set_bnr_sigma_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_sigma_out_addr, UINT32 bnr_sigma_out_msb, UINT32 bnr_sigma_out_lofs);

extern void pre_eng_set_bnr_sta_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_sta_out_addr, UINT32 bnr_sta_out_msb, UINT32 bnr_sta_out_lofs);

extern void pre_eng_set_bnr_gamma_out_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 bnr_gamma_out_addr, UINT32 bnr_gamma_out_msb, UINT32 bnr_gamma_out_lofs);

extern void pre_eng_set_bnr_gamma_wait_en_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_bnr_aided_map_direct_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_bnr_debug_mode_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 dbg_mode);

extern void pre_eng_set_bnr_sigma_option_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 sigma_opt);

extern void pre_eng_set_bnr_prefilter_str_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 prefilter_str);

extern void pre_eng_set_bnr_err_comp_l_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_ERR_COMP *err_comp);

extern void pre_eng_set_bnr_err_comp_r_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_ERR_COMP *err_comp);

extern void pre_eng_set_bnr_err_comp_sft_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_ERR_COMP *err_comp);

extern void pre_eng_set_bnr_noise_profile_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_NP_COEF *np_coef);

extern void pre_eng_set_bnr_ps_down_sample_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_PS_DS_TH *ds_th);

extern void pre_eng_set_bnr_fusion_str_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_FUSION_STR *fusion_str);

extern void pre_eng_set_bnr_fusion_residue_th_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 residue_th);

extern void pre_eng_set_bnr_md_th_lut_l_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_MD_PARAM *md_para);

extern void pre_eng_set_bnr_md_th_lut_r_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_MD_PARAM *md_para);

extern void pre_eng_set_bnr_md_th_base_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_MD_PARAM *md_para);

extern void pre_eng_set_bnr_md_th_coef_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_MD_PARAM *md_para);

extern void pre_eng_get_bnr_md_th_coef_buf_reg(PRE_ENG_HANDLE *p_eng, PRE_BNR_MD_PARAM *md_para);

extern ER pre_eng_cal_bnr_extra_buffer_info(PRE_BNR_BUF_SIZE_INFO *p_bnr_param, UINT32 pre_crop_width, UINT32 pre_crop_height);

extern ER pre_eng_cal_fusion_weight_buffer_info(PRE_FUSION_WEIGHT_BUF_INFO *p_fusion_weight_param, UINT32 pre_crop_width, UINT32 pre_crop_height);

extern void pre_eng_set_hdr_sram_shutdown_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_va_sram_shutdown_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_bnr_sram_shutdown_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_outlier_sram_shutdown_buf_reg(PRE_ENG_HANDLE *p_eng, BOOL enable);

extern void pre_eng_set_dma_ll_addr_buf_reg(PRE_ENG_HANDLE *p_eng, UINT32 in_addr, UINT32 in_addr_msb);




//-------------------------------------------------------


#endif

//@}
