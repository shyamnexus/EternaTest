#ifndef __sif_api_h_
#define __sif_api_h_
#include "sif_drv.h"

#define NVT_SIF_TEST_CMD 1

int nvt_sif_api_write_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char** pucargv);
int nvt_sif_api_send_test_data(PMODULE_INFO pmodule_info, unsigned char ucargc, char** pucargv);
int nvt_sif_api_write_pattern(PMODULE_INFO pmodule_info, unsigned char ucargc, char** pucargv);
int nvt_sif_api_read_reg(PMODULE_INFO pmodule_info, unsigned char ucargc, char** pucargv);

#endif
