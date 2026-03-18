/**
    @brief Source file of DLI dsp elementwise layer.

    @file vendor_ai_dli_dsp_elementwise.cpp

    @ingroup dli_dsp_elementwise

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "string.h"
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_elementwise.hpp"
// #include "vendor_ai_dli_dsp_utils.hpp"
// #include "vendor_ai_dli_cpu_utils.hpp"
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
// VOID dli_nvtnn_dsp_flush_cache_elementwise(NVTNN_ELEMENTWISE_PRMS *p_nvtnn_prms) {
// 	UINT32 allnum = p_nvtnn_prms->in_ofs0.batch_ofs * p_nvtnn_prms->shape.batch_num;
// 	UINT32 input1_tmp_size = allnum * get_bytes_num(&p_nvtnn_prms->in_fmt0);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr0, (UINT32)input1_tmp_size);
	
// 	UINT32 outputsize = allnum * get_bytes_num(&p_nvtnn_prms->out_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->out_addr, (UINT32)outputsize);
// 	if((p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_SQRT)&&(p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_EXP)) {
// 		UINT32 input2_tmp_size = allnum * get_bytes_num(&p_nvtnn_prms->in_fmt1);
// 		hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr1, (UINT32)input2_tmp_size);
// 		hd_common_mem_flush_cache((void *)p_nvtnn_prms->tmp_addr, (UINT32)allnum * sizeof(FLOAT) * 2);
// 	} else {
// 		hd_common_mem_flush_cache((void *)p_nvtnn_prms->tmp_addr, (UINT32)allnum * sizeof(FLOAT));
// 	}
// }

static VOID dli_to_nvtnn_elementwise(NN_DLI_ElEMENTWISE_PARM *p_dli_parm, NVTNN_ELEMENTWISE_PRMS *p_nvtnn_prms)
{	
	NN_DLI_QUANTIZATION_INFO *input2_quant_info=NULL;
	NN_DLI_TENSOR_INFO *input2_info=NULL;
	p_nvtnn_prms->op                    = (NN_ELEMENTWISE_OP)(p_dli_parm->mode);
	NN_DLI_TENSOR_INFO *input1_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input1_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input1_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input1_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	p_nvtnn_prms->in_addr0              = input1_info->data_va;

	// input1_info->trans_va = p_dli_parm->temp_buf_va;
	// getdata_stride_slice(input1_info);
	// UINT32 inputbit1 = input1_quant_info->fmt.frac_bits + input1_quant_info->fmt.int_bits + input1_quant_info->fmt.sign_bits;
	// UINT32 input1_tmp_size = input1_info->shape[NN_DLI_AXIS_W] * input1_info->shape[NN_DLI_AXIS_H] * 
	//                          input1_info->shape[NN_DLI_AXIS_C] * input1_info->shape[NN_DLI_AXIS_N] * (inputbit1/8);
	// if(is_tensor_stride(input1_info)) {
	// 	p_nvtnn_prms->in_addr0          = input1_info->trans_va;
	// 	p_dli_parm->temp_buf_va         = p_dli_parm->temp_buf_va + input1_tmp_size;
	// }
	if((p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_SQRT)&&(p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_EXP)) {
		input2_info	                    = (NN_DLI_TENSOR_INFO *)p_dli_parm->input2_info_va;
		input2_quant_info               = (NN_DLI_QUANTIZATION_INFO *)input2_info->quant_info_va;
		p_nvtnn_prms->in_addr1          = input2_info->data_va;

		// for(int i = 0; i <NN_DLI_AXIS_NUM; i++) {
		// 	printf("sdk in input2_info->strides[%d]: %d\n", i, input2_info->strides[i]);
		// }
		// input2_info->trans_va           = p_dli_parm->temp_buf_va;
		// UINT32 inputbit2 = input2_quant_info->fmt.frac_bits + input2_quant_info->fmt.int_bits + input2_quant_info->fmt.sign_bits;
		// UINT32 input2_tmp_size = input2_info->shape[NN_DLI_AXIS_W] * input2_info->shape[NN_DLI_AXIS_H] * 
		// 						 input2_info->shape[NN_DLI_AXIS_C] * input2_info->shape[NN_DLI_AXIS_N] * (inputbit2/8);
		// getdata_stride_slice(input2_info);
		// if(is_tensor_stride(input2_info)) {
		// 	p_nvtnn_prms->in_addr1          = input2_info->trans_va;
		// 	p_dli_parm->temp_buf_va         = p_dli_parm->temp_buf_va + input2_tmp_size;
		// }
	}
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->shape.width           = input1_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height          = input1_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel         = input1_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num       = input1_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_ofs0.line_ofs       = input1_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->in_ofs0.channel_ofs    = input1_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->in_ofs0.batch_ofs      = input1_info->strides[NN_DLI_STRIDE_N];
	if((p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_SQRT)&&(p_nvtnn_prms->op!=NVTNN_DLI_ELEMENTWISE_OP_EXP)) {
		p_nvtnn_prms->in_ofs1.line_ofs      = input2_info->strides[NN_DLI_STRIDE_H];
		p_nvtnn_prms->in_ofs1.channel_ofs   = input2_info->strides[NN_DLI_STRIDE_C];
		p_nvtnn_prms->in_ofs1.batch_ofs     = input2_info->strides[NN_DLI_STRIDE_N];
		p_nvtnn_prms->in_fmt1               = input2_quant_info->fmt;
		p_nvtnn_prms->isf1                  = input2_quant_info->sf;
	}
	p_nvtnn_prms->out_ofs.line_ofs      = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs   = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs     = output_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->in_fmt0               = input1_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf0                  = input1_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;

	// hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr0, (UINT32)input1_tmp_size*2);
	// dli_nvtnn_dsp_flush_cache_elementwise(p_nvtnn_prms);
// #if 0
// #if ELTWISE_SAVEMID_IO
// 	char out_file_dir1[160];
// 	sprintf(out_file_dir1, "%d_%d.txt", p_nvtnn_prms->in_fmt0.frac_bits, p_nvtnn_prms->op);
// 	FILE *fp1 = fopen(out_file_dir1, "w");
// 	if (fp1) {
// 		for (int n = 0; n < 6000; n++) {
// 			fprintf(fp1, "%.9f\n", ((short *)(p_nvtnn_prms->in_addr0))[n]/(float)(1<<14));
// 		}
// 		fclose(fp1);
// 	}
// #endif
// 	// printf("input1_info->data_va: %ld  p_nvtnn_prms->in_addr0: %ld\n", input1_info->data_va, p_nvtnn_prms->in_addr0);
// 	printf("p_nvtnn_prms->shape.width: %d\n", p_nvtnn_prms->shape.width);
// 	printf("p_nvtnn_prms->shape.height: %d\n", p_nvtnn_prms->shape.height);
// 	printf("p_nvtnn_prms->shape.channel: %d\n", p_nvtnn_prms->shape.channel);
// 	printf("p_nvtnn_prms->shape.batch_num: %d\n", p_nvtnn_prms->shape.batch_num);
// 	printf("p_nvtnn_prms->in_ofs0.line_ofs: %d\n", p_nvtnn_prms->in_ofs0.line_ofs);
// 	printf("p_nvtnn_prms->in_ofs0.channel_ofs: %d\n", p_nvtnn_prms->in_ofs0.channel_ofs);
// 	printf("p_nvtnn_prms->in_ofs0.batch_ofs: %d\n", p_nvtnn_prms->in_ofs0.batch_ofs);
// 	printf("p_nvtnn_prms->in_ofs1.line_ofs: %d\n", p_nvtnn_prms->in_ofs1.line_ofs);
// 	printf("p_nvtnn_prms->in_ofs1.channel_ofs: %d\n", p_nvtnn_prms->in_ofs1.channel_ofs);
// 	printf("p_nvtnn_prms->in_ofs1.batch_ofs: %d\n", p_nvtnn_prms->in_ofs1.batch_ofs);
// 	printf("p_nvtnn_prms->out_ofs.line_ofs: %d\n", p_nvtnn_prms->out_ofs.line_ofs);
// 	printf("p_nvtnn_prms->out_ofs.channel_ofs: %d\n", p_nvtnn_prms->out_ofs.channel_ofs);
// 	printf("p_nvtnn_prms->out_ofs.batch_ofs: %d\n", p_nvtnn_prms->out_ofs.batch_ofs);
// 	printf("p_nvtnn_prms->in_fmt0.frac_bits: %d\n", p_nvtnn_prms->in_fmt0.frac_bits);
// 	printf("p_nvtnn_prms->in_fmt0.int_bits: %d\n", p_nvtnn_prms->in_fmt0.int_bits);
// 	printf("p_nvtnn_prms->in_fmt0.sign_bits: %d\n", p_nvtnn_prms->in_fmt0.sign_bits);
// 	printf("p_nvtnn_prms->in_fmt1.frac_bits: %d\n", p_nvtnn_prms->in_fmt1.frac_bits);
// 	printf("p_nvtnn_prms->in_fmt1.int_bits: %d\n", p_nvtnn_prms->in_fmt1.int_bits);
// 	printf("p_nvtnn_prms->in_fmt1.sign_bits: %d\n", p_nvtnn_prms->in_fmt1.sign_bits);
// 	printf("p_nvtnn_prms->out_fmt.frac_bits: %d\n", p_nvtnn_prms->out_fmt.frac_bits);
// 	printf("p_nvtnn_prms->out_fmt.int_bits: %d\n", p_nvtnn_prms->out_fmt.int_bits);
// 	printf("p_nvtnn_prms->out_fmt.sign_bits: %d\n", p_nvtnn_prms->out_fmt.sign_bits);
// 	printf("p_nvtnn_prms->isf0.mul: %d\n", p_nvtnn_prms->isf0.mul);
// 	printf("p_nvtnn_prms->isf0.shf: %d\n", p_nvtnn_prms->isf0.shf);
// 	printf("p_nvtnn_prms->isf1.mul: %d\n", p_nvtnn_prms->isf1.mul);
// 	printf("p_nvtnn_prms->isf1.shf: %d\n", p_nvtnn_prms->isf1.shf);
// 	printf("p_nvtnn_prms->osf.mul: %d\n", p_nvtnn_prms->osf.mul);
// 	printf("p_nvtnn_prms->osf.shf: %d\n", p_nvtnn_prms->osf.shf);
// 	printf("p_nvtnn_prms->op: %d\n", p_nvtnn_prms->op);
// 	#endif
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_elementwise_init(NN_DLI_ElEMENTWISE_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_elementwise_proc(NN_DLI_ElEMENTWISE_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_ELEMENTWISE_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);
	// Convert parameters
	dli_to_nvtnn_elementwise(p_parm, &nvtnn_prms);
	// Execute dsp
	HD_RESULT ret = nvtnndsp_op_proc((NN_MODE)nvtnn_prms.op, &nvtnn_prms, dsp_core);
	return ret;
}

HD_RESULT dli_dsp_elementwise_uninit(NN_DLI_ElEMENTWISE_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}