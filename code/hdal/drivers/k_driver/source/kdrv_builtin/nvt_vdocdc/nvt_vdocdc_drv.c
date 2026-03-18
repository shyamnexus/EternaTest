#if defined(__LINUX)
#include <plat/efuse_protected.h>
#elif defined(__FREERTOS)
#include <plat/interrupt.h>
#include "efuse_protected.h"
#endif

#include "nvt_vdocdc_drv.h"
#include "nvt_vdocdc_dbg.h"
#include "h26x.h"

//VOS_MODULE_VERSION(VDOCDC, 1, 00, 000, 00);
//VOS_MODULE_VERSION(VDOCDC, KDRV_VDOCDC_VERSION_0, KDRV_VDOCDC_VERSION_1, KDRV_VDOCDC_VERSION_2, KDRV_VDOCDC_VERSION_3);

#if defined(__LINUX)
static void nvt_vdocdc_drv_do_tasklet(unsigned long data)
{
	MODULE_INFO *p_module_info = (MODULE_INFO *)data;
	//nvt_dbg(INFO, "\n");

	/* do something you want*/
	complete(&p_module_info->comp);
}

irqreturn_t nvt_h26x_drv_isr(int irq, void *devid)
{
	MODULE_INFO *p_module_info = (MODULE_INFO *)devid;

	/* simple triggle and response mechanism*/
	complete(&p_module_info->comp);

	/*  Tasklet for bottom half mechanism */
	tasklet_schedule(&p_module_info->tasklet);

	h26x_isr(0);//hk

    return IRQ_HANDLED;
}

static void check_clock(MODULE_INFO *p_module_info)
{
	unsigned int clock = clk_get_rate(p_module_info->pclk[0]);

	if (efuse_check_available_extend(EFUSE_ABILITY_VENC_FREQ, clock)) {
		;
	}
	else {
		if (clock == 530000000) {
			if (efuse_check_available_extend(EFUSE_ABILITY_VENC_FREQ, 480000000)) {
				p_module_info->pclk_pll15 = clk_get(NULL, "pll15");
				clk_set_rate(p_module_info->pclk_pll15, 480000000);
			}
			else {
				p_module_info->pclk_pll15 = clk_get(NULL, "pll15");
				clk_set_rate(p_module_info->pclk_pll15, 400000000);
			}
		}
		else {
			p_module_info->pclk_pll15 = clk_get(NULL, "pll15");
			clk_set_rate(p_module_info->pclk_pll15, 400000000);
		}
	}
}

int nvt_vdocdc_drv_init(MODULE_INFO *p_module_info)
{
	int iRet = 0;

	init_waitqueue_head(&p_module_info->wait_queue);
	spin_lock_init(&p_module_info->spinlock);
	sema_init(&p_module_info->sem, 1);
	init_completion(&p_module_info->comp);
	tasklet_init(&p_module_info->tasklet, nvt_vdocdc_drv_do_tasklet, (unsigned long)p_module_info);

	/* clock enable */
	clk_prepare(p_module_info->pclk[0]);
#if 0
	clk_enable(p_module_info->pclk[0]);
#endif


	h26x_create_resource(0);

	check_clock(p_module_info);

	DBG_INFO("clk_prepare %lu\n", (unsigned long)(clk_get_rate(p_module_info->pclk[0])));
	h26x_open(0, (UINT32)clk_get_rate(p_module_info->pclk[0]), (uintptr_t)p_module_info->io_addr[0], (uintptr_t)ioremap(0x2F0020090, 0x10), p_module_info->pclk[0], NULL);

	/* register IRQ here*/
	if(request_irq(p_module_info->iinterrupt_id[0], nvt_h26x_drv_isr, IRQF_TRIGGER_HIGH, "H26X_INT", p_module_info)) {
		nvt_dbg(ERR, "failed to register an IRQ Int:%d\n", p_module_info->iinterrupt_id[0]);
		iRet = -ENODEV;
		goto FAIL_FREE_IRQ;
	}
	DBG_INFO("IRQ = 0x%d\n", (int)(p_module_info->iinterrupt_id[0]));

	return iRet;

FAIL_FREE_IRQ:

	free_irq(p_module_info->iinterrupt_id[0], p_module_info);

	/* Add error handler here */

	return iRet;
}

int nvt_vdocdc_drv_remove(MODULE_INFO *p_module_info)
{
	//Free IRQ
	free_irq(p_module_info->iinterrupt_id[0], p_module_info);

	h26x_release_resource(0);

	h26x_close(0);

	return 0;
}
#elif defined(__FREERTOS)

static unsigned int g_h26x_chn_cnt = 0;

irqreturn_t nvt_h26x_drv_isr(int irq, void *devid)
{
	h26x_isr(0);//hk

    return IRQ_HANDLED;
}

int nvt_vdocdc_drv_init(void)
{
	if (g_h26x_chn_cnt == 0) {
		request_irq(INT_ID_HEVC, nvt_h26x_drv_isr, IRQF_TRIGGER_HIGH, "H26X_INT", 0);

		h26x_create_resource(0);
		h26x_open(0, 320000000, 0x2f0600000, 0x2f0020090, NULL, NULL);
	}

	g_h26x_chn_cnt++;

	return 0;
}

int nvt_vdocdc_drv_remove(void)
{
	g_h26x_chn_cnt--;

	if (g_h26x_chn_cnt == 0) {
		free_irq(INT_ID_HEVC, 0);
		h26x_release_resource(0);
		h26x_close(0);
	}

	return 0;
}
#endif
