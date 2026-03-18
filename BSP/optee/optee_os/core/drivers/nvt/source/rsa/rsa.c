/*
    RSA Engine driver

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include <string.h>
#include <platform_config.h>
#include <initcall.h>
#include <io.h>
#include <kernel/interrupt.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <tee/cache.h>

#include <kwrap/debug.h>
#include "plat/pll.h"
#include "plat/interrupt.h"
#include "plat/nvt-sramctl.h"
#include "plat/rsa.h"
#include "pll_protected.h"
#include "rsa_reg.h"
#include "rsa_int.h"
#include "rsa_platform.h"

#include "crypto_protected.h"
#include "nvt_sce_cfg.h"

//#define RSA_INTERRUPT_MODE
#define FLGPTN_RSA              FLGPTN_BIT(0)
#define MODULE_FLAG_PTN         FLGPTN_RSA

#ifdef RSA4096_SUPPORT
#define RSA_SRAM_BYTE_SIZE      512             ///< 4096/8
#else
#define RSA_SRAM_BYTE_SIZE      256             ///< 2048/8
#endif
#define RSA_SRAM_WORD_SIZE      (RSA_SRAM_BYTE_SIZE/4)

static  RSA_CLKSEL  uiRSAClockRate = RSA_CLKSEL0;
static  BOOL    bRSAOpened     = FALSE;
static  BOOL    bRSASem        = FALSE;
static  UINT32  uiRSAKey       = RSA_KEY_1024;
static  UINT32  uiRSAmode      = RSA_MODE_NORMAL;

static vaddr_t RSA_ADDR_VBASE = 0;

#define RSA_SETREG(_ofs, _val)   io_write32(RSA_ADDR_VBASE+(_ofs), (_val))
#define RSA_GETREG(_ofs)         io_read32(RSA_ADDR_VBASE+(_ofs))

#ifdef RSA_INTERRUPT_MODE
static ID FLG_ID_RSA;
static struct itr_handler rsa_it_handler;

static enum itr_return rsa_isr(struct itr_handler *h __unused)
{
	T_RSA_INTSTS_REG RsaINTSTS  = {0x00000000};

	RsaINTSTS.reg = RSA_GETREG(RSA_INTSTS_REG_OFS);

	if (RsaINTSTS.bit.RSAED_STS) {
		/* Write 1 Clear interrupt status */
		RSA_SETREG(RSA_INTSTS_REG_OFS, RsaINTSTS.reg);
	}

	/* Set flag */
	iset_flg(FLG_ID_RSA, MODULE_FLAG_PTN);

	return IRQ_HANDLED;
}
#endif

static ER rsa_lock(void)
{
	ER ret;

	if ((ret = rsa_platform_sem_wait()) == E_OK) {
		bRSASem = TRUE;
	}
	return ret;
}

static ER rsa_unlock(void)
{
	if (bRSASem == TRUE) {
		bRSASem = FALSE;
		return rsa_platform_sem_signal();
	}
	else {
		return E_SYS;
	}
}

ER rsa_open(void)
{
	ER ret;

	if ((ret = rsa_lock()) != E_OK) {
		return ret;
	}

	bRSAOpened = TRUE;

	return E_OK;
}

ER rsa_close(void)
{
	ER ret;

	if (!bRSAOpened)
		return E_OK;

	bRSAOpened = FALSE;

	ret = rsa_unlock();

	return ret;
}

