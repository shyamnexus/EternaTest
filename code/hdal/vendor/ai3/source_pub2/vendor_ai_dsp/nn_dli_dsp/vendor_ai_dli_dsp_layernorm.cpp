/**
    @brief Source file of DLI dsp layernorm layer.

    @file vendor_ai_dli_dsp_layernorm.cpp

    @ingroup dli_dsp_layernorm

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_layernorm.hpp"
#include "vendor_ai_dli_cpu_utils.hpp"
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

// VOID dli_nvtnn_dsp_flush_cache_layernorm(NVTNN_LAYER_NORMALIZATION_PRMS *p_nvtnn_prms) {
// 	UINT32 channofs = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
// 	if(p_nvtnn_prms->elementwise_affine) {
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->gamma_addr, channofs * sizeof(float));
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->beta_addr, channofs * sizeof(float));
// 	}
// 	UINT32 alllength = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height * p_nvtnn_prms->shape.channel * p_nvtnn_prms->shape.batch_num;
// 	UINT32 input_size = alllength * get_bytes_num(&p_nvtnn_prms->in_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr, (UINT32)input_size);

// 	UINT32 output_size = alllength * get_bytes_num(&p_nvtnn_prms->out_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->out_addr, (UINT32)output_size);

// 	UINT32 tmpsize = sizeof(FLOAT) * p_nvtnn_prms->out_ofs.batch_ofs;
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->tmp_addr, (UINT32)tmpsize);
// }

static VOID dli_to_nvtnn_layernorm(NN_DLI_LAYER_NORMALIZATION_PARM *p_dli_parm, NVTNN_LAYER_NORMALIZATION_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	NN_DLI_TENSOR_INFO *weight_info, *bias_info;
	NN_DLI_QUANTIZATION_INFO *weight_quant_info, *bias_quant_info;
	p_nvtnn_prms->elementwise_affine    = p_dli_parm->elementwise_affine;
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->shape.width           = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height          = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel         = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num       = input_info->shape[NN_DLI_AXIS_N];

	if(p_nvtnn_prms->elementwise_affine) {
		weight_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->weight_info_va;
		bias_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->bias_info_va;
		weight_quant_info  = (NN_DLI_QUANTIZATION_INFO *)weight_info->quant_info_va;
		bias_quant_info  = (NN_DLI_QUANTIZATION_INFO *)bias_info->quant_info_va;
	}
	p_nvtnn_prms->in_addr               = input_info->data_va;
	if(p_nvtnn_prms->elementwise_affine) {    //should convert to float firstly, dsp need float input vector for gamma, beta
		p_nvtnn_prms->gamma_addr        = weight_info->data_va;
		p_nvtnn_prms->beta_addr         = bias_info->data_va;
		p_nvtnn_prms->gamma_fmt         = weight_quant_info->fmt;
		p_nvtnn_prms->beta_fmt          = bias_quant_info->fmt;

		UINT32 channofs = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
		UINT32 weight_size = sizeof(FLOAT) * channofs;
		weight_info->trans_va = p_dli_parm->temp_buf_va;
		bias_info->trans_va = p_dli_parm->temp_buf_va + weight_size;

		dequantize_tensor(weight_info);
		dequantize_tensor(bias_info);

		p_nvtnn_prms->gamma_addr = weight_info->trans_va;
		p_nvtnn_prms->beta_addr  = bias_info->trans_va;
		p_dli_parm->temp_buf_va  = p_dli_parm->temp_buf_va + weight_size * 2;
	}
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->in_ofs.line_ofs       = p_nvtnn_prms->shape.width;
	p_nvtnn_prms->in_ofs.channel_ofs    = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
	p_nvtnn_prms->in_ofs.batch_ofs      = p_nvtnn_prms->in_ofs.channel_ofs * p_nvtnn_prms->shape.channel;
	p_nvtnn_prms->out_ofs.line_ofs      = p_nvtnn_prms->shape.width;
	p_nvtnn_prms->out_ofs.channel_ofs   = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height;
	p_nvtnn_prms->out_ofs.batch_ofs     = p_nvtnn_prms->in_ofs.channel_ofs * p_nvtnn_prms->shape.channel;
	p_nvtnn_prms->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf                   = input_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	p_nvtnn_prms->eps                   = p_dli_parm->eps;
	/*
		normalized_shape [293, 0, 0, 0, 0, 0], last dimension
		[293, 293, 0, 0, 0, 0] last two dimension
	*/
	for(UINT32 j = 0; j < NN_DLI_NORMALIZED_SHAPE_NUM; j++)
		p_nvtnn_prms->normalized_shape[j] = p_dli_parm->normalized_shape[j];
	if(p_nvtnn_prms->normalized_shape[NN_DLI_AXIS_H]==p_nvtnn_prms->shape.height && 
		p_nvtnn_prms->normalized_shape[NN_DLI_AXIS_W]==p_nvtnn_prms->shape.width){ //[2, 2, 293, 293]
	//pass
	}else if(p_nvtnn_prms->normalized_shape[NN_DLI_AXIS_H]==0 && 
			 p_nvtnn_prms->normalized_shape[NN_DLI_AXIS_W]==p_nvtnn_prms->shape.width){ //[2, 2, 293, 293] --->>> [2, 586, 1, 293]
		p_nvtnn_prms->shape.channel = p_nvtnn_prms->shape.channel * p_nvtnn_prms->shape.height;
		p_nvtnn_prms->shape.height  = 1;
		p_nvtnn_prms->in_ofs.line_ofs        = p_nvtnn_prms->shape.width;
		p_nvtnn_prms->in_ofs.channel_ofs     = p_nvtnn_prms->in_ofs.line_ofs * p_nvtnn_prms->shape.height;
		p_nvtnn_prms->in_ofs.batch_ofs       = p_nvtnn_prms->in_ofs.channel_ofs * p_nvtnn_prms->shape.channel;
		p_nvtnn_prms->out_ofs.line_ofs       = p_nvtnn_prms->shape.width;
		p_nvtnn_prms->out_ofs.channel_ofs    = p_nvtnn_prms->out_ofs.line_ofs * p_nvtnn_prms->shape.height;
		p_nvtnn_prms->out_ofs.batch_ofs      = p_nvtnn_prms->out_ofs.channel_ofs * p_nvtnn_prms->shape.channel;
	}else{
		// printf("warning, layer normalize don't support this normalized_shape. default [height, width]");
	}

	// dli_nvtnn_dsp_flush_cache_layernorm(p_nvtnn_prms);

	#if 0
	char out_file_dir1[360];
	sprintf(out_file_dir1, "input.txt");
	printf("((short *)(input_info->data_va))[0]: %f\n", ((short *)(input_info->data_va))[0]/(float)(1<<12));
	// UINTPTR x = (UINTPTR)1074085220;
	// printf("((short *)(input_info->data_va))[0]: %f\n", ((short *)(x))[0]/(float)(1<<12));
	FILE *fp1 = fopen(out_file_dir1, "w");
	if (fp1) {
		for(int n = 0; n < 256 * 3 * 3; n++) {
			fprintf(fp1, "%.9f\n", ((short *)(p_nvtnn_prms->in_addr))[n]/(float)(1<<16));
		}
		fclose(fp1);
	}
	printf("input1_info->data_va: %#x  p_nvtnn_prms->in_addr0: %#x\n", input_info->data_va, p_nvtnn_prms->in_addr);
	printf("output_info->data_va: %#x  p_nvtnn_prms->out_addr: %#x\n", output_info->data_va, p_nvtnn_prms->out_addr);
	printf("p_dli_parm->temp_buf_va: %#x\n", p_dli_parm->temp_buf_va);
	printf("p_nvtnn_prms->shape.width: %d\n", p_nvtnn_prms->shape.width);
	printf("p_nvtnn_prms->shape.height: %d\n", p_nvtnn_prms->shape.height);
	printf("p_nvtnn_prms->shape.channel: %d\n", p_nvtnn_prms->shape.channel);
	printf("p_nvtnn_prms->shape.batch_num: %d\n", p_nvtnn_prms->shape.batch_num);
	printf("input_info->shape[NN_DLI_AXIS_T]: %d\n", input_info->shape[NN_DLI_AXIS_T]);
	printf("p_nvtnn_prms->in_ofs.line_ofs: %d\n", p_nvtnn_prms->in_ofs.line_ofs);
	printf("p_nvtnn_prms->in_ofs.channel_ofs: %d\n", p_nvtnn_prms->in_ofs.channel_ofs);
	printf("p_nvtnn_prms->in_ofs.batch_ofs: %d\n", p_nvtnn_prms->in_ofs.batch_ofs);
	printf("p_nvtnn_prms->out_ofs.line_ofs: %d\n", p_nvtnn_prms->out_ofs.line_ofs);
	printf("p_nvtnn_prms->out_ofs.channel_ofs: %d\n", p_nvtnn_prms->out_ofs.channel_ofs);
	printf("p_nvtnn_prms->out_ofs.batch_ofs: %d\n", p_nvtnn_prms->out_ofs.batch_ofs);
	printf("p_nvtnn_prms->in_fmt.frac_bits: %d\n", p_nvtnn_prms->in_fmt.frac_bits);
	printf("p_nvtnn_prms->in_fmt.int_bits: %d\n", p_nvtnn_prms->in_fmt.int_bits);
	printf("p_nvtnn_prms->in_fmt.sign_bits: %d\n", p_nvtnn_prms->in_fmt.sign_bits);
	printf("p_nvtnn_prms->out_fmt.frac_bits: %d\n", p_nvtnn_prms->out_fmt.frac_bits);
	printf("p_nvtnn_prms->out_fmt.int_bits: %d\n", p_nvtnn_prms->out_fmt.int_bits);
	printf("p_nvtnn_prms->out_fmt.sign_bits: %d\n", p_nvtnn_prms->out_fmt.sign_bits);
	printf("p_nvtnn_prms->isf.mul: %d\n", p_nvtnn_prms->isf.mul);
	printf("p_nvtnn_prms->isf.shf: %d\n", p_nvtnn_prms->isf.shf);
	printf("p_nvtnn_prms->osf.mul: %d\n", p_nvtnn_prms->osf.mul);
	printf("p_nvtnn_prms->osf.shf: %d\n", p_nvtnn_prms->osf.shf);
	printf("p_nvtnn_prms->eps: %f\n", p_nvtnn_prms->eps);
	printf("p_nvtnn_prms->normalized_shape[0]: %d\n", p_nvtnn_prms->normalized_shape[0]);
	printf("p_nvtnn_prms->normalized_shape[1]: %d\n", p_nvtnn_prms->normalized_shape[1]);
	printf("p_nvtnn_prms->normalized_shape[2]: %d\n", p_nvtnn_prms->normalized_shape[2]);
	printf("p_nvtnn_prms->normalized_shape[3]: %d\n", p_nvtnn_prms->normalized_shape[3]);
	printf("p_nvtnn_prms->normalized_shape[4]: %d\n", p_nvtnn_prms->normalized_shape[4]);
	printf("p_nvtnn_prms->normalized_shape[5]: %d\n", p_nvtnn_prms->normalized_shape[5]);
	printf("p_dli_parm->elementwise_affine: %d\n", p_dli_parm->elementwise_affine);
	#endif
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_layernorm_init(NN_DLI_LAYER_NORMALIZATION_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_layernorm_proc(NN_DLI_LAYER_NORMALIZATION_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_LAYER_NORMALIZATION_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_layernorm(p_parm, &nvtnn_prms);
	HD_RESULT ret = nvtnndsp_op_proc(NN_DLI_LAYER_NORMALIZATION, &nvtnn_prms, dsp_core);

	// char out_file_dir1[360];
	// sprintf(out_file_dir1, "output.txt");
	// FILE *fp1 = fopen(out_file_dir1, "w");
	// if (fp1) {
	// 	for (int n = 0; n < 256*3*3; n++) {
	// 		fprintf(fp1, "%.9f\n", ((UINT8 *)(nvtnn_prms.out_addr))[n]/(float)(1<<6));
	// 	}
	// 	fclose(fp1);
	// }

	// Execute dsp
	return ret;
}

HD_RESULT dli_dsp_layernorm_uninit(NN_DLI_LAYER_NORMALIZATION_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}