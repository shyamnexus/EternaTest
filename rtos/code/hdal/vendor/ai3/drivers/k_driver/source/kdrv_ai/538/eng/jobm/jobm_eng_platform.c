/*
    JOBM module driver

    NT98538 JOBM module driver.

    @file       jobm_eng_platform.c
    @ingjobmp    mIIPPJOBM
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

//---------------------------------------------------------------
#if defined (__LINUX)
#include <linux/interrupt.h>
#include <plat/nvt-sramctl.h>
#else
#include "pll.h"
#include "interrupt.h"
//#include "rtos_ns02201/nvt-sramctl.h"
//#include "pll_protected_ns02201.h"
#include <kwrap/perf.h>
#endif
#include "jobm_eng.h"
#include "jobm_eng_int_platform.h"
#include "jobm_eng_int_dbg.h"


UINT32 jobm_eng_platform_get_chip_id(VOID);

/*
VOID jobm_eng_platform_disable_sram_shutdown(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
    if(p_eng->eng_id == JOBM_ID){
		nvt_disable_sram_shutdown(JOBM_SD);
	}
#elif defined (__FREERTOS)
	nvt_disable_sram_shutdown(JOBM_SD);
#else
#endif
}

VOID jobm_eng_platform_enable_sram_shutdown(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if(p_eng->eng_id == JOBM_ID){
		nvt_enable_sram_shutdown(JOBM_SD);
	}
#elif defined (__FREERTOS)
	if(p_eng->eng_id == JOBM_ID){
		nvt_enable_sram_shutdown(JOBM_SD);
	}
#else
#endif
}

VOID jobm_eng_platform_prepare_clk(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_prepare(p_eng->pclk);
	}
#endif
#elif defined (__FREERTOS)
	if(p_eng->eng_id == JOBM_ID){
		//TODO
		//pll_enableSystemReset(IVE_RSTN);
		//pll_disableSystemReset(IVE_RSTN);
	}
#else
#endif
}

VOID jobm_eng_platform_unprepare_clk(JOBM_ENG_HANDLE *p_eng)
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

VOID jobm_eng_platform_enable_clk(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_enable(p_eng->pclk);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == JOBM_ID){
		pll_enableClock(JOBM_CLK);
    }
#else
#endif
}

VOID jobm_eng_platform_disable_clk(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_disable(p_eng->pclk);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == JOBM_ID){
		pll_disableClock(JOBM_CLK);
    }
#else
#endif
}

INT32 jobm_eng_platform_set_clk_rate(JOBM_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return -1;
	}

	source_clk = p_eng->clock_rate;
	if (jobm_eng_platform_get_chip_id() == CHIP_NA51102) {
        if(source_clk >= 800) {
            parent_clk = clk_get(NULL, "pll29");
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d jobmnd to 800\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            parent_clk = clk_get(NULL, "fix480m");
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d jobmnd to 480\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 320) {
            parent_clk = clk_get(NULL, "fix320m");
            if(source_clk > 320) {
                DBG_ERR("%s: input frequency %d jobmnd to 320\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 240) {
            parent_clk = clk_get(NULL, "fix240m");
            if(source_clk > 240) {
                DBG_ERR("%s: input frequency %d jobmnd to 240\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 240\r\n", __func__, source_clk);
    		parent_clk = clk_get(NULL, "fix240m");
        }
	} else {
		parent_clk = clk_get(NULL, "pll29");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_IVE_480;
	UINT32 source_clk;

	// select clock
	source_clk = p_eng->clock_rate;
	if (jobm_eng_platform_get_chip_id() == CHIP_NA51102) {
        if(source_clk >= 800) {
            selected_clock = PLL_CLKSEL_JOBM_PLL29;
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d jobmnd to 800\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            selected_clock = PLL_CLKSEL_JOBM_480;
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d jobmnd to 480\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 320) {
            selected_clock = PLL_CLKSEL_JOBM_320;
            if(source_clk > 320) {
                DBG_ERR("%s: input frequency %d jobmnd to 320\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 240) {
            selected_clock = PLL_CLKSEL_JOBM_240;
            if(source_clk > 240) {
                DBG_ERR("%s: input frequency %d jobmnd to 240\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 240\r\n", __func__, source_clk);
            selected_clock = PLL_CLKSEL_JOBM_240;
        }
	} else {
		selected_clock = PLL_CLKSEL_JOBM_PLL29;
	}

	pll_setClockRate(PLL_CLKSEL_IVE, selected_clock);
#else
#endif

	return 0;
}
*/
//---------------------------------------------------------------

