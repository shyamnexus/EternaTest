/**
	@brief Header file of definition of DLI dsp utility functions.

	@file vendor_ai_dli_dsp_utils.hpp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_DSP_UTILS_HPP_
#define _VENDOR_AI_DLI_DSP_UTILS_HPP_

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	INCLUDE FILES
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/type.h"
#include "vendor_ai_net/nn_dli.h"

#ifdef __cplusplus
}
#endif

#include <vector>

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
// get slice element from origin input through stride
VOID getdata_stride_slice(NN_DLI_TENSOR_INFO *p_info);

#endif /* _VENDOR_AI_DLI_DSP_UTILS_HPP_ */