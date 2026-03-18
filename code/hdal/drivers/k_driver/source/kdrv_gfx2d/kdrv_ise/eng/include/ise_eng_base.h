#ifndef _ISE_ENG_BASE_H_
#define _ISE_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "kwrap/nvt_type.h"
#include "ise_eng_handle.h"

#define ISE_ENG_STRIPE_SIZE_MAX (0x100)

typedef enum {
	ISE_ENG_INTERRUPT_LLEND        = 0x00000001,
	ISE_ENG_INTERRUPT_LLERR        = 0x00000002,
	ISE_ENG_INTERRUPT_FMD          = 0x80000000,
	ISE_ENG_INTERRUPT_ALL   = 0x80000003,
	ENUM_DUMMY4WORD(ISE_ENG_INTERRUPT)
} ISE_ENG_INTERRUPT;

typedef enum {
	ISE_ENG_SCALE_BILINEAR = 	1,
	ISE_ENG_SCALE_NEAREST  = 	2,
	ISE_ENG_SCALE_INTEGRATION = 3,
	ENUM_DUMMY4WORD(ISE_ENG_SCALE_METHOD)
} ISE_ENG_SCALE_METHOD;

typedef enum {
	ISE_ENG_SCALE_DOWN  = 0,
	ISE_ENG_SCALE_UP	= 1,
	ENUM_DUMMY4WORD(ISE_ENG_SCALE_UD)
} ISE_ENG_SCALE_UD;

typedef enum {
	ISE_ENG_IOFMT_Y8          = 0,    ///< Y-8bit packing
	ISE_ENG_IOFMT_Y4          = 1,    ///< Y-4bit packing
	ISE_ENG_IOFMT_Y1          = 2,    ///< Y-1bit packing
	ISE_ENG_IOFMT_UVP         = 3,    ///< UV-Packed
	ISE_ENG_IOFMT_RGB565      = 4,    ///< RGB565 packing
	ISE_ENG_IOFMT_ARGB8888    = 5,    ///< ARGB8888
	ISE_ENG_IOFMT_ARGB1555    = 6,    ///< ARGB1555
	ISE_ENG_IOFMT_ARGB4444    = 7,    ///< ARGB4444
	ISE_ENG_IOFMT_YUVP        = 8,    ///< YUV-packed

	ENUM_DUMMY4WORD(ISE_ENG_IO_FMT)
} ISE_ENG_IO_FMT;

typedef enum {
	ISE_ENG_OUTMODE_ARGB8888 = 0, ///< Output ARGB8888
	ISE_ENG_OUTMODE_RGB8888 = 1,  ///< Output RGB888
	ISE_ENG_OUTMODE_A8 = 2,       ///< Output A8
	ENUM_DUMMY4WORD(ISE_ENG_ARGB_OUTMODE)
} ISE_ENG_ARGB_OUTMODE;

typedef enum {
	ISE_ENG_BURST_32W = 0,  ///< burst 32 word
	ISE_ENG_BURST_16W = 1,  ///< burst 16 word
	ENUM_DUMMY4WORD(ISE_ENG_BURST_LENGTH)
} ISE_ENG_BURST_LENGTH;

typedef struct _ISE_ENG_ALPHA_SCALE_INFO{
	UINT8 alpha_scl_method;
	UINT8 alpha_th;
} ISE_ENG_ALPHA_SCALE_INFO;

typedef struct _ISE_ENG_SCALE_FACTOR{
	UINT32 h_scl_ud;
	UINT32 h_scl_down_rate;
	UINT32 h_scl_factor;
	UINT32 v_scl_ud;
	UINT32 v_scl_down_rate;
	UINT32 v_scl_factor;
} ISE_ENG_SCALE_FACTOR;

/*typedef struct _ISE_ENG_ISD2_SCALE_INFO{
	INT32  isd2_user_coef[32];
	UINT32 isd2_h_norm;
	UINT32 isd2_v_norm;
	UINT8  isd2_coef_mode;
	UINT8  isd2_filt_adj_h;
	UINT8  isd2_filt_adj_v;
} ISE_ENG_ISD2_SCALE_INFO;*/


typedef struct _ISE_ENG_ISD_SCALE_FACTOR{
	UINT32 isd_coef_mode;

	UINT32 isd_scl_coef_num_h;
	UINT32 isd_scl_base_h;
	UINT32 isd_scl_factor_h[3];
	INT32  isd_user_coef_h[17];

	UINT32 isd_scl_coef_num_v;
	UINT32 isd_scl_base_v;
	UINT32 isd_scl_factor_v[3];
	INT32  isd_user_coef_v[17];

} ISE_ENG_ISD_SCALE_FACTOR;

typedef struct {
	INT32 isd_coef_sum_all_h;
	INT32 isd_coef_sum_half_h;

	INT32 isd_coef_sum_all_v;
	INT32 isd_coef_sum_half_v;
} ISE_ENG_ISD_COEF_SUM;


typedef struct {
	UINT32 h_scl_ofs_int;
	UINT32 v_scl_ofs_int;
	UINT32 h_scl_ofs_dec;
	UINT32 v_scl_ofs_dec;
} ISE_ENG_SCALE_OFFSET;


