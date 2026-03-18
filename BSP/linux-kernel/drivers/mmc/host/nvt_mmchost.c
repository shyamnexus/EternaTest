/**
    NVT mmc function
    To handle mmc modules
    @file       nvt_mmchost.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include "nvt_mmchost.h"
#include "nvt_mmcreg.h"
#include <linux/io.h>
#include <linux/printk.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/slab.h>
#include <linux/clk-provider.h>

#define RESET_TIMEOUT	10000
/************************* cache API ********************************/

#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>

static struct mmc_nvt_host drvdump_info[3];

void dma_flushwritecache(struct device *dev, void *pbuf, u32 len)
{
	dma_addr_t dma_handle;
	/*void *ptr;*/
	/*printk("flash write buf 0x%x, len %d\r\n", pbuf, len);*/
	/*for (ptr = pbuf; ptr < (pbuf + len + PAGE_SIZE); ptr += PAGE_SIZE)
		flush_dcache_page(virt_to_page(ptr));*/
	dma_handle = dma_map_single(dev, pbuf, len, DMA_TO_DEVICE);

	dma_sync_single_for_device(dev, dma_handle, len, DMA_TO_DEVICE);

	dma_unmap_single(dev, dma_handle, len, DMA_TO_DEVICE);
}

void dma_flushreadcache(struct device *dev, void *pbuf, u32 len)
{
	dma_addr_t dma_handle;
	/*void *ptr;*/
	/*printk("flash read buf 0x%x, len %d\r\n", pbuf, len);*/
	/*for (ptr = pbuf; ptr < (pbuf + len + PAGE_SIZE); ptr += PAGE_SIZE)
		flush_dcache_page(virt_to_page(ptr));*/
	dma_handle = dma_map_single(dev, pbuf, len, DMA_FROM_DEVICE);

	dma_sync_single_for_device(dev, dma_handle, len, DMA_FROM_DEVICE);

	dma_unmap_single(dev, dma_handle, len, DMA_FROM_DEVICE);
}

void dma_flushbidircache(struct device *dev, void *pbuf, u32 len)
{
	dma_addr_t dma_handle;
	/*void *ptr;*/
	/*printk("flash read buf 0x%x, len %d\r\n", pbuf, len);*/
	/*for (ptr = pbuf; ptr < (pbuf + len + PAGE_SIZE); ptr += PAGE_SIZE)
		flush_dcache_page(virt_to_page(ptr));*/
	dma_handle = dma_map_single(dev, pbuf, len, DMA_BIDIRECTIONAL);

	dma_sync_single_for_device(dev, dma_handle, len, DMA_BIDIRECTIONAL);

	dma_unmap_single(dev, dma_handle, len, DMA_BIDIRECTIONAL);
}

/************************* cache API ********************************/




/*-------------------------------------------------------*/
/*  Internal function prototypes                         */
/*-------------------------------------------------------*/
static REGVALUE sdiohost_getreg(struct mmc_nvt_host *host, uint32_t id,
	uint32_t offset);
static void sdiohost_setreg(struct mmc_nvt_host *host, uint32_t id,
	uint32_t offset, REGVALUE value);
static uint32_t sdiohost_check_hs400opt(struct mmc_nvt_host *host, uint32_t reg);

/*-------------------------------------------------------*/
/*  Private driver specific variables                    */
/*-------------------------------------------------------*/
static uint32_t vsdio_seg_en[SDIO_HOST_ID_COUNT] = {
	FALSE,
#if (SDIO_HOST_ID_COUNT > 1)
	FALSE,
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	FALSE
#endif
};

#if (SDIO_SCATTER_DMA)
static uint32_t vsdio_seg_num[SDIO_HOST_ID_COUNT] = {
	1,
#if (SDIO_HOST_ID_COUNT > 1)
	1,
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	1
#endif
};
#endif

#if (SDIO_SCATTER_DMA)
#define SDIO_DES_MAX_BUF_SIZE     (64*1024*1024)  /*64MB*/

/*static uint32_t
vuisdio_destab[SDIO_HOST_ID_COUNT][SDIO_DES_TABLE_NUM*SDIO_DES_WORD_SIZE];*/
#endif

/*********************************************************************/
/*  Public(to driver layer) SDIO host controller functions           */
/*********************************************************************/

void sdiohost_setdesen(uint32_t id, uint32_t uien)
{
	if (uien == 0)
		vsdio_seg_en[id] = FALSE;
	else
		vsdio_seg_en[id] = TRUE;
}

void sdiohost_setdestab(uint32_t id, uintptr_t uidesaddr, uint32_t uidesnum,
uint32_t *vuisdio_destab)
{
#if (SDIO_SCATTER_DMA)
	/*union SDIO_DATA_LENGTH_REG datalenreg; */
	/*union SDIO_DMA_DES_START_ADDR_REG dmadesaddrreg; */
	union SDIO_DES_LINE_REG dmadeslinereg;
	union SDIO_DES_LINE1_REG dmadesline1reg;
	union SDIO_DES_LINE2_REG dmadesline2reg;
	uint32_t uitotalsize;
	struct STRG_SEG_DES *pseg_des;
	uint32_t i;

	if (uidesnum > SDIO_DES_TABLE_NUM) {
		pr_err("Descriptor number must < SDIO_DES_TABLE_NUM\r\n");
		return;
	}

	memset((void *)&vuisdio_destab[0], 0,
	(SDIO_DES_TABLE_NUM * SDIO_DES_WORD_SIZE)<<2);

	uitotalsize = 0;
	pseg_des = (struct STRG_SEG_DES *)uidesaddr;

	for (i = 0; i < uidesnum; i++) {
		dmadeslinereg.reg = 0;
		dmadesline1reg.reg = 0;
		dmadesline2reg.reg = 0;

		dmadeslinereg.bit.DMA_DES_VALID = 1;

		if (i == (uidesnum - 1))
			dmadeslinereg.bit.DMA_DES_END = 1;
		else
			dmadeslinereg.bit.DMA_DES_END = 0;

		dmadeslinereg.bit.DMA_DES_ACT = 2;

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (pseg_des->uisegaddr & SDIO_HIGH_ADDR_MASK) {
			dmadeslinereg.bit.DMA_DES_HIGH = (pseg_des->uisegaddr & SDIO_HIGH_ADDR_MASK) >> 32;
		} else {
			dmadeslinereg.bit.DMA_DES_HIGH = 0x0;
		}
#else
		dmadeslinereg.bit.DMA_DES_HIGH = 0x0;
#endif

		dmadesline1reg.bit.DMA_DES_DATA_LEN = pseg_des->uisegsize;
		dmadesline2reg.bit.DMA_DES_DATA_ADDR = pseg_des->uisegaddr;

		vuisdio_destab[(i * SDIO_DES_WORD_SIZE) +
		SDIO_DES_LINE_REG_OFS] = (uint32_t)dmadeslinereg.reg;

		vuisdio_destab[(i * SDIO_DES_WORD_SIZE) +
		(SDIO_DES_LINE1_REG_OFS >> 2)] = (uint32_t)dmadesline1reg.reg;

		vuisdio_destab[(i * SDIO_DES_WORD_SIZE) +
		(SDIO_DES_LINE2_REG_OFS >> 2)] = (uint32_t)dmadesline2reg.reg;

		pseg_des++;
		uitotalsize = dmadesline1reg.bit.DMA_DES_DATA_LEN;

	}
	vsdio_seg_num[id] = uidesnum;
#endif
}

/*
	Get SDIO Busy or not

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return TRUE: ready
		FALSE: busy
*/
bool sdiohost_getrdy(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_BUS_STATUS_REG stsreg;

	stsreg.reg = sdiohost_getreg(host, id, SDIO_BUS_STATUS_REG_OFS);

	return stsreg.bit.BUS_STS_D0;
}

