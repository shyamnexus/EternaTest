/**
	@brief Source file of kflow_ai_net.

	@file kflow_ai_isp.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_ISP_H_
#define _KFLOW_AI_ISP_H_

#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/error_no.h"
#include "kflow_isp/kflow_ai_isp_net.h"
#include "kflow_isp/kflow_isp_change_res.h"

#define NN_ISP_NN_PATH_MAX   (4)
#define KDRV_IPP_MAX_STP_NUM (8)
#define NNISP_DEBUG           0         

#define FLG_ID_JMISP_ALL           0xF
#define FLG_ID_JMISP_A          FLGPTN_BIT(0)
#define FLG_ID_JMISP_B          FLGPTN_BIT(1)
#define FLG_ID_JMISP_C          FLGPTN_BIT(2)
#define FLG_ID_JMISP_D          FLGPTN_BIT(3)

typedef enum {
	NN_ISP_OP_GET_NN_INFO       = 0,   // get slice_height, slice_overlap // call every frame, input NN_ISP_IN_PARAM,   output NN_ISP_NN_INFO
	NN_ISP_OP_GET_UBUF_LAYOUT   = 1,   // get UBUF layout                 // call every frame, input NN_ISP_IN_PARAM,   output NN_ISP_PATH_BUF_INFO
	NN_ISP_OP_GET_MAX_STRP      = 2,   // get NN max stripe width         // call when enable, input NN_ISP_IN_PARAM,   output NN_ISP_NN_STRP
	NN_ISP_OP_CAL_SLICE         = 3,   // set NN to calculate slice info  // call after NN_ISP_OP_GET_MAX_STRP(enable), input NN_ISP_SET_RES_PARAM, output NN_ISP_NN_INFO
	NN_ISP_OP_CHANGE_RES        = 4,   // notify AI to chagne res         // reserve
	NN_ISP_OP_UPDATE            = 5,   // notify AI for event             // call when update event(disable), input NN_ISP_IN_PARAM
	NN_ISP_OP_PUSH_NN_JOB       = 6,   // push nn_job                     // call every frame, input NN_ISP_IPP_NN_JOB
	NN_ISP_OP_PUSH_SLICE_JOB    = 7,   // push slice_job                  // call every slice, input NN_ISP_IPP_SLICE_JOB, output NULL
	NN_ISP_OP_QUERY_AI_STATUS   = 8,   // query ai open                   // call before open/start
	NN_ISP_OP_NKNOWN,

	NN_ISP_OP_PUSH_FRAME        = 100, // push frame                      // call every frame, input NN_ISP_FRAME,     output NN_ISP_FRAME
	NN_ISP_OP_FRAME_MODE        = 101, // get FRAME_MODE                  // call when enable, inpput: NN_ISP_FRAME_MODE_INFO   output UINT32 mode (1 : FRAME_MODE, 0 : NN_JOB Mode)
	NN_ISP_OP_FRAME_UPDATE      = 102, // notify AI for event             // call when update event(disable), input NN_ISP_IN_PARAM;
	NN_ISP_OP_SET_FUSION_WEIGHT = 103, // set FUSION_WEIGHT               // call every frame, input NN_ISP_FRAME,     output NN_ISP_FRAME
	NN_ISP_OP_SET_SIGMA         = 104, // set SIGMA                       // call every frame, input NN_ISP_FRAME,     output NN_ISP_FRAME
	NN_ISP_OP_SET_GAMMA         = 105, // set GAMMA                       // call every frame, input NN_ISP_FRAME,     output NN_ISP_FRAME
	NN_ISP_OP_GET_MOTION_TYPE   = 107, // get motion type                 // call when update event, inpput: N/A ,     output NN_ISP_MOTION_TYPE_INFO
	NN_ISP_OP_SET_ISP_PARAM     = 108, // set isp input param             // call when new set input flow, input: NN_ISP_ISP_PARAM, output: N/A
	NN_ISP_OP_CHK_PARAM_VALID   = 109, // check model/flow setting match  // call every frame, input: NN_IPP_CHK_PARAM, output: NN_AI_CHK_PARAM
	NN_ISP_OP_PER_FRAME_PARAM   = 110, // set per-frame param             // call every frame, input: NN_IPP_PER_FRAME_PARAM, output: N/A
} NN_ISP_OPERATION;

typedef enum {
  NN_ISP_AI_TO_IPP_OP_SLICE_DONE    = 0, // call when jmisp slice done, input NN_ISP_SLICE_DONE_PARAM, out NULL
	NN_ISP_AI_TO_IPP_OP_NKNOWN,
} NN_ISP_AI_TO_IPP_OPERATION;

typedef struct{
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
  UINT32 width;
  UINT32 height;
}NN_ISP_IPP_SLICE_JOB;

typedef struct{
  UINT32 ipp_id;
  UINT32 path_id;
  BOOL   frm_ed_flg;
  UINT32 frm_start_time;
  UINT32 frm_end_time;
}NN_ISP_SLICE_DONE_PARAM;

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
}NN_ISP_PATH_BUF_INFO;

typedef struct{
  UINT32 max_stripe;
  UINT32 min_stripe;
  UINT32 slice_overlap;
  UINT32 slice_max_out_height;
  UINT8  sw_handshake_en;
}NN_ISP_NN_INFO;

typedef struct{
  UINT32 max_stripe;
  UINT32 min_stripe;
}NN_ISP_NN_STRP;

typedef struct{
  BOOL   en;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 isp_id; //indicate which sensor to run
  UINT32 path_id;
}NN_ISP_IN_PARAM;

typedef struct{
  UINT32 stripe_width[KDRV_IPP_MAX_STP_NUM];
  UINT32 stripe_num;
  UINT32 stripe_overlap;
  UINT32 width;
  UINT32 height;
  UINT32 ipp_id;
  UINT32 proc_id;
  UINT32 path_id;
}NN_ISP_SET_RES_PARAM;

typedef struct{
 BOOL en;
 UINT32 proc_id;
 UINT32 isp_id; //indicate which sensor to run
 UINT32 nn_path_id;
}NN_ISP_PATH_INFO;

typedef struct{
  UINT32 ipp_id;
  UINT32 width;
  UINT32 height;
  NN_ISP_PATH_INFO nn_path[NN_ISP_NN_PATH_MAX];
}NN_ISP_IPP_NN_JOB;


typedef struct{ 
  uintptr_t parm_addr ;                   // p_ll_head->parm_addr
  uintptr_t in_buff_ofs; 
  UINT32 eng_type;
  UINT32 mctrl_id;
  UINT32 port_id;
}NN_ISP_INPUT;

typedef struct{ 
  VENDOR_AIS_ISP_MEM_PARM isp_input_addr; // record the effect parameter addr in inputfeat_pingpong
  UINT8 enable_3d_val;
}NN_ISP_ENABLE_3D;

typedef enum {
  NN_ISP_MOTION_TYPE_64 = 0,
  NN_ISP_MOTION_TYPE_16,
  NN_ISP_MOTION_TYPE_4,
  NN_ISP_MOTION_TYPE_MAX,
}NN_ISP_MOTION_TYPE;

typedef struct{
  UINT32 proc_id;
  BOOL en;
  NN_ISP_MOTION_TYPE motion_type;
}NN_ISP_MOTION_TYPE_INFO;

typedef struct{
  UINT32 proc_id;
  UINT32 reserved[8];
}NN_ISP_FRAME_MODE_INFO;

typedef struct{
  UINT8 dgain;      // fmt = 4.4 (4int, 4frac). For example => 1x=16 , 2x=32 , 4x=64
} NN_ISP_PER_FRAME_PARAM_DATA_V3;

typedef struct {
  UINT32 version;
  UINT32 proc_id;
  ULONG  data_va; // cast to NN_ISP_PER_FRAME_PARAM_DATA_Vxx to use, where xx=version
  UINT32 data_size;
  ULONG  reserved[8];
} NN_IPP_PER_FRAME_PARAM;

/*----------------------------------------------------------------------
NN_ISP_OPERATION op : operation selection  ex: NN_ISP_OP_GET_NN_INFO, NN_ISP_OP_GET_UBUF_LAYOUT, NN_ISP_OP_PUSH_NN_JOB
void* in           : input parameter      ex: NN_ISP_IPP_NN_JOB
void* out          : output parameter     ex: NN_ISP_NN_INFO, NN_ISP_PATH_BUF_INFO
return value       : success: 0, fail: -1
----------------------------------------------------------------------*/

