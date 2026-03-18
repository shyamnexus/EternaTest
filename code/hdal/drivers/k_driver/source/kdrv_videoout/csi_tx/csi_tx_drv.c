#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include "csi_tx_drv.h"
#include "csi_tx_ioctl.h"
#include "csi_tx_dbg.h"
#include "csi_tx.h"


/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_csi_tx_drv_wait_cmd_complete(PMODULE_INFO pmodule_info);
int nvt_csi_tx_drv_ioctl(unsigned char uc_if, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);

irqreturn_t nvt_csi_tx_drv_isr(int irq, void *devid);
irqreturn_t nvt_csi_tx_drv_do_tasklet(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
//int i_event_flag = 0;

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_csi_tx_drv_open(PMODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel initial operation here when the device file opened*/

	return csi_tx_open();
}


int nvt_csi_tx_drv_release(PMODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel release operation here when device file closed */
	
	return csi_tx_close();
}

int nvt_csi_tx_drv_init(MODULE_INFO *pmodule_info)
{
	int i_ret = 0;
	int ucloop = 0 ;

	init_waitqueue_head(&pmodule_info->csi_tx_wait_queue);
	//spin_lock_init(&pmodule_info->csi_tx_spinlock);
	sema_init(&pmodule_info->csi_tx_sem, 1);
	init_completion(&pmodule_info->csi_tx_completion);
	//tasklet_init(&pmodule_info->csi_tx_tasklet, nvt_csi_tx_drv_do_tasklet, (unsigned long)pmodule_info);

	/* initial clock here */
	for(ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			clk_prepare(pmodule_info->pclk[ucloop]);

			/* enable clock first. modify this later */
			clk_enable(pmodule_info->pclk[ucloop]);
		}

	}

	/* register IRQ here*/
	if (request_threaded_irq(pmodule_info->iinterrupt_id[0], nvt_csi_tx_drv_isr, nvt_csi_tx_drv_do_tasklet,
			IRQF_TRIGGER_HIGH|IRQF_ONESHOT, "CSI_TX_INT", pmodule_info))
	{
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
		i_ret = -ENODEV;
		goto FAIL_FREE_IRQ;
	}

	/* Add HW Module initialization here when driver loaded */
	csi_tx_create_resource();

	/* CSI_TX Set base address */
	csi_tx_set_base_addr((CSI_TX_DATA_TYPE)pmodule_info->io_addr[0]);

	return i_ret;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add error handler here */

	return i_ret;
}

int nvt_csi_tx_drv_remove(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	//Free IRQ
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add HW Moduel release operation here*/
	csi_tx_release_resource();

	return 0;
}

int nvt_csi_tx_drv_suspend(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_csi_tx_drv_resume(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_csi_tx_drv_ioctl(unsigned char uc_if, MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int i_loop;
	int i_ret = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);

	switch (ui_cmd) {
	case CSI_TX_IOC_START:
		/*call someone to start operation*/
		csi_tx_set_tx_en(TRUE,FALSE);
		break;

	case CSI_TX_IOC_STOP:
		/*call someone to stop operation*/
		csi_tx_set_tx_en(FALSE,TRUE);
		break;

	case CSI_TX_IOC_READ_REG:
		i_ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!i_ret) {
			reg_info.value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info.addr);
			i_ret = copy_to_user((void __user *)ul_arg, &reg_info, sizeof(REG_INFO));
		}
		break;

	case CSI_TX_IOC_WRITE_REG:
		i_ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!i_ret)
			WRITE_REG(reg_info.value, pmodule_info->io_addr[uc_if] + reg_info.addr);
		break;

#if 0
	case CSI_TX_IOC_READ_REG_LIST:
		i_ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!i_ret) {
			for (i_loop = 0 ; i_loop < reg_info_list.count; i_loop++)
				reg_info_list.reg_list[i_loop].value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[i_loop].addr);

			i_ret = copy_to_user((void __user *)ul_arg, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case CSI_TX_IOC_WRITE_REG_LIST:
		i_ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!i_ret)
			for (i_loop = 0 ; i_loop < reg_info_list.count ; i_loop++)
				WRITE_REG(reg_info_list.reg_list[i_loop].value, pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[i_loop].addr);
		break;
#endif
		/* Add other operations here */
	}

	return i_ret;
}

irqreturn_t nvt_csi_tx_drv_isr(int irq, void *devid)
{
	//PMODULE_INFO pmodule_info = (PMODULE_INFO)devid;

	/* simple triggle and response mechanism*/
	//complete(&pmodule_info->csi_tx_completion);


	/*  Tasklet for bottom half mechanism */
	//tasklet_schedule(&pmodule_info->csi_tx_tasklet);
	csi_tx_isr();

	return IRQ_WAKE_THREAD;
}

int nvt_csi_tx_drv_wait_cmd_complete(PMODULE_INFO pmodule_info)
{
	wait_for_completion(&pmodule_info->csi_tx_completion);
	return 0;
}

int nvt_csi_tx_drv_write_reg(PMODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}

int nvt_csi_tx_drv_read_reg(PMODULE_INFO pmodule_info, unsigned long addr)
{
	return READ_REG(pmodule_info->io_addr[0] + addr);
}
#if 0
void nvt_csi_tx_drv_do_tasklet(unsigned long data)
{
	PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

	nvt_dbg(IND, "\n");

	/* do something you want*/
	complete(&pmodule_info->csi_tx_completion);
	
	 // ID
	csi_tx_tasklet(0);
	return IRQ_HANDLED;

}
#endif

irqreturn_t nvt_csi_tx_drv_do_tasklet(int irq, void *devid)
{
	nvt_dbg(IND, "\n");
	
    // ID
	csi_tx_tasklet(0);
	return IRQ_HANDLED;
}

