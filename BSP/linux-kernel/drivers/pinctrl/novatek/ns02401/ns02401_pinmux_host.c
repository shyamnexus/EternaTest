/*
    Pinmux module driver.

    This file is the driver of Piumux module.

    @file       ns02401_pinmux_host.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#include "ns02401_pinmux.h"
#include <plat/pad.h>


static DEFINE_SPINLOCK(top_lock);
#define loc_cpu(flags) spin_lock_irqsave(&top_lock, flags)
#define unl_cpu(flags) spin_unlock_irqrestore(&top_lock, flags)

union TOP_REG0         top_reg0;
union TOP_REG1         top_reg1;
union TOP_REG2         top_reg2;
union TOP_REG3         top_reg3;
union TOP_REG4         top_reg4;
union TOP_REG5         top_reg5;
union TOP_REG6         top_reg6;
union TOP_REG7         top_reg7;
union TOP_REG8         top_reg8;
union TOP_REG9         top_reg9;
union TOP_REG10        top_reg10;
union TOP_REG11        top_reg11;
//union TOP_REG12        top_reg12;
union TOP_REG13        top_reg13;
union TOP_REG14        top_reg14;

union TOP_REGCGPIO0    top_reg_cgpio0;
union TOP_REGJGPIO0    top_reg_jgpio0;
union TOP_REGPGPIO0    top_reg_pgpio0;
union TOP_REGPGPIO1    top_reg_pgpio1;
union TOP_REGEGPIO0    top_reg_egpio0;
union TOP_REGDGPIO0    top_reg_dgpio0;
union TOP_REGBGPIO0    top_reg_bgpio0;
union TOP_REGAGPIO0    top_reg_agpio0;


int confl_detect;
static int dump_gpio_func[GPIO_total] = {0};
static char *dump_func[FUNC_total] = {
	/* 0x04 Storage */
	"FSPI","SDIO","SDIO_2","SDIO_3","SDIO2","EJTAG","EXTROM",
	/* 0x0C ETH */
	"ETH","ETH2",
	/* 0x10 I2C */
	"I2C","I2C2","I2C3","I2C3_2","I2C3_3","I2C3_4","I2C4","I2C4_2","I2C4_3","I2C4_4",
	"I2C5","I2C5_2","I2C5_3","I2C5_4","I2C6","I2C7","I2C8",
	/* 0x18 PWM */
	"PWM","PWM2","PWM3","PWM4","PWM5","PWM6",
	/* 0x30 I2S / HDMI_CEC */
	"I2S","I2S_MCLK","I2S_2","I2S_2_MCLK","I2S2","I2S2_MCLK","I2S2_2","I2S2_2_MCLK","I2S2_3","I2S2_3_MCLK",
	"I2S3","I2S3_MCLK","I2S4","I2S4_MCLK","HDMI_CEC","HDMI2_CEC","HDMI3_CEC",
	/* 0x34 UART */
	"UART","UART_2","UART2","UART2_2","UART3","UART4","UART4_2","UART4_3","UART5","UART5_2",
	"UART_RTSCTS","UART_2_RTSCTS","UART2_RTSCTS","UART2_2_RTSCTS","UART3_RTSCTS",
	/* 0x40 Remote / SDP */
	"Remote","SDP","SDP_2","SDP_RDY","SDP_2_RDY",
	/* 0x44  SPI */
	"SPI","SPI2","SPI2_2","SPI2_3","SPI2_4","SPI_RDY","SPI2_RDY",

	"MISC","LCD"
};


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
				top_reg_addr = ioremap(top_pa, 0x100);
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

/*#define TOP_ROMVER_A            0x00000102
#define TOP_ROMVER_B            0x00000103
uint32_t nvt_get_chip_ver(void)
    {
    void __iomem *top_reg_addr = NULL;
    unsigned int chip_ver;
    phys_addr_t top_pa = NVT_PERIPHERAL_PHYS_BASE+0x7FFC;

    top_reg_addr = ioremap_nocache(top_pa, 0x10);

    if (top_reg_addr) {
        chip_ver =  INW(top_reg_addr);
        iounmap(top_reg_addr);
    } else {
        pr_err("invalid address\n");
        return -ENOMEM;
    }

    if(chip_ver == TOP_ROMVER_A) {
        return CHIPVER_A;
    } else if(chip_ver == TOP_ROMVER_B) {
        return CHIPVER_B;
    } else {
        pr_info("force to Ver B\n");
        return CHIPVER_B;
    }
}*/


static void gpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char *gpio_name[] = {"C_GPIO", "J_GPIO", "P_GPIO", "E_GPIO", "D_GPIO", "B_GPIO", "A_GPIO"};
	char name[10];

	if (start_offset == TOP_REGCGPIO0_OFS) {
		strcpy(name, gpio_name[0]);
	} else if (start_offset == TOP_REGJGPIO0_OFS) {
		strcpy(name, gpio_name[1]);
	} else if (start_offset == TOP_REGPGPIO0_OFS) {
		strcpy(name, gpio_name[2]);
	} else if (start_offset == TOP_REGEGPIO0_OFS) {
		strcpy(name, gpio_name[3]);
	} else if (start_offset == TOP_REGDGPIO0_OFS) {
		strcpy(name, gpio_name[4]);
	} else if (start_offset == TOP_REGBGPIO0_OFS) {
		strcpy(name, gpio_name[5]);
	} else if (start_offset == TOP_REGAGPIO0_OFS) {
		strcpy(name, gpio_name[6]);
	}

	if (gpio_number > 0x20) {
		reg_value = TOP_GETREG(info, start_offset);

		for (i = 0; i < 0x20; i++) {
			if (reg_value & (1 << i)) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else {
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
			}
		}

		reg_value = TOP_GETREG(info, start_offset + 0x4);

		for (j = 0; j < (gpio_number - 0x20); j++) {
			if (reg_value & (1 << j)) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else {
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
			}
			i++;
		}
	} else {
		reg_value = TOP_GETREG(info, start_offset);
		for (i = 0; i < gpio_number; i++) {
			if (reg_value & (1 << i)) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else {
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
			}
		}
	}
}

void pinmux_gpio_parsing(struct nvt_pinctrl_info *info)
{
	pr_info("\n[PIN]       [NO]      [STATUS]\n");

	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, J_GPIO_NUM, TOP_REGJGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, E_GPIO_NUM, TOP_REGEGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
	gpio_info_show(info, B_GPIO_NUM, TOP_REGBGPIO0_OFS);
	gpio_info_show(info, A_GPIO_NUM, TOP_REGAGPIO0_OFS);
}

static uint32_t disp_pinmux_config[] = {
	PINMUX_DISPMUX_SEL_NONE,            // LCD
	PINMUX_DISPMUX_SEL_NONE,            // LCD2
	PINMUX_TV_HDMI_CFG_NORMAL,          // TV
	PINMUX_TV_HDMI_CFG_NORMAL           // HDMI
};

static uint32_t adc_en = MUX_0;

void pinmux_preset(struct nvt_pinctrl_info *info)
{
}

/**
    Get Display PINMUX setting

    Display driver (LCD/TV/HDMI) can get mode setting from pinmux_init()

    @param[in] id   LCD ID
            - @b PINMUX_DISP_ID_LCD: 1st LCD
            - @b PINMUX_DISP_ID_LCD2: 2nd LCD
            - @b PINMUX_DISP_ID_TV: TV
            - @b PINMUX_DISP_ID_HDMI: HDMI

    @return LCD pinmux setting
*/
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id)
{
	if (id <= PINMUX_FUNC_ID_LCD) {
		return disp_pinmux_config[id] & ~(PINMUX_DISPMUX_SEL_MASK | PINMUX_LCD_SEL_FEATURE_MSK);
	} else if (id <= PINMUX_FUNC_ID_HDMI) {
		return disp_pinmux_config[id] & ~PINMUX_HDMI_CFG_MASK;
	}

	return 0;
}
EXPORT_SYMBOL(pinmux_get_dispmode);


/**
    Read pinmux data from controller base

    @param[in] info nvt_pinctrl_info
*/
void pinmux_parsing(struct nvt_pinctrl_info *info)
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
//	union TOP_REG12 local_top_reg12;
	union TOP_REG13 local_top_reg13;
	union TOP_REG14 local_top_reg14;

	union TOP_REGJGPIO0 local_top_reg_jgpio0;

//	union TOP_REGSGPIO0 local_top_reg_sgpio0;
//	union TOP_REGHSIGPIO0 local_top_reg_hgpio0;

	pr_info("%s enter \r\n", __func__);

	/* Enter critical section */
	loc_cpu(flags);

	local_top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
//	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
//	local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg13.reg = TOP_GETREG(info, TOP_REG13_OFS);
	local_top_reg14.reg = TOP_GETREG(info, TOP_REG14_OFS);


	local_top_reg_jgpio0.reg = TOP_GETREG(info, TOP_REGJGPIO0_OFS);
