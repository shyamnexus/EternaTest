#ifndef _EMU_H265_ENC_H_
#define _EMU_H265_ENC_H_
//
#include "kwrap/type.h"

// for hw trigger //
#include "h26x.h"

// for sw driver //
#include "h26x_def.h"
#include "h265enc_api.h"
#include "h26x_common.h"

// for emulation //
#include "emu_h26x_common.h"
#include "emu_h265_common.h"
#include "kdrv_videoenc/kdrv_videoenc_lmt.h"

#define RRC_INFO_COMP (0)

#define MAX_SLICE_HDR_LEN	(128)

#define H26X_MAX_BSCMD_NUM        (H265E_HEIGHT_MAX/64*H265E_TILE_MAX)

typedef struct _rdo_t_265_{
	char  rate_bias_I_32L;
	char  rate_bias_I_16L;
	char  rate_bias_I_08L;
	char  rate_bias_I_04L;
	char  rate_bias_I_16C;
	char  rate_bias_I_08C;

	char  rate_bias_I_04C;
	char  rate_bias_IP_16L;
	char  rate_bias_IP_08L;
	char  rate_bias_IP_04L;
	char  rate_bias_IP_08C;
	char  rate_bias_IP_04C;

	char  rate_bias_P_32L;
	char  rate_bias_P_16L;
	char  rate_bias_P_08L;
	char  rate_bias_P_16C;
	char  rate_bias_P_08C;
	char  rate_bias_P_04C;

	char  cost_bias_skip;
	char  cost_bias_merge;

	char  cost_bias_skip_bg;
	char  cost_bias_merge_bg;
	char  cost_bias_shift;

	char  global_motion_penalty_I32;
	char  global_motion_penalty_I16;
	char  global_motion_penalty_I08;
	char  global_motion_penalty_I32O;
	char  global_motion_penalty_I16O;
	char  global_motion_penalty_I08O;

	unsigned char ime_left_amvp_mode;
	unsigned char ime_scale_64c;
	unsigned char ime_scale_64p;
	unsigned char ime_scale_32c;
	unsigned char ime_scale_32p;
	unsigned char ime_scale_16c;

} rdo_t_265;

typedef struct _var_t_265_{
	unsigned int  var_t;
	unsigned char avg_min;
	unsigned char avg_max;
	unsigned char delta;
	unsigned char i_range_delta;
	unsigned char p_range_delta;
	unsigned char var_merge;
} var_t_265;

typedef struct _fro_t_265_{
	unsigned char enable;
	unsigned int  dc[2][3][4];
	unsigned char ac[2][3][4];
	unsigned char st[2][3][4];
	unsigned char mx[2][3][4];
} fro_t_265;

typedef struct _gdr_t_265_{
	unsigned char enable;
	unsigned int  period;
	unsigned int  number;
	unsigned char gdr_qp_en;
	unsigned char gdr_qp;
} gdr_t_265;

typedef struct _mask_line_t_265_{
	unsigned short x;
	unsigned short y;
	unsigned char comp;
	unsigned char hit_w;
	unsigned int line_chk_add;
} mask_line_t_265;

typedef struct _mask_win_t_265_{
	unsigned char enable;
	unsigned char did;
	unsigned char bitmap;
	unsigned char pal_sel;
	unsigned char line_hit_op;
	unsigned int  alpha;
	unsigned char alpha_type;

	mask_line_t_265 line[4];
} mask_win_t_265;

typedef struct _mask_t_265_{
	unsigned char enable;
	unsigned char bitmap_did;
	unsigned char mosaic_blkw;
	unsigned char mosaic_blkh;

	unsigned char pal_y[16];
	unsigned char pal_cb[16];
	unsigned char pal_cr[16];

	mask_win_t_265 win[16];
} mask_t_265;

typedef struct _roi_win_t_265_{
	unsigned char enable;

	unsigned int left;
	unsigned int top;
	unsigned int width;
	unsigned int height;

	unsigned char qp;
	unsigned char qp_mode;
	unsigned char fro_mode;
	unsigned char lpm_mode;
	unsigned char bgr_mode;
	unsigned char maq_mode;
	//unsigned char tnr_mode;
	unsigned char skip_label;
} roi_win_t_265;

typedef struct _roi_t_265_{
	roi_win_t_265 win[10];
} roi_t_265;

