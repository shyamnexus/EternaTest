/*
    SIE single source platform driver

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#if defined (__LINUX)
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <plat/nvt-sramctl.h>
#else
#include "pll_protected.h"
#include "io_address.h"
#include "interrupt.h"
#include "nvt-sramctl.h"
#endif

#include <plat/top.h>
#include "sie_eng_limit.h"
#include "sie_eng_int.h"
#include "sie_eng_int_platform.h"
#include "sie_eng_int_dbg.h"

int sie_eng_platform_flg_clear(SIE_ENG_HANDLE *p_eng, FLGPTN flg)
{
    return vos_flag_clr(p_eng->flg_id, flg);
}

int sie_eng_platform_flg_set(SIE_ENG_HANDLE *p_eng, FLGPTN flg)
{
    return vos_flag_iset(p_eng->flg_id, flg);
}

int sie_eng_platform_flg_wait(SIE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg)
{
    return vos_flag_wait(p_flgptn, p_eng->flg_id, flg, (TWF_CLR | TWF_ANDW));
}

int sie_eng_platform_flg_wait_timeout(SIE_ENG_HANDLE *p_eng, PFLGPTN p_flgptn, FLGPTN flg, int timeout_ms)
{
    return vos_flag_wait_timeout(p_flgptn, p_eng->flg_id, flg, (TWF_CLR | TWF_ANDW), ((timeout_ms <= 0) ? -1 : vos_util_msec_to_tick(timeout_ms)));   ///< -1 to wait forever
}

static irqreturn_t sie_eng_platform_isr(int irq, void *param)
{
    sie_eng_isr_hw_reg((SIE_ENG_HANDLE *)param);
    return IRQ_HANDLED;
}

int sie_eng_platform_request_irq(SIE_ENG_HANDLE *p_eng)
{
    return request_irq(p_eng->irq_id, sie_eng_platform_isr, IRQF_TRIGGER_HIGH, p_eng->name, p_eng);
}

void sie_eng_platform_release_irq(SIE_ENG_HANDLE *p_eng)
{
    free_irq(p_eng->irq_id, p_eng);
}

int sie_eng_platform_prepare_clk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    /* clk_prepare may sleep */
    if (p_eng->sie_clk)
        clk_prepare(p_eng->sie_clk);            ///< for SIE master clock gating/source/rate control
    if (p_eng->sie_bccclk)
        clk_prepare(p_eng->sie_bccclk);         ///< for SIE bcc clock source control
    if (p_eng->sie_intclk)
        clk_prepare(p_eng->sie_intclk);         ///< for SIE internal clock source control
    if (p_eng->sie_pxclk)
        clk_prepare(p_eng->sie_pxclk);          ///< for SIE pixel clock gating control
    if (p_eng->sie_pxclkpad)
        clk_prepare(p_eng->sie_pxclkpad);       ///< for SIE pixel clock source control
    if (p_eng->sie_mclk)
        clk_prepare(p_eng->sie_mclk);           ///< for SENSOR_MCLK1~4 clock gating/source/rate control
    if (p_eng->sie_tsen_rxclk)
        clk_prepare(p_eng->sie_tsen_rxclk);     ///< for SIE t-sensor clock gating/source control
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_enable_system_reset(SIE_RSTN);  ///< toggle engine hw reset
            pll_disable_system_reset(SIE_RSTN);
            break;
        case SIE_ENG_ID_2:
            pll_enable_system_reset(SIE2_RSTN);
            pll_disable_system_reset(SIE2_RSTN);
            break;
        case SIE_ENG_ID_3:
            pll_enable_system_reset(SIE3_RSTN);
            pll_disable_system_reset(SIE3_RSTN);
            break;
        case SIE_ENG_ID_4:
            pll_enable_system_reset(SIE4_RSTN);
            pll_disable_system_reset(SIE4_RSTN);
            break;
        case SIE_ENG_ID_5:
            pll_enable_system_reset(SIE5_RSTN);
            pll_disable_system_reset(SIE5_RSTN);
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_unprepare_clk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    /* clk_unprepare may sleep */
    if (p_eng->sie_clk)
        clk_unprepare(p_eng->sie_clk);            ///< for SIE master clock gating/source/rate control
    if (p_eng->sie_bccclk)
        clk_unprepare(p_eng->sie_bccclk);         ///< for SIE bcc clock source control
    if (p_eng->sie_intclk)
        clk_unprepare(p_eng->sie_intclk);         ///< for SIE internal clock source control
    if (p_eng->sie_pxclk)
        clk_unprepare(p_eng->sie_pxclk);          ///< for SIE pixel clock gating control
    if (p_eng->sie_pxclkpad)
        clk_unprepare(p_eng->sie_pxclkpad);       ///< for SIE pixel clock source control
    if (p_eng->sie_mclk)
        clk_unprepare(p_eng->sie_mclk);           ///< for SENSOR_MCLK1~4 clock gating/source/rate control
    if (p_eng->sie_tsen_rxclk)
        clk_unprepare(p_eng->sie_tsen_rxclk);     ///< for SIE t-sensor clock gating/source control
