/*
    DRE module driver

    NT96520 DRE module driver.

    @file       dre_eng_platform.c
    @ingroup    mIIPPDRE
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

//---------------------------------------------------------------
#include "dre_eng_int_platform.h"
#include "dre_eng_int_dbg.h"
#include "dre_eng.h"
static SEM_HANDLE semid_dre;
static ID flg_id_dre;

// debug level
unsigned int dre_eng_debug_level = NVT_DBG_WRN;

static irqreturn_t dre_eng_platform_isr(int irq, void *param)
{
	dre_eng_isr_hw_reg((DRE_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

ER dre_eng_platform_sem_wait(VOID)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(semid_dre);
#else
	return SEM_WAIT(semid_dre);
#endif
}

ER dre_eng_platform_sem_signal(VOID)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(semid_dre);
#else
	SEM_SIGNAL(semid_dre);
	return E_OK;
#endif
}

VOID dre_eng_platform_prepare_clk(DRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_prepare(p_eng->pclk);
	}
#else
	pll_enableSystemReset(DRE_RSTN);
	pll_disableSystemReset(DRE_RSTN);
#endif
}


VOID dre_eng_platform_unprepare_clk(DRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_unprepare(p_eng->pclk);
	}
#elif defined (__FREERTOS)
#endif
}

VOID dre_eng_platform_enable_clk(DRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_enable(p_eng->pclk);
	}
#else
	pll_enableClock(DRE_CLK);
#endif
}

VOID dre_eng_platform_disable_clk(DRE_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_disable(p_eng->pclk);
	}
#else
	pll_disableClock(DRE_CLK);
#endif
}

INT32 dre_eng_platform_set_clk_rate(DRE_ENG_HANDLE *p_eng)
{

#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return E_SYS;
	}

	source_clk = p_eng->clock_rate;
	//TODO
	if (1) {//dre_eng_platform_get_chip_id() == CHIP_NA51102) {
		if (source_clk == 240) {
			parent_clk = clk_get(NULL, "fix240m");
		} else if (source_clk == 320) {
			parent_clk = clk_get(NULL, "fix320m");
		} else if (source_clk == 480) {
			parent_clk = clk_get(NULL, "fix480m");
		} else {
			parent_clk = clk_get(NULL, "fix320m");
		}
	} else {
		parent_clk = clk_get(NULL, "fix320m");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )

	UINT32 selected_clock = 0;
	UINT32 source_clk;

	// Turn on power
	// select clock
	source_clk = p_eng->clock_rate;
    //TODO
	if (1) {//dre_eng_platform_get_chip_id() == CHIP_NA51102) {
		if (source_clk == 240) {
			selected_clock = PLL_CLKSEL_DRE_240;
		} else if (source_clk == 320) {
			selected_clock = PLL_CLKSEL_DRE_320;
		} else if (source_clk == 480) {
			selected_clock = PLL_CLKSEL_DRE_480;
		} else {
			selected_clock = PLL_CLKSEL_DRE_480;
		}
	} else {
		selected_clock = PLL_CLKSEL_DRE_480;
	}

	pll_setClockRate(PLL_CLKSEL_DRE, selected_clock);

#else
#endif

	return E_OK;
}

VOID dre_eng_platform_disable_sram_shutdown(DRE_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_DRE_ENGINE0) {

		nvt_disable_sram_shutdown(DRE_SD);
	}
}

VOID dre_eng_platform_enable_sram_shutdown(DRE_ENG_HANDLE *p_eng)
{
	if (p_eng->chip_id == KDRV_CHIP0 &&
		p_eng->eng_id == KDRV_VIDEOPROCS_DRE_ENGINE0) {

		nvt_enable_sram_shutdown(DRE_SD);
	}
}

VOID dre_eng_platform_create_resource(VOID)
{
	OS_CONFIG_FLAG(flg_id_dre);
	SEM_CREATE(semid_dre, 1);
}

VOID dre_eng_platform_release_resource(VOID)
{
	rel_flg(flg_id_dre);
	SEM_DESTROY(semid_dre);
}


VOID dre_eng_platform_request_irq(DRE_ENG_HANDLE *p_eng)
{
    int rlt;
	rlt = request_irq(p_eng->irq_id, dre_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
	if (rlt != 0)
		DBG_WRN("request_irq fail\r\n");
}

VOID dre_eng_platform_release_irq(DRE_ENG_HANDLE *p_eng)
{
	free_irq(p_eng->irq_id, p_eng);
}

ER dre_eng_platform_flg_clear(FLGPTN flg)
{
	return clr_flg(flg_id_dre, flg);
}

ER dre_eng_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg)
{
	ER status;

	status = wai_flg(p_flgptn, flg_id_dre, flg, TWF_CLR | TWF_ORW);

	return status;
}

ER dre_eng_platform_flg_set(FLGPTN flg)
{
	return iset_flg(flg_id_dre, flg);
}

void *dre_eng_malloc(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void dre_eng_free(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

UINT32 dre_eng_platform_get_chip_id(VOID)
{
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
}

UINT64 dre_eng_do_64b_div(UINT64 dividend, UINT64 divisor)
{
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	dividend = (dividend / divisor);
#else
	do_div(dividend, divisor);
#endif
	return dividend;
}
