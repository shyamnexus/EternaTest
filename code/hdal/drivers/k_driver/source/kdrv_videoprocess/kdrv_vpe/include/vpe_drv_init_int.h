#ifndef _VPE_DRV_INIT_INT_H_
#define _VPE_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "vpe_drv_ctl_int.h"
#include "vpe_drv_ll_int.h"
#include "kdrv_vpe_ctl_int.h"

struct vpe_drv_eng_info {
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

struct vpe_drv_module_info {

	struct vpe_drv_ctl drv_ctl;
	struct vpe_drv_ll ll_ctl;
	struct kdrv_vpe_ctl kdrv_ctl;
};

int vpe_drv_module_uninit(void);
int vpe_drv_module_init(void);

int vpe_drv_eng_init(void);
int vpe_drv_eng_uninit(void);
int vpe_drv_eng_set_resource(int idx, struct vpe_drv_eng_info *info);
struct vpe_drv_eng_info* vpe_drv_eng_get_resource(int idx);

int vpe_drv_eng_get_chip_num(void);
int vpe_drv_eng_get_eng_num(void);
int vpe_drv_eng_get_total_ch(void);

#endif  //_VPE_DRV_INIT_INT_H_