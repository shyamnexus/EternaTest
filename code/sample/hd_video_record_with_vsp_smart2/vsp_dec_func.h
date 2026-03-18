#ifndef _VSP_4DEC_FUNC_H
#define _VSP_4DEC_FUNC_H

typedef struct _VIDEO_RECORD {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;
	HD_PATH_ID proc_path2;

	HD_DIM     out_max_dim;
	HD_DIM     out_dim;
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl[2];
	HD_PATH_ID out_path[2];
	HD_VIDEOOUT_HDMI_ID hdmi_id;

	// (3)
	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;
	UINT32     enc_exit;

    // (4) vsp
	UINTPTR fusion_blk[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM];
	UINTPTR fusion_pa[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM];
	UINTPTR fusion_va[VENDOR_VIDEOPROC_VSP_MAX_FRM_UM];
	UINTPTR fusion_work_buf_blk;
	UINTPTR fusion_work_buf_pa;
	UINT32  isp_id;
	UINT32  vpe_isp_id;
	UINT32  ptz_isp_id;	//vsp need second isp_id for ptz parameters, ptz_isp_id must = isp_id+1

	// (5) user pull
	pthread_t  vsp_thread_id;
	pthread_t  pull_thread_id;
	pthread_t  vpe_pull_thread_id;
	pthread_t  gfx_scale_thread_id0;
	pthread_t  gfx_scale_thread_id1;
	pthread_t  enc_thread_id;
	pthread_t  vout_push_thread_id;
	UINT32     proc_exit;
	UINT32     flow_start;
	INT32      wait_ms;

	// (6) vdec
	pthread_t            feed_thread_id;

	HD_VIDEODEC_SYSCAPS  dec_syscaps;
	HD_PATH_ID           dec_path;
	HD_DIM               dec_max_dim;
	UINT32               dec_type;
	UINT32    			 dec_exit;
	char 				 bs_name[128];
	char 				 bs_len[128];
} VIDEO_RECORD;

extern UINT32 g_fps;
extern UINT8 g_folder_num;
extern UINT32 g_vdec_num;

///////////////////////////////////////////////////////////////

HD_RESULT open_module_all_dec(VIDEO_RECORD *p_stream, UINT32 proc_max_dim_w, UINT32 proc_max_dim_h, UINT32 vdec_num);
HD_RESULT close_module_all_dec(VIDEO_RECORD *p_stream, UINT32 vdec_num);
HD_RESULT set_all_dec_cfg(VIDEO_RECORD *p_stream, UINT32 dec_max_dim_w, UINT32 dec_dim_h, UINT32 vdec_num);
BOOL check_test_pattern(VIDEO_RECORD *p_stream, UINT32 dec_type);
HD_RESULT create_all_dec_thread(VIDEO_RECORD *p_stream, UINT32 vdec_num);

//////////////////////////////////////////////////////////////

HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id, UINT32 isp_id, UINT32 ref_3dnr_id, UINT32 in_plxfmt);

#endif
