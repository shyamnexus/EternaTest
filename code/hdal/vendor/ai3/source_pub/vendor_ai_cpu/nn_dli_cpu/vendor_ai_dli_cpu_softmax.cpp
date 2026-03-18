/**
	@brief Source file of DLI cpu softmax layer.

	@file vendor_ai_dli_cpu_softmax.cpp

	@ingroup dli_cpu_softmax

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#if NN_DLI_CPU_ACL_SOFTMAX
#include <iostream>
#include <map>
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_cpu][dli_softmax]"
#include "vendor_ai_debug.h"
//=============================================================
#include "vendor_ai_dli_cpu_utils.hpp"
#include <arm_compute/runtime/NEON/functions/NESoftmaxLayer.h> // for arm_compute::NESoftmaxLayer
#elif NN_DLI_CPU_NVTNN_SOFTMAX
#include "nvtnn/nvtnn_lib.h"
#endif

#include "vendor_ai_dli_cpu_softmax.hpp"

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
#if NN_DLI_CPU_ACL_SOFTMAX
// Do nothing
#elif NN_DLI_CPU_NVTNN_SOFTMAX
static VOID dli_to_nvtnn_softmax(NN_DLI_SOFTMAX_PARM *p_dli_parm, NVTNN_SOFTMAX_PRMS *p_nvtnn_prms)
{
	NN_DLI_TENSOR_INFO *input_info      = (NN_DLI_TENSOR_INFO *)p_dli_parm->input_info_va;
	NN_DLI_TENSOR_INFO *output_info     = (NN_DLI_TENSOR_INFO *)p_dli_parm->output_info_va;
	NN_DLI_QUANTIZATION_INFO *input_quant_info  = (NN_DLI_QUANTIZATION_INFO *)input_info->quant_info_va;
	NN_DLI_QUANTIZATION_INFO *output_quant_info = (NN_DLI_QUANTIZATION_INFO *)output_info->quant_info_va;
	
	p_nvtnn_prms->in_addr               = input_info->data_va;
	p_nvtnn_prms->out_addr              = output_info->data_va;
	p_nvtnn_prms->tmp_addr              = p_dli_parm->temp_buf_va;
	p_nvtnn_prms->shape.width           = input_info->shape[NN_DLI_AXIS_W];  // 0
	p_nvtnn_prms->shape.height          = input_info->shape[NN_DLI_AXIS_H];  // 1
	p_nvtnn_prms->shape.channel         = input_info->shape[NN_DLI_AXIS_C];  // 2
	p_nvtnn_prms->shape.batch_num       = input_info->shape[NN_DLI_AXIS_N];  // 3
	p_nvtnn_prms->in_ofs.line_ofs       = input_info->strides[NN_DLI_STRIDE_H];  // 1
	p_nvtnn_prms->in_ofs.channel_ofs    = input_info->strides[NN_DLI_STRIDE_C];  // 2
	p_nvtnn_prms->in_ofs.batch_ofs      = input_info->strides[NN_DLI_STRIDE_N];  // 3
	p_nvtnn_prms->out_ofs.line_ofs      = output_info->strides[NN_DLI_STRIDE_H];
	p_nvtnn_prms->out_ofs.channel_ofs   = output_info->strides[NN_DLI_STRIDE_C];
	p_nvtnn_prms->out_ofs.batch_ofs     = output_info->strides[NN_DLI_STRIDE_N];
	p_nvtnn_prms->in_fmt                = input_quant_info->fmt;
	p_nvtnn_prms->out_fmt               = output_quant_info->fmt;
	p_nvtnn_prms->isf                   = input_quant_info->sf;
	p_nvtnn_prms->osf                   = output_quant_info->sf;
	// TODO                             = p_dli_parm->beta;
	p_nvtnn_prms->axis                  = NN_AXIS_NUM - 1 - p_dli_parm->axis; // reverse the order
}
#endif
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT dli_cpu_softmax_init(NN_DLI_SOFTMAX_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
	//DBG_IND(" Beta: %f, Axis: %d\n", p_parm->beta, p_parm->axis);
#if NN_DLI_CPU_ACL_SOFTMAX
	UINTPTR input_obj_ptr = build_acl_tensor_f32only(
		reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->input_info_va));
	UINTPTR output_obj_ptr = build_acl_tensor_f32only(
		reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->output_info_va));

	auto p_layer = new arm_compute::NESoftmaxLayer();

	p_layer->configure(
		reinterpret_cast<arm_compute::ITensor *>(input_obj_ptr),
		reinterpret_cast<arm_compute::ITensor *>(output_obj_ptr),
		p_parm->beta,
		p_parm->axis);

	p_parm->obj_ptr = reinterpret_cast<UINTPTR>(p_layer);

	return HD_OK;
#elif NN_DLI_CPU_NVTNN_SOFTMAX
    return HD_OK;
#endif
}

HD_RESULT dli_cpu_softmax_proc(NN_DLI_SOFTMAX_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
#if NN_DLI_CPU_ACL_SOFTMAX
	NN_DLI_TENSOR_INFO *p_input_info = reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->input_info_va);
	NN_DLI_TENSOR_INFO *p_output_info = reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->output_info_va);

	// Calculate temp buffer
	// Temp buffer is shared by inputs and outputs
	UINT32 input_tmp_size = sizeof(FLOAT) * get_tensor_num_element(p_input_info);
	p_input_info->trans_va = p_parm->temp_buf_va;
	p_output_info->trans_va = p_parm->temp_buf_va + input_tmp_size;

	// Dequantize input data: Fix to Float
	dequantize_tensor(p_input_info);

	// Import memory
	import_acl_tensor_f32only(p_input_info);
	import_acl_tensor_f32only(p_output_info);

	// Execute workload
	arm_compute::IFunction *p_layer = reinterpret_cast<arm_compute::IFunction *>(p_parm->obj_ptr);
	p_layer->run();

	// Quantize output data: Float to Fix
	quantize_tensor(p_output_info);

	// Debug: Show raw data
	//show_raw_data<INT16>(" %d", p_parm->output_info.data_va, get_tensor_num_element(&(p_parm->output_info)));

	return HD_OK;
#elif NN_DLI_CPU_NVTNN_SOFTMAX
    NVTNN_SOFTMAX_PRMS nvtnn_prms = {0};

	//DBG_IND("%s: entry\r\n", __func__);

	// Convert parameters
	dli_to_nvtnn_softmax(p_parm, &nvtnn_prms);

	// Execute cpu
	return nvtnn_softmax_process(&nvtnn_prms);
#endif
}

HD_RESULT dli_cpu_softmax_uninit(NN_DLI_SOFTMAX_PARM *p_parm)
{
	//DBG_IND("entry\r\n");
#if NN_DLI_CPU_ACL_SOFTMAX
	NN_DLI_TENSOR_INFO *p_input_info = reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->input_info_va);
	NN_DLI_TENSOR_INFO *p_output_info = reinterpret_cast<NN_DLI_TENSOR_INFO *>(p_parm->output_info_va);

	destroy_acl_tensor(p_input_info);
	destroy_acl_tensor(p_output_info);

	return HD_OK;
#elif NN_DLI_CPU_NVTNN_SOFTMAX
    return HD_OK;
#endif
}
