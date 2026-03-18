/*
    Crypto Engine driver

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
#include "plat/crypto.h"
#include "pll_protected.h"
#include "crypto_protected.h"
#include "crypto_reg.h"
#include "crypto_int.h"
#include "crypto_platform.h"

#include "nvt_sce_cfg.h"

//#define CRYPTO_INTERRUPT_MODE
#define FLGPTN_CRYPTO               FLGPTN_BIT(0)
#define MODULE_FLAG_PTN             FLGPTN_CRYPTO

#define CACHE_LINE_ALIGN            STACK_ALIGNMENT     ///< system cache line 64 or 32, na51090 is ca53 => 64
#define CACHE_LINE_MASK             (CACHE_LINE_ALIGN-1)

#define DMA_ADDR_LO(_addr)          ((UINT32)((UINT64)(_addr)))
#define DMA_ADDR_HI(_addr)          ((UINT32)(((UINT64)(_addr))>>32))

#define DES_BLOCK_SIZE              8
#define AES_BLOCK_SIZE              16

/* DMA Descriptor */
#define SCE_DES_HEADER_SIZE         (32)               ///< word size
#define SCE_DES_BLOCK_SIZE          (4)                ///< word size
#define SCE_DES_BLOCK_NUM_MAX       (32)               ///< block config maximun number, hardware max is 256
#define SCE_DES_TABLE_NUM           (SCE_DES_HEADER_SIZE+(SCE_DES_BLOCK_NUM_MAX*SCE_DES_BLOCK_SIZE))

#define SCE_DES_KEY_OFS             (0)
#define SCE_DES_IV_OFS              (8)
#define SCE_DES_CNT_OFS             (12)
#define SCE_DES_HAEDER_OFS          (16)
#define SCE_DES_CV_OFS              (20)
#define SCE_DES_S0_OFS              (24)
#define SCE_DES_GHASH_OFS           (28)
#define SCE_DES_BLOCK_CFG_OFS       (32)

static  CRYPTO_CLKSEL	uiCryptoClockRate = CRYPTO_CLKSEL0;

static  BOOL         bCryptoOpened     = FALSE;
static  BOOL         bCryptoSem        = FALSE;
static  BOOL         bType             = CRYPTO_TYPE_ENCRYPT;
static  BOOL         bCryptoMode       = FALSE;
static  CRYPTO_MODE  uiMode            = CRYPTO_MODE_AES128;
static  BOOL         bPIOEn            = FALSE;
static  BOOL         bCH0En            = FALSE;
static  BOOL         bCH1En            = FALSE;
static  BOOL         bCH2En            = FALSE;
static  BOOL         bCH3En            = FALSE;

static vaddr_t CRYPTO_ADDR_VBASE = 0;
static _ALIGNED(CACHE_LINE_ALIGN) UINT32 vuiSCE_DesTab[SCE_DMA_COUNT][SCE_DES_TABLE_NUM];

#define CRYPTO_SETREG(_ofs, _val)           io_write32(CRYPTO_ADDR_VBASE+(_ofs), (_val))
#define CRYPTO_GETREG(_ofs)                 io_read32(CRYPTO_ADDR_VBASE+(_ofs))

#define dma_getPhyAddr(_va)                 virt_to_phys((void *)_va)
#define dma_flushWriteCache(_va, _len)      cache_operation(TEE_CACHECLEAN,      (void *)(_va), (_len))
#define dma_flushReadCache(_va, _len)       cache_operation(TEE_CACHEFLUSH,      (void *)(_va), (_len)) ///< use CACHEFLUSH instead of CACHEINVALIDATE, to avoid non-cache alignment invalid cache would cause stack corrupt
#define dma_flushReadCacheDmaEnd(_va, _len) cache_operation(TEE_CACHEINVALIDATE, (void *)(_va), (_len))

#if defined(HOST_AXI_CH)
static void CRYPTOHostAXIChannelEnable(void)
{
	T_CRYPTO_DMA_BUS_CTRL_REG axi_ch_reg = {0};

	axi_ch_reg.reg = CRYPTO_GETREG(CRYPTO_DMA_BUS_CTRL_REG_OFS);
	axi_ch_reg.bit.MONI_CH_DIS = 0;
	CRYPTO_SETREG(CRYPTO_DMA_BUS_CTRL_REG_OFS, axi_ch_reg.reg);

}

static void CRYPTOHostAXIChannelDisable(void)
{
	T_CRYPTO_DMA_BUS_CTRL_REG axi_ch_reg = {0};

	axi_ch_reg.reg = CRYPTO_GETREG(CRYPTO_DMA_BUS_CTRL_REG_OFS);
	axi_ch_reg.bit.MONI_CH_DIS = 1;
	CRYPTO_SETREG(CRYPTO_DMA_BUS_CTRL_REG_OFS, axi_ch_reg.reg);
}
#endif

#ifdef CRYPTO_INTERRUPT_MODE
static ID FLG_ID_CRYPTO;
static struct itr_handler crypto_it_handler;

