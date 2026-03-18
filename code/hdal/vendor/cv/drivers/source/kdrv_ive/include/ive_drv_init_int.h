#ifndef _IVE_DRV_INIT_INT_H_
#define _IVE_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "ive_drv_ctl_int.h"
#include "ive_drv_ll_int.h"
#include "kdrv_ive_ctl_int.h"
#include "ive_drv_ioctl_int.h"

struct ive_drv_eng_info {
	char name[10];
    uintptr_t pbase;   //eng physical address
    void *vbase;	  	  //eng virtual address
    unsigned long mem_len;
    int irq_no;
	struct resource *res;
	struct clk *mclk;
};

struct ive_drv_module_info {

	struct ive_drv_ctl drv_ctl;
	struct ive_drv_ll ll_ctl;
	struct kdrv_ive_ctl kdrv_ctl;
	struct ive_drv_ioctl_ctl ioctl_ctl;
};

int ive_drv_module_uninit(void);
int ive_drv_module_init(void);

int ive_drv_eng_init(void);
int ive_drv_eng_uninit(void);
int ive_drv_eng_set_resource(int idx, struct ive_drv_eng_info *info);
struct ive_drv_eng_info* ive_drv_eng_get_resource(int idx);

int ive_drv_eng_get_chip_num(void);
int ive_drv_eng_get_eng_num(void);
int ive_drv_eng_get_total_ch(void);

#endif  //_IVE_DRV_INIT_INT_H_