typedef struct _rrc_seq_t_265_{
	unsigned char enable;

	unsigned char scale;
	unsigned char range;
	unsigned char step;
	unsigned char min_cost_th;
	unsigned char min_qp;
	unsigned char max_qp;

	unsigned char zero_bit_mode;
	unsigned char ndqp_range;
	unsigned char ndqp_step;
	unsigned char rrc_mode; //0:original, 1: new
	unsigned int idx_start;
} rrc_seq_t_265;

typedef struct _rrc_pic_t_265_{
	unsigned char enable;
	unsigned char init_qp;
	unsigned char pred_wt;

	unsigned int  planned_stop;
	unsigned int  planned_top;
	unsigned int  planned_bot;
	unsigned int  tile_coeff[H265E_TILE_MAX];
//unsigned int  tile_coeff[4];
	unsigned int  frm_cost_lsb;
	unsigned int  frm_cost_msb;
	unsigned int  frm_cmpx_lsb;
	unsigned int  frm_cmpx_msb;
    int beta;
    int th_0;
    int th_1;
    int th_2;
    int th_M;
    int th_3;
    int th_4;
    int mod_u_frm;
    int mod_d_Frm;
	int planned_top_init;
    long long i_all_ref_pred_tmpl_init;
    int target_bits_scale;
	unsigned char tile_qp_rst;
	unsigned char tile_qp_step;
	unsigned char range;
	unsigned char step;
	unsigned char ndqp_range;
	unsigned char ndqp_step;
} rrc_pic_t_265;

typedef struct _sraq_t_265_{
	unsigned char enable;

	unsigned char mode;
	unsigned char ic2;
	unsigned char i_str;
	unsigned char i_str1;
	unsigned char i_str2;
	unsigned char p_str;
	unsigned char p_str1;
	unsigned char p_str2;
	unsigned char min_dqp;
	unsigned char max_dqp;

	unsigned char i_aslog2;
	unsigned char depth;
	unsigned char plane_x;
	unsigned char plane_y;
	unsigned int  th[H26X_AQ_TH_TBL_SIZE];
} sraq_t_265;

typedef struct _lpm_t_265_{
	unsigned char rmd_sad_en;
	unsigned char ime_stop_en;
	unsigned char ime_stop_th;
	unsigned char rdo_stop_en;
	unsigned char rdo_stop_th;
	unsigned char chroma_dm_en;
	unsigned char qp_map_defulat;
	unsigned char ira_en;
	unsigned char ira_th;
} lpm_t_265;

typedef struct _rnd_t_265_{
	unsigned char enable;

	unsigned int  seed;
	unsigned char range;
} rnd_t_265;

typedef struct _scd_t_265_{
	unsigned int  th;
	unsigned char sc;
	unsigned char override_rrc;
} scd_t_265;

typedef struct _tmnr_luma_t_265_{
	unsigned char enable;

	unsigned char avoid_residue_th;
	unsigned char nr_str_3d;
	unsigned char nr_str_2d;
	unsigned char small_vibrat_supp_en;
	unsigned char center_wzero_2d_en;
	unsigned char center_wzero_3d_en;
	unsigned char blur_str;
	unsigned char max_blk_edgvar;
	unsigned char lut_2d_th[4];
	unsigned char lut_3d_th[2][4];
	unsigned char motion_level_th[2][2];
	unsigned char motion_2x2_to8x8_th[2];

	unsigned int base[8];
	unsigned int std[8];
	unsigned int coefa[8];
	unsigned int coefb[8];
	unsigned int line_offset;
} tmnr_luma_t_265;

typedef struct _tmnr_chrm_t_265_{
	unsigned char enable;

	unsigned char avoid_residue_th;
	unsigned char nr_str_3d;
	unsigned char nr_str_2d;
	unsigned char lut_2d_th[4];
	unsigned char lut_3d_th[4];
	unsigned char motion_level_th[2];

	unsigned int mean[2][8];
	unsigned int std[2][8];
	unsigned int line_offset;
} tmnr_chrm_t_265;

