#ifndef _MD_DRV_FLOW_INT_H_
#define _MD_DRV_FLOW_INT_H_

#include "md_drv_ctl_int.h"
#include "md_drv_ll_int.h"
#include "md_eng.h"

#define MD_MAX_LL_CMD_NUM (1)

int md_drv_flow_proc(u16 chip_id, u16 eng_id, MD_ENG_HANDLE *eng_hdl, struct md_drv_job_cfg *job_cfg, struct md_drv_ll_blk *proc_ll_blk);
int md_drv_flow_isr_callback(MD_ENG_HANDLE *eng_hdl, struct md_drv_job_cfg *job_cfg);
int md_drv_flow_dump_job_cfg(struct md_drv_job_cfg *job_cfg, int dump_flag);
unsigned int md_drv_flow_predict_bscore(struct md_drv_job_head *proc_job_head);
int md_drv_flow_sw_balance_proc(struct md_drv_job_head *proc_job_head);
void md_drv_flow_set_msg(int flag);
#endif //_MD_DRV_FLOW_INT_H_
