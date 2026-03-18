/**
	@brief Source file of DLI cpu reverse layer.

	@file vendor_ai_dli_cpu_reverse.cpp

	@ingroup dli_cpu_reverse

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <iostream>
#include <map>
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][dli_reverse]"
#include "vendor_ai_debug.h"
//=============================================================
#include "vendor_ai_dli_cpu_reverse.hpp"
#include "vendor_ai_dli_cpu_utils.hpp"

//#include <arm_compute/runtime/NEON/functions/NEReverseLayer.h> // for arm_compute::NEReverseLayer

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

static HD_RESULT dli_to_nvtnn_reverse(NN_DLI_REVERSE_PARM *p_dli_parm, NN_REVERSE_PARM *p_nvtnn_prm)
{
	NN_DLI_TENSOR_INFO *input_info      = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info     = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	
	p_nvtnn_prm->in_addr               = input_info->data_va;
	p_nvtnn_prm->out_addr              = output_info->data_va;
	p_nvtnn_prm->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prm->shape.width           = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prm->shape.height          = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prm->shape.channel         = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prm->shape.batch_num       = input_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prm->in_ofs.line_ofs       = input_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prm->in_ofs.channel_ofs    = input_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prm->in_ofs.batch_ofs      = input_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prm->out_ofs.line_ofs      = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prm->out_ofs.channel_ofs   = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prm->out_ofs.batch_ofs     = output_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prm->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prm->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prm->isf                   = input_quant_info->sf;
	p_nvtnn_prm->osf                   = output_quant_info->sf;
	p_nvtnn_prm->axis                  = NN_AXIS_NUM - 1 - p_dli_parm->axis; // reverse the order

	return HD_OK;

}

HD_RESULT dli_cpu_reverse_init(NN_DLI_REVERSE_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
	return HD_OK;
}

HD_RESULT dli_cpu_reverse_proc(NN_DLI_REVERSE_PARM *p_parm)
{
	NN_REVERSE_PARM nvtnn_prms = {0};
	dli_to_nvtnn_reverse(p_parm, &nvtnn_prms);
	
    nvtnn_reverse_process(&nvtnn_prms);
	return HD_OK;
}

HD_RESULT dli_cpu_reverse_uninit(NN_DLI_REVERSE_PARM *p_parm)
{
	//DBG_IND("entry\r\n");

	return HD_OK;
}
