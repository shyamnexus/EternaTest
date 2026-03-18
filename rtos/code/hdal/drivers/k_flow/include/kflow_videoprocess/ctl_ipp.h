/**
    IPL_Ctrl Layer

    @file       IPL_Ctrl.h
    @ingroup    mILibIPH
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_IPP_H_
#define _CTL_IPP_H_

#include "kflow_common/type_vdo.h"
#include "ctl_ipp_util.h"
#include "ipp_event.h"

/**********************************/
/*  ipp_ctl_ipl task ctrl         */
/**********************************/
ER ctl_ipp_open_tsk(void);
ER ctl_ipp_close_tsk(void);
ER ctl_ipp_set_resume(BOOL b_flush_evt);
ER ctl_ipp_set_pause(BOOL b_wait_end, BOOL b_flush_evt);
ER ctl_ipp_wait_pause_end(void);


/**********************************/
/*  ipp_ctl_ipl handle ctrl       */
/**********************************/
#define CTL_IPP_HDR_MAX_FRAME_NUM (2)
#define CTL_IPP_COMB_NUM_MAX (1)
#define CTL_IPP_STRP_NUM_MAX (8)	// ref to KDRV_IPP_MAX_STP_NUM
#define CTL_IPP_AI_EFFECT_ID_MAX (16)
#define CTL_IPP_ISP_AIISP_PARAM_MAX	(32)
#define CTL_IPP_ISP_AIISP_RSV_PARAM_MAX	(256)
#define CTL_IPP_BIND_ISP_MAX (3)

typedef void (*CTL_IPP_DUMP_FP)(char *fmt, ...);

typedef enum {
	CTL_IPP_FLOW_UNKNOWN = 0,
	CTL_IPP_FLOW_RAW,
	CTL_IPP_FLOW_DIRECT_RAW,
	CTL_IPP_FLOW_CCIR,
	CTL_IPP_FLOW_DIRECT_CCIR,
	CTL_IPP_FLOW_IME_D2D,
	CTL_IPP_FLOW_IPE_D2D,
	CTL_IPP_FLOW_VR360,
	CTL_IPP_FLOW_DCE_D2D,
	CTL_IPP_FLOW_CAPTURE_RAW,
	CTL_IPP_FLOW_CAPTURE_CCIR,
	CTL_IPP_FLOW_RAWCOLOR,
	CTL_IPP_FLOW_RAW_LITE,
	CTL_IPP_FLOW_RAWCOLOR_LITE,
	CTL_IPP_FLOW_DIRECT_PRE,
	CTL_IPP_FLOW_PRE_D2D,
	CTL_IPP_FLOW_MAX
} CTL_IPP_FLOW_TYPE;

#define CTL_IPP_IS_CAPTURE_FLOW(flow) ((flow == CTL_IPP_FLOW_CAPTURE_RAW) || (flow == CTL_IPP_FLOW_CAPTURE_CCIR))
#define CTL_IPP_IS_DIRECT_FLOW(flow) ((flow == CTL_IPP_FLOW_DIRECT_RAW) || (flow == CTL_IPP_FLOW_DIRECT_CCIR) || (flow == CTL_IPP_FLOW_DIRECT_PRE))

typedef enum {
	CTL_IPP_CBEVT_IN_BUF_PROCEND = 0,
	CTL_IPP_CBEVT_IN_BUF_DROP,
	CTL_IPP_CBEVT_IN_BUF_PROCSTART,
	CTL_IPP_CBEVT_IN_BUF_DIR_PROCEND,
	CTL_IPP_CBEVT_IN_BUF_DIR_DROP,
	CTL_IPP_CBEVT_IN_BUF_MAX
} CTL_IPP_CBEVT_IN_BUF_MSG;

typedef enum {
	CTL_IPP_CBEVT_IN_BUF        = 0 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_OUT_BUF       = 1 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_RHE_ISR   = 2 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_IFE_ISR   = 3 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_DCE_ISR   = 4 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_IPE_ISR   = 5 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_IME_ISR   = 6 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_ENG_IFE2_ISR  = 7 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_DATASTAMP     = 8 | IPP_EVENT_ISR_TAG,	/* p_in: CTL_IPP_DS_CB_INPUT_INFO; p_out: CTL_IPP_DS_CB_OUTPUT_INFO */
	CTL_IPP_CBEVT_PRIMASK       = 9 | IPP_EVENT_ISR_TAG,	/* p_in: CTL_IPP_PM_CB_INPUT_INFO; p_out: CTL_IPP_PM_CB_OUTPUT_INFO */
	CTL_IPP_CBEVT_ENG_SIE_ISR   = 10 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_BUILTIN_EXIT  = 11 | IPP_EVENT_ISR_TAG,	// notify when fastboot builtin ipp is done (resource is released, etc.) and ready to switch to hdal flow
	CTL_IPP_CBEVT_ENG_PRE_ISR   = 12 | IPP_EVENT_ISR_TAG,
	CTL_IPP_CBEVT_MAX = 13,
} CTL_IPP_CBEVT_TYPE;

