#ifndef _TRKE_DRV_INIT_INT_H_
#define _TRKE_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "trke_drv_ctl_int.h"
#include "trke_drv_ll_int.h"
#include "kdrv_trke_ctl_int.h"
#include "trke_drv_ioctl_int.h"

struct trke_drv_eng_info {
	char name[10];
    uintptr_t pbase;   //eng physical address
    void *vbase;	  	  //eng virtual address
    unsigned long mem_len;
    int irq_no;
	struct resource *res;
	struct clk *mclk;
};

struct trke_drv_module_info {

	struct trke_drv_ctl drv_ctl;
	struct trke_drv_ll ll_ctl;
	struct kdrv_trke_ctl kdrv_ctl;
	struct trke_drv_ioctl_ctl ioctl_ctl;
};

int trke_drv_module_uninit(void);
int trke_drv_module_init(void);

int trke_drv_eng_init(void);
int trke_drv_eng_uninit(void);
int trke_drv_eng_set_resource(int idx, struct trke_drv_eng_info *info);
struct trke_drv_eng_info* trke_drv_eng_get_resource(int idx);

int trke_drv_eng_get_chip_num(void);
int trke_drv_eng_get_eng_num(void);
int trke_drv_eng_get_total_ch(void);

#endif  //_TRKE_DRV_INIT_INT_H_