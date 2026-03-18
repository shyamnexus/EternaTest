/********************************************************************
	INCLUDE FILES

********************************************************************/
#ifndef _THIRD_PARTY_PVD_LIB_H_
#define _THIRD_PARTY_PVD_LIB_H_
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "hdal.h"
#include "hd_type.h"
#include "hd_common.h"

#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include "hd_gfx.h"
#include "vendor_ai.h"
#include "kflow_ai_net/kflow_ai_net.h"
#include "vendor_ai_net/nn_net.h"
#include <arm_neon.h>

#include "ext_module_api.h"

// only support PVD model input size: 608x608, 512x512, 416x416, 320x320 
#define PVD_YUV_WIDTH            (512) //416 //320 //608
#define PVD_YUV_HEIGHT           (512) //416 //320 //608
#define PRUNE37PVD               ENABLE
#define LIMIT_FDET_PVD           ENABLE
// #define PVDCNN_MAX_OUTNUM        (50)

#define PVD_VERSION_A  "3"
#define PVD_VERSION_B  "1"
#define PVD_VERSION_C  "6"

enum PVDCNN_DET_SW {
	PVDCNN_PD_DET_SW = 1,
	PVDCNN_VD_DET_SW = 2,
	PVDCNN_ND_DET_SW = 4
};


typedef struct _PVD_ADJUSTABLE_PARAM {
	FLOAT  nms_threshold;
	FLOAT  confindence_threshold;
	INT32  pd_dynamic_limit_module;
	INT32  vd_dynamic_limit_module;
	INT32  nd_dynamic_limit_module;
	INT32  pd_dynamic_sm_thr_num;
	INT32  vd_dynamic_sm_thr_num;
	INT32  nd_dynamic_sm_thr_num;
	FLOAT  cls_threshold;
	INT32 TRACKER_ENBALE;
	INT32  TRACKER_MAX_AGE;
	INT32  TRACKER_MIN_HITS;
	FLOAT  TRACKER_IOU_THRESHOLD;
	INT32  TRACKER_WAKE_FRAMES;
	INT32 debug_info;
} PVD_ADJUSTABLE_PARAM;


// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Print out PVDCNN library version
 * 
 * @return VOID 
 */
VOID pvdcnn_get_version(VOID);


