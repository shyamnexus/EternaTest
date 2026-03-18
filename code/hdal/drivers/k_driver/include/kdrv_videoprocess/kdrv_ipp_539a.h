/**
    Public header file for KDRV_IPP

    This file is the header file that define the API and data type for KDRV_IPP.

    @file       kdrv_ipp.h
    @ingroup    mILibIPLCom
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#ifndef _KDRV_IPP_H_
#define _KDRV_IPP_H_

#include "kdrv_videoprocess/kdrv_pre_539a.h"
#include "kdrv_videoprocess/kdrv_ipe_539a.h"
#include "kdrv_videoprocess/kdrv_ime_539a.h"
#include "kdrv_videoprocess/kdrv_ife_539a.h"
#include "kdrv_videoprocess/kdrv_dce_539a.h"
#include "kdrv_type.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"

#define KDRV_IPP_CHIP_NUM	(1)
#define KDRV_IPP_ENG_NUM	(1)//538 modified
#define KDRV_IPP_CH_NUM		(1)
#define KDRV_IPP_BIND_QUEUE_NUM (3)  // 1.IFE2IME 2.IME_D2D 3. PRE
#define KDRV_IPP_TOTAL_HDL_NUM ( KDRV_IPP_CHIP_NUM * KDRV_IPP_ENG_NUM * KDRV_IPP_BIND_QUEUE_NUM)

#define KDRV_IPP_PROC_MODE_TAG		(0x5A5A0000)
#define KDRV_IPP_PROC_MODE_LINKLIST	(KDRV_IPP_PROC_MODE_TAG | 1)
#define KDRV_IPP_PROC_MODE_CPU		(KDRV_IPP_PROC_MODE_TAG | 2)

#define KDRV_IPP_OWNER_NAME_MAX		(16)
#define KDRV_IPP_MAX_STP_NUM	    (8)
#define KDRV_IPP_NN_ISP_NN_PATH_MAX (4)

#define KDRV_AI_TEST (0)
#define KDRV_IPP_AI_LOG_EN (0)
#define KDRV_IPP_STRIPE_LOG_EN (0)
#define KDRV_IPP_AI_DROP (0)

#define IPP_538_KLOFW_SUPPORT (0)
#define KDRV_IPP_NN_ISP_RING_BUF_NUM (3)

#define KDRV_IPP_E_START_DROP (-100) //for 567 two direct 2nd trig start

typedef enum{
  KDRV_IPP_NNISP_FMT_Y8 = 0,
  KDRV_IPP_NNISP_FMT_BAYER,
  KDRV_IPP_NNISP_FMT_YUV,
  KDRV_IPP_NNISP_FMT_UNKNOWN,
  ENUM_DUMMY4WORD(KDRV_IPP_NNISP_FMT)
} KDRV_IPP_NNISP_FMT;

typedef struct _KDRV_AI_JMISP_FUNC_INFO {
	UINT64 frame_y_addr;
	UINT64 frame_uv_addr;
	UINT64 frame_texture_addr;
	UINT64 out_pingpong_y_addr;
	UINT64 out_pingpong_uv_addr;
	UINT64 out_pingpong_texture_addr;
	UINT64 in_ring_y_start_addr;
	UINT64 in_ring_y_end_addr;
	UINT64 in_ring_uv_start_addr;
	UINT64 in_ring_uv_end_addr;
	UINT32 max_out_lofs;
	UINT32 max_out_height;
	UINT32 isp_id;
	UINT32 stripe_num;
	UINT32 slice_num;
	UINT32 signal_mode_en;
	UINT32 path_id;
	KDRV_IPP_NNISP_FMT img_fmt;
} KDRV_IPP_JMISP_FUNC_INFO;

typedef struct _KDRV_IPP_JMISP_JOB_INFO {
	KDRV_IPP_JMISP_FUNC_INFO func_info[KDRV_IPP_NN_ISP_NN_PATH_MAX];
	UINT32 func_num;
} KDRV_IPP_JMISP_JOB_INFO;

typedef struct{
	ULONG frame_y_addr;
	ULONG frame_uv_addr;
	ULONG frame_texture_addr;
	ULONG pingpong_y_addr;
	ULONG pingpong_uv_addr;
	ULONG pingpong_texture_addr;
	ULONG ring_y_start_addr;
	ULONG ring_y_end_addr;
	ULONG ring_uv_start_addr;
	ULONG ring_uv_end_addr;
}KDRV_IPP_NN_ISP_UBUF_LAYOUT_INFO;

typedef struct{
  UINT32 max_stripe;
  UINT32 min_stripe;
}KDRV_IPP_NN_ISP_NN_STRP;


typedef struct{
  UINT32 max_stripe;
  UINT32 min_stripe;
  UINT32 slice_overlap;
  UINT32 slice_height;
  UINT8  sw_handshake_en;
}KDRV_IPP_NN_ISP_NN_INFO;

typedef struct{
  BOOL   en;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 isp_id;
  UINT32 path_id;
}KDRV_IPP_NN_ISP_IN_PARAM;

typedef struct{
  UINT32 strp_width[KDRV_IPP_MAX_STP_NUM];
  UINT32 strp_num;
  UINT32 strp_overlap;
  UINT32 width;
  UINT32 height;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
}KDRV_IPP_NN_ISP_SET_RES_PARAM;

typedef struct{
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
  UINT32 width;
  UINT32 height;
}KDRV_IPP_NN_ISP_IPP_SLICE_JOB;

typedef struct{
  UINT32 ipp_id;
  UINT32 path_id;
  BOOL   frm_ed_flg;
  UINT32 frm_start_time;
  UINT32 frm_end_time;
}KDRV_IPP_NN_ISP_SLICE_DONE_PARAM;

typedef enum {
	KDRV_IPP_NN_ISP_AI_TO_IPP_OP_SLICE_DONE    = 0, // call when jmisp slice done, input NN_ISP_SLICE_DONE_PARAM, out NULL
	KDRV_IPP_NN_ISP_AI_TO_IPP_OP_UNKNOWN,
} KDRV_IPP_NN_ISP_AI_TO_IPP_OPERATION;

typedef enum {
	KDRV_IPP_NN_ISP_OP_GET_NN_INFO     = 0, // get slice_height, slice_overlap // call every frame, input NN_ISP_IN_PARAM,   output NN_ISP_NN_INFO
	KDRV_IPP_NN_ISP_OP_GET_UBUF_LAYOUT = 1, // get UBUF layout                 // call every frame, input NN_ISP_IN_PARAM,   output NN_ISP_PATH_BUF_INFO
	KDRV_IPP_NN_ISP_OP_GET_MAX_STRP    = 2, // get NN max stripe width         // call when enable, input NN_ISP_IN_PARAM,   output UINT32 max_strp
	KDRV_IPP_NN_ISP_OP_CAL_SLICE       = 3, // set NN to calculate slice info  // call after NN_ISP_OP_GET_MAX_STRP(enable), input NN_ISP_SET_RES_PARAM, output NN_ISP_NN_INFO
	KDRV_IPP_NN_ISP_OP_CHANGE_RES      = 4, // notify AI to chagne res         // reserve
	KDRV_IPP_NN_ISP_OP_UPDATE          = 5, // notify AI for event             // call when update event(disable), input NN_ISP_IN_PARAM
	KDRV_IPP_NN_ISP_OP_PUSH_NN_JOB     = 6, // push nn_job                     // call every frame, input NN_ISP_IPP_NN_JOB
	KDRV_IPP_NN_ISP_OP_PUSH_SLICE_JOB  = 7, // push slice_job                  // call every slice, input NN_ISP_IPP_SLICE_JOB, output NULL
	KDRV_IPP_NN_ISP_OP_QUERY_AI_STATUS = 8, // query ai open/start status      // call before open/start
	KDRV_IPP_NN_ISP_OP_UNKNOWN,
} KDRV_IPP_NN_ISP_OPERATION;

typedef enum {
	KDRV_IPP_STRP_RULE_AI_ISP_MST_PRIOR = 0,
	KDRV_IPP_STRP_RULE_AI_ISP_SST_3840_PRIOR = 1,
	KDRV_IPP_STRP_RULE_CODEC_LOW_LATENCY_PRIOR = 2,
	KDRV_IPP_STRP_RULE_UNKNOWN,
} KDRV_IPP_STRP_RULE;

typedef enum{
	KDRV_IPP_NONE_BIND = 0,
	KDRV_IPP_DIRECT_BIND,
	KDRV_IPP_D2D_BIND
} KDRV_IPP_BIND_TYPE;

typedef struct{
	KDRV_IPP_BIND_TYPE bind_type;
	KDRV_IPP_BIND_MODE bind_mode;
	UINT32 vprc_id;
} KDRV_IPP_CREATE_NODE_PARAM;

typedef struct{
	ULONG job_owner;
} KDRV_IPP_FLUSH_ALL_PARAM;

typedef enum {
	KDRV_IPP_CALLBACK_PRESET = 0,	/* last config timing befor trigger, for buffer alloc, p_info = KDRV_IPP_CB_PRESET_INFO */
	KDRV_IPP_CALLBACK_JOBEND,		/* job end, p_info = KDRV_IPP_CB_JOB_INFO */
	KDRV_IPP_CALLBACK_JOBDROP,		/* job drop, only call at flush flow, p_info = KDRV_IPP_CB_JOB_INFO */
	KDRV_IPP_CALLBACK_JOBSTART,		/* job start, p_info = KDRV_IPP_CB_JOB_INFO */
	KDRV_IPP_CALLBACK_ISR,			/* engine isr callback, p_info = KDRV_IPP_CB_ISR_INFO */
	KDRV_IPP_CALLBACK_OOPS,			/* something wrong, dump information for debug, p_info = KDRV_IPP_CB_JOB_INFO */
	KDRV_IPP_CALLBACK_FREEJOB,		/* notify kdrv job free, p_info = NULL */
	KDRV_IPP_CALLBACK_BUILTIN_EXIT,	/* call at kdrv_ipp task. hdal need set CTL_IPP_CBEVT_BUILTIN_EXIT event */
} KDRV_IPP_CALLBACK_EVT;

