/*
    ISE module driver

    NT96520 ISE module driver.

    @file       ise_eng_platform.c
    @ingroup    mIIPPISE
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

//---------------------------------------------------------------
#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "ise_eng.h"
#include "ise_eng_int_platform.h"
#include "ise_eng_int_dbg.h"

static ID         flg_id_ise[2];
static SEM_HANDLE semid_ise[2];

UINT32 _ise_reg_io_base = 0;

#define ISE_ENG_CLOCK_240        240
#define ISE_ENG_CLOCK_320        320
//#define ISE_ENG_CLOCK_360        360
#define ISE_ENG_CLOCK_480        480

ER ise_eng_platform_sem_signal(UINT32 eng_id)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(semid_ise[eng_id]);
#else
	//SEM_SIGNAL(semid_ise);
	vos_sem_isig(semid_ise[eng_id]);
	return E_OK;
#endif
}

ER ise_eng_platform_sem_wait(UINT32 eng_id)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(semid_ise[eng_id]);
#else
	return SEM_WAIT(semid_ise[eng_id]);
#endif
}

VOID ise_eng_platform_prepare_clk(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("ISE: get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_prepare(p_eng->pclk);
	}
#else
    switch (p_eng->eng_id) {
        case 0://KDRV_GFX2D_ISE0:
			pll_enableSystemReset(ISE_RSTN);
			pll_disableSystemReset(ISE_RSTN);
			break;
        /*case 1://KDRV_GFX2D_ISE1:
			pll_enableSystemReset(ISE2_RSTN);
			pll_disableSystemReset(ISE2_RSTN);
            break;*/
        default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
            break;
    }
#endif
}

VOID ise_eng_platform_unprepare_clk(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("ISE: get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_unprepare(p_eng->pclk);
	}
#endif
}

VOID ise_eng_platform_enable_clk(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("ISE: get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_enable(p_eng->pclk);
	}
#else
    switch (p_eng->eng_id) {
        case 0://KDRV_GFX2D_ISE0:
			pll_enableClock(ISE_CLK);
			break;
        /*case 1://KDRV_GFX2D_ISE1:
            pll_enableClock(ISE2_CLK);
            break;*/
        default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
            break;
    }
#endif
}

VOID ise_eng_platform_disable_clk(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("ISE: get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_disable(p_eng->pclk);
	}
#else
    switch (p_eng->eng_id) {
        case 0://KDRV_GFX2D_ISE0:
			pll_disableClock(ISE_CLK);
			break;
        /*case 1://KDRV_GFX2D_ISE1:
            pll_disableClock(ISE2_CLK);
            break;*/
        default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
            break;
    }
#endif
}

INT32 ise_eng_platform_set_clk_rate(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
		struct clk *parent_clk = NULL;

		if (IS_ERR(p_eng->pclk)) {
			DBG_ERR("ISE: get clk fail...0x%p\r\n", p_eng->pclk);
			return E_SYS;
		}


		if (ise_eng_platform_get_chip_id() == CHIP_NS02302) {
			if(p_eng->clock_rate == ISE_ENG_CLOCK_240){
				parent_clk = clk_get(NULL, "fix240m");
			}else if(p_eng->clock_rate == ISE_ENG_CLOCK_320){
				parent_clk = clk_get(NULL, "fix320m");
			}else if(p_eng->clock_rate == ISE_ENG_CLOCK_480){
				parent_clk = clk_get(NULL, "fix480m");
			}else{
				parent_clk = clk_get(NULL, "pll13");
			}
		}
		else if (ise_eng_platform_get_chip_id() == CHIP_NS02402) {

			if(p_eng->clock_rate == ISE_ENG_CLOCK_240){
				parent_clk = clk_get(NULL, "fix240m");
			}else if(p_eng->clock_rate == ISE_ENG_CLOCK_320){
				parent_clk = clk_get(NULL, "fix320m");
			}else if(p_eng->clock_rate == ISE_ENG_CLOCK_480){
				parent_clk = clk_get(NULL, "pll13");
			}else{
				parent_clk = clk_get(NULL, "pll17");
			}
		} else {
			parent_clk = clk_get(NULL, "fix240m");
		}

		clk_set_parent(p_eng->pclk, parent_clk);
		clk_put(parent_clk);

#elif defined (__FREERTOS )

		UINT32 uiSelectedClock;

		// Turn on power
		// select clock
		if (ise_eng_platform_get_chip_id() == CHIP_NS02302) {
			if (p_eng->clock_rate == ISE_ENG_CLOCK_240) {
			    uiSelectedClock = PLL_CLKSEL_ISE_240;
		    }else if (p_eng->clock_rate == ISE_ENG_CLOCK_320) {
			    uiSelectedClock = PLL_CLKSEL_ISE_320;
		    }else if(p_eng->clock_rate == ISE_ENG_CLOCK_480) {
			    uiSelectedClock = PLL_CLKSEL_ISE_480;
		    }else {
				uiSelectedClock = PLL_CLKSEL_ISE_PLL13;
				if (pll_getPLLEn(PLL_ID_13) == FALSE) {
					pll_setPLLEn(PLL_ID_13, TRUE);
				}
			}
		}
		else if (ise_eng_platform_get_chip_id() == CHIP_NS02402) {

			if (p_eng->clock_rate == ISE_ENG_CLOCK_240) {
				uiSelectedClock = PLL_CLKSEL_ISE_240;
			}else if (p_eng->clock_rate == ISE_ENG_CLOCK_320) {
				uiSelectedClock = PLL_CLKSEL_ISE_320;
			}else if(p_eng->clock_rate == ISE_ENG_CLOCK_480){
				uiSelectedClock = PLL_CLKSEL_ISE_PLL13;
				if (pll_getPLLEn(PLL_ID_13) == FALSE) {
					pll_setPLLEn(PLL_ID_13, TRUE);
				}
			}else{
				uiSelectedClock = PLL_CLKSEL_ISE_PLL17;
				if (pll_getPLLEn(PLL_ID_17) == FALSE) {
					pll_setPLLEn(PLL_ID_17, TRUE);
				}
			}
		} else {
			uiSelectedClock = PLL_CLKSEL_ISE_240;
		}

		switch (p_eng->eng_id) {
			case 0://KDRV_GFX2D_ISE0:
				pll_setClockRate(PLL_CLKSEL_ISE, uiSelectedClock);
				break;
			/*case 1://KDRV_GFX2D_ISE1:
				pll_setClockRate(PLL_CLKSEL_ISE2, uiSelectedClock);
				break;*/
			default:
				DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
				break;
		}
#endif

	return E_OK;
}

