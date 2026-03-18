
#ifndef FDCNN_LIB_H
#define FDCNN_LIB_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_type.h"
#include "hd_gfx.h"
#include "ext_module_api.h"

#include "vendor_ai.h"
#include "kflow_ai_net/kflow_ai_net.h"

/* -----------------------------------------------------------------------------------------*/

#define FDCNN_INIT_READY    (6363)
#define FDCNN_STA_OK		(0)
#define FDCNN_STA_ERR		(-1)

#define FDCNN_LIGHT         (1)

#define FDCNN_MAX_SCORE     (16383) // 2^14 - 1

#define ANCHOR_NUM          (2)
#define LANDMARK_POINT_NUM  (5)

/**
     Config ID.

*/
typedef enum
{
    FDCNN_CFG_SENSITIVITY   = 0,          ///< config FDCNN sensitivity, from 0 to 16384.
	ENUM_DUMMY4WORD(FDCNN_CONFIG)
} FDCNN_CONFIG;

/**
     fdcnn detection result parameter.

*/
typedef struct _FDCNN_RESULT
{
	INT32   category;
	FLOAT 	score;
	INT32 	x;
	INT32 	y;
	INT32 	w;
	INT32 	h;
	NN_I_POINT landmark_points[LANDMARK_POINT_NUM]; //left_eye, right_eye, nose, left_mouth, right_mouth
}FDCNN_RESULT;


// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Print out FDCNN library version
 * 
 * @return VOID 
 */
VOID fdcnn_dump_version(VOID); // fdcnn big version, fdcnn small version, model version, doc version

/**
 * @brief Initialize turnkey FDCNN.
 * 
 * @param fd_buf				: Pre-allocated buffer space for FDCNN to utilize
 * @param run_id 				: Network run id for ai2 engine.
 * @param file_path		        : Absolute path to model bin file and para txt file.
 * @param debug_enable			: Enable debug print or yet.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT fdcnn_init(NN_CFG_BUF_M fd_buf, UINT32 run_id, NN_FILE_PATH *files, UINT32 debug_enable);

/**
 * @brief Turnkey FDCNN process, make sure to call fdcnn_init() before running this process.
 * 
 * @param fd_buf 				: Pre-allocated buffer space for FDCNN to utilize, make sure mem parameters are the same as fdcnn_init()
 * @param fd_src_img 			: Image information for turnkey FDCNN to process (YUV)
 * @param fd_det_output 		: Output detection bounding box information.
 * @param p_src_size 			: Input image size.
 * @param fd_det_num 			: Output detection bounding box number.
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT fdcnn_process(NN_CFG_BUF_M fd_buf, HD_GFX_IMG_BUF *fd_src_img, FDCNN_RESULT **fd_det_output, HD_URECT *p_src_size, UINT32 *fd_det_num);

/**
 * @brief Un-initialize turnkey FDCNN
 * 
 * @param fd_buf 				: Pre-allocated buffer space for FDCNN to utilize, make sure mem parameters are the same as fdcnn_init()
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
HD_RESULT fdcnn_uninit(NN_CFG_BUF_M fd_buf);

/**
 * @brief Get detection result for turnkey FDCNN
 * 
 * @param fd_buf 				: Pre-allocated buffer space for FDCNN to utilize, make sure mem parameters are the same as fdcnn_init()
 * @param p_face 				: Save detection result for turnkey FDCNN
 * @param p_coord 				: Video frame's inforamtion to limit width and height of detection box
 * @param max_outnum 			: The Maxmum number of detections
 * @return HD_RESULT 			: HD_OK = Successful, Others = Failed.
 */
UINT32 fdcnn_getresults(NN_CFG_BUF_M fd_buf, FDCNN_RESULT **fd_det_output, UINT32 max_outnum);

#endif

