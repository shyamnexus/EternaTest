#include <assert.h>
#include <plat/pll.h>
#include <plat/io_address.h>
#include <io.h>
#include <kernel/panic.h>
#include <initcall.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <stddef.h>
#include <trace.h>
#include <util.h>
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include <kwrap/error_no.h>
#include <pll_protected.h>

#include "nvt_sce_cfg.h"
#include "nvt_pll_cfg.h"

static vaddr_t nvt_clk_base_mmu;
static vaddr_t nvt_clk_base_nommu;
static vaddr_t nvt_clk_stbc_base_mmu;
static vaddr_t nvt_clk_stbc_base_nommu;

static  VK_DEFINE_SPINLOCK(my_lock);
#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
#define loc_multi_cores(flags)   loc_cpu(flags)
#define unl_multi_cores(flags)   unl_cpu(flags)

static void PLL_SETREG(uint32_t ofs, uint32_t value)
{
	if (cpu_mmu_enabled()) {
		return io_write32(nvt_clk_base_mmu + ofs, value);
	} else {
		return io_write32(nvt_clk_base_nommu + ofs, value);
	}
}

static uint32_t PLL_GETREG(uint32_t ofs)
{
	if (cpu_mmu_enabled()) {
		return io_read32(nvt_clk_base_mmu + ofs);
	} else {
		return io_read32(nvt_clk_base_nommu + ofs);
	}
}

#if defined(_BSP_NA51102_)

static void PLL_STBC_SETREG(uint32_t ofs, uint32_t value)
{
        if (cpu_mmu_enabled()) {
                return io_write32(nvt_clk_stbc_base_mmu + ofs, value);
        } else {
                return io_write32(nvt_clk_stbc_base_nommu + ofs, value);
        }
}

static uint32_t PLL_STBC_GETREG(uint32_t ofs)
{
        if (cpu_mmu_enabled()) {
                return io_read32(nvt_clk_stbc_base_mmu + ofs);
        } else {
                return io_read32(nvt_clk_stbc_base_nommu + ofs);
        }
}

ER pll_set_STBC_pll_enable(PLL_ID id, BOOL b_enable)
{
        unsigned long flags = 0;
        T_PLL_PLL_PWREN_REG pll_en_reg;
        T_PLL_PLL_STATUS_REG pll_status_reg;
        static int pll_enable_count[PLL_ID_MAX] = {0};

        if ((id >= PLL_ID_MAX) || (id == PLL_ID_1)) {
                DBG_ERR("id out of range: PLL%d\r\n", id);
                return E_ID;
        }

        if (b_enable) {
                pll_enable_count[id]++;
                if (pll_enable_count[id] > 1)
                        return E_OK;
        } else {
                if (pll_enable_count[id] == 0) {
                        DBG_ERR("invald disable calling: PLL%d\r\n", id);
                        return E_ID;
                }

                if (--pll_enable_count[id] > 0)
                        return E_OK;
        }

        loc_multi_cores(flags);
        pll_en_reg.reg = PLL_STBC_GETREG(PLL_PLL_PWREN_REG_OFS);
        if (b_enable) {
                pll_en_reg.reg |= 1 << id;
        } else {
                pll_en_reg.reg &= ~(1 << id);
        }
        PLL_STBC_SETREG(PLL_PLL_PWREN_REG_OFS, pll_en_reg.reg);

        unl_multi_cores(flags);

        if (b_enable) {
                // Wait PLL power is powered on
                while (1) {
                        pll_status_reg.reg = PLL_STBC_GETREG(PLL_PLL_STATUS_REG_OFS);
                        if (pll_status_reg.reg & (1 << id)) {
                                break;
                        }
                }
        }

        //pll_install_cmd();

        return E_OK;
}

#endif


