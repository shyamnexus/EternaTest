#ifndef _ISE_DRV_INIT_INT_H_
#define _ISE_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "ise_drv_ctl_int.h"
#include "ise_drv_ll_int.h"
#include "kdrv_ise_ctl_int.h"

struct ise_drv_eng_info {
	char name[10];
    uintptr_t pbase;   //eng physical address
    void *vbase;	  	  //eng virtual address
    unsigned long mem_len;
    int irq_no;
	struct resource *res;
	struct clk *mclk;
	unsigned long clk_rate; //hz

	//only for suspend/resume used
	int clk_phase;
	int clk_en_cnt;
};

struct ise_drv_module_info {

	struct ise_drv_ctl drv_ctl;
	struct ise_drv_ll ll_ctl;
	struct kdrv_ise_ctl kdrv_ctl;
};

int ise_drv_module_uninit(void);
int ise_drv_module_init(void);

int ise_drv_eng_init(void);
int ise_drv_eng_uninit(void);
int ise_drv_eng_set_resource(int idx, struct ise_drv_eng_info *info);
struct ise_drv_eng_info* ise_drv_eng_get_resource(int idx);

int ise_drv_eng_get_chip_num(void);
int ise_drv_eng_get_eng_num(void);
int ise_drv_eng_get_total_ch(void);

#endif  //_ISE_DRV_INIT_INT_H_