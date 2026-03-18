#ifndef _GRAPH_DRV_FLOW_INT_H_
#define _GRAPH_DRV_FLOW_INT_H_

#include "grph_drv_ctl_int.h"

int graph_drv_flow_proc(u16 chip_id, u16 eng_id, struct graph_drv_job_cfg *job_cfg, unsigned long *eng_st, unsigned long *eng_ed, struct graph_eng_counter *counter);
int graph_drv_flow_dump_job_cfg(struct graph_drv_job_cfg *job_cfg);
unsigned int graph_drv_flow_predict_bscore(struct graph_drv_job_head *proc_job_head);
int graph_drv_flow_sw_balance_proc(struct graph_drv_job_head *proc_job_head);
void graph_drv_flow_set_msg(int flag);
int graph_drv_flow_init(u16 total_ch);
int graph_drv_flow_uninit(u16 total_ch);

#endif //_GRAPH_DRV_FLOW_INT_H_
