/*
    NUE2 module drnue2r

    NT96538 NUE2 module drnue2r.

    @file       nue2_eng_platform.c
    @ingnue2p    mIIPPNUE2
    @note       None

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
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
#include "nue2_eng.h"
#include "nue2_eng_int_platform.h"
#include "nue2_eng_int_dbg.h"
#include <plat/top.h>

UINT32 nue2_eng_platform_get_chip_id(VOID);


VOID nue2_eng_platform_disable_sram_shutdown(NUE2_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
    if(p_eng->eng_id == NUE2_ID){
		nvt_disable_sram_shutdown(NUE2_SD);
	}
#elif defined (__FREERTOS)
	nvt_disable_sram_shutdown(NUE2_SD);
#else
#endif
}

VOID nue2_eng_platform_enable_sram_shutdown(NUE2_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	if(p_eng->eng_id == NUE2_ID){
		nvt_enable_sram_shutdown(NUE2_SD);
	}
#elif defined (__FREERTOS)
	if(p_eng->eng_id == NUE2_ID){
		nvt_enable_sram_shutdown(NUE2_SD);
	}
#else
#endif
}

VOID nue2_eng_platform_prepare_clk(NUE2_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
/*
	if (nvt_get_chip_id() == CHIP_NA51055) {
		DBG_ERR("VPE: do not support\r\n");
	} else {
	struct clk *parent_clk;
	parent_clk = clk_get(NULL, "pll17");
	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);
	}
	*/
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_prepare(p_eng->pclk);
	}
#endif
#elif defined (__FREERTOS)
	if(p_eng->eng_id == NUE2_ID){
		//TODO
		//pll_enableSystemReset(IVE_RSTN);
		//pll_disableSystemReset(IVE_RSTN);
	}
#else
#endif
}

VOID nue2_eng_platform_unprepare_clk(NUE2_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
#if 1
	if (IS_ERR(p_eng->pclk)) {
		DBG_WRN("get clk fail 0x%p\r\n", p_eng->pclk);
	} else {
		clk_unprepare(p_eng->pclk);
	}
#endif
#endif
}

INT32 nue2_eng_platform_enable_clk(NUE2_ENG_HANDLE *p_eng)
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
    if(p_eng->eng_id == NUE2_ID){
		pll_enableClock(NUE2_CLK);
    }
#else
#endif

	return ret;
}

VOID nue2_eng_platform_disable_clk(NUE2_ENG_HANDLE *p_eng)
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
    if(p_eng->eng_id == NUE2_ID){
		pll_disableClock(NUE2_CLK);
    }
#else
#endif
}

INT32 nue2_eng_platform_set_clk_rate(NUE2_ENG_HANDLE *p_eng)
{
	const UINT32 NUE2_MAX_CLK_FREQ = 800;

#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;
	BOOL ret = 0;
	struct device_node *node, *from = NULL;
	UINT32 current_rate = 0;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return -1;
	}

	node = of_find_node_by_name(from, "clk_ai1");
	if (node) {
		if (!of_property_read_u32(node, "current_rate", &current_rate)) {
			// get current rate from dtsi
			p_eng->clock_rate = current_rate/1000000;
		}
	}
	source_clk = p_eng->clock_rate;
	if (nvt_get_chip_id() == CHIP_NS02302) {
	#if 0
	ret = efuse_check_available_extend(EFUSE_ABILITY_CONV_PLL_FREQ, source_clk*1000000);
	#endif
    	if(ret != TRUE) {
        	if(source_clk > 540) {
            		source_clk = 540;
        	}
    	}
	}
	
	if(source_clk >= NUE2_MAX_CLK_FREQ) {
        parent_clk = clk_get(NULL, "pll29");
        if(source_clk > NUE2_MAX_CLK_FREQ) {
            DBG_ERR("%s: input frequency %d ppund to %d\r\n", __func__, source_clk, NUE2_MAX_CLK_FREQ);
        }
        clk_set_rate(parent_clk, 800000000);
    } else if(source_clk >= 600) {
        parent_clk = clk_get(NULL, "pll17");
        if(source_clk > 600) {
            DBG_ERR("%s: input frequency %d ppund to 800\r\n", __func__, source_clk);
        }
        clk_set_rate(parent_clk, 600000000);
    } else if(source_clk >= 480) {
        parent_clk = clk_get(NULL, "fix480m");
        if(source_clk > 480) {
            DBG_ERR("%s: input frequency %d ppund to 480\r\n", __func__, source_clk);
        }
    } else {
        DBG_ERR("%s: illegal input frequency %u, set as 480\r\n", __func__, source_clk);
		parent_clk = clk_get(NULL, "fix480m");
    }

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_NUE2_PLL29;
	UINT32 source_clk;

	// select clock
	source_clk = p_eng->clock_rate;

    if(source_clk >= NUE2_MAX_CLK_FREQ) {
        selected_clock = PLL_CLKSEL_NUE2_PLL29;
        if(source_clk > NUE2_MAX_CLK_FREQ) {
            DBG_ERR("%s: input frequency %d ppund to %d\r\n", __func__, source_clk, NUE2_MAX_CLK_FREQ);
        }
    } else if(source_clk >= 600) {
        selected_clock = PLL_CLKSEL_NUE2_PLL17;
        if(source_clk > 600) {
            DBG_ERR("%s: input frequency %d ppund to %d\r\n", __func__, source_clk, source_clk);
        }
    } else if(source_clk >= 480) {
        selected_clock = PLL_CLKSEL_NUE2_480;
        if(source_clk > 480) {
            DBG_ERR("%s: input frequency %d ppund to 480\r\n", __func__, source_clk);
        }
    } else {
        DBG_ERR("%s: illegal input frequency %u, set as PLL29\r\n", __func__, source_clk);
        selected_clock = PLL_CLKSEL_NUE2_PLL29;
    }

	pll_setClockRate(PLL_CLKSEL_NUE2, selected_clock);