typedef struct _tmnr_t_265_{
	unsigned char enable;

	unsigned char ne_blk_sample_step;
	unsigned char ref_w;
	unsigned char sta_w;
	unsigned char display_motion_map_en;
	unsigned char display_motion_map_mode;
	unsigned char out_3d_en;
	unsigned char out_2d_en;
	unsigned char mt_en[2];
	unsigned char fcs_en;
	unsigned char fcs_str;
	unsigned char fcs_th;
	unsigned char mby_step;
	unsigned char mbx_step;
	unsigned char lcuy_step;
	unsigned char lcux_step;

	unsigned int info_x_num;
	unsigned int info_x_oft;
	unsigned int info_y_num;
	unsigned int info_y_oft;

	tmnr_luma_t_265 luma;
	tmnr_chrm_t_265 chrm;
    unsigned char refr_1d;              //0~1,  default=0
    unsigned char refw_1d;              //0~1,  default=0
    unsigned char m_sce;                //0~1,  default=0

    unsigned char ref_as_src;              //bool  new in 321.
    unsigned char status_ctrl;             // U2   new in 321.
    unsigned char err_compensate;          //bool  new in 321.
} tmnr_t_265;

typedef struct _tmnr_ycc_t_265_{
#if 0
    unsigned short yuv_enc_dct_qtbl_encp[16][3];
    unsigned short yuv_enc_dct_qtbl_decp[16][3];
    unsigned short yuv_enc_dct_qtbl_dc[16];
#else
	unsigned char yuv_enc_dct_level_th[8];
	unsigned char yuv_enc_dct_qtbl_idx[16];
#endif
    unsigned short dith_seed0,dith_seed1;
    unsigned char yuv_dec_shift_en;
    unsigned char yuv_enc_shift_en;
    unsigned char dith_en;
}tmnr_ycc_t_265;

typedef struct _tmnr_dithering_t_265_{
    unsigned char dithering_en;            //BOOL, new in 321
    unsigned char dithering_bitY;          //U3    new in 321
    unsigned char dithering_bitU;          //U3    new in 321
    unsigned char dithering_bitV;          //U3    new in 321
    unsigned int dithering_seed[2];       //U3    new in 321
}tmnr_dithering_t_265;

typedef struct _osg_graph_t_265_{
	unsigned char type;
	unsigned int  width ;
	unsigned int  height;
	unsigned int lofs;
	unsigned int  uv_type;
} osg_graph_t_265;

typedef struct _osg_colorkey_t_265_{
	unsigned char key_en;
	unsigned char alpha_en;
	unsigned char alpha;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} osg_colorkey_t_265;

typedef struct _osg_palette_t_265_{
	unsigned char alpha;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} osg_palette_t_265;

typedef struct _osg_disp_t_265_{
	unsigned char mode;
	unsigned int  x_str;
	unsigned int  y_str;
	unsigned char bg_alpha;
	unsigned char fg_alpha;
	unsigned char mask_type;
	unsigned char mask_bd_size;
	unsigned char mask_blk_size;
	unsigned char mask_y[2];
	unsigned char mask_cb;
	unsigned char mask_cr;
} osg_disp_t_265;

typedef struct _osg_gcac_t_265_{
	unsigned char enable;

	unsigned char blk_width;
	unsigned char blk_height;
	unsigned char blk_h_num;
	unsigned char org_color_lv;
	unsigned char inv_color_lv;
	unsigned char nor_diff_th;
	unsigned char inv_diff_th;
	unsigned char full_eval_mode;
	unsigned char eval_lum_targ;
	unsigned char sta_only_mode;
} osg_gcac_t_265;

typedef struct _osg_qpmap_t_265_{
	unsigned char lpm_mode;
	unsigned char tnr_mode;
	unsigned char fro_mode;
	unsigned char qp_mode;
	char qp_val;
	unsigned char bgr_mode;
	unsigned char maq_mode;
	unsigned char skip_label;
} osg_qpmap_t_265;

typedef struct _osg_win_t_265_{
	unsigned char enable;

	osg_graph_t_265 graph;
	osg_disp_t_265  disp;
	osg_gcac_t_265  gcac;
	osg_qpmap_t_265 qpmap;
	osg_colorkey_t_265 key;

	unsigned char mosaic_blk_sz;
} osg_win_t_265;

typedef struct _osg_t_265_{
	unsigned char rgb2yuv[3][3];
	osg_win_t_265 win[32];
	osg_palette_t_265 palette[16];

	unsigned char mosaic_bitmap_blk_sz;
	unsigned char chroma_alpha;
	unsigned char overlap_type;
} osg_t_265;

typedef struct _maq_t_265_{
	unsigned char mode;
	unsigned char roi_th;
	unsigned char dqp_8x8to16x16_th;
	char dqp[6];
	unsigned char dqpnum;
    unsigned char dqp_mot_th;
	unsigned char mode1;
	unsigned char mode2;
} maq_t_265;

