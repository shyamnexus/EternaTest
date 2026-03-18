#ifndef __HEVC_RESMGR_H__
#define __HEVC_RESMGR_H__

#include "../../vps/h265/vps_hevc.h"
#include "../../h265dec_comm_def.h"

#include "kwrap/list.h"

enum
{
	UNUSED_FOR_REFERENCE = 0,
	USED_FOR_SHORT_TERM = 1,
	USED_FOR_LONG_TERM = 2,
	USED_FOR_REFERENCE = 3,
};

typedef enum
{
	ENG_HEVC_NONE = 0,
	ENG_HEVC_PARSER = (1 << 0),
	ENG_HEVC_AD0 = (1 << 1),
	ENG_HEVC_BIN0 = (1 << 2),
	ENG_HEVC_INUSE = (1 << 3),

}EN_HEVC_Engine;

#define REGISTER(name,list)    \
union {                        \
    struct                     \
        list;                  \
    int value;                 \
	}name

#define REGISTER2(name,list0,list1)    \
union {                                \
    struct list0;                      \
    struct list1;                      \
    int value;                         \
	}name                                  \

typedef struct
{
	REGISTER(reg000,
	{
		int swrst                   : 1;
		int dec_st                  : 1;
		int bs_cmd_adr_valid        : 1;
		int bin_adr_valid           : 1;
        int swrst_axi_wt_en         : 1;
		int reserved0               : 11;
		int pic_fsh_int             : 1;
		int bsdma_end_int           : 1;
        int reserved1               : 2;
        int bindma_end_int          : 1;
        int reserved2               : 1;
		int hw_timeout_int			: 1;
		int reserved3               : 9;
	};
	int dec_ctrl);

	REGISTER(reg004,
	{
		int pic_width_in_luma_samples : 16;
		int pic_height_in_luma_samples : 16;
	});


	REGISTER(reg008,
	{
		int chroma_format_idc                   : 2;
		int separate_colour_plane_flag          : 1;
		int pcm_enabled_flag                    : 1;
		int log2_max_pic_order_cnt_lsb_minus4   : 4;
		int lists_modification_present_flag     : 1;
		int reserved0                           : 3;
		int pcm_bit_depth_luma_minus1           : 4;
		int pcm_bit_depth_chroma_minus1         : 4;
		int reserved1                           : 4;
		int pic_struct_uv                       : 2;
		int pic_struct_y                        : 2;
		int pic_ilf_first_wr_disable_flag       : 1;
	});

	REGISTER(reg00c,
	{
		int log2_min_cu_size_minus3     : 3;
		int reserved0                   : 1;
		int log2_ctb_size_minus3        : 3;
		int reserved1                   : 1;
		int log2_min_tu_size_minus2     : 3;
		int reserved2                   : 1;
		int log2_max_tu_size_minus2     : 3;
		int reserved3                   : 1;
		int log2_min_pcm_cu_size_minus3 : 3;
		int reserved4                   : 1;
		int log2_max_pcm_cu_size_minus3 : 3;
		int reserved5                   : 1;
		int max_tf_hier_dep_inter       : 3;
		int reserved6                   : 1;
		int max_tf_hier_dep_intra       : 3;
		int scaling_list_enable_flag    : 1;
	});

	REGISTER(reg010,
	{
		int amp_enabled_flag                : 1;
		int sao_enabled_flag                : 1;
		int pcm_loop_filter_disable_flag    : 1;
		int long_term_ref_pics_present_flag : 1;
		int sps_temporal_mvp_enable_flag    : 1;
		int sign_data_hiding_flag           : 1;
		int cabac_init_present_flag         : 1;
		int constrained_intra_pred_flag     : 1;
        int num_ref_idx_l0_default_active_minus1 : 4;
		int num_ref_idx_l1_default_active_minus1 : 4;
		int num_short_term_ref_pic_sets     : 7;
		int reserved0                       : 1;
		int num_long_term_ref_pics_sps      : 6;
		int auto_reset_enable               : 1;
		int fifo_mode_enable                : 1;
	});

	REGISTER(reg014,
	{
		int pic_init_qp_minus26             : 6;
		int transform_skip_enabled_flag     : 1;
		int cu_qp_delta_enabled_flag        : 1;
		int diff_cu_qp_delta_depth          : 3;
		int pic_slc_lvl_chroma_qp_ofs_present_flag : 1;
		int pic_cb_qp_offset                : 5;
		int reserved0                       : 3;
		int pic_cr_qp_offset                : 5;
		int pic_init_qp_minus26_bit6        : 1;
	});

	REGISTER(reg018,
	{
		int weighted_pred_flag                  : 1;
		int weighted_bipred_flag                : 1;
		int output_flag_present_flag            : 1;
		int transquant_bypass_enable_flag       : 1;
		int dependent_slc_enabled_flag          : 1;
		int entropy_coding_sync_enabled_flag    : 1;
		int slice_hdr_ext_present               : 1;
		int tiles_enabled_flag                  : 1;
		int num_tile_columns_minus1             : 5;
		int reserved0                           : 3;
		int num_tile_rows_minus1                : 6;
	});

	REGISTER(reg01c,
	{
		int tile_x_bd_00 : 9;
		int tile_x_bd_01 : 9;
		int tile_x_bd_02 : 9;
        int msb_bit_of_tile_x_bd_00 : 1;
        int msb_bit_of_tile_x_bd_01 : 1;
        int msb_bit_of_tile_x_bd_02 : 1;
	});

	REGISTER(reg020,
	{
		int tile_x_bd_03 : 9;
		int tile_x_bd_04 : 9;
		int tile_x_bd_05 : 9;
        int msb_bit_of_tile_x_bd_03 : 1;
        int msb_bit_of_tile_x_bd_04 : 1;
        int msb_bit_of_tile_x_bd_05 : 1;
	});

	REGISTER(reg024,
	{
		int tile_x_bd_06 : 9;
		int tile_x_bd_07 : 9;
		int tile_x_bd_08 : 9;
        int msb_bit_of_tile_x_bd_06 : 1;
        int msb_bit_of_tile_x_bd_07 : 1;
        int msb_bit_of_tile_x_bd_08 : 1;
	});

	REGISTER(reg028,
	{
		int tile_x_bd_09 : 9;
		int tile_x_bd_0a : 9;
		int tile_x_bd_0b : 9;
        int msb_bit_of_tile_x_bd_09 : 1;
        int msb_bit_of_tile_x_bd_0a : 1;
        int msb_bit_of_tile_x_bd_0b : 1;
	});

	REGISTER(reg02c,
	{
		int tile_x_bd_0c : 9;
		int tile_x_bd_0d : 9;
		int tile_x_bd_0e : 9;
        int msb_bit_of_tile_x_bd_0c : 1;
        int msb_bit_of_tile_x_bd_0d : 1;
        int msb_bit_of_tile_x_bd_0e : 1;
	});

	REGISTER(reg030,
	{
		int tile_x_bd_0f : 9;
		int tile_x_bd_10 : 9;
		int tile_x_bd_11 : 9;
        int msb_bit_of_tile_x_bd_0f : 1;
        int msb_bit_of_tile_x_bd_10 : 1;
        int msb_bit_of_tile_x_bd_11 : 1;
	});

	REGISTER(reg034,
	{
		int tile_x_bd_12 : 9;
		int tile_x_bd_13 : 9;
		int tile_x_bd_14 : 9;
        int msb_bit_of_tile_x_bd_12 : 1;
        int msb_bit_of_tile_x_bd_13 : 1;
        int msb_bit_of_tile_x_bd_14 : 1;
	});

	REGISTER(reg038,
	{
		int tile_x_bd_15 : 9;
		int tile_x_bd_16 : 9;
		int tile_x_bd_17 : 9;
        int msb_bit_of_tile_x_bd_15 : 1;
        int msb_bit_of_tile_x_bd_16 : 1;
        int msb_bit_of_tile_x_bd_17 : 1;
	});

	REGISTER(reg03c,
	{
		int tile_x_bd_18 : 9;
		int tile_x_bd_19 : 9;
		int tile_x_bd_1a : 9;
        int msb_bit_of_tile_x_bd_18 : 1;
        int msb_bit_of_tile_x_bd_19 : 1;
        int msb_bit_of_tile_x_bd_1a : 1;
	});

	REGISTER(reg040,
	{
		int tile_x_bd_1b : 9;
		int tile_x_bd_1c : 9;
		int tile_x_bd_1d : 9;
        int msb_bit_of_tile_x_bd_1b : 1;
        int msb_bit_of_tile_x_bd_1c : 1;
        int msb_bit_of_tile_x_bd_1d : 1;
	});

	REGISTER(reg044,
	{
		int tile_x_bd_1e : 9;
		int tile_x_bd_1f : 9;
        int reserved_9bit: 9;
        int msb_bit_of_tile_x_bd_1e : 1;
        int msb_bit_of_tile_x_bd_1f : 1;
	});

	REGISTER(reg048,
	{
		int tile_y_bd_00 : 8;
		int tile_y_bd_01 : 8;
		int tile_y_bd_02 : 8;
		int tile_y_bd_03 : 8;
	});

	REGISTER(reg04c,
	{
		int tile_y_bd_04 : 8;
		int tile_y_bd_05 : 8;
		int tile_y_bd_06 : 8;
		int tile_y_bd_07 : 8;
	});

	REGISTER(reg050,
	{
		int tile_y_bd_08 : 8;
		int tile_y_bd_09 : 8;
		int tile_y_bd_0a : 8;
		int tile_y_bd_0b : 8;
	});

	REGISTER(reg054,
	{
		int tile_y_bd_0c : 8;
		int tile_y_bd_0d : 8;
		int tile_y_bd_0e : 8;
		int tile_y_bd_0f : 8;
	});

	REGISTER(reg058,
	{
		int tile_y_bd_10 : 8;
		int tile_y_bd_11 : 8;
		int tile_y_bd_12 : 8;
		int tile_y_bd_13 : 8;
	});

	REGISTER(reg05c,
	{
		int tile_y_bd_14 : 8;
		int tile_y_bd_15 : 8;
		int tile_y_bd_16 : 8;
		int tile_y_bd_17 : 8;
	});

	REGISTER(reg060,
	{
		int tile_y_bd_18 : 8;
		int tile_y_bd_19 : 8;
		int tile_y_bd_1a : 8;
		int tile_y_bd_1b : 8;
	});

	REGISTER(reg064,
	{
		int tile_y_bd_1c : 8;
		int tile_y_bd_1d : 8;
		int tile_y_bd_1e : 8;
		int tile_y_bd_1f : 8;
	});

	REGISTER(reg068,
	{
		int tile_y_bd_20 : 8;
		int tile_y_bd_21 : 8;
		int tile_y_bd_22 : 8;
		int tile_y_bd_23 : 8;
	});

	REGISTER(reg06c,
	{
		int tile_y_bd_24 : 8;
		int tile_y_bd_25 : 8;
		int tile_y_bd_26 : 8;
		int tile_y_bd_27 : 8;
	});

	REGISTER(reg070,
	{
		int tile_y_bd_28 : 8;
		int tile_y_bd_29 : 8;
		int tile_y_bd_2a : 8;
		int tile_y_bd_2b : 8;
	});

	REGISTER(reg074,
	{
		int tile_y_bd_2c : 8;
		int tile_y_bd_2d : 8;
		int tile_y_bd_2e : 8;
		int tile_y_bd_2f : 8;
	});

	REGISTER(reg078,
	{
		int tile_y_bd_30 : 8;
		int tile_y_bd_31 : 8;
		int tile_y_bd_32 : 8;
		int tile_y_bd_33 : 8;
	});


	REGISTER(reg07c,
	{
		int tile_y_bd_34 : 8;
		int tile_y_bd_35 : 8;
		int tile_y_bd_36 : 8;
		int tile_y_bd_37 : 8;
	});

	REGISTER(reg080,
	{
		int tile_y_bd_38 : 8;
		int tile_y_bd_39 : 8;
		int tile_y_bd_3a : 8;
		int tile_y_bd_3b : 8;
	});

	REGISTER(reg084,
	{
		int tile_y_bd_3c : 8;
		int tile_y_bd_3d : 8;
		int tile_y_bd_3e : 8;
		int tile_y_bd_3f : 8;
	});

	REGISTER(reg088,
	{
		int loop_filter_across_tiles_enabled_flag : 1;
		int loop_filter_across_slc_enabled_flag : 1;
		int deblocking_filter_control_present_flag : 1;
		int deblocking_filter_override_enabled_flag : 1;
		int pps_deblocking_filter_disabled_flag : 1;
		int log2_parallel_merge_lvl_m2      : 3;
		int pic_beta_offset_div2            : 4;
		int pic_tc_offset_div2              : 4;
	});

	REGISTER2(reg08c,
	{
		unsigned int wd_load : 32;
	},
	{
		int poc : 32;
	});

	REGISTER(reg090,
	{
		int slice_nr : 18;
	});

	REGISTER(reg094,
	{
		unsigned int bs_cmd_addr : 32;
	});

	REGISTER(reg098,
	{
		unsigned int rpic_addr : 32;
	});

	REGISTER(reg09c,
	{
		unsigned int rlist_addr : 32;
	});

	REGISTER(reg0a0,
	{
		int reserved;
	});

	REGISTER(reg0a4,
	{
		unsigned int mb_info_addr : 32;
	});

	REGISTER(reg0a8,
	{
		unsigned int bin_uri_addr : 32;
	});

	REGISTER(reg0ac,
	{
		int reserved;
	});

	REGISTER(reg0b0,
	{
		unsigned int ilf_upper_row_addr : 32;
	});

	REGISTER(reg0b4,
	{
		int reserved;
	});

	REGISTER(reg0b8,
	{
		unsigned int ilf_rec_y : 32;
	});

	REGISTER(reg0bc,
	{
		unsigned int ilf_rec_uv : 32;
	});

	REGISTER(reg0c0,
	{
		unsigned int y_line_ofs : 16;
		unsigned int uv_line_ofs : 16;
	});

	REGISTER(reg0c4,
	{
		unsigned int iqt_qs_addr : 32;
	});

	REGISTER(reg0c8,
	{
        int reserved;
	});

	REGISTER(reg0cc,
	{
		unsigned int dma_conf : 32;
	});

	REGISTER(reg0d0,
	{
		int len_slice_addr      : 5;
		int pic_num_neg_pics    : 5;
		int pic_num_pos_pics    : 5;
		int pic_num_delta_poc   : 5;
		int pic_num_lt_sps      : 6;
		int pic_sum_num_lt      : 6;
	});

	REGISTER(reg0d4,
	{
		int len_list_entry : 4;
	});

	REGISTER(reg0d8,
	{
		unsigned int bin_out_addr : 32;
	});

	REGISTER(reg0dc,
	{
		int pic_fsh_int_en      : 1;
		int bsdma_end_int_en    : 1;
		int reserved0           : 2;
		int bin_end_int_en      : 1;
        int hw_timeout_int_en   : 1;
	};
	int int_en);

	REGISTER2(reg0e0,
	{
		unsigned int bin_buf_size : 32;
	},
	{
		unsigned int wd_load : 32;
	});

	REGISTER(reg0e4,
	{
		int bin_out_bit_len : 32;
	});

	REGISTER(reg0e8,
	{
		int bs_consumed_bit_len : 32;
	});

	REGISTER(reg0ec,
	{
		int eced_max_dec_len : 32;
	});

	REGISTER(reg0f0,
	{
		volatile int ilf_chk_sum : 32;
	});

	REGISTER(reg0f4,
	{
		int eced_max_ctb_nr : 32;
	});

	REGISTER(reg0f8,
	{
		int sp_pos_sel_option           : 1;
		int ilf_chroma_q_ofs_option     : 1;
		int ira_hm91_en                 : 1;
		int ad_ver_ctl                  : 5;
		int burst_pcm_en                : 1;
		int slice_en                    : 1;
	};
	int hm_ver_ctrl);

	REGISTER(reg0fc,
	{
		int pps_ilf_cb_qp_ofs               : 5;
		int pps_ilf_cr_qp_ofs               : 5;
		int strong_intra_smoothing_en_flag  : 1;
		int num_extra_slice_header_bits     : 3;
	});

	REGISTER2(reg100,
	{
		int pic_cycle_cnt_ad    : 1;
		int ctrl_state          : 1;
		int AD_dbg              : 4;
	},
	{
		int pic_cycle_cnt_bin   : 1;
		int fsm_state           : 1;
		int fifo_status         : 2;
		int reserved            : 1;
		int ctb_pos             : 3;
	});

	REGISTER2(reg104,
	{
		unsigned int bin_dma_cmd_addr : 32;
	},
	{
		unsigned int mv_pred_err_res_best_ref_addr_y : 32;
	});

	REGISTER2(reg108,
	{
		int max_tile_row_len : 32;
	},
	{
		unsigned int mv_pred_err_res_best_ref_addr_uv : 32;
	});

	REGISTER(reg10c,
	{
		int mv_pred_err_res_best_poc : 16;
		int mv_pred_err_res_best_dpbidx : 5;
	});

	REGISTER(reg110,
	{
		int reserved;
	});

	REGISTER2(reg114,
	{
		int err_res_en              : 1;
		int err_res_tile_wpp_en     : 1;
		int err_corr_ad_en          : 4;
		int err_corr_ad_en_ot       : 6;
		int err_detect_ad_en        : 20;
	},
	{
		int reserved0               : 2;
		int err_corr_bin_en         : 14;
        int err_detection           : 1;
        int reserved1               : 15;
	});

	REGISTER(reg118,
	{
		int ext_err_trig_test_en    : 1;
		int ext_err_trig_cycle      : 15;
		int ext_err_trig_ctb_y      : 8;
		int ext_err_trig_ctb_x      : 8;
	});

	REGISTER(reg11c,
	{
		int num_of_conceal_mb : 32;
	});

	REGISTER(reg120,
	{
		int bit_depth_luma_minus8       : 2;
		/* mc dma means how to read the dram data into internal sram */
		int mc_dma_mode_y               : 2; /* [0] 8 => not dec => 8		[1] 8(compressd) => dec => 10		[2] 10 => not dec => 10 */
		int reserved0                   : 4;
		int bit_depth_chroma_minus8     : 2;
		int mc_dma_mode_uv              : 2; /* [0] 8 => not dec => 8            [1] 8(compressd) => dec => 10      [2] 10 => not dec => 10 */
		int reserved1                   : 1;
		int decode_pic_order_cnt_lsb    : 1;
        int reserved2                   : 18;
	});

	REGISTER(reg124,
	{
		unsigned int ctb_num_in_slc_addr : 32;
	});

	REGISTER(reg128,
	/* support 4096 height, add by CW */
	{
		int tile_y_bd_00 : 1;
		int tile_y_bd_01 : 1;
		int tile_y_bd_02 : 1;
		int tile_y_bd_03 : 1;
		int tile_y_bd_04 : 1;
		int tile_y_bd_05 : 1;
		int tile_y_bd_06 : 1;
		int tile_y_bd_07 : 1;
		int tile_y_bd_08 : 1;
		int tile_y_bd_09 : 1;
		int tile_y_bd_0a : 1;
		int tile_y_bd_0b : 1;
		int tile_y_bd_0c : 1;
		int tile_y_bd_0d : 1;
		int tile_y_bd_0e : 1;
		int tile_y_bd_0f : 1;
		int tile_y_bd_10 : 1;
		int tile_y_bd_11 : 1;
		int tile_y_bd_12 : 1;
		int tile_y_bd_13 : 1;
		int tile_y_bd_14 : 1;
		int tile_y_bd_15 : 1;
		int tile_y_bd_16 : 1;
		int tile_y_bd_17 : 1;
		int tile_y_bd_18 : 1;
		int tile_y_bd_19 : 1;
		int tile_y_bd_1a : 1;
		int tile_y_bd_1b : 1;
		int tile_y_bd_1c : 1;
		int tile_y_bd_1d : 1;
		int tile_y_bd_1e : 1;
		int tile_y_bd_1f : 1;
	});

	REGISTER(reg12c,
	{
		int tile_y_bd_20 : 1;
		int tile_y_bd_21 : 1;
		int tile_y_bd_22 : 1;
		int tile_y_bd_23 : 1;
		int tile_y_bd_24 : 1;
		int tile_y_bd_25 : 1;
		int tile_y_bd_26 : 1;
		int tile_y_bd_27 : 1;
		int tile_y_bd_28 : 1;
		int tile_y_bd_29 : 1;
		int tile_y_bd_2a : 1;
		int tile_y_bd_2b : 1;
		int tile_y_bd_2c : 1;
		int tile_y_bd_2d : 1;
		int tile_y_bd_2e : 1;
		int tile_y_bd_2f : 1;
		int tile_y_bd_30 : 1;
		int tile_y_bd_31 : 1;
		int tile_y_bd_32 : 1;
		int tile_y_bd_33 : 1;
		int tile_y_bd_34 : 1;
		int tile_y_bd_35 : 1;
		int tile_y_bd_36 : 1;
		int tile_y_bd_37 : 1;
		int tile_y_bd_38 : 1;
		int tile_y_bd_39 : 1;
		int tile_y_bd_3a : 1;
		int tile_y_bd_3b : 1;
		int tile_y_bd_3c : 1;
		int tile_y_bd_3d : 1;
		int tile_y_bd_3e : 1;
		int tile_y_bd_3f : 1;
	});

	REGISTER(reg130,
	{
		unsigned int ilf_upper_info_addr : 32;
	});

	REGISTER(reg134,
	{
		unsigned int ilf_upper_si_addr : 32;
	});

	REGISTER(reg138,
	{
		int ec_lossy_mode_y : 8;
		int ec_lossy_mode_uv : 8;

	});

	REGISTER(reg13c,
	{
		int tile_y_bd_40 : 9;
		int tile_y_bd_41 : 9;
		int tile_y_bd_42 : 9;
        int msb_bit_of_tile_y_bd_40 : 1;
        int msb_bit_of_tile_y_bd_41 : 1;
        int msb_bit_of_tile_y_bd_42 : 1;
	});

	REGISTER(reg140,
	{
		int msb_bit_of_tile_y_bd_00 : 1;
		int msb_bit_of_tile_y_bd_01 : 1;
		int msb_bit_of_tile_y_bd_02 : 1;
		int msb_bit_of_tile_y_bd_03 : 1;
		int msb_bit_of_tile_y_bd_04 : 1;
		int msb_bit_of_tile_y_bd_05 : 1;
		int msb_bit_of_tile_y_bd_06 : 1;
		int msb_bit_of_tile_y_bd_07 : 1;
		int msb_bit_of_tile_y_bd_08 : 1;
		int msb_bit_of_tile_y_bd_09 : 1;
		int msb_bit_of_tile_y_bd_0a : 1;
		int msb_bit_of_tile_y_bd_0b : 1;
		int msb_bit_of_tile_y_bd_0c : 1;
		int msb_bit_of_tile_y_bd_0d : 1;
		int msb_bit_of_tile_y_bd_0e : 1;
		int msb_bit_of_tile_y_bd_0f : 1;
		int msb_bit_of_tile_y_bd_10 : 1;
		int msb_bit_of_tile_y_bd_11 : 1;
		int msb_bit_of_tile_y_bd_12 : 1;
		int msb_bit_of_tile_y_bd_13 : 1;
		int msb_bit_of_tile_y_bd_14 : 1;
		int msb_bit_of_tile_y_bd_15 : 1;
		int msb_bit_of_tile_y_bd_16 : 1;
		int msb_bit_of_tile_y_bd_17 : 1;
		int msb_bit_of_tile_y_bd_18 : 1;
		int msb_bit_of_tile_y_bd_19 : 1;
		int msb_bit_of_tile_y_bd_1a : 1;
		int msb_bit_of_tile_y_bd_1b : 1;
		int msb_bit_of_tile_y_bd_1c : 1;
		int msb_bit_of_tile_y_bd_1d : 1;
		int msb_bit_of_tile_y_bd_1e : 1;
		int msb_bit_of_tile_y_bd_1f : 1;
	});

	REGISTER(reg144,
	{
		int msb_bit_of_tile_y_bd_20 : 1;
		int msb_bit_of_tile_y_bd_21 : 1;
		int msb_bit_of_tile_y_bd_22 : 1;
		int msb_bit_of_tile_y_bd_23 : 1;
		int msb_bit_of_tile_y_bd_24 : 1;
		int msb_bit_of_tile_y_bd_25 : 1;
		int msb_bit_of_tile_y_bd_26 : 1;
		int msb_bit_of_tile_y_bd_27 : 1;
		int msb_bit_of_tile_y_bd_28 : 1;
		int msb_bit_of_tile_y_bd_29 : 1;
		int msb_bit_of_tile_y_bd_2a : 1;
		int msb_bit_of_tile_y_bd_2b : 1;
		int msb_bit_of_tile_y_bd_2c : 1;
		int msb_bit_of_tile_y_bd_2d : 1;
		int msb_bit_of_tile_y_bd_2e : 1;
		int msb_bit_of_tile_y_bd_2f : 1;
		int msb_bit_of_tile_y_bd_30 : 1;
		int msb_bit_of_tile_y_bd_31 : 1;
		int msb_bit_of_tile_y_bd_32 : 1;
		int msb_bit_of_tile_y_bd_33 : 1;
		int msb_bit_of_tile_y_bd_34 : 1;
		int msb_bit_of_tile_y_bd_35 : 1;
		int msb_bit_of_tile_y_bd_36 : 1;
		int msb_bit_of_tile_y_bd_37 : 1;
		int msb_bit_of_tile_y_bd_38 : 1;
		int msb_bit_of_tile_y_bd_39 : 1;
		int msb_bit_of_tile_y_bd_3a : 1;
		int msb_bit_of_tile_y_bd_3b : 1;
		int msb_bit_of_tile_y_bd_3c : 1;
		int msb_bit_of_tile_y_bd_3d : 1;
		int msb_bit_of_tile_y_bd_3e : 1;
		int msb_bit_of_tile_y_bd_3f : 1;
    });

	REGISTER(reg148,
	{
		int reserved;
	});

	REGISTER(reg14c,
	{
		int reserved;
	});

	REGISTER(reg150,
	{
		unsigned int rpic_mcinfo_addr : 32;
	});

	REGISTER(reg154,
	{
        int reserved;
	});

	REGISTER(reg158,
	{
		int reserved;
	});

	REGISTER(reg15c,
	{
		int reserved;
	});

	REGISTER(reg160,
	{
		int reserved;
	});

	REGISTER(reg164,
	{
		int reserved;
	});

	REGISTER(reg168,
	{
		int reserved;
	});

	REGISTER(reg16c,
	{
		int reserved;
	});

	REGISTER(reg170,
	{
		int reserved;
	});

	REGISTER(reg174,
	{
		int reserved;
	});

	REGISTER(reg178,
	{
		int reserved;
	});

	REGISTER(reg17c,
	{
		int reserved;
	});

	REGISTER(reg180,
	{
		int reserved;
	});

	REGISTER(reg184,
	{
		int reserved;
	});

	REGISTER(reg188,
	{
		int reserved;
	});

	REGISTER(reg18c,
	{
		int reserved;
	});

	REGISTER(reg190,
	{
		int reserved;
	});

	REGISTER(reg194,
	{
		int reserved;
	});

	REGISTER(reg198,
	{
		int reserved;
	});

	REGISTER(reg19c,
	{
		int reserved;
	});

	REGISTER(reg1a0,
	{
		int reserved;
	});

	REGISTER(reg1a4,
	{
		int reserved;
	});

	REGISTER(reg1a8,
	{
		int reserved;
	});

	REGISTER(reg1ac,
	{
		int reserved;
	});

	REGISTER(reg1b0,
	{
		int reserved;
	});

	REGISTER(reg1b4,
	{
		int reserved;
	});

	REGISTER(reg1b8,
	{
		int reserved;
	});

	REGISTER(reg1bc,
	{
		int reserved;
	});

	REGISTER(reg1c0,
	{
		int reserved;
	});

	REGISTER(reg1c4,
	{
		int reserved;
	});

	REGISTER(reg1c8,
	{
		int reserved;
	});

	REGISTER(reg1cc,
	{
		int reserved;
	});

	REGISTER(reg1d0,
	{
		int extra_wr_en         : 1;
        int reserved0           : 3;
        int extra_wr_uv_swap    : 1;
        int reserved1           : 3;
		int extra_wr_mode       : 4;
        int extra_wr_sce_en     : 1;
        int reserved2           : 19;
	});

	REGISTER(reg1d4,
	{
		unsigned int extra_wr_rec_addr_y : 32;
	});

	REGISTER(reg1d8,
	{
		unsigned int extra_wr_rec_addr_uv : 32;
	});

	REGISTER(reg1dc,
	{
		int extra_wr_rec_y_line_ofs : 16;
		int extra_wr_rec_uv_line_ofs : 16;
	});

	REGISTER(reg1e0,
	{
		int first_wr_uv_swap    : 1;
        int reserved            : 31;
	});

	REGISTER(reg1e4,
	{
		int reserved;
    });

	REGISTER(reg1e8,
    {
		int reserved;
    });

	REGISTER(reg1ec,
	{
		int reserved;
	});

	REGISTER(reg1f0,
	{
		int reserved;
	});

	REGISTER(reg1f4,
	{
		int reserved;
	});

	REGISTER(reg1f8,
	{
		int reserved;
	});

	/* support 4096 height, add by CW */
	REGISTER(reg1fc,
	{
		int tile_y_bd_00 : 1;
		int tile_y_bd_01 : 1;
		int tile_y_bd_02 : 1;
		int tile_y_bd_03 : 1;
		int tile_y_bd_04 : 1;
		int tile_y_bd_05 : 1;
		int tile_y_bd_06 : 1;
		int tile_y_bd_07 : 1;
		int tile_y_bd_08 : 1;
		int tile_y_bd_09 : 1;
		int tile_y_bd_0a : 1;
		int tile_y_bd_0b : 1;
		int tile_y_bd_0c : 1;
		int tile_y_bd_0d : 1;
		int tile_y_bd_0e : 1;
		int tile_y_bd_0f : 1;
		int tile_y_bd_10 : 1;
		int tile_y_bd_11 : 1;
		int tile_y_bd_12 : 1;
		int tile_y_bd_13 : 1;
		int tile_y_bd_14 : 1;
		int tile_y_bd_15 : 1;
		int tile_y_bd_16 : 1;
		int tile_y_bd_17 : 1;
		int tile_y_bd_18 : 1;
		int tile_y_bd_19 : 1;
		int tile_y_bd_1a : 1;
		int tile_y_bd_1b : 1;
		int tile_y_bd_1c : 1;
		int tile_y_bd_1d : 1;
		int tile_y_bd_1e : 1;
		int tile_y_bd_1f : 1;
	});

	REGISTER(reg200,
	{
		int tile_y_bd_20 : 1;
		int tile_y_bd_21 : 1;
		int tile_y_bd_22 : 1;
		int tile_y_bd_23 : 1;
		int tile_y_bd_24 : 1;
		int tile_y_bd_25 : 1;
		int tile_y_bd_26 : 1;
		int tile_y_bd_27 : 1;
		int tile_y_bd_28 : 1;
		int tile_y_bd_29 : 1;
		int tile_y_bd_2a : 1;
		int tile_y_bd_2b : 1;
		int tile_y_bd_2c : 1;
		int tile_y_bd_2d : 1;
		int tile_y_bd_2e : 1;
		int tile_y_bd_2f : 1;
		int tile_y_bd_30 : 1;
		int tile_y_bd_31 : 1;
		int tile_y_bd_32 : 1;
		int tile_y_bd_33 : 1;
		int tile_y_bd_34 : 1;
		int tile_y_bd_35 : 1;
		int tile_y_bd_36 : 1;
		int tile_y_bd_37 : 1;
		int tile_y_bd_38 : 1;
		int tile_y_bd_39 : 1;
		int tile_y_bd_3a : 1;
		int tile_y_bd_3b : 1;
		int tile_y_bd_3c : 1;
		int tile_y_bd_3d : 1;
		int tile_y_bd_3e : 1;
		int tile_y_bd_3f : 1;
	});

	REGISTER(reg204,
	{
		int reserved;
	});

    REGISTER(reg208,
	{
		int msb_bit_of_tile_x_bd_00 : 1;
		int msb_bit_of_tile_x_bd_01 : 1;
		int msb_bit_of_tile_x_bd_02 : 1;
		int msb_bit_of_tile_x_bd_03 : 1;
		int msb_bit_of_tile_x_bd_04 : 1;
		int msb_bit_of_tile_x_bd_05 : 1;
		int msb_bit_of_tile_x_bd_06 : 1;
		int msb_bit_of_tile_x_bd_07 : 1;
		int msb_bit_of_tile_x_bd_08 : 1;
		int msb_bit_of_tile_x_bd_09 : 1;
		int msb_bit_of_tile_x_bd_0a : 1;
		int msb_bit_of_tile_x_bd_0b : 1;
		int msb_bit_of_tile_x_bd_0c : 1;
		int msb_bit_of_tile_x_bd_0d : 1;
		int msb_bit_of_tile_x_bd_0e : 1;
		int msb_bit_of_tile_x_bd_0f : 1;
		int msb_bit_of_tile_x_bd_10 : 1;
		int msb_bit_of_tile_x_bd_11 : 1;
		int msb_bit_of_tile_x_bd_12 : 1;
		int msb_bit_of_tile_x_bd_13 : 1;
		int msb_bit_of_tile_x_bd_14 : 1;
		int msb_bit_of_tile_x_bd_15 : 1;
		int msb_bit_of_tile_x_bd_16 : 1;
		int msb_bit_of_tile_x_bd_17 : 1;
		int msb_bit_of_tile_x_bd_18 : 1;
		int msb_bit_of_tile_x_bd_19 : 1;
		int msb_bit_of_tile_x_bd_1a : 1;
		int msb_bit_of_tile_x_bd_1b : 1;
		int msb_bit_of_tile_x_bd_1c : 1;
		int msb_bit_of_tile_x_bd_1d : 1;
		int msb_bit_of_tile_x_bd_1e : 1;
		int msb_bit_of_tile_x_bd_1f : 1;
	});

    REGISTER(reg20c,
	{
		int msb_bit_of_tile_y_bd_00 : 1;
		int msb_bit_of_tile_y_bd_01 : 1;
		int msb_bit_of_tile_y_bd_02 : 1;
		int msb_bit_of_tile_y_bd_03 : 1;
		int msb_bit_of_tile_y_bd_04 : 1;
		int msb_bit_of_tile_y_bd_05 : 1;
		int msb_bit_of_tile_y_bd_06 : 1;
		int msb_bit_of_tile_y_bd_07 : 1;
		int msb_bit_of_tile_y_bd_08 : 1;
		int msb_bit_of_tile_y_bd_09 : 1;
		int msb_bit_of_tile_y_bd_0a : 1;
		int msb_bit_of_tile_y_bd_0b : 1;
		int msb_bit_of_tile_y_bd_0c : 1;
		int msb_bit_of_tile_y_bd_0d : 1;
		int msb_bit_of_tile_y_bd_0e : 1;
		int msb_bit_of_tile_y_bd_0f : 1;
		int msb_bit_of_tile_y_bd_10 : 1;
		int msb_bit_of_tile_y_bd_11 : 1;
		int msb_bit_of_tile_y_bd_12 : 1;
		int msb_bit_of_tile_y_bd_13 : 1;
		int msb_bit_of_tile_y_bd_14 : 1;
		int msb_bit_of_tile_y_bd_15 : 1;
		int msb_bit_of_tile_y_bd_16 : 1;
		int msb_bit_of_tile_y_bd_17 : 1;
		int msb_bit_of_tile_y_bd_18 : 1;
		int msb_bit_of_tile_y_bd_19 : 1;
		int msb_bit_of_tile_y_bd_1a : 1;
		int msb_bit_of_tile_y_bd_1b : 1;
		int msb_bit_of_tile_y_bd_1c : 1;
		int msb_bit_of_tile_y_bd_1d : 1;
		int msb_bit_of_tile_y_bd_1e : 1;
		int msb_bit_of_tile_y_bd_1f : 1;
	});

    REGISTER(reg210,
	{
		int msb_bit_of_tile_y_bd_20 : 1;
		int msb_bit_of_tile_y_bd_21 : 1;
		int msb_bit_of_tile_y_bd_22 : 1;
		int msb_bit_of_tile_y_bd_23 : 1;
		int msb_bit_of_tile_y_bd_24 : 1;
		int msb_bit_of_tile_y_bd_25 : 1;
		int msb_bit_of_tile_y_bd_26 : 1;
		int msb_bit_of_tile_y_bd_27 : 1;
		int msb_bit_of_tile_y_bd_28 : 1;
		int msb_bit_of_tile_y_bd_29 : 1;
		int msb_bit_of_tile_y_bd_2a : 1;
		int msb_bit_of_tile_y_bd_2b : 1;
		int msb_bit_of_tile_y_bd_2c : 1;
		int msb_bit_of_tile_y_bd_2d : 1;
		int msb_bit_of_tile_y_bd_2e : 1;
		int msb_bit_of_tile_y_bd_2f : 1;
		int msb_bit_of_tile_y_bd_30 : 1;
		int msb_bit_of_tile_y_bd_31 : 1;
		int msb_bit_of_tile_y_bd_32 : 1;
		int msb_bit_of_tile_y_bd_33 : 1;
		int msb_bit_of_tile_y_bd_34 : 1;
		int msb_bit_of_tile_y_bd_35 : 1;
		int msb_bit_of_tile_y_bd_36 : 1;
		int msb_bit_of_tile_y_bd_37 : 1;
		int msb_bit_of_tile_y_bd_38 : 1;
		int msb_bit_of_tile_y_bd_39 : 1;
		int msb_bit_of_tile_y_bd_3a : 1;
		int msb_bit_of_tile_y_bd_3b : 1;
		int msb_bit_of_tile_y_bd_3c : 1;
		int msb_bit_of_tile_y_bd_3d : 1;
		int msb_bit_of_tile_y_bd_3e : 1;
		int msb_bit_of_tile_y_bd_3f : 1;
	});
}
ST_HEVC_REG;

