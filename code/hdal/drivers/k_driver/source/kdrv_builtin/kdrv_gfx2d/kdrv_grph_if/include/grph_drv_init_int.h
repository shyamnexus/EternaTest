#ifndef _GRAPH_DRV_INIT_INT_H_
#define _GRAPH_DRV_INIT_INT_H_

#include "kwrap/type.h"
#include "grph_drv_ctl_int.h"
#include "kdrv_grph_ctl_int.h"

struct graph_drv_eng_info {
	char name[10];
    uintptr_t pbase;   //eng physical address
    void *vbase;	  	  //eng virtual address
    unsigned long mem_len;
    int irq_no;
	struct resource *res;
	struct clk *mclk;
};

struct graph_drv_module_info {

	struct graph_drv_ctl drv_ctl;
	struct kdrv_graph_ctl kdrv_ctl;
};

int graph_drv_module_uninit(void);
int graph_drv_module_init(void);

int graph_drv_eng_init(void);
int graph_drv_eng_uninit(void);
int graph_drv_eng_set_resource(int idx, struct graph_drv_eng_info *info);
struct graph_drv_eng_info* graph_drv_eng_get_resource(int idx);

int graph_drv_eng_get_chip_num(void);
int graph_drv_eng_get_eng_num(void);
int graph_drv_eng_get_total_ch(void);

#endif  //_GRAPH_DRV_INIT_INT_H_