static enum itr_return crypto_isr(struct itr_handler *h __unused)
{
	int set_flag = 0;
	T_CRYPTO_INT_STS_REG intSts = {0};
	T_CRYPTO_CONTROL_REG RegCryotoCtrl = {0x00000000};

	DBG_ERR("Crypto interrupt\r\n");

	/* read interrupt status */
	intSts.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);

	/* read control register */
	RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);

	/* clear interrupt status */
	CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, intSts.reg);

	/* PIO */
	if (bPIOEn && intSts.bit.PIO_DONE_STS) {
		bPIOEn = 0;
		set_flag++;
		if (RegCryotoCtrl.bit.PIO_EN) {
			DBG_DUMP("PIO_EN unclear\r\n");
		}
	}

	/* DMA_CH#0 */
	if (bCH0En && intSts.bit.CH0_DONE_STS) {
		bCH0En = 0;
		set_flag++;
		if (RegCryotoCtrl.bit.DMA_CH0_EN) {
			DBG_DUMP("DMA_CH0_EN unclear\r\n");
		}
	}

	/* DMA_CH#1 */
	if (bCH1En && intSts.bit.CH1_DONE_STS) {
		bCH1En = 0;
		set_flag++;
		if (RegCryotoCtrl.bit.DMA_CH1_EN) {
			DBG_DUMP("DMA_CH1_EN unclear\r\n");
		}
	}

	/* DMA_CH#2 */
	if (bCH2En && intSts.bit.CH2_DONE_STS) {
		bCH2En = 0;
		set_flag++;
		if (RegCryotoCtrl.bit.DMA_CH2_EN) {
			DBG_DUMP("CH2_EN unclear\r\n");
		}
	}

	/* DMA_CH#3 */
	if (bCH3En && intSts.bit.CH3_DONE_STS) {
		bCH3En = 0;
		set_flag++;
		if (RegCryotoCtrl.bit.DMA_CH3_EN) {
			DBG_DUMP("CH3_EN unclear\r\n");
		}
	}

	/* issue flag */
	if (set_flag) {
		DBG_ERR("Crypto set_flag\r\n");
		iset_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
	}

	return ITRR_HANDLED;
}
#endif

static ER crypto_lock(void)
{
	ER ret;

	if ((ret = crypto_platform_sem_wait()) == E_OK) {
		bCryptoSem = TRUE;
	}
	return ret;
}

static ER crypto_unlock(void)
{
	if (bCryptoSem == TRUE) {
		bCryptoSem = FALSE;
		crypto_platform_sem_signal();
		return E_OK;
	} else {
		return E_SYS;
	}
}

