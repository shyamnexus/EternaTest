#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/slab.h>

#include "kwrap/type.h"
#include "uvcp_drv.h"
#include "uvcp_ioctl.h"
#include "uvcp_dbg.h"
#include "uvcp.h"
//#ifdef _NVT_UVCP2_SUPPORT
#include "uvcp2.h"
//#endif
#include "uvcp_int.h"
#include "uvcp2_int.h"

extern irqreturn_t uvcp_isr(int irq, void *devid);
//#ifdef _NVT_UVCP2_SUPPORT
extern irqreturn_t uvcp2_isr(int irq, void *devid);
//#endif
/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_uvcp_drv_ioctl(unsigned char uc_if, UVCP_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
uintptr_t _UVCP_REG_BASE_ADDR[MODULE_REG_NUM];

/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_uvcp_drv_open(PUVCP_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	uvcp_api("%s\n", __func__);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_uvcp_drv_release(PUVCP_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	uvcp_api("%s\n", __func__);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_uvcp_drv_init(UVCP_MODULE_INFO *pmodule_info)
{
	int ret = 0;
	unsigned char ucloop;

	uvcp_api("%s\n", __func__);

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM; ucloop++) {
		_UVCP_REG_BASE_ADDR[ucloop] = (uintptr_t)pmodule_info->io_addr[ucloop];
		uvcp_api("UVCP[%d]-Addr = 0x%08lX\n", ucloop, _UVCP_REG_BASE_ADDR[ucloop]);
	}

	/* initial clock here */
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			clk_prepare_enable(pmodule_info->pclk[ucloop]);
			clk_set_phase(pmodule_info->pclk[ucloop], 0);
		}
	}

//#ifdef _NVT_UVCP2_SUPPORT
#if 1
	if (request_irq(pmodule_info->iinterrupt_id[0], uvcp_isr, IRQF_TRIGGER_HIGH, "UVCP_INT", pmodule_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[0]);
		ret = -ENODEV;
		goto FAIL_FREE_IRQ;
	} else {
		uvcp_api("irq uvcp request ok.\n");
	}

	/*
	if (request_irq(pmodule_info->iinterrupt_id[1], uvcp2_isr, IRQF_TRIGGER_HIGH, "UVCP2_INT", pmodule_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[1]);
		ret = -ENODEV;
		goto FAIL_FREE_IRQ;
	} else {
		uvcp_api("irq uvcp2 request ok.\n");
	}
	*/
#else
	/* register IRQ here*/
	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		if (request_irq(pmodule_info->iinterrupt_id[ucloop], uvcp_isr, IRQF_TRIGGER_HIGH, "UVCP_INT", pmodule_info)) {
			nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[ucloop]);
			ret = -ENODEV;
			goto FAIL_FREE_IRQ;
		} else {
			uvcp_api("irq request ok.\n");
		}
	}
#endif
	#if _UVCP_KERN_SELFTEST
	{
		u32 input_size = 2048, output_size = 4096;
		u8 *pbuf_in,*pbuf_out;

		uvcp_open();

		uvcp_set_config(UVCP_CONFIG_ID_HEADER_LENGTH, 		12);
		uvcp_set_config(UVCP_CONFIG_ID_DATALEN_PER_UF, 		1024);
		uvcp_set_config(UVCP_CONFIG_ID_REFERENCE_CLK, 		30);
		uvcp_set_config(UVCP_CONFIG_ID_SOF_START_VAL, 		0x1234);
		uvcp_set_config(UVCP_CONFIG_ID_COUNTER_START_VAL, 	0x1234);
		uvcp_set_config(UVCP_CONFIG_ID_PTS, 				0x1234);

		pbuf_in = kmalloc(4096, GFP_KERNEL);
		pbuf_out = kmalloc(4096, GFP_KERNEL);

		memset(pbuf_in,  0xAA, 4096);
		memset(pbuf_out, 0xFF, 4096);

		uvcp_config_dma((uintptr_t)pbuf_in, input_size, (uintptr_t)pbuf_out, &output_size);

		uvcp_trigger(TRUE, TRUE);

		uvcp_wait_complete();
	}
	#endif

	return ret;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->iinterrupt_id[0], pmodule_info);
	free_irq(pmodule_info->iinterrupt_id[1], pmodule_info);


	return ret;

}

int nvt_uvcp_drv_remove(UVCP_MODULE_INFO *pmodule_info)
{
	unsigned char ucloop;

	uvcp_api("%s\n", __func__);

	/* Add HW Moduel release operation here*/


	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			/* disable clock */
			clk_disable_unprepare(pmodule_info->pclk[ucloop]);
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		/* Free IRQ */
		free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);
	}

	return 0;
}