/*
	Enable CKGEN clock for specified SDIO channel.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_enclock(struct mmc_nvt_host *host, uint32_t id)
{
	clk_enable(host->clk);
}

/*
	Disable CKGEN clock for specified SDIO channel.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_disclock(struct mmc_nvt_host *host, uint32_t id)
{
	clk_disable(host->clk);
}


/*
	Enable SD clock for specified SDIO channel.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_enclockout(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;

	clk_ctrl.reg = sdiohost_getreg(host, id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.CLK_DIS = 0;

    // MUST check hs400 option before any reg 0x38 write
    clk_ctrl.reg = sdiohost_check_hs400opt(host, clk_ctrl.reg);
	sdiohost_setreg(host, id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
}

/*
	Disable SD clock for specified SDIO channel.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_disclockout(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;
	clk_ctrl.reg = sdiohost_getreg(host, id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.CLK_DIS = 1;

    // MUST check hs400 option before any reg 0x38 write
    clk_ctrl.reg = sdiohost_check_hs400opt(host, clk_ctrl.reg);
	sdiohost_setreg(host, id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
}

void sdiohost_setphyrst(struct mmc_nvt_host *host)
{
	union SDIO_PHY_REG phyreg, phyreg_read;
	u32 i = 0;

	phyreg.reg = sdiohost_getreg(host, host->id, SDIO_PHY_REG_OFS);
	phyreg.bit.PHY_SW_RST = 1;
	sdiohost_setreg(host, host->id, SDIO_PHY_REG_OFS, phyreg.reg);
	while (1)
	{
		phyreg_read.reg = sdiohost_getreg(host, host->id, SDIO_PHY_REG_OFS);
		if ((phyreg_read.bit.PHY_SW_RST == 0) || (i == RESET_TIMEOUT))
			break;

		i++;
	}

	if (i == RESET_TIMEOUT)
		pr_err("phy reset timeout\n");
}

void sdiohost_setphysample(struct mmc_nvt_host *host, bool internal, bool before)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.SAMPLE_CLK_EDGE = host->neg_sample_edge;
	dly0_reg.bit.SRC_CLK_SEL = internal;
	dly0_reg.bit.PAD_CLK_SEL = before;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

void sdiohost_setphyclkoutdly(struct mmc_nvt_host *host, uint32_t outdly_sel)
{
	union SDIO_CLOCK_CTRL2_REG clock_ctrl;

	clock_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL2_REG_OFS);
	clock_ctrl.bit.OUTDLY_SEL = outdly_sel;
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL2_REG_OFS, clock_ctrl.reg);
}

void sdiohost_setphyclkindly(struct mmc_nvt_host *host, uint32_t indly_sel)
{
	union SDIO_CLOCK_CTRL2_REG clock_ctrl;

	clock_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL2_REG_OFS);
	clock_ctrl.bit.INDLY_SEL = indly_sel;
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL2_REG_OFS, clock_ctrl.reg);
}

void sdiohost_set_phasecmd(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DLY_PHASE_SEL = sel;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

uint32_t sdiohost_get_phasecmd(struct mmc_nvt_host *host)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);

	return dly0_reg.bit.DLY_PHASE_SEL;
}

void sdiohost_set_phasedata(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DATA_DLY_PHASE_SEL = sel;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

uint32_t sdiohost_get_phasedata(struct mmc_nvt_host *host)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);

	return dly0_reg.bit.DATA_DLY_PHASE_SEL;
}

void sdiohost_set_phaseunit(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DLY_SEL = sel;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

uint32_t sdiohost_get_phaseunit(struct mmc_nvt_host *host)
{
    union SDIO_DLY0_REG dly0_reg;

    dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);

    return dly0_reg.bit.DLY_SEL;
}

void sdiohost_set_detclr(struct mmc_nvt_host *host)
{
	union SDIO_DLY3_REG dly3_reg;

	if ((clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1))
			clk_set_phase(host->clk, 0);

	dly3_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY3_REG_OFS);

	dly3_reg.bit.DET_CLR = 1;
	sdiohost_setreg(host, host->id, SDIO_DLY3_REG_OFS, dly3_reg.reg);

	dly3_reg.bit.DET_CLR = 0;
	sdiohost_setreg(host, host->id, SDIO_DLY3_REG_OFS, dly3_reg.reg);

	if ((!clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1))
			clk_set_phase(host->clk, 1);
}

uint32_t sdiohost_get_detcmdout(struct mmc_nvt_host *host)
{
	union SDIO_DLY6_REG dly6_reg;

	dly6_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY6_REG_OFS);

	return dly6_reg.bit.DET_CMD_OUT;
}

uint32_t sdiohost_get_detdataout(struct mmc_nvt_host *host)
{
	union SDIO_DLY2_REG dly2_reg;

	dly2_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY2_REG_OFS);

	return dly2_reg.bit.DET_DATA_OUT;
}

void sdiohost_set_detphasecmpen(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY4_REG dly4_reg;

	dly4_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY4_REG_OFS);
	dly4_reg.bit.PHASE_COMP_EN = sel;
	sdiohost_setreg(host, host->id, SDIO_DLY4_REG_OFS, dly4_reg.reg);
}

void sdiohost_set_detdata(struct mmc_nvt_host *host, bool data_is_8bit)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DET_DATA = data_is_8bit;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

void sdiohost_set_ddroutdlyen(struct mmc_nvt_host *host, bool outdly_en, bool data_outdly_inv, bool cmd_outdly_inv)
{
	union SDIO_DLY5_REG dly5_reg;

	dly5_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY5_REG_OFS);
	dly5_reg.bit.OUT_DLY_EN = outdly_en;
	dly5_reg.bit.DATA_OUT_DLY_INV = data_outdly_inv;
	dly5_reg.bit.CMD_OUT_DLY_INV = cmd_outdly_inv;
	sdiohost_setreg(host, host->id, SDIO_DLY5_REG_OFS, dly5_reg.reg);
}

void sdiohost_set_ddroutdly(struct mmc_nvt_host *host, u32 outdly_sel)
{
	union SDIO_DLY5_REG dly5_reg;

	dly5_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY5_REG_OFS);
	dly5_reg.bit.OUT_DLY_SEL = outdly_sel;
	sdiohost_setreg(host, host->id, SDIO_DLY5_REG_OFS, dly5_reg.reg);
}

void sdiohost_set_ddr41phase(struct mmc_nvt_host *host, bool ddr41_en)
{
	union SDIO_DLY5_REG dly5_reg;

	dly5_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY5_REG_OFS);
	dly5_reg.bit.DDR_41PHASE_SEL = ddr41_en;
	sdiohost_setreg(host, host->id, SDIO_DLY5_REG_OFS, dly5_reg.reg);
}

void sdiohost_set_cmd41phase(struct mmc_nvt_host *host, bool cmd41_en)
{
	union SDIO_DLY5_REG dly5_reg;

	dly5_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY5_REG_OFS);
	dly5_reg.bit.CMD_41PHASE_SEL = cmd41_en;
	sdiohost_setreg(host, host->id, SDIO_DLY5_REG_OFS, dly5_reg.reg);
}

void sdiohost_set_ddrdataswap(struct mmc_nvt_host *host, bool swap)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DDR_DATA_SWAP = swap;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

void sdiohost_set_ddrstartedge(struct mmc_nvt_host *host, bool is_pos_start)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DDR_START_EDGE = is_pos_start;

	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

void sdiohost_set_ddrclken(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;

	clk_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.CLK_DDR = enable;

    // MUST check hs400 option before any reg 0x38 write
    clk_ctrl.reg = sdiohost_check_hs400opt(host, clk_ctrl.reg);
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
}

void sdiohost_set_datastrobe(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;

	clk_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.HS400_MODE_SEL = enable;

    // MUST check hs400 option before any reg 0x38 write
    clk_ctrl.reg = sdiohost_check_hs400opt(host, clk_ctrl.reg);
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
}

void sdiohost_set_hs400es(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;

	clk_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.MODE_HS400_ENHANCE_SEL = enable;
	
    // MUST check hs400 option before any reg 0x38 write
    clk_ctrl.reg = sdiohost_check_hs400opt(host, clk_ctrl.reg);
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
}

void sdiohost_set_hs400_sync_opt(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_CLOCK_CTRL_REG clk_ctrl;

	clk_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS);
	clk_ctrl.bit.HS400_SYNC_OPTION = enable;
	sdiohost_setreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS, clk_ctrl.reg);
	clk_ctrl.reg = sdiohost_getreg(host, host->id, SDIO_CLOCK_CTRL_REG_OFS);
}

/*
	Set PAD drive/sink of clock pin for specified SDIO channel.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] driving  desired driving value * 10, unit: mA
				valid value: 50 ~ 200

	@return
		- @b E_OK: sucess
		- @b Else: fail
*/
int sdiohost_setpaddriving(struct mmc_nvt_host *host, SDIO_SPEED_MODE mode)
{
    ER ret = E_OK;
	uint32_t data_uidriving, cmd_uidriving, clk_uidriving;
    uint32_t pad_ds;
    uint8_t pin_num = SDIO_PIN_NUM_4_BIT, i;

	if (host->id >= SDIO_HOST_ID_COUNT)
		return E_SYS;

    // get data, cmd, clk pad_driving
	if (mode == SDIO_MODE_DS) {
		data_uidriving = host->pad_driving[SDIO_DS_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_DS_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_DS_MODE_CLK];
	} else if (mode == SDIO_MODE_HS) {
		data_uidriving = host->pad_driving[SDIO_HS_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_HS_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_HS_MODE_CLK];
	} else if (mode == SDIO_MODE_SDR50) {
		data_uidriving = host->pad_driving[SDIO_SDR50_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_SDR50_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_SDR50_MODE_CLK];
	} else {
		data_uidriving = host->pad_driving[SDIO_SDR104_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_SDR104_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_SDR104_MODE_CLK];
	}

    if (CHECK_SDIO_SUP(host->id)) {
        // check 4-bits/8-bits data
        if (host->mmc) {
            if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) &&
                CHECK_SDIO_8_BITS_SUP(host->id)) {
                pin_num = SDIO_PIN_NUM_8_BIT;
            }
        } else {
            pr_err("mmc is NULL, bypass 8 bit mode driving\r\n");
        }

        // set cmd pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CMD);
		if (pad_ds != 0)
        	ret = pad_set_drivingsink(pad_ds, cmd_uidriving);
		if (ret < 0)
			return ret;

        // set data pad_driving
        for (i = SDIO_PIN_DATA0; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, i);
			if (pad_ds != 0)
            	ret = pad_set_drivingsink(pad_ds, data_uidriving);
			if (ret < 0)
				return ret;
        }

        // set clk pad_driving
        pad_ds = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_DS, host->mmc_pinmux, SDIO_PIN_CLK);
		if (pad_ds != 0)
        	ret = pad_set_drivingsink(pad_ds, clk_uidriving);
		if (ret < 0)
			return ret;
    }

    return E_OK;
}

/*
	Reset SDIO host controller.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_reset(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_CMD_REG cmdreg;
	int i = 0;

	cmdreg.reg = sdiohost_getreg(host, id, SDIO_CMD_REG_OFS);
	cmdreg.bit.SDC_RST = 1;
	sdiohost_setreg(host, id, SDIO_CMD_REG_OFS, cmdreg.reg);

	while (1) {
		cmdreg.reg = sdiohost_getreg(host, id, SDIO_CMD_REG_OFS);

		if ((cmdreg.bit.SDC_RST == 0) || (i == RESET_TIMEOUT))
			break;

		i++;
	}

	if (i == RESET_TIMEOUT)
		pr_err("sdc reset timeout\n");
}

/*
	Asynchrous reset SDIO host controller.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
			- @b SDIO_HOST_ID_3: SDIO3

	@return void
*/
void sdiohost_resetasync(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_DLY1_REG        dlyreg1;
	union SDIO_PHY_REG         phyreg;
	union SDIO_PHY_REG         phyregread;
	union SDIO_FIFO_SWITCH_REG fifoswitch;
	int i = 10;

	// toggle RSTN to reset register table and HW state machine
	while (i) {
		if (__clk_is_enabled(host->clk)) {
			clk_disable_unprepare(host->clk);
		} else {
			break;
		}

		i--;
	}
	if (i == 0) {
		pr_err("SDIO%d cannot disable clock\n", id);
	}

	/* PHY_SW_RESET needs to enable CG clk_en and TOP sdio_func */
	clk_prepare_enable(host->clk);

	{
		PIN_GROUP_CONFIG pinmux_config[1];
		uint32_t pin_sdio_cfg;

		if (CHECK_SDIO_SUP(host->id)) {
			// update pinmux config
			pinmux_config->pin_function = PIN_FUNC_SDIO;
			if (nvt_pinmux_capture(pinmux_config, 1)) {
				pr_err("Get SDIO%d pinmux config fail\n", host->id + 1);
			}

			pin_sdio_cfg = (host->mmc_pinmux & PIN_SDIO_CFG_MASK(host->id));
			pinmux_config->config |= pin_sdio_cfg;
			if (nvt_pinmux_update(pinmux_config, 1)) {
				pr_err("Switch SDIO%d pinmux error\n", host->id + 1);
			}
		}
	}

	/* patch begin for sd write hang-up or write byte access error */
	fifoswitch.reg = sdiohost_getreg(host, id, SDIO_FIFO_SWITCH_REG_OFS);
	fifoswitch.bit.FIFO_SWITCH_DLY = 1;
	sdiohost_setreg(host, id, SDIO_FIFO_SWITCH_REG_OFS, fifoswitch.reg);
	/* patch end for sd write hang-up or write byte access error */

	phyreg.reg = sdiohost_getreg(host, id, SDIO_PHY_REG_OFS);
	phyreg.bit.PHY_SW_RST = 1;
	sdiohost_setreg(host, id, SDIO_PHY_REG_OFS, phyreg.reg);

	while (1) {
		phyregread.reg = sdiohost_getreg(host, id, SDIO_PHY_REG_OFS);
		if (phyregread.bit.PHY_SW_RST == 0) {
			break;
		}
	}

	dlyreg1.reg = sdiohost_getreg(host, id, SDIO_DLY1_REG_OFS);
	dlyreg1.bit.DATA_READ_DLY = 2;
	sdiohost_setreg(host, id, SDIO_DLY1_REG_OFS, dlyreg1.reg);
}