ER crypto_setConfig(CRYPTO_CONFIG_ID cfgid, UINT32 cfgvalue)
{
	ER ret = E_NOSPT;
	T_CRYPTO_CONFIG_REG CryotoConfig = {0x00000000};

	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return ret;
	}

	switch (cfgid) {
#ifdef CRYPTO_CLOCK_RATE_SUPPORT
	case CRYPTO_CONFIG_ID_CLOCK_RATE: {
			switch (cfgvalue) {
				case CRYPTO_CLKSEL0:
					uiCryptoClockRate = cfgvalue;
					ret = E_OK;
				break;

				case CRYPTO_CLKSEL1:
					uiCryptoClockRate = cfgvalue;
					ret = E_OK;
				break;

				case CRYPTO_CLKSEL2:
					if (CRYPTO_CLKSEL2 != PLL_NOT_SUPPORT) {
						uiCryptoClockRate = cfgvalue;
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

	case CRYPTO_CONFIG_ID_MODE: {
			if (cfgvalue >= CRYPTO_MODE_NUM) {
				DBG_ERR("param err -> op(%d) param(%d)", (int)cfgid, (int)cfgvalue);
				break;
			}

			uiMode = cfgvalue;

			CryotoConfig.reg = CRYPTO_GETREG(CRYPTO_CONFIG_REG_OFS);
			CryotoConfig.bit.MODE = uiMode;
			CRYPTO_SETREG(CRYPTO_CONFIG_REG_OFS, CryotoConfig.reg);
			bCryptoMode = TRUE;

			ret = E_OK;
		}
		break;

	case CRYPTO_CONFIG_ID_TYPE: {
			if (cfgvalue >= CRYPTO_TYPE_NUM) {
				DBG_ERR("param err -> op(%d) param(%d)", (int)cfgid, (int)cfgvalue);
				break;
			}

			bType = cfgvalue;

			CryotoConfig.reg = CRYPTO_GETREG(CRYPTO_CONFIG_REG_OFS);
			CryotoConfig.bit.TYPE = bType;
			CRYPTO_SETREG(CRYPTO_CONFIG_REG_OFS, CryotoConfig.reg);

			ret = E_OK;
		}
		break;

	default:
		break;
	}

	return ret;
}

ER crypto_open(void)
{
	ER ret;
	T_CRYPTO_CONFIG_REG  RegCryotoConfig = {0x00000000};

	if ((ret = crypto_lock()) != E_OK) {
		return ret;
	}

	bCryptoOpened = TRUE;

	/* Set CRYPTO_EN Enable */
	RegCryotoConfig.reg = CRYPTO_GETREG(CRYPTO_CONFIG_REG_OFS);
	RegCryotoConfig.bit.CRYPTO_EN = 1;
	CRYPTO_SETREG(CRYPTO_CONFIG_REG_OFS, RegCryotoConfig.reg);

	return E_OK;
}

ER crypto_close(void)
{
	ER ret;
	T_CRYPTO_CONFIG_REG  RegCryotoConfig = {0x00000000};

	if (!bCryptoOpened) {
		return E_OK;
	}

	bCryptoOpened = FALSE;
	bCryptoMode   = FALSE;

	/* Set CRYPTO_EN Enable */
	RegCryotoConfig.reg = CRYPTO_GETREG(CRYPTO_CONFIG_REG_OFS);
	RegCryotoConfig.bit.CRYPTO_EN = 0;
	CRYPTO_SETREG(CRYPTO_CONFIG_REG_OFS, RegCryotoConfig.reg);

	ret = crypto_unlock();

	return ret;
}

void crypto_setKey(UINT8 *ucKey)
{
	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	if (!bCryptoMode) {
		DBG_ERR("Pls use set Mode first.\r\n");
		return;
	}

	switch (uiMode) {
	case CRYPTO_MODE_DES:
		CRYPTO_SETREG(CRYPTO_KEY0_REG_OFS, MKWORD(ucKey + 0));
		CRYPTO_SETREG(CRYPTO_KEY1_REG_OFS, MKWORD(ucKey + 4));
		break;
	case CRYPTO_MODE_3DES:
		CRYPTO_SETREG(CRYPTO_KEY0_REG_OFS, MKWORD(ucKey + 0));
		CRYPTO_SETREG(CRYPTO_KEY1_REG_OFS, MKWORD(ucKey + 4));
		CRYPTO_SETREG(CRYPTO_KEY2_REG_OFS, MKWORD(ucKey + 8));
		CRYPTO_SETREG(CRYPTO_KEY3_REG_OFS, MKWORD(ucKey + 12));
		CRYPTO_SETREG(CRYPTO_KEY4_REG_OFS, MKWORD(ucKey + 16));
		CRYPTO_SETREG(CRYPTO_KEY5_REG_OFS, MKWORD(ucKey + 20));
		break;
	case CRYPTO_MODE_AES128:
		CRYPTO_SETREG(CRYPTO_KEY0_REG_OFS, MKWORD(ucKey + 0));
		CRYPTO_SETREG(CRYPTO_KEY1_REG_OFS, MKWORD(ucKey + 4));
		CRYPTO_SETREG(CRYPTO_KEY2_REG_OFS, MKWORD(ucKey + 8));
		CRYPTO_SETREG(CRYPTO_KEY3_REG_OFS, MKWORD(ucKey + 12));
		break;
	case CRYPTO_MODE_AES256:
		CRYPTO_SETREG(CRYPTO_KEY0_REG_OFS, MKWORD(ucKey + 0));
		CRYPTO_SETREG(CRYPTO_KEY1_REG_OFS, MKWORD(ucKey + 4));
		CRYPTO_SETREG(CRYPTO_KEY2_REG_OFS, MKWORD(ucKey + 8));
		CRYPTO_SETREG(CRYPTO_KEY3_REG_OFS, MKWORD(ucKey + 12));
		CRYPTO_SETREG(CRYPTO_KEY4_REG_OFS, MKWORD(ucKey + 16));
		CRYPTO_SETREG(CRYPTO_KEY5_REG_OFS, MKWORD(ucKey + 20));
		CRYPTO_SETREG(CRYPTO_KEY6_REG_OFS, MKWORD(ucKey + 24));
		CRYPTO_SETREG(CRYPTO_KEY7_REG_OFS, MKWORD(ucKey + 28));
		break;
	default:
		DBG_ERR("uiMode=%d not supported!\r\n", uiMode);
		break;
	}
}

void crypto_setInput(UINT8 *ucInput)
{
	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	if (!bCryptoMode) {
		DBG_ERR("Pls use set Mode first.\r\n");
		return;
	}

	switch (uiMode) {
	case CRYPTO_MODE_DES:
	case CRYPTO_MODE_3DES:
		CRYPTO_SETREG(CRYPTO_INPUT_0_REG_OFS, MKWORD(ucInput + 0));
		CRYPTO_SETREG(CRYPTO_INPUT_1_REG_OFS, MKWORD(ucInput + 4));
		break;
	case CRYPTO_MODE_AES128:
	case CRYPTO_MODE_AES256:
		CRYPTO_SETREG(CRYPTO_INPUT_0_REG_OFS, MKWORD(ucInput + 0));
		CRYPTO_SETREG(CRYPTO_INPUT_1_REG_OFS, MKWORD(ucInput + 4));
		CRYPTO_SETREG(CRYPTO_INPUT_2_REG_OFS, MKWORD(ucInput + 8));
		CRYPTO_SETREG(CRYPTO_INPUT_3_REG_OFS, MKWORD(ucInput + 12));
		break;
	default:
		DBG_ERR("uiMode=%d not supported!\r\n", uiMode);
		break;
	}
}

void crypto_getOutput(UINT8 *Output)
{
	UINT32 value[4];

	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	if (!bCryptoMode) {
		DBG_WRN("Pls use set Mode first.\r\n");
		return;
	}

	switch (uiMode) {
	case CRYPTO_MODE_DES:
	case CRYPTO_MODE_3DES:
		value[0] = CRYPTO_GETREG(CRYPTO_OUTPUT_0_REG_OFS);
		value[1] = CRYPTO_GETREG(CRYPTO_OUTPUT_1_REG_OFS);

		memcpy(Output, value, 8);
		break;
	case CRYPTO_MODE_AES128:
	case CRYPTO_MODE_AES256:
		value[0] = CRYPTO_GETREG(CRYPTO_OUTPUT_0_REG_OFS);
		value[1] = CRYPTO_GETREG(CRYPTO_OUTPUT_1_REG_OFS);
		value[2] = CRYPTO_GETREG(CRYPTO_OUTPUT_2_REG_OFS);
		value[3] = CRYPTO_GETREG(CRYPTO_OUTPUT_3_REG_OFS);

		memcpy(Output, value, 16);
		break;
	default:
		DBG_ERR("uiMode=%d not supported!\r\n", uiMode);
		break;
	}
}

void crypto_pio_enable(void)
{
#ifdef CRYPTO_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
	UINT32 busy_count = 0;
#endif
	T_CRYPTO_CONTROL_REG  RegCryotoCtrl = {0x00000000};
	T_CRYPTO_INT_EN_REG   RegINTEN      = {0x00000000};
	T_CRYPTO_INT_STS_REG  RegINTSTS     = {0x00000000};

	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	bPIOEn = TRUE;

	/* Set interrupt enable */
	RegINTEN.reg = CRYPTO_GETREG(CRYPTO_INT_EN_REG_OFS);
	RegINTEN.bit.PIO_DONE_EN = 1;
	CRYPTO_SETREG(CRYPTO_INT_EN_REG_OFS, RegINTEN.reg);

	/* Clear interrupt status */
	RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);
	RegINTSTS.bit.PIO_DONE_STS = 1;
	CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);

#ifdef CRYPTO_INTERRUPT_MODE
	/* Clear flag */
	clr_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
#endif

	/* Set PIO Enable */
	RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);
	RegCryotoCtrl.bit.PIO_EN = 1;
	CRYPTO_SETREG(CRYPTO_CONTROL_REG_OFS, RegCryotoCtrl.reg);

