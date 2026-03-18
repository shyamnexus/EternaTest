/**
    TGE module platform driver

    @file       tge_platform.c
    @ingroup    mIIPPTGE

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#if defined(__LINUX)
#include <linux/clk.h>
#else
#include "io_address.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#endif

#include "kwrap/semaphore.h"
#include "kwrap/spinlock.h"
#include "kwrap/util.h"

#include "tge_platform.h"
#include "tge_dbg.h"

#if defined(__LINUX)
static struct clk *tge_clk         = NULL;    ///< VD/HD/FLASH/MSH clock gating
static struct clk *tge_flash_a_clk = NULL;    ///< FlashA clock selection
static struct clk *tge_msh_a_clk   = NULL;    ///< MshA   clock selection

static char *tge_sn_clk_name[] = {"sn_mclk1", "sn_mclk2", "sn_mclk3", "sn_mclk4"};
#endif

static SEM_HANDLE SEMID_TGE;
static ID         FLG_ID_TGE;

static int tge_plat_inited = 0;
static int tge_irq         = -1;            ///< this will be set after resource create

ULONG tge_vbase = 0;                        ///< this will be set after resource create

#if defined(__FREERTOS)
irqreturn_t tge_platform_isr(int irq, void *devid)
{
    if (tge_plat_inited) {
        tge_isr();
    }
    return IRQ_HANDLED;
}
#endif

int tge_platform_is_inited(void)
{
    return tge_plat_inited;
}

ER tge_platform_create_resource(TGE_RESOURCE *pRes)
{
    int ret;

    if (tge_plat_inited) {
        DBG_WRN("TGE resource already created!\r\n");
        return E_OK;
    }

    /* check I/O base */
    if (!pRes->reg_io_base) {
        DBG_ERR("TGE register I/O address invalid!\r\n");
        return E_PAR;
    }
    tge_vbase = pRes->reg_io_base;

    /* init event flag */
    ret = OS_CONFIG_FLAG(FLG_ID_TGE);
    if (ret < 0) {
        DBG_ERR("TGE event flag create failed!\r\n");
        goto err_flg;
    }

    /* init semaphore */
    ret = SEM_CREATE(SEMID_TGE, 1);
    if (ret < 0) {
        DBG_ERR("TGE semaphore create failed!\r\n");
        goto err_sem;
    }

#if defined(__LINUX)
    /* set TGE VD/HD clock node */
    if (pRes->tge_clk) {
        tge_clk = pRes->tge_clk;
    }

    /* set TGE FlashA clock node */
    if (pRes->flash_a_clk) {
        tge_flash_a_clk = pRes->flash_a_clk;
    }

    /* set TGE MshA clock node */
    if (pRes->msh_a_clk) {
        tge_msh_a_clk = pRes->msh_a_clk;
    }
#endif

#if defined(__FREERTOS)
    /* request interrupt */
    if (pRes->irq_id > 0) {
        ret = request_irq(pRes->irq_id, tge_platform_isr, IRQF_TRIGGER_HIGH, "tge", NULL);
        if (ret < 0) {
            DBG_ERR("TGE request interrupt failed!\r\n");
            goto err_irq;
        }
    }
#endif

    if (pRes->irq_id > 0) {
        tge_irq = pRes->irq_id;
    }

    tge_plat_inited = 1;

    tge_platform_prepare_clk();

    return E_OK;

#if defined(__FREERTOS)
err_irq:
    SEM_DESTROY(SEMID_TGE);
#endif

err_sem:
    rel_flg(FLG_ID_TGE);

err_flg:
    return E_SYS;
}

void tge_platform_release_resource(void)
{
    if (!tge_plat_inited)
        return;

#if defined(__FREERTOS)
    if (tge_irq > 0) {
        free_irq(tge_irq, NULL);
        tge_irq = -1;
    }
#endif

    tge_platform_unprepare_clk();

#if defined(__LINUX)
    tge_clk         = NULL;
    tge_flash_a_clk = NULL;
    tge_msh_a_clk   = NULL;
#endif

    /* release flag */
    rel_flg(FLG_ID_TGE);

    /* destroy semaphore */
    SEM_DESTROY(SEMID_TGE);

    /* clear i/o base */
    tge_vbase = 0;

    /* clear flag */
    tge_plat_inited = 0;
}