/*
	Enable FIFO 512 Bytes (Only SDIO3).

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_3: SDIO3

	@return void
*/
void sdiohost_setblkfifoen(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_PHY_REG         phyreg;

	phyreg.reg = sdiohost_getreg(host, host->id, SDIO_PHY_REG_OFS);
	phyreg.bit.BLK_FIFO_EN = enable;
	sdiohost_setreg(host, host->id, SDIO_PHY_REG_OFS, phyreg.reg);
}

/*
	Send SD command to SD bus.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] cmd      command value
	@param[in] rsptype  response type
			- @b SDIO_HOST_RSP_NONE: no response is required
			- @b SDIO_HOST_RSP_SHORT: need short (32 bits) response
			- @b SDIO_HOST_RSP_LONG: need long (128 bits) response
	@param[in] beniointdetect enable SDIO INT detect after command end
			- @b TRUE: enable SDIO INT detection
			- @b FALSE: keep SDIO INT detection

	@return command result
	- @b SDIO_HOST_CMD_OK: command execution success
	- @b SDIO_HOST_RSP_TIMEOUT: response timeout. no response got from card.
	- @b SDIO_HOST_RSP_CRCFAIL: response CRC fail.
	- @b SDIO_HOST_CMD_FAIL: other fail.
*/
int sdiohost_sendcmd(struct mmc_nvt_host *host, uint32_t id, uint32_t cmd,
SDIO_HOST_RESPONSE rsptype, bool beniointdetect)
{

	union SDIO_CMD_REG cmdreg;
#if (!SDIO_SCATTER_DMA)
	/* Temp solution for FPGA */
	sdiohost_setreg(host, id, 0x1FC, 1);
#endif
	/*cmdreg.reg = 0;*/
	cmdreg.reg = sdiohost_getreg(host, id, SDIO_CMD_REG_OFS);
	cmdreg.bit.CMD_IDX = 0;
	cmdreg.bit.NEED_RSP = 0;
	cmdreg.bit.LONG_RSP = 0;
	cmdreg.bit.RSP_TIMEOUT_TYPE = 0;
	cmdreg.bit.ENABLE_SDIO_INT_DETECT = beniointdetect;
	if (rsptype != SDIO_HOST_RSP_NONE) {
		/* Need response */
		cmdreg.bit.NEED_RSP = 1;

		switch (rsptype) {

		default:
			break;

		case SDIO_HOST_RSP_LONG:
			cmdreg.bit.LONG_RSP = 1;
			break;

		case SDIO_HOST_RSP_SHORT_TYPE2:
			cmdreg.bit.RSP_TIMEOUT_TYPE = 1;
			break;

		case SDIO_HOST_RSP_LONG_TYPE2:
			cmdreg.bit.RSP_TIMEOUT_TYPE = 1;
			cmdreg.bit.LONG_RSP = 1;
			break;
		}
	}
	cmdreg.bit.CMD_IDX = cmd;
	sdiohost_setreg(host, id, SDIO_CMD_REG_OFS, cmdreg.reg);

	if (id >= SDIO_HOST_ID_COUNT)
		return SDIO_HOST_CMD_FAIL;

	/* clear software status */
	/*clr_flg(FLG_ID_SDIO, vsdiohosts[id].flgCommand);*/

	/* Start command/data transmits */
	cmdreg.bit.CMD_EN = 1;
	sdiohost_setreg(host, id, SDIO_CMD_REG_OFS, cmdreg.reg);

	return SDIO_HOST_CMD_OK;
}

/*
	Set SDIO command argument.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] arg command argument
			- possible value: 0x0000_0000 ~ 0xFFFF_FFFF

	@return void
*/
void sdiohost_setarg(struct mmc_nvt_host *host, uint32_t id, uint32_t arg)
{
	sdiohost_setreg(host, id, SDIO_ARGU_REG_OFS, arg);
}

/*
	Get SDIO command response.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[out] prsp command response from card

	@return void
*/
void sdiohost_getshortrsp(struct mmc_nvt_host *host, uint32_t id,
uint32_t *prsp)
{
	union SDIO_RSP0_REG rspreg;

	rspreg.reg = sdiohost_getreg(host, id, SDIO_RSP0_REG_OFS);
	*prsp = (uint32_t) rspreg.reg;
}

/*
	Get SDIO command long response.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[out] prsp3 command response from card (bit 127..96)
	@param[out] prsp2 command response from card (bit 95..64)
	@param[out] prsp1 command response from card (bit 63..32)
	@param[out] prsp0 command response from card (bit 31..0)

	@return void
*/
void sdiohost_getlongrsp(struct mmc_nvt_host *host, uint32_t id,
uint32_t *prsp3, uint32_t *prsp2, uint32_t *prsp1, uint32_t *prsp0)
{
	union SDIO_RSP0_REG rsp0reg;
	union SDIO_RSP1_REG rsp1reg;
	union SDIO_RSP2_REG rsp2reg;
	union SDIO_RSP3_REG rsp3reg;

	rsp0reg.reg = sdiohost_getreg(host, id, SDIO_RSP0_REG_OFS);
	*prsp0 = (uint32_t) rsp0reg.reg;
	rsp1reg.reg = sdiohost_getreg(host, id, SDIO_RSP1_REG_OFS);
	*prsp1 = (uint32_t) rsp1reg.reg;
	rsp2reg.reg = sdiohost_getreg(host, id, SDIO_RSP2_REG_OFS);
	*prsp2 = (uint32_t) rsp2reg.reg;
	rsp3reg.reg = sdiohost_getreg(host, id, SDIO_RSP3_REG_OFS);
	*prsp3 = (uint32_t) rsp3reg.reg;
}

/*
	Enable SDIO interrupt.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] bits SDIO controller interrupt enable bits

	@return void
*/
void sdiohost_setinten(struct mmc_nvt_host *host, uint32_t id, uint32_t bits)
{
	union SDIO_INT_MASK_REG intenreg;
	unsigned long flags;

	spin_lock_irqsave(&host->int_mask_lock, flags);

	intenreg.reg = sdiohost_getreg(host, id, SDIO_INT_MASK_REG_OFS);
	intenreg.reg |= bits;
	sdiohost_setreg(host, id, SDIO_INT_MASK_REG_OFS, intenreg.reg);

	spin_unlock_irqrestore(&host->int_mask_lock, flags);
}

/*
	Disable SDIO interrupt.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] bits SDIO controller interrupt enable bits

	@return void
*/
void sdiohost_disinten(struct mmc_nvt_host *host, uint32_t id, uint32_t bits)
{
	union SDIO_INT_MASK_REG intenreg;
	unsigned long flags;

	spin_lock_irqsave(&host->int_mask_lock, flags);

	intenreg.reg = sdiohost_getreg(host, id, SDIO_INT_MASK_REG_OFS);
	intenreg.reg &= ~bits;
	sdiohost_setreg(host, id, SDIO_INT_MASK_REG_OFS, intenreg.reg);

	spin_unlock_irqrestore(&host->int_mask_lock, flags);
}

/*
	Set SDIO bus width.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] Width    SDIO controller bus width
			- @b SDIO_BUS_WIDTH1: 1 bit data bus
			- @b SDIO_BUS_WIDTH4: 4 bits data bus
			- @b SDIO_BUS_WIDTH8: 8 bits data bus

	@return void
*/
void sdiohost_setbuswidth(struct mmc_nvt_host *host, uint32_t id,
uint32_t width)
{
	union SDIO_BUS_WIDTH_REG widthreg;

	widthreg.reg = 0;
	widthreg.bit.BUS_WIDTH = width;
	sdiohost_setreg(host, id, SDIO_BUS_WIDTH_REG_OFS, widthreg.reg);
}

/*
	Get SDIO bus width.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return SDIO controller bus Width
			- @b SDIO_BUS_WIDTH1
			- @b SDIO_BUS_WIDTH4
			- @b SDIO_BUS_WIDTH8
*/
uint32_t sdiohost_getbuswidth(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_BUS_WIDTH_REG widthreg;

	widthreg.reg = sdiohost_getreg(host, id, SDIO_BUS_WIDTH_REG_OFS);

	return widthreg.bit.BUS_WIDTH;
}

/*
	Set SDIO bus clock.

	@param[in] id       SD host ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] uiclock  SD bus clock in Hz
	@param[out] ns  ns one cycle

	@return void
*/
void sdiohost_setbusclk(struct mmc_nvt_host *host, uint32_t id,
uint32_t uiclock, uint32_t *ns)
{
	clk_set_rate(host->clk, uiclock);

	if (ns)
		*ns = (1000000) / (uiclock/1000);
}

/*
	Get SDIO bus clock.

	@param[in] id       SD host ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return unit of Hz
*/
uint32_t sdiohost_getbusclk(struct mmc_nvt_host *host, uint32_t id)
{
	return clk_get_rate(host->clk);
}

/*
	Get SDIO controller block size.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return data size per block (unit: byte) (0x0001 ~ 0xFFFF)
*/
uint32_t sdiohost_getblksize(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_DATA_CTRL_REG datactrlreg;

	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);
	return datactrlreg.bit.BLK_SIZE;
}

/*
	Set SDIO controller block size.

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] size data size per block (unit: byte)
			- possible value: 0x0001 ~ 0xFFFF

	@return void
*/
void sdiohost_setblksize(struct mmc_nvt_host *host, uint32_t id, uint32_t size)
{
	union SDIO_DATA_CTRL_REG datactrlreg;

	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.BLK_SIZE = size;
	sdiohost_setreg(host, id, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);
}

/*
	Set SDIO controller data timeout.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] timeout  time out value between data blocks (unit: SD clock)

	@return void
*/
void sdiohost_setdatatimeout(struct mmc_nvt_host *host, uint32_t id,
uint32_t timeout)
{
	union SDIO_DATA_TIMER_REG timerreg;

	timerreg.bit.TIMEOUT = timeout;
	sdiohost_setreg(host, id, SDIO_DATA_TIMER_REG_OFS, timerreg.reg);
}

/*
	Reset SDIO controller data state machine.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return void
*/
void sdiohost_resetdata(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_DATA_CTRL_REG    datactrlreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
	/* //#NT#Fix SDIO data state machine abnormal when DATA CRC/Timeout
	occurs before FIFO count complete */
	union SDIO_STATUS_REG	stsreg;

	/* SDIO bug: force to clear data end status to exit
	waiting data end state*/
	stsreg.reg          = 0;
	stsreg.bit.DATA_END = 1;
	sdiohost_setreg(host, id, SDIO_STATUS_REG_OFS, stsreg.reg);


	fifoctrlreg.reg = 0;
	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	while (1) {
		fifoctrlreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_CONTROL_REG_OFS);
		if (fifoctrlreg.bit.FIFO_EN == 0)
			break;
	}

	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.DATA_EN = 0;
	sdiohost_setreg(host, id, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	/* Fix SDIO data state machine abnormal when DATA
	CRC/Timeout occurs before FIFO count complete */
	/* Do software reset to reset SD state machine */
	sdiohost_reset(host, id);
}