typedef struct
{
	struct vos_list_head listR, listO; /* listR for reference, listO for output. They are used for frame buffer only. */
	union
	{
		struct vos_list_head listCabacBuf;	/* used for cabac buffer only */
		struct vos_list_head listFrameBuf;
	};
	unsigned int in_use;
	int retry;
	unsigned int output_senario;
	int PicOrderCntVal;
	int pic_order_cnt_lsb;
	int used_for_reference;	/* 1 : short term,  2 : long term */
	uintptr_t bufferAddr;	/* Allocated, Physical */
	unsigned int bufferSize;
	unsigned int bufferStride;
	uintptr_t mbInfoAddr;	/* Allocated, Physical */
	unsigned int mbInfoSize;
	unsigned int totalSize;
	unsigned int bit_depth_luma_minus8;
	unsigned int bit_depth_chroma_minus8;
	unsigned int pic_width_in_luma_samples;
	unsigned int pic_height_in_luma_samples;
	int paired_idx;	/* another parity of picutre to compose one frame used by interlaced picture */
	int resolution;
	int dummy;
	int order;

	/***************************/
	int va_idx;      /* real index frame buffer in VA */
	/***************************/
	uintptr_t bufferAddr2nd;
	unsigned int bufferSize2nd;
	unsigned int bufferStride2nd;
	/***************************/
	unsigned int tid_plus1; //Fandi
}
ST_BUFFER;

