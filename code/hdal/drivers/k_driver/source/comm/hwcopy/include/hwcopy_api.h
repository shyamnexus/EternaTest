#ifndef __hwcopy_api_h_
#define __hwcopy_api_h_
#include "hwcopy_drv.h"

int nvt_hwcopy_api_auto_test(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_hwcopy_api_write_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_hwcopy_api_write_pattern(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_hwcopy_api_read_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);

#endif
