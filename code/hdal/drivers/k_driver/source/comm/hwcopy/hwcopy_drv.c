#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include "hwcopy_drv.h"
#include "hwcopy_ioctl.h"
#include "hwcopy_dbg.h"
#include "hwcopy_platform.h"

/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_hwcopy_drv_wait_cmd_complete(PMODULE_INFO pmodule_info);
int nvt_hwcopy_drv_ioctl(unsigned char if_id, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_hwcopy_drv_do_tasklet(unsigned long data);
irqreturn_t nvt_hwcopy_drv_isr(int irq, void *devid);
irqreturn_t nvt_hwcopy_ep_drv_isr(int irq, void *devid);
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
int nvt_hwcopy_drv_open(PMODULE_INFO pmodule_info, unsigned char if_id)
{
	nvt_dbg(IND, "%d\n", if_id);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_hwcopy_drv_release(PMODULE_INFO pmodule_info, unsigned char if_id)
{
	nvt_dbg(IND, "%d\n", if_id);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_hwcopy_drv_init(MODULE_INFO *pmodule_info)
{
	int err = 0;
	struct clk *pclk;

	init_waitqueue_head(&pmodule_info->hwcopy_wait_queue);
	//spin_lock_init(&pmodule_info->hwcopy_spinlock);
	sema_init(&pmodule_info->hwcopy_sem, 1);
	init_completion(&pmodule_info->hwcopy_completion);
	tasklet_init(&pmodule_info->hwcopy_tasklet[0], nvt_hwcopy_drv_do_tasklet, (unsigned long)pmodule_info);

	/* allocate graphic resource here */
	hwcopy_platform_create_resource(pmodule_info);

	/* initial clock here */
	pclk = clk_get(NULL, "fix480m");
	if (IS_ERR(pclk)) {
		printk("%s: get source fix480m fail\r\n", __func__);
	}
	clk_set_parent(pmodule_info->pclk[0], pclk);
	clk_prepare(pmodule_info->pclk[0]);

	clk_put(pclk);

    /* register IRQ here*/
    if(pmodule_info->chip_idx == 0){
        if (request_irq(pmodule_info->iinterrupt_id[0], nvt_hwcopy_drv_isr, IRQF_TRIGGER_HIGH, "HWCOPY_INT", pmodule_info)) {
            nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
            err = -ENODEV;
            goto FAIL_FREE_IRQ;
        }
    }
    else{
        if (request_irq(pmodule_info->iinterrupt_id[0], nvt_hwcopy_ep_drv_isr, IRQF_TRIGGER_HIGH, "HWCOPY_EP_INT", pmodule_info)) {
            nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
            err = -ENODEV;
            goto FAIL_FREE_IRQ;
        }
    }



	/* Add HW Module initialization here when driver loaded */

	printk("%s: done\n", __func__);

	return err;

FAIL_FREE_IRQ:
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add error handler here */

	return err;
}

int nvt_hwcopy_drv_remove(MODULE_INFO *pmodule_info)
{
	//Free IRQ
	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add HW Moduel release operation here*/

	/* release OS resources */
	hwcopy_platform_release_resource();

	return 0;
}

int nvt_hwcopy_drv_suspend(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_hwcopy_drv_resume(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_hwcopy_drv_ioctl(unsigned char if_id, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long argc)
{
	//REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int loop_count;
	int err = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", if_id, cmd);



	switch (cmd) {
	case HWCOPY_IOC_START:
		/*call someone to start operation*/
		break;

	case HWCOPY_IOC_STOP:
		/*call someone to stop operation*/
		break;
#if 0
	case HWCOPY_IOC_READ_REG:
		err = copy_from_user(&reg_info, (void __user *)argc, sizeof(REG_INFO));
		if (!err) {
			reg_info.reg_value = READ_REG(pmodule_info->io_addr[if_id] + reg_info.reg_addr);
			err = copy_to_user((void __user *)argc, &reg_info, sizeof(REG_INFO));
		}
		break;

	case HWCOPY_IOC_WRITE_REG:
		err = copy_from_user(&reg_info, (void __user *)argc, sizeof(REG_INFO));
		if (!err)
			WRITE_REG(reg_info.reg_value, pmodule_info->io_addr[if_id] + reg_info.reg_addr);
		break;

	case HWCOPY_IOC_READ_REG_LIST:
		err = copy_from_user(&reg_info_list, (void __user *)argc, sizeof(REG_INFO_LIST));
		if (!err) {
			for (loop_count = 0 ; loop_count < reg_info_list.reg_cnt; loop_count++)
				reg_info_list.reg_list[loop_count].reg_value = READ_REG(pmodule_info->io_addr[if_id] + reg_info_list.reg_list[loop_count].reg_addr);

			err = copy_to_user((void __user *)argc, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case HWCOPY_IOC_WRITE_REG_LIST:
		err = copy_from_user(&reg_info_list, (void __user *)argc, sizeof(REG_INFO_LIST));
		if (!err)
			for (loop_count = 0 ; loop_count < reg_info_list.reg_cnt ; loop_count++)
				WRITE_REG(reg_info_list.reg_list[loop_count].reg_value, pmodule_info->io_addr[if_id] + reg_info_list.reg_list[loop_count].reg_addr);
		break;
#endif
	/* Add other operations here */
	}

	return err;
}

irqreturn_t nvt_hwcopy_drv_isr(int irq, void *devid)
{
#if 0
	PMODULE_INFO pmodule_info = (PMODULE_INFO)devid;

	/* simple triggle and response mechanism*/
	complete(&pmodule_info->xxx_completion);


	/*  Tasklet for bottom half mechanism */
	tasklet_schedule(&pmodule_info->xxx_tasklet);
#endif
	copy_isr(HWCOPY_ID_0);
	return IRQ_HANDLED;
}

irqreturn_t nvt_hwcopy_ep_drv_isr(int irq, void *devid)
{
#if 0
	PMODULE_INFO pmodule_info = (PMODULE_INFO)devid;

	/* simple triggle and response mechanism*/
	complete(&pmodule_info->xxx_completion);


	/*  Tasklet for bottom half mechanism */
	tasklet_schedule(&pmodule_info->xxx_tasklet);
#endif
	copy_isr(HWCOPY_ID_1);
	return IRQ_HANDLED;
}

int nvt_hwcopy_drv_wait_cmd_complete(PMODULE_INFO pmodule_info)
{
	wait_for_completion(&pmodule_info->hwcopy_completion);
	return 0;
}

int nvt_hwcopy_drv_write_reg(PMODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}

int nvt_hwcopy_drv_read_reg(PMODULE_INFO pmodule_info, unsigned long addr)
{
	return READ_REG(pmodule_info->io_addr[0] + addr);
}

void nvt_hwcopy_drv_do_tasklet(unsigned long data)
{
	PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

	nvt_dbg(IND, "task task = %d \n",pmodule_info->chip_idx);

	hwcopy_platform_ist(pmodule_info->chip_idx, data);
	/* do something you want*/
//	complete(&pmodule_info->xxx_completion);
}