typedef enum {
	KDRV_IPP_TRIG_START = 0,	/* mode, start.					data_type: TBD				*/
	KDRV_IPP_TRIG_LOAD,			/* mode, load.					data_type: TBD				*/
	KDRV_IPP_TRIG_STOP,			/* mode, stop.					data_type: TBD				*/
	KDRV_IPP_TRIG_MAX
} KDRV_IPP_OP_MODE;

typedef enum {
	KDRV_IPP_JOB_TS_GENNODE = 0,/* gen node */
	KDRV_IPP_JOB_TS_PUTJOB,		/* put job in queue */
	KDRV_IPP_JOB_TS_PROC_START,	/* kdrv process job start */
	KDRV_IPP_JOB_TS_PROC_END,	/* kdrv process job end */
	KDRV_IPP_JOB_TS_ENG_LOAD,	/* engine trigger/load */
	KDRV_IPP_JOB_TS_ENG_START,	/* engine isr frame start */
	KDRV_IPP_JOB_TS_ENG_END,	/* engine isr frame end */
	KDRV_IPP_JOB_TS_JOB_END,	/* callback to kflow */
	KDRV_IPP_JOB_TS_NN_START,	/* callback to kflow */
	KDRV_IPP_JOB_TS_NN_END,		/* callback to kflow */
	KDRV_IPP_JOB_TS_MAX
} KDRV_IPP_JOB_TS;