/**
    Get PLL Enable

    @param[in] id           PLL ID

    @return
        - @b TRUE: PLL is enabled
        - @b FALSE: PLL is disabled or id is out of range
*/
BOOL pll_get_pll_enable(PLL_ID id)
{
	T_PLL_PLL_PWREN_REG pll_en_reg;

	if (id > PLL_ID_MAX) {
		DBG_ERR("id out of range: 0x%x\r\n", id);
		return FALSE;
	}

	pll_en_reg.reg = PLL_GETREG(PLL_PLL_PWREN_REG_OFS);
	if (pll_en_reg.reg & (1 << (id + 1))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

ER pll_set_pll_enable(PLL_ID id, BOOL b_enable)
{
	unsigned long flags = 0;
	T_PLL_PLL_PWREN_REG pll_en_reg;
	T_PLL_PLL_STATUS_REG pll_status_reg;
	static int pll_enable_count[PLL_ID_MAX] = {0};

	if ((id >= PLL_ID_MAX) || (id == PLL_ID_1)) {
		DBG_ERR("id out of range: PLL%d\r\n", id);
		return E_ID;
	}

	if (b_enable) {
		pll_enable_count[id]++;
		if (pll_enable_count[id] > 1)
			return E_OK;
	} else {
		if (pll_enable_count[id] == 0) {
			DBG_ERR("invald disable calling: PLL%d\r\n", id);
			return E_ID;
		}

		if (--pll_enable_count[id] > 0)
			return E_OK;
	}

	loc_multi_cores(flags);
	pll_en_reg.reg = PLL_GETREG(PLL_PLL_PWREN_REG_OFS);
	if (b_enable) {
		pll_en_reg.reg |= 1 << id;
	} else {
		pll_en_reg.reg &= ~(1 << id);
	}
	PLL_SETREG(PLL_PLL_PWREN_REG_OFS, pll_en_reg.reg);

	unl_multi_cores(flags);

	if (b_enable) {
		// Wait PLL power is powered on
		while (1) {
			pll_status_reg.reg = PLL_GETREG(PLL_PLL_STATUS_REG_OFS);
			if (pll_status_reg.reg & (1 << id)) {
				break;
			}
		}
	}

	//pll_install_cmd();

	return E_OK;
}

/*
    Get bit mask for specific module clock setting

    Get bit mask for specific module clock setting

    @param[in] ui_num    Specific module

    @return bit mask for specific module clock
*/
static uint32_t pll_get_clock_mask(uint32_t ui_num)
{
	switch (ui_num) {
	case PLL_CLKSEL_APB:
		return PLL_CLKSEL_APB_MASK;
	/*R5*/
	case PLL_CLKSEL_HASH:
		return PLL_CLKSEL_HASH_MASK;
	case PLL_CLKSEL_RSA:
		return PLL_CLKSEL_RSA_MASK;
	case PLL_CLKSEL_CRYPTO:
		return PLL_CLKSEL_CRYPTO_MASK;
	case PLL_CLKSEL_TRNG:
		return PLL_CLKSEL_TRNG_MASK;
	case PLL_CLKSEL_TRNGRO_CLKSRC:
		return PLL_CLKSEL_TRNGRO_CLKSRC_MASK;

	case PLL_CLKSEL_TRNG_CLKDIV:
		return PLL_CLKSEL_TRNG_CLKDIV_MASK;


	/*R20*/
	case PLL_CLKSEL_TRNG_RO_CLKDIV:
		return PLL_CLKSEL_TRNG_RO_CLKDIV_MASK;
	case PLL_CLKSEL_TRNG_RO_DELAY:
		return PLL_CLKSEL_TRNG_RO_DELAY_MASK;

	default:
		DBG_ERR("Non-supported Clk Mask ID! (0x%x)\r\n", (UINT)ui_num);
		return 0;
	}
}
/**
    Enable module clock

    Enable module clock, module clock must be enabled that it could be work correctly
    @param[in] num  Module enable ID, one module at a time

    @return void
*/
void pll_enable_clock(CG_EN num)
{
	REGVALUE    reg_data;
	uint32_t     ui_reg_offset;
	unsigned long flags = 0;

	ui_reg_offset = (num >> 5) << 2;

	//race condition protect. enter critical section
	loc_multi_cores(flags);
	reg_data  = PLL_GETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset);

	reg_data |= 1 << (num & 0x1F);

	PLL_SETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset, reg_data);

	//race condition protect. leave critical section
	unl_multi_cores(flags);
}

/**
    Set module clock rate

    Set module clock rate, one module at a time.

    @param[in] ui_num	Module ID(PLL_CLKSEL_*), one module at a time.
						Please refer to pll.h
    @param[in] ui_value	Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h

    @return void
*/
void pll_set_clock_rate(PLL_CLKSEL clk_sel, uint32_t ui_value)
{
	REGVALUE reg_data;
	uint32_t ui_mask, ui_reg_offset;
	unsigned long flags = 0;

	ui_mask = pll_get_clock_mask(clk_sel);
	ui_reg_offset = (clk_sel >> 5) << 2;

	//race condition protect. enter critical section
	loc_multi_cores(flags);

	reg_data = PLL_GETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset);
	reg_data &= ~ui_mask;
	reg_data |= ui_value;
	PLL_SETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset, reg_data);

	//race condition protect. leave critical section
	unl_multi_cores(flags);
}

/**
    Get module clock rate

    Get module clock rate, one module at a time.

    @param[in] ui_num	Module ID(PLL_CLKSEL_*), one module at a time.
						Please refer to pll.h

    @return Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h
*/
uint32_t pll_get_clock_rate(PLL_CLKSEL clk_sel)
{
	uint32_t      ui_mask, ui_reg_offset;
	REGVALUE    reg_data;

	if (clk_sel == PLL_CLKSEL_MAX_ITEM) {
		return PLL_CLKSEL_MAX_ITEM;
	}

	ui_mask = pll_get_clock_mask(clk_sel);
	ui_reg_offset = (clk_sel >> 5) << 2;

	reg_data = PLL_GETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset);
	reg_data &= ui_mask;

	return (uint32_t)reg_data;
}

