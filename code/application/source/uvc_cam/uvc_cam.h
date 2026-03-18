#ifndef _UVC_CAM_H_
#include "hdal.h"
#include "kwrap/stdio.h"
#include "kwrap/type.h"
#include "kwrap/semaphore.h"
#include "kwrap/task.h"
#include "kwrap/examsys.h"
#include "kwrap/sxcmd.h"
#include <kwrap/cmdsys.h>
#include "kwrap/error_no.h"
#include <kwrap/util.h>
#include "kwrap/perf.h"
#include <pthread.h>

#define _UVC_CAM_H_
// Define Bits Containing Capabilities of the control for the Get_Info request (section 4.1.2 in the UVC spec 1.1)
#define SUPPORT_GET_REQUEST							0x01
#define SUPPORT_SET_REQUEST							0x02
#define DISABLED_DUE_TO_AUTOMATIC_MODE				0x04
#define AUTOUPDATE_CONTROL							0x08
#define ASNCHRONOUS_CONTROL							0x10
#define RESERVED_BIT5								0x20
#define RESERVED_BIT6								0x40
#define RESERVED_BIT7								0x80

#define VDO_SIZE_W_8M      3840
#define VDO_SIZE_H_8M      2160
#define VDO_SIZE_W_5M      2592
#define VDO_SIZE_H_5M      1944
#define VDO_SIZE_W_4M      2560
#define VDO_SIZE_H_4M      1440
#define VDO_SIZE_W_4M_1520 2688
#define VDO_SIZE_H_4M_1520 1520
#define VDO_SIZE_W_2M      1920
#define VDO_SIZE_H_2M      1080
#define VDO_SIZE_W_720P    1280
#define VDO_SIZE_H_720P    720

//Max sensor size
#define MAX_CAP_SIZE_W cap_size_w
#define MAX_CAP_SIZE_H cap_size_h
//Max bitstream size
#define MAX_BS_W cap_size_w
#define MAX_BS_H cap_size_h
#define MAX_BS_FPS 30

#define MJPEG_TBR_MAX   (13*3*1024*1024*8)
#define MJPEG_TBR_8M    (13*3*1024*1024)
#define MJPEG_TBR_2M    (18*1024*1024)
#define MJPEG_TBR_720P  (7*1024*1024)

#define H264_TBR_MAX    (8*1024*1024)
#define H264_TBR_2M     (8*1024*1024)
#define H264_TBR_720P   (8*1024*1024)

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
	HD_PATH_ID proc_main_path;
	HD_PATH_ID proc_sub_path;
	BOOL       proc_sub_start;

	HD_DIM  enc_main_max_dim;
	HD_DIM  enc_main_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_main_path;

	// (4) user pull
	VK_TASK_HANDLE enc_thread_id;
	UINT32         enc_exit;
	HD_VIDEO_CODEC codec_type;
	UINT32 fps;
	BOOL isYUV;

	// (5) YUV pull
	VK_TASK_HANDLE  acquire_thread_id;
	UINT32     acquire_exit;

} VIDEO_STREAM;

extern UINT32 cap_size_w;
extern UINT32 cap_size_h;

extern UINT32 flow_audio_start;
extern UINT32 encode_start;
extern UINT32 acquire_start;
extern UINT32 g_capbind;
extern UINT32 g_capbind_tmp;

extern pthread_mutex_t flow_start_lock;
extern UINT32 sen1_vcap_id;

extern VIDEO_STREAM* get_video_stream(void);
extern HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, int en_slice, HD_DIM* p_proc_dim);
extern HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 pull_allow, UINT32 b_one_buf);
extern HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, int fps, HD_DIM *p_out_dim);
extern HD_RESULT set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, UINT32 enc_type, UINT32 bitrate, UINT32 frame_rate);
extern void release_all_old_bs(VIDEO_STREAM* p_stream0);

#endif	/* _UVC_CAM_H_ */
