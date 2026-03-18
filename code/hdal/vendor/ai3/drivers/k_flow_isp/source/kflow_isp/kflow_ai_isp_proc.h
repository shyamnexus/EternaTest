/**
	@brief Header file of debug definition of vendor net flow sample.

	@file kflow_ai_isp_proc.h

	@ingroup kflow ai net proc header file

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _KFLOW_AI_ISP_PROC_H_
#define _KFLOW_AI_ISP_PROC_H_



int kflow_ai_isp_proc_create(void);
void kflow_ai_isp_proc_remove(void);
extern int kflow_ai_isp_dbg_cal_slice(NN_ISP_FRAME* isp_input_frame) ; 
extern int kflow_ai_isp_dbg_push_frame(VENDOR_AIS_ISP_INPUT_INFO* isp_input_info); 
extern int kflow_ai_isp_dbg_set_ref_frame(VENDOR_AIS_ISP_INPUT_INFO* isp_input_info);
extern int kflow_ai_isp_dbg_set_isp_param(NN_ISP_ISP_PARAM* isp_param);

#endif  /* _KFLOW_AI_ISP_PROC_H_ */
