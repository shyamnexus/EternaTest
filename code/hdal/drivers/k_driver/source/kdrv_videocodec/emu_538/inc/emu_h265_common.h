#ifndef _EMU_H265_COMMON_H_
#define _EMU_H265_COMMON_H_

#include "kwrap/type.h"

// for hw trigger //
#include "h26x.h"

// for sw driver //
#include "h26x_def.h"
#include "h26xenc_api.h"
#include "h265enc_api.h"

// for emulation //
#include "emu_h26x_common.h"
#include "kdrv_videoenc/kdrv_videoenc_lmt.h"

#define H265_AUTO_JOB_SEL	      (0) //0
#define H265_TEST_MAIN_LOOP       (0) // enable H265_AUTO_JOB_SEL

#define H265_LINEOFFSET_MAX 6144

typedef enum{
	FF_RDO_265 = 0,
	FF_VAR_265,
	FF_FRO_265,
	FF_MASK_265,
	FF_GDR_265,
	FF_ROI_265,
	FF_RRC_SEQ_265,
	FF_RRC_PIC_265,
	FF_SRAQ_265,	// 8
	FF_LPM_265,
	FF_RND_265,
	FF_SCD_265,
	FF_TMNR_265,
	FF_TMNR_SCE_265,
	FF_OSG_265,
	FF_MAQ_265,
	FF_JND_265,		//16
	FF_BGR_265,
	FF_DITHER_265,
	FF_RMD_265,
	FF_TNR_265,
	FF_LAMBDA_265,
	FF_ESKIP_265,
	FF_SPN_265,
	FF_QP_RELATED_265,	// 24
	FF_SHA256_265,
	FF_TURBO_265,
	FF_SLICE_MODE_265,
	FF_RES_SUP_265,
	FF_GMV_265,
	FF_IMV_CTRL_265,
	FF_MOT_FILTER_265,
	FF_MOT_DETECT_265,
	FF_PART_SKIP_265,
	FF_WATER_MARK_265,
	FF_MAX_NUM_265 = 48
}e_FPGA_FUNC_265; // FF = FPGA_FUNC

typedef struct _file_t_265_{
	H26XFile info;
	H26XFile src;
	H26XFile es;
	H26XFile ilf_sideinfo;
	H26XFile slice_hdr_len;
	H26XFile slice_hdr_es;
	H26XFile nal_len;
	H26XFile mbqp;
	H26XFile tnr_out;
	H26XFile colmv;

	H26XFile tmnr_ref;
	H26XFile tmnr_mot;
	H26XFile tmnr_out;
	H26XFile tmnr_mot_out;
	H26XFile tmnr_dbg_sum;

	H26XFile osg_grap[32];
	H26XFile seq;
	H26XFile pic;
	H26XFile chk;
	H26XFile motion_bit;

	H26XFile video_hdr_len;
	H26XFile video_hdr_es; // vps + sps + pps

	//sdec
	H26XFile sdebsy;
	H26XFile sdebsuv;
	H26XFile sde_reg;

	H26XFile rrc_info;
}file_t_265;

typedef struct _info_t_265_{
	unsigned char yuv_name[256];
	// b'0: dump src, b'1: sdec enable, b'2-3: sdec rotation, b'4: sdec cb cr swap, b'5: 32x8 block mode
	unsigned int dump_src_en;
	unsigned int width;
	unsigned int height;
	unsigned int frame_num;
	unsigned int cmd_len;
}info_t_265;

