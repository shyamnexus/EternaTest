/**
    DRE_Ctrl Layer

    @file       ctl_dre.h
    @ingroup    mIDRE_Ctrl
    @note

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifndef _CTL_DRE_H_
#define _CTL_DRE_H_

#include "kwrap/error_no.h"
#include "kflow_common/type_vdo.h"
#include "kdrv_videoprocess/dre/kdrv_dre_lmt.h"

#if defined(__aarch64__)
#define CTL_DRE_LMT_MAX_VSP_FRM_NUM		8	//maximum support vsp frame number, sw limitation, modify for clamp max frame number
#else	//32bit only support 4 frame vsp
#define CTL_DRE_LMT_MAX_VSP_FRM_NUM		4	//maximum support vsp frame number, sw limitation, modify for clamp max frame number
#endif
#define CTL_DRE_LMT_MAX_BLEND_FRM_NUM	2	//max dre blending function support frame number, HW limitation, don't modify this value
#define CTL_DRE_LMT_MAX_LAYER_NUM	    10
/* limit */

/* dre callback fp type */
typedef INT32 (*CTL_DRE_EVENT_FP)(UINT32 event, void *p_in, void *p_out);

/* dre error type */
#define CTL_DRE_E_OK				(E_OK)
#define CTL_DRE_E_SYS				(E_SYS)
#define CTL_DRE_E_PAR				(E_PAR)		///< parameters error
#define CTL_DRE_E_NOMEM				(E_NOMEM)	///< no memory error
#define CTL_DRE_E_NOSPT				(E_NOSPT)	///< not support error
#define CTL_DRE_E_QOVR				(E_QOVR)	///< queue overflow error
#define CTL_DRE_E_ID				(E_ID)		///< id error
#define CTL_DRE_E_INDATA			(-101)		///< input data error
#define CTL_DRE_E_FLUSH				(-102)		///< flush error
#define CTL_DRE_E_DROP_INPUT_ONLY	(-103)		///<
#define CTL_DRE_E_OUTDATA			(-104)		///< output data error
#define CTL_DRE_E_NULL				(-105)		///< null pointer error
#define CTL_DRE_E_STATE				(-106)		///< state error
#define CTL_DRE_E_KDRV_OPEN			(-200)		///< kdrv open error
#define CTL_DRE_E_KDRV_CLOSE		(-201)		///< kdrv close error
#define CTL_DRE_E_KDRV_SET			(-202)		///< kdrv set error
#define CTL_DRE_E_KDRV_GET			(-203)		///< kdrv get error
#define CTL_DRE_E_KDRV_TRIG			(-204)		///< kdrv trigger error
#define CTL_DRE_E_KDRV_STRP			(-205)		///< kdrv stripe cal error
#define CTL_DRE_E_KDRV_DROP			(-206)		///< kdrv stripe cal error

/* process type */
typedef enum {
	CTL_DRE_OP_MODE_SIMP = 0,	//Simplified process, smaller working buf and normal quality
	CTL_DRE_OP_MODE_COMP,		//Complete process, larger working buf and better quality
	CTL_DRE_OP_MODE_MAX,
} CTL_DRE_OP_MODE;

typedef struct{
	ULONG va;	//virtual address
	ULONG pa;	//physical address
}CTL_DRE_BUF_ADDR;

typedef struct{
	CTL_DRE_BUF_ADDR addr;
	UINT32 lofs;
}CTL_DRE_FUSION_WT_IN;

typedef struct {
	USIZE max_img_size;			//for working buf
	CTL_DRE_OP_MODE op_mode;	//for working buf
} CTL_DRE_FUNC_CFG;

/* init/query config */
typedef struct {
	UINT32 handle_num;			/* number of handles */
	UINT32 queue_num;			/* number of queue nodes */
	CTL_DRE_FUNC_CFG func_cfg;	/* function cfg for buffer query */
} CTL_DRE_CTX_BUF_CFG;


/* dre callback event type */
typedef enum {
	CTL_DRE_CBEVT_IN_BUF = 0,
	CTL_DRE_CBEVT_OUT_BUF,
	CTL_DRE_CBEVT_MAX,
} CTL_DRE_CBEVT_TYPE;

