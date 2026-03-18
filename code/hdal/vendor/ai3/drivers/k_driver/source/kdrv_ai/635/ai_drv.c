#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/vmalloc.h>
#include "kwrap/type.h"
#include "ai_drv.h"
#include "ai_lib.h"
#include "ai_ioctl.h"
#if defined(__FREERTOS)
#include "kwrap/debug.h"
#else
#include "kdrv_ai_dbg.h"
#endif
#include "kdrv_ai.h"
#include <linux/soc/nvt/fmem.h>
#include <kwrap/verinfo.h>
#if defined(_BSP_NA51055_)
#include <plat-na51055/nvt-sramctl.h>
#endif
#if defined(_BSP_NA51102_)
//#include <plat-na51102/nvt-sramctl.h>
#endif

#include "kdrv_ai_int.h"
#include "kwrap/flag.h"
#include "kdrv_ai_version.h"
#include "ai_api.h"
#include "rou_eng.h"
#include "nue2_eng.h"
#include "conv_eng.h"
#include "jobm_eng.h"
#include "lsu_eng.h"
#include "util_eng.h"
#include "cal_eng.h"
#include "ppu_eng.h"
#include "jmisp_eng.h"

/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_ai_drv_wait_cmd_complete(PAI_INFO pmodule_info);
int nvt_ai_drv_ioctl(unsigned char if_id, AI_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_ai_drv_do_tasklet(unsigned long data);
irqreturn_t nvt_ai_drv_isr(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/

typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/

UINT8 * kdrv_ai_vmalloc(UINT32 v_size)
{
	char *v_buff;

	v_buff = (char *) vmalloc(v_size);

	return (UINT8 *) v_buff;
}
EXPORT_SYMBOL(kdrv_ai_vmalloc);

VOID kdrv_ai_vfree(UINT8 *v_buff)
{
	if (v_buff) {
		vfree((char *)v_buff);
	}

	return;
}
EXPORT_SYMBOL(kdrv_ai_vfree);


int nvt_ai_drv_open(PAI_INFO pmodule_info, unsigned char if_id)
{
	/* Add HW Module initial operation here when the device file opened*/

	return 0;
}


int nvt_ai_drv_release(PAI_INFO pmodule_info, unsigned char if_id)
{
	/* Add HW Module release operation here when device file closed */

	return 0;
}

int nvt_ai_drv_init(AI_INFO *pmodule_info)
{
	int err = 0, i = 0;	
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid input\r\n");
		return -EINVAL;
	}
	
	err = kdrv_ai_chk_efuse_ai_eng();
	if (err != 0) {
		return err;
	}

	err = kdrv_ai_chk_efuse_kdrv_ai_api();
	if (err != 0) {
		return err;
	}
	
	err = kdrv_ai_chk_efuse_ai_api();
	if (err != 0) {
		return err;
	}
	
	err = kdrv_ai_chk_efuse_ai_lib();
	if (err != 0) {
		return err;
	}
	
	/*err = kdrv_ai_chk_efuse_kdrv_ai_int();
	if (err != 0) {
		return err;
	}*/
	
	init_waitqueue_head(&pmodule_info->ai_wait_queue);
	vk_spin_lock_init(&pmodule_info->ai_spinlock);
	vk_sema_init(&pmodule_info->ai_sem, 1);
	init_completion(&pmodule_info->ai_completion);
	tasklet_init(&pmodule_info->ai_tasklet, nvt_ai_drv_do_tasklet, (unsigned long)pmodule_info);


	/* register IRQ here*/
	for (i = 0; i < MODULE_IRQ_NUM; i++) {
		if (request_irq(pmodule_info->iinterrupt_id[i], nvt_ai_drv_isr, IRQF_TRIGGER_HIGH, "AI_INT", pmodule_info)) {
			nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[i]);
			err = -ENODEV;
			goto FAIL_FREE_IRQ;
		}
	}
	
	for (i = 0; i < MODULE_REG_NUM; i++) {
		ai_eng_io_addr[i] = (uintptr_t)pmodule_info->io_addr[i];
	}

	return err;

FAIL_FREE_IRQ:
	free_irq(pmodule_info->iinterrupt_id[i], pmodule_info);

	/* Add error handler here */

	return err;
}

int nvt_ai_drv_remove(AI_INFO *pmodule_info)
{
	UINT32 i = 0;
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid input\r\n");
		return -EINVAL;
	}

	for (i = 0; i < MODULE_IRQ_NUM; i++) {
		free_irq(pmodule_info->iinterrupt_id[i], pmodule_info);
	}

	return 0;
}

int nvt_ai_drv_suspend(AI_INFO *pmodule_info)
{
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid input\r\n");
		return -EINVAL;
	}

	/* Add suspend operation here*/

	return 0;
}