typedef struct _seq_t_265_{
	unsigned int obj_size[FF_MAX_NUM_265];

	unsigned int width;
	unsigned int height;

	unsigned char tile_en;
	unsigned char tile_num;
	unsigned short tile_width[H265E_TILE_MAX];

	unsigned int dis_loopfilter_idc;
	int dblk_alpha;
	int dblk_beta;

	int chrm_qp_offset;
	int sec_chrm_qp_offset;

	unsigned char svc_layer;
	unsigned int ltr_interval;
	unsigned char ltr_ref_pre;

	unsigned char sao;
	unsigned char sao_range_ext_en;
	unsigned char sao_merge_value;

	unsigned char min_qp;
	unsigned char max_qp;

	unsigned char fbc_en;
	unsigned char gray_en;
	unsigned char gray_mode_color_en;
	unsigned char fastsearch_en;
	unsigned int  slice_row_num;

	// fpga only (hack parameter) : differ from driver settings  //
	unsigned char sdecmps_en;
	unsigned char sdecmps_rotate;
	unsigned char sdecmps_cbcr_iv;
	unsigned char sdecmps_blk_mode;
	unsigned char hw_pad_en;
	unsigned char flxsr_en;
	unsigned char rec_sr_en;
	unsigned short srh_rng_en;
	unsigned short rec_srh_rng_en;

	unsigned char log2_mincu_size;
	unsigned char cu_qp_delta_en;
	unsigned char save_delta_qp;

	unsigned char ira_idrect_mode_wt;

	//unsigned char ime_left_amvp_mode;
	unsigned char tmvp_en;
	unsigned char jnd_en;

	unsigned char str_filter_flag;
	unsigned char rc_cost_mode;

	unsigned char mbqp_rot;
	unsigned char simple_merge;
	unsigned char tu4_disable;
	unsigned char tu32_disable;
	unsigned char i8_pred_en;

	unsigned int yuv_width;
	unsigned int yuv_height;

	unsigned char sps_log2_max_poc_lsb_minus4;
	unsigned int slice_number;

	unsigned char inter_only;
	unsigned char ime_get_intra_mv;
	unsigned char debreath;
	unsigned char ime_amvp_mode;
	unsigned char ilf_lpm;
	unsigned char ime_amvp_mvcost;
} seq_t_265;

typedef struct _pic_t_265_{
	unsigned char slice_type;
	unsigned char qp;
	unsigned int  total_slice_hdr_len;
	unsigned char rec_out;
	unsigned char disable_smart_rec;
	unsigned char fskip_en;
	unsigned short dist_scale_factor;
	unsigned char ime_dist_scale;
	unsigned char sao_luma_en;
	unsigned char sao_chroma_en;
	unsigned short sao_cb_lambda;
	unsigned short sao_cr_lambda;

	unsigned char pic_sum_num_lt;
	unsigned char st_ref_pic_sets;
	unsigned char lt_ref_pic_sets;
	unsigned char pic_num_neg_pics;
	unsigned char pic_num_pos_pics;
	unsigned char pic_num_delta_poc;
	unsigned char pic_num_lt_sps;

	unsigned char pps_deblocking_filter_control_present_flag;
	unsigned char pps_deblocking_filter_override_enable_flag;

	unsigned char pps_slice_chroma_qp_offset_present_flag;


	// fpga only //
	unsigned char mask_en;
	unsigned char roi_en;
	unsigned char rrc_en;
	unsigned char mbqp_en;
	unsigned char rnd_en;
	unsigned char tmnr_en;
	unsigned char tmnr_sce_en;
	unsigned char osg_en;
	unsigned char skipfrm_en;
	unsigned char maq_en;
	unsigned char dithering_en;
	unsigned char tnr_en;
	unsigned char lambda_en;
	unsigned char sao_en;
	unsigned char sha256_rst;
	#if 0
	unsigned char turbo_en;
	#endif
	unsigned char slice_mode;
	unsigned char custcu16_priority;
	unsigned char gmv_en;
	unsigned char part_frm_skip_en;
	unsigned char mbqp_mosaic_ctrl;
	unsigned int rec_checksum_wofbc;
	unsigned char motion_detect_en;
} pic_t_265;

