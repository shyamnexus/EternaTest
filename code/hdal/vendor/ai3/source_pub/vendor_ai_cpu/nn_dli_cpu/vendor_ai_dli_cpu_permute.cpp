/**
    @brief Source file of DLI cpu permute layer.

    @file vendor_ai_dli_cpu_permute.cpp

    @ingroup dli_cpu_permute

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <vector>

//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][dli_permute]"
#include "vendor_ai_debug.h"
//=============================================================
#include "vendor_ai_dli_cpu_utils.hpp"
#include "vendor_ai_dli_cpu_permute.hpp"

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

static VOID dli_to_nvtnn_permute(NN_DLI_PERMUTE_PARM *p_dli_parm, NN_PERMUTE_PARM *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info      = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info     = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	
	p_nvtnn_prms->in_addr               = input_info->data_va;
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->shape.width           = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height          = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel         = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num       = input_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_ofs.line_ofs       = input_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->in_ofs.channel_ofs    = input_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->in_ofs.batch_ofs      = input_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->out_ofs.line_ofs      = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs   = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs     = output_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf                   = input_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	// TODO 
	p_nvtnn_prms->order[0]              = p_dli_parm->perm.order[0]; // permute the order
	p_nvtnn_prms->order[1]              = p_dli_parm->perm.order[1]; // permute the order
	p_nvtnn_prms->order[2]              = p_dli_parm->perm.order[2]; // permute the order
	p_nvtnn_prms->order[3]              = p_dli_parm->perm.order[3]; // permute the order
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_cpu_permute_init(NN_DLI_PERMUTE_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_cpu_permute_proc(NN_DLI_PERMUTE_PARM *p_parm)
{
	NN_PERMUTE_PARM nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_permute(p_parm, &nvtnn_prms);

	// Execute nvtnn
	return nvtnn_permute_process(&nvtnn_prms);
}

HD_RESULT dli_cpu_permute_uninit(NN_DLI_PERMUTE_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
