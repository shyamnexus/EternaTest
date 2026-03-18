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

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
extern HD_RESULT vendor_ai_dsp_cust_init  (uintptr_t parm_addr, UINT32 net_id);
extern HD_RESULT vendor_ai_dsp_cust_uninit(uintptr_t parm_addr, UINT32 net_id);
extern HD_RESULT vendor_ai_dsp_cust(uintptr_t parm_addr, UINT32 net_id);
extern HD_RESULT vendor_ai_dsp_cust_set_tmp_buf(NN_DATA_V30 tmp_buf, uintptr_t parm_addr);
extern HD_RESULT vendor_ai_dsp_nvtnn(uintptr_t parm_addr, UINT32 net_id);

#endif  /* _CUSTOM_NN_H_ */