typedef enum {
	KDRV_IPP_STATUS_START = 0,	/* status:start.					data_type: TBD				*/
	KDRV_IPP_STATUS_STOP,		/* status:stop.						data_type: TBD				*/
	KDRV_IPP_STATUS_STOP_END,	/* status:stop_end.					data_type: TBD				*/
	KDRV_IPP_STATUS_MAX
} KDRV_IPP_STATUS;

typedef struct {
	UINT32 job_head_num;
	UINT32 job_cfg_num;
	UINT32 ll_blk_num;
	UINT32 reserved;
	UINT32 cnn_sram_en;
} KDRV_IPP_CTX_BUF_CFG;

typedef struct {
	KDRV_IPP_STRP_RULE strp_rule;
} KDRV_IPP_IO_CFG;

typedef struct {
	KDRV_IPP_IO_CFG *p_iocfg;	// [set][get] IPP engine in/out configuration
} KDRV_IPP_JOB_PAR;

typedef struct {
	KDRV_IPP_JOB_PAR ipp;
	KDRV_PRE_JOB_CFG pre;
	KDRV_IFE_JOB_CFG ife;
	KDRV_DCE_JOB_CFG dce;
	KDRV_IPE_JOB_CFG ipe;
	KDRV_IME_JOB_CFG ime;
} KDRV_IPP_JOB_CFG_ALL;