typedef struct _jnd_t_265_{
	unsigned char enable;
	unsigned char str;
	unsigned char level;
	unsigned char th;
	unsigned char c_str;
	unsigned char c_r5_flag;
	unsigned char bila_flag;
	unsigned char lsigma_th;
	unsigned char lsigma;

	unsigned char jnd_la_ker;
	unsigned char jnd_y_th1;
	unsigned char jnd_y_th2;
	unsigned char jnd_cm_th1;
	unsigned char jnd_cm_th2;
	unsigned char jnd_u_th1;
	unsigned char jnd_u_th2;
	unsigned char jnd_v_th1;
	unsigned char jnd_v_th2;
} jnd_t_265;

typedef struct _bgr_t_265_{
	unsigned char enable;
    unsigned char bgr_typ;
    int  bgr_th[2];
    int  bgr_qp[2];
    int  bgr_vt[2];
    int  bgr_dq[2];
#if 0
    int  bgr_dth[2];
    int  bgr_bth[2];
#endif
} bgr_t_265;


typedef struct _rmd_t_265_{
	unsigned char rmd_cost_tweak_pl;
	unsigned char rmd_cost_tweak_dc;
	unsigned char ira_modebits_de;
	int    rmd_pl_y;
	int    rmd_dc_y;
	int    rmd_hr_y;
	int    rmd_vt_y;
	int    rmd_ot_Y;
	int    rmd_shift;
} rmd_t_265;

typedef struct _tnr_t_265_{
	int  nr_3d_mode;						// 0 = nr off, 1 = nr on
	int  tnr_osd_mode;
	int  mctf_p2p_pixel_blending;
	int  tnr_p2p_sad_mode;
	int  tnr_mctf_sad_mode;
	int  tnr_mctf_bias_mode;

	int  nr_3d_adp_th_p2p[3];
	int  nr_3d_adp_weight_p2p[3];
	int  tnr_p2p_border_check_th;
	int  tnr_p2p_border_check_sc;
	int  tnr_p2p_input;
	int  tnr_p2p_input_weight;
	int  cur_p2p_mctf_motion_th;
	int  ref_p2p_mctf_motion_th;
	int  tnr_p2p_mctf_motion_wt[4];

	int  nr3d_temporal_spatial_y[3];
	int  nr3d_temporal_spatial_c[3];
	int  nr3d_temporal_range_y[3];
	int  nr3d_temporal_range_c[3];
	int  nr3d_clampy_th;
	int  nr3d_clampy_div;
	int  nr3d_clampc_th;
	int  nr3d_clampc_div;

	int  nr3d_temporal_spatial_y_mctf[3];
	int  nr3d_temporal_spatial_c_mctf[3];
	int  nr3d_temporal_range_y_mctf[3];
	int  nr3d_temporal_range_c_mctf[3];
	int  nr3d_clampy_th_mctf;
	int  nr3d_clampy_div_mctf;
	int  nr3d_clampc_th_mctf;
	int  nr3d_clampc_div_mctf;

    int  cur_motion_rat_th;
    int  cur_motion_sad_th;
    int  ref_motion_twr_p2p_th[2];
    int  cur_motion_twr_p2p_th[2];
    int  ref_motion_twr_mctf_th[2];
    int  cur_motion_twr_mctf_th[2];
    int  nr3d_temporal_spatial_y_1[3];
    int  nr3d_temporal_spatial_c_1[3];
    int  nr3d_temporal_spatial_y_mctf_1[3];
    int  nr3d_temporal_spatial_c_mctf_1[3];
    int  sad_twr_p2p_th [2];
    int  sad_twr_mctf_th[2];
} tnr_t_265;

typedef struct _lambda_t_265_{
#if CHIP_NT98539
	char delta_sta;
	char delta_mot;
	char delta_grd;
	unsigned char delta_grd_th;
#else
    int adaptlambda_en;
    int lambda_table[52];
    int sqrt_lambda_table[52];
#endif
} lambda_t_265;

typedef struct _eskip_t_265_{
	unsigned char md_th;
	unsigned char mv_th;
	unsigned int  skip_cost_th;
	unsigned int  skip_cost_th1;
	#if CHIP_NT98539
	unsigned char md_lpm_ira_stop_en;
	unsigned char md_early_skip_en;
	#endif
} eskip_t_265;

