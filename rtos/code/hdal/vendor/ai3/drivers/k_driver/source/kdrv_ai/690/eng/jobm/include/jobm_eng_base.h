#ifndef _JOBM_ENG_BASE_H_
#define _JOBM_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "jobm_eng_handle.h"


typedef enum _JOBM_ENG_ID {
	JOBM_ID  = 0,
	JOBM_ID_MAX_NUM,
	ENUM_DUMMY4WORD(JOBM_ENG_ID)
} JOBM_ENG_ID;

/*
typedef struct {
    uintptr_t addr;
    UINT32 lnfst;
} IVE_ENG_DRAM_INFO;


typedef enum {
	IVE_ENG_PATH_CANNY     = 0,
	IVE_ENG_PATH_VA        = 1,
    IVE_ENG_PATH_CSC       = 2,
    IVE_ENG_PATH_HISTOGRAM = 3,
    IVE_ENG_PATH_NCC       = 4,
    IVE_ENG_PATH_CPY       = 5,
    IVE_ENG_PATH_THRES     = 6,
    IVE_ENG_PATH_IMG_OP    = 7,
    IVE_ENG_PATH_BIT16TO8  = 8,
    IVE_ENG_PATH_CCL       = 9,
    IVE_ENG_PATH_ST_CORNER = 10,
	IVE_ENG_PATH_LBP       = 11,
	IVE_ENG_PATH_MAX
} IVE_ENG_PATH_SEL;

typedef enum _IVE_ENG_ID {
	IVE_ID  = 0,
	IVE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(IVE_ENG_ID)
} IVE_ENG_ID;

typedef enum{
    IVE_ENG_CANNY_GEN_FILT_EN    = 0x1,
    IVE_ENG_CANNY_ORST_FILT_EN   = 0x2,
    IVE_ENG_CANNY_EDGE_FILT_EN   = 0x4,
    IVE_ENG_CANNY_NON_MAX_SUP_EN = 0x8,
    IVE_ENG_CANNY_THRES_EN       = 0x10,
    IVE_ENG_CANNY_POSTPROC_EN    = 0x20,
    IVE_ENG_CANNY_FUNC_MAX
}IVE_ENG_CANNY_FUNC;

typedef enum{
    IVE_ENG_VA_GEN_FILT_EN    = 0x1,
    IVE_ENG_VA_MAP_EN         = 0x2,
    IVE_ENG_VA_EDGE_FILT_EN   = 0x4,
    IVE_ENG_VA_THRES_EN       = 0x8,
    IVE_ENG_VA_INTEGRAL_EN    = 0x10,
    IVE_ENG_VA_FUNC_MAX
}IVE_ENG_VA_FUNC;

typedef struct {
    UINT8 enable;
    UINT8 coeff[10];
	UINT8 input_fmt;
	UINT8 input_sub_ratio;
} IVE_ENG_GEN_FILT;

typedef enum{
    IVE_ENG_ORST_MODE_MEDIAN = 0,
    IVE_ENG_ORST_MODE_MAX    = 1,
    IVE_ENG_ORST_MODE_MIN    = 2,
}IVE_ENG_ORST_MODE;


typedef struct {
    UINT8 enable;
    IVE_ENG_ORST_MODE mode;
} IVE_ENG_ORST_FILT;

typedef enum{
    IVE_ENG_EDGE_MODE_BI_DIR         = 0,
    IVE_ENG_EDGE_MODE_NO_DIR         = 1,
	IVE_ENG_EDGE_MODE_ALPHA_BLENDING = 2,
}IVE_ENG_EDGE_MODE;

typedef enum{
    IVE_ENG_EDGE_KER_5X5             = 0,
    IVE_ENG_EDGE_KER_7X7             = 1,
}IVE_ENG_EDGE_KERNEL_MODE;

typedef struct {
    UINT8 enable;
    IVE_ENG_EDGE_MODE mode;
	IVE_ENG_EDGE_KERNEL_MODE ker_mode;
    UINT8 shift_bit;
    INT8 coeff[2][25];
    UINT8 angle_slp;
	UINT8 alpha_blending_factor;
} IVE_ENG_EDGE_FILT;


typedef struct {
    UINT8 enable;
    UINT8 edge_mag_th;
} IVE_ENG_NON_MAX_SUP;


typedef enum{
    IVE_ENG_INTGAL_IN_Y = 0,
    IVE_ENG_INTGAL_IN_UV = 1,
    IVE_ENG_INTGAL_IN_HSV_RGB = 2,
}IVE_ENG_INTGAL_IN_FMT;

typedef enum{
    IVE_ENG_INTGAL_OUT_SUM = 0,
    IVE_ENG_INTGAL_OUT_SQUARE = 1,
    IVE_ENG_INTGAL_OUT_SQUARE_SUM = 2,
}IVE_ENG_INTGAL_OUT_FMT;

typedef struct {
    UINT8 enable;
    IVE_ENG_INTGAL_IN_FMT in_fmt;
    IVE_ENG_INTGAL_OUT_FMT out_fmt;
	UINT8 out_sub_ratio;
} IVE_ENG_INTEGRAL_IMG;

typedef struct {
    UINT8 th_min_val;
    UINT8 th_mid_val;
    UINT8 th_max_val;
} IVE_ENG_THRES_TH;

typedef struct {
    UINT8 enable;
    UINT8 mode; // 1~5 for canny & 1~11 for Lut path
    //UINT8 tap[15]; // only for mode 0
    UINT16 low_th;
    UINT16 high_th;
    IVE_ENG_THRES_TH th_val;
} IVE_ENG_THRES;

typedef enum{
    IVE_ENG_POSTPROC_MODE_MORPH = 0,
    IVE_ENG_POSTPROC_MODE_HYSTER = 1,
}IVE_ENG_POSTPROC_MODE;

typedef enum{
    IVE_ENG_POSTPROC_MORPH_DILATION = 0,
    IVE_ENG_POSTPROC_MORPH_EROSION = 1,
}IVE_ENG_POSTPROC_MORPH_OP;


typedef struct {
    UINT8 enable;
    IVE_ENG_POSTPROC_MODE mode;
    IVE_ENG_POSTPROC_MORPH_OP morph_op;
    UINT8 mask_en_bit[24]; // 0~1
	UINT8 hyst_low_th;
	UINT8 hyst_high_th;
} IVE_ENG_POST_PROC;

typedef enum{
    IVE_ENG_CANNY_OUT_8BIT      = 0,
    IVE_ENG_CANNY_OUT_12BIT     = 1,
    IVE_ENG_CANNY_OUT_16BIT     = 2, 
    IVE_ENG_CANNY_OUT_20BIT     = 3, 
    IVE_ENG_CANNY_OUT_32BIT     = 4, 
    IVE_ENG_CANNY_OUT_8BIT_2CH  = 5, 
    IVE_ENG_CANNY_OUT_16BIT_2CH = 6, 
}IVE_ENG_CANNY_OUT_SEL;

typedef struct {
    //IVE_ENG_CANNY_FUNC func_en;
    IVE_ENG_GEN_FILT gen_filter;
    IVE_ENG_ORST_FILT orst_filter;
    IVE_ENG_EDGE_FILT edge_filter;
    IVE_ENG_NON_MAX_SUP non_max_sup;
    IVE_ENG_THRES threshold;
    IVE_ENG_POST_PROC post_proc;
    IVE_ENG_CANNY_OUT_SEL out_sel;
} IVE_ENG_CANNY_PATH;

typedef enum{
    IVE_ENG_MAP_MODE0     = 0,
    IVE_ENG_MAP_MODE1     = 1,
}IVE_ENG_MAP_MODE;

typedef struct {
    UINT8 enable;
	IVE_ENG_MAP_MODE mode;
	UINT8 index_shift;
} IVE_ENG_MAP;

typedef struct {
    IVE_ENG_GEN_FILT gen_filter;
    IVE_ENG_MAP map;
    IVE_ENG_EDGE_FILT edge_filter;
    IVE_ENG_THRES threshold;
    IVE_ENG_INTEGRAL_IMG integral;
    UINT8 input_fmt;
} IVE_ENG_VA_PATH;


typedef enum{
    IVE_ENG_CSC_IN_YUV420 = 0,
    IVE_ENG_CSC_IN_YVU420 = 1,
    IVE_ENG_CSC_IN_UYVY = 2,
    IVE_ENG_CSC_IN_YUYV = 3,
    IVE_ENG_CSC_IN_VYUY = 4,
    IVE_ENG_CSC_IN_YVYU = 5,
}IVE_ENG_CSC_YUV_IN_FMT;

typedef enum{
    IVE_ENG_CSC_OUT_RGB_PLANAR = 0,
    IVE_ENG_CSC_OUT_RGB_PACKED = 1,
    IVE_ENG_CSC_OUT_HSV_PLANAR = 2,
    IVE_ENG_CSC_OUT_HSV_PACKED = 3,
    IVE_ENG_CSC_OUT_LAB_PLANAR = 4,
    IVE_ENG_CSC_OUT_LAB_PACKED = 5,
}IVE_ENG_CSC_YUV_OUT_FMT;

typedef enum{
    IVE_ENG_CSC_LAB_D65 = 0,
    IVE_ENG_CSC_LAB_D50 = 1,
}IVE_ENG_CSC_LAB_FMT;

typedef struct {
    IVE_ENG_CSC_YUV_IN_FMT in_fmt;
    IVE_ENG_CSC_YUV_OUT_FMT out_fmt;
    UINT8 hue_shift; // 0~127
    IVE_ENG_CSC_LAB_FMT lab_fmt;
    UINT8 gamma_en; // 0~1
    INT16 coef[9]; // -16384 ~ 16383
    INT16 bias[3]; // -256 ~ 255
} IVE_ENG_CSC_PATH;


typedef enum{
    IVE_ENG_HIST_MODE_0 = 0, // output histogram to addr0
    IVE_ENG_HIST_MODE_1 = 1, //  output histogram to addr0 and keep cdf in register
    IVE_ENG_HIST_MODE_2 = 2, //  output histogram to addr0 and output cdf in addr1
	IVE_ENG_HIST_MODE_3 = 3,
}IVE_ENG_HIST_MODE;


typedef struct {
    IVE_ENG_HIST_MODE mode;
    //UINT32 hist_cdf_min; // when hist_mode=1, the output result will keep in this variable
	UINT32 coef_a;
    UINT32 coef_b;
    UINT8 shift_bit;
} IVE_ENG_HISTOGRAM_PATH;


typedef struct {
    UINT32 numerator_lsb;
    UINT32 numerator_msb;
    UINT32 quad_sum0_lsb;
    UINT32 quad_sum0_msb;
    UINT32 quad_sum1_lsb;
    UINT32 quad_sum1_msb;
} IVE_ENG_NCC_PATH;


typedef struct {
    UINT8 hor_seg_size;
    UINT16 ver_seg_size;
    UINT8 element_size;
    UINT16 ver_seg_num;
} IVE_ENG_DMA_CPY_PATH;

typedef enum{
    IVE_ENG_IMG_OP_ADD = 0, // (coef_a * in0 + coef_b * in1) >> shift_bit
    IVE_ENG_IMG_OP_SUB = 1, // abs(in0-in1) or (in0 - in1) >> 1
    IVE_ENG_IMG_OP_AND = 2, // in0 & in1
    IVE_ENG_IMG_OP_OR  = 3, // in0 | in1
    IVE_ENG_IMG_OP_XOR = 4, // in0 ^ in1
}IVE_ENG_IMG_OP_MODE;

typedef enum{
    IVE_ENG_IMG_SUB_ABS = 0,
    IVE_ENG_IMG_SUB_IN0_SUB_IN1 = 1, // (in0 - in1) >> 1
}IVE_ENG_IMG_SUB_MODE;


typedef struct {
    IVE_ENG_IMG_OP_MODE mode;
    IVE_ENG_IMG_SUB_MODE sub_mode;
    UINT32 coef_a;
    UINT32 coef_b;
    UINT8 shift_bit;
} IVE_ENG_IMG_OP_PATH;


typedef struct {
    UINT8 mode; // 0~4
    UINT32 coef_a;
    UINT32 coef_b;
    INT8 coef_bias;
    UINT8 shift_bit;
} IVE_ENG_BIT16_TO_8_PATH;


typedef enum{
    IVE_ENG_CCL_8_CONN = 0,
    IVE_ENG_CCL_4_CONN = 1,
}IVE_ENG_CCL_MODE;

typedef enum{
    IVE_ENG_CCL_TRIG_1ST = 0,
    IVE_ENG_CCL_TRIG_2ND = 1,
}IVE_ENG_CCL_TRIG_MODE;

typedef struct {
    IVE_ENG_CCL_MODE mode;
    IVE_ENG_CCL_TRIG_MODE trig_mode;
    UINT8 region_x_num;
    UINT16 region_y_num; // 1~256
    UINT8 foregjobmnd_val;
    UINT16 region_w;
    UINT16 region_h;
} IVE_ENG_CCL_PATH;


typedef struct {
    UINT8 mode; // 0~1
    UINT8 quality_lvl;
    UINT8 min_dist;
	UINT8 blk_dist;
	UINT16 stripe_blk_x_num;
	UINT16 blk_x_num;
	UINT16 blk_y_num;
	UINT32 first_stage_point_num;
    UINT16 max_corner_num;
    UINT16 sort_topn;
    UINT16 max_eigen_val;
} IVE_ENG_ST_PATH;

typedef enum{
    IVE_ENG_LBP_NORMAL = 0,
    IVE_ENG_LBP_ABS    = 1,
}IVE_ENG_LBP_CMP_MODE;

typedef struct {
    IVE_ENG_LBP_CMP_MODE mode;
    UINT8 threshold;
} IVE_ENG_LBP_PATH;

typedef struct {
    UINT16 img_width;
    UINT16 img_height;
    IVE_ENG_DRAM_INFO in0;
    IVE_ENG_DRAM_INFO in1;
    IVE_ENG_DRAM_INFO out0;
    IVE_ENG_DRAM_INFO out1;
    IVE_ENG_DRAM_INFO out2;
    IVE_ENG_PATH_SEL ive_path;
    IVE_ENG_CANNY_PATH canny_path;
	IVE_ENG_VA_PATH va_path;
    IVE_ENG_CSC_PATH csc_path;
    IVE_ENG_HISTOGRAM_PATH hist_path;
    IVE_ENG_NCC_PATH ncc_path;
    IVE_ENG_DMA_CPY_PATH dma_cpy_path;
    IVE_ENG_THRES thres_path;
    IVE_ENG_IMG_OP_PATH img_op_path;
    IVE_ENG_BIT16_TO_8_PATH bit16_to_8_path;
    IVE_ENG_CCL_PATH ccl_path;
    IVE_ENG_ST_PATH st_path;
	IVE_ENG_LBP_PATH lbp_path;
} IVE_ENG_STRUCT;


extern VOID ive_eng_set_img_size(IVE_ENG_HANDLE *p_eng, UINT32 img_w, UINT32 img_h);
extern VOID ive_eng_set_func_mode(IVE_ENG_HANDLE *p_eng, IVE_ENG_PATH_SEL path);
extern VOID ive_eng_set_canny_func_enable(IVE_ENG_HANDLE *p_eng, UINT32 Enable_func);
extern VOID ive_eng_set_va_func_enable(IVE_ENG_HANDLE *p_eng, UINT32 Enable_func);
extern VOID ive_eng_set_va_input_fmt(IVE_ENG_HANDLE *p_eng, UINT32 input_fmt);
extern VOID ive_eng_set_gen_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_GEN_FILT gen_filt);
extern VOID ive_eng_set_orst_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_ORST_FILT orst_filt);
extern VOID ive_eng_set_edge_filt(IVE_ENG_HANDLE *p_eng, IVE_ENG_EDGE_FILT edge_filt);
extern VOID ive_eng_set_non_max_sup(IVE_ENG_HANDLE *p_eng, IVE_ENG_NON_MAX_SUP non_max_sup);
extern VOID ive_eng_set_integral_img(IVE_ENG_HANDLE *p_eng, IVE_ENG_INTEGRAL_IMG integral_img);
extern VOID ive_eng_set_thres(IVE_ENG_HANDLE *p_eng, IVE_ENG_THRES thres);
extern VOID ive_eng_set_map(IVE_ENG_HANDLE *p_eng, IVE_ENG_MAP map);
//extern VOID ive_eng_set_thres_lut_mode0_tap(IVE_ENG_HANDLE *p_eng, UINT8 *tap);
extern VOID ive_eng_set_post_proc(IVE_ENG_HANDLE *p_eng, IVE_ENG_POST_PROC postproc);
extern VOID ive_eng_set_canny_out_fmt(IVE_ENG_HANDLE *p_eng, IVE_ENG_CANNY_OUT_SEL out_fmt);
extern VOID ive_eng_set_path_sel(IVE_ENG_HANDLE *p_eng, IVE_ENG_PATH_SEL path_sel);
extern VOID ive_eng_set_csc_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_CSC_PATH csc_path);
extern VOID ive_eng_set_histogram_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_HISTOGRAM_PATH hist_path);
extern UINT32 ive_eng_get_hist_cdf_min(IVE_ENG_HANDLE *p_eng);
extern IVE_ENG_NCC_PATH ive_eng_get_ncc_rlt(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_set_dma_cpy_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_DMA_CPY_PATH dma_cpy_path);
extern VOID ive_eng_set_img_op_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_IMG_OP_PATH img_op_path);
extern VOID ive_eng_set_bit16_to_8_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_BIT16_TO_8_PATH bit16_8_path);
extern VOID ive_eng_set_ccl_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_CCL_PATH ccl_path);
extern VOID ive_eng_set_st_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_ST_PATH st_path);
extern UINT16 ive_eng_get_st_corner_num(IVE_ENG_HANDLE *p_eng);
extern UINT16 ive_eng_get_st_max_eigen_value(IVE_ENG_HANDLE *p_eng);
extern VOID ive_eng_set_lbp_path(IVE_ENG_HANDLE *p_eng, IVE_ENG_LBP_PATH lbp_path);
*/
#endif

