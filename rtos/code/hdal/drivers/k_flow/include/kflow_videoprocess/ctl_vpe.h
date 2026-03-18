/**
    VPE_Ctrl Layer

    @file       ctl_vpe.h
    @ingroup    mIVPE_Ctrl
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_VPE_H_
#define _CTL_VPE_H_

#include "kwrap/error_no.h"
#include "kflow_common/type_vdo.h"

/* limit */
#define CTL_VPE_LMT_MAX_OUT_PATH_ID			CTL_VPE_OUT_PATH_ID_MAX
#define CTL_VPE_LITE_LMT_MAX_OUT_PATH_ID 	CTL_VPE_OUT_PATH_ID_2
#define CTL_VPE_PTZ_MAX_ANGLE 				1800
#define CTL_VPE_PALETTE_MAX					8
#define CTL_VPE_MASK_NUM_MAX				8
#if defined(__aarch64__)
#define CTL_VPE_VSP_FRM_NUM_MAX 			8
#else	//32bit only support 4 frame vsp, ULONG reserved only 32bit
#define CTL_VPE_VSP_FRM_NUM_MAX 			4
#endif

/* vpe callback fp type */
typedef INT32 (*CTL_VPE_EVENT_FP)(UINT32 event, void *p_in, void *p_out);

/* vpe error type */
#define CTL_VPE_E_OK				(E_OK)
#define CTL_VPE_E_SYS				(E_SYS)
#define CTL_VPE_E_PAR				(E_PAR)		///< parameters error
#define CTL_VPE_E_NOMEM				(E_NOMEM)	///< no memory error
#define CTL_VPE_E_NOSPT				(E_NOSPT)	///< not support error
#define CTL_VPE_E_QOVR				(E_QOVR)	///< queue overflow error
#define CTL_VPE_E_ID				(E_ID)		///< id error
#define CTL_VPE_E_INDATA			(-101)		///< input data error
#define CTL_VPE_E_FLUSH				(-102)		///< flush error
#define CTL_VPE_E_DROP_INPUT_ONLY	(-103)		///<
#define CTL_VPE_E_OUTDATA			(-104)		///< output data error
#define CTL_VPE_E_NULL				(-105)		///< null pointer error
#define CTL_VPE_E_STATE				(-106)		///< state error
#define CTL_VPE_E_KDRV_OPEN			(-200)		///< kdrv open error
#define CTL_VPE_E_KDRV_CLOSE		(-201)		///< kdrv close error
#define CTL_VPE_E_KDRV_SET			(-202)		///< kdrv set error
#define CTL_VPE_E_KDRV_GET			(-203)		///< kdrv get error
#define CTL_VPE_E_KDRV_TRIG			(-204)		///< kdrv trigger error
#define CTL_VPE_E_KDRV_STRP			(-205)		///< kdrv stripe cal error
#define CTL_VPE_E_KDRV_DROP			(-206)		///< kdrv stripe cal error

typedef struct{
	ULONG va;	/* virtual address */
	ULONG pa;	/* physical address */
}CTL_VPE_BUF_ADDR;

typedef enum {
	CTL_VPE_FLOW_VPE,		/* vpe */
	CTL_VPE_FLOW_VPE_PTZ,	/* N.S. for 538 */
	CTL_VPE_FLOW_MAX
} CTL_VPE_FLOW_TYPE;

/* init/query config */
typedef struct {
	UINT32 handle_num;			/* number of handles */
	UINT32 queue_num;			/* number of queue nodes */
} CTL_VPE_CTX_BUF_CFG;


/* vpe callback event type */
typedef enum {
	CTL_VPE_CBEVT_IN_BUF = 0,
	CTL_VPE_CBEVT_OUT_BUF,
	CTL_VPE_CBEVT_MAX,
} CTL_VPE_CBEVT_TYPE;

typedef struct {
	CTL_VPE_CBEVT_TYPE cbevt;
	CTL_VPE_EVENT_FP fp;
} CTL_VPE_REG_CB_INFO;

