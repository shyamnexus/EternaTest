#ifndef _IQ_LIB_H_
#define _IQ_LIB_H_

//=============================================================================
// define
//=============================================================================
// WDR related define
#define IQLIB_WDR_DARK_TH                    10
#define IQLIB_WDR_DR_TH                      60
#define IQLIB_WDR_DARK_COMPENSATE            2
#define IQLIB_HISTO_MAX_SIZE                 128

// Defog related define
#define IQLIB_DEFOG_AIRLIGHT_TH              1024 / 3
#define IQLIB_DEFOG_AIRLIGHT_DIFF_MAX_RATIO  30
#define IQLIB_DEFOG_CNT_RATIO_TH1            5
#define IQLIB_DEFOG_CNT_RATIO_TH2            40
#define IQLIB_DFG_AIRLIGHT_NUM               3
#define IQLIB_SUBOUT_MAX_SIZE                (32 * 32)

// HDR related define
#define IQLIB_SHDR_FCURVE_LEFT_NUM           65
#define IQLIB_SHDR_FCURVE_RIGHT_NUM          17
#define IQLIB_SHDR_FCURVE_END_NUM            16
#define IQLIB_TONE_LEFT_NUM                  65
#define IQLIB_TONE_RIGHT_NUM                 17
#define IQLIB_GAMMA_LEN                      129

#define IQLIB_SHDR_FCURVE_LINEAR_BOUND_EN    0
#define IQLIB_SHDR_HBS_W_MAX                 100
#define IQLIB_SHDR_HBS_TABLE_NUM             (IQLIB_SHDR_FCURVE_RIGHT_NUM + IQLIB_SHDR_FCURVE_LEFT_NUM + IQLIB_SHDR_FCURVE_END_NUM)

//=============================================================================
// struct & enum definition
//=============================================================================
typedef struct _IQLIB_WDR {
	UINT32 dark_th;                                  ///< [Input] Range : 0~255
	UINT32 dr_th;                                    ///< [Input] Range : 0~255
	UINT32 dark_compensate;                          ///< [Input]
	UINT16 *hist_stcs_pre_wdr;                       ///< [Input]
	BOOL   dbg_en;                                   ///< [Input]
	UINT32 dr_level;                                 ///< [Output]
} IQLIB_WDR;

typedef struct _IQLIB_DEFOG_AIRLIGHT {
	UINT32 th;                                       ///< [Input]
	UINT32 diff_max_ratio;                           ///< [Input]
	UINT16 airlight[IQLIB_DFG_AIRLIGHT_NUM];         ///< [Input]
	BOOL   dbg_en;                                   ///< [Input]
	UINT16 r;                                        ///< [Output]
	UINT16 g;                                        ///< [Output]
	UINT16 b;                                        ///< [Output]
} IQLIB_DEFOG_AIRLIGHT;

typedef struct _IQLIB_DEFOG {
	UINT16 fog_level;
	UINT8 outbld_lum_wt;
	UINT8 gain_th;
} IQLIB_DEFOG;

typedef struct _IQLIB_DEFOG_STRENGTH {
	UINT16 dr_th;                                    ///< [Input]
	UINT16 fog_mod_level_lb;                         ///< [Input]
	UINT32 subimg_size_w;                            ///< [Input]
	UINT32 subimg_size_h;                            ///< [Input]
	UINT32 cnt_ratio_th1;                            ///< [Input]
	UINT32 cnt_ratio_th2;                            ///< [Input]
	UINT16 *subout_min;                              ///< [Input]
	UINT16 *subout_avg;                              ///< [Input]
	UINT16 *dynamic_range;                           ///< [Input]
	BOOL   dbg_en;                                   ///< [Input]
	IQLIB_DEFOG result;                              ///< [Output]
} IQLIB_DEFOG_STRENGTH;

typedef struct _IQLIB_HBS {
	UINT32 lum_th;                ///< range 0~100
	UINT32 w_start;               ///< range 0~100, 0: original Fcurve, 100: linear Fcurve
	UINT32 w_slope;               ///< range 0~1000, 100: slope = -1, 200: slope = -2
} IQLIB_HBS;

typedef struct _IQLIB_SHDR_FCURVE_INFO {
	BOOL dbg_en;
	BOOL auto_ev_en;
	BOOL is_8ev;
	UINT32 frame_num;
	UINT32 ev_ratio;
	UINT32 tm_ratio;
	IQLIB_HBS hbs_param;
	UINT32 *left_lut;   // length = IQ_SHDR_FCURVE_LEFT_NUM
	UINT32 *right_lut;  // length = IQ_SHDR_FCURVE_RIGHT_NUM
	UINT32 *end_lut;    // length = IQ_SHDR_FCURVE_END_NUM
} IQLIB_SHDR_FCURVE_INFO;

typedef struct _IQLIB_SHDR_FCURVE {
	UINT8 ev_fmt;
	UINT32 *left_lut;   // length = IQ_SHDR_FCURVE_LEFT_NUM
	UINT32 *right_lut;  // length = IQ_SHDR_FCURVE_RIGHT_NUM
	UINT32 *end_lut;    // length = IQ_SHDR_FCURVE_END_NUM
} IQLIB_SHDR_FCURVE;

//=============================================================================
// extern functions
//=============================================================================
extern INT32 iq_intpl(INT32 index, INT32 l_value, INT32 h_value, INT32 l_index, INT32 h_index);
extern void iq_intpl_tbl_uint8(UINT8 *l_value, UINT8 *h_value, INT32 size, UINT8 *target, INT32 index, INT32 l_index, INT32 h_index);
extern void iq_intpl_tbl_uint16(UINT16 *l_value, UINT16 *h_value, INT32 size, UINT16 *target, INT32 index, INT32 l_index, INT32 h_index);
extern void iq_intpl_tbl_uint32(UINT32 *l_value, UINT32 *h_value, INT32 size, UINT32 *target, INT32 index, INT32 l_index, INT32 h_index);
extern UINT64 iq_uint64_div(UINT64 numerator, UINT64 denominator);

extern void iq_lib_gamma_detone(UINT16 *tone_l, UINT16 *tone_r, UINT32 *degamma);
extern void iq_lib_wdr_strength(IQLIB_WDR *wdr);
extern void iq_lib_defog_airlight(IQLIB_DEFOG_AIRLIGHT *defog_airlight);
extern void iq_lib_defog_strength(IQLIB_DEFOG_STRENGTH *defog_strength);
extern void iq_lib_defog_fog_mod_level_to_lut(UINT16 level, UINT16 *lut);
extern void iq_lib_shdr_fcurve(IQLIB_SHDR_FCURVE_INFO *input, IQLIB_SHDR_FCURVE *output);
extern UINT32 iq_lib_get_fcurve_y(UINT32 x);

#endif
