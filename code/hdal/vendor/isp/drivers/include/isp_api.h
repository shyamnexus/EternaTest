#ifndef _ISP_API_
#define _ISP_API_

#if defined(__KERNEL__) || defined(__FREERTOS)
#include "kflow_common/isp_if.h"
#include "kflow_common/type_vdo.h"
#include "kflow_videocapture/ctl_sie_utility.h"
#include "isp_alg_if.h"
#endif

//=============================================================================
// struct & enum definition
//=============================================================================
#define ISP_CA_W_WINNUM             32
#define ISP_CA_H_WINNUM             32
#define ISP_CA_MAX_WINNUM           (ISP_CA_W_WINNUM * ISP_CA_H_WINNUM)
#define ISP_LA_W_WINNUM             32
#define ISP_LA_H_WINNUM             32
#define ISP_LA_MAX_WINNUM           (ISP_LA_W_WINNUM * ISP_LA_H_WINNUM)
#define ISP_LA_HIST_NUM             128
#define ISP_VA_W_WINNUM             8
#define ISP_VA_H_WINNUM             8
#define ISP_VA_MAX_WINNUM           (ISP_VA_W_WINNUM * ISP_VA_H_WINNUM)
#define ISP_INDEP_VA_WIN_NUM        5
#define ISP_HISTO_MAX_SIZE          128
#define ISP_DFG_AIRLIGHT_NUM        3
#define ISP_SUBOUT_W_WINNUM         32
#define ISP_SUBOUT_H_WINNUM         32
#define ISP_SUBOUT_MAX_SIZE         (ISP_SUBOUT_W_WINNUM * ISP_SUBOUT_H_WINNUM)
#define ISP_SEN_MFRAME_MAX_NUM      2
#define ISP_SEN_NAME_LEN            32
#define ISP_YUV_OUT_CH              6
#define ISP_CGAIN_NUM               3

typedef enum _ISP_AE_STATUS {
	ISP_AE_STATUS_STABLE     = 0,
	ISP_AE_STATUS_FINE       = 1,
	ISP_AE_STATUS_COARSE     = 2,
	ISP_AE_STATUS_LIMIT      = 3,
	ENUM_DUMMY4WORD(ISP_AE_STATUS)
} ISP_AE_STATUS;

typedef enum _ISP_SENSOR_PRESET_MODE {
	ISP_SENSOR_PRESET_DEFAULT,
	ISP_SENSOR_PRESET_CHGMODE,
	ISP_SENSOR_PRESET_AE,
	ENUM_DUMMY4WORD(ISP_SENSOR_PRESET_MODE)
} ISP_SENSOR_PRESET_MODE;

typedef enum _ISP_CHIP_ID {
	ISP_CHIP_ID_NT9852X,
	ISP_CHIP_ID_NT98528,
	ISP_CHIP_ID_NT98530,
	ISP_CHIP_ID_NT98560,
	ENUM_DUMMY4WORD(ISP_CHIP_ID)
} ISP_CHIP_ID;

typedef struct _ISP_ENC_ISP_RATIO {
	UINT32 enc_edge_ratio;         ///< Legal range : 0~1023, 256=1X
	UINT32 enc_2dnr_ratio;         ///< Legal range : 0~1023, 256=1X
	UINT32 enc_3dnr_ratio;         ///< Legal range : 0~1023, 256=1X
} ISP_ENC_ISP_RATIO;

typedef struct _ISP_RGBIR_INFO {
	UINT32 ir_level;               ///< Legal range : 0~255
	UINT32 saturation;             ///< Legal range : 0~1023
} ISP_RGBIR_INFO;

typedef struct _ISP_SENSOR_CTRL {
	UINT32 exp_time[ISP_SEN_MFRAME_MAX_NUM];
	UINT32 gain_ratio[ISP_SEN_MFRAME_MAX_NUM];
} ISP_SENSOR_CTRL;