typedef struct
{
	struct vos_list_head list; /* link to pic_list in order for BIN */
	unsigned int leakage;
	unsigned int owner;
	int non_referenced;
	int retry;
	int order;
	int dummy;
	int bumped;
	int resolution;
	int numOfUserData;
	unsigned int output_senario;
	int smallPictureSetting;
	int num_of_conceal_mb;
	int max_dec_len;
	unsigned int pic_width_in_luma_samples;
	unsigned int pic_height_in_luma_samples;
	unsigned int bit_depth_luma_minus8;
	unsigned int bit_depth_chroma_minus8;
	unsigned int sps_max_sub_layers_minus1;
	unsigned int sps_max_dec_pic_buffering;
	unsigned int sps_max_num_reorder_pics;
	unsigned short conf_win_left_offset;
	unsigned short conf_win_right_offset;
	unsigned short conf_win_top_offset;
	unsigned short conf_win_bottom_offset;
	unsigned char  video_full_range_flag;
	unsigned int num_slices;

	uintptr_t iqt_qs_addr;	/* Allocate Virtual */
    uintptr_t iqt_qs_addr_pa;
	uintptr_t rpic_addr;	/* Allocate Virtual */
    uintptr_t rpic_addr_pa;
	uintptr_t rpic_mcinfo_addr;	/* Allocate Virtual */
    uintptr_t rpic_mcinfo_addr_pa;
	uintptr_t rlist_addr;	/* Allocate Virtual */
    uintptr_t rlist_addr_pa;
	uintptr_t bin_dma_cmd_addr;	/* Allocate Physical */
	unsigned int max_tile_row_length;

	uintptr_t max_ctb_addr;
	uintptr_t slice_dram_addr; /* Physical address */
	unsigned int num_extra_CabacBuffers;
	ST_BUFFER *firstCabacBuffer;
	ST_BUFFER *curCabacBuffer;
	int curCabacIndex; /* the curCabacBuffer position in firstCabacBuffer list */
	ST_BUFFER *frameBuffer;
	ST_SLICE stSlice;
	struct {
		int frIdx;      /* index of frame buffer */
		int isLongTerm;
		int pocComp;   /* true if it's POC <= POC of current picture */
		int rpicIdx;    /* index of reference picture */
	} RefPicList[2][MAX_HEVC_RLIST_ENTRY];
	int best_ref_idx;
	ST_HEVC_REG regs;

#if !NOT_PARSING_SEI
	SEI_USER_DATA_REGISTERED_ITU_T_T35 stUserDataITUTT35[MAX_HEVC_USER_DATA_NUM];
	SEI_FRAME_PACKING_ARRANGEMENT stFPA;
	SEI_HDR_COMPATIBILITY_INFO stHCI;
	SEI_MASTERING_DISPLAY_COLOUR_VOLUME stMDCV;
#endif
}
ST_PIC_RESOURCE;

