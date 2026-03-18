#ifndef __module_api_h_
#define __module_api_h_
#include "ctl_dre_drv.h"


extern void nvt_ctl_dre_install_cmd(void);
extern void nvt_ctl_dre_uninstall_cmd(void);
#if defined(__LINUX)
int ctl_dre_cmd_execute(unsigned char argc, char **argv);
#endif

#endif
