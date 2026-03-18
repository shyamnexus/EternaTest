#ifndef _IPP_DEBUG_INT_H_
#define _IPP_DEBUG_INT_H_
/**
    ipp_debug_int.h


    @file       ipp_debug_int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#include "kwrap/cpu.h"
#include "comm/ddr_arb.h"
#include "ctl_ipp_util.h"
#include "ctl_ipp_dbg.h"
#include "ctl_ipp_int.h"
#include "ctl_ipp_buf_int.h"
#include "kdrv_videoprocess/kdrv_ipp.h"

#if defined(__LINUX)
#define CTL_IPP_INT_ROOT_PATH "//mnt//sd//"
#define CTL_IPP_INT_PATH_SPLT "//"
#elif defined(__FREERTOS)
#define CTL_IPP_INT_ROOT_PATH "A:\\"
#define CTL_IPP_INT_PATH_SPLT "\\"
#endif

int ctl_ipp_int_printf(const char *fmtstr, ...);

/**
	time stamp log
*/
#define CTL_IPP_DBG_TS_LOG_NUM (50)	// max log number
#define CTL_IPP_DBG_TS_LOG_PRINT (16) 	// default print log number

typedef enum {
	CTL_IPP_DBG_TS_SND = 0,		/* hdal send event */
	CTL_IPP_DBG_TS_RCV,			/* ctl_ipp thread receive event */
	CTL_IPP_DBG_TS_PUTJOB,		/* ctl_ipp putjob to kdrv_ipp */
	CTL_IPP_DBG_TS_PROC_START,	/* kdrv_ipp thread process start */
	CTL_IPP_DBG_TS_PROC_END,	/* kdrv_ipp thread process end */
	CTL_IPP_DBG_TS_ENG_LOAD,	/* engine trigger/load */
	CTL_IPP_DBG_TS_ENG_START,	/* engine isr frame start */
	CTL_IPP_DBG_TS_ENG_END,		/* engine isr frame end */
	CTL_IPP_DBG_TS_JOB_END,		/* ctl_ipp buf thread callback to hdal push/release buffer */
	CTL_IPP_DBG_TS_SND_FPS,		/* send event fps */
	CTL_IPP_DBG_TS_PRO_FPS,		/* process end fps */
	CTL_IPP_DBG_TS_NN_PROC,		/* nnisp process time */
	CTL_IPP_DBG_TS_MAX
} CTL_IPP_DBG_TS_TYPE;

typedef struct _CTL_IPP_DBG_TS_NODE {
	CTL_IPP_HANDLE *p_hdl;
	UINT32 vd_cnt;
	UINT32 ts[CTL_IPP_DBG_TS_MAX];
	INT32 err_msg;
	CTL_IPP_LIST_HEAD list;
} CTL_IPP_DBG_TS_NODE;

UINT32 ctl_ipp_dbg_ts_pool_init(UINT32 num, CTL_IPP_BUF_ADDR buf_addr, UINT32 is_query);
INT32 ctl_ipp_dbg_ts_pool_uninit(void);
CTL_IPP_DBG_TS_NODE *ctl_ipp_dbg_get_ts_node(CTL_IPP_HANDLE *p_hdl, UINT32 vd_cnt);
void ctl_ipp_dbg_set_ts_node_ready(CTL_IPP_DBG_TS_NODE *p_node, INT32 err);
void ctl_ipp_dbg_set_ts(CTL_IPP_DBG_TS_NODE *p_node, UINT32 evt, UINT32 timestamp);
void ctl_ipp_dbg_set_ts_fps(CTL_IPP_INFO_LIST_ITEM *ctrl_info, UINT32 evt, UINT32 fps_evt);
void ctl_ipp_dbg_ts_dump(int (*dump)(const char *fmt, ...), BOOL print_all);

