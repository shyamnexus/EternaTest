/**
 * @file kdrv_videoenc.h
 * @brief type definition of KDRV API.
 * @author ALG2
 * @date in the year 2018
 */

#ifndef __KDRV_VIDEOENC_H__
#define __KDRV_VIDEOENC_H__

#include "kwrap/type.h"
#include "kdrv_type.h"

typedef enum {
	KDRV_VDOENC_ID_1,                			        ///< video encoder ID 1
	KDRV_VDOENC_ID_2,                			        ///< video encoder ID 2
	KDRV_VDOENC_ID_3,                			        ///< video encoder ID 3
	KDRV_VDOENC_ID_4,                			        ///< video encoder ID 4
	KDRV_VDOENC_ID_5,                			        ///< video encoder ID 5
	KDRV_VDOENC_ID_6,                			        ///< video encoder ID 6
	KDRV_VDOENC_ID_7,                			        ///< video encoder ID 7
	KDRV_VDOENC_ID_8,                			        ///< video encoder ID 8
	KDRV_VDOENC_ID_9,                			        ///< video encoder ID 9
	KDRV_VDOENC_ID_10,               			        ///< video encoder ID 10
	KDRV_VDOENC_ID_11,               			        ///< video encoder ID 11
	KDRV_VDOENC_ID_12,               			        ///< video encoder ID 12
	KDRV_VDOENC_ID_13,               			        ///< video encoder ID 13
	KDRV_VDOENC_ID_14,               			        ///< video encoder ID 14
	KDRV_VDOENC_ID_15,               			        ///< video encoder ID 15
	KDRV_VDOENC_ID_16,               			        ///< video encoder ID 16
	KDRV_VDOENC_ID_17,               			        ///< video encoder ID 17
	KDRV_VDOENC_ID_18,               			        ///< video encoder ID 18
	KDRV_VDOENC_ID_19,               			        ///< video encoder ID 19
	KDRV_VDOENC_ID_20,               			        ///< video encoder ID 20
	KDRV_VDOENC_ID_21,               			        ///< video encoder ID 21
	KDRV_VDOENC_ID_22,               			        ///< video encoder ID 22
	KDRV_VDOENC_ID_23,               			        ///< video encoder ID 23
	KDRV_VDOENC_ID_24,               			        ///< video encoder ID 24
	KDRV_VDOENC_ID_25,               			        ///< video encoder ID 25
	KDRV_VDOENC_ID_26,               			        ///< video encoder ID 26
	KDRV_VDOENC_ID_27,               			        ///< video encoder ID 27
	KDRV_VDOENC_ID_28,               			        ///< video encoder ID 28
	KDRV_VDOENC_ID_29,               			        ///< video encoder ID 29
	KDRV_VDOENC_ID_30,               			        ///< video encoder ID 30
	KDRV_VDOENC_ID_31,               			        ///< video encoder ID 31
	KDRV_VDOENC_ID_32,               			        ///< video encoder ID 32
	KDRV_VDOENC_ID_MAX,              			        ///< video encoder ID maximum
	ENUM_DUMMY4WORD(KDRV_VDOENC_ID)
} KDRV_VDOENC_ID;

typedef enum {
	VDOENC_TYPE_JPEG = 0,
	VDOENC_TYPE_H264,
	VDOENC_TYPE_H265,
} KDRV_VDOENC_TYPE;

typedef enum {
    KDRV_VDOENC_OK = 0,
    KDRV_VDOENC_FAIL = 0xffff
} KDRV_VDOENC_STATUS;

typedef enum {
    KDRV_VODENC_I_SLICE = 0,
    KDRV_VODENC_P_SLICE,
    KDRV_VODENC_KP_SLICE,
} KDRV_VDOENC_SLICE_TYPE;


/*********** video encode ************/
typedef enum {
	KDRV_VDOENC_DAR_DEFAULT,         		///< default video display aspect ratio (the same as encoded image)
	KDRV_VDOENC_DAR_16_9,            		///< 16:9 video display aspect ratio
	KDRV_VDOENC_DAR_MAX,             		///< maximum video display aspect ratio
	ENUM_DUMMY4WORD(KDRV_VDOENC_DAR)
} KDRV_H26XENC_DAR;

typedef enum {
	KDRV_VDOENC_SVC_DISABLE = 0,       		///< disable SVC
	KDRV_VDOENC_SVC_2X,          			///< enable  SVC_2X (SVC layer 1)
	KDRV_VDOENC_SVC_4X,          			///< enable  SVC_2X (SVC layer 2)
	KDRV_VDOENC_SVC_MAX,             		///< SVC setting maximum
	ENUM_DUMMY4WORD(KDRV_VDOENC_SVC)
} KDRV_VDOENC_SVC;

typedef enum {
	KDRV_VDOENC_PROFILE_BASELINE,
	KDRV_VDOENC_PROFILE_MAIN,
	KDRV_VDOENC_PROFILE_HIGH,
	KDRV_VDOENC_PROFILE_MAX,
} KDRV_H26XENC_PROFILE;

typedef enum {
	KDRV_VDOENC_CAVLC = 0,
	KDRV_VDOENC_CABAC,
	ENUM_DUMMY4WORD(KDRV_VDOENC_ENTROPY)
}KDRV_VDOENC_ENTROPY;

/************ rate control ************/
typedef enum {
	VDOENC_RC_MODE_CBR = 1,
	VDOENC_RC_MODE_VBR = 2,
	VDOENC_RC_MODE_FIX_QP = 3,
	VDOENC_RC_MODE_EVBR = 4,
	VDOENC_RC_MODE_CVBR = 5,
} KDRV_VDOENC_RC_MODE;

/************ quality level ************/
typedef enum {
	KDRV_VDOENC_QUALITY_MAIN = 0,	///< set tile config and search range as original policy
	KDRV_VDOENC_QUALITY_BASE		///< set tile config and search range as saving memory
} KDRV_VDOENC_QLVL;

typedef struct {
	UINT32 enable;				///< [r/w] enable CBR. 0: disable, 1: enable, defulat: 0
	UINT32 static_time;			///< [r/w] Rate control's static time.      default:  0, range: 0~20
	UINT32 byte_rate;			///< [r/w] Byte rate (byte rate per second)
	UINT32 frame_rate;			///< [r/w] Frame rate
	UINT32 gop;					///< [r/w] Gop Size
	UINT32 init_i_qp;			///< [r/w] Rate control's init I qp.        default: 26, range: 0~51
	UINT32 min_i_qp;			///< [r/w] Rate control's min I qp.         default:  1, range: 0~51
	UINT32 max_i_qp;			///< [r/w] Rate control's max I qp.         default: 51, range: 0~51
	UINT32 init_p_qp;			///< [r/w] Rate control's init P qp.        default: 26, range: 0~51
	UINT32 min_p_qp;			///< [r/w] Rate control's min P qp.         default:  1, range: 0~51
	UINT32 max_p_qp;			///< [r/w] Rate control's max P qp.         default: 51, range: 0~51
	INT32  ip_weight;			///< [r/w] Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32 key_p_period;                    ///< [r/w] Key P frame interval.            default: 0, range: 0~4096
	INT32  kp_weight;                       ///< [r/w] Rate control's KP/P frame weight.default: 0, range: -100~100
	INT32  p2_weight;                       ///< [r/w] Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32  p3_weight;                       ///< [r/w] Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32  lt_weight;                       ///< [r/w] Rate control's long term P/P frame weight. default: 0, range: -100~100
	INT32  motion_aq_str;                   ///< [r/w] Motion aq strength for smart ROI. default: 0, range: -15~15
	UINT32 max_frame_size;                  ///< [r/w] max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
	UINT32 svc_weight_mode;		///< [r/w] SVC weight mode: 0: weight = qp (origin SVC RC), 1: weight = bit size
	UINT32 br_tolerance;		///< [r/w] bitrate tolerance. default 2: range: 0~6
	uintptr_t user_data_addr;	///< [r/w] address of RC user data
	UINT32 user_data_size;		///< [r/w] size of RC user data
} KDRV_VDOENC_CBR;

typedef struct {
	UINT32 enable;				///< [r/w] enable VBR. 0: disable, 1: enable, defulat: 0
	UINT32 static_time;			///< [r/w] Rate control's static time.      default:  0, range: 0~20
	UINT32 byte_rate;			///< [r/w] Byte rate (byte rate per second)
	UINT32 frame_rate;			///< [r/w] Frame rate
	UINT32 gop;					///< [r/w] Gop Size
	UINT32 init_i_qp;			///< [r/w] Rate control's init I qp.        default: 26, range: 0~51
	UINT32 min_i_qp;			///< [r/w] Rate control's min I qp.         default:  1, range: 0~51
	UINT32 max_i_qp;			///< [r/w] Rate control's max I qp.         default: 51, range: 0~51
	UINT32 init_p_qp;			///< [r/w] Rate control's init P qp.        default: 26, range: 0~51
	UINT32 min_p_qp;			///< [r/w] Rate control's min P qp.         default:  1, range: 0~51
	UINT32 max_p_qp;			///< [r/w] Rate control's max P qp.         default: 51, range: 0~51
	INT32  ip_weight;			///< [r/w] Rate control's I/P frame weight. default:  0, range: -100~100
	UINT32 change_pos;			///< [r/w] Early limit bitate.              default:  0, range: 0~100 (0: disable)
	UINT32 key_p_period;			///< [r/w] Key P frame interval.            default: 0, range: 0~4096
	INT32  kp_weight;			///< [r/w] Rate control's KP/P frame weight.default: 0, range: -100~100
	INT32  p2_weight;			///< [r/w] Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32  p3_weight;			///< [r/w] Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32  lt_weight;			///< [r/w] Rate control's long term P/P frame weight. default: 0, range: -100~100
	INT32  motion_aq_str;			///< [r/w] Motion aq strength for smart ROI. default: 0, range: -15~15
	UINT32 max_frame_size;			///< [r/w] max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
	UINT32 policy;				///< [r/w] vbr policy. default: 0, 0: old, 1: new
	UINT32 svc_weight_mode;		///< [r/w] SVC weight mode: 0: weight = qp (origin SVC RC), 1: weight = bit size
	UINT32 br_tolerance;		///< [r/w] bitrate tolerance. default 2: range: 0~6
	uintptr_t user_data_addr;	///< [r/w] address of RC user data
	UINT32 user_data_size;		///< [r/w] size of RC user data
} KDRV_VDOENC_VBR;

