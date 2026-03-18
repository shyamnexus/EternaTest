#if defined (__LINUX)
#include <linux/interrupt.h>
#else
#include "interrupt.h"
#endif

#include "pre_eng.h"
#include "pre_eng_int_platform.h"
#include "pre_eng_int_dbg.h"
#include "kdrv_builtin/kdrv_ipp_builtin.h"

//uintptr_t _pre_reg_io_base = 0;

static ID     flg_id_pre;//, flg_id_pre2;
static SEM_HANDLE semid_pre;


// debug level
unsigned int pre_eng_debug_level = NVT_DBG_WRN;

ER pre_eng_platform_sem_signal(PRE_ENG_HANDLE *p_eng)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(semid_pre);
#else
	SEM_SIGNAL(semid_pre);

	return E_OK;
#endif
}

ER pre_eng_platform_sem_wait(PRE_ENG_HANDLE *p_eng)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(semid_pre);

#else
	return SEM_WAIT(semid_pre);

#endif
}

VOID pre_eng_platform_prepare_clk(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_prepare(p_eng->pclk);
	}
#elif defined (__FREERTOS)
	pll_enableSystemReset(PRE_RSTN);
	pll_disableSystemReset(PRE_RSTN);
#else
#endif
}

VOID pre_eng_platform_unprepare_clk(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_unprepare(p_eng->pclk);
	}
#elif defined (__FREERTOS)
#else
#endif
}

VOID pre_eng_platform_enable_clk(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_enable(p_eng->pclk);
	}
#elif defined (__FREERTOS)
	pll_enableClock(PRE_CLK);
#else
#endif
}

VOID pre_eng_platform_disable_clk(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_disable(p_eng->pclk);
	}
#elif defined (__FREERTOS)
	pll_disableClock(PRE_CLK);
#else
#endif
}

INT32 pre_eng_platform_set_clk_rate(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	struct clk *pre_sram_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	if (pre_eng_platform_get_chip_id() == CHIP_NS02402) {
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

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

	/* sram_sel */
	parent_clk = clk_get(NULL, "pre_sram_pre");
	if (IS_ERR(parent_clk)) {
		DBG_ERR("getting parent_clk error\n");
	}

	/* Get pre sram control node*/
	pre_sram_clk = clk_get(NULL, "pre_shared_sram");
	if (IS_ERR(pre_sram_clk)) {
		DBG_ERR("getting pre_sram_clk error\n");
	}

	/* Set clk select*/
	clk_set_parent(pre_sram_clk, parent_clk);

	clk_put(pre_sram_clk);
	clk_put(parent_clk);


#elif defined (__FREERTOS )
	UINT32 source_clk, select_clk;

	source_clk = p_eng->clock_rate;

	if (pre_eng_platform_get_chip_id() == CHIP_NS02402) {
		if (source_clk == 240) {
			select_clk = PLL_CLKSEL_PRE_IFE_240;
		} else if (source_clk == 320) {
			select_clk = PLL_CLKSEL_PRE_IFE_320;
		} else if (source_clk > 480) {
			select_clk = PLL_CLKSEL_PRE_IFE_PLL17;
			if (pll_getPLLEn(PLL_ID_17) == FALSE) {
				pll_setPLLEn(PLL_ID_17, TRUE);
			}
		} else {
			select_clk = PLL_CLKSEL_PRE_IFE_PLL13;
			if (pll_getPLLEn(PLL_ID_13) == FALSE) {
				pll_setPLLEn(PLL_ID_13, TRUE);
			}
		}
		pll_setClockRate(PLL_CLKSEL_PRE_IFE, select_clk);
	} else {
		select_clk = PLL_CLKSEL_PRE_IFE_240;
	}

	pll_setClockRate(PLL_CLKSEL_PRE_IFE, select_clk);

	/* set sram sel for pre */
	pll_setClockRate(PLL_CLKSEL_PRE_SRAM, PLL_CLKSEL_PRE_SRAM_PRE);

#endif

	return E_OK;
}

VOID pre_eng_platform_disable_sram_shutdown(PRE_ENG_HANDLE *p_eng)
{

#if defined (__LINUX)
		nvt_disable_sram_shutdown(PRE_SD);
#elif defined (__FREERTOS )
		nvt_disable_sram_shutdown(PRE_SD);
#endif
}


VOID pre_eng_platform_enable_sram_shutdown(PRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
		nvt_enable_sram_shutdown(PRE_SD);
#elif defined (__FREERTOS )
		nvt_enable_sram_shutdown(PRE_SD);
#endif

}

ER pre_eng_platform_flg_clear(UINT32 eng_id, FLGPTN flg)
{
	return clr_flg(flg_id_pre, flg);
}
//---------------------------------------------------------------

ER pre_eng_platform_flg_wait(UINT32 eng_id, PFLGPTN p_flgptn, FLGPTN flg)
{
	ER status;

	status = wai_flg(p_flgptn, flg_id_pre, flg, TWF_CLR | TWF_ORW);

	return status;
}
//---------------------------------------------------------------

ER pre_eng_platform_flg_set(UINT32 eng_id, FLGPTN flg)
{
	return iset_flg(flg_id_pre, flg);
}
//---------------------------------------------------------------

static irqreturn_t pre_eng_platform_isr(int irq, void *param)
{
	pre_eng_isr_hw_reg((PRE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID pre_eng_platform_request_irq(PRE_ENG_HANDLE *p_eng)
{
	int ret;
	ret = request_irq(p_eng->irq_id, pre_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);

	if (ret != 0) {
		DBG_WRN("pre request_irq fail\n");
	}
}

VOID pre_eng_platform_release_irq(PRE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

VOID pre_eng_platform_create_resource(VOID)
{
	OS_CONFIG_FLAG(flg_id_pre);
	SEM_CREATE(semid_pre, 1);
}

VOID pre_eng_platform_release_resource(VOID)
{
	rel_flg(flg_id_pre);
	SEM_DESTROY(semid_pre);
}

void *PRE_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void PRE_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

UINT32 pre_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}