/**
	debug base info after handle close
*/
typedef struct {
	CHAR name[CTL_IPP_HANDLE_NAME_MAX];
	CTL_IPP_FLOW_TYPE flow;
	UINT32 ctl_snd_evt_cnt;		/* ctl send event count */
	UINT32 ctl_frm_str_cnt;		/* ctl ife frame start count */
	UINT32 ctl_proc_end_cnt;	/* ctl process end count */
	UINT32 ctl_drop_cnt;		/* ctl drop frame count, include ctl_drop, kdf_drop */
	UINT32 kdf_snd_evt_cnt;		/* kdf send event count */
	UINT32 kdf_frm_str_cnt;		/* kdf ife frame start count */
	UINT32 kdf_proc_end_cnt;	/* kdf process end count */
	UINT32 kdf_drop_cnt;		/* kdf drop frame count, include ctl_drop, kdf_drop */
	UINT32 kdf_reset_cnt;		/* kdf drop frame count, include ctl_drop, kdf_drop */
	UINT32 in_buf_re_cb_cnt;	/* direct mode input buffer release count */
	UINT32 in_buf_re_cnt;		/* direct mode input buffer release count */
	UINT32 in_buf_drop_cnt;		/* direct mode input buffer drop count */
	UINT32 in_pro_skip_cnt;		/* direct mode skip process cfg count */
} CTL_IPP_DBG_BASE_INFO;


/**
	handle debug information
*/
BOOL ctl_ipp_dbg_hdl_is_init(void);
void ctl_ipp_dbg_hdl_pool_set(CTL_IPP_HANDLE_POOL *pool);
void ctl_ipp_dbg_hdl_stripe_set(CTL_IPP_HANDLE *p_hdl, CTL_IPP_BASEINFO *p_base);
void ctl_ipp_dbg_hdl_dump_all(BOOL dump_free_head, int (*dump)(const char *fmt, ...));
void ctl_ipp_dbg_hdl_dump_buf(CTL_IPP_HANDLE *p_hdl, int (*dump)(const char *fmt, ...));
void ctl_ipp_dbg_hdl_set_dump_kdrv_job(UINT32 cnt);
void ctl_ipp_dbg_hdl_dump_kdrv_job(int (*dump)(const char *fmt, ...));
void ctl_ipp_dbg_hdl_dump_fboot_dtsi_info(int (*dump)(const char *fmt, ...));
void ctl_ipp_dbg_baseinfo_dump(CTL_IPP_BASEINFO *p_base_info, BOOL is_from_pool, int (*dump)(const char *fmt, ...));
void ctl_ipp_dbg_savefile(CHAR *f_name, ULONG addr, UINT32 size);
void ctl_ipp_dbg_saveyuv(CTL_IPP_HANDLE *p_hdl, CTL_IPP_OUT_BUF_INFO *p_buf);
void ctl_ipp_dbg_saveyuv_cfg(CHAR *handle_name, CHAR *filepath, UINT32 count, UINT8 bit);
void ctl_ipp_dbg_save_vdofrm_image(VDO_FRAME *p_vdofrm, CHAR *f_name);
void ctl_ipp_dbg_dump_dtsi_flow_begin(CTL_IPP_HANDLE *p_hdl, KDRV_IPP_TRIG_PARAM *p_trig_param);
void ctl_ipp_dbg_dump_dtsi(CHAR *f_name);
void ctl_ipp_dbg_dtsi_init(void);

/**
	debug common buffer infor
*/
#define CTL_IPP_DBG_BUF_LOG_NUM (50)	// max log number
#define CTL_IPP_DBG_BUF_LOG_PRINT (16) 	// default print log number

typedef struct {
	CTL_IPP_HANDLE *p_hdl;
	UINT32 pid;
	UINT32 type;
	UINT32 ts_start;
	UINT32 ts_end;
	ULONG buf_addr;
} CTL_IPP_DBG_BUF_CBTIME_LOG;

