#ifndef _H264PS_VLD_H_
#define _H264PS_VLD_H_

#include "bitstream.h"
#include "rbspstream.h"

#define H264DEC_QMATSIZE              224
#define H264DEC_DEC_DPB_FRAME_SIZE    48  //!< TODO: check this value
#define H264DEC_RPIC_DPB_FRAME_SIZE   80

//AVC Profile IDC definitions
#define H264DEC_BASELINE              66      //!< YUV 4:2:0/8  "Baseline"
#define H264DEC_MAIN                  77      //!< YUV 4:2:0/8  "Main"
#define H264DEC_EXTENDED              88      //!< YUV 4:2:0/8  "Extended"
#define H264DEC_FREXT_HP              100     //!< YUV 4:2:0/8 "High"
#define H264DEC_FREXT_Hi10P           110     //!< YUV 4:2:0/10 "High 10"
#define H264DEC_MULTIVIEW_HIGH        118
#define H264DEC_FREXT_Hi422           122     //!< YUV 4:2:2/10 "High 4:2:2"
#define H264DEC_STEREO_HIGH           128
#define H264DEC_FREXT_Hi444           244     //!< YUV 4:4:4/14 "High 4:4:4"
#define H264DEC_FREXT_CAVLC444        44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"

//add for user data
#define H264DEC_SEI_QUE_LENGTH          8
#define H264DEC_SEI_DATA_SIZE           1024
#define H264DEC_SEI_DATA_NUMBER         8
#define H264DEC_SEI_DATA_SECTION_SIZE   128
#define H264DEC_USER_DATA_NONE          0
#define H264DEC_USER_DATA_ITU_T_T35     1
#define H264DEC_USER_DATA_UNREGISTERED  2
typedef enum
{
    NOT_SUPPORT_EVENT_NON		,
    NOT_SUPPORT_EVENT_FORMAT  	,
    NOT_SUPPORT_EVENT_RESOLUTION,
    NOT_SUPPORT_EVENT_FRAMERATE ,
	NOT_SUPPORT_EVENT_PROFILE	,
}AVC_NOT_SUPPORT_EVENT;
typedef enum
{
    PROCESS_NAL_DROP    =-2,
    PROCESS_NAL_FAIL    =-1,
    PROCESS_NAL_OK      = 0,
    PROCESS_NAL_EMPTY   = 1,
	PROCESS_NAL_NOT_SUPPORT = 2,
}PROCESS_NAL_STATUS;

typedef enum
{
    NAL_TYPE_UNSPECIFIED0   = 0x00,
    NAL_TYPE_NON_IDR        = 0x01,
    NAL_TYPE_DP_A           = 0x02,
    NAL_TYPE_DP_B           = 0x03,
    NAL_TYPE_DP_C           = 0x04,
    NAL_TYPE_IDR            = 0x05,
    NAL_TYPE_SEI            = 0x06,
    NAL_TYPE_SPS            = 0x07,
    NAL_TYPE_PPS            = 0x08,
    NAL_TYPE_AUD            = 0x09,
    NAL_TYPE_EO_SEQ         = 0x0a,
    NAL_TYPE_EO_STR         = 0x0b,
    NAL_TYPE_FILLER         = 0x0c,
    NAL_TYPE_SPS_EXT        = 0x0d,
    NAL_TYPE_PREFIX         = 0x0e,
    NAL_TYPE_SUB_SPS        = 0x0f,
    NAL_TYPE_RESERVED10     = 0x10,
    NAL_TYPE_RESERVED11     = 0x11,
    NAL_TYPE_RESERVED12     = 0x12,
    NAL_TYPE_AUX_SLCIE      = 0x13,
    NAL_TYPE_SLICE_EXT      = 0x14,
    NAL_TYPE_RESERVED21     = 0x15,
    NAL_TYPE_RESERVED22     = 0x16,
    NAL_TYPE_RESERVED23     = 0x17,
    NAL_TYPE_UNSPECIFIED24  = 0x18,
    NAL_TYPE_UNSPECIFIED25  = 0x19,
    NAL_TYPE_UNSPECIFIED26  = 0x1A,
    NAL_TYPE_UNSPECIFIED27  = 0x1B,
    NAL_TYPE_UNSPECIFIED28  = 0x1C,
    NAL_TYPE_UNSPECIFIED29  = 0x1D,
    NAL_TYPE_UNSPECIFIED30  = 0x1E,
    NAL_TYPE_UNSPECIFIED31  = 0x1F,
    NAL_TYPE_NOT_FOUND      = 0x20,
} H264DEC_NAL_TYPE;

