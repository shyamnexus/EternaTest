// SPDX-License-Identifier: GPL-2.0
/*
 * PRU-ICSS platform driver for various TI SoCs
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <common.h>
#include <dm.h>
#include <dm/of_access.h>
#include <errno.h>
#include <clk.h>
#include <malloc.h>
#include <soc.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <power-domain.h>
#include <dm/device_compat.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/arch/hardware.h>
#include <asm/cache.h>
#include <div64.h>
#include <sif_api.h>
#include "sif_int.h"
#include "sif_reg.h"

#define SIF_VERSION "1.0.00"

// Static function declaration
static void     sif_setMode(SIF_CH Ch, SIF_MODE Mode);
static void     sif_setBusClock(SIF_CH Ch, UINT32 uiBusClock);
static void     sif_setSEND(SIF_CH Ch, UINT32 uiSEND);
static void     sif_setSENS(SIF_CH Ch, UINT32 uiSENS);
static void     sif_setSENH(SIF_CH Ch, UINT32 uiSENH);

static void     sif_setTranLength(SIF_CH Ch, UINT32 uiTranLength);
static void     sif_setShiftDIR(SIF_CH Ch, SIF_DIR Direction);
static void     sif_setDelay(SIF_CH Ch, UINT32 uiDelay);
static void     sif_setDMADelayTag(SIF_CH Ch, UINT16 uiTag);
static void     sif_setBurstDelay(SIF_CH Ch, UINT32 uiBurstDelay);
static void     sif_setBurstIntval1(SIF_CH Ch, UINT32 uiIntVal);
static void     sif_setBurstIntval2(SIF_CH Ch, UINT32 uiIntVal);
static void     sif_setBurstSRC(SIF_CH Ch, SIF_VD_SOURCE uiIntVal);

static UINT32   sif_getMode(SIF_CH Ch);
static UINT32   sif_getBusClockRegister(SIF_CH Ch);
static UINT32   sif_getBusClock(SIF_CH Ch);
static UINT32   sif_getSENDCount(SIF_CH Ch);
static UINT32   sif_getSEND(SIF_CH Ch);
static UINT32   sif_getSENSRegister(SIF_CH Ch);
static UINT32   sif_getSENS(SIF_CH Ch);
static UINT32   sif_getSENHRegister(SIF_CH Ch);
static UINT32   sif_getSENH(SIF_CH Ch);
static UINT32   sif_getTranLength(SIF_CH Ch);
static UINT32   sif_getShiftDIR(SIF_CH Ch);
static UINT32   sif_getExtraDelay(SIF_CH Ch);
static UINT32   sif_getDelay(SIF_CH Ch);
static UINT32   sif_getBurstDelay(SIF_CH Ch);
static UINT32   sif_getBurstIntval1(SIF_CH Ch);
static UINT32   sif_getBurstIntval2(SIF_CH Ch);
static UINT32   sif_getBurstSRC(SIF_CH Ch);

// Open status (bit-wise)
static UINT32 uiSIFOpenStatus = 0;

// Data port selection
static const UINT32 uiSIFDPSel[SIF_TOTAL_CHANNEL] = {
	SIF_DPSEL_CH0,
	SIF_DPSEL_CH1,
	SIF_DPSEL_CH2,
	SIF_DPSEL_CH3
};
// (DMA) Continuous mode
static SIF_CONTMODE SIFContMode[SIF_TOTAL_CHANNEL] = {
	SIF_CONTMODE_OFF,
	SIF_CONTMODE_OFF
};
// (DMA) Continuous mode address bits
static UINT32           uiSIFContAddrBits[SIF_TOTAL_CHANNEL] = {
	SIF_CONTADDRBITS_DEFAULT,
	SIF_CONTADDRBITS_DEFAULT
};
// (PIO) Remaining bits
static UINT32 uiSIFRemainingBits[SIF_TOTAL_CHANNEL] = {
	0,
	0,
	0,
	0
};

// (DMA) Delay mode
static SIF_DMA_DELAY SIFDelayMode[SIF_TOTAL_CHANNEL] = {
	SIF_DMA_DELAY_OFF,
	SIF_DMA_DELAY_OFF
};

extern void invalidate_dcache_range(ulong start_addr, ulong stop);
extern void flush_dcache_range(ulong start_addr, ulong stop);

struct nvt_sif_priv {
	UINT32 pinmux_value;;
};


/*
    Configure SIF mode

    Configure SIF mode.

    @param[in] Ch           SIF channel that will be configured
    @param[in] Mode         SIF mode
    @return void
*/
static void sif_setMode(SIF_CH Ch, SIF_MODE Mode)
{
	union SIF_MODE_REG sif_mode_reg;

	// Set register
	sif_mode_reg.reg = SIF_GETREG(SIF_MODE_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_mode_reg.bit.Mode_Ch0 = Mode;
			break;

		case SIF_CH1:
			sif_mode_reg.bit.Mode_Ch1 = Mode;
			break;

		case SIF_CH2:
			sif_mode_reg.bit.Mode_Ch2 = Mode;
			break;

		case SIF_CH3:
			sif_mode_reg.bit.Mode_Ch3 = Mode;
			break;
	}
	SIF_SETREG(SIF_MODE_REG_OFS, sif_mode_reg.reg);

	// When mode is SIF_MODE_08 ~ SIF_MODE_15, SIF delay between successive
	// transmission will be related to SENS. SENS is not valid in these modes,
	// so we can set SENS to zero to remove the relationship.
	// Is this a BUG? Ask DE to fix this issue in next IC if possible
	if (Mode >= SIF_MODE_08) {
		sif_setSENS(Ch, 0);
	}
}


/*
    Configure SIF bus clock

    Configure SIF bus clock.

    @param[in] Ch           SIF channel that will be configured
    @param[in] Mode         Desired bus clock (Unit: Hz)
    @return void
*/
static void sif_setBusClock(SIF_CH Ch, UINT32 uiBusClock)
{
	union SIF_CLKCTRL0_REG sif_clkctrl0_reg;
	UINT32 uiDivider;

	// Calculate divider
	uiDivider = SIF_DIVIDER_MIN;
	while (1) {
		if (((SIF_SOURCE_CLOCK / ((uiDivider + 1) << 1)) <= uiBusClock) || (uiDivider == SIF_DIVIDER_MAX)) {
			break;
		}

		uiDivider++;
	}

	// Set register
	sif_clkctrl0_reg.reg = SIF_GETREG(SIF_CLKCTRL0_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
	    case SIF_CH0:
			sif_clkctrl0_reg.bit.ClkDiv_Ch0 = uiDivider;
			break;

		case SIF_CH1:
			sif_clkctrl0_reg.bit.ClkDiv_Ch1 = uiDivider;
			break;

		case SIF_CH2:
			sif_clkctrl0_reg.bit.ClkDiv_Ch2 = uiDivider;
			break;

		case SIF_CH3:
			sif_clkctrl0_reg.bit.ClkDiv_Ch3 = uiDivider;
			break;
	}
	SIF_SETREG(SIF_CLKCTRL0_REG_OFS, sif_clkctrl0_reg.reg);
}


/*
    Configure SIF SEN duration

    Configure SIF SEN duration.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiSEND       Desired SEN duration (Unit: ns)
    @return void
*/
static void sif_setSEND(SIF_CH Ch, UINT32 uiSEND)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;
	UINT32  uiValue;
    UINT32 fPeriod;    // Get half bus clock period in ns
    fPeriod = (1000000000 / sif_getBusClock(Ch)) / 2;
 
	// Calculate duration setting
	uiValue = SIF_SEND_MIN;
	while (1) {
		if (((UINT32)(uiValue * fPeriod) >= uiSEND) || (uiValue == SIF_SEND_MAX)) {
			break;
		}

		uiValue++;
	}

	// Convert to real register setting
	// 16       --> Register 0
	// 1 ~ 15   --> Register 1 ~ 15
	if (uiValue == SIF_SEND_MAX) {
		uiValue = 0;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SEND_Ch0 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SEND_Ch1 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SEND_Ch2 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SEND_Ch3 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;
	}
}


/*
    Configure SIF SEN setup time

    Configure SIF SEN setup time.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiSENS       Desired SEN setup time (Unit: ns)
    @return void
*/
static void sif_setSENS(SIF_CH Ch, UINT32 uiSENS)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;
	UINT32  uiValue;
    UINT32 fPeriod;    // Get half bus clock period in ns
    fPeriod = (1000000000 / sif_getBusClock(Ch)) / 2;
 
	// Calculate setup time setting
	uiValue = SIF_SENS_MIN;
	while (1) {
		if (((UINT32)(uiValue * fPeriod) >= uiSENS) || (uiValue == SIF_SENS_MAX)) {
			break;
		}

		uiValue++;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SENS_Ch0 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SENS_Ch1 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SENS_Ch2 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SENS_Ch3 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;
	}
}


