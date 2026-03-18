#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include "ime_eng_int_dbg.h"
#include "ime_eng.h"
#include "ime_eng_int_platform.h"
#include <plat/top.h>



//ULONG _ime_reg_io_base = 0;

static SEM_HANDLE semid_ime;
//static SEM_HANDLE semid_ime2;


static ID     flg_id_ime;
//static ID     flg_id_ime2;



ER ime_eng_platform_sem_wait(IME_ENG_HANDLE *p_eng)
{
	ER erReturn = E_OK;

#if defined __UITRON || defined __ECOS
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		erReturn = wai_sem(semid_ime);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  erReturn = wai_sem(semid_ime2);
	//}
#elif defined(__LINUX) || defined(__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		erReturn = SEM_WAIT(semid_ime);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  erReturn = SEM_WAIT(semid_ime2);
	//}
#else
#endif

	return erReturn;
}
//---------------------------------------------------------------

ER ime_eng_platform_sem_signal(IME_ENG_HANDLE *p_eng)
{
	ER erReturn = E_OK;

#if defined __UITRON || defined __ECOS
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		erReturn = sig_sem(semid_ime);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  erReturn = sig_sem(semid_ime2);
	//}
#elif defined(__LINUX) || defined(__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		SEM_SIGNAL(semid_ime);
		erReturn = E_OK;
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  SEM_SIGNAL(semid_ime2);
	//  erReturn = E_OK;
	//}
#else
#endif

	return erReturn;
}
//---------------------------------------------------------------




VOID ime_eng_platform_prepare_clk(IME_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;

    if (ime_eng_platform_get_chip_id() == CHIP_NS02302) {
    	if (IS_ERR(p_eng->pclk)) {
    		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
    	} else {    	
    		if(nvt_get_chip_ver()==CHIPVER_A){
    			clk_prepare(p_eng->pclk);
    		} else/* if (nvt_get_chip_ver()==CHIPVER_B)*/{
    			clk_prepare(p_eng->pclk);
    			parent_clk = clk_get(NULL, "pll17");
    			clk_prepare(parent_clk);
    			clk_put(parent_clk);			
    		}
    	}
	}
#elif defined (__FREERTOS)

	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_enableSystemReset(IME_RSTN);
		pll_disableSystemReset(IME_RSTN);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_enableSystemReset(IME2_RSTN);
		pll_disableSystemReset(IME2_RSTN);
	} else {

	}
#endif
#else
#endif
}

VOID ime_eng_platform_unprepare_clk(IME_ENG_HANDLE *p_eng)
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
#elif defined (__FREERTOS)
#else
#endif
}

VOID ime_eng_platform_enable_clk(IME_ENG_HANDLE *p_eng)
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
#elif defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_enableClock(IME_CLK);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_enableClock(IME2_CLK);
	} else {

	}
#endif
#else
#endif
}

VOID ime_eng_platform_disable_clk(IME_ENG_HANDLE *p_eng)
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
#elif defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		pll_disableClock(IME_CLK);
	} else {

	}

#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		pll_disableClock(IME2_CLK);
	} else {

	}
#endif
#else
#endif
}

INT32 ime_eng_platform_set_clk_rate(IME_ENG_HANDLE *p_eng)
{

#if defined (__LINUX)
	struct clk *parent_clk;

	UINT32 source_clk;
	//ULONG pll_clock_rate;
	//int ret;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	if (ime_eng_platform_get_chip_id() == CHIP_NS02302) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else {

			if(nvt_get_chip_ver()==CHIPVER_A){
				if(source_clk >IME_CHIP_A_PLL_13_MAX){
					DBG_ERR("IME(A): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IME_CHIP_A_PLL_13_MAX);
					return E_SYS;
				}
				parent_clk = clk_get(NULL, "pll13");
				clk_set_rate(parent_clk, source_clk*1000000);
			} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
				if(source_clk >IME_CHIP_B_PLL_17_MAX){
					DBG_ERR("IME(B): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IME_CHIP_B_PLL_17_MAX);
					return E_SYS;
				}
				//parent_clk = clk_get(NULL, "pll13");
				parent_clk = clk_get(NULL, "pll17");
				clk_set_rate(parent_clk, source_clk*1000000);
			}

			//pll_clock_rate = 450000000;
			//ret = clk_set_rate(parent_clk, pll_clock_rate);
		}
	} else {
		parent_clk = clk_get(NULL, "fix240m");
	}


	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);


