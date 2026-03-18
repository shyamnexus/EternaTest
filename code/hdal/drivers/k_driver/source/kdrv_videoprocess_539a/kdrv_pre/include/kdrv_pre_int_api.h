#ifndef __KDRV_PRE_INT_API_H_
#define __KDRV_PRE_INT_API_H_

extern void nvt_kdrv_pre_install_cmd(void);
extern void nvt_kdrv_pre_uninstall_cmd(void);
#if defined(__LINUX)
int kdrv_pre_cmd_execute(unsigned char argc, char **argv);
#endif

#endif

