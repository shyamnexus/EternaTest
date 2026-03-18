#ifndef _MD_DRV_INIT_INT_H_
#define _MD_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "md_drv_ctl_int.h"
#include "md_drv_ll_int.h"
#include "kdrv_md_ctl_int.h"
#include "md_drv_ioctl_int.h"

struct md_drv_eng_info {
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

struct md_drv_module_info {

	struct md_drv_ctl drv_ctl;
	struct md_drv_ll ll_ctl;
	struct kdrv_md_ctl kdrv_ctl;
	struct md_drv_ioctl_ctl ioctl_ctl;
};

int md_drv_module_uninit(void);
int md_drv_module_init(void);

int md_drv_eng_init(void);
int md_drv_eng_uninit(void);
int md_drv_eng_set_resource(int idx, struct md_drv_eng_info *info);
struct md_drv_eng_info* md_drv_eng_get_resource(int idx);

int md_drv_eng_get_chip_num(void);
int md_drv_eng_get_eng_num(void);
int md_drv_eng_get_total_ch(void);

#endif  //_MD_DRV_INIT_INT_H_