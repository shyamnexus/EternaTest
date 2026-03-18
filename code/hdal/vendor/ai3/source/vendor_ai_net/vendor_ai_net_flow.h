/**
	@brief Header file of definition of vendor user-space net flow sample.

	@file net_flow_user_sample.h

	@ingroup net_flow_user_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_NET_FLOW_H_
#define _VENDOR_AI_NET_FLOW_H_

#define NET_FLOW_USR_SAMPLE_LAYER_OUT	TRUE

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "kflow_ai_net/kflow_ai_net.h"
#include "vendor_ai_net/nn_parm.h"
#include "vendor_ai_internal.h"
#include "vendor_ai.h"
#include "vendor_ai_plugin.h"
#include "vendor_ai_comm.h"
#include "vendor_ai_net/nn_diff.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

#define AI2_MAX_BATCH_NUM  16

#define AI_ROUND(x) (x + 0.5)

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
//extern HD_RESULT vendor_ais_init(void);
//extern HD_RESULT vendor_ais_uninit(void);
//extern HD_RESULT vendor_ais_lock_net(UINT32 net_id);
//extern HD_RESULT vendor_ais_unlock_net(UINT32 net_id);
extern HD_RESULT vendor_ais_comm_lock(void);
extern HD_RESULT vendor_ais_comm_unlock(void);
extern HD_RESULT vendor_ais_op_lock(void);
extern HD_RESULT vendor_ais_op_unlock(void);
extern HD_RESULT vendor_ais_open_net(UINT32 net_id);
extern HD_RESULT vendor_ais_close_net(UINT32 net_id);
extern HD_RESULT vendor_ais_open_isp_net(UINT32 net_id);
extern HD_RESULT vendor_ais_close_isp_net(UINT32 net_id);
extern HD_RESULT vendor_ais_cache_check(UINT32 net_id, uintptr_t addr);
extern HD_RESULT vendor_ais_remap_kerl_mem(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info);
extern HD_RESULT vendor_ais_unmap_kerl_mem(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info);
extern HD_RESULT vendor_ais_pars_kerl_mem(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info);
extern INT32 vendor_ais_flow_user_init(VENDOR_AIS_FLOW_MAP_MEM_PARM mem, UINT32 net_id);
extern INT32 vendor_ais_flow_user_uninit(UINT32 net_id);
#if 0 //LL_SUPPORT_ROI
extern HD_RESULT vendor_ais_set_ll_info(AI_DRV_LL_USR_INFO *ll_usr_info, UINT32 net_id);
#endif

extern HD_RESULT vendor_ais_start_net(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 net_id, UINT32 order, INT32 wait_ms, UINT32 ddr_id);
extern HD_RESULT vendor_ais_push_net(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 net_id, UINT32 order);
extern HD_RESULT vendor_ais_pull_net(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 net_id, INT32 wait_ms);
extern HD_RESULT vendor_ais_proc_net(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 net_id, UINT32 order, INT32 wait_ms);
extern HD_RESULT vendor_ais_stop_net(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 net_id);
extern HD_RESULT vendor_ais_get_obuf_path(CHAR* path);

extern HD_RESULT vendor_ais_unpars_kerl_mem(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info);
extern HD_RESULT vendor_ais_proc_input_init(NN_MODE mode, NN_DATA_V30 *p_1st_imem, VENDOR_AIS_FLOW_MEM_PARM mem, UINT32 net_id);
extern HD_RESULT vendor_ais_proc_input_uninit(NN_DATA_V30 *p_1st_imem, VENDOR_AIS_FLOW_MEM_PARM mem, UINT32 net_id);
extern HD_RESULT vendor_ais_proc_input_init2(NN_MODE mode, NN_DATA_V30 *p_1st_imem, VENDOR_AIS_FLOW_MEM_PARM mem, UINT32 proc_idx, UINT32 net_id, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_input_uninit2(NN_DATA_V30 *p_1st_imem, VENDOR_AIS_FLOW_MEM_PARM mem, UINT32 proc_idx, UINT32 net_id, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_output_init(VENDOR_AI_BUF *ai_buf, UINT32 net_id, UINT32 layer_id, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_output_uninit(VENDOR_AI_BUF *ai_buf, UINT32 net_id, UINT32 layer_id, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_output_addr(UINT32 net_id, VENDOR_AI_BUF *out_buf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_output_addr_multi_ll (UINT32 net_id, VENDOR_AI_BUF *out_buf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_input_addr_of_output_layer_multi_ll (UINT32 net_id, VENDOR_AI_BUF *out_buf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_input_addr_of_output_layer(UINT32 net_id, VENDOR_AI_BUF *ai_out_bufbuf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_input_addr_of_output_layer_all_res(UINT32 net_id, VENDOR_AI_BUF *out_buf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 proc_idx, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_assign_output_addr_all_res(UINT32 net_id, VENDOR_AI_BUF *out_buf, NN_GEN_MODE_CTRL *p_mctrl, UINT32 proc_idx, UINT32 port_id);
extern NN_GEN_MODE_CTRL *vendor_ais_get_proclayer(UINT32 layer, VENDOR_AIS_FLOW_MEM_PARM mem);
extern HD_RESULT vendor_ais_update_proclayer(UINT32 layer, UINT32 net_id);
extern HD_RESULT vendor_ais_get_net_info(NN_GEN_NET_INFO *p_info, uintptr_t net_addr);
extern HD_RESULT vendor_ais_chk_vers(UINT32 chip_id, UINT32 gentool_vers, UINT32 net_id);
extern UINT32 vendor_ais_net_get_input_layer_index(VENDOR_AIS_FLOW_MEM_PARM mem);

extern BOOL vendor_ais_is_dybatch_model(VENDOR_AIS_FLOW_MEM_PARM *p_mem);
extern HD_RESULT vendor_ais_pars_diff_mem(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AI_DIFF_MODEL_RESINFO *p_diff_resinfo, UINT32 net_id);
extern HD_RESULT vendor_ais_update_diff_mem(VENDOR_AIS_FLOW_UPDATE_NET_INFO *p_info, VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, NN_DIFF_MODEL_HEAD* p_diff_model_head);

extern HD_RESULT vendor_ais_pars_diff_batch_mem(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AI_DIFF_MODEL_RESINFO *p_diff_resinfo, UINT32 net_id);
//extern HD_RESULT vendor_ais_unpars_diff_batch_mem(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AIS_DIFF_BATCH_MODEL_INFO *p_diff_info, UINT32 net_id);
//extern HD_RESULT vendor_ais_get_diff_batch_id_num(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, UINT32 *batch_id_num);
//extern HD_RESULT vendor_ais_get_diff_batch_id_info(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, UINT32 *max_batch_num, UINT32* p_batch_id);
extern HD_RESULT vendor_ais_set_usr_info(VOID* p_usr_info, UINT32 net_id);
HD_RESULT vendor_ais_pars_diff_batch(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AI_DIFF_MODEL_RESINFO *p_diff_resinfo, UINT32 net_id);
//HD_RESULT vendor_ais_unpars_diff_batch(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AI_BATCH_INFO *p_diff_batch, UINT32 net_id);

typedef HD_RESULT (*VENDOR_AIS_JOB_CB)(UINT32 proc_id, UINT32 job_id);

extern HD_RESULT vendor_ais_net_reg_JOB(VENDOR_AIS_JOB_CB fp);
extern HD_RESULT vendor_ais_net_builtin_JOB_done(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);

#if NN_DLI
extern HD_RESULT vendor_ais_net_builtin_CPU_init(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#endif
extern HD_RESULT vendor_ais_net_builtin_CPU_exec(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#if NN_DLI
extern HD_RESULT vendor_ais_net_builtin_CPU_exit(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#endif

#if (AI_SUPPORT_DSP == 1)
#if NN_DLI
extern HD_RESULT vendor_ais_net_builtin_DSP_init(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#endif
extern HD_RESULT vendor_ais_net_builtin_DSP_exec(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#if NN_DLI
extern HD_RESULT vendor_ais_net_builtin_DSP_exit(UINT32 proc_id, UINT32 job_id, VENDOR_AIS_FLOW_MEM_PARM mem);
#endif
#endif

extern HD_RESULT vendor_ai_init_job_graph(UINT32 net_id);
extern HD_RESULT vendor_ai_uninit_job_graph(UINT32 net_id);
extern HD_RESULT vendor_ai_sig_job_graph(UINT32 net_id);
extern HD_RESULT vendor_ai_wait_job_graph(UINT32 net_id, UINT32* job_id);

extern HD_RESULT vendor_ais_get_diff_batch_mem(VENDOR_AIS_FLOW_MAP_MEM_PARM *p_mem, VENDOR_AIS_FLOW_MEM_PARM* p_diff_mem);
//extern HD_RESULT vendor_ais_get_multiscale_max_dim(UINT32 proc_id, HD_DIM *p_max_dim);
extern NN_DIFF_MODEL_HEAD *vendor_ais_get_diff_model_head(UINT32 proc_id);
//extern HD_RESULT vendor_ais_get_diff_model_head(VENDOR_AIS_FLOW_MEM_PARM *p_mem, NN_DIFF_MODEL_HEAD* p_diff_model_head);
extern HD_RESULT vendor_ais_set_model_id(NN_DIFF_MODEL_HEAD* p_diff_model_head, VENDOR_AI_DIFF_MODEL_RESINFO *p_resinfo);
extern INT32 vendor_ai_get_kflow_version(CHAR* kflow_version);
extern INT32 vendor_ai_get_kflow_isp_version(CHAR* kflow_isp_version);
extern INT32 vendor_ai_get_kdrv_version(CHAR* kdrv_version);

extern UINT32 vendor_ais_net_get_fc_cmd_size(UINT32 max_weight_h, UINT32 net_id);
extern HD_RESULT vendor_ais_set_fc_ll_cmd(VENDOR_AI_OP_FC_CMDBUF *cmd_buf, UINT32 net_id);
extern UINT32 vendor_ais_net_get_mau_cmd_size(UINT32 in_width, UINT32 in_byte, UINT32 in_obj, UINT32 mau_cal_mode, UINT32 mau_mat_op);
extern HD_RESULT vendor_ais_set_mau_ll_cmd(VENDOR_AI_OP_MAU_CMDBUF *cmd_buf, UINT32 net_id);
extern HD_RESULT vendor_ais_has_pool(BOOL* has_pool);
extern HD_RESULT vendor_ais_get_isp_cb(uintptr_t *isp_cb);
extern HD_RESULT vendor_ais_set_isp_cb(uintptr_t *isp_cb);
extern HD_RESULT vendor_ais_set_isp_pool_id(UINT32 proc_id, UINT32 pool_id, UINT32 need_ubuf_size, UINT32 need_core_num, UINT32 share_workbuf);
extern HD_RESULT vendor_ais_dbg_isp_push_frame(NN_DATA_V30 *p_imem, UINT32 net_id);
extern HD_RESULT vendor_ais_dbg_isp_set_ref_frame(NN_DATA_V30 *p_imem, UINT32 net_id, UINT32 ref_frame_id);
extern HD_RESULT vendor_ais_dbg_isp_set_isp_param(UINT32 net_id, UINT32 param_num, UINT32 *param_size, ULONG *param_phyaddr);
extern HD_RESULT vendor_ais_dbg_isp_cal_slice(UINT32 height, UINT32 width, UINT32 net_id);
extern HD_RESULT vendor_ais_get_shared_workbuf(UINT32 pool_id, VENDOR_AI3_CFG_BUF* share_workbuf);
extern HD_RESULT vendor_ais_proc_set_isp_input(NN_DATA_V30 *p_imem, UINT32 net_id, UINT32 layer_id, UINT32 port_id);
extern HD_RESULT vendor_ais_proc_set_isp_input_info(NN_DATA_V30 *p_imem, UINT32 net_id, UINT32 layer_id, UINT32 port_id);
extern HD_RESULT vendor_ais_pars_jmisp_net(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info) ; 
extern HD_RESULT vendor_ais_close_jmisp_net(VENDOR_AIS_FLOW_MAP_MEM_INFO *p_info);
extern HD_RESULT vendor_ais_get_isp_jobm_core(UINT32 proc_id, INT32 *jobm_core);
extern HD_RESULT vendor_ais_set_isp_core_mask(UINT32 proc_id, UINT32 core_mask);
extern HD_RESULT vendor_ais_set_isp_ub_mask(UINT32 proc_id, UINT32 ub_mask);

extern HD_RESULT vendor_ais_bind_dla_core(VENDOR_AIS_FLOW_MEM_PARM net_mem, UINT32 proc_id, UINT32 core_mask);

extern HD_RESULT vendor_ais_get_workbuf_size_from_jmisp_info(VENDOR_AIS_FLOW_MEM_PARM* jmisp_info, UINT32* jmisp_info_buf_size);
extern HD_RESULT vendor_ais_get_isp_ubuf(UINT32 proc_id, INT32 *ubuf_idx, UINT32 need_size_in_mb);
extern HD_RESULT vendor_ais_free_isp_ubuf(UINT32 proc_id, INT32 ubuf_idx, UINT32 need_size_in_mb);
extern HD_RESULT vendor_ais_get_normal_ubuf(UINT32 proc_id, INT32 *ubuf_idx, UINT32 need_size_in_mb);
extern HD_RESULT vendor_ais_free_normal_ubuf(UINT32 proc_id, INT32 ubuf_idx, UINT32 need_size_in_mb);

extern HD_RESULT vendor_ais_gblk_init(UINT32 net_id);
extern HD_RESULT vendor_ais_gblk_uninit(UINT32 net_id);
extern HD_RESULT vendor_ais_set_pool_id(UINT32 proc_id, UINT32 pool_id, UINT32 need_ubuf_num, UINT32 need_core_num);
extern HD_RESULT vendor_ais_get_gblktile_ubuf(UINT32 proc_id, INT32 *ubuf_idx);
extern HD_RESULT vendor_ais_get_gblktile_jobm_core(UINT32 proc_id, INT32 *jobm_core);
extern HD_RESULT vendor_ais_set_gblktile_core_mask(UINT32 proc_id, UINT32 core_mask);
extern HD_RESULT vendor_ais_set_gblktile_ub_mask(UINT32 proc_id, UINT32 ub_mask);
extern HD_RESULT vendor_ais_get_dtsi_info(VENDOR_AI3_DEV_INFO* dtsi_info);

extern HD_RESULT vendor_ais_set_usage_limit(UINT32 usage_limit) ; 
extern HD_RESULT vendor_ais_sw_reset(void) ;

extern HD_RESULT vendor_ais_postproc_init(UINT32 net_id);
extern HD_RESULT vendor_ais_postproc_uninit(UINT32 net_id);

#endif  /* _VENDOR_AI_NET_FLOW_H_ */