ER rsa_setConfig(RSA_CONFIG_ID cfgid, UINT32 cfgvalue)
{
	ER ret = E_NOSPT;
	T_RSA_CONFIG_REG RsaConfig = {0x00000000};

	if (!bRSAOpened) {
		DBG_ERR("RSA engine not open\r\n");
		return ret;
	}

	switch (cfgid) {
#ifdef RSA_CLOCK_RATE_SUPPORT
		case RSA_CONFIG_ID_CLOCK_RATE:
			{
				switch (cfgvalue) {
					case RSA_CLKSEL0:
						uiRSAClockRate = cfgvalue;
						ret = E_OK;
					break;

					case RSA_CLKSEL1:
						uiRSAClockRate = cfgvalue;
						ret = E_OK;
					break;

					case RSA_CLKSEL2:
						if (RSA_CLKSEL0 != PLL_NOT_SUPPORT) {
							uiRSAClockRate = cfgvalue;
							ret = E_OK;
						} else
							DBG_ERR("cfgvalue = %d not support", (int)cfgvalue);
					break;

					default:
						DBG_ERR("cfgvalue = %d not support", (int)cfgvalue);
					break;
				}
			}
			break;
#endif

		case RSA_CONFIG_ID_KEY_WIDTH:
			{
				if (cfgvalue >= RSA_KEY_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

#ifndef RSA4096_SUPPORT
				if (cfgvalue == RSA_KEY_4096) {
					DBG_ERR("RSA key width 4096 not support\r\n");
					break;
				}
#endif
				uiRSAKey = cfgvalue;

				RsaConfig.reg = RSA_GETREG(RSA_CONFIG_REG_OFS);
				RsaConfig.bit.RSA_KEY_WIDTH = uiRSAKey;
				RSA_SETREG(RSA_CONFIG_REG_OFS, RsaConfig.reg);

				ret = E_OK;
			}
			break;

		case RSA_CONFIG_ID_MODE:
			{
				if (cfgvalue >= RSA_MODE_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				uiRSAmode = cfgvalue;

				RsaConfig.reg = RSA_GETREG(RSA_CONFIG_REG_OFS);
				RsaConfig.bit.RSA_MODE = uiRSAmode;
				RSA_SETREG(RSA_CONFIG_REG_OFS, RsaConfig.reg);

				ret = E_OK;
			}
			break;

		case RSA_CONFIG_ID_CRC32_POLY_INIT:
			{
				RSA_SETREG(RSA_CRC32_DEFAULT_REG_OFS, cfgvalue);
				ret = E_OK;
			}
			break;

		case RSA_CONFIG_ID_CRC32_POLY_PARAM:
			{
				RSA_SETREG(RSA_CRC32_POLY_REG_OFS, cfgvalue);
				ret = E_OK;
			}
			break;

		case RSA_CONFIG_ID_REORDER:
			{
				if (cfgvalue >= RSA_ORDER_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				RsaConfig.reg = RSA_GETREG(RSA_CONFIG_REG_OFS);
				RsaConfig.bit.RSA_BYTEODR = (cfgvalue == RSA_ORDER_LSB_FIRST) ? 0 : 1;
				RSA_SETREG(RSA_CONFIG_REG_OFS, RsaConfig.reg);

				ret = E_OK;
			}
			break;

		default:
			ret = E_NOSPT;
			break;
	}
	return ret;
}

void rsa_setkey_n(UINT32 *key, UINT32 len, UINT32 msb_first)
{
	UINT32 i;

	if (!bRSAOpened) {
		DBG_ERR("RSA engine not open\r\n");
		return;
	}

	if (!len || len > RSA_SRAM_WORD_SIZE) {
		DBG_ERR("key_n length = %d(word) not support\r\n", (int)len);
		return;
	}

	/* Set KEY Address to zero */
	RSA_SETREG(RSA_KEY_N_ADDR_REG_OFS, 0x00000000);

	if (msb_first) {
		for(i=len;i>0;i--) {
			RSA_SETREG(RSA_KEY_N_REG_OFS, key[i-1]);
		}
	}
	else {
		for(i=0;i<len;i++) {
			RSA_SETREG(RSA_KEY_N_REG_OFS, (key[i]));
		}
	}
	for(i=0;i<(RSA_SRAM_WORD_SIZE-len);i++) {
		RSA_SETREG(RSA_KEY_N_REG_OFS, 0x00000000);
	}
}

void rsa_setkey_ed(UINT32 *key, UINT32 len, UINT32 msb_first)
{
	UINT32 i;

	if (!bRSAOpened) {
		DBG_ERR("RSA engine not open\r\n");
		return;
	}

	if (!len || len > RSA_SRAM_WORD_SIZE) {
		DBG_ERR("key_ed length = %d(word) not support\r\n", (int)len);
		return;
	}

	/* Set KEY Address to zero */
	RSA_SETREG(RSA_KEY_ED_ADDR_REG_OFS, 0x00000000);

	if (msb_first) {
		for (i=len;i>0;i--) {
			RSA_SETREG(RSA_KEY_ED_REG_OFS, key[i-1]);
		}
	}
	else {
		for(i=0;i<len;i++) {
			RSA_SETREG(RSA_KEY_ED_REG_OFS, (key[i]));
		}
	}
	for(i=0;i<(RSA_SRAM_WORD_SIZE-len);i++) {
		RSA_SETREG(RSA_KEY_ED_REG_OFS, 0x00000000);
	}
}

void rsa_pio_enable(UINT32 *data, UINT32 len, UINT32 msb_first)
{
#ifdef RSA_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
	UINT32 busy_count = 0;
#endif
	UINT32 i;
	T_RSA_CONTROL_REG rsa_ctrl    = {0x00000000};
	T_RSA_INTEN_REG   rsa_int_en  = {0x00000000};
	T_RSA_INTSTS_REG  rsa_int_sts = {0x00000000};

	if (!bRSAOpened) {
		DBG_ERR("RSA engine not open\r\n");
		return;
	}

	if (!len || len > RSA_SRAM_WORD_SIZE) {
		DBG_ERR("data length = %d(word) not support\r\n", (int)len);
		return;
	}

	/* Set interrupt enable */
	rsa_int_en.reg = RSA_GETREG(RSA_INTEN_REG_OFS);
	rsa_int_en.bit.RSAED_INTEN = 1;
	RSA_SETREG(RSA_INTEN_REG_OFS, rsa_int_en.reg);

	/* Clear interrupt status */
	rsa_int_sts.reg = RSA_GETREG(RSA_INTSTS_REG_OFS);
	rsa_int_sts.bit.RSAED_STS = 1;
	RSA_SETREG(RSA_INTSTS_REG_OFS, rsa_int_sts.reg);

#ifdef RSA_INTERRUPT_MODE
	/* Clear flag */
	clr_flg(FLG_ID_RSA, MODULE_FLAG_PTN);
#endif

	/* Set DATA Address to zero */
	RSA_SETREG(RSA_DATA_ADDR_REG_OFS, 0x00000000);

	if (msb_first) {
		for (i=len;i>0;i--) {
			RSA_SETREG(RSA_DATA_REG_OFS, data[i-1]);
		}
	}
	else {
		for (i=0;i<len;i++) {
			RSA_SETREG(RSA_DATA_REG_OFS, (data[i]));
		}
	}
	for (i=0;i<(RSA_SRAM_WORD_SIZE-len);i++) {
		RSA_SETREG(RSA_DATA_REG_OFS, 0x00000000);
	}

	/* Set RSA_EN Enable */
	rsa_ctrl.reg = RSA_GETREG(RSA_CONTROL_REG_OFS);
	rsa_ctrl.bit.RSA_EN = 1;
	RSA_SETREG(RSA_CONTROL_REG_OFS, rsa_ctrl.reg);

	/* Wait for PIO transmitted */
#ifdef RSA_INTERRUPT_MODE
	wai_flg(&FlagPtn, FLG_ID_RSA, MODULE_FLAG_PTN, TWF_ORW | TWF_CLR);
#else
	while (1) {
		rsa_int_sts.reg = RSA_GETREG(RSA_INTSTS_REG_OFS);

		if (rsa_int_sts.bit.RSAED_STS) {
			/* Write 1 Clear interrupt status */
			RSA_SETREG(RSA_INTSTS_REG_OFS, rsa_int_sts.reg);
			break;
		}

		busy_count++;
		if (busy_count > 100000000) {
			DBG_ERR("rsa busy_count = 0x%x\r\n", busy_count);
		}
	}
#endif
}

void rsa_getOutput(UINT32 *Output, UINT32 len)
{
	UINT32 i;

	if (!bRSAOpened) {
		DBG_ERR("RSA engine not open\r\n");
		return;
	}

	if (!len || len > RSA_SRAM_WORD_SIZE) {
		DBG_ERR("data length = %d(word) not support\r\n", (int)len);
		return;
	}

	/* Set DATA Address to zero */
	RSA_SETREG(RSA_DATA_ADDR_REG_OFS, 0x00000000);

	for (i=0;i<len;i++) {
		Output[i] = RSA_GETREG(RSA_DATA_REG_OFS);
	}
}

static void rsa_platform_init(void)
{
	/* Disable rsa Module SRAM Shut Down */
	nvt_disable_sram_shutdown(RSA_SD);

#if !defined(PLL_FREE)
	PLL_EN rsa_pll = PLLEN0;
#endif

	pll_set_clock_rate(PLL_CLKSEL_RSA, (UINT32)uiRSAClockRate);

#if !defined(PLL_FREE)
#if defined(SCE_USE_STBC_CLK)
	pll_set_STBC_pll_enable(rsa_pll, 1);
#else
	pll_set_pll_enable(rsa_pll, 1);
#endif
#endif

	/* disabel RSA module reset */
	pll_disable_system_reset(RSA_RSTN);

	/* enable RSA module master clock */
	pll_enable_clock(RSA_CLK);
}

static TEE_Result nvt_rsa_init(void)
{
	/* get register access base */
	RSA_ADDR_VBASE = (vaddr_t)phys_to_virt(IOADDR_RSA_REG_BASE, MEM_AREA_IO_SEC, 0x100);

	/* rsa clock and reset init */
	rsa_platform_init();

	/* create platform resource */
	rsa_platform_create_resource();

#ifdef RSA_INTERRUPT_MODE
	/* create flag */
	cre_flg(&FLG_ID_RSA, NULL, (char *)"FLG_ID_RSA");
	clr_flg(FLG_ID_RSA, 0xFFFFFFFF);

	rsa_it_handler.it      = INT_ID_RSA;
	rsa_it_handler.flags   = ITRF_TRIGGER_LEVEL;
	rsa_it_handler.handler = rsa_isr;
	rsa_it_handler.data    = NULL;

	itr_add(&rsa_it_handler);
	itr_enable(rsa_it_handler.it);
#endif

	return TEE_SUCCESS;
}

driver_init(nvt_rsa_init);
