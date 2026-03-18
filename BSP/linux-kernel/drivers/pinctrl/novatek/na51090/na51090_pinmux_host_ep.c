/*
	Pinmux module driver.

	This file is the driver of Piumux module.

	@file		na51055_pinmux_host.c
	@ingroup
	@note		Nothing.

	Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#include "na51090_pinmux.h"
#include <plat/pad.h>


static spinlock_t top_lock;
#define loc_cpu(flags) spin_lock_irqsave(&top_lock, flags)
#define unl_cpu(flags) spin_unlock_irqrestore(&top_lock, flags)

#define TOP_CTRL0_REG_OFS           0x00
#define TOP_CTRL1_REG_OFS           0x04
#define TOP_CTRL2_REG_OFS           0x08
#define TOP_CTRL3_REG_OFS           0x0C
#define TOP_CTRL4_REG_OFS           0x10
#define TOP_CTRL5_REG_OFS           0x14
#define TOP_CTRL6_REG_OFS           0x18
#define TOP_CTRL7_REG_OFS           0x1C
#define TOP_CTRL8_REG_OFS           0x20
#define TOP_CTRL9_REG_OFS           0x24
#define TOP_CTRL10_REG_OFS          0x28
#define TOP_CTRL11_REG_OFS          0x2C
#define TOP_CTRL12_REG_OFS          0x30
//#define TOP_CTRL13_REG_OFS          0x74
//#define TOP_CTRL14_REG_OFS          0x80
#define TOP_CGPIO0_REG_OFS          0xC0
#define TOP_VCAP_MUX_REG_OFS        0xC0
#define TOP_ETH_MUX_REG_OFS         0xC8
#define TOP_LCD_MUX_REG_OFS         0xCC

static union TOP_REG0 top_reg0;
static union TOP_REG1 top_reg1;
static union TOP_REG2 top_reg2;
static union TOP_REG3 top_reg3;
static union TOP_REG4 top_reg4;
static union TOP_REG5 top_reg5;
static union TOP_REG6 top_reg6;
static union TOP_REG7 top_reg7;
//static union TOP_REG8 top_reg8;
static union TOP_REG9 top_reg9;
static union TOP_REG10 top_reg10;
static union TOP_REG11 top_reg11;
static union TOP_REG12 top_reg12;

static union TOP_REGCGPIO0 top_reg_cgpio0;
static union TOP_REGJGPIO0 top_reg_jgpio0;
static union TOP_REGPGPIO0 top_reg_pgpio0;
static union TOP_REGPGPIO1 top_reg_pgpio1;
static union TOP_REGEGPIO0 top_reg_egpio0;
static union TOP_REGDGPIO0 top_reg_dgpio0;
static union TOP_REGSGPIO0 top_reg_sgpio0;
static union TOP_REGSGPIO1 top_reg_sgpio1;
static union TOP_REGSGPIO2 top_reg_sgpio2;
static union TOP_REGBGPIO0 top_reg_bgpio0;

static int store_extclk2[64] = {};
int record_ch = 0;
void pinmux_preset_ep(struct nvt_pinctrl_info *info)
{
	spin_lock_init(&top_lock);
}

EXPORT_SYMBOL(pinmux_preset_ep);

/**
	Read pinmux data from controller base

	Read pinmux data from controller base

	@param[in] info	nvt_pinctrl_info
*/

