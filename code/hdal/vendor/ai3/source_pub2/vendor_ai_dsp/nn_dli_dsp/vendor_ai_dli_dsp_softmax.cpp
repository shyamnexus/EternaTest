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
#include "vendor_ai_util.h"
#include "vendor_ai_dli_dsp_softmax.hpp"

/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define SOFTMAX_PRINT_PARM      FALSE

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

static VOID dli_to_nvtnn_softmax(NN_DLI_SOFTMAX_PARM *p_dli_parm, NVTNN_SOFTMAX_PRMS *p_nvtnn_prms)
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
	// TODO                             = p_dli_parm->beta;
	p_nvtnn_prms->axis                  = NN_AXIS_NUM - 1 - p_dli_parm->axis; // reverse the order

#if SOFTMAX_PRINT_PARM
	NVTNN_SOFTMAX_PRMS *p_parm = p_nvtnn_prms;
	DBGLH(p_nvtnn_prms->in_addr);
	DBGLH(p_nvtnn_prms->out_addr);
	DBGLH(p_nvtnn_prms->tmp_addr);
	DBGD(p_parm->shape.width);
	DBGD(p_parm->shape.height);
	DBGD(p_parm->shape.channel);
	DBGD(p_parm->shape.batch_num);
	DBGD(p_parm->in_ofs.line_ofs);
	DBGD(p_parm->in_ofs.channel_ofs);
	DBGD(p_parm->in_ofs.batch_ofs);
	DBGD(p_parm->out_ofs.line_ofs);
	DBGD(p_parm->out_ofs.channel_ofs);
	DBGD(p_parm->out_ofs.batch_ofs);
	DBGD(p_parm->in_fmt.frac_bits);
	DBGD(p_parm->in_fmt.int_bits);
	DBGD(p_parm->in_fmt.sign_bits);
	DBGD(p_parm->out_fmt.frac_bits);
	DBGD(p_parm->out_fmt.int_bits);
	DBGD(p_parm->out_fmt.sign_bits);
	DBGD(p_parm->isf.mul);
	DBGD(p_parm->isf.shf);
	DBGD(p_parm->osf.mul);
	DBGD(p_parm->osf.shf);
	DBGD(p_parm->axis);
#endif
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_softmax_init(NN_DLI_SOFTMAX_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_softmax_proc(NN_DLI_SOFTMAX_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_SOFTMAX_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_softmax(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_SOFTMAX, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_softmax_uninit(NN_DLI_SOFTMAX_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