#ifdef CRYPTO_INTERRUPT_MODE
	/* Wait for PIO transmitted */
	wai_flg(&FlagPtn, FLG_ID_CRYPTO, MODULE_FLAG_PTN, TWF_ORW | TWF_CLR);
#else
	/* polling to check dma channel done */
	while (1) {
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);

		if (bPIOEn && RegINTSTS.bit.PIO_DONE_STS) {
			// Write 1 Clear interrupt status
			CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);
			bPIOEn = 0;
			return;
		}

		busy_count++;
		if (busy_count > 10000000) {
			DBG_ERR("Crypto PIO busy_count = %u\r\n", busy_count);
		}
	}
#endif
}

static void crypto_dma_enable(UINT32 id, vaddr_t addr, UINT32 blk_inten)
{
#ifdef CRYPTO_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
	UINT32 busy_count = 0;
#endif
	T_CRYPTO_CONTROL_REG      RegCryotoCtrl = {0x00000000};
	T_CRYPTO_INT_EN_REG       RegINTEN      = {0x00000000};
	T_CRYPTO_INT_STS_REG      RegINTSTS     = {0x00000000};
	T_CRYPTO_DMA_CH0_ADDR_REG RegDMACh0     = {0x00000000};
	T_CRYPTO_DMA_CH1_ADDR_REG RegDMACh1     = {0x00000000};
	T_CRYPTO_DMA_CH2_ADDR_REG RegDMACh2     = {0x00000000};
	T_CRYPTO_DMA_CH3_ADDR_REG RegDMACh3     = {0x00000000};
#ifdef CRYPTO_DMA_36BIT_SUPPORT
	T_CRYPTO_DMA_CH0_ADDR_H_REG RegDMACh0_H = {0x00000000};
	T_CRYPTO_DMA_CH1_ADDR_H_REG RegDMACh1_H = {0x00000000};
	T_CRYPTO_DMA_CH2_ADDR_H_REG RegDMACh2_H = {0x00000000};
	T_CRYPTO_DMA_CH3_ADDR_H_REG RegDMACh3_H = {0x00000000};
#endif
	paddr_t desc_paddr;

	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	/* check DMA descriptor address alignment */
	desc_paddr = (paddr_t)dma_getPhyAddr(addr);
	if (((uintptr_t)desc_paddr) & 0x3) {
		DBG_ERR("Crypto DMA descriptor paddr=0x%08lx not word alignment\r\n", (uintptr_t)desc_paddr);
		return;
	}
#if defined(HOST_AXI_CH)
	//AXI channel enable
	CRYPTOHostAXIChannelEnable();
#endif
	switch (id) {
	case SCE_DMA_CH0:
		bCH0En = TRUE;

		/* Set interrupt enable */
		RegINTEN.reg = CRYPTO_GETREG(CRYPTO_INT_EN_REG_OFS);
		RegINTEN.bit.CH0_DONE_EN = 1;
		if (blk_inten) {
			RegINTEN.bit.CH0_BLKED_EN = 1;
		}
		CRYPTO_SETREG(CRYPTO_INT_EN_REG_OFS, RegINTEN.reg);

		/* Clear interrupt status */
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);
		RegINTSTS.bit.CH0_DONE_STS = 1;
		RegINTSTS.bit.CH0_BLKED_STS = 1;
		CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);

		/* Set DMA Descriptor Src Addr */
		RegDMACh0.reg = CRYPTO_GETREG(CRYPTO_DMA_CH0_ADDR_REG_OFS);
		RegDMACh0.bit.CH0_ADDR = DMA_ADDR_LO(desc_paddr);
		CRYPTO_SETREG(CRYPTO_DMA_CH0_ADDR_REG_OFS, RegDMACh0.reg);

