#ifndef _ISP_MSG_H_
#define _ISP_MSG_H_

//=============================================================================
// struct & definition
//=============================================================================

//=============================================================================
// extern functions
//=============================================================================
#if defined(__FREERTOS)
extern void isp_msg_show_info(void);
extern void isp_msg_show_ca(UINT32 isp_proc_id);
extern void isp_msg_show_la(UINT32 isp_proc_id);
extern void isp_msg_show_va(UINT32 isp_proc_id);
extern void isp_msg_show_ca_acc_cnt(UINT32 isp_proc_id);
extern void isp_msg_show_histo(UINT32 isp_proc_id);
#else
extern void isp_msg_show_info(struct seq_file *sfile);
extern void isp_msg_show_ca(struct seq_file *sfile, UINT32 isp_proc_id);
extern void isp_msg_show_la(struct seq_file *sfile, UINT32 isp_proc_id);
extern void isp_msg_show_va(struct seq_file *sfile, UINT32 isp_proc_id);
extern void isp_msg_show_ca_acc_cnt(struct seq_file *sfile, UINT32 isp_proc_id);
extern void isp_msg_show_histo(struct seq_file *sfile, UINT32 isp_proc_id);
#endif

#endif

