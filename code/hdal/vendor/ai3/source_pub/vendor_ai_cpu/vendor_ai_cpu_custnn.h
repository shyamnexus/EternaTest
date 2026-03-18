/**
	@brief Header file of definition of network custom layer.

	@file custnn.h

	@ingroup custnn

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _CUSTOM_NN_H_
#define _CUSTOM_NN_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "vendor_ai_net/nn_verinfo.h"
#include "vendor_ai_net/nn_net.h"
#include "vendor_ai_net/nn_parm.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define CUST_DYNNAMIC_SHPAE    0  /* specific customer layer test flow for dynamic output shape update */

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern HD_RESULT vendor_ai_cpu_cust_init  (uintptr_t parm_addr, UINT32 net_id);
extern HD_RESULT vendor_ai_cpu_cust_uninit(uintptr_t parm_addr, UINT32 net_id);
extern HD_RESULT vendor_ai_cpu_cust(uintptr_t parm_addr, UINT32 net_id, UINT32 layer_id, VOID* usr_info);
extern HD_RESULT vendor_ai_cpu_cust_set_tmp_buf(NN_DATA tmp_buf, uintptr_t parm_addr);
extern HD_RESULT vendor_ai_cpu_cust_set_out_dim(UINT32 proc_id, UINT32 layer_id, UINT32 out_id, NN_CUSTOM_DIM* p_output_dim);
#define CUST_MAX_OUT_NUM 8
#define CUST_MAX_MAP_NUM 8
extern UINT32 vendor_ai_cpu_cust_get_layer_id(uintptr_t parm_addr);
extern HD_RESULT _vendor_ais_get_net_input_info_list(UINT32 proc_id, VENDOR_AI_NET_INPUT_INFO *p_info);

extern HD_RESULT vendor_ai_cust_set_next_info(UINT32 max_out, UINT32 max_map, uintptr_t* next_cust_parm_addr, UINT32* next_cust_match_in_idx);
extern HD_RESULT vendor_ai_cust_set_next_process_mctrl(uintptr_t next_parm_addr, UINT32 next_match_in_idx, UINT32 out_id, UINT32 map_id, UINT32 net_id);
extern HD_RESULT vendor_ai_cpu_cust_set_next_parm_addr(UINT32 proc_id, UINT32 layer_id);

#if (CUST_PROC_ALLOC_MEM==1)
extern HD_RESULT _vendor_ai_net_set_usr_context(UINT32 proc_id,  uintptr_t* next_cust_parm_addr, UINT32* next_cust_match_in_idx);
extern HD_RESULT _vendor_ai_net_get_usr_context(UINT32 proc_id,  uintptr_t** next_cust_parm_addr, UINT32** next_cust_match_in_idx);
#endif

extern HD_RESULT vendor_ai_cpu_nvtnn(uintptr_t parm_addr, UINT32 net_id);

#endif  /* _CUSTOM_NN_H_ */