#else
    /* Do nothing */
#endif

    return ret;
}

int sie_eng_platform_enable_clk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_clk)
        clk_enable(p_eng->sie_clk);                 ///< must do clock prepare before enable
    if (p_eng->sie_pxclk)
        clk_enable(p_eng->sie_pxclk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_enable_clock(SIE_CLK);
            pll_enable_clock(SIE_PXCLK);    ///< parallel sensor pixel clock
            break;
        case SIE_ENG_ID_2:
            pll_enable_clock(SIE2_CLK);
            break;
        case SIE_ENG_ID_3:
            pll_enable_clock(SIE3_CLK);
            pll_enable_clock(SIE3_PXCLK);   ///< parallel sensor pixel clock
            break;
        case SIE_ENG_ID_4:
            pll_enable_clock(SIE4_CLK);
            break;
        case SIE_ENG_ID_5:
            pll_enable_clock(SIE5_CLK);
            pll_enable_clock(SIE5_PXCLK);   ///< parallel sensor pixel clock
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_disable_clk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_clk)
        clk_disable(p_eng->sie_clk);
    if (p_eng->sie_pxclk)
        clk_disable(p_eng->sie_pxclk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_disable_clock(SIE_CLK);
            pll_disable_clock(SIE_PXCLK);   ///< parallel sensor pixel clock
            break;
        case SIE_ENG_ID_2:
            pll_disable_clock(SIE2_CLK);
            break;
        case SIE_ENG_ID_3:
            pll_disable_clock(SIE3_CLK);
            pll_disable_clock(SIE3_PXCLK);  ///< parallel sensor pixel clock
            break;
        case SIE_ENG_ID_4:
            pll_disable_clock(SIE4_CLK);
            break;
        case SIE_ENG_ID_5:
            pll_disable_clock(SIE5_CLK);
            pll_disable_clock(SIE5_PXCLK);  ///< parallel sensor pixel clock
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_set_clk_rate(SIE_ENG_HANDLE *p_eng, UINT32 clk_rate)
{
    int ret = 0;

    if (p_eng->eng_id >= SIE_ENG_ID_MAX) {
        sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
        return -1;
    }

    if (!clk_rate || clk_rate > p_eng->max_clk_rate) {
        sie_err("%s Clock_Rate=%d over spec(MAX: %dHz)", p_eng->name, (int)clk_rate, (int)p_eng->max_clk_rate);
        ret = -1;
        goto exit;
    }

#if defined (__LINUX)
    if (!p_eng->sie_clk)
        return 0;

    ret = clk_set_rate(p_eng->sie_clk, clk_rate);
    if (ret < 0) {
        sie_err("%s set module clock rate %uHz failed!\n", p_eng->name, (u32)clk_rate);
    }
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_set_clock_freq(SIECLK_FREQ,  clk_rate);
            break;
        case SIE_ENG_ID_2:
            pll_set_clock_freq(SIE2CLK_FREQ, clk_rate);
            break;
        case SIE_ENG_ID_3:
            pll_set_clock_freq(SIE3CLK_FREQ, clk_rate);
            break;
        case SIE_ENG_ID_4:
            pll_set_clock_freq(SIE4CLK_FREQ, clk_rate);
            break;
        case SIE_ENG_ID_5:
            pll_set_clock_freq(SIE5CLK_FREQ, clk_rate);
            break;
        default:
            break;
    }
#endif

exit:
    return ret;
}

