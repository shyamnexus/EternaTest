#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "ife_eng.h"
#include "ife_eng_int_platform.h"
#include "ife_eng_int_dbg.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"
#include <plat/top.h>

//uintptr_t _ife_reg_io_base = 0;

static ID     flg_id_ife;//, flg_id_ife2;
static SEM_HANDLE semid_ife, semid_ife2;


// debug level
unsigned int ife_eng_debug_level = NVT_DBG_WRN;

ER ife_eng_platform_sem_signal(IFE_ENG_HANDLE *p_eng)
{
#if defined __UITRON || defined __ECOS
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IFE_ENGINE0) {
		return sig_sem(semid_ife);
	} else {
		return sig_sem(semid_ife2);
	}
#else
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IFE_ENGINE0) {
		SEM_SIGNAL(semid_ife);
	}  else {
		SEM_SIGNAL(semid_ife2);
	}
	
	return E_OK;
#endif
}

ER ife_eng_platform_sem_wait(IFE_ENG_HANDLE *p_eng)
{
#if defined __UITRON || defined __ECOS
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IFE_ENGINE0) {
		return wai_sem(semid_ife);
	} else {
		return wai_sem(semid_ife2);
	}
#else
	if (p_eng->eng_id == KDRV_VIDEOPROCS_IFE_ENGINE0) {
		return SEM_WAIT(semid_ife);
	} else {
		return SEM_WAIT(semid_ife2);
	}
#endif
}

VOID ife_eng_platform_prepare_clk(IFE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
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
#elif defined (__FREERTOS)
	pll_enableSystemReset(IFE_RSTN);
	pll_disableSystemReset(IFE_RSTN);
#else
#endif
}

VOID ife_eng_platform_unprepare_clk(IFE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		if(nvt_get_chip_ver()==CHIPVER_A){
		clk_unprepare(p_eng->pclk);
    } else/* if (nvt_get_chip_ver()==CHIPVER_B)*/{
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

VOID ife_eng_platform_enable_clk(IFE_ENG_HANDLE *p_eng)
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
	pll_enableClock(IFE_CLK);
#else
#endif
}

VOID ife_eng_platform_disable_clk(IFE_ENG_HANDLE *p_eng)
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
	pll_disableClock(IFE_CLK);
#else
#endif
}

INT32 ife_eng_platform_set_clk_rate(IFE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	struct clk *ife_sram_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	if (ife_eng_platform_get_chip_id() == CHIP_NS02302) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else {
			if(nvt_get_chip_ver()==CHIPVER_A){
				if(source_clk >IFE_CHIP_A_PLL_13_MAX){
					DBG_ERR("IFE(A): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IFE_CHIP_A_PLL_13_MAX);
					return E_SYS;
				}			
			parent_clk = clk_get(NULL, "pll13");
    		clk_set_rate(parent_clk, source_clk*1000000);
    	} else /*if (nvt_get_chip_ver()==CHIPVER_B)*/{
				if(source_clk >IFE_CHIP_B_PLL_17_MAX){
					DBG_ERR("IFE(B): set clk rate fail...%d(Mhz) > %d(Mhz)\r\n", source_clk, IFE_CHIP_B_PLL_17_MAX);
					return E_SYS;
				}
				parent_clk = clk_get(NULL, "pll17");
				clk_set_rate(parent_clk, source_clk*1000000);
			}
		}
	} else {
		parent_clk = clk_get(NULL, "fix240m");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

	/* sram_sel */
	parent_clk = clk_get(NULL, "ife_sram_ife");
	if (IS_ERR(parent_clk)) {
		DBG_ERR("getting parent_clk error\n");
	}

	/* Get ife sram control node*/
	ife_sram_clk = clk_get(NULL, "ife_shared_sram");
	if (IS_ERR(ife_sram_clk)) {
		DBG_ERR("getting ife_sram_clk error\n");
	}

	/* Set clk select*/
	clk_set_parent(ife_sram_clk, parent_clk);

	clk_put(ife_sram_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 source_clk, select_clk;

	source_clk = p_eng->clock_rate;


		if (ife_eng_platform_get_chip_id() == CHIP_NS02302) {
			if (source_clk == 240) {
				select_clk = PLL_CLKSEL_IFE_240;
			} else if (source_clk == 320) {
				select_clk = PLL_CLKSEL_IFE_320;
			}  else {
				select_clk = PLL_CLKSEL_IFE_PLL13;
				if (pll_getPLLEn(PLL_ID_13) == FALSE) {
					pll_setPLLEn(PLL_ID_13, TRUE);
				}
			}
		} else {
			select_clk = PLL_CLKSEL_IFE_240;
		}

		pll_setClockRate(PLL_CLKSEL_IFE, select_clk);

	/* set sram sel for ife */
	pll_setClockRate(PLL_CLKSEL_IFE_SRAM, PLL_CLKSEL_IFE_SRAM_IFE);
#endif

	return E_OK;
}

VOID ife_eng_platform_disable_sram_shutdown(IFE_ENG_HANDLE *p_eng)
{
	
#if defined (__LINUX)
		nvt_disable_sram_shutdown(IFE_SD);
#elif defined (__FREERTOS )
		nvt_disable_sram_shutdown(IFE_SD);
#endif
}


VOID ife_eng_platform_enable_sram_shutdown(IFE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
		nvt_enable_sram_shutdown(IFE_SD);
#elif defined (__FREERTOS )
		nvt_enable_sram_shutdown(IFE_SD);
#endif

}

ER ife_eng_platform_flg_clear(UINT32 eng_id, FLGPTN flg)
{
	return clr_flg(flg_id_ife, flg);
}
//---------------------------------------------------------------

ER ife_eng_platform_flg_wait(UINT32 eng_id, PFLGPTN p_flgptn, FLGPTN flg)
{
	ER status;

	status = wai_flg(p_flgptn, flg_id_ife, flg, TWF_CLR | TWF_ORW);

	return status;
}
//---------------------------------------------------------------

ER ife_eng_platform_flg_set(UINT32 eng_id, FLGPTN flg)
{
	return iset_flg(flg_id_ife, flg);
}
//---------------------------------------------------------------

static irqreturn_t ife_eng_platform_isr(int irq, void *param)
{
	ife_eng_isr_hw_reg((IFE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID ife_eng_platform_request_irq(IFE_ENG_HANDLE *p_eng)
{
	int ret;
	ret = request_irq(p_eng->irq_id, ife_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);

	if (ret != 0) {
		DBG_WRN("ife request_irq fail\n");
	}
}

VOID ife_eng_platform_release_irq(IFE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

VOID ife_eng_platform_create_resource(VOID)
{
	OS_CONFIG_FLAG(flg_id_ife);
	SEM_CREATE(semid_ife, 1);
}

VOID ife_eng_platform_release_resource(VOID)
{
	rel_flg(flg_id_ife);
	SEM_DESTROY(semid_ife);
}

void *IFE_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void IFE_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

UINT32 ife_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}