typedef struct {
	UINT32 enable;				///< [r/w] enable EVBR. 0: disable, 1: enable, defulat: 0
	UINT32 static_time;			///< [r/w] Rate control's static time.      default:  0, range: 0~20
	UINT32 byte_rate;			///< [r/w] Byte rate (byte rate per second)
	UINT32 frame_rate;			///< [r/w] Frame rate
	UINT32 gop;					///< [r/w] Gop Size
	UINT32 key_p_period;		///< [r/w] Key P frame interval.  default: frame rate*2, range: 0~4096
	UINT32 init_i_qp;			///< [r/w] Rate control's init I qp.        default: 26, range: 0~51
	UINT32 min_i_qp;			///< [r/w] Rate control's min I qp.         default:  1, range: 0~51
	UINT32 max_i_qp;			///< [r/w] Rate control's max I qp.         default: 51, range: 0~51
	UINT32 init_p_qp;			///< [r/w] Rate control's init P qp.        default: 26, range: 0~51
	UINT32 min_p_qp;			///< [r/w] Rate control's min P qp.         default:  1, range: 0~51
	UINT32 max_p_qp;			///< [r/w] Rate control's max P qp.         default: 51, range: 0~51
	INT32  ip_weight;			///< [r/w] Rate control's I/P frame weight. default:  0, range: -100~100
	INT32  kp_weight;		        ///< [r/w] Rate control's KP/P frame weight. default: 0, range: -100~100
	INT32  motion_aq_st;		        ///< [r/w] Motion aq strength for smart ROI. default: -6, range: -15~15
	UINT32 still_frm_cnd;		        ///< [r/w] Condition of still environment of EVBR. default: 100, range: 1~4096
	UINT32 motion_ratio_thd;	        ///< [r/w] Threshold of motion ratio to decide motion frame and still frame. default: 30, range: 1~100
	UINT32 i_psnr_cnd;			///< [r/w] Still mode qp of I frame.        default: 28, range: 0~51
	UINT32 p_psnr_cnd;			///< [r/w] Still mode qp of P frame.        default: 36, range: 0~51
	UINT32 kp_psnr_cnd;		///< [r/w] Still mode qp of key P frame.    default: 30, range: 0~51
	INT32  p2_weight;                       ///< [r/w] Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32  p3_weight;                       ///< [r/w] Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	INT32  lt_weight;                       ///< [r/w] Rate control's long term P/P frame weight. default: 0, range: -100~100
	UINT32 max_frame_size;                  ///< [r/w] max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
	UINT32 svc_weight_mode;		///< [r/w] SVC weight mode: 0: weight = qp (origin SVC RC), 1: weight = bit size
	UINT32 br_tolerance;		///< [r/w] bitrate tolerance. default 2: range: 0~6
	UINT32 evbr_init_state;    ///< Initial state of EVBR. 0: motion state, 1: still state, 2: keep previous state
	uintptr_t user_data_addr;	///< [r/w] address of RC user data
	UINT32 user_data_size;		///< [r/w] size of RC user data
} KDRV_VDOENC_EVBR;

typedef struct {
	UINT32 enable;				///< [r/w] enable CVBR. 0: disable, 1: enable, defulat: 0
	UINT32 static_time;			///< [r/w] Rate control's static time.      default:  0, range: 0~20
	UINT32 byte_rate;			///< [r/w] Byte rate (byte rate per second)
	UINT32 frame_rate;			///< [r/w] Frame rate
	UINT32 gop;					///< [r/w] Gop Size
	UINT32 key_p_period;		///< [r/w] Key P frame interval.            default: 0, range: 0~4096
	UINT32 init_i_qp;			///< [r/w] Rate control's init I qp.        default: 26, range: 0~51
	UINT32 min_i_qp;			///< [r/w] Rate control's min I qp.         default:  1, range: 0~51
	UINT32 max_i_qp;			///< [r/w] Rate control's max I qp.         default: 51, range: 0~51
	UINT32 init_p_qp;			///< [r/w] Rate control's init P qp.        default: 26, range: 0~51
	UINT32 min_p_qp;			///< [r/w] Rate control's min P qp.         default:  1, range: 0~51
	UINT32 max_p_qp;			///< [r/w] Rate control's max P qp.         default: 51, range: 0~51
	INT32  ip_weight;			///< [r/w] Rate control's I/P frame weight. default:  0, range: -100~100
	INT32  kp_weight;			///< [r/w] Rate control's KP/P frame weight.default: 0, range: -100~100
	INT32  lt_weight;			///< [r/w] Rate control's long term P/P frame weight. default: 0, range: -100~100
	INT32  p2_weight;			///< [r/w] Rate control's SVC P2/P frame weight. default: 0, range: -100~100
	INT32  p3_weight;			///< [r/w] Rate control's SVC P3/P frame weight. default: 0, range: -100~100
	UINT32 change_pos;			///< [r/w] Early limit bitate.              default:  0, range: 0~100 (0: disable)
	INT32  motion_aq_str;		///< [r/w] Motion aq strength for smart ROI. default: 0, range: -15~15
	UINT32 max_frame_size;		///< [r/w] max frame size. Default: 0, 0: limited by buffer size. Others: max size limited (byte)
	UINT32 br_tolerance;		///< [r/w] bitrate tolerance. default 2: range: 0~6
	UINT32 st_max_bitrate;		///< [r/w] maximal bitrate (short term)
	UINT32 lt_min_bitrate;		///< [r/w] minimal bitrate (long term)
	UINT32 lt_measure_period;	///< [r/w] long term measure period (unit minute)
} KDRV_VDOENC_CVBR;

typedef struct {
	UINT32 enable;				///< [r/w] enable fixQP. 0: disable, 1: enable, defulat: 0
	UINT32 fix_i_qp;            ///< [r/w] Fix qp of I frame. default: 26, range: 0~51
	UINT32 fix_p_qp;            ///< [r/w] Fix qp of P frame. default: 26, range: 0~51
	UINT32 frame_rate;			///< [r/w] Frame rate
} KDRV_VDOENC_FIXQP;

typedef struct {
	KDRV_VDOENC_RC_MODE     rc_mode;    ///< rate control mode. default: 1, range: 1~4 (1: CBR, 2: VBR, 3: FixQP, 4: EVBR)
	union {
		KDRV_VDOENC_CBR     cbr;        ///< parameter of rate control mode CBR
		KDRV_VDOENC_VBR     vbr;        ///< parameter of rate control mode VBR
		KDRV_VDOENC_FIXQP   fixqp;      ///< parameter of rate control mode FixQP
		KDRV_VDOENC_EVBR    evbr;       ///< parameter of rate control mode EVBR
		KDRV_VDOENC_CVBR    cvbr;       ///< parameter of rate control mode CVBR
	} rc_param;
}KDRV_VDOENC_RATE_CONTROL;

/************ row rc ************/
typedef struct {
	BOOL            enable;             ///< [r/w] enable row rc. default: 1, range: 0~1 (0: disable, 1: enable)
	UINT8           i_qp_range;         ///< [r/w] qp range of I frame for row-level rata control. default: 2, range: 0~15
	UINT8           i_qp_step;          ///< [r/w] qp step  of I frame for row-level rata control. default: 1, range: 0~15
	UINT8           p_qp_range;         ///< [r/w] qp range of P frame for row-level rata control. default: 4, range: 0~15
	UINT8           p_qp_step;          ///< [r/w] qp step  of P frame for row-level rata control. default: 1, range: 0~15
	UINT8           min_i_qp;           ///< [r/w] min qp of I frame for row-level rata control. default:  1, range: 0~51
	UINT8           max_i_qp;           ///< [r/w] max qp of I frame for row-level rata control. default: 51, range: 0~51
	UINT8           min_p_qp;           ///< [r/w] min qp of P frame for row-level rata control. default:  1, range: 0~51
	UINT8           max_p_qp;           ///< [r/w] max qp of P frame for row-level rata control. default: 51, range: 0~51
} KDRV_VDOENC_ROW_RC;

/************ encode set param ************/
/********* user-defined QP map *******/
typedef struct {
	BOOL            enable;             ///< [w] enable user qp. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8*          qp_map_addr;        ///< [w] buffer address of user qp map. two bytes per cu16
											///< bit[0:5] qp value (default: 0; if qp mode is 3 then qp value means fixed qp [range: 0~51], otherwise qp value means delta qp [range: -32~31])
	UINT32			qp_map_size;		///<[w] user qp map size
	UINT32			qp_map_loft;		///<[w] user qp map lineoffset
	UINT8			qp_map_mosaic_ctrl;	///<[w] user qp map mosaic ctrl;
}KDRV_VDOENC_USR_QP;

/************ aq ************/
#define KDRV_H26XENC_AQ_MAP_TABLE_NUM   30 ///< AP table num

typedef struct {
	BOOL            enable;             ///< [r/w] AQ enable. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8           i_str;              ///< [r/w] aq strength of I frame. default: 3, range: 1~8
	UINT8           p_str;              ///< [r/w] aq strength of P frame. default: 3, range: 1~8
	INT8            max_delta_qp;       ///< [r/w] max delta qp of aq. default: -6, range: 0 ~ 8
	INT8            min_delta_qp;       ///< [r/w] min delta qp of aq. default:  6, range:  -8 ~ 0
	BOOL            mode;                ///< [r/w] AQ mode. default: 0, 0: original, 1: dependent on cu size, HEVC only
	UINT8           i_str1;              ///< [r/w] aq strength of I frame of CU32. default: 3(same as i_str), range: 1~8, HEVC only
	UINT8           p_str1;              ///< [r/w] aq strength of P frame of CU32. default: 3(same as i_str), range: 1~8, HEVC only
	UINT8           i_str2;              ///< [r/w] aq strength of I frame of CU16. default: 1, range: 1~8, HEVC only
	UINT8           p_str2;              ///< [r/w] aq strength of P frame of CU16. default: 1, range: 1~8, HEVC only
	UINT8           depth;              ///< NVR only. AQ depth. default: 2, range(H.264): 2, range(H.265): 0~2 (0: cu64, 1: cu32, 2: cu16)
	INT16           thd_table[KDRV_H26XENC_AQ_MAP_TABLE_NUM];
} KDRV_VDOENC_AQ;

