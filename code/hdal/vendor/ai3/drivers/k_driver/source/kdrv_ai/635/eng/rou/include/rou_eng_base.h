#ifndef _ROU_ENG_BASE_H_
#define _ROU_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "rou_eng_handle.h"


typedef enum _ROU_ENG_ID {
	ROU_ID  = 0,
    ROU2_ID = 1,
	ROU_ID_MAX_NUM,
	ENUM_DUMMY4WORD(ROU_ENG_ID)
} ROU_ENG_ID;


#endif