/*
    Configure SIF SEN hold time

    Configure SIF SEN hold time.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiSENH       Desired SEN hold time (Unit: ns)
    @return void
*/
static void sif_setSENH(SIF_CH Ch, UINT32 uiSENH)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;

	UINT32  uiValue;
    UINT32 fPeriod;    // Get half bus clock period in ns
    fPeriod = (1000000000 / sif_getBusClock(Ch)) / 2;

	// Calculate setup time setting
	uiValue = SIF_SENH_MIN;
	while (1) {
		if (((UINT32)(uiValue * fPeriod) >= uiSENH) || (uiValue == SIF_SENH_MAX)) {
			break;
		}
		uiValue++;
	}

	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SENH_Ch0 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			sif_timing0_reg.bit.SENH_Ch1 = uiValue;
			SIF_SETREG(SIF_TIMING0_REG_OFS, sif_timing0_reg.reg);
			break;

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SENH_Ch2 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			sif_timing1_reg.bit.SENH_Ch3 = uiValue;
			SIF_SETREG(SIF_TIMING1_REG_OFS, sif_timing1_reg.reg);
			break;
		}
}


/*
    Configure SIF transmission length

    Configure SIF transmission length.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiTranLength Transfer length
    @return void
*/
static void sif_setTranLength(SIF_CH Ch, UINT32 uiTranLength)
{
	union SIF_TXSIZE0_REG sif_txsize0_reg;

	// Set register
	sif_txsize0_reg.reg = SIF_GETREG(SIF_TXSIZE0_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_txsize0_reg.bit.TxSize_Ch0 = uiTranLength;
			break;

		case SIF_CH1:
			sif_txsize0_reg.bit.TxSize_Ch1 = uiTranLength;
			break;

		case SIF_CH2:
			sif_txsize0_reg.bit.TxSize_Ch2 = uiTranLength;
			break;

		case SIF_CH3:
			sif_txsize0_reg.bit.TxSize_Ch3 = uiTranLength;
			break;
	}
	SIF_SETREG(SIF_TXSIZE0_REG_OFS, sif_txsize0_reg.reg);
}


/*
    Configure SIF bit shift out direction

    Configure SIF bit shift out direction.

    @param[in] Ch           SIF channel that will be configured
    @param[in] Direction    Shift out direction
    @return void
*/
static void sif_setShiftDIR(SIF_CH Ch, SIF_DIR Direction)
{
	union SIF_CONF1_REG sif_conf1_reg;

	// Set register
	sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_conf1_reg.bit.Dir_Ch0 = Direction;
			break;

		case SIF_CH1:
			sif_conf1_reg.bit.Dir_Ch1 = Direction;
			break;

		case SIF_CH2:
			sif_conf1_reg.bit.Dir_Ch2 = Direction;
			break;

		case SIF_CH3:
			sif_conf1_reg.bit.Dir_Ch3 = Direction;
			break;
	}
	SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);
}


/*
    Configure SIF delay between successive transmission

    Configure SIF delay between successive transmission.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiDelay      Delay between successive transmission (Unit: ns)
    @return void
*/
static void sif_setDelay(SIF_CH Ch, UINT32 uiDelay)
{
	union SIF_DELAY0_REG sif_delay0_reg;
	UINT32  uiValue, uiExtra;
    UINT32 fPeriod;    // Get half bus clock period in ns
    fPeriod = 1000000000 / SIF_SOURCE_CLOCK;

	// PIO mode will send data one SIF clock faster than DMA mode.
	// The formula to calculate delay time is based on DMA mode.
	// To make sure the delay time is enought for both mode,
	// we will add one SIF clock time (about 10 ns)to the delay value
	uiDelay += 10;

	// Get extra delay
	uiExtra = sif_getExtraDelay(Ch);

	// Calculate delay setting
	uiValue = SIF_DELAY_MIN;
	while (1) {
		if (((UINT32)((uiExtra + (uiValue << 5)) * fPeriod) >= uiDelay) || (uiValue == SIF_DELAY_MAX)) {
			break;
		}

		uiValue++;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			sif_delay0_reg.bit.Delay_Ch0 = uiValue;
			SIF_SETREG(SIF_DELAY0_REG_OFS, sif_delay0_reg.reg);
			break;

		case SIF_CH1:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			sif_delay0_reg.bit.Delay_Ch1 = uiValue;
			SIF_SETREG(SIF_DELAY0_REG_OFS, sif_delay0_reg.reg);
			break;

		case SIF_CH2:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			sif_delay0_reg.bit.Delay_Ch2 = uiValue;
			SIF_SETREG(SIF_DELAY0_REG_OFS, sif_delay0_reg.reg);
			break;

		case SIF_CH3:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			sif_delay0_reg.bit.Delay_Ch3 = uiValue;
			SIF_SETREG(SIF_DELAY0_REG_OFS, sif_delay0_reg.reg);
			break;
	}
}


/*
    Configure SIF burst delay after SIE VD signal

    Configure SIF burst delay after SIE VD signal.

    @param[in] Ch           SIF channel that will be configured
    @param[in] uiBurstDelay Delay between VD signal and burst transmission (Unit: ns)
    @return void
*/
static void sif_setBurstDelay(SIF_CH Ch, UINT32 uiBurstDelay)
{
	union SIF_DMA0_CTRL3_REG sif_dma0ctrl3_reg;
	union SIF_DMA1_CTRL3_REG sif_dma1ctrl3_reg;
	UINT32  uiValue = SIF_DELAY_MIN;
	UINT32  fPeriod;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Get source clock period in ns
	fPeriod = 1000000000 / SIF_SOURCE_CLOCK;

	// Calculate the minimum unit for different clk sources.
	// The delay will be rounded up if less than 1 unit.
	if (uiBurstDelay != 0) {
		if (uiBurstDelay < fPeriod) {
			uiBurstDelay = fPeriod;
		}

		while (1) {
			if ((uiBurstDelay < fPeriod) || (uiValue == SIF_BURST_DELAY_MAX)) {
				break;
			}

			uiValue++;
			uiBurstDelay -= fPeriod;
		}
	} else {
		uiValue = 0;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl3_reg.reg = SIF_GETREG(SIF_DMA0_CTRL3_REG_OFS);
			sif_dma0ctrl3_reg.bit.DMA_Burst_Delay = uiValue;
			SIF_SETREG(SIF_DMA0_CTRL3_REG_OFS, sif_dma0ctrl3_reg.reg);
			break;

		case SIF_CH1:
			sif_dma1ctrl3_reg.reg = SIF_GETREG(SIF_DMA1_CTRL3_REG_OFS);
			sif_dma1ctrl3_reg.bit.DMA_Burst_Delay = uiValue;
			SIF_SETREG(SIF_DMA1_CTRL3_REG_OFS, sif_dma1ctrl3_reg.reg);
			break;
	}
}


/*
    Configure SIF burst interval between 1st and 2nd transmission

    Configure SIF burst interval between 1st and 2nd transmission

    @param[in] Ch           SIF channel that will be configured
    @param[in] Interval VD numbers between transimissions.
    @return void
*/
static void sif_setBurstIntval1(SIF_CH Ch, UINT32 uiIntVal)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			sif_dma0ctrl0_reg.bit.DMA0_Burst_IntVal1 = uiIntVal;
			SIF_SETREG(SIF_DMA0_CTRL0_REG_OFS, sif_dma0ctrl0_reg.reg);
			break;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			sif_dma1ctrl0_reg.bit.DMA1_Burst_IntVal1 = uiIntVal;
			SIF_SETREG(SIF_DMA1_CTRL0_REG_OFS, sif_dma1ctrl0_reg.reg);
			break;
	}
}


/*
    Configure SIF burst interval between 2nd and 3rd transmission

    Configure SIF burst interval between 2nd and 3rd transmission.

    @param[in] Ch           SIF channel that will be configured
    @param[in] Interval VD numbers between transimissions.
    @return void
*/
static void sif_setBurstIntval2(SIF_CH Ch, UINT32 uiIntVal)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			sif_dma0ctrl0_reg.bit.DMA0_Burst_IntVal2 = uiIntVal;
			SIF_SETREG(SIF_DMA0_CTRL0_REG_OFS, sif_dma0ctrl0_reg.reg);
			break;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			sif_dma1ctrl0_reg.bit.DMA1_Burst_IntVal2 = uiIntVal;
			SIF_SETREG(SIF_DMA1_CTRL0_REG_OFS, sif_dma1ctrl0_reg.reg);
			break;
	}
}


/*
    Configure SIF VD signal source

    Configure SIF VD signal source.

    @param[in] Ch           SIF channel that will be configured
    @param[in] Input source of SIE VD channel.
    @return void
*/
static void sif_setBurstSRC(SIF_CH Ch, SIF_VD_SOURCE uiIntVal)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			sif_dma0ctrl0_reg.bit.DMA0_Sync_Src = uiIntVal;
			SIF_SETREG(SIF_DMA0_CTRL0_REG_OFS, sif_dma0ctrl0_reg.reg);
			break;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			sif_dma1ctrl0_reg.bit.DMA1_Sync_Src = uiIntVal;
			SIF_SETREG(SIF_DMA1_CTRL0_REG_OFS, sif_dma1ctrl0_reg.reg);
			break;
	}
}