/************ roi ************/
typedef struct {
	BOOL            enable;             ///< [r/w] enable roi qp. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT16          coord_X;            ///< [r/w] coordinate x of roi. range: 0~encode width -1
	UINT16          coord_Y;            ///< [r/w] coordinate y of roi. range: 0~encode height-1
	UINT16          width;              ///< [r/w]  width of roi. range: 0~encode width -1
	UINT16          height;             ///< [r/w] height of roi. range: 0~encode height-1
	INT8            qp;                 ///< [r/w] qp value. default: 0; if qp mode is 3 then qp value means fixed qp (range: 0~51), otherwise qp value means delta qp (range: -32~31)
	UINT8			qp_mode;			///< [r/w] [520]qp mode , TODO
} KDRV_VDOENC_ROI_WIN;

typedef struct {
	//UINT8           roi_qp_mode;        ///< [r/w][520][RESERVED, remove to each window] roi qp mode.  default: 0, range: 0~1 (0: delta qp, 1: fixed qp)
	KDRV_VDOENC_ROI_WIN    st_roi[10]; ///< [r/w] roi window settings. ROIs can be overlaid, and the priority of the ROIs is based on index number, index 0 is highest priority and index 9 is lowest.
	UINT32 uiDeltaQp;	// [RESERVED] //
	UINT32 uiRoiCount;	// [RESERVED] //
} KDRV_VDOENC_ROI;

/************ multi slice ************/
typedef struct  {
#if 0
	BOOL	enable;            			///< [r/w] enable multiple slice. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32	slice_row_num;     			///< [r/w] number of macroblock/ctu rows occupied by a slice, range: 1 ~ number of macroblock/ctu row
#else
	BOOL	enable;            			///< [r/w] enable multiple slice. default: 0, range: 0~1 (0: disable, 1: enable)

	BOOL    slice_mode;     			///< [r/w] multi-slice mode. default: 0, range: 0~1 (0: same number of rows per slice, 1: user defined rows in slices)
	UINT32  slice_row_num[4];     		///< [r/w] number of macroblock/ctu rows occupied by a slice; number of rows in slice_0 ~ slice_3, range: 1 ~ number of macroblock/ctu row
	INT32  slice_i_idx;     			///< [r/w] I slice index in multi slice. default: -1(disable), range: -1 ~ macroblock/ctu rows*tile index -1
#endif
} KDRV_VDOENC_SLICE_SPLIT;

/************ gdr ************/
typedef struct {
	BOOL            enable;             ///< [r/w] enable gdr. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          period;             ///< [r/w] intra refresh period. default: 0, range: 0~0xFFFFFFFF (0: always refresh, others: intra refresh frame period)
	UINT32          number;             ///< [r/w] intra refresh row number. default: 1, range: 1 ~ number of macroblock/ctu row
	BOOL            enable_gdr_i_frm;   ///< [r/w] enable gdr i-frame scheme. default: 0, range: 0~1 (0: disable, 1: enable)
	BOOL            enable_gdr_qp;      ///< [r/w] enable gdr scheme. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT32          gdr_qp;             ///< [r/w] intra gdr qp, range: 0~51
} KDRV_VDOENC_GDR;

/************ source decompression  ************/
typedef struct {
	BOOL enable;						///< [r/w] y and cbcr decompression enable.
	UINT32 width;
	UINT32 height;
	UINT32 y_lofst;
	UINT32 c_lofst;
} KDRV_VDOENC_SDC;

/************ osg ************/
typedef struct {
	BOOL overlap_en;
} KDRV_VDOENC_OSG_GLOBAL;

typedef struct {
	UINT8  rgb2yuv[3][3];
} KDRV_VDOENC_OSG_RGB;

typedef struct {
	UINT8 idx;

	UINT8 alpha;
	UINT8 red;
	UINT8 green;
	UINT8 blue;
} KDRV_VDOENC_OSG_PAL;

typedef struct {
	UINT8  type;
	UINT16 width;
	UINT16 height;
	UINT16 line_offset;
	uintptr_t addr;
	uintptr_t addr_pa;
} KDRV_VDOENC_OSG_GRAP;

typedef struct {
	UINT8  mode;
	UINT16 str_x;
	UINT16 str_y;
	UINT8  bg_alpha;
	UINT8  fg_alpha;
	UINT8  mask_type;
	UINT8  mask_bd_size;
	UINT8  mask_y[2];
	UINT8  mask_cb;
	UINT8  mask_cr;
} KDRV_VDOENC_OSG_DISP;

typedef struct {
	BOOL enable;

	UINT8 blk_width;
	UINT8 blk_height;
	UINT8 blk_num;
	UINT8 org_color_level;
	UINT8 inv_color_level;
	UINT8 nor_diff_th;
	UINT8 inv_diff_th;
	UINT8 sta_only_mode;
	UINT8 full_eval_mode;
	UINT8 eval_lum_targ;
} KDRV_VDOENC_OSG_GCAC;

typedef struct {
	UINT8 lpm_mode;
	UINT8 tnr_mode;
	UINT8 fro_mode;
	UINT8 qp_mode;
	INT8  qp;
} KDRV_VDOENC_OSG_QPMAP;

typedef struct {
	BOOL  enable;
	BOOL  alpha_en;
	UINT8 alpha;
	UINT8 red;
	UINT8 green;
	UINT8 blue;
} KDRV_VDOENC_OSG_COLOR_KEY;

typedef struct {
	UINT8 blk_sz;
} KDRV_VDOENC_OSG_MOSAIC;

typedef struct {
	UINT8 layer_idx;
	UINT8 win_idx;
	UINT8 ddr_id;

	BOOL enable;

	KDRV_VDOENC_OSG_GRAP st_grap;
	KDRV_VDOENC_OSG_DISP st_disp;
	KDRV_VDOENC_OSG_GCAC st_gcac;
	KDRV_VDOENC_OSG_QPMAP st_qp_map;
	KDRV_VDOENC_OSG_COLOR_KEY st_key;
	KDRV_VDOENC_OSG_MOSAIC st_mosaic;
} KDRV_VDOENC_OSG_WIN;

/************ mask ************/
#define MAX_MASK_PAL_NUM	16
typedef struct {
	UINT8 mosaic_blk_w;
	UINT8 mosaic_blk_h;
	UINT8 pal_y[MAX_MASK_PAL_NUM];
	UINT8 pal_cb[MAX_MASK_PAL_NUM];
	UINT8 pal_cr[MAX_MASK_PAL_NUM];
} KDRV_VDOENC_MASK_INIT;

typedef struct {
	UINT8 mosaic_blk_w;       ///< [r/w] mosaic block width. range: 0 ~ 255
	UINT8 mosaic_blk_h;       ///< [r/w] mosaic block height. range: 0 ~ 255
} KDRV_VDOENC_MASK_MOSAIC;

typedef struct {
    uint8_t pal_y[8];           ///< [r/w] palette colors y. range: 0 ~ 255
    uint8_t pal_cb[8];          ///< [r/w] palette colors cb. range: 0 ~ 255
    uint8_t pal_cr[8];          ///< [r/w] palette colors cr. range: 0 ~ 255
} KDRV_VDOENC_MASK_PALETTE;

typedef struct {
	UINT32 pos_x;             ///< [r/w] mask window postion x. range: 0 ~ encode width - 1
	UINT32 pos_y;             ///< [r/w] mask window postion y. range: 0 ~ encode height - 1
} KDRV_VDOENC_MASK_POS;

typedef struct {
	UINT8 mask_idx;           ///< [r/w] index of mask. default: 0, range: 0~7
	UINT8 enable;             ///< [r/w] enable mask window. default: 0, range: 0~1 (0: disable, 1: enable)
	UINT8 mosaic_en;          ///< [r/w] mask window de-identified method. range: 0~1 (0: orignial, 1: mosaic)
	UINT8 line_hit_opt;       ///< [r/w] mask window line hit operation. 0: and(inside), 1: or(outside), 2: border, 3: single line or concave border, 4: full concave mask
	UINT32 alpha;             ///< [r/w] mask window blending alpha. range : 0 ~ 256
	UINT8 alpha_type;         ///< [r/w] mosaic: 0: alpha blending with source and mosaic, 1: alpha blending with source after mosiac and mask color
	KDRV_VDOENC_MASK_POS st_pos[4]; ///< [r/w] mask window position configure settings.
	UINT8 pal_sel;
	UINT8 hit_width[4];
	UINT8 clockwise;
	UINT8 line_chk_th;
} KDRV_VDOENC_MASK_WIN;
/************ end of mask ************/

/************ color to gray ************/
typedef struct {
	BOOL enable;                ///< [r/w] color to gray enable
	BOOL color_to_gray;         ///< [r/w] final image color to gray
	BOOL src_color_to_gray;     ///< [r/w] source image colot to gray
} KDRV_VDOENC_GRAY;
/************ end if color to gray ************/


typedef struct {
	uintptr_t motion_buf_addr[3];
	UINT32 motion_buf_loft;
	UINT8  motion_buf_num;
} KDRV_VDOENC_MOT_ADDR;

