
/*
 * True number generater
 * Copyright 2023 Novatek.
 */
#include <compiler.h>
#include <crypto/crypto.h>
#include <rng_support.h>
#include <tee/tee_cryp_utl.h>
#include <types_ext.h>
#include <assert.h>
#include <plat/io_address.h>
#include <io.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <stddef.h>
#include <trace.h>
#include <initcall.h>
#include <util.h>
#include <plat/pll.h>
#include <pll_protected.h>
#include <kwrap/debug.h>


/* Registers */

static vaddr_t nvt_trng_mmu;
static vaddr_t nvt_trng_nommu;

#define RAND_CASR_EN			0x00000100
#define POST_PROC_EN			0x00000200
#define TRNG_CONTROL_REG_OFS      0x00
#define TRNG_SEEDL_REG_OFS        0x04
#define TRNG_SEEDH_REG_OFS        0x08
#define TRNG_RANDNUM_REG_OFS      0x0C



static void TRNG_SETREG(uint32_t ofs, uint32_t value)
{
	if (cpu_mmu_enabled()) {
		return io_write32(nvt_trng_mmu + ofs, value);
	} else {
		return io_write32(nvt_trng_nommu + ofs, value);
	}
}

static uint32_t TRNG_GETREG(uint32_t ofs)
{
	if (cpu_mmu_enabled()) {
		return io_read32(nvt_trng_mmu + ofs);
	} else {
		return io_read32(nvt_trng_nommu + ofs);
	}
}

/**
    Enable/Disable CASR.

    Enable/Disable CASR.

    @param[in] b_en  enable/disable for the CASR function
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
static void trng_set_casr(BOOL b_en)
{
	uint32_t trng_ctrlreg;

	trng_ctrlreg = TRNG_GETREG(TRNG_CONTROL_REG_OFS);

	if (b_en) {
		trng_ctrlreg |= RAND_CASR_EN;
	} else {
		trng_ctrlreg &= ~(RAND_CASR_EN);
	}

	TRNG_SETREG(TRNG_CONTROL_REG_OFS, trng_ctrlreg);
	DBG_IND("trng_set_casr %d\r\n", b_en);
}

/**
    Enable/Disable Post processing.

    TDES is embedded for TRNG post-prossing. Enable/disable this function

    @param[in] b_en  enable/disable for the post processing
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
static void trng_set_postproc(BOOL b_en)
{
	uint32_t trng_ctrlreg;

	trng_ctrlreg = TRNG_GETREG(TRNG_CONTROL_REG_OFS);

	if (b_en) {
		trng_ctrlreg |= POST_PROC_EN;
	} else {
		trng_ctrlreg &= ~(POST_PROC_EN);
	}

	TRNG_SETREG(TRNG_CONTROL_REG_OFS, trng_ctrlreg);
	DBG_IND("trng_set_postproc %d\r\n", b_en);
}


static void nvt_trng_reset(void)
{
	pll_set_clock_rate(PLL_CLKSEL_TRNG, PLL_CLKSEL_TRNG_160);
	pll_set_clock_rate(PLL_CLKSEL_TRNGRO_CLKSRC, PLL_CLKSEL_TRNGRO_CLKSRC_ROSC);

	pll_enable_system_reset(TRNG_RSTN);
	pll_disable_system_reset(TRNG_RSTN);

	pll_enable_clock(TRNG_CLK);
	
#if defined(TRNG_USE_STBC_CLK)
	pll_set_STBC_pll_enable((CG_EN)TRNG_RO_CLK_DEF, ENABLE);
#else
	pll_enable_clock((CG_EN)TRNG_RO_CLK_DEF);
#endif

	trng_set_casr(ENABLE);
	trng_set_postproc(ENABLE);
}


uint8_t hw_get_random_byte(void)
{
	return (uint8_t)TRNG_GETREG(TRNG_RANDNUM_REG_OFS);
}


TEE_Result crypto_rng_init(const void *data __unused,
				  size_t dlen __unused)
{
	nvt_trng_mmu = (vaddr_t)phys_to_virt(IOADDR_TRNG_REG_BASE, MEM_AREA_IO_SEC, 0x100);
	nvt_trng_nommu = ((paddr_t)IOADDR_TRNG_REG_BASE);

	nvt_trng_reset();

	DMSG("crypto_rng_init done\n");

	return TEE_SUCCESS;
}


void crypto_rng_add_event(enum crypto_rng_src sid __unused,
				 unsigned int *pnum __unused,
				 const void *data __unused,
				 size_t dlen __unused)
{
	DMSG("crypto_rng_add_event\n");
}

TEE_Result crypto_rng_read(void *buf, size_t blen)
{
	uint8_t *b = buf;
	size_t n;
	if (!b)
		return TEE_ERROR_BAD_PARAMETERS;

	for (n = 0; n < blen; n++)
		b[n] = hw_get_random_byte();

	return TEE_SUCCESS;
}



//service_init(nvt_trng_init);