/*
    Configure SIF DMA Delay Tag

    Configure SIF DMA Delay Tag

    @param[in] Ch           SIF channel that will be configured
    @param[in] Enable / Disable DMA delay
    @return void
*/
static void sif_setDMADelayTag(SIF_CH Ch, UINT16 uiTag)
{
	union SIF_DMA0_DLY_REG sif_dma0dly_reg;
	union SIF_DMA1_DLY_REG sif_dma1dly_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Set register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0dly_reg.reg = SIF_GETREG(SIF_DMA0_DLY_REG_OFS);
			sif_dma0dly_reg.bit.SIF_DLY_TAG = uiTag;
			SIF_SETREG(SIF_DMA0_DLY_REG_OFS, sif_dma0dly_reg.reg);
			break;

		case SIF_CH1:
			sif_dma1dly_reg.reg = SIF_GETREG(SIF_DMA1_DLY_REG_OFS);
			sif_dma1dly_reg.bit.SIF_DLY_TAG = uiTag;
			SIF_SETREG(SIF_DMA1_DLY_REG_OFS, sif_dma1dly_reg.reg);
			break;
	}
}


/*
    Get SIF mode

    Get SIF mode.

    @param[in] Ch           SIF channel that will be probed

    @return SIF mode
*/
static UINT32 sif_getMode(SIF_CH Ch)
{
	union SIF_MODE_REG sif_mode_reg;

	// Get mode from register
	sif_mode_reg.reg = SIF_GETREG(SIF_MODE_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			return sif_mode_reg.bit.Mode_Ch0;

		case SIF_CH1:
			return sif_mode_reg.bit.Mode_Ch1;

		case SIF_CH2:
			return sif_mode_reg.bit.Mode_Ch2;

		case SIF_CH3:
			return sif_mode_reg.bit.Mode_Ch3;
	}
}


/*
    Get SIF bus clock divider register setting

    Get SIF bus clock divider register setting.

    @param[in] Ch           SIF channel that will be probed

    @return Bus clock divider register
*/
static UINT32 sif_getBusClockRegister(SIF_CH Ch)
{
	union SIF_CLKCTRL0_REG sif_clkctrl0_reg;

	// Get divider from register
	sif_clkctrl0_reg.reg = SIF_GETREG(SIF_CLKCTRL0_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			return (sif_clkctrl0_reg.bit.ClkDiv_Ch0);

		case SIF_CH1:
			return (sif_clkctrl0_reg.bit.ClkDiv_Ch1);

		case SIF_CH2:
			return (sif_clkctrl0_reg.bit.ClkDiv_Ch2);

		case SIF_CH3:
			return (sif_clkctrl0_reg.bit.ClkDiv_Ch3);
	}
}


/*
    Get SIF bus clock

    Get SIF bus clock.

    @param[in] Ch           SIF channel that will be probed

    @return Bus clock (Unit: Hz)
*/
static UINT32 sif_getBusClock(SIF_CH Ch)
{
	return (SIF_SOURCE_CLOCK / ((sif_getBusClockRegister(Ch) + 1) << 1));
}


/*
    Get SIF SEN duration count

    Get SIF SEN duration count.

    @param[in] Ch           SIF channel that will be probed

    @return SEN duration count
*/
static UINT32 sif_getSENDCount(SIF_CH Ch)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;
	UINT32  uiValue;

	// Get SEND from register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			uiValue = sif_timing0_reg.bit.SEND_Ch0;
			break;

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			uiValue = sif_timing0_reg.bit.SEND_Ch1;
			break;

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			uiValue = sif_timing1_reg.bit.SEND_Ch2;
			break;

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			uiValue = sif_timing1_reg.bit.SEND_Ch3;
			break;
	}

	// Convert from register to real setting
	// Register 0       --> 16
	// Register 1 ~ 15  --> 1 ~ 15
	if (uiValue == 0) {
		uiValue = SIF_SEND_MAX;
	}

	return uiValue;
}


/*
    Get SIF SEN duration

    Get SIF SEN duration.

    @param[in] Ch           SIF channel that will be probed

    @return SEN duration (Unit: ns)
*/
static UINT32 sif_getSEND(SIF_CH Ch)
{
    return (UINT32)((500000000 / sif_getBusClock(Ch)) * sif_getSENDCount(Ch));
}


/*
    Get SIF SEN setup time register setting

    Get SIF SEN setup time register setting.

    @param[in] Ch           SIF channel that will be probed

    @return SEN setup time (Unit: ns)
*/
static UINT32 sif_getSENSRegister(SIF_CH Ch)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;

	// Get SENS from register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			return sif_timing0_reg.bit.SENS_Ch0;

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			return sif_timing0_reg.bit.SENS_Ch1;

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			return sif_timing1_reg.bit.SENS_Ch2;

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			return sif_timing1_reg.bit.SENS_Ch3;
	}
}


/*
    Get SIF SEN setup time

    Get SIF SEN setup time.

    @param[in] Ch           SIF channel that will be probed

    @return SEN setup time (Unit: ns)
*/
static UINT32 sif_getSENS(SIF_CH Ch)
{
    return (UINT32)((500000000 / sif_getBusClock(Ch)) * sif_getSENSRegister(Ch));
}


/*
    Get SIF SEN hold time register setting

    Get SIF SEN hold time register setting.

    @param[in] Ch           SIF channel that will be probed

    @return SEN hold time register
*/
static UINT32 sif_getSENHRegister(SIF_CH Ch)
{
	union SIF_TIMING0_REG sif_timing0_reg;
	union SIF_TIMING1_REG sif_timing1_reg;

	// Get SENH from register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			return (sif_timing0_reg.bit.SENH_Ch0);

		case SIF_CH1:
			sif_timing0_reg.reg = SIF_GETREG(SIF_TIMING0_REG_OFS);
			return (sif_timing0_reg.bit.SENH_Ch1);

		case SIF_CH2:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			return (sif_timing1_reg.bit.SENH_Ch2);

		case SIF_CH3:
			sif_timing1_reg.reg = SIF_GETREG(SIF_TIMING1_REG_OFS);
			return (sif_timing1_reg.bit.SENH_Ch3);
	}
}


/*
    Get SIF SEN hold time

    Get SIF SEN hold time.

    @param[in] Ch           SIF channel that will be probed

    @return SEN hold time (Unit: ns)
*/
static UINT32 sif_getSENH(SIF_CH Ch)
{
    return (UINT32)((500000000 / sif_getBusClock(Ch)) * sif_getSENHRegister(Ch));
}


/*
    Get SIF transmission length

    Get SIF transmission length.

    @param[in] Ch           SIF channel that will be probed

    @return Transmission length
*/
static UINT32 sif_getTranLength(SIF_CH Ch)
{
	union SIF_TXSIZE0_REG     sif_txsize0_reg;

	// Get transmission length from register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:    
		case SIF_CH0:
			sif_txsize0_reg.reg = SIF_GETREG(SIF_TXSIZE0_REG_OFS);
			return sif_txsize0_reg.bit.TxSize_Ch0;

		case SIF_CH1:
			sif_txsize0_reg.reg = SIF_GETREG(SIF_TXSIZE0_REG_OFS);
			return sif_txsize0_reg.bit.TxSize_Ch1;

		case SIF_CH2:
			sif_txsize0_reg.reg = SIF_GETREG(SIF_TXSIZE0_REG_OFS);
			return sif_txsize0_reg.bit.TxSize_Ch2;
			break;

		case SIF_CH3:
			sif_txsize0_reg.reg = SIF_GETREG(SIF_TXSIZE0_REG_OFS);
			return sif_txsize0_reg.bit.TxSize_Ch3;
			break;
	}
}


/*
    Get SIF bit shift out direction

    Get SIF bit shift out direction.

    @param[in] Ch           SIF channel that will be probed

    @return Shift out direction
        - @b SIF_DIR_LSB    : LSb is shifted out first
        - @b SIF_DIR_MSB    : MSb is shifted out first
*/
static UINT32 sif_getShiftDIR(SIF_CH Ch)
{
	union SIF_CONF1_REG sif_conf1_reg;

	// Get shift direction from register
	sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			return sif_conf1_reg.bit.Dir_Ch0;

		case SIF_CH1:
			return sif_conf1_reg.bit.Dir_Ch1;

		case SIF_CH2:
			return sif_conf1_reg.bit.Dir_Ch2;

		case SIF_CH3:
			return sif_conf1_reg.bit.Dir_Ch3;
	}
}