struct _ST_HEVC_CONTEXT
{
	void *parent;
	void *myself;

	/* common info */
	unsigned int pic_width_in_luma_samples;
	unsigned int pic_height_in_luma_samples;
	unsigned int bit_depth_luma_minus8;
	unsigned int bit_depth_chroma_minus8;
	int progressive;
	int highestTid;
	int resolution;
	unsigned int sps_max_dec_pic_buffering;
	unsigned int sps_max_num_reorder_pics;
	int thumbnail;
	int new_memory_allocation;

	/* Performance fine-tune */
	int cur_order;
	int next_order;
	int max_decode_order;
	int max_display_order;
	int live_cvrt_frame_cnt;

	/* nal information set */
	PST_VPS pstVPS;
	PST_SPS pstSPS;
	PST_PPS pstPPS;

	ST_SLICE stSH[1];

#if !NOT_PARSING_SEI
	SEI_ACTIVE_PARAMETER_SET stAPS;
	SEI_PIC_TIMING stPicTiming;
	SEI_USER_DATA_REGISTERED_ITU_T_T35 stUserDataITUTT35[MAX_HEVC_USER_DATA_NUM];
	SEI_FRAME_PACKING_ARRANGEMENT stFPA;
	SEI_HDR_COMPATIBILITY_INFO stHCI;
	SEI_MASTERING_DISPLAY_COLOUR_VOLUME stMDCV;
#endif

