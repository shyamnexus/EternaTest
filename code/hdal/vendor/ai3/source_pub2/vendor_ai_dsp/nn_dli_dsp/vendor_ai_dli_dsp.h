/**
	@brief Header file of definition of DLI dsp.

	@file vendor_ai_dli_dsp.h

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_DSP_H_
#define _VENDOR_AI_DLI_DSP_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h" // for HD_RESULT

#include "vendor_ai_net/nn_dli.h"
#include "nvtnn/nvtnndsp_lib.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern HD_RESULT dli_dsp_init(NN_DLI_LAYER_PARM *p_parm);
extern HD_RESULT dli_dsp_proc(NN_DLI_LAYER_PARM *p_parm, NVTNN_SENDTO dsp_core);
extern HD_RESULT dli_dsp_uninit(NN_DLI_LAYER_PARM *p_parm);

#ifdef __cplusplus
}
#endif

#endif  /* _VENDOR_AI_DLI_DSP_H_ */