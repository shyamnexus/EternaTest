#ifndef _THIRD_PARTY_SAMPLE_IPC_H_
#define _THIRD_PARTY_SAMPLE_IPC_H_


#include "hdal.h"
#include "hd_debug.h"
#include "vendor_ai.h"

#include "ext_module_api.h"

#define VENDOR_AI_CFG       0x000f0000  //ai project config

#define HD_VIDEOPROC_PATH(dev_id, in_id, out_id)    (((dev_id) << 16) | (((in_id) & 0x00ff) << 8)| ((out_id) & 0x00ff))
#define VDO_FRAME_FORMAT    HD_VIDEO_PXLFMT_YUV420
#define DBGINFO_BUFSIZE()	(0x200)
#define VDO_YUV_BUFSIZE(w, h, pxlfmt)	(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

#define AI_MODEL_NUM     (5)

typedef struct _VIDEO_LIVEVIEW {
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

	HD_DIM  out_max_dim;
	HD_DIM  out_dim;

	// (3)
	HD_VIDEOOUT_SYSCAPS out_syscaps;
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;

	// (4) --
	HD_VIDEOPROC_SYSCAPS proc_alg_syscaps;
	HD_PATH_ID proc_alg_ctrl;
	HD_PATH_ID proc_alg_path;

	HD_DIM  proc_alg_max_dim;
	HD_DIM  proc_alg_dim;

	// (5) --
	HD_PATH_ID mask_alg_path;

	HD_VIDEOOUT_HDMI_ID hdmi_id;
} VIDEO_LIVEVIEW_S;

typedef struct _AI_THREAD_PARM {
	NN_CFG_BUF_M rslt_mem;
	NN_CFG_BUF_M pd_mem;
	NN_CFG_BUF_M cdd_mem;
	NN_CFG_BUF_M fd_mem;
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	NN_CFG_BUF_M md_mem;  // MD
	NN_CFG_BUF_M sw_tracker_mem;
	VIDEO_LIVEVIEW_S stream;
} AI_THREAD_PARM_S;

typedef struct _PVD_THREAD_PARM {
	NN_CFG_BUF_M rslt_mem;
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	VIDEO_LIVEVIEW_S stream;
} PVD_THREAD_PARM_S;


typedef struct _FAD_THREAD_PARM {
	NN_CFG_BUF_M pvd_mem;
	NN_CFG_BUF_M limit_fdet_mem;
	NN_CFG_BUF_M pe_mem;
	NN_CFG_BUF_M buffer_pool_mem;
    NN_CFG_BUF_M ba_mem;
	NN_CFG_BUF_M rslt_mem;
	NN_CFG_BUF_M sw_tracker_mem;
	VIDEO_LIVEVIEW_S  stream;
} FAD_THREAD_PARM_S;

typedef struct _SAMPLE_CONFIGURATION_PARAM
{
	UINT32 debug_mode;
	BOOL save_result;
	BOOL odt_enable;
	BOOL sw_tracker_enable;
	UINT32 frames_per_second;
	BOOL ai_mode[AI_MODEL_NUM];
	int max_distance_mode[3];
} SAMPLE_CFG_PARAM_S;

typedef struct _SAMPLE_BUFSIZE
{
	int pdcnn_bufsize;
	int fdcnn_bufsize;
	int cddcnn_bufsize;
	int pvdcnn_bufsize;
	int limit_fdet_bufsize;
	int pecnn_bufsize;
	int dis_bufsize;
	int odt_bufsize;
	int sw_tracker_bufsize;
	int buffer_pool_bufsize;
	int bacnn_bufsize;
	int result_bufsize;
	int md_bufsize;
	int total_bufsize;
} SAMPLE_CFG_BUFSIZE_S;

