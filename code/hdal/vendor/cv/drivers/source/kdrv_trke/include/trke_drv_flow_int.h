#ifndef _TRKE_DRV_FLOW_INT_H_
#define _TRKE_DRV_FLOW_INT_H_

#include "trke_drv_ctl_int.h"
#include "trke_drv_ll_int.h"
#include "trke_eng.h"

#define TRKE_MAX_LL_CMD_NUM (51 * TRKE_DRV_LAYER_MAX_NUM)

int trke_drv_flow_proc(u16 chip_id, u16 eng_id, TRKE_ENG_HANDLE *eng_hdl, struct trke_drv_job_cfg *job_cfg, struct trke_drv_ll_blk *proc_ll_blk);
int trke_drv_flow_isr_callback(TRKE_ENG_HANDLE *eng_hdl, struct trke_drv_job_cfg *job_cfg);
int trke_drv_flow_dump_job_cfg(struct trke_drv_job_cfg *job_cfg, int dump_flag);
unsigned int trke_drv_flow_predict_bscore(struct trke_drv_job_head *proc_job_head);
int trke_drv_flow_sw_balance_proc(struct trke_drv_job_head *proc_job_head);
void trke_drv_flow_set_msg(int flag);
#endif //_TRKE_DRV_FLOW_INT_H_