//	local_top_reg_sgpio0.reg = TOP_GETREG(info, TOP_REGSGPIO0_OFS);
//	local_top_reg_hgpio0.reg = TOP_GETREG(info, TOP_REGHSIGPIO0_OFS);

	/* Parsing SDIO */
	value = PIN_SDIO_CFG_NONE;

	if (local_top_reg1.bit.SDIO_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO_1;
	} else if (local_top_reg1.bit.SDIO_EN == MUX_2) {
		value |= PIN_SDIO_CFG_SDIO_2;
	} else if (local_top_reg1.bit.SDIO_EN == MUX_3) {
		value |= PIN_SDIO_CFG_SDIO_3;
	}

	if (local_top_reg1.bit.SDIO2_EN == MUX_1)  {
		value |= PIN_SDIO_CFG_SDIO2_1;
	}
	if (local_top_reg1.bit.SDIO2_BUS_WIDTH == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO2_BUS_WIDTH;
	}
	if (local_top_reg1.bit.SDIO2_DS_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO2_DS;
	}

	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/* Parsing NAND */
	value = PIN_NAND_CFG_NONE;

	if (local_top_reg1.bit.FSPI_EN == MUX_1) {
		value |= PIN_NAND_CFG_NAND_1;
	} else if (local_top_reg1.bit.FSPI_EN == MUX_2) {
		value |= PIN_NAND_CFG_NAND_8BIT;
	}

	if (local_top_reg1.bit.FSPI_CS1_EN) {
		value |= PIN_NAND_CFG_NAND_CS1;
	}

	info->top_pinmux[PIN_FUNC_NAND].config = value;
	info->top_pinmux[PIN_FUNC_NAND].pin_function = PIN_FUNC_NAND;

	/* Parsing ETH */
	value = PIN_ETH_CFG_NONE;

	if (local_top_reg3.bit.ETH == MUX_1) {
		value |= PIN_ETH_CFG_ETH_RGMII_1;
	} else if (local_top_reg3.bit.ETH == MUX_2) {
		value |= PIN_ETH_CFG_ETH_RMII_1;
	} else if (local_top_reg3.bit.ETH == MUX_3) {
		value |= PIN_ETH_CFG_ETH_SGMII_1;
	}

	if (local_top_reg3.bit.ETH_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH_EXTPHYCLK;
	}

	if (local_top_reg3.bit.ETH2 == MUX_1) {
		value |= PIN_ETH_CFG_ETH2_RGMII_1;
	} else if (local_top_reg3.bit.ETH2 == MUX_2) {
		value |= PIN_ETH_CFG_ETH2_RMII_1;
	} else 	if (local_top_reg3.bit.ETH2 == MUX_3) {
		value |= PIN_ETH_CFG_ETH2_SGMII_1;
	}

	if (local_top_reg3.bit.ETH2_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH2_EXTPHYCLK;
	}

	info->top_pinmux[PIN_FUNC_ETH].config = value;
	info->top_pinmux[PIN_FUNC_ETH].pin_function = PIN_FUNC_ETH;

	/* Parsing I2C */
	value = PIN_I2C_CFG_NONE;

	if (local_top_reg4.bit.I2C == MUX_1) {
		value |= PIN_I2C_CFG_I2C_1;
	}

	if (local_top_reg4.bit.I2C2 == MUX_1) {
		value |= PIN_I2C_CFG_I2C2_1;
	}

	if (local_top_reg4.bit.I2C3 == MUX_1) {
		value |= PIN_I2C_CFG_I2C3_1;
	} else if (local_top_reg4.bit.I2C3 == MUX_2) {
		value |= PIN_I2C_CFG_I2C3_2;
	} else if (local_top_reg4.bit.I2C3 == MUX_3) {
		value |= PIN_I2C_CFG_I2C3_3;
	} else if (local_top_reg4.bit.I2C3 == MUX_4) {
		value |= PIN_I2C_CFG_I2C3_4;
	}

	if (local_top_reg4.bit.I2C4 == MUX_1) {
		value |= PIN_I2C_CFG_I2C4_1;
	} else if (local_top_reg4.bit.I2C4 == MUX_2) {
		value |= PIN_I2C_CFG_I2C4_2;
	} else if (local_top_reg4.bit.I2C4 == MUX_3) {
		value |= PIN_I2C_CFG_I2C4_3;
	} else if (local_top_reg4.bit.I2C4 == MUX_4) {
		value |= PIN_I2C_CFG_I2C4_4;
	}

	if (local_top_reg4.bit.I2C5 == MUX_1) {
		value |= PIN_I2C_CFG_I2C5_1;
	} else if (local_top_reg4.bit.I2C5 == MUX_2) {
		value |= PIN_I2C_CFG_I2C5_2;
	} else if (local_top_reg4.bit.I2C5 == MUX_3) {
		value |= PIN_I2C_CFG_I2C5_3;
	} else if (local_top_reg4.bit.I2C5 == MUX_4) {
		value |= PIN_I2C_CFG_I2C5_4;
	}

	if (local_top_reg4.bit.I2C6 == MUX_1) {
		value |= PIN_I2C_CFG_I2C6_1;
	}

	if (local_top_reg4.bit.I2C7 == MUX_1) {
		value |= PIN_I2C_CFG_I2C7_1;
	}

	if (local_top_reg4.bit.I2C8 == MUX_1) {
		value |= PIN_I2C_CFG_I2C8_1;
	}

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/* Parsing PWM */
	value = PIN_PWM_CFG_NONE;

	if (local_top_reg6.bit.PWM == MUX_1) {
		value |= PIN_PWM_CFG_PWM_1;
	}

	if (local_top_reg6.bit.PWM2 == MUX_1) {
		value |= PIN_PWM_CFG_PWM2_1;
	}

	if (local_top_reg6.bit.PWM3 == MUX_1) {
		value |= PIN_PWM_CFG_PWM3_1;
	}

	if (local_top_reg6.bit.PWM4 == MUX_1) {
		value |= PIN_PWM_CFG_PWM4_1;
	}

	if (local_top_reg6.bit.PWM5 == MUX_1) {
		value |= PIN_PWM_CFG_PWM5_1;
	}

	if (local_top_reg6.bit.PWM6 == MUX_1) {
		value |= PIN_PWM_CFG_PWM6_1;
	}

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/* Parsing AUDIO */
	value = PIN_AUDIO_CFG_NONE;

	if (local_top_reg8.bit.I2S == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_1;
	} else if (local_top_reg8.bit.I2S == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_2;
	}
	if (local_top_reg8.bit.I2S_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_1;
	} else if (local_top_reg8.bit.I2S_MCLK == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_2;
	}

	if (local_top_reg8.bit.I2S2 == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S2_1;
	} else if (local_top_reg8.bit.I2S2 == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S2_2;
	} else if (local_top_reg8.bit.I2S2 == MUX_3) {
		value |= PIN_AUDIO_CFG_I2S2_3;
	}

	if (local_top_reg8.bit.I2S2_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S2_MCLK_1;
	} else if (local_top_reg8.bit.I2S2_MCLK == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S2_MCLK_2;
	} else if (local_top_reg8.bit.I2S2_MCLK == MUX_3) {
		value |= PIN_AUDIO_CFG_I2S2_MCLK_3;
	}

	if (local_top_reg8.bit.I2S3 == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S3_1;
	}

	if (local_top_reg8.bit.I2S3_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S3_MCLK_1;
	}

	if (local_top_reg8.bit.I2S4 == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S4_1;
	}

	if (local_top_reg8.bit.I2S4_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S4_MCLK_1;
	}

	info->top_pinmux[PIN_FUNC_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_AUDIO].pin_function = PIN_FUNC_AUDIO;

	/* Parsing UART */
	value = PIN_UART_CFG_NONE;

	if (local_top_reg9.bit.UART == MUX_1) {
		value |= PIN_UART_CFG_UART_1;
	} else if (local_top_reg9.bit.UART == MUX_2) {
		value |= PIN_UART_CFG_UART_2;
	}
	if (local_top_reg9.bit.UART_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART_RTSCTS_1;
	} else if (local_top_reg9.bit.UART_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART_RTSCTS_2;
	}

	if (local_top_reg9.bit.UART2 == MUX_1) {
		value |= PIN_UART_CFG_UART2_1;
	} else if (local_top_reg9.bit.UART2 == MUX_2) {
		value |= PIN_UART_CFG_UART2_2;
	}
	if (local_top_reg9.bit.UART2_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART2_RTSCTS_1;
	} else if (local_top_reg9.bit.UART2_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART2_RTSCTS_2;
	}

	if (local_top_reg9.bit.UART3 == MUX_1) {
		value |= PIN_UART_CFG_UART3_1;
	}
	if (local_top_reg9.bit.UART3_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART3_RTSCTS_1;
	}

	if (local_top_reg9.bit.UART4 == MUX_1) {
		value |= PIN_UART_CFG_UART4_1;
	} else if (local_top_reg9.bit.UART4 == MUX_2) {
		value |= PIN_UART_CFG_UART4_2;
	} else if (local_top_reg9.bit.UART4 == MUX_3) {
		value |= PIN_UART_CFG_UART4_3;
	}

	if (local_top_reg9.bit.UART5 == MUX_1) {
		value |= PIN_UART_CFG_UART5_1;
	} else if (local_top_reg9.bit.UART5 == MUX_2) {
		value |= PIN_UART_CFG_UART5_2;
	}

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/* Parsing REMOTE */
	value = PIN_REMOTE_CFG_NONE;

	if (local_top_reg10.bit.REMOTE == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_1;
	} else if (local_top_reg10.bit.REMOTE == MUX_2) {
		value |= PIN_REMOTE_CFG_REMOTE_2;
	} else if (local_top_reg10.bit.REMOTE == MUX_3) {
		value |= PIN_REMOTE_CFG_REMOTE_3;
	} else if (local_top_reg10.bit.REMOTE == MUX_4) {
		value |= PIN_REMOTE_CFG_REMOTE_4;
	} else if (local_top_reg10.bit.REMOTE == MUX_5) {
		value |= PIN_REMOTE_CFG_REMOTE_5;
	} else if (local_top_reg10.bit.REMOTE == MUX_6) {
		value |= PIN_REMOTE_CFG_REMOTE_6;
	} else if (local_top_reg10.bit.REMOTE == MUX_7) {
		value |= PIN_REMOTE_CFG_REMOTE_7;
	} else if (local_top_reg10.bit.REMOTE == MUX_8) {
		value |= PIN_REMOTE_CFG_REMOTE_8;
	} else if (local_top_reg10.bit.REMOTE == MUX_9) {
		value |= PIN_REMOTE_CFG_REMOTE_9;
	} else if (local_top_reg10.bit.REMOTE == MUX_10) {
		value |= PIN_REMOTE_CFG_REMOTE_10;
	}

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/* Parsing SDP */
	value = PIN_SDP_CFG_NONE;

	if (local_top_reg10.bit.SDP == MUX_1) {
		value |= PIN_SDP_CFG_SDP_1;
	} else if (local_top_reg10.bit.SDP == MUX_2) {
		value |= PIN_SDP_CFG_SDP_2;
	}

	if (local_top_reg10.bit.SDP_RDY == MUX_1) {
		value |= PIN_SDP_CFG_SDP_RDY_1;
	} else if (local_top_reg10.bit.SDP_RDY == MUX_2) {
		value |= PIN_SDP_CFG_SDP_RDY_2;
	}

	info->top_pinmux[PIN_FUNC_SDP].config = value;
	info->top_pinmux[PIN_FUNC_SDP].pin_function = PIN_FUNC_SDP;

	/* Parsing SPI */
	value = PIN_SPI_CFG_NONE;

	if (local_top_reg11.bit.SPI == MUX_1) {
		value |= PIN_SPI_CFG_SPI_1;
	}
	if (local_top_reg11.bit.SPI_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI_BUS_WIDTH;
	}
	if (local_top_reg11.bit.SPI_RDY == MUX_1) {
		value |= PIN_SPI_CFG_SPI_RDY_1;
	}

	if (local_top_reg11.bit.SPI2 == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_1;
	} else if (local_top_reg11.bit.SPI2 == MUX_2) {
		value |= PIN_SPI_CFG_SPI2_2;
	} else if (local_top_reg11.bit.SPI2 == MUX_3) {
		value |= PIN_SPI_CFG_SPI2_3;
	} else if (local_top_reg11.bit.SPI2 == MUX_4) {
		value |= PIN_SPI_CFG_SPI2_4;
	}
	if (local_top_reg11.bit.SPI2_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_BUS_WIDTH;
	}
	if (local_top_reg11.bit.SPI2_RDY == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_RDY_1;
	}

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/* Parsing MISC */
	value = PIN_MISC_CFG_NONE;

 	if (local_top_reg5.bit.RTC_CLK == MUX_1) {
		value |= PIN_MISC_CFG_RTC_CLK_1;
	}

	if (local_top_reg5.bit.RTC_EXT_CLK == MUX_1) {
		value |= PIN_MISC_CFG_RTC_EXT_CLK_1;
	}

	if (local_top_reg5.bit.RTC_DIV_OUT == MUX_1) {
		value |= PIN_MISC_CFG_RTC_DIV_OUT_1;
	}

	if (local_top_reg5.bit.EXT_CLK == MUX_1) {
		value |= PIN_MISC_CFG_EXT_CLK_1;
	} else if (local_top_reg5.bit.EXT_CLK == MUX_2) {
		value |= PIN_MISC_CFG_EXT_CLK_2;
	}

	if (local_top_reg5.bit.EXT2_CLK == MUX_1) {
		value |= PIN_MISC_CFG_EXT2_CLK_1;
	} else if (local_top_reg5.bit.EXT2_CLK == MUX_2) {
		value |= PIN_MISC_CFG_EXT2_CLK_2;
	}

	if (local_top_reg7.bit.CLKOUT_12M == MUX_1) {
		value |= PIN_MISC_CFG_CLKOUT;
	}

	if (local_top_reg13.bit.SATA_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA_LED_1;
	} else if (local_top_reg13.bit.SATA_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA_LED_2;
	} else if (local_top_reg13.bit.SATA_LED == MUX_3) {
		value |= PIN_MISC_CFG_SATA_LED_3;
	} else if (local_top_reg13.bit.SATA_LED == MUX_4) {
		value |= PIN_MISC_CFG_SATA_LED_4;
	}

	if (local_top_reg13.bit.SATA2_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA2_LED_1;
	} else if (local_top_reg13.bit.SATA2_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA2_LED_2;
	} else if (local_top_reg13.bit.SATA2_LED == MUX_3) {
		value |= PIN_MISC_CFG_SATA2_LED_3;
	} else if (local_top_reg13.bit.SATA2_LED == MUX_4) {
		value |= PIN_MISC_CFG_SATA2_LED_4;
	}

	if (local_top_reg13.bit.SATA3_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA3_LED_1;
	} else if (local_top_reg13.bit.SATA3_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA3_LED_2;
	}

	if (local_top_reg13.bit.SATA4_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA4_LED_1;
	} else if (local_top_reg13.bit.SATA4_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA4_LED_2;
	}

	if (local_top_reg13.bit.SATA5_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA5_LED_1;
	} else if (local_top_reg13.bit.SATA5_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA5_LED_2;
	}

	if (local_top_reg13.bit.SATA6_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA6_LED_1;
	} else if (local_top_reg13.bit.SATA6_LED == MUX_2) {
		value |= PIN_MISC_CFG_SATA6_LED_2;
	}

	if (adc_en == MUX_1) {
		value |= PIN_MISC_CFG_ADC;
	}

	if (local_top_reg8.bit.HDMI_CEC == MUX_1) {
		value |= PIN_MISC_CFG_HDMI_CEC;
	}
	if (local_top_reg8.bit.HDMI2_CEC == MUX_1) {
		value |= PIN_MISC_CFG_HDMI2_CEC;
	}
	if (local_top_reg8.bit.HDMI3_CEC == MUX_1) {
		value |= PIN_MISC_CFG_HDMI3_CEC;
	}

	if (local_top_reg1.bit.EJTAG_EN == MUX_1) {
		value |= PIN_MISC_CFG_CPU_ICE;
	}

	info->top_pinmux[PIN_FUNC_MISC].config = value;
	info->top_pinmux[PIN_FUNC_MISC].pin_function = PIN_FUNC_MISC;

	/* Parsing PCIE */
	value = PIN_PCIE_CFG_NONE;

	if (local_top_reg14.bit.PCIE_MODE_SEL == 1) {
		value |= PIN_PCIE_CFG_CTRL1_MODE_EP;
	}
	if (local_top_reg14.bit.PCIE2_MODE_SEL == 1) {
		value |= PIN_PCIE_CFG_CTRL2_MODE_EP;
	}
	if (local_top_reg14.bit.PCIE3_MODE_SEL == 1) {
		value |= PIN_PCIE_CFG_CTRL3_MODE_EP;
	}
	if (local_top_reg14.bit.PCIE4_MODE_SEL == 1) {
		value |= PIN_PCIE_CFG_CTRL4_MODE_EP;
	}
	if (local_top_reg_jgpio0.bit.JGPIO_5 == GPIO_ID_EMUM_FUNC) {
		value |= PIN_PCIE_CFG_RSTN;
	}
	if ((local_top_reg14.bit.PCIE_REFCLK_SRC == 0) &&
		(local_top_reg14.bit.PCIE_REFCLK_PAD_OUT_EN == 1)) {
		value |= PIN_PCIE_CFG_REFCLK_OUTEN;
	}

	info->top_pinmux[PIN_FUNC_PCIE].config = value;
	info->top_pinmux[PIN_FUNC_PCIE].pin_function = PIN_FUNC_PCIE;

	/* Parsing PCIE select */
	value = PINMUX_PCIEMUX_SEL_RSTN;
	if (local_top_reg14.bit.PCIE_RESETN_RELEASE == 1) {
		value |= PINMUX_PCIEMUX_RSTN_HIGHZ;
	} else {
		value |= PINMUX_PCIEMUX_RSTN_OUT_LOW;
	}

	info->top_pinmux[PIN_FUNC_SEL_PCIE].config = value;
	info->top_pinmux[PIN_FUNC_SEL_PCIE].pin_function = PIN_FUNC_SEL_PCIE;

	/* Parsing LCD */
	info->top_pinmux[PIN_FUNC_LCD].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD];
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/* Parsing TV */
	info->top_pinmux[PIN_FUNC_TV].config = disp_pinmux_config[PINMUX_FUNC_ID_TV];
	info->top_pinmux[PIN_FUNC_TV].pin_function = PIN_FUNC_TV;