/**
 * @brief Initialize turnkey PVDCNN.
 * 
 * @param pvd_buf				: Pre-allocated buffer space for PVDCNN to utilize
 * @param run_id 				: Network run id for ai2 engine.
 * @param PVD_MAX_DISTANCE_MODE : 0: No maximum distance mode, 1/2/3: Maximum distance mode with different inference time.
 * @param shared_model_mem 		: Mem address of preloaded model bin, pass in NULL if no use for it.
 * @param shared_io_mem 		: Mem address of shared io memory, pass in NULL if no use for it.
 * @param model_bin_path		: Absolute path to model bin file.
 * @param param_path			: Absolute path to PVDCNN parameters txt file.
 * @param debug_enable			: Enable debug print or yet.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_init(NN_CFG_BUF_M pvd_buf, UINT32 run_id, UINT PVD_MAX_DISTANCE_MODE, NN_CFG_BUF_M *shared_model_mem, NN_CFG_BUF_M *shared_io_mem, NN_FILE_PATH *pvd_files, UINT32 debug_enable);


/**
 * @brief Un-initialize turnkey PVDCNN
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_uninit(NN_CFG_BUF_M pvd_buf);


/**
 * @brief Turnkey PVDCNN process, make sure to call pvdcnn_init() before running this process.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param p_vendor_img 			: Image information for turnkey PVDCNN to process (YUV)
 * @param limit_fdet_buf 		: If given address is not NULL or the buffer size > 0, it will perform limit_fdet process during PVDCNN.
 * @param pvd_det_output		: Detection output bounding box information
 * @param pvd_det_num			: Detection output bounding box total number
 * @param PVDCNN_MODE			: Define PVDCNN output categories, refer to PVDCNN_DET_SW
 * 								  Ex. PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW = Get all detection output
 * 								  Ex. PVDCNN_PD_DET_SW                                       = Only get person detection output
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_process(NN_CFG_BUF_M pvd_buf, void* p_vendor_img, NN_CFG_BUF_M *limit_fdet_buf, NN_RESULT_M* pvd_det_output, INT32* pvd_det_num, INT32 PVDCNN_MODE);


/**
 * @brief Turnkey PVDCNN process, make sure to call pvdcnn_init() before running this process.
 * 		  NOTE: User DO NOT need to allocate an external buffer space for this function, the returned pointer address *pvd_det_output is the address of internal buffer of PVDCNN lib.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param p_vendor_img 			: Image information for turnkey PVDCNN to process (YUV)
 * @param limit_fdet_buf 		: If given address is not NULL or the buffer size > 0, it will perform limit_fdet process during PVDCNN.
 * @param pvd_det_output 		: Detection output bounding box information
 * 								  NOTE: Returned pointer address is pointed to the internal buffer of PVDCNN lib
 * @param pvd_det_num 			: Detection output bounding box total number
 * @param PVDCNN_MODE 			: Define PVDCNN output categories, refer to PVDCNN_DET_SW
 * 								  Ex. PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW = Get all detection output
 * 								  Ex. PVDCNN_PD_DET_SW                                       = Only get person detection output
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_process_inplace(NN_CFG_BUF_M pvd_buf, void* p_vendor_img, NN_CFG_BUF_M *limit_fdet_buf, NN_RESULT_M** pvd_det_output, INT32* pvd_det_num, INT32 PVDCNN_MODE);


/**
 * @brief Turnkey PVDCNN process, make sure to call pvdcnn_init() before running this process.
 * 		  NOTE: User DO NOT need to allocate an external buffer space for this function, the returned pointer address *pvd_det_output is the address of internal buffer of PVDCNN lib.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param p_vendor_img 			: Image information for turnkey PVDCNN to process (YUV)
 * @param limit_fdet_buf 		: If given address is not NULL or the buffer size > 0, it will perform limit_fdet process during PVDCNN.
 * @param roi_crop_buf 		    : If given address is not NULL or the buffer size > 0, it will perform GFX crop during PVDCNN.
 * @param pvd_det_output 		: Detection output bounding box information
 * 								  NOTE: Returned pointer address is pointed to the internal buffer of PVDCNN lib
 * @param pvd_det_num 			: Detection output bounding box total number
 * @param PVDCNN_MODE 			: Define PVDCNN output categories, refer to PVDCNN_DET_SW
 * 								  Ex. PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW = Get all detection output
 * 								  Ex. PVDCNN_PD_DET_SW                                       = Only get person detection output
 * @param roi_list              : The regions list in image for pvd model to detection
 * @param roi_num               : The number of regions in image for pvd model to detection
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_process_inplace_roi(NN_CFG_BUF_M pvd_buf, void* p_vendor_img, NN_CFG_BUF_M *limit_fdet_buf, 
									NN_CFG_BUF_M *roi_crop_buf, NN_RESULT_M** pvd_det_output, INT32* pvd_det_num, 
									INT32 PVDCNN_MODE, NN_F_RECT* roi_list, INT32 roi_num);


/**
 * @brief Turnkey PVDCNN process, make sure to call pvdcnn_init() before running this process.
 * 		  NOTE: User DO NOT need to allocate an external buffer space for this function, the returned pointer address *pvd_det_output is the address of internal buffer of PVDCNN lib.
 * 		  NOTE: This function will only output at max 1 PD result
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param p_vendor_img 			: Image information for turnkey PVDCNN to process (YUV)
 * @param limit_fdet_buf 		: If given address is not NULL or the buffer size > 0, it will perform limit_fdet process during PVDCNN. 
 * @param pvd_det_output 		: Detection output bounding box information
 * 								  NOTE: Returned pointer address is pointed to the internal buffer of PVDCNN lib
 * @param pvd_det_num 			: Detection output bounding box total number
 * @param PVDCNN_MODE 			: Define PVDCNN output categories, refer to PVDCNN_DET_SW
 * 								  Ex. PVDCNN_PD_DET_SW | PVDCNN_VD_DET_SW | PVDCNN_ND_DET_SW = Get all detection output
 * 								  Ex. PVDCNN_PD_DET_SW                                       = Only get person detection output
 * @param max_roi_num 			: Limit fdet will only perform up to N times bbox check, N = max_roi_num
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_process_inplace_getone_pd(NN_CFG_BUF_M pvd_buf, void* p_vendor_img, NN_CFG_BUF_M *limit_fdet_buf, NN_RESULT_M** pvd_det_output, INT32* pvd_det_num, INT32 PVDCNN_MODE, UINT32 max_roi_num);


/**
 * @brief Get the last pvdcnn_process() function call result, without doing network inference flow.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param core_mask 		    : Specify core mask here.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_bind_core(NN_CFG_BUF_M pvd_buf, UINT32 core_mask);


/**
 * @brief Get the last pvdcnn_process() function call result, without doing network inference flow.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param pvd_det_output 		: Detection output bounding box information (based on latest pvdcnn_process() function call output)
 * 								  NOTE: Returned pointer address is pointed to the internal buffer of PVDCNN lib
 * @param pvd_det_num 			: Detection output bounding box total number (based on latest pvdcnn_process() function call output)
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_get_results(NN_CFG_BUF_M pvd_buf, NN_RESULT_M** pvd_det_output, INT32* pvd_det_num);


/**
 * @brief Get the last pvdcnn_process() function call result, without doing network inference flow.
 *        This function gets the raw pvd detection result without filtering or stabilization mechanism
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param pvd_raw_det			: Detection output bounding box information (based on latest pvdcnn_process() function call output)
 * 								  NOTE: Returned pointer address is pointed to the internal buffer of PVDCNN lib
 * @param pvd_det_num 			: Detection output bounding box total number (based on latest pvdcnn_process() function call output)
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_get_raw_det_results(NN_CFG_BUF_M pvd_buf, NN_RESULT_M** pvd_raw_det, INT32* pvd_det_num);


/**
 * @brief Get the current deploying PVDCNN lib parameters 
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param params 				: Returned struct member contains value of current deploying parameters
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_params_get(NN_CFG_BUF_M pvd_buf, PVD_ADJUSTABLE_PARAM* params);


/**
 * @brief Set and update PVDCNN lib parameters 
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param params 				: Please specify the required parameters to adjustment.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_params_set(NN_CFG_BUF_M pvd_buf, PVD_ADJUSTABLE_PARAM* params);


/**
 * @brief Set and update PVDCNN lib experimental parameters 
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init()
 * @param param_choice 			: Please specify the parameters choice to adjustment.
 * @param set_param_value		: The value to be set
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_exp_params_set(NN_CFG_BUF_M pvd_buf, INT param_choice, FLOAT set_param_value);


/**
 * @brief Check PVDCNN lib roilist parameters 
 * 
 * @param need_gfx_buf 			: If or not need buffer for roilist to scale regions.
 * @param det_roi_num 			: The number of regions of interest.
 * @param roi_list				: The list of regions of interest.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_roilist_paras_check(BOOL *need_gfx_buf, UINT32 det_roi_num, NN_F_RECT* roi_list);


/**
 * @brief Set the individual maximum detection num for each PVD category.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init().
 * @param category_id 			: 1 - Person, 2 - Vehichle, 3 - motor, setting any other id won't do anything.
 * @param max_det_num 			: Set maximum detection num for specfied category_id.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_set_category_max_det(NN_CFG_BUF_M pvd_buf, UINT32 category_id, UINT32 max_det_num);


/**
 * @brief Set the individual confidence threshold for each PVD category.
 * 
 * @param pvd_buf 				: Pre-allocated buffer space for PVDCNN to utilize, make sure mem parameters are the same as pvdcnn_init().
 * @param category_id 			: 1 - Person, 2 - Vehichle, 3 - motor, setting any other id won't do anything.
 * @param threshold 			: Set detection confidence threshold for specfied category_id.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_set_category_conf_threshold(NN_CFG_BUF_M pvd_buf, UINT32 category_id, FLOAT threshold);


/**
 * @brief Parse the required working buffer size for PVDCNN
 * 
 * @param model_buf 			: Model must have been already loaded into this buffer space.
 * @param workbuf_size 			: (return) PVDCNN working buffer require size.
 * @param pvd_yuv_width 		: Specify PVD input size choice, it should match the given model in the buffer.
 * @param pvd_yuv_height 		: Specify PVD input size choice, it should match the given model in the buffer.
 * @param PVD_MAX_DISTANCE_MODE : 0: No maximum distance mode, 1/2/3: Maximum distance mode with different inference time.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_parse_workbuf_from_modelbuf(NN_CFG_BUF_M *model_buf, UINT32 *workbuf_size, UINT32 pvd_yuv_width, UINT32 pvd_yuv_height, UINT PVD_MAX_DISTANCE_MODE);

/**
 * @brief Parse the required mode/working buffer size from PVD combin model
 * 		  (Experimental function, please make sure that correct steps are set before using this function)
 * 
 * @param combine_model_buf 	: Model must have been already loaded into this buffer space.
 * @param pvd_model_size 		: (Return) PVD model size
 * @param pvd_buf_size 			: (Return) PVD working buffer size
 * @param limit_fdet_model_size : (Return) limit_fdet model size
 * @param limit_fdet_buf_size 	: (Return) limit_fdet working buffer size
 * @param pvd_size 				: Specify PVD input size choice, it should match the given model in the buffer.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT pvdcnn_parse_buffer_info_from_combin_model_buf(NN_CFG_BUF_M *combine_model_buf, UINT32 *pvd_model_size, UINT32 *pvd_buf_size, UINT32 *limit_fdet_model_size, UINT32 *limit_fdet_buf_size, UINT32 pvd_size);

#endif