/*-------------------------------------------------------------------
NN_ISP_AI_TO_IPP_OPERATION op : operation selection
void* in           : input parameter      ex: NN_ISP_IPP_NN_JOB
void* out          : output parameter     ex: NN_ISP_NN_INFO, NN_ISP_PATH_BUF_INFO
return value       : success: 0, fail: -1
--------------------------------------------------------------------*/

typedef ER (*KFLOW_AI_ISP_CB)(NN_ISP_OPERATION op, void* in, void* out);
//current kflow

#if (FLOW_AI_ISP == 1)
extern ER kflow_isp_set_cb(uintptr_t fp);
extern ER kflow_isp_cb (NN_ISP_OPERATION op, void* in, void* out);
extern ER kflow_isp_init_jmisp_net(void);
extern ER kflow_isp_uninit_jmisp_net(void);
extern ER kflow_isp_reset_jmisp_net(void);
extern ER kflow_isp_pars_jmisp_net (VENDOR_AIS_ISP_PARM *isp_parm);
extern ER kflow_isp_close_jmisp_net(UINT32 net_id) ;
extern ER kflow_isp_set_input (VENDOR_AIS_ISP_INPUT_INFO *p_isp_input_info);
extern ER kflow_isp_set_input_info (VENDOR_AIS_ISP_INPUT_INFO *p_isp_input_info);
extern ER kflow_isp_assign_all_input_addr (UINT32 net_id) ; 
extern void kflow_isp_set_prioity(UINT32 job_priority) ;
extern ER kflow_isp_free_ubuf(UINT32 proc_id, INT32 idx, UINT32 size) ; 
extern ER kflow_isp_get_ubuf(UINT32 proc_id, INT32 *available_idx, UINT32 need_size) ; 
extern ER kflow_isp_push_slice (UINT32 proc_id, UINT32 push_time) ;
extern ER kflow_isp_set_first_slice (UINT32 proc_id) ;
extern ER kflow_isp_set_sw_mode_job (UINT32 proc_id); 
extern ER kflow_isp_set_base_addr (UINT32 proc_id);
extern ER kflow_isp_set_weight_loc(VENDOR_AIS_ISP_WEIGHT_LOC * loc);
extern INT32 kflow_isp_get_jmisp_id(UINT32 proc_id) ; 
extern ER kflow_isp_push_frame (NN_ISP_FRAME *input, NN_ISP_FRAME *output) ;
extern ER kflow_isp_set_pool_info(VENDOR_AIS_ISP_POOL_INFO * pool_info);
extern ER kflow_isp_set_default_pool_info(VENDOR_AIS_ISP_POOL_INFO * pool_info);
extern ER kflow_isp_get_proc_id_by_pool(VENDOR_AIS_ISP_POOL_INFO* pool_info);
extern ER kflow_isp_get_jobm_core(UINT32 proc_id, INT32 *available_idx);
extern ER kflow_isp_get_st_in_core_mask(UINT32 proc_id, INT32 *available_idx);
extern ER kflow_isp_set_core_mask(VENDOR_AIS_ISP_MASK* mask_info);
extern ER kflow_isp_set_ub_mask(VENDOR_AIS_ISP_MASK* mask_info);
extern ER kflow_ai_isp_get_workbuf_size_from_jmisp_info(VENDOR_AIS_ISP_JMISP_INFO_PARM* jmisp_info_param);
extern ER kflow_isp_get_ppu_quan_out_scale_shift(UINT32 proc_id, AI_TUNING_QUAN_PARAM *p_ppu_quan_param);
extern ER kflow_isp_set_ppu_quan_out_scale_shift(UINT32 proc_id, AI_TUNING_QUAN_PARAM *p_ppu_quan_param);
extern ER kflow_isp_cal_gain(const AI_TUNING_QUAN_PARAM* i_para, AI_TUNING_QUAN_PARAM* o_para, const AI_FLOAT_FORMAT gain);
#endif 
#endif //_KFLOW_AI_ISP_H_