/*
	HS400_SYNC_OPTION wrapper.

	Check 0x38 register value since HS400_SYNC_OPTION is write only.
	If HS400_SYNC_OPTION_STA = 1, set HS400_SYNC_OPTION to 1.

	@param[in] id	DIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
			- @b SDIO_HOST_ID_3: SDIO3
	@param[in] offset   offset in SDIO controller (word alignment)
	@param[in] value    register value

	@return 0x38 register value after wrap.
*/
#if defined(SYNC_OPTION_FIX)
static uint32_t sdiohost_check_hs400opt(struct mmc_nvt_host *host, uint32_t reg)
{
	// do nothing
    return reg;
}
#else
static uint32_t sdiohost_check_hs400opt(struct mmc_nvt_host *host, uint32_t reg)
{
    union SDIO_CLOCK_CTRL_REG clk_ctrl;

    clk_ctrl.reg = reg;
    clk_ctrl.bit.HS400_SYNC_OPTION = clk_ctrl.bit.HS400_SYNC_OPTION_STA;
    return clk_ctrl.reg;
}
#endif

void sdiohost_delayd(struct mmc_nvt_host *host, uint32_t uid)
{
	uint32_t i, uidummy;

	for (i = uid; i; i--)
		uidummy = sdiohost_getreg(host, 0, SDIO_CMD_REG_OFS);

}

void sdiohost_waitfifoempty(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_FIFO_STATUS_REG fifostsreg;
	uint32_t read0, read1;

	read0 = 0;
	while (1) {
		fifostsreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_STATUS_REG_OFS);
		read1 = fifostsreg.bit.FIFO_EMPTY;
		if (read0 & read1)
			break;
		else
			read0 = read1;
	}
}
void sdiohost_clrfifoen(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;

	fifoctrlreg.reg = sdiohost_getreg(host, id, SDIO_FIFO_CONTROL_REG_OFS);
	fifoctrlreg.bit.FIFO_EN = 0;
	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
}
uint32_t sdiohost_getfifodir(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;

	fifoctrlreg.reg = sdiohost_getreg(host, id, SDIO_FIFO_CONTROL_REG_OFS);

	return fifoctrlreg.bit.FIFO_DIR;
}

