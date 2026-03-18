/**
	@brief Header file of definition of ai net postproc.

	@file ai_cpu_post_accuracy.h

	@ingroup ai_cpu_post_accuracy

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _AI_CPU_POST_ACCURACY_H_
#define _AI_CPU_POST_ACCURACY_H_

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define LABEL_LEN           256         ///< maximal length of class label
#define MAX_CLASS_NUMBER    1000
#define TOP_N               5

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	TYPE DEFINITION
********************************************************************/
/**
	Data dimensions.
*/
typedef struct _AI_NET_SHAPE {
	INT32 num;                          ///< Caffe blob: batch size
	INT32 channels;                     ///< Caffe blob: number of channels
	INT32 height;                       ///< Caffe blob: height
	INT32 width;                        ///< Caffe blob: width
	INT32 lineofs;                      ///< lineoffset: distance between two lines (rows); unit: byte
} AI_NET_SHAPE;

/**
	Output class.
*/
typedef struct _AI_NET_OUTPUT_CLASS {
	INT32 no;                           ///< class number
	FLOAT score;                        ///< class score
} AI_NET_OUTPUT_CLASSS;

/**
	Parameters of accuracy calculation
*/
typedef struct _AI_NET_ACCURACY_PARM {
	uintptr_t in_addr;                  ///< [in]  address of input data
	AI_NET_OUTPUT_CLASSS *classes;      ///< [out] top scoring class list
	AI_NET_SHAPE shape;                 ///< [in]  input/output data dimensions (height/width/lineofs is not used)
	INT32 top_n;                        ///< [in/out] number of top scoring classes
	INT32 *class_idx;                   ///< [in]  address of class index buffer
} AI_NET_ACCURACY_PARM;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
extern HD_RESULT ai_net_accuracy_process(AI_NET_ACCURACY_PARM *p_parm);
#ifdef __cplusplus
}
#endif
#endif  /* _AI_CPU_POST_ACCURACY_H_ */
