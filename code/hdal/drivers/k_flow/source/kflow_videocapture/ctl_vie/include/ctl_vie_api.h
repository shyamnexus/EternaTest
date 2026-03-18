#ifndef __ctl_vie_module_api_h_
#define __ctl_vie_module_api_h_
#include "ctl_vie_drv.h"
#include "kwrap/type.h"

#define CTL_VIE_TEST_CMD DISABLE //test cmd, for develop using

BOOL ctl_vie_cmd_set_dbg_type(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_set_dbg_level(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_save_mem(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_save_yuv(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_set_isp_dbg_type( unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_dump_signal(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_ccir_header_chk(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_manual(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_info(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_dbg_log_rate(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_buf(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_set_err_log_rate(unsigned char argc, char **pargv);
#if CTL_VIE_TEST_CMD//Test Cmd
BOOL ctl_vie_cmd_on( unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_off(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_open(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_close(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_start(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_stop(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_trig(unsigned char argc, char **pargv);
BOOL ctl_vie_cmd_test(unsigned char argc, char **pargv);
#endif

#if defined(__LINUX)
int ctl_vie_cmd_execute(unsigned char argc, char **argv);
#endif

#endif
