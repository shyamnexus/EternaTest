#ifndef _AI_H_
#define _AI_H_

#include "aiisp_pq.h"

//=============================================================================
// struct & enum definition
//=============================================================================
typedef enum _AI_MODE_SEL {
	AI_MODE_SEL_DISABLE,
	AI_MODE_SEL_0,
	AI_MODE_SEL_1,
	ENUM_DUMMY4WORD(AI_MODE_SEL)
} AI_MODE_SEL;

//=============================================================================
// extern functions
//=============================================================================
extern HD_RESULT ai_mem_config(void);
extern HD_RESULT ai_open(HD_PATH_ID proc_ctrl);
extern HD_RESULT ai_init(void);
extern HD_RESULT ai_start(void);
extern HD_RESULT ai_set_isp(void);
extern HD_RESULT ai_stop(void);
extern HD_RESULT ai_close(void);
extern HD_RESULT ai_exit(void);
extern HD_RESULT ai_get_model_name1(CHAR *ai_name1);
extern HD_RESULT ai_set_model_name1(CHAR *ai_name1);
extern HD_RESULT ai_get_model_name2(CHAR *ai_name2);
extern HD_RESULT ai_set_model_name2(CHAR *ai_name2);
extern HD_RESULT ai_model_sel(AI_MODE_SEL sel);
extern HD_RESULT ai_set_param(AIISP_PQ_FINAL_PARAM *param);
extern HD_RESULT ai_model_trig(void);
#endif

