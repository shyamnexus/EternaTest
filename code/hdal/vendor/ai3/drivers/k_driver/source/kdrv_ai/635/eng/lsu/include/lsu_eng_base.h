#ifndef _LSU_ENG_BASE_H_
#define _LSU_ENG_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "kwrap/nvt_type.h"
#include "lsu_eng_handle.h"


typedef enum _LSU_ENG_ID {
	LSU_ID  = 0,
	LSU2_ID  = 1,
	LSU_ID_MAX_NUM,
	ENUM_DUMMY4WORD(LSU_ENG_ID)
} LSU_ENG_ID;


#endif