/* buffer io operation for both in/out buf callback */
typedef enum {
	CTL_VPE_BUF_NEW = 0,	/* new buffer */
	CTL_VPE_BUF_PUSH,		/* push ready buffer */
	CTL_VPE_BUF_LOCK,		/* lock buffer */
	CTL_VPE_BUF_UNLOCK,		/* unlock buffer */
	CTL_VPE_TRIG_END,		/* trigger kdrv end */
	CTL_VPE_BUF_IO_MAX,
} CTL_VPE_BUF_IO;


/* snd event information, also for in bufio callback */
/*
	data_addr = pointer of input VDO_FRAME
	VDO_FRAME.reserved[0] = MAKEFOURCC('H', 'A', 'L', 'N'), (opt.)
			 .reserved[1] = input buffer height alignment, (opt.)
			 .reserved[2] = 2d_lut_width[0...11], 2d_lut_height[12...23], 2d_lut_lofs[24...35], (opt.), user input 2dlut size, lofs and address
			 .reserved[3] = (ULONG)2d_lut phy address, (opt.)
			 .reserved[4] = out_shift[0..7]
			 .reserved[5] = source in crop start y[0...15]|x[16...31], (opt.)
			 .reserved[6] = source in crop start h[0...15]|w[16...31], (opt.)
			 .reserved[7] = frame_idx[0...15]|blend_num[16...31], vsp flow only
*/
typedef struct {
	ULONG 	user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32	buf_id;		/* buf_id */
	ULONG	data_addr;	/* data address */
	UINT32	rev;
	INT32   err_msg;
} CTL_VPE_EVT;

typedef struct {
	ULONG 	user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32  frm_num;	/* total input frame number */
	UINT32	*buf_id;	/* buf_id array of frm_num size */
	ULONG	*data_addr; /* data_addr array of frm_num size */
	UINT32	rev;
	INT32   err_msg;
} CTL_VPE_EVT_MFRM;

/* in crop config */
typedef enum {
	CTL_VPE_IN_CROP_AUTO = 0,	/* reference input header information */
	CTL_VPE_IN_CROP_NONE,		/* no crop, process full image */
	CTL_VPE_IN_CROP_USER,		/* user define crop window */
	CTL_VPE_IN_CROP_MODE_MAX,
} CTL_VPE_IN_CROP_MODE;

typedef struct {
	CTL_VPE_IN_CROP_MODE mode;	/* input crop mode select */
	URECT crp_window;			/* only effect when CTL_VPE_IN_CROP_USER */
} CTL_VPE_IN_CROP;

/*
out path config
	CTL_VPE_OUT_PATH_ID_5:
		only valid when 2dlut enable,
		only suuport scl_size, n.s. crop
		dce out size will = path5 scale size when path5 enable,
		output fmt y only, value: 0 or 255,
		support start offset
*/

typedef enum {
	//main path, set by CTL_VPE_ITEM_OUT_PATH
	CTL_VPE_OUT_PATH_ID_1 = 0,
	CTL_VPE_OUT_PATH_ID_2,
	CTL_VPE_OUT_PATH_ID_3,
	CTL_VPE_OUT_PATH_ID_4,
	CTL_VPE_OUT_PATH_ID_5,		//538:not support, 539A:support, dce output, only valid when 2dlut enable, size = dce out, n.s. scale or crop, output fmt y only, value: 0 or 255;
	//ext path, set by CTL_VPE_ITEM_OUT_EXT_PATH
	CTL_VPE_OUT_PATH_ID_1_SEG,	//output path1 segment out 1
	CTL_VPE_OUT_PATH_ID_2_SEG,	//output path2 segment out 1
	CTL_VPE_OUT_PATH_ID_3_SEG,	//output path3 segment out 1
	CTL_VPE_OUT_PATH_ID_4_SEG,	//output path4 segment out 1
	CTL_VPE_OUT_PATH_ID_5_SEG,	//538:not support, 539A:support, output path5 segment out 1, 2dlut output, only valid when dce enable, size = dce out, n.s. scale or crop, output fmt y only, value: 0 or 255;
	CTL_VPE_OUT_PATH_ID_1_SEG2,	//538:not support, 539A:support, output path1 segment out 2
	CTL_VPE_OUT_PATH_ID_2_SEG2,	//538:not support, 539A:support, output path2 segment out 2
	CTL_VPE_OUT_PATH_ID_3_SEG2,	//538:not support, 539A:support, output path3 segment out 2
	CTL_VPE_OUT_PATH_ID_4_SEG2,	//538:not support, 539A:support, output path4 segment out 2
	CTL_VPE_OUT_PATH_ID_5_SEG2,	//538:not support, 539A:support, output path5 segment out 2, 2dlut output, only valid when dce enable, size = dce out, n.s. scale or crop, output fmt y only, value: 0 or 255;
	CTL_VPE_OUT_PATH_ID_MAX,
} CTL_VPE_OUT_PATH_ID;