typedef struct {
	CTL_DRE_CBEVT_TYPE cbevt;	/* callback event */
	CTL_DRE_EVENT_FP fp;
} CTL_DRE_REG_CB_INFO;

/* buffer io operation for both in/out buf callback */
typedef enum {
	CTL_DRE_BUF_NEW = 0,	/* new buffer */
	CTL_DRE_BUF_PUSH,		/* push ready buffer */
	CTL_DRE_BUF_LOCK,		/* lock buffer */
	CTL_DRE_BUF_UNLOCK,		/* unlock buffer */
	CTL_DRE_BUF_IO_MAX,
} CTL_DRE_BUF_IO;


/* in bufio callback event */
typedef struct {
	ULONG 	user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32	buf_id;		/* buffer id */
	ULONG	data_addr;	/* buffer address */
	UINT32	rev;
	INT32   err_msg;
} CTL_DRE_EVT;

/* blending snd event information */
typedef struct {
	ULONG 	user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32	buf_id[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];		/* buffer id */
	ULONG	data_addr[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];	/* vdo_frm address */
	UINT32	rev;
	INT32   err_msg;
} CTL_DRE_EVT_BLEND;

typedef struct {
	ULONG 	user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32	buf_id;		/* buffer id */
	ULONG	data_addr;	/* vdo_frm address */
	UINT32	rev;
	INT32   err_msg;
} CTL_DRE_EVT_VSP;

/* in crop config */
typedef enum {
	CTL_DRE_IN_CROP_AUTO = 0,	/* reference input header information */
	CTL_DRE_IN_CROP_NONE,		/* no crop, process full image */
	CTL_DRE_IN_CROP_USER,		/* user define crop window */
	CTL_DRE_IN_CROP_MODE_MAX,
} CTL_DRE_IN_CROP_MODE;

typedef struct {
	CTL_DRE_IN_CROP_MODE mode[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];	/* input crop mode select */
	URECT crp_window[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];			/* only effect when CTL_DRE_IN_CROP_USER */
} CTL_DRE_IN_CROP;

/* buffer info for out bufio_callback used */
typedef struct {
	ULONG user_data;	/* for hdal using in vsp flow, kflow just pass to output vdo_frm */
	UINT32 buf_size;	/* buffer size(for new buffer using) */
	UINT32 buf_id;		/* buffer id */
	CTL_DRE_BUF_ADDR buf_addr;	/* buffer address */
	VDO_FRAME vdo_frm;
	UINT32 lock_cnt;
	INT32 err_msg;
} CTL_DRE_OUT_BUF_INFO;

typedef struct {
	CTL_DRE_BUF_ADDR addr;
	UINT32 size;
} CTL_DRE_BUFCFG;

typedef struct {
	USIZE  bg_size;		/* background size(buffer size) */
	UPOINT dst_pos;		/* destination position base on background */
} CTL_DRE_OUT_PATH;

typedef struct {
	UINT32 buf_size;			/* return buffer size */
} CTL_DRE_PRIVATE_BUF;

/* dre flush config for CTL_DRE_ITEM_FLUSH
	NULL -> flush all path
	else, flush pid in config
*/
typedef struct {
	UINT32 reserved;
} CTL_DRE_FLUSH_CONFIG;

/**
	dre process mode
*/
typedef enum {
	CTL_DRE_PROC_FUSION_SIMP = 0,  	/* Simplified process, less working buf and normal quality */
	CTL_DRE_PROC_FUSION_COMP,		/* Complete process, more working buf and better quality */
	CTL_DRE_PROC_MSNR_SIMP,			/* Simplified process, less working buf and normal quality */
	CTL_DRE_PROC_MSNR_COMP,			/* Complete process, more working buf and better quality */
	CTL_DRE_PROC_MODE_MAX,
} CTL_DRE_PROC_MODE;

/**
	vsp, video stiching process
*/
typedef struct {
	UINT8 enable;
} CTL_DRE_VSP_CONFIG;

/**
	weight table
*/
typedef struct {
	CTL_DRE_FUSION_WT_IN fusion_wt_in[CTL_DRE_LMT_MAX_VSP_FRM_NUM][CTL_DRE_LMT_MAX_LAYER_NUM];
} CTL_DRE_WT_CONFIG;