	int decodable;
	int numOfUserData;
	int poc_random_access;
	unsigned char  prev_rap_is_bla;
	int poc_cra;
	int needToSeek;
	int prev_au_cpb_removal_delay_minus1;

	/* CABAC buffer */
	int numCabacBuf;
	ST_BUFFER cabacBuffer[MAX_HEVC_CABAC_NUMBER];	/* Adjust by CW, original array size is 128 */
	ST_BUFFER dummyCabacBuffer[1];

	/* frame buffer */
	int numFrameBuf;
	int numDPBuf;   /* used current, inclusive of dummy frame */
	struct vos_list_head frameList;
	struct vos_list_head dummyFrameList;
	struct vos_list_head freeList;
	void* fidMap[MAX_HEVC_FRAME_NUMBER];
	ST_BUFFER frameBuffer[MAX_HEVC_FRAME_NUMBER];

	/* mics buffer */
	unsigned int dma_conf;
	/**********************************************/
	/* can be placed in sram0 or dram */
	uintptr_t bin_uri_addr;
	uintptr_t upper_row_addr;
	uintptr_t upper_row_info_addr;
	uintptr_t upper_row_si_addr;
	/**********************************************/
	uintptr_t bsdma_addr[1];
	uintptr_t bsdma_addr_pa[1];

