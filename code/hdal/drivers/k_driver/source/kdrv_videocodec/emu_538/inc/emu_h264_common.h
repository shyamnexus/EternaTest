#ifndef _EMU_H264_COMMON_H_
#define _EMU_H264_COMMON_H_

#include "kwrap/type.h"

// for hw trigger //
#include "h26x.h"

// for sw driver //
#include "h26x_def.h"
#include "h26xenc_api.h"
#include "h264enc_api.h"

// for emulation //
#include "emu_h26x_common.h"

#define cmp_smart_rec 0 //520 x
#define H264_LINEOFFSET_MAX 6144


//#if (EMU_H26X == ENABLE || AUTOTEST_H26X == ENABLE)
typedef struct _file_t_ {
	H26XFile info;
	H26XFile src;
	H26XFile es;
	H26XFile slice_hdr_len;
	H26XFile slice_hdr_es;
	H26XFile slice_len;
	H26XFile mbqp;
	H26XFile seq;
	H26XFile pic;
	H26XFile chk;
	H26XFile sdebsy;
	H26XFile sdebsuv;
	H26XFile tmnr_mt;
	H26XFile osg_grap[32];
	H26XFile motion_bit;
	H26XFile colmv;
	H26XFile source_out;

	//decode
	H26XFile video_hdr_len;
	H26XFile video_hdr_es; // sps + pps
	//sdec
	H26XFile sde_reg;

	//wtmk
	H26XFile wtmk[8];

	//codec_md
	H26XFile codec_md;
} file_t;

typedef struct _info_t_ {
	unsigned char yuv_name[256];
	unsigned int dump_src_en;
	unsigned int width;
	unsigned int height;
	unsigned int frame_num;
	unsigned int cmd_len;
} info_t;

typedef enum {
	FF_RDO = 0,
	FF_VAR,
	FF_FRO,
	FF_MASK,
	FF_GDR,
	FF_ROI,
	FF_RRC_SEQ,
	FF_RRC_PIC,
	FF_SRAQ,
	FF_LPM,
	FF_RND,
	FF_SCD,
	FF_TMNR,
	FF_OSG,
	FF_MAQ,
	FF_JND,
	FF_BGR,
	FF_DITHER,
	FF_RMD,
	FF_TNR,
	FF_LAMBDA,
	FF_ESKIP,
	FF_SPN,
	FF_SHA,
	FF_TURBO,
	FF_RESSUP,
	FF_GMV,
	FF_WTMK,
	FF_MDMF,
	FF_PAR_FSKIP,
	FF_MAX_NUM = 32
} e_FPGA_FUNC; // FF = FPGA_FUNC

typedef struct _seq_t_ {
	unsigned int obj_size[FF_MAX_NUM];

	unsigned int width;
	unsigned int height;

	unsigned int dis_loopfilter_idc;
	int dblk_alpha;
	int dblk_beta;

	int chrm_qp_offset;
	int sec_chrm_qp_offset;

	unsigned char min_qp;
	unsigned char max_qp;

	unsigned char fbc_en;
	unsigned char gray_en;
	unsigned char gray_mode_color_en;
	unsigned char fastsearch_en;
	unsigned char entropy_coding;
	unsigned char tran8x8;
	unsigned int  slice_row_num;

	// fpga only (hack parameter) : differ from driver settings  //
	unsigned char icm_use_rec_pixel;
	unsigned char cabac_init_idc;
	unsigned int  log2_max_fno;
	unsigned int  log2_max_poc;
	unsigned char sraq_save_dqp_en;
	unsigned char ime_left_amvp_mode;
	unsigned char sdecmps_en;
	unsigned char sdecmps_rotate;
	unsigned char sdecmps_cbcr_iv;
	unsigned char hw_pad_en;
	unsigned int  tmnr_his_size;
	unsigned char flxsr_en;
	unsigned char force_skip_flag;

#if 1
	/* 538 is not used, CWTSAO */
	unsigned char tile_en;
	unsigned char tile_num;
	unsigned int  tile_w[6];
	unsigned char avc_tile_mdf;
#endif
	unsigned char ae_fifo;
	int config_loop_filter;
	unsigned char ec_sw_config;
	unsigned char sdecmps_blk_mode;

	char gdr_slice_mode;
	char gdr_slice_num;
	int gdr_slice_row_num[5];
	int gdr_i_slice_idx;

	char set_cu16_qp;

	int me_srh;
	int me_srv;
	int ec_srh;
	int ec_srv;

	unsigned int max_bs_len;

	/* new feature */
	unsigned char debreath_en;
	unsigned char ime_amvp_mod;
	unsigned char ime_get_intra_mv_en;
	unsigned char mosaic_ctrl;
} seq_t;