typedef struct {
	CTL_DRE_BUF_ADDR addr[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];	/* pixel mask input address */
	UINT32 lofs[CTL_DRE_LMT_MAX_BLEND_FRM_NUM];				/* should be 4-bytes align */
} CTL_DRE_PIXEL_MASK;

/* dre get/set item */
typedef enum {
	CTL_DRE_ITEM_REG_CB_IMM = 0,    /* Callback function register.      SET-Only,   data_type: CTL_DRE_REG_CB_INFO          */
	CTL_DRE_ITEM_ALGID_IMM,         /* dre ALG ISP id                   GET/SET,    data_type: UINT32                       */
	CTL_DRE_ITEM_FLUSH,             /* dre flush all buffer(in&out)     SET-Only,   data_type: CTL_DRE_FLUSH_CONFIG or NULL */
	CTL_DRE_ITEM_IN_CROP,           /* input crop config           		GET/SET,    data_type: CTL_DRE_IN_CROP              */
	CTL_DRE_ITEM_PROC_MODE,     	/* dre process mode configuration.  GET/SET,   	data_type: CTL_DRE_PROC_MODE        	*/
	CTL_DRE_ITEM_PROC_QUALITY_LVL,  /* dre proc quality level. 			GET/SET,   	data_type: UINT32 min:0, max:6 			*/
	CTL_DRE_ITEM_PUSHEVT_INQ,       /* dre get push event number in que GET-Only,   data_type: UINT32                       */
	CTL_DRE_ITEM_PRIVATE_BUF,       /* dre get private buffer       	GET-Only,   data_type: CTL_DRE_PRIVATE_BUF         	*/
	CTL_DRE_ITEM_BUFCFG,            /* dre inner buffer configuration.  SET-Only,   data_type: CTL_DRE_BUFCFG           	*/
	CTL_DRE_ITEM_OUT_PATH,			/* output path config           	GET/SET,    data_type: CTL_DRE_OUT_PATH             */
	CTL_DRE_ITEM_VSP_IMM,			/* dre vsp flow config              GET/SET,    data_type: CTL_DRE_VSP_CONFIG           */
	CTL_DRE_ITEM_WT_IMM,			/* blending weighting table         SET-Only,   data_type: CTL_DRE_WT_CONFIG            */
	CTL_DRE_ITEM_PIXEL_MASK,		/* pixel mask parameters,   		GET/SET,    data_type: CTL_DRE_PIXEL_MASK           */
	CTL_DRE_ITEM_APPLY,             /* Apply parameters to next trigger SET-Only,   data_type: NULL	                        */
	CTL_DRE_ITEM_MAX
} CTL_DRE_ITEM;

/* dre ioctl item */
typedef enum {
	CTL_DRE_IOCTL_SNDEVT_BLEND = 0, /* Send event.     			data_type: CTL_DRE_EVT_BLEND */
	CTL_DRE_IOCTL_SNDEVT_VSP,      	/* Send event for vsp flow. data_type: CTL_DRE_EVT_VSP */
	CTL_DRE_IOCTL_MAX
} CTL_DRE_IOCTL;

UINT32 ctl_dre_query(CTL_DRE_CTX_BUF_CFG ctx_buf_cfg);
INT32 ctl_dre_init(CTL_DRE_CTX_BUF_CFG ctx_buf_cfg, CTL_DRE_BUF_ADDR addr, UINT32 buf_size);
INT32 ctl_dre_uninit(void);
INT32 ctl_dre_alloc_working_buf(CTL_DRE_BUF_ADDR addr, UINT32 buf_size);
ULONG ctl_dre_open(CHAR *name);
INT32 ctl_dre_close(ULONG hdl);
INT32 ctl_dre_set(ULONG hdl, UINT32 item, void *data);
INT32 ctl_dre_get(ULONG hdl, UINT32 item, void *data);
INT32 ctl_dre_ioctl(ULONG hdl, UINT32 item, void *data);

void ctl_dre_dump_all(int (*dump)(const char *fmt, ...));

#endif
