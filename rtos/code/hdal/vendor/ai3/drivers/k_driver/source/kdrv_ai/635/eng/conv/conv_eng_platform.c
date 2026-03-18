/*
    CONV module drconvr

    NT96690 CONV module drconvr.

    @file       conv_eng_platform.c
    @ingconvp    mIIPPCONV
    @note       None

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

//---------------------------------------------------------------
#if defined (__LINUX)
#include <linux/interrupt.h>
#include <plat/nvt-sramctl.h>
#include <linux/of_device.h>
#include "efuse_protected_ns02401.h"
#else
#include "pll.h"
#include "interrupt.h"
#include <kwrap/perf.h>
#include "rtos_ns02401/nvt-sramctl.h"
#include "pll_protected_ns02401.h"
#endif
#include "conv_eng.h"
#include "conv_eng_int_platform.h"
#include "conv_eng_int_dbg.h"
#include <plat/top.h>
UINT32 conv_eng_platform_get_chip_id(VOID);

VOID conv_eng_platform_disable_sram_shutdown(CONV_ENG_HANDLE *p_eng)
{
    if(p_eng->eng_id == CONV_ID){
		nvt_disable_sram_shutdown(CONV_SD);
	}
}

VOID conv_eng_platform_enable_sram_shutdown(CONV_ENG_HANDLE *p_eng)
{
	if(p_eng->eng_id == CONV_ID){
		nvt_enable_sram_shutdown(CONV_SD);
	}
}

VOID conv_eng_platform_prepare_clk(CONV_ENG_HANDLE *p_eng)
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
	if(p_eng->eng_id == CONV_ID){
		//TODO
		//pll_enableSystemReset(IVE_RSTN);
		//pll_disableSystemReset(IVE_RSTN);
	}
#else
#endif
}

VOID conv_eng_platform_unprepare_clk(CONV_ENG_HANDLE *p_eng)
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

INT32 conv_eng_platform_enable_clk(CONV_ENG_HANDLE *p_eng)
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
    if(p_eng->eng_id == CONV_ID){
		pll_enableClock(CONV_CLK);
    }
#else
#endif

	return ret;
}

VOID conv_eng_platform_disable_clk(CONV_ENG_HANDLE *p_eng)
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
	if(p_eng->eng_id == CONV_ID){
		pll_disableClock(CONV_CLK);
    }
#else
#endif
}

UINT32 conv_eng_platform_get_clk_rate(CONV_ENG_HANDLE *p_eng)
{
	UINT32 clk_rate = 0;

#if defined (__LINUX)
	clk_rate = clk_get_rate(p_eng->pclk);
#elif defined (__FREERTOS )
	pll_getClockRate(PLL_CLKSEL_CONV);
#else
#endif

	return clk_rate;
}

INT32 conv_eng_platform_set_clk_rate(CONV_ENG_HANDLE *p_eng)
{
#if defined (__LINUX)
	struct clk *parent_clk;
	UINT32 source_clk;	
	//BOOL ret = 0;
	struct device_node *node, *from = NULL;
	UINT32 current_rate = 0;

	if (IS_ERR(p_eng->pclk)) {
		DBG_ERR("get clk fail...0x%p\r\n", p_eng->pclk);
		return -1;
	}

	node = of_find_node_by_name(from, "clk_ai0");
	if (node) {
		if (!of_property_read_u32(node, "current_rate", &current_rate)) {
			// get current rate from dtsi
			p_eng->clock_rate = current_rate/1000000;
		}
	}
	source_clk = p_eng->clock_rate;
	/*
	if (nvt_get_chip_id() == CHIP_NS02401) {
	ret = efuse_check_available_extend(EFUSE_ABILITY_CONV_PLL_FREQ, source_clk*1000000);
    if(ret != TRUE) {
        if(source_clk > 540) {
            source_clk = 540;
        }
    }
	}
	*/
	if (conv_eng_platform_get_chip_id() == CHIP_NS02401) {
        if(source_clk >= 850) {
            parent_clk = clk_get(NULL, "pll28");
            if(source_clk > 850) {
                DBG_ERR("%s: input frequency %d convnd to 850\r\n", __func__, source_clk);
            }
			clk_set_rate(parent_clk, 850000000);
        } else if(source_clk >= 800) {
            parent_clk = clk_get(NULL, "pll29");
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d convnd to 800\r\n", __func__, source_clk);
            }
			clk_set_rate(parent_clk, 800000000);
        } else if(source_clk >= 600) {
            parent_clk = clk_get(NULL, "pll17");
            if(source_clk > 600) {
                DBG_ERR("%s: input frequency %d convnd to 600\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            parent_clk = clk_get(NULL, "fix480m");
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d convnd to 480\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 850\r\n", __func__, source_clk);
    		parent_clk = clk_get(NULL, "pll28");
        }
	} else {
		parent_clk = clk_get(NULL, "pll28");
	}

	clk_set_parent(p_eng->pclk, parent_clk);
	clk_put(parent_clk);

#elif defined (__FREERTOS )
	UINT32 selected_clock = PLL_CLKSEL_CONV_PLL28;
	UINT32 source_clk;

	// select clock
	source_clk = p_eng->clock_rate;
	if (conv_eng_platform_get_chip_id() == CHIP_NS02401) {
        if(source_clk >= 850) {
            selected_clock = PLL_CLKSEL_CONV_PLL28;
            if(source_clk > 850) {
                DBG_ERR("%s: input frequency %d convnd to 850\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 800) {
            selected_clock = PLL_CLKSEL_CONV_PLL29;
            if(source_clk > 800) {
                DBG_ERR("%s: input frequency %d convnd to 800\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 600) {
            selected_clock = PLL_CLKSEL_CONV_PLL17;
            if(source_clk > 600) {
                DBG_ERR("%s: input frequency %d convnd to 600\r\n", __func__, source_clk);
            }
        } else if(source_clk >= 480) {
            selected_clock = PLL_CLKSEL_CONV_480;
            if(source_clk > 480) {
                DBG_ERR("%s: input frequency %d convnd to 4800\r\n", __func__, source_clk);
            }
        } else {
            DBG_ERR("%s: illegal input frequency %u, set as 850\r\n", __func__, source_clk);
            selected_clock = PLL_CLKSEL_CONV_PLL28;
        }
	} else {
		selected_clock = PLL_CLKSEL_CONV_PLL28;
	}

	if(p_eng->eng_id == CONV_ID){
		pll_setClockRate(PLL_CLKSEL_CONV, selected_clock);
	}	
#else
#endif

	return 0;
}

//---------------------------------------------------------------

ER conv_eng_platform_flg_clear(CONV_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return clr_flg(p_eng->flg_id_conv, flg);
}

//---------------------------------------------------------------

ER conv_eng_platform_flg_wait(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
	return wai_flg(p_flgptn, p_eng->flg_id_conv, flg, TWF_CLR | TWF_ORW);
}

//---------------------------------------------------------------

ER conv_eng_platform_flg_wait_timeout(CONV_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timout_tick)
{
	return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id_conv, flg, TWF_ORW, timout_tick);
}

//---------------------------------------------------------------

ER conv_eng_platform_flg_set(CONV_ENG_HANDLE *p_eng, FLGPTN flg)
{
	return iset_flg(p_eng->flg_id_conv, flg);
}

//---------------------------------------------------------------

static irqreturn_t conv_eng_platform_isr(int irq, void *param)
{
	conv_eng_isr_hw_reg((CONV_ENG_HANDLE *)param);
	return IRQ_HANDLED;
}

VOID conv_eng_platform_request_irq(CONV_ENG_HANDLE *p_eng)
{
    if(p_eng->irq_id!=0){
		int rlt;

		rlt = request_irq(p_eng->irq_id, conv_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
		if (rlt != 0)
			DBG_WRN("request_irq fail\r\n");
	}
}

VOID conv_eng_platform_release_irq(CONV_ENG_HANDLE *p_eng)
{
	if(p_eng->irq_id!=0){
		free_irq(p_eng->irq_id, p_eng);
	}
}

VOID conv_eng_platform_create_resource(CONV_ENG_HANDLE *p_eng)
{
	OS_CONFIG_FLAG(p_eng->flg_id_conv);
}

VOID conv_eng_platform_release_resource(CONV_ENG_HANDLE *p_eng)
{
	rel_flg(p_eng->flg_id_conv);
}


UINT32 conv_eng_platform_get_chip_id(VOID)
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

void *CONV_ENG_MALLOC(UINT32 size)
{
	void *p_buf = NULL;

#if defined(__LINUX)
	p_buf = kmalloc(size, GFP_KERNEL);
#elif defined(__FREERTOS)
	p_buf = malloc(size);
#endif

	return p_buf;
}

void CONV_ENG_FREE(void *ptr)
{
#if defined(__LINUX)
	kfree(ptr);
#elif defined(__FREERTOS)
	free(ptr);
#endif
}

void CONV_ENG_SETREG(uintptr_t ofs, UINT32 value)
{
#if defined(__LINUX)
    iowrite32(value, (void*)(ofs));
#elif defined(__FREERTOS)
    OUTW((ofs), value);
#endif
}

UINT32 CONV_ENG_GETREG(uintptr_t ofs)
{
    UINT32 value;

#if defined(__LINUX)
	value = ioread32((void*)(ofs));
#elif defined(__FREERTOS)
	value = INW(ofs);
#endif
    return value;
}

INT32 conv_eng_platform_dma_idle(CONV_ENG_HANDLE *p_eng)
{
#if defined(__FREERTOS)
    const VOS_TICK time_limit = 3000000; // 3 sec
    VOS_TICK tick_s, tick_e;
    VOS_TICK dur_tick = 0;  // us

    vos_perf_mark(&tick_s);
    while((conv_eng_get_dma_idle(p_eng)==0) && dur_tick < time_limit) {
        vos_perf_mark(&tick_e);
        dur_tick = (tick_e - tick_s);
    }
    if(dur_tick > time_limit) {
        DBG_ERR("CONV wait DMA idle time out, %d > %d\n\r", dur_tick, time_limit);
        return -1;
    }
    return 0;
#else
    const UINT64 time_limit = 3000000000; // 3 sec
    UINT64 s_time, e_time;
    UINT64 dur_time = 0; // ns

    s_time = ktime_get_real_ns();
    while((conv_eng_get_dma_idle(p_eng)==0) && dur_time < time_limit) {
        e_time = ktime_get_real_ns();
        dur_time = (e_time - s_time);
    }
    if(dur_time > time_limit) {
        DBG_ERR("CONV wait DMA idle time out\n\r");
        return -1;
    }
    return 0;
#endif
}