/*
    Get SIF extra delay by controller's design

    Get SIF extra delay by controller's design.

    @param[in] Ch           SIF channel that will be probed

    @return Extra delay
*/
static UINT32 sif_getExtraDelay(SIF_CH Ch)
{
	UINT32  uiCheck, uiMore;

	// Formula of delay time is:
	// When SIF mode is 00 ~ 07
	//   if SENH is odd
	//     (clock divider + 1) + 4 + delay * 32
	//   else
	//     (clock divider + 1) * 2 + 4 + delay * 32
	// When SIF mode is 08 ~ 15
	//   if (SENH + SEND) is odd
	//     (clock divider + 1) + 4 + delay * 32 + (clock divider + 1) * SENS
	//   else
	//     (clock divider + 1) * 2 + 4 + delay * 32 + (clock divider + 1) * SENS
	//
	// We will set SENS to zero when upper layer set mode to 08 ~ 15

	uiCheck = sif_getSENHRegister(Ch);

	// Mode 08 ~ Mode 15
	if (sif_getMode(Ch) >= SIF_MODE_08) {
		uiCheck += sif_getSENDCount(Ch);
		uiMore = (sif_getBusClockRegister(Ch) + 1) * sif_getSENSRegister(Ch);
	} else {
		uiMore = 0;
	}

	// Odd
	if (uiCheck & 0x1) {
		return (((sif_getBusClockRegister(Ch) + 1) + 4) + uiMore);
	}
	// Even
	else {
		return ((((sif_getBusClockRegister(Ch) + 1) << 1) + 4) + uiMore);
	}
}


/*
    Get SIF delay between successive transmission

    Get SIF delay between successive transmission.

    @param[in] Ch           SIF channel that will be probed

    @return Delay between successive transmission (Unit: ns)
*/
static UINT32 sif_getDelay(SIF_CH Ch)
{
	union SIF_DELAY0_REG sif_delay0_reg;
	UINT32  uiValue;

	// Get register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			uiValue = sif_delay0_reg.bit.Delay_Ch0;
			break;

		case SIF_CH1:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			uiValue = sif_delay0_reg.bit.Delay_Ch1;
			break;

		case SIF_CH2:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			uiValue = sif_delay0_reg.bit.Delay_Ch2;
			break;

		case SIF_CH3:
			sif_delay0_reg.reg = SIF_GETREG(SIF_DELAY0_REG_OFS);
			uiValue = sif_delay0_reg.bit.Delay_Ch3;
			break;
	}

    return (UINT32)((1000000000 / SIF_SOURCE_CLOCK) * ((uiValue << 5) + sif_getExtraDelay(Ch)));
}


/*
    Get SIF burst delay after SIE VD signal

    Get SIF burst delay after SIE VD signal.

    @param[in] Ch           SIF channel that will be configured
    @return uiBurstDelay Delay between VD signal and burst transmission (Unit: ns)
*/
static UINT32 sif_getBurstDelay(SIF_CH Ch)
{
	union SIF_DMA0_CTRL3_REG sif_dma0ctrl3_reg;
	union SIF_DMA1_CTRL3_REG sif_dma1ctrl3_reg;
	UINT32  uiValue;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return 0;
	}

	// Get register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl3_reg.reg = SIF_GETREG(SIF_DMA0_CTRL3_REG_OFS);
			uiValue = sif_dma0ctrl3_reg.bit.DMA_Burst_Delay;
			break;

		case SIF_CH1:
			sif_dma1ctrl3_reg.reg = SIF_GETREG(SIF_DMA1_CTRL3_REG_OFS);
			uiValue = sif_dma1ctrl3_reg.bit.DMA_Burst_Delay;
			break;
	}
	return (UINT32)(1000000000 / SIF_SOURCE_CLOCK * uiValue);
}


/*
    Get SIF burst interval between 1st and 2nd transmission

    Get SIF burst interval between 1st and 2nd transmission

    @param[in] Ch           SIF channel that will be configured
    @return burst interval
*/
static UINT32 sif_getBurstIntval1(SIF_CH Ch)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return 0;
	}

	// Get register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			return sif_dma0ctrl0_reg.bit.DMA0_Burst_IntVal1;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			return sif_dma1ctrl0_reg.bit.DMA1_Burst_IntVal1;
	}
}


/*
    Get SIF burst interval between 2nd and 3rd transmission

    Get SIF burst interval between 2nd and 3rd transmission

    @param[in] Ch           SIF channel that will be configured
    @return burst interval
*/
static UINT32 sif_getBurstIntval2(SIF_CH Ch)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return 0;
	}

	// Get register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			return sif_dma0ctrl0_reg.bit.DMA0_Burst_IntVal2;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			return sif_dma1ctrl0_reg.bit.DMA1_Burst_IntVal2;
	}
}


/*
    Get SIF VD signal source

    Get SIF VD signal source

    @param[in] Ch           SIF channel that will be configured
    @return SIE VD channel
*/
static UINT32 sif_getBurstSRC(SIF_CH Ch)
{
	union SIF_DMA0_CTRL0_REG sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG sif_dma1ctrl0_reg;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return 0;
	}

	// Get register
	switch (Ch) {
		// The channel validation is done in the function that will
		// call this API. We can skip the checking here.
		default:
		case SIF_CH0:
			sif_dma0ctrl0_reg.reg = SIF_GETREG(SIF_DMA0_CTRL0_REG_OFS);
			return sif_dma0ctrl0_reg.bit.DMA0_Sync_Src;

		case SIF_CH1:
			sif_dma1ctrl0_reg.reg = SIF_GETREG(SIF_DMA1_CTRL0_REG_OFS);
			return sif_dma1ctrl0_reg.bit.DMA1_Sync_Src;
	}
}


/**
    Configure SIF

    Configuration for specific SIF channel.

    @param[in] Ch           SIF channel that will be configured
    @param[in] ConfigID     Configuration ID
    @param[in] uiConfig     Configuration value
    @return void
*/
void sif_setConfig(SIF_CH Ch, SIF_CONFIG_ID ConfigID, UINT32 uiConfig)
{
	union SIF_STS_REG sif_sts_reg;

	// Check channel number
	if (Ch >= SIF_TOTAL_CHANNEL) {
		printf("Invalid channel!\r\n");
		return;
	}

	// Allow to config SIF's auto pinmux configuration when SIF is not open
	if ((ConfigID != SIF_CONFIG_ID_AUTOPINMUX) && ((uiSIFOpenStatus & ((UINT32)1 << Ch)) == 0)) {
		printf("SIF_CH%d is not opened!\r\n", Ch);
		return;
	}

	// Check channel is busy or not
	sif_sts_reg.reg = SIF_GETREG(SIF_STS_REG_OFS);
	if (sif_sts_reg.reg & SIF_STATUS_CH(Ch)) {
		printf("SIF_CH%d is busy!\r\n", Ch);
		return;
	}

	switch (ConfigID) {
		case SIF_CONFIG_ID_BUSMODE:
			// Check parameter
			if (uiConfig > SIF_MODE_15) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = SIF_MODE_15;
			}
			sif_setMode(Ch, (SIF_MODE)uiConfig);
			break;		

		case SIF_CONFIG_ID_SEND:
			sif_setSEND(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_SENS:
			sif_setSENS(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_SENH:
			sif_setSENH(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_BUSCLOCK:
			sif_setBusClock(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_LENGTH:
			// Check parameter
			if ((uiConfig < SIF_TRANLEN_MIN) || (uiConfig > SIF_TRANLEN_MAX)) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = SIF_TRANLEN_DEFAULT;
			}

			sif_setTranLength(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_DIR:
			// Check parameter
			if (uiConfig > SIF_DIR_MSB) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = SIF_DIR_LSB;
			}

			sif_setShiftDIR(Ch, (SIF_DIR)uiConfig);
			break;

		case SIF_CONFIG_ID_DELAY:
			sif_setDelay(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_BURST_SRC:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if (uiConfig > SIF_VD_SIE5) {
				printf("Invalid burst source!\r\n");
				break;
			}
			sif_setBurstSRC(Ch, (SIF_VD_SOURCE) uiConfig);
			break;

		case SIF_CONFIG_ID_CONTMODE:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}

			if ((SIF_CONTMODE)uiConfig > SIF_CONTMODE_ON) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = (UINT32)SIF_CONTMODE_OFF;
			}

			SIFContMode[Ch] = (SIF_CONTMODE)uiConfig;
			break;

		case SIF_CONFIG_ID_CONTADDRBITS:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if ((uiConfig < SIF_CONTADDRBITS_MIN) || (uiConfig > SIF_CONTADDRBITS_MAX)) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = SIF_CONTADDRBITS_DEFAULT;
			}

			uiSIFContAddrBits[Ch] = uiConfig;
			break;

		case SIF_CONFIG_ID_BURST_DELAY:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if (uiConfig > SIF_UI_BURST_DELAY_MAX) {
				printf("Invalid delay time!\r\n");
				break;
			}
			sif_setBurstDelay(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_BURST_INTVAL1:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if (uiConfig > 7) {
				printf("Invalid burst interval!\r\n");
				break;
			}
			sif_setBurstIntval1(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_BURST_INTVAL2:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if (uiConfig > 7) {
				printf("Invalid burst interval!\r\n");
				break;
			}
			sif_setBurstIntval2(Ch, uiConfig);
			break;

		case SIF_CONFIG_ID_DMA_DELAY:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}

			if ((SIF_DMA_DELAY)uiConfig > SIF_DMA_DELAY_ON) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = (UINT32)SIF_DMA_DELAY_OFF;
			}

			SIFDelayMode[Ch] = (SIF_DMA_DELAY)uiConfig;
			break;

		case SIF_CONFIG_ID_DMA_DELAY_TAG:
			// Check parameter
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return;
			}
			if (uiConfig > 0xFFFF) {
				printf("Invalid delay tag!\r\n");
				break;
			}

			sif_setDMADelayTag(Ch, uiConfig);
			break;

#if 0
		case SIF_CONFIG_ID_AUTOPINMUX:
			// Check parameter
			if (uiConfig > (UINT32)TRUE) {
				printf("Invalid value (%d) for ID %d!\r\n", uiConfig, ConfigID);
				uiConfig = (UINT32)TRUE;
			}

			// Project want to enable auto-pinmux function, and SIF channel is not opened yet!
			// We have to switch pinmux to GPIO to make sure pinmux is corrent before calling
			// sif_open() and sif_close().

			// Wait semaphore
			SEM_WAIT(SEMID_SIF);

	#ifndef __KERNEL__
			if ((uiConfig == (UINT32)TRUE) && ((uiSIFOpenStatus & ((UINT32)1 << Ch)) == 0)) {
				pinmux_setPinmux((PINMUX_FUNC_ID)SIFPinmuxID[Ch], PINMUX_SIF_SEL_INACTIVE);
			}
	#endif
			bSIFAutoPinmux[Ch] = (BOOL)uiConfig;
			// Signal semaphore
			SEM_SIGNAL(SEMID_SIF);
			break;
#endif
		default:
			printf("Not supported ID (%d)!\r\n", ConfigID);
			break;
	}
}