/*
	ctl_ipp buffer info type
*/
typedef enum {
	CTL_IPP_BUF_TYPE_UNKNOWN = 0,
	CTL_IPP_BUF_TYPE_PRIVATE,		// internal used (ex. nnisp frame mode, aided map)
	CTL_IPP_BUF_TYPE_OUTPUT,		// output path buffer. user can pull out and use it
	CTL_IPP_BUF_TYPE_EXAM,			// internal used (exam code)
	CTL_IPP_BUF_TYPE_MAX,
} CTL_IPP_BUF_TYPE;

typedef enum {
	CTL_IPP_ALGID_IQ = 0,
	CTL_IPP_ALGID_MAX,
} CTL_IPP_ALGID_TYPE;

/*
	stripe rule select
	for stripe auto mode usage -> kdrv_ipp auto calculate stripe
	rule definition is define in kdrv_ipp
*/
typedef enum {
	CTL_IPP_STRP_RULE_AI_ISP_MST_PRIOR = 0,
	CTL_IPP_STRP_RULE_AI_ISP_SST_3840_PRIOR = 1,
	CTL_IPP_STRP_RULE_CODEC_LOW_LATENCY_PRIOR = 2,
	CTL_IPP_STRP_RULE_MAX,
} CTL_IPP_STRP_RULE_SELECT;

typedef enum {
	CTL_IPP_AI_FILT_0,
	CTL_IPP_AI_FILT_1,
	CTL_IPP_AI_FILT_2,
	CTL_IPP_AI_FILT_3,
	CTL_IPP_AI_FILT_MAX,

	CTL_IPP_AI_FILT_IFE0 = CTL_IPP_AI_FILT_0,
	CTL_IPP_AI_FILT_IFE1 = CTL_IPP_AI_FILT_1,
	CTL_IPP_AI_FILT_IME0 = CTL_IPP_AI_FILT_2,
	CTL_IPP_AI_FILT_IME1 = CTL_IPP_AI_FILT_3,
} CTL_IPP_AI_FILT_ID;

typedef struct {
	ULONG va;
	ULONG pa;
} CTL_IPP_BUF_ADDR;

typedef struct {
	UINT32	buf_id;
	ULONG	data_addr;
	UINT32	rev;
	INT32   err_msg;
} CTL_IPP_EVT;

typedef struct {
	UINT32	buf_id[CTL_IPP_HDR_MAX_FRAME_NUM];
	ULONG	data_addr[CTL_IPP_HDR_MAX_FRAME_NUM];
	UINT32	rev;
	INT32   err_msg;
} CTL_IPP_EVT_HDR;

typedef struct {
	UINT32	buf_id;
	ULONG	buf_addr;
	ULONG   reserved; /* for eis use to pass VDO_FRAME vdo_frm */
} CTL_IPP_DIR_EVT;

typedef union {
	CTL_IPP_EVT dft_evt;		/* default event type */
	CTL_IPP_DIR_EVT dir_evt;	/* direct mode event type */
} CTL_IPP_EVT_UNION;

typedef struct {
	CTL_IPP_CBEVT_TYPE cbevt;
	IPP_EVENT_FP fp;
} CTL_IPP_REG_CB_INFO;

typedef struct {
	CTL_IPP_ALGID_TYPE type;
	UINT32 id;
	UINT32 frame_num; // shdr frame number for different fcurve initial value. 0~1: shdr disable. n: n frame
} CTL_IPP_ALGID;

typedef struct {
	CTL_IPP_IN_CROP_MODE mode;
	URECT crp_window;	/* only effect when CTL_IPP_IN_CROP_USER */
} CTL_IPP_IN_CROP;

typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	BOOL enable;
	VDO_PXLFMT fmt;
	UINT32 lofs;
	URECT pre_crp_window;	// crop before scale
	USIZE size;				// scale size
	URECT crp_window;		// crop after scale
} CTL_IPP_OUT_PATH;

typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	BOOL enable;
} CTL_IPP_OUT_PATH_FLIP;

typedef struct {
	CTL_IPP_BUF_ADDR start_addr;
	UINT32 size;
} CTL_IPP_BUFCFG;

typedef struct {
	USIZE max_size;
	VDO_PXLFMT pxlfmt;
	CTL_IPP_FUNC func_en;
	UINT32 buf_size;
	UINT32 max_strp_num; // 530 not support. use CTL_IPP_ITEM_STRP_NUM to get max stripe number
} CTL_IPP_PRIVATE_BUF;

/**
	Buffer info for bufio_callback used
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT32 buf_size;
	UINT32 buf_id;
	CTL_IPP_BUF_ADDR buf_addr;
	VDO_FRAME vdo_frm;
	/* reserved usage of vdo_frm
		reserve[0:3] low latency(VDO_LOW_DLY_INFO)
		reserve[0:1] height aling {MAKEFOURCC('H', 'A', 'L', 'N'), height}
		reserve[2:3] slice index {MAKEFOURCC('S', 'L', 'I', 'C'), (slice_cnt << 16 | slice_idx)}
		reserve[0:2] 3DNR motion sum & edge sum {MAKEFOURCC('3', 'D', 'S', 'M'), motion_sum, edge_sum}
	*/
	UINT32 lock_cnt;
	INT32 err_msg;
} CTL_IPP_OUT_BUF_INFO;

/**
	flush config
	after flush a path, all buffer io callback of the path is finished
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
} CTL_IPP_FLUSH_CONFIG;

/**
	output buffer mode
	one buffer mode will skip buf_tag check in direct_raw flow
	default is disable
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT32 one_buf_mode_en;
} CTL_IPP_OUT_PATH_BUFMODE;

/**
	output path md enable
	enable will attach md info as metadata to path output
	must enable 3dnr first to activate this function
	default is disable
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT32 enable;
} CTL_IPP_OUT_PATH_MD;

/**
	output path yuv buffer height alignment
	will use vdoframe.reserve[0], [1]
	VDO_FRAME.reserved[0] = MAKEFOURCC('H', 'A', 'L', 'N'); // height align
	VDO_FRAME.reserved[1] = y_height after align
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT32 align;
} CTL_IPP_OUT_PATH_HALIGN;

/**
	output path push order
	push from small to big
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT8 order;
} CTL_IPP_OUT_PATH_ORDER;

/**
	output path region
	enable will output image data to region of background
*/
typedef struct {
	CTL_IPP_OUT_PATH_ID pid;
	UINT8 enable;
	UINT32 bgn_lofs;
	USIZE bgn_size;
	UPOINT region_ofs;
} CTL_IPP_OUT_PATH_REGION;

/**
	slice push config
	enable seperate for each path
	cnt is the total slice number (range: 2 ~ CTL_IPP_SLICE_IDX_CNT_MAX)
	ex: cnt = 8, push 8 times per frame
*/
#define CTL_IPP_SLICE_IDX_CNT_MAX (0xED)
typedef struct {
	UINT8 enable[CTL_IPP_OUT_PATH_ID_MAX];
	UINT8 cnt;
} CTL_IPP_SLICE_PUSH;

/**
	AI information
*/
typedef struct {
	UINT32 proc_id;
	ULONG pa;			///< physical address
	ULONG va;
	UINT32 width;
	UINT32 height;
	UINT32 line_ofs;
	UINT32 fmt; 		///< hdal fmt
	UINT32 proc_time;
} CTL_IPP_NN_ISP_FRAME;

typedef struct {
  BOOL en;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 isp_id; //indicate which sensor to run
  UINT32 path_id;
} CTL_IPP_NN_ISP_IN_PARAM;

typedef struct {
  UINT32 max_stripe;
  UINT32 min_stripe;
} CTL_IPP_NN_ISP_NN_STRP;

typedef struct {
  UINT32 stripe_width[CTL_IPP_STRP_NUM_MAX];
  UINT32 stripe_num;
  UINT32 stripe_overlap;
  UINT32 width;
  UINT32 height;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
} CTL_IPP_NN_ISP_SET_RES_PARAM;

typedef struct {
  UINT32 max_stripe;
  UINT32 min_stripe;
  UINT32 slice_overlap;
  UINT32 slice_max_out_height;
  UINT8  sw_handshake_en;
} CTL_IPP_NN_ISP_NN_INFO;

