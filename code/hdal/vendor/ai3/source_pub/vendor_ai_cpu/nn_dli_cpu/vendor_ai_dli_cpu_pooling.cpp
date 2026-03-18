/**
	@brief Source file of DLI cpu pooling layer.

	@file vendor_ai_dli_cpu_pooling.cpp

	@ingroup dli_cpu_pooling

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <iostream>
#include <map>
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][dli_pooling]"
#include "vendor_ai_debug.h"
//=============================================================
#include "vendor_ai_dli_cpu_pooling.hpp"
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
static HD_RESULT dli_to_nvtnn_pooling(NN_DLI_POOLING_PARM *p_dli_parm, NN_POOL_PARM *p_nvtnn_prms)
{
	if (p_dli_parm->dilation.height > 0 || p_dli_parm->dilation.width > 0)
		return HD_ERR_NOT_SUPPORT;

	NN_DLI_TENSOR_INFO *input_info  = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	p_nvtnn_prms->in_addr  = input_info->data_va;
	p_nvtnn_prms->out_addr = output_info->data_va;
	p_nvtnn_prms->pool.pool_shf = 0;
	p_nvtnn_prms->pool.sclshf.in_scale = input_quant_info->sf.mul;
	p_nvtnn_prms->pool.sclshf.in_shift = input_quant_info->sf.shf;
	p_nvtnn_prms->pool.sclshf.out_scale = output_quant_info->sf.mul;
	p_nvtnn_prms->pool.sclshf.out_shift = output_quant_info->sf.shf;

	p_nvtnn_prms->in_ofs.line_ofs     = input_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->in_ofs.channel_ofs  = input_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->in_ofs.batch_ofs	  = input_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->out_ofs.line_ofs    = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs   = output_info->strides[NN_DLI_STRIDE_N];

	// global
	if (input_info->shape[NN_DLI_AXIS_W] == p_dli_parm->pool_info.pool_size.width &&
		input_info->shape[NN_DLI_AXIS_H] == p_dli_parm->pool_info.pool_size.height){

		p_nvtnn_prms->pool.global.avg_mul = 1;
		p_nvtnn_prms->pool.global.avg_shf = 0;

		if (p_dli_parm->pool_info.pool_type == NN_DLI_POOLING_OP_MAX)
			p_nvtnn_prms->pool.mode = NN_AI_POOL_GLOBAL_MAX;
		else if (p_dli_parm->pool_info.pool_type == NN_DLI_POOLING_OP_AVE)
			p_nvtnn_prms->pool.mode = NN_AI_POOL_GLOBAL_AVG;
		else
			return HD_ERR_NOT_SUPPORT;
	} else {
		p_nvtnn_prms->pool.local.ker_h = p_dli_parm->pool_info.pool_size.height;
		p_nvtnn_prms->pool.local.ker_w = p_dli_parm->pool_info.pool_size.width;
		p_nvtnn_prms->pool.local.ker_stridex = p_dli_parm->pool_info.pad_stride_info.stride[0];
		p_nvtnn_prms->pool.local.ker_stridey = p_dli_parm->pool_info.pad_stride_info.stride[1];
		p_nvtnn_prms->pool.local.pad.pad_val = 0;
		p_nvtnn_prms->pool.local.pad.top_pad_num = p_dli_parm->pool_info.pad_stride_info.pad_top;
		p_nvtnn_prms->pool.local.pad.bot_pad_num = p_dli_parm->pool_info.pad_stride_info.pad_bottom;
		p_nvtnn_prms->pool.local.pad.left_pad_num = p_dli_parm->pool_info.pad_stride_info.pad_left;
		p_nvtnn_prms->pool.local.pad.right_pad_num = p_dli_parm->pool_info.pad_stride_info.pad_right;

		if (p_dli_parm->count_inclue_pad)
			p_nvtnn_prms->pool.local.pool_div_type = NN_AI_NON_BOUNDARY;
		else
			p_nvtnn_prms->pool.local.pool_div_type = NN_AI_CONSIDER_BOUNDARY;

		if (p_dli_parm->celi_mode)
			p_nvtnn_prms->pool.local.pool_cal_type = NN_AI_CEIL;
		else
			p_nvtnn_prms->pool.local.pool_cal_type = NN_AI_FLOOR;

		if (p_dli_parm->pool_info.pool_type == NN_DLI_POOLING_OP_MAX)
			p_nvtnn_prms->pool.mode = NN_AI_POOL_LOCAL_MAX;
		else if (p_dli_parm->pool_info.pool_type == NN_DLI_POOLING_OP_AVE)
			p_nvtnn_prms->pool.mode = NN_AI_POOL_LOCAL_AVG;
		else
			return HD_ERR_NOT_SUPPORT;
	}

	p_nvtnn_prms->tmp_addr        = p_dli_parm->temp_buf_va;

	p_nvtnn_prms->shape.width     = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height    = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel   = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num = input_info->shape[NN_DLI_AXIS_N];

	p_nvtnn_prms->in_fmt          = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt         = output_quant_info->fmt;
	p_nvtnn_prms->isf             = input_quant_info->sf;
	p_nvtnn_prms->osf             = output_quant_info->sf;

	return HD_OK;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_cpu_pooling_init(NN_DLI_POOLING_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
	return HD_OK;
}

HD_RESULT dli_cpu_pooling_proc(NN_DLI_POOLING_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
	HD_RESULT result = HD_OK;

	NN_POOL_PARM nvtnn_prms = {0};
	result = dli_to_nvtnn_pooling(p_parm, &nvtnn_prms);
	if (result != HD_OK)
		return result;

    result = nvtnn_pool_process(&nvtnn_prms);

	return result;
}

HD_RESULT dli_cpu_pooling_uninit(NN_DLI_POOLING_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
	return HD_OK;
}
