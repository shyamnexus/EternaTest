#ifndef _DRE_DRV_FLOW_INT_H_
#define _DRE_DRV_FLOW_INT_H_

#include "dre_drv_ctl_int.h"
#include "dre_drv_ll_int.h"
#include "dre_drv_fusion_int.h"
#include "dre_eng.h"

#define DRE_MAX_LL_CMD_NUM (1000)

int dre_drv_flow_proc(u16 chip_id, u16 eng_id, DRE_ENG_HANDLE *eng_hdl, struct dre_drv_job_cfg *job_cfg,
							struct dre_drv_ll_blk *proc_ll_blk, struct dre_drv_ctl_sw_job_info *sw_job_info, struct dre_drv_fusion_layer *fusion_layer);

int dre_drv_flow_dump_job_cfg(struct dre_drv_job_cfg *job_cfg, int dump_flag);
int dre_drv_flow_dump_layer_cfg(struct dre_drv_fusion_layer *fusion_layer);
unsigned int dre_drv_flow_predict_bscore(struct dre_drv_job_head *proc_job_head);
int dre_drv_flow_sw_balance_proc(struct dre_drv_job_head *proc_job_head);
void dre_drv_flow_set_msg(int flag);
void dre_drv_flow_set_layer_dbg(int flag);
//int dre_drv_flow_sw_proc(struct dre_drv_job_cfg *job_cfg, struct dre_drv_ctl_sw_job_info *sw_job_info);
#endif //_DRE_DRV_FLOW_INT_H_