#ifdef CRYPTO_DMA_36BIT_SUPPORT
		/* Set DMA Descriptor Src Addr_H */
		RegDMACh0_H.reg = CRYPTO_GETREG(CRYPTO_DMA_CH0_ADDR_H_REG_OFS);
		RegDMACh0_H.bit.CH0_ADDR_H = DMA_ADDR_HI(desc_paddr) & 0xff;
		CRYPTO_SETREG(CRYPTO_DMA_CH0_ADDR_H_REG_OFS, RegDMACh0_H.reg);
#endif

#ifdef CRYPTO_INTERRUPT_MODE
		/* Clear flag */
		clr_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
#endif

		/* Set DMA Enable */
		RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);
		RegCryotoCtrl.bit.DMA_CH0_EN = 1;
		CRYPTO_SETREG(CRYPTO_CONTROL_REG_OFS, RegCryotoCtrl.reg);
		break;
	case SCE_DMA_CH1:
		bCH1En = TRUE;

		/* Set interrupt enable */
		RegINTEN.reg = CRYPTO_GETREG(CRYPTO_INT_EN_REG_OFS);
		RegINTEN.bit.CH1_DONE_EN = 1;
		if (blk_inten) {
			RegINTEN.bit.CH1_BLKED_EN = 1;
		}
		CRYPTO_SETREG(CRYPTO_INT_EN_REG_OFS, RegINTEN.reg);

		/* Clear interrupt status */
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);
		RegINTSTS.bit.CH1_DONE_STS = 1;
		RegINTSTS.bit.CH1_BLKED_STS = 1;
		CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);

		/* Set DMA Descriptor Src Addr */
		RegDMACh1.reg = CRYPTO_GETREG(CRYPTO_DMA_CH1_ADDR_REG_OFS);
		RegDMACh1.bit.CH1_ADDR = DMA_ADDR_LO(desc_paddr);
		CRYPTO_SETREG(CRYPTO_DMA_CH1_ADDR_REG_OFS, RegDMACh1.reg);

#ifdef CRYPTO_DMA_36BIT_SUPPORT
		/* Set DMA Descriptor Src Addr_H */
		RegDMACh1_H.reg = CRYPTO_GETREG(CRYPTO_DMA_CH1_ADDR_H_REG_OFS);
		RegDMACh1_H.bit.CH1_ADDR_H = DMA_ADDR_HI(desc_paddr) & 0xff;
		CRYPTO_SETREG(CRYPTO_DMA_CH1_ADDR_H_REG_OFS, RegDMACh1_H.reg);
#endif

#ifdef CRYPTO_INTERRUPT_MODE
		/* Clear flag */
		clr_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