/**
    Get SIF configuration

    Get SIF configuration for specific channel.

    @param[in] Ch           SIF channel that will be probed
    @param[in] ConfigID     Configuration ID
    @return Configuration value.
*/
UINT32 sif_getConfig(SIF_CH Ch, SIF_CONFIG_ID ConfigID)
{
	// Check channel number
	if (Ch >= SIF_TOTAL_CHANNEL) {
		printf("Invalid channel!\r\n");
		return 0;
	}

	// Allow to get SIF's auto pinmux configuration when SIF is not open
	if ((ConfigID != SIF_CONFIG_ID_AUTOPINMUX) && ((uiSIFOpenStatus & (1 << Ch)) == 0)) {
		printf("SIF_CH%d is not opened!\r\n", Ch);
		return 0;
	}

	switch (ConfigID) {
		case SIF_CONFIG_ID_BUSMODE:
			return sif_getMode(Ch);

		case SIF_CONFIG_ID_BUSCLOCK:
			return sif_getBusClock(Ch);

		case SIF_CONFIG_ID_SEND:
			return sif_getSEND(Ch);

		case SIF_CONFIG_ID_SENS:
			return sif_getSENS(Ch);

		case SIF_CONFIG_ID_SENH:
			return sif_getSENH(Ch);

		case SIF_CONFIG_ID_LENGTH:
			return sif_getTranLength(Ch);

		case SIF_CONFIG_ID_DIR:
			return sif_getShiftDIR(Ch);

		case SIF_CONFIG_ID_DELAY:
			return sif_getDelay(Ch);

		case SIF_CONFIG_ID_BURST_SRC:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return sif_getBurstSRC(Ch);

		case SIF_CONFIG_ID_CONTMODE:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return (UINT32)SIFContMode[Ch];

		case SIF_CONFIG_ID_CONTADDRBITS:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return (UINT32)uiSIFContAddrBits[Ch];

		case SIF_CONFIG_ID_BURST_DELAY:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return sif_getBurstDelay(Ch);

		case SIF_CONFIG_ID_BURST_INTVAL1:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return sif_getBurstIntval1(Ch);

		case SIF_CONFIG_ID_BURST_INTVAL2:
			if (Ch > SIF_CH1) {
				printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
				return 0;
			}
			return sif_getBurstIntval2(Ch);
#if 0
		case SIF_CONFIG_ID_AUTOPINMUX:
			return (UINT32)bSIFAutoPinmux[Ch];
#endif
		default:
			printf("Not supported ID (%d)!\r\n", ConfigID);
			return 0;
	}
}


/**
    Open SIF channel

    This function should be called before calling any other functions.

    @param[in] Ch       SIF channel number
    @return Open status
        - @b E_SYS      The channel is already opened
        - @b E_OK       Everything is oK
*/
ER sif_open(SIF_CH Ch)
{
	union SIF_INT_DES_REG int_des_reg;
	UINT32 reg;
	UINT32 i;

	// The channel is already opened
	if (uiSIFOpenStatus & (1 << Ch)) {
		printf("SIF_CH%d is already opened!\r\n", Ch);
		return -1;
	}

	// The channel is opened now
	uiSIFOpenStatus |= ((UINT32)1 << Ch);

	// All SIF channels are closed
	if (uiSIFOpenStatus == ((UINT32)1 << Ch)) {
		// Reset remain bits
		for (i = 0; i < SIF_TOTAL_CHANNEL; i++) {
			uiSIFRemainingBits[i] = 0;
		}
		
		//set interrupt destionation to CPU
		int_des_reg.reg = 0;
		int_des_reg.bit.INT_TO_CPU1 =  (0xF);
		//int_des_reg.bit.INT_TO_CPU2 =  (0xF);
		SIF_SETREG(SIF_INT_DES_REG_OFS, int_des_reg.reg);
 
		// Enable SIF clock
		reg = INW(IOADDR_CG_REG_BASE + 0x74);
		reg |= (0x1 << 9);
		OUTW(IOADDR_CG_REG_BASE + 0x74, reg);
	}
	return 1;
}


/**
    Close SIF channel

    Release SIF channel and let other application use SIF.

    @param[in] Ch       SIF channel number
    @return Close status
        - @b E_SYS      The channel is already closed
        - @b E_OK       Everything is oK
*/
ER sif_close(SIF_CH Ch)
{
	union SIF_INTSTS0_REG sif_intsts0_reg;
	unsigned int time_start, time_now;
	UINT32 reg;

	// The channel is already closed
	if (uiSIFOpenStatus == 0) {
		printf("SIF_CH%d is already closed!\r\n", Ch);
		return -1;
	}

	// The channel is now closed
	uiSIFOpenStatus &= ~(1 << Ch);

	// There is no more SIF channel is opened
	if (uiSIFOpenStatus == 0) {
		time_start = get_timer(0);
		// Wait for previous txfed flag
		while (1) {
			time_now = get_timer(0);
			sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
			if ((sif_intsts0_reg.reg & SIF_INT_END(Ch))) {
				SIF_SETREG(SIF_INTSTS0_REG_OFS, SIF_INT_END(Ch));
			} else {
				break;
			}
			if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
				printf("sif_close timeout\n\r");
				break;
			}
		}

		// clear all interrupts
		SIF_SETREG(SIF_INTSTS0_REG_OFS, 0xFFFFFFFF);
		
		// do sif rstn
		reg = INW(IOADDR_CG_REG_BASE + 0x94);
		reg &= ~(0x1 << 9);
		OUTW(IOADDR_CG_REG_BASE + 0x94, reg);
		reg = INW(IOADDR_CG_REG_BASE + 0x94);
		reg |= (0x1 << 9);
		OUTW(IOADDR_CG_REG_BASE + 0x94, reg);

		// Disable sif clock
		reg = INW(IOADDR_CG_REG_BASE + 0x74);
		reg &= ~(0x1 << 9);
		OUTW(IOADDR_CG_REG_BASE + 0x74, reg);
	}
	return 1;
}


/**
    Check if SIF channel is opened or not

    Check if SIF channel is opened or not.

    @param[in] Ch       SIF channel number
    @return SIF channel open status
        - @b TRUE   : SIF is opened
        - @b FALSE  : SIF is closed
*/
BOOL sif_isOpened(SIF_CH Ch)
{
	return (((uiSIFOpenStatus & (1 << Ch)) != 0) ? TRUE : FALSE);
}