ER tge_platform_flg_clear(FLGPTN flg)
{
    if (!tge_plat_inited)
        return E_SYS;

    clr_flg(FLG_ID_TGE, flg);
    return E_OK;
}

ER tge_platform_flg_set(FLGPTN flg)
{
    if (!tge_plat_inited)
        return E_SYS;

    iset_flg(FLG_ID_TGE, flg);
    return E_OK;
}

ER tge_platform_flg_wait(PFLGPTN p_flgptn, FLGPTN flg)
{
    if (!tge_plat_inited)
        return E_SYS;

    return vos_flag_wait(p_flgptn, FLG_ID_TGE, flg, (TWF_CLR | TWF_ANDW));
}

ER tge_platform_flg_wait_timeout(PFLGPTN p_flgptn, FLGPTN flg, int timeout_ms)
{
    if (!tge_plat_inited)
        return E_SYS;

    return vos_flag_wait_timeout(p_flgptn, FLG_ID_TGE, flg, (TWF_CLR | TWF_ANDW), ((timeout_ms <= 0) ? -1 : vos_util_msec_to_tick(timeout_ms)));   ///< -1 to wait forever
}

ER tge_platform_sem_wait(void)
{
    if (!tge_plat_inited)
        return E_SYS;

    return SEM_WAIT(SEMID_TGE);
}

ER tge_platform_sem_signal(void)
{
    if (!tge_plat_inited)
        return E_SYS;

    SEM_SIGNAL(SEMID_TGE);
    return E_OK;
}

void tge_platform_prepare_clk(void)
{
    if (!tge_plat_inited)
        return;

#if defined(__LINUX)
    if (tge_clk)
        clk_prepare(tge_clk);
    if (tge_flash_a_clk)
        clk_prepare(tge_flash_a_clk);
    if (tge_msh_a_clk)
        clk_prepare(tge_msh_a_clk);
#else
    pll_enable_system_reset(TGE_RSTN);  ///< toggle engine hw reset
    pll_disable_system_reset(TGE_RSTN);
#endif
}

void tge_platform_unprepare_clk(void)
{
    if (!tge_plat_inited)
        return;

#if defined(__LINUX)
    if (tge_clk)
        clk_unprepare(tge_clk);
    if (tge_flash_a_clk)
        clk_unprepare(tge_flash_a_clk);
    if (tge_msh_a_clk)
        clk_unprepare(tge_msh_a_clk);
#endif
}

void tge_platform_enable_clk(void)
{
    if (!tge_plat_inited)
        return;

#if defined(__LINUX)
    if (tge_clk)
        clk_enable(tge_clk);
    if (tge_flash_a_clk)
        clk_enable(tge_flash_a_clk);
    if (tge_msh_a_clk)
        clk_enable(tge_msh_a_clk);
#else
    pll_enableClock(TGE_CLK);
#endif
}

void tge_platform_disable_clk(void)
{
    if (!tge_plat_inited)
        return;

#if defined(__LINUX)
    if (tge_clk)
        clk_disable(tge_clk);
    if (tge_flash_a_clk)
        clk_disable(tge_flash_a_clk);
    if (tge_msh_a_clk)
        clk_disable(tge_msh_a_clk);
#else
    pll_disableClock(TGE_CLK);
#endif
}