typedef union {
	KDRV_IPP_JOB_PAR ipp;
	KDRV_PRE_JOB_CFG pre;
	KDRV_IFE_JOB_CFG ife;
	KDRV_DCE_JOB_CFG dce;
	KDRV_IPE_JOB_CFG ipe;
	KDRV_IME_JOB_CFG ime;
} KDRV_IPP_JOB_CFG_UNION;

#if 0
typedef struct {
	KDRV_IME_JOB_CFG ime_cfg;
	KDRV_IPE_JOB_CFG ipe_cfg;
	KDRV_DCE_JOB_CFG dce_cfg;
	UINT32 owner;
} KDRV_IPP_CB_PRESET_INFO;
#endif
typedef struct{
	KDRV_PRE_VA_RST            pre_va_rslt;
	KDRV_PRE_VA_INDEP_RSLT     pre_inde_va_rslt;
	KDRV_PRE_HIST_RSLT         pre_hist_rslt;
#if (ipp_tmp_block_539A)
	KDRV_IFE_VA_RST            ife_va_rslt;
	KDRV_IFE_VA_INDEP_RSLT     ife_inde_va_rslt;
	KDRV_IFE_HIST_RSLT         ife_hist_rslt;
#endif
	KDRV_IPE_VA_RST            ipe_va_rslt;
	KDRV_IPE_VA_INDEP_RSLT     ipe_inde_va_rslt;
	KDRV_IPE_EDGE_STCS_RESULT  edge_stcs_rslt;
	KDRV_IPE_DEFOG_STCS_RESULT defog_stcs_rslt;
	//KDRV_IME_VA_RESULT         ime_va_rslt;
	//KDRV_IME_VA_INDEP_RESULT   ime_inde_va_rslt;

	KDRV_PRE_VA_INDEP_RSLT     pre_inde_va_rslt_tmp;
#if (ipp_tmp_block_539A)
	KDRV_IFE_VA_INDEP_RSLT     ife_inde_va_rslt_tmp;
#endif
	KDRV_IPE_VA_INDEP_RSLT     ipe_inde_va_rslt_tmp;
	//KDRV_IME_VA_INDEP_RESULT   ime_inde_va_rslt_tmp;
	KDRV_PRE_HIST_RSLT         pre_hist_rslt_tmp;
#if (ipp_tmp_block_539A)
	KDRV_IFE_HIST_RSLT         ife_hist_rslt_tmp;
#endif
	KDRV_IPE_DEFOG_STCS_RESULT defog_stcs_rslt_tmp;
	KDRV_IPE_EDGE_STCS_RESULT  edge_stcs_rslt_tmp;
} KDRV_IPP_CB_STCS_OUTPUT;