#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_IME_240;
	UINT32 source_clk;

	// Turn on power
	// select clock
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		source_clk = p_eng->clock_rate;
		if (ime_eng_platform_get_chip_id() == CHIP_NS02302) {		
			if (source_clk == 240) {
				selected_clock = PLL_CLKSEL_IME_240;
			} else if (source_clk == 320) {
				selected_clock = PLL_CLKSEL_IME_320;
			} else {
				selected_clock = PLL_CLKSEL_IME_PLL13;

				//if (pll_getPLLEn(PLL_ID_13) == TRUE) {
				//	pll_set_pll_enable(PLL_ID_13, FALSE);
				//}

				//pll_set_pll_freq(PLL_ID_13, 450000000);

				//if (pll_getPLLEn(PLL_ID_13) == FALSE) {
				//	pll_setPLLEn(PLL_ID_13, TRUE);
				//}
			}

		} else {
			selected_clock = PLL_CLKSEL_IME_240;
		}

		pll_setClockRate(PLL_CLKSEL_IME, selected_clock);
	}
#if 0
	else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		source_clk = p_eng->clock_rate;
		if (ime_eng_platform_get_chip_id() == CHIP_NA51102) {
			if (source_clk == 240) {
				selected_clock = PLL_CLKSEL_IME2_240;
			} else if (source_clk == 320) {
				selected_clock = PLL_CLKSEL_IME2_320;
			} else if (source_clk == 480) {
				selected_clock = PLL_CLKSEL_IME2_480;
			} else {
				selected_clock = PLL_CLKSEL_IME2_PLL26;

				if (pll_getPLLEn(PLL_ID_26) == TRUE) {
					pll_set_pll_enable(PLL_ID_26, FALSE);
				}

				pll_set_pll_freq(PLL_ID_26, 400000000);

				if (pll_getPLLEn(PLL_ID_26) == FALSE) {
					pll_setPLLEn(PLL_ID_26, TRUE);
				}
			}

		} else {
			selected_clock = PLL_CLKSEL_IME2_240;
		}

		pll_setClockRate(PLL_CLKSEL_IME2, selected_clock);
	} else {

	}
#endif
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

VOID ime_eng_platform_disable_sram_shutdown(IME_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 && p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
#if defined (__LINUX)
		nvt_disable_sram_shutdown(IME_SD);
#elif defined (__FREERTOS)
		nvt_disable_sram_shutdown(IME_SD);
#else
#endif
	}

#if 0
	else if (p_eng->chip_id == KDRV_CHIP0 && p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
#if defined (__LINUX)
		nvt_disable_sram_shutdown(IME2_SD);
#elif defined (__FREERTOS)
		nvt_disable_sram_shutdown(IME2_SD);
#else
#endif
	}
#endif
}

VOID ime_eng_platform_enable_sram_shutdown(IME_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 && p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
#if defined (__LINUX)
		nvt_enable_sram_shutdown(IME_SD);
#elif defined (__FREERTOS)
		nvt_enable_sram_shutdown(IME_SD);
#else
#endif
	}
#if 0
	else if (p_eng->chip_id == KDRV_CHIP0 && p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
#if defined (__LINUX)
		nvt_enable_sram_shutdown(IME2_SD);
#elif defined (__FREERTOS)
		nvt_enable_sram_shutdown(IME2_SD);
#else
#endif
	}
#endif
}

