#ifndef _POU_ENG_BASE_H_
#define _POU_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "pou_eng_handle.h"

typedef enum _POU_ENG_ID {
	POU_ID  = 0,
	POU_ID_MAX_NUM,
	ENUM_DUMMY4WORD(POU_ENG_ID)
} POU_ENG_ID;

#endif
