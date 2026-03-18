/**
	@brief Source file of DLI dsp.

	@file vendor_ai_dli_dsp.cpp

	@ingroup dli_dsp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "vendor_ai_net/nn_net.h"
#include "vendor_ai_util.h"

#include "vendor_ai_dli_dsp.h"
#include "vendor_ai_dli_dsp_layers.hpp"

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Types Declarations                                                    */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Extern Function Prototype                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Global Variables                                                      */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Functions                                                             */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_init(NN_DLI_LAYER_PARM *p_parm)
{
	HD_RESULT result = HD_OK;

	//DBG_IND("%s: Sign: 0x%X, Mode: 0x%X, Size: %d\n", __func__, p_parm->sign, p_parm->mode, p_parm->parm_size);

	switch (p_parm->mode) {
		case NN_MODE::NN_DLI_SOFTMAX:
			result = dli_dsp_softmax_init(reinterpret_cast<NN_DLI_SOFTMAX_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_FC:
			result = dli_dsp_fc_init(reinterpret_cast<NN_DLI_FC_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_DILATEDCONV:
			result = dli_dsp_dilatedconv_init(reinterpret_cast<NN_DLI_CONV_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_DIV:
		case NN_MODE::NN_DLI_EXP:
		case NN_MODE::NN_DLI_SQRT:
		case NN_MODE::NN_DLI_POW:
			result = dli_dsp_elementwise_init(reinterpret_cast<NN_DLI_ElEMENTWISE_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_INSTANCE_NORMALIZATION:
			result = dli_dsp_instancenorm_init(reinterpret_cast<NN_DLI_INSTANCE_NORMALIZATION_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_LAYER_NORMALIZATION:
			result = dli_dsp_layernorm_init(reinterpret_cast<NN_DLI_LAYER_NORMALIZATION_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_DECONVOLUTION:
			result = dli_dsp_deconvolution_init(reinterpret_cast<NN_DLI_DECONVOLUTION_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_DECONVOLUTION_DEPTHWISE:
			result = dli_dsp_deconvolution_depthwise_init(reinterpret_cast<NN_DLI_DECONVOLUTION_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_MATMUL:
			result = dli_dsp_matmul_init(reinterpret_cast<NN_DLI_MATMUL_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_PRIORBOX:
			result = dli_dsp_priorbox_init(reinterpret_cast<NN_DLI_PRIORBOX_PARM *>(p_parm));
			break;
		case NN_MODE::NN_DLI_RESIZE:
			result = dli_dsp_resize_init(reinterpret_cast<NN_DLI_RESIZE_PARM *>(p_parm));
			break;
		default:
			result = HD_ERR_NOT_SUPPORT;
			break;
	}

	if (result != HD_OK) {
		DBG_ERR("%s: err, Mode 0x%X\n", __func__, p_parm->mode);
		p_parm->obj_ptr = UINTPTR_MAX; // mark as init fail
	}

	return result;
}

HD_RESULT dli_dsp_proc(NN_DLI_LAYER_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	HD_RESULT result = HD_OK;

	//DBG_IND("%s: Sign: 0x%X, Mode: 0x%X, Size: %d\n", __func__, p_parm->sign, p_parm->mode, p_parm->parm_size);

	if (p_parm->obj_ptr == UINTPTR_MAX) {
		return HD_ERR_NOT_SUPPORT;
	}
	switch (p_parm->mode) {
		case NN_MODE::NN_DLI_SOFTMAX:
			result = dli_dsp_softmax_proc(reinterpret_cast<NN_DLI_SOFTMAX_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_FC:
			result = dli_dsp_fc_proc(reinterpret_cast<NN_DLI_FC_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_DILATEDCONV:
			result = dli_dsp_dilatedconv_proc(reinterpret_cast<NN_DLI_CONV_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_DIV:
		case NN_MODE::NN_DLI_EXP:
		case NN_MODE::NN_DLI_SQRT:
		case NN_MODE::NN_DLI_POW:
			result = dli_dsp_elementwise_proc(reinterpret_cast<NN_DLI_ElEMENTWISE_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_INSTANCE_NORMALIZATION:
			result = dli_dsp_instancenorm_proc(reinterpret_cast<NN_DLI_INSTANCE_NORMALIZATION_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_LAYER_NORMALIZATION:
			result = dli_dsp_layernorm_proc(reinterpret_cast<NN_DLI_LAYER_NORMALIZATION_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_DECONVOLUTION:
			result = dli_dsp_deconvolution_proc(reinterpret_cast<NN_DLI_DECONVOLUTION_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_DECONVOLUTION_DEPTHWISE:
			result = dli_dsp_deconvolution_depthwise_proc(reinterpret_cast<NN_DLI_DECONVOLUTION_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_MATMUL:
			result = dli_dsp_matmul_proc(reinterpret_cast<NN_DLI_MATMUL_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_PRIORBOX:
			result = dli_dsp_priorbox_proc(reinterpret_cast<NN_DLI_PRIORBOX_PARM *>(p_parm), dsp_core);
			break;
		case NN_MODE::NN_DLI_RESIZE:
			result = dli_dsp_resize_proc(reinterpret_cast<NN_DLI_RESIZE_PARM *>(p_parm), dsp_core);
			break;
		default:
			result = HD_ERR_NOT_SUPPORT;
			break;
	}
	if (result != HD_OK) {
		DBG_ERR("%s: err, Mode 0x%X\n", __func__, p_parm->mode);
	}

	return result;
}

HD_RESULT dli_dsp_uninit(NN_DLI_LAYER_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