int nvt_uvcp_drv_ioctl(unsigned char uc_if, UVCP_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	int ret = 0;
	UVCP_IOCAPI_DATA apidata;
//#ifdef  _NVT_UVCP2_SUPPORT
	UVCP_IOCAPI_DATA apidata2;
//#endif
	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);
	uvcp_api("%s IF-%d cmd:%x\n", __func__, uc_if, ui_cmd);

	switch (ui_cmd) {
	case UVCP_IOC_OPEN: {
		uvcp_open();
	} break;

	case UVCP_IOC_CLOSE: {
		uvcp_close();
	} break;

	case UVCP_IOC_WAIT_COMPLETE: {
		uvcp_wait_complete();
	} break;

	case UVCP_IOC_WAIT_BREAK: {
		uvcp_wait_break();
	} break;


	case UVCP_IOC_IS_OPENED: {
		UINT32 is_opened;

		ret = copy_from_user(&apidata, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {

			is_opened = (UINT32) uvcp_is_opened();

			ret = copy_to_user((void __user *)apidata.out1, &is_opened, sizeof(UINT32));

			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		} else {
			DBG_ERR("copy_from_user\n");
		}

	} break;

	case UVCP_IOC_TRIGGER: {

		ret = copy_from_user(&apidata, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			uvcp_trigger((BOOL)apidata.in1, (BOOL)apidata.in2);
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP_IOC_SET_CONFIG: {

		ret = copy_from_user(&apidata, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			uvcp_set_config((UVCP_CONFIG_ID)apidata.in1, (UINT32)apidata.in2);
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP_IOC_GET_CONFIG: {
		UINT32 retdata;

		ret = copy_from_user(&apidata, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			retdata = uvcp_get_config((UVCP_CONFIG_ID)apidata.in1);

			ret = copy_to_user((void __user *)apidata.out1, &retdata, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP_IOC_CFG_DMA: {
		UINT32 output_size;

		ret = copy_from_user(&apidata, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			ret = copy_from_user(&output_size, (void __user *)apidata.out1, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_from_user1\n");

			uvcp_config_dma(apidata.in4, apidata.in5, apidata.in6, &output_size);
		} else {
			DBG_ERR("copy_from_user2\n");
		}
	} break;


//#ifdef  _NVT_UVCP2_SUPPORT
	case UVCP2_IOC_OPEN: {
		uvcp2_open();
	} break;

	case UVCP2_IOC_CLOSE: {
		uvcp2_close();
	} break;

	case UVCP2_IOC_WAIT_COMPLETE: {
		uvcp2_wait_complete();
	} break;

	case UVCP2_IOC_WAIT_BREAK: {
		uvcp2_wait_break();
	} break;


	case UVCP2_IOC_IS_OPENED: {
		UINT32 is_opened;

		ret = copy_from_user(&apidata2, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {

			is_opened = (UINT32) uvcp2_is_opened();

			ret = copy_to_user((void __user *)apidata2.out1, &is_opened, sizeof(UINT32));

			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		} else {
			DBG_ERR("copy_from_user\n");
		}

	} break;

	case UVCP2_IOC_TRIGGER: {

		ret = copy_from_user(&apidata2, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			uvcp2_trigger((BOOL)apidata2.in1, (BOOL)apidata2.in2);
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP2_IOC_SET_CONFIG: {

		ret = copy_from_user(&apidata2, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			uvcp2_set_config((UVCP_CONFIG_ID)apidata2.in1, (UINT32)apidata2.in2);
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP2_IOC_GET_CONFIG: {
		UINT32 retdata;

		ret = copy_from_user(&apidata2, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			retdata = uvcp2_get_config((UVCP_CONFIG_ID)apidata2.in1);

			ret = copy_to_user((void __user *)apidata2.out1, &retdata, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_to_user\n");
		} else {
			DBG_ERR("copy_from_user\n");
		}
	} break;

	case UVCP2_IOC_CFG_DMA: {
		UINT32 output_size;

		ret = copy_from_user(&apidata2, (void __user *)ul_arg, sizeof(UVCP_IOCAPI_DATA));
		if (!ret) {
			ret = copy_from_user(&output_size, (void __user *)apidata2.out1, sizeof(UINT32));
			if (ret != 0)
				DBG_ERR("copy_from_user1\n");

			uvcp2_config_dma(apidata2.in4, apidata2.in5, apidata2.in6, &output_size);
		} else {
			DBG_ERR("copy_from_user2\n");
		}
	} break;
//#endif
	default:
		DBG_ERR("UNKNOWN OP 0x%08X\n", ui_cmd);
		break;
	}

	return ret;
}

#ifdef CONFIG_PM
int nvt_uvcp_drv_suspend(UVCP_MODULE_INFO *pmodule_info)
{
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	uvcp_suspend();

	/* disable clock */
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			/* disable clock */
			clk_disable_unprepare(pmodule_info->pclk[ucloop]);
		}
	}

	return 0;
}

int nvt_uvcp_drv_resume(UVCP_MODULE_INFO *pmodule_info)
{
	unsigned char ucloop;

	nvt_dbg(IND, "\n");

	/* initial clock here */
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			clk_prepare_enable(pmodule_info->pclk[ucloop]);
			clk_set_phase(pmodule_info->pclk[ucloop], 0);
		}
	}

	uvcp_resume();

	return 0;
}
#endif