void ctl_ipp_dbg_outbuf_cbtime_dump(int (*dump)(const char *fmt, ...), BOOL print_all);
void ctl_ipp_dbg_outbuf_log_set(CTL_IPP_HANDLE *p_hdl, CTL_IPP_BUF_IO_CFG io_type, CTL_IPP_OUT_BUF_INFO *p_buf, UINT64 ts_start, UINT64 ts_end);
void ctl_ipp_dbg_inbuf_log_set(CTL_IPP_HANDLE *p_hdl, ULONG buf_addr, CTL_IPP_CBEVT_IN_BUF_MSG type, UINT64 ts_start, UINT64 ts_end);
void ctl_ipp_dbg_inbuf_cbtime_dump(int (*dump)(const char *fmt, ...), BOOL print_all);

/**
	debug current used output buffer usage
*/
#define CTL_IPP_DBG_BUF_USED_NUM (25)	// max log number
#define CTL_IPP_DBG_BUF_USED_MAX_NUM (2)
#define CTL_IPP_DBG_BUF_USED_TYPE_NUM (8)

typedef enum {
	CTL_IPP_DBG_BUF_USED_UNKNOWN = 0,
	CTL_IPP_DBG_BUF_USED_NEW_BEGIN,
	CTL_IPP_DBG_BUF_USED_NEW_END,
	CTL_IPP_DBG_BUF_USED_PUSH_BEGIN,
	CTL_IPP_DBG_BUF_USED_PUSH_END,
	CTL_IPP_DBG_BUF_USED_LOCK_BEGIN,
	CTL_IPP_DBG_BUF_USED_LOCK_END,
	CTL_IPP_DBG_BUF_USED_UNLOCK_BEGIN,
	CTL_IPP_DBG_BUF_USED_UNLOCK_END,
	CTL_IPP_DBG_BUF_USED_MAX,

	CTL_IPP_DBG_BUF_USED_SKIP,
} CTL_IPP_DBG_BUF_USED_TYPE;

typedef struct {
	UINT8 used;
	CTL_IPP_HANDLE *p_hdl;
	CTL_IPP_INFO_LIST_ITEM *ctrl_info;
	CHAR name[CTL_IPP_HANDLE_NAME_MAX];
	UINT8 pid;
	UINT8 type[CTL_IPP_DBG_BUF_USED_TYPE_NUM];
	UINT32 ts_start;
	UINT8 loc_cnt;
	ULONG buf_addr;
	UINT32 frm_cnt;
} CTL_IPP_DBG_BUF_USED_LOG;

void ctl_ipp_dbg_outbuf_used_set(CTL_IPP_HANDLE *p_hdl, CTL_IPP_INFO_LIST_ITEM *ctrl_info, CTL_IPP_DBG_BUF_USED_TYPE type, CTL_IPP_OUT_BUF_INFO *p_buf, UINT32 ts);
void ctl_ipp_dbg_outbuf_used_dump(int (*dump)(const char *fmt, ...));

/**
	debug context buffer log
*/
#define CTL_IPP_DBG_CTX_BUF_NAME_MAX (16)

typedef enum {
	CTL_IPP_DBG_CTX_BUF_QUERY = 0,
	CTL_IPP_DBG_CTX_BUF_ALLOC,
	CTL_IPP_DBG_CTX_BUF_FREE,
	CTL_IPP_DBG_CTX_BUF_CNN_SRAM_EN,
	CTL_IPP_DBG_CTX_BUF_OP_MAX
} CTL_IPP_DBG_CTX_BUF_OP;

typedef struct {
	CHAR name[CTL_IPP_DBG_CTX_BUF_NAME_MAX];
	CTL_IPP_DBG_CTX_BUF_OP op;
	UINT32 size;
	CTL_IPP_BUF_ADDR addr;
	UINT32 cfg_num;
} CTL_IPP_DBG_CTX_BUF_LOG;

void ctl_ipp_dbg_ctxbuf_log_set(CHAR *name, CTL_IPP_DBG_CTX_BUF_OP op, UINT32 size, CTL_IPP_BUF_ADDR addr, UINT32 n);
void ctl_ipp_dbg_ctxbuf_log_dump(int (*dump)(const char *fmt, ...));


