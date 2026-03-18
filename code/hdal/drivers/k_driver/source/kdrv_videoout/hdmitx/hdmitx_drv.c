#include <linux/wait.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/clk.h>

#include "kwrap/type.h"
#include "hdmitx_drv.h"
#include "hdmitx_ioctl.h"
#include "hdmitx_dbg.h"
#include "hdmitx.h"
#include "hdmitx_int.h"
#include "kwrap/task.h"


/*===========================================================================*/
/* Function declaration                                                      */
/*===========================================================================*/
int nvt_hdmitx_drv_wait_cmd_complete(PHDMITX_MODULE_INFO pmodule_info);
int nvt_hdmitx_drv_ioctl(unsigned char uc_if, HDMITX_MODULE_INFO *pmodule_info, unsigned int cmd, unsigned long arg);

irqreturn_t nvt_hdmitx_drv_isr(int irq, void *devid);
/*===========================================================================*/
/* Define                                                                    */
/*===========================================================================*/
typedef irqreturn_t (*irq_handler_t)(int, void *);

/*===========================================================================*/
/* Global variable                                                           */
/*===========================================================================*/
PHDMITX_MODULE_INFO phdmitx_mod_info;


void __iomem *_HDMITX_REG_BASE_ADDR[MODULE_REG_NUM];

#define HDMITX_I2C_NAME "hdmi2c"
#define HDMITX_I2C_ADDR (HDMI_DDCSLAVE_ADDR>>1)

static struct i2c_board_info hdmitx_i2c_device = {
	.type = HDMITX_I2C_NAME,
	.addr = HDMITX_I2C_ADDR,
};

HDMITX_I2C_INFO *hdmitx_i2c_info;
static int test_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	hdmitx_i2c_info = NULL;
	hdmitx_i2c_info = kzalloc(sizeof(HDMITX_I2C_INFO), GFP_KERNEL);
	if (hdmitx_i2c_info == NULL) {
		printk("%s fail: kzalloc not OK.\n", __FUNCTION__);
		return -ENOMEM;
	}

	hdmitx_i2c_info->iic_client  = client;
	hdmitx_i2c_info->iic_adapter = client->adapter;


	i2c_set_clientdata(client, hdmitx_i2c_info);

	return 0;
}
static int test_i2c_remove(struct i2c_client *client)
{
	kfree(hdmitx_i2c_info);
	hdmitx_i2c_info = NULL;
	return 0;
}
static const struct i2c_device_id hdmitx_i2c_id[] = {
	{ HDMITX_I2C_NAME, 0 },
	{ }
};
static struct i2c_driver hdmitx_i2c_driver = {
	.driver = {
		.name  = HDMITX_I2C_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = test_i2c_probe,
	.remove   = test_i2c_remove,
	.id_table = hdmitx_i2c_id
};



/*===========================================================================*/
/* Function define                                                           */
/*===========================================================================*/
int nvt_hdmitx_drv_open(PHDMITX_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel initial operation here when the device file opened*/

	return 0;
}


int nvt_hdmitx_drv_release(PHDMITX_MODULE_INFO pmodule_info, unsigned char uc_if)
{
	nvt_dbg(IND, "%d\n", uc_if);

	/* Add HW Moduel release operation here when device file closed */

	return 0;
}

int nvt_hdmitx_drv_init(HDMITX_MODULE_INFO *pmodule_info)
{
	int ret = 0;
	int i2c_num = 10; // 10 for i2c[11]
	unsigned char ucloop;

	hdmitx_api("%s\n", __func__);

	/* initial clock here */
	for (ucloop = 0 ; ucloop < MODULE_CLK_NUM; ucloop++) {
		if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {

			clk_prepare(pmodule_info->pclk[ucloop]);

			/* enable clock first. modify this later */
			//clk_enable(pmodule_info->pclk[ucloop]);
		}
	}

	/* register IRQ here*/
	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {

		if (request_irq(pmodule_info->iinterrupt_id[ucloop], nvt_hdmitx_drv_isr, IRQF_SHARED | IRQF_TRIGGER_HIGH, "HDMITX_INT", pmodule_info)) {
			nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", pmodule_info->iinterrupt_id[ucloop]);
			ret = -ENODEV;
			goto FAIL_FREE_IRQ;
		}
	}

	for (ucloop = 0 ; ucloop < MODULE_REG_NUM; ucloop++) {
		_HDMITX_REG_BASE_ADDR[ucloop] = pmodule_info->io_addr[ucloop];
		nvt_dbg(IND, "HDMITX[%d]-Addr = 0x%p\n", ucloop, _HDMITX_REG_BASE_ADDR[ucloop]);
	}

	hdmitx_create_resource();
	phdmitx_mod_info = pmodule_info;

	/* Hook i2c driver */
	if (i2c_new_device(i2c_get_adapter(i2c_num), &hdmitx_i2c_device) == NULL) {
		nvt_dbg(ERR, "%s fail: i2c_new_device not OK.\n", __FUNCTION__);
		ret = -ENODEV;
		goto FAIL_FREE_IRQ;
	} else {
		printk("hdmi hook i2c[%d] driver done.\r\n",i2c_num+1);
	}
	
	if (i2c_add_driver(&hdmitx_i2c_driver) != 0) {
		nvt_dbg(ERR, "%s fail: i2c_add_driver not OK.\n", __FUNCTION__);
		ret = -ENODEV;
		goto FAIL_FREE_IRQ;
	}


	return ret;

FAIL_FREE_IRQ:

	free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);