void sdiohost_set_detmode(struct mmc_nvt_host *host, bool det_mode)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, host->id, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DET_MODE = det_mode;
	sdiohost_setreg(host, host->id, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

/*
	Setup SDIO controller data transfer in DMA mode.

	@param[in] id           SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] uidmaaddress buffer DRAM address
			- possible value: 0x000_0000 ~ 0xFFF_FFFF
	@param[in] uidatalength total transfer length
			- possible value: 0x000_0001 ~ 0x3FF_FFFF
	@param[in] bisread      read/write mode
			- @b TRUE: indicate data read transfer
			- @b FALSE: indicate data write transfer

	@return void
*/
void sdiohost_setupdatatransferdma(struct mmc_nvt_host *host, uint32_t id,
uintptr_t uidmaaddress, uint32_t uidatalength, bool bisread, uint32_t *vuisdio_destab)
{
	union SDIO_DATA_CTRL_REG datactrlreg;
	union SDIO_DATA_LENGTH_REG datalenreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
#if (SDIO_SCATTER_DMA)
	dma_addr_t uidmadesaddr;
	union SDIO_DMA_DES_START_ADDR_REG dmadesaddrreg;
	union SDIO_DMA_DES_HIGH_START_ADDR_REG h_dmadesaddrreg;
	union SDIO_DES_LINE_REG dmadeslinereg;
	union SDIO_DES_LINE1_REG dmadesline1reg;
	union SDIO_DES_LINE2_REG dmadesline2reg;
	/*uint32_t i;*/
#else
	union SDIO_DMA_START_ADDR_REG dmaaddrreg;
	UINT32 dma_high_address = 0;
#endif
	//uint32_t uibusclk;

	/* for debug */
	/*fifoctrlreg.reg = sdiohost_getreg(id, SDIO_FIFO_CONTROL_REG_OFS);
	if (fifoctrlreg.bit.FIFO_EN == 1) {
		fifoctrlreg.reg = 0;
		sdiohost_setreg(id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
	}*/
	/* for debug end */
#if 0
	/* dummy read for patch */
	sdiohost_delayd(host, 2);

	uibusclk = sdiohost_getbusclk(host, id);

	if (uibusclk >= 48000000)
		sdiohost_delayd(host, 3);
	else if ((uibusclk >= 24000000) && (uibusclk < 48000000))
		sdiohost_delayd(host, 6);
	else if ((uibusclk >= 12000000) && (uibusclk < 24000000))
		sdiohost_delayd(host, 9);
	else
		sdiohost_delayd(host, 21);

	/* patch for sd fifo bug end */
#endif
	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);

#if 0
	/* multiple read => disable SDIO INT detection after transfer end */
	if (bisread && (uidatalength > datactrlreg.bit.BLK_SIZE))
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 1;
	else
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 0;
#else
	/*
	 * The DIS_SDIO_INT_PERIOD is designed for reading infinite blocks.
	 * But there are some compatibility and performance issues, so it should not be used.
	 */
	datactrlreg.bit.DIS_SDIO_INT_PERIOD = 0;
#endif

	/*move data en after fifo en*/
	/*datactrlreg.bit.DATA_EN = 1;*/
	sdiohost_setreg(host, id, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

#if (SDIO_SCATTER_DMA)
	if (vsdio_seg_en[id] == FALSE) {
		dmadeslinereg.reg = 0;
		dmadesline1reg.reg = 0;
		dmadesline2reg.reg = 0;

		dmadeslinereg.bit.DMA_DES_VALID = 1;
		dmadeslinereg.bit.DMA_DES_END = 1;
		dmadeslinereg.bit.DMA_DES_ACT = 2;

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (uidmaaddress & SDIO_HIGH_ADDR_MASK) {
			dmadeslinereg.bit.DMA_DES_HIGH = (uidmaaddress & SDIO_HIGH_ADDR_MASK) >> 32;
		} else {
			dmadeslinereg.bit.DMA_DES_HIGH = 0x0;
		}
#else
		dmadeslinereg.bit.DMA_DES_HIGH = 0x0;
#endif

		dmadesline1reg.bit.DMA_DES_DATA_LEN = uidatalength;
		dmadesline2reg.bit.DMA_DES_DATA_ADDR = uidmaaddress;

		vuisdio_destab[SDIO_DES_LINE_REG_OFS] =
			(uint32_t)dmadeslinereg.reg;
		vuisdio_destab[SDIO_DES_LINE1_REG_OFS>>2] =
			(uint32_t)dmadesline1reg.reg;
		vuisdio_destab[SDIO_DES_LINE2_REG_OFS>>2] =
			(uint32_t)dmadesline2reg.reg;
	}

	dmadesaddrreg.reg = 0;
	uidmadesaddr = virt_to_phys((uint32_t*)&vuisdio_destab[0]);
	dmadesaddrreg.bit.DES_ADDR = uidmadesaddr;
	sdiohost_setreg(host, id, SDIO_DMA_DES_START_ADDR_REG_OFS, dmadesaddrreg.reg);

	h_dmadesaddrreg.reg = 0;
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	if (uidmadesaddr & SDIO_HIGH_ADDR_MASK) {
		h_dmadesaddrreg.bit.DES_HIGH_ADDR = (uidmadesaddr & SDIO_HIGH_ADDR_MASK) >> 32;
	} else {
		h_dmadesaddrreg.bit.DES_HIGH_ADDR = 0x0;
	}
#else
	h_dmadesaddrreg.bit.DES_HIGH_ADDR = 0x0;
#endif
	sdiohost_setreg(host, id, SDIO_DMA_DES_HIGH_START_ADDR_REG_OFS, h_dmadesaddrreg.reg);

	datalenreg.reg = 0;
	datalenreg.bit.LENGTH = uidatalength;
	sdiohost_setreg(host, id, SDIO_DATA_LENGTH_REG_OFS, datalenreg.reg);

	fifoctrlreg.reg = 0;

	if (vsdio_seg_en[id] == FALSE) {
		/* Flush cache in data DMA*/
		if (!bisread) {
			dma_flushwritecache(mmc_dev(host->mmc), (void *)uidmaaddress, uidatalength);
			fifoctrlreg.bit.FIFO_DIR = 1;
		} else {
			dma_flushreadcache(mmc_dev(host->mmc), (void *)uidmaaddress, uidatalength);
		}

		/* Flush cache in Des*/
		dma_flushbidircache(mmc_dev(host->mmc), (void *)&vuisdio_destab[0],
		SDIO_DES_WORD_SIZE<<2);
	} else {
		#if 0
		for (i = 0; i < vsdio_seg_num[id]; i++) {
			/* Flush cache in data DMA*/
			if (!bisread) {
				dma_flushwritecache(mmc_dev(host->mmc), (void *)(mmc_dev(host->mmc), (vuisdio_destab
				[(i * SDIO_DES_WORD_SIZE) +
				(SDIO_DES_LINE2_REG_OFS>>2)])),
				vuisdio_destab[(i * SDIO_DES_WORD_SIZE +
				SDIO_DES_LINE1_REG_OFS)>>2]);
				fifoctrlreg.bit.FIFO_DIR = 1;
			} else {
				dma_flushreadcache(mmc_dev(host->mmc), (void *)(mmc_dev(host->mmc), (vuisdio_destab
				[(i * SDIO_DES_WORD_SIZE) +
				(SDIO_DES_LINE2_REG_OFS>>2)])),
				vuisdio_destab[(i * SDIO_DES_WORD_SIZE +
				SDIO_DES_LINE1_REG_OFS)>>2]);
			}

		}
		#else
		if (!bisread)
			fifoctrlreg.bit.FIFO_DIR = 1;
		else
			fifoctrlreg.bit.FIFO_DIR = 0;
		#endif

		/* Flush cache in Des*/
		dma_flushbidircache(mmc_dev(host->mmc), (void *)(&vuisdio_destab[0]),
		(SDIO_DES_WORD_SIZE<<2)*SDIO_DES_TABLE_NUM);
	}
#else
	dmaaddrreg.reg = 0;
	dmaaddrreg.bit.DRAM_ADDR = virt_to_phys((uint32_t *)uidmaaddress);
	sdiohost_setreg(host, id, SDIO_DMA_START_ADDR_REG_OFS, dmaaddrreg.reg);

	dma_high_address = virt_to_phys((uint32_t *)uidmaaddress);
	dma_high_address = (dma_high_address & SDIO_HIGH_ADDR_MASK) >> 32;
	sdiohost_setreg(host, id, SDIO_DMA_DES_HIGH_START_ADDR_REG_OFS, dma_high_address);

	datalenreg.reg = 0;
	datalenreg.bit.LENGTH = uidatalength;
	sdiohost_setreg(host, id, SDIO_DATA_LENGTH_REG_OFS, datalenreg.reg);

	fifoctrlreg.reg = 0;

	/* Flush cache in DMA mode*/
	if (!bisread) {
		dma_flushwritecache(mmc_dev(host->mmc), (void *)uidmaaddress, uidatalength);
		fifoctrlreg.bit.FIFO_DIR = 1;
	} else {
		dma_flushreadcache(mmc_dev(host->mmc), (void *)uidmaaddress, uidatalength);
	}
#endif

	fifoctrlreg.bit.FIFO_MODE = 1;
	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.DATA_EN = 1;
	sdiohost_setreg(host, id, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	fifoctrlreg.bit.FIFO_EN = 1;
	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
}

/*
	Setup SDIO controller data transfer in PIO mode.

	@param[in] id           SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] uidmaaddress buffer DRAM address
			- possible value: 0x000_0000 ~ 0xFFF_FFFF
	@param[in] uidatalength total transfer length
			- possible value: 0x000_0001 ~ 0x3FF_FFFF
	@param[in] bisread      read/write mode
			- @b TRUE: indicate data read transfer
			- @b FALSE: indicate data write transfer

	@return void
*/
void sdiohost_setupdatatransferpio(struct mmc_nvt_host *host, uint32_t id,
uintptr_t uidmaaddress, uint32_t uidatalength, bool bisread)
{
	union SDIO_DATA_CTRL_REG datactrlreg;
	union SDIO_DATA_LENGTH_REG datalenreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
#if (!SDIO_SCATTER_DMA)
	union SDIO_DMA_START_ADDR_REG dmaaddrreg;
#endif
	//uint32_t uibusclk;
#if 0
	/* dummy read for patch */
	sdiohost_delayd(host, 2);

	uibusclk = sdiohost_getbusclk(host, id);

	if (uibusclk >= 48000000)
		sdiohost_delayd(host, 3);
	else if ((uibusclk >= 24000000) && (uibusclk < 48000000))
		sdiohost_delayd(host, 6);
	else if ((uibusclk >= 12000000) && (uibusclk < 24000000))
		sdiohost_delayd(host, 9);
	else
		sdiohost_delayd(host, 21);

	/* patch for sd fifo bug end */
#endif

	datactrlreg.reg = sdiohost_getreg(host, id, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.DIS_SDIO_INT_PERIOD = 0;

	datactrlreg.bit.DATA_EN = 1;
	sdiohost_setreg(host, id, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);


#if (!SDIO_SCATTER_DMA)
	dmaaddrreg.reg = 0;
	sdiohost_setreg(host, id, SDIO_DMA_START_ADDR_REG_OFS, dmaaddrreg.reg);
#endif

	datalenreg.reg = 0;
	datalenreg.bit.LENGTH = uidatalength;
	sdiohost_setreg(host, id, SDIO_DATA_LENGTH_REG_OFS, datalenreg.reg);

	fifoctrlreg.reg = 0;

	if (!bisread)
		fifoctrlreg.bit.FIFO_DIR = 1;

	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	fifoctrlreg.bit.FIFO_EN = 1;
	sdiohost_setreg(host, id, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
}

/*
	Write SDIO data blocks.


	@note This function should only be called in PIO mode.

	@param[in] id           SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] pbuf         buffer DRAM address
	@param[in] uiLength     total length (block alignment)

	@return
			- @b E_OK: success
			- @b E_SYS: data CRC or data timeout error
*/
int sdiohost_writeblock(struct mmc_nvt_host *host, uint32_t id, uint8_t *pbuf,
uint32_t uilength)
{
	uint32_t  uiwordcount, i, *pbufword;
	uint32_t  uifullcount, uiremaincount;
	bool    bwordalignment;
	union SDIO_DATA_PORT_REG    datareg;
	union SDIO_FIFO_STATUS_REG  fifostsreg;
	uint32_t dstatus;

	uiwordcount     = (uilength + sizeof(uint32_t) - 1) / sizeof(uint32_t);
	uifullcount     = uiwordcount / SDIO_HOST_DATA_FIFO_DEPTH;
	uiremaincount   = uiwordcount % SDIO_HOST_DATA_FIFO_DEPTH;
	pbufword        = (uint32_t *)pbuf;

	if ((uintptr_t)pbuf & (uintptr_t)0x3)
		bwordalignment = FALSE;
	else
		bwordalignment = TRUE;

	while (uifullcount) {
		fifostsreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_STATUS_REG_OFS);

		if (fifostsreg.bit.FIFO_EMPTY) {
			if (bwordalignment == TRUE) {
				/* Word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					sdiohost_setreg(host, id,
					SDIO_DATA_PORT_REG_OFS, *pbufword++);
				}
			} else {
				/* Not word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					datareg.reg = *pbuf++;
					datareg.reg |= (*pbuf++) << 8;
					datareg.reg |= (*pbuf++) << 16;
					datareg.reg |= (*pbuf++) << 24;

					sdiohost_setreg(host, id,
					SDIO_DATA_PORT_REG_OFS, datareg.reg);
				}
			}

			uifullcount--;
		}

		dstatus = sdiohost_getstatus(host, id);
		if ((dstatus & SDIO_STATUS_REG_DATA_CRC_FAIL) || (dstatus & SDIO_STATUS_REG_DATA_TIMEOUT)) {
			return E_SYS;
		}
	}

	if (uiremaincount) {
		while (1) {
			fifostsreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_STATUS_REG_OFS);

			if (fifostsreg.bit.FIFO_EMPTY)
				break;

			dstatus = sdiohost_getstatus(host, id);
			if ((dstatus & SDIO_STATUS_REG_DATA_CRC_FAIL) || (dstatus & SDIO_STATUS_REG_DATA_TIMEOUT)) {
				return E_SYS;
			}
		}

		if (bwordalignment == TRUE) {
			/* Word alignment*/
			for (i = uiremaincount; i; i--) {
				sdiohost_setreg(host, id,
				SDIO_DATA_PORT_REG_OFS, *pbufword++);
			}
		} else {
			/* Not word alignment*/
			for (i = uiremaincount; i; i--) {
				datareg.reg = *pbuf++;
				datareg.reg |= (*pbuf++) << 8;
				datareg.reg |= (*pbuf++) << 16;
				datareg.reg |= (*pbuf++) << 24;

				sdiohost_setreg(host, id,
				SDIO_DATA_PORT_REG_OFS, datareg.reg);
			}
		}
	}

	return E_OK;
}

/*
	Read SDIO data blocks.

	@note This function should only be called in PIO mode.

	@param[in] id           SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[out] pbuf        buffer DRAM address
	@param[in] uiLength     total length (block alignment)

	@return
		- @b E_OK: success
		- @b E_SYS: data CRC or data timeout error
*/
int sdiohost_readblock(struct mmc_nvt_host *host, uint32_t id, uint8_t *pbuf,
uint32_t uilength)
{
	uint32_t  uiwordcount, i, *pbufword;
	uint32_t  uifullcount, uiremaincount;
	bool    bwordalignment;
	union SDIO_DATA_PORT_REG    datareg;
	union SDIO_FIFO_STATUS_REG  fifostsreg;
	uint32_t dstatus;

	uiwordcount     = (uilength + sizeof(uint32_t) - 1) / sizeof(uint32_t);
	uifullcount     = uiwordcount / SDIO_HOST_DATA_FIFO_DEPTH;
	uiremaincount   = uiwordcount % SDIO_HOST_DATA_FIFO_DEPTH;
	pbufword        = (uint32_t *)pbuf;

	if ((uintptr_t)pbuf & (uintptr_t)0x3)
		bwordalignment = FALSE;
	else
		bwordalignment = TRUE;

	while (uifullcount) {
		fifostsreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_STATUS_REG_OFS);

		if (fifostsreg.bit.FIFO_FULL) {
			if (bwordalignment == TRUE) {
				/* Word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					*pbufword++ = sdiohost_getreg(host, id,
					SDIO_DATA_PORT_REG_OFS);
				}
			} else {
				/* Not word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					datareg.reg = sdiohost_getreg(host, id,
					SDIO_DATA_PORT_REG_OFS);

					*pbuf++ = datareg.reg & 0xFF;
					*pbuf++ = (datareg.reg>>8) & 0xFF;
					*pbuf++ = (datareg.reg>>16) & 0xFF;
					*pbuf++ = (datareg.reg>>24) & 0xFF;
				}
			}

			uifullcount--;
		}

		dstatus = sdiohost_getstatus(host, id);
		if ((dstatus & SDIO_STATUS_REG_DATA_CRC_FAIL) || (dstatus & SDIO_STATUS_REG_DATA_TIMEOUT)) {
			return E_SYS;
		}
	}

	if (uiremaincount) {
		while (1) {
			fifostsreg.reg = sdiohost_getreg(host, id,
			SDIO_FIFO_STATUS_REG_OFS);

			if (fifostsreg.bit.FIFO_CNT == uiremaincount)
				break;

			dstatus = sdiohost_getstatus(host, id);
			if ((dstatus & SDIO_STATUS_REG_DATA_CRC_FAIL) || (dstatus & SDIO_STATUS_REG_DATA_TIMEOUT)) {
				return E_SYS;
			}
		}

		if (bwordalignment == TRUE) {
			/* Word alignment*/
			for (i = uiremaincount; i; i--) {
				*pbufword++ = sdiohost_getreg(host, id,
				SDIO_DATA_PORT_REG_OFS);
			}
		} else {
			/* Not word alignment*/
			for (i = uiremaincount; i; i--) {
				datareg.reg = sdiohost_getreg(host, id,
				SDIO_DATA_PORT_REG_OFS);

				*pbuf++ = datareg.reg & 0xFF;
				*pbuf++ = (datareg.reg>>8) & 0xFF;
				*pbuf++ = (datareg.reg>>16) & 0xFF;
				*pbuf++ = (datareg.reg>>24) & 0xFF;
			}
		}
	}

	return E_OK;
}

uint32_t sdiohost_setiointen(struct mmc_nvt_host *host, uint32_t id, bool ben)
{
	union SDIO_INT_MASK_REG intenreg;
	unsigned long flags;

	spin_lock_irqsave(&host->int_mask_lock, flags);

	intenreg.reg = sdiohost_getreg(host, id, SDIO_INT_MASK_REG_OFS);
	intenreg.bit.SDIO_INT_INT_EN = ben;
	sdiohost_setreg(host, id, SDIO_INT_MASK_REG_OFS, intenreg.reg);

	spin_unlock_irqrestore(&host->int_mask_lock, flags);

	return TRUE;
}

bool sdiohost_getiointen(struct mmc_nvt_host *host, uint32_t id)
{
    union SDIO_INT_MASK_REG intenreg;
    unsigned long flags;
    bool result = FALSE;

    spin_lock_irqsave(&host->int_mask_lock, flags);

    intenreg.reg = sdiohost_getreg(host, id, SDIO_INT_MASK_REG_OFS);

    if (intenreg.bit.SDIO_INT_INT_EN) {
        result = TRUE;
    }

    spin_unlock_irqrestore(&host->int_mask_lock, flags);

    return result;
}
/** ********************************************************************


  Private(in driver layer) SDIO host controller functions


***********************************************************************/

/*
    Get SDIO controller register.

    @param[in] id           SDIO channel ID
					- @b SDIO_HOST_ID_1: SDIO
					- @b SDIO_HOST_ID_2: SDIO2
    @param[in] offset       register offset in SDIO controller (word alignment)

    @return register value
*/
static REGVALUE sdiohost_getreg(struct mmc_nvt_host *host, uint32_t id,
uint32_t offset)
{
	REGVALUE value;

	value = nvt_readl(host->base + offset);

	return value;
}

/*
	Set SDIO controller register.

	@param[in] id	DIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] offset   offset in SDIO controller (word alignment)
	@param[in] value    register value

	@return void
*/
static void sdiohost_setreg(struct mmc_nvt_host *host, uint32_t id,
uint32_t offset, REGVALUE value)
{
	nvt_writel(value, host->base + offset);
}

/*uint32_t Flag = 0;*/
#include <linux/delay.h>

void sdiohost_delay(uint32_t uius)
{
	udelay(uius);
}

int sdiohost_sendsdcmd(struct mmc_nvt_host *host, uint32_t id, uint32_t cmdpart,
uint32_t param)
{
	int status;
	bool benintdetect = FALSE;
	SDIO_HOST_RESPONSE rsptype = SDIO_HOST_RSP_NONE;
#if 0
	pr_info("CMD%d arg %x %x\r\n",
	cmdpart&(~(SDIO_CMD_REG_APP_CMD | SDIO_CMD_REG_VOLTAGE_SWITCH_DETECT
	| SDIO_CMD_REG_ABORT | SDIO_CMD_REG_NEED_RSP | SDIO_CMD_REG_LONG_RSP
	| SDIO_CMD_REG_RSP_TYPE2)),
	param, cmdpart);
#endif
	if ((cmdpart & SDIO_CMD_REG_LONG_RSP) == SDIO_CMD_REG_LONG_RSP) {
		if (cmdpart & SDIO_CMD_REG_RSP_TYPE2)
			rsptype = SDIO_HOST_RSP_LONG_TYPE2;
		else
			rsptype = SDIO_HOST_RSP_LONG;
	} else if (cmdpart & SDIO_CMD_REG_VOLTAGE_SWITCH_DETECT) {
		rsptype = SDIO_HOST_RSP_VOLT_DETECT;
	} else if (cmdpart & SDIO_CMD_REG_NEED_RSP) {

		if (cmdpart & SDIO_CMD_REG_RSP_TYPE2)
			rsptype = SDIO_HOST_RSP_SHORT_TYPE2;
		else
			rsptype = SDIO_HOST_RSP_SHORT;
	}

	if (cmdpart & SDIO_CMD_REG_ABORT)
		benintdetect = TRUE;

	sdiohost_setarg(host, id, param);
	/*pr_info("bEnIntDetect %d\r\n",bEnIntDetect);*/

	status = sdiohost_sendcmd(host, id, cmdpart&SDIO_CMD_REG_INDEX,
	rsptype, benintdetect);

	return E_OK;
}
static void sdiohost_axi_enable(struct mmc_nvt_host *host, u32 enable)
{
	union SDIO_AXI0_REG axi0_reg;

	axi0_reg.reg = sdiohost_getreg(host, host->id, SDIO_AXI0_REG_OFS);
	axi0_reg.bit.AXI_CH_DIS = !enable;
	sdiohost_setreg(host, host->id, SDIO_AXI0_REG_OFS, axi0_reg.reg);
}


int sdiohost_open(struct mmc_nvt_host *host, uint32_t id, int voltage_switch)
{
	sdiohost_setpaddriving(host, SDIO_MODE_DS);

	sdiohost_resetdata(host, id);

	sdiohost_resetasync(host, id);
	
	sdiohost_axi_enable(host, true);

	sdiohost_setphysample(host, false, true);

	/* Pass TG1.1-15: init freq should <=400Khz. */
	/* Set to 399KHz to guarantee pass TG1.1-15 */
	/*sdiohost_setbusclk(id, 399000, &ns);*/

	/* Delay 1 ms (SD spec) after clock is outputted. */
	/* (Delay 1024 us to reduce code size) */
	sdiohost_delay(1024);

	/*sdioHost_setClkGating(id, bClkGating);*/
	/* restore clock gating*/

	/* Interrupt Enable */
	sdiohost_setinten(host, id,
		SDIO_STATUS_REG_DATA_TIMEOUT |
		SDIO_STATUS_REG_DATA_CRC_FAIL|
		SDIO_STATUS_REG_DATA_CRC_OK |
		SDIO_STATUS_REG_DATA_END |
		SDIO_STATUS_REG_RSP_CRC_FAIL |
		SDIO_STATUS_REG_RSP_TIMEOUT |
		SDIO_STATUS_REG_RSP_CRC_OK |
		SDIO_STATUS_REG_CMD_SEND);

	sdiohost_setdatatimeout(host, id, 0x10000000);

	/*sdiohost_enclockout(host, id);*/

	return E_OK;
}


void sdiohost_setstatus(struct mmc_nvt_host *host, uint32_t id,
uint32_t status)
{
	sdiohost_setreg(host, id, SDIO_STATUS_REG_OFS, status);
}

uint32_t sdiohost_getstatus(struct mmc_nvt_host *host, uint32_t id)
{
	return sdiohost_getreg(host, id, SDIO_STATUS_REG_OFS);
}

static void extern_pad_set_power(struct mmc_nvt_host *host, PAD_POWER_STRUCT *pad_power)
{
	if (pad_power->pad_power == PAD_1P8V) {
		gpio_direction_output(host->ldo_gpio, host->ldo_gpio_value);
	} else {
		gpio_direction_output(host->ldo_gpio, !host->ldo_gpio_value);
	}
}

void sdiohost_setpower(struct mmc_nvt_host *host, SDIO_HOST_SETPOWER_VOL vol)
{
	PAD_POWER_STRUCT pad_power;
	PAD_POWERID power_id;
	PAD_POWER power_vol;

	if (host->host_power == vol) return;

	memset(&pad_power, 0, sizeof(PAD_POWER_STRUCT));

	pad_power.enable = 1;

	if (host->id == SDIO_HOST_ID_1) {
		power_id = PAD_POWERID_MC0;
	} else if (host->id == SDIO_HOST_ID_2) {
		power_id = PAD_POWERID_MC1;
		if (nvt_get_chip_id() == CHIP_NS02301) {
			return;
		}
    } else if (host->id == SDIO_HOST_ID_3) {
		// mmc donot need to set pad power
		return;
    } else {
		pr_err("%s: Invalid host id %d\n", __func__, host->id);
		return;
    }

	if (vol == SDIO_HOST_SETPOWER_VOL_3P3) {
		power_vol = PAD_3P3V;
	} else if (vol == SDIO_HOST_SETPOWER_VOL_1P8) {
		power_vol = PAD_1P8V;
	} else {
		// PAD control has no power off feature
		return;
	}
	host->host_power = vol;

	pad_power.pad_power_id = power_id;
	pad_power.pad_power = power_vol;

	if (nvt_get_chip_id() == CHIP_NS02301 && host->ldo_gpio) {
		extern_pad_set_power(host, &pad_power);
	} else {
		pad_set_power(&pad_power);
	}
}

int sdiohost_getpower(struct mmc_nvt_host *host)
{
	PAD_POWER_STRUCT pad_power;
	PAD_POWERID power_id;
	int power_status = SDIO_HOST_SETPOWER_VOL_3P3;

	memset(&pad_power, 0, sizeof(PAD_POWER_STRUCT));

	if(host->id == SDIO_HOST_ID_1) {
		power_id = PAD_POWERID_MC0;
		if (nvt_get_chip_id() == CHIP_NS02301) {
			return power_status;
		}
	} else if(host->id == SDIO_HOST_ID_2) {
		power_id = PAD_POWERID_MC1;
		if (nvt_get_chip_id() == CHIP_NS02301) {
			return power_status;
		}
	} else {
		pr_err("%s: Invalid host id %d\n", __func__, host->id);
		return power_status;
	}

	pad_power.pad_power_id = power_id;
	pad_get_power(&pad_power);

	power_status = pad_power.pad_power;

	return power_status;
}

static int sdiohost_getpower_enable(struct mmc_nvt_host *host)
{
	PAD_POWER_STRUCT pad_power[1] = {0};

	if (host->id == SDIO_HOST_ID_1) {
		pad_power->pad_power_id = PAD_POWERID_MC0;
		if (nvt_get_chip_id() == CHIP_NS02301) {
			return 1;
		}
	} else if (host->id == SDIO_HOST_ID_2) {
		pad_power->pad_power_id = PAD_POWERID_MC1;
		if (nvt_get_chip_id() == CHIP_NS02301) {
			return 1;
		}
	} else {
		return 1;
	}

	pad_get_power(pad_power);

	return pad_power->enable;
}

void sdiohost_power_switch(struct mmc_nvt_host *host, bool enable)
{
	if (enable) {
		if (!sdiohost_getpower_enable(host))
			sdiohost_setpower(host, host->max_voltage);
	} else
		sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_0);
}

void sdiohost_power_down(struct mmc_nvt_host *host)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	uint32_t pin_sdio_cfg, pin_number;
	uint8_t pin_num = SDIO_PIN_NUM_4_BIT, i;

	if (CHECK_SDIO_SUP(host->id)) {
		// update pinmux config
		pinmux_config->pin_function = PIN_FUNC_SDIO;
		if (nvt_pinmux_capture(pinmux_config, 1)) {
			pr_err("Get SDIO%d pinmux config fail\n", host->id + 1);
		}

		pin_sdio_cfg = (host->mmc_pinmux & PIN_SDIO_CFG_MASK(host->id));
		pinmux_config->config &= ~pin_sdio_cfg;
		if (nvt_pinmux_update(pinmux_config, 1)) {
			pr_err("Switch SDIO%d pinmux error\n", host->id + 1);
		}

		// check 4-bits/8-bits data
		if (host->mmc) {
			if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) && CHECK_SDIO_8_BITS_SUP(host->id)) {
				pin_num = SDIO_PIN_NUM_8_BIT;
			}
		} else {
			pr_err("mmc is NULL, bypass 8 bit mode pull down and output low\r\n");
		}

		// set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
		for (i = SDIO_PIN_CMD; i < pin_num; i++) {
			pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_GPIO, host->mmc_pinmux, i);
			gpio_direction_input(pin_number);
		}

		// set CLK PAD to pullnone to avoid current leakage
		pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_PIN, host->mmc_pinmux, SDIO_PIN_CLK);
		pad_set_pull_updown(pin_number, PAD_NONE);

		// set CLK GPIO direction to output low to avoid current leakage
		pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_GPIO, host->mmc_pinmux, SDIO_PIN_CLK);
		gpio_direction_output(pin_number, 0);
	}

	sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_0);

	if (host->cp_gpio)
		gpio_set_value_cansleep(host->cp_gpio, !host->cp_gpio_value);

	/* SD spec rev9.10 section 6.4.1.2:
	 * Host needs to keep power line level less than 0.5V and more than 1ms before power ramp up.
	 */
	if (host->power_down_delay_ms > 0) {
		msleep(host->power_down_delay_ms);
	} else {
		msleep(1);
	}
}