typedef struct {
	ULONG cfg_owner;
	KDRV_IPP_JOB_CFG_ALL job;
	UINT32 *p_ts_arr;	/* timestamp arr, KDRV_IPP_JOB_TS */
	UINT32 cfg_status;
	UINT32 proc_ipp_num;				// number of ipp that process this job
	BOOL proc_ipp_id[KDRV_IPP_ENG_NUM];	// TRUE if this ipp id is used to process this job
	UINT32 cur_ipp_id;					// ipp id of this cb comes from, used in multiple ipp that have multiple cb
	KDRV_IPP_CB_STCS_OUTPUT* p_stcs_out_data; // ipp statistic
} KDRV_IPP_CB_JOB_INFO;

typedef struct {
	KDRV_IPP_ENG eng;
	UINT32 interrupt;
	ULONG cfg_owner;
	KDRV_IPP_JOB_CFG_UNION job;	/* job of corresponding engine */
	UINT32 proc_ipp_num;				// number of ipp that process this job
	BOOL proc_ipp_id[KDRV_IPP_ENG_NUM];	// TRUE if this ipp id is used to process this job
	UINT32 cur_ipp_id;					// ipp id of this cb comes from, used in multiple ipp that have multiple cb
} KDRV_IPP_CB_ISR_INFO;

typedef struct {
	UINT32 event;
	ULONG job_owner;
	UINT32 job_id;
	UINT8 job_num;
	UINT8 err_num;
	UINT8 done_num;
	void* p_info;
	UINT64 timestamp;	/* last frame start timestamp for direct mode */
	KDRV_IPP_CB_STCS_OUTPUT* p_stcs_out;
} KDRV_IPP_CALLBACK_INFO;

typedef struct {
	KDRV_IPP_ENG eng;
	UINT32 param_id;
	void *p_data;
} KDRV_IPP_ENG_INFO;

typedef struct{
	KDRV_IPP_OPMODE op_mode;
	UINT32 in_width;
	UINT32 in_height;

	KDRV_IPP_FMT pre_in_fmt; //539A add
	KDRV_IPP_FMT ife_in_fmt;

	KDRV_IPP_OPMODE pre_op_mode;
	KDRV_IPP_OPMODE ife_op_mode;
	KDRV_IPP_OPMODE ipe_op_mode;
	KDRV_IPP_OPMODE ime_op_mode;
	KDRV_IPP_FMT ime_in_fmt;
	KDRV_IPP_FMT ime_out_fmt[KDRV_IME_PATH_NUM_MAX];
	KDRV_IME_SCL_METHOD_SEL scl_method_sel;
	UINT8 _3dnr_en;         /* 3dnr enable */
	UINT8 lca_en;           /* lca enable*/
	UINT8 pxl_subout_en;    /* private mask pixelation enable*/
	UINT8 ime_path_en[KDRV_IME_PATH_NUM_MAX];
	USIZE ime_in_size;    /* use to calculate max scale down rate */
	USIZE ime_out_size[KDRV_IME_PATH_NUM_MAX];
	UINT32 lca_width;       /* 0 for disable */
	//539A add
	UINT8 pre_bnr_enable;

	BOOL ife_nn_enable;
	UINT32 ife_nn_max_strp_size;

	BOOL ime_nn_enable;
	UINT32 ime_nn_max_strp_size;

	KDRV_IPP_STRP_RULE strp_rule;

} KDRV_IPP_CAL_STRP_INFO;

typedef struct{
	UINT32 mod_stripe_num;
	UINT32 pre_in_strp[KDRV_IPP_MAX_STP_NUM];
	UINT32 pre_ovlp;
	UINT32 ife_in_strp[KDRV_IPP_MAX_STP_NUM];
	UINT32 ife_ovlp;
	UINT32 ipe_in_strp[KDRV_IPP_MAX_STP_NUM];
	UINT32 ipe_ovlp;
	UINT32 ime_in_strp[KDRV_IPP_MAX_STP_NUM];
	UINT32 ime_ovlp;
	UINT32 tmnr_out_strp[KDRV_IPP_MAX_STP_NUM];
	KDRV_IME_STRP_RST ime_qry_rst;
}KDRV_IPP_STRP_RSLT;