typedef struct _SAMPLE_FILE_DIR{
	NN_FILE_PATH pd_files;
	NN_FILE_PATH fd_files;
	NN_FILE_PATH cdd_files;
	NN_FILE_PATH pvd_files;
	NN_FILE_PATH limit_files;
	char fad_para_file[PATH_LENGTH_M];
	char sw_para_file[PATH_LENGTH_M];
	char pe_model_file[PATH_LENGTH_M];
	char ba_model_file[PATH_LENGTH_M];
}SAMPLE_FILE_DIR_S;

typedef struct _SAMPLE_SYSTEM_CFG_PARAM{
	INT32 enc_type;
	INT32 enc2_type;
	UINT32 shdr_mode;
	UINT32 sensor_id;
	UINT32 fps;
	UINT32 g_yuvcompress;
	UINT32 g_osg;
	int main_strem_size;
	int sub_strem_size;
	int sub2_strem_size;
	int sample_id;
} SAMPLE_SYS_CFG_S;


// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Read ODT parameter from txt file
 * 
 * @param filename		: Absolute path to parameters txt file.
 * @return VOID 
 */
int sample_read_odt(CHAR *filename);

/**
 * @brief Read absolute path of model's bin file and parameters txt file.
 * 
 * @param filename		: Input parameters txt file's absolute path.
 * @param filePaths		: Output txt file's absolute paths.
 * @param pvd_yuv_width		: PVDCNN model input image's width.
 * @param pvd_yuv_height	: PVDCNN model input image's height.
 * @param pvd_prune370		: Which PVDCNN model(1: pruning or 0: normal) choose to running.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_get_param_files(CHAR *filename, SAMPLE_FILE_DIR_S *filePaths, int pvd_yuv_width, int pvd_yuv_height, int pvd_prune370);

/**
 * @brief Read buffer configuration for all models from txt file.
 * 
 * @param filename		: Input parameters txt file's absolute path.
 * @param model_bufsize		: Output buffer config of all models.
 * @param length		: The length of model_bufsize.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_read_all_bufsize(char *filename, int *model_bufsize, int length);

/**
 * @brief Get buffer configuration for models (according to User's configuration).
 * 
 * @param p_buf_configs		: Output buffer config of models.
 * @param filename		: Input parameters txt file's absolute path.
 * @param pvd_yuv_width		: PVDCNN model input image's width.
 * @param pvd_yuv_height	: PVDCNN model input image's height.
 * @param pvd_prune370		: Choose PVDCNN model(1: pruning or 0: normal) to running.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_get_buf_cfg(SAMPLE_CFG_BUFSIZE_S *p_buf_configs, CHAR *filename, int pvd_yuv_width, int pvd_yuv_height, int pvd_prune370);

/**
 * @brief Read samples config parameters.
 * 
 * @param sm_cfig		: Output config parameters of sample.
 * @param filename		: Input parameters txt file's absolute path.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_read_sample_cfg(SAMPLE_CFG_PARAM_S* sm_cfig, char *params_file);

/**
 * @brief Get all config parameter for sample and models.
 * 
 * @param filename		: Input parameters txt file's absolute path.
 * @param p_buf_configs		: Output buffer config parameters of models.
 * @param sm_cfig		: Output config parameters of sample.
 * @param pvd_yuv_width		: PVDCNN model input image's width.
 * @param pvd_yuv_height	: PVDCNN model input image's height.
 * @param pvd_prune370		: Choose PVDCNN model(1: pruning or 0: normal) to running.
 * @param USE_MD	    	: Switch motion_detection.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_get_cfg(CHAR *filename, SAMPLE_CFG_BUFSIZE_S *p_buf_configs, SAMPLE_CFG_PARAM_S* sm_cfig, int pvd_yuv_width, int pvd_yuv_height, int pvd_prune370, int USE_MD);

/**
 * @brief Get config parameter for video stream.
 * 
 * @param filename		: Input parameters txt file's absolute path.
 * @param p_sys_cfg		: Output system config parameters.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT sample_read_system_param(char *filename, SAMPLE_SYS_CFG_S *p_sys_cfg);

#endif

