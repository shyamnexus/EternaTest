/**
    @brief Source file of DLI dsp deconvolution layer.

    @file vendor_ai_dli_dsp_deconvolution.cpp

    @ingroup dli_dsp_deconvolution

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_deconvolution.hpp"
// #include "vendor_ai_dli_dsp_utils.hpp"
// extern "C" {
// #include "hd_common.h"
// }
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

// VOID dli_nvtnn_dsp_flush_cache_deconvolution(NVTNN_DECONVOLUTION_PRMS *p_nvtnn_prms) {
// 	UINT32 in_size = p_nvtnn_prms->in_ofs.batch_ofs * p_nvtnn_prms->inshape.batch_num;
// 	UINT32 input_size = in_size * get_bytes_num(&p_nvtnn_prms->in_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr, (UINT32)input_size);

// 	UINT32 weight_num = p_nvtnn_prms->weight_ofs.batch_ofs * p_nvtnn_prms->wtshape.batch_num;
// 	UINT32 weight_size = weight_num * get_bytes_num(&p_nvtnn_prms->weight_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->weight_addr, (UINT32)weight_size);

// 	UINT32 final_out_size = p_nvtnn_prms->out_ofs.batch_ofs * p_nvtnn_prms->outshape.batch_num;
// 	UINT32 output_size = final_out_size * get_bytes_num(&p_nvtnn_prms->out_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->out_addr, (UINT32)output_size);

// 	UINT32 kernel_extent_w = p_nvtnn_prms->dilation_w * (p_nvtnn_prms->kernel_w - 1) + 1;
// 	UINT32 kernel_extent_h = p_nvtnn_prms->dilation_h * (p_nvtnn_prms->kernel_h - 1) + 1;
// 	UINT32 tmpoutwidth = (p_nvtnn_prms->inshape.width - 1) * p_nvtnn_prms->stride_w + kernel_extent_w + p_nvtnn_prms->output_padding;
// 	UINT32 tmpoutheight = (p_nvtnn_prms->inshape.height - 1) * p_nvtnn_prms->stride_h + kernel_extent_h + p_nvtnn_prms->output_padding;
// 	UINT32 tmp_output_size = tmpoutwidth * tmpoutheight * p_nvtnn_prms->num_output * p_nvtnn_prms->inshape.batch_num;

// 	UINT32 tmpsize = sizeof(INT32) * tmp_output_size;
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->tmp_addr, (UINT32)tmpsize);

// 	if(p_nvtnn_prms->bias) {
// 		UINT32 biasnum = p_nvtnn_prms->num_output;
// 		UINT32 biassize = biasnum * get_bytes_num(&p_nvtnn_prms->bias_fmt);
// 		hd_common_mem_flush_cache((void *)p_nvtnn_prms->bias_addr, (UINT32)biassize);
// 	}
// }

static VOID dli_to_nvtnn_deconvolution(NN_DLI_DECONVOLUTION_PARM *p_dli_parm, NVTNN_DECONVOLUTION_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *weight_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->weight_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *weight_quant_info  = (NN_DLI_QUANTIZATION_INFO *)weight_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	p_nvtnn_prms->kernel_w              = p_dli_parm->weights_info.kernel_width;
	p_nvtnn_prms->kernel_h              = p_dli_parm->weights_info.kernel_height;
	p_nvtnn_prms->stride_w              = p_dli_parm->conv_info.stride[0];
	p_nvtnn_prms->stride_h              = p_dli_parm->conv_info.stride[1];
	p_nvtnn_prms->dilation_w            = p_dli_parm->dilation.width;    //default 1
	p_nvtnn_prms->dilation_h            = p_dli_parm->dilation.height;   //default 1
	p_nvtnn_prms->pad_w                 = p_dli_parm->conv_info.pad_left;
	p_nvtnn_prms->pad_h                 = p_dli_parm->conv_info.pad_top;
	p_nvtnn_prms->group                 = p_dli_parm->num_groups;       //default 1
	p_nvtnn_prms->num_output            = output_info->shape[NN_DLI_AXIS_C];  //output channel
	p_nvtnn_prms->output_padding        = p_dli_parm->output_padding;  //default 0
	p_nvtnn_prms->bias                  = (UINT32)(p_dli_parm->bias);           //deconvolution bias term

	p_nvtnn_prms->in_addr               = input_info->data_va;
	p_nvtnn_prms->weight_addr           = weight_info->data_va;
	if(p_nvtnn_prms->bias) {
		NN_DLI_TENSOR_INFO *bias_info   = (NN_DLI_TENSOR_INFO *)p_dli_parm->bias_info_va;
		NN_DLI_QUANTIZATION_INFO *bias_quant_info  = (NN_DLI_QUANTIZATION_INFO *)bias_info->quant_info_va;
		p_nvtnn_prms->bias_addr         = bias_info->data_va;
		p_nvtnn_prms->bias_fmt          = bias_quant_info->fmt;
		p_nvtnn_prms->bsf               = bias_quant_info->sf;
	}
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->inshape.width         = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->inshape.height        = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->inshape.channel       = input_info->shape[NN_DLI_AXIS_C]; //lineofs
	p_nvtnn_prms->inshape.batch_num     = input_info->shape[NN_DLI_AXIS_N];
    /*
	pytorch weight shape:        [inchannel,  outchannel, kernel_h, kernel_w]
	this file weight shape:      [outchannel, kernel_h,   kernel_w, inchannel]
    */
	p_nvtnn_prms->wtshape.width         = p_nvtnn_prms->kernel_w;
	p_nvtnn_prms->wtshape.height        = p_nvtnn_prms->kernel_h;
	p_nvtnn_prms->wtshape.channel       = p_nvtnn_prms->inshape.channel;
	p_nvtnn_prms->wtshape.batch_num     = p_nvtnn_prms->num_output;

	p_nvtnn_prms->in_ofs.line_ofs       = p_nvtnn_prms->inshape.channel;
	p_nvtnn_prms->in_ofs.channel_ofs    = p_nvtnn_prms->inshape.channel * p_nvtnn_prms->inshape.width;
	p_nvtnn_prms->in_ofs.batch_ofs      = p_nvtnn_prms->in_ofs.channel_ofs * p_nvtnn_prms->inshape.height;
	p_nvtnn_prms->weight_ofs.line_ofs   = p_nvtnn_prms->wtshape.channel;  //=p_nvtnn_prms->wtshape.channel
	p_nvtnn_prms->weight_ofs.channel_ofs = p_nvtnn_prms->wtshape.channel * p_nvtnn_prms->wtshape.width; //=p_nvtnn_prms->wtshape.channel * p_nvtnn_prms->wtshape.width
	p_nvtnn_prms->weight_ofs.batch_ofs  = p_nvtnn_prms->weight_ofs.channel_ofs * p_nvtnn_prms->wtshape.height;
	p_nvtnn_prms->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prms->weight_fmt            = weight_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf                   = input_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	p_nvtnn_prms->wsf                   = weight_quant_info->sf;

	UINT32 kernel_extent_w = p_nvtnn_prms->dilation_w * (p_nvtnn_prms->kernel_w - 1) + 1;
	UINT32 kernel_extent_h = p_nvtnn_prms->dilation_h * (p_nvtnn_prms->kernel_h - 1) + 1;
	UINT32 tmpoutwidth = (p_nvtnn_prms->inshape.width - 1) * p_nvtnn_prms->stride_w + kernel_extent_w + p_nvtnn_prms->output_padding;
	UINT32 tmpoutheight = (p_nvtnn_prms->inshape.height - 1) * p_nvtnn_prms->stride_h + kernel_extent_h + p_nvtnn_prms->output_padding;

	p_nvtnn_prms->outshape.width = tmpoutwidth - p_nvtnn_prms->pad_w * 2;
	p_nvtnn_prms->outshape.height = tmpoutheight - p_nvtnn_prms->pad_h * 2;
	p_nvtnn_prms->outshape.channel = output_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->outshape.batch_num = p_nvtnn_prms->inshape.batch_num;
	p_nvtnn_prms->out_ofs.line_ofs = p_nvtnn_prms->outshape.width;
	p_nvtnn_prms->out_ofs.channel_ofs = p_nvtnn_prms->out_ofs.line_ofs * p_nvtnn_prms->outshape.height;
	p_nvtnn_prms->out_ofs.batch_ofs = p_nvtnn_prms->out_ofs.channel_ofs * p_nvtnn_prms->outshape.channel;

	// dli_nvtnn_dsp_flush_cache_deconvolution(p_nvtnn_prms);

	#if 0
	printf("input1_info->data_va: %d  p_nvtnn_prms->in_addr0: %d\n", input_info->data_va, p_nvtnn_prms->in_addr);
	printf("p_nvtnn_prms->inshape.width: %d\n", p_nvtnn_prms->inshape.width);
	printf("p_nvtnn_prms->inshape.height: %d\n", p_nvtnn_prms->inshape.height);
	printf("p_nvtnn_prms->inshape.channel: %d\n", p_nvtnn_prms->inshape.channel);
	printf("p_nvtnn_prms->inshape.batch_num: %d\n", p_nvtnn_prms->inshape.batch_num);
	printf("p_nvtnn_prms->outshape.width: %d\n", p_nvtnn_prms->outshape.width);
	printf("p_nvtnn_prms->outshape.height: %d\n", p_nvtnn_prms->outshape.height);
	printf("p_nvtnn_prms->outshape.channel: %d\n", p_nvtnn_prms->outshape.channel);
	printf("p_nvtnn_prms->outshape.batch_num: %d\n", p_nvtnn_prms->outshape.batch_num);
	printf("p_nvtnn_prms->wtshape.width: %d\n", p_nvtnn_prms->wtshape.width);
	printf("p_nvtnn_prms->wtshape.height: %d\n", p_nvtnn_prms->wtshape.height);
	printf("p_nvtnn_prms->wtshape.channel: %d\n", p_nvtnn_prms->wtshape.channel);
	printf("p_nvtnn_prms->wtshape.batch_num: %d\n", p_nvtnn_prms->wtshape.batch_num);

	printf("p_nvtnn_prms->in_ofs.line_ofs: %d\n", p_nvtnn_prms->in_ofs.line_ofs);
	printf("p_nvtnn_prms->in_ofs.channel_ofs: %d\n", p_nvtnn_prms->in_ofs.channel_ofs);
	printf("p_nvtnn_prms->in_ofs.batch_ofs: %d\n", p_nvtnn_prms->in_ofs.batch_ofs);
	printf("p_nvtnn_prms->weight_ofs.line_ofs: %d\n", p_nvtnn_prms->weight_ofs.line_ofs);
	printf("p_nvtnn_prms->weight_ofs.channel_ofs: %d\n", p_nvtnn_prms->weight_ofs.channel_ofs);
	printf("p_nvtnn_prms->weight_ofs.batch_ofs: %d\n", p_nvtnn_prms->weight_ofs.batch_ofs);
	printf("p_nvtnn_prms->out_ofs.line_ofs: %d\n", p_nvtnn_prms->out_ofs.line_ofs);
	printf("p_nvtnn_prms->out_ofs.channel_ofs: %d\n", p_nvtnn_prms->out_ofs.channel_ofs);
	printf("p_nvtnn_prms->out_ofs.batch_ofs: %d\n", p_nvtnn_prms->out_ofs.batch_ofs);
	printf("p_nvtnn_prms->in_fmt.frac_bits: %d\n", p_nvtnn_prms->in_fmt.frac_bits);
	printf("p_nvtnn_prms->in_fmt.int_bits: %d\n", p_nvtnn_prms->in_fmt.int_bits);
	printf("p_nvtnn_prms->in_fmt.sign_bits: %d\n", p_nvtnn_prms->in_fmt.sign_bits);
	printf("p_nvtnn_prms->out_fmt.frac_bits: %d\n", p_nvtnn_prms->out_fmt.frac_bits);
	printf("p_nvtnn_prms->out_fmt.int_bits: %d\n", p_nvtnn_prms->out_fmt.int_bits);
	printf("p_nvtnn_prms->out_fmt.sign_bits: %d\n", p_nvtnn_prms->out_fmt.sign_bits);
	printf("p_nvtnn_prms->weight_fmt.frac_bits: %d\n", p_nvtnn_prms->weight_fmt.frac_bits);
	printf("p_nvtnn_prms->weight_fmt.int_bits: %d\n", p_nvtnn_prms->weight_fmt.int_bits);
	printf("p_nvtnn_prms->weight_fmt.sign_bits: %d\n", p_nvtnn_prms->weight_fmt.sign_bits);
	if(p_nvtnn_prms->bias) {
		printf("p_nvtnn_prms->bias_fmt.frac_bits: %d\n", p_nvtnn_prms->bias_fmt.frac_bits);
		printf("p_nvtnn_prms->bias_fmt.int_bits: %d\n", p_nvtnn_prms->bias_fmt.int_bits);
		printf("p_nvtnn_prms->bias_fmt.sign_bits: %d\n", p_nvtnn_prms->bias_fmt.sign_bits);
		printf("p_nvtnn_prms->bias: %d\n", p_nvtnn_prms->bias);
	}
	printf("p_nvtnn_prms->isf.mul: %d\n", p_nvtnn_prms->isf.mul);
	printf("p_nvtnn_prms->isf.shf: %d\n", p_nvtnn_prms->isf.shf);
	printf("p_nvtnn_prms->wsf.mul: %d\n", p_nvtnn_prms->wsf.mul);
	printf("p_nvtnn_prms->wsf.shf: %d\n", p_nvtnn_prms->wsf.shf);
	printf("p_nvtnn_prms->bsf.mul: %d\n", p_nvtnn_prms->bsf.mul);
	printf("p_nvtnn_prms->bsf.shf: %d\n", p_nvtnn_prms->bsf.shf);
	printf("p_nvtnn_prms->osf.mul: %d\n", p_nvtnn_prms->osf.mul);
	printf("p_nvtnn_prms->osf.shf: %d\n", p_nvtnn_prms->osf.shf);

	printf("p_nvtnn_prms->kernel_w: %d\n", p_nvtnn_prms->kernel_w);
	printf("p_nvtnn_prms->kernel_h: %d\n", p_nvtnn_prms->kernel_h);
	printf("p_nvtnn_prms->stride_w: %d\n", p_nvtnn_prms->stride_w);
	printf("p_nvtnn_prms->stride_h: %d\n", p_nvtnn_prms->stride_h);
	printf("p_nvtnn_prms->dilation_w: %d\n", p_nvtnn_prms->dilation_w);
	printf("p_nvtnn_prms->dilation_h: %d\n", p_nvtnn_prms->dilation_h);
	printf("p_nvtnn_prms->pad_w: %d\n", p_nvtnn_prms->pad_w);
	printf("p_nvtnn_prms->pad_h: %d\n", p_nvtnn_prms->pad_h);
	printf("p_nvtnn_prms->group: %d\n", p_nvtnn_prms->group);
	printf("p_nvtnn_prms->num_output: %d\n", p_nvtnn_prms->num_output);
	printf("p_nvtnn_prms->output_padding: %d\n", p_nvtnn_prms->output_padding);
	#endif
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_deconvolution_init(NN_DLI_DECONVOLUTION_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_deconvolution_proc(NN_DLI_DECONVOLUTION_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_DECONVOLUTION_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_deconvolution(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_DLI_DECONVOLUTION, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_deconvolution_uninit(NN_DLI_DECONVOLUTION_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
