/*
    LSU module driver

    NT98635 LSU module driver.

    @file       lsu_eng_platform.c
    @inglsup    mIIPPLSU
    @note       None

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

//---------------------------------------------------------------
#if defined (__LINUX)
#include <linux/interrupt.h>
#include <plat/nvt-sramctl.h>
#include <linux/of_device.h>
#include "efuse_protected_ns02401.h"
#else
#include "rtos_ns02401/pll.h"
#include "interrupt.h"
#include <kwrap/perf.h>
#include "rtos_ns02401/nvt-sramctl.h"
#include "pll_protected_ns02401.h"
#endif
#include "lsu_eng.h"
#include "lsu_eng_int_platform.h"
#include "lsu_eng_int_dbg.h"


UINT32 lsu_eng_platform_get_chip_id(VOID);


VOID lsu_eng_platform_disable_sram_shutdown(LSU_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
    if(p_eng->eng_id == LSU_ID){
		nvt_disable_sram_shutdown(LSU_SD);
	} else if(p_eng->eng_id == LSU2_ID){
		//nvt_disable_sram_shutdown(LSU2_SD); //TODO
	}
#elif defined (__FREERTOS)
	if(p_eng->eng_id == LSU_ID){
		nvt_disable_sram_shutdown(LSU_SD);
	} else if(p_eng->eng_id == LSU2_ID){
		nvt_disable_sram_shutdown(LSU2_SD);
	}
#elif
#else
#endif
}

VOID lsu_eng_platform_enable_sram_shutdown(LSU_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if(p_eng->eng_id == LSU_ID){
		nvt_enable_sram_shutdown(LSU_SD);
	} else if(p_eng->eng_id == LSU2_ID){
		//nvt_enable_sram_shutdown(LSU2_SD); //TODO
	}
#elif defined (__FREERTOS)
	if(p_eng->eng_id == LSU_ID){
		nvt_enable_sram_shutdown(LSU_SD);
	} else if(p_eng->eng_id == LSU2_ID){
		nvt_enable_sram_shutdown(LSU2_SD);
	}
#else
#endif
}

VOID lsu_eng_platform_prepare_clk(LSU_ENG_HANDLE *p_eng)
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
	if(p_eng->eng_id == LSU_ID){
		//TODO
		//pll_enableSystemReset(LSU_RSTN);
		//pll_disableSystemReset(LSU_RSTN);
	}
#else
#endif
}

VOID lsu_eng_platform_unprepare_clk(LSU_ENG_HANDLE *p_eng)
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

INT32 lsu_eng_platform_enable_clk(LSU_ENG_HANDLE *p_eng)
{
	INT32 ret = 0;
	
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		ret = clk_enable(p_eng->pclk);
	}
#endif
#elif  defined (__FREERTOS)
    if(p_eng->eng_id == LSU_ID){
		pll_enableClock(LSU_CLK);
    } else if(p_eng->eng_id == LSU2_ID){
		pll_enableClock(LSU2_CLK);
    }
#else
#endif

	return ret;
}

VOID lsu_eng_platform_disable_clk(LSU_ENG_HANDLE *p_eng)
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
    if(p_eng->eng_id == LSU_ID){
		pll_disableClock(LSU_CLK);
    } else if(p_eng->eng_id == LSU2_ID){
		pll_disableClock(LSU2_CLK);
    }
#else
#endif
}

INT32 lsu_eng_platform_set_clk_rate(LSU_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;
	struct device_node *node, *from = NULL;
	UINT32 current_rate = 0;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return -1;
	}
	if (p_eng->eng_id == LSU_ID) {
		node = of_find_node_by_name(from, "clk_ai3");
	} else {
		node = of_find_node_by_name(from, "clk_ai8");
	}
	if (node) {
		if (!of_property_read_u32(node, "current_rate", &current_rate)) {
			// get current rate from dtsi
			p_eng->clock_rate = current_rate/1000000;
		}
	}
	source_clk = p_eng->clock_rate;
	
	if (lsu_eng_platform_get_chip_id() == CHIP_NS02401) {
        if(source_clk >= 800) {
            parent_clk = clk_get(NULL, "pll29");
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d round to 800\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 600) {
            parent_clk = clk_get(NULL, "pll17");
            if(source_clk > 600) {
                DBG_ERR("%s: input frequency %d round to 600\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            parent_clk = clk_get(NULL, "fix480m");
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d round to 480\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 480\r\n", __func__, source_clk);
    		parent_clk = clk_get(NULL, "fix480m");
        }
	} else {
		parent_clk = clk_get(NULL, "pll29");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_480 : PLL_CLKSEL_LSU2_480;
	UINT32 source_clk;

	// select clock
	source_clk = p_eng->clock_rate;
	if (lsu_eng_platform_get_chip_id() == CHIP_NS02401) {
        if(source_clk >= 850) {
            selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_PLL28 : PLL_CLKSEL_LSU2_PLL28;
            if(source_clk > 850) {
                DBG_ERR("%s: input frequency %d round to 850\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 800) {
            selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_PLL29 : PLL_CLKSEL_LSU2_PLL29;
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d round to 800\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 600) {
            selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_PLL17 : PLL_CLKSEL_LSU2_PLL17;
            if(source_clk > 600) {
                DBG_ERR("%s: input frequency %d round to 600\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_480 : PLL_CLKSEL_LSU2_480;
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d round to 480\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 480\r\n", __func__, source_clk);
            selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_480 : PLL_CLKSEL_LSU2_480;
        }
	} else {
		selected_clock = p_eng->eng_id == LSU_ID ? PLL_CLKSEL_LSU_PLL28 : PLL_CLKSEL_LSU2_PLL29;
	}

	if(p_eng->eng_id == LSU_ID) {
		pll_setClockRate(PLL_CLKSEL_LSU, selected_clock);
	} else if(p_eng->eng_id == LSU2_ID) {
		pll_setClockRate(PLL_CLKSEL_LSU2, selected_clock);
	}
#endif

	return 0;
}

//---------------------------------------------------------------

ER lsu_eng_platform_flg_clear(LSU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return clr_flg(p_eng->flg_id_lsu, flg);
}

//---------------------------------------------------------------

ER lsu_eng_platform_flg_wait(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return wai_flg(p_flgptn, p_eng->flg_id_lsu, flg, TWF_CLR | TWF_ORW);
}

//---------------------------------------------------------------

ER lsu_eng_platform_flg_wait_timeout(LSU_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick)
{
	return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id_lsu, flg, TWF_ORW, timout_tick);
}

//---------------------------------------------------------------

ER lsu_eng_platform_flg_set(LSU_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return iset_flg(p_eng->flg_id_lsu, flg);
}

//---------------------------------------------------------------

static irqreturn_t lsu_eng_platform_isr(int irq, void *param)
{
	lsu_eng_isr_hw_reg((LSU_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID lsu_eng_platform_request_irq(LSU_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		int rlt;

		rlt = request_irq(p_eng->irq_id, lsu_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
		if (rlt != 0)
			DBG_WRN("request_irq fail\r\n");
	}
}

VOID lsu_eng_platform_release_irq(LSU_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		free_irq(p_eng->irq_id, p_eng);
	}
}

VOID lsu_eng_platform_create_resource(LSU_ENG_HANDLE *p_eng)
{
	OS_CONFIG_FLAG(p_eng->flg_id_lsu);
}

VOID lsu_eng_platform_release_resource(LSU_ENG_HANDLE *p_eng)
{
	rel_flg(p_eng->flg_id_lsu);
}


UINT32 lsu_eng_platform_get_chip_id(VOID)
{
#if 1
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
#else
	return CHIP_NS02401;
#endif
}

void *LSU_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void LSU_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

void LSU_ENG_SETREG(uintptr_t ofs, UINT32 value)
{
#if defined(__LINUX)
    iowrite32(value, (void*)(ofs));
#elif defined(__FREERTOS)
    OUTW((ofs), value);
#endif
}

UINT32 LSU_ENG_GETREG(uintptr_t ofs)
{
    UINT32 value;

#if defined(__LINUX)
	value = ioread32((void*)(ofs));
#elif defined(__FREERTOS)
	value = INW(ofs);
#endif
    return value;
}

INT32 lsu_eng_platform_dma_idle(LSU_ENG_HANDLE *p_eng)
{
#if defined(__FREERTOS)
    const VOS_TICK time_limit = 3000000; // 3 sec
    VOS_TICK tick_s, tick_e;
    VOS_TICK dur_tick = 0;  // us

    vos_perf_mark(&tick_s);
    while((lsu_eng_get_dma_idle(p_eng)==0) && dur_tick < time_limit) {
        vos_perf_mark(&tick_e);
        dur_tick = (tick_e - tick_s);
    }
    if(dur_tick > time_limit) {
        DBG_ERR("LSU wait DMA idle time out, %d > %d\n\r", dur_tick, time_limit);
        return -1;
    }
    return 0;
#else
    const UINT64 time_limit = 3000000000; // 3 sec
    UINT64 s_time, e_time;
    UINT64 dur_time = 0; // ns

    s_time = ktime_get_real_ns();
    while((lsu_eng_get_dma_idle(p_eng)==0) && dur_time < time_limit) {
        e_time = ktime_get_real_ns();
        dur_time = (e_time - s_time);
    }
    if(dur_time > time_limit) {
        DBG_ERR("LSU wait DMA idle time out\n\r");
        return -1;
    }
    return 0;
#endif
}

UINT32 lsu_eng_platform_get_clk_rate(LSU_ENG_HANDLE *p_eng)
{
	UINT32 clk_rate = 0;

#if defined (__LINUX)
	clk_rate = clk_get_rate(p_eng->pclk);
#elif defined (__FREERTOS )
	clk_rate = pll_getClockRate(PLL_CLKSEL_LSU);
#else
#endif

	return clk_rate;
}