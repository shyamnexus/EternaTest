#ifndef __VPS_HEVC_H__
#define __VPS_HEVC_H__

#include "hevc_rbspstream.h"
#include "../../h265dec_comm_def.h"

static const unsigned int SubWidthC[4] = { 1, 2, 2, 1 };
static const unsigned int SubHeightC[4] = { 1, 2, 1, 1 };

typedef struct
{
	struct
	{
		int nal_no : 7;
		int nal_type : 10;
		int payload_type : 9;
	};
}QHeader;

enum SliceType
{
	B_SLICE,
	P_SLICE,
	I_SLICE
};

typedef enum
{
	NAL_UNIT_ERROR = -1,
	NAL_UNIT_CODED_SLICE_TRAIL_N = 0,	/* 0 */
	NAL_UNIT_CODED_SLICE_TRAIL_R,		/* 1 */

	NAL_UNIT_CODED_SLICE_TSA_N,		/* 2 */
	NAL_UNIT_CODED_SLICE_TSA_R,		/* 3 */

	NAL_UNIT_CODED_SLICE_STSA_N,		/* 4 */
	NAL_UNIT_CODED_SLICE_STSA_R,		/* 5 */

	NAL_UNIT_CODED_SLICE_RADL_N,		/* 6 */
	NAL_UNIT_CODED_SLICE_RADL_R,		/* 7 */

	NAL_UNIT_CODED_SLICE_RASL_N,		/* 8 */
	NAL_UNIT_CODED_SLICE_RASL_R,		/* 9 */

	NAL_UNIT_RSV_VCL_N10,
	NAL_UNIT_RSV_VCL_R11,
	NAL_UNIT_RSV_VCL_N12,
	NAL_UNIT_RSV_VCL_R13,
	NAL_UNIT_RSV_VCL_N14,
	NAL_UNIT_RSV_VCL_R15,

	NAL_UNIT_CODED_SLICE_BLA_W_LP,		/* 16 */
	NAL_UNIT_CODED_SLICE_BLA_W_RADL,	/* 17 */
	NAL_UNIT_CODED_SLICE_BLA_N_LP,		/* 18 */
	NAL_UNIT_CODED_SLICE_IDR_W_RADL,	/* 19 */
	NAL_UNIT_CODED_SLICE_IDR_N_LP,		/* 20 */
	NAL_UNIT_CODED_SLICE_CRA_NUT,		/* 21 */
	NAL_UNIT_RSV_IRAP_VCL22,
	NAL_UNIT_RSV_IRAP_VCL23,

	NAL_UNIT_RESERVED_24,
	NAL_UNIT_RESERVED_25,
	NAL_UNIT_RESERVED_26,
	NAL_UNIT_RESERVED_27,
	NAL_UNIT_RESERVED_28,
	NAL_UNIT_RESERVED_29,
	NAL_UNIT_RESERVED_30,
	NAL_UNIT_RESERVED_31,

	NAL_UNIT_VPS,						/* 32 */
	NAL_UNIT_SPS,						/* 33 */
	NAL_UNIT_PPS,						/* 34 */
	NAL_UNIT_ACCESS_UNIT_DELIMITER,		/* 35 */
	NAL_UNIT_EOS,						/* 36 */
	NAL_UNIT_EOB,						/* 37 */
	NAL_UNIT_FILLER_DATA,				/* 38 */
	NAL_UNIT_SEI_PREFIX,				/* 39 Prefix SEI */
	NAL_UNIT_SEI_SUFFIX,				/* 40 Suffix SEI */
	NAL_UNIT_RESERVED_41,
	NAL_UNIT_RESERVED_42,
	NAL_UNIT_RESERVED_43,
	NAL_UNIT_RESERVED_44,
	NAL_UNIT_RESERVED_45,
	NAL_UNIT_RESERVED_46,
	NAL_UNIT_RESERVED_47,
	NAL_UNIT_UNSPECIFIED_48,
	NAL_UNIT_UNSPECIFIED_49,
	NAL_UNIT_UNSPECIFIED_50,
	NAL_UNIT_UNSPECIFIED_51,
	NAL_UNIT_UNSPECIFIED_52,
	NAL_UNIT_UNSPECIFIED_53,
	NAL_UNIT_UNSPECIFIED_54,
	NAL_UNIT_UNSPECIFIED_55,
	NAL_UNIT_UNSPECIFIED_56,
	NAL_UNIT_UNSPECIFIED_57,
	NAL_UNIT_UNSPECIFIED_58,
	NAL_UNIT_UNSPECIFIED_59,
	NAL_UNIT_UNSPECIFIED_60,
	NAL_UNIT_UNSPECIFIED_61,
	NAL_UNIT_UNSPECIFIED_62,
	NAL_UNIT_UNSPECIFIED_63,
	NAL_UNIT_INVALID,
	NAL_UNIT_STREAM_SYNC,				/* user defined */
}NalUnitType;

