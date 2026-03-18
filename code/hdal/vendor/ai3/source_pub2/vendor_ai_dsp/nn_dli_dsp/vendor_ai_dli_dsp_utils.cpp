/**
	@brief Source file of DLI dsp utility functions.

	@file vendor_ai_dli_dsp_utils.cpp

	@ingroup dli_dsp_utils

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include <cstring>

#include "vendor_ai_util.h"
#include "nvtnn/nvtnn_lib.h"
#include "vendor_ai_dli_dsp_utils.hpp"
#include "vendor_ai_dli_cpu_utils.hpp"
/*-----------------------------------------------------------------------------*/
/* Local Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define DEBUG_INFO 0 // 0: off, 1: on

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

static VOID calculate_strides(
	const UINT32 shape[NN_DLI_AXIS_NUM],
	UINT32 strides[NN_DLI_AXIS_NUM],
	UINT32 element_size)
{
	strides[0]=element_size;
	for(UINT32 i=1;i<NN_DLI_AXIS_NUM;i++) {
		if(shape[i]==0)
			break;
		strides[i]=shape[i-1]*strides[i-1];
	}
	return;
}
// get slice element from origin input through stride
static VOID slice_stride(UINTPTR p_in /*FLOAT*/, UINTPTR p_out /*VOID*/, UINT32 dim,
	const UINT32 shape[NN_DLI_AXIS_NUM],
	const UINT32 in_strides[NN_DLI_AXIS_NUM],
	const UINT32 out_strides[NN_DLI_AXIS_NUM],
	UINT32 inputbit, UINT32 sign_bits)
{
	if(shape[dim] == 0) {
		DBG_ERR("shape[%d] should not be 0\n", dim);
		return;
	}

	if(dim == 0) {
		if(inputbit==16 && sign_bits==1)
			*((short *)p_out) = *((short *)p_in);
		else if(inputbit==16 && sign_bits==0)
			*((unsigned short *)p_out) = *((unsigned short *)p_in);
		else if(inputbit==8 && sign_bits==1)
			*((char *)p_out) = *((char *)p_in);
		else if(inputbit==8 && sign_bits==0)
			*((unsigned char *)p_out) = *((unsigned char *)p_in);
		return;
	}
	// printf("in_strides[%d]=%d, out_strides[%d]=%d\n", dim, in_strides[dim], dim, out_strides[dim]);
	// exit(0);
	/*
	                       info short          out float reverse
	dim 0  p_info->shape: 1,     p_info->strides: 2,      out_strides: 2
	dim 1  p_info->shape: 2,     p_info->strides: 2,      out_strides: 2
	dim 2  p_info->shape: 3000,  p_info->strides: eight,  out_strides: four
	dim 3  p_info->shape: 1,     p_info->strides: 24000,  out_strides: 12000
	*/
	for(UINT32 i=0;i<shape[dim];i++) {
		slice_stride(p_in, p_out, dim - 1, shape, in_strides, out_strides, inputbit, sign_bits);
		p_in+=in_strides[dim];
		p_out+=out_strides[dim];
	}
	return;
}

// get slice element from origin input through stride
VOID getdata_stride_slice(NN_DLI_TENSOR_INFO *p_info) {
	if(p_info == nullptr)
		return;
	if(is_quantized_type_all(p_info->data_type) == FALSE)
		return;
	if(p_info->quant_info_nums == 0)
		return;

	NN_DLI_QUANTIZATION_INFO *p_quant_info =
		reinterpret_cast<NN_DLI_QUANTIZATION_INFO *>(p_info->quant_info_va);

	UINT32 inputbit = p_quant_info->fmt.frac_bits + p_quant_info->fmt.int_bits + p_quant_info->fmt.sign_bits;
	UINT32 sign_bits = p_quant_info->fmt.sign_bits;
	if(is_tensor_stride(p_info)) {
		UINT32 out_strides[NN_DLI_AXIS_NUM]={};
		INT32 num_dim=NN_DLI_AXIS_NUM;
		
		calculate_strides(p_info->shape, out_strides, inputbit/8);
		while(num_dim > 1 && p_info->shape[num_dim-1]==0) {
			num_dim--;
		}
		
		// for(int i =0 ; i < NN_DLI_AXIS_NUM; i++) {
		// 	printf("p_info->shape: %d, p_info->strides: %d, out_strides: %d\n", p_info->shape[i], p_info->strides[i], out_strides[i]);
		// }

		slice_stride(p_info->data_va, p_info->trans_va, num_dim-1,
			p_info->shape, p_info->strides, out_strides,
			inputbit, sign_bits);
	}
	return;
}