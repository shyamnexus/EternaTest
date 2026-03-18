/*
    Hash Engine driver

    Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
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
#include "plat/hash.h"
#include "pll_protected.h"
#include "hash_reg.h"
#include "hash_int.h"
#include "hash_platform.h"

#include "crypto_protected.h"
#include "nvt_sce_cfg.h"

//#define HASH_INTERRUPT_MODE
#define FLGPTN_HASH                 FLGPTN_BIT(0)
#define MODULE_FLAG_PTN             FLGPTN_HASH

#define DMA_ADDR_LO(_addr)          ((UINT32)((UINT64)(_addr)))
#define DMA_ADDR_HI(_addr)          ((UINT32)(((UINT64)(_addr))>>32))

static  HASH_CLKSEL  uiHASHClockRate = HASH_CLKSEL0;
static  BOOL    bHASHOpened     = FALSE;
static  BOOL    bHASHSem        = FALSE;
static  UINT32  uiHASHMode      = HASH_MODE_SHA1;
static  BOOL    bHASH_IVSEL     = HASH_IV_Default;
static  BOOL    bHASH_ACCMODE   = HASH_ACCMODE_PIO;
static  BOOL    bHASH_DMA_PAD   = HASH_DMA_PAD_DISABLE;
static  BOOL    bHASH_BYPASS    = HASH_BYPASS_NORMAL;
static  BOOL    bHASH_IPAD      = FALSE;
static  BOOL    bHASH_OPAD      = FALSE;

static vaddr_t HASH_ADDR_VBASE = 0;

#define HASH_SETREG(_ofs, _val)             io_write32(HASH_ADDR_VBASE+(_ofs), (_val))
#define HASH_GETREG(_ofs)                   io_read32(HASH_ADDR_VBASE+(_ofs))

#define dma_getPhyAddr(_va)                 virt_to_phys((void *)_va)
#define dma_flushWriteCache(_va, _len)      cache_operation(TEE_CACHECLEAN,      (void *)(_va), (_len))
#define dma_flushReadCache(_va, _len)       cache_operation(TEE_CACHEFLUSH,      (void *)(_va), (_len)) ///< use CACHEFLUSH instead of CACHEINVALIDATE, to avoid non-cache alignment invalid cache would cause stack corrupt
#define dma_flushReadCacheDmaEnd(_va, _len) cache_operation(TEE_CACHEINVALIDATE, (void *)(_va), (_len))

#ifdef HASH_INTERRUPT_MODE
static ID FLG_ID_HASH;
static struct itr_handler hash_it_handler;

static enum itr_return hash_isr(struct itr_handler *h __unused)
{
	T_HASH_INT_STS_REG HashINTSTS  = {0x00000000};

	HashINTSTS.reg = HASH_GETREG(HASH_INT_STS_REG_OFS);

	if (HashINTSTS.bit.HASH_BED_STS){
		/* Write 1 Clear interrupt status */
		HASH_SETREG(HASH_INT_STS_REG_OFS, HashINTSTS.reg);
	}

	/* Set flag */
	iset_flg(FLG_ID_HASH, MODULE_FLAG_PTN);

	return IRQ_HANDLED;
}
#endif

#if defined(HOST_AXI_CH)
static void HASHHostAXIChannelEnable(void) {
        T_HASH_AXI_CHANNEL_REG axi_ch_reg = {0};

        axi_ch_reg.reg = HASH_GETREG(HASH_AXI_CHANNEL_REG_OFS);
        axi_ch_reg.bit.MONI_CH_DIS = 0;
        HASH_SETREG(HASH_AXI_CHANNEL_REG_OFS, axi_ch_reg.reg);

}

static void HASHHostAXIChannelDisable(void) {
        T_HASH_AXI_CHANNEL_REG axi_ch_reg = {0};

        axi_ch_reg.reg = HASH_GETREG(HASH_AXI_CHANNEL_REG_OFS);
        axi_ch_reg.bit.MONI_CH_DIS = 1;
        HASH_SETREG(HASH_AXI_CHANNEL_REG_OFS, axi_ch_reg.reg);
}
#endif


static ER hash_lock(void)
{
	ER ret;

	if ((ret = hash_platform_sem_wait()) == E_OK) {
		bHASHSem = TRUE;
	}
	return ret;
}

static ER hash_unlock(void)
{
	if (bHASHSem == TRUE) {
		bHASHSem = FALSE;
		return hash_platform_sem_signal();
	}
	else {
		return E_SYS;
	}
}