typedef enum
{
	SCALING_LIST_4x4 = 0,
	SCALING_LIST_8x8,
	SCALING_LIST_16x16,
	SCALING_LIST_32x32,
	SCALING_LIST_SIZE_NUM
}ScalingListSize;

typedef struct
{
  /*	  Explicit weighted prediction parameters parsed in slice header,
   *  or Implicit weighted prediction parameters (8 bits depth values)
   */
  unsigned char		bPresentFlag;
  unsigned int		uiLog2WeightDenom;
  int		iWeight;
  int		iOffset;

  /* Weighted prediction scaling values built from above parameters (bitdepth scaled) */
  int	w;
  int	o;
  int	offset;
  int	shift;
}wpScalingParam;

typedef struct
{
	unsigned char	general_profile_idc;	/* 5bit */
	unsigned char	general_progressive_source_flag;	/* 1bit */
	unsigned char	general_interlaced_source_flag;		/* 1bit */
	unsigned char	sub_layer_profile_present_flag[MAX_VPS_SUB_LAYER_SIZE];	/* 1bit */
	unsigned char	sub_layer_level_present_flag[MAX_VPS_SUB_LAYER_SIZE];		/* 1bit */
}PTL;	/* profile_tier_level */

typedef struct
{
	unsigned int	scaling_list_dc_coef[SCALING_LIST_SIZE_NUM][6];
	unsigned char	ScalingList[SCALING_LIST_SIZE_NUM][6][64];	/* FIXME */
}SLD;	/* scaling_list_data */

typedef struct
{
	/* hrd_parameters; */
	unsigned char	nal_hrd_parameters_present_flag;	/* 1bit */
	unsigned char	vcl_hrd_parameters_present_flag;	/* 1bit */
	unsigned char	sub_pic_hrd_params_present_flag;	/* 1bit */
	unsigned char	au_cpb_removal_delay_length_minus1;		/* 5bit */
}HP;

typedef struct
{
	unsigned int	NumNegativePics;
	unsigned int	NumPositivePics;
	unsigned int	NumDeltaPocs;
	int	DeltaPocS0[MAX_NUM_REF_PICS];	/* ue(v) */
	int	DeltaPocS1[MAX_NUM_REF_PICS];	/* ue(v) */
	unsigned char	UsedByCurrPicS0[MAX_NUM_REF_PICS];	/* 1bit */
	unsigned char	UsedByCurrPicS1[MAX_NUM_REF_PICS];	/* 1bit */

	unsigned int curr_num_delta_pocs;
}STRPS;	/* short_term_ref_pic_set */

typedef struct
{
	unsigned char	video_signal_type_present_flag;	/* 1bit */
	unsigned char	video_full_range_flag;	/* 1bit */
	unsigned char	field_seq_flag;	/* 1bit */
	unsigned char	frame_field_info_present_flag;	/* 1bit */
	unsigned char	vui_hrd_parameters_present_flag;	/* 1bit */

	HP		hrd_parameters;
}VUI;	/* vui_parameters */

typedef struct
{
    /* don't change the order
    *  if need add new member, add them in the last
    */
	QHeader	header;
	unsigned char	valid;
	PTL	profile_tier_level;
	unsigned char	vps_timing_info_present_flag;
	unsigned int	vps_num_units_in_tick;
	unsigned int	vps_time_scale;
	HP	hrd_parameters;
}ST_VPS, *PST_VPS;


