/**
	@brief Source file of kflow_ai_net.

	@file kflow_ai_jmisp.h

	@ingroup kflow_ai_net

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#ifndef _KFLOW_AI_JMISP_H_
#define _KFLOW_AI_JMISP_H_

#include "kwrap/type.h"
#include "kwrap/platform.h"
#include "kwrap/error_no.h"
#include "kflow_ai_net/kflow_ai_core.h" //for LIST_HEAD

//current kflow
extern KFLOW_AI_ENGINE_CTX* kflow_jmisp_get_engine(void);
extern int kflow_jmisp_set_output_path(char *path);
extern INT32 kflow_jmisp_copy_timeline_info_and_run_next(UINT32 cycle_eng, KFLOW_AI_ENGINE_ID kflow_ai_eng_id);

#endif //_KFLOW_AI_JMISP_H_
