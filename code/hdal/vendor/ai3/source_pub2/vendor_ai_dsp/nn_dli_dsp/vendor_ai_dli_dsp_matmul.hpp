/**
    @brief Header file of definition of DLI dsp matmul layer.

    @file vendor_ai_dli_dsp_matmul.hpp

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_DSP_MATMUL_HPP_
#define _VENDOR_AI_DLI_DSP_MATMUL_HPP_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "hd_type.h"
#include "vendor_ai_net/nn_dli.h"
#include "nvtnn/nvtnndsp_lib.h"

#ifdef __cplusplus
}
#endif

/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
    TYPE DEFINITION
********************************************************************/

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT dli_dsp_matmul_init(NN_DLI_MATMUL_PARM *p_parm);
HD_RESULT dli_dsp_matmul_proc(NN_DLI_MATMUL_PARM *p_parm, NVTNN_SENDTO dsp_core);
HD_RESULT dli_dsp_matmul_uninit(NN_DLI_MATMUL_PARM *p_parm);

#endif  /* _VENDOR_AI_DLI_DSP_MATMUL_HPP_ */