void pinmux_parsing_ep(struct nvt_pinctrl_info *info)
{
	u32 value;
	unsigned long flags = 0;
	union TOP_REG0 local_top_reg0;
	union TOP_REG1 local_top_reg1;
	union TOP_REG2 local_top_reg2;
	union TOP_REG3 local_top_reg3;
	union TOP_REG4 local_top_reg4;
	union TOP_REG5 local_top_reg5;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
	union TOP_REG8 local_top_reg8;
	union TOP_REG9 local_top_reg9;
	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;
	union TOP_REG12 local_top_reg12;
	union TOP_REGPGPIO0 local_top_reg_pgpio0;
	union TOP_REGPGPIO1 local_top_reg_pgpio1;
	union TOP_REGEGPIO0 local_top_reg_egpio0;
	union TOP_REGSGPIO1 local_top_reg_sgpio1;
	union TOP_REGSGPIO2 local_top_reg_sgpio2;
	union TOP_REGDGPIO0 local_top_reg_dgpio0;

	loc_cpu(flags);
	local_top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg_pgpio0.reg = TOP_GETREG(info, TOP_REGPGPIO0_OFS);
	local_top_reg_pgpio1.reg = TOP_GETREG(info, TOP_REGPGPIO1_OFS);
	local_top_reg_egpio0.reg = TOP_GETREG(info, TOP_REGEGPIO0_OFS);
	local_top_reg_sgpio1.reg = TOP_GETREG(info, TOP_REGSGPIO1_OFS);
	local_top_reg_sgpio2.reg = TOP_GETREG(info, TOP_REGSGPIO2_OFS);
	local_top_reg_dgpio0.reg = TOP_GETREG(info, TOP_REGDGPIO0_OFS);

	/*Parsing UART*/
	value = 0x0;
	switch (local_top_reg7.bit.UART) {
	case UART_MUX_1:
		value |= PIN_UART_CFG_CH_1ST_PINMUX;
		break;
	case UART_MUX_2:
		value |= PIN_UART_CFG_CH_2ND_PINMUX;
		break;
	case UART_MUX_3:
		value |= PIN_UART_CFG_CH_3RD_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg7.bit.UART_CTS_RTS != 0x0)
		value |= PIN_UART_CFG_CH_CTSRTS;

	switch (local_top_reg7.bit.UART2) {
	case UART_MUX_1:
		value |= PIN_UART_CFG_CH2_1ST_PINMUX;
		break;
	case UART_MUX_2:
		value |= PIN_UART_CFG_CH2_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg7.bit.UART2_CTS_RTS != 0x0)
		value |= PIN_UART_CFG_CH2_CTSRTS;

	if (local_top_reg7.bit.UART3 == UART_MUX_1)
		value |= PIN_UART_CFG_CH3_1ST_PINMUX;

	switch (local_top_reg7.bit.UART4) {
	case UART_MUX_1:
		value |= PIN_UART_CFG_CH4_1ST_PINMUX;
		break;
	case UART_MUX_2:
		value |= PIN_UART_CFG_CH4_2ND_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg7.bit.UART5) {
	case UART_MUX_1:
		value |= PIN_UART_CFG_CH5_1ST_PINMUX;
		break;
	case UART_MUX_2:
		value |= PIN_UART_CFG_CH5_2ND_PINMUX;
		break;
	default:
		break;
	}

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/*Parsing I2C*/
	value = 0x0;
	switch (local_top_reg4.bit.I2C) {
	case MUX_1:
		value |= PIN_I2C_CFG_CH_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_I2C_CFG_CH_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg4.bit.I2C2 == MUX_1)
		value |= PIN_I2C_CFG_CH2_1ST_PINMUX;

	switch (local_top_reg4.bit.I2C3) {
	case MUX_1:
		value |= PIN_I2C_CFG_CH3_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_I2C_CFG_CH3_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_I2C_CFG_CH3_3RD_PINMUX;
		break;
	case MUX_4:
		value |= PIN_I2C_CFG_CH3_4TH_PINMUX;
		break;
	default:
		break;
	}

	if ((local_top_reg_pgpio1.bit.PGPIO_37 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_pgpio1.bit.PGPIO_38 == GPIO_ID_EMUM_FUNC))
		value |= PIN_I2C_CFG_HDMI_1ST_PINMUX;

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/*Parsing SDIO*/
	value = 0x0;
	switch (local_top_reg1.bit.SDIO) {
	case MUX_1:
		value |= PIN_SDIO_CFG_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_SDIO_CFG_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg1.bit.SDIO2 == MUX_1)
		value |= PIN_SDIO2_CFG_1ST_PINMUX;

	if (local_top_reg1.bit.SDIO2_BUS_WIDTH == SDIO_BUS_WIDTH_8)
		value |= PIN_SDIO2_CFG_BUS_WIDTH;

	if (local_top_reg1.bit.SDIO2_DS == MUX_EN)
		value |= PIN_SDIO2_CFG_DS;

	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/*Parsing SPI*/
	value = 0x0;
	if (local_top_reg1.bit.SPI_EN == MUX_EN)
		value |= PIN_SPI_CFG_CH_1ST_PINMUX;

	if (local_top_reg1.bit.SPI_BUS_WIDTH == SPI_BUS_WIDTH_4)
		value |= PIN_SPI_CFG_CH_BUS_WIDTH;

	if (local_top_reg1.bit.SPI_CS1_EN == MUX_EN)
		value |= PIN_SPI_CFG_CH_CS1;

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/*Parsing EXTCLK*/
	value = 0x0;
	if (local_top_reg5.bit.EXT_CLK == MUX_EN)
		value |= PIN_EXTCLK_CFG_CH_1ST_PINMUX;

	if ((local_top_reg_pgpio0.bit.PGPIO_27 == GPIO_ID_EMUM_FUNC) && store_extclk2[info->ep_ch])
		value |= PIN_EXTCLK_CFG_CH2_1ST_PINMUX;

	if (local_top_reg_pgpio0.bit.PGPIO_28 == GPIO_ID_EMUM_FUNC)
		value |= PIN_EXTCLK_CFG_CH3_1ST_PINMUX;

	if (local_top_reg_pgpio0.bit.PGPIO_29 == GPIO_ID_EMUM_FUNC)
		value |= PIN_EXTCLK_CFG_CH4_1ST_PINMUX;

	info->top_pinmux[PIN_FUNC_EXTCLK].config = value;
	info->top_pinmux[PIN_FUNC_EXTCLK].pin_function = PIN_FUNC_EXTCLK;

	/*Parsing SSP*/
	value = 0x0;
	if (local_top_reg11.bit.I2S == MUX_EN)
		value |= PIN_SSP_CFG_CH_1ST_PINMUX;

	if (local_top_reg11.bit.I2S_MCLK == MUX_EN)
		value |= PIN_SSP_CFG_CH_1ST_MCLK;

	if (local_top_reg11.bit.I2S_TX == MUX_EN)
		value |= PIN_SSP_CFG_CH_1ST_TX;

	if (local_top_reg11.bit.I2S_RX == MUX_EN)
		value |= PIN_SSP_CFG_CH_1ST_RX;

	if (local_top_reg11.bit.I2S2 == MUX_EN)
		value |= PIN_SSP_CFG_CH2_1ST_PINMUX;

	if (local_top_reg11.bit.I2S2_MCLK == MUX_EN)
		value |= PIN_SSP_CFG_CH2_1ST_MCLK;

	if (local_top_reg11.bit.I2S2_TX == MUX_EN)
		value |= PIN_SSP_CFG_CH2_1ST_TX;

	if (local_top_reg11.bit.I2S2_RX == MUX_EN)
		value |= PIN_SSP_CFG_CH2_1ST_RX;

	switch (local_top_reg11.bit.I2S3) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH3_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH3_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_SSP_CFG_CH3_3RD_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg11.bit.I2S3_MCLK) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH3_1ST_MCLK;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH3_2ND_MCLK;
		break;
	case MUX_3:
		value |= PIN_SSP_CFG_CH3_3RD_MCLK;
		break;
	default:
		break;
	}

	switch (local_top_reg11.bit.I2S3_TX) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH3_1ST_TX;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH3_2ND_TX;
		break;
	case MUX_3:
		value |= PIN_SSP_CFG_CH3_3RD_TX;
		break;
	default:
		break;
	}

	switch (local_top_reg11.bit.I2S3_RX) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH3_1ST_RX;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH3_2ND_RX;
		break;
	case MUX_3:
		value |= PIN_SSP_CFG_CH3_3RD_RX;
		break;
	default:
		break;
	}

	if (local_top_reg12.bit.I2S4 == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_PINMUX;

	if (local_top_reg12.bit.I2S4_MCLK == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_MCLK;

	if (local_top_reg12.bit.I2S4_TX == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_TX;

	if (local_top_reg12.bit.I2S4_RX == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_RX;

	info->top_pinmux[PIN_FUNC_SSP].config = value;
	info->top_pinmux[PIN_FUNC_SSP].pin_function = PIN_FUNC_SSP;

	/*Parsing LCD*/
	value = 0x0;
	if (local_top_reg2.bit.LCD310 == MUX_2)
		value |= PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX;

	if (local_top_reg2.bit.LCD310L == MUX_2)
		value |= PIN_LCD_CFG_LCD310L_RGB888_1ST_PINMUX;

	if (local_top_reg2.bit.LCD310_DE == MUX_EN)
		value |= PIN_LCD_CFG_LCD310_DE_PINMUX;

	if (local_top_reg2.bit.LCD310 == MUX_1)
		value |= PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX;

	if (local_top_reg2.bit.LCD310L == MUX_1)
		value |= PIN_LCD_CFG_LCD310L_BT1120_1ST_PINMUX;

	if (local_top_reg2.bit.LCD210 == MUX_1)
		value |= PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX;

	info->top_pinmux[PIN_FUNC_LCD].config = value;
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/*Parsing REMOTE*/
	value = 0x0;
	if (local_top_reg5.bit.IRDA)
		value |= (PIN_REMOTE_CFG_1ST_PINMUX << (local_top_reg5.bit.IRDA - 1));

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/*Parsing VCAP*/
	value = 0x0;
	if (local_top_reg9.bit.CAP == MUX_EN)
		value |= PIN_VCAP_CFG_CAP0_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP1 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP1_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP1_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP2 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP2_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP2_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP3 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP3_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP3_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP4 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP4_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP4_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP4_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP5 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP5_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP5_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP5_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP6 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP6_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP6_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP6_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg9.bit.CAP7 == MUX_EN)
		value |= PIN_VCAP_CFG_CAP7_1ST_PINMUX;

	switch (local_top_reg9.bit.CAP7_CLK) {
	case MUX_1:
		value |= PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_VCAP_CFG_CAP7_CLK_2ND_PINMUX;
		break;
	default:
		break;
	}

	info->top_pinmux[PIN_FUNC_VCAP].config = value;
	info->top_pinmux[PIN_FUNC_VCAP].pin_function = PIN_FUNC_VCAP;

	/*Parsing ETH*/
	value = 0x0;
	switch (local_top_reg3.bit.ETH) {
	case MUX_1:
		value |= PIN_ETH_CFG_RGMII_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_ETH_CFG_RMII_1ST_PINMUX;
		break;
	case MUX_3:
		value |= PIN_ETH_CFG_RGMII_2ND_PINMUX;
		break;
	case MUX_4:
		value |= PIN_ETH_CFG_RMII_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg3.bit.ETH_REFCLK == MUX_EN)
		value |= PIN_ETH_CFG_REFCLK_PINMUX;

	if (local_top_reg3.bit.ETH_RST == MUX_EN)
		value |= PIN_ETH_CFG_RST_PINMUX;

	if (local_top_reg3.bit.ETH_MDC_MDIO)
		value |= PIN_ETH_CFG_MDC_MDIO_PINMUX;


	switch (local_top_reg3.bit.ETH2) {
	case MUX_1:
		value |= PIN_ETH2_CFG_RGMII_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_ETH2_CFG_RMII_1ST_PINMUX;
		break;
	case MUX_3:
		value |= PIN_ETH2_CFG_RGMII_2ND_PINMUX;
		break;
	case MUX_4:
		value |= PIN_ETH2_CFG_RMII_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg3.bit.ETH2_REFCLK == MUX_EN)
		value |= PIN_ETH2_CFG_REFCLK_PINMUX;

	if (local_top_reg3.bit.ETH2_RST == MUX_EN)
		value |= PIN_ETH2_CFG_RST_PINMUX;

	if (local_top_reg3.bit.ETH2_MDC_MDIO)
		value |= PIN_ETH2_CFG_MDC_MDIO_PINMUX;

	info->top_pinmux[PIN_FUNC_ETH].config = value;
	info->top_pinmux[PIN_FUNC_ETH].pin_function = PIN_FUNC_ETH;

	/*Parsing MISC*/
	value = 0x0;
	if (local_top_reg1.bit.EJTAG_EN == MUX_EN)
		value |= PIN_MISC_CFG_CPU_ICE;

	if (local_top_reg1.bit.EXTROM_EN == MUX_EN)
		value |= PIN_MISC_CFG_BMC;

	if (local_top_reg5.bit.RTC_CAL == MUX_EN)
		value |= PIN_MISC_CFG_RTC_CAL_OUT;

	if (local_top_reg5.bit.TVDAC_TEST_CLK == MUX_EN)
		value |= PIN_MISC_CFG_DAC_RAMP_TP;

	if (local_top_reg_pgpio1.bit.PGPIO_39 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_HDMI_HOTPLUG;

	if (local_top_reg_pgpio1.bit.PGPIO_35 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_VGA_HS;

	if (local_top_reg_pgpio1.bit.PGPIO_36 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_VGA_VS;

	switch (local_top_reg5.bit.SATA_LED) {
	case MUX_1:
		value |= PIN_MISC_CFG_SATA_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_MISC_CFG_SATA_LED_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_MISC_CFG_SATA_LED_3RD_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg5.bit.SATA2_LED) {
	case MUX_1:
		value |= PIN_MISC_CFG_SATA2_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_MISC_CFG_SATA2_LED_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_MISC_CFG_SATA2_LED_3RD_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg5.bit.SATA3_LED) {
	case MUX_1:
		value |= PIN_MISC_CFG_SATA3_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_MISC_CFG_SATA3_LED_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_MISC_CFG_SATA3_LED_3RD_PINMUX;
		break;
	default:
		break;
	}

	if ((local_top_reg_dgpio0.bit.DGPIO_1 == GPIO_ID_EMUM_FUNC) &&
		(local_top_reg4.bit.CLK_12M ==MUX_EN))
		value |= PIN_MISC_CFG_12M_CLK;

	info->top_pinmux[PIN_FUNC_MISC].config = value;
	info->top_pinmux[PIN_FUNC_MISC].pin_function = PIN_FUNC_MISC;

	/*Parsing PWM*/
	value = 0x0;
	if (local_top_reg6.bit.PWM == MUX_EN)
		value |= PIN_PWM_CFG_CH_1ST_PINMUX;

	if (local_top_reg6.bit.PWM2 == MUX_EN)
		value |= PIN_PWM_CFG_CH2_1ST_PINMUX;

	if (local_top_reg6.bit.PWM3 == MUX_EN)
		value |= PIN_PWM_CFG_CH3_1ST_PINMUX;

	if (local_top_reg6.bit.PWM4 == MUX_EN)
		value |= PIN_PWM_CFG_CH4_1ST_PINMUX;

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/*Parsing MIPI*/
	value = 0x0;

	if ((local_top_reg9.bit.CAP4 != MUX_EN) && (local_top_reg9.bit.CAP4_CLK != MUX_2) && \
	    (local_top_reg_sgpio1.bit.SGPIO_36 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_37 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_DAT0;

	if ((local_top_reg9.bit.CAP4 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_38 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_39 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_DAT1;

	if ((local_top_reg9.bit.CAP4 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_40 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_41 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_CLK0;

	if ((local_top_reg9.bit.CAP4 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_42 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_43 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_DAT2;

	if ((local_top_reg9.bit.CAP4 != MUX_EN) && (local_top_reg9.bit.CAP4_CLK != MUX_1) && \
	    (local_top_reg_sgpio1.bit.SGPIO_44 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_45 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_DAT3;

	if ((local_top_reg_sgpio1.bit.SGPIO_46 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_47 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY_CLK1;

	if ((local_top_reg9.bit.CAP5 != MUX_EN) && (local_top_reg9.bit.CAP5_CLK != MUX_2) && \
	    (local_top_reg_sgpio1.bit.SGPIO_48 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_49 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_DAT0;

	if ((local_top_reg9.bit.CAP5 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_50 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_51 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_DAT1;

	if ((local_top_reg9.bit.CAP5 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_52 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_53 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_CLK0;

	if ((local_top_reg9.bit.CAP5 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_54 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_55 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_DAT2;

	if ((local_top_reg9.bit.CAP5 != MUX_EN) && (local_top_reg9.bit.CAP5_CLK != MUX_1) && \
	    (local_top_reg_sgpio1.bit.SGPIO_56 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_57 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_DAT3;

	if ((local_top_reg_sgpio1.bit.SGPIO_58 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_59 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY2_CLK1;

	if ((local_top_reg9.bit.CAP6 != MUX_EN) && (local_top_reg9.bit.CAP6_CLK != MUX_2) && \
	    (local_top_reg_sgpio1.bit.SGPIO_60 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_61 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_DAT0;

	if ((local_top_reg9.bit.CAP6 != MUX_EN) && \
	    (local_top_reg_sgpio1.bit.SGPIO_62 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio1.bit.SGPIO_63 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_DAT1;

	if ((local_top_reg9.bit.CAP6 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_64 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_65 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_CLK0;

	if ((local_top_reg9.bit.CAP6 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_66 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_67 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_DAT2;

	if ((local_top_reg9.bit.CAP6 != MUX_EN) && (local_top_reg9.bit.CAP6_CLK != MUX_1) && \
	    (local_top_reg_sgpio2.bit.SGPIO_68 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_69 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_DAT3;

	if ((local_top_reg_sgpio2.bit.SGPIO_70 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_71 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY3_CLK1;

	if ((local_top_reg9.bit.CAP7 != MUX_EN) && (local_top_reg9.bit.CAP7_CLK != MUX_1) && \
	    (local_top_reg_sgpio2.bit.SGPIO_72 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_73 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_DAT0;

	if ((local_top_reg9.bit.CAP7 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_74 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_75 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_DAT1;

	if ((local_top_reg9.bit.CAP7 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_76 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_77 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_CLK0;

	if ((local_top_reg9.bit.CAP7 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_78 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_79 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_DAT2;

	if ((local_top_reg9.bit.CAP7 != MUX_EN) && \
	    (local_top_reg_sgpio2.bit.SGPIO_80 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_81 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_DAT3;

	if ((local_top_reg5.bit.IRDA != 4) && (local_top_reg5.bit.IRDA != 5) && \
	    (local_top_reg_sgpio2.bit.SGPIO_82 == GPIO_ID_EMUM_FUNC) && \
	    (local_top_reg_sgpio2.bit.SGPIO_83 == GPIO_ID_EMUM_FUNC))
		value |= PIN_MIPI_CFG_PHY4_CLK1;

	info->top_pinmux[PIN_FUNC_MIPI].config = value;
	info->top_pinmux[PIN_FUNC_MIPI].pin_function = PIN_FUNC_MIPI;
	unl_cpu(flags);
}
EXPORT_SYMBOL(pinmux_parsing_ep);

static ER pinmux_config_uart(uint32_t config, u32 ep_ch)
{
	if (config == PIN_UART_CFG_NONE) {
	} else {
		if (config & (PIN_UART_CFG_CH_1ST_PINMUX | PIN_UART_CFG_CH_2ND_PINMUX | PIN_UART_CFG_CH_3RD_PINMUX | PIN_UART_CFG_CH_CTSRTS)) {
			if (config & PIN_UART_CFG_CH_1ST_PINMUX) {
				if (config & PIN_UART_CFG_CH_CTSRTS) {
					top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
					top_reg7.bit.UART_CTS_RTS = UART_MUX_1;
				}
				top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART = UART_MUX_1;
			} else if (config & PIN_UART_CFG_CH_2ND_PINMUX) {
				if (config & PIN_UART_CFG_CH_CTSRTS) {
					if (top_reg11.bit.I2S3_TX == MUX_2) {
						pr_err("PIN_UART_CFG_CH_CTSRTS conflict with I2S3_2_TX\r\n");
						return E_OBJ;
					}
					top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
					top_reg7.bit.UART_CTS_RTS = UART_MUX_2;
				}
				if (top_reg11.bit.I2S == MUX_EN) {
					pr_err("PIN_UART_CFG_CH_2ND_PINMUX conflict with I2S\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_UART_CFG_CH_2ND_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_UART_CFG_CH_2ND_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART = UART_MUX_2;
			} else {
				if (config & PIN_UART_CFG_CH_CTSRTS) {
					if (top_reg4.bit.I2C == MUX_2) {
						pr_err("PIN_UART_CFG_CH_3RD_PINMUX conflict with I2C_2\r\n");
						return E_OBJ;
					}
					if ((top_reg5.bit.IRDA == 12) || (top_reg5.bit.IRDA == 13)) {
						pr_err("PIN_UART_CFG_CH_3RD_PINMUX conflict with IRDA11/12\r\n");
						return E_OBJ;
					}
					top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
					top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
					top_reg7.bit.UART_CTS_RTS = UART_MUX_3;
				}
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART = UART_MUX_3;
			}
		}
		if (config & (PIN_UART_CFG_CH2_1ST_PINMUX | PIN_UART_CFG_CH2_2ND_PINMUX | PIN_UART_CFG_CH2_CTSRTS)) {

			if (config & PIN_UART_CFG_CH2_1ST_PINMUX) {
				if (config & PIN_UART_CFG_CH2_CTSRTS) {
					if (top_reg4.bit.I2C3 == MUX_3) {
						pr_err("PIN_UART_CFG_CH2_CTSRTS conflict with I2C3_3\r\n");
						return E_OBJ;
					}
					top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
					top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
					top_reg7.bit.UART2_CTS_RTS = UART_MUX_1;
				}
				top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART2 = UART_MUX_1;
			} else if (config & PIN_UART_CFG_CH2_2ND_PINMUX) {
				if (config & PIN_UART_CFG_CH2_CTSRTS) {
					if (top_reg4.bit.I2C3 == MUX_4) {
						pr_err("PIN_UART_CFG_CH2_CTSRTS conflict with I2C3_4\r\n");
						return E_OBJ;
					}
					top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
					top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
					top_reg7.bit.UART2_CTS_RTS = UART_MUX_2;
				}
				if (top_reg1.bit.SDIO2 == MUX_1) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with SDIO2\r\n");
					return E_OBJ;
				}
				top_reg7.bit.UART2 = UART_MUX_2;
				top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			}
		}
		if (config & (PIN_UART_CFG_CH3_1ST_PINMUX)) {
			if (top_reg12.bit.I2S4_TX == MUX_EN) {
				pr_err("PIN_UART_CFG_CH3_1ST_PINMUX conflict with I2S4_TX\r\n");
				return E_OBJ;
			}
			if (top_reg12.bit.I2S4_MCLK == MUX_EN) {
				pr_err("PIN_UART_CFG_CH3_1ST_PINMUX conflict with I2S4_MCLK\r\n");
				return E_OBJ;
			}
			if (top_reg12.bit.I2S4_RX == MUX_EN) {
				pr_err("PIN_UART_CFG_CH3_1ST_PINMUX conflict with I2S4_RX\r\n");
				return E_OBJ;
			}
			if (top_reg5.bit.SATA_LED == MUX_2) {
				pr_err("PIN_UART_CFG_CH3_1ST_PINMUX conflict with SATA_2_LED\r\n");
				return E_OBJ;
			}
			if (top_reg5.bit.SATA2_LED == MUX_2) {
				pr_err("PIN_UART_CFG_CH3_1ST_PINMUX conflict with SATA2_2_LED\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.UART3 = UART_MUX_1;
		}
		if (config & (PIN_UART_CFG_CH4_1ST_PINMUX | PIN_UART_CFG_CH4_2ND_PINMUX)) {
			if (config & PIN_UART_CFG_CH4_1ST_PINMUX) {
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = UART_MUX_1;
			} else {
				if (top_reg4.bit.I2C == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with I2C\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 6) || (top_reg5.bit.IRDA == 7)) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with IRDA5/6\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = UART_MUX_2;
			}
		}
		if (config & (PIN_UART_CFG_CH5_1ST_PINMUX | PIN_UART_CFG_CH5_2ND_PINMUX)) {
			if (config & PIN_UART_CFG_CH5_1ST_PINMUX) {
				if ((top_reg11.bit.I2S3_TX == MUX_3) || (top_reg11.bit.I2S3_RX == MUX_3)) {
					pr_err("PIN_UART_CFG_CH5_1ST_PINMUX conflict with I2S3_3_TXRX\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_UART_CFG_CH5_1ST_PINMUX conflict with JTAG\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART5 = UART_MUX_1;
			} else {
				if (top_reg4.bit.I2C2 == MUX_1) {
					pr_err("PIN_UART_CFG_CH5_2ND_PINMUX conflict with I2C2\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 8) || (top_reg5.bit.IRDA == 9)) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with IRDA7/8\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART5 = UART_MUX_2;
			}
		}
	}

	return E_OK;
}

static ER pinmux_config_i2c(uint32_t config, u32 ep_ch)
{
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & (PIN_I2C_CFG_CH_1ST_PINMUX | PIN_I2C_CFG_CH_2ND_PINMUX)) {
			if (config & PIN_I2C_CFG_CH_1ST_PINMUX) {
				if (top_reg7.bit.UART4 == UART_MUX_2) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with UART4_2\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 6) || (top_reg5.bit.IRDA == 7)) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with IRDA5/6\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C = MUX_1;
				pad_set_pull_updown_ep(PAD_PIN_PGPIO12, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_PGPIO13, PAD_NONE, ep_ch);
			} else {
				if (top_reg7.bit.UART_CTS_RTS == UART_MUX_3) {
					pr_err("PIN_I2C_CFG_CH_2ND_PINMUX conflict with UART3_CTSRTS\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 12) || (top_reg5.bit.IRDA == 13)) {
					pr_err("PIN_I2C_CFG_CH_2ND_PINMUX conflict with IRDA11/12\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C = MUX_2;
				pad_set_pull_updown_ep(PAD_PIN_DGPIO9, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_DGPIO10, PAD_NONE, ep_ch);
			}
		}
		if (config & (PIN_I2C_CFG_CH2_1ST_PINMUX)) {
			if (top_reg7.bit.UART5 == UART_MUX_2) {
				pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with UART5_2\r\n");
				return E_OBJ;
			}
			if ((top_reg5.bit.IRDA == 8) || (top_reg5.bit.IRDA == 9)) {
				pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with IRDA7/8\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.I2C2 = MUX_1;
			pad_set_pull_updown_ep(PAD_PIN_PGPIO19, PAD_NONE, ep_ch);
			pad_set_pull_updown_ep(PAD_PIN_PGPIO20, PAD_NONE, ep_ch);
		}
		if (config & (PIN_I2C_CFG_CH3_1ST_PINMUX | PIN_I2C_CFG_CH3_2ND_PINMUX | PIN_I2C_CFG_CH3_3RD_PINMUX | PIN_I2C_CFG_CH3_4TH_PINMUX)) {
			if (config & PIN_I2C_CFG_CH3_1ST_PINMUX) {
				if ((top_reg5.bit.IRDA == 10)) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with IRDA9\r\n");
					return E_OBJ;
				}
				top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_1;
				pad_set_pull_updown_ep(PAD_PIN_PGPIO33, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_PGPIO34, PAD_NONE, ep_ch);
			} else if (config & PIN_I2C_CFG_CH3_2ND_PINMUX) {
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with JTAG\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 1) || (top_reg5.bit.IRDA == 2)) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with IRDA0/1\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_MCLK == MUX_3) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with I2S3_3_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with I2S3_3\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_2;
				pad_set_pull_updown_ep(PAD_PIN_JGPIO0, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_JGPIO1, PAD_NONE, ep_ch);
			} else if (config & PIN_I2C_CFG_CH3_3RD_PINMUX) {
				if (top_reg7.bit.UART2_CTS_RTS == UART_MUX_1) {
					pr_err("PIN_I2C_CFG_CH3_3RD_PINMUX conflict with UART2_CTSRTS\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_3;
				pad_set_pull_updown_ep(PAD_PIN_PGPIO6, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_PGPIO7, PAD_NONE, ep_ch);
			} else {
				if (top_reg1.bit.SDIO2 == MUX_1) {
					pr_err("PIN_I2C_CFG_CH3_4TH_PINMUX conflict with SDIO2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART2_CTS_RTS == UART_MUX_2) {
					pr_err("PIN_I2C_CFG_CH3_4TH_PINMUX conflict with UART2_2_CTS/RTS\r\n");
					return E_OBJ;
				}
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_4;
				pad_set_pull_updown_ep(PAD_PIN_CGPIO10, PAD_NONE, ep_ch);
				pad_set_pull_updown_ep(PAD_PIN_CGPIO11, PAD_NONE, ep_ch);
			}

		}
		if (config & (PIN_I2C_CFG_HDMI_1ST_PINMUX)) {
			top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown_ep(PAD_PIN_PGPIO37, PAD_NONE, ep_ch);
			pad_set_pull_updown_ep(PAD_PIN_PGPIO38, PAD_NONE, ep_ch);
		}
	}

	return E_OK;
}

static ER pinmux_config_sdio(uint32_t config, u32 ep_ch)
{

	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & (PIN_SDIO_CFG_1ST_PINMUX | PIN_SDIO_CFG_2ND_PINMUX)) {
			if (config & PIN_SDIO_CFG_1ST_PINMUX) {
				if (top_reg4.bit.I2C == MUX_1) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2C\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == UART_MUX_2) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with UART4_2\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if ((top_reg11.bit.I2S == MUX_EN) || (top_reg11.bit.I2S_MCLK == MUX_EN) || (top_reg11.bit.I2S_TX == MUX_EN) || (top_reg11.bit.I2S_RX == MUX_EN)) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2S\r\n");
					return E_OBJ;
				}
				if ((top_reg11.bit.I2S3_TX == MUX_2) || (top_reg11.bit.I2S3_RX == MUX_2)) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2S3_2 RXTX\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART == UART_MUX_2) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with UART_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART_CTS_RTS == UART_MUX_2) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with UART_2_CTSRTS\r\n");
					return E_OBJ;
				}

				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO = MUX_1;
			} /*else {
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO = MUX_2;
			}*/
		}
		if (config & (PIN_SDIO2_CFG_1ST_PINMUX | PIN_SDIO2_CFG_BUS_WIDTH | PIN_SDIO2_CFG_DS)) {
			if (top_reg1.bit.SPI_EN == MUX_EN) {
				pr_err("PIN_SDIO2_CFG_1ST_PINMUX conflict with SPI\r\n");
				return E_OBJ;
			}
			if (config & (PIN_SDIO2_CFG_1ST_PINMUX)) {
				if (top_reg7.bit.UART2 == UART_MUX_2) {
					pr_err("PIN_SDIO2_CFG_1ST_PINMUX conflict with UART2_2\r\n");
					return E_OBJ;
				}
				if (top_reg4.bit.I2C3 == MUX_4) {
					pr_err("PIN_SDIO2_CFG_1ST_PINMUX conflict with I2C3_4\r\n");
					return E_OBJ;
				}
				top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO2 = MUX_1;
			}
			if (config & (PIN_SDIO2_CFG_BUS_WIDTH)) {
				top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO2_BUS_WIDTH = SDIO_BUS_WIDTH_8;
			}
			if (config & (PIN_SDIO2_CFG_DS)) {
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO2_DS = MUX_EN;
			}
		}
	}

	return E_OK;
}

static ER pinmux_config_spi(uint32_t config, u32 ep_ch)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if ((top_reg1.bit.SDIO2_BUS_WIDTH == SDIO_BUS_WIDTH_8) || (top_reg1.bit.SDIO2_DS == MUX_EN)) {
			pr_err("PIN_SPI_CFG_CH_1ST_PINMUX conflict with SDIO2/EMMC\r\n");
			return E_OBJ;
		}
		if (config & (PIN_SPI_CFG_CH_1ST_PINMUX)) {
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.SPI_EN = MUX_EN;
		}
		if (config & (PIN_SPI_CFG_CH_BUS_WIDTH)) {
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.SPI_BUS_WIDTH = SPI_BUS_WIDTH_4;
		}
		if (config & (PIN_SPI_CFG_CH_CS1)) {
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.SPI_CS1_EN = MUX_EN;
		}
	}

	return E_OK;
}

static ER pinmux_config_extclk(uint32_t config, u32 ep_ch)
{
	if (config == PIN_EXTCLK_CFG_NONE) {
	} else {
		if (config & (PIN_EXTCLK_CFG_CH_1ST_PINMUX)) {
			if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
				pr_err("PIN_EXTCLK_CFG_CH_1ST_PINMUX conflict with LCD310/LCD310L RGB888\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.EXT_CLK = MUX_EN;
		}
		if (config & (PIN_EXTCLK_CFG_CH2_1ST_PINMUX)) {
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			store_extclk2[record_ch] = 1;
		}
		if (config & (PIN_EXTCLK_CFG_CH3_1ST_PINMUX)) {
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
		}
		if (config & (PIN_EXTCLK_CFG_CH4_1ST_PINMUX)) {
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
		}
	}

	return E_OK;
}

static ER pinmux_config_ssp(uint32_t config, u32 ep_ch)
{
	if (config == PIN_SSP_CFG_NONE) {
	} else {
		//I2S
		if (config & (PIN_SSP_CFG_CH_1ST_PINMUX | PIN_SSP_CFG_CH_1ST_MCLK | PIN_SSP_CFG_CH_1ST_TX | PIN_SSP_CFG_CH_1ST_RX)) {
			if (config & PIN_SSP_CFG_CH_1ST_PINMUX) {
				if (top_reg7.bit.UART == UART_MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_PINMUX conflict with UART_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH_1ST_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH_1ST_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH_1ST_MCLK) {
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH_1ST_MCLK conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_RX == MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_MCLK conflict with I2S3_2_RX\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_MCLK = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH_1ST_TX) {
				if (top_reg11.bit.I2S3_TX == MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_TX conflict with I2S3_2_TX\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH_1ST_TX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH_1ST_TX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART_CTS_RTS == UART_MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_TX conflict with UART_2_CTSRTS\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_TX = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH_1ST_RX) {
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH_1ST_RX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH_1ST_RX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}

				if (top_reg7.bit.UART_CTS_RTS == UART_MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_RX conflict with UART_2_CTSRTS\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_RX = MUX_EN;
			}
		}
		//I2S2
		if (config & (PIN_SSP_CFG_CH2_1ST_PINMUX | PIN_SSP_CFG_CH2_1ST_MCLK | PIN_SSP_CFG_CH2_1ST_TX | PIN_SSP_CFG_CH2_1ST_RX)) {
			if (config & PIN_SSP_CFG_CH2_1ST_PINMUX) {
				top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2 = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH2_1ST_MCLK) {
				if (top_reg11.bit.I2S3 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH2_1ST_MCLK conflict with I2S3_2\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_MCLK = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH2_1ST_TX) {
				if (top_reg11.bit.I2S3 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH2_1ST_TX conflict with I2S3_2\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_TX = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH2_1ST_RX) {
				top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_RX = MUX_EN;
			}
		}
		//I2S3
		if (config & (PIN_SSP_CFG_CH3_1ST_PINMUX | PIN_SSP_CFG_CH3_2ND_PINMUX | PIN_SSP_CFG_CH3_3RD_PINMUX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_PINMUX) {
				if (top_reg5.bit.SATA2_LED == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with SATA2_3_LED\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA3_LED == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with SATA3_3_LED\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_1;
			} else if (config & PIN_SSP_CFG_CH3_2ND_PINMUX) {
				if (top_reg11.bit.I2S2_MCLK == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with I2S2_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S2_TX == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with I2S2_TX\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_2;
			} else {
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_3RD_PINMUX conflict with JTAG\r\n");
					return E_OBJ;
				}
				if (top_reg4.bit.I2C3 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_3RD_PINMUX conflict with I2C3_2\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.IRDA == 2) {
					pr_err("PIN_SSP_CFG_CH3_3RD_PINMUX conflict with IRDA1\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_3;
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_MCLK | PIN_SSP_CFG_CH3_2ND_MCLK | PIN_SSP_CFG_CH3_3RD_MCLK)) {
			if (config & PIN_SSP_CFG_CH3_1ST_MCLK) {
				if (top_reg5.bit.SATA_LED == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_1ST_MCLK conflict with SATA_3_LED\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_MCLK = MUX_1;
			} else if (config & PIN_SSP_CFG_CH3_2ND_MCLK) {
				if ((top_reg2.bit.LCD310 == MUX_2) || ((top_reg2.bit.LCD310L == MUX_2))) {
					pr_err("PIN_SSP_CFG_CH3_2ND_MCLK conflict with LCD310/LCD310L RGB888\r\n");
					return E_OBJ;
				}
				if (top_reg6.bit.PWM3 == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_2ND_MCLK conflict with PWM3\r\n");
					return E_OBJ;
				}
				top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_MCLK = MUX_2;
			} else {
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_3RD_MCLK conflict with JTAG\r\n");
					return E_OBJ;
				}
				if (top_reg4.bit.I2C3 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_3RD_MCLK conflict with I2C3_2\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.IRDA == 1) {
					pr_err("PIN_SSP_CFG_CH3_3RD_MCLK conflict with IRDA0\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_MCLK = MUX_3;
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_TX | PIN_SSP_CFG_CH3_2ND_TX | PIN_SSP_CFG_CH3_3RD_TX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_TX) {
				if (top_reg7.bit.UART == UART_MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_1ST_TX conflict with UART_3\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_TX = MUX_1;
			} else if (config & PIN_SSP_CFG_CH3_2ND_TX) {
				if (top_reg11.bit.I2S_TX == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_2ND_TX conflict with I2S_TX\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_2ND_TX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH3_2ND_TX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART_CTS_RTS == UART_MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_2ND_TX conflict with UART_2_CTSRTS\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_TX = MUX_2;
			} else {
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_3RD_TX conflict with JTAG\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART5 == UART_MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_3RD_TX conflict with UART5\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_TX = MUX_3;
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_RX | PIN_SSP_CFG_CH3_2ND_RX | PIN_SSP_CFG_CH3_3RD_RX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_RX) {
				top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_RX = MUX_1;
			} else if (config & PIN_SSP_CFG_CH3_2ND_RX) {
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH3_2ND_RX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S_MCLK == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_2ND_RX conflict with I2S_MCLK\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_RX = MUX_2;
			} else {
				if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH3_3RD_RX conflict with JTAG\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART5 == UART_MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_3RD_RX conflict with UART5\r\n");
					return E_OBJ;
				}
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_RX = MUX_3;
			}
		}
		//I2S4
		if (config & (PIN_SSP_CFG_CH4_1ST_PINMUX | PIN_SSP_CFG_CH4_1ST_MCLK | PIN_SSP_CFG_CH4_1ST_TX | PIN_SSP_CFG_CH4_1ST_RX)) {
			if (config & PIN_SSP_CFG_CH4_1ST_PINMUX) {
				if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
					pr_err("PIN_SSP_CFG_CH4_1ST_PINMUX conflict with LCD310/LCD310L\r\n");
					return E_OBJ;
				}
				top_reg_pgpio1.bit.PGPIO_40 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio1.bit.PGPIO_41 = GPIO_ID_EMUM_FUNC;
				top_reg12.bit.I2S4 = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH4_1ST_MCLK) {
				if (top_reg7.bit.UART3 == UART_MUX_1) {
					pr_err("PIN_SSP_CFG_CH4_1ST_MCLK conflict with UART3\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA2_LED == MUX_2) {
					pr_err("PIN_SSP_CFG_CH4_1ST_MCLK conflict with SATA2_2_LED\r\n");
					return E_OBJ;
				}
				if (top_reg12.bit.I2S4_RX == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH4_1ST_MCLK conflict with I2S4_RX\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg12.bit.I2S4_MCLK = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH4_1ST_TX) {
				if (top_reg5.bit.SATA_LED == MUX_2) {
					pr_err("PIN_SSP_CFG_CH4_1ST_TX conflict with SATA_2_LED\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART3 == UART_MUX_1) {
					pr_err("PIN_SSP_CFG_CH4_1ST_TX conflict with UART3\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg12.bit.I2S4_TX = MUX_EN;
			}
			if (config & PIN_SSP_CFG_CH4_1ST_RX) {
				if (top_reg7.bit.UART3 == UART_MUX_1) {
					pr_err("PIN_SSP_CFG_CH4_1ST_RX conflict with UART3\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA2_LED == MUX_2) {
					pr_err("PIN_SSP_CFG_CH4_1ST_RX conflict with SATA2_2_LED\r\n");
					return E_OBJ;
				}
				if (top_reg12.bit.I2S4_MCLK == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH4_1ST_RX conflict with I2S4_MCLK\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg12.bit.I2S4_RX = MUX_EN;
			}
		}
	}

	return E_OK;
}

static ER pinmux_config_lcd(uint32_t config, u32 ep_ch)
{
	if (config == PIN_LCD_CFG_NONE) {
	} else {
		top_reg_bgpio0.bit.BGPIO_0 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_1 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_2 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_3 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_4 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_5 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_6 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_7 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_8 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_9 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_10 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_11 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_12 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_13 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_14 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_15 = GPIO_ID_EMUM_FUNC;
		top_reg_bgpio0.bit.BGPIO_16 = GPIO_ID_EMUM_FUNC;
		if (config & (PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX | PIN_LCD_CFG_LCD310L_RGB888_1ST_PINMUX | PIN_LCD_CFG_LCD310_DE_PINMUX)) {
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with I2C\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART4 == UART_MUX_2) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with UART4_2\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_1) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with SDIO\r\n");
				return E_OBJ;
			}
			if ((top_reg11.bit.I2S == MUX_EN) || (top_reg11.bit.I2S_MCLK == MUX_EN) || (top_reg11.bit.I2S_TX == MUX_EN) || (top_reg11.bit.I2S_RX == MUX_EN)) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with I2S\r\n");
				return E_OBJ;
			}
			if ((top_reg11.bit.I2S3_TX == MUX_2) || (top_reg11.bit.I2S3_RX == MUX_2)) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with I2S3_2 RXTX\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART == UART_MUX_2) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with UART_2\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART_CTS_RTS == UART_MUX_2) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with UART_2_CTSRTS\r\n");
				return E_OBJ;
			}
			if (top_reg5.bit.EXT_CLK == MUX_EN) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with EXT_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg6.bit.PWM3 == MUX_EN) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with PWM3\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3_MCLK ==  MUX_2) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with I2S3_2_MCLK\r\n");
				return E_OBJ;
			}
			if (top_reg12.bit.I2S4 == MUX_EN) {
				pr_err("LCD_CFG_LCD310_RGB888 conflict with I2S4\r\n");
				return E_OBJ;
			}

			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_40 = GPIO_ID_EMUM_FUNC;
			if (config & PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX) {
				top_reg2.bit.LCD310 = MUX_2;
			} else {
				top_reg2.bit.LCD310L = MUX_2;
			}
			if (config & PIN_LCD_CFG_LCD310_DE_PINMUX) {
				if (top_reg12.bit.I2S4 == MUX_EN) {
					pr_err("PIN_LCD_CFG_LCD310_DE_PINMUX conflict with I2S4\r\n");
					return E_OBJ;
				}
				top_reg_pgpio1.bit.PGPIO_41 = GPIO_ID_EMUM_FUNC;
				top_reg2.bit.LCD310_DE = MUX_EN;
			}
		} else {
			if (config & PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX) {
				top_reg2.bit.LCD310 = MUX_1;
			} else if (config & PIN_LCD_CFG_LCD310L_BT1120_1ST_PINMUX) {
				top_reg2.bit.LCD310L = MUX_1;
			} else {
				top_reg2.bit.LCD210 = MUX_1;
			}
		}
	}

	return E_OK;
}

static ER pinmux_config_remote(uint32_t config, u32 ep_ch)
{
	if (config == PIN_REMOTE_CFG_NONE) {
	} else {
		if (config & PIN_REMOTE_CFG_1ST_PINMUX) {
			if (top_reg1.bit.EJTAG_EN == MUX_EN) {
				pr_err("PIN_REMOTE_CFG_1ST_PINMUX conflict with JTAG\r\n");
				return E_OBJ;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("PIN_REMOTE_CFG_1ST_PINMUX conflict with I2C3_2\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3_MCLK == MUX_3) {
				pr_err("PIN_REMOTE_CFG_1ST_PINMUX conflict with I2S3_3\r\n");
				return E_OBJ;
			}

			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 1;
		} else if (config & PIN_REMOTE_CFG_2ND_PINMUX) {
			if (top_reg1.bit.EJTAG_EN == MUX_EN) {
				pr_err("PIN_REMOTE_CFG_2ND_PINMUX conflict with JTAG\r\n");
				return E_OBJ;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("PIN_REMOTE_CFG_2ND_PINMUX conflict with I2C3_2\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3 == MUX_3) {
				pr_err("PIN_REMOTE_CFG_2ND_PINMUX conflict with I2S3_3\r\n");
				return E_OBJ;
			}

			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 2;
		} else if (config & PIN_REMOTE_CFG_3RD_PINMUX) {
			if (top_reg5.bit.SATA3_LED == MUX_1) {
				pr_err("PIN_REMOTE_CFG_3RD_PINMUX conflict with SATA3_LED\r\n");
				return E_OBJ;
			}
			top_reg_pgpio1.bit.PGPIO_44 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 3;
		} else if (config & PIN_REMOTE_CFG_4TH_PINMUX) {
			top_reg_sgpio2.bit.SGPIO_82 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 4;
		} else if (config & PIN_REMOTE_CFG_5TH_PINMUX) {
			top_reg_sgpio2.bit.SGPIO_83 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 5;
		} else if (config & PIN_REMOTE_CFG_6TH_PINMUX) {
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("PIN_REMOTE_CFG_6TH_PINMUX conflict with I2C\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART4 == UART_MUX_2) {
				pr_err("PIN_REMOTE_CFG_6TH_PINMUX conflict with UART4_2\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_1) {
				pr_err("PIN_REMOTE_CFG_6TH_PINMUX conflict with SDIO\r\n");
				return E_OBJ;
			}
			if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
				pr_err("PIN_REMOTE_CFG_6TH_PINMUX conflict with LCD310/LCD310L\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 6;
		} else if (config & PIN_REMOTE_CFG_7TH_PINMUX) {
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("PIN_REMOTE_CFG_7TH_PINMUX conflict with I2C\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART4 == UART_MUX_2) {
				pr_err("PIN_REMOTE_CFG_7TH_PINMUX conflict with UART4_2\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_1) {
				pr_err("PIN_REMOTE_CFG_7TH_PINMUX conflict with SDIO\r\n");
				return E_OBJ;
			}
			if ((top_reg2.bit.LCD310 == MUX_2) || (top_reg2.bit.LCD310L == MUX_2)) {
				pr_err("PIN_REMOTE_CFG_7TH_PINMUX conflict with LCD310/LCD310L\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 7;
		} else if (config & PIN_REMOTE_CFG_8TH_PINMUX) {
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("PIN_REMOTE_CFG_8TH_PINMUX conflict with I2C2\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART5 == UART_MUX_2) {
				pr_err("PIN_REMOTE_CFG_8TH_PINMUX conflict with UART5_2\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 8;
		} else if (config & PIN_REMOTE_CFG_9TH_PINMUX) {
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("PIN_REMOTE_CFG_9TH_PINMUX conflict with I2C2\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART5 == UART_MUX_2) {
				pr_err("PIN_REMOTE_CFG_9TH_PINMUX conflict with UART5_2\r\n");
				return E_OBJ;
			}
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 9;
		} else if (config & PIN_REMOTE_CFG_10TH_PINMUX) {
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("PIN_REMOTE_CFG_9TH_PINMUX conflict with I2C3\r\n");
				return E_OBJ;
			}
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 10;
		} else if (config & PIN_REMOTE_CFG_11TH_PINMUX) {
			if (top_reg1.bit.EXTROM_EN == MUX_EN) {
				pr_err("PIN_REMOTE_CFG_11TH_PINMUX conflict with BMC\r\n");
				return E_OBJ;
			}
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 11;
		} else if (config & PIN_REMOTE_CFG_12TH_PINMUX) {
			if (top_reg4.bit.I2C == MUX_2) {
				pr_err("PIN_REMOTE_CFG_12TH_PINMUX conflict with I2C_2\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART_CTS_RTS == UART_MUX_3) {
				pr_err("PIN_REMOTE_CFG_12TH_PINMUX conflict with UART_3_CTSRTS\r\n");
				return E_OBJ;
			}
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 12;
		} else {
			if (top_reg4.bit.I2C == MUX_2) {
				pr_err("PIN_REMOTE_CFG_13TH_PINMUX conflict with I2C_2\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART_CTS_RTS == UART_MUX_3) {
				pr_err("PIN_REMOTE_CFG_13TH_PINMUX conflict with UART_3_CTSRTS\r\n");
				return E_OBJ;
			}
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 13;
		}
	}

	return E_OK;
}

static ER pinmux_config_vcap(uint32_t config, u32 ep_ch)
{
	if (config == PIN_VCAP_CFG_NONE) {
	} else {
		//CAP0
		if (config & (PIN_VCAP_CFG_CAP0_1ST_PINMUX | PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX)) {
			if ((top_reg3.bit.ETH == MUX_3) || (top_reg3.bit.ETH == MUX_4)) {
				pr_err("PIN_VCAP_CFG_CAP0 conflict with ETH_2_XXX\r\n");
				return E_OBJ;
			}
			if (config & PIN_VCAP_CFG_CAP0_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP_CLK = MUX_1;
			} else {
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP_CLK = MUX_2;
			}
		}
		//CAP1
		if (config & (PIN_VCAP_CFG_CAP1_1ST_PINMUX | PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX)) {
			if ((top_reg3.bit.ETH == MUX_3) || (top_reg3.bit.ETH == MUX_4)) {
				pr_err("PIN_VCAP_CFG_CAP1 conflict with ETH_2_XXX\r\n");
				return E_OBJ;
			}
			if (config & PIN_VCAP_CFG_CAP1_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP1 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP1_CLK = MUX_1;
			} else {
				top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP1_CLK = MUX_2;
			}
		}
		//CAP2
		if (config & (PIN_VCAP_CFG_CAP2_1ST_PINMUX | PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX)) {
			if ((top_reg3.bit.ETH2 == MUX_3) || (top_reg3.bit.ETH2 == MUX_4)) {
				pr_err("PIN_VCAP_CFG_CAP2 conflict with ETH2_2_XXX\r\n");
				return E_OBJ;
			}
			if (config & PIN_VCAP_CFG_CAP2_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_26 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP2 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP2_CLK = MUX_1;
			} else {
				top_reg_sgpio0.bit.SGPIO_27 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP2_CLK = MUX_2;
			}
		}
		//CAP3
		if (config & (PIN_VCAP_CFG_CAP3_1ST_PINMUX | PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX)) {
			if ((top_reg3.bit.ETH2 == MUX_3) || (top_reg3.bit.ETH2 == MUX_4)) {
				pr_err("PIN_VCAP_CFG_CAP3 conflict with ETH2_2_XXX\r\n");
				return E_OBJ;
			}
			if (config & PIN_VCAP_CFG_CAP3_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_28 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_29 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_30 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio0.bit.SGPIO_31 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio1.bit.SGPIO_32 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio1.bit.SGPIO_33 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio1.bit.SGPIO_34 = GPIO_ID_EMUM_GPIO;
				top_reg_sgpio1.bit.SGPIO_35 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP3 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_27 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP3_CLK = MUX_1;
			} else {
				top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_GPIO;
				top_reg9.bit.CAP3_CLK = MUX_2;
			}
		}
		//CAP4
		if (config & (PIN_VCAP_CFG_CAP4_1ST_PINMUX | PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP4_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP4_1ST_PINMUX) {
				top_reg_sgpio1.bit.SGPIO_37 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_38 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_39 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_40 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_41 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_42 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_43 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_44 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP4 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX) {
				top_reg_sgpio1.bit.SGPIO_45 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP4_CLK = MUX_1;
			} else {
				top_reg_sgpio1.bit.SGPIO_36 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP4_CLK = MUX_2;
			}
		}
		//CAP5
		if (config & (PIN_VCAP_CFG_CAP5_1ST_PINMUX | PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP5_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP5_1ST_PINMUX) {
				top_reg_sgpio1.bit.SGPIO_49 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_50 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_51 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_52 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_53 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_54 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_55 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_56 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP5 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX) {
				top_reg_sgpio1.bit.SGPIO_57 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP5_CLK = MUX_1;
			} else {
				top_reg_sgpio1.bit.SGPIO_48 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP5_CLK = MUX_2;
			}
		}
		//CAP6
		if (config & (PIN_VCAP_CFG_CAP6_1ST_PINMUX | PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP6_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP6_1ST_PINMUX) {
				top_reg_sgpio1.bit.SGPIO_61 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_62 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_63 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_64 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_65 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_66 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_67 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_68 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP6 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX) {
				top_reg_sgpio2.bit.SGPIO_69 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP6_CLK = MUX_1;
			} else {
				top_reg_sgpio1.bit.SGPIO_60 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP6_CLK = MUX_2;
			}
		}
		//CAP7
		if (config & (PIN_VCAP_CFG_CAP7_1ST_PINMUX | PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP7_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP7_1ST_PINMUX) {
				top_reg_sgpio2.bit.SGPIO_73 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_74 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_75 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_76 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_77 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_78 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_79 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio2.bit.SGPIO_80 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP7 = MUX_EN;
			}
			if (config & PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX) {
				top_reg_sgpio2.bit.SGPIO_81 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP7_CLK = MUX_1;
			} else {
				top_reg_sgpio2.bit.SGPIO_72 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP7_CLK = MUX_2;
			}
		}

	}

	return E_OK;
}

static ER pinmux_config_eth(uint32_t config, u32 ep_ch)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & (PIN_ETH_CFG_RGMII_1ST_PINMUX | PIN_ETH_CFG_RGMII_2ND_PINMUX | PIN_ETH_CFG_RMII_1ST_PINMUX | PIN_ETH_CFG_RMII_2ND_PINMUX)) {
			if (config & PIN_ETH_CFG_RGMII_1ST_PINMUX) {
				top_reg_egpio0.bit.EGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH = MUX_1;
			} else if (config & PIN_ETH_CFG_RGMII_2ND_PINMUX) {
				if ((top_reg9.bit.CAP == MUX_EN) || (top_reg9.bit.CAP1 == MUX_EN)) {
					pr_err("PIN_ETH_CFG_RGMII_2ND_PINMUX conflict with CAP0/1\r\n");
					return E_OBJ;
				}
				if ((top_reg9.bit.CAP_CLK == MUX_2) || (top_reg9.bit.CAP1_CLK == MUX_1)) {
					pr_err("PIN_ETH_CFG_RGMII_2ND_PINMUX conflict with CAP_2_CLK/CAP1_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH = MUX_3;
			} else if (config & PIN_ETH_CFG_RMII_1ST_PINMUX) {
				top_reg_egpio0.bit.EGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH = MUX_2;
			} else {

				if ((top_reg9.bit.CAP == MUX_EN) || (top_reg9.bit.CAP1 == MUX_EN)) {
					pr_err("PIN_ETH_CFG_RMII_2ND_PINMUX conflict with CAP0/1\r\n");
					return E_OBJ;
				}
				if ((top_reg9.bit.CAP_CLK == MUX_2) || (top_reg9.bit.CAP1_CLK == MUX_1)) {
					pr_err("PIN_ETH_CFG_RMII_2ND_PINMUX conflict with CAP_2_CLK/CAP1_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH = MUX_4;
			}
		}
		if (config & PIN_ETH_CFG_REFCLK_PINMUX) {
			if (config & (PIN_ETH_CFG_RGMII_1ST_PINMUX | PIN_ETH_CFG_RMII_1ST_PINMUX)) {
				top_reg_egpio0.bit.EGPIO_0 = GPIO_ID_EMUM_FUNC;
			} else {
				if ((top_reg9.bit.CAP_CLK == MUX_1)) {
					pr_err("PIN_ETH_CFG_REFCLK_PINMUX conflict with CAP_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
			}
			top_reg3.bit.ETH_REFCLK = MUX_EN;
		}
		if (config & PIN_ETH_CFG_RST_PINMUX) {
			pr_warn("PIN_ETH_CFG_RST_PINMUX is obsolete, skip it...\r\n");
		}
		if (config & PIN_ETH_CFG_MDC_MDIO_PINMUX) {
			if (config & (PIN_ETH_CFG_RGMII_1ST_PINMUX | PIN_ETH_CFG_RMII_1ST_PINMUX)) {
				top_reg_egpio0.bit.EGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH_MDC_MDIO = MUX_1;
			} else {
				if ((top_reg9.bit.CAP1 == MUX_EN)) {
					pr_err("PIN_ETH_CFG_MDC_MDIO_PINMUX conflict with CAP1\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH_MDC_MDIO = MUX_2;
			}
		}

		//ETH2
		if (config & (PIN_ETH2_CFG_RGMII_1ST_PINMUX | PIN_ETH2_CFG_RGMII_2ND_PINMUX | PIN_ETH2_CFG_RMII_1ST_PINMUX | PIN_ETH2_CFG_RMII_2ND_PINMUX)) {
			if (config & PIN_ETH2_CFG_RGMII_1ST_PINMUX) {
				top_reg_egpio0.bit.EGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_24 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2 = MUX_1;
			} else if (config & PIN_ETH2_CFG_RGMII_2ND_PINMUX) {
				if ((top_reg9.bit.CAP2 == MUX_EN) || (top_reg9.bit.CAP3 == MUX_EN)) {
					pr_err("PIN_ETH2_CFG_RGMII_2ND_PINMUX conflict with CAP2/3\r\n");
					return E_OBJ;
				}
				if ((top_reg9.bit.CAP2_CLK == MUX_2) || (top_reg9.bit.CAP3_CLK == MUX_1)) {
					pr_err("PIN_ETH2_CFG_RGMII_2ND_PINMUX conflict with CAP2_2_CLK/CAP3_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2 = MUX_3;
			} else if (config & PIN_ETH2_CFG_RMII_1ST_PINMUX) {
				top_reg_egpio0.bit.EGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2 = MUX_2;
			} else {
				if ((top_reg9.bit.CAP2 == MUX_EN) || (top_reg9.bit.CAP3 == MUX_EN)) {
					pr_err("PIN_ETH2_CFG_RMII_2ND_PINMUX conflict with CAP2/3\r\n");
					return E_OBJ;
				}
				if ((top_reg9.bit.CAP2_CLK == MUX_2) || (top_reg9.bit.CAP3_CLK == MUX_1)) {
					pr_err("PIN_ETH2_CFG_RMII_2ND_PINMUX conflict with CAP2_2_CLK/CAP3_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2 = MUX_4;
			}
		}
		if (config & PIN_ETH2_CFG_REFCLK_PINMUX) {
			if (config & (PIN_ETH2_CFG_RGMII_1ST_PINMUX | PIN_ETH2_CFG_RMII_1ST_PINMUX)) {
				top_reg_egpio0.bit.EGPIO_16 = GPIO_ID_EMUM_FUNC;
			} else {
				if ((top_reg9.bit.CAP_CLK == MUX_2) || (top_reg9.bit.CAP2_CLK == MUX_1)) {
					pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with CAP_2_CLK/CAP2_CLK\r\n");
					return E_OBJ;
				}
				top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			}
			top_reg3.bit.ETH2_REFCLK = MUX_EN;
		}
		if (config & PIN_ETH2_CFG_RST_PINMUX) {
			pr_warn("PIN_ETH2_CFG_RST_PINMUX is obsolete, skip it...\r\n");
		}
		if (config & PIN_ETH2_CFG_MDC_MDIO_PINMUX) {
			if (config & (PIN_ETH2_CFG_RGMII_1ST_PINMUX | PIN_ETH2_CFG_RMII_1ST_PINMUX)) {
				top_reg_egpio0.bit.EGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2_MDC_MDIO = MUX_1;
			} else {
				if ((top_reg9.bit.CAP3 == MUX_EN)) {
					pr_err("PIN_ETH2_CFG_MDC_MDIO_PINMUX conflict with CAP3\r\n");
					return E_OBJ;
				}
				top_reg_sgpio1.bit.SGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2_MDC_MDIO = MUX_2;
			}
		}

	}

	return E_OK;
}

static ER pinmux_config_misc(uint32_t config, u32 ep_ch)
{
	if (config == PIN_ETH_CFG_NONE) {
		if ((config & PIN_MISC_CFG_CPU_ICE) == 0x0) {
			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_GPIO;
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_GPIO;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_GPIO;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_GPIO;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_GPIO;
			top_reg1.bit.EJTAG_EN = MUX_DIS;
		}
	} else {
		if (config & PIN_MISC_CFG_CPU_ICE) {
			if (top_reg7.bit.UART5 == UART_MUX_1) {
				pr_err("PIN_MISC_CFG_CPU_ICE conflict with UART5\r\n");
				return E_OBJ;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("PIN_MISC_CFG_CPU_ICE conflict with I2C3_2\r\n");
				return E_OBJ;
			}
			if ((top_reg5.bit.IRDA == 1) || (top_reg5.bit.IRDA == 2)) {
				pr_err("PIN_MISC_CFG_CPU_ICE conflict with IRDA0/1\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3_MCLK == MUX_3) {
				pr_err("PIN_MISC_CFG_CPU_ICE conflict with I2S3_3_MCLK\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3 == MUX_3) {
				pr_err("PIN_MISC_CFG_CPU_ICE conflict with I2S3_3\r\n");
				return E_OBJ;
			}
			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.EJTAG_EN = MUX_EN;
		} else {
			if ((top_reg7.bit.UART5 != UART_MUX_1) && (top_reg4.bit.I2C3 != MUX_2) && (top_reg11.bit.I2S3_MCLK != MUX_3) && (top_reg11.bit.I2S3 != MUX_3)) {
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_GPIO;
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_GPIO;
				top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_GPIO;
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_GPIO;
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_GPIO;
			}
			top_reg1.bit.EJTAG_EN = MUX_DIS;
		}
		if (config & PIN_MISC_CFG_BMC) {
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.EXTROM_EN = MUX_EN;
		}
		if (config & PIN_MISC_CFG_RTC_CAL_OUT) {
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.RTC_CAL = MUX_EN;
		}
		if (config & PIN_MISC_CFG_DAC_RAMP_TP) {
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.TVDAC_TEST_CLK = MUX_EN;
		}
		if (config & PIN_MISC_CFG_HDMI_HOTPLUG) {
			top_reg_pgpio1.bit.PGPIO_39 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MISC_CFG_12M_CLK) {
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.CLK_12M = MUX_EN;
		}
		if (config & PIN_MISC_CFG_VGA_HS) {
			top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MISC_CFG_VGA_VS) {
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
		}
		if (config & (PIN_MISC_CFG_SATA_LED_1ST_PINMUX | PIN_MISC_CFG_SATA_LED_2ND_PINMUX | PIN_MISC_CFG_SATA_LED_3RD_PINMUX)) {
			if (config & PIN_MISC_CFG_SATA_LED_1ST_PINMUX) {
				top_reg_pgpio1.bit.PGPIO_42 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA_LED = MUX_1;
			} else if (config & PIN_MISC_CFG_SATA_LED_2ND_PINMUX) {
				if (top_reg12.bit.I2S4_TX == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA_LED_2ND_PINMUX conflict with I2S4_TX\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART3 == UART_MUX_1) {
					pr_err("PIN_MISC_CFG_SATA_LED_2ND_PINMUX conflict with UART3\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA_LED = MUX_2;
			} else {
				if (top_reg11.bit.I2S3_MCLK == MUX_1) {
					pr_err("PIN_MISC_CFG_SATA_LED_3RD_PINMUX conflict with I2S3_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.EXTROM_EN == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA_LED_3RD_PINMUX conflict with EXTROM_EN\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA_LED = MUX_3;
			}
		}
		if (config & (PIN_MISC_CFG_SATA2_LED_1ST_PINMUX | PIN_MISC_CFG_SATA2_LED_2ND_PINMUX | PIN_MISC_CFG_SATA2_LED_3RD_PINMUX)) {
			if (config & PIN_MISC_CFG_SATA2_LED_1ST_PINMUX) {
				top_reg_pgpio1.bit.PGPIO_43 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA2_LED = MUX_1;
			} else if (config & PIN_MISC_CFG_SATA2_LED_2ND_PINMUX) {
				if (top_reg7.bit.UART3 == UART_MUX_1) {
					pr_err("PIN_MISC_CFG_SATA2_LED_2ND_PINMUX conflict with UART3\r\n");
					return E_OBJ;
				}
				if (top_reg12.bit.I2S4_MCLK == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA2_LED_2ND_PINMUX conflict with I2S4_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg12.bit.I2S4_RX == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA2_LED_2ND_PINMUX conflict with I2S4_RX\r\n");
					return E_OBJ;
				}
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA2_LED = MUX_2;
			} else {
				if (top_reg11.bit.I2S3 == MUX_1) {
					pr_err("PIN_MISC_CFG_SATA2_LED_3RD_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA2_LED = MUX_3;
			}
		}
		if (config & (PIN_MISC_CFG_SATA3_LED_1ST_PINMUX | PIN_MISC_CFG_SATA3_LED_2ND_PINMUX | PIN_MISC_CFG_SATA3_LED_3RD_PINMUX)) {
			if (config & PIN_MISC_CFG_SATA3_LED_1ST_PINMUX) {
				if (top_reg5.bit.IRDA == 3) {
					pr_err("PIN_MISC_CFG_SATA3_LED_1ST_PINMUX conflict with IRDA2\r\n");
					return E_OBJ;
				}
				top_reg_pgpio1.bit.PGPIO_44 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA3_LED = MUX_1;
			} else if (config & PIN_MISC_CFG_SATA3_LED_2ND_PINMUX) {
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA3_LED = MUX_2;
			} else {
				if (top_reg11.bit.I2S3 == MUX_1) {
					pr_err("PIN_MISC_CFG_SATA3_LED_3RD_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA3_LED = MUX_3;
			}
		}

	}

	return E_OK;
}

static ER pinmux_config_pwm(uint32_t config, u32 ep_ch)
{
	if (config == PIN_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PWM_CFG_CH_1ST_PINMUX) {
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM = MUX_EN;
		}
		if (config & PIN_PWM_CFG_CH2_1ST_PINMUX) {
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM2 = MUX_EN;
		}
		if (config & PIN_PWM_CFG_CH3_1ST_PINMUX) {
			if (top_reg11.bit.I2S3_MCLK ==  MUX_2) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with I2S3_2_MCLK\r\n");
				return E_OBJ;
			}
			if ((top_reg2.bit.LCD310 == MUX_2) || ((top_reg2.bit.LCD310L == MUX_2))) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with LCD310/LCD310L RGB888\r\n");
				return E_OBJ;
			}
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM3 = MUX_EN;
		}
		if (config & PIN_PWM_CFG_CH4_1ST_PINMUX) {
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM4 = MUX_EN;
		}
	}

	return E_OK;
}

static ER pinmux_config_mipi(uint32_t config, u32 ep_ch)
{
	if (config == PIN_MIPI_CFG_NONE) {
	} else {
		//PHY 0
		if (config & (PIN_MIPI_CFG_PHY_DAT0 | PIN_MIPI_CFG_PHY_DAT1 | PIN_MIPI_CFG_PHY_DAT2 | PIN_MIPI_CFG_PHY_DAT3 | PIN_MIPI_CFG_PHY_CLK0 | PIN_MIPI_CFG_PHY_CLK1)) {
			top_reg_sgpio1.bit.SGPIO_36 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_37 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_38 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_39 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_40 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_41 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_42 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_43 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_44 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_45 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_46 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_47 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_SGPIO36, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO37, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO38, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO39, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO40, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO41, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO42, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO43, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO44, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO45, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO46, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO47, PAD_NONE);
		}
		if (config & PIN_MIPI_CFG_PHY_DAT0) {
			if (top_reg9.bit.CAP4 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY_DAT0 conflict with CAP4\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP4_CLK == MUX_2) {
				pr_err("PIN_MIPI_CFG_PHY_DAT0 conflict with CAP4_2_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_36 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_37 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY_DAT1) {
			if (top_reg9.bit.CAP4 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY_DAT1 conflict with CAP4\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_38 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_39 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY_CLK0) {
			if (top_reg9.bit.CAP4 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY_CLK0 conflict with CAP4\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_40 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_41 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY_DAT2) {
			if (top_reg9.bit.CAP4 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY_DAT2 conflict with CAP4\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_42 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_43 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY_DAT3) {
			if (top_reg9.bit.CAP4 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY_DAT3 conflict with CAP4\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP4_CLK == MUX_1) {
				pr_err("PIN_MIPI_CFG_PHY_DAT3 conflict with CAP4_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_44 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_45 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY_CLK1) {
			//top_reg_sgpio1.bit.SGPIO_46 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_47 = GPIO_ID_EMUM_FUNC;
		}

		//PHY 2
		if (config & (PIN_MIPI_CFG_PHY2_DAT0 | PIN_MIPI_CFG_PHY2_DAT1 | PIN_MIPI_CFG_PHY2_DAT2 | PIN_MIPI_CFG_PHY2_DAT3 | PIN_MIPI_CFG_PHY2_CLK0 | PIN_MIPI_CFG_PHY2_CLK1)) {
			top_reg_sgpio1.bit.SGPIO_48 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_49 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_50 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_51 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_52 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_53 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_54 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_55 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_56 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_57 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_58 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_59 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_SGPIO48, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO49, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO50, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO51, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO52, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO53, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO54, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO55, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO56, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO57, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO58, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO59, PAD_NONE);
		}

		if (config & PIN_MIPI_CFG_PHY2_DAT0) {
			if (top_reg9.bit.CAP5 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT0 conflict with CAP5\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP5_CLK == MUX_2) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT0 conflict with CAP5_2_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_48 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_49 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY2_DAT1) {
			if (top_reg9.bit.CAP5 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT1 conflict with CAP5\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_50 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_51 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY2_CLK0) {
			if (top_reg9.bit.CAP5 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY2_CLK0 conflict with CAP5\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_52 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_53 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY2_DAT2) {
			if (top_reg9.bit.CAP5 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT2 conflict with CAP5\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_54 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_55 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY2_DAT3) {
			if (top_reg9.bit.CAP5 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT3 conflict with CAP5\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP5_CLK == MUX_1) {
				pr_err("PIN_MIPI_CFG_PHY2_DAT3 conflict with CAP5_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_56 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_57 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY2_CLK1) {
			//top_reg_sgpio1.bit.SGPIO_58 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_59 = GPIO_ID_EMUM_FUNC;
		}

		//PHY 3
		if (config & (PIN_MIPI_CFG_PHY3_DAT0 | PIN_MIPI_CFG_PHY3_DAT1 | PIN_MIPI_CFG_PHY3_DAT2 | PIN_MIPI_CFG_PHY3_DAT3 | PIN_MIPI_CFG_PHY3_CLK0 | PIN_MIPI_CFG_PHY3_CLK1)) {
			top_reg_sgpio1.bit.SGPIO_60 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_61 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_62 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio1.bit.SGPIO_63 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_64 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_65 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_66 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_67 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_68 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_69 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_70 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_71 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_SGPIO60, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO61, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO62, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO63, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO64, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO65, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO66, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO67, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO68, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO69, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO70, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO71, PAD_NONE);
		}

		if (config & PIN_MIPI_CFG_PHY3_DAT0) {
			if (top_reg9.bit.CAP6 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT0 conflict with CAP6\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP6_CLK == MUX_2) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT0 conflict with CAP6_2_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_60 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_61 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY3_DAT1) {
			if (top_reg9.bit.CAP6 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT1 conflict with CAP6\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio1.bit.SGPIO_62 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio1.bit.SGPIO_63 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY3_CLK0) {
			if (top_reg9.bit.CAP6 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY3_CLK0 conflict with CAP6\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_64 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_65 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY3_DAT2) {
			if (top_reg9.bit.CAP6 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT2 conflict with CAP6\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_66 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_67 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY3_DAT3) {
			if (top_reg9.bit.CAP6 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT3 conflict with CAP6\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP6_CLK == MUX_1) {
				pr_err("PIN_MIPI_CFG_PHY3_DAT3 conflict with CAP6_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_68 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_69 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY3_CLK1) {
			//top_reg_sgpio2.bit.SGPIO_70 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_71 = GPIO_ID_EMUM_FUNC;
		}

		//PHY 4
		if (config & (PIN_MIPI_CFG_PHY4_DAT0 | PIN_MIPI_CFG_PHY4_DAT1 | PIN_MIPI_CFG_PHY4_DAT2 | PIN_MIPI_CFG_PHY4_DAT3 | PIN_MIPI_CFG_PHY4_CLK0 | PIN_MIPI_CFG_PHY4_CLK1)) {
			top_reg_sgpio2.bit.SGPIO_72 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_73 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_74 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_75 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_76 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_77 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_78 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_79 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_80 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_81 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_82 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio2.bit.SGPIO_83 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_SGPIO72, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO73, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO74, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO75, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO76, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO77, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO78, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO79, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO80, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO81, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO82, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO83, PAD_NONE);
		}

		if (config & PIN_MIPI_CFG_PHY4_DAT0) {
			if (top_reg9.bit.CAP7 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT0 conflict with CAP7\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP7_CLK == MUX_2) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT0 conflict with CAP7_2_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_72 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_73 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY4_DAT1) {
			if (top_reg9.bit.CAP7 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT1 conflict with CAP7\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_74 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_75 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY4_CLK0) {
			if (top_reg9.bit.CAP7 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY4_CLK0 conflict with CAP7\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_76 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_77 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY4_DAT2) {
			if (top_reg9.bit.CAP7 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT2 conflict with CAP7\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_78 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_79 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY4_DAT3) {
			if (top_reg9.bit.CAP7 == MUX_EN) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT3 conflict with CAP7\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP7_CLK == MUX_1) {
				pr_err("PIN_MIPI_CFG_PHY4_DAT3 conflict with CAP7_CLK\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_80 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_81 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_MIPI_CFG_PHY4_CLK1) {
			if ((top_reg5.bit.IRDA == 4) || (top_reg5.bit.IRDA == 5)) {
				pr_err("PIN_MIPI_CFG_PHY4_CLK1 conflict with IRDA[3][4]\r\n");
				return E_OBJ;
			}
			//top_reg_sgpio2.bit.SGPIO_82 = GPIO_ID_EMUM_FUNC;
			//top_reg_sgpio2.bit.SGPIO_83 = GPIO_ID_EMUM_FUNC;
		}


	}

	return E_OK;
}

ER pinmux_parsing_i2c_ep(uint32_t config, u32 ep_ch)
{
	int ret = 0;
	PAD_PULL pad_pull;
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & (PIN_I2C_CFG_CH_1ST_PINMUX | PIN_I2C_CFG_CH_2ND_PINMUX)) {
			if (config & PIN_I2C_CFG_CH_1ST_PINMUX) {
				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO12, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C 1st FUNC_EN && PGPIO12 pull up => force PGPIO12 pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO12, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PAD_PIN_PGPIO12 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO12, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO13, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C 1st FUNC_EN && PGPIO13 pull up => force PGPIO13 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO13, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PAD_PIN_PGPIO13 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO13, PAD_NONE, ep_ch);
				}
			} else {
				if (pad_get_pull_updown_ep(PAD_PIN_DGPIO9, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C 2ND FUNC_EN && DGPIO9 pull up => force DGPIO9 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_DGPIO9, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PAD_PIN_DGPIO9 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_DGPIO9, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_DGPIO10, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C 2ND FUNC_EN && DGPIO10 pull up => force DGPIO10 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_DGPIO10, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get DGPIO10 Fail, force pull none\n");
					pad_set_pull_updown_ep(PAD_PIN_DGPIO10, PAD_NONE, ep_ch);
				}
			}
		}
		if (config & (PIN_I2C_CFG_CH2_1ST_PINMUX)) {
			if (pad_get_pull_updown_ep(PAD_PIN_PGPIO19, &pad_pull, ep_ch) == E_OK) {
				if (pad_pull != PAD_NONE) {
					ret++;
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("EP[%d]I2C2 1st FUNC_EN && PGPIO19 pull up => force PGPIO19 = pull none!!!\n", ep_ch);
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO19, PAD_NONE, ep_ch);
				}
			} else {
				ret++;
				pr_err("Get PGPIO19 Fail, force pull none\n");
				pad_set_pull_updown_ep(PAD_PIN_PGPIO19, PAD_NONE, ep_ch);
			}

			if (pad_get_pull_updown_ep(PAD_PIN_PGPIO20, &pad_pull, ep_ch) == E_OK) {
				if (pad_pull != PAD_NONE) {
					ret++;
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("EP[%d]I2C2 1st FUNC_EN && PGPIO20 pull up => force PGPIO20 = pull none!!!\n", ep_ch);
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO20, PAD_NONE, ep_ch);
				}
			} else {
				ret++;
				pr_err("Get PGPIO20 Fail, force pull none\r\n");
				pad_set_pull_updown_ep(PAD_PIN_PGPIO20, PAD_NONE, ep_ch);
			}
		}
		if (config & (PIN_I2C_CFG_CH3_1ST_PINMUX | PIN_I2C_CFG_CH3_2ND_PINMUX | PIN_I2C_CFG_CH3_3RD_PINMUX | PIN_I2C_CFG_CH3_4TH_PINMUX)) {
			if (config & PIN_I2C_CFG_CH3_1ST_PINMUX) {
				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO33, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 1st FUNC_EN && PGPIO33 pull up => force PGPIO33 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO33, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PGPIO33 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO33, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO34, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 1st FUNC_EN && PGPIO34 pull up => force PGPIO34 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO34, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PGPIO34 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO34, PAD_NONE, ep_ch);
				}
			} else if (config & PIN_I2C_CFG_CH3_2ND_PINMUX) {
				if (pad_get_pull_updown_ep(PAD_PIN_JGPIO0, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 2nd FUNC_EN && JGPIO0 pull up => force JGPIO0 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_JGPIO0, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get JGPIO0 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_JGPIO0, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_JGPIO1, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 2nd FUNC_EN && JGPIO1 pull up => force JGPIO1 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_JGPIO1, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get JGPIO1 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_JGPIO1, PAD_NONE, ep_ch);
				}
			} else if (config & PIN_I2C_CFG_CH3_3RD_PINMUX) {
				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO6, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 3rd FUNC_EN && PGPIO6 pull up => force PGPIO6 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO6, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PGPIO6 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO6, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_PGPIO7, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 3rd FUNC_EN && PGPIO7 pull up => force PGPIO7 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_PGPIO7, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get PGPIO7 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO7, PAD_NONE, ep_ch);
				}
			} else {
				if (pad_get_pull_updown_ep(PAD_PIN_CGPIO10, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 4th FUNC_EN && CGPIO10 pull up => force CGPIO10 = pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_CGPIO10, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get CGPIO10 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_CGPIO10, PAD_NONE, ep_ch);
				}

				if (pad_get_pull_updown_ep(PAD_PIN_CGPIO11, &pad_pull, ep_ch) == E_OK) {
					if (pad_pull != PAD_NONE) {
						ret++;
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("EP[%d]I2C3 4th FUNC_EN && CGPIO11 pull up => force CGPIO11 pull none!!!\n", ep_ch);
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown_ep(PAD_PIN_CGPIO11, PAD_NONE, ep_ch);
					}
				} else {
					ret++;
					pr_err("Get CGPIO11 Fail, force pull none\r\n");
					pad_set_pull_updown_ep(PAD_PIN_CGPIO11, PAD_NONE, ep_ch);
				}
			}
		}
		if (config & (PIN_I2C_CFG_HDMI_1ST_PINMUX)) {
			if (pad_get_pull_updown_ep(PAD_PIN_PGPIO37, &pad_pull, ep_ch) == E_OK) {
				if (pad_pull != PAD_NONE) {
					ret++;
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("EP[%d]I2C hdmi 1st FUNC_EN && PGPIO37 pull up => force PGPIO37 pull none!!!\n", ep_ch);
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO37, PAD_NONE, ep_ch);
				}
			} else {
				ret++;
				pr_err("Get PAD_PIN_PGPIO37 Fail, force PAD_NONE\r\n");
				pad_set_pull_updown_ep(PAD_PIN_PGPIO37, PAD_NONE, ep_ch);
			}

			if (pad_get_pull_updown_ep(PAD_PIN_PGPIO38, &pad_pull, ep_ch) == E_OK) {
				if (pad_pull != PAD_NONE) {
					ret++;
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("EP[%d]I2C hdmi 1st FUNC_EN && PGPIO38 pull up => force PGPIO38 pull none!!!\n", ep_ch);
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown_ep(PAD_PIN_PGPIO38, PAD_NONE, ep_ch);
				}
			} else {
				ret++;
				pr_err("Get PAD_PIN_PGPIO38 Fail, force PAD_NONE\r\n");
				pad_set_pull_updown_ep(PAD_PIN_PGPIO38, PAD_NONE, ep_ch);
			}
		}
	}

	return ret;

}

typedef int (*PINMUX_CONFIG_HDL)(uint32_t, u32);
static PINMUX_CONFIG_HDL pinmux_config_hdl[] = {
	pinmux_config_uart,
	pinmux_config_i2c,
	pinmux_config_sdio,
	pinmux_config_spi,
	pinmux_config_extclk,
	pinmux_config_ssp,
	pinmux_config_lcd,
	pinmux_config_remote,
	pinmux_config_vcap,
	pinmux_config_eth,
	pinmux_config_misc,
	pinmux_config_pwm,
	pinmux_config_mipi
};

/**
	Configure pinmux controller

	Configure pinmux controller by upper layer

	@param[in] info	nvt_pinctrl_info
	@return void
*/

ER pinmux_init_ep(struct nvt_pinctrl_info *info)
{
	unsigned long i;
	int err;
	unsigned long flags = 0;

	/*Enter critical section*/
	loc_cpu(flags);

	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = 0;
	//top_reg8.reg = 0x01;
	top_reg9.reg = 0;
	top_reg10.reg = 0;   //PCIE boot check
	top_reg11.reg = 0;
	top_reg12.reg = 0;
	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_jgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio1.reg = 0xFFFFFFFF;
	top_reg_egpio0.reg = 0xFFFFFFFF;
	top_reg_dgpio0.reg = 0xFFFFFFFF;
	top_reg_sgpio0.reg = 0xFFFFFFFF;
	top_reg_sgpio1.reg = 0xFFFFFFFF;
	top_reg_sgpio2.reg = 0xFFFFFFFF;
	top_reg_bgpio0.reg = 0xFFFFFFFF;
	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_jgpio0.reg = 0x0000000;
	} else {
		top_reg_jgpio0.reg = 0x000001F;
	}
	if (top_reg0.bit.PCIE_BOOT_SEL) {
		top_reg_jgpio0.bit.JGPIO_5 = 1;
		top_reg10.bit.PCIE_MODE_SEL = 1;
	}
	if (top_reg0.bit.PCIE_REFCLK_BOOT_SEL) {
		top_reg10.bit.PCIE_REFCLK_SRC = 1;
	}
	top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);

	store_extclk2[info->ep_ch] = 0;
	record_ch = info->ep_ch;

	for (i = 0; i < PIN_FUNC_MAX; i++) {
		if (info->top_pinmux[i].pin_function != i) {
			pr_err("top_config[%ld].pinFunction context error\n", i);
			/*Leave critical section*/
			unl_cpu(flags);
			return E_CTX;
		}

		err = pinmux_config_hdl[i](info->top_pinmux[i].config, info->ep_ch);
		if (err != E_OK) {
			pr_err("top_config[%ld].config config error\n", i);
			/*Leave critical section*/
			unl_cpu(flags);
			return err;
		}
	}

	TOP_SETREG(info, TOP_REG1_OFS, top_reg1.reg);
	TOP_SETREG(info, TOP_REG2_OFS, top_reg2.reg);
	TOP_SETREG(info, TOP_REG3_OFS, top_reg3.reg);
	TOP_SETREG(info, TOP_REG4_OFS, top_reg4.reg);
	TOP_SETREG(info, TOP_REG5_OFS, top_reg5.reg);
	TOP_SETREG(info, TOP_REG6_OFS, top_reg6.reg);
	TOP_SETREG(info, TOP_REG7_OFS, top_reg7.reg);
	//TOP_SETREG(TOP_REG8_OFS, top_reg8.reg);
	TOP_SETREG(info, TOP_REG9_OFS, top_reg9.reg);
	TOP_SETREG(info, TOP_REG10_OFS, top_reg10.reg);
	TOP_SETREG(info, TOP_REG11_OFS, top_reg11.reg);
	TOP_SETREG(info, TOP_REG12_OFS, top_reg12.reg);

	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGJGPIO0_OFS, top_reg_jgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGEGPIO0_OFS, top_reg_egpio0.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO0_OFS, top_reg_sgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO1_OFS, top_reg_sgpio1.reg);
	TOP_SETREG(info, TOP_REGSGPIO2_OFS, top_reg_sgpio2.reg);
	TOP_SETREG(info, TOP_REGBGPIO0_OFS, top_reg_bgpio0.reg);

	/*Leave critical section*/
	unl_cpu(flags);

	return E_OK;
}
EXPORT_SYMBOL(pinmux_init_ep);