typedef enum {
	CTL_VPE_OUT_SCALE_METHOD_AUTO = 0,	/* depend on in/out size auto selection */
	CTL_VPE_OUT_SCALE_METHOD_BILINEAR,	/* path0/1/2/3 */
	CTL_VPE_OUT_SCALE_METHOD_ISD2,		/* path1/2/3, only scale down */
	CTL_VPE_OUT_SCALE_METHOD_MAX
} CTL_VPE_OUT_SCALE_METHOD_SEL;

typedef enum {
	CTL_VPE_OUT_SEG_OFF,
	CTL_VPE_OUT_SEG_LEFT_RIGHT,
	CTL_VPE_OUT_SEG_UP_DOWN,
	CTL_VPE_OUT_SEG_OP_MAX
} CTL_VPE_OUT_SEG_OP;

typedef struct {
	CTL_VPE_OUT_PATH_ID pid;	/* path id */
	CTL_VPE_OUT_SEG_OP op;
	UINT32 seg_pos;		/* segment position from source path */
	USIZE bg_size;		/* background size(buffer size) */
	UPOINT dst_pos;		/* destination position base on background */
} CTL_VPE_OUT_EXT_PATH;

/*
fmt
in:
	VDO_PXLFMT_YUV420
	VDO_PXLFMT_YUV420_NVX2
out:
	VDO_PXLFMT_YUV420
	VDO_PXLFMT_YUV422_YUYV
	VDO_PXLFMT_YUV422_YVYU
	VDO_PXLFMT_YUV422_UYVY
	VDO_PXLFMT_YUV422_VYUY
	VDO_PXLFMT_YUV420_NVX2
*/

typedef struct {
	CTL_VPE_OUT_PATH_ID pid;	/* path id */
	BOOL enable;				/* output path enable */
	VDO_PXLFMT fmt;				/* ouptut image fmt */
	CTL_VPE_OUT_SCALE_METHOD_SEL scl_method;	/* scale method select */
	USIZE bg_size;		/* background size(buffer size) */
	USIZE scl_size;		/* scale out size, output to dst_pos */
	URECT pre_scl_crop;	/* crop before scale */
	URECT post_scl_crop;/* crop after scale */
	URECT out_window;	/* output window base on background */
	UPOINT dst_pos;		/* destination position base on out window */
	URECT hole_region;	/* hole region */
	UINT8 out_bg_sel;	/* out window bg color select, palette index  */
} CTL_VPE_OUT_PATH;

/* buffer info for out bufio_callback used */
typedef struct {
	ULONG user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	CTL_VPE_OUT_PATH_ID pid;
	UINT32 buf_size;
	UINT32 buf_id;
	CTL_VPE_BUF_ADDR buf_addr;
	VDO_FRAME vdo_frm;
	UINT32 lock_cnt;
	INT32 err_msg;
} CTL_VPE_OUT_BUF_INFO;

typedef struct {
	CTL_VPE_BUF_ADDR start_addr;
	UINT32 size;
} CTL_VPE_BUFCFG;

typedef struct {
	UINT32 buf_size;	/* return buffer size */
} CTL_VPE_PRIVATE_BUF;

/* vpe flush config for CTL_VPE_ITEM_FLUSH
	NULL -> flush all path
	else, flush pid in config
*/
typedef struct {
	CTL_VPE_OUT_PATH_ID pid;	/* path id */
} CTL_VPE_FLUSH_CONFIG;