int nvt_ai_drv_resume(AI_INFO *pmodule_info)
{
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid input\r\n");
		return -EINVAL;
	}

	/* Add resume operation here*/

	return 0;
}

int nvt_ai_drv_ioctl(unsigned char if_id, AI_INFO *pmodule_info, unsigned int cmd, unsigned long argc)
{
	REG_INFO reg_info;
	REG_INFO_LIST reg_info_list;
	int loop_count;
	int err = 0;

	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid input\r\n");
		return -EINVAL;
	}

	switch (cmd) {
	case AI_IOC_START:
		/*call someone to start operation*/
		break;

	case AI_IOC_STOP:
		/*call someone to stop operation*/
		break;

	case AI_IOC_READ_REG:
		err = copy_from_user(&reg_info, (void __user *)argc, sizeof(REG_INFO));
		if (!err) {
			reg_info.reg_value = READ_REG(pmodule_info->io_addr[if_id] + reg_info.reg_addr);
			err = copy_to_user((void __user *)argc, &reg_info, sizeof(REG_INFO));
		}
		break;

	case AI_IOC_WRITE_REG:
		err = copy_from_user(&reg_info, (void __user *)argc, sizeof(REG_INFO));
		if (!err) {
			WRITE_REG(reg_info.reg_value, pmodule_info->io_addr[if_id] + reg_info.reg_addr);
		}
		break;

	case AI_IOC_READ_REG_LIST:
		err = copy_from_user(&reg_info_list, (void __user *)argc, sizeof(REG_INFO_LIST));
		if (!err) {
			if (reg_info_list.reg_cnt <= MODULE_REG_LIST_NUM) {
				for (loop_count = 0 ; loop_count < reg_info_list.reg_cnt ; loop_count++) {
					reg_info_list.reg_list[loop_count].reg_value = READ_REG(pmodule_info->io_addr[if_id] + reg_info_list.reg_list[loop_count].reg_addr);
				}
			} else {
				DBG_ERR("Loop bound error!\r\n");
			}
			err = copy_to_user((void __user *)argc, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case AI_IOC_WRITE_REG_LIST:
		err = copy_from_user(&reg_info_list, (void __user *)argc, sizeof(REG_INFO_LIST));
		if (!err) {
			if (reg_info_list.reg_cnt <= MODULE_REG_LIST_NUM) {
				for (loop_count = 0 ; loop_count < reg_info_list.reg_cnt ; loop_count++) {
					WRITE_REG(reg_info_list.reg_list[loop_count].reg_value, pmodule_info->io_addr[if_id] + reg_info_list.reg_list[loop_count].reg_addr);
				}
			} else {
				DBG_ERR("Loop bound error!\r\n");
			}
		}
		break;

	case AI_IOC_GET_VER: {
		CHAR version_info[32] = KDRV_AI_IMPL_VERSION;
		err = copy_to_user((void __user *)argc, version_info, sizeof(KDRV_AI_IMPL_VERSION));
	}
		break;
	
	case AI_IOC_PAT_TEST: {
		AI_PAT_PARM parm = {0};
		UINT32 i = 0;
		UINT32 infinite_loop = 0;
		UINT32 pass_cnt = 0;
		CONV_ENG_HANDLE* p_conv_handle = NULL;
		JOBM_ENG_HANDLE* p_jm_handle = NULL;
		
		err = copy_from_user(&parm, (void __user *)argc, sizeof(AI_PAT_PARM));
		
		if (err) {
			printk("[AI test] copy_from_user fail\n");
			return err;
		}
		// prepare parameter
		if (parm.round == 0) {
			infinite_loop = 1;
			parm.round = 1;
		}
		
		// run engine
		err = kdrv_ai_init();
		if (err) {
			printk("[AI test] init fail\n");
			return err;
		}
		p_conv_handle = conv_eng_get_handle(0, 0);
		p_jm_handle   = jobm_eng_get_handle(0, 0);
		if (p_conv_handle == NULL || p_jm_handle == NULL) {
			printk("[AI test] get handle fail\n");
			return err;
		}
		for (i = 0; i < parm.round; i++) {
			kdrv_ai_trigger(KDRV_AI_ENG_JOBM_JLA, parm.cmd_pa);
			jobm_eng_wait_framend(p_jm_handle, 0);
			// cmp chksum
			if (parm.golden_chksum != conv_eng_get_out_chksum(p_conv_handle)) {
				printk("[AI test] run %d cmp fail! emu = 0x%08X, golden = 0x%08X\n", pass_cnt, 
						conv_eng_get_out_chksum(p_conv_handle), parm.golden_chksum);
				break;
			}
			pass_cnt++;
			if (parm.log_cnt > 0 && (pass_cnt%parm.log_cnt) == 0) {
				if (infinite_loop) printk("[AI test] run %d pass\n", pass_cnt);
				else printk("[AI test] run %d/%d pass\n", pass_cnt, parm.round);
			}
			if (infinite_loop) i--;
		}
		kdrv_ai_uninit();
	}
		break;
		
	default :
		nvt_dbg(ERR, "unknown ai ioctl cmd: %08x\r\n", cmd);
		break;
	}

	return err;
}

irqreturn_t nvt_ai_drv_isr(int irq, void *devid)
{
	PAI_INFO pmodule_info = (PAI_INFO)devid;
	if (devid == NULL) {
		nvt_dbg(ERR, "invalid devid\r\n");
		return IRQ_NONE;
	}
	/* simple triggle and response mechanism*/
	complete(&pmodule_info->ai_completion);


	/*  Tasklet for bottom half mechanism */
	tasklet_schedule(&pmodule_info->ai_tasklet);
	
	if (irq == pmodule_info->iinterrupt_id[0]) {
		CONV_ENG_HANDLE* p_handle = conv_eng_get_handle(0, 0);
		conv_eng_isr_hw_reg(p_handle);
	}
	
	if (irq == pmodule_info->iinterrupt_id[1]) {
		NUE2_ENG_HANDLE* p_handle = nue2_eng_get_handle(0, 0);
		nue2_eng_isr_hw_reg(p_handle);
	}
	
	if (irq == pmodule_info->iinterrupt_id[2]) {
		PPU_ENG_HANDLE* p_handle = ppu_eng_get_handle(0, 0);
		ppu_eng_isr_hw_reg(p_handle);
	}
	
	if (irq == pmodule_info->iinterrupt_id[3]) {
		LSU_ENG_HANDLE* p_handle = lsu_eng_get_handle(0, 0);
		lsu_eng_isr_hw_reg(p_handle);
	}	
	
	if (irq == pmodule_info->iinterrupt_id[4]) {
		JOBM_ENG_HANDLE* p_handle = jobm_eng_get_handle(0, 0);
		jobm_eng_isr_hw_reg(p_handle);
	}
		
	if (irq == pmodule_info->iinterrupt_id[5]) {
		JMISP_ENG_HANDLE* p_handle = jmisp_eng_get_handle(0, 0);
		jmisp_eng_isr_hw_reg(p_handle);
	}
	
	if (irq == pmodule_info->iinterrupt_id[6]) {
		UTIL_ENG_HANDLE* p_handle = util_eng_get_handle(0, 0);
		util_eng_isr_hw_reg(p_handle);
	}
	
	if (irq == pmodule_info->iinterrupt_id[7]) {
		ROU_ENG_HANDLE* p_handle = rou_eng_get_handle(0, 0);
		rou_eng_isr_hw_reg(p_handle);
	}	
	
	if (irq == pmodule_info->iinterrupt_id[8]) {
		CAL_ENG_HANDLE* p_handle = cal_eng_get_handle(0, 0);
		cal_eng_isr_hw_reg(p_handle);
	}

	if (irq == pmodule_info->iinterrupt_id[9]) {
		ROU_ENG_HANDLE* p_handle = rou_eng_get_handle(0, 1);
		rou_eng_isr_hw_reg(p_handle);
	}

	if (irq == pmodule_info->iinterrupt_id[10]) {
		LSU_ENG_HANDLE* p_handle = lsu_eng_get_handle(0, 1);
		lsu_eng_isr_hw_reg(p_handle);
	}


	return IRQ_HANDLED;
}

int nvt_ai_drv_wait_cmd_complete(PAI_INFO pmodule_info)
{
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid pmodule_info\r\n");
		return -EINVAL;
	}
	
	wait_for_completion(&pmodule_info->ai_completion);
	return 0;
}

int nvt_ai_drv_write_reg(PAI_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid pmodule_info\r\n");
		return -EINVAL;
	}
	
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}

int nvt_ai_drv_read_reg(PAI_INFO pmodule_info, unsigned long addr)
{
	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid pmodule_info\r\n");
		return -EINVAL;
	}
	
	return READ_REG(pmodule_info->io_addr[0] + addr);
}

void nvt_ai_drv_do_tasklet(unsigned long data)
{
	PAI_INFO pmodule_info = (PAI_INFO)data;

	if (pmodule_info == NULL) {
		nvt_dbg(ERR, "invalid pmodule_info\r\n");
		return;
	}
	//nvt_dbg(IND, "\n");

	/* do something you want*/
	complete(&pmodule_info->ai_completion);
}