void sdiohost_power_up(struct mmc_nvt_host *host)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	uint32_t pin_sdio_cfg, pin_number;
	uint8_t pin_num = SDIO_PIN_NUM_4_BIT, i;
	bool restore_autogating = false;

	if (host->cp_gpio)
		gpio_set_value_cansleep(host->cp_gpio, host->cp_gpio_value);

	/* SD spec rev9.10 section 6.4.1.4:
	 * Recommendation of Power ramp up:
	 * The ramp up time should be 0.1ms to 35ms for 2.7-3.6V power supply and host shall wait until VDD is stable.
	 * After 1ms VDD stable time, host provides at least 74 clocks before issuing the first command.
	 */
	if (clk_get_phase(host->clk)) {
		clk_set_phase(host->clk, 0);
		restore_autogating = true;
	}
	if (host->power_up_delay_ms > 0) {
		msleep(host->power_up_delay_ms);
	} else {
		msleep(36);
	}

	sdiohost_setpower(host, host->max_voltage);

	if (CHECK_SDIO_SUP(host->id)) {
		// check 4-bits/8-bits data
		if (host->mmc) {
			if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) && CHECK_SDIO_8_BITS_SUP(host->id)) {
				pin_num = SDIO_PIN_NUM_8_BIT;
			}
		} else {
			pr_err("mmc is NULL, bypass 8 bit input/pull_up\r\n");
		}

		// set CMD/DATA GPIO direction to input to avoid card identification in SPI mode
		for (i = SDIO_PIN_CMD; i < pin_num; i++) {
			pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_GPIO, host->mmc_pinmux, i);
			gpio_direction_input(pin_number);
		}

		// update pinmux config
		pinmux_config->pin_function = PIN_FUNC_SDIO;
		if (nvt_pinmux_capture(pinmux_config, 1)) {
			pr_err("Get SDIO%d pinmux config fail\n", host->id + 1);
		}

		pin_sdio_cfg = (host->mmc_pinmux & PIN_SDIO_CFG_MASK(host->id));
		pinmux_config->config |= pin_sdio_cfg;
		if (nvt_pinmux_update(pinmux_config, 1)) {
			pr_err("Switch SDIO%d pinmux error\n", host->id + 1);
		}

		/* SD spec rev9.10 section 6:
		 * Pull-up resistors protect the CMD and the DAT lines against bus floating before card identification.
		 */
		// set CMD/DATA PAD to pullup
		for (i = SDIO_PIN_CMD; i < pin_num; i++) {
			pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_PIN, host->mmc_pinmux, i);
			pad_set_pull_updown(pin_number, PAD_PULLUP);
		}

		// set CLK PAD to pullnone
		pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_PIN, host->mmc_pinmux, SDIO_PIN_CLK);
		if (pin_number != SDIO_UNUSED) {
			pad_set_pull_updown(pin_number, PAD_NONE);
		}
	}

	/* SD spec rev9.10 section 6.4.1.1:
	 * A device shall be ready to accept the first command within 1ms from detecting VDD min.
	 * Device may use up to 74 clocks for preparation before receiving the first command.
	 */
	if (host->power_up_clk_period_ms > 0) {
		msleep(host->power_up_clk_period_ms);
	} else {
		msleep(1);
	}
	
	if (restore_autogating == true) {
		clk_set_phase(host->clk, 1);
	}
}