typedef struct {
	UINT8  nr_3d_mode;						///< [r/w] codec tnr enable/disable, range: 0 ~ 1 (0: disable, 1: enable)
	UINT8  tnr_osd_mode;					/// RESERVED
	UINT8  mctf_p2p_pixel_blending;			///< [r/w] codec tnr blending enable, range: 0 ~ 1 (0: disable, 1: enable)
	UINT8  tnr_p2p_sad_mode;				///< [r/w] codec tnr p2p sad mode, range: 0 ~ 3 (0: pixel, 1: 8x8, 2:16x16, 3:auto)
	UINT8  tnr_mctf_sad_mode;				///< [r/w] codec tnr mctf sad mode, range: 0 ~ 3 (0: pixel, 1: 8x8, 2:16x16, 3:auto)
	UINT8  tnr_mctf_bias_mode;				///< [r/w] codec tnr mctf bias mode, range: 0 ~ 3 (0: pixel, 1: 8x8, 2:16x16, 3:auto)

	UINT8  nr_3d_adp_th_p2p[3];				///< [r/w] codec tnr p2p sad threshold, range: 0 ~ 255
	UINT8  nr_3d_adp_weight_p2p[3];			///< [r/w] codec tnr p2p weight, range: 0 ~ 16
	UINT8  tnr_p2p_border_check_th;			///< [r/w] codec tnr p2p border check threshold, range: 0 ~ 255
	UINT8  tnr_p2p_border_check_sc;			///< [r/w] codec tnr p2p border check scale, range: 0 ~ 7
	UINT8  tnr_p2p_input;					///< [r/w] codec tnr p2p input, range: 0 ~ 1 (0: original source , 1: mctf result)
	UINT8  tnr_p2p_input_weight;			///< [r/w] codec tnr p2p input weight, range: 0 ~ 3
	UINT8  cur_p2p_mctf_motion_th;			///< [r/w] codec tnr p2p mctf current motion threshold, range: 0 ~ 255
	UINT8  ref_p2p_mctf_motion_th;			///< [r/w] codec tnr p2p mctf reference motion threshold, range: 0 ~ 255
	UINT8  tnr_p2p_mctf_motion_wt[4];		///< [r/w] codec tnr p2p mctf motion weight, range: 0 ~ 3

	UINT8  nr3d_temporal_spatial_y[3];		///< [r/w] codec tnr p2p reference luma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_spatial_c[3];		///< [r/w] codec tnr p2p reference chroma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_range_y[3];		///< [r/w] codec tnr p2p luma range, range: 0 ~ 255
	UINT8  nr3d_temporal_range_c[3];		///< [r/w] codec tnr p2p chroma range, range: 0 ~ 255
	UINT8  nr3d_clampy_th;					///< [r/w] codec tnr p2p clamp luma threshold, range: 0 ~ 255
	UINT8  nr3d_clampy_div;					///< [r/w] codec tnr p2p clamp luma lsr, range: 0 ~ 7
	UINT8  nr3d_clampc_th;					///< [r/w] codec tnr p2p clamp chroma threshold, range: 0 ~ 255
	UINT8  nr3d_clampc_div;					///< [r/w] codec tnr p2p clamp chroma lsr, range: 0 ~ 7

	UINT8  nr3d_temporal_spatial_y_mctf[3];	///< [r/w] codec tnr mctf reference luma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_spatial_c_mctf[3];	///< [r/w] codec tnr mctf reference chroma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_range_y_mctf[3];	///< [r/w] codec tnr mctf luma range, range: 0 ~ 255
	UINT8  nr3d_temporal_range_c_mctf[3];	///< [r/w] codec tnr mctf chroma range, range: 0 ~ 255
	UINT8  nr3d_clampy_th_mctf;				///< [r/w] codec tnr mctf clamp luma threshold, range: 0 ~ 255
	UINT8  nr3d_clampy_div_mctf;			///< [r/w] codec tnr mctf clamp luma lsr, range: 0 ~ 7
	UINT8  nr3d_clampc_th_mctf;				///< [r/w] codec tnr mctf clamp chroma threshold, range: 0 ~ 255
	UINT8  nr3d_clampc_div_mctf;			///< [r/w] codec tnr mctf clamp chroma lsr, range: 0 ~ 7

	UINT8  cur_motion_rat_th;				///< [r/w] codec tnr ratio threshold for current motion, range: 0 ~ 15
	UINT8  cur_motion_sad_th;				///< [r/w] codec tnr sad threshold for current motion, range: 0 ~ 255
	UINT8  ref_motion_twr_p2p_th[2];		///< [r/w] codec tnr refresh threshold current motion for  p2p's temporal weight, range: 0 ~ 255
	UINT8  cur_motion_twr_p2p_th[2];		///< [r/w] codec tnr refresh threshold reference motion for  p2p's temporal weight, range: 0 ~ 255
	UINT8  ref_motion_twr_mctf_th[2];		///< [r/w] codec tnr refresh threshold current motion for  mctf's temporal weight, range: 0 ~ 255
	UINT8  cur_motion_twr_mctf_th[2];		///< [r/w] codec tnr refresh threshold reference motion for  mctf's temporal weight, range: 0 ~ 255
	UINT8  nr3d_temporal_spatial_y_1[3];	///< [r/w] codec tnr p2p reference luma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_spatial_c_1[3];	///< [r/w] codec tnr p2p reference chroma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_spatial_y_mctf_1[3];	///< [r/w] codec tnr mctf reference luma spatial, range: 0 ~ 7
	UINT8  nr3d_temporal_spatial_c_mctf_1[3];	///< [r/w] codec tnr mctf reference chroma spatial, range: 0 ~ 7

	UINT8  sad_twr_p2p_th [2];				///< [r/w] codec tnr the refresh threshold of p2p sad for  p2p's temporal weight, range: 0 ~ 255
	UINT8  sad_twr_mctf_th[2];				///< [r/w] codec tnr the refresh threshold of mctf sad for mctf's temporal weight, range: 0 ~ 255
} KDRV_VDOENC_3DNR;

typedef struct {
	void (*vdoenc_3dnr_cb)(UINT32 path_id, ULONG config);
	UINT32 id;
} KDRV_VDOENC_3DNRCB;

/************ rdo ************/
typedef enum {
	VDOENC_RDO_CODEC_264 = 0,
	VDOENC_RDO_CODEC_265 = 1
} KDRV_VDOENC_RDO_CODEC;

typedef struct {
	UINT8 avc_intra_4x4_cost_bias;			// [r/w] default: 8, range: 0~31, //INTRA 4x4 block number decrease as the value increase
	UINT8 avc_intra_8x8_cost_bias;			// [r/w] default: 8, range: 0~31, //INTRA 8x8 block number decrease as the value increase
	UINT8 avc_intra_16x16_cost_bias;		// [r/w] default: 8, range: 0~31, //INTRA 16x16 block number decrease as the value increase
	UINT8 avc_inter_tu4_cost_bias;			// [r/w] default: 8, range: 0~31, //INTER TU4 block number decrease as the value increase
	UINT8 avc_inter_tu8_cost_bias;			// [r/w] default: 8, range: 0~31, //INTER TU8 block number decrease as the value increase
	UINT8 avc_inter_skip_cost_bias;			// [r/w] default: 8, range: 0~31, //Skip mode number decrease as the value increase
} KDRV_VDOENC_RDO_264;

typedef struct {
	UINT8 hevc_intra_32x32_cost_bias;		// [r/w] default: 0, range: 0~15, //INTRA 32x32 block number decrease as the value increase
	UINT8 hevc_intra_16x16_cost_bias;		// [r/w] default: 0, range: 0~15, //INTRA 16x16 block number decrease as the value increase
	UINT8 hevc_intra_8x8_cost_bias;			// [r/w] default: 0, range: 0~15, //INTRA 8x8 block number decrease as the value increase
	INT8  hevc_inter_skip_cost_bias;		// [r/w] default: 0, range: -16~15, //skip mode number decrease as the value increase
	INT8  hevc_inter_merge_cost_bias;		// [r/w] default: 0, range: -16~15, //merge mode number decrease as the value increase
	UINT8 hevc_inter_64x64_cost_bias;		// [r/w] default: 14, range: 0~31, //INTER 64x64 block number decrease as the value increase
	UINT8 hevc_inter_64x32_32x64_cost_bias;		// [r/w] default: 28, range: 0~31, //INTER 64x32 and 32x64 block number decrease as the value increase
	UINT8 hevc_inter_32x32_cost_bias;		// [r/w] default: 14, range: 0~31, //INTER 32x32 block number decrease as the value increase
	UINT8 hevc_inter_32x16_16x32_cost_bias;		// [r/w] default: 28, range: 0~31, //INTER 32x16 and 16x32 block number decrease as the value increase
	UINT8 hevc_inter_16x16_cost_bias;		// [r/w] default: 7, range: 0~31, //INTER 16x16 block number decrease as the value increase
} KDRV_VDOENC_RDO_265;

typedef struct {
	KDRV_VDOENC_RDO_CODEC		rdo_codec;	///< rdo codec. 0: h264, 1: h265
	union {
		KDRV_VDOENC_RDO_264	rdo_264;        ///< parameter of h264 rdo
		KDRV_VDOENC_RDO_265	rdo_265;        ///< parameter of h265 rdo
	} rdo_param;
}KDRV_VDOENC_RDO;

typedef struct {
	BOOL enable;
	UINT8 avc_bg_skip_bias;		// H264 skip bias of bg. default 8, range 0~31
	INT8 hevc_bg_skip_bias;		// H265 skip bias of bg. default 0, range -16~15
	INT8 hevc_bg_merge_bias;	// H265 merge bias of bg. default 0, range -16~15
	UINT8 bg_bias_shift;		// shift bit of bg bias. default 0, range 0~3
	UINT8 mode;					// default 0, range 0~1
}KDRV_VDOENC_BG_RDO;

/************ jnd ************/
typedef struct {
	UINT8		enable;				///< [r/w] jnd enable. default: 0, range: 0~3 (0: disable, 1: luma and chroma both enable, 2: luma enable, 3: chroma enable)
	UINT8		str;				///< [r/w] jnd filter strength. default: 7, range: 0~15
	UINT8		level;				///< [r/w] jnd edge level. default: 11, range: 0~15
	UINT8		threshold;			///< [r/w] jnd CM edge detection threshold. default: 5, range: 0~255
	UINT8		c_str;				///< [r/w] jnd chroma filter strength. default: 7 (same as luma str), range: 0~15
	UINT8		t_str;				///< [r/w] jnd temporal filter strength, default 3, range: 0~15 (0: disable)
} KDRV_VDOENC_JND;

/************ Smart roi info ************/
#define KDRV_VDOENC_SMART_ROI_MAX_NUM 	32
#define KDRV_VDOENC_SMART_ROI_CLASS_NUM	5
typedef struct {
	UINT32 x;
	UINT32 y;
} KDRV_VDOENC_POSITION;

typedef struct {
	KDRV_VDOENC_POSITION position[2];   ///< top left and bottom right position of bounding box
	UINT32 class_id;                    ///< bounding box class
} KDRV_VDOENC_BBOX;

typedef struct {
	UINT32 base_width;
	UINT32 base_height;                                     ///< base resolution
	UINT8 bbox_num;                                        	///< number of bounding boxes, range: KDRV_VDOENC_SMART_ROI_MAX_NUM
	KDRV_VDOENC_BBOX bbox[KDRV_VDOENC_SMART_ROI_MAX_NUM];   ///< bounding boxes
	UINT64 timestamp;                                       ///< timestamp of bounding boxes
} KDRV_VDOENC_SMART_BBOX;

