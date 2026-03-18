#ifndef _FLOW_UVC_H
#define _FLOW_UVC_H

#include "kwrap/task.h"
#include <pthread.h>			//for pthread API
#include <FreeRTOS_POSIX.h>
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <hdal.h>

#define HID_FUNC                   0
#define CT_PU_FUNC                 0

//Max sensor size
#define MAX_CAP_SIZE_W cap_size_w
#define MAX_CAP_SIZE_H cap_size_h
//Max bitstream size
#define MAX_BS_W cap_size_w
#define MAX_BS_H cap_size_h
#define MAX_BS_FPS 30

//UVC Extension Unit Control Selectors
#define EU_CONTROL_ID_01							0x01
#define EU_CONTROL_ID_02							0x02
#define EU_CONTROL_ID_03							0x03
#define EU_CONTROL_ID_04							0x04
#define EU_CONTROL_ID_05							0x05
#define EU_CONTROL_ID_06							0x06
#define EU_CONTROL_ID_07							0x07
#define EU_CONTROL_ID_08							0x08
#define EU_CONTROL_ID_09							0x09
#define EU_CONTROL_ID_10							0x0A
#define EU_CONTROL_ID_11							0x0B
#define EU_CONTROL_ID_12							0x0C
#define EU_CONTROL_ID_13							0x0D
#define EU_CONTROL_ID_14							0x0E
#define EU_CONTROL_ID_15							0x0F
#define EU_CONTROL_ID_16							0x10

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

	HD_DIM  enc_main_max_dim;
	HD_DIM  enc_main_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_main_path;

	// (4) user pull
	VK_TASK_HANDLE enc_thread_id;
	UINT32         enc_exit;
	HD_VIDEO_CODEC codec_type;

	// (5) YUV pull
	VK_TASK_HANDLE  acquire_thread_id;
	UINT32     acquire_exit;
#if UVC_SHUTTER
	// shutter mask
	HD_PATH_ID vp_shutter_path;
#endif
} VIDEO_STREAM;

extern UINT32 cap_size_w;
extern UINT32 cap_size_h;

extern UINT32 flow_audio_start;
extern UINT32 encode_start;
extern UINT32 acquire_start;
extern UINT32 g_capbind;
extern BOOL shutterFlag;

extern pthread_mutex_t flow_start_lock;

int flow_uvc(void);
extern VIDEO_STREAM* get_video_stream(void);
extern HD_RESULT set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, int en_slice);
extern HD_RESULT set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, HD_VIDEO_PXLFMT pxlfmt, UINT32 pull_allow, UINT32 b_one_buf);
extern HD_RESULT set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, int fps, HD_DIM *p_out_dim);

#endif
