#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>

#include "kwrap/type.h"
#include "dsiphy_drv.h"
#include "dsiphy_ioctl.h"
#include "dsiphy_dbg.h"
#include "dsiphy.h"
//#include "dsiphy_int.h"
#include <kdrv_builtin/kdrv_builtin.h>


/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_dsiphy_drv_wait_cmd_complete(PDSIPHY_MODULE_INFO pmodule_info);
int nvt_dsiphy_drv_ioctl(unsigned char uc_if, DSIPHY_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_dsiphy_drv_do_tasklet(unsigned long data);
irqreturn_t nvt_dsiphy_drv_isr(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
//ULONG _DSIPHY_REG_BASE_ADDR[MODULE_REG_NUM];

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_dsiphy_drv_open(PDSIPHY_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	nvt_dbg(IND, "%s\n", __func__);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_dsiphy_drv_release(PDSIPHY_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	nvt_dbg(IND, "%s\n", __func__);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_dsiphy_drv_init(DSIPHY_MODULE_INFO *pmodule_info)
{
	int ret = 0;
	//unsigned char ucloop;
	///int fastboot = kdrv_builtin_is_fastboot();

	nvt_dbg(IND, "%s\n", __func__);

	dsiphy_set_base_addr((DSIPHY_DATA_TYPE)pmodule_info->io_addr[0]);
	/*
	if (!fastboot) {
		//dsiphy_init(); // robin
	}*/

	return ret;
}

int nvt_dsiphy_drv_remove(DSIPHY_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "%s\n", __func__);

	/* Add HW Moduel release operation here*/

	return 0;
}

int nvt_dsiphy_drv_suspend(DSIPHY_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	nvt_dbg(IND, "%s\n", __func__);

	/* Add suspend operation here*/

	return 0;
}

int nvt_dsiphy_drv_resume(DSIPHY_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	nvt_dbg(IND, "%s\n", __func__);

	/* Add resume operation here*/

	return 0;
}

int nvt_dsiphy_drv_ioctl(unsigned char uc_if, DSIPHY_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int loop;
	int ret = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);

	nvt_dbg(IND, "%s IF-%d cmd:%x\n", __func__, uc_if, ui_cmd);

	switch (ui_cmd) {
	case DSIPHY_IOC_START:
		/*call someone to start operation*/
		break;

	case DSIPHY_IOC_STOP:
		/*call someone to stop operation*/
		break;

	case DSIPHY_IOC_READ_REG:
		ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!ret) {
			reg_info.ui_value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info.ui_addr);
			ret = copy_to_user((void __user *)ul_arg, &reg_info, sizeof(REG_INFO));
		}
		break;

	case DSIPHY_IOC_WRITE_REG:
		ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!ret) {
			WRITE_REG(reg_info.ui_value, pmodule_info->io_addr[uc_if] + reg_info.ui_addr);
		}
		break;
/*
	case DSIPHY_IOC_READ_REG_LIST:
		ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!ret) {
			for (loop = 0 ; loop < reg_info_list.ui_count; loop++) {
				reg_info_list.reg_list[loop].ui_value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[loop].ui_addr);
			}

			ret = copy_to_user((void __user *)ul_arg, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case DSIPHY_IOC_WRITE_REG_LIST:
		ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!ret)
			for (loop = 0 ; loop < reg_info_list.ui_count ; loop++) {
				WRITE_REG(reg_info_list.reg_list[loop].ui_value, pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[loop].ui_addr);
			}
		break;
*/
		/* Add other operations here */
	}

	return ret;
}

irqreturn_t nvt_dsiphy_drv_isr(int irq, void *devid)
{
	//PDSIPHY_MODULE_INFO pmodule_info = (PDSIPHY_MODULE_INFO)devid;

	nvt_dbg(IND, "%s irq=%d\n", __func__, irq);

	/* simple triggle and response mechanism*/
	//complete(&pmodule_info->dsiphy_completion);


	/*  Tasklet for bottom half mechanism */
	//tasklet_schedule(&pmodule_info->dsiphy_tasklet);

	return IRQ_HANDLED;
}

int nvt_dsiphy_drv_wait_cmd_complete(PDSIPHY_MODULE_INFO pmodule_info)
{
	nvt_dbg(IND, "%s\n", __func__);

	//wait_for_completion(&pmodule_info->dsiphy_completion);

	return 0;
}

int nvt_dsiphy_drv_write_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	int idx;
	idx = (addr>>16)&0xF;
	
	//for EC
	if(((addr >> 16) & 0xF) == 0x8) {
		idx = 2;
	}

	if (pmodule_info->io_addr[idx]) {
		WRITE_REG(value, pmodule_info->io_addr[idx] + (addr & 0xFFFF));
		//DBG_ERR("DSIPHY%d_W_REG[0x%03X]=0x%08X\n", idx, (unsigned int)(addr & 0xFFFF), (unsigned int)value);
	}

	return 0;
}

int nvt_dsiphy_drv_read_reg(PDSIPHY_MODULE_INFO pmodule_info, unsigned long addr)
{
	int ret = 0, idx;
	idx = (addr>>16)&0xF;

	//for EC
	if(((addr >> 16) & 0xF) == 0x8) {
		idx = 2;
	}

	if (pmodule_info->io_addr[idx]) {
		ret = READ_REG(pmodule_info->io_addr[idx] + (addr & 0xFFFF));
		//DBG_ERR("DSIPHY%d_R_REG[0x%03X]=0x%08X\n", idx, (unsigned int)(addr & 0xFFFF), (unsigned int)ret);
	}

	return ret;
}

void nvt_dsiphy_drv_do_tasklet(unsigned long data)
{
	//PDSIPHY_MODULE_INFO pmodule_info = (PDSIPHY_MODULE_INFO)data;

	nvt_dbg(IND, "\n");

	nvt_dbg(IND, "%s\n", __func__);

	/* do something you want*/
	//complete(&pmodule_info->dsiphy_completion);
}