typedef struct
{
    /* don't change the order
    *  if need add new member, add them in the last
    */
	QHeader	header;
	STRPS	*short_term_ref_pic_set;	/* the last one is local rps */
	unsigned char	valid;
	unsigned char	sps_video_parameter_set_id;
	unsigned char	sps_max_sub_layers_minus1;	/* 3bit */
	PTL	profile_tier_level;
	unsigned int	chroma_format_idc;	/* ue(v) */
	unsigned char	separate_colour_plane_flag;	/* 1bit */
	unsigned int	pic_width_in_luma_samples;		/* ue(v) */
	unsigned int	pic_height_in_luma_samples;	/* ue(v) */
	unsigned short	conf_win_left_offset;		/* ue(v) */
	unsigned short	conf_win_right_offset;		/* ue(v) */
	unsigned short	conf_win_top_offset;		/* ue(v) */
	unsigned short	conf_win_bottom_offset;	/* ue(v) */
	unsigned int	bit_depth_luma_minus8;		/* ue(v) */
	unsigned int	bit_depth_chroma_minus8;
	unsigned int	log2_max_pic_order_cnt_lsb_minus4;	/* ue(v) */
	unsigned int	sps_max_dec_pic_buffering[MAX_SPS_SUB_LAYER_SIZE];	/* ue(v) */
	unsigned int	sps_max_num_reorder_pics[MAX_SPS_SUB_LAYER_SIZE];		/* ue(v) */
	unsigned int	log2_min_luma_coding_block_size_minus3;		/* ue(v) */
	unsigned int	log2_diff_max_min_luma_coding_block_size;	/* ue(v) */
	unsigned int	log2_min_transform_block_size_minus2;		/* ue(v) */
	unsigned int	log2_diff_max_min_transform_block_size;		/* ue(v) */
	unsigned int	max_transform_hierarchy_depth_inter;		/* ue(v) */
	unsigned int	max_transform_hierarchy_depth_intra;		/* ue(v) */
	unsigned char	scaling_list_enable_flag;		/* 1bit */
	unsigned char	sps_scaling_list_data_present_flag;	/* 1bit */
	SLD	scaling_list_data;
	unsigned char	amp_enabled_flag;		/* 1bit */
	unsigned char	sample_adaptive_offset_enabled_flag;	/* 1bit */
	unsigned char	pcm_enabled_flag;		/* 1bit */
	unsigned char	pcm_sample_bit_depth_luma_minus1;	/* 4bit */
	unsigned char	pcm_sample_bit_depth_chroma_minus1;	/* 4bit */
	unsigned int	log2_min_pcm_luma_coding_block_size_minus3;		/* ue(v) */
	unsigned int	log2_diff_max_min_pcm_luma_coding_block_size;	/* ue(v) */
	unsigned char	pcm_loop_filter_disable_flag;		/* 1bit */
	unsigned int	num_short_term_ref_pic_sets;		/* ue(v) */
	unsigned char	long_term_ref_pics_present_flag;	/* 1bit */
	unsigned int	num_long_term_ref_pics_sps;		/* ue(v) */
	unsigned int	lt_ref_pic_poc_lsb_sps[MAX_LT_RP_SIZE];	/* u(v) */
	unsigned char	used_by_curr_pic_lt_sps_flag[MAX_LT_RP_SIZE];	/* 1bit */
	unsigned char	sps_temporal_mvp_enable_flag;		/* 1bit */
	unsigned char	strong_intra_smoothing_enable_flag;	/* 1bit */
	unsigned char	vui_parameters_present_flag;		/* 1bit */
	VUI	vui_parameters;

	/* ----- add by CW ----- */
	unsigned int	max_ref_num;
}ST_SPS, *PST_SPS;

typedef struct
{
    /* don't change the order
    *  if need add new member, add them in the last
    */
	QHeader	header;
	unsigned char	valid;
	unsigned int	pps_seq_parameter_set_id;	/* ue(v) */
	unsigned char	dependent_slice_segments_enabled_flag;	/* 1bit */
	unsigned char	output_flag_present_flag;	/* 1bit */
	unsigned char	num_extra_slice_header_bits;
	unsigned char	sign_data_hiding_enabled_flag;	/* 1bit */
	unsigned char	cabac_init_present_flag;	/* 1bit */
	unsigned int	num_ref_idx_l0_default_active_minus1;	/* ue(v) */
	unsigned int	num_ref_idx_l1_default_active_minus1;	/* ue(v) */
	int	init_qp_minus26;	/* se(v) */
	unsigned char	constrained_intra_pred_flag;	/* 1bit */
	unsigned char	transform_skip_enabled_flag;	/* 1bit */
	unsigned char	cu_qp_delta_enabled_flag;
	unsigned int	diff_cu_qp_delta_depth;	/* ue(v) */
	int	pps_cb_qp_offset;		/* se(v) */
	int	pps_cr_qp_offset;		/* se(v) */
	unsigned char	pps_slice_chroma_qp_offsets_present_flag;	/* 1bit */
	unsigned char	weighted_pred_flag;	/* 1bit */
	unsigned char	weighted_bipred_flag;	/* 1bit */
	unsigned char	transquant_bypass_enabled_flag;		/* 1bit */
	unsigned char	tiles_enabled_flag;	/* 1bit */
	unsigned char	entropy_coding_sync_enabled_flag;	/* 1bit */
	unsigned int	num_tile_columns_minus1;	/* ue(v) */
	unsigned int	num_tile_rows_minus1;	/* ue(v) */
	unsigned char	uniform_spacing_flag;	/* 1bit */
	unsigned char	to_calculate_each_tile_boundary;
	unsigned short	column_width[MaxTileCols];
	unsigned short	row_height[MaxTileRows];
	unsigned char	loop_filter_across_tiles_enabled_flag;	/* 1bit */
	unsigned char	pps_loop_filter_across_slices_enabled_flag;	/* 1bit */
	unsigned char	deblocking_filter_control_present_flag;
	unsigned char	deblocking_filter_override_enabled_flag;
	unsigned char	pps_deblocking_filter_disabled_flag;
	int	pps_beta_offset_div2;
	int	pps_tc_offset_div2;
	unsigned char	pps_scaling_list_data_present_flag;
	SLD	scaling_list_data;
	unsigned char	lists_modification_present_flag;
	unsigned int	log2_parallel_merge_level_minus2;
	unsigned char	slice_segment_header_extension_present_flag;

/* #ifdef HEVC_REE */
	/* TODO : The following members don't need to copy from VPS to VPU */
	unsigned int	smallPictureSetting;
	unsigned int	ctbSizeY;
	unsigned int	picWidthInCtbsY;
	unsigned int	picHeightInCtbsY;
	unsigned short	colBd[MaxTileCols + 1];
	unsigned short	rowBd[MaxTileRows + 1];
/* #endif */
}ST_PPS, *PST_PPS;

