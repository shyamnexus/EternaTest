#ifndef __AI_API_h_
#define __AI_API_h_

#include "ai_drv.h"

int nvt_ai_api_write_reg(PAI_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_ai_api_write_pattern(PAI_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_kdrv_ai_api_test(PAI_INFO pmodule_info, unsigned char argc, char** pargv);
int nvt_ai_api_read_reg(PAI_INFO pmodule_info, unsigned char ucargc, char **pucargv);
#if defined(__FREERTOS)
int nvt_kdrv_ai_module_test(PAI_INFO pmodule_info, unsigned char argc, char pargv[][10]);
#else
int nvt_kdrv_ai_module_test(PAI_INFO pmodule_info, unsigned char argc, char **pargv);
//int nvt_kdrv_ai_module_test(PAI_INFO pmodule_info, unsigned char argc, char pargv[][10]);
#endif

int nvt_kdrv_ai_reset(PAI_INFO pmodule_info, unsigned char argc, char **pargv);

ER kdrv_ai_chk_efuse_kdrv_ai_api(VOID);
ER kdrv_ai_chk_efuse_ai_lib(VOID);
ER kdrv_ai_chk_efuse_ai_eng(VOID);
ER kdrv_ai_chk_efuse_ai_api(VOID);


#endif
