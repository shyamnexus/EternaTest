/**
	@brief Header file of definition of DLI pooling layer.

	@file vendor_ai_dli_cpu_pooling.hpp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_CPU_POOLING_HPP_
#define _VENDOR_AI_DLI_CPU_POOLING_HPP_

/********************************************************************
 MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	INCLUDE FILES
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "hd_type.h"
#include "vendor_ai_net/nn_dli.h"
#include "nvtnn/nvtnn_lib.h"

#ifdef __cplusplus
}
#endif


/********************************************************************
	TYPE DEFINITION
********************************************************************/

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT dli_cpu_pooling_init(NN_DLI_POOLING_PARM *p_parm);
HD_RESULT dli_cpu_pooling_proc(NN_DLI_POOLING_PARM *p_parm);
HD_RESULT dli_cpu_pooling_uninit(NN_DLI_POOLING_PARM *p_parm);

#endif  /* _VENDOR_AI_DLI_CPU_POOLING_HPP_ */