typedef struct
{
	QHeader	header;
	uintptr_t	sliceStartAddr;
	uintptr_t	sliceEndAddr;
	unsigned int	sliceSize;
	NalUnitType	nalType;
	unsigned char	valid;
	unsigned char	first_slice_segment_in_pic_flag;
	unsigned char	no_output_of_prior_pics_flag;
	unsigned char	slice_pic_parameter_set_id;
	unsigned int	slice_segment_address;

	/* begin of dependent slice information */
#define dependent_start	slice_type

	unsigned int	slice_type;
	unsigned char	pic_output_flag;
	int	pic_order_cnt_lsb;
	int	PicOrderCntVal;

	/* short & long term reference information  */
	STRPS	short_term_ref_pic_set;
	unsigned int	num_long_term_sps;
	unsigned int	num_long_term_ref_pics;
	int	pocLt[33];	/* u(v) */
	unsigned char	UsedByCurrPicLt[33];	/* 1bit */
	unsigned char	delta_poc_msb_present_flag[33];

	unsigned char	ref_pic_list_modification_flag_l0;
	unsigned char	ref_pic_list_modification_flag_l1;
	unsigned int	num_ref_idx_l0_active;
	unsigned int	num_ref_idx_l1_active;
	int	list_entry_l0[32];
	int	list_entry_l1[32];

	unsigned char	slice_sao_luma_flag;
	unsigned char	slice_sao_chroma_flag;
	unsigned char	mvd_l1_zero_flag;
	unsigned char	cabac_init_flag;
	unsigned char	collocated_from_l0_flag;
	unsigned int	collocated_ref_idx;
	unsigned int	five_minus_max_num_merge_cand;
	int	slice_qp;
	int	slice_cb_qp_offset;
	int	slice_cr_qp_offset;
	unsigned char	deblocking_filter_override_flag;
	unsigned char	slice_deblocking_filter_disabled_flag;
	int	slice_beta_offset_div2;
	int	slice_tc_offset_div2;
	unsigned char	slice_loop_filter_across_slices_enabled_flag;

	/* end of depnedent slice information */
#define dependent_end num_entry_point_offsets

	unsigned int	num_entry_point_offsets;

	/* -------------------------------------- */
	/* add by CW */
	NalUnitType	Pre_nalType;
	int	Pre_PicOrderCntVal;
	unsigned char	reset_poc_msb_flag;
	unsigned char	num_ref_idx_active_override_flag; //1bit
	unsigned char	temporal_id_plus1; //Fandi
}ST_SLICE, *PST_SLICE;

#define slice_dependent_size (offsetof(ST_SLICE, dependent_end) - offsetof(ST_SLICE, dependent_start))

#if !NOT_PARSING_SEI
typedef struct
{
	QHeader	header;

	unsigned char	valid;
	unsigned char	active_video_parameter_set_id;	/* 4bits */
	unsigned char	self_contained_cvs_flag;		/* 1bit */
	unsigned char	no_parameter_set_update_flag;	/* 1 bit */
	unsigned int	num_sps_ids_minus1;
	unsigned int	active_seq_parameter_set_id[MAX_SPS_NUM];
}SEI_ACTIVE_PARAMETER_SET;