#if 0
	/* Parsing LCD_PINOUT_SEL */
	value = PINMUX_LCD_TYPE_PINOUT_SEL_NONE;

	if (local_top_reg2.bit.LCD_PINOUT_SEL == MUX_0) {
		value |= PINMUX_LCD_TYPE_PINOUT_SEL_LCD;
	} else if (local_top_reg2.bit.LCD_PINOUT_SEL == MUX_1) {
		value |= PINMUX_LCD_TYPE_PINOUT_SEL_LCD2;
	} else if (local_top_reg2.bit.LCD_PINOUT_SEL == MUX_2) {
		value |= PINMUX_LCD_TYPE_PINOUT_SEL_LCDLITE;
	}

	if (local_top_reg2.bit.LCD2_PINOUT_SEL == MUX_0) {
		value |= PINMUX_LCD2_TYPE_PINOUT_SEL_LCD;
	} else if (local_top_reg2.bit.LCD2_PINOUT_SEL == MUX_1) {
		value |= PINMUX_LCD2_TYPE_PINOUT_SEL_LCD2;
	} else if (local_top_reg2.bit.LCD2_PINOUT_SEL == MUX_2) {
		value |= PINMUX_LCD2_TYPE_PINOUT_SEL_LCDLITE;
	}

	info->top_pinmux[PIN_FUNC_LCD_PINOUT_SEL].config = value;
	info->top_pinmux[PIN_FUNC_LCD_PINOUT_SEL].pin_function = PIN_FUNC_LCD_PINOUT_SEL;
#endif

	/* Parsing SEL_LCD */
	value = PINMUX_LCD_SEL_GPIO;

	if (local_top_reg2.bit.LCD_TYPE == MUX_1) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR656_16BITS;
		} else {
			value = PINMUX_LCD_SEL_CCIR656;
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_2) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR601_16BITS;

			if (local_top_reg2.bit.CCIR_FIELD == MUX_1) {
				value |= PINMUX_LCD_SEL_FIELD;
			}
			if (local_top_reg2.bit.LCD_HS_VS == MUX_0) {
				value |= PINMUX_LCD_SEL_NO_HVSYNC;
			}
		} else {
			value = PINMUX_LCD_SEL_CCIR601;

			if (local_top_reg2.bit.CCIR_FIELD == MUX_1) {
				value |= PINMUX_LCD_SEL_FIELD;
			}
			if (local_top_reg2.bit.LCD_HS_VS == MUX_0) {
				value |= PINMUX_LCD_SEL_NO_HVSYNC;
			}
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_3) {
		value = PINMUX_LCD_SEL_PARALLE_RGB565;

		if (local_top_reg2.bit.LCD_HS_VS == MUX_0) {
			value |= PINMUX_LCD_SEL_NO_HVSYNC;
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_8) {
		value = PINMUX_LCD_SEL_PARALLE_RGB666;

		if (local_top_reg2.bit.LCD_HS_VS == MUX_0) {
			value |= PINMUX_LCD_SEL_NO_HVSYNC;
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_10) {
		value = PINMUX_LCD_SEL_PARALLE_RGB888;

		if (local_top_reg2.bit.LCD_HS_VS == MUX_0) {
			value |= PINMUX_LCD_SEL_NO_HVSYNC;
		}
	}

	if (local_top_reg2.bit.PLCD_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD].pin_function = PIN_FUNC_SEL_LCD;

#if 0
	/* Parsing SEL_LCD2 */
	value = PINMUX_LCD_SEL_GPIO;

	if (local_top_reg2.bit.LCD2_TYPE == MUX_1) {
		value = PINMUX_LCD_SEL_CCIR656;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_2) {
		value = PINMUX_LCD_SEL_CCIR601;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_4) {
		if (lcd2_rgb_6bit_flag) {
			value = PINMUX_LCD_SEL_SERIAL_RGB_6BITS;
		} else {
			value = PINMUX_LCD_SEL_SERIAL_RGB_8BITS;
		}
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_6) {
		value = PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_9) {
		value = PINMUX_LCD_SEL_MIPI;
	} else if (local_top_reg2.bit.MEMIF_SEL == MUX_1) {
		if (local_top_reg2.bit.MEMIF_TYPE == MUX_1) {
			if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDIO;
			} else if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO;
			}
		} else if (local_top_reg2.bit.MEMIF_TYPE == MUX_2) {
			if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_PARALLE_MI_8BITS;
			} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_PARALLE_MI_9BITS;
			}
		}
	}

	if (local_top_reg2.bit.TE_SEL == MUX_1) {
		value |= PINMUX_LCD_SEL_TE_ENABLE;
	}

	if (local_top_reg2.bit.TE2_SEL == MUX_1) {
		value |= PINMUX_LCD_SEL_TE2_ENABLE;
	}

	if (local_top_reg2.bit.PLCD2_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD2].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD2].pin_function = PIN_FUNC_SEL_LCD2;
#endif

	/* Leave critical section */
	unl_cpu(flags);

	pr_info("%s leave \r\n", __func__);
}

void gpio_func_keep(int start, int count, int func)
{
	int i = 0;

	for (i = start; i < start + count; i++) {
		dump_gpio_func[i] = func;
	}
}

int gpio_conflict_detect(int start,int count,int func)
{
	int i = 0;
	int confl_mod;
	int confl_flag = 0;
	for(i = start; i < start+count; i++) {
		confl_mod = dump_gpio_func[i];
		//printf("%d\r\n",confl_mod);

		if(confl_mod > 0) {
			pr_err("%s conflict with %s\r\n",dump_func[func-1],dump_func[confl_mod-1]);
			confl_flag++;
			break;
		}
	}
	return confl_flag;
}




