/**
    @brief Header file of definition of DLI cpu permute layer.

    @file vendor_ai_dli_cpu_permute.hpp

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/
#ifndef _VENDOR_AI_DLI_CPU_PERMUTE_HPP_
#define _VENDOR_AI_DLI_CPU_PERMUTE_HPP_

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
    MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
    TYPE DEFINITION
********************************************************************/

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT dli_cpu_permute_init(NN_DLI_PERMUTE_PARM *p_parm);
HD_RESULT dli_cpu_permute_proc(NN_DLI_PERMUTE_PARM *p_parm);
HD_RESULT dli_cpu_permute_uninit(NN_DLI_PERMUTE_PARM *p_parm);

#endif  /* _VENDOR_AI_DLI_CPU_PERMUTE_HPP_ */
