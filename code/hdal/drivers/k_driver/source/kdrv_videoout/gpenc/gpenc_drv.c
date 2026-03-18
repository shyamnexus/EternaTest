#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include "gpenc_drv.h"
#include "gpenc_ioctl.h"
#include "gpenc_dbg.h"
#include "gpenc.h"
#include "kdrv_videoout/kdrv_vdoout_gpe.h"


/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_gpenc_drv_wait_cmd_complete(PMODULE_INFO pmodule_info);
int nvt_gpenc_drv_ioctl(unsigned char uc_if, MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_gpenc_drv_do_tasklet(unsigned long data);
irqreturn_t nvt_gpenc_drv_isr(int irq, void *devid);
irqreturn_t nvt_gpenc2_drv_isr(int irq, void *devid);

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
int nvt_gpenc_drv_open(PMODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_gpenc_drv_release(PMODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}
//#define test
int nvt_gpenc_drv_init(MODULE_INFO *pmodule_info)
{
	int i_ret = 0;
	unsigned char ucloop;
#ifdef test
	KDRV_VDDO_GPE_PARAM p_kdrv_gpe_ctrl ={0};
	UINT32 engine = KDRV_VDOOUT_GPE_ENGINE1;
	UINT32 handler = KDRV_DEV_ID(0, engine, 0);
#endif

	init_waitqueue_head(&pmodule_info->gpenc_wait_queue);
	//spin_lock_init(&pmodule_info->gpenc_spinlock);
	sema_init(&pmodule_info->gpenc_sem, 1);
	init_completion(&pmodule_info->gpenc_completion);
	tasklet_init(&pmodule_info->gpenc_tasklet, nvt_gpenc_drv_do_tasklet, (unsigned long)pmodule_info);

	/* initial clock here */
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {

			clk_prepare(pmodule_info->pclk[ucloop]);

			/* enable clock first. modify this later */
			clk_enable(pmodule_info->pclk[ucloop]);
		}
	}

	/* register IRQ here*/
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (ucloop ==  0) {
			if (request_irq(pmodule_info->iinterrupt_id[ucloop], nvt_gpenc_drv_isr, IRQF_TRIGGER_HIGH, "GPENC_INT", pmodule_info)) {
				nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[ucloop]);
				i_ret = -ENODEV;
				goto FAIL_FREE_IRQ;
			}
		} else {
			if (request_irq(pmodule_info->iinterrupt_id[ucloop], nvt_gpenc2_drv_isr, IRQF_TRIGGER_HIGH, "GPENC2_INT", pmodule_info)) {
				nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[ucloop]);
				i_ret = -ENODEV;
				goto FAIL_FREE_IRQ;
			}
		}
	}


	/* Add HW Module initialization here when driver loaded */
	gpenc_create_resource();
	gpenc_set_base_addr((GPENCDATA_TYPE)pmodule_info->io_addr[0]);

	gpenc2_create_resource();
	gpenc2_set_base_addr((GPENCDATA_TYPE)pmodule_info->io_addr[1]);

#ifdef test
	kdrv_vddo_gpe_open(0, engine);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_SRCIDX.idx = GPENC_SRC_IDX_G3;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_IDX_WIDTH, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_SRCWIDTH.width = GPENC_SRC_WIDTH_2B;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_SRC_WIDTH, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_READADDR.addr = 0x10000000;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_READ_ADDR, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_WRITEADDR.addr = 0x20000000;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_WRITE_ADDR, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_IMGWIDTH.img_w = 640;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_IMG_W, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_IMGHEIGH.img_h = 480;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_IMG_H, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_IMGLNOFS.img_lnofs = 640*2;
	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_IMGLNOFS.img_lnofs = p_kdrv_gpe_ctrl.SEL.KDRV_GPE_IMGLNOFS.img_lnofs>>2; //word align
	kdrv_vddo_gpe_set(handler, VDDO_GPE_IMG_OFS, &p_kdrv_gpe_ctrl);

	printk("wait trigger done\r\n");

	kdrv_vddo_gpe_trigger(handler, NULL);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_WAITDONE.wait = TRUE;
	kdrv_vddo_gpe_set(handler, VDDO_GPE_WAIT_DONE, &p_kdrv_gpe_ctrl);

	p_kdrv_gpe_ctrl.SEL.KDRV_GPE_ENCSIZE.size = 0;
	kdrv_vddo_gpe_get(handler, VDDO_GPE_ENC_SIZE, &p_kdrv_gpe_ctrl);
	printk("size = %d\r\n", p_kdrv_gpe_ctrl.SEL.KDRV_GPE_ENCSIZE.size);