/**
	motion type (sync with KDRV_IFE_MOTION_TYPE / KDRV_IPE_MOTION_TYPE)
*/
typedef enum {
	CTL_IPP_NN_ISP_MOTION_TYPE_64 = 0,
	CTL_IPP_NN_ISP_MOTION_TYPE_16,
	CTL_IPP_NN_ISP_MOTION_TYPE_4,
	CTL_IPP_NN_ISP_MOTION_TYPE_MAX,
} CTL_IPP_NN_ISP_MOTION_TYPE;

typedef struct {
	UINT32 proc_id;
	BOOL en;
	CTL_IPP_NN_ISP_MOTION_TYPE type;
} CTL_IPP_NN_MOTION_TYPE_PARAM;

typedef struct {
	UINT32 proc_id;
	UINT32 param_num;
	UINT32 param_size[CTL_IPP_ISP_AIISP_PARAM_MAX];
	ULONG  param_phyaddr[CTL_IPP_ISP_AIISP_PARAM_MAX];
} CTL_IPP_NN_ISP_ISP_PARAM;

#define CTL_IPP_NN_CHK_IPP_VER	1	// current IPP version. VER should "+1" if following struct changed
typedef struct {
	UINT32 version;					// (in)IPP struct version
	UINT32 proc_id;					// (in)IPP want to check which proc_id
	ULONG reserved[31];
} CTL_IPP_NN_IPP_CHK_PARAM;
STATIC_ASSERT(sizeof(CTL_IPP_NN_IPP_CHK_PARAM) <= 0x100);

#define CTL_IPP_NN_CHK_AI_VER	1	// the MAX version of AI which current SDK available to check. refer to NN_CHK_AI_VER in kflow_ai_isp_net.h
typedef struct {
	UINT32 version;					// (out)AI struct version, should be set as CTL_IPP_NN_CHK_AI_VER
	UINT32 fmt;						// (out)for IPP, but only BPP(b'23~b'16) is actually valid
	ULONG reserved[31];
} CTL_IPP_NN_AI_CHK_PARAM;
STATIC_ASSERT(sizeof(CTL_IPP_NN_AI_CHK_PARAM) <= 0x100);

typedef struct {
	UINT32 version;
	UINT32 proc_id;
	ULONG data_va;					// cast to NN_ISP_PER_FRAME_PARAM_DATA_Vxx to use, where xx=version
	UINT32 data_size;
	ULONG reserved[8];
} CTL_IPP_NN_ISP_PER_FRAME_PARAM;

typedef enum {
	CTL_IPP_NN_ISP_OP_GET_MAX_STRP			= 2,	// get NN max stripe width			// call when enable					input: CTL_IPP_NN_ISP_IN_PARAM			output: CTL_IPP_NN_ISP_NN_STRP
	CTL_IPP_NN_ISP_OP_CAL_SLICE				= 3,	// set NN to calculate slice info	// call after NN_ISP_OP_GET_MAX_STRP(enable)	input: CTL_IPP_NN_ISP_SET_RES_PARAM		output: CTL_IPP_NN_ISP_NN_INFO
	CTL_IPP_NN_ISP_OP_PUSH_FRAME			= 100,	// push frame						// call every frame					input: CTL_IPP_NN_ISP_FRAME				output: CTL_IPP_NN_ISP_FRAME
	CTL_IPP_NN_ISP_OP_FRAME_MODE			= 101,	// get FRAME_MODE 1:FRAME_MODE 0:NN_JOB Mode	// call when enable		input: N/A								output: UINT32 mode
	CTL_IPP_NN_ISP_OP_FRAME_UPDATE			= 102,	// notify AI for event				// call when update event(disable)	input: CTL_IPP_NN_ISP_IN_PARAM			output: N/A
	CTL_IPP_NN_ISP_OP_SET_FUSION_WEIGHT		= 103,	// set FUSION_WEIGHT				// call every frame					input: CTL_IPP_NN_ISP_FRAME				output: CTL_IPP_NN_ISP_FRAME
	CTL_IPP_NN_ISP_OP_SET_SIGMA				= 104,	// set SIGMA						// call every frame					input: CTL_IPP_NN_ISP_FRAME				output: CTL_IPP_NN_ISP_FRAME
	CTL_IPP_NN_ISP_OP_SET_GAMMA				= 105,	// set GAMMA						// call every frame					input: CTL_IPP_NN_ISP_FRAME				output: CTL_IPP_NN_ISP_FRAME
	CTL_IPP_NN_ISP_OP_SET_MULTI_FRAME		= 106,	// set multiple frame				// call every frame					input: CTL_IPP_NN_ISP_MULTI_FRAME_INFO	output: CTL_IPP_NN_ISP_MULTI_FRAME_INFO
	CTL_IPP_NN_ISP_OP_GET_MOTION_TYPE		= 107,	// get motion type					// call when update event			input: N/A								output: CTL_IPP_NN_MOTION_TYPE_PARAM
	CTL_IPP_NN_ISP_OP_SET_ISP_PARAM			= 108,	// set isp parameter				// call when update event			input: CTL_IPP_NN_ISP_ISP_PARAM			output: N/A
	CTL_IPP_NN_ISP_OP_CHK_PARAM_VALID		= 109,	// check model & flow setting match	// call every frame					input: CTL_IPP_NN_IPP_CHK_PARAM			output: CTL_IPP_NN_AI_CHK_PARAM
	CTL_IPP_NN_ISP_OP_SET_PER_FRAME_PARAM	= 110,	// set isp reserved parameter		// call every frame					input: CTL_IPP_NN_ISP_PER_FRAME_PARAM	output: N/A
} CTL_IPP_NN_ISP_OPERATION;

