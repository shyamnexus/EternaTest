/*
	Pinmux module driver.

	This file is the driver of Piumux module.

	@file		na51055_pinmux_host.c
	@ingroup
	@note		Nothing.

	Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/
#include <linux/version.h>
#include "na51103_pinmux.h"
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

union TOP_REG0 top_reg0;
union TOP_REG1 top_reg1;
union TOP_REG2 top_reg2;
union TOP_REG3 top_reg3;
union TOP_REG4 top_reg4;
union TOP_REG5 top_reg5;
union TOP_REG6 top_reg6;
union TOP_REG7 top_reg7;
union TOP_REG8 top_reg8;
union TOP_REG9 top_reg9;
union TOP_REG10 top_reg10;
union TOP_REG11 top_reg11;
//union TOP_REG12 top_reg12;

union TOP_REGCGPIO0 top_reg_cgpio0;
union TOP_REGJGPIO0 top_reg_jgpio0;
union TOP_REGPGPIO0 top_reg_pgpio0;
union TOP_REGPGPIO1 top_reg_pgpio1;
union TOP_REGEGPIO0 top_reg_egpio0;
union TOP_REGDGPIO0 top_reg_dgpio0;
union TOP_REGSGPIO0 top_reg_sgpio0;
union TOP_REGSGPIO1 top_reg_sgpio1;
union TOP_REGSGPIO2 top_reg_sgpio2;
union TOP_REGBGPIO0 top_reg_bgpio0;

int confl_detect;
static int dump_gpio_func[GPIO_total]={0};
static char *dump_func[FUNC_total]={"FSPI","SDIO1","SDIO1_2","SPI","SPI2_1","SPI2_2","EJTAG","EXTROM","LCD310","LCD310_2","LCD310_RGB888",
     "LCD210","LCD210_2","LCD310_DE","ETH","ETH2","I2C","I2C_2","I2C2","I2C2_2","I2C2_3","I2C2_4","I2C2_5","I2C3",
     "I2C3_2","I2C_HDMI","EXT_CLK","EXT2_CLK","EXT3_CLK","EXT4_CLK","IRDA","RTC_CAL","TVDAC_TEST_CLK_IN","PWM","PWM2",
     "PWM3","UART","UART2_1","UART2_2","UART2_3","UART2_4","UART3","UART4_1","UART4_2","UART4_3","UART4_4","UART4_5",
     "UART4_6","UART_CTS_RTS","UART2_CTS_RTS","UART3_RTS","CAP","CAP1","CAP2","CAP3","CAP_CLK1","CAP_CLK2","CAP1_CLK1",
     "CAP1_CLK2","CAP2_CLK1","CAP2_CLK2","CAP3_CLK","CAP3_CLK2","CAP3_CLK3","I2S","I2S_MCLK","I2S_TX","I2S_RX","I2S2",
     "I2S2_MCLK","I2S2_TX","I2S2_RX","I2S3_1","I2S3_1_MCLK","I2S3_1_TX","I2S3_1_RX","I2S3_2","I2S3_2_MCLK","I2S3_2_TX",
     "I2S3_2_RX","I2S3_3","I2S3_3_MCLK","I2S3_3_TX","I2S3_3_RX","I2S4_1","I2S4_1_MCLK","I2S4_1_TX","I2S4_1_RX","I2S4_2",
     "I2S4_2_MCLK","I2S4_2_TX","I2S4_2_RX","MISC"};

//static int store_extclk2 = 0;
struct nvt_pinctrl_info info_get_id[1] = {0};
#include <linux/of.h>
uint32_t nvt_get_chip_id(void)
{
	union TOP_VERSION_REG top_version;
	struct device_node *top;
	static void __iomem *top_reg_addr = NULL;;
	//u32 value[6] = {};

	if (!top_reg_addr) {
		top = of_find_compatible_node(NULL, NULL, "nvt,nvt_top");
		if (top) {
			const __be32 *cell;
			cell = of_get_property(top, "reg", NULL);
			if (cell) {
				phys_addr_t top_pa = 0;
				top_pa = of_read_number(cell, of_n_addr_cells(top));
				//printk("size = %d\r\n", of_n_addr_cells(top));
				//printk("%s: get reg addr 0x%lx \r\n", __func__, top_pa);
				#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
				top_reg_addr = ioremap(top_pa, 0x100);
				#else
				top_reg_addr = ioremap_nocache(top_pa, 0x100);
				#endif
			} else {
				pr_err("*** %s not get top reg ***\n", __func__);
				return -ENOMEM;
			}
		} else {
			pr_err("*** %s not get dts node ***\n", __func__);
			return -ENOMEM;
		}
	}

	if (top_reg_addr) {
		info_get_id->top_base = top_reg_addr;
		top_version.reg = TOP_GETREG(info_get_id, TOP_VERSION_REG_OFS);
	} else {
		pr_err("invalid pinmux address\n");
		return -ENOMEM;
	}

	return top_version.bit.CHIP_ID;
}
EXPORT_SYMBOL(nvt_get_chip_id);

static void gpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char* gpio_name[] = {"C_GPIO", "J_GPIO", "P_GPIO", "E_GPIO", "D_GPIO", "S_GPIO", "B_GPIO"};
	char name[10];

	if (start_offset == TOP_REGCGPIO0_OFS)
		strcpy(name, gpio_name[0]);
	else if (start_offset == TOP_REGJGPIO0_OFS)
		strcpy(name, gpio_name[1]);
	else if (start_offset == TOP_REGPGPIO0_OFS)
		strcpy(name, gpio_name[2]);
	else if (start_offset == TOP_REGEGPIO0_OFS)
		strcpy(name, gpio_name[3]);
	else if (start_offset == TOP_REGDGPIO0_OFS)
		strcpy(name, gpio_name[4]);
	else if (start_offset == TOP_REGSGPIO0_OFS)
		strcpy(name, gpio_name[5]);
	else if (start_offset == TOP_REGBGPIO0_OFS)
		strcpy(name, gpio_name[6]);


	if (gpio_number > 0x20) {
		reg_value = TOP_GETREG(info, start_offset);

		for (i = 0; i < 0x20; i++) {
			if (reg_value & (1 << i))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
		}

		reg_value = TOP_GETREG(info, start_offset + 0x4);

		for (j = 0; j < (gpio_number - 0x20); j++) {
			if (reg_value & (1 << j))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
			i++;
		}
	} else {
		reg_value = TOP_GETREG(info, start_offset);
		for (i = 0; i < gpio_number; i++) {
			if (reg_value & (1 << i))
				pr_info("%s%-2d       GPIO\n", name, i);
			else
				pr_info("%s%-2d     FUNCTION\n", name, i);
		}
	}

}

void pinmux_gpio_parsing(struct nvt_pinctrl_info *info)
{
	pr_info("\n  PIN         STATUS\n");

	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, J_GPIO_NUM, TOP_REGJGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, E_GPIO_NUM, TOP_REGEGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
	gpio_info_show(info, S_GPIO_NUM, TOP_REGSGPIO0_OFS);
	//gpio_info_show(info, B_GPIO_NUM, TOP_REGBGPIO0_OFS);
}

void pinmux_preset(struct nvt_pinctrl_info *info)
{
	spin_lock_init(&top_lock);
}

/**
	Read pinmux data from controller base

	Read pinmux data from controller base

	@param[in] info	nvt_pinctrl_info
*/

