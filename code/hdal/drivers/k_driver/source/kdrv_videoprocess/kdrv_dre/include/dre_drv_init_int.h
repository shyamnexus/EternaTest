#ifndef _DRE_DRV_INIT_INT_H_
#define _DRE_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "dre_drv_ctl_int.h"
#include "dre_drv_ll_int.h"
#include "kdrv_dre_ctl_int.h"

struct dre_drv_eng_info {
	char name[10];
    uintptr_t pbase;   //eng physical address
    void *vbase;	  	  //eng virtual address
    unsigned long mem_len;
    int irq_no;
	struct resource *res;
	struct clk *mclk;
};

struct dre_drv_module_info {

	struct dre_drv_ctl drv_ctl;
	struct dre_drv_ll ll_ctl;
	struct kdrv_dre_ctl kdrv_ctl;
};

int dre_drv_module_uninit(void);
int dre_drv_module_init(void);

int dre_drv_eng_init(void);
int dre_drv_eng_uninit(void);
int dre_drv_eng_set_resource(int idx, struct dre_drv_eng_info *info);
struct dre_drv_eng_info* dre_drv_eng_get_resource(int idx);

int dre_drv_eng_get_chip_num(void);
int dre_drv_eng_get_eng_num(void);
int dre_drv_eng_get_total_ch(void);

#endif  //_DRE_DRV_INIT_INT_H_