/*----------------------------------------------*/
/*          PINMUX Interface Functions          */
/*----------------------------------------------*/
static int pinmux_config_sdio(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_SDIO_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 2, func_SDIO);
			confl_detect += gpio_conflict_detect(PGPIO_15, 4, func_SDIO);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 2, func_SDIO);
			gpio_func_keep(PGPIO_15, 4, func_SDIO);
		} else if (config & PIN_SDIO_CFG_SDIO_2) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_SDIO_2);
			confl_detect += gpio_conflict_detect(PGPIO_22, 4, func_SDIO_2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_2;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 2, func_SDIO_2);
			gpio_func_keep(PGPIO_22, 4, func_SDIO_2);
		} else if (config & PIN_SDIO_CFG_SDIO_3) {

			confl_detect += gpio_conflict_detect(EGPIO_24, 6, func_SDIO_3);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_3;
			top_reg_egpio0.bit.EGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_29 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_24, 6, func_SDIO_3);
		}

		if (config & PIN_SDIO_CFG_SDIO2_1) {

			confl_detect += gpio_conflict_detect(CGPIO_8, 6, func_SDIO2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO2_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_8, 6, func_SDIO2);

			if (config & PIN_SDIO_CFG_SDIO2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_14, 4, func_SDIO2);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.SDIO2_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_14, 4, func_SDIO2);
			}
			if (config & PIN_SDIO_CFG_SDIO2_DS) {

				confl_detect += gpio_conflict_detect(CGPIO_6, 1, func_SDIO2);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.SDIO2_DS_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_6, 1, func_SDIO2);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_nand(uint32_t config)
{
	if (config == PIN_NAND_CFG_NONE) {
	} else {
		if ((config & PIN_NAND_CFG_NAND_1) || (config & PIN_NAND_CFG_NAND_8BIT)) {

			confl_detect += gpio_conflict_detect(CGPIO_0 , 6, func_FSPI);

			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.FSPI_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0,6,func_FSPI);

			if (config & PIN_NAND_CFG_NAND_CS1) {

				confl_detect += gpio_conflict_detect(CGPIO_9, 1, func_FSPI);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.FSPI_CS1_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_9, 1, func_FSPI);
			}
			if (config & PIN_NAND_CFG_NAND_8BIT) {

				confl_detect += gpio_conflict_detect(CGPIO_6, 1, func_FSPI);
				confl_detect += gpio_conflict_detect(CGPIO_10, 4, func_FSPI);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.FSPI_EN = MUX_2;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_6, 1, func_FSPI);
				gpio_func_keep(CGPIO_10, 4, func_FSPI);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_eth(uint32_t config)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & PIN_ETH_CFG_ETH_RGMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_2 , 14 , func_ETH);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_1;
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
			gpio_func_keep(EGPIO_2, 14, func_ETH);

		} else if (config & PIN_ETH_CFG_ETH_RMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_2 , 4 , func_ETH);
			confl_detect += gpio_conflict_detect(EGPIO_9 , 3 , func_ETH);
			confl_detect += gpio_conflict_detect(EGPIO_14 , 2 , func_ETH);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_2;
			top_reg_egpio0.bit.EGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_2, 4, func_ETH);
			gpio_func_keep(EGPIO_9, 3, func_ETH);
			gpio_func_keep(EGPIO_14, 2, func_ETH);

		} else if (config & PIN_ETH_CFG_ETH_SGMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_14 , 2 , func_ETH);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_3;
			top_reg_egpio0.bit.EGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_14, 2, func_ETH);

		}

		if((config & PIN_ETH_CFG_ETH_RGMII_1) || \
			(config & PIN_ETH_CFG_ETH_RMII_1) || \
			(config & PIN_ETH_CFG_ETH_SGMII_1)){

			if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {
				top_reg3.bit.ETH_EXT_PHY_CLK = MUX_1;

				confl_detect += gpio_conflict_detect(EGPIO_0, 1, func_ETH);
	            if(confl_detect > 0) {
	                return E_PAR;
	            }

				top_reg_egpio0.bit.EGPIO_0 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(EGPIO_0, 1, func_ETH);
			}
		}

		if (config & PIN_ETH_CFG_ETH2_RGMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_18, 14, func_ETH2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH2 = MUX_1;
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
			top_reg_egpio0.bit.EGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_31 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_18, 14, func_ETH2);

		} else if (config & PIN_ETH_CFG_ETH2_RMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_18, 4, func_ETH2);
			confl_detect += gpio_conflict_detect(EGPIO_25, 3, func_ETH2);
			confl_detect += gpio_conflict_detect(EGPIO_30, 2, func_ETH2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH2 = MUX_2;
			top_reg_egpio0.bit.EGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_31 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_18, 4, func_ETH2);
			gpio_func_keep(EGPIO_25, 3, func_ETH2);
			gpio_func_keep(EGPIO_30, 2, func_ETH2);

		} else if (config & PIN_ETH_CFG_ETH2_SGMII_1) {

			confl_detect += gpio_conflict_detect(EGPIO_30, 2, func_ETH2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH2 = MUX_3;
			top_reg_egpio0.bit.EGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_31 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_30, 2, func_ETH2);

		}

		if((config & PIN_ETH_CFG_ETH2_RGMII_1) || \
			(config & PIN_ETH_CFG_ETH2_RMII_1) || \
			(config & PIN_ETH_CFG_ETH2_SGMII_1)){

			if (config & PIN_ETH_CFG_ETH2_EXTPHYCLK) {
				top_reg3.bit.ETH2_EXT_PHY_CLK = MUX_1;

				confl_detect += gpio_conflict_detect(EGPIO_16, 1, func_ETH);
	            if(confl_detect > 0) {
	                return E_PAR;
	            }

				top_reg_egpio0.bit.EGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(EGPIO_16, 1, func_ETH);
			}
		}
	}
	return E_OK;
}

static int pinmux_config_i2c(uint32_t config)
{
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & PIN_I2C_CFG_I2C_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 2, func_I2C);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO13, PAD_NONE);
			gpio_func_keep(PGPIO_12, 2, func_I2C);
		}

		if (config & PIN_I2C_CFG_I2C2_1) {
			confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_I2C2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO19, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO20, PAD_NONE);
			gpio_func_keep(PGPIO_19, 2, func_I2C2);
		}

		if (config & PIN_I2C_CFG_I2C3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_32, 2, func_I2C3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C3 = MUX_1;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO32, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO33, PAD_NONE);
			gpio_func_keep(PGPIO_32, 2, func_I2C3);

		} else if (config & PIN_I2C_CFG_I2C3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_16, 2, func_I2C3_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_CGPIO16, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_CGPIO17, PAD_NONE);
			gpio_func_keep(CGPIO_16, 2, func_I2C3_2);

		} else if (config & PIN_I2C_CFG_I2C3_3) {

			confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_I2C3_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C3 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_DGPIO10, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_DGPIO11, PAD_NONE);
			gpio_func_keep(DGPIO_10, 2, func_I2C3_3);

		} else if (config & PIN_I2C_CFG_I2C3_4) {

			confl_detect += gpio_conflict_detect(PGPIO_6, 2, func_I2C3_4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C3 = MUX_4;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO7, PAD_NONE);
			gpio_func_keep(PGPIO_6, 2, func_I2C3_4);
		}

		if (config & PIN_I2C_CFG_I2C4_1) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 2, func_I2C4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C4 = MUX_1;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_DGPIO3, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_DGPIO4, PAD_NONE);
			gpio_func_keep(DGPIO_3, 2, func_I2C4);

		} else if (config & PIN_I2C_CFG_I2C4_2) {

			confl_detect += gpio_conflict_detect(JGPIO_0, 2, func_I2C4_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C4 = MUX_2;
			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_JGPIO0, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_JGPIO1, PAD_NONE);
			gpio_func_keep(JGPIO_0, 2, func_I2C4_2);

		} else if (config & PIN_I2C_CFG_I2C4_3) {

			confl_detect += gpio_conflict_detect(PGPIO_26, 2, func_I2C4_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C4 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO26, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO27, PAD_NONE);
			gpio_func_keep(PGPIO_26, 2, func_I2C4_3);

		} else if (config & PIN_I2C_CFG_I2C4_4) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 2, func_I2C4_4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C4 = MUX_4;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
			gpio_func_keep(PGPIO_10, 2, func_I2C4_4);

		}

		if (config & PIN_I2C_CFG_I2C5_1) {

			confl_detect += gpio_conflict_detect(DGPIO_6, 2, func_I2C5);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_1;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_DGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_DGPIO7, PAD_NONE);
			gpio_func_keep(DGPIO_6, 2, func_I2C5);

		} else if (config & PIN_I2C_CFG_I2C5_2) {

			confl_detect += gpio_conflict_detect(JGPIO_3, 2, func_I2C5_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_2;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_JGPIO3, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_JGPIO4, PAD_NONE);
			gpio_func_keep(JGPIO_3, 2, func_I2C5_2);

		} else if (config & PIN_I2C_CFG_I2C5_3) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 2, func_I2C5_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_3;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_CGPIO12, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
			gpio_func_keep(CGPIO_12, 2, func_I2C5_3);
		} else if (config & PIN_I2C_CFG_I2C5_4) {

			confl_detect += gpio_conflict_detect(PGPIO_36, 2, func_I2C5_4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_3;
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO36, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO37, PAD_NONE);
			gpio_func_keep(PGPIO_36, 2, func_I2C5_4);
		}

		if (config & PIN_I2C_CFG_I2C6_1) {

			confl_detect += gpio_conflict_detect(PGPIO_39, 2, func_I2C6);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_1;
			top_reg_pgpio1.bit.PGPIO_39 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_40 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO39, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO40, PAD_NONE);
			gpio_func_keep(PGPIO_39, 2, func_I2C6);

		}
		if (config & PIN_I2C_CFG_I2C7_1) {

			confl_detect += gpio_conflict_detect(PGPIO_42, 2, func_I2C7);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_1;
			top_reg_pgpio1.bit.PGPIO_42 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_43 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO42, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO43, PAD_NONE);
			gpio_func_keep(PGPIO_42, 2, func_I2C7);

		}
		if (config & PIN_I2C_CFG_I2C8_1) {

			confl_detect += gpio_conflict_detect(PGPIO_45, 2, func_I2C8);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg4.bit.I2C5 = MUX_1;
			top_reg_pgpio1.bit.PGPIO_45 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_46 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO45, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO46, PAD_NONE);
			gpio_func_keep(PGPIO_45, 2, func_I2C8);
		}
	}
	return E_OK;
}


static int pinmux_config_pwm(uint32_t config)
{
	if (config == PIN_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PWM_CFG_PWM_1) {

			confl_detect += gpio_conflict_detect(PGPIO_28, 1 ,func_PWM);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_28, 1, func_PWM);
		}

		if (config & PIN_PWM_CFG_PWM2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_29, 1, func_PWM2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_29, 1, func_PWM2);
		}

		if (config & PIN_PWM_CFG_PWM3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_30, 1, func_PWM3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_30, 1, func_PWM3);
		}

		if (config & PIN_PWM_CFG_PWM4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_31, 1, func_PWM4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_31, 1, func_PWM4);
		}

		if (config & PIN_PWM_CFG_PWM5_1) {

			confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_PWM5);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM5 = MUX_1;
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_0, 1, func_PWM5);
		}

		if (config & PIN_PWM_CFG_PWM6_1) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_PWM6);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg6.bit.PWM6 = MUX_1;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_PWM6);
		}
	}
	return E_OK;
}


static int pinmux_config_audio(uint32_t config)
{
	if (config & PIN_AUDIO_CFG_NONE) {
	} else {
		if (config & PIN_AUDIO_CFG_I2S_1) {

			confl_detect += gpio_conflict_detect(PGPIO_15, 4, func_I2S);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S = MUX_1;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_15, 4, func_I2S);

		} else if (config & PIN_AUDIO_CFG_I2S_2) {

			confl_detect += gpio_conflict_detect(PGPIO_34, 4 ,func_I2S_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S = MUX_2;
			top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_34, 4, func_I2S_2);
		}

		if (config & PIN_AUDIO_CFG_I2S_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_14, 1, func_I2S_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_14, 1, func_I2S_MCLK);

		} else if (config & PIN_AUDIO_CFG_I2S_MCLK_2) {

			confl_detect += gpio_conflict_detect(PGPIO_38, 1 , func_I2S_2_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S_MCLK = MUX_2;
			top_reg_pgpio1.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_38, 1, func_I2S_2_MCLK);
		}

		if (config & PIN_AUDIO_CFG_I2S2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_22, 4, func_I2S2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_22, 4, func_I2S2);

		} else if (config & PIN_AUDIO_CFG_I2S2_2) {

			confl_detect += gpio_conflict_detect(DGPIO_1, 4, func_I2S2_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_1, 4, func_I2S2_2);

		} else if (config & PIN_AUDIO_CFG_I2S2_3) {

			confl_detect += gpio_conflict_detect(DGPIO_6, 4, func_I2S2_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_6, 4, func_I2S2_3);
		}

		if (config & PIN_AUDIO_CFG_I2S2_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_I2S2_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_I2S2_MCLK);

		} else if(config & PIN_AUDIO_CFG_I2S2_MCLK_2) {

			confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_I2S2_2_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2_MCLK = MUX_2;
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_0, 1, func_I2S2_2_MCLK);

		} else if(config & PIN_AUDIO_CFG_I2S2_MCLK_3) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_I2S2_3_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S2_MCLK = MUX_3;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_I2S2_3_MCLK);
		}

		if (config & PIN_AUDIO_CFG_I2S3_1) {

			confl_detect += gpio_conflict_detect(CGPIO_14, 4, func_I2S3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S3 = MUX_1;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 4, func_I2S3);
		}

		if (config & PIN_AUDIO_CFG_I2S3_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_28, 1, func_I2S3_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S3_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_28, 1, func_I2S3_MCLK);
		}

		if (config & PIN_AUDIO_CFG_I2S4_1) {

			confl_detect += gpio_conflict_detect(EGPIO_25, 4, func_I2S4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S4 = MUX_1;
			top_reg_egpio0.bit.EGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg_egpio0.bit.EGPIO_28 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_25, 4, func_I2S4);
		}

		if (config & PIN_AUDIO_CFG_I2S4_MCLK_1) {

			confl_detect += gpio_conflict_detect(EGPIO_24, 1, func_I2S4_MCLK);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.I2S4_MCLK = MUX_1;
			top_reg_egpio0.bit.EGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_24, 1, func_I2S4_MCLK);
		}
	}

	return E_OK;
}