typedef enum _FPGA_REPORT_265_ {
	FPGA_CHKSUM0_265,
	FPGA_REC_CHKSUM_265,
	FPGA_SRC_CHKSUM_Y_265,
	FPGA_SRC_CHKSUM_C_265,
	FPGA_TNR_OUT_CHKSUM_Y_265,
	FPGA_TNR_OUT_CHKSUM_C_265,
	FPGA_BS_LEN_265,
	FPGA_BS_CHKSUM_265,
	FPGA_CHKSUM8_265,
	FPGA_SAO_INFO_265,
	FPGA_RC_EST_BITLEN_265,
	FPGA_RC_RDO_COST_LOW_265,
	FPGA_RC_RDO_COST_HIGH_265,
	FPGA_RC_FRM_SIZE_265,
	FPGA_RC_FRM_COST_LOW_265,
	FPGA_RC_FRM_COST_HIGH_265,
	FPGA_RC_FRM_COMPLXTY_LOW_265,
	FPGA_RC_FRM_COMPLXTY_HIGH_265,
	FPGA_RC_FRM_SSE_DIST_LOW_265,
	FPGA_RC_FRM_SSE_DIST_HIGH_265,
	FPGA_RC_FRM_QP_SUM_265,
	FPGA_RC_FRM_COEFF_0_1_265,
	FPGA_RC_FRM_COEFF_2_265,
	FPGA_SRAQ2_265,
	FPGA_IME_CHKSUM_LSB_265,
	FPGA_IME_CHKSUM_MSB_265,
	FPGA_EC_REC_CHKSUM_265,
	FPGA_SIDE_INFO_CHKSUM_265,
	FPGA_SLICE_NUM_265,
	FPGA_STATS_INTER_CNT_265,
	FPGA_STATS_SKIP_CNT_265,
	FPGA_STATS_MERGE_CNT_265,
	FPGA_STATS_IRA4_CNT_265,
	FPGA_STATS_IRA8_CNT_265,
	FPGA_STATS_IRA16_CNT_265,
	FPGA_STATS_IRA32_CNT_265,
	FPGA_STATS_CU64_CNT_265,
	FPGA_STATS_CU32_CNT_265,
	FPGA_STATS_CU16_CNT_265,
	FPGA_STATS_SCD_INTER_CNT_265,
	FPGA_STATS_SCD_IRANG_CNT_265,
	FPGA_PSNR_FRM_Y_LSB_265,	// 0
	FPGA_PSNR_FRM_Y_MSB_265,
	FPGA_PSNR_FRM_U_LSB_265,
	FPGA_PSNR_FRM_U_MSB_265,
	FPGA_PSNR_FRM_V_LSB_265,
	FPGA_PSNR_FRM_V_MSB_265,	// 5
	FPGA_PSNR_ROI_Y_LSB_265,
	FPGA_PSNR_ROI_Y_MSB_265,
	FPGA_PSNR_ROI_U_LSB_265,
	FPGA_PSNR_ROI_U_MSB_265,
	FPGA_PSNR_ROI_V_LSB_265,	// 10
	FPGA_PSNR_ROI_V_MSB_265,
	FPGA_PSNR_MOT_Y_LSB_265,
	FPGA_PSNR_MOT_Y_MSB_265,
	FPGA_PSNR_MOT_U_LSB_265,
	FPGA_PSNR_MOT_U_MSB_265,	// 15
	FPGA_PSNR_MOT_V_LSB_265,
	FPGA_PSNR_MOT_V_MSB_265,
	FPGA_PSNR_BGR_Y_LSB_265,
	FPGA_PSNR_BGR_Y_MSB_265,
	FPGA_PSNR_BGR_U_LSB_265,	// 20
	FPGA_PSNR_BGR_U_MSB_265,
	FPGA_PSNR_BGR_V_LSB_265,
	FPGA_PSNR_BGR_V_MSB_265,	// 25
	FPGA_ROI_CNT_265,
	FPGA_CRC_HIT_Y_CNT_265,
	FPGA_CRC_HIT_C_CNT_265,
	FPGA_EC_REAL_LEN_SUM_265,
	FPGA_EC_SKIP_LEN_SUM_265,
	FPGA_EC_TOTAL_LEN_SUM_265,
	FPGA_MOT_CNT_265,
	FPGA_BGR_CNT_265,
	FPGA_TMNR_Y_CHKSUM_265,
	FPGA_TMNR_C_CHKSUM_265,
	FPGA_JND_Y_CHKSUM_265,
	FPGA_JND_C_CHKSUM_265,
	FPGA_MASK_Y_CHKSUM_265,
	FPGA_MASK_C_CHKSUM_265,
	FPGA_OSG_Y_CHKSUM_265,
	FPGA_OSG_C_CHKSUM_265,
	FPGA_JND_GRAD_LSB_265,
	FPGA_JND_GRAD_CNT_265,
	FPGA_RRC_COEF4_265,
	FPGA_MOTION_CNT_265,
	FPGA_SPN_Y_CHKSUM_265,
	FPGA_SPN_C_CHKSUM_265,
	FPGA_GMV_CHK_0_265,
	FPGA_GMV_CHK_1_265,
	FPGA_GMV_CHK_2_265,
	FPGA_GMV_CHK_3_265,
	FPGA_GMV_CHK_4_265,
	FPGA_GMV_CHK_5_265,
	FPGA_GMV_CHK_6_265,
	FPGA_GMV_CHK_7_265,
	FPGA_GMV_CHK_8_265,
	FPGA_GMV_CHK_9_265,
	FPGA_GMV_CHK_10_265,
	FPGA_GMV_CHK_11_265,
	FPGA_GMV_CHK_12_265,
	FPGA_GMV_CHK_13_265,
	FPGA_GMV_CHK_14_265,
	FPGA_GMV_CHK_15_265,
	FPGA_GMV_CHK_16_265,
	FPGA_GMV_CHK_17_265,
	FPGA_GMV_CHK_18_265,
	FPGA_GMV_CHK_19_265,
	FPGA_GMV_CHK_20_265,
	FPGA_GMV_CHK_21_265,
	FPGA_GMV_CHK_22_265,
	FPGA_GMV_CHK_23_265,
	FPGA_GMV_CHK_24_265,
	FPGA_GMV_CHK_25_265,
	FPGA_GMV_CHK_26_265,
	FPGA_GMV_CHK_27_265,
	FPGA_GMV_CHK_28_265,
	FPGA_GMV_CHK_29_265,
	FPGA_GMV_CHK_30_265,
	FPGA_GMV_CHK_31_265,
	FPGA_GMV_CHK_32_265,
	FPGA_GMV_CHK_33_265,
	FPGA_GMV_CHK_34_265,
	FPGA_GMV_CHK_35_265,
	FPGA_GMV_CHK_36_265,
	FPGA_GMV_CHK_37_265,
	FPGA_GMV_CHK_38_265,
	FPGA_GMV_CHK_39_265,
	FPGA_JND_GRAD_MSB_265,
	FPGA_MD_STAT_0_265,
	FPGA_MD_STAT_1_265,
	FPGA_MD_STAT_2_265,
	FPGA_MD_STAT_3_265,
	FPGA_MD_STAT_4_265,
	FPGA_MD_STAT_5_265,
	FPGA_MD_STAT_6_265,
	FPGA_MD_STAT_7_265,
	FPGA_REPORT_NUMBER_265
}FPGA_REPORT_265;