ER tge_platform_set_flash_a_clk_src(TGE_FLASH_CLKSRC_SEL ClkSrc)
{
    int ret = E_OK;

    if (!tge_plat_inited)
        return E_SYS;

#if defined(__LINUX)
    if (ClkSrc > FLASHCLKSRC_MCLK4) {
        DBG_ERR("TGE set FlashA clock source=%d invalid\r\n", (int)ClkSrc);
        return E_PAR;
    }

    if (tge_flash_a_clk) {
        struct clk *parent_clk;

        parent_clk = clk_get(NULL, tge_sn_clk_name[ClkSrc]);
        if (IS_ERR(parent_clk)) {
            DBG_ERR("TGE get FlashA parent clock %s failed!(err:%ld)\r\n", tge_sn_clk_name[ClkSrc], PTR_ERR(parent_clk));
            return E_SYS;
        }

        ret = clk_set_parent(tge_flash_a_clk, parent_clk);
        if (ret < 0) {
            ret = E_SYS;
            DBG_ERR("TGE set FlashA parent clock to %s failed!\n", tge_sn_clk_name[ClkSrc]);
        }
        clk_put(parent_clk);
    }
#else
    switch (ClkSrc) {
        case FLASHCLKSRC_MCLK:
            pll_setClockRate(PLL_CLKSEL_TGE_FLASH, PLL_CLKSEL_TGE_FLASH_SIEMCLK);
            break;
        case FLASHCLKSRC_MCLK2:
            pll_setClockRate(PLL_CLKSEL_TGE_FLASH, PLL_CLKSEL_TGE_FLASH_SIEMCLK2);
            break;
        case FLASHCLKSRC_MCLK3:
            pll_setClockRate(PLL_CLKSEL_TGE_FLASH, PLL_CLKSEL_TGE_FLASH_SIEMCLK3);
            break;
        case FLASHCLKSRC_MCLK4:
            pll_setClockRate(PLL_CLKSEL_TGE_FLASH, PLL_CLKSEL_TGE_FLASH_SIEMCLK4);
            break;
        default:
            DBG_ERR("TGE set FlashA clock source=%d invalid\r\n", (int)ClkSrc);
            ret = E_PAR;
            break;
    }
#endif

    return ret;
}

ER tge_platform_set_msh_a_clk_src(TGE_MSH_CLKSRC_SEL ClkSrc)
{
    int ret = E_OK;

    if (!tge_plat_inited)
        return E_SYS;

#if defined(__LINUX)
    if (ClkSrc > MSHCLKSRC_MCLK4) {
        DBG_ERR("TGE set MshA clock source=%d invalid\r\n", (int)ClkSrc);
        return E_PAR;
    }

    if (tge_msh_a_clk) {
        struct clk *parent_clk;

        parent_clk = clk_get(NULL, tge_sn_clk_name[ClkSrc]);
        if (IS_ERR(parent_clk)) {
            DBG_ERR("TGE get MshA parent clock %s failed!(err:%ld)\r\n", tge_sn_clk_name[ClkSrc], PTR_ERR(parent_clk));
            return E_SYS;
        }

        ret = clk_set_parent(tge_msh_a_clk, parent_clk);
        if (ret < 0) {
            ret = E_SYS;
            DBG_ERR("TGE set MshA parent clock to %s failed!\n", tge_sn_clk_name[ClkSrc]);
        }
        clk_put(parent_clk);
    }
#else
    switch (ClkSrc) {
        case MSHCLKSRC_MCLK:
            pll_setClockRate(PLL_CLKSEL_TGE_MSH, PLL_CLKSEL_TGE_MSH_SIEMCLK);
            break;
        case MSHCLKSRC_MCLK2:
            pll_setClockRate(PLL_CLKSEL_TGE_MSH, PLL_CLKSEL_TGE_MSH_SIEMCLK2);
            break;
        case MSHCLKSRC_MCLK3:
            pll_setClockRate(PLL_CLKSEL_TGE_MSH, PLL_CLKSEL_TGE_MSH_SIEMCLK3);
            break;
        case MSHCLKSRC_MCLK4:
            pll_setClockRate(PLL_CLKSEL_TGE_MSH, PLL_CLKSEL_TGE_MSH_SIEMCLK4);
            break;
        default:
            DBG_ERR("TGE set MshA clock source=%d invalid\r\n", (int)ClkSrc);
            ret = E_PAR;
            break;
    }
#endif

    return ret;
}