static int pinmux_config_uart(uint32_t config)
{
	if (config == PIN_UART_CFG_NONE) {
	} else {
		if (config & PIN_UART_CFG_UART_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 2, func_UART);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 2, func_UART);

		} else if (config & PIN_UART_CFG_UART_2) {

			confl_detect += gpio_conflict_detect(PGPIO_26, 2, func_UART_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART = MUX_2;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_26, 2, func_UART_2);
		}

		if ((config & PIN_UART_CFG_UART_1) || \
			(config & PIN_UART_CFG_UART_2)) {

			if (config & PIN_UART_CFG_UART_RTSCTS_1) {

				confl_detect += gpio_conflict_detect(PGPIO_2, 2, func_UART_RTSCTS);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg9.bit.UART = MUX_1;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_2, 2, func_UART_RTSCTS);

			} else if (config & PIN_UART_CFG_UART_RTSCTS_2) {

				confl_detect += gpio_conflict_detect(PGPIO_28, 2, func_UART_2_RTSCTS);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg9.bit.UART = MUX_2;
				top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_28, 2, func_UART_2_RTSCTS);
			}
		}

		if (config & PIN_UART_CFG_UART2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_4, 2, func_UART2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 2, func_UART2);

		} else if (config & PIN_UART_CFG_UART2_2) {

			confl_detect += gpio_conflict_detect(CGPIO_8, 2, func_UART2_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_8, 2, func_UART2_2);

		}

		if ((config & PIN_UART_CFG_UART2_1) || \
			(config & PIN_UART_CFG_UART2_2)) {

			if (config & PIN_UART_CFG_UART2_RTSCTS_1) {

				confl_detect += gpio_conflict_detect(PGPIO_6, 2 , func_UART2_RTSCTS);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg9.bit.UART2_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_6, 2, func_UART2_RTSCTS);
			} else if (config & PIN_UART_CFG_UART2_RTSCTS_2) {

				confl_detect += gpio_conflict_detect(CGPIO_10, 2 , func_UART2_2_RTSCTS);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg9.bit.UART2_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_10, 2, func_UART2_2_RTSCTS);
			}
		}

		if (config & PIN_UART_CFG_UART3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_UART3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 2, func_UART3);

			if (config & PIN_UART_CFG_UART3_RTSCTS_1) {

				confl_detect += gpio_conflict_detect(DGPIO_8, 2, func_UART3_RTSCTS);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg9.bit.UART3_RTSCTS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_8, 2, func_UART3_RTSCTS);
			}
		}

		if (config & PIN_UART_CFG_UART4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 2, func_UART4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_10, 2, func_UART4);

		} else if (config & PIN_UART_CFG_UART4_2) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 2, func_UART4_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART4 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 2, func_UART4_2);

		} else if (config & PIN_UART_CFG_UART4_3) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_UART4_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART4 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 2, func_UART4_3);

		}

		if (config & PIN_UART_CFG_UART5_1) {

			confl_detect += gpio_conflict_detect(DGPIO_8, 2, func_UART5);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART5 = MUX_1;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_8, 2, func_UART5);

		} else if (config & PIN_UART_CFG_UART5_2) {

			confl_detect += gpio_conflict_detect(CGPIO_14, 2, func_UART5_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg9.bit.UART5 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 2, func_UART5_2);
		}
	}

	return E_OK;
}