typedef enum _en_H264DEC_SEI_TYPE_t
{
    SEI_BUFFERING_PERIOD = 0,                   //payloadType =  0
    SEI_PIC_TIMING,                             //payloadType =  1
    SEI_PAN_SCAN_RECT,                          //payloadType =  2
    SEI_FILLER_PAYLOAD,                         //payloadType =  3
    SEI_USER_DATA_REGISTERED_ITU_T_T35,         //payloadType =  4
    SEI_USER_DATA_UNREGISTERED,                 //payloadType =  5
    SEI_RECOVERY_POINT,                         //payloadType =  6
    SEI_DEC_REF_PIC_MARKING_REPETITION,         //payloadType =  7
    SEI_SPARE_PIC,                              //payloadType =  8
    SEI_SCENE_INFO,                             //payloadType =  9
    SEI_SUB_SEQ_INFO,                           //payloadType = 10
    SEI_SUB_SEQ_LAYER_CHARACTERISTICS,          //payloadType = 11
    SEI_SUB_SEQ_CHARACTERISTICS,                //payloadType = 12
    SEI_FULL_FRAME_FREEZE,                      //payloadType = 13
    SEI_FULL_FRAME_FREEZE_RELEASE,              //payloadType = 14
    SEI_FULL_FRAME_SNAPSHOT,                    //payloadType = 15
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START,   //payloadType = 16
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END,     //payloadType = 17
    SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET,     //payloadType = 18
    SEI_FILM_GRAIN_CHARACTERISTICS,             //payloadType = 19
    SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE,   //payloadType = 20
    SEI_STEREO_VIDEO_INFO,                      //payloadType = 21
    SEI_POST_FILTER_HINTS,                      //payloadType = 22
    SEI_TONE_MAPPING,                           //payloadType = 23
    SEI_SCALABILITY,                            //payloadType = 24
    SEI_SUB_PIC_SCALABLE_LAYER,                 //payloadType = 25
    SEI_NON_REQUIRED_LAYER,                     //payloadType = 26
    SEI_PRIORITY_LAYER,                         //payloadType = 27
    SEI_LAYERS_NOT_PRESENT,                     //payloadType = 28
    SEI_DEPENDENCY_CHANGE,                      //payloadType = 29
    SEI_SCALABLE_NESTING,                       //payloadType = 30
    SEI_BASE_LAYER_TEMPORAL_HRD,                //payloadType = 31
    SEI_QUALITY_LAYER_INTEGRITY_CHECK,          //payloadType = 32
    SEI_REDUNDANT_PIC_PROPERTY,                 //payloadType = 33
    SEI_TL0_DEP_REP_INDEX,                      //payloadType = 34
    SEI_TL_SWITCHING_POINT,                     //payloadType = 35
    SEI_PARALLEL_DECODING,                      //payloadType = 36
    SEI_MVC_SCALABLE_NESTING,                   //payloadType = 37
    SEI_VIEW_SCALABILITY,                       //payloadType = 38
    SEI_MULTIVIEW_SCENE,                        //payloadType = 39
    SEI_MULTIVIEW_ACQUISITION,                  //payloadType = 40
    SEI_NON_REQUIRED_VIEW_COMPONENT,            //payloadType = 41
    SEI_VIEW_DEPENDENCY_CAHNGE,                 //payloadType = 42
    SEI_OPERATION_POINTS_NOT_PRESENT,           //payloadType = 43
    SEI_BASE_VIEW_TEMPORAL_HRD,                 //payloadType = 44
    SEI_FRAME_PACKING_ARRANGEMENT,              //payloadType = 45
    SEI_RESERVED_SEI_MESSAGE,                   //none

    SEI_MAX_ELEMENTS  //!< number of maximum syntax elements
} H264DEC_SEI_TYPE_t;