/**
output path yuv buffer height alignment
will use vdoframe.reserve[0], [1]

vsp flow:
	VDO_FRAME.reserved[0] = MAKEFOURCC('H', 'A', 'L', 'N'); // height align
	VDO_FRAME.reserved[1] = y_height after align
	VDO_FRAME.reserved[2] = MAKEFOURCC('V', 'S', 'P', 'O');	//vsp main out, vsp only
  	  or 	              = MAKEFOURCC('B', 'L', 'D', 'I'); //blend out, vsp only
	VDO_FRAME.reserved[3] = frame 0~3 post_crp_ovlp_width, vsp only
	VDO_FRAME.reserved[4] = frame 4~5 post_crp_ovlp_width, vsp only
	VDO_FRAME.reserved[5] = frame 0~3 post_crp_mid_width, vsp only
	VDO_FRAME.reserved[6] = frame 4~5 post_crp_mid_width, vsp only
	VDO_FRAME.reserved[7] = bg width and frame number, vsp only
normal flow:
	VDO_FRAME.reserved[0] = MAKEFOURCC('H', 'A', 'L', 'N'); // height align
	VDO_FRAME.reserved[1] = y_height after align
	VDO_FRAME.reserved[2] =
	VDO_FRAME.reserved[3] =
	VDO_FRAME.reserved[4] =
	VDO_FRAME.reserved[5] =
	VDO_FRAME.reserved[6] =
	VDO_FRAME.reserved[7] =
*/
typedef struct {
	CTL_VPE_OUT_PATH_ID pid;	/* path id */
	UINT32 align;
} CTL_VPE_OUT_PATH_HALIGN;

/**
	vsp, video stiching process
*/
typedef struct {
	UINT8 enable;								/* vsp flow enable */
	UINT32 ovlp_width[CTL_VPE_VSP_FRM_NUM_MAX];	/*  */
	USIZE scale_out_size[CTL_VPE_VSP_FRM_NUM_MAX];	/*  */
	UINT32 dynamic_ofs_max[CTL_VPE_VSP_FRM_NUM_MAX];	/* N.S. */
} CTL_VPE_VSP_CONFIG;

typedef struct {
	UINT8 pal_y;
	UINT8 pal_cb;
	UINT8 pal_cr;
} CTL_VPE_PAL_COLOR;

/**
	palette color
*/
typedef struct {
	UINT32 palette_idx; 				//0~CTL_VPE_PALETTE_MAX-1
	CTL_VPE_PAL_COLOR palette_color;	//y/cb/cr color
} CTL_VPE_PALETTE_INFO;

typedef enum {
	CTL_VPE_MASK_INSIDE = 0,//valid inside roi win
	CTL_VPE_MASK_OUTSIDE,	//valid outside roi win
	CTL_VPE_MASK_BORDER,	//valid on roi win border
	CTL_VPE_MASK_MAX,
} CTL_VPE_MASK_AREA;

typedef enum {
	CTL_VPE_MASK_SHAPE_4 = 0,
	CTL_VPE_MASK_SHAPE_6,
	CTL_VPE_MASK_SHAPE_8,
	CTL_VPE_MASK_SHAPE_10,
	CTL_VPE_MASK_SHAPE_MAX
} CTL_VPE_MASK_SHAPE ;

#define CTL_VPE_MASK_PT_MAX 10
typedef struct {
	UINT8 en;				//0:disable, 1:enable
    UINT8 mosaic_en;		//0:original(use palette color) 1:mosaic
    UINT8 pal_sel;			//0~7. ref CTL_VPE_PALETTE_INFO
    UINT16 alpha;			//alpha 0 ~ 256, 0: 100%, 256:0%
    IPOINT roi_pt[CTL_VPE_MASK_PT_MAX];		//mask for corner point, roi_pt[4-9] valid when shape_6/8/10
	CTL_VPE_MASK_AREA area;					//0:inside, 1:outside, 2:border(border only valid at VPE_DRV_MASK_SHAPE_4)
	CTL_VPE_MASK_SHAPE shape; 				//valid at win idx[0/2/4/6]
} CTL_VPE_MASK_WIN;