typedef struct _ISP_SENSOR_PRESET {
	ISP_SENSOR_PRESET_MODE mode;
	UINT32 exp_time[ISP_SEN_MFRAME_MAX_NUM];    // ISP_SENSOR_PRESET_AE usage
	UINT32 gain_ratio[ISP_SEN_MFRAME_MAX_NUM];  // ISP_SENSOR_PRESET_AE usage
} ISP_SENSOR_PRESET_CTRL;

typedef struct _ISP_SENSOR_DIRECTION {
	BOOL mirror;
	BOOL flip;
} ISP_SENSOR_DIRECTION;

typedef struct _ISP_SENSOR_MODE_INFO {
	ISP_SEN_DATA_FMT data_fmt;
	ISP_SEN_MODE_TYPE mode_type;
	UINT32 max_gain;
	UINT32 min_gain;
	UINT32 min_expt;
	URECT act_size[ISP_SEN_MFRAME_MAX_NUM];
	USIZE crp_size;
	UINT32 row_time;
	UINT32 row_time_step;
	UINT32 line_length;
	UINT32 frame_length;
	UINT32 dft_fps;
} ISP_SENSOR_MODE_INFO;

typedef struct _ISP_SENSOR_MODE_MANUAL {
	BOOL enable;
	ISP_SEN_MODE_TYPE mode_type;
	UINT32 frame_num;
	ISP_SEN_DATA_FMT data_fmt;
} ISP_SENSOR_MODE_MANUAL;

typedef struct _ISP_FUNC_INFO {
	BOOL sie_valid;
	BOOL ipp_valid;
	BOOL ae_valid;
	BOOL af_valid;
	BOOL awb_valid;
	BOOL defog_valid;
	BOOL shdr_valid;
	BOOL wdr_valid;
	USIZE yuv_out_ch[ISP_YUV_OUT_CH];
} ISP_FUNC_INFO;

typedef struct _ISP_YUV_INFO {
	UINT32 pid;
	BOOL info_vaild;
	VDO_PXLFMT pxlfmt;
	ISIZE size;
	UINT32 pw[VDO_MAX_PLANE];
	UINT32 ph[VDO_MAX_PLANE];
	UINT32 loff[VDO_MAX_PLANE];
	ULONG addr[VDO_MAX_PLANE];
	ULONG psy_addr[VDO_MAX_PLANE];
} ISP_YUV_INFO;

typedef struct _ISP_RAW_INFO {
	BOOL info_vaild;
	VDO_PXLFMT pxlfmt;
	ISIZE size;
	UINT32 pw;
	UINT32 ph;
	UINT32 loff;
	ULONG addr;
	ULONG psy_addr;
} ISP_RAW_INFO;

typedef struct _ISP_SENSOR_INFO {
	CHAR name[ISP_ID_MAX_NUM][ISP_SEN_NAME_LEN];
	UINT32 src_id_mask[ISP_ID_MAX_NUM];
	UINT32 ipp_id_table[ISP_ID_MAX_NUM];
} ISP_SENSOR_INFO;

// 539A only
typedef struct _ISP_BNR_STA_INFO {
	BOOL info_vaild;
	UINT32 enable;
	ULONG buf_addr;
	ULONG buf_phyaddr;
	UINT32 lofs;
} ISP_BNR_STA_INFO;

typedef struct _ISP_3DNR_STA_INFO {
	BOOL info_vaild;
	UINT32 enable;
	ULONG buf_addr;
	UINT32 max_sample_num;
	UINT32 lofs;
	USIZE sample_step;
	USIZE sample_num;
	UPOINT sample_st;
} ISP_3DNR_STA_INFO;

typedef struct _ISP_MD_STA_INFO {
	BOOL vaild;
	UINT32 data[ISP_VA_MAX_WINNUM];
} ISP_MD_STA_INFO;