typedef enum _process_nal_status
{
    DECODED_FAIL            =0x80000000,
    DECODED_GAPS_NOT_ALLOW  =0x80000080,
    DECODED_DROP_NOT_I      =0x80000040,
    DECODED_UNKNOW_FAIL     =0x80000020,
    DECODED_CMDQUE_FAIL     =0x80000010,
    DECODED_SEI_FAIL        =0x80000008,
    DECODED_PPS_FAIL        =0x80000004,
    DECODED_SPS_FAIL        =0x80000002,
    DECODED_SLICE_FAIL      =0x80000001,
    DECODED_OK              =0x00,
    DECODED_SLICE_OK        =0x01,
    DECODED_SPS_OK          =0x02,
    DECODED_PPS_OK          =0x04,
    DECODED_SEI_OK          =0x08,
}ProcessNalStatus;

typedef enum{
    P_SLICE = 0,
    B_SLICE,
    I_SLICE,
    SP_SLICE,
    SI_SLICE
}H264Dec_eSliceType;

typedef enum
{
    FRAME = 0,
    TOP_FIELD,
    BTM_FIELD
}H264Dec_ePicStructure;

typedef struct _ST_H264DEC_NAL
{
    UINT8   nal_unit_type;
    UINT8   nal_ref_idc;
    UINT8   u8Id;               //for directly mapping to dec data struct.  SPS[id] or PPS[id]
    UINT8   u8NotSupportType;
    UINT32  u32SeiDataSize;     //for contain USER data or ....etc, total contained sei side in
    UINT32  u32FirstMbInSlice;
    UINT32  u32NalStartOffset;
    UINT32  u32NalEndOffset;

    /* check duplicate frame, add by cwtsao */
    UINT8   prev_nal_ref_idc;
}ST_H264DEC_NAL, *PST_H264DEC_NAL;

#if !NOT_PARSING_H264_SEI
typedef struct _ST_H264DEC_SEI_ENTRY
{
    UINT32  type;
    UINT32  size;
}ST_H264DEC_SEI_ENTRY;

typedef struct _ST_H264PS_SEI
{
    ST_H264DEC_NAL          nalUnit;
    UINT32                  u32SeiNum;
    ST_H264DEC_SEI_ENTRY    stEntry[H264DEC_SEI_DATA_NUMBER];
    UINT8                   u8SeiData[H264DEC_SEI_DATA_SIZE];
}ST_H264DEC_SEI, *PST_H264DEC_SEI;
#endif
///////////////////////////////////////////////////////

typedef struct _sT_H264DEC_HRD
{

    UINT8   initial_cpb_removal_delay_length_minus1;
    UINT8   cpb_removal_delay_length_minus1;
    UINT8   dpb_output_delay_length_minus1;
}ST_H264DEC_HRD;
typedef struct _ST_H264DEC_SeqParSet
{
    ST_H264DEC_NAL  nalUnit;
    BOOL    bValid;
    BOOL    bUpdated;

    UINT8   seq_parameter_set_id;

    UINT8   profile_idc;
    UINT8   u8DecFrmBufSize;

    UINT32  chroma_format_idc;
    BOOL    separate_colour_plane_flag;
    UINT32  bit_depth_luma_minus8;
    UINT32  bit_depth_chroma_minus8;
    BOOL    seq_scaling_matrix_present_flag;
    BOOL    seq_scaling_list_present_flag[8];

    BOOL    bUseDefaultScalingMatrix4x4Flag[6];
    BOOL    bUseDefaultScalingMatrix8x8Flag[2];
    UINT8   u8ScalingList4x4[6][16];
    UINT8   u8ScalingList8x8[2][64];

    UINT32  log2_max_frame_num_minus4;
    UINT8   pic_order_cnt_type;
    UINT8   log2_max_pic_order_cnt_lsb_minus4;
    BOOL    delta_pic_order_always_zero_flag;
    INT32   offset_for_non_ref_pic;
    INT32   offset_for_top_to_bottom_field;
    UINT32  num_ref_frames_in_pic_order_cnt_cycle;
    INT32   offset_for_ref_frame[256];
    UINT8   max_num_ref_frames;
    BOOL    gaps_in_frame_num_value_allowed_flag;
    UINT32  pic_width_in_mbs_minus1;
    UINT32  pic_height_in_map_units_minus1;
    BOOL    frame_mbs_only_flag;
    BOOL    mb_adaptive_frame_field_flag;
    BOOL    direct_8x8_inference_flag;
    BOOL    frame_cropping_flag;
    UINT16  frame_crop_left_offset;
    UINT16  frame_crop_right_offset;
    UINT16  frame_crop_top_offset;
    UINT16  frame_crop_bottom_offset;

    // VUI //
    BOOL    vui_parameters_present_flag;
    BOOL    video_signal_type_present_flag;
    BOOL    video_full_range_flag;
    BOOL    nal_hrd_parameters_present_flag;
    ST_H264DEC_HRD stNal_Hrd;
    BOOL    vcl_hrd_parameters_present_flag;
    ST_H264DEC_HRD stVcl_Hrd;
    BOOL    pic_struct_present_flag;
}ST_H264DEC_SeqParSet, *PST_H264DEC_SeqParSet;

