/**
    @brief Source file of DLI dsp norm layer.

    @file vendor_ai_dli_dsp_norm.cpp

    @ingroup dli_dsp_norm

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_norm.hpp"
#include "vendor_ai_dli_cpu_utils.hpp"
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

static VOID dli_to_nvtnn_norm(NN_DLI_NORM_PARM *p_dli_parm, NVTNN_NORM_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info		        = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info	            = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_TENSOR_INFO *scale_buf_info	        = (NN_DLI_TENSOR_INFO *)p_dli_parm->scale_buf_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	
	p_nvtnn_prms->in_addr                = input_info->data_va;
	p_nvtnn_prms->out_addr               = output_info->data_va;
	p_nvtnn_prms->scale_addr             = scale_buf_info->data_va;
	p_nvtnn_prms->tmp_addr               = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->shape.width            = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height           = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel          = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num        = input_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_ofs.line_ofs        = p_nvtnn_prms->shape.width;
	p_nvtnn_prms->in_ofs.channel_ofs     = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
	p_nvtnn_prms->in_ofs.batch_ofs       = p_nvtnn_prms->in_ofs.channel_ofs * p_nvtnn_prms->shape.channel;

	p_nvtnn_prms->out_ofs.line_ofs       = p_nvtnn_prms->shape.width;
	p_nvtnn_prms->out_ofs.channel_ofs    = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
	p_nvtnn_prms->out_ofs.batch_ofs      = p_nvtnn_prms->out_ofs.channel_ofs * p_nvtnn_prms->shape.channel;

	p_nvtnn_prms->in_fmt                 = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt                = output_quant_info->fmt;
	p_nvtnn_prms->isf                    = input_quant_info->sf;
	p_nvtnn_prms->osf                    = output_quant_info->sf;
	p_nvtnn_prms->across_spatial         = (UINT8)p_dli_parm->across_spatial;
	p_nvtnn_prms->channel_shared         = (UINT8)p_dli_parm->channel_shared;
	p_nvtnn_prms->eps                    = p_dli_parm->eps;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_norm_init(NN_DLI_NORM_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_norm_proc(NN_DLI_NORM_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_NORM_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_norm(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_DLI_NORM, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_norm_uninit(NN_DLI_NORM_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
