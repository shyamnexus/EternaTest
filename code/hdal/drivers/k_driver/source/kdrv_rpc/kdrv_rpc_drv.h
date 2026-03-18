#ifndef __KDRV_RPC_DRV_H__
#define __KDRV_RPC_DRV_H__

#include <kwrap/tasklet.h>

#define MODULE_IRQ_NUM          1
#define MODULE_REG_NUM          1

#define RPC_INT_MODE_TASKLET	0
#define RPC_INT_MODE_ISR		1
#define RPC_INT_MODE			RPC_INT_MODE_ISR


typedef struct _KDRV_RPC_MODULE_INFO {
#if (RPC_INT_MODE == RPC_INT_MODE_TASKLET)
	//struct tasklet_struct tasklet[MODULE_IRQ_NUM];
	struct vos_tasklet_struct  tasklet[MODULE_IRQ_NUM];
#endif
	void                  *io_addr[MODULE_REG_NUM];
	int                   iinterrupt_id[MODULE_IRQ_NUM];
} KDRV_RPC_MODULE_INFO, *PKDRV_RPC_MODULE_INFO;



extern int  kdrv_rpc_drv_init(KDRV_RPC_MODULE_INFO *pmodule_info);
extern void kdrv_rpc_drv_exit(KDRV_RPC_MODULE_INFO *pmodule_info);
extern void kdrv_rpc_drv_suspend(void);
extern void kdrv_rpc_drv_resume(void);
#endif