typedef struct{
	KDRV_IPP_CAL_STRP_INFO cal_info;
	KDRV_IPP_STRP_RSLT rst;
} KDRV_IPP_STRP_INFO;

typedef struct{
	KDRV_IME_IN_LMT ime_lmt;
	KDRV_IPP_FMT fmt;
} KDRV_IME_IN_LMT_INFO;

typedef struct{
	KDRV_IME_LMT ime_lmt;
	KDRV_IPP_FMT fmt;
} KDRV_IME_LMT_INFO;



typedef enum {
	KDRV_IPP_PARAM_GEN_NODE = 0,	/* Generate config node.             SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_FLUSH_JOB,		/* flush current job.				 SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_FLUSH_CFG,		/* flush current cfg.				 SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_FLUSH_ALL,		/* flush all jobs.					 SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_IPP_CFG,			/* set kdrv_ipp cfg.				 SET-Only,   data_type: KDRV_IPP_JOB_PAR */
	KDRV_IPP_PARAM_PRE_CFG,			/* set kdrv_pre cfg.				 SET-Only,   data_type: KDRV_PRE_JOB_CFG */
	KDRV_IPP_PARAM_IFE_CFG,			/* set kdrv_ife cfg.				 SET-Only,   data_type: KDRV_IFE_JOB_CFG */
#if 0 //690 removed
	KDRV_IPP_PARAM_DCE_CFG,			/* set kdrv_dce cfg.				 SET-Only,   data_type: KDRV_DCE_JOB_CFG */
#endif
	KDRV_IPP_PARAM_IPE_CFG,			/* set kdrv_ipe cfg.				 SET-Only,   data_type: KDRV_IPE_JOB_CFG */
	KDRV_IPP_PARAM_IME_CFG,			/* set kdrv_ime cfg.				 SET-Only,   data_type: KDRV_IME_JOB_CFG */
	KDRV_IPP_PARAM_IFE2_CFG,		/* set kdrv_ife2 cfg.				 SET-Only,   data_type: KDRV_IFE2_JOB_CFG */
	KDRV_IPP_PARAM_JOB_CFG,			/* get cur job .					 GET-Only,   data_type: KDRV_IPP_JOB_CFG_ALL */
	KDRV_IPP_PARAM_JOB_NUM,			/* get cur job number.				 GET-Only,   data_type: UINT32 */
	KDRV_IPP_PARAM_GET_ENG_INFO,	/* kdrv_eng get wrapper.			 GET-Only,   data_type: KDRV_IPP_ENG_INFO */
	KDRV_IPP_PARAM_QUERY_ENG_INFO,	/* kdrv_eng query wrapper.			 GET-Only,   data_type: KDRV_IPP_ENG_INFO */
	KDRV_IPP_PARAM_HARD_RESET,		/* hard reset all ipp engine.		 SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_DMA_ABORT,		/* dma abort all ipp engine.	     SET-Only,   data_type: NULL */
	KDRV_IPP_PARAM_GET_STRP_RSLT,	/* get stripe info directly		     GET-Only,   data_type: KDRV_IPP_STRP_INFO */
	KDRV_IPP_PARAM_GET_JOB_SPLT,	/* get if cur job is split by size	 GET-Only,   data_type: BOOL */
	KDRV_IPP_PARAM_GET_IME_IN_LMT,	/* get ime eng input limitation info GET-Only,   data_type: KDRV_IPP_IME_LMT_IN_INFO */
	KDRV_IPP_PARAM_GET_IME_LMT,		/* get ime eng output limitation infoGET-Only,   data_type: KDRV_IPP_IME_LMT_INFO */
	KDRV_IPP_PARAM_GET_AI_TO_IPP_CB,/* get ai_ti_ipp callback function   GET-Only,   data_type: ai_to_ipp callback func*/
	KDRV_IPP_PARAM_ID_MAX,
} KDRV_IPP_PARAM_ID;