typedef enum {
	CTL_VPE_MOSAIC_SZ_8 = 0,
	CTL_VPE_MOSAIC_SZ_16,
	CTL_VPE_MOSAIC_SZ_32,
	CTL_VPE_MOSAIC_SZ_64,
	CTL_VPE_MOSAIC_SZ_MAX,
} CTL_VPE_MOSAIC_BLK_SIZE;

typedef struct {
	CTL_VPE_MOSAIC_BLK_SIZE mosaic_blk_sz;
    CTL_VPE_MASK_WIN win[CTL_VPE_MASK_NUM_MAX];
} CTL_VPE_MASK_INFO;

/* vpe get/set item */
typedef enum {
	CTL_VPE_ITEM_REG_CB_IMM = 0,    /* Callback function register.      		SET-Only,   data_type: CTL_VPE_REG_CB_INFO          */
	CTL_VPE_ITEM_ALGID_IMM,         /* ALG ISP id                   			GET/SET,    data_type: UINT32                       */
	CTL_VPE_ITEM_FLUSH,             /* flush all buffer(in&out)     			SET-Only,   data_type: CTL_VPE_FLUSH_CONFIG or NULL */
	CTL_VPE_ITEM_APPLY,             /* Apply parameters to next trigger 		SET-Only,   data_type: NULL                         */
	CTL_VPE_ITEM_IN_CROP,           /* input crop config            			GET/SET,    data_type: CTL_VPE_IN_CROP              */
	CTL_VPE_ITEM_OUT_PATH,          /* output path config           			GET/SET,    data_type: CTL_VPE_OUT_PATH             */
	CTL_VPE_ITEM_PUSHEVT_INQ,       /* get push event number in que 			GET-Only,   data_type: UINT32                       */
	CTL_VPE_ITEM_PRIVATE_BUF,       /* get private buffer       				GET-Only,   data_type: CTL_VPE_PRIVATE_BUF         	*/
	CTL_VPE_ITEM_BUFCFG,            /* inner buffer configuration.  			SET-Only,   data_type: CTL_VPE_BUFCFG           	*/
	CTL_VPE_ITEM_OUT_PATH_HALIGN,   /* output path config           			GET/SET,    data_type: CTL_VPE_OUT_PATH_HALIGN      */
	CTL_VPE_ITEM_VSP_IMM,			/* (VPE only) vsp flow config       		GET/SET,    data_type: CTL_VPE_VSP_CONFIG           */
	CTL_VPE_ITEM_PALETTE,			/* palette info       						GET/SET,    data_type: CTL_VPE_PALETTE_INFO         */
	CTL_VPE_ITEM_MASK,				/* mask info       							GET/SET,    data_type: CTL_VPE_MASK_INFO  	        */
	CTL_VPE_ITEM_OUT_EXT_PATH,      /* output ext path config         			GET/SET,    data_type: CTL_VPE_OUT_EXT_PATH         */
	CTL_VPE_ITEM_MAX
} CTL_VPE_ITEM;

/* vpe ioctl item */
typedef enum {
	CTL_VPE_IOCTL_SNDEVT = 0,       /* Send event.          							data_type: CTL_VPE_EVT      */
	CTL_VPE_IOCTL_SNDEVT_MFRM,      /* Send event for multi frame(vsp flow), VPE only.  data_type: CTL_VPE_EVT_MFRM */
	CTL_VPE_IOCTL_MAX
} CTL_VPE_IOCTL;

UINT32 ctl_vpe_query(CTL_VPE_CTX_BUF_CFG ctx_buf_cfg);
INT32 ctl_vpe_init(CTL_VPE_CTX_BUF_CFG ctx_buf_cfg, CTL_VPE_BUF_ADDR buf_addr, UINT32 buf_size);
INT32 ctl_vpe_uninit(void);
ULONG ctl_vpe_open(CHAR *name, CTL_VPE_FLOW_TYPE flow);
INT32 ctl_vpe_close(ULONG hdl);
INT32 ctl_vpe_set(ULONG hdl, UINT32 item, void *data);
INT32 ctl_vpe_get(ULONG hdl, UINT32 item, void *data);
INT32 ctl_vpe_ioctl(ULONG hdl, UINT32 item, void *data);

void ctl_vpe_dump_all(int (*dump)(const char *fmt, ...));

#endif
