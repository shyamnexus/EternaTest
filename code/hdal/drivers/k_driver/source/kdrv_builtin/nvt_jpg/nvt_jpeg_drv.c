#ifdef __KERNEL__
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/soc/nvt/fmem.h>
// #include <mach/fmem.h>

#include <nvt_api_ver.h>

#include "jpeg_drv.h"
#include "jpeg_ioctl.h"
#include "jpeg_dbg.h"
//#include "../include/jpeg.h"
//#include "jpeg_enc.h"
#include "jpeg_file.h"
#include "jpeg_main.h"
//#include "../include/jpg_enc.h"
#include "jpeg_platform.h"
#include "kdrv_jpeg_queue.h"
//#include <plat-na51055/top.h>

//extern void jpg_patgen(void);
//extern void jpg_patdecgen(void);

extern struct clk *p_jpeg_clk[JPEG_MAX_ENTITYS];

// UINT32      jpeg_clock;
/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_jpg_drv_wait_cmd_complete(PJPG_MODULE_INFO pmodule_info);
int nvt_jpg_drv_ioctl(unsigned char interface, JPG_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
void nvt_jpg_drv_do_tasklet(unsigned long data);
irqreturn_t nvt_jpg_drv_isr(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
//static DEFINE_SPINLOCK(my_lock);

//#define loc_cpu(myflags)   spin_lock_irqsave(&my_lock, myflags)
//#define unl_cpu(myflags)   spin_unlock_irqrestore(&my_lock, myflags)

typedef irqreturn_t (*irq_handler_t)(int, void *);

extern JPEG_ENGINE_DATA jpeg_eng_data[JPEG_MAX_ENTITYS];

extern UINT32 jpeg_isr(UINT32 entity_idx);
extern int jpeg_process_done(UINT32 engine_idx, UINT32 status);
extern int jpeg_start_schedule(UINT32 chip, UINT32 engine);
extern void jpeg_drv_bottom_half(unsigned int chip_idx, unsigned int eng_idx, int int_status);

//KDRV_JPEG_TRIG_INFO g_jpeg_trig_info;

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
//int iEventFlag = 0;

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_jpg_drv_open(PJPG_MODULE_INFO pmodule_info, unsigned char interface)
{
	DBG_INFO("%d\n", interface);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}

int nvt_jpg_drv_release(PJPG_MODULE_INFO pmodule_info, unsigned char interface)
{
	DBG_INFO("%d\n", interface);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_jpg_drv_init(JPG_MODULE_INFO *pmodule_info)
{
	int ret = 0;
	// struct clk *mclk;
	extern NVT_API_CHK_DECLARE(nvt_jpeg);

	NVT_API_CHK_CALL(nvt_jpeg);

	init_waitqueue_head(&pmodule_info->jpg_wait_queue);
	//spin_lock_init(&pmodule_info->jpg_spinlock);
	sema_init(&pmodule_info->jpg_sem, 1);
	init_completion(&pmodule_info->jpg_completion);
	tasklet_init(&pmodule_info->jpg_tasklet, nvt_jpg_drv_do_tasklet, (unsigned long)pmodule_info);

	/* initial clock here */
	/*
	if  (nvt_get_chip_id() == CHIP_NA51055) {
		mclk = clk_get(NULL, "pllf320");
		if (IS_ERR(mclk)) {
			printk("%s: get source pllf320 fail\r\n", __func__);
		}
		jpeg_clock = 320;
	} else
	*/

#if 0 //NA51107-2588: load dtsi clk
	{
		mclk = clk_get(NULL, "fix480m");
		if (IS_ERR(mclk)) {
			printk("%s: get source fix480m fail\r\n", __func__);
		}
		// jpeg_clock = 480;
	}
	clk_set_parent(pmodule_info->mclk[pmodule_info->engine_idx], mclk);
#endif

	//jpeg clk set
	p_jpeg_clk[pmodule_info->engine_idx] = pmodule_info->mclk[pmodule_info->engine_idx];

	clk_prepare(pmodule_info->mclk[pmodule_info->engine_idx]);
#if defined(__FREERTOS)
	//rtos auto gating not ready
	clk_enable(pmodule_info->mclk[pmodule_info->engine_idx]);
	jpeg_eng_data[pmodule_info->engine_idx].clock_status = 1;
#else
	//someone else may open the clk, which ensures that clk is disabled
	clk_enable(pmodule_info->mclk[pmodule_info->engine_idx]);
	clk_disable(pmodule_info->mclk[pmodule_info->engine_idx]);
	jpeg_eng_data[pmodule_info->engine_idx].clock_status = 0;
#endif

	// clk_put(mclk);
	//printk("clk_prepare\n");

	//check auto gating is OFF
	clk_set_phase(pmodule_info->mclk[pmodule_info->engine_idx], 0);

	/* register IRQ here*/
	// check command: cat /proc/interrupts
	if (pmodule_info->engine_idx == 0) {
		if (request_irq(pmodule_info->iinterrupt_id[pmodule_info->engine_idx], nvt_jpg_drv_isr, IRQF_TRIGGER_HIGH, "JPEG1_INT", pmodule_info)) {
			DBG_ERR("failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[pmodule_info->engine_idx]);
			ret = -ENODEV;
			goto FAIL_FREE_IRQ;
		}
	}
	// if (pmodule_info->engine_idx == 1) {
	// 	if (request_irq(pmodule_info->iinterrupt_id[pmodule_info->engine_idx], nvt_jpg_drv_isr, IRQF_TRIGGER_HIGH, "JPEG2_INT", pmodule_info)) {
	// 		DBG_ERR("failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[pmodule_info->engine_idx]);
	// 		ret = -ENODEV;
	// 		goto FAIL_FREE_IRQ;
	// 	}
	// }
	// if (pmodule_info->engine_idx == 2) {
	// 	if (request_irq(pmodule_info->iinterrupt_id[pmodule_info->engine_idx], nvt_jpg_drv_isr, IRQF_TRIGGER_HIGH, "JPEGL_INT", pmodule_info)) {
	// 		DBG_ERR("failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[pmodule_info->engine_idx]);
	// 		ret = -ENODEV;
	// 		goto FAIL_FREE_IRQ;
	// 	}
	// }
	// printk("request_irq[%d] = 0x%d \n", pmodule_info->engine_idx, (int)(pmodule_info->iinterrupt_id[pmodule_info->engine_idx]));

	/* Add HW Module initialization here when driver loaded */
	// if (pmodule_info->engine_idx == 0)
	// 	jpeg_set_baseaddr((uintptr_t)pmodule_info->io_addr[pmodule_info->engine_idx]);

	//printk("%d io_addr = %lx \n", (int)pmodule_info->engine_idx, (uintptr_t)pmodule_info->io_addr[pmodule_info->engine_idx]);

	jpeg_create_resource(0 /*pmodule_info->chip_idx*/, pmodule_info->engine_idx);

	//jpg_patgen();
	//jpg_patdecgen();

	// init dsp trig info
	jpeg_platform_triginfo_init();
	//memset(&g_jpeg_trig_info, 0x00, sizeof(g_jpeg_trig_info));
	//g_jpeg_trig_info.p_queue = kdrv_jpeg_get_queue_by_coreid();
	//g_jpeg_trig_info.tri_func = (JPEG_TRI_FUNC) jpeg_trigger;


	// init jpeg queue
	kdrv_jpeg_queue_init_p();

	return ret;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->iinterrupt_id[pmodule_info->engine_idx], pmodule_info);

	/* Add error handler here */

	return ret;
}

int nvt_jpg_drv_remove(JPG_MODULE_INFO *pmodule_info)
{

	//Free IRQ
	free_irq(pmodule_info->iinterrupt_id[pmodule_info->engine_idx], pmodule_info);

	/* Add HW Moduel release operation here*/

	return 0;
}

int nvt_jpg_drv_suspend(JPG_MODULE_INFO *pmodule_info)
{
	//DBG_INFO("\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_jpg_drv_resume(JPG_MODULE_INFO *pmodule_info)
{
	//DBG_INFO("\n");
	/* Add resume operation here*/

	return 0;
}

int nvt_jpg_drv_ioctl(unsigned char interface, JPG_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg)
{
	//REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int loop;
	int ret = 0;

	DBG_INFO("IF-%d cmd:%x\n", interface, cmd);

	switch (cmd) {
	case XXX_IOC_START:
		/*call someone to start operation*/
		break;

	case XXX_IOC_STOP:
		/*call someone to stop operation*/
		break;
#if 0
	case XXX_IOC_READ_REG:
		ret = copy_from_user(&reg_info, (void __user *)arg, sizeof(REG_INFO));
		if (!ret) {
			reg_info.reg_value = READ_REG(pmodule_info->io_addr[interface] + reg_info.reg_addr);
			ret = copy_to_user((void __user *)arg, &reg_info, sizeof(REG_INFO));
		}
		break;

	case XXX_IOC_WRITE_REG:
		ret = copy_from_user(&reg_info, (void __user *)arg, sizeof(REG_INFO));
		if (!ret) {
			WRITE_REG(reg_info.reg_value, pmodule_info->io_addr[interface] + reg_info.reg_addr);
		}
		break;

	case XXX_IOC_READ_REG_LIST:
		ret = copy_from_user(&reg_info_list, (void __user *)arg, sizeof(REG_INFO_LIST));
		if (!ret) {
			for (loop = 0 ; loop < reg_info_list.reg_listcount; loop++) {
				reg_info_list.reg_list[loop].reg_value = READ_REG(pmodule_info->io_addr[interface] + reg_info_list.reg_list[loop].reg_addr);
			}

			ret = copy_to_user((void __user *)arg, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;

	case XXX_IOC_WRITE_REG_LIST:
		ret = copy_from_user(&reg_info_list, (void __user *)arg, sizeof(REG_INFO_LIST));
		if (!ret)
			for (loop = 0 ; loop < reg_info_list.reg_listcount ; loop++) {
				WRITE_REG(reg_info_list.reg_list[loop].reg_value, pmodule_info->io_addr[interface] + reg_info_list.reg_list[loop].reg_addr);
			}
		break;
#endif

		/* Add other operations here */
	}

	return ret;
}

irqreturn_t nvt_jpg_drv_isr(int irq, void *devid)
{
	PJPG_MODULE_INFO pmodule_info = (PJPG_MODULE_INFO)devid;

#if SUP_NON_BLK_MODE
	if (NULL == jpeg_eng_data[JPEG_ENTITY_IDX(0, pmodule_info->engine_idx)].fire_job) {
#else
	if (1) {
#endif
		/* simple triggle and response mechanism*/
		complete(&pmodule_info->jpg_completion);

		jpeg_isr(pmodule_info->engine_idx);

		/* Tasklet for bottom half mechanism */
		tasklet_schedule(&pmodule_info->jpg_tasklet);
	}
	else {
		// handling isr
		UINT32 status;

		status = jpeg_isr(pmodule_info->engine_idx);
		jpeg_drv_bottom_half(0, pmodule_info->engine_idx, status);
		/* put in bottom_half */
		// jpeg_mark_engine_end(0, pmodule_info->engine_idx, 0);
		// //fire_job = jpeg_eng_data[pmodule_info->engine_idx].
		// jpeg_process_done(pmodule_info->engine_idx, status);
		// jpeg_set_engine_idle(0, pmodule_info->engine_idx);
		// jpeg_start_schedule(0, JPEG_ALL_ENGINE/*pmodule_info->engine_idx*/);
	}

	return IRQ_HANDLED;
}

int nvt_jpg_drv_wait_cmd_complete(PJPG_MODULE_INFO pmodule_info)
{
	wait_for_completion(&pmodule_info->jpg_completion);
	return 0;
}

int nvt_jpg_drv_write_reg(PJPG_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	WRITE_REG(value, pmodule_info->io_addr[0] + addr);
	return 0;
}

int nvt_jpg_drv_read_reg(PJPG_MODULE_INFO pmodule_info, unsigned long addr)
{
	return READ_REG(pmodule_info->io_addr[0] + addr);
}

void nvt_jpg_drv_do_tasklet(unsigned long data)
{
	PJPG_MODULE_INFO pmodule_info = (PJPG_MODULE_INFO)data;

	//DBG_INFO("\n");
	//printk("nvt_jpg_drv_do_tasklet\r\n");
	jpeg_platform_ist(pmodule_info->engine_idx);

	/* do something you want*/
	complete(&pmodule_info->jpg_completion);
}

#endif

//KDRV_JPEG_TRIG_INFO *kdrv_jpeg_get_triginfo_by_coreid(void)
//{
//	return &g_jpeg_trig_info;
//}