	/* picture resource */
	ST_PIC_RESOURCE picResource[MAX_PIC_RESOURCE_SIZE];

	/* picture list */
	struct vos_list_head picList;

	/* frame list */
	struct vos_list_head referenceList;
	struct vos_list_head outputList;

	void *curPic;		/* point to ST_PIC_RESOURCE * & used by HEVC_PARSER */
	void *curSlice;

	/*************************/
	/* DEBUG */
	int consecutive_no_frame_buf;
	/*************************/

	/*------- add by CW ---------*/
    int reserved_last_frm_flag;  /* for LosePicFlag=0 */
    unsigned int hw_timeout_value;
	int fw_disable;
    int uv_swap_flag;
    int ewt_uv_swap_flag;
    int ewt_sce_en;
	void *recordLastPic;	/* point to ST_PIC_RESOURCE * & used by record last HEVC_PARSER pic */
	int	chip_idx;
	int	chn_idx;
	int	ll_idx;
	unsigned int	hdal_max_ref_num;
	uintptr_t	ref_addr;
	uintptr_t	ref_mbinfo_addr;
	int	ref_poc;
	int	nal_type;               // record nal_type
	char		release_buf_list[MAX_HEVC_FRAME_NUMBER];
	int		release_buf_count;
	int		allocate_buf_idx;
	DecLLJob *hevcd_list_header;  // using by link list mode
	unsigned char	sw_timeout_flag;
	unsigned char	ctb_size;
	unsigned char	vps_sps_pps_flag;
	unsigned char	hdal_allocate_buffer_flag;
    unsigned char   tiles_enable_flag;
	unsigned int   max_dec_buf;
	unsigned int	b_drop_rec; //Fandi

