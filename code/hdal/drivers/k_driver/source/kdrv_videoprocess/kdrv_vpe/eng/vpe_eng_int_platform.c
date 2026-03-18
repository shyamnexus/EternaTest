/*
    VPR module driver

    NT98321 VPE module driver.

    @file       vpe_eng_platform.c
    @ingroup    mIVPE
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

//---------------------------------------------------------------
#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "vpe_eng.h"
#include "vpe_eng_int_platform.h"

#if defined(__FREERTOS)
#include "efuse_protected.h"
#else
#include <plat/efuse_protected.h>
#endif
//uintptr_t _vpe_reg_io_base[VPE_ID_MAX_NUM] = {0, 0};
//static ID     flg_id_vpe[VPE_ID_MAX_NUM];

VOID vpe_eng_platform_disable_sram_shutdown(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
		if(p_eng->eng_id == VPE_ID){
			//nvt_disable_sram_shutdown(VPE_SD);
		}
#elif defined (__FREERTOS)
		/*
		if(p_eng->eng_id == VPE_ID){
			nvt_disable_sram_shutdown(VPE_SD);
		}else if(p_eng->eng_id == VPE2_ID){
			nvt_disable_sram_shutdown(VPE2_SD);
		}else if(p_eng->eng_id == VPE3_ID){
			nvt_disable_sram_shutdown(VPE3_SD);
		}
		*/
#else
#endif
}
VOID vpe_eng_platform_enable_sram_shutdown(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if(p_eng->eng_id == VPE_ID){
		//nvt_enable_sram_shutdown(VPE_SD);
	}
#elif defined (__FREERTOS)
	/*
	if(p_eng->eng_id == VPE_ID){
		nvt_enable_sram_shutdown(VPE_SD);
	}else if(p_eng->eng_id == VPE2_ID){
	    nvt_enable_sram_shutdown(VPE2_SD);
	}else if(p_eng->eng_id == VPE3_ID){
	    nvt_enable_sram_shutdown(VPE3_SD);
	}
	*/

	
#else
#endif
}

VOID vpe_eng_platform_prepare_clk(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 0
	if (nvt_get_chip_id() == CHIP_NA51055) {
		DBG_ERR("VPE: do not support\r\n");
	} else {
	struct clk *parent_clk;
	parent_clk = clk_get(NULL, "pll17");
	clk_set_parent(p_eng->mclk, parent_clk);
	clk_put(parent_clk);
	}

	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_prepare(p_eng->mclk);
	}
#endif	
#elif defined (__FREERTOS)
	if(p_eng->eng_id == VPE_ID){
		pll_enableSystemReset(VPE_RSTN);
		pll_disableSystemReset(VPE_RSTN);
	}/*else if(p_eng->eng_id == VPE2_ID){
		pll_enableSystemReset(VPE2_RSTN);
		pll_disableSystemReset(VPE2_RSTN);
    }else if(p_eng->eng_id == VPE3_ID){
		pll_enableSystemReset(VPE3_RSTN);
		pll_disableSystemReset(VPE3_RSTN);
    }*/
#else
#endif
}

VOID vpe_eng_platform_unprepare_clk(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 0
	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_unprepare(p_eng->mclk);
	}
#endif
#endif
}


VOID vpe_eng_platform_enable_clk(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_enable(p_eng->mclk);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == VPE_ID){
		pll_enableClock(VPE_CLK);
    }/*else if(p_eng->eng_id == VPE2_ID){
		pll_enableClock(VPE2_CLK);
    }else if(p_eng->eng_id == VPE3_ID){
		pll_enableClock(VPE3_CLK);
    }*/

#else
#endif

}

VOID vpe_eng_platform_disable_clk(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_disable(p_eng->mclk);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == VPE_ID){
		pll_disableClock(VPE_CLK);
    	//pll_disable_apb_clock(VPE536_PCLK);
    	//pll_disable_bus_clock(VPE536_CLK);
    }/*else if(p_eng->eng_id == VPE2_ID){
		pll_disableClock(VPE2_CLK);
    }else if(p_eng->eng_id == VPE3_ID){
		pll_disableClock(VPE3_CLK);
    }*/
#else
#endif
}


VOID vpe_eng_platform_enable_gating(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_set_phase(p_eng->mclk, 1);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == VPE_ID){
		pll_clearClkAutoGating(VPE_M_GCLK);
		pll_clearPclkAutoGating(VPE_GCLK);
    }/*else if(p_eng->eng_id == VPE2_ID){
		pll_clearClkAutoGating(VPE2_M_GCLK);
		pll_clearPclkAutoGating(VPE2_GCLK);
    }else if(p_eng->eng_id == VPE3_ID){
		pll_clearClkAutoGating(VPE3_M_GCLK);
		pll_clearPclkAutoGating(VPE3_GCLK);
    }*/
#else
#endif
}


VOID vpe_eng_platform_disable_gating(VPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->mclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->mclk);
	} else {
		clk_set_phase(p_eng->mclk, 0);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == VPE_ID){
		pll_clearClkAutoGating(VPE_M_GCLK);
		pll_clearPclkAutoGating(VPE_GCLK);

    }/*else if(p_eng->eng_id == VPE2_ID){
		pll_clearClkAutoGating(VPE2_M_GCLK);
		pll_clearPclkAutoGating(VPE2_GCLK);
    }else if(p_eng->eng_id == VPE3_ID){
		pll_clearClkAutoGating(VPE3_M_GCLK);
		pll_clearPclkAutoGating(VPE3_GCLK);
    }*/
#else
#endif
}