ER jobm_eng_platform_flg_clear(JOBM_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return clr_flg(p_eng->flg_id_jobm, flg);
}

//---------------------------------------------------------------

ER jobm_eng_platform_flg_wait(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return wai_flg(p_flgptn, p_eng->flg_id_jobm, flg, TWF_CLR | TWF_ORW);
}

//---------------------------------------------------------------

ER jobm_eng_platform_flg_wait_timeout(JOBM_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick)
{
	return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id_jobm, flg, TWF_CLR | TWF_ORW, timout_tick);
}

//---------------------------------------------------------------

ER jobm_eng_platform_flg_set(JOBM_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return iset_flg(p_eng->flg_id_jobm, flg);
}

//---------------------------------------------------------------

static irqreturn_t jobm_eng_platform_isr(int irq, void *param)
{
	jobm_eng_isr_hw_reg((JOBM_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID jobm_eng_platform_request_irq(JOBM_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		int rlt;

		rlt = request_irq(p_eng->irq_id, jobm_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
		if (rlt != 0)
			DBG_WRN("request_irq fail\r\n");
	}
}

VOID jobm_eng_platform_release_irq(JOBM_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		free_irq(p_eng->irq_id, p_eng);
	}
}

VOID jobm_eng_platform_create_resource(JOBM_ENG_HANDLE *p_eng)
{
	OS_CONFIG_FLAG(p_eng->flg_id_jobm);
}

VOID jobm_eng_platform_release_resource(JOBM_ENG_HANDLE *p_eng)
{
	rel_flg(p_eng->flg_id_jobm);
}


UINT32 jobm_eng_platform_get_chip_id(VOID)
{
#if 0
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
#endif

	return CHIP_NS02302;
}

void *JOBM_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void JOBM_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

void JOBM_ENG_SETREG(uintptr_t ofs, UINT32 value)
{
#if defined(__LINUX)
    iowrite32(value, (void*)(ofs));
#elif defined(__FREERTOS)
    OUTW((ofs), value);
#endif
}

UINT32 JOBM_ENG_GETREG(uintptr_t ofs)
{
    UINT32 value;

#if defined(__LINUX)
	value = ioread32((void*)(ofs));
#elif defined(__FREERTOS)
	value = INW(ofs);
#endif
    return value;
}

INT32 jobm_eng_platform_dma_idle(JOBM_ENG_HANDLE *p_eng)
{
#if defined(__FREERTOS)
    const VOS_TICK time_limit = 3000000; // 3 sec
    VOS_TICK tick_s, tick_e;
    VOS_TICK dur_tick = 0;  // us

    vos_perf_mark(&tick_s);
    while((jobm_eng_chk_dma_channel_idle_reg(p_eng)==0) && dur_tick < time_limit) {
        vos_perf_mark(&tick_e);
        dur_tick = (tick_e - tick_s);
    }
    if(dur_tick > time_limit) {
        DBG_ERR("JOBM wait DMA idle time out, %d > %d\n\r", dur_tick, time_limit);
        return -1;
    }
#else
    const UINT64 time_limit = 3000000000; // 3 sec
    UINT64 s_time, e_time;
    UINT64 dur_time = 0; // ns

    s_time = ktime_get_real_ns();
    while((jobm_eng_chk_dma_channel_idle_reg(p_eng)==0) && dur_time < time_limit) {
        e_time = ktime_get_real_ns();
        dur_time = (e_time - s_time);
    }
    if(dur_time > time_limit) {
        DBG_ERR("JOBM wait DMA idle time out\n\r");
        return -1;
    }    
#endif

	return 0;
}