typedef struct _spn_t_265_{
	bool Enable;
	int EdgeWeightTh;
	int EdgeWeightGain;
	int EdgeSharpStr1;
	//int EdgeSharpStr2;
	//int FlatSharpStr;
	int CoringTh;
	int BrightHaloClip;
	int DarkHaloClip;
	int NoiseLevel;
	int BlendInvGamma;

	int NoiseCurve[17];

	int w_con_eng;
	int th_flat;
	int th_edge;
	int w_low;
	int w_high;
	int slope_con_eng;
	int th_flat_HLD;
	int th_edge_HLD;
	int th_HLD_lum;
	int w_low_HLD;
	int w_high_HLD;
	int slope_con_eng_HLD;

	int flat_region_str;
	int edge_region_str;
	int transition_region_str;

	int motion_edgeWeight_str;
	int static_edgeWeight_str;

	bool ShowSharpInfo;
	int EWGCurve[9];
} spn_t_265;

typedef struct _qp_related_t_265_{
	unsigned char ime_coherent_qp;
	unsigned char rdo_coherent_qp;
	unsigned char qp_merge;
//	unsigned char max_cu_dqp_depth;
} qp_related_t_265;

typedef struct _sha256_t_265_{
	unsigned int state[8];
} sha256_t_265;

typedef struct _turbo_t_265_{
	#if CHIP_NT98539
	unsigned char turbo_ime_scale_cbias;
	unsigned char turbo_ime_scale_pbias;
	unsigned char turbo_tx_th_init;
	unsigned char turbo_mv_th;
	unsigned char turbo_y8_single_mode;
	unsigned char merge_bias;
	#else
	unsigned char turbo_en;
	unsigned char turbo_inter_type;
	#endif
} turbo_t_265;

typedef struct _slice_mode_t_265_{
	unsigned char slice_i_enable;
	unsigned short slice_row[4];
	unsigned short slice_i_idx;
} slice_mode_t_265;

typedef struct _res_sup_t_265_{
	unsigned char en;
	unsigned char en_c;
	unsigned char opt;
	unsigned char noise_th;
	unsigned char mean_bnd1;
	unsigned char mean_bnd2;

	char peak_th;
	unsigned char peak_rng;
	unsigned char edge_level;
	unsigned char la_th;
	char ib_th1;
	unsigned char ib_th2;

	unsigned char qp_bnd1;
	unsigned char qp_bnd2;
	unsigned short str;
	unsigned short str_c;
} res_sup_t_265;

typedef struct _gmv_t_265_{
	unsigned char ctrl;
	unsigned char enable[16];
	unsigned char x[16];
	unsigned char y[16];
} gmv_t_265;

typedef struct _imv_ctrl_t_265_{
	unsigned char imv_ctrl;
	unsigned char imv_frm_pos;
} imv_ctrl_t_265;

typedef struct _mot_filter_t_265_{
	unsigned char enable;
	unsigned char thr;
	char filter_dqp;
	char label_dap;
} mot_filter_t_265;

typedef struct _mot_detect_t_265_{
	unsigned char enable;
	unsigned char mode;
	char dqp[2];
	unsigned char gain;
	unsigned char frm_offset;
	unsigned char q_offset;
	unsigned char q_step;
	#if CHIP_NT98539
	unsigned char lpm_ira_stop_en;
	unsigned char early_skip_en;
	#endif
} mot_detect_t_265;

typedef struct _partial_frm_skip_t_265_{
	unsigned char enable;
	unsigned char left;
	unsigned char top;
	unsigned char right;
	unsigned char bot;
} partial_frm_skip_t_265;

typedef struct _water_mark_t_265_ {
	unsigned char enable[8];
	unsigned char uv[8];
	unsigned char str[8];
	unsigned int  left_top[8];
	unsigned char len[8];
} water_mark_t_265;

typedef struct _h265_emu_t_{
	H26XENC_VAR  var_obj;
	H265ENC_INFO info_obj;
	H265ENC_INIT init_obj;
}h265_emu_t;