static int pinmux_config_remote(uint32_t config)
{
	if (config == PIN_REMOTE_CFG_NONE) {
	} else {
		if (config & PIN_REMOTE_CFG_REMOTE_1) {

			confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_1;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_2) {

			confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_2;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_15, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_3) {

			confl_detect += gpio_conflict_detect(CGPIO_17, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_3;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_4) {

			confl_detect += gpio_conflict_detect(JGPIO_2, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_4;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(JGPIO_2, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_5) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_5;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_6) {

			confl_detect += gpio_conflict_detect(PGPIO_38, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_6;
			top_reg_pgpio1.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_38, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_7) {

			confl_detect += gpio_conflict_detect(DGPIO_1, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_7;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_1, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_8) {

			confl_detect += gpio_conflict_detect(DGPIO_7, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_8;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_7, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_9) {

			confl_detect += gpio_conflict_detect(DGPIO_10, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_9;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_10, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_10) {

			confl_detect += gpio_conflict_detect(EGPIO_16, 1, func_Remote);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg10.bit.REMOTE = MUX_10;
			top_reg_egpio0.bit.EGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_16, 1, func_Remote);

		}
	}

	return E_OK;
}

static int pinmux_config_sdp(uint32_t config)
{
	if (config == PIN_SDP_CFG_NONE) {
	} else {
		if (config & PIN_SDP_CFG_SDP_1) {

			confl_detect += gpio_conflict_detect(PGPIO_22, 4, func_SDP);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.SDP = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_22, 4, func_SDP);

			if (config & PIN_SDP_CFG_SDP_RDY_1) {
				confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_SDP_RDY);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg10.bit.SDP_RDY = MUX_1;
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_21, 1, func_SDP_RDY);
			}

		} else if (config & PIN_SDP_CFG_SDP_2) {

			confl_detect += gpio_conflict_detect(DGPIO_1, 4, func_SDP_2);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.SDP = MUX_2;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_1, 4, func_SDP_2);

			if (config & PIN_SDP_CFG_SDP_RDY_2) {
				confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_SDP_2_RDY);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg10.bit.SDP_RDY = MUX_2;
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_0, 1, func_SDP_2_RDY);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_spi(uint32_t config)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & PIN_SPI_CFG_SPI_1) {

			confl_detect += gpio_conflict_detect(PGPIO_22, 3, func_SPI);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg11.bit.SPI = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_22, 3, func_SPI);

			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_25, 1, func_SPI);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_25, 1, func_SPI);
			}
			if (config & PIN_SPI_CFG_SPI_RDY_1) {

				confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_SPI_RDY);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI_RDY = MUX_1;
				top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_21, 1, func_SPI_RDY);
			}
		}

		if (config & PIN_SPI_CFG_SPI2_1) {

			confl_detect += gpio_conflict_detect(CGPIO_14, 3, func_SPI2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg11.bit.SPI2 = MUX_1;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 3, func_SPI2);

			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_17, 1, func_SPI2);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_17, 1, func_SPI2);
			}

		} else if (config & PIN_SPI_CFG_SPI2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_34, 3, func_SPI2_2);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg11.bit.SPI2 = MUX_2;
			top_reg_pgpio1.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_34, 3, func_SPI2_2);

			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_37, 1 , func_SPI2_2);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_37, 1, func_SPI2_2);
			}

		} else if (config & PIN_SPI_CFG_SPI2_3) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 3, func_SPI2_3);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg11.bit.SPI2 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 3, func_SPI2_3);

			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_3, 1, func_SPI2_3);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_3, 1, func_SPI2_3);
			}

		} else if (config & PIN_SPI_CFG_SPI2_4) {

			confl_detect += gpio_conflict_detect(DGPIO_1, 3, func_SPI2_4);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg11.bit.SPI2 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_1, 3, func_SPI2_4);

			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(DGPIO_4, 1, func_SPI2_4);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_4, 1, func_SPI2_4);
			}
			if (config & PIN_SPI_CFG_SPI2_RDY_1) {

				confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_SPI2_RDY);
				if(confl_detect > 0) {
					return E_PAR;
				}
				top_reg11.bit.SPI2_RDY = MUX_1;
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_0, 1, func_SPI2_RDY);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_misc(uint32_t config)
{
	if (config == PIN_MISC_CFG_NONE) {
	} else {
		if (config & PIN_MISC_CFG_RTC_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_2, 1, func_MISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg5.bit.RTC_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_2, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_RTC_EXT_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.RTC_EXT_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_RTC_DIV_OUT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.RTC_DIV_OUT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_CLKOUT) {
			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg7.bit.CLKOUT_12M = MUX_1;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_EXT_CLK_1) {
			confl_detect += gpio_conflict_detect(PGPIO_26, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.EXT_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_26, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_EXT_CLK_2) {
			confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.EXT_CLK = MUX_2;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_EXT2_CLK_1) {
			confl_detect += gpio_conflict_detect(PGPIO_27, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.EXT2_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_27, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_EXT2_CLK_2) {
			confl_detect += gpio_conflict_detect(DGPIO_11, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg5.bit.EXT2_CLK = MUX_2;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_11, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_SATA_LED_1) {

			confl_detect += gpio_conflict_detect(PGPIO_36, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA_LED = MUX_1;
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_36, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA_LED_2) {

			confl_detect += gpio_conflict_detect(PGPIO_2, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA_LED = MUX_2;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_2, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA_LED_3) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA_LED = MUX_3;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_10, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA_LED_4) {

			confl_detect += gpio_conflict_detect(JGPIO_2, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA_LED = MUX_4;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(JGPIO_2, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_SATA2_LED_1) {

			confl_detect += gpio_conflict_detect(PGPIO_37, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA2_LED = MUX_1;
			top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_37, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA2_LED_2) {

			confl_detect += gpio_conflict_detect(PGPIO_3, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA2_LED = MUX_2;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_3, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA2_LED_3) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA2_LED = MUX_3;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_11, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA2_LED_4) {

			confl_detect += gpio_conflict_detect(JGPIO_3, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA2_LED = MUX_4;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(JGPIO_3, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_SATA3_LED_1) {

			confl_detect += gpio_conflict_detect(PGPIO_38, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA3_LED = MUX_1;
			top_reg_pgpio1.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_38, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA3_LED_2) {

			confl_detect += gpio_conflict_detect(JGPIO_4, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA3_LED = MUX_2;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(JGPIO_4, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_SATA4_LED_1) {

			confl_detect += gpio_conflict_detect(EGPIO_20, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA4_LED = MUX_1;
			top_reg_egpio0.bit.EGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_20, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA4_LED_2) {

			confl_detect += gpio_conflict_detect(EGPIO_0, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA4_LED = MUX_2;
			top_reg_egpio0.bit.EGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_0, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_SATA5_LED_1) {

			confl_detect += gpio_conflict_detect(EGPIO_21, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA5_LED = MUX_1;
			top_reg_egpio0.bit.EGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_21, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA5_LED_2) {

			confl_detect += gpio_conflict_detect(PGPIO_26, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA5_LED = MUX_2;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_26, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_SATA6_LED_1) {

			confl_detect += gpio_conflict_detect(EGPIO_22, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA6_LED = MUX_1;
			top_reg_egpio0.bit.EGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(EGPIO_22, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SATA6_LED_2) {

			confl_detect += gpio_conflict_detect(PGPIO_27, 1, func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg13.bit.SATA6_LED = MUX_2;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_27, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_ADC) {

			confl_detect += gpio_conflict_detect(AGPIO_0 , 4 , func_MISC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			adc_en = MUX_1;
			top_reg_agpio0.bit.AGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(AGPIO_0, 4, func_MISC);
		}

		if (config & PIN_MISC_CFG_HDMI_CEC) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_HDMI_CEC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.HDMI_CEC = MUX_1;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_HDMI_CEC);

		}

		if (config & PIN_MISC_CFG_HDMI2_CEC) {

			confl_detect += gpio_conflict_detect(DGPIO_10, 1, func_HDMI2_CEC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.HDMI_CEC = MUX_1;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_10, 1, func_HDMI2_CEC);

		}

		if (config & PIN_MISC_CFG_HDMI3_CEC) {

			confl_detect += gpio_conflict_detect(DGPIO_11, 1, func_HDMI3_CEC);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg8.bit.HDMI_CEC = MUX_1;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_11, 1, func_HDMI3_CEC);

		}

		if (config & PIN_MISC_CFG_CPU_ICE) {

			confl_detect += gpio_conflict_detect(JGPIO_0, 5, func_EJTAG);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg1.bit.EJTAG_EN = MUX_1;
			top_reg_jgpio0.bit.JGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_jgpio0.bit.JGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(JGPIO_0, 5, func_EJTAG);

		}
	}

	return E_OK;
}

static int pinmux_config_pcie(uint32_t config)
{

	if (config == PIN_PCIE_CFG_NONE) {
	} else {
		if (config & PIN_PCIE_CFG_CTRL1_MODE_EP) {
			top_reg14.bit.PCIE_MODE_SEL = 1;
		}
		if (config & PIN_PCIE_CFG_CTRL2_MODE_EP) {
			top_reg14.bit.PCIE2_MODE_SEL = 1;
		}
		if (config & PIN_PCIE_CFG_CTRL3_MODE_EP) {
			top_reg14.bit.PCIE3_MODE_SEL = 1;
		}
		if (config & PIN_PCIE_CFG_CTRL4_MODE_EP) {
			top_reg14.bit.PCIE4_MODE_SEL = 1;
		}
		if (config & PIN_PCIE_CFG_RSTN) {
//			top_reg14.bit.PCIE_RESETN_RELEASE = 1;
			top_reg_jgpio0.bit.JGPIO_5 = GPIO_ID_EMUM_FUNC;
		}
		if (config & PIN_PCIE_CFG_REFCLK_OUTEN) {
			top_reg14.bit.PCIE_REFCLK_SRC = 0;			// from MPLL24
			top_reg14.bit.PCIE_REFCLK_PAD_OUT_EN = 1;	// output enable
		} else {
			top_reg14.bit.PCIE_REFCLK_SRC = 1;			// from REFCLK pad
			top_reg14.bit.PCIE_REFCLK_PAD_OUT_EN = 0;	// output disable (input)
		}
	}

	return E_OK;
}

static int pinmux_config_lcd(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD] = config;

	return E_OK;
}

static int pinmux_config_tv(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_TV_HDMI_CFG_MASK;
	if ((tmp != PINMUX_TV_HDMI_CFG_NORMAL) && (tmp != PINMUX_TV_HDMI_CFG_PINMUX_ON)) {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_TV] = config;

	return E_OK;
}

#if 0
static int pinmux_config_lcd_pinout_sel(uint32_t config)
{

	if (config == PINMUX_LCD_TYPE_PINOUT_SEL_NONE) {
	} else {

		if (config & PINMUX_LCD_TYPE_PINOUT_SEL_LCD) {
			top_reg2.bit.LCD_PINOUT_SEL = MUX_0;
		} else if (config & PINMUX_LCD_TYPE_PINOUT_SEL_LCD2) {
			top_reg2.bit.LCD_PINOUT_SEL = MUX_1;
		} else if (config & PINMUX_LCD_TYPE_PINOUT_SEL_LCDLITE) {
			top_reg2.bit.LCD_PINOUT_SEL = MUX_2;
		}

		if (config & PINMUX_LCD2_TYPE_PINOUT_SEL_LCD) {
			top_reg2.bit.LCD2_PINOUT_SEL = MUX_0;
		} else if (config & PINMUX_LCD2_TYPE_PINOUT_SEL_LCD2) {
			top_reg2.bit.LCD2_PINOUT_SEL = MUX_1;
		} else if (config & PINMUX_LCD2_TYPE_PINOUT_SEL_LCDLITE) {
			top_reg2.bit.LCD2_PINOUT_SEL = MUX_2;
		}

	}
	return E_OK;
}
#endif

static int pinmux_select_primary_lcd(uint32_t config)
{
	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {

			confl_detect += gpio_conflict_detect(BGPIO_0 , 9, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_bgpio0.bit.BGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(BGPIO_0, 8, func_LCD);

		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR656_16BITS) {

			confl_detect += gpio_conflict_detect(BGPIO_0, 17, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
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
			gpio_func_keep(BGPIO_0, 17, func_LCD);

		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {

			confl_detect += gpio_conflict_detect(BGPIO_0 , 9, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_bgpio0.bit.BGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(BGPIO_0, 8, func_LCD);

			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(DGPIO_8, 1, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_8, 1, func_LCD);
			}

			if ((config & PINMUX_LCD_SEL_NO_HVSYNC) == 0) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.LCD_HS_VS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 2, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601_16BITS) {

			confl_detect += gpio_conflict_detect(BGPIO_0 , 17, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}
			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
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
			gpio_func_keep(BGPIO_0, 17, func_LCD);

			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(DGPIO_8, 1, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_8, 1, func_LCD);
			}

			if ((config & PINMUX_LCD_SEL_NO_HVSYNC) == 0) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.LCD_HS_VS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 2, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) {

			confl_detect += gpio_conflict_detect(BGPIO_0, 6, func_LCD);
			confl_detect += gpio_conflict_detect(BGPIO_9, 6, func_LCD);
			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_32, 2, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_3;
			top_reg_bgpio0.bit.BGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(BGPIO_0, 6, func_LCD);
			gpio_func_keep(BGPIO_9, 6, func_LCD);
			gpio_func_keep(DGPIO_5, 1, func_LCD);
			gpio_func_keep(PGPIO_8, 2, func_LCD);
			gpio_func_keep(PGPIO_32, 2, func_LCD);

			if ((config & PINMUX_LCD_SEL_NO_HVSYNC) == 0) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.LCD_HS_VS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 2, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666) {

			confl_detect += gpio_conflict_detect(BGPIO_0, 7, func_LCD);
			confl_detect += gpio_conflict_detect(BGPIO_9, 6, func_LCD);
			confl_detect += gpio_conflict_detect(DGPIO_5, 2, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_32, 2, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_3;
			top_reg_bgpio0.bit.BGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_bgpio0.bit.BGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(BGPIO_0, 7, func_LCD);
			gpio_func_keep(BGPIO_9, 6, func_LCD);
			gpio_func_keep(DGPIO_5, 2, func_LCD);
			gpio_func_keep(PGPIO_8, 2, func_LCD);
			gpio_func_keep(PGPIO_32, 2, func_LCD);

			if ((config & PINMUX_LCD_SEL_NO_HVSYNC) == 0) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.LCD_HS_VS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 2, func_LCD);
			}

		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) {

			confl_detect += gpio_conflict_detect(BGPIO_0, 9, func_LCD);
			confl_detect += gpio_conflict_detect(BGPIO_9, 8, func_LCD);
			confl_detect += gpio_conflict_detect(DGPIO_5, 4, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_32, 2, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_3;
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
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(BGPIO_0, 17, func_LCD);
			gpio_func_keep(DGPIO_5, 4, func_LCD);
			gpio_func_keep(PGPIO_8, 2, func_LCD);
			gpio_func_keep(PGPIO_32, 2, func_LCD);

			if ((config & PINMUX_LCD_SEL_NO_HVSYNC) == 0) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 2, func_LCD);
				if(confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.LCD_HS_VS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 2, func_LCD);
			}
		} else {
			pr_err("%s: Type[0x%x] not support \r\n", __func__, pinmux_type);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {
			confl_detect += gpio_conflict_detect(DGPIO_9 , 1, func_LCD);
			if(confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.PLCD_DE = MUX_1;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_9, 1, func_LCD);
		}

	}

	return E_OK;
}

#if 0
static int pinmux_select_secondary_lcd(uint32_t config)
{
	return E_OK;
}
#endif

static int pinmux_select_pcie(uint32_t config)
{
#if 0
	u32 pinmux_type;
	u32 pinmux;

	pinmux_type = config & (PINMUX_PCIEMUX_SEL_MASK);
	pinmux = config & ~(PINMUX_PCIEMUX_SEL_MASK);

	if (pinmux_type == PINMUX_PCIEMUX_SEL_RSTN) {
		if (pinmux == PINMUX_PCIEMUX_RSTN_OUT_LOW) {
			top_reg14.bit.PCIE_RESETN_RELEASE = 0;
			return E_OK;
		} else if (pinmux == PINMUX_PCIEMUX_RSTN_HIGHZ) {
			top_reg14.bit.PCIE_RESETN_RELEASE = 1;
			return E_OK;
		}
	}
#endif
	return E_OK;
}

#if 0
ER pinmux_parsing_i2c(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		//< I2C_1   (S_GPIO[13..12])
		//< I2C_2   (P_GPIO[29..28])
		if (config & (PIN_I2C_CFG_I2C_1 | PIN_I2C_CFG_I2C_2)) {
			if (config & PIN_I2C_CFG_I2C_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && SGPIO12 pull up => force SGPIO12 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO12 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO13, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && SGPIO13 pull up => force SGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO13 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO28, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 2ND FUNC_EN && PGPIO28 pull up => force PGPIO28 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO28, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO28 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO28, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO29, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 2ND FUNC_EN && PGPIO29 pull up => force PGPIO29 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO29, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO29 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO29, PAD_NONE);
				}
			}
		}
		//< I2C2_1  (S_GPIO[15..14])
		//< I2C2_2  (S_GPIO[31..30])
		if (config & (PIN_I2C_CFG_I2C2_1 | PIN_I2C_CFG_I2C2_2)) {
			if (config & PIN_I2C_CFG_I2C2_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO14, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO14 pull up => force SGPIO14 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO14 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO15, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO15 pull up => force SGPIO15 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO15 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO30, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO30 pull up => force PGPIO30 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO30, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO30 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO30, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO31, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO31 pull up => force PGPIO31 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO31 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
				}
			}
		}

		//< I2C3_1  (S_GPIO[29..28])
		//< I2C3_2  (P_GPIO[13..12])
		if (config & (PIN_I2C_CFG_I2C3_1 | PIN_I2C_CFG_I2C3_2)) {
			if (config & PIN_I2C_CFG_I2C3_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO28, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO28 pull up => force SGPIO28 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO28, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO28 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_SGPIO28, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO29, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO29 pull up => force SGPIO29 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO29, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO29 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO29, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO12 pull up => force PGPIO12 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO12 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO13, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO13 pull up => force PGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO13, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO13 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO13, PAD_NONE);
				}
			}
		}

		//< I2C4_1  (S_GPIO[31..30])
		//< I2C4_2  (P_GPIO[15..14])
		//< I2C4_3  (D_GPIO[5..4])
		if (config & (PIN_I2C_CFG_I2C4_1 | PIN_I2C_CFG_I2C4_2 | PIN_I2C_CFG_I2C4_3)) {
			if (config & PIN_I2C_CFG_I2C4_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO30, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 1st FUNC_EN && SGPIO30 pull up => force SGPIO30 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO30, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO30 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO30, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO31, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 1st FUNC_EN && SGPIO31 pull up => force SGPIO31 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO31, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO31 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO31, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C4_2) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO14, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 2nd FUNC_EN && PGPIO14 pull up => force PGPIO14 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO14, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO14 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO14, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO15, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 2nd FUNC_EN && PGPIO15 pull up => force PGPIO15 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO15, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO15 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO15, PAD_NONE);
				}
				// PIN_I2C_CFG_I2C4_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_DGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 3rd FUNC_EN && DGPIO4 pull up => force DGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO4 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 3rd FUNC_EN && DGPIO5 pull up => force DGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO5 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DGPIO5, PAD_NONE);
				}
			}
		}

		//PIN_I2C_CFG_I2C5_1  ->I2C5_1  (S_GPIO[17..16])
		//PIN_I2C_CFG_I2C5_2  ->I2C5_2  (P_GPIO[17..16])
		//PIN_I2C_CFG_I2C5_3  ->I2C5_3  (C_GPIO[ 5..4])
		if (config & (PIN_I2C_CFG_I2C5_1 | PIN_I2C_CFG_I2C5_2 | PIN_I2C_CFG_I2C5_3)) {
			if (config & PIN_I2C_CFG_I2C5_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO16, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 1st FUNC_EN && SGPIO16 pull up => force SGPIO16 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO16 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO17, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 1st FUNC_EN && SGPIO17 pull up => force SGPIO17 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO17, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO17 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO17, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C5_2) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO16, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 2nd FUNC_EN && PGPIO16 pull up => force PGPIO16 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO16, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO16 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO16, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO17, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 2nd FUNC_EN && PGPIO17 pull up => force PGPIO17 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO17, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO17 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO17, PAD_NONE);
				}
				// PIN_I2C_CFG_I2C5_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_CGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 3rd FUNC_EN && CGPIO4 pull up => force CGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO4 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 3rd FUNC_EN && CGPIO5 pull up => force CGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO5 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO5, PAD_NONE);
				}
			}
		}
	}
	return E_OK;
}

ER pinmux_parsing_i2cII(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2CII_CFG_NONE) {
	} else {
		//PIN_I2CII_CFG_I2C6_1  ->  I2C6_1  (S_GPIO[19..18])
		//PIN_I2CII_CFG_I2C6_2  ->  I2C6_2  (P_GPIO[19..18])
		//PIN_I2CII_CFG_I2C6_3  ->  I2C6_3  (C_GPIO[ 7..6])
		if (config & (PIN_I2CII_CFG_I2C6_1 | PIN_I2CII_CFG_I2C6_2 | PIN_I2CII_CFG_I2C6_3)) {
			if (config & PIN_I2CII_CFG_I2C6_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO18, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C6 1st FUNC_EN && SGPIO18 pull up => force SGPIO18 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO18, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO18 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO18, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO19, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C6 1st FUNC_EN && SGPIO19 pull up => force SGPIO19 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO19, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO19 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO19, PAD_NONE);
				}
			} else if (config & PIN_I2CII_CFG_I2C6_2) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO18, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C6 2nd FUNC_EN && PGPIO18 pull up => force PGPIO18 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO18, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO18 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO18, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO19, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C6 2nd FUNC_EN && PGPIO19 pull up => force PGPIO19 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO19, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO19 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO19, PAD_NONE);
				}
				// PIN_I2CII_CFG_I2C6_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_CGPIO6, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C6 3rd FUNC_EN && CGPIO6 pull up => force CGPIO6 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO6 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO7, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 3rd FUNC_EN && CGPIO7 pull up => force CGPIO7 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO7 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
				}
			}
		}
		//PIN_I2CII_CFG_I2C7_1  -> I2C7_1  (S_GPIO[21..20])
		//PIN_I2CII_CFG_I2C7_2  -> I2C7_2  (P_GPIO[21..20])
		if (config & (PIN_I2CII_CFG_I2C7_1 | PIN_I2CII_CFG_I2C7_2)) {
			if (config & PIN_I2CII_CFG_I2C7_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO20, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C7 1st FUNC_EN && SGPIO20 pull up => force SGPIO20 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO20, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO20 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_SGPIO20, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO21, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C7 1st FUNC_EN && SGPIO21 pull up => force SGPIO21 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO21, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO21 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO21, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO20, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C7 2nd FUNC_EN && PGPIO20 pull up => force PGPIO20 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO20, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO20 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO20, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO21, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C7 2nd FUNC_EN && PGPIO21 pull up => force PGPIO21 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO21 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
				}
			}
		}
		//#define PIN_I2CII_CFG_I2C8_1 -> I2C8_1  (S_GPIO[23..22])
		//#define PIN_I2CII_CFG_I2C8_2 -> I2C8_2  (P_GPIO[23..22])
		if (config & (PIN_I2CII_CFG_I2C8_1 | PIN_I2CII_CFG_I2C8_2)) {
			if (config & PIN_I2CII_CFG_I2C8_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO22, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C8 1st FUNC_EN && SGPIO22 pull up => force SGPIO22 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO22, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO22 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_SGPIO22, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO23, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C8 1st FUNC_EN && SGPIO23 pull up => force SGPIO23 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO23, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO23 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO23, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO22, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C8 2nd FUNC_EN && PGPIO22 pull up => force PGPIO22 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO22 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO23, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C8 2nd FUNC_EN && PGPIO23 pull up => force PGPIO23 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO23, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO23 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO23, PAD_NONE);
				}
			}
		}
		// PIN_I2CII_CFG_I2C9_1  -> I2C9_1  (P_GPIO[25..24])
		// PIN_I2CII_CFG_I2C9_2  -> I2C9_2  (L_GPIO[28..27])
		if (config & (PIN_I2CII_CFG_I2C9_1 | PIN_I2CII_CFG_I2C9_2)) {
			if (config & PIN_I2CII_CFG_I2C9_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO24, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C9 1st FUNC_EN && PGPIO24 pull up => force PGPIO24 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO24, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO24 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO24, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO25, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C9 1st FUNC_EN && PGPIO25 pull up => force PGPIO25 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO25, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO25 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO25, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_LGPIO27, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C9 2nd FUNC_EN && LGPIO27 pull up => force LGPIO27 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO27, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO27 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_LGPIO27, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_LGPIO28, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C9 2nd FUNC_EN && LGPIO28 pull up => force LGPIO28 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO28, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO28 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO28, PAD_NONE);
				}
			}
		}
		// PIN_I2CII_CFG_I2C10_1 -> I2C10_1 (P_GPIO[27..26])
		// PIN_I2CII_CFG_I2C10_2 -> I2C10_2 (D_GPIO[ 1..0])
		if (config & (PIN_I2CII_CFG_I2C10_1 | PIN_I2CII_CFG_I2C10_2)) {
			if (config & PIN_I2CII_CFG_I2C10_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO26, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C10 1st FUNC_EN && PGPIO26 pull up => force PGPIO26 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO26, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO26 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO26, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO27, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C10 1st FUNC_EN && PGPIO27 pull up => force PGPIO27 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO27, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO27 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO27, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_DGPIO0, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C10 2nd FUNC_EN && DGPIO0 pull up => force DGPIO0 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO0, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO0 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_DGPIO0, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DGPIO1, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C10 2nd FUNC_EN && DGPIO1 pull up => force DGPIO1 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO1, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO1 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DGPIO1, PAD_NONE);
				}
			}
		}
		// PIN_I2CII_CFG_I2C11_1 -> I2C11_1 (P_GPIO[37..36])
		if (config & PIN_I2CII_CFG_I2C11_1) {
			if (pad_get_pull_updown(PAD_PIN_PGPIO36, &pad_pull) == E_OK) {
				if (pad_pull != PAD_NONE) {
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("!!!I2C11 1st FUNC_EN && PGPIO36 pull up => force PGPIO36 = pull none!!!\n");
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown(PAD_PIN_PGPIO36, PAD_NONE);
				}
			} else {
				pr_err("Get PAD_PIN_PGPIO36 Fail, force pull none\n");
				pad_set_pull_updown(PAD_PIN_PGPIO36, PAD_NONE);
			}

			if (pad_get_pull_updown(PAD_PIN_PGPIO37, &pad_pull) == E_OK) {
				if (pad_pull != PAD_NONE) {
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("!!!I2C11 1st FUNC_EN && PGPIO37 pull up => force PGPIO37 = pull none!!!\n");
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown(PAD_PIN_PGPIO37, PAD_NONE);
				}
			} else {
				pr_err("Get PAD_PIN_PGPIO37 Fail, force pull none\r\n");
				pad_set_pull_updown(PAD_PIN_PGPIO37, PAD_NONE);
			}
		}
	}
	return E_OK;
}

