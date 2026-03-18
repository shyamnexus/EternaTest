/**
	@brief Header file of definition of DLI fc_post layer.

	@file vendor_ai_dli_cpu_fc_post.hpp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_CPU_FC_POST_HPP_
#define _VENDOR_AI_DLI_CPU_FC_POST_HPP_

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
HD_RESULT dli_cpu_fc_post_init(NN_DLI_FC_POST_PARM *p_parm);
HD_RESULT dli_cpu_fc_post_proc(NN_DLI_FC_POST_PARM *p_parm);
HD_RESULT dli_cpu_fc_post_uninit(NN_DLI_FC_POST_PARM *p_parm);

#endif  /* _VENDOR_AI_DLI_CPU_FC_POST_HPP_ */
