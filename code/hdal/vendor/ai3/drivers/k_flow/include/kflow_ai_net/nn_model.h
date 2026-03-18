/**
    @brief Header file of NN engine definition of vendor net flow sample.

    @file nn_model.h

    @ingroup net_flow_sample

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/
#ifndef _NN_MODEL_H_
#define _NN_MODEL_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
//#include "nn_verinfo.h"
#include "kwrap/type.h"
#include "kdrv_ai.h"

/*-----------------------------------------------------------------------------*/
/* old kdrv_ai define/struct                                                   */
/*-----------------------------------------------------------------------------*/
#ifdef KFLOW_AI_VP_ENV
#define AI_SUPPORT_NET_MAX			4
#else
#define AI_SUPPORT_NET_MAX			128
#endif 
#define kdrv_ai_drv_get_net_supported_num()    (AI_SUPPORT_NET_MAX)

#define AI_SYNC_ALIGN_CEIL(a) ALIGN_CEIL_32(a)
#define AI_SYNC_ALIGN_FLOOR(a) ALIGN_FLOOR_32(a)



typedef enum {
    AI_MODE_NULL = 0,
    AI_MODE_NEURAL = 1,
    AI_MODE_ROI_POOL,
    AI_MODE_SVM,
    AI_MODE_FC,
    AI_MODE_PERMUTE,
    AI_MODE_REORG,
    AI_MODE_ANCHOR,
    AI_MODE_SOFTMAX,
    AI_MODE_PREPROC
    // ENUM_DUMMY4WORD(KDRV_AI_MODE)
} KDRV_AI_MODE;

typedef struct _KDRV_AI_LL_HEAD {
	uintptr_t parm_addr;            ///< linked list parameter starting address (pa)
	UINT32 parm_size;               ///< linked list parameter size
	KDRV_AI_MODE mode;              ///< select ai mode for first engine triggered (ex: mode=KDRV_AI_MODE_NEURAL)
	KDRV_AI_ENG eng;                ///< engine type
} KDRV_AI_LL_HEAD;

/*
    AI trigger mode
*/
typedef enum {
	AI_TRIG_MODE_APP     = 0,
	AI_TRIG_MODE_LL      = 1,
	AI_TRIG_MODE_FC      = 2,
	AI_TRIG_MODE_PREC    = 3,
	ENUM_DUMMY4WORD(KDRV_AI_TRIG_MODE)
} KDRV_AI_TRIG_MODE;

#endif  /* _NN_MODEL_H_ */