typedef struct _ISE_ENG_SCALE_FILTER_INFO{
	BOOL   h_filt_en;
	UINT32 h_filt_coef;
	BOOL   v_filt_en;
	UINT32 v_filt_coef;
} ISE_ENG_SCALE_FILTER;

typedef struct _ISE_ENG_STRIPE_INFO{
	UINT32 overlap_mode;
	UINT32 overlap_size_sel;
	UINT32 stripe_size;
} ISE_ENG_STRIPE_INFO;


typedef struct _ISE_ENG_CTRL_INFO{
	UINT32 io_fmt;
	UINT32 scl_method;
	UINT32 argb_out_mode;
	UINT32 in_burst;
	UINT32 out_burst;
	UINT32 ovlp_mode;
} ISE_ENG_CTRL_INFO;

typedef struct _ISE_ENG_INPUT_SIZE_PARAM{
	UINT16 in_h_size;
	UINT16 in_v_size;
}ISE_ENG_INPUT_SIZE_PARAM;

typedef struct _ISE_ENG_OUTPUT_SIZE_PARAM{
	UINT16 out_h_size;
	UINT16 out_v_size;
}ISE_ENG_OUTPUT_SIZE_PARAM;

extern INT32 ise_eng_cal_scale_factor(UINT32 in_width, UINT32 in_height, UINT32 out_width, UINT32 out_height, UINT32 scl_method, ISE_ENG_SCALE_FACTOR *p_scl_factor, ISE_ENG_ISD_SCALE_FACTOR *p_isd_para);
extern INT32 ise_eng_cal_scale_filter(UINT32 in_width, UINT32 in_height, UINT32 out_width, UINT32 out_height, ISE_ENG_SCALE_FILTER *p_scl_filter);
extern INT32 ise_eng_cal_stripe(UINT32 in_width, UINT32 out_width, UINT32 scl_method, ISE_ENG_STRIPE_INFO *p_stripe);

extern void ise_eng_set_scale_filter_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_SCALE_FILTER *p_scl_filter);
extern void ise_eng_set_alpha_scale_info_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ALPHA_SCALE_INFO *alpha_info);
extern void ise_eng_set_scale_rate_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_SCALE_FACTOR *scale_factor);
extern void ise_eng_set_scale_method_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ISD_SCALE_FACTOR *isd_scale_factor);
extern void ise_eng_set_scl_ofs_buf_reg(ISE_ENG_HANDLE *p_eng, ISE_ENG_SCALE_OFFSET *p_scl_ofs);
//extern void ise_eng_set_integration_scale_rate_buf_reg (ISE_ENG_ISD_SCALE_FACTOR *isd_scale_factor);
//extern void ise_eng_set_integration_scale_coef_buf_reg (ISE_ENG_ISD_SCALE_FACTOR *isd_scale_coef);
extern void ise_eng_set_isd_mode_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ISD_SCALE_FACTOR *isd_scale_coef);
extern void ise_eng_set_isd_coef_num_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ISD_SCALE_FACTOR *isd_scale_coef);
extern void ise_eng_set_isd_scale_coef_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ISD_SCALE_FACTOR *isd_scale_coef);
extern void ise_eng_set_ctrl_buf_reg(ISE_ENG_HANDLE *p_eng, ISE_ENG_CTRL_INFO *ise_ctrl);
extern void ise_eng_set_in_size_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 width, UINT32 height);
extern void ise_eng_set_in_lofs_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 lofs);
extern void ise_eng_set_in_addr_buf_reg(ISE_ENG_HANDLE *p_eng, ULONG phy_addr);
extern void ise_eng_set_in_strp_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 st_size, UINT32 ovlp_size);
extern void ise_eng_set_out_size_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 width, UINT32 height);
extern void ise_eng_set_out_lofs_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 lofs);
extern void ise_eng_set_out_addr_buf_reg(ISE_ENG_HANDLE *p_eng, ULONG phy_addr);
extern void ise_eng_set_int_en_buf_reg(ISE_ENG_HANDLE *p_eng, UINT32 inte_en);
extern void ise_eng_set_integration_scale_rate_buf_reg (ISE_ENG_HANDLE *p_eng, ISE_ENG_ISD_SCALE_FACTOR *isd_scale_factor);

extern void ise_eng_set_scl_isd_coef_h_all(ISE_ENG_HANDLE *p_eng,ISE_ENG_ISD_COEF_SUM *p_isd_coef_sum);
extern void ise_eng_set_scl_isd_coef_h_half(ISE_ENG_HANDLE *p_eng,ISE_ENG_ISD_COEF_SUM *p_isd_coef_sum);
extern void ise_eng_set_scl_isd_coef_v_all(ISE_ENG_HANDLE *p_eng,ISE_ENG_ISD_COEF_SUM *p_isd_coef_sum);
extern void ise_eng_set_scl_isd_coef_v_half(ISE_ENG_HANDLE *p_eng,ISE_ENG_ISD_COEF_SUM *p_isd_coef_sum);


#endif

