#ifndef _SW_TRACKER_H_
#define _SW_TRACKER_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "hd_type.h"
#include "vendor_ai.h"
//#include "kflow_ai_net/kflow_ai_net.h"

typedef VENDOR_AI3_CFG_BUF NN_CFG_BUF_M;

typedef struct _PDCNN_TRACKID_RESULT
{
	INT32   category;
	FLOAT 	score;
	FLOAT 	x1;
	FLOAT 	y1;
	FLOAT 	x2;
	FLOAT 	y2;
	INT32	trackID;
	UINT32  feature_addr_idx;	
}PDCNN_TRACKID_RESULT;

typedef struct {
	int enable;
	int min_hits;
	int max_age;
	float iou_threshold;
	int max_tracker_num;
} sort_tracker_params;


// ============================================================
// =                    Interface function                    =
// ============================================================

/**
 * @brief Read parameters for software tracking
 * 
 * @param sw_tracker_param : Target destination for loaded tracker parameters
 * @param paramsfile 	   : The path to tracker parameters text file
 * @return HD_RESULT 	   : if HD_RESULT != HD_OK, text file may not exist.
 */
HD_RESULT sort_read_param(sort_tracker_params *sw_tracker_param, CHAR* paramsfile);


/**
 * @brief Initialize tracking, be sure to run this once before calling sort_update()
 * 
 * @param in_buf 			: The source buffer space for assigning tracker buffer.
 * @param sw_tracker_buf 	: The assigned tracker buffer information.
 * @param p_tracking_result : Input/Output object information.
 * @param sw_tracker_param  : Tracker parameters (make sure to appointed these value before initialization)
 * @param para_file  		: The path to tracker parameters text file
 * @return HD_RESULT 
 */
HD_RESULT sort_init(NN_CFG_BUF_M *in_buf, NN_CFG_BUF_M *sw_tracker_buf, PDCNN_TRACKID_RESULT **p_tracking_result, sort_tracker_params *sw_tracker_param, CHAR *para_file);


/**
 * @brief Get the input object information, do tracker logic and assigned tracker id to output objects
 * 
 * @param sw_tracker_buf 	: The assigned tracker buffer information.
 * @param sw_tracker_param  : Tracker parameters
 * @param in_objs 			: Input object informations.
 * @param in_obj_num 		: Total input object number
 * @param out_objs 			: Output object informations (After tracker logic is done, assign trackerID to these)
 * @param out_obj_num       : Total output object number (Tracker reported object number, may different from input object number)
 * @return VOID 
 */
VOID sort_update(NN_CFG_BUF_M *sw_tracker_buf, const sort_tracker_params *sw_tracker_param, PDCNN_TRACKID_RESULT *in_objs, UINT32 *in_obj_num, PDCNN_TRACKID_RESULT *out_objs, UINT32 *out_obj_num);


/**
 * @brief Get the object information, do tracker logic and assigned tracker id to objects
 *        This function do exact same functionality as sort_update(), read from objs and store output result into objs.
 * 
 * @param sw_tracker_buf 	: The assigned tracker buffer information.
 * @param sw_tracker_param  : Tracker parameters
 * @param objs 				: Input/Output object information (Bounding box coordinates may be different before/after function call)
 * @param obj_num 			: Input/Output total object number (BBox number may be different before/after function call)
 * @return VOID 
 */
VOID sort_update_inplace(NN_CFG_BUF_M *sw_tracker_buf, const sort_tracker_params *sw_tracker_param, PDCNN_TRACKID_RESULT *objs, UINT32 *obj_num);

#endif // _SW_TRACKER_H_
