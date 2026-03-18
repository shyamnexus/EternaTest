#ifndef __CTL_IPP_EXAM_INT_H__
#define __CTL_IPP_EXAM_INT_H__

#define CTL_IPP_EXAM_HDL_MAX 			4
#define CTL_IPP_EXAM_VB_INFO_LEN		32

typedef struct {
	void *fp;	// CTL_IPP_EXAM_MAIN_FP
	UINT32 hdl_num;
	CTL_IPP_FLOW_TYPE flow[CTL_IPP_EXAM_HDL_MAX];
	UINT32 d2d_hdl_num;	// use for d2d + direct test
	CTL_IPP_SIE_ID sie_id[CTL_IPP_DIR_INFO_NUM];
	USIZE in_size;
	USIZE in_size_rtc;
	URECT in_crop;
	VDO_PXLFMT in_fmt;
	BOOL out_en[CTL_IPP_OUT_PATH_ID_MAX];
	URECT out_pre_crp[CTL_IPP_OUT_PATH_ID_MAX];
	USIZE out_size[CTL_IPP_OUT_PATH_ID_MAX];
	URECT out_post_crp[CTL_IPP_OUT_PATH_ID_MAX];
	VDO_PXLFMT out_fmt[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 fps;
	BOOL _3dnr_en;
	BOOL pm_en;
	BOOL ai_en;
	CTL_IPP_IPP_NNISP_MODE nnisp_mode;
} CTL_IPP_EXAM_SCPT_MAIN_INFO;

typedef INT32 (*CTL_IPP_EXAM_MAIN_FP)(CTL_IPP_EXAM_SCPT_MAIN_INFO *info);

typedef struct {
	UINT32 cnt;
	UINT32 size;
} CTL_IPP_EXAM_MEM_INFO;

typedef struct {
	CHAR	filename[128];
	UINT32	width;
	UINT32	height;
	UINT32	lofs;
	UINT32	enc_rate;
	UINT32 	frm_idx;
	VDO_PXLFMT fmt;
	CTL_IPP_BUF_ADDR addr;
	UINT32 	size;
} CTL_IPP_EXAM_IMGFILE;

typedef enum {
	CTL_IPP_EXAM_SAVE_RST_STS_NONE = 0,
	CTL_IPP_EXAM_SAVE_RST_STS_START,
	CTL_IPP_EXAM_SAVE_RST_STS_PUSH,
	CTL_IPP_EXAM_SAVE_RST_STS_END,
} CTL_IPP_EXAM_SAVE_RST_STS;

typedef INT32 (*CTL_IPP_EXAM_TRIG_FP)(UINT32 id);

typedef struct {
	UINT32 id;										// handle id (0/1/...)
	ULONG ipp_hdl;									// ctl_ipp handle (CTL_IPP_HANDLE)
	UINT32 eng_id;									// direct mode ipp engine id
	CTL_IPP_SIE_ID sie_id[CTL_IPP_DIR_INFO_NUM];	// sie id for all direct info (patgen)
	CHAR* desc;										// handle descrption
	UINT32 frm_num;									// shdr frame number
	UINT32 comb_num;								// sensor combine mode number
	CTL_IPP_SIE_ID sie_sync_trig_main;				// main sie id for sync trig multiple sie
	UINT32 sie_sync_trig_bit;						// sie id bit for trigger multiple sie synchronously
	VDO_FRAME vdofrm[CTL_IPP_HDR_MAX_FRAME_NUM];	// vdo frame for all plane
	UINT32 buf_id[CTL_IPP_OUT_PATH_ID_MAX];			// id for output buffer
	CTL_IPP_BUF_INFO *private_buf;					// private buffer blk
	USIZE path_size[CTL_IPP_OUT_PATH_ID_MAX];		// yuv push out size (all path)
	CTL_IPP_OUT_BUF_INFO buf_info[CTL_IPP_OUT_PATH_ID_MAX]; // yuv push out buffer info (all path)
	THREAD_HANDLE tsk_id;							// exam task id
	TIMER_ID timer_id;								// exam timer cb id
	BOOL trig_en;									// exam task trigger enable
	UINT32 trig_cnt_cur;							// exam task & direct cb current trigger count
	UINT32 trig_cnt_max;							// exam task & direct cb max trigger count
	UINT32 trig_delay_us;							// exam task trigger interval
	CTL_IPP_EXAM_SAVE_RST_STS save_rst[CTL_IPP_OUT_PATH_ID_MAX];	// auto save result when buffer push cb
	UINT32 save_rst_frm_cnt;
	UINT32 in_push_num;								// input frame push number
	UINT32 in_start_num;							// input frame start number
	UINT32 in_done_num;								// input frame done number
	UINT32 in_drop_num;								// input frame drop number
	UINT32 dir_in_done_num;							// direct mode input done number
	UINT32 dir_in_drop_num;							// direct mode input drop number
	UINT32 out_new_num[CTL_IPP_OUT_PATH_ID_MAX];	// output buffer new number
	UINT32 out_rel_num[CTL_IPP_OUT_PATH_ID_MAX];	// output buffer done number
	UINT32 prv_trig_cnt_cur;						// previous trig_cnt_cur
	UINT32 prv_in_push_num;							// previous in_push_num
	UINT32 prv_in_done_num;							// previous in_done_num
	UINT32 prv_in_drop_num;							// previous in_drop_num
	UINT32 prv_out_new_num[CTL_IPP_OUT_PATH_ID_MAX];// previous out_new_num
	UINT32 prv_out_rel_num[CTL_IPP_OUT_PATH_ID_MAX];// previous out_rel_num
	CTL_IPP_EXAM_TRIG_FP fp_trig;					// trigger function
	BOOL low_delay_en[CTL_IPP_OUT_PATH_ID_MAX];		// low delay enable
	BOOL dir_slice_en[CTL_IPP_OUT_PATH_ID_MAX];		// direct mode slice enable
	UINT32 dir_slice_cnt;							// direct mode slice count
	BOOL two_dir_en;								// sie send 2 engine's vd interleaved
	UINT32 bind_hdl_id[CTL_IPP_OUT_PATH_ID_MAX];
} CTL_IPP_EXAM_HDL;

typedef struct {
	CHAR filename[128];
	BOOL rlock;
	BOOL wlock;
	CTL_IPP_BUF_INFO *p_buf_info;
} CTL_IPP_EXAM_CHK_INFO;

typedef struct {
	UINT32 flow;
	UINT32 _3dnr_en;
	UINT32 pxl_en;
	UINT32 in_width;
	UINT32 in_height;
	UINT32 in_fmt;
	UINT32 out_en[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 out_width[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 out_height[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 out_fmt[CTL_IPP_OUT_PATH_ID_MAX];
	UINT32 check_output;	// read real image and check output result
} CTL_IPP_EXAM_D2D_STRP_SIZE_FIX_INFO;

typedef enum {
	CTL_IPP_EXAM_OFILE_OPT_TAR,
	CTL_IPP_EXAM_OFILE_OPT_CHK,
} CTL_IPP_EXAM_OFILE_OPT;

typedef struct {
	UINT32 total_cnt;
	UINT32 pass_cnt;
	UINT32 fail_cnt;
	UINT32 skip_cnt;
} CTL_IPP_EXAM_CHK_RSLT;

typedef enum {
	CTL_IPP_EXAM_CHK_RSLT_TYPE_PASS,
	CTL_IPP_EXAM_CHK_RSLT_TYPE_FAIL,
	CTL_IPP_EXAM_CHK_RSLT_TYPE_SKIP,
} CTL_IPP_EXAM_CHK_RSLT_TYPE;

typedef struct {
	UINT32 sie_sync_trig_bit;	// bits of started sie
} CTL_IPP_EXAM_TRIG_SIE_INFO;

extern CTL_IPP_BUF_INFO *ctl_ipp_exam_buf_get(UINT32 buf_size, BOOL print, CHAR *info);
extern INT32 ctl_ipp_exam_buf_lock(CTL_IPP_BUF_INFO *p_buf_info, BOOL print);
extern INT32 ctl_ipp_exam_buf_unlock(CTL_IPP_BUF_INFO *p_buf_info, BOOL print);
extern UINT32 ctl_ipp_exam_buf_get_lock_num(CTL_IPP_BUF_INFO *p_buf_info);
extern THREAD_DECLARE(ctl_ipp_exam_tsk_job, p1);
extern void ctl_ipp_exam_job_timer_cb(UINT32 id, ULONG event);

extern INT32 ctl_ipp_exam_set_input_img_by_vdoframe(UINT32 id, VDO_FRAME *p_vdofrm);

extern INT32 ctl_ipp_exam_in_buf_cb0(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_in_buf_cb1(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_in_buf_cb2(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_in_buf_cb3(UINT32 msg, void *in, void *out);

extern INT32 ctl_ipp_exam_out_buf_cb0(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_out_buf_cb1(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_out_buf_cb2(UINT32 msg, void *in, void *out);
extern INT32 ctl_ipp_exam_out_buf_cb3(UINT32 msg, void *in, void *out);

extern void ctl_ipp_exam_job_timer_cb0(ULONG event);
extern void ctl_ipp_exam_job_timer_cb1(ULONG event);
extern void ctl_ipp_exam_job_timer_cb2(ULONG event);
extern void ctl_ipp_exam_job_timer_cb3(ULONG event);

extern INT32 ctl_ipp_exam_trig(UINT32 id);
extern INT32 ctl_ipp_exam_dump(UINT32 id);
extern INT32 ctl_ipp_exam_dump_mem(void);

#endif
