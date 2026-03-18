#ifndef _CONV_ENG_BASE_H_
#define _CONV_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "conv_eng_handle.h"


typedef enum _CONV_ENG_ID {
	CONV_ID  = 0,
	CONV2_ID  = 1,
	CONV3_ID  = 2,
	CONV4_ID  = 3,
	CONV_ID_MAX_NUM,
	ENUM_DUMMY4WORD(CONV_ENG_ID)
} CONV_ENG_ID;

#endif

