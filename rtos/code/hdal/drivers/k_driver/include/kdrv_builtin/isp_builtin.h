#ifndef _ISP_BUILTIN_H_
#define _ISP_BUILTIN_H_

#include "kwrap/type.h"
#if defined(__KERNEL__)
// TODO: isp builtin
#if 1
#include "linux/soc/nvt/rcw_macro.h"
#else
#include "mach/rcw_macro.h"
#endif
#endif

//=============================================================================
// struct & enum definition
//=============================================================================
#define ISP_BUILTIN_ID_MAX_NUM      5
#define ISP_SEN_MFRAME_MAX_NUM      2

typedef struct _ISP_BUILTIN_CGAIN {
	UINT32 r;
	UINT32 g;
	UINT32 b;
} ISP_BUILTIN_CGAIN;

typedef struct _ISP_BUILTIN_SHDR_EV_RATIO {
	UINT32 ratio[ISP_SEN_MFRAME_MAX_NUM];
} ISP_BUILTIN_SHDR_EV_RATIO;

typedef struct _ISP_BUILTIN_SENSOR_CTRL {
	UINT32 exp_time[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 gain_ratio[ISP_SEN_MFRAME_MAX_NUM];
} ISP_BUILTIN_SENSOR_CTRL;

//=============================================================================
// init parameter for RTOS builtin
//=============================================================================
#if defined(__FREERTOS)
typedef struct _ISP_BUILTIN_INIT_PARAM {
	UINT32 sensor_preset_name[2];
	UINT32 sensor_chgmode_fps[2];
	UINT32 sensor_preset_expt[2];
	UINT32 sensor_preset_gain[2];
	UINT32 sensor_expt_max[2];
	UINT32 sensor_i2c_id[2];
	UINT32 sensor_i2c_addr[2];
	UINT32 sensor_cur_mode[2];
	UINT32 sensor_row_time[2];
	UINT32 sensor_vd[2];
	UINT32 sensor_dft_fps[2];
	UINT32 sensor_frame_num[2];
	UINT32 sensor_max_gain[2];
	UINT32 isp_path[2];
	UINT32 isp_d_gain[2];
	UINT32 isp_r_gain[2];
	UINT32 isp_g_gain[2];
	UINT32 isp_b_gain[2];
	UINT32 isp_shdr_enable[2];
	UINT32 isp_shdr_path[2];
	UINT32 isp_shdr_mask[2];
	UINT32 isp_nr_lv[2];
	UINT32 isp_3dnr_lv[2];
	UINT32 isp_sharpness_lv[2];
	UINT32 isp_ssaturation_lv[2];
	UINT32 isp_contrast_lv[2];
	UINT32 isp_brightness_lv[2];
	UINT32 isp_night_mode[2];
} ISP_BUILTIN_INIT_PARAM;
#endif

//=============================================================================
// extern functions
//=============================================================================
extern UINT32 isp_builtin_get_i2c_id(UINT32 id);
extern UINT32 isp_builtin_get_i2c_addr(UINT32 id);
extern BOOL isp_builtin_get_shdr_enable(UINT32 id);
extern UINT32 isp_builtin_get_shdr_id_mask(UINT32 id);
extern UINT32 isp_builtin_get_sensor_name(UINT32 id);
extern UINT32 isp_builtin_get_chgmode_fps(UINT32 id);
extern UINT32 isp_builtin_get_sensor_expt_max(UINT32 id);
extern UINT32 isp_builtin_get_total_gain(UINT32 id);
extern UINT32 isp_builtin_get_ct(UINT32 id);
extern UINT32 isp_builtin_get_lv(UINT32 id);
extern UINT32 isp_builtin_get_shdr_tm_ratio(UINT32 id);
extern ISP_BUILTIN_SHDR_EV_RATIO *isp_builtin_get_shdr_ev_ratio(UINT32 id);
extern UINT32 isp_builtin_get_shdr_smoothed_tm_ratio(UINT32 id);
extern UINT32 isp_builtin_get_overexposure_offset(UINT32 id);
extern UINT32 isp_builtin_get_compensation_ratio(UINT32 id);
extern ISP_BUILTIN_CGAIN *isp_builtin_get_cgain(UINT32 id);
extern UINT32 isp_builtin_get_dgain(UINT32 id);
extern UINT32 isp_builtin_get_nr_lv(UINT32 id);
extern UINT32 isp_builtin_get_3dnr_lv(UINT32 id);
extern UINT32 isp_builtin_get_sharpness_lv(UINT32 id);
extern UINT32 isp_builtin_get_saturation_lv(UINT32 id);
extern UINT32 isp_builtin_get_contrast_lv(UINT32 id);
extern UINT32 isp_builtin_get_brightness_lv(UINT32 id);
extern UINT32 isp_builtin_get_night_mode(UINT32 id);
extern UINT32 isp_builtin_get_ae_stitch_mode(UINT32 id);
extern UINT32 isp_builtin_get_awb_stitch_mode(UINT32 id);
extern ISP_BUILTIN_SENSOR_CTRL *isp_builtin_get_sensor_gain(UINT32 id);
extern ISP_BUILTIN_SENSOR_CTRL *isp_builtin_get_sensor_expt(UINT32 id);
extern BOOL isp_builtin_get_sensor_valid(UINT32 id);
#if defined(__FREERTOS)
extern void isp_builtin_reg_i2c_cb(void *cb_fp);
extern void isp_builtin_ae_log(UINT32 id);

#endif
extern UINT32 isp_builtin_get_isp_version(void);
extern UINT32 isp_builtin_get_ae_version(void);
extern UINT32 isp_builtin_get_awb_version(void);
extern UINT32 isp_builtin_get_iq_version(void);
extern UINT32 isp_builtin_get_sensor_version(void);
extern void isp_builtin_get_ae_param(UINT32 id, ULONG *param_addr);
extern void isp_builtin_get_awb_param(UINT32 id, ULONG *param_addr);
extern void isp_builtin_get_iq_param(UINT32 id, ULONG *param_addr);
extern void isp_builtin_uninit_i2c(UINT32 id);
extern INT32 isp_builtin_set_transfer_i2c(UINT32 id, void *msgs, INT32 num);
extern void isp_builtin_set_cgain(UINT32 id, ISP_BUILTIN_CGAIN *value);
extern void isp_builtin_set_sensor_bypass(BOOL bypass);
extern void isp_builtin_set_cgain_bypass(BOOL bypass);

#if defined(__FREERTOS)
extern ER isp_builtin_init(ISP_BUILTIN_INIT_PARAM *init_param);
#endif
#endif

