#ifndef __KDRV_SIE_API_H_
#define __KDRV_SIE_API_H_

#include "kdrv_sie_int.h"

#define KDRV_SIE_TEST_CMD DISABLE //test cmd, for develop using

#if defined (__FREERTOS)
#define KDRV_SIE_TEST_CMD_WITH_REAL_SENSOR KDRV_SIE_TEST_CMD
#else
#define KDRV_SIE_TEST_CMD_WITH_REAL_SENSOR DISABLE
#endif

#if KDRV_SIE_TEST_CMD
BOOL kdrv_sie_cmd_test(unsigned char argc, char **pargv);
BOOL kdrv_sie_cmd_magic(unsigned char argc, char **pargv);
#endif

BOOL kdrv_sie_cmd_set_dbg_type(unsigned char argc, char **pargv);
BOOL kdrv_sie_cmd_set_dbg_level(unsigned char argc, char **pargv);
BOOL kdrv_sie_cmd_set_eng_dbglv(unsigned char argc, char **pargv);
BOOL kdrv_sie_cmd_info(unsigned char argc, char **pargv);

#if defined(__LINUX)
int kdrv_sie_cmd_execute(unsigned char argc, char **argv);
#endif

#endif