/**
	debug isr sequence in direct mode
*/
#define CTL_IPP_DBG_DIRECT_ISR_ENABLE (0)
#define CTL_IPP_DBG_DIRECT_ISR_NUM 32
#define CTL_IPP_DBG_DIR_ISR_NAME_MAX	(16)

typedef struct {
	CHAR name[CTL_IPP_DBG_DIR_ISR_NAME_MAX];
	UINT32 cnt;
	ULONG addr;
} CTL_IPP_DBG_DIR_ISR_SEQ;


void ctl_ipp_dbg_dump_direct_isr_sequence(CTL_IPP_HANDLE *p_hdl);
void ctl_ipp_dbg_set_direct_isr_sequence(CTL_IPP_HANDLE *p_hdl, CHAR* name, ULONG addr);

/**
	debug privcay mask
*/
#define CTL_IPP_DBG_PRIMASK_COLOR_POOL (8)

typedef struct {
	UINT8 enable;
	UINT8 color[CTL_IPP_DBG_PRIMASK_COLOR_POOL][3];
	USIZE msk_size;
} CTL_IPP_DBG_PRIMASK_CTL;

typedef enum {
	CTL_IPP_DBG_PRIMASK_CFG_SIZE = 2,	/* data_type: USIZE */
	CTL_IPP_DBG_PRIMASK_CFG_MAX,
} CTL_IPP_DBG_PRIMASK_CFG;

INT32 ctl_ipp_dbg_primask_cb(UINT32 msg, void *in, void *out);
void ctl_ipp_dbg_primask_en(UINT32 en);
void ctl_ipp_dbg_primask_cfg(UINT32 cfg, void *data);

// dmawp buf_type shift bit
typedef enum {
	CTL_IPP_DBG_DMA_WP_BUF_IFE_IN0_SFT = 0,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_IN1_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_IN2_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_IN_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_IN_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_REF_IN_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBIN_WDR_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_DEFOG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_LCE_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBIN_PM_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA_DBG_SFT,
	CTL_IPP_DBG_DMA_WP_BUF_SFT_MAX,
} CTL_IPP_DBG_DMA_WP_BUF_SFT;

#define CTL_IPP_DBG_DMA_WP_BUF_DISABLE 					0
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_IN0 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_IN0_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_IN1 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_IN1_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_IN2 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_IN2_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_IN 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_IN_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_IN 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_IN_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_REF_IN				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_REF_IN_SFT)

#define CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBIN_WDR 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBIN_WDR_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_DEFOG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_DEFOG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_LCE 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBIN_LCE_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBIN_PM 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBIN_PM_SFT)

#define CTL_IPP_DBG_DMA_WP_BUF_IPE_OUT 					((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT 				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT 				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT 				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT 				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT 				((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT_DBG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P1_OUT_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT_DBG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P2_OUT_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT_DBG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P3_OUT_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT_DBG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P4_OUT_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT_DBG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_P5_OUT_DBG_SFT)

#define CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR_DBG		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_WDR_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IFE_SUBOUT_VA_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG_DBG 	((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_DEFOG_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_LCE_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_VA_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA_DBG		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IPE_SUBOUT_CFA_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_VA_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MV_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI_DBG 	((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_MS_ROI_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_FCVG_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM 			((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM_DBG 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_PM_DBG_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA 		((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA_SFT)
#define CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA_DBG 	((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_IME_SUBOUT_3DNR_STA_DBG_SFT)

#define CTL_IPP_DBG_DMA_WP_BUF_ALL						(((CTL_IPP_DBG_DMA_WP_BUF_TYPE)1 << CTL_IPP_DBG_DMA_WP_BUF_SFT_MAX) - 1)
typedef UINT64 CTL_IPP_DBG_DMA_WP_BUF_TYPE;	// bit field define by CTL_IPP_DBG_DMA_WP_BUF_XXX (see above)