typedef struct {
	BOOL enable;                                            ///< enable AI smart encode, range: 0~2
	UINT8 fg_str[KDRV_VDOENC_SMART_ROI_CLASS_NUM];          ///< foreground strength of each classes, range: 0~255
	UINT8 mode;                                             ///< 0: adjust qp only, 1: adjust qp + BGRDO
} KDRV_VDOENC_SMART_ROI;

/************ dynamic frame rate ************/
typedef struct {
	BOOL 	enable;					///< enable dynamic frame rate, range: 0~1,     default: 0
	UINT32 	min_fps;				///< the minimal frame rate,    range: 1~fps-1, default: fps / 3
	UINT32 	motion_sensitivity;		///< the bias for fps refresh,  range: 0~15,    default: 15
} KDRV_VDOENC_DYNAMIC_FR;

/************ dynamic gop ************/
typedef struct {
	BOOL      enable;				///< enable dynamic frame rate,      range: 0~1,        default: 0
	UINT32    max_gop;				///< the maximal gop of dynamic gop, range: gop ~ 4096, default: gop * 5 ~ gop * 10
	UINT32    motion_sensitivity;	///< the bias for fps refresh,       range: 0~15,       default: 8
} KDRV_VDOENC_DYNAMIC_GOP;

typedef struct {
	uintptr_t addr;
	UINT32 size;
} KDRV_VDOENC_LL_MEM;
/************ end encode set param ************/

/************ encode get param ************/
/************ get memory info ********/
typedef struct {
	uintptr_t addr;
	UINT32 size;
} KDRV_VDOENC_H264_TILE_MEM;
/************ end encode set param ************/

/************ encode get param ************/
/************ get memory info ********/
typedef struct {
	UINT32 width;							///< [w] encode frame width
	UINT32 height;							///< [w] encode frame height
	UINT32 svc_layer;						///< [w] encode svc layer
	UINT32 ltr_interval;					///< [w] encode long-term interval
	BOOL   tile_mode_en;                    ///< [w] encode tile mode enable
	BOOL   d2d_mode_en;                     ///< [w] encode d2d mode enable
	BOOL   gdc_mode_en;                     ///< [w] encode gdc mode enable
	BOOL   d2d_1tile_en;                    ///< [w] encode d2d 1tile enable
	UINT32 size;							///< [r]     return encode memory require size
	BOOL   colmv_en;                        ///< [w] encode colocated mv enable
	BOOL   comm_recfrm_en;                  ///< [w] encode reconstruct frame from common pool
	UINT32 recfrm_size;                     ///< [w] encode  common reconstruct frame size
	UINT32 recfrm_num;                      ///< [w] encode  common reconstruct frame number
	KDRV_VDOENC_QLVL quality_level;         ///< [w] encode quality level for setting search range and tile config
	uintptr_t ltr_bs_buffer;			///< [w] encode long-term internal bs buffer size
	KDRV_VDOENC_TYPE codec_type;			///< [w] encode codec_type , only support h264/h265, not support jpeg
	BOOL   gdr_i_frm_en;                    ///< [w] encode gdr i-frame scheme
} KDRV_VDOENC_MEM_INFO;

/************ get sps/pps ********/
typedef struct {
	uintptr_t addr;
	UINT32 size;
}KDRV_VDOENC_DESC;

typedef struct {
    UINT8    ratio_base;
    UINT8    edge_ratio;     ///< [r] edge str = origin edge str * edge ratio / ratio_base
    UINT8    dn_2d_ratio;    ///< [r] 2dnr str = origin 2dnr str * 2dnr str / ratio_base
    UINT8    dn_3d_ratio;    ///< [r] 3dnr str = origin 3dbr str * 3dnr str / ratio_base
} KDRV_VDOENC_ISP_RATIO;

typedef struct {
    void (*vdoenc_isp_cb)(UINT32 path_id, ULONG config);
    UINT32   id;
} KDRV_VDOENC_ISPCB;

typedef struct {
	UINT32 size;
} KDRV_VDOENC_LL_MEM_INFO;

typedef struct {
	UINT16 align_w;						///< [r/w] source out yuv align width
	UINT16 align_h;						///< [r/w] source out yuv align height
} KDRV_VDOENC_SRCOUTYUV_WH;

typedef struct {
	UINT32 max_width;
	UINT32 max_height;
	UINT32 size;
} KDRV_VDOENC_H264_TILE_BUF_INFO;

typedef struct {
	UINT32 width;		// input width
	UINT32 height;		// input height
	UINT32 max_i_qp;	// input max_i_qp
	//UINT32 max_p_qp;	// input max_p_qp
	UINT32 min_size;	// return min buffer size
} KDRV_VDOENC_H26X_MIN_BS_BUF_INFO;

typedef struct {
    UINT32 max_bitrate;		// input max bitrate
    UINT32 enc_buf_ms;		// input enc_buf_ms
    UINT32 bitrate;			// input bitrate
    UINT32 gop_num;			// input gop
    UINT32 frame_rate_base;	// input frame rate
    UINT32 frame_rate_incr;	// input frame rate
    UINT32 min_i_ratio;		// input/output
    UINT32 min_p_ratio;		// input/output
} KDRV_VDOENC_MIN_BS_RATIO_INFO;

/************ end encode get param ************/




/************ RESERVED  *****************/
/************ source decompress *****************/
typedef struct {
        UINT32 sideinfo_addr;                                   ///< [w] source decompress sideinfo addr
        UINT32 ktable_0;                                                ///< [w] source decompress ktable0
        UINT32 ktable_1;                                                ///< [w] source decompress ktable1
        UINT32 ktable_2;                                                ///< [w] source decompress ktable2
        UINT32 stripe_num;                                              ///< [w] source decompress stripe number
        UINT32 stripe_01_size;                                  ///< [w] source decompress stripe01 size, [12:0] stripe0 size, [28:16] stripe1 size
        UINT32 stripe_23_size;                                  ///< [w] source decomppres stripe23 size, [12:0] stripe2 size, [28:16] stripe3 size
} KDRV_H26XENC_SRC_COMPRESS_INFO;

/************ MD bitmap info ************/
typedef struct {
    UINT32 md_width;
    UINT32 md_height;
    UINT32 md_lofs;
    uintptr_t md_buf_adr;
    UINT32 rotation;
    UINT32 roi_xy;
    UINT32 roi_wh;
} KDRV_VDOENC_MD_INFO;

/************ end RESERVED  *****************/

/*********** encode init ***********/
typedef struct {
	BOOL  bEnable;			///< [r/w] Enable sharpen
	UINT8 ucConEng; 		///< [r/w] The weight of the local contrast  0~8
	UINT16 usSlopConEng; 	///< [r/w] Transition region slop 0~4095
	UINT8 ucBHC;			///< [r/w] Bright halo clip ratio 0~128
	UINT8 ucDHC;			///< [r/w] Dark halo clip ratio 0~128
	UINT8 ucEWT;			///< [r/w] Edge weight coring threshold 0~255
	UINT8 ucEWG;			///< [r/w] Edge weight gain 0~255
	UINT8 ucEdgeSharpStr1;	///< [r/w] Sharpen strength 1 of edge region 0~255
	UINT8 ucCT; 			///< [r/w] Coring threshold 0~255
	UINT8 ucNL; 			///< [r/w] Noise level 0~255
	UINT8 ucBIG;			///< [r/w] Blending ratio of HPF results 0~255
	UINT16 usFlatTh;		///< [r/w] flat region threshold 0~2047
	UINT16 usEdgeTh;		///< [r/w] Edge region threshold 0~2047
	UINT8 ucEdgeStr;		///< [r/w] Edge weight strength 0~255
	UINT8 ucTransitionStr;	///< [r/w] Transition region weight strength 0~255
	UINT8 ucMotionStr;		///< [r/w] Motion region edge weight 0~255
	UINT8 ucStaticStr;		///< [r/w] Static region edge weight strength 0~255
	UINT8 ucFlatStr;		///< [r/w] Flag region weight strength 0~255
	UINT8 ucNC[17]; 		///< [r/w] 17 control points of noise modulation curve 0~255
	UINT16 usEWG[9]; 		///< [r/w] 9 control points of EWG
	BOOL bShowSharpInfo;    ///< [r/w] Show debug mode info(0: normal, 1: regionclass)
} KDRV_H26XENC_SPN;

typedef struct {
	void (*vdoenc_spn_cb)(UINT32 path_id, ULONG config);
	UINT32 id;
} KDRV_VDOENC_SPNCB;

typedef struct {
	BOOL enable;
	INT8 str;
	UINT8 start_idx;
	UINT8 end_idx;
} KDRV_VDOENC_MAQDIFF;

typedef struct {
	BOOL enable;
	UINT8 mode;			///< [r/w] lpm mode, 0: disable LPM mode(default), 1: PQ mode(recommend), 2: power saving mode
} KDRV_VDOENC_LPM;

#define KDRV_VDOENC_WATERMARK_COUNT 8
typedef struct {
	BOOL enable[KDRV_VDOENC_WATERMARK_COUNT];
	UINT8 uv[KDRV_VDOENC_WATERMARK_COUNT];
	UINT8 str[KDRV_VDOENC_WATERMARK_COUNT];
	UINT32 left_top[KDRV_VDOENC_WATERMARK_COUNT];
	UINT8 len[KDRV_VDOENC_WATERMARK_COUNT];

	uintptr_t addr[KDRV_VDOENC_WATERMARK_COUNT];
} KDRV_VDOENC_WATERMARK;

typedef struct {
	uintptr_t addr;
} KDRV_VDOENC_WATERMARK_ADDR;

typedef struct {
	BOOL   bMotDetectEn;			///< [r/w] motion detect enable, range: 0 ~ 1
	INT8   cMotDetectDQP;			///< [r/w] cu16 delatQP, range: -16 ~ 15, suggest: 0
	INT8   cMotDetectUsrOffset;		///< [r/w] user offset, range: -16 ~ 15, suggest: 0
	UINT16 usMotDetectBoundaryThr;	///< [r/w] outlier threshold, range: 0 ~ 1000, suggest: 15

	BOOL   bMotFilterEn;			///< [r/w] motion filter enable, range: 0 ~ 1,
	UINT8  ucMotFilterThr;			///< [r/w] motion filter threshold, range: 0 ~ 7, suggest: 1
	INT8   cMotFilterDQP;			///< [r/w] motion filter DQP, range: -16 ~ 15, suggest: 0
} KDRV_VDOENC_MOTION_DETECT;

