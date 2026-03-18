#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include "graphic_drv.h"
#include "graphic_ioctl.h"
#include "graphic_dbg.h"
#include "grph_platform.h"

/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
extern void grph_platform_init(MODULE_INFO *pmodule_info);

int nvt_graphic_drv_wait_cmd_complete(PMODULE_INFO pmodule_info);
int nvt_graphic_drv_ioctl(unsigned char if_id, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_graphic_drv_do_tasklet(unsigned long data);
void nvt_graphic_drv_do_tasklet2(unsigned long data);
void nvt_graphic_drv_do_tasklet3(unsigned long data);
irqreturn_t nvt_graphic_drv_isr(int irq, void *devid);
irqreturn_t nvt_graphic_drv_isr2(int irq, void *devid);
irqreturn_t nvt_graphic_drv_isr3(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
//int iEventFlag = 0;

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_graphic_drv_open(PMODULE_INFO pmodule_info, unsigned char if_id)
{
	nvt_dbg(IND, "%d\n", if_id);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_graphic_drv_release(PMODULE_INFO pmodule_info, unsigned char if_id)
{
	nvt_dbg(IND, "%d\n", if_id);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_graphic_drv_init(MODULE_INFO *pmodule_info)
{
	int err = 0;
	struct clk *pclk;

	init_waitqueue_head(&pmodule_info->xxx_wait_queue);
	// coverity[side_effect_free]: spin_lock_init is kernel API, do not change
	spin_lock_init(&pmodule_info->xxx_spinlock);
	sema_init(&pmodule_info->xxx_sem, 1);
	init_completion(&pmodule_info->xxx_completion);
	tasklet_init(&pmodule_info->xxx_tasklet[0], nvt_graphic_drv_do_tasklet, (unsigned long)pmodule_info);
	tasklet_init(&pmodule_info->xxx_tasklet[1], nvt_graphic_drv_do_tasklet2, (unsigned long)pmodule_info);
	tasklet_init(&pmodule_info->xxx_tasklet[2], nvt_graphic_drv_do_tasklet3, (unsigned long)pmodule_info);

	/* allocate graphic resource here */
	grph_platform_init(pmodule_info);

	/* initial clock here */
	pclk = clk_get(NULL, "fix480m");
	if (IS_ERR(pclk)) {
		printk("%s: get source fix480m fail\r\n", __func__);
	}
	clk_set_parent(pmodule_info->pclk[0], pclk);
	clk_set_parent(pmodule_info->pclk[1], pclk);
	clk_set_parent(pmodule_info->pclk[2], pclk);
	clk_prepare(pmodule_info->pclk[0]);
	clk_prepare(pmodule_info->pclk[1]);
	clk_prepare(pmodule_info->pclk[2]);

	clk_put(pclk);


	/* register IRQ here*/
	if (request_irq(pmodule_info->iinterrupt_id[0], nvt_graphic_drv_isr, IRQF_TRIGGER_HIGH, "GRPH_INT", pmodule_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
		err = -ENODEV;
		goto FAIL_FREE_IRQ;
	}
	if (request_irq(pmodule_info->iinterrupt_id[1], nvt_graphic_drv_isr2, IRQF_TRIGGER_HIGH, "GRPH2_INT", pmodule_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[1]);
		err = -ENODEV;
		goto FAIL_FREE_IRQ2;
	}
	if (request_irq(pmodule_info->iinterrupt_id[2], nvt_graphic_drv_isr3, IRQF_TRIGGER_HIGH, "GRPH3_INT", pmodule_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[2]);
		err = -ENODEV;
		goto FAIL_FREE_IRQ3;
	}


	/* Add HW Module initialization here when driver loaded */

//	printk("%s: done\n", __func__);

	return err;

FAIL_FREE_IRQ3:
	free_irq(pmodule_info->iinterrupt_id[2], pmodule_info);

FAIL_FREE_IRQ2:
	free_irq(pmodule_info->iinterrupt_id[1], pmodule_info);

FAIL_FREE_IRQ:
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add error handler here */

	return err;
}

int nvt_graphic_drv_remove(MODULE_INFO *pmodule_info)
{
	//Free IRQ
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);
	free_irq(pmodule_info->iinterrupt_id[1], pmodule_info);
	free_irq(pmodule_info->iinterrupt_id[2], pmodule_info);

	/* Add HW Moduel release operation here*/

	/* release OS resources */
	grph_platform_uninit();

	return 0;
}

int nvt_graphic_drv_suspend(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_graphic_drv_resume(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_graphic_drv_ioctl(unsigned char if_id, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long argc)
{
	int err = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", if_id, cmd);



	switch (cmd) {
	case XXX_IOC_START:
		/*call someone to start operation*/
		break;

	case XXX_IOC_STOP:
		/*call someone to stop operation*/
		break;

	/* Add other operations here */
	}

	return err;
}

irqreturn_t nvt_graphic_drv_isr(int irq, void *devid)
{
	graph_isr();
	return IRQ_HANDLED;
}

irqreturn_t nvt_graphic_drv_isr2(int irq, void *devid)
{
	graph2_isr();
	return IRQ_HANDLED;
}

irqreturn_t nvt_graphic_drv_isr3(int irq, void *devid)
{
	graph3_isr();
	return IRQ_HANDLED;
}

int nvt_graphic_drv_wait_cmd_complete(PMODULE_INFO pmodule_info)
{
	wait_for_completion(&pmodule_info->xxx_completion);
	return 0;
}

#if 0
int nvt_graphic_drv_write_reg(PMODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}
#endif

#if 0
int nvt_graphic_drv_read_reg(PMODULE_INFO pmodule_info, unsigned long addr)
{
	return READ_REG(pmodule_info->io_addr[0] + addr);
}
#endif

void nvt_graphic_drv_do_tasklet(unsigned long data)
{
//	PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

//	nvt_dbg(IND, "\n");

	grph_platform_ist(GRPH_ID_1, data);
	/* do something you want*/
//	complete(&pmodule_info->xxx_completion);
}

void nvt_graphic_drv_do_tasklet2(unsigned long data)
{
//        PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

//        nvt_dbg(IND, "\n");

	grph_platform_ist(GRPH_ID_2, data);
        /* do something you want*/
//        complete(&pmodule_info->xxx_completion);
}

void nvt_graphic_drv_do_tasklet3(unsigned long data)
{
//        PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

//        nvt_dbg(IND, "\n");

	grph_platform_ist(GRPH_ID_3, data);
        /* do something you want*/
//        complete(&pmodule_info->xxx_completion);
}

