#if defined __KERNEL__
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io_address.h"
#include "interrupt.h"
#endif
#include "cc.h"
#include <kwrap/flag.h>
#include "kdrv_rpc/kdrv_rpc.h"
#include "kdrv_rpc_drv.h"
#include "kdrv_rpc_debug.h"
#include "kdrv_rpc_platform.h"

ID                 g_rpc_flgid;
#if defined(__FREERTOS)
static KDRV_RPC_MODULE_INFO *g_pmodule_info;
#endif

irqreturn_t kdrv_rpc_drv_isr(int irq, void *devid)
{

	nvt_cc_isr();
#if (RPC_INT_MODE == RPC_INT_MODE_TASKLET)
	{
		#if defined __KERNEL__
		KDRV_RPC_MODULE_INFO *pmodule_info = devid;
		#else
		KDRV_RPC_MODULE_INFO *pmodule_info = g_pmodule_info;
		#endif
		vos_tasklet_schedule(&pmodule_info->tasklet[0]);
	}
#else
	nvt_cc_ist();
#endif
	return IRQ_HANDLED;
}


#if (RPC_INT_MODE == RPC_INT_MODE_TASKLET)
static void kdrv_rpc_drv_do_tasklet(unsigned long data)
{
	DBG_IND("\r\n");
	nvt_cc_ist();
}
#endif
// rtos
/*
#else
irqreturn_t kdrv_rpc_drv_isr(int irq, void *devid)
{
	cc_isr();
	cc_ist();
	return IRQ_HANDLED;

}
#endif
*/
int kdrv_rpc_create_resource(KDRV_RPC_MODULE_INFO *pmodule_info)
{
#if (RPC_INT_MODE == RPC_INT_MODE_TASKLET)
	vos_tasklet_init(&pmodule_info->tasklet[0], kdrv_rpc_drv_do_tasklet, (unsigned long)pmodule_info);
#endif
#if defined __KERNEL__
	if (request_irq(pmodule_info->iinterrupt_id[0], kdrv_rpc_drv_isr, IRQF_SHARED | IRQF_TRIGGER_HIGH, "CC_IRQ", pmodule_info)) {
		DBG_ERR("failed to register an IRQ Int:%d\r\n", pmodule_info->iinterrupt_id[0]);
		return -1;
	}
#else
	g_pmodule_info = pmodule_info;
	pmodule_info->io_addr[0] = (void *)IOADDR_CC_REG_BASE;
	pmodule_info->iinterrupt_id[0] = INT_ID_CC;
	request_irq(INT_ID_CC, kdrv_rpc_drv_isr, IRQF_TRIGGER_HIGH, "cc", 0);
#endif
	OS_CONFIG_FLAG(g_rpc_flgid);
	clr_flg(g_rpc_flgid, 0xFFFFFFFF);
	return 0;
}


void kdrv_rpc_release_resource(KDRV_RPC_MODULE_INFO *pmodule_info)
{
#if defined __KERNEL__
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);
#endif
	rel_flg(g_rpc_flgid);
}


#ifdef __KERNEL__
EXPORT_SYMBOL(kdrv_rpc_open);
EXPORT_SYMBOL(kdrv_rpc_close);
EXPORT_SYMBOL(kdrv_rpc_trigger);
EXPORT_SYMBOL(kdrv_rpc_set);
EXPORT_SYMBOL(kdrv_rpc_get);
#endif