typedef struct {
	UINT32 uiChannelId;
	UINT32 uiSliceIdx;
	BOOL   bIsIFrame;
	UINT32 uiBsOnlySize;
	UINT32 uiTotalSize;
} KDRV_H26XENC_SLICE_LOW_LATENCY_RESULT;

typedef struct {
	BOOL bEnable;

	void (*vdoenc_slice_low_latency_cb)(void *info);
} KDRV_VDOENC_SLICE_LOW_LATENCY_CB;

typedef struct {
	BOOL enable;
	uintptr_t data_addr;
	UINT32 data_length;
} KDRV_VDOENC_USER_DATA;

typedef struct {
	UINT32 ISP_ISO;
	UINT32 ISP_RESERVED[7];
} KDRV_H26XENC_ISP;

typedef struct {
	void (*vdoenc_isp_param_cb)(UINT32 path_id, ULONG config);
	UINT32 id;
} KDRV_VDOENC_ISP_PARAM_CB;

typedef struct {
	BOOL enable;
	UINT32 machine_id[4];
} KDRV_VDOENC_SEI_SIGNED_VDO;

typedef struct {
	uintptr_t buf_addr;						///< [w] encode buffer addr
	UINT32 buf_size;                        ///< [w] encode buffer size
	UINT32 width;                           ///< [w] frame uiWidth
	UINT32 height;                          ///< [w] frame uiHeight
	UINT32 byte_rate;                       ///< [w] byte rate
	UINT32 frame_rate;                      ///< [w] frame rate
	UINT32 gop;								///< [w] GOP
	UINT32 init_i_qp;                       ///< [w] I Qp initial value
	UINT32 min_i_qp;                        ///< [w] I Qp min value
	UINT32 max_i_qp;                        ///< [w] I Qp max value
	UINT32 init_p_qp;                       ///< [w] P Qp initial value
	UINT32 min_p_qp;                        ///< [w] P Qp min value
	UINT32 max_p_qp;                        ///< [w] P Qp max value
	UINT32 user_qp_en; 						///< [w] use user Qp (0:default, 1:enable)
	UINT32 static_time;       				///< [w] rc static time (unit: sec)
	INT32  ip_weight;						///< [w] rc I/P weight
	KDRV_H26XENC_DAR e_dar;                 ///< [w] video display aspect ratio
	KDRV_VDOENC_SVC e_svc;                 ///< [w] SVC Layer
	UINT32 ltr_interval;      				///< [w] long-term reference frame interval (range: 0 ~ 4095)
	UINT32 ltr_pre_ref;                     ///< [w] long-term reference setting (0:all long-term reference to IDR frame, 1:reference latest long-term reference frame)
	UINT32 rotate;           	            ///< [w] rotate (0:disable, 1:CCW, 2:CW)
	UINT32 jpeg_yuv_format;                 ///< [w] YUV format for jpeg
	UINT32 fast_search; 					///< [w] fast Search (0:small range, 1:big range)
	KDRV_H26XENC_PROFILE e_profile;			///< [w] profile
	UINT32 multi_layer;						///< [w] multi temporary layer
	UINT32 color_range; 					///< [w] color range (0:tv range (15-235), 1:full range (0-255))
	UINT32 project_mode;					///< [w] prj mode for brc (0:IPCam, 1:DVCam)
	UINT32 sei_idf_en;						///< [w] add SEI ID data (0: disable, 1:enable)
	KDRV_VDOENC_ENTROPY e_entropy;			///< entropy mode (0 : cavlc (H264 only), 1 : cabac)
	UINT8  level_idc;						///< level idc
	UINT32 sar_width;						///< Sample Aspect Ratio width, ex: 1920 x 1080, width = 1920
	UINT32 sar_height;						///< Sample Aspect Ratio height, ex: 1920 x 1080, height = 1080 (not 1088)
	BOOL   gray_en;							///< encode gray enable (0 : disable , 1 : enable)
	UINT8  disable_db;						///< disable_filter_idc (1'b0=Filter, 1'b1=No Filter, 2'b10 = Slice Mode, 3'b100 = Tile Mode(h265 only))
	INT8   db_alpha; 		  				///< deblocking Alpha & C0 offset div. 2, {-6, -5, ... 0, +1, .. +6}
	INT8   db_beta;			  				///< deblocking Beta offset div. 2, {-6, -5, ... 0, +1, .. +6}
	BOOL   bVUIEn;							///< encode VUI header(0:disable, 1 : enable)
	UINT8  matrix_coef;						///< matrix_coef of VUI header
	UINT8  transfer_characteristics;		///< transfer_characteristics of VUI header
	UINT8  colour_primaries;				///< colour_primaries of VUI header
	UINT8  video_format;					///< video_format of VUI header
	BOOL   time_present_flag;				///< time_present_flag of VUI header, if frame_rate = 0, default disable time_present_flag
	// 680
	UINT32 uiRecCompressEn;					///< [NOT USE, PLEASE USE bFBCEn]H26x reconstruct compression (ECLS)
	// [520] //
	BOOL bFBCEn;							///< reconstruct compression control (0:disalbe, 1:enable)
	BOOL bTileEn;                           ///< tile mode control (0:disable, 1:enable)
	BOOL bD2dEn;                            ///< d2d mode control (0:disable, 1:enable)
	BOOL gdc_mode_en;                       ///< d2d mode for gdc control (0:disable, 1:enable)
	BOOL bD2d1TileEn;                       ///< d2d 1tile control (0:disable, 1:enable)
	INT8 chrm_qp_idx;
	INT8 sec_chrm_qp_idx;
	BOOL hw_padding_en;
	BOOL sao_en;                          ///< Sample Adaptive Offset (0: enable, 1: disable)
	BOOL sao_luma_flag;                   ///< sao luma flag
	BOOL sao_chroma_flag;                 ///< sao chroma flag
	BOOL bGDRIFrm;                        ///< gdr I frame scheme flag
	// 520 fastboot builtin
	BOOL builtin_init;                    ///< fastboot builtin_init flag
	BOOL colmv_en;                        ///< colocated mv (0:disable, 1:enable)
	// 560 UVC
	BOOL comm_recfrm_en;                  ///< common reconstruct frame (0:disable, 1:enable)
	uintptr_t recfrm_addr[3];                ///< common reconstruct frame address
	UINT32 recfrm_size;                   ///< common reconstruct frame size
	UINT32 recfrm_num;                    ///< common reconstruct frame number
	KDRV_VDOENC_QLVL quality_level;		  ///< quality level for setting search range and tile config
	uintptr_t ltr_bs_buffer;				  ///< bs buffer for ltr
	UINT32 max_path_num;				  ///< video encode max path number
	BOOL b_src_mb_mode;				  ///< 32x8 mb mode, rotation: 1 or 2 => mb_mode: 0 or 1; rotation: 0 => mb_mode: 0
	BOOL bNonTileI;							///< H264 only for I frame encode non-tile . (0 : original, I frame tile encode, 1 : updated, I frame non-tile )

	// NVR //
	KDRV_VDOENC_TYPE codec_type;
	BOOL low_delay_mode;				/// < low delay mode : not support
	BOOL gdpr_svc_en;
} KDRV_VDOENC_INIT;

/************ encode one frame info *******************/
typedef struct {
	uintptr_t y_addr;									///< [w] encoding y address
	uintptr_t c_addr;                                  	///< [w] encoding cb address
	UINT32 y_line_offset;                               ///< [w] Y line offset
	UINT32 c_line_offset;                              	///< [w] UV line offset
	uintptr_t bs_start_addr;                            ///< [w] output bit-stream start address if enc space not enough
	uintptr_t bs_addr_1;                                ///< [w] 1st output bit-stream address
	UINT32 bs_size_1;                                   ///< [w] 1st output bit-stream size
	uintptr_t bs_addr_2;                                   ///< [w] 2nd output bit-stream address
	UINT32 bs_size_2;                                   ///< [w] 2nd output bit-stream size
	uintptr_t bs_end_addr;                                 ///< [w] output bit-stream end address
	UINT32 src_out_en;                              	///< [w] enable/disable src output
	uintptr_t src_out_y_addr;                            	///< [w] src output Y address
	uintptr_t src_out_c_addr;                              ///< [w] src output UV address
	UINT32 src_out_y_line_offset;                       ///< [w] src output Y line offset
	UINT32 src_out_c_line_offset;                       ///< [w] src output UV line offset
	UINT32 bs_shift;                                   	///< [w] I frame need VPS(not 4-byte align), BS should shift
	// 520 source compression different with 510/680 , do not use src_cmps_en and src_cmps_info, use src_cmps_y_en and src_cmps_c_en//
	UINT32 src_cmps_en;                            		///< [==== RESERVED ===== ] [w] src compression
	KDRV_H26XENC_SRC_COMPRESS_INFO src_cmps_info;   	///< [==== RESERVED ===== ][w] src compression info
	UINT32 quality;                                   	///< [w] initial quality value for jpeg (0~100)
	uintptr_t raw_y_addr;                                  ///< [r] RAW Yaddr
	UINT32 svc_hdr_size;                                ///< [==== RESERVED ===== ][r] SVC header size
	UINT32 temproal_id;                                	///< [r] SVC temporal layer ID (0, 1, 2)
	UINT32 frm_type; 									///< [==== RESERVED ===== ][r] (0:P, 1:B, 2:I, 3:IDR, 4:KP)
	UINT32 encode_err;									///< [r] output: re-encode or not
	UINT32 re_encode_en;								///< [r] output: re-encode or not
	UINT32 nxt_frm_type;								///< [r] suggest next frame type
	UINT32 base_qp;										///< [r] base qo
	UINT32 retstart_interval;							///< [w] JPEG restart interval. default: 0, range: 0~65536
	UINT32 encode_time;                                 ///<  [==== RESERVED ===== ]HW encode time(unit: us)
	UINT32 encode_width;								///< [w] JPEG encode width.
	UINT32 encode_height;								///< [w] JPEG encode height.
	UINT32 target_rate;                                 ///< [w] target rate (in bytes)
	UINT32 frame_rate;                                  ///< [w] frame rate
	UINT32 vbr_mode;                                    ///< [w] VBR mode setting
	UINT32 motion_ratio;                                ///< [w] enc motion ratio
	// KDRV_JPEGYUV_FORMAT in_fmt;						///< [w] JPEG input YUV format.0: YUV422,1:YUV420
	// for 520 //
	BOOL   src_cbcr_iv;
	BOOL   src_out_mode;
	uintptr_t nalu_len_addr;
	UINT64 time_stamp;
	BOOL   skip_frm_en;
	KDRV_VDOENC_SDC st_sdc;								/// < [w] source decompression parameter
	BOOL   src_d2d_en;									/// < [w] source d2d enable, 0: disable, 1 : enable
	UINT8  src_d2d_mode;								/// < [w] source d2d mode, (0: codec 1 tile ISP 1 strip, 1: codec 1 tile ISP 2 strip, (2~4 h265 only) 2: codec 2 tile ISP 1 strip, 3 : codec 2 tile ISP 2 strip, 4 : codec 3 tile ISP 3 strip)
	UINT32 src_d2d_strp_size[3];						/// < [w] source d2d stripe size
	BOOL   fmt_trans_en;                                /// < [w] JPEG format trans from 420 to 422
	UINT32 min_quality;                                 ///< [w] min quality value for jpeg (0~100)
	UINT32 max_quality;                                 ///< [w] max quality value for jpeg (0~100)
	UINT32 y_mse;										///< [r] y mean square error
	UINT32 u_mse;										///< [r] u mean square error
	UINT32 v_mse;										///< [v] v mean square error
	UINT32 nalu_num;                                    ///< [r] nalu_num
	uintptr_t nalu_size_addr;                           ///< [r] address for nalu_size(s)
	UINT32 bs_offset;							///< [r] jpeg encode output bitstream offset
//#if SUPPORT_JPEGENC_SLICE
	BOOL   enc_slice_en;
	UINT32 sec_Width;                           ///< [r] Raw Image Width, This value should be 16-multiplier.
	UINT32 sec_Height;                          ///< [r] Raw Image Height, This value should be 8 or 16 multiplier, depends on output bs format.
	UINT32 slice_cnt;							///< [r] slice index
//#endif
	UINT32 evbr_still_flag;						///< [r] output evbr state
	UINT32 job_id;
	UINT32 src_ddr_id;
	UINT32 bs_ddr_id;
	UINT32 engine_idx;
	uintptr_t y_addr_pa;									///< [w] encoding y physical address
	uintptr_t uv_addr_pa;									///< [w] encoding uv physical address
	uintptr_t bs_addr_pa;									///< [w] output bs physical address
	uintptr_t bs_addr_pa_2;									///< [w] 2nd output bs physical address
	UINT32 re_trigger;
	UINT32 first_bs_length;
	UINT32 sha_chksum[8];									///< [w] sha256 checksum
	UINT32 motion_sum_3dnr;
	UINT32 edge_sum_3dnr;
} KDRV_VDOENC_PARAM;