ER pinmux_parsing_i2cIII(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		// PIN_I2CIII_CFG_I2C12_1 -> I2C12_1  (P_GPIO[46..45])
		if (config & PIN_I2CIII_CFG_I2C12_1) {
			if (pad_get_pull_updown(PAD_PIN_PGPIO45, &pad_pull) == E_OK) {
				if (pad_pull != PAD_NONE) {
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("!!!I2C12 1st FUNC_EN && PGPIO45 pull up => force PGPIO45 = pull none!!!\n");
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown(PAD_PIN_PGPIO45, PAD_NONE);
				}
			} else {
				pr_err("Get PAD_PIN_PGPIO45 Fail, force pull none\n");
				pad_set_pull_updown(PAD_PIN_PGPIO45, PAD_NONE);
			}

			if (pad_get_pull_updown(PAD_PIN_PGPIO46, &pad_pull) == E_OK) {
				if (pad_pull != PAD_NONE) {
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pr_err("!!!I2C12 1st FUNC_EN && PGPIO46 pull up => force PGPIO46 = pull none!!!\n");
					pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					pad_set_pull_updown(PAD_PIN_PGPIO46, PAD_NONE);
				}
			} else {
				pr_err("Get PAD_PIN_PGPIO46 Fail, force pull none\r\n");
				pad_set_pull_updown(PAD_PIN_PGPIO46, PAD_NONE);
			}
		}
		// PIN_I2CIII_CFG_I2C13_1 -> I2C13_1  (P_GPIO[39..38])
		// PIN_I2CIII_CFG_I2C13_2 -> I2C13_2  (D_GPIO[ 3..2])
		if (config & (PIN_I2CIII_CFG_I2C13_1 | PIN_I2CIII_CFG_I2C13_2)) {
			if (config & PIN_I2CIII_CFG_I2C13_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO38, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C13 1st FUNC_EN && PGPIO38 pull up => force PGPIO38 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO38, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO38 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO38, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO39, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C13 1st FUNC_EN && PGPIO39 pull up => force PGPIO39 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO39, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO39 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO39, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_DGPIO2, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C13 2nd FUNC_EN && DGPIO2 pull up => force DGPIO2 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO2, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO2 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_DGPIO2, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DGPIO3, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C13 2nd FUNC_EN && DGPIO3 pull up => force DGPIO3 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO3, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO3 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DGPIO3, PAD_NONE);
				}
			}
		}
		// PIN_I2CIII_CFG_I2C14_1 -> I2C14_1  (P_GPIO[41..40])
		// PIN_I2CIII_CFG_I2C14_2 -> I2C14_2  (D_GPIO[ 5..4])
		if (config & (PIN_I2CIII_CFG_I2C14_1 | PIN_I2CIII_CFG_I2C14_2)) {
			if (config & PIN_I2CIII_CFG_I2C14_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO40, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C14 1st FUNC_EN && PGPIO40 pull up => force PGPIO40 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO40, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO40 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO40, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO41, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C14 1st FUNC_EN && PGPIO41 pull up => force PGPIO41 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO41, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO41 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO41, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_DGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C14 2nd FUNC_EN && DGPIO4 pull up => force DGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO4 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_DGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C14 2nd FUNC_EN && DGPIO5 pull up => force DGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DGPIO5 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DGPIO5, PAD_NONE);
				}
			}
		}
		// PIN_I2CIII_CFG_I2C15_1 -> I2C15_1  (P_GPIO[ 1..0])
		// PIN_I2CIII_CFG_I2C15_2 -> I2C15_2(DSI_GPIO[ 1..0])
		// PIN_I2CIII_CFG_I2C15_3 -> I2C15_3  (LGPIO [18..17])
		if (config & (PIN_I2CIII_CFG_I2C15_1 | PIN_I2CIII_CFG_I2C15_2 | PIN_I2CIII_CFG_I2C15_3)) {
			if (config & PIN_I2CIII_CFG_I2C15_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO0, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 1st FUNC_EN && PGPIO0 pull up => force PGPIO0 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO0, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO0 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO0, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO1, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 1st FUNC_EN && PGPIO1 pull up => force PGPIO1 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO1, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO1 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO1, PAD_NONE);
				}
			} else if (config & PIN_I2CIII_CFG_I2C15_2) {
				if (pad_get_pull_updown(PAD_PIN_DSIGPIO0, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 2nd FUNC_EN && DSIGPIO0 pull up => force DSIGPIO0 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO0, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO0 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO0, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DSIGPIO1, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 2nd FUNC_EN && DSIGPIO1 pull up => force DSIGPIO1 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO1, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO1 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO1, PAD_NONE);
				}
				// PIN_I2CIII_CFG_I2C15_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_LGPIO17, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 3rd FUNC_EN && LGPIO17 pull up => force LGPIO17 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO17, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO17 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO17, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_LGPIO18, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C15 3rd FUNC_EN && LGPIO18 pull up => force LGPIO18 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO18, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO18 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO18, PAD_NONE);
				}
			}
		}
		// PIN_I2CIII_CFG_I2C16_1 -> I2C16_1  (P_GPIO[ 3..2])
		// PIN_I2CIII_CFG_I2C16_2 -> I2C16_2(DSI_GPIO[ 3..2])
		if (config & (PIN_I2CIII_CFG_I2C16_1 | PIN_I2CIII_CFG_I2C16_2)) {
			if (config & PIN_I2CIII_CFG_I2C16_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO2, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C16 1st FUNC_EN && PGPIO2 pull up => force PGPIO2 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO2, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO2 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO2, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO3, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C16 1st FUNC_EN && PGPIO3 pull up => force PGPIO3 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO3, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO3 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO3, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_DSIGPIO2, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C16 2nd FUNC_EN && DSIGPIO2 pull up => force DSIGPIO2 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO2, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO2 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO2, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DSIGPIO3, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C16 2nd FUNC_EN && DSIGPIO3 pull up => force DSIGPIO3 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO3, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO3 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO3, PAD_NONE);
				}
			}
		}
		// PIN_I2CIII_CFG_I2C17_1 -> I2C17_1  (P_GPIO[ 5..4])
		// PIN_I2CIII_CFG_I2C17_2 -> I2C17_2(DSI_GPIO[ 7..6])
		// PIN_I2CIII_CFG_I2C17_3 -> I2C17_3  (L_GPIO[22..21])
		if (config & (PIN_I2CIII_CFG_I2C17_1 | PIN_I2CIII_CFG_I2C17_2 | PIN_I2CIII_CFG_I2C17_3)) {
			if (config & PIN_I2CIII_CFG_I2C17_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 1st FUNC_EN && PGPIO4 pull up => force PGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO4 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 1st FUNC_EN && PGPIO5 pull up => force PGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO5 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO5, PAD_NONE);
				}
			} else if (config & PIN_I2CIII_CFG_I2C17_2) {
				if (pad_get_pull_updown(PAD_PIN_DSIGPIO6, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 2nd FUNC_EN && DSIGPIO6 pull up => force DSIGPIO6 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO6, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO6 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO6, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DSIGPIO7, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 2nd FUNC_EN && DSIGPIO7 pull up => force DSIGPIO7 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO7, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO7 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO7, PAD_NONE);
				}
				// PIN_I2CIII_CFG_I2C17_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_LGPIO21, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 3rd FUNC_EN && LGPIO21 pull up => force LGPIO21 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO21, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO21 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO21, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_LGPIO22, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C17 3rd FUNC_EN && LGPIO22 pull up => force LGPIO22 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO22, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO22 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO22, PAD_NONE);
				}
			}
		}
	}
	return E_OK;
}