#endif

		/* Set DMA Enable */
		RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);
		RegCryotoCtrl.bit.DMA_CH1_EN = 1;
		CRYPTO_SETREG(CRYPTO_CONTROL_REG_OFS, RegCryotoCtrl.reg);
		break;
	case SCE_DMA_CH2:
		bCH2En = TRUE;

		/* Set interrupt enable */
		RegINTEN.reg = CRYPTO_GETREG(CRYPTO_INT_EN_REG_OFS);
		RegINTEN.bit.CH2_DONE_EN = 1;
		if (blk_inten) {
			RegINTEN.bit.CH2_BLKED_EN = 1;
		}
		CRYPTO_SETREG(CRYPTO_INT_EN_REG_OFS, RegINTEN.reg);

		/* Clear interrupt status */
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);
		RegINTSTS.bit.CH2_DONE_STS = 1;
		RegINTSTS.bit.CH2_BLKED_STS = 1;
		CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);

		/* Set DMA Descriptor Src Addr */
		RegDMACh2.reg = CRYPTO_GETREG(CRYPTO_DMA_CH2_ADDR_REG_OFS);
		RegDMACh2.bit.CH2_ADDR = DMA_ADDR_LO(desc_paddr);
		CRYPTO_SETREG(CRYPTO_DMA_CH2_ADDR_REG_OFS, RegDMACh2.reg);

#ifdef CRYPTO_DMA_36BIT_SUPPORT
		/* Set DMA Descriptor Src Addr_H */
		RegDMACh2_H.reg = CRYPTO_GETREG(CRYPTO_DMA_CH2_ADDR_H_REG_OFS);
		RegDMACh2_H.bit.CH2_ADDR_H = DMA_ADDR_HI(desc_paddr) & 0xff;
		CRYPTO_SETREG(CRYPTO_DMA_CH2_ADDR_H_REG_OFS, RegDMACh2_H.reg);
#endif

#ifdef CRYPTO_INTERRUPT_MODE
		/* Clear flag */
		clr_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
#endif

		/* Set DMA Enable */
		RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);
		RegCryotoCtrl.bit.DMA_CH2_EN = 1;
		CRYPTO_SETREG(CRYPTO_CONTROL_REG_OFS, RegCryotoCtrl.reg);
		break;
	case SCE_DMA_CH3:
		bCH3En = TRUE;

		/* Set interrupt enable */
		RegINTEN.reg = CRYPTO_GETREG(CRYPTO_INT_EN_REG_OFS);
		RegINTEN.bit.CH3_DONE_EN = 1;
		if (blk_inten) {
			RegINTEN.bit.CH3_BLKED_EN = 1;
		}
		CRYPTO_SETREG(CRYPTO_INT_EN_REG_OFS, RegINTEN.reg);

		/* Clear interrupt status */
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);
		RegINTSTS.bit.CH3_DONE_STS = 1;
		RegINTSTS.bit.CH3_BLKED_STS = 1;
		CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);

		/* Set DMA Descriptor Src Addr */
		RegDMACh3.reg = CRYPTO_GETREG(CRYPTO_DMA_CH3_ADDR_REG_OFS);
		RegDMACh3.bit.CH3_ADDR = DMA_ADDR_LO(desc_paddr);
		CRYPTO_SETREG(CRYPTO_DMA_CH3_ADDR_REG_OFS, RegDMACh3.reg);

#ifdef CRYPTO_DMA_36BIT_SUPPORT
		/* Set DMA Descriptor Src Addr_H */
		RegDMACh3_H.reg = CRYPTO_GETREG(CRYPTO_DMA_CH3_ADDR_H_REG_OFS);
		RegDMACh3_H.bit.CH3_ADDR_H = DMA_ADDR_HI(desc_paddr) & 0xff;
		CRYPTO_SETREG(CRYPTO_DMA_CH3_ADDR_H_REG_OFS, RegDMACh3_H.reg);
#endif

#ifdef CRYPTO_INTERRUPT_MODE
		/* Clear flag */
		clr_flg(FLG_ID_CRYPTO, MODULE_FLAG_PTN);
#endif

		/* Set DMA Enable */
		RegCryotoCtrl.reg = CRYPTO_GETREG(CRYPTO_CONTROL_REG_OFS);
		RegCryotoCtrl.bit.DMA_CH3_EN = 1;
		CRYPTO_SETREG(CRYPTO_CONTROL_REG_OFS, RegCryotoCtrl.reg);
		break;
	default:
		DBG_ERR("Crypto DMA_CH#%d not supported!\r\n", id);
		return;
	}

#ifdef CRYPTO_INTERRUPT_MODE
	wai_flg(&FlagPtn, FLG_ID_CRYPTO, MODULE_FLAG_PTN, TWF_ORW | TWF_CLR);
