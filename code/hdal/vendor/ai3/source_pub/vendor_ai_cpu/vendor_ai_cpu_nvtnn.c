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
#define __CLASS__ 				"[ai][lib_cpu][nvtnn]"
#include "vendor_ai_debug.h"
//=============================================================

#include "vendor_ai_cpu_nvtnn.h"
#include "vendor_ai_cpu/vendor_ai_cpu_builtin.h"

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define CUST_PRINT_PARM     TRUE

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
HD_RESULT vendor_ai_cpu_nvtnn(NN_GEN_ENG_TYPE eng, uintptr_t parm_addr, UINT32 net_id)
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
	/*
	case NVTNN_PRELU: {
		}
		break;
	*/
	default:
		DBG_DUMP("CPU layer type id %d: not support\r\n", (INT)layer_type_id);
		return HD_ERR_NOT_SUPPORT;
	}

	return HD_OK;
}
