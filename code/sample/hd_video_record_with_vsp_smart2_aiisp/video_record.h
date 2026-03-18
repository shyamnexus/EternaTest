#ifndef _VIDEO_RECORD_H_
#define _VIDEO_RECORD_H_
#include "hd_type.h"
#include "vendor_videoprocess.h"

#define IME_POSTSHARPEN_ENABLE     1
#define AI_ENABLE        1

#define NET_PATH_ID		UINT32


typedef struct _NET_IN_CONFIG {
	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	UINT32 type;
	UINT32 batch;
	UINT32 time;
} NET_IN_CONFIG;

typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
	CHAR label_filename[256];

} NET_PROC_CONFIG;

typedef enum {
	VPRC_OUTPUT_VOUT,
	VPRC_OUTPUT_MAX,
	ENUM_DUMMY4WORD(VPRC_OUTPUT)
} VPRC_OUTPUT;

typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;

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
	#if (IME_POSTSHARPEN_ENABLE)
	HD_PATH_ID proc_path_3dnr;
	#endif

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

	// (4) network
	NET_PROC_CONFIG net_proc_cfg;
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID net_path;
	NET_PATH_ID in_path;

	//ai sync - begin
	pthread_t  net_proc_thread_id;
	pthread_t  net_proc_sub_thread_id;
	UINT32 net_proc_start;
	UINT32 net_proc_exit;
	UINT32 net_proc_oneshot;
	pthread_t  net_cap_thread_id;
	//ai sync - end

	//ai async - begin
	pthread_t  net_push_thread_id;
	UINT32 net_push_start;
	UINT32 net_push_exit;
	UINT32 net_push_oneshot;
	pthread_t  net_pull_thread_id;
	UINT32 net_pull_start;
	UINT32 net_pull_exit;
	UINT32 net_pull_oneshot;
	#define NET_QUEUE_DEPTH 2
	UINT32 net_queue_in_id; //ready to alloc before push in
	UINT32 net_queue_out_id; //ready to free after pull out
	UINT32 net_queue_cnt; //count
	HD_VIDEO_FRAME net_queue[NET_QUEUE_DEPTH+1];
	//ai async - end

	uintptr_t ai_cb;  //isp call to ai
	uintptr_t isp_cb;  //ai call to isp
	UINT32 shot_count;
	BOOL fix_pattern;
	BOOL dump_yuv;
	VPRC_OUTPUT vprc_output;
	pthread_t    socket_thread_id;


} VIDEO_RECORD;

extern HD_RESULT input_init(void);
extern HD_RESULT input_uninit(void);
extern HD_RESULT network_init(void);
extern HD_RESULT network_uninit(void);
extern INT32 network_mem_config(NET_PATH_ID net_path, void* p_cfg);
extern HD_RESULT network_open(NET_PATH_ID net_path);
extern HD_RESULT network_close(NET_PATH_ID net_path);
extern HD_RESULT network_bind_cb(VIDEO_RECORD *p_stream);
extern HD_RESULT network_bind_isp_cb(VIDEO_RECORD *p_stream);
extern HD_RESULT network_set_isp_ai_cfg(VIDEO_RECORD *p_stream, UINT32 path_id, BOOL enable);
extern HD_RESULT network_set_buf_by_in_path_list(VIDEO_RECORD *p_stream, UINT32 para_id);
extern HD_RESULT network_start(VIDEO_RECORD *p_stream);
extern HD_RESULT network_stop(VIDEO_RECORD *p_stream);

#endif