void sdiohost_power_cycle(struct mmc_nvt_host *host, uint32_t delay_ms)
{
	gpio_set_value_cansleep(host->cp_gpio, !host->cp_gpio_value);
	sdiohost_setpower(host, SDIO_HOST_SETPOWER_VOL_0);
	sdiohost_power_down(host);
	msleep(delay_ms);
	sdiohost_power_up(host);
	sdiohost_setpower(host, host->max_voltage);
	gpio_set_value_cansleep(host->cp_gpio, host->cp_gpio_value);
}

/*
	Get SDIO Data status

	@param[in] id   SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2

	@return TRUE: ready
		FALSE: busy
*/
uint32_t sdiohost_getdata_status(struct mmc_nvt_host *host, uint32_t id)
{
	union SDIO_BUS_STATUS_REG stsreg;

	stsreg.reg = sdiohost_getreg(host, id, SDIO_BUS_STATUS_REG_OFS);

	return stsreg.reg;
}

u32 sdiohost_set_voltage_switch(struct mmc_nvt_host *host)
{
    union SDIO_DATA_CTRL_REG datactrl_reg;
    union SDIO_VOL_SWITCH_TIMER_REG timer_reg;
    union SDIO_INT_MASK_REG int_mask_reg;
    unsigned long flags;
    bool unlock_flag = false;

    host->voltage_switch_timeout = 0;

    timer_reg.reg = clk_get_rate(host->clk) / 1000;
    sdiohost_setreg(host, host->id, SDIO_VOL_SWITCH_TIMER_REG_OFS, timer_reg.reg);

    spin_lock_irqsave(&host->int_mask_lock, flags);
    unlock_flag = true;

    int_mask_reg.reg = sdiohost_getreg(host, host->id, SDIO_INT_MASK_REG_OFS);
    int_mask_reg.bit.VOL_SWITCH_END_INT_EN = 1;
    int_mask_reg.bit.VOL_SWITCH_TIMEOUT_INT_EN = 1;
    sdiohost_setreg(host, host->id, SDIO_INT_MASK_REG_OFS, int_mask_reg.reg);

    datactrl_reg.reg = sdiohost_getreg(host, host->id, SDIO_DATA_CTRL_REG_OFS);
    datactrl_reg.bit.WAIT_VOL_SWITCH_EN = 1;
    sdiohost_setreg(host, host->id, SDIO_DATA_CTRL_REG_OFS, datactrl_reg.reg);

	if (unlock_flag) {
        spin_unlock_irqrestore(&host->int_mask_lock, flags);
    }

    wait_for_completion(&host->voltage_switch_complete);

	spin_lock_irqsave(&host->int_mask_lock, flags);
    unlock_flag = true;
    
	int_mask_reg.reg = sdiohost_getreg(host, host->id, SDIO_INT_MASK_REG_OFS);
    int_mask_reg.bit.VOL_SWITCH_END_INT_EN = 0;
    int_mask_reg.bit.VOL_SWITCH_TIMEOUT_INT_EN = 0;
    sdiohost_setreg(host, host->id, SDIO_INT_MASK_REG_OFS, int_mask_reg.reg);

    if (host->voltage_switch_timeout) {
        pr_err("voltage switch timeout\n");
        if ((host->cp_gpio) && (host->id == SDIO_HOST_ID_1))
            sdiohost_power_cycle(host, 500);

        if (unlock_flag) {
            spin_unlock_irqrestore(&host->int_mask_lock, flags);
        }

        return -EIO;
    } else {
        if (unlock_flag) {
            spin_unlock_irqrestore(&host->int_mask_lock, flags);
        }

        return 0;
    }
}

int sdiohost_getcmd(struct mmc_nvt_host *host)
{
	return sdiohost_getreg(host, host->id, \
		SDIO_CMD_REG_OFS) & SDIO_CMD_REG_INDEX;
}

void sdio_copy_info(struct mmc_nvt_host *info)
{
	if (info->id == SDIO_HOST_ID_1)
		drvdump_info[0] = *info;
	else if (info->id == SDIO_HOST_ID_2)
		drvdump_info[1] = *info;
	else
		drvdump_info[2] = *info;
}

void sdiohost_set_paddirection(struct mmc_nvt_host *host)
{
    uint32_t pin_number;
    uint8_t pin_num = SDIO_PIN_NUM_4_BIT, i;

    if (CHECK_SDIO_SUP(host->id)) {
        // check 4-bits/8-bits data
        if (host->mmc) {
            if ((host->mmc->caps & MMC_CAP_8_BIT_DATA) &&
                CHECK_SDIO_8_BITS_SUP(host->id)) {
                pin_num = SDIO_PIN_NUM_8_BIT;
            }
        } else {
            pr_err("mmc is NULL, bypass 8 bit pull up\r\n");
        }

        // set PAD pull without CLK pin
        for (i = SDIO_PIN_CMD; i < pin_num; i++)
        {
            pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_PIN, host->mmc_pinmux, i);
            pad_set_pull_updown(pin_number, PAD_PULLUP);
        }

		pin_number = mmc_pinval_get(host->id, SDIO_PIN_TYPE_PAD_PIN, host->mmc_pinmux, SDIO_PIN_CLK);
		if (pin_number != SDIO_UNUSED) {
			pad_set_pull_updown(pin_number, PAD_NONE);
		}
    }
}