typedef enum _FPGA_TMNR_DBG_INFO_265_{
	FPGA_TMNR_DBG_SRC_Y,
	FPGA_TMNR_DBG_SRC_C,
	FPGA_TMNR_DBG_READ_Y,
	FPGA_TMNR_DBG_READ_C,
	FPGA_TMNR_DBG_WRITE_Y,
	FPGA_TMNR_DBG_WRITE_C,
	FPGA_TMNR_DBG_HISTOGRAM,
	FPGA_TMNR_DBG_MOT_IN,
	FPGA_TMNR_DBG_MOT_OUT,
	FPGA_TMNR_DBG_NUMBER_265
} FPGA_TMNR_DBG_INFO_265;

typedef struct _chk_t_265_{
	unsigned int result[FPGA_REPORT_NUMBER_265];
#if 0
	unsigned int tmnr_dbg_info[FPGA_TMNR_DBG_NUMBER_265];
#endif
} chk_t_265;

typedef struct _h265_folder_t_{
	char name[64];

	unsigned int idx;
	unsigned int pat_num;
} h265_folder_t;

typedef struct _h265_perf_t_{
	unsigned int cycle_sum;
	unsigned int bslen_sum;
	unsigned int cycle_max_frm;
	unsigned int cycle_max;
	unsigned int cycle_max_bslen;
	unsigned int cycle_i_frm;
	unsigned int bslen_i_frm;
} h265_perf_t;

#endif	// _EMU_H265_COMMON_H_