void pinmux_parsing(struct nvt_pinctrl_info *info)
{
	u32 value;
	unsigned long flags = 0;
	//union TOP_REG0 local_top_reg0;
	union TOP_REG1 local_top_reg1;
	union TOP_REG2 local_top_reg2;
	union TOP_REG3 local_top_reg3;
	union TOP_REG4 local_top_reg4;
	union TOP_REG5 local_top_reg5;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
//	union TOP_REG8 local_top_reg8;
	union TOP_REG9 local_top_reg9;
//	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;
//	union TOP_REG12 local_top_reg12;
	union TOP_REGPGPIO0 local_top_reg_pgpio0;
	//union TOP_REGPGPIO1 local_top_reg_pgpio1;
	//union TOP_REGEGPIO0 local_top_reg_egpio0;
	//union TOP_REGSGPIO1 local_top_reg_sgpio1;
	//union TOP_REGSGPIO2 local_top_reg_sgpio2;

	loc_cpu(flags);
	//local_top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	//local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	//local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	//local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg_pgpio0.reg = TOP_GETREG(info, TOP_REGPGPIO0_OFS);
	//local_top_reg_pgpio1.reg = TOP_GETREG(info, TOP_REGPGPIO1_OFS);
	//local_top_reg_egpio0.reg = TOP_GETREG(info, TOP_REGEGPIO0_OFS);
	//local_top_reg_sgpio1.reg = TOP_GETREG(info, TOP_REGSGPIO1_OFS);
	//local_top_reg_sgpio2.reg = TOP_GETREG(info, TOP_REGSGPIO2_OFS);

	/*Parsing UART*/
	value = 0x0;
	switch (local_top_reg7.bit.UART) {
	case MUX_1:
		value |= PIN_UART_CFG_CH_1ST_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg7.bit.UART_CTS_RTS != 0x0)
		value |= PIN_UART_CFG_CH_CTSRTS;

	switch (local_top_reg7.bit.UART2) {
	case MUX_1:
		value |= PIN_UART_CFG_CH2_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_UART_CFG_CH2_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_UART_CFG_CH2_3RD_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg7.bit.UART2_CTS_RTS != 0x0)
		value |= PIN_UART_CFG_CH2_CTSRTS;

	if (local_top_reg7.bit.UART3 == MUX_1)
		value |= PIN_UART_CFG_CH3_1ST_PINMUX;

	if (local_top_reg7.bit.UART3_RTS)
		value |= PIN_UART_CFG_CH3_RTS;
	switch (local_top_reg7.bit.UART4) {
	case UART_MUX_1:
		value |= PIN_UART_CFG_CH4_1ST_PINMUX;
		break;
	case UART_MUX_2:
		value |= PIN_UART_CFG_CH4_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_UART_CFG_CH4_3RD_PINMUX;
		break;
	case MUX_4:
		value |= PIN_UART_CFG_CH4_4TH_PINMUX;
		break;
	case MUX_5:
		value |= PIN_UART_CFG_CH4_5TH_PINMUX;
		break;
	case MUX_6:
		value |= PIN_UART_CFG_CH4_6TH_PINMUX;
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

	switch (local_top_reg4.bit.I2C2) {
	case MUX_1:
		value |= PIN_I2C_CFG_CH2_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_I2C_CFG_CH2_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_I2C_CFG_CH2_3RD_PINMUX;
		break;
	case MUX_4:
		value |= PIN_I2C_CFG_CH2_4TH_PINMUX;
		break;
	case MUX_5:
		value |= PIN_I2C_CFG_CH2_5TH_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg4.bit.I2C3) {
	case MUX_1:
		value |= PIN_I2C_CFG_CH3_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_I2C_CFG_CH3_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg4.bit.I2C_HDMI)
		value |= PIN_I2C_CFG_HDMI_1ST_PINMUX;

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/* Parsing SDIO */
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
	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/*Parsing SPI*/
	value = 0x0;
	if (local_top_reg1.bit.FSPI_EN == MUX_EN)
		value |= PIN_SPI_CFG_CH_1ST_PINMUX;

	if (local_top_reg1.bit.FSPI_BUS_WIDTH == SPI_BUS_WIDTH_4)
		value |= PIN_SPI_CFG_CH_BUS_WIDTH;

	if (local_top_reg1.bit.FSPI_CS1_EN == MUX_EN)
		value |= PIN_SPI_CFG_CH_CS1;

	switch (local_top_reg1.bit.SPI) {
	case MUX_1:
		value |= PIN_SPI_CFG_CH2_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_SPI_CFG_CH2_2ND_PINMUX;
		break;
	default:
		break;
	}
	if (local_top_reg1.bit.SPI_BUS_WIDTH == MUX_EN)
		value |= PIN_SPI_CFG_CH2_BUS_WIDTH;

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/*Parsing EXTCLK*/
	value = 0x0;
	if (local_top_reg5.bit.EXT_CLK == MUX_EN)
		value |= PIN_EXTCLK_CFG_CH_1ST_PINMUX;

	if (local_top_reg5.bit.EXT2_CLK == MUX_EN)
		value |= PIN_EXTCLK_CFG_CH2_1ST_PINMUX;

	if (local_top_reg5.bit.EXT3_CLK == MUX_EN)
		value |= PIN_EXTCLK_CFG_CH3_1ST_PINMUX;

	if (local_top_reg5.bit.EXT4_CLK == MUX_EN)
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

	switch (local_top_reg11.bit.I2S4) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH4_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH4_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg11.bit.I2S4_MCLK == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_MCLK;

	switch (local_top_reg11.bit.I2S4_TX) {
	case MUX_1:
		value |= PIN_SSP_CFG_CH4_1ST_TX;
		break;
	case MUX_2:
		value |= PIN_SSP_CFG_CH4_2ND_TX;
		break;
	default:
		break;
	}

	if (local_top_reg11.bit.I2S4_RX == MUX_EN)
		value |= PIN_SSP_CFG_CH4_1ST_RX;

	info->top_pinmux[PIN_FUNC_SSP].config = value;
	info->top_pinmux[PIN_FUNC_SSP].pin_function = PIN_FUNC_SSP;

	/*Parsing LCD*/
	value = 0x0;
	switch (local_top_reg2.bit.LCD310) {
	case MUX_1:
		value |= PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX;
		break;
	case MUX_3:
		value |= PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg2.bit.LCD310_DE == MUX_EN)
		value |= PIN_LCD_CFG_LCD310_DE_PINMUX;

	switch (local_top_reg2.bit.LCD210) {
	case MUX_1:
		value |= PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX;
		break;
		break;
	default:
		break;
	}

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
	case MUX_3:
		value |= PIN_VCAP_CFG_CAP3_CLK_3RD_PINMUX;
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
	default:
		break;
	}

	if (local_top_reg3.bit.ETH2_REFCLK == MUX_EN)
		value |= PIN_ETH2_CFG_REFCLK_PINMUX;

	if (local_top_reg3.bit.ETH2_RST == MUX_EN)
		value |= PIN_ETH2_CFG_RST_PINMUX;

	if (local_top_reg3.bit.ETH2_MDC_MDIO)
		value |= PIN_ETH2_CFG_MDC_MDIO_PINMUX;

	

	switch (local_top_reg5.bit.ETH_ACT_LED) {
	case MUX_1:
		value |= PIN_ETH_CFG_ACT_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_ETH_CFG_ACT_LED_2ND_PINMUX;
		break;
	default:
		break;
	}

	switch (local_top_reg5.bit.ETH_LINK_LED) {
	case MUX_1:
		value |= PIN_ETH_CFG_LINK_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_ETH_CFG_LINK_LED_2ND_PINMUX;
		break;
	default:
		break;
	}

	if (local_top_reg3.bit.ETH_PHY_SEL) {
		value |= PIN_ETH_CFG_PHY_SEL;
	}

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

	if (local_top_reg_pgpio0.bit.PGPIO_31 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_HDMI_HOTPLUG;

	if (local_top_reg_pgpio0.bit.PGPIO_27 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_VGA_HS;

	if (local_top_reg_pgpio0.bit.PGPIO_28 == GPIO_ID_EMUM_FUNC)
		value |= PIN_MISC_CFG_VGA_VS;

	switch (local_top_reg5.bit.SATA_LED) {
	case MUX_1:
		value |= PIN_MISC_CFG_SATA_LED_1ST_PINMUX;
		break;
	case MUX_2:
		value |= PIN_MISC_CFG_SATA_LED_2ND_PINMUX;
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
	default:
		break;
	}

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

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/*Parsing VCAPINT*/
	value = 0x0;

	info->top_pinmux[PIN_FUNC_VCAPINT].config = value;
	info->top_pinmux[PIN_FUNC_VCAPINT].pin_function = PIN_FUNC_VCAPINT;

	unl_cpu(flags);
}

/*uint32_t pinmux_get_config(PIN_FUNC pinfunc)
{
	uint32_t ret = 0;

	if(pinfunc < PIN_FUNC_MAX) {
		ret = pin_config[pinfunc];
	}

	return ret;
}*/
void gpio_func_keep(int start,int count,int func)
{
    int i=0;

    for(i=start;i<start+count;i++)
    {
        dump_gpio_func[i]=func;
    }
}

int gpio_conflict_detect(int start,int count,int func)
{
    int i=0;
    int confl_mod;
    int confl_flag=0;
    for(i=start;i<start+count;i++)
    {
        confl_mod=dump_gpio_func[i];
        //printf("%d\r\n",confl_mod);

        if(confl_mod>0)
        {
            pr_err("\033[0;31m%s conflict with %s\r\n\033[0m",dump_func[func-1],dump_func[confl_mod-1]);
            confl_flag++;
            break;
        }
    }
    return confl_flag;

}



ER pinmux_config_uart(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_UART_CFG_NONE) {
	} else {
		if (config & (PIN_UART_CFG_CH_1ST_PINMUX | PIN_UART_CFG_CH_CTSRTS)) {
            confl_detect+=gpio_conflict_detect(PGPIO_0,2,func_UART);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg7.bit.UART = MUX_EN;
			gpio_func_keep(PGPIO_0,2,func_UART);
			if (config & PIN_UART_CFG_CH_CTSRTS) {
				/*if (top_reg1.bit.SPI == MUX_1) {
					pr_err("PIN_UART_CFG_CH_CTSRTS conflict with SPI2\r\n");
					return E_OBJ;

				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_2,2,func_UART_CTS_RTS);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART_CTS_RTS = MUX_1;
				gpio_func_keep(PGPIO_2,2,func_UART_CTS_RTS);
			}
		}
		if (config & (PIN_UART_CFG_CH2_1ST_PINMUX | PIN_UART_CFG_CH2_2ND_PINMUX | PIN_UART_CFG_CH2_3RD_PINMUX | PIN_UART_CFG_CH2_CTSRTS)) {
			if (config & PIN_UART_CFG_CH2_1ST_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_5) {
					pr_err("PIN_UART_CFG_CH2_1ST_PINMUX conflict with I2C2_5\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_MCLK == MUX_3) {
					pr_err("PIN_UART_CFG_CH2_1ST_PINMUX conflict with I2S3_3_MCLK\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_5) || (top_reg5.bit.IRDA == MUX_6)) {
					pr_err("PIN_UART_CFG_CH2_1ST_PINMUX conflict with IRDA4 or IRDA5\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_4,2,func_UART2_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART2 = MUX_1;
				gpio_func_keep(PGPIO_4,2,func_UART2_1);
			}
			if (config & PIN_UART_CFG_CH2_2ND_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_1) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with I2C2\r\n");
				}
				if (top_reg11.bit.I2S3 == MUX_2) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with I2S3_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_EN) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3 == MUX_3) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with I2S3_3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_RX == MUX_3) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with I2S3_3_RX\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP3_CLK == MUX_3) {
					pr_err("PIN_UART_CFG_CH2_2ND_PINMUX conflict with CAP3_3_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_14,2,func_UART2_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART2 = MUX_2;
				gpio_func_keep(PGPIO_14,2,func_UART2_2);
			}
			if (config & PIN_UART_CFG_CH2_3RD_PINMUX) {
				/*if (top_reg4.bit.I2C == MUX_2) {
					pr_err("PIN_UART_CFG_CH2_3RD_PINMUX conflict with I2C_2\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_7) || (top_reg5.bit.IRDA == MUX_8)) {
					pr_err("PIN_UART_CFG_CH2_3RD_PINMUX conflict with IRDA6 or IRDA7\r\n");
					return E_OBJ;
				}*/
                confl_detect+=gpio_conflict_detect(DGPIO_0,2,func_UART2_3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART2 = MUX_3;
				gpio_func_keep(DGPIO_0,2,func_UART2_3);
			}
			if (config & PIN_UART_CFG_CH2_CTSRTS) {
				/*if (top_reg3.bit.ETH == MUX_1) {
					pr_err("PIN_UART_CFG_CH2_CTSRTS conflict with RGMII\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_2) {
					pr_err("PIN_UART_CFG_CH2_CTSRTS conflict with LCD310_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_2) {
					pr_err("PIN_UART_CFG_CH2_CTSRTS conflict with LCD210_2_BT1120\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(EGPIO_1,1,func_UART2_CTS_RTS);
                confl_detect+=gpio_conflict_detect(EGPIO_6,1,func_UART2_CTS_RTS);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_egpio0.bit.EGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART2_CTS_RTS = MUX_1;
				gpio_func_keep(EGPIO_1,1,func_UART2_CTS_RTS);
                gpio_func_keep(EGPIO_6,1,func_UART2_CTS_RTS);
			}
		}
		if (config & (PIN_UART_CFG_CH3_1ST_PINMUX | PIN_UART_CFG_CH3_RTS)) {
			if (config & PIN_UART_CFG_CH3_1ST_PINMUX) {
                confl_detect+=gpio_conflict_detect(PGPIO_6,2,func_UART3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART3 = MUX_1;
				gpio_func_keep(PGPIO_6,2,func_UART3);
			}
			if (config & PIN_UART_CFG_CH3_RTS) {
				/*if (top_reg4.bit.I2C == MUX_3) {
					pr_err("PIN_UART_CFG_CH3_RTS conflict with I2C_3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_MCLK == MUX_1) {
					pr_err("PIN_UART_CFG_CH3_RTS conflict with I2S3_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_2) {
					pr_err("PIN_UART_CFG_CH3_RTS conflict with LCD310_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_2) {
					pr_err("PIN_UART_CFG_CH3_RTS conflict with LCD210_2_BT1120\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_5,1,func_UART3_RTS);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART3_RTS = MUX_1;
				gpio_func_keep(DGPIO_5,1,func_UART3_RTS);
			}
		}
		if (config & (PIN_UART_CFG_CH4_1ST_PINMUX | PIN_UART_CFG_CH4_2ND_PINMUX | PIN_UART_CFG_CH4_3RD_PINMUX | PIN_UART_CFG_CH4_4TH_PINMUX | PIN_UART_CFG_CH4_5TH_PINMUX | PIN_UART_CFG_CH4_6TH_PINMUX)) {
			if (config & PIN_UART_CFG_CH4_1ST_PINMUX) {
				/*if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with EJTAG\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4_TX == MUX_EN) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with I2S4_TX\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4_RX == MUX_EN) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with I2S4_RX\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA_LED == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with SATA_LED\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA2_LED == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with SATA2_LED\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_3) || (top_reg5.bit.IRDA == MUX_4)) {
					pr_err("PIN_UART_CFG_CH2_3RD_PINMUX conflict with IRDA2 or IRDA3\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(JGPIO_3,2,func_UART4_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_1;
				gpio_func_keep(JGPIO_3,2,func_UART4_1);
			}
			if (config & PIN_UART_CFG_CH4_2ND_PINMUX) {
				/*if (top_reg4.bit.I2C == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with I2C\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4 == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_2ND_PINMUX conflict with I2S4_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_8,2,func_UART4_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_2;
				gpio_func_keep(PGPIO_8,2,func_UART4_2);
			}
			if (config & PIN_UART_CFG_CH4_3RD_PINMUX) {
				/*if (top_reg4.bit.I2C3 == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_3RD_PINMUX conflict with I2C3_2\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S2 == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_3RD_PINMUX conflict with I2S2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_3RD_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_UART_CFG_CH4_3RD_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_16,2,func_UART4_3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_3;
				gpio_func_keep(PGPIO_16,2,func_UART4_3);
			}
			if (config & PIN_UART_CFG_CH4_4TH_PINMUX) {
				/*if (top_reg4.bit.I2C3 == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_4TH_PINMUX conflict with I2C3\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_4TH_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SPI == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_4TH_PINMUX conflict with SPI_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_20,2,func_UART4_4);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_4;
				gpio_func_keep(PGPIO_20,2,func_UART4_4);
			}
			if (config & PIN_UART_CFG_CH4_5TH_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_5TH_PINMUX conflict with I2C2_2\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.SATA_LED == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_1ST_PINMUX conflict with SATA_2_LED\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_2,2,func_UART4_5);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_5;
				gpio_func_keep(DGPIO_2,2,func_UART4_5);
			}
			if (config & PIN_UART_CFG_CH4_6TH_PINMUX) {
				/*if (top_reg11.bit.I2S3 == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_6TH_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_RX == MUX_1) {
					pr_err("PIN_UART_CFG_CH4_6TH_PINMUX conflict with I2S3_RX\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.ETH_ACT_LED == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_6TH_PINMUX conflict with ETH_2_ACT_LED\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_UART_CFG_CH4_6TH_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_7,2,func_UART4_6);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg7.bit.UART4 = MUX_6;
				gpio_func_keep(DGPIO_7,2,func_UART4_6);
			}

		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_i2c(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & (PIN_I2C_CFG_CH_1ST_PINMUX | PIN_I2C_CFG_CH_2ND_PINMUX)) {
			if (config & PIN_I2C_CFG_CH_1ST_PINMUX) {
				/*if (top_reg7.bit.UART4 == UART_MUX_2) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with UART4_2\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH_1ST_PINMUX conflict with I2S4_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_8,2,func_I2C_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C = MUX_1;
				pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_PGPIO8, PAD_NONE);
				gpio_func_keep(PGPIO_8,2,func_I2C_1);
			} else {
				/*if (top_reg7.bit.UART2 == UART_MUX_3) {
					pr_err("PIN_I2C_CFG_CH_2ND_PINMUX conflict with UART2_3\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == 7) || (top_reg5.bit.IRDA == 8)) {
					pr_err("PIN_I2C_CFG_CH_2ND_PINMUX conflict with IRDA6/7\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_0,2,func_I2C_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C = MUX_2;
				pad_set_pull_updown(PAD_PIN_DGPIO1, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_DGPIO0, PAD_NONE);
				gpio_func_keep(DGPIO_0,2,func_I2C_2);
			}
		}
		if (config & (PIN_I2C_CFG_CH2_1ST_PINMUX | PIN_I2C_CFG_CH2_2ND_PINMUX | PIN_I2C_CFG_CH2_3RD_PINMUX | PIN_I2C_CFG_CH2_4TH_PINMUX | PIN_I2C_CFG_CH2_5TH_PINMUX)) {
			if (config & PIN_I2C_CFG_CH2_1ST_PINMUX) {
				/*if (top_reg11.bit.I2S3 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with I2S3_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART2 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with UART2_2\r\n");
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP3_CLK == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with CAP3_3_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with I2S3_3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_RX == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_1ST_PINMUX conflict with I2S3_3_RX\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_14,2,func_I2C2_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C2 = MUX_1;
				pad_set_pull_updown(PAD_PIN_PGPIO14, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_PGPIO15, PAD_NONE);
				gpio_func_keep(PGPIO_14,2,func_I2C2_1);
			}
			if (config & PIN_I2C_CFG_CH2_2ND_PINMUX) {
				/*if (top_reg7.bit.UART4 == MUX_5) {
					pr_err("PIN_I2C_CFG_CH2_2ND_PINMUX conflict with UART4_5\r\n");
				}
				if (top_reg5.bit.SATA_LED == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_2ND_PINMUX conflict with SATA_2_LED\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_9) || (top_reg5.bit.IRDA == MUX_A)) {
					pr_err("PIN_UART_CFG_CH2_3RD_PINMUX conflict with IRDA8 or IRDA9\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_2,2,func_I2C2_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C2 = MUX_2;
				pad_set_pull_updown(PAD_PIN_DGPIO2, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_DGPIO3, PAD_NONE);
				gpio_func_keep(DGPIO_2,2,func_I2C2_2);
			}
			if (config & PIN_I2C_CFG_CH2_3RD_PINMUX) {
				/*if (top_reg11.bit.I2S3 == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_MCLK == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with I2S3_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with LCD310_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with LCD210_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SPI == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with SPI_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART3_RTS == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_3RD_PINMUX conflict with UART3_RTS\r\n");
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_5,2,func_I2C2_3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C2 = MUX_3;
				pad_set_pull_updown(PAD_PIN_DGPIO5, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_DGPIO6, PAD_NONE);
				gpio_func_keep(DGPIO_5,2,func_I2C2_3);
			}
			if (config & PIN_I2C_CFG_CH2_4TH_PINMUX) {
				/*if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with EJTAG\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4_MCLK == MUX_EN) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with I2S4_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4 == MUX_EN) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with I2S4\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.ETH_ACT_LED == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with ETH_ACT_LED\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_1)) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with IRDA0\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SPI == MUX_1) {
					pr_err("PIN_I2C_CFG_CH2_4TH_PINMUX conflict with SPI\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(JGPIO_0,2,func_I2C2_4);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C2 = MUX_4;
				pad_set_pull_updown(PAD_PIN_JGPIO0, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_JGPIO1, PAD_NONE);
				gpio_func_keep(JGPIO_0,2,func_I2C2_4);
			}
			if (config & PIN_I2C_CFG_CH2_5TH_PINMUX) {
				/*if (top_reg7.bit.UART2 == MUX_2) {
					pr_err("PIN_I2C_CFG_CH2_5TH_PINMUX conflict with UART2_2\r\n");
				}
				if ((top_reg5.bit.IRDA == MUX_5) || (top_reg5.bit.IRDA == MUX_6)) {
					pr_err("PIN_I2C_CFG_CH2_5TH_PINMUX conflict with IRDA4/5\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_5TH_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3_MCLK == MUX_3) {
					pr_err("PIN_I2C_CFG_CH2_5TH_PINMUX conflict with I2S3_MCLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_4,2,func_I2C2_5);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C2 = MUX_5;
				pad_set_pull_updown(PAD_PIN_PGPIO4, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_PGPIO5, PAD_NONE);
				gpio_func_keep(PGPIO_4,2,func_I2C2_5);
			}

		}

		if (config & (PIN_I2C_CFG_CH3_1ST_PINMUX | PIN_I2C_CFG_CH3_2ND_PINMUX)) {
			if (config & PIN_I2C_CFG_CH3_1ST_PINMUX) {
				/*if (top_reg7.bit.UART4 == MUX_4) {
					pr_err("PIN_I2C_CFG_CH3_1ST_PINMUX conflict with UART4_4\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_I2C_CFG_CH3_1ST_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SPI == MUX_2) {
					pr_err("PIN_I2C_CFG_CH3_1ST_PINMUX conflict with SPI_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_20,2,func_I2C3_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_1;
				pad_set_pull_updown(PAD_PIN_PGPIO20, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
				gpio_func_keep(PGPIO_20,2,func_I2C3_1);
			}
			if (config & PIN_I2C_CFG_CH3_2ND_PINMUX) {
				/*if (top_reg11.bit.I2S2 == MUX_1) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with I2S2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with UART4_3\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_I2C_CFG_CH3_2ND_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_16,2,func_I2C3_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg4.bit.I2C3 = MUX_2;
				pad_set_pull_updown(PAD_PIN_PGPIO16, PAD_NONE);
				pad_set_pull_updown(PAD_PIN_PGPIO17, PAD_NONE);
				gpio_func_keep(PGPIO_16,2,func_I2C3_2);
			}
		}
		if (config & (PIN_I2C_CFG_HDMI_1ST_PINMUX)) {
			confl_detect+=gpio_conflict_detect(PGPIO_29,2,func_I2C_HDMI);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg4.bit.I2C_HDMI = MUX_1;
			pad_set_pull_updown(PAD_PIN_PGPIO29, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO30, PAD_NONE);
			gpio_func_keep(PGPIO_29,2,func_I2C_HDMI);
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_sdio(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & (PIN_SDIO_CFG_1ST_PINMUX | PIN_SDIO_CFG_2ND_PINMUX)) {
			if (config & PIN_SDIO_CFG_1ST_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_1) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2C2\r\n");
					return E_OBJ;
				}
				if (top_reg4.bit.I2C3 == MUX_2) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2C3_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART2 == UART_MUX_2) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with UART2_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == UART_MUX_3) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with UART4_3\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_3))
				{
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with LCD310 RGB888\r\n");
					return E_OBJ;
				}
				if ((top_reg2.bit.LCD310 == MUX_3))
				{
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with LCD310 RGB888\r\n");
					return E_OBJ;
				}
				if ((top_reg6.bit.PWM3 == MUX_EN))
				{
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with PWM3\r\n");
					return E_OBJ;
				}
				if ((top_reg11.bit.I2S2 == MUX_EN) || (top_reg11.bit.I2S2_TX == MUX_EN) || (top_reg11.bit.I2S2_RX == MUX_EN)) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2S2\r\n");
					return E_OBJ;
				}*/
				//TBD I2S2 and I2S3 check
				/*if ((top_reg11.bit.I2S3 == MUX_2)|| (top_reg11.bit.I2S3_TX == MUX_2)) {
					pr_err("PIN_SDIO_CFG_1ST_PINMUX conflict with I2S3_2 RXTX\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_14,2,func_SDIO_1);
                confl_detect+=gpio_conflict_detect(PGPIO_17,3,func_SDIO_1);
                confl_detect+=gpio_conflict_detect(PGPIO_23,1,func_SDIO_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO = MUX_1;
				gpio_func_keep(PGPIO_14,2,func_SDIO_1);
                gpio_func_keep(PGPIO_17,3,func_SDIO_1);
                gpio_func_keep(PGPIO_23,1,func_SDIO_1);
			} else {
				confl_detect+=gpio_conflict_detect(DGPIO_6,4,func_SDIO_2);
                confl_detect+=gpio_conflict_detect(PGPIO_20,2,func_SDIO_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SDIO = MUX_2;
				gpio_func_keep(DGPIO_6,4,func_SDIO_2);
                gpio_func_keep(PGPIO_20,2,func_SDIO_2);
			}
		}
	}
	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_spi(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & (PIN_SPI_CFG_CH_1ST_PINMUX | PIN_SPI_CFG_CH_BUS_WIDTH | PIN_SPI_CFG_CH_CS1)) {
			if (config & (PIN_SPI_CFG_CH_1ST_PINMUX)) {
				confl_detect+=gpio_conflict_detect(CGPIO_0,4,func_SPI);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.FSPI_EN = MUX_EN;
				gpio_func_keep(CGPIO_0,4,func_SPI);

				if (config & (PIN_SPI_CFG_CH_BUS_WIDTH)) {
					gpio_conflict_detect(CGPIO_4,2,func_SPI);
                    if(confl_detect>0)
                    {
                        return E_OBJ;
                    }
					top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
					top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
					top_reg1.bit.FSPI_BUS_WIDTH = SPI_BUS_WIDTH_4;
					gpio_func_keep(CGPIO_4,2,func_SPI);
				}
				if (config & (PIN_SPI_CFG_CH_CS1)) {
					gpio_conflict_detect(CGPIO_6,1,func_SPI);
                    if(confl_detect>0)
                    {
                        return E_OBJ;
                    }
					top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
					top_reg1.bit.FSPI_CS1_EN = MUX_EN;
					gpio_func_keep(CGPIO_6,1,func_SPI);
				}
			}
		}
		if (config & (PIN_SPI_CFG_CH2_1ST_PINMUX | PIN_SPI_CFG_CH2_2ND_PINMUX | PIN_SPI_CFG_CH2_BUS_WIDTH)) {
			if (config & (PIN_SPI_CFG_CH2_1ST_PINMUX)) {
				/*if (top_reg1.bit.EJTAG_EN == MUX_EN) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with EJTAG\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4_MCLK == MUX_EN) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with I2S4_MCLK\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S4 == MUX_EN) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with I2S4\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.ETH_ACT_LED == MUX_1) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with ETH_ACT_LED\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_1)) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with IRDA0\r\n");
					return E_OBJ;
				}
				if (top_reg4.bit.I2C2 == MUX_4) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with I2C2_4\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART_CTS_RTS == MUX_1) {
					pr_err("PIN_SPI_CFG_CH2_1ST_PINMUX conflict with UART_RTS\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(JGPIO_0,2,func_SPI2_1);
                confl_detect+=gpio_conflict_detect(PGPIO_2,1,func_SPI2_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SPI = MUX_1;
				gpio_func_keep(JGPIO_0,2,func_SPI2_1);
                gpio_func_keep(PGPIO_2,1,func_SPI2_1);
				if ((config & PIN_SPI_CFG_CH2_BUS_WIDTH)) {
					confl_detect+=gpio_conflict_detect(PGPIO_3,1,func_SPI2_1);
                    if(confl_detect>0)
                    {
                        return E_OBJ;
                    }
					top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
					top_reg1.bit.SPI_BUS_WIDTH = MUX_EN;
					gpio_func_keep(PGPIO_3,1,func_SPI2_1);
				}
			}
			if (config & (PIN_SPI_CFG_CH2_2ND_PINMUX)) {
				/*if (top_reg4.bit.I2C3 == MUX_1) {
					pr_err("PIN_SPI_CFG_CH2_2ND_PINMUX conflict with I2C3\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_4) {
					pr_err("PIN_SPI_CFG_CH2_2ND_PINMUX conflict with UART4_4\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_SPI_CFG_CH2_2ND_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}

				if (top_reg4.bit.I2C2 == MUX_3) {
					pr_err("PIN_SPI_CFG_CH2_2ND_PINMUX conflict with I2C2_3\r\n");
					return E_OBJ;
				}
				if (top_reg11.bit.I2S3 == MUX_1) {
					pr_err("PIN_SPI_CFG_CH2_2ND_PINMUX conflict with I2S3\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_20,2,func_SPI2_2);
                confl_detect+=gpio_conflict_detect(DGPIO_6,1,func_SPI2_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg1.bit.SPI = MUX_1;
				gpio_func_keep(PGPIO_20,2,func_SPI2_2);
                gpio_func_keep(DGPIO_6,1,func_SPI2_2);
				if ((config & PIN_SPI_CFG_CH2_BUS_WIDTH)) {
					confl_detect+=gpio_conflict_detect(DGPIO_7,1,func_SPI2_2);
                    if(confl_detect>0)
                    {
                        return E_OBJ;
                    }
					top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
					top_reg1.bit.SPI_BUS_WIDTH = MUX_EN;
					gpio_func_keep(DGPIO_7,1,func_SPI2_2);
				}

			}

		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_extclk(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);


	if (config == PIN_EXTCLK_CFG_NONE) {
	} else {
		if (config & (PIN_EXTCLK_CFG_CH_1ST_PINMUX)) {
			/*if (top_reg11.bit.I2S_MCLK == MUX_1) {
				pr_err("PIN_EXTCLK_CFG_CH_1ST_PINMUX conflict with I2S_MCLK\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(PGPIO_22,1,func_EXT_CLK);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.EXT_CLK = MUX_EN;
			gpio_func_keep(PGPIO_22,1,func_EXT_CLK);
		}
		if (config & (PIN_EXTCLK_CFG_CH2_1ST_PINMUX)) {
			/*if (top_reg11.bit.I2S2_MCLK == MUX_1) {
				pr_err("PIN_EXTCLK_CFG_CH2_1ST_PINMUX conflict with I2S2_MCLK\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP_CLK == MUX_2) {
				pr_err("PIN_EXTCLK_CFG_CH2_1ST_PINMUX conflict with CAP_2_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_1) {
				pr_err("PIN_EXTCLK_CFG_CH2_1ST_PINMUX conflict with SDIO\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_3) {
				pr_err("PIN_EXTCLK_CFG_CH2_1ST_PINMUX conflict with LCD310_RGB\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(PGPIO_23,1,func_EXT2_CLK);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.EXT2_CLK = MUX_EN;
			gpio_func_keep(PGPIO_23,1,func_EXT2_CLK);

		}
		if (config & (PIN_EXTCLK_CFG_CH3_1ST_PINMUX)) {
			/*if (top_reg9.bit.CAP1_CLK == MUX_1) {
				pr_err("PIN_EXTCLK_CFG_CH3_1ST_PINMUX conflict with CAP1_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP2_CLK == MUX_2) {
				pr_err("PIN_EXTCLK_CFG_CH3_1ST_PINMUX conflict with CAP2_2_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_3) {
				pr_err("PIN_EXTCLK_CFG_CH3_1ST_PINMUX conflict with LCD310_RGB\r\n");
				return E_OBJ;
			}
			if ((top_reg3.bit.ETH2 == MUX_1) || (top_reg3.bit.ETH2 == MUX_2)) {
				pr_err("PIN_EXTCLK_CFG_CH3_1ST_PINMUX conflict with ETH2\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(SGPIO_9,1,func_EXT3_CLK);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.EXT3_CLK = MUX_EN;
			gpio_func_keep(SGPIO_9,1,func_EXT3_CLK);
		}
		if (config & (PIN_EXTCLK_CFG_CH4_1ST_PINMUX)) {
			/*if (top_reg9.bit.CAP2_CLK == MUX_1) {
				pr_err("PIN_EXTCLK_CFG_CH4_1ST_PINMUX conflict with CAP2_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP3_CLK == MUX_2) {
				pr_err("PIN_EXTCLK_CFG_CH4_1ST_PINMUX conflict with CAP3_2_CLK\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(SGPIO_18,1,func_EXT4_CLK);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.EXT4_CLK = MUX_EN;
			gpio_func_keep(SGPIO_18,1,func_EXT4_CLK);
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_ssp(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_SSP_CFG_NONE) {
	} else {
		//I2S
		if (config & (PIN_SSP_CFG_CH_1ST_PINMUX | PIN_SSP_CFG_CH_1ST_MCLK | PIN_SSP_CFG_CH_1ST_TX | PIN_SSP_CFG_CH_1ST_RX)) {
			if (config & PIN_SSP_CFG_CH_1ST_PINMUX) {
				/*if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_10,2,func_I2S);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S = MUX_EN;
				gpio_func_keep(PGPIO_10,2,func_I2S);
			}
			if (config & PIN_SSP_CFG_CH_1ST_MCLK) {
				/*if (top_reg5.bit.EXT_CLK == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH_1ST_MCLK conflict with EXT_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_22,1,func_I2S_MCLK);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_MCLK = MUX_EN;
				gpio_func_keep(PGPIO_22,1,func_I2S_MCLK);
			}
			if (config & PIN_SSP_CFG_CH_1ST_TX) {
				/*if (top_reg11.bit.I2S4_TX == MUX_2) {
					pr_err("PIN_SSP_CFG_CH_1ST_TX conflict with I2S4_2_TX\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_13,1,func_I2S_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_TX = MUX_EN;
				gpio_func_keep(PGPIO_13,1,func_I2S_TX);
			}
			if (config & PIN_SSP_CFG_CH_1ST_RX) {
				/*if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH_1ST_RX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_12,1,func_I2S_RX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S_RX = MUX_EN;
				gpio_func_keep(PGPIO_12,1,func_I2S_RX);
			}
		}
		//I2S2
		if (config & (PIN_SSP_CFG_CH2_1ST_PINMUX | PIN_SSP_CFG_CH2_1ST_MCLK | PIN_SSP_CFG_CH2_1ST_TX | PIN_SSP_CFG_CH2_1ST_RX)) {
			if (config & PIN_SSP_CFG_CH2_1ST_PINMUX) {
				/*if (top_reg4.bit.I2C3 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH2_1ST_PINMUX conflict with I2C3_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH2_1ST_PINMUX conflict with UART4_3\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH2_1ST_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH2_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_16,2,func_I2S2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2 = MUX_EN;
				gpio_func_keep(PGPIO_16,2,func_I2S2);
			}
			if (config & PIN_SSP_CFG_CH2_1ST_MCLK) {
				/*if (top_reg5.bit.EXT2_CLK == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH2_1ST_MCLK conflict with EXT2_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP_CLK == MUX_2) {
					pr_err("PIN_SSP_CFG_CH2_1ST_MCLK conflict with CAP_2_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH2_1ST_MCLK conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH2_1ST_MCLK conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_23,1,func_I2S2_MCLK);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_MCLK = MUX_EN;
				gpio_func_keep(PGPIO_23,1,func_I2S2_MCLK);
			}
			if (config & PIN_SSP_CFG_CH2_1ST_TX) {
				/*if (top_reg11.bit.I2S3_TX == MUX_2) {
					pr_err("PIN_SSP_CFG_CH2_1ST_TX conflict with I2S3_2_TX\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH2_1ST_TX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg6.bit.PWM3 == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH2_1ST_TX conflict with PWM3\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310_DE == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH2_1ST_TX conflict with LCD310_RGB DE\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_19,1,func_I2S2_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_TX = MUX_EN;
				gpio_func_keep(PGPIO_19,1,func_I2S2_TX);
			}
			if (config & PIN_SSP_CFG_CH2_1ST_RX) {
				/*if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH2_1ST_RX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH2_1ST_RX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_18,1,func_I2S2_RX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S2_RX = MUX_EN;
				gpio_func_keep(PGPIO_18,1,func_I2S2_RX);
			}
		}
		//I2S3
		if (config & (PIN_SSP_CFG_CH3_1ST_PINMUX | PIN_SSP_CFG_CH3_2ND_PINMUX | PIN_SSP_CFG_CH3_3RD_PINMUX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with I2C2_3\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_6) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with UART4_6\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with SDIO_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SPI == MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_1ST_PINMUX conflict with SPI_2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_6,2,func_I2S3_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_1;
				gpio_func_keep(DGPIO_6,2,func_I2S3_1);
			} else if (config & PIN_SSP_CFG_CH3_2ND_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with I2C2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_2) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with UART4_2\r\n");
					return E_OBJ;
				}
				if (top_reg1.bit.SDIO == MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with SDIO\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP3_CLK == MUX_3) {
					pr_err("PIN_SSP_CFG_CH3_2ND_PINMUX conflict with CAP3_3_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_14,2,func_I2S3_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_2;
				gpio_func_keep(PGPIO_14,2,func_I2S3_2);
			} else {
				confl_detect+=gpio_conflict_detect(PGPIO_5,1,func_I2S3_3);
                confl_detect+=gpio_conflict_detect(PGPIO_14,1,func_I2S3_3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3 = MUX_3;
				gpio_func_keep(PGPIO_5,1,func_I2S3_3);
                gpio_func_keep(PGPIO_14,1,func_I2S3_3);
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_MCLK | PIN_SSP_CFG_CH3_3RD_MCLK)) {
			if (config & PIN_SSP_CFG_CH3_1ST_MCLK) {
				confl_detect+=gpio_conflict_detect(DGPIO_5,1,func_I2S3_1_MCLK);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_MCLK = MUX_1;
				gpio_func_keep(DGPIO_5,1,func_I2S3_1_MCLK);
			} else {
			    /*if (top_reg11.bit.I2S2_MCLK == MUX_1) {
					pr_err("PIN_SSP_CFG_CH3_3RD_MCLK conflict with I2S3_2_MCLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_4,1,func_I2S3_3_MCLK);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_MCLK = MUX_3;
				gpio_func_keep(PGPIO_4,1,func_I2S3_3_MCLK);
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_TX | PIN_SSP_CFG_CH3_2ND_TX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_TX) {
				confl_detect+=gpio_conflict_detect(DGPIO_9,1,func_I2S3_1_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_TX = MUX_1;
				gpio_func_keep(DGPIO_9,1,func_I2S3_1_TX);
			} else {
				confl_detect+=gpio_conflict_detect(PGPIO_19,1,func_I2S3_2_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_TX = MUX_2;
			    gpio_func_keep(PGPIO_19,1,func_I2S3_2_TX);
			}
		}
		if (config & (PIN_SSP_CFG_CH3_1ST_RX | PIN_SSP_CFG_CH3_3RD_RX)) {
			if (config & PIN_SSP_CFG_CH3_1ST_RX) {
				confl_detect+=gpio_conflict_detect(DGPIO_8,1,func_I2S3_1_RX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_RX = MUX_1;
				gpio_func_keep(DGPIO_8,1,func_I2S3_1_RX);
			} else {
				confl_detect+=gpio_conflict_detect(PGPIO_15,1,func_I2S3_3_RX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S3_RX = MUX_3;
				gpio_func_keep(DGPIO_8,1,func_I2S3_1_RX);
			}
		}
		//I2S4
		if (config & (PIN_SSP_CFG_CH4_1ST_PINMUX | PIN_SSP_CFG_CH4_1ST_MCLK | PIN_SSP_CFG_CH4_1ST_TX | PIN_SSP_CFG_CH4_1ST_RX | PIN_SSP_CFG_CH4_2ND_PINMUX | PIN_SSP_CFG_CH4_2ND_TX)) {
			if (config & PIN_SSP_CFG_CH4_1ST_PINMUX) {
				confl_detect+=gpio_conflict_detect(JGPIO_1,2,func_I2S4_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4 = MUX_EN;
				gpio_func_keep(JGPIO_1,2,func_I2S4_1);
			}
			if (config & PIN_SSP_CFG_CH4_1ST_MCLK) {
				confl_detect+=gpio_conflict_detect(JGPIO_0,1,func_I2S4_1_MCLK);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4_MCLK = MUX_EN;
				gpio_func_keep(JGPIO_0,1,func_I2S4_1_MCLK);
			}
			if (config & PIN_SSP_CFG_CH4_1ST_TX) {
				confl_detect+=gpio_conflict_detect(JGPIO_4,1,func_I2S4_1_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4_TX = MUX_EN;
				gpio_func_keep(JGPIO_4,1,func_I2S4_1_TX);
			}
			if (config & PIN_SSP_CFG_CH4_1ST_RX) {
				confl_detect+=gpio_conflict_detect(JGPIO_3,1,func_I2S4_1_RX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4_RX = MUX_EN;
				gpio_func_keep(JGPIO_3,1,func_I2S4_1_RX);
			}
			if (config & PIN_SSP_CFG_CH4_2ND_PINMUX) {
				confl_detect+=gpio_conflict_detect(PGPIO_8,2,func_I2S4_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4 = MUX_2;
                gpio_func_keep(PGPIO_8,2,func_I2S4_2);
			}
			if (config & PIN_SSP_CFG_CH4_2ND_TX) {
				/*if (top_reg11.bit.I2S_TX == MUX_EN) {
					pr_err("PIN_SSP_CFG_CH4_2ND_TX conflict with I2S_TX\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(PGPIO_13,1,func_I2S4_2_TX);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg11.bit.I2S4_TX = MUX_2;
                gpio_func_keep(PGPIO_13,1,func_I2S4_2_TX);
			}

		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_lcd(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_LCD_CFG_NONE) {
	} else {
		if (config & (PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX | PIN_LCD_CFG_LCD310_DE_PINMUX)) {
			if (config & PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX){
				/*if (top_reg9.bit.CAP == MUX_EN) {
					pr_err("PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX conflict with CAP0\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP1 == MUX_EN) {
					pr_err("PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX conflict with CAP1\r\n");
					return E_OBJ;
				}
				if (top_reg3.bit.ETH2 == MUX_1) {
					pr_err("PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX conflict with ETH2 RGMII\r\n");
					return E_OBJ;
				}
				if (top_reg3.bit.ETH2 == MUX_2) {
					pr_err("PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX conflict with ETH2 RMII\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.EXT3_CLK == MUX_EN) {
					pr_err("PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX conflict with EXT3_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(SGPIO_0,18,func_LCD310_RGB888);
                confl_detect+=gpio_conflict_detect(PGPIO_10,3,func_LCD310_RGB888);
                confl_detect+=gpio_conflict_detect(PGPIO_14,5,func_LCD310_RGB888);
                confl_detect+=gpio_conflict_detect(PGPIO_23,1,func_LCD310_RGB888);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
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
				top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;

				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
                top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg2.bit.LCD310 = MUX_3;
				gpio_func_keep(SGPIO_0,18,func_LCD310_RGB888);
                gpio_func_keep(PGPIO_10,3,func_LCD310_RGB888);
                gpio_func_keep(PGPIO_14,5,func_LCD310_RGB888);
                gpio_func_keep(PGPIO_23,1,func_LCD310_RGB888);
				if (config & PIN_LCD_CFG_LCD310_DE_PINMUX) {
					/*if (top_reg11.bit.I2S2_TX == MUX_EN) {
						pr_err("PIN_LCD_CFG_LCD310_DE_PINMUX conflict with I2S2_TX\r\n");
						return E_OBJ;
					}
					if (top_reg11.bit.I2S3_TX == MUX_2) {
						pr_err("PIN_LCD_CFG_LCD310_DE_PINMUX conflict with I2S3_2_TX\r\n");
						return E_OBJ;
					}
					if (top_reg1.bit.SDIO == MUX_1) {
						pr_err("PIN_LCD_CFG_LCD310_DE_PINMUX conflict with SDIO\r\n");
						return E_OBJ;
					}
					if (top_reg6.bit.PWM3 == MUX_EN){
						pr_err("PIN_LCD_CFG_LCD310_DE_PINMUX conflict with PWM3\r\n");
						return E_OBJ;
					}*/
					confl_detect+=gpio_conflict_detect(PGPIO_19,1,func_LCD310_DE);
                    if(confl_detect>0)
                    {
                        return E_OBJ;
                    }
					top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
					top_reg2.bit.LCD310_DE = MUX_EN;
					gpio_func_keep(PGPIO_19,1,func_LCD310_DE);
		        }
			}
		}
		if (config & (PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX | PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX)) {
			/*if (top_reg9.bit.CAP == MUX_EN) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX/PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX conflict with CAP0\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP1 == MUX_EN) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX/PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX conflict with CAP1\r\n");
				return E_OBJ;
			}
			if (top_reg3.bit.ETH2 == MUX_1) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX/PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX conflict with ETH2 RGMII\r\n");
				return E_OBJ;
			}
			if (top_reg3.bit.ETH2 == MUX_2) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX/PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX conflict with ETH2 RMII\r\n");
				return E_OBJ;
			}*/
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;

			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			if (config & PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX) {
				confl_detect+=gpio_conflict_detect(SGPIO_0,9,func_LCD310_1);
                confl_detect+=gpio_conflict_detect(SGPIO_10,8,func_LCD310_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg2.bit.LCD310 = MUX_1;
				gpio_func_keep(SGPIO_0,9,func_LCD310_1);
                gpio_func_keep(SGPIO_10,8,func_LCD310_1);
			} else {
				confl_detect+=gpio_conflict_detect(SGPIO_0,9,func_LCD210_1);
                confl_detect+=gpio_conflict_detect(SGPIO_0,9,func_LCD210_1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg2.bit.LCD210 = MUX_1;
				gpio_func_keep(SGPIO_0,9,func_LCD210_1);
                gpio_func_keep(SGPIO_10,8,func_LCD210_1);
			}
		}

		if (config & (PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX | PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX)) {
			/*if ((top_reg3.bit.ETH == MUX_1) || (top_reg3.bit.ETH == MUX_2)) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX/PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX conflict with ETH RGMII/RMII\r\n");
				return E_OBJ;
			}
			if (top_reg7.bit.UART2_CTS_RTS == MUX_1) {
				pr_err("PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX/PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX conflict with UART2_CTS_RTS\r\n");
				return E_OBJ;
			}*/
			top_reg_egpio0.bit.EGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_1 = GPIO_ID_EMUM_FUNC;
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
			top_reg_egpio0.bit.EGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			if (config & PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX) {
				confl_detect+=gpio_conflict_detect(EGPIO_0,16,func_LCD310_2);
                confl_detect+=gpio_conflict_detect(DGPIO_5,1,func_LCD310_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg2.bit.LCD310 = MUX_2;
				gpio_func_keep(EGPIO_0,16,func_LCD310_2);
                gpio_func_keep(DGPIO_5,1,func_LCD310_2);
			} else {
				confl_detect+=gpio_conflict_detect(EGPIO_0,16,func_LCD210_2);
                confl_detect+=gpio_conflict_detect(DGPIO_5,1,func_LCD210_2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg2.bit.LCD210 = MUX_2;
				gpio_func_keep(EGPIO_0,16,func_LCD210_2);
                gpio_func_keep(DGPIO_5,1,func_LCD210_2);
			}

		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_remote(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_REMOTE_CFG_NONE) {
	} else {
		if (config & PIN_REMOTE_CFG_1ST_PINMUX) {
			confl_detect+=gpio_conflict_detect(JGPIO_1,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 1;
			gpio_func_keep(JGPIO_1,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_2ND_PINMUX) {
			confl_detect+=gpio_conflict_detect(JGPIO_2,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 2;
			gpio_func_keep(JGPIO_2,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_3RD_PINMUX) {
			confl_detect+=gpio_conflict_detect(JGPIO_3,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 3;
			gpio_func_keep(JGPIO_3,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_4TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(JGPIO_4,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 4;
			gpio_func_keep(JGPIO_4,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_5TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(PGPIO_4,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 5;
			gpio_func_keep(PGPIO_4,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_6TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(PGPIO_5,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 6;
			gpio_func_keep(PGPIO_5,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_7TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(DGPIO_0,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 7;
			gpio_func_keep(DGPIO_0,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_8TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(DGPIO_1,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 8;
			gpio_func_keep(DGPIO_1,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_9TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(DGPIO_2,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 9;
			gpio_func_keep(DGPIO_2,1,func_IRDA);
		} else if (config & PIN_REMOTE_CFG_10TH_PINMUX) {
			confl_detect+=gpio_conflict_detect(DGPIO_3,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 10;
			gpio_func_keep(DGPIO_3,1,func_IRDA);
		} else {
			confl_detect+=gpio_conflict_detect(DGPIO_4,1,func_IRDA);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.IRDA = 11;
			gpio_func_keep(DGPIO_4,1,func_IRDA);
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_vcap(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		//CAP0
		if (config & (PIN_VCAP_CFG_CAP0_1ST_PINMUX | PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP0_1ST_PINMUX) {
				/*if (top_reg3.bit.ETH2 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP0_1ST_PINMUX conflict with ETH2_RGMII\r\n");
					return E_OBJ;
				}
				if (top_reg3.bit.ETH2 == MUX_2) {
					pr_err("PIN_VCAP_CFG_CAP0_1ST_PINMUX conflict with ETH2_RMII\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP0_1ST_PINMUX conflict with LCD310_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP0_1ST_PINMUX conflict with LCD210_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_VCAP_CFG_CAP0_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(SGPIO_1,8,func_CAP);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP = MUX_EN;
				gpio_func_keep(SGPIO_1,8,func_CAP);
			}
			if (config & PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP_CLK = MUX_1;
				gpio_func_keep(SGPIO_0,1,func_CAP_CLK1);
			} else if (config & PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX) {
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP_CLK = MUX_2;
				gpio_func_keep(PGPIO_23,1,func_CAP_CLK2);
			}
		}
		//CAP1
		if (config & (PIN_VCAP_CFG_CAP1_1ST_PINMUX | PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX)){
			if (config & PIN_VCAP_CFG_CAP1_1ST_PINMUX) {
				/*if (top_reg3.bit.ETH2 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP1_1ST_PINMUX conflict with ETH2_RGMII\r\n");
					return E_OBJ;
				}
				if (top_reg3.bit.ETH2 == MUX_2) {
					pr_err("PIN_VCAP_CFG_CAP1_1ST_PINMUX conflict with ETH2_RMII\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP1_1ST_PINMUX conflict with LCD310_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_1) {
					pr_err("PIN_VCAP_CFG_CAP1_1ST_PINMUX conflict with LCD210_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_VCAP_CFG_CAP1_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(SGPIO_10,8,func_CAP1);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP1 = MUX_EN;
				gpio_func_keep(SGPIO_10,8,func_CAP1);
			}
			if (config & PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP1_CLK = MUX_1;
				gpio_func_keep(SGPIO_9,1,func_CAP1_CLK1);
			} else  if (config & PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP1_CLK = MUX_2;
				gpio_func_keep(SGPIO_0,1,func_CAP1_CLK2);
			}
		}
		//CAP2
		if (config & (PIN_VCAP_CFG_CAP2_1ST_PINMUX | PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP2_1ST_PINMUX) {
				confl_detect+=gpio_conflict_detect(SGPIO_19,8,func_CAP2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP2 = MUX_EN;
				gpio_func_keep(SGPIO_19,8,func_CAP2);
			}
			if (config & PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP2_CLK = MUX_1;
				gpio_func_keep(SGPIO_18,1,func_CAP2_CLK1);
			} else if (config & PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP2_CLK = MUX_2;
				gpio_func_keep(SGPIO_9,1,func_CAP2_CLK2);
			}
		}
		//CAP3
		if (config & (PIN_VCAP_CFG_CAP3_1ST_PINMUX | PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX | PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX | PIN_VCAP_CFG_CAP3_CLK_3RD_PINMUX)) {
			if (config & PIN_VCAP_CFG_CAP3_1ST_PINMUX) {
				confl_detect+=gpio_conflict_detect(SGPIO_28,8,func_CAP3);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_29 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_30 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_31 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_32 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_33 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_34 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio1.bit.SGPIO_35 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP3 = MUX_EN;
				gpio_func_keep(SGPIO_28,8,func_CAP3);
			}
			if (config & PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_27 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP3_CLK = MUX_1;
				gpio_func_keep(SGPIO_27,1,func_CAP3_CLK1);
			} else if (config & PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX) {
				top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP3_CLK = MUX_2;
				gpio_func_keep(SGPIO_18,1,func_CAP3_CLK2);
			} else if (config & PIN_VCAP_CFG_CAP3_CLK_3RD_PINMUX){
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg9.bit.CAP3_CLK = MUX_3;
				gpio_func_keep(SGPIO_14,1,func_CAP3_CLK3);
			}
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_eth(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & (PIN_ETH_CFG_RGMII_1ST_PINMUX | PIN_ETH_CFG_RMII_1ST_PINMUX)) {
			if (config & PIN_ETH_CFG_RGMII_1ST_PINMUX) {
				/*if (top_reg2.bit.LCD310 == MUX_2) {
					pr_err("PIN_ETH_CFG_RGMII_1ST_PINMUX conflict with LCD310_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_2) {
					pr_err("PIN_ETH_CFG_RGMII_1ST_PINMUX conflict with LCD210_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART2_CTS_RTS == MUX_1) {
					pr_err("PIN_ETH_CFG_RGMII_1ST_PINMUX conflict with UART2_CTS\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(EGPIO_2,12,func_ETH);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
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
				gpio_func_keep(EGPIO_2,12,func_ETH);
			} else if (config & PIN_ETH_CFG_RMII_1ST_PINMUX) {
				/*if (top_reg2.bit.LCD310 == MUX_2) {
					pr_err("PIN_ETH_CFG_RMII_1ST_PINMUX conflict with LCD310_2_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_2) {
					pr_err("PIN_ETH_CFG_RMII_1ST_PINMUX conflict with LCD210_2_BT1120\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(EGPIO_2,4,func_ETH);
                confl_detect+=gpio_conflict_detect(EGPIO_9,3,func_ETH);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_egpio0.bit.EGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_egpio0.bit.EGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH = MUX_2;
				gpio_func_keep(EGPIO_2,4,func_ETH);
                gpio_func_keep(EGPIO_9,3,func_ETH);
			}
		}
		if (config & PIN_ETH_CFG_REFCLK_PINMUX) {
			confl_detect+=gpio_conflict_detect(EGPIO_0,1,func_ETH);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_egpio0.bit.EGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg3.bit.ETH_REFCLK = MUX_EN;
			gpio_func_keep(EGPIO_0,1,func_ETH);
		}
		if (config & PIN_ETH_CFG_MDC_MDIO_PINMUX) {
			/*if (top_reg2.bit.LCD310 == MUX_2) {
				pr_err("PIN_ETH_CFG_MDC_MDIO_PINMUX conflict with LCD310_2_BT1120\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD210 == MUX_2) {
				pr_err("PIN_ETH_CFG_MDC_MDIO_PINMUX conflict with LCD210_2_BT1120\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(EGPIO_14,2,func_ETH);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_egpio0.bit.EGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg3.bit.ETH_MDC_MDIO = MUX_1;
			gpio_func_keep(EGPIO_14,2,func_ETH);
		}
		if (config & PIN_ETH_CFG_ACT_LED_1ST_PINMUX) {
			/*if (top_reg4.bit.I2C2 == MUX_4) {
				pr_err("PIN_ETH_CFG_ACT_LED_1ST_PINMUX conflict with I2C2_4\r\n");
			}
			if (top_reg11.bit.I2S4 == MUX_EN) {
				pr_err("PIN_ETH_CFG_ACT_LED_1ST_PINMUX conflict with I2S4\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SPI == MUX_1) {
				pr_err("PIN_ETH_CFG_ACT_LED_1ST_PINMUX conflict with SPI\r\n");
				return E_OBJ;
			}
			if ((top_reg5.bit.IRDA == MUX_1)) {
				pr_err("PIN_MISC_CFG_SATA_LED_1ST_PINMUX conflict with IRDA0\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(JGPIO_1,1,func_ETH);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.ETH_ACT_LED = MUX_1;
			gpio_func_keep(JGPIO_1,1,func_ETH);
		}
		if (config & PIN_ETH_CFG_LINK_LED_1ST_PINMUX) {
			/*if (top_reg11.bit.I2S4 == MUX_EN) {
				pr_err("PIN_ETH_CFG_LINK_LED_1ST_PINMUX conflict with I2S4\r\n");
				return E_OBJ;
			}
			if ((top_reg5.bit.IRDA == MUX_2)) {
				pr_err("PIN_ETH_CFG_LINK_LED_1ST_PINMUX conflict with IRDA1\r\n");
				return E_OBJ;
			}*/
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.ETH_LINK_LED = MUX_1;
			gpio_func_keep(JGPIO_2,1,func_ETH);
		}


		//ETH2
		if (config & (PIN_ETH2_CFG_RGMII_1ST_PINMUX | PIN_ETH2_CFG_RMII_1ST_PINMUX)) {
			if (config & PIN_ETH2_CFG_RGMII_1ST_PINMUX) {
				/*if (top_reg9.bit.CAP == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with CAP0\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP1 == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with CAP1\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP1_CLK == MUX_1) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with CAP1_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP2_CLK == MUX_2) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with CAP2_2_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_1) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with LCD310_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_1) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with LCD210_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.EXT3_CLK == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RGMII_1ST_PINMUX conflict with EXT3_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(SGPIO_2,12,func_ETH2);
                if(confl_detect>0)
                {
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
				top_reg3.bit.ETH2 = MUX_1;
				gpio_func_keep(SGPIO_2,12,func_ETH2);
			} else {
				/*if (top_reg9.bit.CAP == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with CAP0\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP1 == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with CAP1\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP1_CLK == MUX_1) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with CAP1_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg9.bit.CAP2_CLK == MUX_2) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with CAP2_2_CLK\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_1) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with LCD310_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD210 == MUX_1) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with LCD210_BT1120\r\n");
					return E_OBJ;
				}
				if (top_reg2.bit.LCD310 == MUX_3) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with LCD310_RGB\r\n");
					return E_OBJ;
				}
				if (top_reg5.bit.EXT3_CLK == MUX_EN) {
					pr_err("PIN_ETH2_CFG_RMII_1ST_PINMUX conflict with EXT3_CLK\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(SGPIO_2,4,func_ETH2);
                confl_detect+=gpio_conflict_detect(SGPIO_9,3,func_ETH2);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg3.bit.ETH2 = MUX_2;
				gpio_func_keep(SGPIO_2,4,func_ETH2);
                gpio_func_keep(SGPIO_9,3,func_ETH2);
			}
		}
		if (config & PIN_ETH2_CFG_REFCLK_PINMUX) {
			/*if (top_reg9.bit.CAP_CLK == MUX_1) {
				pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with CAP0_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg9.bit.CAP1_CLK == MUX_2) {
				pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with CAP1_2_CLK\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_1) {
				pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with LCD310_BT1120\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD210 == MUX_1) {
				pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with LCD210_BT1120\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_3) {
				pr_err("PIN_ETH2_CFG_REFCLK_PINMUX conflict with LCD310_RGB\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(SGPIO_0,1,func_ETH2);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg3.bit.ETH2_REFCLK = MUX_EN;
			gpio_func_keep(SGPIO_0,1,func_ETH2);
		}
		if (config & PIN_ETH2_CFG_MDC_MDIO_PINMUX) {
			/*if (top_reg9.bit.CAP1 == MUX_EN) {
				pr_err("PIN_ETH2_CFG_MDC_MDIO_PINMUX conflict with CAP1\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_1) {
				pr_err("PIN_ETH2_CFG_MDC_MDIO_PINMUX conflict with LCD310_BT1120\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD210 == MUX_1) {
				pr_err("PIN_ETH2_CFG_MDC_MDIO_PINMUX conflict with LCD210_BT1120\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310 == MUX_3) {
				pr_err("PIN_ETH2_CFG_MDC_MDIO_PINMUX conflict with LCD310_RGB\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(SGPIO_14,2,func_ETH2);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg3.bit.ETH2_MDC_MDIO = MUX_EN;
			gpio_func_keep(SGPIO_14,2,func_ETH2);
		}
		if (config & PIN_ETH_CFG_ACT_LED_2ND_PINMUX) {
			/*if (top_reg7.bit.UART4 == MUX_6) {
				pr_err("PIN_ETH_CFG_ACT_LED_2ND_PINMUX conflict with UART4_6\r\n");
			}
			if (top_reg11.bit.I2S3_RX == MUX_EN) {
				pr_err("PIN_ETH_CFG_ACT_LED_2ND_PINMUX conflict with I2S3_RX\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_2) {
				pr_err("PIN_ETH_CFG_ACT_LED_2ND_PINMUX conflict with SDIO_2\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(DGPIO_8,1,func_ETH2);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.ETH_ACT_LED = MUX_2;
			gpio_func_keep(DGPIO_8,1,func_ETH2);
		}
		if (config & PIN_ETH_CFG_LINK_LED_2ND_PINMUX) {
			if (top_reg11.bit.I2S3_TX == MUX_EN) {
				pr_err("PIN_ETH_CFG_LINK_LED_2ND_PINMUX conflict with I2S3_TX\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_2) {
				pr_err("PIN_ETH_CFG_LINK_LED_2ND_PINMUX conflict with SDIO_2\r\n");
				return E_OBJ;
			}
			confl_detect+=gpio_conflict_detect(DGPIO_9,1,func_ETH2);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.ETH_LINK_LED = MUX_2;
            gpio_func_keep(DGPIO_9,1,func_ETH2);
		}

		if (config & PIN_ETH_CFG_PHY_SEL) {
			top_reg3.bit.ETH_PHY_SEL = 1;
		} else {
			top_reg3.bit.ETH_PHY_SEL = 0;
		}

	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_misc(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & PIN_MISC_CFG_CPU_ICE) {
			confl_detect+=gpio_conflict_detect(JGPIO_0,5,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.EJTAG_EN = MUX_EN;
			gpio_func_keep(JGPIO_0,5,func_misc);

		}
		if (config & PIN_MISC_CFG_BMC) {
			confl_detect+=gpio_conflict_detect(DGPIO_5,4,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg1.bit.EXTROM_EN = MUX_EN;
			gpio_func_keep(DGPIO_5,4,func_misc);
		}
		if (config & PIN_MISC_CFG_RTC_CAL_OUT) {
			confl_detect+=gpio_conflict_detect(PGPIO_2,1,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.RTC_CAL = MUX_EN;
			gpio_func_keep(PGPIO_2,1,func_misc);
		}
		if (config & PIN_MISC_CFG_DAC_RAMP_TP) {
			confl_detect+=gpio_conflict_detect(PGPIO_5,1,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg5.bit.TVDAC_TEST_CLK = MUX_EN;
			gpio_func_keep(PGPIO_5,1,func_misc);
		}
		if (config & PIN_MISC_CFG_HDMI_HOTPLUG) {
			confl_detect+=gpio_conflict_detect(PGPIO_31,1,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_31,1,func_misc);
		}
		if (config & PIN_MISC_CFG_VGA_HS) {
			confl_detect+=gpio_conflict_detect(PGPIO_27,1,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_27,1,func_misc);
		}
		if (config & PIN_MISC_CFG_VGA_VS) {
			confl_detect+=gpio_conflict_detect(PGPIO_28,1,func_misc);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_28,1,func_misc);
		}
		if (config & (PIN_MISC_CFG_SATA_LED_1ST_PINMUX | PIN_MISC_CFG_SATA_LED_2ND_PINMUX)) {
			if (config & PIN_MISC_CFG_SATA_LED_1ST_PINMUX) {
				/*if (top_reg7.bit.UART4 == MUX_1) {
					pr_err("PIN_MISC_CFG_SATA_LED_1ST_PINMUX conflict with UART4\r\n");
				}
				if (top_reg11.bit.I2S4_RX == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA_LED_1ST_PINMUX conflict with I2S4_RX\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_3)) {
					pr_err("PIN_MISC_CFG_SATA_LED_1ST_PINMUX conflict with IRDA2\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(JGPIO_3,1,func_misc);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA_LED = MUX_1;
				gpio_func_keep(JGPIO_3,1,func_misc);
			} else if (config & PIN_MISC_CFG_SATA_LED_2ND_PINMUX) {
				/*if (top_reg4.bit.I2C2 == MUX_2) {
					pr_err("PIN_MISC_CFG_SATA_LED_2ND_PINMUX conflict with I2C2_2\r\n");
					return E_OBJ;
				}
				if (top_reg7.bit.UART4 == MUX_5) {
					pr_err("PIN_MISC_CFG_SATA_LED_2ND_PINMUX conflict with UART4_5\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_A)) {
					pr_err("PIN_MISC_CFG_SATA_LED_2ND_PINMUX conflict with IRDA9\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_3,1,func_misc);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA_LED = MUX_2;
				gpio_func_keep(DGPIO_3,1,func_misc);
			}
		}
		if (config & (PIN_MISC_CFG_SATA2_LED_1ST_PINMUX | PIN_MISC_CFG_SATA2_LED_2ND_PINMUX)) {
			if (config & PIN_MISC_CFG_SATA2_LED_1ST_PINMUX) {
				/*if (top_reg7.bit.UART4 == MUX_1) {
					pr_err("PIN_MISC_CFG_SATA2_LED_1ST_PINMUX conflict with UART4\r\n");
				}
				if (top_reg11.bit.I2S4_TX == MUX_EN) {
					pr_err("PIN_MISC_CFG_SATA2_LED_1ST_PINMUX conflict with I2S4_TX\r\n");
					return E_OBJ;
				}
				if ((top_reg5.bit.IRDA == MUX_4)) {
					pr_err("PIN_MISC_CFG_SATA2_LED_1ST_PINMUX conflict with IRDA3\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(JGPIO_4,1,func_misc);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA2_LED = MUX_1;
				gpio_func_keep(JGPIO_4,1,func_misc);
			} else if (config & PIN_MISC_CFG_SATA2_LED_2ND_PINMUX) {
				/*if ((top_reg5.bit.IRDA == MUX_B)) {
					pr_err("PIN_MISC_CFG_SATA2_LED_2ND_PINMUX conflict with IRDA10\r\n");
					return E_OBJ;
				}*/
				confl_detect+=gpio_conflict_detect(DGPIO_4,1,func_misc);
                if(confl_detect>0)
                {
                    return E_OBJ;
                }
				top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg5.bit.SATA2_LED = MUX_2;
				gpio_func_keep(DGPIO_4,1,func_misc);
			}
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


ER pinmux_config_pwm(uint32_t config)
{
	//unsigned long flags = 0;

	//loc_cpu(flags);

	if (config == PIN_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PWM_CFG_CH_1ST_PINMUX) {
			confl_detect+=gpio_conflict_detect(PGPIO_24,1,func_PWM);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM = MUX_EN;
			gpio_func_keep(PGPIO_24,1,func_PWM);
		}
		if (config & PIN_PWM_CFG_CH2_1ST_PINMUX) {
			confl_detect+=gpio_conflict_detect(PGPIO_25,1,func_PWM2);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM2 = MUX_EN;
			gpio_func_keep(PGPIO_25,1,func_PWM2);
		}
		if (config & PIN_PWM_CFG_CH3_1ST_PINMUX) {
			/*if (top_reg11.bit.I2S2_TX == MUX_EN) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with I2S2_TX\r\n");
				return E_OBJ;
			}
			if (top_reg11.bit.I2S3_TX == MUX_2) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with I2S3_2_TX\r\n");
				return E_OBJ;
			}
			if (top_reg1.bit.SDIO == MUX_1) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with SDIO\r\n");
				return E_OBJ;
			}
			if (top_reg2.bit.LCD310_DE == MUX_EN) {
				pr_err("PIN_PWM_CFG_CH3_1ST_PINMUX conflict with LCD310_DE\r\n");
				return E_OBJ;
			}*/
			confl_detect+=gpio_conflict_detect(PGPIO_19,1,func_PWM3);
            if(confl_detect>0)
            {
                return E_OBJ;
            }
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg6.bit.PWM3 = MUX_EN;
			gpio_func_keep(PGPIO_19,1,func_PWM3);
		}
	}

	//unl_cpu(flags);

	return E_OK;
}


#if 0
ER pinmux_config_mipi(uint32_t config)
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
#endif
int pinmux_config_vcap_interal(uint32_t config)
{
	return E_OK;
}
typedef int (*PINMUX_CONFIG_HDL)(uint32_t);
static PINMUX_CONFIG_HDL pinmux_config_hdl[] =
{
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
	pinmux_config_vcap_interal
};

/**
	Configure pinmux controller

	Configure pinmux controller by upper layer

	@param[in] info	nvt_pinctrl_info
	@return void
*/

ER pinmux_init(struct nvt_pinctrl_info *info)
{
	unsigned long i;
	int err;
	unsigned long flags = 0;

	//pr_info("%s, %s\r\n", __func__, DRV_VERSION);

	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = 0;
	top_reg9.reg = 0;
	top_reg11.reg = 0;
	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_jgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_egpio0.reg = 0xFFFFFFFF;
	top_reg_dgpio0.reg = 0xFFFFFFFF;
	top_reg_sgpio0.reg = 0xFFFFFFFF;
	top_reg_sgpio1.reg = 0xFFFFFFFF;
	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_jgpio0.reg = 0x0000000;
	} else {
		top_reg_jgpio0.reg = 0x000001F;
	}
	//clean dump table
    for(i=0;i<GPIO_total;i++)
    {
        dump_gpio_func[i]=0;
    }
	/*Enter critical section*/
	loc_cpu(flags);

	for (i = 0; i < PIN_FUNC_MAX; i++) {
		if (info->top_pinmux[i].pin_function != i) {
			pr_err("top_config[%ld].pinFunction context error\n", i);
			/*Leave critical section*/
			unl_cpu(flags);
			return E_CTX;
		}

		err = pinmux_config_hdl[i](info->top_pinmux[i].config);
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
	//TOP_SETREG(info, TOP_REG10_OFS, top_reg10.reg);
	TOP_SETREG(info, TOP_REG11_OFS, top_reg11.reg);
	//TOP_SETREG(info, TOP_REG12_OFS, top_reg12.reg);

	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGJGPIO0_OFS, top_reg_jgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	//TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGEGPIO0_OFS, top_reg_egpio0.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO0_OFS, top_reg_sgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO1_OFS, top_reg_sgpio1.reg);
	//TOP_SETREG(info, TOP_REGSGPIO2_OFS, top_reg_sgpio2.reg);
	//TOP_SETREG(info, TOP_REGBGPIO0_OFS, top_reg_bgpio0.reg);

	/*Leave critical section*/
	unl_cpu(flags);

	return E_OK;
}
/*void print_desh(int count,int idx,int func)
{
    int desh=15;
    int j=0;
    if(idx)
    {
        desh=desh-2;
        if(count>=9)
        {
            desh=desh-1;
        }
    }
    else
    {
        if(count>9)
        {
            desh=desh-1;
        }
    }
    if(func)
    {
        for(j=0;j<desh;j++)
        {
            printk("-");
        }
    }
    else
    {
        for(j=0;j<desh;j++)
        {
            printk("-");
        }
    }
}*/
void gpio_func_show(void)
{
   int i=0;

   int gpio_count=0;
   int func_num;
   //print C_GPIO
   for(i=CGPIO_0;i<C_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {
            if(gpio_count<10)
            {
                printk("\033[0;32mC_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mC_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }


        }
        else
        {
            if(gpio_count<10)
            {
                printk("C_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("C_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //J_GPIO
   gpio_count=0;
   for(i=JGPIO_0;i<J_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mJ_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mJ_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }

        }
        else
        {
            if(gpio_count<10)
            {
                printk("J_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("J_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //P_GPIO
   gpio_count=0;
   for(i=PGPIO_0;i<P_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

           if(gpio_count<10)
            {
                printk("\033[0;32mP_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mP_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
           if(gpio_count<10)
            {
                printk("P_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("P_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //E_GPIO
   gpio_count=0;
   for(i=EGPIO_0;i<E_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mE_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mE_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("E_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("E_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //D_GPIO
   gpio_count=0;
   for(i=DGPIO_0;i<D_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

           if(gpio_count<10)
            {
                printk("\033[0;32mD_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mD_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
             if(gpio_count<10)
            {
                printk("D_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("D_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //S_GPIO
   gpio_count=0;
   for(i=SGPIO_0;i<S_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mS_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mS_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
             if(gpio_count<10)
            {
                printk("S_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("S_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
}
EXPORT_SYMBOL(gpio_func_show);