typedef INT32 (*CTL_IPP_AI_CB)(CTL_IPP_NN_ISP_OPERATION op, void *p_in, void *p_out);

typedef struct {
	UINT32 enable;										// node enable
	UINT32 proc_id;										// real ai eng id
	UINT32 effect2proc[CTL_IPP_AI_EFFECT_ID_MAX];		// effect id map to proc id
} CTL_IPP_AI_FILTER;

typedef struct {
	CTL_IPP_AI_CB ai_cb;								// [SET] kdrv_ai callback for kdrv_ipp [GET] kdrv_ipp callback for kdrv_ai
	CTL_IPP_AI_FILTER ai_filter[CTL_IPP_AI_FILT_MAX];	// [SET] ai node info
} CTL_IPP_AI_INFO;

typedef struct {
	CTL_IPP_SIE_ID sie_id[CTL_IPP_HDR_MAX_FRAME_NUM][CTL_IPP_COMB_NUM_MAX];
} CTL_IPP_BIND_SIE_INFO;

typedef struct {
	BOOL enable;	// thermal mode enable
} CTL_IPP_THERMAL_INFO;

typedef struct {
	BOOL preroll_mode_enable;		// preroll mode enable

	/**
		fastboot queue mode means kdrv builtin ipp will queue number of yuv buffer and process yuv data after come back to hdal
		this info will dump to dtsi to let builtin know if need queue yuv buffer
		queue num will be used when queue mode is enable, and hdal use other api to set queue mode enable
		hdal should set queue num according to vproc depth number
	*/
	UINT32 que_num[CTL_IPP_OUT_PATH_ID_MAX];

	/**
		fastboot sitch mode is used for stich 2 sensor ime output imgs into one img in builtin
		this info will dump to dtsi to provide information for builtin do the stitch
	*/
	UINT32 stitch_en[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_buf_y_size[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_buf_uv_size[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_addr_y_ofs[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_addr_uv_ofs[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_img_width[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 stitch_img_height[CTL_IPP_OUT_PATH_ID_MAX];
} CTL_IPP_FBOOT_DTSI_INFO;

typedef struct {
	UINT32 bind_num;		// number of handles use same isp_id group. only 0 or 2 ~ CTL_IPP_BIND_ISP_MAX and can not be 1
	UINT32 bind_id;			// handle index in current isp_id group. must be 0 ~ (bind_num - 1)
} CTL_IPP_BIND_ISP_INFO;

/* *
	CTL_IPP_GET/SET ITEM
*/
typedef enum {
	CTL_IPP_ITEM_FLOWTYPE = 0,      /* flow type.                       GET-Only,   data_type: CTL_IPP_FLOW_TYPE        */
	CTL_IPP_ITEM_REG_CB_IMM,        /* Callback function register.      SET-Only,   data_type: CTL_IPP_REG_CB_INFO      */
	CTL_IPP_ITEM_UNREG_CB_IMM,      /* Callback function unregister.    SET-Only,   data_type: CTL_IPP_REG_CB_INFO      */
	CTL_IPP_ITEM_ALGID_IMM,         /* IPP ALG ISP id                   GET/SET,    data_type: CTL_IPP_ALGID            */
	CTL_IPP_ITEM_BUFCFG,            /* ipp inner buffer configuration.  GET-Only,   data_type: CTL_IPP_BUFCFG           */
	CTL_IPP_ITEM_IN_CROP,           /* Input Crop window.               GET/SET,    data_type: CTL_IPP_IN_CROP          */
	CTL_IPP_ITEM_OUT_PATH,          /* Output path Info.                GET/SET,    data_type: CTL_IPP_OUT_PATH         */
	CTL_IPP_ITEM_FUNCEN,            /* ipp function enable.             GET/SET,    data_type: CTL_IPP_FUNC             */
	CTL_IPP_ITEM_SCL_METHOD,        /* ipp output scale method.         GET/SET,    data_type: CTL_IPP_SCL_METHOD_SEL   */
	CTL_IPP_ITEM_FLIP,              /* ipp flip/mirror.                 GET/SET,    data_type: CTL_IPP_FLIP_TYPE        */
	CTL_IPP_ITEM_3DNR_REFPATH_SEL,  /* ipp 3dnr reference path select.  GET/SET,    data_type: CTL_IPP_OUT_PATH_ID      */
	CTL_IPP_ITEM_OUT_COLORSPACE,	/* ipp output color space			GET/SET,	data_type: CTL_IPP_OUT_COLOR_SPACE	*/
	CTL_IPP_ITEM_APPLY,             /* Apply parameters to next trigger SET-Only,   data_type: UINT32                   */
	CTL_IPP_ITEM_BUFQUY,            /* ipp get private buf              GET-Only,   data_type: CTL_IPP_PRIVATE_BUF      */
	CTL_IPP_ITEM_FLUSH,             /* ipp flush all buffer(in&out)     SET-Only,   data_type: CTL_IPP_FLUSH_CONFIG or NULL */
	CTL_IPP_ITEM_PUSHEVT_INQ,       /* ipp get push event number in que GET-Only,   data_type: UINT32                   */
	CTL_IPP_ITEM_OUT_PATH_FLIP,     /* Output path flip(ime only)       GET/SET,    data_type: CTL_IPP_OUT_PATH_FLIP    */
	CTL_IPP_ITEM_LOW_DELAY_PATH_SEL,/* low delay mode path_sel          GET/SET,    data_type: CTL_IPP_OUT_PATH_ID      */
	CTL_IPP_ITEM_LOW_DELAY_BP,		/* low delay mode bp timing(lines)  GET/SET,    data_type: UINT32                   */
	CTL_IPP_ITEM_OUT_PATH_BUFMODE,	/* ipp output path buffer mode      GET/SET,    data_type: CTL_IPP_OUT_PATH_BUFMODE */
	CTL_IPP_ITEM_OUT_PATH_MD,		/* ipp output path with md enable   GET/SET,    data_type: CTL_IPP_OUT_PATH_MD      */
	CTL_IPP_ITEM_STRP_RULE,			/* ipp stripe rule select(dram mode)GET/SET,    data_type: CTL_IPP_STRP_RULE_SELECT */
	CTL_IPP_ITEM_OUT_PATH_HALIGN,	/* ipp output path height align     GET/SET,    data_type: CTL_IPP_OUT_PATH_HALIGN  */
	CTL_IPP_ITEM_OUT_PATH_ORDER_IMM,/* ipp output path push order       GET/SET,    data_type: CTL_IPP_OUT_PATH_ORDER   */
	CTL_IPP_ITEM_OUT_PATH_REGION,	/* ipp output path region           GET/SET,    data_type: CTL_IPP_OUT_PATH_REGION  */
	CTL_IPP_ITEM_SLICE_PUSH_IMM,    /* slice push configuration         GET/SET,    data_type: CTL_IPP_SLICE_PUSH       */
	CTL_IPP_ITEM_STRP_NUM, 			/* ipp max stripe number            GET-Only,   data_type: UINT32                   */
	CTL_IPP_ITEM_AI_INFO,           /* AI infor for kdrv                GET/SET     data_type: CTL_IPP_AI_INFO          */
	CTL_IPP_ITEM_BIND_SIE,          /* direct mode bind sie             SET-Only    data_type: CTL_IPP_BIND_SIE_INFO    */
	CTL_IPP_ITEM_THERMAL_INFO,      /* thermal info                     SET-Only    data_type: CTL_IPP_THERMAL_INFO     */
	CTL_IPP_ITEM_FBOOT_DTSI_INFO,	/* user manual dtsi info            GET/SET     data_type: CTL_IPP_FBOOT_DTSI_INFO  */
	CTL_IPP_ITEM_MAX_FUNC, 			/* ipp max support function         GET-Only,   data_type: CTL_IPP_FUNC             */
	CTL_IPP_ITEM_BIND_ISP,			/* multiple handle use same isp_id  SET-Only    data_type: CTL_IPP_BIND_ISP_INFO    */
	CTL_IPP_ITEM_MAX
} CTL_IPP_ITEM;

typedef enum {
	CTL_IPP_IOCTL_SNDEVT = 0,       	/* Send event.          data_type: CTL_IPP_EVT      */
	CTL_IPP_IOCTL_SNDSTART,         	/* Send event.          data_type: void		        */
	CTL_IPP_IOCTL_SNDSTOP,          	/* Send event.          data_type: void      	 	*/
	CTL_IPP_IOCTL_SNDEVT_HDR,       	/* Send event.          data_type: CTL_IPP_EVT_HDR  */
	CTL_IPP_IOCTL_SNDWAKEUP,        	/* Send event.          data_type: void		        */
	CTL_IPP_IOCTL_SNDSLEEP,         	/* Send event.          data_type: void      	 	*/
	CTL_IPP_IOCTL_DMA_ABORT,			/* Set dma abort.       data_type: void      	 	*/
	CTL_IPP_IOCTL_MAX
} CTL_IPP_IOCTL;

typedef enum {
	CTL_IPP_VR360_TOP = 0,         /* VR360 process top position */
	CTL_IPP_VR360_BOTTOM,          /* VR360 process bottom position */
	CTL_IPP_VR360_MAX
} CTL_IPP_VR360_POSITION;

typedef enum {
	CTL_IPP_DIRECT_IPP_STS_STOP,	/* hdal not set ipp ready for sie preparing */
	CTL_IPP_DIRECT_IPP_STS_READY,	/* hdal already set ipp ready for sie preparing */
	CTL_IPP_DIRECT_IPP_STS_START,	/* ipp already prepared by hdal or sie */
} CTL_IPP_DIRECT_IPP_STS;

typedef enum {
	CTL_IPP_DIRECT_START 		= 0x00000001, 	/* Direct mode start */
	CTL_IPP_DIRECT_STOP  		= 0x00000002,	/* Direct mode stop */
	CTL_IPP_DIRECT_PROCESS  	= 0x00000004,	/* Direct mode process */
	CTL_IPP_DIRECT_IN_RE 		= 0x00000008,	/* Direct mode release input buffer */
	CTL_IPP_DIRECT_IN_DROP 		= 0x00000010,	/* Direct mode drop input buffer */
	CTL_IPP_DIRECT_SKIP  		= 0x00000020,	/* Direct mode skip cfg */
	CTL_IPP_DIRECT_GET_STS 		= 0x00000040,	/* get ipp status. in: NULL. out: CTL_IPP_DIRECT_IPP_STS */
	CTL_IPP_DIRECT_PREPARE 		= 0x00000080,	/* set ipp prepare, change status to CTL_IPP_DIRECT_IPP_STS_START. in: NULL. out: NULL */
	CTL_IPP_DIRECT_UNPREPARE 	= 0x00000100,	/* set ipp unprepare, change status to CTL_IPP_DIRECT_IPP_STS_READY. in: NULL. out: NULL */
	CTL_IPP_DIRECT_MAX
} CTL_IPP_DIRECT_EVENT;

/**
	SIE direct cb info
*/
typedef enum {
	CTL_IPP_DUAL_DIRECT_UNKNOWN,
	CTL_IPP_DUAL_DIRECT_DISABLE,		///< single direct mode
	CTL_IPP_DUAL_DIRECT_INTERLACE,		///< output interlace mode for vcap0 and vcap1
										///<   sie1(o) | sie1(x) | sie1(o) | sie1(x) ...
										///<   sie2(x) | sie2(o) | sie2(x) | sie2(o) ...
	CTL_IPP_DUAL_DIRECT_PROGRESSIVE,	///< output progressive mode for vcap0 and vcap1
										///<    sie1   |         |  sie1   |         ...
										///<           |  sie2   |         |  sie2   ...
} CTL_IPP_DUAL_DIRECT_MODE;

// direct callback info (sync with CTL_SIE_DIRECT_CB_TO_IPP_INFO in ctl_sie.h)
typedef struct {
	VDO_FRAME vdo_frm;			// hdal video frame info. refer to CTL_SIE_HEADER_INFO
	UINT32 sie_id;				// sie id
	UINT32 comm_buf_id;			// comm buf id
	ULONG comm_buf_addr;		// comm buf addr
	BOOL ife_chk_data;			// direct start only
								// 		* sie1 case : ife_chk_mode(560 hw only support 0, 530 hw support 0,1 default 1)
								// 		* sie2~x case :ife_chk_en
	UINT32 ring_buf_len;		// ring buffer length
	UINT32 sen_row_time;		// row time (usx10)

	// combine mode
	BOOL comb_en;				// combine mode enable
	UINT32 comb_idx_bit;		// indicate which sie's will be combined (ex. CTL_SIE_ID_IDX_1 | CTL_SIE_ID_IDX_4)
	UINT32 comb_num;			// combine mode stripe number (1 ~ CTL_IPP_COMB_NUM_MAX)
	UINT32 comb_order;			// combine mode stripe idx (0 ~ comb_num-1)
	UINT32 comb_width;			// combine mode stripe width (with overlap width)
	UINT32 comb_ovlp_width;		// combine mode overlap width (right overlap width)

	// dual direct mode
	CTL_IPP_DUAL_DIRECT_MODE dual_mode;	// dual direct sie mode
} CTL_IPP_DIRECT_CB_FROM_SIE_INFO;

// direct callback info (sync with CTL_SIE_DIRECT_CB_FROM_IPP_INFO in ctl_sie.h)
typedef struct {
	UINT32 sie_id;				// sie id
} CTL_IPP_DIRECT_CB_TO_SIE_INFO;

typedef enum {
	CTL_IPP_DIRECT_SIE_STS_STOP,	/* hdal not set sie ready for ipp preparing */
	CTL_IPP_DIRECT_SIE_STS_READY,	/* hdal already set sie ready for ipp preparing */
	CTL_IPP_DIRECT_SIE_STS_START,	/* sie already prepared by hdal or ipp */
} CTL_IPP_DIRECT_SIE_STS;

typedef enum {
	CTL_IPP_DIRECT_SIE_GET_STS, 	/* get sie status. in: NULL. out: CTL_IPP_DIRECT_SIE_STS */
	CTL_IPP_DIRECT_SIE_PREPARE,		/* set sie prepare, change status to CTL_IPP_DIRECT_SIE_STS_START. in: NULL. out: NULL */
	CTL_IPP_DIRECT_SIE_UNPREPARE,	/* set sie unprepare, change status to CTL_IPP_DIRECT_SIE_STS_READY. in: NULL. out: NULL */
	CTL_IPP_DIRECT_SIE_MAX,
} CTL_IPP_DIRECT_SIE_EVENT;

typedef struct {
	UINT32 n;					/* kflow handle number */
	UINT32 kdrv_que_depth;		/* kdrv queue number. suggest 3 for best perfomance & memory usage */
	UINT32 kdrv_ll_en;			/* if enable, allocate linklist buffer for kdrv */
	UINT32 kdrv_cnn_sram_en;	/* if enable, dce will use cnn sram */
} CTL_IPP_CTX_BUF_CFG;

UINT32 ctl_ipp_query(CTL_IPP_CTX_BUF_CFG ctx_buf_cfg);
INT32 ctl_ipp_init(CTL_IPP_CTX_BUF_CFG ctx_buf_cfg, CTL_IPP_BUF_ADDR buf_addr, UINT32 buf_size);
INT32 ctl_ipp_uninit(void);
ULONG ctl_ipp_open(CHAR *name, CTL_IPP_FLOW_TYPE flow);
INT32 ctl_ipp_close(ULONG hdl);
INT32 ctl_ipp_set(ULONG hdl, UINT32 item, void *data);
INT32 ctl_ipp_get(ULONG hdl, UINT32 item, void *data);
INT32 ctl_ipp_ioctl(ULONG hdl, UINT32 item, void *data);

/* wait jarkko prototype */
INT32 ctl_ipp_direct_flow_cb(UINT32 event, void *p_in, void *p_out);
ULONG ctl_ipp_get_dir_fp(void *data);

#endif // _CTL_IPP_H_
