#ifndef _IME_ENG_INT_CAL_H_
#define _IME_ENG_INT_CAL_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __KERNEL__
#include <linux/version.h>

#include "kwrap/type.h"
//#include <mach/rcw_macro.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
#include <linux/soc/nvt/rcw_macro.h>
#else
#include "mach/rcw_macro.h"
#endif

#include "kwrap/error_no.h"
#elif defined(__FREERTOS)
#include "kwrap/type.h"
#include "rcw_macro.h"
#include "kwrap/error_no.h"
#else
#endif

#include "ime_eng_comm.h"
#include "ime_eng_int.h"

typedef struct _IME_PM_LINE_COEFS_ {
    INT32 coefs[4];
} IME_PM_LINE_COEFS;



/**
    IME structure - privacy mask coordinate
*/
typedef struct _IME_PM_POINT_ {
	INT32 coord_x; ///< coordinate of horizontal direction
	INT32 coord_y; ///< coordinate of vertical direction
} IME_PM_POINT;


typedef struct _IME_TMNR_STATISTIC_PARAM_ {
	UINT32 sample_step_hori;  ///< step size of statistical sampling in horizontal direction
	UINT32 sample_step_vert;  ///< step size of statistical sampling in vertical direction
	UINT32 sample_num_x;   ///< total number of statistical sampling in horizontal direction
	UINT32 sample_num_y;   ///< total number of statistical sampling in vertical direction
	UINT32 sample_st_x;    ///< start point of horizontal direction for statistical sampling process
	UINT32 sample_st_y;    ///< start point of vertical direction for statistical sampling process
} KDRV_IME_TMNR_STATISTIC_PARAM;

typedef struct _IME_TMNR_BUF_SIZE_INFO_ {
	UINT32 in_size_h;               ///< input image width
	UINT32 in_size_v;               ///< input image height

	UINT32 in_sta_max_num;          ///< statistic data maxima number

	UINT32 get_mv_lofs;             ///< get motion vector buffer lineoffset
	UINT32 get_mv_size;             ///< get motion vector buffer size

	UINT32 get_ms_lofs;             ///< get motion status buffer lineoffset
	UINT32 get_ms_size;             ///< get motion status buffer size

	UINT32 get_ms_roi_lofs;         ///< get motion status buffer lineoffset for ROI
	UINT32 get_ms_roi_size;         ///< get motion status buffer size for ROI

	KDRV_IME_TMNR_STATISTIC_PARAM get_sta_param;    ///< get statistic parameters
	UINT32 get_sta_lofs;                            ///< get statistic data buffer lineoffset
	UINT32 get_sta_size;                            ///< get statistic data buffer size

	UINT32 get_fcp_lofs;            ///< get fast converge buffer lineoffset
	UINT32 get_fcp_size;            ///< get fast converge buffer size
} IME_TMNR_BUF_SIZE_INFO;

typedef struct _IME_TMNR_PATCH_SEL_INFO_ {
	UINT32 mix_ratio[2];   ///< Mix ratio in patch selection
	UINT32 mix_th[2];  ///< Mix threshold in patch selection
	UINT32 edge_wet;   ///< Start point of edge adjustment
	UINT32 edge_th[2];    ///< Edge adjustment threshold in patch selection

	UINT32 mix_slope[2]; ///< Mix slope in patch selection
	UINT32 edge_slope;  ///< Edge adjustment slope in patch selection
} IME_TMNR_PATCH_SEL_INFO;


extern UINT32 get_coef_val[32];

extern UINT32 ime_int_to_2comp(INT32 val, INT32 bits);

extern UINT32 ime_eng_cal_scaling_isd_init_kernel_number(UINT32 in_size, UINT32 out_size);

extern VOID ime_eng_cal_convex_hull_coefs(IME_PM_POINT *p_pm_cvx_point, INT32 pnt_num, IME_PM_LINE_COEFS *p_line_coefs);

extern BOOL ime_eng_verify_isd_output_size(UINT32 stp_max, UINT32 in_size_h, UINT32 out_size_h);

extern ER ime_eng_cal_tmnr_extra_buffer_info(IME_TMNR_BUF_SIZE_INFO *p_tmnr_param);

extern ER ime_eng_cal_tmnr_patch_selection_info(IME_TMNR_PATCH_SEL_INFO *p_tmnr_ps_param);

extern ER ime_cal_pm_image_scale_factor_params(IME_SIZE_INFO *p_in_img_size, IME_SIZE_INFO *p_out_img_size, UINT32 scale_method, IME_SCALE_FACTOR_INFO *p_scl_factor_info);

#ifdef __cplusplus
}
#endif


#endif


