#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "ipe_eng_int_platform.h"
#include "ipe_eng_int.h"
#include "ipe_eng_int_dbg.h"
#include "ipe_eng.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include <plat/top.h>

//ULONG _ipe_reg_io_base = 0;

static SEM_HANDLE semid_ipe, semid_ipe2;
static ID flg_id_ipe, flg_id_ipe2;


ER ipe_eng_platform_sem_wait(IPE_ENG_HANDLE *p_eng)
{
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
#if defined __UITRON || defined __ECOS
            return wai_sem(semid_ipe2);
#else
            return SEM_WAIT(semid_ipe2);
#endif

    }
    else
#endif
    {
#if defined __UITRON || defined __ECOS
            return wai_sem(semid_ipe);
#else
            return SEM_WAIT(semid_ipe);
#endif

    }

}

ER ipe_eng_platform_sem_signal(IPE_ENG_HANDLE *p_eng)
{
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {

#if defined __UITRON || defined __ECOS
	return sig_sem(semid_ipe2);
#else
	SEM_SIGNAL(semid_ipe2);
	return E_OK;
#endif
    }
    else
#endif
    {
#if defined __UITRON || defined __ECOS
            return sig_sem(semid_ipe);
#else
            SEM_SIGNAL(semid_ipe);
            return E_OK;
#endif

    }
}



VOID ipe_eng_platform_prepare_clk(IPE_ENG_HANDLE *p_eng)
{
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
#if defined (__LINUX)
            if (IS_ERR(p_eng->pclk)) {
                DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
            } else {
                clk_prepare(p_eng->pclk);
            }
#else
            pll_enableSystemReset(IPE2_RSTN);
            pll_disableSystemReset(IPE2_RSTN);
#endif
    }
    else
#endif
    {
#if defined (__LINUX)
										struct clk *parent_clk;
                    if (IS_ERR(p_eng->pclk)) {
                        DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
                    } else {
                    		if(nvt_get_chip_ver()==CHIPVER_A){
                        clk_prepare(p_eng->pclk);
                        }else/* if (nvt_get_chip_ver()==CHIPVER_B)*/{
													clk_prepare(p_eng->pclk);
													parent_clk = clk_get(NULL, "pll17");
													clk_prepare(parent_clk);
													clk_put(parent_clk);			
												}
                    }
#else
                    pll_enableSystemReset(IPE_RSTN);
                    pll_disableSystemReset(IPE_RSTN);
#endif

    }


}

VOID ipe_eng_platform_unprepare_clk(IPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		if(nvt_get_chip_ver()==CHIPVER_A){
			clk_unprepare(p_eng->pclk);
		} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
			clk_unprepare(p_eng->pclk);
			parent_clk = clk_get(NULL, "pll17");
			clk_unprepare(parent_clk);
			clk_put(parent_clk);			
		}
	}
#endif
}

VOID ipe_eng_platform_enable_clk(IPE_ENG_HANDLE *p_eng)
{
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
#if defined (__LINUX)
            if (IS_ERR(p_eng->pclk)) {
                DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
            } else {
                clk_enable(p_eng->pclk);
            }
#else
            pll_enableClock(IPE2_CLK);
#endif
    }
    else
#endif
    {
#if defined (__LINUX)
										struct clk *parent_clk;
                    if (IS_ERR(p_eng->pclk)) {
                        DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
                    } else {
                        if(nvt_get_chip_ver()==CHIPVER_A){
                        clk_enable(p_eng->pclk);
												} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
													clk_enable(p_eng->pclk);
													parent_clk = clk_get(NULL, "pll17");
													clk_enable(parent_clk);
													clk_put(parent_clk);			
												}		
                    }
#else
                    pll_enableClock(IPE_CLK);
#endif

    }


}

VOID ipe_eng_platform_disable_clk(IPE_ENG_HANDLE *p_eng)
{
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {

#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_disable(p_eng->pclk);
	}
#else
	pll_disableClock(IPE2_CLK);
#endif
    }
    else
#endif
    {
#if defined (__LINUX)
			//struct clk *parent_clk;
            if (IS_ERR(p_eng->pclk)) {
                DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
            } else {
				if(nvt_get_chip_ver()==CHIPVER_A){
                	clk_disable(p_eng->pclk);
				} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
					clk_disable(p_eng->pclk);
					//parent_clk = clk_get(NULL, "pll17");
					//clk_disable(parent_clk);
					//clk_put(parent_clk);
				}
            }
#else
            pll_disableClock(IPE_CLK);
#endif

    }
}

