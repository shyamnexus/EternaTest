
#ifndef LIMIT_FDET_LIB_H
#define LIMIT_FDET_LIB_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
// #include <signal.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_type.h"
#include "hd_gfx.h"

#include "vendor_ai.h"
#include "kflow_ai_net/kflow_ai_net.h"

#include "ext_module_api.h"

#define LIMIT_FDET_WIDTH			(96)
#define LIMIT_FDET_HEIGHT			(96)
#define LIMIT_FDET_VERSION_A		0
#define LIMIT_FDET_VERSION_B		14

#define LIMIT_FDET_NUM_CLS           (7)

enum limit_fdet_process_mode {
	LIMIT_FDET_MODE_PD = 0,
	LIMIT_FDET_MODE_CDD = 1,
	LIMIT_FDET_MODE_PVD = 2,
	LIMIT_FDET_MODE_PVD_PD_EARLY_EXIT = 3
};

typedef struct _LIMIT_FDET_ADJUSTABLE_PARAM {
	FLOAT   detection_threshold;
	FLOAT   cls_threshold[LIMIT_FDET_NUM_CLS];
	INT32   debug_print;
} LIMIT_FDET_ADJUSTABLE_PARAM;

// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Initialize turnkey limit_fdet.
 * 
 * @param buf				: Pre-allocated buffer space for limit_fdet to utilize
 * @param run_id 			: Network run id for ai2 engine.
 * @param shared_model_mem 	: Mem address of preloaded model bin, pass in NULL if no use for it.
 * @param shared_io_mem 	: Mem address of shared io memory, pass in NULL if no use for it.
 * @param model_bin_path 	: Absolute path to model bin file.
 * @param param_path 		: Absolute path to limit_fdet parameters txt file.
 * @param debug_enable 		: Enable debug print or yet.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_init(NN_CFG_BUF_M buf, UINT32 run_id, NN_CFG_BUF_M *shared_model_mem, NN_CFG_BUF_M *shared_io_mem, NN_FILE_PATH *files, UINT32 debug_enable);


/**
 * @brief Un-initialize turnkey limit_fdet
 * 
 * @param buf 				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_uninit(NN_CFG_BUF_M buf);


/**
 * @brief Turnkey limit_fdet process, make sure to call limit_fdet_init() before running this process.
 * 
 * @param buf				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @param p_vendor_img 		: Image information for turnkey limit_fdet to process (YUV)
 * @param p_in_results 		: Input detection bounding box information.
 * @param in_num 			: Input detection bounding box number.
 * @param p_out_results 	: Output detection bounding box information.
 * @param out_num 			: Output detection bounding box number.
 * @param limit_fdet_mode 	: Define limit_fdet mode, refer to limit_fdet_process_mode
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_process(NN_CFG_BUF_M buf, void* p_vendor_img, NN_RESULT_M *p_in_results, INT32 in_num, NN_RESULT_M *p_out_results, INT32 *out_num, UINT32 limit_fdet_mode);


/**
 * @brief Turnkey limit_fdet extracting feature process, make sure to call limit_fdet_init() before running this process.
 * 
 * @param buf 				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @param p_vendor_img 		: Image information for turnkey limit_fdet to process (YUV)	
 * @param ptr_nn_result 	: Input detection bounding box information.
 * @param output_feature 	: Output feature address. (This address buffer space should be pre-allocated)
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_net_extract_feature(NN_CFG_BUF_M buf, void* p_vendor_img, NN_RESULT_M* ptr_nn_result, FLOAT* output_feature);


/**
 * @brief Turnkey limit_fdet process, make sure to call limit_fdet_init() before running this process.
 *        NOTE: This function will only output at max 1 PD result
 * 
 * @param buf				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @param p_vendor_img		: Image information for turnkey limit_fdet to process (YUV)
 * @param p_in_results 		: Input detection bounding box information.
 * @param in_num 			: Input detection bounding box number.
 * @param p_out_results 	: Output detection bounding box information.
 * @param out_num 			: Output detection bounding box number.
 * @param max_roi_num 		: Limit fdet will only perform up to N times bbox check, N = max_roi_num
 * @return HD_RESULT		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_process_getone(NN_CFG_BUF_M buf, void* p_vendor_img, NN_RESULT_M *p_in_results, INT32 in_num, NN_RESULT_M *p_out_results, INT32 *out_num, UINT32 max_roi_num);


/**
 * @brief Get the current deploying limit_fdet parameters 
 * 
 * @param buf 				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @param params 			: Returned struct member contains value of current deploying parameters
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_params_get(NN_CFG_BUF_M buf, LIMIT_FDET_ADJUSTABLE_PARAM* params);


/**
 * @brief Set and update limit_fdet parameters 
 * 
 * @param buf 				: Pre-allocated buffer space for limit_fdet to utilize, make sure mem parameters are the same as limit_fdet_init()
 * @param params 			: Please specify the required parameters to update.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_params_set(NN_CFG_BUF_M buf, LIMIT_FDET_ADJUSTABLE_PARAM* params);


/**
 * @brief Parse the required working buffer size for limit_fdet
 * 
 * @param model_buf 		: Model must have been already loaded into this buffer space.
 * @param workbuf_size 		: (return) limit_fdet working buffer require size.
 * @return HD_RESULT 		: HD_OK = Successful, Others = Failed.
 */
HD_RESULT limit_fdet_parse_workbuf_from_modelbuf(NN_CFG_BUF_M *model_buf, UINT32 *workbuf_size);


#endif

