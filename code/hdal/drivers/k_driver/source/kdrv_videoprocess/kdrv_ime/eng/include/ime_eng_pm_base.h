

#ifndef _IME_ENG_PM_BASE_H_
#define _IME_ENG_PM_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "kwrap/type.h"
#include "ime_eng_int_comm.h"


/**
    IME enum - privacy mask set number selection
*/
typedef enum _IME_PM_SET_SEL_ {
	IME_PM_SET0 = 0,  ///< set0
	IME_PM_SET1 = 1,  ///< set1
	IME_PM_SET2 = 2,  ///< set2
	IME_PM_SET3 = 3,  ///< set3
	IME_PM_SET4 = 4,  ///< set4
	IME_PM_SET5 = 5,  ///< set5
	IME_PM_SET6 = 6,  ///< set6
	IME_PM_SET7 = 7,  ///< set7
	ENUM_DUMMY4WORD(IME_PM_SET_SEL)
} IME_PM_SET_SEL;

/**
    IME enum - privacy mask type
*/
typedef enum _IME_PM_MASK_TYPE_ {
	IME_PM_MASK_TYPE_YUV = 0,  ///< Using YUV color
	IME_PM_MASK_TYPE_PXL = 1,  ///< Using pixilation
	ENUM_DUMMY4WORD(IME_PM_MASK_TYPE)
} IME_PM_MASK_TYPE;


/**
    IME enum - privacy mask pixelation source selection
*/
typedef enum _IME_PM_PXL_SRC_SEL_ {
	IME_PM_PIXELATION_DRAM = 0,     ///< from DRAM
	IME_PM_PIXELATION_LCA  = 1,     ///< from LCA
	IME_PM_PIXELATION_CURF = 2,     ///< from current frame
	ENUM_DUMMY4WORD(IME_PM_PXL_SRC_SEL)
} IME_PM_PXL_SRC_SEL;

/**
    IME enum - privacy mask pixelation blocks size selection
*/
typedef enum _IME_PM_PXL_BLK_SIZE_ {
	IME_PM_PIXELATION_08 = 0,  ///< 8x8
	IME_PM_PIXELATION_16 = 1,  ///< 16x16
	IME_PM_PIXELATION_32 = 2,  ///< 32x32
	IME_PM_PIXELATION_64 = 3,  ///< 64x64
	ENUM_DUMMY4WORD(IME_PM_PXL_BLK_SIZE)
} IME_PM_PXL_BLK_SIZE;

/**
    IME enum - privacy mask processing location selection
*/
typedef enum _IME_PM_PROC_LOCATION_ {
	IME_PM_PROC_LOCATION_PRE_3DNR = 0,      ///< pre-3DNR
	IME_PM_PROC_LOCATION_PRE_SCROP = 1,     ///< scale pre-crop
	ENUM_DUMMY4WORD(IME_PM_PROC_LOCATION)
} IME_PM_PROC_LOCATION;


typedef enum _IME_PM_CONVEX_POLYGON_SEL_ {
	IME_PM_CONVEX_POLYGON_HOLLOW    = 0,
	IME_PM_CONVEX_POLYGON_4         = 1,
	IME_PM_CONVEX_POLYGON_6         = 2,
	IME_PM_CONVEX_POLYGON_8         = 3,
	IME_PM_CONVEX_POLYGON_10        = 4,
	ENUM_DUMMY4WORD(IME_PM_CONVEX_POLYGON_SEL)
} IME_PM_CONVEX_POLYGON_SEL;


/**
    IME structure - privacy mask using color mask
*/
typedef struct _IME_PM_MASK_COLOR_ {
	UINT32 pm_color_y; ///< Y channel value, range: [0, 255]
	UINT32 pm_color_u; ///< U channel value, range: [0, 255]
	UINT32 pm_color_v; ///< V channel value, range: [0, 255]
} IME_PM_MASK_COLOR;



extern VOID ime_eng_set_privacy_mask_pxl_image_input_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 size_h, UINT32 size_v);
extern VOID ime_eng_set_privacy_mask_pxl_image_format_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_fmt);
extern VOID ime_eng_set_privacy_mask_pxl_image_input_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 lofs);
extern VOID ime_eng_set_privacy_mask_pxl_image_input_dma_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 addr_lsb32, UINT32 addr_msb32);

extern VOID ime_eng_set_privacy_mask_pxl_image_output_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 lofs);
extern VOID ime_eng_set_privacy_mask_pxl_image_output_dma_addr_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 addr_lsb32, UINT32 addr_msb32);

extern VOID ime_eng_set_privacy_mask_pxl_image_output_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 size_h, UINT32 size_v);
extern VOID ime_eng_set_privacy_mask_pxl_subout_enable_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_en);
extern VOID ime_eng_set_privacy_mask_pxl_blk_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_size);


extern VOID ime_eng_set_privacy_mask_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, IME_FUNC_EN set_en);
extern VOID ime_eng_set_privacy_mask_type_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, IME_PM_MASK_TYPE set_type);
extern VOID ime_eng_set_privacy_mask_color_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, IME_PM_MASK_COLOR *set_color);
extern VOID ime_eng_set_privacy_mask_weight_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, UINT32 set_wet);
extern VOID ime_eng_set_privacy_mask_line0_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);
extern VOID ime_eng_set_privacy_mask_line1_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);
extern VOID ime_eng_set_privacy_mask_line2_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);
extern VOID ime_eng_set_privacy_mask_line3_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);
extern VOID ime_eng_set_privacy_mask_line4_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);
extern VOID ime_eng_set_privacy_mask_line5_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, INT32 *p_coefs);

extern VOID ime_eng_set_privacy_mask_polygon_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel, IME_PM_CONVEX_POLYGON_SEL set_poly_sel);

extern VOID ime_eng_set_privacy_mask_pxl_scale_factor_buf_reg(IME_ENG_HANDLE *p_eng, IME_SCALE_FACTOR_INFO *p_set_factor);
extern VOID ime_eng_set_privacy_mask_pxl_source_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_PXL_SRC_SEL set_src);


extern VOID ime_eng_set_privacy_mask_process_localtion_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_PROC_LOCATION set_localtion);

extern VOID ime_eng_set_fisheye_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_FUNC_EN set_en);
extern VOID ime_eng_set_fisheye_center_pos_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_cent_x, UINT32 set_cent_y);
extern VOID ime_eng_set_fisheye_process_range_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_valid_r, UINT32 set_decs_r);
extern VOID ime_eng_set_fisheye_gain_divide_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 set_div_mul, UINT32 set_div_sft);


#if (IME_GET_API_EN == 1)
extern IME_FUNC_EN ime_eng_get_privacy_mask_enable_buf_reg(IME_ENG_HANDLE *p_eng, IME_PM_SET_SEL set_sel);

extern VOID ime_eng_get_privacy_mask_pxl_image_size_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_size_h, UINT32 *p_size_v);
extern VOID ime_eng_get_privacy_mask_pxl_image_lineoffset_buf_reg(IME_ENG_HANDLE *p_eng, UINT32 *p_lofs);
extern VOID ime_eng_get_privacy_mask_pxl_image_dma_addr_buf_reg(IME_ENG_HANDLE *p_eng, ULONG *p_addr);
#endif


#ifdef __cplusplus
}
#endif

#endif