	/* allocate max buffer size */
	unsigned int	max_qmat_size;
	unsigned int	max_bsdma_size;
	unsigned int	max_bin_bsdma_size;

	unsigned int	yuv_width_thrd;
	unsigned char	sub_yuv_ratio;
	unsigned char	sub_yuv_en;      // sub YUV enable
};
typedef struct _ST_HEVC_CONTEXT ST_HEVC_CONTEXT, *PST_HEVC_CONTEXT;

/* ----------------------------------------------------------- */
static inline EN_HEVC_Engine engType(unsigned int owner)
{
	return (EN_HEVC_Engine)(owner & (ENG_HEVC_INUSE - 1));
}

int resetAllResource(PST_HEVC_CONTEXT hevc);
int initAllEngine(int chip_idx);
void* getEngine(void* user, EN_HEVC_Engine type);
void* getEngineOwner(int chip_idx, int chn_idx, EN_HEVC_Engine type);
int freeEngine(void* user, void* engine);
EN_HEVC_Engine getEngineType(void *user, void *engine);
void *getNewPicture(PST_HEVC_CONTEXT hevc);
void undoGetPicture(void *res);
int freePicture(PST_HEVC_CONTEXT hevc, void *res);
int assignPictureTo(void *res, EN_HEVC_Engine newOwnerType);
int freeFrameBuffer(PST_HEVC_CONTEXT hevc, void *res);
int dropReferenceFrame(ST_HEVC_CONTEXT *hevc, void *res);
int displayOutputFrame(ST_HEVC_CONTEXT *hevc, void *res);
int dropOutputFrame(ST_HEVC_CONTEXT *hevc, void *res);
int assignFrame2ReferenceList(ST_HEVC_CONTEXT *hevc, void *res);
int assignFrame2OutputList(ST_HEVC_CONTEXT *hevc, void *res);
int assignFrameTo(ST_HEVC_CONTEXT *hevc, void *res, int need_for_output);

int try2triggerEngine(ST_HEVC_CONTEXT *hevc, EN_HEVC_Engine types);
int doEngineIsr(PST_HEVC_CONTEXT hevc, int *next_ad_st);
int doEngineIsrByPass(ST_HEVC_CONTEXT *hevc, int *next_ad_st);
void errorHandling2Trigger(PST_HEVC_CONTEXT hevc);
void TriggerBIN(PST_HEVC_CONTEXT hevc);

#if !NOT_PARSING_SEI
void addUserDataToPicture(void *res, SEI_USER_DATA_REGISTERED_ITU_T_T35 *usd);
void addFPAtoPicture(void *res, SEI_FRAME_PACKING_ARRANGEMENT *fpa);
void addHCItoPicture(void *res, SEI_HDR_COMPATIBILITY_INFO *hci);
void addMDCVtoPicture(void *res, SEI_MASTERING_DISPLAY_COLOUR_VOLUME *mdcv);
#endif

#endif