/**
	type for ISP_CA_RST
	ca result, array size should be ca window num_x * num_y
	user need to prepare memory for these pointer
	CA Output R/G/B/Cnt/IRth/Rth/Gth/Bth @16bit for each window
	each array size should be window num_x * num_y * 2(16bit)
*/
typedef struct _ISP_CA_RSLT {
	UINT16 r[ISP_CA_MAX_WINNUM];
	UINT16 g[ISP_CA_MAX_WINNUM];
	UINT16 b[ISP_CA_MAX_WINNUM];
	UINT16 ir[ISP_CA_MAX_WINNUM];
	UINT16 acc_cnt[ISP_CA_MAX_WINNUM];
} ISP_CA_RSLT;

/**
	type for ISP_LA_RST
	la result, array size should be la window num_x * num_y
	histogram array size = 64
	user need to prepare memory for these pointer
	p_buf_lum_1 buffer size: la_win_w * la_win_h * 2
	p_buf_lum_2 buffer size: la_win_w * la_win_h * 2
	p_buf_histogram buffer size: 64 * 2
*/
typedef struct _ISP_LA_RSLT {
	UINT16 lum_1[ISP_LA_MAX_WINNUM];    // pre gamma result
	UINT16 lum_2[ISP_LA_MAX_WINNUM];    // post-gamma result
	UINT16 histogram[ISP_LA_HIST_NUM];  // Only support SIE 1~4
} ISP_LA_RSLT;

/**
	type for CTL_IPP_ISP_IFE_VA_RST
	va result, array size should be window num_x * num_y
	user need to prepare memory for these pointer
*/
typedef struct _ISP_IFE_VA_RSLT {
	UINT32 lum[ISP_VA_MAX_WINNUM];
	UINT32 g1_h[ISP_VA_MAX_WINNUM];
	UINT32 g1_v[ISP_VA_MAX_WINNUM];
	UINT32 g2_h[ISP_VA_MAX_WINNUM];
	UINT32 g2_v[ISP_VA_MAX_WINNUM];
	UINT32 g1_h_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g1_v_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g2_h_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g2_v_cnt[ISP_VA_MAX_WINNUM];
} ISP_IFE_VA_RSLT;

