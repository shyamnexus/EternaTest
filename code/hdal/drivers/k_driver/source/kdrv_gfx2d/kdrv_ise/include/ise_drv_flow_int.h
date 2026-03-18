#ifndef _ISE_DRV_FLOW_INT_H_
#define _ISE_DRV_FLOW_INT_H_

#include "ise_drv_ctl_int.h"
#include "ise_drv_ll_int.h"
#include "ise_eng.h"

#define ISE_MAX_LL_CMD_NUM (40)

int ise_drv_flow_proc(u16 chip_id, u16 eng_id, ISE_ENG_HANDLE *eng_hdl, struct ise_drv_job_cfg *job_cfg, struct ise_drv_ll_blk *proc_ll_blk);
int ise_drv_flow_dump_job_cfg(struct ise_drv_job_cfg *job_cfg, int dump_flag);
unsigned int ise_drv_flow_predict_bscore(struct ise_drv_job_head *proc_job_head);
int ise_drv_flow_sw_balance_proc(struct ise_drv_job_head *proc_job_head);
void ise_drv_flow_set_msg(int flag);
int ise_drv_flow_out_cache_flush_proc(struct ise_drv_job_head *proc_job_head);
int ise_drv_flow_in_cache_flush_proc(struct ise_drv_job_cfg *job_cfg);
#endif //_ISE_DRV_FLOW_INT_H_