#else
#endif

	return 0;
}

//---------------------------------------------------------------

ER nue2_eng_platform_flg_clear(NUE2_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return clr_flg(p_eng->flg_id_nue2, flg);
}

//---------------------------------------------------------------

ER nue2_eng_platform_flg_wait(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return wai_flg(p_flgptn, p_eng->flg_id_nue2, flg, TWF_CLR | TWF_ORW);
}

//---------------------------------------------------------------

ER nue2_eng_platform_flg_wait_timeout(NUE2_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick)
{
	return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id_nue2, flg, TWF_ORW, timout_tick);
}

//---------------------------------------------------------------

ER nue2_eng_platform_flg_set(NUE2_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return iset_flg(p_eng->flg_id_nue2, flg);
}

//---------------------------------------------------------------

static irqreturn_t nue2_eng_platform_isr(int irq, void *param)
{
	nue2_eng_isr_hw_reg((NUE2_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID nue2_eng_platform_request_irq(NUE2_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		int rlt;

		rlt = request_irq(p_eng->irq_id, nue2_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
		if (rlt != 0)
			DBG_WRN("request_irq fail\r\n");
	}
}

VOID nue2_eng_platform_release_irq(NUE2_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		free_irq(p_eng->irq_id, p_eng);
	}
}

VOID nue2_eng_platform_create_resource(NUE2_ENG_HANDLE *p_eng)
{
	OS_CONFIG_FLAG(p_eng->flg_id_nue2);
}

VOID nue2_eng_platform_release_resource(NUE2_ENG_HANDLE *p_eng)
{
	rel_flg(p_eng->flg_id_nue2);
}


UINT32 nue2_eng_platform_get_chip_id(VOID)
{
#if 0
	static UINT32 chip_id = 0;

	if (chip_id == 0) {
		chip_id = nvt_get_chip_id();
	}

	return chip_id;
#endif

	return CHIP_NS02401;
}

void *NUE2_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void NUE2_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

void NUE2_ENG_SETREG(uintptr_t ofs, UINT32 value)
{
#if defined(__LINUX)
    iowrite32(value, (void*)(ofs));
#elif defined(__FREERTOS)
    OUTW((ofs), value);
#endif
}

UINT32 NUE2_ENG_GETREG(uintptr_t ofs)
{
    UINT32 value;

#if defined(__LINUX)
	value = ioread32((void*)(ofs));
#elif defined(__FREERTOS)
	value = INW(ofs);
#endif
    return value;
}

INT32 nue2_eng_platform_dma_idle(NUE2_ENG_HANDLE *p_eng)
{
#if defined(__FREERTOS)
    const VOS_TICK time_limit = 3000000; // 3 sec
    VOS_TICK tick_s, tick_e;
    VOS_TICK dur_tick = 0;  // us

    vos_perf_mark(&tick_s);
    while((nue2_eng_get_dma_idle(p_eng)==0) && dur_tick < time_limit) {
        vos_perf_mark(&tick_e);
        dur_tick = (tick_e - tick_s);
    }
    if(dur_tick > time_limit) {
        DBG_ERR("NUE2 wait DMA idle time out, %d > %d\n\r", dur_tick, time_limit);
        return -1;
    }
    return 0;
#else
    const UINT64 time_limit = 3000000000; // 3 sec
    UINT64 s_time, e_time;
    UINT64 dur_time = 0; // ns

    s_time = ktime_get_real_ns();
    while((nue2_eng_get_dma_idle(p_eng)==0) && dur_time < time_limit) {
        e_time = ktime_get_real_ns();
        dur_time = (e_time - s_time);
    }
    if(dur_time > time_limit) {
        DBG_ERR("NUE2 wait DMA idle time out\n\r");
        return -1;
    }
    return 0;
#endif
}

UINT32 nue2_eng_platform_get_clk_rate(NUE2_ENG_HANDLE *p_eng)
{
    UINT32 clk_rate = 0;

#if defined(__LINUX)
    clk_rate = clk_get_rate(p_eng->pclk);
#elif defined(__FREERTOS)
    clk_rate = pll_getClockRate(PLL_CLKSEL_NUE2);
#endif

    return clk_rate;
}