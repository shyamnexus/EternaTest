/**
	@brief Source file of NvtNN layers.

	@file net_nvtnn_layer.c

	@ingroup nvtnnnn

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "hd_type.h"
//#include "vendor_ai_util.h"
//=============================================================
#define __CLASS__ 				"[ai][lib_dsp][nvtnn]"
#include "vendor_ai_debug.h"
//=============================================================

#include "vendor_ai_dsp_nvtnn.h"

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define CUST_PRINT_PARM     TRUE

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT vendor_ai_dsp_nvtnn(NN_GEN_ENG_TYPE eng, uintptr_t parm_addr, UINT32 net_id)
{
	/*
		custom parm format:
		1. custom_layer_type_id(UINT32)
		2. weight_num(UINT32), weight_size_1(UINT32), weight_size_2(UINT32), ...
		   (weights at p_head->model.va)
		3. str_parm_len(UINT32), bin_parm_len(UINT32), str_parm, bin_parm
	*/
	NN_CUSTOM_PARM *p_head  = (NN_CUSTOM_PARM *)(parm_addr);
	UINT32 *p_layer_type_id = (UINT32 *)(p_head + 1);
	UINT32  layer_type_id   = *p_layer_type_id;

	switch (layer_type_id & NN_LAYER_TYPE_ID_MASK) {
#if NN_USE_DSP
	case NVTNN_PRELU: {
			NVTNN_PRELU_PRMS *p_parm = p_bin_parm;
			p_parm->in_addr    = p_head->input.pa;
			p_parm->out_addr   = p_head->output.pa;
			p_parm->slope_addr = p_head->model.pa;
			nvtnn_prelu_process(p_parm, NVTNN_SENDTO_DSP1);
		}
		break;
#endif
	default:
		DBG_DUMP("DSP layer type id %d: not support\r\n", (INT)layer_type_id);
		return HD_ERR_NOT_SUPPORT;
	}

	return HD_OK;
}