typedef struct _ST_H264DEC_PicParSet
{
    ST_H264DEC_NAL nalUnit;
    BOOL   	bValid;

    UINT32  pic_parameter_set_id;
    UINT32  seq_parameter_set_id;
    BOOL    entropy_coding_mode_flag;
    BOOL    bottom_field_pic_order_in_frame_present_flag;
    UINT32  num_slice_groups_minus1;
    UINT32  slice_group_map_type;
//  UINT32  run_length_minus1[ num_slice_groups_minus1 ]
//  UINT32  top_left[ num_slice_groups_minus1 ]
//  UINT32  bottom_right[ num_slice_groups_minus1 ]
    BOOL    slice_group_change_direction_flag;
    UINT32  slice_group_change_rate_minus1;
    UINT32  pic_size_in_map_units_minus1;
//  UINT32  slice_group_id[ pic_size_in_map_units_minus1 ]

    UINT8   num_ref_idx_l0_default_active_minus1;
    UINT8   num_ref_idx_l1_default_active_minus1;
    BOOL    weighted_pred_flag;
    UINT8   weighted_bipred_idc;
    INT32   pic_init_qp_minus26;
    INT32   pic_init_qs_minus26;
    INT32   chroma_qp_index_offset;
    BOOL    deblocking_filter_control_present_flag;
    BOOL    constrained_intra_pred_flag;
    BOOL    redundant_pic_cnt_present_flag;
    BOOL    transform_8x8_mode_flag;

    BOOL    pic_scaling_matrix_present_flag;
    BOOL    pic_scaling_list_present_flag[8];
    INT32   second_chroma_qp_index_offset;

    BOOL    bUseDefaultScalingMatrix4x4Flag[6];
    BOOL    bUseDefaultScalingMatrix8x8Flag[2];
    UINT8   u8ScalingList4x4[6][16];
    UINT8   u8ScalingList8x8[2][64];
}ST_H264DEC_PicParSet, *PST_H264DEC_PicParSet;

typedef struct _ST_H264DEC_RefPicMarking
{
    UINT8   memory_management_control_operation;
    INT32   difference_of_pic_nums_minus1;
    INT32   long_term_pic_num;
    UINT32  long_term_frame_idx;
    UINT32  max_long_term_frame_idx_plus1;
}ST_H264DEC_RefPicMarking, *PST_H264DEC_RefPicMarking;

