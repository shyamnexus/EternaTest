/**
	@brief Header file of internal definition of vendor net flow sample.

	@file net_flow_sample_int.h

	@ingroup net_flow_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NET_FLOW_SAMPLE_INT_H_
#define _NET_FLOW_SAMPLE_INT_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "kflow_ai_net/kflow_ai_net.h"
#include "kflow_ai_net/kflow_ai_net_comm.h"
#include "kflow_ai_net/nn_net.h"

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
ER nvt_ai_chk_vers(VENDOR_AIS_FLOW_VERS *p_vers_info);

void nvt_ai_mem_info_update_1(VENDOR_AIS_MEM_INFO *mem_info);

ER nvt_ai_global_init(void);
ER nvt_ai_global_uninit(void);

//init/uninit/reset
ER nvt_ai_get_max_net(VENDOR_AIS_FLOW_ID *id_info);
ER nvt_ai_reset_net(void);
ER nvt_ai_reset_net_path(UINT32 net_id);
ER nvt_ai_init_net(VENDOR_AIS_FLOW_ID *id_info);
ER nvt_ai_uninit_net(void);
ER nvt_ai_uninit_net_path(UINT32 net_id);

//lock/unlock for state open/start
ER nvt_ai_lock_net(UINT32 net_id);
ER nvt_ai_unlock_net(UINT32 net_id);


ER nvt_ai_open_net(VENDOR_AIS_FLOW_MAP_MEM_PARM  *p_mem, UINT32 net_id);
ER nvt_ai_close_net(UINT32 net_id);

ER nvt_ai_get_id(UINT32 *proc_id);
ER nvt_ai_release_id(UINT32 proc_id);
ER nvt_ai_comm_lock(VOID);
ER nvt_ai_comm_unlock(VOID);
ER nvt_ai_comm_usr_lock(VOID);
ER nvt_ai_comm_usr_unlock(VOID);
ER nvt_ai_op_lock(VOID);
ER nvt_ai_op_unlock(VOID);
ER nvt_ai_normal_get_ubuf(UINT32 proc_id, INT32 *available_idx, UINT32 need_size);
ER nvt_ai_normal_free_ubuf(UINT32 proc_id, INT32 idx, UINT32 size);
ER nvt_ai_has_pool(BOOL* has_pool);
ER kflow_ai_set_pool_info(VENDOR_AIS_GBlkTile_POOL_INFO * pool_info);
ER kflow_ai_set_default_pool_info(VENDOR_AIS_GBlkTile_POOL_INFO * pool_info);
ER kflow_ai_get_gblktile_ubuf(UINT32 proc_id, INT32 *available_idx);
ER kflow_ai_get_gblktile_jobm_core(UINT32 proc_id, INT32 *available_idx);
ER kflow_gblktile_set_core_mask(VENDOR_AIS_GBlkTile_MASK* mask_info);
ER kflow_gblktile_set_ub_mask(VENDOR_AIS_GBlkTile_MASK* mask_info);
ER kflow_get_dtsi_info(VENDOR_AIS_DTSI_INFO* dtsi_info);

#endif  /* _NET_FLOW_SAMPLE_INT_H_ */