#else
	/* polling to check dma channel done */
	while (1) {
		RegINTSTS.reg = CRYPTO_GETREG(CRYPTO_INT_STS_REG_OFS);

		switch (id) {
		case SCE_DMA_CH0:
			if (bCH0En && RegINTSTS.bit.CH0_DONE_STS) {
				// Write 1 Clear interrupt status
				CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);
				bCH0En = 0;
				return;
			}
			break;
		case SCE_DMA_CH1:
			if (bCH1En && RegINTSTS.bit.CH1_DONE_STS) {
				// Write 1 Clear interrupt status
				CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);
				bCH1En = 0;
				return;
			}
			break;
		case SCE_DMA_CH2:
			if (bCH2En && RegINTSTS.bit.CH2_DONE_STS) {
				// Write 1 Clear interrupt status
				CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);
				bCH2En = 0;
				return;
			}
			break;
		case SCE_DMA_CH3:
			if (bCH3En && RegINTSTS.bit.CH3_DONE_STS) {
				// Write 1 Clear interrupt status
				CRYPTO_SETREG(CRYPTO_INT_STS_REG_OFS, RegINTSTS.reg);
				bCH3En = 0;
				return;
			}
			break;
		default:
			return;
		}

		busy_count++;
		if (busy_count > 10000000) {
			DBG_ERR("Crypto DMA_CH#%d busy_count = %u desc_addr = 0x%08lx\r\n", id, busy_count, (uintptr_t)addr);
		}
	}
#endif
#if defined(HOST_AXI_CH)
	//AXI channel enable
	CRYPTOHostAXIChannelDisable();
#endif
}

void crypto_dma_setDesTab(UINT32 id, pCRYPT_DMA_SET ptrDmaCrypt, pCRYPT_SEG_DES ptrSeg, UINT32 uiDesNum)
{
	UINT32 i;
	UINT32 block_size;
	UINT32 key_len       = 0;
	UINT32 header_config = 0;
	UINT32 block_config  = 0;
	UINT8 *ucKey = ptrDmaCrypt->key;
	UINT8 *ucIV  = ptrDmaCrypt->iv;
	UINT8 *ucCNT = ptrDmaCrypt->cnt;
	paddr_t src_paddr;
	paddr_t dst_paddr;

	if (!bCryptoOpened) {
		DBG_ERR("Crypto engine not open\r\n");
		return;
	}

	if (uiDesNum > SCE_DES_BLOCK_NUM_MAX) {
		DBG_ERR("Descriptor number=%d(MAX:%d)\r\n", (int)uiDesNum, SCE_DES_BLOCK_NUM_MAX);
		return;
	}

	memset((void *)&vuiSCE_DesTab[id][0], 0, SCE_DES_TABLE_NUM * sizeof(UINT32));
	dma_flushWriteCache((vaddr_t)&vuiSCE_DesTab[id][0], SCE_DES_TABLE_NUM * sizeof(UINT32));

	switch (ptrDmaCrypt->mode) {
	case CRYPTO_MODE_DES:
		key_len    = 2;
		block_size = DES_BLOCK_SIZE;
		break;
	case CRYPTO_MODE_3DES:
		key_len    = 6;
		block_size = DES_BLOCK_SIZE;
		break;
	case CRYPTO_MODE_AES128:
		key_len    = 4;
		block_size = AES_BLOCK_SIZE;
		break;
	case CRYPTO_MODE_AES256:
		key_len    = 8;
		block_size = AES_BLOCK_SIZE;
		break;
	default:
		DBG_ERR("Pls Set Crypto Mode\r\n");
		return;
	}

	/* Header, Key */
	for (i = 0; i < key_len; i++) {
		vuiSCE_DesTab[id][i + SCE_DES_KEY_OFS] = MKWORD(ucKey + i * 4);
	}
	/* Header, IV */
	for (i = 0; i < 4; i++) {
		vuiSCE_DesTab[id][i + SCE_DES_IV_OFS] = MKWORD(ucIV + i * 4);
	}
	/* Header, Count */
	for (i = 0; i < 4; i++) {
		vuiSCE_DesTab[id][i + SCE_DES_CNT_OFS] = MKWORD(ucCNT + i * 4);
	}

	header_config = 0;
	header_config = (UINT32)(((ptrDmaCrypt->get_s0) << 16) |
							 ((ptrDmaCrypt->key_src) << 12)         |
							 ((ptrDmaCrypt->opmode) << 8)           |
							 ((ptrDmaCrypt->mode) << 4)             |
							 (ptrDmaCrypt->type));
	vuiSCE_DesTab[id][SCE_DES_HAEDER_OFS] = (UINT32)header_config;

	/* Block Config */
	for (i = 0; i < uiDesNum; i++) {
		/* check source dma address align */
		src_paddr = (paddr_t)dma_getPhyAddr((vaddr_t)ptrSeg->src_addr);
		if (src_paddr & 0x3) {
			DBG_ERR("Des[%d] src_paddr=0x%08lx not word align\r\n", i, (uintptr_t)src_paddr);
			return;
		}

		/* check destination dma address align */
		dst_paddr = (paddr_t)dma_getPhyAddr((vaddr_t)ptrSeg->dst_addr);
		if (dst_paddr & 0x3) {
			DBG_ERR("Des[%d] dst_addr=0x%08lx not word align\r\n", i, (uintptr_t)dst_paddr);
			return;
		}

		/* check dma size block alignment */
		if (ptrSeg->len % block_size) {
			DBG_ERR("Des[%d] size=%d not block align(%d)\r\n", i, (int)ptrSeg->len, (int)block_size);
			return;
		}

		block_config = 0;
		vuiSCE_DesTab[id][i * SCE_DES_BLOCK_SIZE + SCE_DES_BLOCK_CFG_OFS + 0] = DMA_ADDR_LO(src_paddr);
		vuiSCE_DesTab[id][i * SCE_DES_BLOCK_SIZE + SCE_DES_BLOCK_CFG_OFS + 1] = DMA_ADDR_LO(dst_paddr);
		vuiSCE_DesTab[id][i * SCE_DES_BLOCK_SIZE + SCE_DES_BLOCK_CFG_OFS + 2] = (UINT32)ptrSeg->len;
		block_config = (UINT32)(((ptrSeg->non_flush) << 8) | ((ptrSeg->block_inten) << 4) | (ptrSeg->last_block));
#ifdef CRYPTO_DMA_36BIT_SUPPORT
		block_config |= (((DMA_ADDR_HI(src_paddr) & 0xff) << 16) |
						 ((DMA_ADDR_HI(dst_paddr) & 0xff) << 24));
#endif
		vuiSCE_DesTab[id][i * SCE_DES_BLOCK_SIZE + SCE_DES_BLOCK_CFG_OFS + 3] = (UINT32)block_config;

		dma_flushWriteCache((vaddr_t)&vuiSCE_DesTab[id][i * SCE_DES_BLOCK_SIZE + SCE_DES_BLOCK_CFG_OFS], 4*4);
		dma_flushWriteCache((vaddr_t)ptrSeg->src_addr, (UINT32)ptrSeg->len);
		dma_flushReadCache((vaddr_t)ptrSeg->dst_addr, (UINT32)ptrSeg->len);
		ptrSeg = (CRYPT_SEG_DES *)ptrSeg->pNext;
	}
	dma_flushWriteCache((vaddr_t)&vuiSCE_DesTab[id][0], SCE_DES_TABLE_NUM * sizeof(UINT32));
	dma_flushReadCache((vaddr_t)&vuiSCE_DesTab[id][0],  SCE_DES_TABLE_NUM * sizeof(UINT32));
	crypto_dma_enable(id, (vaddr_t)&vuiSCE_DesTab[id][0], 0);
	dma_flushReadCacheDmaEnd((vaddr_t)&vuiSCE_DesTab[id][0], SCE_DES_TABLE_NUM*4*4);

	memcpy(ptrDmaCrypt->s0,    &vuiSCE_DesTab[id][SCE_DES_S0_OFS],    16);
	memcpy(ptrDmaCrypt->tag,   &vuiSCE_DesTab[id][SCE_DES_S0_OFS],    16);
	memcpy(ptrDmaCrypt->ghash, &vuiSCE_DesTab[id][SCE_DES_GHASH_OFS], 16);
	memcpy(ptrDmaCrypt->cv,    &vuiSCE_DesTab[id][SCE_DES_CV_OFS],    16);

	if (ptrDmaCrypt->opmode == CRYPTO_OPMODE_GCM) {
		for (i = 0; i < 16; i++) {
			ptrDmaCrypt->tag[i] = ptrDmaCrypt->ghash[i] ^ ptrDmaCrypt->s0[i];
		}
	}
}