void sdio_drvdump(void)
{
	PAD_DRIVINGSINK driving;
	PAD_POWER_STRUCT pad_power[1] = {0};
    uint32_t pad_ds;
    uint8_t pin_num, i;

    // dump SDIO1
    if (CHECK_SDIO_SUP(SDIO_HOST_ID_1)) {
        pr_info("SDIO1 Frequency %d Hz\n", (u32) clk_get_rate(drvdump_info[0].clk));
        if (drvdump_info[0].cd_gpio)
            pr_info("SDIO1 cd_gpio %d cd_detect_edge %d\n", drvdump_info[0].cd_gpio, drvdump_info[0].cd_detect_edge);

        if (drvdump_info[0].cp_gpio)
            pr_info("SDIO1 cp_gpio %d cp_gpio_value %d\n", drvdump_info[0].cp_gpio, drvdump_info[0].cp_gpio_value);

        if (drvdump_info[0].max_voltage == SDIO_HOST_SETPOWER_VOL_1P8)
            pr_info("SDIO1 max_voltage 1800 mV\n");
        else
            pr_info("SDIO1 max_voltage 3300 mV\n");

        pad_power->pad_power_id = PAD_POWERID_MC0;
        pad_get_power(pad_power);

        if (pad_power->pad_power)
            pr_info("SDIO1 pad_power 1800 mV\n");
        else
            pr_info("SDIO1 pad_power 3300 mV\n");

        pr_info("SDIO1 voltage_switch %d\n", drvdump_info[0].voltage_switch);

        pin_num = SDIO_PIN_NUM_4_BIT;

        if (CHECK_SDIO_8_BITS_SUP(SDIO_HOST_ID_1)) {
            pin_num = SDIO_PIN_NUM_8_BIT;
        }

        for (i = SDIO_PIN_CLK; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(SDIO_HOST_ID_1, SDIO_PIN_TYPE_PAD_DS, PIN_SDIO_CFG_1ST_PINMUX, i);
            pad_get_drivingsink(pad_ds, &driving);
            pr_info("SDIO PIN %d, driving level: %d\n", i, driving);
        }
    }

#if (SDIO_HOST_ID_COUNT > 1)
    // dump SDIO2
    if (CHECK_SDIO_SUP(SDIO_HOST_ID_2)) {
        pr_info("SDIO2 Frequency %d Hz\n", (u32) clk_get_rate(drvdump_info[1].clk));
        if (drvdump_info[1].cd_gpio)
            pr_info("SDIO2 cd_gpio %d cd_detect_edge %d\n", drvdump_info[1].cd_gpio, drvdump_info[1].cd_detect_edge);

        if (drvdump_info[1].cp_gpio)
            pr_info("SDIO2 cp_gpio %d cp_gpio_value %d\n", drvdump_info[1].cp_gpio, drvdump_info[1].cp_gpio_value);

        pin_num = SDIO_PIN_NUM_4_BIT;

        if (CHECK_SDIO_8_BITS_SUP(SDIO_HOST_ID_2)) {
            pin_num = SDIO_PIN_NUM_8_BIT;
        }

        for (i = SDIO_PIN_CLK; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(SDIO_HOST_ID_2, SDIO_PIN_TYPE_PAD_DS, PIN_SDIO2_CFG_1ST_PINMUX, i);
            pad_get_drivingsink(pad_ds, &driving);
            pr_info("SDIO2 PIN %d, driving level: %d\n", i, driving);
        }
    }
#endif

#if (SDIO_HOST_ID_COUNT > 2)
    // dump SDIO3
    if (CHECK_SDIO_SUP(SDIO_HOST_ID_3)) {
        pr_info("SDIO3 Frequency %d Hz\n", (u32) clk_get_rate(drvdump_info[2].clk));
        if (drvdump_info[2].cd_gpio)
            pr_info("SDIO3 cd_gpio %d cd_detect_edge %d\n", drvdump_info[2].cd_gpio, drvdump_info[2].cd_detect_edge);

        if (drvdump_info[2].cp_gpio)
            pr_info("SDIO3 cp_gpio %d cp_gpio_value %d\n", drvdump_info[2].cp_gpio, drvdump_info[2].cp_gpio_value);

        pin_num = SDIO_PIN_NUM_4_BIT;

        if (CHECK_SDIO_8_BITS_SUP(SDIO_HOST_ID_3)) {
            pin_num = SDIO_PIN_NUM_8_BIT;
        }

        for (i = SDIO_PIN_CLK; i < pin_num; i++)
        {
            pad_ds = mmc_pinval_get(SDIO_HOST_ID_3, SDIO_PIN_TYPE_PAD_DS, PIN_SDIO_CFG_SDIO3_1, i);
            pad_get_drivingsink(pad_ds, &driving);
            pr_info("SDIO3 PIN %d, driving level: %d\n", i, driving);
        }
    }
#endif
}


void sdiohost_set_dlldetclr(struct mmc_nvt_host *host, uint32_t isHS400)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg;
	union DATA_PHASE_DELAY_DLL_REG data_dll_reg;

	if ((clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1))
			clk_set_phase(host->clk, 0);

	cmd_dll_reg.reg = sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);
	data_dll_reg.reg = sdiohost_getreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS);

	if (cmd_dll_reg.bit.CAL_END == 1)
		cmd_dll_reg.bit.CAL_END = 1;
	if (data_dll_reg.bit.CAL_END == 1)
		data_dll_reg.bit.CAL_END = 1;
	
	// mask 3T for safe use 
	cmd_dll_reg.bit.CAL_MASK_CNT = 3;
	data_dll_reg.bit.CAL_MASK_CNT = 3;

	if (isHS400) {
		// use 12T for hs400
		data_dll_reg.bit.CAL_12T_SEL = 1;
	}
	cmd_dll_reg.bit.CAL_12T_SEL = 0;
	
	cmd_dll_reg.bit.CAL_EN = 1;
	data_dll_reg.bit.CAL_EN = 1;

	sdiohost_setreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS, cmd_dll_reg.reg);
	sdiohost_setreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS, data_dll_reg.reg);

	if ((!clk_get_phase(host->clk)) && (host->id == SDIO_HOST_ID_1))
			clk_set_phase(host->clk, 1);
}

int sdiohost_get_dllresult(struct mmc_nvt_host *host, unsigned int *cmd_phase_sel, unsigned int *data_phase_sel)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg = {0};
	union DATA_PHASE_DELAY_DLL_REG data_dll_reg = {0};

	while (true) {
		cmd_dll_reg.reg = sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);
		data_dll_reg.reg = sdiohost_getreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS);

		if (cmd_dll_reg.bit.CAL_END && data_dll_reg.bit.CAL_END) {
			break;
		}
	}

	if (cmd_dll_reg.bit.CAL_ERR) {
		pr_err("cmd dll tuning fail\r\n");
		return -EIO;
	}

	if (data_dll_reg.bit.CAL_ERR) {
		pr_err("data dll tuning fail\r\n");
		return -EIO;
	}

	*cmd_phase_sel = cmd_dll_reg.bit.CAL_OUT;
	*data_phase_sel = data_dll_reg.bit.CAL_OUT;

	// pr_err("SDIO%d cmd cal out = 0x%x, data cal out = 0x%x\r\n", host->id, cmd_dll_reg.reg, data_dll_reg.reg);

	// clear result
	sdiohost_setreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS, cmd_dll_reg.reg);
	sdiohost_setreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS, data_dll_reg.reg);

	return 0;
}

void sdiohost_set_cmddll(struct mmc_nvt_host *host, u32 sel)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg = {0};

	cmd_dll_reg.reg = sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);
	cmd_dll_reg.bit.DATA_DLY_SEL_TOTAL = sel;
	sdiohost_setreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS, cmd_dll_reg.reg);
}

void sdiohost_set_datadll(struct mmc_nvt_host *host, u32 sel)
{
	union DATA_PHASE_DELAY_DLL_REG data_dll_reg = {0};

	data_dll_reg.reg = sdiohost_getreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS);
	data_dll_reg.bit.DATA_DLY_SEL_TOTAL = sel;
	sdiohost_setreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS, data_dll_reg.reg);
}

int sdiohost_set_dllresult(struct mmc_nvt_host *host, unsigned int *cmd_phase_sel, unsigned int *data_phase_sel)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg = {0};
	union DATA_PHASE_DELAY_DLL_REG data_dll_reg = {0};

	cmd_dll_reg.reg =  sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);
	data_dll_reg.reg = sdiohost_getreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS);
	cmd_dll_reg.bit.DATA_DLY_SEL_TOTAL = *cmd_phase_sel;
	data_dll_reg.bit.DATA_DLY_SEL_TOTAL = *data_phase_sel;

	cmd_dll_reg.bit.CAL_12T_SEL = 0;
	data_dll_reg.bit.CAL_12T_SEL = 1;

	sdiohost_setreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS, cmd_dll_reg.reg);
	sdiohost_setreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS, data_dll_reg.reg);
	sdiohost_setphyrst(host);

	cmd_dll_reg.reg =  sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);
	data_dll_reg.reg = sdiohost_getreg(host, host->id, DATA_PHASE_DELAY_DLL_REG_OFS);

	pr_info("set SDIO%d cmd cal out = 0x%x, data cal out = 0x%x\r\n", \
	host->id, cmd_dll_reg.reg, data_dll_reg.reg);

	return 0;
}

void sdiohost_set_dlldetcmd(struct mmc_nvt_host *host)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg;

	cmd_dll_reg.reg = sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);

	if (cmd_dll_reg.bit.CAL_END == 1)
		cmd_dll_reg.bit.CAL_END = 1;

	// mask 3T for safe use 
	cmd_dll_reg.bit.CAL_MASK_CNT = 0;
	cmd_dll_reg.bit.CAL_12T_SEL = 1;	
	cmd_dll_reg.bit.CAL_EN = 1;

	sdiohost_setreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS, cmd_dll_reg.reg);
}

uint32_t sdiohost_get_dllcmd(struct mmc_nvt_host *host)
{
	union CMD_PHASE_DELAY_DLL_REG cmd_dll_reg = {0};
	uint32_t cmd_phase_sel;

	while (true) {
		cmd_dll_reg.reg = sdiohost_getreg(host, host->id, CMD_PHASE_DELAY_DLL_REG_OFS);

		if (cmd_dll_reg.bit.CAL_END) {
			break;
		}
	}

	if (cmd_dll_reg.bit.CAL_ERR) {
		pr_err("cmd dll tuning fail\r\n");
		return -EIO;
	}

	cmd_phase_sel = cmd_dll_reg.bit.CAL_OUT;

	// pr_err("*************SDIO%d cmd cal out = 0x%x\r\n", host->id, cmd_dll_reg.reg);

	return cmd_phase_sel;
}

void sdiohost_wifi1v8_on(struct mmc_nvt_host *host)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	uint32_t pin_sdio_cfg;

	if (CHECK_SDIO_SUP(host->id)) {

		// update pinmux config
		pinmux_config->pin_function = PIN_FUNC_SDIO;
		if (nvt_pinmux_capture(pinmux_config, 1)) {
			pr_err("Get SDIO%d pinmux config fail\n", host->id + 1);
		}

		pin_sdio_cfg = 0x10;
		pinmux_config->config |= pin_sdio_cfg;
		if (nvt_pinmux_update(pinmux_config, 1)) {
			pr_err("Switch SDIO%d pinmux error\n", host->id + 1);
		}

	}
}

// defaut register 0x0~0x6C
// 0xF0~0x10C
// 0x160~0x17C
const int default_reg[] = { \
	0x20000000, 0x0, 0x0, 0x0, \
	0x0, 0x0, 0x0, 0x0, \
	0x0, 0xFFF, 0x0, 0x0, \
	0x0, 0x0, 0x400, 0x0, \
	0xFF03, 0x00, 0xFFFFFFFF, 0x0, \
	0x2000, 0xA0000000, 0x0, 0x0, \
	0x0, 0x60, 0x0, 0x0, \
	0x0, 0x0, 0x0, 0x8, \
	0xe1a0000d, 0x0, 0x100, 0x0, \
	0x10808, 0x0, 0x0, 0x803, \
	0x05f0000, 0x07a0000, 0x0c90100, 0x0bf0100 \
};
void sdiohost_set_swreset(struct mmc_nvt_host *host)
{
	uint32_t i = 0, j = 0;

	// swoftware reset
	sdiohost_setreg(host, host->id, SDIO_PHY_REG_OFS, 1);
	sdiohost_setreg(host, host->id, SDIO_CMD_REG_OFS, 0x400);

	for (i = 0; i <= 0x6C; i+=4) {
		sdiohost_setreg(host, host->id, i, default_reg[j]);
		j++;
	}

	for (i = 0x6C; i < 0xF0; i+=4) {
		sdiohost_setreg(host, host->id, i, 0);
	}

	for (i = 0xF0; i <= 0x10C; i+=4) {
		sdiohost_setreg(host, host->id, i, default_reg[j]);
		j++;
	}

	for (i = 0x110; i < 0x160; i+=4) {
		sdiohost_setreg(host, host->id, i, 0);
	}

	for (i = 0x160; i <= 0x17C; i+=4) {
		sdiohost_setreg(host, host->id, i, default_reg[j]);
		j++;
	}
	sdiohost_setreg(host, host->id, 0x1B0, 1);
	sdiohost_setreg(host, host->id, 0x1FC, 0);
}