ER pinmux_parsing_i2cIV(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2CIV_CFG_NONE) {
	} else {
		// PIN_I2CIV_CFG_I2C18_1 -> I2C18_1  (P_GPIO[ 7..6])
		// PIN_I2CIV_CFG_I2C18_2 -> I2C18_2 DSI_GPIO[ 9..8])
		// PIN_I2CIV_CFG_I2C18_3 -> I2C18_3  (P_GPIO[24..23])
		if (config & (PIN_I2CIV_CFG_I2C18_1 | PIN_I2CIV_CFG_I2C18_2 | PIN_I2CIV_CFG_I2C18_3)) {
			if (config & PIN_I2CIV_CFG_I2C18_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO6, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 1st FUNC_EN && PGPIO6 pull up => force PGPIO6 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO6, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO6 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO6, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO7, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 1st FUNC_EN && PGPIO7 pull up => force PGPIO7 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO7, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO7 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO7, PAD_NONE);
				}
			} else if (config & PIN_I2CIV_CFG_I2C18_2) {
				if (pad_get_pull_updown(PAD_PIN_DSIGPIO8, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 2nd FUNC_EN && DSIGPIO8 pull up => force DSIGPIO8 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO8, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO8 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO8, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DSIGPIO9, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 2nd FUNC_EN && DSIGPIO9 pull up => force DSIGPIO9 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO9, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO9 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO9, PAD_NONE);
				}
				// PIN_I2CIV_CFG_I2C18_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_PGPIO23, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 3rd FUNC_EN && PGPIO23 pull up => force PGPIO23 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO23, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO23 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO23, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO24, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C18 3rd FUNC_EN && PGPIO24 pull up => force PGPIO24 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO24, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO24 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO24, PAD_NONE);
				}
			}
		}
		// PIN_I2CIV_CFG_I2C19_1 -> I2C19_1  (P_GPIO[ 9..8])
		// PIN_I2CIV_CFG_I2C19_2 -> I2C19_2 DSI_GPIO[11..10])
		// PIN_I2CIV_CFG_I2C19_3 -> I2C19_3  (L_GPIO[26..25])
		if (config & (PIN_I2CIV_CFG_I2C19_1 | PIN_I2CIV_CFG_I2C19_2 | PIN_I2CIV_CFG_I2C19_3)) {
			if (config & PIN_I2CIV_CFG_I2C19_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO8, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 1st FUNC_EN && PGPIO8 pull up => force PGPIO8 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO8, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO8 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO8, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO9, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 1st FUNC_EN && PGPIO9 pull up => force PGPIO9 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO9 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
				}
			} else if (config & PIN_I2CIV_CFG_I2C19_2) {
				if (pad_get_pull_updown(PAD_PIN_DSIGPIO10, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 2nd FUNC_EN && DSIGPIO10 pull up => force DSIGPIO10 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO10, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO10 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO10, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_DSIGPIO11, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 2nd FUNC_EN && DSIGPIO11 pull up => force DSIGPIO11 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_DSIGPIO11, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_DSIGPIO11 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_DSIGPIO11, PAD_NONE);
				}
				// PIN_I2CIV_CFG_I2C19_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_LGPIO25, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 3rd FUNC_EN && LGPIO25 pull up => force LGPIO25 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO25, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO25 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO25, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_LGPIO26, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C19 3rd FUNC_EN && LGPIO26 pull up => force LGPIO26 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO26, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO26 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO26, PAD_NONE);
				}
			}
		}
		// PIN_I2CIV_CFG_I2C20_1 -> I2C20_1  (P_GPIO[11..10])
		// PIN_I2CIV_CFG_I2C20_2 -> I2C20_2  (L_GPIO[30..29])
		if (config & (PIN_I2CIV_CFG_I2C20_1 | PIN_I2CIV_CFG_I2C20_2)) {
			if (config & PIN_I2CIII_CFG_I2C14_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO10, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C20 1st FUNC_EN && PGPIO10 pull up => force PGPIO10 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO10 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO11, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C20 1st FUNC_EN && PGPIO11 pull up => force PGPIO11 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO11 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_LGPIO29, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C20 2nd FUNC_EN && LGPIO29 pull up => force LGPIO29 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO29, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO29 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_LGPIO29, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_LGPIO30, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C20 2nd FUNC_EN && LGPIO30 pull up => force LGPIO30 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_LGPIO30, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_LGPIO30 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_LGPIO30, PAD_NONE);
				}
			}
		}

	}
	return E_OK;
}
#endif


typedef int (*PINMUX_CONFIG_HDL)(uint32_t);
static PINMUX_CONFIG_HDL pinmux_config_hdl[] = {
	pinmux_config_sdio,
	pinmux_config_nand,
	pinmux_config_eth,
	pinmux_config_i2c,
	pinmux_config_pwm,
	pinmux_config_audio,
	pinmux_config_uart,
	pinmux_config_remote,
	pinmux_config_sdp,
	pinmux_config_spi,
	pinmux_config_misc,
	pinmux_config_lcd,
	pinmux_config_tv,
	pinmux_select_primary_lcd,
	pinmux_config_pcie,
	pinmux_select_pcie,
};

/**
    Configure pinmux controller

    Configure pinmux controller by upper layer

    @param[in] info nvt_pinctrl_info
    @return void
*/
ER pinmux_init(struct nvt_pinctrl_info *info)
{
	uint32_t i;
	int err;
	unsigned long flags = 0;

	pr_info("%s, %s\r\n", __func__, DRV_VERSION);

	/* Enter critical section */
	loc_cpu(flags);

	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);	// system
	top_reg8.reg = 0;
	top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);	// uart
	top_reg10.reg = 0;
	top_reg11.reg = 0;
//	top_reg12.reg = 0;
	top_reg13.reg = 0;
	top_reg14.reg = TOP_GETREG(info, TOP_REG14_OFS); // for PCIE BS.

	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_jgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio1.reg = 0xFFFFFFFF;
	top_reg_dgpio0.reg = 0xFFFFFFFF;
	top_reg_egpio0.reg = 0xFFFFFFFF;
	top_reg_bgpio0.reg = 0xFFFFFFFF;
	top_reg_agpio0.reg = 0xFFFFFFFF;

	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_jgpio0.reg = 0x00000020;
	} else {
		top_reg_jgpio0.reg = 0x0000003F;
	}

	/* Clean dump table */
	for (i = 0; i < GPIO_total; i++) {
		dump_gpio_func[i] = 0;
	}

	/* Clean software record */
	adc_en = MUX_0;

	/* Go through all functions */
	for (i = 0; i < PIN_FUNC_MAX; i++) {
		if (info->top_pinmux[i].pin_function != i) {
			pr_err("top_config[%d].pinFunction context error\n", i);
			/*Leave critical section*/
			unl_cpu(flags);
			return E_CTX;
		}

		err = pinmux_config_hdl[i](info->top_pinmux[i].config);
		if (err != E_OK) {
			pr_err("top_config[%d].config config error\n", i);
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
	TOP_SETREG(info, TOP_REG8_OFS, top_reg8.reg);
	TOP_SETREG(info, TOP_REG9_OFS, top_reg9.reg);
	TOP_SETREG(info, TOP_REG10_OFS, top_reg10.reg);
	TOP_SETREG(info, TOP_REG11_OFS, top_reg11.reg);
	//TOP_SETREG(info, TOP_REG12_OFS, top_reg12.reg);
	TOP_SETREG(info, TOP_REG13_OFS, top_reg13.reg);
	TOP_SETREG(info, TOP_REG14_OFS, top_reg14.reg);

	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGJGPIO0_OFS, top_reg_jgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGEGPIO0_OFS, top_reg_egpio0.reg);
	TOP_SETREG(info, TOP_REGBGPIO0_OFS, top_reg_bgpio0.reg);
	TOP_SETREG(info, TOP_REGAGPIO0_OFS, top_reg_agpio0.reg);

	/* Leave critical section */
	unl_cpu(flags);

	return E_OK;
}

void gpio_func_show(void)
{
	int i = 0;

	int gpio_count = 0;
	int func_num;
	//print C_GPIO
	for (i = CGPIO_0; i < C_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mC_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mC_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("C_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("C_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// J_GPIO
	gpio_count = 0;
	for (i = JGPIO_0; i < J_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mJ_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mJ_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("J_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("J_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// P_GPIO
	gpio_count = 0;
	for (i = PGPIO_0; i < P_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mP_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mP_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("P_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("P_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// E_GPIO
	gpio_count = 0;
	for (i = EGPIO_0; i < E_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mE_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mE_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("E_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("E_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// D_GPIO
	gpio_count = 0;
	for (i = DGPIO_0; i < D_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mD_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mD_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("D_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("D_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// B_GPIO
	gpio_count = 0;
	for (i = BGPIO_0; i < B_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {
			if (gpio_count < 10) {
				printk("\033[0;32mB_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mB_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("B_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("B_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	// A_GPIO
	gpio_count = 0;
	for (i = AGPIO_0; i < A_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mA_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mA_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}

		} else {
			if (gpio_count < 10) {
				printk("A_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("A_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

}
EXPORT_SYMBOL(gpio_func_show);
