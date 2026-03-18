#ifndef __gpenc_api_h_
#define __gpenc_api_h_
#include "gpenc_drv.h"

int nvt_gpenc_api_write_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_gpenc_api_write_pattern(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_gpenc_api_read_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);

#endif