VOID ise_eng_platform_disable_sram_shutdown(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	switch (p_eng->eng_id) {
		case 0://KDRV_GFX2D_ISE0:
			nvt_disable_sram_shutdown(ISE_SD);
			break;
		/*case 1://KDRV_GFX2D_ISE1:
			nvt_disable_sram_shutdown(ISE2_SD);
			break;*/
		default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
			break;
	}
#elif defined (__FREERTOS )
	switch (p_eng->eng_id) {
		case 0://KDRV_GFX2D_ISE0:
			nvt_disable_sram_shutdown(ISE_SD);
			break;
		/*case 1://KDRV_GFX2D_ISE1:
			nvt_disable_sram_shutdown(ISE2_SD);
			break;*/
		default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
			break;
	}
#endif

}

VOID ise_eng_platform_enable_sram_shutdown(ISE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	switch (p_eng->eng_id) {
		case 0://KDRV_GFX2D_ISE0:
			nvt_enable_sram_shutdown(ISE_SD);
			break;
		/*case 1://KDRV_GFX2D_ISE1:
			nvt_enable_sram_shutdown(ISE2_SD);
			break;*/
		default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
			break;
	}
#elif defined (__FREERTOS )
	switch (p_eng->eng_id) {
		case 0://KDRV_GFX2D_ISE0:
			nvt_enable_sram_shutdown(ISE_SD);
			break;
		/*case 1://KDRV_GFX2D_ISE1:
			nvt_enable_sram_shutdown(ISE2_SD);
			break;*/
		default:
			DBG_ERR("ISE: end id error!! eng_id:%d \r\n", p_eng->eng_id);
			break;
	}
#endif
}


//-----------------------------------------------------------------------------------------------------------------------
ER ise_eng_platform_flg_clear(FLGPTN flg, UINT32 eng_id)
{
	return clr_flg(flg_id_ise[eng_id], flg);
}
//---------------------------------------------------------------

ER ise_eng_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg, UINT32 eng_id)
{
	ER status;

	status = wai_flg(p_flgptn, flg_id_ise[eng_id], flg, TWF_CLR | TWF_ORW);

	return status;
}
//---------------------------------------------------------------

ER ise_eng_platform_flg_set(FLGPTN flg, UINT32 eng_id)
{
	return iset_flg(flg_id_ise[eng_id], flg);
}
//---------------------------------------------------------------

static irqreturn_t ise_eng_platform_isr(int irq, void *param)
{
	ise_eng_isr_hw_reg((ISE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID ise_eng_platform_request_irq(ISE_ENG_HANDLE *p_eng)
{
	if (request_irq(p_eng->irq_id, ise_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng))
		DBG_WRN("ise request_irq fail\n");
		//DBG_ERR("failed to register an IRQ Int:%d,name:%s,hdl:%x\n", (int)p_eng->irq_id,p_eng->name,((UINT32)p_eng));
}

VOID ise_eng_platform_release_irq(ISE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

VOID ise_eng_platform_create_resource(VOID)
{
	OS_CONFIG_FLAG(flg_id_ise[0]);
	SEM_CREATE(semid_ise[0], 1);

	//OS_CONFIG_FLAG(flg_id_ise[1]);
	//SEM_CREATE(semid_ise[1], 1);
}

VOID ise_eng_platform_release_resource(VOID)
{
	rel_flg(flg_id_ise[0]);
	SEM_DESTROY(semid_ise[0]);

	//rel_flg(flg_id_ise[1]);
	//SEM_DESTROY(semid_ise[1]);
}

UINT32 ise_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}

void *ISE_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void ISE_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}



