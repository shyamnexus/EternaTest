#ifndef _IVE_DRV_FLOW_INT_H_
#define _IVE_DRV_FLOW_INT_H_

#include "ive_drv_ctl_int.h"
#include "ive_drv_ll_int.h"
#include "ive_eng.h"

#define IVE_MAX_LL_CMD_NUM (54)

int ive_drv_flow_proc(u16 chip_id, u16 eng_id, IVE_ENG_HANDLE *eng_hdl, struct ive_drv_job_cfg *job_cfg, struct ive_drv_ll_blk *proc_ll_blk);
int ive_drv_flow_isr_callback(IVE_ENG_HANDLE *eng_hdl, struct ive_drv_job_cfg *job_cfg);
int ive_drv_flow_dump_job_cfg(struct ive_drv_job_cfg *job_cfg, int dump_flag);
unsigned int ive_drv_flow_predict_bscore(struct ive_drv_job_head *proc_job_head);
int ive_drv_flow_sw_balance_proc(struct ive_drv_job_head *proc_job_head);
void ive_drv_flow_set_msg(int flag);
#endif //_IVE_DRV_FLOW_INT_H_