#endif

	return i_ret;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);

	/* Add error handler here */

	return i_ret;
}

int nvt_gpenc_drv_remove(MODULE_INFO *pmodule_info)
{
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		//Free IRQ
		free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);
	}

	/* Add HW Moduel release operation here*/
	gpenc_release_resource();
	gpenc2_release_resource();

	return 0;
}

int nvt_gpenc_drv_suspend(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_gpenc_drv_resume(MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_gpenc_drv_ioctl(unsigned char uc_if, MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int i_loop;
	int i_ret = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);



	switch (ui_cmd) {
	case GPENC_IOC_START:
		/*call someone to start operation*/
		break;

	case GPENC_IOC_STOP:
		/*call someone to stop operation*/
		break;

	case GPENC_IOC_READ_REG:
		i_ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!i_ret) {
			reg_info.value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info.addr);
			i_ret = copy_to_user((void __user *)ul_arg, &reg_info, sizeof(REG_INFO));
		}
		break;

	case GPENC_IOC_WRITE_REG:
		i_ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!i_ret)
			WRITE_REG(reg_info.value, pmodule_info->io_addr[uc_if] + reg_info.addr);
		break;

#if 0
	case GPENC_IOC_READ_REG_LIST:
		i_ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!i_ret) {
			for (i_loop = 0 ; i_loop < reg_info_list.count; i_loop++)
				reg_info_list.reg_list[i_loop].value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[i_loop].addr);

			i_ret = copy_to_user((void __user *)ul_arg, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case GPENC_IOC_WRITE_REG_LIST:
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

irqreturn_t nvt_gpenc_drv_isr(int irq, void *devid)
{
	int ret;

	PMODULE_INFO pmodule_info = (PMODULE_INFO)devid;

	/* simple triggle and response mechanism*/
	//complete(&pmodule_info->gpenc_completion);

	//nvt_dbg(IND, "irqnum = %d is\r\n", irq);

	/*  Tasklet for bottom half mechanism */
	//tasklet_schedule(&pmodule_info->gpenc_tasklet);
	if (irq == pmodule_info->iinterrupt_id[0]) {
		//nvt_dbg(IND, "gpenc_isr\r\n");
		gpenc_isr();
		ret = IRQ_HANDLED;
	} else {
		nvt_dbg(IND, "irqnum = %d is none\r\n", irq);
		ret = IRQ_NONE;
	}

	return ret;
}

irqreturn_t nvt_gpenc2_drv_isr(int irq, void *devid)
{
	int ret;

	PMODULE_INFO pmodule_info = (PMODULE_INFO)devid;

	/* simple triggle and response mechanism*/
	//complete(&pmodule_info->gpenc_completion);

	//nvt_dbg(IND, "irqnum = %d is\r\n", irq);

	/*  Tasklet for bottom half mechanism */
	//tasklet_schedule(&pmodule_info->gpenc_tasklet);
	if (irq == pmodule_info->iinterrupt_id[1]){
		//nvt_dbg(IND, "gpenc_isr2\r\n");
		gpenc2_isr();
		ret = IRQ_HANDLED;
	} else {
		nvt_dbg(IND, "irqnum = %d is none\r\n", irq);
		ret = IRQ_NONE;
	}

	return ret;
}


int nvt_gpenc_drv_wait_cmd_complete(PMODULE_INFO pmodule_info)
{
	wait_for_completion(&pmodule_info->gpenc_completion);
	return 0;
}

int nvt_gpenc_drv_write_reg(PMODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}

int nvt_gpenc_drv_read_reg(PMODULE_INFO pmodule_info, unsigned long addr)
{
	return READ_REG(pmodule_info->io_addr[0] + addr);
}

void nvt_gpenc_drv_do_tasklet(unsigned long data)
{
	PMODULE_INFO pmodule_info = (PMODULE_INFO)data;

	nvt_dbg(IND, "\n");

	/* do something you want*/
	complete(&pmodule_info->gpenc_completion);
}