ER hash_setConfig(HASH_CONFIG_ID cfgid, UINT32 cfgvalue)
{
	ER ret = E_NOSPT;
	T_HASH_CONFIG_REG HashConfig = {0x00000000};

	if (!bHASHOpened) {
		DBG_ERR("Hash engine not open\r\n");
		return ret;
	}

	switch (cfgid) {
#ifdef HASH_CLOCK_RATE_SUPPORT
		case HASH_CONFIG_ID_CLOCK_RATE:
			{
				switch (cfgvalue) {
					case HASH_CLKSEL0:
						uiHASHClockRate = cfgvalue;
						ret = E_OK;
					break;

					case HASH_CLKSEL1:
						uiHASHClockRate = cfgvalue;
						ret = E_OK;
					break;

					case HASH_CLKSEL2:
						if (HASH_CLKSEL2 != PLL_NOT_SUPPORT) {
							uiHASHClockRate = cfgvalue;
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

		case HASH_CONFIG_ID_MODE:
			{
				if (cfgvalue >= HASH_MODE_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				uiHASHMode = cfgvalue;

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.MODE = uiHASHMode;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_IV_SEL:
			{
				if (cfgvalue >= HASH_IV_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				bHASH_IVSEL = cfgvalue;

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.IV_SEL = bHASH_IVSEL;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_ACCMODE:
			{
				if (cfgvalue >= HASH_ACCMODE_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				bHASH_ACCMODE = cfgvalue;

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.ACCMOD = bHASH_ACCMODE;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_DMA_PAD:
			{
				if (cfgvalue >= HASH_DMA_PAD_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				bHASH_DMA_PAD = cfgvalue;

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.DMA_PAD = bHASH_DMA_PAD;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_BYPASS:
			{
				if (cfgvalue >= HASH_BYPASS_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				if (cfgvalue == HASH_BYPASS_HwCopy)
					bHASH_ACCMODE = HASH_ACCMODE_DMA;
				else
					bHASH_ACCMODE = HASH_ACCMODE_PIO;

				bHASH_BYPASS = cfgvalue;

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.ACCMOD = bHASH_ACCMODE;
				HashConfig.bit.BYPASS = bHASH_BYPASS;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_INI:
			{
				if (cfgvalue >= HASH_INI_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.INI = cfgvalue;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_IPAD:
			{
				if (cfgvalue >= HASH_INI_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.IPAD = cfgvalue;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);
				bHASH_IPAD = cfgvalue;

				ret = E_OK;
			}
			break;

		case HASH_CONFIG_ID_OPAD:
			{
				if (cfgvalue >= HASH_INI_NUM) {
					DBG_ERR("param err -> op(%d) param(%d)\r\n", (int)cfgid, (int)cfgvalue);
					break;
				}

				HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
				HashConfig.bit.OPAD = cfgvalue;
				HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);
				bHASH_OPAD = cfgvalue;

				ret = E_OK;
			}
			break;

		default:
			ret = E_NOSPT;
			break;
	}
	return ret;
}

ER hash_open(void)
{
	ER ret;

	if ((ret = hash_lock()) != E_OK) {
		return ret;
	}

	bHASHOpened = TRUE;

	return E_OK;
}

ER hash_close(void)
{
	ER ret;

	if (!bHASHOpened)
		return E_OK;

	bHASHOpened = FALSE;

	ret = hash_unlock();

	return ret;
}

void hash_setKey(UINT8 *ucKey, UINT32 uiLen)
{
	UINT32 i;
	UINT32 size;
	UINT8  hash_key[64];

	if (!bHASHOpened) {
		DBG_ERR("Hash engine not open\r\n");
		return;
	}

	for (i=0; i<16; i++) {
		HASH_SETREG(HASH_KEY0_REG_OFS+i*4, 0x00000000);
	}

	if (uiLen > 64) {
		DBG_WRN("HASH key length is over range.\r\n");
		return;
	}
	if (uiLen < 64) {
		memcpy(hash_key, ucKey, uiLen);
		memset(hash_key+uiLen, 0x00, 64-uiLen);
	}
	if (uiLen == 64) {
		memcpy(hash_key, ucKey, uiLen);
	}

	if ((uiLen%4)==0)
		size = (uiLen/4);
	else
		size = (uiLen/4)+1;

	for (i=0;i<size;i++) {
		HASH_SETREG(HASH_KEY0_REG_OFS+i*4, MKWORD(hash_key+i*4));
	}
}

void hash_setIV(UINT8 *ucIV)
{
	if (!bHASHOpened) {
		DBG_ERR("Hash engine not open\r\n");
		return;
	}

	switch (uiHASHMode) {
		case HASH_MODE_SHA1:
		case HASH_MODE_HMAC_SHA1:
			HASH_SETREG(HASH_IV0_REG_OFS, MKWORD(ucIV+0));
			HASH_SETREG(HASH_IV1_REG_OFS, MKWORD(ucIV+4));
			HASH_SETREG(HASH_IV2_REG_OFS, MKWORD(ucIV+8));
			HASH_SETREG(HASH_IV3_REG_OFS, MKWORD(ucIV+12));
			HASH_SETREG(HASH_IV4_REG_OFS, MKWORD(ucIV+16));
			break;
		case HASH_MODE_SHA256:
		case HASH_MODE_HMAC_SHA256:
			HASH_SETREG(HASH_IV0_REG_OFS, MKWORD(ucIV+0));
			HASH_SETREG(HASH_IV1_REG_OFS, MKWORD(ucIV+4));
			HASH_SETREG(HASH_IV2_REG_OFS, MKWORD(ucIV+8));
			HASH_SETREG(HASH_IV3_REG_OFS, MKWORD(ucIV+12));
			HASH_SETREG(HASH_IV4_REG_OFS, MKWORD(ucIV+16));
			HASH_SETREG(HASH_IV5_REG_OFS, MKWORD(ucIV+20));
			HASH_SETREG(HASH_IV6_REG_OFS, MKWORD(ucIV+24));
			HASH_SETREG(HASH_IV7_REG_OFS, MKWORD(ucIV+28));
			break;
		default:
			DBG_WRN("Pls check which HASH Mode are selected.\r\n");
			break;
	}
}

void hash_getOutput(UINT8 *Output, UINT32 len)
{
	int    i;
	UINT32 hash_output[8];

	if(!bHASHOpened) {
		DBG_ERR("Hash engine not open\r\n");
		return;
	}

	/* read hash value from register */
	for (i=0; i<8; i++) {
		hash_output[i] = HASH_GETREG(HASH_OUTPUT_0_REG_OFS+(4*i));
	}

	/* copy data to output buffer */
	if (len) {
		memcpy(Output, hash_output, ((len <= 32) ? len : 32));
	}
}

void hash_dma_enable(vaddr_t addr, UINT32 size, UINT32 pad_len)
{
#ifdef HASH_INTERRUPT_MODE
	FLGPTN FlagPtn;
#else
	UINT32 busy_count = 0;
#endif
	T_HASH_CONFIG_REG         HashConfig  = {0x00000000};
	T_HASH_DMA_START_ADDR_REG HashSrcAddr = {0x00000000};
	T_HASH_INT_STS_REG        HashINTSTS  = {0x00000000};
	T_HASH_INT_EN_REG         HashINTEN   = {0x00000000};
	T_HASH_TRANSSIZE_REG      HashSize    = {0x00000000};
	T_HASH_PADLEN_REG         HashPad     = {0x00000000};
#ifdef HASH_DMA_36BIT_SUPPORT
	T_HASH_DMA_START_ADDR_H_REG HashSrcAddr_H = {0x00000000};
#endif
	UINT32 block = 0, blk_r = 0;
	paddr_t dma_paddr;

	if (!bHASHOpened) {
		DBG_ERR("Hash engine not open\r\n");
		return;
	}

	dma_paddr = dma_getPhyAddr(addr);
	if (((uintptr_t)dma_paddr) & 0x3) {
		DBG_WRN("vaddr=0x%08lx paddr=0x%08lx not word align, size = 0x%x, pad_len = 0x%x\r\n", (uintptr_t)addr, (uintptr_t)dma_paddr, size, pad_len);
		return;
	}

	block = size/64;
	blk_r = size%64;
	if(blk_r > 55)
		block = block+2;
	else
		block = block+1;

#if defined(HOST_AXI_CH)
        //AXI channel enable
        HASHHostAXIChannelEnable();
#endif

	/* Set interrupt enable */
	HashINTEN.reg = HASH_GETREG(HASH_INT_EN_REG_OFS);
	HashINTEN.bit.HASH_BED_EN = 1;
	HASH_SETREG(HASH_INT_EN_REG_OFS, HashINTEN.reg);

	/* Clear interrupt status */
	HashINTSTS.reg = HASH_GETREG(HASH_INT_STS_REG_OFS);
	HashINTSTS.bit.HASH_BED_STS = 1;
	HASH_SETREG(HASH_INT_STS_REG_OFS, HashINTSTS.reg);

	/* Set DMA Transfer size */
	HashSize.reg = HASH_GETREG(HASH_TRANSSIZE_REG_OFS);
	HashSize.bit.HASH_TRANS_SIZE = size;
	HASH_SETREG(HASH_TRANSSIZE_REG_OFS, HashSize.reg);

	/* Set DMA Padding size */
	HashPad.reg = HASH_GETREG(HASH_PADLEN_REG_OFS);
	HashPad.bit.HASH_PAD_LEN = pad_len;
	HASH_SETREG(HASH_PADLEN_REG_OFS, HashPad.reg);

	/* Set DMA Src Addr */
	HashSrcAddr.reg = HASH_GETREG(HASH_DMA_START_ADDR_REG_OFS);
	HashSrcAddr.bit.HASH_SRC_ADDR = DMA_ADDR_LO(dma_paddr);
	HASH_SETREG(HASH_DMA_START_ADDR_REG_OFS, HashSrcAddr.reg);

#ifdef HASH_DMA_36BIT_SUPPORT
	/* Set DMA Src Addr_H */
	HashSrcAddr_H.reg = HASH_GETREG(HASH_DMA_START_ADDR_H_REG_OFS);
	HashSrcAddr_H.bit.HASH_SRC_ADDR_H = DMA_ADDR_HI(dma_paddr) & 0xff;
	HASH_SETREG(HASH_DMA_START_ADDR_H_REG_OFS, HashSrcAddr_H.reg);
#endif

#ifdef HASH_INTERRUPT_MODE
	/* Clear flag */
	clr_flg(FLG_ID_HASH, MODULE_FLAG_PTN);
#endif

	/* Flush cache */
	dma_flushWriteCache(addr, ((pad_len) ? (block*64) : size));

	/* Set HASH_EN Enable */
	HashConfig.reg = HASH_GETREG(HASH_CONFIG_REG_OFS);
	HashConfig.bit.HASH_EN = 1;
	HASH_SETREG(HASH_CONFIG_REG_OFS, HashConfig.reg);

#ifdef HASH_INTERRUPT_MODE
	/* Wait for PIO transmitted */
	wai_flg(&FlagPtn, FLG_ID_HASH, MODULE_FLAG_PTN, TWF_ORW | TWF_CLR);
#else
	while (1) {
		HashINTSTS.reg = HASH_GETREG(HASH_INT_STS_REG_OFS);

		if (HashINTSTS.bit.HASH_BED_STS){
			// Write 1 Clear interrupt status
			HASH_SETREG(HASH_INT_STS_REG_OFS, HashINTSTS.reg);
			break;
		}

		busy_count++;
		if (busy_count > 10000000) {
			DBG_ERR("hash busy_count=%u addr=0x%08lx, size=%u, pad_len=%u\r\n", busy_count, (uintptr_t)addr, size, pad_len);
		}
	}
#endif
#if defined(HOST_AXI_CH)
        //AXI channel disable
        HASHHostAXIChannelDisable();
#endif
}

static void hash_platform_init(void)
{
#if !defined(PLL_FREE)
	PLL_EN hash_pll = PLLEN0;
#endif

	pll_set_clock_rate(PLL_CLKSEL_HASH, (UINT32)uiHASHClockRate);

#if !defined(PLL_FREE)
#if defined(SCE_USE_STBC_CLK)
	pll_set_STBC_pll_enable((PLL_ID)hash_pll, 1);
#else
	pll_set_pll_enable((PLL_ID)hash_pll, 1);
#endif
#endif

	/* disabel Hash module reset */
	pll_disable_system_reset(HASH_RSTN);

	/* enable Hash module master clock */
	pll_enable_clock(HASH_CLK);

}

static TEE_Result nvt_hash_init(void)
{
	/* get register access base */
	HASH_ADDR_VBASE = (vaddr_t)phys_to_virt(IOADDR_HASH_REG_BASE, MEM_AREA_IO_SEC, 0x100);

	/* rsa clock and reset init */
	hash_platform_init();

	/* create platform resource */
	hash_platform_create_resource();

#ifdef HASH_INTERRUPT_MODE
	/* create flag */
	cre_flg(&FLG_ID_HASH, NULL, (char *)"FLG_ID_HASH");
	clr_flg(FLG_ID_HASH, 0xFFFFFFFF);

	hash_it_handler.it      = INT_ID_HASH;
	hash_it_handler.flags   = ITRF_TRIGGER_LEVEL;
	hash_it_handler.handler = hash_isr;
	hash_it_handler.data    = NULL;

	itr_add(&hash_it_handler);
	itr_enable(hash_it_handler.it);
#endif

	return TEE_SUCCESS;
}

driver_init(nvt_hash_init);
