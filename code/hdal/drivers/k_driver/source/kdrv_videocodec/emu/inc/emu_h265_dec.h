#ifndef _EMU_H265_DEC_H_
#define _EMU_H265_DEC_H_

#include "kwrap/type.h"

// for hw trigger //
#include "h26x.h"

// for sw driver //
#include "h26x_def.h"
#include "h265dec_api.h"

// for emulation //
#include "emu_h26x_common.h"
#include "emu_h265_common.h"

typedef struct _h265d_pat_t_ {
	char name[128];
	unsigned int idx;
	unsigned int pic_num;

	file_t_265 file;

	unsigned int seq_obj_size;
	unsigned int pic_obj_size;
	unsigned int chk_obj_size;

	info_t_265 info;
	seq_t_265  seq;
	pic_t_265  pic;
	chk_t_265  chk;

	unsigned int rand_seed;

	h265_perf_t perf;

	uintptr_t rec_y_addr[FRM_IDX_MAX + 1];
	uintptr_t rec_uv_addr[FRM_IDX_MAX + 1];
	BOOL rec_buf_used[FRM_IDX_MAX + 1];

	uintptr_t bsdma_buf_addr; // for test dec random bsdma
	unsigned int bsdma_buf_size;// for test dec random bsdma
	unsigned int uiDecResPicBsLen;// for test dec random bsdma
	uintptr_t uiHwBsAddr;// for test dec random bsdma

	unsigned int islice_recsum; //for frame skip mode

	unsigned int  err_bs_idx;
	unsigned char org_bs_data;
	unsigned char err_bs_data;

	int frm_buf_num;
}h265d_pat_t;

typedef struct _h265d_emu_t_ {
	H26XDEC_VAR  var_obj;
	H265DEC_INIT init_obj;
	H265DEC_INFO info_obj;
} h265d_emu_t;

typedef struct _h265d_ctx_t_ {
	h265_folder_t folder;
	h265d_pat_t    pat;
	h265d_emu_t   emu;
} h265d_ctx_t;

BOOL emu_h265d_setup(h26x_ctrl_t *p_ctrl);
BOOL emu_h265d_setup_one_job(h26x_ctrl_t *p_ctrl, unsigned int start_folder_idx, unsigned int end_folder_idx,unsigned int start_pat_idx, unsigned int end_pat_idx, unsigned int end_frm_num);
BOOL  emu_h265d_prepare_one_pic(h26x_job_t *p_job, h26x_ver_item_t *p_ver_item);
BOOL emu_h265d_chk_one_pic(h26x_job_t *p_job, UINT32 interrupt, unsigned int rec_out_en, unsigned int rnd_bs_buf);
void emu_h265d_set_nxt_bsbuf(h26x_job_t *p_job);
#endif	// _EMU_H265_DEC_H_