static void crypto_platform_init(void)
{
#if !defined(PLL_FREE)
	PLL_EN crypto_pll = PLLEN0;
#endif

	pll_set_clock_rate(PLL_CLKSEL_CRYPTO, (UINT32)uiCryptoClockRate);

#if !defined(PLL_FREE)
#if defined(SCE_USE_STBC_CLK)
	pll_set_STBC_pll_enable(crypto_pll, 1);
#else
	pll_set_pll_enable(crypto_pll, 1);
#endif
#endif

	/* disabel Crypto module reset */
	pll_disable_system_reset(CRYPTO_RSTN);

	/* enable Crypto module master clock */
	pll_enable_clock(CRYPTO_CLK);
}

/* register the interrupt */
static TEE_Result nvt_crypto_init(void)
{
	/* get register access base */
	CRYPTO_ADDR_VBASE = (vaddr_t)phys_to_virt(IOADDR_CRYPTO_REG_BASE, MEM_AREA_IO_SEC, 0x100);

	/* crypto clock and reset init */
	crypto_platform_init();

	/* create platform resource */
	crypto_platform_create_resource();

#ifdef CRYPTO_INTERRUPT_MODE
	/* create flag */
	cre_flg(&FLG_ID_CRYPTO, NULL, (char *)"FLG_ID_CRYPTO");
	clr_flg(FLG_ID_CRYPTO, 0xFFFFFFFF);

	crypto_it_handler.it      = INT_ID_CRYPTO;
	crypto_it_handler.flags   = ITRF_TRIGGER_LEVEL;
	crypto_it_handler.handler = crypto_isr;
	crypto_it_handler.data    = NULL;

	itr_add(&crypto_it_handler);
	itr_enable(crypto_it_handler.it);
#endif

	return TEE_SUCCESS;
}

driver_init(nvt_crypto_init);
