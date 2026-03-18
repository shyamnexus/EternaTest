/**
    @brief Source file of DLI dsp instancenorm layer.

    @file vendor_ai_dli_dsp_instancenorm.cpp

    @ingroup dli_dsp_instancenorm

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2022.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "nvtnn/nvtnndsp_lib.h"
#include "vendor_ai_dli_dsp_instancenorm.hpp"
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

// VOID dli_nvtnn_dsp_flush_cache_instancenorm(NVTNN_INSTANCE_NORMALIZATION_PRMS *p_nvtnn_prms) {
// 	UINT32 channel = p_nvtnn_prms->shape.channel;
// 	UINT32 channelsize = sizeof(FLOAT) * channel;
// 	if(p_nvtnn_prms->track_running_stats) {
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->mean_addr, channelsize);
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->variance_addr, channelsize);
// 	}
// 	if(p_nvtnn_prms->affine) {
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->gamma_addr, channelsize);
// 		hd_common_mem_flush_cache((VOID *)p_nvtnn_prms->beta_addr, channelsize);
// 	}
// 	UINT32 alllength = p_nvtnn_prms->shape.width * p_nvtnn_prms->shape.height * p_nvtnn_prms->shape.channel * p_nvtnn_prms->shape.batch_num;
// 	UINT32 input_size = alllength * get_bytes_num(&p_nvtnn_prms->in_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->in_addr, (UINT32)input_size);

// 	UINT32 output_size = alllength * get_bytes_num(&p_nvtnn_prms->out_fmt);
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->out_addr, (UINT32)output_size);

// 	UINT32 tmpsize = sizeof(FLOAT) * p_nvtnn_prms->out_ofs.batch_ofs;
// 	hd_common_mem_flush_cache((void *)p_nvtnn_prms->tmp_addr, (UINT32)tmpsize);
// }

static VOID dli_to_nvtnn_instancenorm(NN_DLI_INSTANCE_NORMALIZATION_PARM *p_dli_parm, NVTNN_INSTANCE_NORMALIZATION_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info	   = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;

	p_nvtnn_prms->track_running_stats   = p_dli_parm->track_running_stats;
	p_nvtnn_prms->affine                = p_dli_parm->affine;
	p_nvtnn_prms->shape.width           = input_info->shape[NN_DLI_AXIS_W];
	p_nvtnn_prms->shape.height          = input_info->shape[NN_DLI_AXIS_H];
	p_nvtnn_prms->shape.channel         = input_info->shape[NN_DLI_AXIS_C];
	p_nvtnn_prms->shape.batch_num       = input_info->shape[NN_DLI_AXIS_N];

	NN_DLI_QUANTIZATION_INFO *mean_quant_info=NULL, *variance_quant_info=NULL, *gamma_quant_info=NULL, *beta_quant_info=NULL;
	NN_DLI_TENSOR_INFO *mean_info=NULL, *variance_info=NULL, *gamma_info=NULL, *beta_info=NULL;
	if(p_nvtnn_prms->track_running_stats) {
		mean_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->mean_info_va;
		variance_info   = (NN_DLI_TENSOR_INFO *)p_dli_parm->variance_info_va;
		mean_quant_info  = (NN_DLI_QUANTIZATION_INFO *)mean_info->quant_info_va;
		variance_quant_info  = (NN_DLI_QUANTIZATION_INFO *)variance_info->quant_info_va;
	}
	if(p_nvtnn_prms->affine) {
		gamma_info		= (NN_DLI_TENSOR_INFO *)p_dli_parm->gamma_info_va;
		beta_info       = (NN_DLI_TENSOR_INFO *)p_dli_parm->beta_info_va;
		gamma_quant_info  = (NN_DLI_QUANTIZATION_INFO *)gamma_info->quant_info_va;
		beta_quant_info  = (NN_DLI_QUANTIZATION_INFO *)beta_info->quant_info_va;
	}

	UINT32 channel = p_nvtnn_prms->shape.channel;
	UINT32 channelsize = sizeof(FLOAT) * channel;
	if(p_nvtnn_prms->track_running_stats) {    //should convert to float firstly, dsp need float input vector for mean, variance
		p_nvtnn_prms->mean_addr         = mean_info->data_va;
		p_nvtnn_prms->variance_addr     = variance_info->data_va;
		p_nvtnn_prms->mean_fmt          = mean_quant_info->fmt;
		p_nvtnn_prms->variance_fmt      = variance_quant_info->fmt;

		mean_info->trans_va = p_dli_parm->temp_buf_va;
		variance_info->trans_va = p_dli_parm->temp_buf_va + channelsize;

		dequantize_tensor(mean_info);
		dequantize_tensor(variance_info);

		p_nvtnn_prms->mean_addr  = mean_info->trans_va;
		p_nvtnn_prms->variance_addr  = variance_info->trans_va;
		p_dli_parm->temp_buf_va  = p_dli_parm->temp_buf_va + channelsize * 2;
	}
	if(p_nvtnn_prms->affine) {    //should convert to float firstly, dsp need float input vector for gamma, beta
		p_nvtnn_prms->gamma_addr        = gamma_info->data_va;
		p_nvtnn_prms->beta_addr         = beta_info->data_va;
		p_nvtnn_prms->gamma_fmt         = gamma_quant_info->fmt;
		p_nvtnn_prms->beta_fmt          = beta_quant_info->fmt;

		gamma_info->trans_va = p_dli_parm->temp_buf_va;
		beta_info->trans_va = p_dli_parm->temp_buf_va + channelsize;

		dequantize_tensor(gamma_info);
		dequantize_tensor(beta_info);

		p_nvtnn_prms->gamma_addr  = gamma_info->trans_va;
		p_nvtnn_prms->beta_addr  = beta_info->trans_va;
		p_dli_parm->temp_buf_va  = p_dli_parm->temp_buf_va + channelsize * 2;
	}
	p_nvtnn_prms->in_addr               = input_info->data_va;
	p_nvtnn_prms->out_addr              = output_info->data_va;
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

	// dli_nvtnn_dsp_flush_cache_instancenorm(p_nvtnn_prms);
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_dsp_instancenorm_init(NN_DLI_INSTANCE_NORMALIZATION_PARM *p_parm)
{
	//DBG_IND("%s: entry\r\n", __func__);

	return HD_OK;
}

HD_RESULT dli_dsp_instancenorm_proc(NN_DLI_INSTANCE_NORMALIZATION_PARM *p_parm, NVTNN_SENDTO dsp_core)
{
	NVTNN_INSTANCE_NORMALIZATION_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_instancenorm(p_parm, &nvtnn_prms);

	// Execute dsp
	return nvtnndsp_op_proc(NN_DLI_INSTANCE_NORMALIZATION, &nvtnn_prms, dsp_core);
}

HD_RESULT dli_dsp_instancenorm_uninit(NN_DLI_INSTANCE_NORMALIZATION_PARM *p_parm)
{
	// Do nothing

	return HD_OK;
}