ER ime_eng_platform_flg_clear(IME_ENG_HANDLE *p_eng, FLGPTN flg)
{
	ER erReturn = E_OK;

#if defined (__LINUX) || defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		erReturn = clr_flg(flg_id_ime, flg);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
	//  erReturn = clr_flg(flg_id_ime2, flg);
	//}
#endif

	return erReturn;
}

//---------------------------------------------------------------

ER ime_eng_platform_flg_wait(IME_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	ER status = E_OK;

	//DBG_ERR("ime_eng_platform_flg_wait id: %08x\r\n", p_eng->eng_id);

#if defined (__LINUX) || defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		status = wai_flg(p_flgptn, flg_id_ime, flg, TWF_CLR | TWF_ORW);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  status = wai_flg(p_flgptn, flg_id_ime2, flg, TWF_CLR | TWF_ORW);
	//}
#endif

	return status;
}


//---------------------------------------------------------------

ER ime_eng_platform_flg_set(IME_ENG_HANDLE *p_eng, FLGPTN flg)
{
	ER erReturn = E_OK;

#if defined (__LINUX) || defined (__FREERTOS)
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		erReturn = iset_flg(flg_id_ime, flg);
	}
	//else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
	//  erReturn = iset_flg(flg_id_ime2, flg);
	//}
#endif

	return erReturn;
}


//---------------------------------------------------------------

#if 0
static irqreturn_t ime_eng_platform_isr(int irq, void *param)
{
	ime_eng_isr_hw_reg((IME_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}
#endif

VOID ime_eng_platform_request_irq(IME_ENG_HANDLE *p_eng)
{
//#if (defined(_NVT_EMULATION_) == OFF)
	/* builtin ime already request irq,
	    register callback to builtin ime
	*/
	//request_irq(p_eng->irq_id, ime_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);

#if FPGA_BUILTIN_SUPPORT
	ime_builtin_reg_isr_cb(KDRV_DEV_ID(p_eng->chip_id, p_eng->eng_id, 0), ime_eng_isr_hw_reg);
#endif

//#endif
}

VOID ime_eng_platform_release_irq(IME_ENG_HANDLE *p_eng)
{
	/* builtin ime already request irq,
	    un-register callback to builtin ime
	*/
	//free_irq(p_eng->irq_id, p_eng);
#if FPGA_BUILTIN_SUPPORT
	ime_builtin_reg_isr_cb(KDRV_DEV_ID(p_eng->chip_id, p_eng->eng_id, 0), NULL);
#endif
}

VOID ime_eng_platform_create_resource(IME_ENG_HANDLE *p_eng)
{
#if 0
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		OS_CONFIG_FLAG(flg_id_ime);
		SEM_CREATE(semid_ime, 1);
	} else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		OS_CONFIG_FLAG(flg_id_ime2);
		SEM_CREATE(semid_ime2, 1);
	}
#endif

#if defined (__LINUX) || defined (__FREERTOS)
	OS_CONFIG_FLAG(flg_id_ime);
	SEM_CREATE(semid_ime, 1);

	//OS_CONFIG_FLAG(flg_id_ime2);
	//SEM_CREATE(semid_ime2, 1);
#endif
}


VOID ime_eng_platform_release_resource(IME_ENG_HANDLE *p_eng)
{
#if 0
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE0) {
		rel_flg(flg_id_ime);
		SEM_DESTROY(semid_ime);
	} else if (p_eng->eng_id == KDRV_VIDEOPROCS_IME_ENGINE1) {
		rel_flg(flg_id_ime2);
		SEM_DESTROY(semid_ime2);
	}
#endif

#if defined (__LINUX) || defined (__FREERTOS)
	rel_flg(flg_id_ime);
	SEM_DESTROY(semid_ime);

	//rel_flg(flg_id_ime2);
	//SEM_DESTROY(semid_ime2);
#endif
}


void *IME_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void IME_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

UINT32 ime_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

#if defined (__LINUX) || defined (__FREERTOS)
	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}
#endif

	return chip_id;
}