/**
    Send data

    Send data out of specific channel.
    If the channel is LSb out first, shift out sequence will be uiData0 [b0 b1..b31],
    uiData1 [b0 b1..b31], uiData2 [b0 b1..b31].
    If the channel is MSb out first, Shift out sequence will be uiData0 [b31 b30..b0],
    uiData1 [b31 b30..b0], uiData2 [b31 b30..b0].
    You have to do data alignment before calling this function.

    @note Don't call this function in ISR.

    @param[in] Ch       SIF channel
    @param[in] uiData0  SIF data 0
    @param[in] uiData1  SIF data 1
    @param[in] uiData2  SIF data 2
    @return void
*/
void sif_send(SIF_CH Ch, UINT32 uiData0, UINT32 uiData1, UINT32 uiData2)
{
	union SIF_DATA0_REG     sif_data0_reg;
	union SIF_DATA1_REG     sif_data1_reg;
	union SIF_DATA2_REG     sif_data2_reg;
	union SIF_DATASEL_REG   sif_datasel_reg;
	union SIF_CONF0_REG     sif_conf0_reg;
	volatile union SIF_INTSTS0_REG   sif_intsts0_reg;
	unsigned int            time_start, time_now;
	BOOL                    bNewTran;
	BOOL                    waitType;

	//sif_intsts0_reg.reg = 0;

	// Check channel number and SIF driver is opened
	if ((Ch >= SIF_TOTAL_CHANNEL) || ((uiSIFOpenStatus & (1 << Ch)) == 0)) {
		printf("Invalid channel or SIF isn't opened!\r\n");
		return;
	}

	// New transmission
	if (uiSIFRemainingBits[Ch] == 0) {
		bNewTran = TRUE;
	} else {
		bNewTran = FALSE;
	}

	// Update remaining bits
	if (bNewTran == TRUE) {
		uiSIFRemainingBits[Ch] = sif_getTranLength(Ch);
	}

	if (uiSIFRemainingBits[Ch] > SIF_MAX_DATAREG_BITS) {
		uiSIFRemainingBits[Ch] -= SIF_MAX_DATAREG_BITS;
		//wait data empty
		waitType = FALSE;
	} else {
		uiSIFRemainingBits[Ch] = 0;
		//wait txfed
		waitType = TRUE;
	}

	// Select data port for specific channel
	sif_datasel_reg.reg = 0;
	sif_datasel_reg.bit.DP_Sel = uiSIFDPSel[Ch];
	SIF_SETREG(SIF_DATASEL_REG_OFS, sif_datasel_reg.reg);

	// Set data register
	sif_data0_reg.reg = 0;
	sif_data0_reg.bit.DR0 = uiData0;
	SIF_SETREG(SIF_DATA0_REG_OFS, sif_data0_reg.reg);

	sif_data1_reg.reg = 0;
	sif_data1_reg.bit.DR1 = uiData1;
	SIF_SETREG(SIF_DATA1_REG_OFS, sif_data1_reg.reg);

	sif_data2_reg.reg = 0;
	sif_data2_reg.bit.DR2 = uiData2;
	SIF_SETREG(SIF_DATA2_REG_OFS, sif_data2_reg.reg);

	// Start or restart transmission
	if (bNewTran == TRUE) {
		// Start
		sif_conf0_reg.reg = SIF_CONF_START_CH(Ch);
	} else {
		// Restart
		sif_conf0_reg.reg = SIF_CONF_RESTART_CH(Ch);
	}
	SIF_SETREG(SIF_CONF0_REG_OFS, sif_conf0_reg.reg);


	if (waitType == FALSE) {
		// busy waiting for tx empty to 1 and clear it
		// for >96 bit restart case
		time_start = get_timer(0);
		while (1) {
			time_now = get_timer(0);
			sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
			if ((sif_intsts0_reg.reg & SIF_INT_EMPTY(Ch))) {
				SIF_SETREG(SIF_INTSTS0_REG_OFS, SIF_INT_EMPTY(Ch));
				break;
			}
			if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
				printf("sif_send wait txempty timeout\n\r");
				return;
			}
		}
	} else if (waitType == TRUE) {
		// busy waiting for txfed to 1 and clear it
		// for the last part transfer or first 96bit transfer case
		time_start = get_timer(0);
		while (1) {
			time_now = get_timer(0);
			sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
			if ((sif_intsts0_reg.reg & SIF_INT_END(Ch))) {
				SIF_SETREG(SIF_INTSTS0_REG_OFS, SIF_INT_END(Ch));
				break;
			}
			if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
				printf("sif_send wait txfed timeout\n\r");
				return;
			}
		}
	}
}


/**
    Wait for SIF PIO transmission end

    Call this API will wait for SIF specific channel's PIO transmission end.

    @note Don't call this function in ISR.

    @param[in] Ch   Which channel that you want to wait for

    @return void
*/
void sif_waitTransferEnd(SIF_CH Ch)
{
	union SIF_INTSTS0_REG sif_intsts0_reg;
	unsigned int time_start, time_now;

	// Check channel number and SIF driver is opened
	if ((Ch >= SIF_TOTAL_CHANNEL) || ((uiSIFOpenStatus & (1 << Ch)) == 0)) {
		printf("Invalid channel or SIF isn't opened!\r\n");
		return;
	}

	// Polling txfed status becomes to zero
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_END(Ch)) == 0) {
			break;
		}
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("waitTransferEnd txfed timeout\n\r");
			return;
		}
	}
}


/**
    Send data via DMA

    Send data via DMA.

    @note Don't call this function in ISR.

    @param[in] Ch           SIF Channel
    @param[in] uiAddr       SIF DMA starting address (Word alignment)
    @param[in] uiDMALength  SIF DMA transfer length in byte unit (Word alignment, include dummy bits).
                            DMA transfer length must be >= 4 and <= 0xFFFC.
    @return void
*/
void sif_sendDMA(SIF_CH Ch, ulong uiAddr, UINT32 uiDMALength)
{
	union SIF_CONF1_REG             sif_conf1_reg;
	union SIF_CONF2_REG             sif_conf2_reg;
	union SIF_DMA0_STARTADDR_REG    sif_dma0start_reg;
	union SIF_DMA1_STARTADDR_REG    sif_dma1start_reg;
	union SIF_DMA0_CTRL0_REG        sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG        sif_dma1ctrl0_reg;
	union SIF_DMA0_CTRL1_REG        sif_dma0ctrl1_reg;
	union SIF_DMA1_CTRL1_REG        sif_dma1ctrl1_reg;
	union SIF_INTSTS0_REG           sif_intsts0_reg;
	uintptr_t pa;
	unsigned int time_start, time_now;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Check channel number and SIF driver is opened
	if ((Ch >= SIF_TOTAL_CHANNEL) || ((uiSIFOpenStatus & (1 << Ch)) == 0)) {
		printf("Invalid channel or SIF isn't opened!\r\n");
		return;
	}

	// Check parameter
	if ((uiAddr & 0x3) ||
		(uiDMALength & 0x3) ||
		(uiDMALength > SIF_DMA_SIZE_MAX)) {
		printf("Invalid parameter!\r\n");
		return;
	}

	// Flush cache
	invalidate_dcache_range((uintptr_t)uiAddr, (uintptr_t)roundup(uiAddr + uiDMALength, ARCH_DMA_MINALIGN));
	pa = uiAddr;

	if (SIFDelayMode[Ch]) {
		union SIF_DMA0_DLY_REG sif_dma0dly_reg;
		union SIF_DMA1_DLY_REG sif_dma1dly_reg;
		ulong i, *pbuffer = (ulong *)uiAddr;
		UINT32 tag, remap;

		if (Ch == SIF_CH0) {
			sif_dma0dly_reg.reg = SIF_GETREG(SIF_DMA0_DLY_REG_OFS);
			tag = sif_dma0dly_reg.bit.SIF_DLY_TAG;
		} else if(Ch == SIF_CH1) {
			sif_dma1dly_reg.reg = SIF_GETREG(SIF_DMA1_DLY_REG_OFS);
			tag = sif_dma1dly_reg.bit.SIF_DLY_TAG;
		}

		remap = uiDMALength;

		for(i = 0; i < (uiDMALength >> 2); i++) {
			if ((*pbuffer >> 16) == tag) {
				remap -= 4;
			}
			pbuffer = pbuffer+1;
		}
		uiDMALength = remap;
		//DBG_DUMP("remap size = %d\r\n",uiDMALength);
	}

	// wait previous transfer are finished
	// polling dma end int sts becomes to zero
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_DMA(Ch)) == 0) {
			break;
		}
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("sif_sendDMA wait previous dma timeout\n\r");
			return;
		}
	}
	// polling pio txfed int sts becomes to zero
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_END(Ch)) == 0) {
			break;
		}
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("sif_sendDMA wait previous pio timeout\n\r");
			return;
		}
	}

	switch (Ch) {
		default:
		case SIF_CH0:
			// Configure DMA mode and address bits
			sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
			sif_conf1_reg.bit.Cont_Ch0 = SIFContMode[Ch];
			SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);
			sif_conf2_reg.reg = SIF_GETREG(SIF_CONF2_REG_OFS);
			sif_conf2_reg.bit.Ch0_Addr_Bits = uiSIFContAddrBits[Ch];
			SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);

			// Configure DMA starting address and length
			sif_dma0start_reg.reg = 0;
			sif_dma0start_reg.bit.DMA_StartAddr = pa & 0xffffffff;
			SIF_SETREG(SIF_DMA0_STARTADDR_REG_OFS, sif_dma0start_reg.reg);
			sif_dma0ctrl1_reg.reg = 0;
			sif_dma0ctrl1_reg.bit.DMA0_BufSize1 = uiDMALength;
			SIF_SETREG(SIF_DMA0_CTRL1_REG_OFS, sif_dma0ctrl1_reg.reg);

			// Configure DMA channel and enable DMA
			sif_dma0ctrl0_reg.reg = 0;
			sif_dma0ctrl0_reg.bit.DMA0_Sync_En = DISABLE;
			sif_dma0ctrl0_reg.bit.DMA0_En = ENABLE;
			sif_dma0ctrl0_reg.bit.DMA0_Delay = SIFDelayMode[Ch];
			SIF_SETREG(SIF_DMA0_CTRL0_REG_OFS, sif_dma0ctrl0_reg.reg);

			break;

		case SIF_CH1:
			// Configure DMA mode and address bits
			sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
			sif_conf1_reg.bit.Cont_Ch1 = SIFContMode[Ch];
			SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);
			sif_conf2_reg.reg = SIF_GETREG(SIF_CONF2_REG_OFS);
			sif_conf2_reg.bit.Ch1_Addr_Bits = uiSIFContAddrBits[Ch];
			SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);

			// Configure DMA starting address and length
			sif_dma1start_reg.reg = 0;
			sif_dma1start_reg.bit.DMA_StartAddr = pa & 0xffffffff;
			SIF_SETREG(SIF_DMA1_STARTADDR_REG_OFS, sif_dma1start_reg.reg);
			sif_dma1ctrl1_reg.reg = 0;
			sif_dma1ctrl1_reg.bit.DMA1_BufSize1 = uiDMALength;
			SIF_SETREG(SIF_DMA1_CTRL1_REG_OFS, sif_dma1ctrl1_reg.reg);

			// Configure DMA channel and enable DMA
			sif_dma1ctrl0_reg.reg = 0;
			sif_dma1ctrl0_reg.bit.DMA1_Sync_En = DISABLE;
			sif_dma1ctrl0_reg.bit.DMA1_En = ENABLE;
			sif_dma1ctrl0_reg.bit.DMA1_Delay = SIFDelayMode[Ch];
			SIF_SETREG(SIF_DMA1_CTRL0_REG_OFS, sif_dma1ctrl0_reg.reg);
			break;
	}
}


