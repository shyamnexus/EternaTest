/**
	@brief Header file of vendor ai net cat.

	@file vendor_ai_net_cat.h

	@ingroup vendor_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _VENDOR_AI_NET_CAT_H_
#define _VENDOR_AI_NET_CAT_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "kflow_ai_net/kflow_ai_net.h"

/********************************************************************
	TYPE DEFINITION
********************************************************************/


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_ai_cat_init(UINT32 net_id);
HD_RESULT vendor_ai_cat_uninit(UINT32 net_id);

#endif  /* _VENDOR_AI_NET_CAT_H_ */
