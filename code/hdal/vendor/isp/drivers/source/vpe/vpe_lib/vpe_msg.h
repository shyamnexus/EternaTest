#ifndef _VPE_MSG_H_
#define _VPE_MSG_H_

#include "vpe_alg_int.h"

//=============================================================================
// struct & definition
//=============================================================================

//=============================================================================
// extern functions
//=============================================================================
#if defined(__FREERTOS)
extern void vpe_msg_show_info(void);
extern void vpe_msg_show_dce_ctrl(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_sharpen(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_gdc(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_2dlut(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_2dlut_expand(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_drt(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_dctg(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_flip_rot(VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
#else
extern void vpe_msg_show_info(struct seq_file *sfile);
extern void vpe_msg_show_dce_ctrl(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_sharpen(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_gdc(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_2dlut(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_2dlut_expand(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_drt(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_dctg(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
extern void vpe_msg_show_flip_rot(struct seq_file *sfile, VPE_ID vpe_proc_id, UINT32 vpe_proc_idx);
#endif

#endif

