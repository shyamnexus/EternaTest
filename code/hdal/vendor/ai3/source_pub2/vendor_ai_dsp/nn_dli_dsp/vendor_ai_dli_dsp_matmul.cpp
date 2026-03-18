/**
    @brief Source file of DLI dsp matmul layer.

    @file vendor_ai_dli_dsp_matmul.cpp

    @ingroup dli_dsp_matmul

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_matmul.hpp"
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

// VOID dli_nvtnn_dsp_flush_cache_matmul(NVTNN_MATMUL_PRMS *p_nvtnn_prms) {
// 	UINT32 in_size0 = p_nvtnn_prms->in_ofs0.batch_ofs * p_nvtnn_prms->in_shape0.batch_num;
// 	UINT32 in_size1 = p_nvtnn_prms->in_ofs1.batch_ofs * p_nvtnn_prms->in_shape1.batch_num;
// 	UINT32 out_size = p_nvtnn_prms->in_shape0.width * p_nvtnn_prms->in_shape1.height * p_nvtnn_prms->in_shape0.channel * p_nvtnn_prms->in_shape0.batch_num;
// 	UINT32 input0_size = in_size0 * get_bytes_num(&p_nvtnn_prms->in_fmt0);
// 	UINT32 input1_size = in_size1 * get_bytes_num(&p_nvtnn_prms->in_fmt1);
// 	UINT32 output_size = out_size * get_bytes_num(&p_nvtnn_prms->out_fmt);
// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->in_addr0, input0_size);
// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->in_addr1, input1_size);
// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->in_addr1, output_size);
// 	hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->in_addr1, output_size);
// }

static VOID dli_to_nvtnn_matmul(NN_DLI_MATMUL_PARM *p_dli_parm, NVTNN_MATMUL_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input1_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input1_info_va;
	NN_DLI_TENSOR_INFO *input2_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input2_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input1_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input1_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *input2_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input2_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	
	p_nvtnn_prms->in_addr0               = input1_info->data_va;
	p_nvtnn_prms->in_addr1               = input2_info->data_va;
	p_nvtnn_prms->out_addr               = output_info->data_va;
	p_nvtnn_prms->tmp_addr               = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->in_shape0.width        = input1_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->in_shape0.height       = input1_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->in_shape0.channel      = input1_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->in_shape0.batch_num    = input1_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_shape1.width        = input2_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->in_shape1.height       = input2_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->in_shape1.channel      = input2_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->in_shape1.batch_num    = input2_info->shape[NN_DLI_AXIS_N];
	p_nvtnn_prms->in_ofs0.line_ofs       = p_nvtnn_prms->in_shape0.width;
	p_nvtnn_prms->in_ofs0.channel_ofs    = p_nvtnn_prms->in_shape0.width * p_nvtnn_prms->in_shape0.height;
	p_nvtnn_prms->in_ofs0.batch_ofs      = p_nvtnn_prms->in_ofs0.channel_ofs * p_nvtnn_prms->in_shape0.channel;
	p_nvtnn_prms->in_ofs1.line_ofs       = p_nvtnn_prms->in_shape1.width;
	p_nvtnn_prms->in_ofs1.channel_ofs    = p_nvtnn_prms->in_shape1.width * p_nvtnn_prms->in_shape1.height;
	p_nvtnn_prms->in_ofs1.batch_ofs      = p_nvtnn_prms->in_ofs1.channel_ofs * p_nvtnn_prms->in_shape1.channel;

	/*
	line_ofs: width

	p_nvtnn_prms->in_shape0.width:       900
	p_nvtnn_prms->in_shape0.height:      100
	p_nvtnn_prms->in_shape0.channel:     1     
	p_nvtnn_prms->in_shape0.batch_num:   1     

	p_nvtnn_prms->in_shape0.width:       100
	p_nvtnn_prms->in_shape0.height:      60
	p_nvtnn_prms->in_shape0.channel:     1     
	p_nvtnn_prms->in_shape0.batch_num:   1 

	output_width:        900
	output_height:       60
	output_channel:      1
	output_batch_num:    1
	output need transpose 
	*/
	p_nvtnn_prms->out_ofs.line_ofs       = p_nvtnn_prms->in_shape0.width;
	p_nvtnn_prms->out_ofs.channel_ofs    = p_nvtnn_prms->in_shape0.width * p_nvtnn_prms->in_shape1.height;
	p_nvtnn_prms->out_ofs.batch_ofs      = p_nvtnn_prms->out_ofs.channel_ofs * p_nvtnn_prms->in_shape0.channel;

	p_nvtnn_prms->in_fmt0                = input1_quant_info->fmt;
	p_nvtnn_prms->in_fmt1                = input2_quant_info->fmt;
	p_nvtnn_prms->out_fmt                = output_quant_info->fmt;
	p_nvtnn_prms->isf0                   = input1_quant_info->sf;
	p_nvtnn_prms->isf1                   = input2_quant_info->sf;
	p_nvtnn_prms->osf                    = output_quant_info->sf;

	// dli_nvtnn_dsp_flush_cache_matmul(p_nvtnn_prms);
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_matmul_init(NN_DLI_MATMUL_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_matmul_proc(NN_DLI_MATMUL_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_MATMUL_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_matmul(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_DLI_MATMUL, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_matmul_uninit(NN_DLI_MATMUL_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}