typedef struct
{
	QHeader	header;

	unsigned char	valid;
	unsigned short	display_primaries_x[3];
	unsigned short	display_primaries_y[3];
	unsigned short	white_point_x;
	unsigned short	white_point_y;
	unsigned int	max_display_mastering_luminance;
	unsigned int	min_display_mastering_luminance;
}SEI_MASTERING_DISPLAY_COLOUR_VOLUME;

typedef struct
{
	QHeader	header;

	unsigned char	valid;
	unsigned char	hdr_transfer_characteristics_idc;
}SEI_HDR_COMPATIBILITY_INFO;

typedef struct
{
	QHeader	header;

	unsigned char	source_scan_type;
	unsigned char	duplicate_flag;
	unsigned int	au_cpb_removal_delay_minus1;
	unsigned int	pic_dpb_output_delay;
}SEI_PIC_TIMING;

typedef struct
{
	QHeader	header;

	unsigned char	valid;
	unsigned int	bp_seq_parameter_set_id;
	unsigned int	cpb_delay_offset;
	unsigned int	dpb_delay_offset;
	unsigned char	concatenation_flag;
	unsigned int	au_cpb_removal_delay_delta_minus1;
}SEI_BUFFERING_PERIOD;

typedef struct
{
	QHeader	header;
	unsigned int	len;
	unsigned char	data[128];
}SEI_USER_DATA_REGISTERED_ITU_T_T35;

typedef struct
{
	QHeader	header;
	unsigned int	len;
	unsigned char	data[128];
}SEI_FRAME_PACKING_ARRANGEMENT;
#endif

typedef struct
{
	void	*parent;

	/* common info */
	unsigned int	pic_width_in_luma_samples;
	unsigned int	pic_height_in_luma_samples;
	int	HighestTid;
	unsigned int	sps_max_dec_pic_buffering;
	unsigned int	sps_max_num_reorder_pics;

	/* nal information set */
	PST_VPS pstVPS;
	PST_SPS pstSPS;
	PST_PPS pstPPS;

	ST_SLICE	stSH[1];

	struct
	{
		int	nal_type;
		void		*rbsp;		/* unfinished rbsp stream */
		uintptr_t	startAddr;	/* stream position */
		uintptr_t	endAddr;		/* stream position */
	}
	nal_todo;

#if !NOT_PARSING_SEI
	SEI_ACTIVE_PARAMETER_SET	stAPS;
	SEI_PIC_TIMING	stPicTiming;
	SEI_BUFFERING_PERIOD	stBufferingPeriod;
	SEI_USER_DATA_REGISTERED_ITU_T_T35	stUserDataITUTT35;
	SEI_FRAME_PACKING_ARRANGEMENT	stFPA;
	SEI_HDR_COMPATIBILITY_INFO		stHCI;
	SEI_MASTERING_DISPLAY_COLOUR_VOLUME	stMDCV;
#endif
	hevc_rbspstream	rbspStream;

	int needToSeek;

	/*----------------------------------*/
	/* add by CW */
	int	res_id;	/* indicate the id to VPS/SPS/VPS */
	unsigned int	sei_payload_type;
	char		no_more_slice_flag;
	int	chip_idx;
}ST_VPS_HEVC_CONTEXT, *PST_VPS_HEVC_CONTEXT;

static inline int isIDR(NalUnitType type)
{
	return type == NAL_UNIT_CODED_SLICE_IDR_W_RADL
		|| type == NAL_UNIT_CODED_SLICE_IDR_N_LP;
}

static inline int isBLA(NalUnitType type)
{
	return type == NAL_UNIT_CODED_SLICE_BLA_W_LP
		|| type == NAL_UNIT_CODED_SLICE_BLA_W_RADL
		|| type == NAL_UNIT_CODED_SLICE_BLA_N_LP;
}

static inline int isCRA(NalUnitType type)
{
	return type == NAL_UNIT_CODED_SLICE_CRA_NUT;
}

static inline int isIRAP(NalUnitType type)
{
	return isIDR(type) || isBLA(type) || isCRA(type);
}

static inline int isIntra(int slice_type)
{
	return slice_type == I_SLICE;
}

static inline int isInterB(int slice_type)
{
	return slice_type == B_SLICE;
}

static inline int isInterP(int slice_type)
{
	return slice_type == P_SLICE;
}

#endif
