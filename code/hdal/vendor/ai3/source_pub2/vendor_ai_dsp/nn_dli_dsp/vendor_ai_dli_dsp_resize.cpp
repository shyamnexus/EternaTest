/**
    @brief Source file of DLI dsp dsp resize layer.

    @file vendor_ai_dli_dsp_resize.cpp

    @ingroup dli_dsp_resize

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_resize.hpp"

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
static VOID dli_to_nvtnn_resize(NN_DLI_RESIZE_PARM *p_dli_parm, NVTNN_RESIZE_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info  = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	p_nvtnn_prms->in_addr             = input_info->data_va;
	p_nvtnn_prms->out_addr            = output_info->data_va;
	p_nvtnn_prms->tmp_addr            = p_dli_parm->temp_buf_va;

	p_nvtnn_prms->shape.width         = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height        = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel       = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num     = input_info->shape[NN_DLI_AXIS_N];

	p_nvtnn_prms->in_ofs.line_ofs     = input_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->in_ofs.channel_ofs  = input_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->in_ofs.batch_ofs    = input_info->strides[NN_DLI_STRIDE_N];

	p_nvtnn_prms->out_ofs.line_ofs    = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs   = output_info->strides[NN_DLI_STRIDE_N];

	p_nvtnn_prms->in_fmt              = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt             = output_quant_info->fmt;

	p_nvtnn_prms->dst_rows = output_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->dst_cols = output_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->tmp_step = 0;
    p_nvtnn_prms->filter_size = 0;

	p_nvtnn_prms->interpolation_policy = p_dli_parm->interpolation_policy;
	p_nvtnn_prms->border_mode  		   = p_dli_parm->border_mode;
	p_nvtnn_prms->sampling_policy  	   = p_dli_parm->sampling_policy;
	p_nvtnn_prms->align_corners  	   = p_dli_parm->align_corners;
	p_nvtnn_prms->__padding  		   = p_dli_parm->__padding;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_resize_init(NN_DLI_RESIZE_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_resize_proc(NN_DLI_RESIZE_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_RESIZE_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_resize(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_DLI_RESIZE, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_resize_uninit(NN_DLI_RESIZE_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}