INT32 ipe_eng_platform_set_clk_rate(IPE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	#if(1)
	if (ipe_eng_platform_get_chip_id() == CHIP_NS02302) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else {
			if(nvt_get_chip_ver()==CHIPVER_A){
				if(source_clk >IPE_CHIP_A_PLL_13_MAX){
					DBG_ERR("IPE(A): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IPE_CHIP_A_PLL_13_MAX);
					return E_SYS;
				}
				parent_clk = clk_get(NULL, "pll13");
				clk_set_rate(parent_clk, source_clk*1000000);
			} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
				if(source_clk >IPE_CHIP_B_PLL_17_MAX){
					DBG_ERR("IPE(B): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IPE_CHIP_B_PLL_17_MAX);
					return E_SYS;
				}
				parent_clk = clk_get(NULL, "pll17");
				clk_set_rate(parent_clk, source_clk*1000000);
			}
		}
	} else
    #endif
	{
		parent_clk = clk_get(NULL, "fix240m");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock;
	UINT32 source_clk;

	// Turn on power
	pmc_turnonPower(PMC_MODULE_IPE);

	// select clock
#if(1)
	source_clk = p_eng->clock_rate;
	if (ipe_eng_platform_get_chip_id() == CHIP_NS02302)

	{
		if (source_clk == 240) {
			selected_clock = PLL_CLKSEL_IPE_240;
		} else if (source_clk == 320) {
			selected_clock = PLL_CLKSEL_IPE_320;
		} else {
			selected_clock = PLL_CLKSEL_IPE_PLL13;

            if (pll_getPLLEn(PLL_ID_13) == FALSE) {
                pll_setPLLEn(PLL_ID_13, TRUE);
            }
		}

	}
	else
    #endif
	{
		selected_clock = PLL_CLKSEL_IPE_240;
	}

    #if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
	    pll_setClockRate(PLL_CLKSEL_IPE2, selected_clock);
    }
    else
    #endif
    {
        pll_setClockRate(PLL_CLKSEL_IPE, selected_clock);
    }
#endif

#if 0
#if defined (__LINUX)

#elif defined (__FREERTOS )
	pll_setClockRate(PLL_CLKSEL_IPE, PLL_CLKSEL_IPE_240);
#endif
#endif
	return E_OK;
}

VOID ipe_eng_platform_disable_sram_shutdown(IPE_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE0) {
#if defined (__LINUX)
                nvt_disable_sram_shutdown(IPE_SD);
#elif defined (__FREERTOS)
                nvt_disable_sram_shutdown(IPE_SD);
#else
#endif

	}
#if(0)
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1) {
#if defined (__LINUX)
                nvt_disable_sram_shutdown(IPE2_SD);
#elif defined (__FREERTOS)
                nvt_disable_sram_shutdown(IPE2_SD);
#else
#endif

	}
#endif
}


VOID ipe_eng_platform_enable_sram_shutdown(IPE_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE0) {
#if defined (__LINUX)
                nvt_enable_sram_shutdown(IPE_SD);
#elif defined (__FREERTOS)
                nvt_enable_sram_shutdown(IPE_SD);
#else
#endif

	}
#if(0)
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1) {
#if defined (__LINUX)
                nvt_enable_sram_shutdown(IPE2_SD);
#elif defined (__FREERTOS)
                nvt_enable_sram_shutdown(IPE2_SD);
#else
#endif

	}
#endif

}



ER ipe_eng_platform_flg_clear(IPE_ENG_HANDLE *p_eng, FLGPTN flg)
{
    ER return_value = E_OK;

#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
        return_value =  clr_flg(flg_id_ipe2, flg);
    }
	else
#endif
	{
    	return_value =  clr_flg(flg_id_ipe, flg);
	}

    return return_value;
}

ER ipe_eng_platform_flg_set(UINT32 eng_id, FLGPTN flg)
{
#if(0)
    if(eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
	    return iset_flg(flg_id_ipe2, flg);
    else
#endif
        return iset_flg(flg_id_ipe, flg);
}

ER ipe_eng_platform_flg_wait(IPE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	ER status;
#if(0)
    if(p_eng->eng_id == KDRV_VIDEOPROCS_IPE_ENGINE1)
    {
        status = wai_flg(p_flgptn, flg_id_ipe2, flg, TWF_CLR | TWF_ORW);
    }
    else
#endif
    {
        status = wai_flg(p_flgptn, flg_id_ipe, flg, TWF_CLR | TWF_ORW);
    }

	return status;
}


static irqreturn_t ipe_eng_platform_isr(int irq, void *param)
{
	ipe_eng_isr_hw_reg((IPE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID ipe_eng_platform_request_irq(IPE_ENG_HANDLE *p_eng)
{
    int rlt;

	rlt = request_irq(p_eng->irq_id, ipe_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
	if (rlt != 0)
		DBG_WRN("request_irq fail\r\n");
}

VOID ipe_eng_platform_release_irq(IPE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

VOID ipe_eng_platform_create_resource(VOID)
{
	OS_CONFIG_FLAG(flg_id_ipe);
	SEM_CREATE(semid_ipe, 1);

    OS_CONFIG_FLAG(flg_id_ipe2);
	SEM_CREATE(semid_ipe2, 1);
}

VOID ipe_eng_platform_release_resource(VOID)
{
	rel_flg(flg_id_ipe);
	SEM_DESTROY(semid_ipe);
    rel_flg(flg_id_ipe2);
	SEM_DESTROY(semid_ipe2);
}

void *IPE_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void IPE_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

UINT32 ipe_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}