INT32 vpe_eng_platform_set_clk_rate(VPE_ENG_HANDLE *p_eng)
{

#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->mclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->mclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	if (vpe_eng_platform_get_chip_id() == CHIP_NS02302) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else if (source_clk == 480) {
			parent_clk = clk_get(NULL, "fix480m");
		} else {
			parent_clk = clk_get(NULL, "pll17");
		}
	}
	else if (vpe_eng_platform_get_chip_id() == CHIP_NS02402) {
		if(source_clk > VPE_ENG_539A_OVER_STANDARD_RATE){
			if (efuse_check_available_extend(EFUSE_ABILITY_VPE_FREQ, VPE_ENG_539A_OVER_STANDARD_RATE*1000000) == FALSE){
				if (efuse_check_available_extend(EFUSE_ABILITY_VPE_FREQ, VPE_ENG_539A_STANDARD_RATE*1000000) == FALSE){
					DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_538_STANDARD_RATE));
					source_clk = VPE_ENG_538_STANDARD_RATE;
				}else{
					DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_539A_STANDARD_RATE));
					source_clk = VPE_ENG_539A_STANDARD_RATE;
				}
			}else{
				DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_539A_OVER_STANDARD_RATE));
				source_clk = VPE_ENG_539A_OVER_STANDARD_RATE;
			}
		}else if(source_clk > VPE_ENG_539A_STANDARD_RATE){
			if (efuse_check_available_extend(EFUSE_ABILITY_VPE_FREQ, VPE_ENG_539A_OVER_STANDARD_RATE*1000000) == FALSE){
				if (efuse_check_available_extend(EFUSE_ABILITY_VPE_FREQ, VPE_ENG_539A_STANDARD_RATE*1000000) == FALSE){
					DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_538_STANDARD_RATE));
					source_clk = VPE_ENG_538_STANDARD_RATE;
				}else{
					DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_539A_STANDARD_RATE));
					source_clk = VPE_ENG_539A_STANDARD_RATE;
				}
				//return E_SYS;
			}
		}else if(source_clk > VPE_ENG_538_STANDARD_RATE){
			if (efuse_check_available_extend(EFUSE_ABILITY_VPE_FREQ, VPE_ENG_539A_STANDARD_RATE*1000000) == FALSE){
				DBG_ERR("%s clock_rate=%d over spec(MAX: %dMHz)", p_eng->name, (int)source_clk, (int)(VPE_ENG_538_STANDARD_RATE));
				source_clk = VPE_ENG_538_STANDARD_RATE;
			}
		}
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else if (source_clk > 480) {
			parent_clk = clk_get(NULL, "pll17");
		} else {
			parent_clk = clk_get(NULL, "pll13");
		}
	} else {
		parent_clk = clk_get(NULL, "fix240m");
	}

	clk_set_parent(p_eng->mclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_VPE_480;
	UINT32 source_clk;

	// Turn on power
	// select clock
	source_clk = p_eng->clock_rate;
	if (vpe_eng_platform_get_chip_id() == 1/*CHIP_NS02203*/) {
		if (source_clk == 240) {
			selected_clock = PLL_CLKSEL_VPE_240;
		} else if (source_clk == 320) {
			selected_clock = PLL_CLKSEL_VPE_320;
		} else if (source_clk == 480) {
			selected_clock = PLL_CLKSEL_VPE_480;
		} /*else {
			selected_clock = PLL_CLKSEL_VPE_PLL17;

			if (pll_getPLLEn(PLL_ID_17) == FALSE) {
				pll_setPLLEn(PLL_ID_17, TRUE);
			}
		}
		*/

	} else {
		selected_clock = PLL_CLKSEL_VPE_240;
	}

	pll_setClockRate(PLL_CLKSEL_VPE, selected_clock);
#else
#endif

#if 0
#if defined (__LINUX)

#elif defined (__FREERTOS )
	pll_setClockRate(PLL_CLKSEL_IME, PLL_CLKSEL_IME_240);
#endif
#endif
	return E_OK;
}




//---------------------------------------------------------------

ER vpe_eng_platform_flg_clear(VPE_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return clr_flg(p_eng->flg_id_vpe, flg);
}
//---------------------------------------------------------------

ER vpe_eng_platform_flg_wait(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return wai_flg(p_flgptn, p_eng->flg_id_vpe, flg, TWF_CLR | TWF_ORW);
}

//---------------------------------------------------------------

ER vpe_eng_platform_flg_wait_timeout(VPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick)
{
	return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id_vpe, flg, TWF_ORW, timout_tick);

}
//---------------------------------------------------------------

ER vpe_eng_platform_flg_set(VPE_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return iset_flg(p_eng->flg_id_vpe, flg);
}
//---------------------------------------------------------------

static irqreturn_t vpe_eng_platform_isr(int irq, void *param)
{
	vpe_eng_isr_hw_reg((VPE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID vpe_eng_platform_request_irq(VPE_ENG_HANDLE *p_eng)
{
    int rlt;

	rlt = request_irq(p_eng->irq_id, vpe_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
	if (rlt != 0)
		DBG_WRN("request_irq fail\r\n");
}

VOID vpe_eng_platform_release_irq(VPE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

VOID vpe_eng_platform_create_resource(VPE_ENG_HANDLE *p_eng)
{
	OS_CONFIG_FLAG(p_eng->flg_id_vpe);
}

VOID vpe_eng_platform_release_resource(VPE_ENG_HANDLE *p_eng)
{
	rel_flg(p_eng->flg_id_vpe);
}


UINT32 vpe_eng_platform_get_chip_id(VOID)
{
#if 1
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
#else
	return 1;//CHIP_NS02203;

#endif
}


void *vpe_eng_malloc(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void vpe_eng_free(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}