typedef struct{
	ULONG reg_base_addr;
	UINT32 reg_num;
	KDRV_IPP_ENG id;
}KDRV_IPP_DTSI_ENG_INFO;

typedef struct{
	UINT8 eng_num;
	KDRV_IPP_DTSI_ENG_INFO *eng_info;
	KDRV_IME_PM_INFO pm_info[KDRV_IME_PM_SET_IDX_MAX];
	UINT32 dtsi_checksum;
}KDRV_IPP_DTSI_INFO;

typedef enum{
	KDRV_IPP_JOB_ST_OP_GET_DTSI_INFO   = 1,
	KDRV_IPP_JOB_ST_OP_CHK_DTSI_CHKSUM = 2,
	KDRV_IPP_JOB_ST_OP__ID_MAX
}KDRV_IPP_JOB_START_OP;

typedef struct {
	ULONG job_owner;
	CHAR job_owner_name[KDRV_IPP_OWNER_NAME_MAX];
	UINT32 write_reg_mode;	/* KDRV_IPP_PROC_MODE_LINKLIST / KDRV_IPP_PROC_MODE_CPU */
	UINT32 op;
	KDRV_IPP_JOB_START_OP job_st_op;
	UINT32 (*job_start_cb)(void *hdl, UINT32 param_id, void *data);
	KDRV_IPP_BIND_MODE bind_mode;

} KDRV_IPP_TRIG_PARAM;

typedef struct {
	ULONG base_info;
	UINT32 isp_id;
	KDRV_IPP_CB_STCS_OUTPUT* p_stcs_out_data;
	INT32 (*ipp_to_ai_cb)(KDRV_IPP_NN_ISP_OPERATION op, void* in, void* out);

	#if(KDRV_AI_TEST)
	ULONG kdrv_ai_pa;
	ULONG kdrv_ai_va;
	ULONG ubuf_pa;
	#endif
} KDRV_IPP_NODE_INFO;

typedef struct{
	ULONG va;
	ULONG pa;
} KDRV_IPP_BUF_ADDR;

ULONG kdrv_ipp_buf_query(KDRV_IPP_CTX_BUF_CFG cfg);
INT32 kdrv_ipp_init(KDRV_IPP_CTX_BUF_CFG ctx_buf_cfg, KDRV_IPP_BUF_ADDR buf_addr, UINT32 buf_size);
INT32 kdrv_ipp_uninit(void);
INT32 kdrv_ipp_open(void* node, UINT32 hdl_id);
INT32 kdrv_ipp_close(void *p_node, UINT32 hdl_id);
INT32 kdrv_ipp_set(void* node, UINT32 hdl_id, UINT32 param_id, void *p_data);
INT32 kdrv_ipp_get(void* node, UINT32 hdl_id, UINT32 param_id, void *p_data);
INT32 kdrv_ipp_trigger(void* node,  void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
INT32 kdrv_ipp_direct_drop_old_job(void* p_node, UINT32 hdl_id, ULONG last_addr, ULONG job_owner);
INT32 kdrv_ipp_direct_trigger(void* node,  void *p_param, KDRV_CALLBACK_FUNC *p_cb_func, VOID *p_user_data);
INT32 kdrv_ipp_direct_wait_fmd(void* node, UINT32 hdl_id, UINT32 timeout, UINT32 cnt, UINT32 reserved);
UINT32 kdrv_ipp_dbg_mode(void *p_hdl, UINT32 param_id, void *data);
INT32 kdrv_ipp_cal_stripe(void *p_handle, KDRV_IPP_STRP_INFO* info);
INT32 kdrv_ipp_create_node(void** hdl_node, UINT32 hdl_id, KDRV_IPP_CREATE_NODE_PARAM* p_param);
INT32 kdrv_ipp_destory_node(void* hdl_node);
INT32 kdrv_ipp_module_printout_handler(uintptr_t data);
void kdrv_ipp_debug_dumpmem_log(ULONG addr, ULONG length);
void kdrv_ipp_util_dump(void *sfile);
#endif //_KDRV_IPP_H_
