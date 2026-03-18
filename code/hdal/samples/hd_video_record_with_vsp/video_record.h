#ifndef _VIDEO_RECORD_H_
#define _VIDEO_RECORD_H_
#include "hd_type.h"
#include "vendor_videoprocess.h"
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
	pthread_t  sde_thread_id0;
	pthread_t  sde_thread_id1;
	UINT32     proc_exit;
	UINT32     flow_start;
	INT32      wait_ms;

	
} VIDEO_RECORD;


#endif