/**
    Send burst data via DMA

    Send burst data via DMA.

    @note Don't call this function in ISR.

    @param[in] Ch            SIF Channel
    @param[in] uiAddr        SIF DMA starting address (Word alignment)
    @param[in] uiBurstNumber SIF DMA burst transmission number, range 1~3.
    @param[in] uiDMALength1  SIF DMA transfer length1 in byte unit (Word alignment, include dummy bits).
                             DMA transfer length must be >= 4 and <= 0xFFFC.
    @param[in] uiDMALength2  SIF DMA transfer length2 in byte unit (Word alignment, include dummy bits).
                             DMA transfer length must be >= 4 and <= 0xFFFC.
    @param[in] uiDMALength3  SIF DMA transfer length3 in byte unit (Word alignment, include dummy bits).
                             DMA transfer length must be >= 4 and <= 0xFFFC.
    @return void
*/
void sif_send_burst_DMA(SIF_CH Ch, ulong uiAddr, UINT8 uiBurstNumber, UINT32 uiDMALength1, UINT32 uiDMALength2, UINT32 uiDMALength3)
{
	union SIF_CONF1_REG             sif_conf1_reg;
	union SIF_CONF2_REG             sif_conf2_reg;
	union SIF_DMA0_STARTADDR_REG    sif_dma0start_reg;
	union SIF_DMA1_STARTADDR_REG    sif_dma1start_reg;
	union SIF_DMA0_CTRL0_REG        sif_dma0ctrl0_reg;
	union SIF_DMA1_CTRL0_REG        sif_dma1ctrl0_reg;
	union SIF_DMA0_CTRL1_REG        sif_dma0ctrl1_reg;
	union SIF_DMA1_CTRL1_REG        sif_dma1ctrl1_reg;
	union SIF_DMA0_CTRL2_REG        sif_dma0ctrl2_reg;
	union SIF_DMA1_CTRL2_REG        sif_dma1ctrl2_reg;
	union SIF_INTSTS0_REG           sif_intsts0_reg;
	uintptr_t pa;
	unsigned int time_start, time_now;

	if (Ch > SIF_CH1) {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return;
	}

	// Check channel number, and SIF driver is opened
	if ((Ch >= SIF_TOTAL_CHANNEL) || ((uiSIFOpenStatus & (1 << Ch)) == 0)) {
		printf("Invalid channel or SIF isn't opened!\r\n");
		return;
	}

	// Check parameter
	if ((uiAddr & 0x3) || (uiDMALength1 & 0x3) || (uiDMALength1 > SIF_DMA_SIZE_MAX) ||
		(uiDMALength2 & 0x3) || (uiDMALength2 > SIF_DMA_SIZE_MAX) ||
		(uiDMALength3 & 0x3) || (uiDMALength3 > SIF_DMA_SIZE_MAX) ||
		(uiBurstNumber > 0x3) || (uiBurstNumber < 0x1)) {
		printf("Invalid parameter!\r\n");
		return;
	}

	// Check DMAbuffer size with burst number condition
	if ((uiBurstNumber == 0x3) &&
		((uiDMALength1 == 0x0) || (uiDMALength2 == 0x0) || (uiDMALength3 == 0x0))) {
		printf("Invalid DMA SIZE 1!\r\n");
		return;
	} else if ((uiBurstNumber == 0x2) &&
			   ((uiDMALength1 == 0x0) || (uiDMALength2 == 0x0))) {
		printf("Invalid DMA SIZE 2!\r\n");
		return;
	} else {
		if ((uiDMALength1 == 0x0)) {
			printf("Invalid DMA SIZE 3!\r\n");
			return;
		}
	}

	// Flush cache
	invalidate_dcache_range((uintptr_t)uiAddr, (uintptr_t)roundup(uiAddr + uiDMALength1 + uiDMALength2 + uiDMALength3, ARCH_DMA_MINALIGN));
	pa = uiAddr;

	// wait previous transfer are finished
	// polling dma end int sts becomes to zero
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_DMA(Ch)) == 0) {
			break;
		}
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("sif_sendDMA wait previous dma timeout\n\r");
			return;
		}
	}
	// polling pio txfed int sts becomes to zero
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_END(Ch)) == 0) {
			break;
		}
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("sif_sendDMA wait previous pio timeout\n\r");
			return;
		}
	}

	switch (Ch) {
		default:
		case SIF_CH0:
			// Configure DMA mode and address bits
			sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
			sif_conf1_reg.bit.Cont_Ch0 = SIFContMode[Ch];
		 	SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);
			sif_conf2_reg.reg = SIF_GETREG(SIF_CONF2_REG_OFS);
			sif_conf2_reg.bit.Ch0_Addr_Bits = uiSIFContAddrBits[Ch];
		 	SIF_SETREG(SIF_CONF2_REG_OFS, sif_conf2_reg.reg);

			// Configure DMA starting address and length
			sif_dma0start_reg.reg = 0;
			sif_dma0start_reg.bit.DMA_StartAddr = pa & 0xffffffff;
			SIF_SETREG(SIF_DMA0_STARTADDR_REG_OFS, sif_dma0start_reg.reg);
			sif_dma0ctrl1_reg.reg = 0;
			sif_dma0ctrl1_reg.bit.DMA0_BufSize1 = uiDMALength1;
			sif_dma0ctrl1_reg.bit.DMA0_BufSize2 = uiDMALength2;
			SIF_SETREG(SIF_DMA0_CTRL1_REG_OFS, sif_dma0ctrl1_reg.reg);
			sif_dma0ctrl2_reg.reg = 0;
			sif_dma0ctrl2_reg.bit.DMA0_BufSize3 = uiDMALength3;
			SIF_SETREG(SIF_DMA0_CTRL2_REG_OFS, sif_dma0ctrl2_reg.reg);

			// Configure DMA channel and enable DMA
			sif_dma0ctrl0_reg.reg = 0;
			sif_dma0ctrl0_reg.bit.DMA0_Burst_N =  uiBurstNumber;
			sif_dma0ctrl0_reg.bit.DMA0_Sync_En =  ENABLE;
			sif_dma0ctrl0_reg.bit.DMA0_En  =      ENABLE;
			SIF_SETREG(SIF_DMA0_CTRL0_REG_OFS, sif_dma0ctrl0_reg.reg);
			break;

		case SIF_CH1:
			// Configure DMA mode and address bits
			sif_conf1_reg.reg = SIF_GETREG(SIF_CONF1_REG_OFS);
			sif_conf1_reg.bit.Cont_Ch1 = SIFContMode[Ch];
		 	SIF_SETREG(SIF_CONF1_REG_OFS, sif_conf1_reg.reg);
			sif_conf2_reg.reg = SIF_GETREG(SIF_CONF2_REG_OFS);
			sif_conf2_reg.bit.Ch1_Addr_Bits = uiSIFContAddrBits[Ch];
		 	SIF_SETREG(SIF_CONF2_REG_OFS, sif_conf2_reg.reg);

			// Configure DMA starting address and length
			sif_dma1start_reg.reg = 0;
			sif_dma1start_reg.bit.DMA_StartAddr = pa & 0xffffffff;
			SIF_SETREG(SIF_DMA1_STARTADDR_REG_OFS, sif_dma1start_reg.reg);
			sif_dma1ctrl1_reg.reg = 0;
			sif_dma1ctrl1_reg.bit.DMA1_BufSize1 = uiDMALength1;
			sif_dma1ctrl1_reg.bit.DMA1_BufSize2 = uiDMALength2;
			SIF_SETREG(SIF_DMA1_CTRL1_REG_OFS, sif_dma1ctrl1_reg.reg);
			sif_dma1ctrl2_reg.reg = 0;
			sif_dma1ctrl2_reg.bit.DMA1_BufSize3 = uiDMALength3;
			SIF_SETREG(SIF_DMA1_CTRL2_REG_OFS, sif_dma1ctrl2_reg.reg);

			// Configure DMA channel and enable DMA
			sif_dma1ctrl0_reg.reg = 0;
			sif_dma1ctrl0_reg.bit.DMA1_Burst_N =  uiBurstNumber;
			sif_dma1ctrl0_reg.bit.DMA1_Sync_En =  ENABLE;
			sif_dma1ctrl0_reg.bit.DMA1_En  =      ENABLE;
			SIF_SETREG(SIF_DMA1_CTRL0_REG_OFS, sif_dma1ctrl0_reg.reg);
			break;
	}
}

