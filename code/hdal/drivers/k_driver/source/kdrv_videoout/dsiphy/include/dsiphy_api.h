#ifndef __DSIPHY_API_H_
#define __DSIPHY_API_H_
#include "dsiphy_drv.h"

int nvt_dsiphy_api_write_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_dsiphy_api_write_pattern(PDSIPHY_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);
int nvt_dsiphy_api_read_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned char ucargc, char **pucargv);

#endif
