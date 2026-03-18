#ifndef SYS_FDT_H
#define SYS_FDT_H
#include <shm_info.h>

extern void *fdt_get_app(void);
extern void *fdt_get_sensor(void);
extern int fdt_set_app(void *p_fdt_app);
extern SHMINFO *fdt_get_shminfo(void);
extern int is_fastboot(void);
#endif