	/* Add error handler here */

	return ret;
}

int nvt_hdmitx_drv_remove(HDMITX_MODULE_INFO *pmodule_info)
{
	unsigned char ucloop;

	hdmitx_api("%s\n", __func__);

	//hdmitx_close();//test

	i2c_unregister_device(hdmitx_i2c_info->iic_client);

	i2c_del_driver(&hdmitx_i2c_driver);

	for (ucloop = 0 ; ucloop < MODULE_IRQ_NUM; ucloop++) {
		/* Free IRQ */
		free_irq(pmodule_info->iinterrupt_id[ucloop], pmodule_info);

		//if (!(IS_ERR(pmodule_info->pclk[ucloop]))) {
			/* disable clock */
			//clk_disable(pmodule_info->pclk[ucloop]);
		//}

	}

	/* Add HW Moduel release operation here*/
	phdmitx_mod_info = NULL;
	hdmitx_release_resource();

	return 0;
}

int nvt_hdmitx_drv_suspend(HDMITX_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add suspend operation here*/

	return 0;
}

int nvt_hdmitx_drv_resume(HDMITX_MODULE_INFO *pmodule_info)
{
	nvt_dbg(IND, "\n");

	/* Add resume operation here*/

	return 0;
}

int nvt_hdmitx_drv_ioctl(unsigned char uc_if, HDMITX_MODULE_INFO *pmodule_info, unsigned int ui_cmd, unsigned long ul_arg)
{
	REG_INFO reg_info;
	//REG_INFO_LIST reg_info_list;
	//int loop;
	int ret = 0;

	nvt_dbg(IND, "IF-%d cmd:%x\n", uc_if, ui_cmd);

	switch (ui_cmd) {
	case HDMITX_IOC_START:
		/*call someone to start operation*/
		break;

	case HDMITX_IOC_STOP:
		/*call someone to stop operation*/
		break;

	case HDMITX_IOC_READ_REG:
		ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!ret) {
			reg_info.ui_value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info.ui_addr);
			ret = copy_to_user((void __user *)ul_arg, &reg_info, sizeof(REG_INFO));
		}
		break;

	case HDMITX_IOC_WRITE_REG:
		ret = copy_from_user(&reg_info, (void __user *)ul_arg, sizeof(REG_INFO));
		if (!ret) {
			WRITE_REG(reg_info.ui_value, pmodule_info->io_addr[uc_if] + reg_info.ui_addr);
		}
		break;
/*
	case HDMITX_IOC_READ_REG_LIST:
		ret = copy_from_user(&reg_info_list, (void __user *)ul_arg, sizeof(REG_INFO_LIST));
		if (!ret) {
			for (loop = 0 ; loop < reg_info_list.ui_count; loop++) {
				reg_info_list.reg_list[loop].ui_value = READ_REG(pmodule_info->io_addr[uc_if] + reg_info_list.reg_list[loop].ui_addr);
			}

			ret = copy_to_user((void __user *)ul_arg, &reg_info_list, sizeof(REG_INFO_LIST));
		}
		break;
	case HDMITX_IOC_WRITE_REG_LIST:
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

int nvt_hdmitx_drv_write_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned long addr, unsigned long value)
{
	int idx;

	idx = (addr>>16)&0xF;

	if (pmodule_info->io_addr[idx]) {
		WRITE_REG(value, pmodule_info->io_addr[idx] + (addr&0xFFFF));
		hdmitx_api("HDMITX%d_W_REG[0x%03X]=0x%08X\n", idx, (unsigned int)(addr&0xFFFF), (unsigned int)value);
	}

	return 0;
}

int nvt_hdmitx_drv_read_reg(PHDMITX_MODULE_INFO pmodule_info, unsigned long addr)
{
	int ret = 0, idx;

	idx = (addr>>16)&0xF;

	if (pmodule_info->io_addr[idx]) {
		ret = READ_REG(pmodule_info->io_addr[idx] + (addr&0xFFFF));
		//hdmitx_api("HDMITX%d_R_REG[0x%03X]=0x%08X\n", idx, (addr&0xFFFF), Ret);
	}

	return ret;
}


irqreturn_t nvt_hdmitx_drv_isr(int irq, void *devid)
{
	PHDMITX_MODULE_INFO pmodule_info = (PHDMITX_MODULE_INFO)devid;

	if (pmodule_info == phdmitx_mod_info) {

		/* hdmitx_api("%s irq=%d\n", __func__, irq); */

		if (irq == pmodule_info->iinterrupt_id[0]) {
			if (hdmitx_isr_check()) {
				hdmitx_isr(irq, devid);
				return IRQ_HANDLED;
			} else {
				return IRQ_NONE;
			}
		}
	}

	return IRQ_NONE;
}

int nvt_hdmitx_drv_task_should_stop(void)
{
	return kthread_should_stop();
}