/**
    Wait for SIF DMA transmission end

    Call this API will wait for SIF DMA transmission end.

    @note Don't call this function in ISR.

    @return void
*/
void sif_waitDMATransferEnd(SIF_CH Ch)
{
	union SIF_INTSTS0_REG sif_intsts0_reg;
	unsigned int time_start, time_now;

	// Check if SIF driver is opened
	if (uiSIFOpenStatus == 0) {
		printf("SIF isn't opened!\r\n");
		return;
	}

	// Polling dma end status becomes to 1 and clear it
	time_start = get_timer(0);
	while (1) {
		time_now = get_timer(0);
		sif_intsts0_reg.reg = SIF_GETREG(SIF_INTSTS0_REG_OFS);
		if ((sif_intsts0_reg.reg & SIF_INT_DMA(Ch))) {
			SIF_SETREG(SIF_INTSTS0_REG_OFS, SIF_INT_DMA(Ch));
			break;
		} 
		if ((time_now - time_start) > WAIT_SIF_TIMEOUT) {
			printf("waitDMATransferEnd dma timeout\n\r");
			return;
		}
	}
}


BOOL sif_dmapack_start(UINT32 *current_index)
{
	if (current_index != NULL) {
		*current_index = 0;
	} else {
		printf("index NULL\r\n");
	}
	return FALSE;
}


BOOL sif_dmapack_data(SIF_CH Ch, ulong desc_buf_addr, UINT32 *current_index, UINT32 max_buf_size, UINT32 data_bit_len, UINT32 *data)
{
	union SIF_DMA0_DLY_REG sif_dma0dly_reg;
	union SIF_DMA1_DLY_REG sif_dma1dly_reg;
	ulong *pDesc;
	UINT32 i, wordcnt, tag;

	pDesc = (ulong*)(desc_buf_addr + *current_index);
	wordcnt = (data_bit_len + 31) >> 5;

	if ((*current_index + (wordcnt << 2)) > max_buf_size) {
		printf("DMA Buf Size not enough %d %d\r\n", *current_index, max_buf_size);
		return TRUE;
	}

	if (Ch == SIF_CH0) {
		sif_dma0dly_reg.reg = SIF_GETREG(SIF_DMA0_DLY_REG_OFS);
		tag = sif_dma0dly_reg.bit.SIF_DLY_TAG;
	} else if (Ch == SIF_CH1) {
		sif_dma1dly_reg.reg = SIF_GETREG(SIF_DMA1_DLY_REG_OFS);
		tag = sif_dma1dly_reg.bit.SIF_DLY_TAG;
	} else {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return TRUE;
	}

	for (i = 0; i < wordcnt; i++) {
		pDesc[i] = data[i];
		if ((data[i] >> 16) == tag) {
			printf("DATA(0x%08X) Hits DelayTAG\r\n", data[i]);
		}
	}

	*current_index += (wordcnt << 2);
	return FALSE;
}


BOOL sif_dmapack_delay_us(SIF_CH Ch, ulong desc_buf_addr, UINT32 *current_index, UINT32 max_buf_size, UINT32 delay_us)
{
	union SIF_DMA0_DLY_REG sif_dma0dly_reg;
	union SIF_DMA1_DLY_REG sif_dma1dly_reg;
	ulong *pDesc;
	UINT64 tickcount;
	UINT32 i, wordcnt, tag;

	pDesc = (ulong*)(desc_buf_addr + *current_index);
	tickcount = (UINT64)delay_us * SIF_SOURCE_CLOCK;
	do_div(tickcount, 1000000);
	tickcount += 65535;
	do_div(tickcount, 65535);
	wordcnt = (UINT32)tickcount;
 
	if ((*current_index + (wordcnt << 2)) > max_buf_size) {
		printf("DMA Buf Size not enough %d %d\r\n", *current_index, max_buf_size);
		return TRUE;
	}

	if (Ch == SIF_CH0) {
		sif_dma0dly_reg.reg = SIF_GETREG(SIF_DMA0_DLY_REG_OFS);
		tag = sif_dma0dly_reg.bit.SIF_DLY_TAG << 16;
	} else if (Ch == SIF_CH1) {
		sif_dma1dly_reg.reg = SIF_GETREG(SIF_DMA1_DLY_REG_OFS);
		tag = sif_dma1dly_reg.bit.SIF_DLY_TAG << 16;
	} else {
		printf("SIF_CH%d does NOT support DMA!\r\n", Ch);
		return TRUE;
	}

	for(i = 0; i < wordcnt; i++) {
		if (tickcount > 65535) {
			pDesc[i] = 65535 + tag;
			tickcount -= 65535;
		} else {
			pDesc[i] = tickcount + tag;
		}
		//printf("DLY(0x%08X)\r\n" ,pDesc[i]);
	}

	*current_index += (wordcnt << 2);
	return FALSE;
}


static void sif_parse_parameter(struct nvt_sif_priv *sif)
{
	/* Enable it after dts parsing ready*/
	int nodeoffset;
	u32 *cell = NULL;
	UINT32 reg;

	nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/top/sif");
	cell = (u32 *)fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "pinmux", NULL);
	sif->pinmux_value = __be32_to_cpu(cell[0]);

	// TODO: set pinmux via pinmux driver
	if (sif->pinmux_value == 0x1) {
		//sif 0_1 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 0);
		reg |= (0x1 << 0);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set SGPIO4/5/6 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xC0);
		reg &= ~(0x7 << 4);
		OUTW(IOADDR_TOP_REG_BASE + 0xC0, reg);
	} else if (sif->pinmux_value == 0x10) {
		//sif 1_1 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 4);
		reg |= (0x1 << 4);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set SGPIO9/10/11 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xC0);
		reg &= ~(0x7 << 9);
		OUTW(IOADDR_TOP_REG_BASE + 0xC0, reg);
	} else if (sif->pinmux_value == 0x100) {
		//sif 2_1 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 8);
		reg |= (0x1 << 8);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set LGPIO22/23/24 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xB8);
		reg &= ~(0x7 << 22);
		OUTW(IOADDR_TOP_REG_BASE + 0xB8, reg);
	}  else if (sif->pinmux_value == 0x200) {
		//sif 2_2 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 8);
		reg |= (0x2 << 8);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set PGPIO18/19/20 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xA8);
		reg &= ~(0x7 << 18);
		OUTW(IOADDR_TOP_REG_BASE + 0xA8, reg);
	} 
	else if (sif->pinmux_value == 0x400) {
		//sif 2_3 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 8);
		reg |= (0x3 << 8);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set DGPIO8/9/10 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xB4);
		reg &= ~(0x7 << 8);
		OUTW(IOADDR_TOP_REG_BASE + 0xB4, reg);
	}  else if (sif->pinmux_value == 0x1000) {
		//sif 3_1 pinmux
		reg = INW(IOADDR_TOP_REG_BASE + 0x48);
		reg &= ~(0xF << 12);
		reg |= (0x1 << 12);
		OUTW(IOADDR_TOP_REG_BASE + 0x48, reg);

		//set PGPIO13/14/15 to function mode
		reg = INW(IOADDR_TOP_REG_BASE + 0xA8);
		reg &= ~(0x7 << 13);
		OUTW(IOADDR_TOP_REG_BASE + 0xA8, reg);
	}

}


void sif_init(void)
{
	struct nvt_sif_priv *sif;
	printf("SIF version: %s\n", SIF_VERSION);

	sif = malloc(sizeof(struct nvt_sif_priv));
	if (!sif) {
		printf("alloc nvt_sif_priv failed!\n");
		return;
	}

	sif_parse_parameter(sif);
}