/**
	type for CTL_IPP_ISP_IFE_VA_INDEP_RST
*/
typedef struct _ISP_IFE_VA_INDEP_RSLT {
	UINT32 lum[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_h[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_v[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_h[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_v[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_h_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_v_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_h_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_v_cnt[ISP_INDEP_VA_WIN_NUM];
} ISP_IFE_VA_INDEP_RSLT;

/**
	type for CTL_IPP_ISP_IPE_VA_RST
	va result, array size should be window num_x * num_y
	user need to prepare memory for these pointer
*/
typedef struct _ISP_IPE_VA_RSLT {
	UINT32 g1_h[ISP_VA_MAX_WINNUM];
	UINT32 g1_v[ISP_VA_MAX_WINNUM];
	UINT32 g2_h[ISP_VA_MAX_WINNUM];
	UINT32 g2_v[ISP_VA_MAX_WINNUM];
	UINT32 g1_h_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g1_v_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g2_h_cnt[ISP_VA_MAX_WINNUM];
	UINT32 g2_v_cnt[ISP_VA_MAX_WINNUM];
} ISP_IPE_VA_RSLT;

/**
	type for CTL_IPP_ISP_IPE_VA_INDEP_RST
*/
typedef struct _ISP_IPE_VA_INDEP_RSLT {
	UINT32 g1_h[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_v[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_h[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_v[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_h_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g1_v_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_h_cnt[ISP_INDEP_VA_WIN_NUM];
	UINT32 g2_v_cnt[ISP_INDEP_VA_WIN_NUM];
} ISP_IPE_VA_INDEP_RSLT;

/**
	type for KDRV_DCE_PARAM_IPL_HIST_RSLT
*/
typedef struct _ISP_HISTO_RSLT {
	UINT16 hist_stcs_pre_wdr[ISP_HISTO_MAX_SIZE];           ///< histogram statistics
	UINT16 hist_stcs_post_wdr[ISP_HISTO_MAX_SIZE];          ///< histogram statistics
} ISP_HISTO_RSLT;

/**
	type for KDRV_IPE_PARAM_IPL_DEFOG_STCS_RSLT
*/
typedef struct _ISP_DEFOG_STCS {
	UINT16 dfg_airlight[ISP_DFG_AIRLIGHT_NUM];         ///< the defog statistics
} ISP_DEFOG_STCS;

typedef struct _ISP_IPE_SUBOUT_BUF {
	UINT16 min[ISP_SUBOUT_MAX_SIZE];     ///< Legal range : 0~1023, the min value of block in subout image
	UINT16 avg[ISP_SUBOUT_MAX_SIZE];     ///< Legal range : 0~1023, the avg value of block in subout image
} ISP_IPE_SUBOUT_BUF;

typedef struct _ISP_AE_INIT_INFO {
	UINT32 lv;
	UINT32 total_gain;
	UINT32 d_gain;
	UINT32 expt_max;
	UINT32 overexposure_offset;
	UINT32 stitch_mode;
	UINT32 shdr_tm_ratio;
	UINT32 shdr_ev_ratio[ISP_SEN_MFRAME_MAX_NUM];
	ULONG  param_addr;
	UINT32 compensation_ratio;
} ISP_AE_INIT_INFO;

typedef struct _ISP_AWB_INIT_INFO {
	UINT32 ct;
	UINT32 r_gain;
	UINT32 g_gain;
	UINT32 b_gain;
	UINT32 stitch_mode;
	ULONG  param_addr;
} ISP_AWB_INIT_INFO;

typedef struct _ISP_IQ_INIT_INFO {
	UINT32 nr_lv;
	UINT32 _3dnr_lv;
	UINT32 sharpness_lv;
	UINT32 saturation_lv;
	UINT32 contrast_lv;
	UINT32 brightness_lv;
	UINT32 night_mode;
	ULONG  param_addr;
} ISP_IQ_INIT_INFO;

typedef struct _ISP_SENSOR_INIT_INFO {
	UINT32 expt;
	UINT32 gain;
} ISP_SENSOR_INIT_INFO;

//=============================================================================
// extern functions
//=============================================================================
#if defined(__KERNEL__) || defined(__FREERTOS)
extern void isp_api_reg_if(UINT32 id_list);
extern void isp_api_unreg_if(void);
extern void isp_api_reg_enc_if(void);
extern void isp_api_unreg_enc_if(void);
extern ER isp_api_get_expt(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl);
extern ER isp_api_get_gain(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl);
extern ER isp_api_get_sync_time(ISP_ID id, UINT32 *sync_time);
extern ER isp_api_get_frame_num(ISP_ID id, BOOL is_from_sie, UINT32 *frame_num);
extern ER isp_api_get_chg_fps(ISP_ID id, UINT32 *frame_num);
extern ER isp_api_get_sensor_mode_info(ISP_ID id, ISP_SENSOR_MODE_INFO *mode_param);
extern ER isp_api_get_md_sta(ISP_ID id, ISP_MD_STA_INFO *md_sta);
extern ER isp_api_get_ir_info(ISP_ID id, ISP_RGBIR_INFO *ir_info);
extern ER isp_api_get_enc_isp_ratio(ISP_ID id, ISP_ENC_ISP_RATIO *enc_isp_ratio);
extern ISP_SENSOR_INIT_INFO *isp_api_get_fastboot_sensor_info(void);
extern BOOL isp_api_get_fastboot_valid(void);
extern BOOL isp_api_get_fastboot_sensor_valid(UINT32 id);
extern ER isp_api_set_expt(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl);
extern ER isp_api_set_gain(ISP_ID id, ISP_SENSOR_CTRL *sensor_ctrl);
extern ER isp_api_set_preset(ISP_ID id, ISP_SENSOR_PRESET_CTRL *preset_ctrl);
extern void isp_api_set_iq_param(ISP_ID id, ISP_IQ_ITEM isp_iq_item, void *param);
#endif

#endif