typedef struct _ST_H264DEC_SLICE
{
    ST_H264DEC_NAL nalUnit;
    UINT32  u32CurSeqParSetId;
    UINT32  u32PreSeqParSetId;
    UINT32  u32PrePicParSetId;
    UINT32  u32PreFrmNum;
    UINT32  u32PreviousFrmNum;
#if !NOT_PARSING_H264_SEI
    INT32   s32PicStructure;
#endif
    BOOL    bFirstFieldFlag;
    BOOL    bIdrFlag;

    UINT32  first_mb_in_slice;
    H264Dec_eSliceType slice_type;
    UINT32  pic_parameter_set_id;
    UINT32  frame_num;

    H264Dec_ePicStructure ePicStruct;
    BOOL    field_pic_flag;
    BOOL    bottom_field_flag;
    UINT32  idr_pic_id;
    INT32   pic_order_cnt_lsb;
    INT32   delta_pic_order_cnt_bottom;
    INT32   delta_pic_order_cnt[2];

    BOOL    num_ref_idx_active_override_flag;
    UINT32  num_ref_idx_l0_active_minus1;
    UINT32  u32NumRefIdxL0Active;
    UINT32  num_ref_idx_l1_active_minus1;
    UINT32  u32NumRefIdxL1Active;


    // H264DecRefPicListReorder //
    BOOL    ref_pic_list_reordering_flag_l0;
    UINT32  reordering_of_pic_nums_idc_l0[32];
    UINT32  abs_diff_pic_num_minus1_l0[32];
    UINT32  long_term_pic_num_l0[32];

    BOOL    ref_pic_list_reordering_flag_l1;
    UINT32  reordering_of_pic_nums_idc_l1[32];
    UINT32  abs_diff_pic_num_minus1_l1[32];
    UINT32  long_term_pic_num_l1[32];

    // RefPicMarking //
    UINT8   u8RefPicMarkingIdx;
    ST_H264DEC_RefPicMarking stRefPicMarking[64];
    BOOL    no_output_of_prior_pics_flag;
    BOOL    long_term_reference_flag;
    BOOL    adaptive_ref_pic_marking_mode_flag;
}ST_H264DEC_SLICE, *PST_H264DEC_SLICE;

typedef struct _ST_H264PS_DRV_CTX
{
    ST_H264DEC_NAL          stCurNalUnit;
	PST_H264DEC_SeqParSet   pstSPS; // +1 allocate for parsing tmp used
    PST_H264DEC_SeqParSet   pstCurSPS;
    PST_H264DEC_SeqParSet   pstActiveSPS;
    PST_H264DEC_SeqParSet   pstPreActiveSPS;
	PST_H264DEC_PicParSet   pstPPS; // +1 allocate for parsing tmp used
    PST_H264DEC_PicParSet   pstCurPPS;
    PST_H264DEC_PicParSet   pstPreActivePPS;

#if !NOT_PARSING_H264_SEI
    INT32                   s32PicStructure;
#endif

    ST_H264DEC_SLICE        stSlice;
#if !NOT_PARSING_H264_SEI
    ST_H264DEC_SEI          stSei;
#endif
    rbspstream              stRbspStream;
    bstream                 *pstBitStream;
	BOOL                    bFindHeader;


	/*----------------------------------*/
	/* add by HK */
	BOOL		no_more_slice_flag; //TODO
	UINT32 		slice_hdr_len; //ERROR BITSTREAM
	UINT32 		u32RbspStart; //ERROR BITSTREAM

	/*-------add by CW ----------*/
	void *parent;
	int	chip_idx;
} ST_H264PS_DRV_CTX, *PST_H264PS_DRV_CTX;

typedef struct _va_buf_cfg
{
	INT32     idxofva;
	INT32     blk;
	INT32     idofblk;
	INT32     flag_extra;
	UINT32    frmsize;
	UINT32    allocsize;
	UINT32    witdh;
	UINT32    height;
	uintptr_t   y_addr;
	uintptr_t   uv_addr;
	UINT32    	ystride;
	UINT32    	uvstride;
	UINT32   	priv_used;
} va_buf_cfg;

PROCESS_NAL_STATUS H264PS_SLICE(PST_H264PS_DRV_CTX pstCtx);
PROCESS_NAL_STATUS H264PS_SPS(PST_H264PS_DRV_CTX pstCtx);
PROCESS_NAL_STATUS H264PS_PPS(PST_H264PS_DRV_CTX pstCtx);
#if !NOT_PARSING_H264_SEI
PROCESS_NAL_STATUS H264PS_SEI(PST_H264PS_DRV_CTX pstCtx);
#endif
PROCESS_NAL_STATUS H264PS_PREFIX(PST_H264PS_DRV_CTX pstCtx);
PROCESS_NAL_STATUS H264PS_SUB_SPS(PST_H264PS_DRV_CTX pstCtx);
PROCESS_NAL_STATUS H264PS_SLICE_EXT(PST_H264PS_DRV_CTX pstCtx);
int H264PS_DRV_ProcessNal(PST_H264PS_DRV_CTX pstCtx, void **phdrq_data, unsigned int *phdrq_size);
#endif //_H264PS_VLD_H_
