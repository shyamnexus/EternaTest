/**
    @brief Source file of DLI dsp softmax layer.

    @file vendor_ai_dli_dsp_softmax.cpp

    @ingroup dli_dsp_softmax

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_fc.hpp"

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

static VOID dli_to_nvtnn_fc(NN_DLI_FC_PARM *p_dli_parm, NVTNN_FC_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *weight_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->weight_info_va;
	NN_DLI_TENSOR_INFO *bias_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->bias_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *weight_quant_info  = (NN_DLI_QUANTIZATION_INFO *)weight_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *bias_quant_info  = (NN_DLI_QUANTIZATION_INFO *)bias_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
		
	p_nvtnn_prms->in_addr               = input_info->data_va;
	p_nvtnn_prms->weight_addr           = weight_info->data_va;
	p_nvtnn_prms->bias_addr             = bias_info->data_va;
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->in_shape.width        = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->in_shape.height       = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->in_shape.channel      = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->in_shape.batch_num    = input_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->out_shape.width       = output_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->out_shape.height      = output_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->out_shape.channel     = output_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->out_shape.batch_num   = output_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_ofs.line_ofs       = input_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->in_ofs.channel_ofs    = input_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->in_ofs.batch_ofs      = input_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->out_ofs.line_ofs      = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs   = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs     = output_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prms->weight_fmt            = weight_quant_info->fmt;
	p_nvtnn_prms->bias_fmt              = bias_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf                   = input_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	// TODO                             
	p_nvtnn_prms->transpose             = p_dli_parm->transpose_weights;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_fc_init(NN_DLI_FC_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_fc_proc(NN_DLI_FC_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_FC_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_fc(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_FC, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_fc_uninit(NN_DLI_FC_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