#if defined(_NVT_NVR_SDK_)
/************ tmnr info ************/
typedef struct {
    UINT8 luma_dn_en;             ///< Y channel TMNR on/off. default: 1, range: 0~1 (0: disable, 1: enable)
    UINT8 chroma_dn_en;           ///<  CbCr channel TMNR on/off. default: 1, range: 0~1 (0: disable, 1: enable)
    UINT8 tmnr_fcs_en;            ///<  TMNR False Color Supression enable. default:0, range: 0~1 (Note: chroma_dn_en shoulde be 1)
    UINT8 nr_str_y_3d;            ///<  Y channel temporal NR strength. default: 4, range: 0~32
    UINT8 nr_str_y_2d;            ///<  Y channel  spatial NR strength. default: 4, range: 0~32
    UINT8 nr_str_c_3d;            ///<  CbCr channel temporal NR strength. default: 4, range: 0~32
    UINT8 nr_str_c_2d;            ///<  CbCr channel  spatial NR strength. default: 4, range: 0~32
    UINT8 blur_str_y;             ///<  Difference Y image blurred strength. default: 1, range 0~2 (0: No blur, 1: low-strength blur, 2: high-strength blur)
    UINT8 center_wzero_y_2d_en;   ///<  Apply zero to center weight of Y  spatial Bilateral filter. default: 1, range 0~1
    UINT8 center_wzero_y_3d_en;   ///<  Apply zero to center weight of Y temporal Bilateral filter. default: 1, range 0~1
    UINT8 small_vibrat_supp_y_en; ///<  Y channel small vibration suppression ON/OFF. default: 0, range 0~1
    UINT8 avoid_residue_th_y;     ///< Y channel temporal absolute difference bigger than this threshold will not be suppressed to zero after NR. default: 1, range 1~4
    UINT8 avoid_residue_th_c;     ///<  CbCr channel temporal absolute difference bigger than this threshold will not be suppressed to zero after NR. default: 1, range 1~4
    UINT32 y_base[8];             ///<  Y channel noise model parameter: base noise level. default: {100,100,100,100,100,100,100,100}, range: 0~255*64
    UINT8 motion_level_th_y_k1;   ///<  Y channel motion level threshold 1 adjustment. default:  8, range: 0~32
    UINT8 motion_level_th_y_k2;   ///<  Y channel motion level threshold 2 adjustment. default: 16, range: 0~32 (Note: K2 > K1)
    UINT8 motion_level_th2_y_k1;  ///<  Y channel motion level threshold 1 adjustment. default:  8, range: 0~32
    UINT8 motion_level_th2_y_k2;  ///<  Y channel motion level threshold 2 adjustment. default: 16, range: 0~32 (Note: K2 > K1)
    UINT8 motion_level_th_c_k1;   ///<  CbCr channel motion level threshold 1 adjustment. default:  8, range: 0~32
    UINT8 motion_level_th_c_k2;   ///<  CbCr channel motion level threshold 2 adjustment. default: 16, range: 0~32 (Note: K2 > K1)
    UINT32 y_coefa[8];            ///<  Y channel noise model parameter: slope of line. default: {8,8,8,8,8,8,8,8}, range: 0~48
    UINT32 y_coefb[8];            ///<  Y channel noise model parameter: intercept of line. default: {100,100,100,80,70,50,50,50}, range: 0~255*64
    UINT32 y_std[8];              ///<  Y channel noise model parameter: noise standard deviation. default: {20,20,20,20,20,20,20,20}, range: 0~255*64
    UINT32 cb_mean[8];            ///<  Cb channel noise model parameter: noise mean. default: {100,100,100,100,100,100,100,100}, range: 0~255*25
    UINT32 cb_std[8];             ///<  Cb channel nosie model parameter: noise standard deviation. default: {20,20,20,20,20,20,20,20}, range 0~255*25
    UINT32 cr_mean[8];            ///<  Cr channel noise model parameter: noise mean. default: {100,100,100,100,100,100,100,100}, range: 0~255*25
    UINT32 cr_std[8];             ///<  Cr channel nosie model parameter: noise standard deviation. default: {20,20,20,20,20,20,20,20}, range 0~255*25
    UINT8 lut_y_3d_1_th[4];       ///<  LUT of Y channel 3D_1 filter. default: {11,33,55,77}, range: 0~127
    UINT8 lut_y_3d_2_th[4];       ///<  LUT of Y channel 3D_2 filter. default: {40,14, 7, 3}, range: 0~127
    UINT8 lut_y_2d_th[4];         ///<  LUT of Y channel 2D   filter. default: {11,33,55,77}, range: 0~127
    UINT8 lut_c_3d_th[4];         ///<  LUT of CbCr channel 3D filter. default: {37,19,11, 7}, range: 0~127
    UINT8 lut_c_2d_th[4];         ///<  LUT of CbCr channel 2D filter. default: {11,33,55,77}, range: 0~127
    UINT8 tmnr_fcs_str;           ///<  TMNR_False Color Supression strength. default: 4, range: 0~15
    UINT8 tmnr_fcs_th;            ///<  TMNR False Color Supression threshold. default: 32, range: 0~255

    UINT8 ref_cmpr_en;            ///< TMNR reference compression enable. range: 0~1
    UINT8 err_compensate;
    UINT8 dithering_enable;       ///<  TMNR source dithering enable. range: 0~1
    UINT8 dithering_bit_y;        ///<  TMNR source dithering y. range: 1~32
    UINT8 dithering_bit_cb;       ///<  TMNR source dithering cb. range: 1~32
    UINT8 dithering_bit_cr;       ///<  TMNR source dithering cr. range: 1~32
} KDRV_VODENC_TMNR_PARAM;

/************ reference buffer info ************/
typedef struct {
    UINT32 buf_id;
    UINT32 chip_id;
    uintptr_t addr_pa;
    UINT32 size;
    UINT32 ddr_no;
} KDRV_VDOENC_REF_BUF_INFO;

/************ enc_info buffer info ************/
typedef struct {
	uintptr_t addr_pa;
	UINT32 size;
} KDRV_VDOENC_INFO_BUF_INFO;

/************ tmnr buffer info ************/
typedef struct {
	UINT32 width;							///< [w] encode frame width
	UINT32 height;							///< [w] encode frame height
	BOOL   tmnr_ycc_enable;                 ///< [w] tmnr ycc enable
	UINT32 size;							///< [r]  return encode memory require size
} KDRV_VDOENC_TMNR_MEM_INFO;

typedef struct {
    KDRV_VDOENC_STATUS status;
    UINT32 job_id;

    KDRV_VDOENC_SLICE_TYPE slice_type;
    UINT32 bs_len;
    INT32 vps_offset;
    INT32 sps_offset;
    INT32 pps_offset;
    INT32 bs_offset;
    UINT32 svc_layer;

    UINT32 inter_mb_cnt;
    UINT32 intra16_mb_cnt;
    UINT32 intra8_mb_cnt;
    UINT32 intra4_mb_cnt;
    UINT32 skip_mb_cnt;

    UINT32 intra32_cu_num;
    UINT32 intra16_cu_num;
    UINT32 intra8_cu_num;
    UINT32 inter64_cu_num;
    UINT32 inter32_cu_num;
    UINT32 inter16_cu_num;
    UINT32 skip_cu_num;
    UINT32 merge_cu_num;

    UINT32 y_psne_mse;
    UINT32 u_psne_mse;
    UINT32 v_psne_mse;

    UINT32 frame_qp;
    UINT32 evbr_still_state;
    UINT32 motion_ratio;
}KDRV_VDOENC_RESULT;