/**
    Get the module clock frequency

    This api get the module clock frequency.

    @param[in] clock_id    Module select ID, refer to structure PLL_CLKFREQ.
    @param[out] p_freq   Return clock frequency. Unit in Hertz.

    @return
     - @b E_ID:     clock_id is not support in this API.
     - @b E_PAR:    Target frequency can not be divided with no remainder.
     - @b E_OK:     Done and success.
*/
ER pll_get_clock_freq(PLL_CLKFREQ clock_id, uint32_t *p_freq)
{
	uint32_t source_clock = 0, divider = 0, clock_source = 0;

	if (clock_id >= PLL_CLKFREQ_MAXNUM) {
		return E_ID;
	}

	if (p_freq == NULL) {
		DBG_ERR("input p_freq is NULL\r\n");
		return E_PAR;
	}

	// Get Src Clock Frequency
	switch (clock_id) {
	case TRNGCLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_TRNG);
			divider = pll_get_clock_rate(PLL_CLKSEL_TRNG_CLKDIV) >> (PLL_CLKSEL_TRNG_CLKDIV  & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_TRNG_240:
				source_clock = 240000000;
				break;
			case PLL_CLKSEL_TRNG_160:
				source_clock = 160000000;
				break;
			default:
				return E_PAR;
			}
		}
		break;

	case APBCLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_APB);

			switch (clock_source) {
			case PLL_CLKSEL_APB_120:
				source_clock = 120000000;
				break;
			case PLL_CLKSEL_APB_150:
				source_clock = 150000000;
				break;
			default:
				return E_PAR;
			}
		}
		break;

	default:
		return E_PAR;
	}

	*p_freq = source_clock / (divider + 1);

	return E_OK;
}

/**
    Module (hardware) reset on.

    This function will reset module.

    @param[in] num  Reset bit number of type CG_RSTN, only one at a time

    @return void
*/
void pll_enable_system_reset(CG_RSTN num)
{
	REGVALUE    reg_data;
	uint32_t      ui_reg_offset;
	unsigned long flags = 0;

	ui_reg_offset = (num >> 5) << 2;

	//race condition protect. enter critical section
	loc_multi_cores(flags);
	reg_data = PLL_GETREG(PLL_SYS_RST0_REG_OFS + ui_reg_offset);
	reg_data &= ~(1 << (num & 0x1F));

	PLL_SETREG(PLL_SYS_RST0_REG_OFS + ui_reg_offset, reg_data);

	//race condition protect. leave critical section
	unl_multi_cores(flags);
}

/**
    Module (hardware) reset off.

    This fulction will enable module.

    @param[in] num  Reset bit number of type CG_RSTN, only one at a time

    @return void
*/
void pll_disable_system_reset(CG_RSTN num)
{
	REGVALUE    reg_data;
	uint32_t    ui_reg_offset;
	unsigned long flags = 0;

	ui_reg_offset = (num >> 5) << 2;

	//race condition protect. enter critical section
	loc_multi_cores(flags);

	reg_data = PLL_GETREG(PLL_SYS_RST0_REG_OFS + ui_reg_offset);
	reg_data |= 1 << (num & 0x1F);

	PLL_SETREG(PLL_SYS_RST0_REG_OFS + ui_reg_offset, reg_data);

	//race condition protect. leave critical section
	unl_multi_cores(flags);
}

/*
    Get TRNG RO clock sel

    Get TRNG RO clock sel

    @param[out] pui_trng_ro_sel    TRNG RO sel
    @param[out] pui_divider          Clock divider of TRNG

    @return void
*/
void pll_get_trng_ro_sel(uint32_t *pui_trng_ro_sel, uint32_t *pui_divider)
{
	REGVALUE    reg_data;


	reg_data = PLL_GETREG(PLL_TRNG_REG_OFS);

	*pui_divider = reg_data & 0xFF;
	*pui_trng_ro_sel = (reg_data & 0xF00) >> 8;

}

static TEE_Result nvt_clk_init(void)
{
	nvt_clk_base_mmu = (vaddr_t)phys_to_virt(IOADDR_CG_REG_BASE, MEM_AREA_IO_SEC, 0x30);
	nvt_clk_base_nommu = ((paddr_t)IOADDR_CG_REG_BASE);
#if defined(_BSP_NA51102_)
	nvt_clk_stbc_base_mmu = (vaddr_t)phys_to_virt(IOADDR_STBC_CG_REG_BASE, MEM_AREA_IO_SEC);
    nvt_clk_stbc_base_nommu = ((paddr_t)IOADDR_STBC_CG_REG_BASE);
#endif

	DMSG("nvt_clk_init done\n");
	return TEE_SUCCESS;
}

service_init(nvt_clk_init);