typedef struct _pic_t_ {
	unsigned char slice_type;
	unsigned char qp;
	unsigned int  total_slice_hdr_len;

	// fpga only //
	unsigned char mask_en;
	unsigned char roi_en;
	unsigned char rrc_en;
	unsigned char mbqp_en;
	unsigned char rnd_en;
	unsigned char tmnr_en;
	unsigned char osg_en;
	unsigned char skipfrm_en;
	unsigned char maq_en;
	unsigned char dithering_en;
	unsigned char tnr_en;
	unsigned char lambda_en;
	unsigned char spn_en;

#if 0
	/* 538 is not used, CWTSAO */
	unsigned char turbo_en;
	int tb_skip_th_check;
#endif
	unsigned char res_sup_en;
	unsigned char gmv_en;
	unsigned char wtmk_en;
	unsigned char mdmf_en;
	unsigned char par_fskip_en;   // for 08/17 pat
	unsigned int add_frm_num;
} pic_t;

typedef enum _FPGA_REPORT_ {
	FPGA_REC_CHKSUM,
	FPGA_EC_REC_CHKSUM,
	FPGA_SRC_Y_CHKSUM,
	FPGA_SRC_C_CHKSUM,
	FPGA_TNR_OUT_Y_CHKSUM,
	FPGA_TNR_OUT_C_CHKSUM,
	FPGA_BS_LEN,
	FPGA_BS_CHKSUM,
	FPGA_RRC_RDOPT_COST_LSB,
	FPGA_RRC_RDOPT_COST_MSB,
	FPGA_RRC_SIZE,
	FPGA_RRC_FRM_COST_LSB,
	FPGA_RRC_FRM_COST_MSB,
	FPGA_RRC_FRM_COMPLEXITY_LSB,
	FPGA_RRC_FRM_COMPLEXITY_MSB,
	FPGA_RRC_COEFF,
	FPGA_RRC_COEFF2,         ///   567 is not used, CWTSAO ??
	FPGA_RRC_QP_SUM,
	FPGA_SRAQ_ISUM_ACT_LOG,
	FPGA_PSNR_FRM_Y_LSB,
	FPGA_PSNR_FRM_Y_MSB,
	FPGA_PSNR_FRM_U_LSB,
	FPGA_PSNR_FRM_U_MSB,
	FPGA_PSNR_FRM_V_LSB,
	FPGA_PSNR_FRM_V_MSB,
	FPGA_PSNR_ROI_Y_LSB,
	FPGA_PSNR_ROI_Y_MSB,
	FPGA_PSNR_ROI_U_LSB,
	FPGA_PSNR_ROI_U_MSB,
	FPGA_PSNR_ROI_V_LSB,
	FPGA_PSNR_ROI_V_MSB,
	FPGA_PSNR_MOT_Y_LSB,
	FPGA_PSNR_MOT_Y_MSB,
	FPGA_PSNR_MOT_U_LSB,
	FPGA_PSNR_MOT_U_MSB,
	FPGA_PSNR_MOT_V_LSB,
	FPGA_PSNR_MOT_V_MSB,
	FPGA_PSNR_BGR_Y_LSB,
	FPGA_PSNR_BGR_Y_MSB,
	FPGA_PSNR_BGR_U_LSB,
	FPGA_PSNR_BGR_U_MSB,
	FPGA_PSNR_BGR_V_LSB,
	FPGA_PSNR_BGR_V_MSB,
	FPGA_ROI_CNT,
	FPGA_MOT_CNT,
	FPGA_BGR_CNT,
	FPGA_CRC_HIT_Y_CNT,
	FPGA_CRC_HIT_C_CNT,
	FPGA_SCD_REPORT,
	FPGA_TMNR_MT_OUT,
	FPGA_TMNR_HIS,
	FPGA_TMNR_Y_CHKSUM,
	FPGA_TMNR_C_CHKSUM,
	FPGA_MASK_Y_CHKSUM,
	FPGA_MASK_C_CHKSUM,
	FPGA_OSG_Y_CHKSUM,
	FPGA_OSG_C_CHKSUM,
	FPGA_MOTION_CNT,
	FPGA_OSG_0_Y_CHKSUM,
	FPGA_OSG_0_C_CHKSUM,
	FPGA_JND_Y_CHKSUM,
	FPGA_JND_C_CHKSUM,
	FPGA_JND_GRAD,
	FPGA_JND_GRAD_CNT,
	FPGA_DITHER_Y_CHKSUM,
	FPGA_DITHER_C_CHKSUM,
	FPGA_TMNR_REF_Y_CHKSUM,
	FPGA_TMNR_REF_C_CHKSUM,
	FPGA_TMNR_REC_Y_CHKSUM,
	FPGA_TMNR_REC_C_CHKSUM,
	FPGA_SOURCE_OUT_Y_CHKSUM,
	FPGA_SOURCE_OUT_C_CHKSUM,
	FPGA_TILE_RRC_SIZE,
	FPGA_TILE_RRC_FRM_COST_LSB,
	FPGA_TILE_RRC_FRM_COST_MSB,
	FPGA_TILE_RRC_FRM_COMPLEXITY_LSB,
	FPGA_TILE_RRC_FRM_COMPLEXITY_MSB,
	FPGA_TILE_RRC_QP_SUM,
	FPGA_TILE1_RRC_SIZE,
	FPGA_TILE1_RRC_FRM_COST_LSB,
	FPGA_TILE1_RRC_FRM_COST_MSB,
	FPGA_TILE1_RRC_FRM_COMPLEXITY_LSB,
	FPGA_TILE1_RRC_FRM_COMPLEXITY_MSB,
	FPGA_TILE1_RRC_QP_SUM,
	FPGA_TILE2_RRC_SIZE,
	FPGA_TILE2_RRC_FRM_COST_LSB,
	FPGA_TILE2_RRC_FRM_COST_MSB,
	FPGA_TILE2_RRC_FRM_COMPLEXITY_LSB,
	FPGA_TILE2_RRC_FRM_COMPLEXITY_MSB,
	FPGA_TILE2_RRC_QP_SUM,
	FPGA_GMV_CHK_0,
	FPGA_GMV_CHK_1,
	FPGA_GMV_CHK_2,
	FPGA_GMV_CHK_3,
	FPGA_GMV_CHK_4,
	FPGA_GMV_CHK_5,
	FPGA_GMV_CHK_6,
	FPGA_GMV_CHK_7,
	FPGA_GMV_CHK_8,
	FPGA_GMV_CHK_9,
	FPGA_GMV_CHK_10,
	FPGA_GMV_CHK_11,
	FPGA_GMV_CHK_12,
	FPGA_GMV_CHK_13,
	FPGA_GMV_CHK_14,
	FPGA_GMV_CHK_15,
	FPGA_GMV_CHK_16,
	FPGA_GMV_CHK_17,
	FPGA_GMV_CHK_18,
	FPGA_GMV_CHK_19,
	FPGA_GMV_CHK_20,
	FPGA_GMV_CHK_21,
	FPGA_GMV_CHK_22,
	FPGA_GMV_CHK_23,
	FPGA_GMV_CHK_24,
	FPGA_GMV_CHK_25,
	FPGA_GMV_CHK_26,
	FPGA_GMV_CHK_27,
	FPGA_GMV_CHK_28,
	FPGA_GMV_CHK_29,
	FPGA_GMV_CHK_30,
	FPGA_GMV_CHK_31,
	FPGA_GMV_CHK_32,
	FPGA_GMV_CHK_33,
	FPGA_GMV_CHK_34,
	FPGA_GMV_CHK_35,
	FPGA_GMV_CHK_36,
	FPGA_GMV_CHK_37,
	FPGA_GMV_CHK_38,
	FPGA_GMV_CHK_39,
	FPGA_MD_STAT_0,
	FPGA_MD_STAT_1,
	FPGA_MD_STAT_2,
	FPGA_MD_STAT_3,
	FPGA_MD_STAT_4,
	FPGA_MD_STAT_5,
	FPGA_MD_STAT_6,
	FPGA_MD_STAT_7,
	FPGA_REPORT_NUMBER
} FPGA_REPORT;

typedef struct _chk_t_ {
	unsigned int result[FPGA_REPORT_NUMBER];
} chk_t;

typedef struct _h264_folder_t_ {
	char name[64];

	unsigned int idx;
	unsigned int pat_num;
} h264_folder_t;

typedef struct _h264_perf_t_ {
	unsigned int cycle_sum;
	unsigned int bslen_sum;
	unsigned int cycle_max_frm;
	unsigned int cycle_max;
	unsigned int cycle_max_bslen;
} h264_perf_t;

//#endif // (EMU_H26X == ENABLE || AUTOTEST_H26X == ENABLE)

#endif	// _EMU_H264_COMMON_H_