#define CTL_IPP_DBG_DMA_WP_ALIGN 256 		// 530 dma wp address and size need align 64 words (256 bytes)
#define CTL_IPP_DBG_DMA_WP_EXB_SZ 512		// dma wp allocate extra buffer size. set to 2 times of CTL_IPP_DBG_DMA_WP_ALIGN to prevent buf size = 0 after alignment

typedef enum {
	CTL_IPP_DBG_DMA_WP_LEVEL_UNWRITE = DMA_WPLEL_UNWRITE,
	CTL_IPP_DBG_DMA_WP_LEVEL_DETECT = DMA_WPLEL_DETECT,
	CTL_IPP_DBG_DMA_WP_LEVEL_UNREAD = DMA_RPLEL_UNREAD,
	CTL_IPP_DBG_DMA_WP_LEVEL_UNRW = DMA_RWPLEL_UNRW,
	CTL_IPP_DBG_DMA_WP_LEVEL_AUTO = 0xFF, // set protect level by different channel
} CTL_IPP_DBG_DMA_WP_LEVEL;

typedef struct {
	BOOL one_buf_mode_en; // one buffer mode share yuv buffer with codec
	CTL_IPP_FLOW_TYPE flow; // PIXELIZTION and 3DNR subin/subout in direct mode reference same buffer
	UINT8 low_delay_enable;
	UINT8 low_delay_path;
	UINT8 slice_enable;
	UINT8 slice_path_bit;
} CTL_IPP_DBG_DMA_WP_BUF_EXT;

ER ctl_ipp_dbg_dma_wp_set_enable(BOOL en, CTL_IPP_DBG_DMA_WP_BUF_TYPE type);
CTL_IPP_DBG_DMA_WP_BUF_TYPE ctl_ipp_dbg_dma_wp_get_enable(void);
void ctl_ipp_dbg_dma_wp_set_exb_enable(BOOL en);
BOOL ctl_ipp_dbg_dma_wp_get_exb_enable(void);
ER ctl_ipp_dbg_dma_wp_set_prot_level(CTL_IPP_DBG_DMA_WP_LEVEL level);
ER ctl_ipp_dbg_dma_wp_start(CTL_IPP_DBG_DMA_WP_BUF_TYPE type, CTL_IPP_BUF_ADDR addr, UINT32 size, CTL_IPP_DBG_DMA_WP_BUF_EXT *ext_data);
ER ctl_ipp_dbg_dma_wp_end(CTL_IPP_DBG_DMA_WP_BUF_TYPE type, CTL_IPP_BUF_ADDR addr);
ER ctl_ipp_dbg_dma_wp_proc_start(CTL_IPP_INFO_LIST_ITEM *ctrl_info);
ER ctl_ipp_dbg_dma_wp_proc_end(CTL_IPP_INFO_LIST_ITEM *ctrl_info, BOOL update_ref);
ER ctl_ipp_dbg_dma_wp_proc_end_ref(CTL_IPP_INFO_LIST_ITEM *last_ctrl_info);
ER ctl_ipp_dbg_dma_wp_proc_end_push(CTL_IPP_INFO_LIST_ITEM *ctrl_info, BOOL update_ref);
ER ctl_ipp_dbg_dma_wp_task_start(CTRL_IPP_BUF_ITEM item, CTL_IPP_BUF_ADDR addr, UINT32 size);
ER ctl_ipp_dbg_dma_wp_task_end(CTRL_IPP_BUF_ITEM item, CTL_IPP_BUF_ADDR addr);
ER ctl_ipp_dbg_dma_wp_drop(CTL_IPP_INFO_LIST_ITEM *ctrl_info);
ER ctl_ipp_dbg_dma_wp_clear(void);
void ctl_ipp_dbg_dma_wp_dump(int (*dump)(const char *fmt, ...));

#endif //_IPP_DEBUG_INT_H_
