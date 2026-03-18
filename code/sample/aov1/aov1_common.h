#ifndef _AOV1_COMMON_H_
#define _AOV1_COMMON_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>			//for pthread API
#include <hdal.h>
#include <hd_logger.h>
#include <hd_debug.h>

#include "aov1_filesave.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#define msleep(x)    			usleep(1000*(x))
#define MASK_NUMBER 2

// 1fps or 3fps control
#define PD_USE_GPIO 0
#define PD_USE_RTC 1
#define MULTI_SENSOR (0)

#if (MULTI_SENSOR == 1)
#define SENSOR_MAX_NUM (2)
#else
#define SENSOR_MAX_NUM (1)
#endif

#define DDR_ID DDR_ID0

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

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pullg_md_pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flag_ai_start;
	UINT32     flag_enc_start;

	pthread_t  prc_thread_id;
	UINT32     flag_prc_start;
	UINT32     prc_exit;
	UINT32     prc_loop;
	UINT32     prc_count;

	//MD 
	HD_DIM  proc_alg_max_dim;
	HD_PATH_ID proc_alg_path;
	UINT32     flag_md_pull;

	HD_DIM  proc_pvd_max_dim;
	HD_PATH_ID proc_pvd_path;

	FILESAVE_CTX *fs_ctx;

	HD_PATH_ID enc_mask_path[MASK_NUMBER];
} VIDEO_RECORD;

typedef enum _SYSTEM_STATUS {
	NORMAL_MODE                =  0,
	LOW_POWER_MODE                =  1,
	SYSTEM_MODE_MAX,
	ENUM_DUMMY4WORD(SYSTEM_STATUS)
} SYSTEM_STATUS;

#endif	// _AOV1_COMMON_H_