typedef struct _h265_pat_t_{
	char name[128];
	unsigned int idx;
	unsigned int pic_num;

	file_t_265 file;

	unsigned int seq_obj_size;
	unsigned int pic_obj_size;
	unsigned int chk_obj_size;

	info_t_265 info;
	seq_t_265  seq;
	pic_t_265  pic;
	chk_t_265  chk;

	unsigned char rotate;
	unsigned char src_cbcr_iv;
	unsigned int  rrc_chk_en;
	unsigned char rrc_mode;
	unsigned char aq_chk_en;

	//unsigned int slice_number;
	unsigned int bsdma_buf_size;
	uintptr_t bsdma_buf_addr;
	unsigned int slice_hdr_buf_size;
	uintptr_t slice_hdr_buf_addr;
	unsigned int slice_hdr_len[H26X_MAX_BSCMD_NUM];
	unsigned int slice_len[H26X_MAX_BSCMD_NUM];
	uintptr_t source_out_y_tmp_addr;
	uintptr_t source_out_c_tmp_addr;

	unsigned int bs_buf_size;
	uintptr_t bs_buf_addr;
	unsigned int bs_buf_size2;
	uintptr_t bs_buf_addr2;
	unsigned int bs_buf_32b;
	unsigned int res_bs_size;

	uintptr_t tmp_src_y_addr;	// use for soucre post processing //
	uintptr_t tmp_src_c_addr;

	uintptr_t tmp_tmnr_mot;

	uintptr_t tmp_big_share_mem_addr; // use for mbqp rotate/rotate/source out //
	unsigned int rand_seed;

	h265_perf_t perf;
	unsigned int stable_bs_len;
	unsigned int rnd_slc_hdr;
	uintptr_t tmp_mbqp_addr;
	uintptr_t mbqp_addr;
	unsigned int src_d2d_en;
	unsigned int src_d2d_mode;

	unsigned int userqp_rotate;
	unsigned int usr_qp_map_offset;

	unsigned int performance_check;
	unsigned int performance_osg_range;
	sha256_t_265 sha256;
	unsigned char blk_mode;
	unsigned char video_hdr_len;
	unsigned char video_hdr_es[MAX_SLICE_HDR_LEN];

	uintptr_t tmp_svc_ltr_addr; // use for svc/ ltr //
	uintptr_t rrc_buf_addr;
	unsigned int rrc_buf_size;

	unsigned char srcout2src;

	unsigned char slice_ll_en;
	uintptr_t slice_ll_dummy_adr;
	uintptr_t slice_ll_len_adr;
	unsigned int *slice_ll_dummy;
	unsigned int *slice_ll_len;
	unsigned int slice_ll_len_acl;
	unsigned int slice_idx;
	unsigned int slice_acl_int;
}h265_pat_t;

typedef struct _h265_ctx_t_{
	h265_folder_t folder;
	h265_emu_t    emu;
	h265_pat_t    pat;
}h265_ctx_t;

BOOL emu_h265_setup(h26x_ctrl_t *p_ctrl, UINT8 pucDir[265], h26x_srcd2d_t *p_src_d2d, char src_path_d2d[2][128]);
BOOL emu_h265_setup_one_job(h26x_ctrl_t *p_ctrl, unsigned int start_folder_idx, unsigned int end_folder_idx,unsigned int start_pat_idx, unsigned int end_pat_idx, unsigned int end_frm_num, UINT8 pucDir[265], h26x_srcd2d_t *p_src_d2d);
int emu_h265_prepare_one_sequence(h26x_job_t *p_job, h265_ctx_t *ctx, h26x_mem_t *p_mem, h26x_ver_item_t *p_ver_item);
BOOL emu_h265_prepare_one_pic(h26x_job_t *p_job, h26x_ver_item_t *p_ver_item, h26x_srcd2d_t *p_src_d2d);
BOOL emu_h265_chk_one_pic(h26x_ctrl_t *p_ctrl, h26x_job_t *p_job, UINT32 interrupt, unsigned int chip_idx);
int emu_h265_init(h26x_job_t *p_job, h26x_ver_item_t *p_ver_item, h26x_srcd2d_t *p_src_d2d);
void emu_h265_set_nxt_bsbuf(h26x_job_t *p_job, h26x_ver_item_t *p_ver_item,UINT32 interrupt);
void emu_h265_reset_bsbuf(h26x_job_t *p_job, UINT32 bs_len, unsigned int write_prot);
void emu_h265_close_one_pat(h265_pat_t *p_pat);

void emu_h265_set_nxt_ll_slice(h26x_job_t *p_job);
BOOL emu_h265_job_debug(h26x_job_t *p_job, UINT32 line);

#endif	// _EMU_H265_ENC_H_