int sie_eng_platform_set_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_CLKSRC clk_src)
{
    int ret = 0;

#if defined (__LINUX)
    struct clk *parent_clk = NULL;
    char *clk_name[] = {"curr", "fix480m", "pll17", "pll5", "pll13", "pll12", "fix320m", "fix192m", "pll10"};

    if (p_eng->eng_id >= SIE_ENG_ID_MAX) {
        sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
        return -1;
    }

    if (clk_src >= SIE_ENG_CLKSRC_MAX) {
        sie_err("%s set module clk_src=%d invalid\n", p_eng->name, clk_src);
        return -1;
    }

    if (!p_eng->sie_clk || (clk_src == SIE_ENG_CLKSRC_CURR))
        return 0;

    if (clk_src == SIE_ENG_CLKSRC_PLL17) {
        if ((p_eng->plat_id != CHIP_NS02402) ||
            ((p_eng->plat_id == CHIP_NS02402) && (p_eng->eng_id > SIE_ENG_ID_2))) {     ///< only 539A SIE1/2 supported
            sie_err("%s set module clk_src=%d not supported\n", p_eng->name, clk_src);
            return -1;
        }
    }

    parent_clk = clk_get(NULL, clk_name[clk_src]);
    if (IS_ERR(parent_clk)) {
        sie_err("%s get module parent clock %s failed!(err:%ld)\n", p_eng->name, clk_name[clk_src], PTR_ERR(parent_clk));
        return -1;
    }

    ret = clk_set_parent(p_eng->sie_clk, parent_clk);
    if (ret < 0) {
        sie_err("%s set module parent clock to %s failed!\n", p_eng->name, clk_name[clk_src]);
    }
    clk_put(parent_clk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            if (clk_src == SIE_ENG_CLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_480);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL17) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_PLL17);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_PLL13);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_PLL12);
            }
            else if (clk_src == SIE_ENG_CLKSRC_320) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_320);
            }
            else if (clk_src == SIE_ENG_CLKSRC_192) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_192);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_CLKSRC, PLL_CLKSEL_SIE_CLKSRC_PLL10);
            }
            break;
        case SIE_ENG_ID_2:
            if (clk_src == SIE_ENG_CLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_480);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL17) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_PLL17);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_PLL13);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_PLL12);
            }
            else if (clk_src == SIE_ENG_CLKSRC_320) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_320);
            }
            else if (clk_src == SIE_ENG_CLKSRC_192) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_192);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_CLKSRC, PLL_CLKSEL_SIE2_CLKSRC_PLL10);
            }
            break;
        case SIE_ENG_ID_3:
            if (clk_src == SIE_ENG_CLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_480);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_PLL13);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_PLL12);
            }
            else if (clk_src == SIE_ENG_CLKSRC_320) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_320);
            }
            else if (clk_src == SIE_ENG_CLKSRC_192) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_192);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
                pll_set_clock_rate(PLL_CLKSEL_SIE3_CLKSRC, PLL_CLKSEL_SIE3_CLKSRC_PLL10);
            }
            break;
        case SIE_ENG_ID_4:
            if (clk_src == SIE_ENG_CLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_480);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_PLL13);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_PLL12);
            }
            else if (clk_src == SIE_ENG_CLKSRC_320) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_320);
            }
            else if (clk_src == SIE_ENG_CLKSRC_192) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_192);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_CLKSRC, PLL_CLKSEL_SIE4_CLKSRC_PLL10);
            }
            break;
        case SIE_ENG_ID_5:
            if (clk_src == SIE_ENG_CLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_480);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_PLL13);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_PLL12);
            }
            else if (clk_src == SIE_ENG_CLKSRC_320) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_320);
            }
            else if (clk_src == SIE_ENG_CLKSRC_192) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_192);
            }
            else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
                pll_set_clock_rate(PLL_CLKSEL_SIE5_CLKSRC, PLL_CLKSEL_SIE5_CLKSRC_PLL10);
            }
            break;
        default:
            sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }

    /* check clock parent enable or not */
    if (ret == 0) {
        if (clk_src == SIE_ENG_CLKSRC_PLL5) {
            if (!pll_get_pll_enable(PLL_ID_5)) {
                pll_set_pll_enable(PLL_ID_5, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_CLKSRC_PLL13) {
            if (!pll_get_pll_enable(PLL_ID_13)) {
                pll_set_pll_enable(PLL_ID_13, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_CLKSRC_PLL12) {
            if (!pll_get_pll_enable(PLL_ID_12)) {
                pll_set_pll_enable(PLL_ID_12, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_CLKSRC_PLL10) {
            if (!pll_get_pll_enable(PLL_ID_10)) {
                pll_set_pll_enable(PLL_ID_10, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_CLKSRC_PLL17) {
            if (!pll_get_pll_enable(PLL_ID_17)) {
                pll_set_pll_enable(PLL_ID_17, TRUE);
            }
        }
    }
#endif

    return ret;
}

int sie_eng_platform_set_pixel_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_PXCLKSRC clk_src)
{
    return 0;   ///< not supported
}

int sie_eng_platform_set_bcc_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_BCCCLKSRC bccclk_src)
{
    return 0;   ///< not supported
}

int sie_eng_platform_set_internal_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_INTCLKSRC intclk_src)
{
    int ret = 0;

#if defined (__LINUX)
    struct clk *parent_clk = NULL;
    char *clk_name[] = {"curr", "siex_intclk0", "siex_intclk1"};
    int   clk_idx = 0;

    if (p_eng->eng_id >= SIE_ENG_ID_MAX) {
        sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
        return -1;
    }

    if (intclk_src >= SIE_ENG_INTCLKSRC_MAX) {
        sie_err("%s set internal clk_src=%d invalid\n", p_eng->name, intclk_src);
        return -1;
    }

    if (!p_eng->sie_intclk || (intclk_src == SIE_ENG_INTCLKSRC_CURR))
        return 0;

    switch (p_eng->eng_id) {
        case SIE_ENG_ID_2:
            if (intclk_src == SIE_ENG_INTCLKSRC_SIE2) {
                clk_idx = 1;
            }
            else if (intclk_src == SIE_ENG_INTCLKSRC_SIE1) {
                clk_idx = 2;
            }
            break;
        case SIE_ENG_ID_4:
            if (intclk_src == SIE_ENG_INTCLKSRC_SIE4) {
                clk_idx = 1;
            }
            else if (intclk_src == SIE_ENG_INTCLKSRC_SIE3) {
                clk_idx = 2;
            }
            break;
        default:
            break;
    }

    if (clk_idx == 0)
        return 0;

    parent_clk = clk_get(NULL, clk_name[clk_idx]);
    if (IS_ERR(parent_clk)) {
        sie_err("%s get internal parent clock %s failed!(err:%ld)\n", p_eng->name, clk_name[clk_idx], PTR_ERR(parent_clk));
        return -1;
    }

    ret = clk_set_parent(p_eng->sie_intclk, parent_clk);
    if (ret < 0) {
        sie_err("%s set internal parent clock to %s failed!\n", p_eng->name, clk_name[clk_idx]);
    }
    clk_put(parent_clk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_2:
            if (intclk_src == SIE_ENG_INTCLKSRC_SIE2) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_INT_CLKSRC,  PLL_CLKSEL_SIE2_INT_CLKSRC_SIE2);
            }
            else if (intclk_src == SIE_ENG_INTCLKSRC_SIE1) {
                pll_set_clock_rate(PLL_CLKSEL_SIE2_INT_CLKSRC,  PLL_CLKSEL_SIE2_INT_CLKSRC_SIE1);
            }
            break;
        case SIE_ENG_ID_4:
            if (intclk_src == SIE_ENG_INTCLKSRC_SIE4) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_INT_CLKSRC,  PLL_CLKSEL_SIE4_INT_CLKSRC_SIE4);
            }
            else if (intclk_src == SIE_ENG_INTCLKSRC_SIE3) {
                pll_set_clock_rate(PLL_CLKSEL_SIE4_INT_CLKSRC,  PLL_CLKSEL_SIE4_INT_CLKSRC_SIE3);
            }
            break;
        case SIE_ENG_ID_1:
        case SIE_ENG_ID_3:
        case SIE_ENG_ID_5:
            /* Not supported */
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_set_sensor_mclk_rate(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id, UINT32 clk_rate)
{
    int ret = 0;

    /* check max clock rate */
    if (!clk_rate || clk_rate > SIE_MCLK_MAX_CLK_FREQ) {
        sie_err("SENSOR_MCLK%d clock_rate=%d over spec(MAX: %dHz)", (int)(mclk_id+1), (int)clk_rate, SIE_MCLK_MAX_CLK_FREQ);
        return -1;
    }

#if defined (__LINUX)
    if (p_eng->sie_mclk) {
        ret = clk_set_rate(p_eng->sie_mclk, clk_rate);
        if (ret < 0) {
            sie_err("%s set sensor clock rate %uHz failed!\n", p_eng->name, (u32)clk_rate);
        }
    }
#else
    switch (mclk_id) {
        case SIE_ENG_MCLK_ID_1:
            pll_set_clock_freq(SIEMCLK_FREQ,  clk_rate);
            break;
        case SIE_ENG_MCLK_ID_2:
            pll_set_clock_freq(SIEMCLK2_FREQ, clk_rate);
            break;
        case SIE_ENG_MCLK_ID_3:
            pll_set_clock_freq(SIEMCLK3_FREQ, clk_rate);
            break;
        case SIE_ENG_MCLK_ID_4:
            pll_set_clock_freq(SIEMCLK4_FREQ, clk_rate);
            break;
        default:
            ret = -1;
            sie_err("mclk_id=%d invalid\r\n", (int)mclk_id);
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_set_sensor_mclk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id, SIE_ENG_MCLKSRC clk_src)
{
    int ret = 0;

#if defined (__LINUX)
    struct clk *parent_clk = NULL;
    char *clk_name[] = {"curr", "fix480m", "pll5", "pll6", "pll12"};

    if (clk_src >= SIE_ENG_MCLKSRC_MAX) {
        sie_err("%s set sensor clk_src=%d invalid\n", p_eng->name, clk_src);
        return -1;
    }

    if (!p_eng->sie_mclk || (clk_src == SIE_ENG_MCLKSRC_CURR))
        return 0;

    parent_clk = clk_get(NULL, clk_name[clk_src]);
    if (IS_ERR(parent_clk)) {
        sie_err("%s get sensor parent clock %s failed!(err:%ld)\n", p_eng->name, clk_name[clk_src], PTR_ERR(parent_clk));
        return -1;
    }

    ret = clk_set_parent(p_eng->sie_mclk, parent_clk);
    if (ret < 0) {
        sie_err("%s set sensor parent clock to %s failed!\n", p_eng->name, clk_name[clk_src]);
    }
    clk_put(parent_clk);
#else
    switch (mclk_id) {
        case SIE_ENG_MCLK_ID_1:
            if (clk_src == SIE_ENG_MCLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_480);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL5);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL6) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL6);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLKSRC, PLL_CLKSEL_SIE_MCLKSRC_PLL12);
            }
            break;
        case SIE_ENG_MCLK_ID_2:
            if (clk_src == SIE_ENG_MCLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_480);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL5);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL6) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL6);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK2SRC, PLL_CLKSEL_SIE_MCLK2SRC_PLL12);
            }
            break;
        case SIE_ENG_MCLK_ID_3:
            if (clk_src == SIE_ENG_MCLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_480);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL5);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL6) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL6);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK3SRC, PLL_CLKSEL_SIE_MCLK3SRC_PLL12);
            }
            break;
        case SIE_ENG_MCLK_ID_4:
            if (clk_src == SIE_ENG_MCLKSRC_480) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_480);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL5) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL5);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL6) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL6);
            }
            else if (clk_src == SIE_ENG_MCLKSRC_PLL12) {
                pll_set_clock_rate(PLL_CLKSEL_SIE_MCLK4SRC, PLL_CLKSEL_SIE_MCLK4SRC_PLL12);
            }
            break;
        default:
            sie_err("mclk_id=%d invalid\n", (int)mclk_id);
            ret = -1;
            break;
    }

    /* check clock parent enable or not */
    if (ret == 0) {
        if (clk_src == SIE_ENG_MCLKSRC_PLL5) {
            if (!pll_get_pll_enable(PLL_ID_5)) {
                pll_set_pll_enable(PLL_ID_5, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_MCLKSRC_PLL6) {
            if (!pll_get_pll_enable(PLL_ID_6)) {
                pll_set_pll_enable(PLL_ID_6, TRUE);
            }
        }
        else if (clk_src == SIE_ENG_MCLKSRC_PLL12) {
            if (!pll_get_pll_enable(PLL_ID_12)) {
                pll_set_pll_enable(PLL_ID_12, TRUE);
            }
        }
    }
#endif

    return ret;
}

int sie_eng_platform_enable_sensor_mclk(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_mclk)
        clk_enable(p_eng->sie_mclk);
#else
    switch (mclk_id) {
        case SIE_ENG_MCLK_ID_1:
            pll_enable_clock(SIE_MCLK);
            break;
        case SIE_ENG_MCLK_ID_2:
            pll_enable_clock(SIE_MCLK2);
            break;
        case SIE_ENG_MCLK_ID_3:
            pll_enable_clock(SIE_MCLK3);
            break;
        case SIE_ENG_MCLK_ID_4:
            pll_enable_clock(SIE_MCLK4);
            break;
        default:
            sie_err("mclk_id=%d invalid\r\n", mclk_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_disable_sensor_mclk(SIE_ENG_HANDLE *p_eng, SIE_ENG_MCLK_ID_T mclk_id)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_mclk)
        clk_disable(p_eng->sie_mclk);
#else
    switch (mclk_id) {
        case SIE_ENG_MCLK_ID_1:
            pll_disable_clock(SIE_MCLK);
            break;
        case SIE_ENG_MCLK_ID_2:
            pll_disable_clock(SIE_MCLK2);
            break;
        case SIE_ENG_MCLK_ID_3:
            pll_disable_clock(SIE_MCLK3);
            break;
        case SIE_ENG_MCLK_ID_4:
            pll_disable_clock(SIE_MCLK4);
            break;
        default:
            sie_err("mclk_id=%d invalid\r\n", mclk_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_enable_tsen_rxclk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_tsen_rxclk)
        clk_enable(p_eng->sie_tsen_rxclk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
        case SIE_ENG_ID_2:
        case SIE_ENG_ID_3:
        case SIE_ENG_ID_4:
            /* Not supported */
            break;
        case SIE_ENG_ID_5:
            pll_enable_clock(SIE5_STSEN_RX_CLK);
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_disable_tsen_rxclk(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_tsen_rxclk)
        clk_disable(p_eng->sie_tsen_rxclk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
        case SIE_ENG_ID_2:
        case SIE_ENG_ID_3:
        case SIE_ENG_ID_4:
            /* Not supported */
            break;
        case SIE_ENG_ID_5:
            pll_disable_clock(SIE5_STSEN_RX_CLK);
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_set_tsen_rxclk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_TSEN_RXCLKSRC clk_src)
{
    int ret = 0;

#if defined (__LINUX)
    struct clk *parent_clk = NULL;
    char *clk_name[] = {"curr", "siex_tsen_pxclk", "siex_tsen_mclk3"};
    int   clk_idx = 0;

    if (p_eng->eng_id >= SIE_ENG_ID_MAX) {
        sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
        return -1;
    }

    if (clk_src >= SIE_ENG_TSEN_RXCLKSRC_MAX) {
        sie_err("%s set tsen rx clksrc=%d invalid\n", p_eng->name, clk_src);
        return -1;
    }

    if (!p_eng->sie_tsen_rxclk || (clk_src == SIE_ENG_TSEN_RXCLKSRC_CURR))
        return 0;

    switch (p_eng->eng_id) {
        case SIE_ENG_ID_5:
            if (clk_src == SIE_ENG_TSEN_RXCLKSRC_PXCLK) {
                clk_idx = 1;
            }
            else if (clk_src == SIE_ENG_TSEN_RXCLKSRC_SN_MCLK3) {
                clk_idx = 2;
            }
            break;
        default:
            break;
    }

    if (clk_idx == 0)
        return 0;

    parent_clk = clk_get(NULL, clk_name[clk_idx]);
    if (IS_ERR(parent_clk)) {
        sie_err("%s get tsen parent clock %s failed!(err:%ld)\n", p_eng->name, clk_name[clk_idx], PTR_ERR(parent_clk));
        return -1;
    }

    ret = clk_set_parent(p_eng->sie_tsen_rxclk, parent_clk);
    if (ret < 0) {
        sie_err("%s set tsen parent clock to %s failed!\n", p_eng->name, clk_name[clk_idx]);
    }
    clk_put(parent_clk);
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
        case SIE_ENG_ID_2:
        case SIE_ENG_ID_3:
        case SIE_ENG_ID_4:
            /* Not supported */
            break;
        case SIE_ENG_ID_5:
            if (clk_src == SIE_ENG_TSEN_RXCLKSRC_PXCLK) {
                pll_set_clock_rate(PLL_CLKSEL_STSEN_RX_CLKSRC,  PLL_CLKSEL_STSEN_RX_CLKSRC_PXCLK);
            }
            else if (clk_src == SIE_ENG_TSEN_RXCLKSRC_SN_MCLK3) {
                pll_set_clock_rate(PLL_CLKSEL_STSEN_RX_CLKSRC,  PLL_CLKSEL_STSEN_RX_CLKSRC_SIE_MACLK3);
            }
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_disable_sram_shutdown(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            nvt_disable_sram_shutdown(SIE_SD);
            break;
        case SIE_ENG_ID_2:
            nvt_disable_sram_shutdown(SIE2_SD);
            break;
        case SIE_ENG_ID_3:
            nvt_disable_sram_shutdown(SIE3_SD);
            break;
        case SIE_ENG_ID_4:
            nvt_disable_sram_shutdown(SIE4_SD);
            break;
        case SIE_ENG_ID_5:
            nvt_disable_sram_shutdown(SIE5_SD);
            if (p_eng->plat_id == CHIP_NS02402) {
                nvt_disable_sram_shutdown(VIE_SD);  ///< 539A SIE5 and VIE shared sram
            }
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }

    return ret;
}

int sie_eng_platform_enable_sram_shutdown(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            nvt_enable_sram_shutdown(SIE_SD);
            break;
        case SIE_ENG_ID_2:
            nvt_enable_sram_shutdown(SIE2_SD);
            break;
        case SIE_ENG_ID_3:
            nvt_enable_sram_shutdown(SIE3_SD);
            break;
        case SIE_ENG_ID_4:
            nvt_enable_sram_shutdown(SIE4_SD);
            break;
        case SIE_ENG_ID_5:
            nvt_enable_sram_shutdown(SIE5_SD);
            if (p_eng->plat_id == CHIP_NS02402) {
                nvt_enable_sram_shutdown(VIE_SD);  ///< 539A SIE5 and VIE shared sram, the shutdown control on VIE
            }
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }

    return ret;
}

int sie_eng_platform_disable_clk_auto_gating(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_clk) {
        ret = clk_set_phase(p_eng->sie_clk, 0);
    }
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_clear_clk_auto_gating(SIE_M_GCLK);
            break;
        case SIE_ENG_ID_2:
            pll_clear_clk_auto_gating(SIE2_M_GCLK);
            break;
        case SIE_ENG_ID_3:
            pll_clear_clk_auto_gating(SIE3_M_GCLK);
            break;
        case SIE_ENG_ID_4:
            pll_clear_clk_auto_gating(SIE4_M_GCLK);
            break;
        case SIE_ENG_ID_5:
            pll_clear_clk_auto_gating(SIE5_M_GCLK);
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_enable_clk_auto_gating(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_clk) {
        ret = clk_set_phase(p_eng->sie_clk, 1);
    }
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            pll_set_clk_auto_gating(SIE_M_GCLK);
            break;
        case SIE_ENG_ID_2:
            pll_set_clk_auto_gating(SIE2_M_GCLK);
            break;
        case SIE_ENG_ID_3:
            pll_set_clk_auto_gating(SIE3_M_GCLK);
            break;
        case SIE_ENG_ID_4:
            pll_set_clk_auto_gating(SIE4_M_GCLK);
            break;
        case SIE_ENG_ID_5:
            pll_set_clk_auto_gating(SIE5_M_GCLK);
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_get_clk_auto_gating(SIE_ENG_HANDLE *p_eng)
{
    int ret = 0;

#if defined (__LINUX)
    if (p_eng->sie_clk) {
        ret = clk_get_phase(p_eng->sie_clk);
    }
#else
    switch (p_eng->eng_id) {
        case SIE_ENG_ID_1:
            ret = pll_get_clk_auto_gating(SIE_M_GCLK)  ? 1 : 0;
            break;
        case SIE_ENG_ID_2:
            ret = pll_get_clk_auto_gating(SIE2_M_GCLK) ? 1 : 0;
            break;
        case SIE_ENG_ID_3:
            ret = pll_get_clk_auto_gating(SIE3_M_GCLK) ? 1 : 0;
            break;
        case SIE_ENG_ID_4:
            ret = pll_get_clk_auto_gating(SIE4_M_GCLK) ? 1 : 0;
            break;
        case SIE_ENG_ID_5:
            ret = pll_get_clk_auto_gating(SIE5_M_GCLK) ? 1 : 0;
            break;
        default:
            sie_err("eng_id=%d invalid\r\n", (int)p_eng->eng_id);
            ret = -1;
            break;
    }
#endif

    return ret;
}

int sie_eng_platform_set_shared_sram_clk_src(SIE_ENG_HANDLE *p_eng, SIE_ENG_SHARED_SRAM_CLKSRC clk_src)
{
    int ret = 0;

    if (p_eng->eng_id >= SIE_ENG_ID_MAX) {
        sie_err("eng_id=%d invalid\n", (int)p_eng->eng_id);
        return -1;
    }

    if (clk_src >= SIE_ENG_SHARED_SRAM_CLKSRC_MAX) {
        sie_err("%s set shared sram clk_src=%d invalid\n", p_eng->name, clk_src);
        return -1;
    }

    if (clk_src == SIE_ENG_SHARED_SRAM_CLKSRC_CURR)
        return 0;

    if ((p_eng->eng_id == SIE_ENG_ID_5) && (p_eng->plat_id == CHIP_NS02402)) {  ///< only 539A SIE5 supported
#if defined (__LINUX)
        struct clk *sram_clk   = NULL;
        struct clk *parent_clk = NULL;
        char *clk_name[] = {"curr", "sie5_sram_sie5", "sie5_sram_vie"};

        sram_clk = clk_get(NULL, "sie5_share_sram");
        if (IS_ERR(sram_clk)) {
            sie_err("%s get shared sram clock node failed!(err:%ld)\n", p_eng->name, PTR_ERR(sram_clk));
            return -1;
        }

        parent_clk = clk_get(NULL, clk_name[clk_src]);
        if (IS_ERR(parent_clk)) {
            sie_err("%s get shared sram parent clock %s failed!(err:%ld)\n", p_eng->name, clk_name[clk_src], PTR_ERR(parent_clk));
            clk_put(sram_clk);
            return -1;
        }

        ret = clk_set_parent(sram_clk, parent_clk);
        if (ret < 0) {
            sie_err("%s set shared sram parent clock to %s failed!\n", p_eng->name, clk_name[clk_src]);
        }
        clk_put(parent_clk);
        clk_put(sram_clk);
#else
        if (clk_src == SIE_ENG_SHARED_SRAM_CLKSRC_SIE) {
            pll_set_clock_rate(PLL_CLKSEL_SIE5_SRAM, PLL_CLKSEL_SIE5_SRAM_SIE5);
        }
        else if (clk_src == SIE_ENG_SHARED_SRAM_CLKSRC_VIE) {
            pll_set_clock_rate(PLL_CLKSEL_SIE5_SRAM, PLL_CLKSEL_SIE5_SRAM_VIE);
        }
#endif
    }

    return ret;
}