/************ module info ************/
typedef struct {
	unsigned int max_width;
	unsigned int max_height;
	unsigned int max_chip;
	unsigned int max_engine;
	unsigned int max_chn_id;
	unsigned int max_act_chn;
} KDRV_VDOENC_MODULE_INFO;

typedef struct {
	INT32 (*callback)(VOID *callback_info, VOID *user_data);
	INT32 (*reserve_buf)(UINT32 phy_addr, int ddr_id);
	INT32 (*free_buf)(UINT32 phy_addr, int ddr_id);
} KDRV_VDOENC_CALLBACK_FUNC;

typedef struct {
    UINT32 frame_id;
    KDRV_VDOENC_PARAM param;
    KDRV_VDOENC_RESULT result;
    VOID *user_priv;
}KDRV_VDOENC_FRAME;
#endif

typedef enum {
	VDOENC_SET_CBR, 						///< set CBR info
	VDOENC_SET_EVBR, 						///< set EVBR info
	VDOENC_SET_VBR, 						///< set VBR info
	VDOENC_SET_CVBR, 						///< set CVBR info
	VDOENC_SET_FIXQP, 						///< set FIXQP info
	VDOENC_SET_ROWRC, 						///< set ROWRC info
	VDOENC_SET_QPMAP, 						///< set QP Map
	VDOENC_SET_AQ, 							///< set AQ info
	VDOENC_SET_3DNR,						///< set 3DNR info
	VDOENC_SET_3DNRCB, 						///< set 3DNR setting callback
	VDOENC_SET_ROI, 						///< set ROI info
	VDOENC_SET_SMART_ROI, 					///< set Smart ROI info (from OD)
	VDOENC_SET_SMART_BBOX,					///< set smart bounding box
	VDOENC_SET_MD, 							///< set MD info
	VDOENC_SET_SLICESPLIT,					///< set Slice Split info
	VDOENC_SET_GDR,							///< set intra refresh
	VDOENC_SET_RESET_IFRAME, 				///< set reset i frame
	VDOENC_SET_DUMPINFO, 					///< set dump info
	VDOENC_SET_RESET, 						///< set reset
	VDOENC_SET_COE,					    	///< set COE info
	VDOENC_SET_JPEG_FREQ,					///< set JPEG engine freq
	VDOENC_SET_CODEC,						///< set codec type
	VDOENC_SET_INIT,						///< set init encode info
	VDOENC_SET_CLOSE,						///< set video encode channel close
	VDOENC_SET_OSG_GLOBAL,					///< set osg global
	VDOENC_SET_OSG_RGB,						///< set osg rgb
	VDOENC_SET_OSG_PAL,						///< set osg pal
	VDOENC_SET_OSG_WIN,						///< set osg window
	VDOENC_SET_MOT_ADDR,					///< set motion buffer address
	VDOENC_SET_ISPCB,						///< set ISP callback function
	VDOENC_SET_RDO,							///< set rdo info
	VDOENC_SET_JND,							///< set jnd info
	VDOENC_SET_LL_MEM,						///< set link-list command buffer address
	VDOENC_SET_GOPNUM,						///< set gop number
	VDOENC_SET_LSC, 						///< set long start code
	VDOENC_SET_JPEG_OSG_RGB,
	VDOENC_SET_JPEG_OSG_PAL,
	VDOENC_SET_JPEG_OSG_WIN,
	VDOENC_SET_JPEG_ROTATION,
	VDOENC_SET_JPEG_MASK_INIT,
	VDOENC_SET_JPEG_MASK_WIN,
	VDOENC_SET_JPEG_GRAY,
	VDOENC_SET_JPEG_USER_DATA,
	VDOENC_SET_JPEG_STOP,
	VDOENC_SET_DMA_ABORT,                   ///< set dma abort
	VDOENC_SET_SPN,                         ///< set post sharpen
	VDOENC_SET_SPNCB,
	VDOENC_SET_MAQDIFF,
	VDOENC_SET_OSDMASKCB,
	VDOENC_SET_MASK_INIT,
	VDOENC_SET_MASK_WIN,
	VDOENC_SET_H264_TILE_BUF,
	VDOENC_SET_SEI_BS_CHKSUM,				///< set sei payload for bs checksum
	VDOENC_SET_LPM, 						///< set low power mode
	VDOENC_SET_USER_DATA,
	VDOENC_SET_BG_RDO,						///< set bg rdo
	VDOENC_SET_WATERMARK,					///< set watermark
	VDOENC_SET_MOTION_DETECT,				///< set motion detect
	VDOENC_SET_SLICE_LOW_LATENCY_CB,
	VDOENC_SET_DYNAMIC_FR,				///< set dynamic frame rate
	VDOENC_SET_DYNAMIC_GOP,						///< set dynamic gop
	VDOENC_SET_SCENE_DET,					///< set scene detection
	VDOENC_SET_SEI_SIGNED_VIDEO,
	VDOENC_SET_STOP,

	// NVR //
	VDOENC_SET_MASK_MOSAIC,
	VDOENC_SET_MASK_PAL,
	VDOENC_SET_TMNR_EN,
	VDOENC_SET_TMNR_PARAM,
	VDOENC_SET_CB_FUN,
	VDOENC_SET_REF_BUF,
	VDOENC_SET_RELEASE_REF_BUF,
	VDOENC_SET_INFO_BUF,
	ENUM_DUMMY4WORD(KDRV_VDOENC_SET_PARAM_ID)
} KDRV_VDOENC_SET_PARAM_ID;

typedef enum {
	VDOENC_GET_MEM_SIZE,					///< get codec needed size
	VDOENC_GET_DESC, 						///< get header description
	VDOENC_GET_ISIFRAME,					///< check whether is I frame
	VDOENC_GET_GOPNUM, 						///< get gop number
	VDOENC_GET_CBR, 						///< get CBR info
	VDOENC_GET_EVBR, 						///< get EVBR info
	VDOENC_GET_VBR, 						///< get VBR info
	VDOENC_GET_FIXQP, 						///< get FIXQP info
	VDOENC_GET_AQ, 							///< get AQ info
	VDOENC_GET_RC, 							///< get RC info
	VDOENC_GET_3DNR, 						///< get 3DNR info
	VDOENC_GET_GDR,							///< get GDR info
	VDOENC_GET_COE, 						///< get COE info
	VDOENC_GET_JPEG_FREQ, 					///< get JPEG engine frequency
	VDOENC_GET_BS_LEN,						///< get currenct encoded length
	VDOENC_GET_ISP_RATIO,					///< get ISP ratio
	VDOENC_GET_ROWRC,						///< get ROWRC info
	VDOENC_GET_RDO,							///< get rdo info
	VDOENC_GET_LL_MEM_SIZE,					///< get link-list command buffer memory size
	VDOENC_GET_SRCOUTYUV_WH,				///< get source out yuv align width/height
	VDOENC_GET_RECONFRM_SIZE,				///< get reconstruct frame size
	VDOENC_GET_RECONFRM_NUM,				///< get reconstruct frame number

	VDOENC_GET_JPEG_OSG_RGB,
	VDOENC_GET_JPEG_OSG_PAL,
	VDOENC_GET_JPEG_OSG_WIN,
	VDOENC_GET_JPEG_ROTATION,
	VDOENC_GET_JPEG_MASK_INIT,
	VDOENC_GET_JPEG_MASK_WIN,
	VDOENC_GET_H264_TILE_BUF_SIZE,
	VDOENC_GET_H26X_MIN_BS_BUF_SIZE,
	VDOENC_GET_H26X_MIN_BS_RATIO,
	VDOENC_GET_JND,
	VDOENC_GET_MOTION_DETECT,
	VDOENC_GET_SEI_SIGNED_VIDEO,

	// NVR //
	VDOENC_GET_TMNR_MEM_SIZE,				///< get tmnr needed size
	VDOENC_GET_ROI,
	VDOENC_GET_QPMAP,
	VDOENC_GET_SLICESPLIT,
	VDOENC_GET_OSG_RGB,
	VDOENC_GET_OSG_WIN,
	VDOENC_GET_MASK_MOSAIC,
	VDOENC_GET_MASK_PAL,
	VDOENC_GET_MASK_WIN,
	VDOENC_GET_REF_BUF,
	VDOENC_GET_TMNR_EN,
	VDOENC_GET_TMNR_PARAM,
	VDOENC_GET_MODULE_INFO,
	ENUM_DUMMY4WORD(KDRV_VDOENC_GET_PARAM_ID)
} KDRV_VDOENC_GET_PARAM_ID;
/*!
 * @fn INT32 kdrv_videoenc_open(UINT32 chip, UINT32 engine, UINT32 max_path_num)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @param max_path_num	the max encode path number
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_open(UINT32 chip, UINT32 engine, UINT32 max_path_num);

/*!
 * @fn INT32 kdrv_videoenc_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_close(UINT32 chip, UINT32 engine);

/*!
 * @fn INT32 kdrv_videoenc_trigger(KDRV_VDOENC_ID handler, KDRV_H26XENC_PARAM *p_enc_param,
								  VOID *p_cb_func,
								  VOID *user_data);
 * @brief trigger hardware engine
 * @param handler 				the handler of hardware
 * @param p_enc_param		 	the encode one frame settings
 * @param p_cb_func 			the callback function
 * @param user_data 			the private user data
 * @return return 0 on success, -1 on error
 */
#if defined(_NVT_NVR_SDK_)
INT32 kdrv_videoenc_trigger(UINT32 id, void *p_param);
#else
INT32 kdrv_videoenc_trigger(UINT32 id, KDRV_VDOENC_PARAM *p_enc_param,
						  KDRV_CALLBACK_FUNC *p_cb_func,
						  VOID *p_user_data);
#endif
/*!
 * @fn INT32 kdrv_videoenc_get(KDRV_VDOENC_ID handler, DEC_ID id, VOID *param)
 * @brief set parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_get(UINT32 id, KDRV_VDOENC_GET_PARAM_ID parm_id, VOID *param);

/*!
 * @fn INT32 kdrv_videoenc_set(KDRV_VDOENC_ID handler, DEC_ID id, VOID *param)
 * @brief get parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_videoenc_set(UINT32 id, KDRV_VDOENC_SET_PARAM_ID parm_id, VOID *param);


#endif

