/*
    Pinmux module driver.

    This file is the driver of Piumux module.

    @file       ns02302_pinmux_host.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#include "ns02302_pinmux.h"
// #include <plat/pad.h>


static DEFINE_SPINLOCK(top_lock);
#if 0
#define loc_cpu(flags) spin_lock_irqsave(&top_lock, flags)
#define unl_cpu(flags) spin_unlock_irqrestore(&top_lock, flags)
#else
#define loc_cpu(flags)
#define unl_cpu(flags)
#endif

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
union TOP_REG12        top_reg12;
union TOP_REG13        top_reg13;
union TOP_REG14        top_reg14;
union TOP_REG15        top_reg15;
union TOP_REG16        top_reg16;
union TOP_REG17        top_reg17;
union TOP_REG18        top_reg18;
union TOP_REGCGPIO0    top_reg_cgpio0;
union TOP_REGPGPIO0    top_reg_pgpio0;
union TOP_REGDGPIO0    top_reg_dgpio0;
union TOP_REGLGPIO0    top_reg_lgpio0;
union TOP_REGSGPIO0    top_reg_sgpio0;
union TOP_REGHSIGPIO0  top_reg_hsigpio0;
union TOP_REGAGPIO0    top_reg_agpio0;

int confl_detect;
static int dump_gpio_func[GPIO_MAX] = {0};
static char *dump_func[FUNC_total] = {"FSPI", "SDIO", "SDIO2", "SDIO3", "EJTAG", "EXTROM", "ETH", "I2C_1", "I2C2_1", "I2C2_2",
									  "I2C3_1", "I2C3_2", "I2C3_3", "I2C4_1", "I2C4_2", "I2C4_3", "I2C5_1", "PWM_1", "PWM_2", "PWM_3", "PWM_4", "PWM_5", "PWM1_1", "PWM1_2", "PWM1_3", "PWM1_4", "PWM1_5", "PWM2_1", "PWM2_2", "PWM2_3", "PWM2_4", "PWM2_5", "PWM3_1", "PWM3_2", "PWM3_3", "PWM3_4", "PWM3_5", "PWM4_1", "PWM4_2", "PWM4_3", "PWM4_4", "PWM4_5", "PWM5_1", "PWM5_2", "PWM5_3", "PWM5_4", "PWM5_5", "PWM6_1", "PWM6_2", "PWM6_3", "PWM6_4", "PWM6_5", "PWM7_1", "PWM7_2", "PWM7_3", "PWM7_4", "PWM7_5", "PWM8_1", "PWM8_2", "PWM8_4", "PWM8_5", "PWM9_1", "PWM9_2", "PWM9_4", "PWM9_5", "PWM10_1", "PWM10_2", "PWM10_4", "PWM10_5", "PWM11_1", "PWM11_2", "PWM11_4", "PWM11_5",
									  "CCNT", "CCNT2", "CCNT3", "SENSOR", "SENSOR2", "SENSOR3", "SENSORMISC", "SN1_MCLK", "SN1_XVSXHS", "SN2_MCLK", "SN2_XVSXHS", "SN3_MCLK", "SN3_XVSXHS",
									  "SN4_MCLK", "SN4_XVSXHS", "MIPI",
									  "I2S_1", "I2S_1_MCLK", "I2S_2", "I2S_2_MCLK",
									  "AUDIO_DMIC", "AUDIO_EXT_MCLK", "UART", "UART2_1", "UART2_2", "UART3_1", "UART3_2", "UART3_4", "UART3_5", "UART4_1", "UART4_2", "UART5_1",
									  "UART6_1", "UART2_CTS_RTS", "UART2_DTROE",
									  "UART3_CTS_RTS", "UART3_DTROE", "UART4_CTS_RTS", "UART4_DTROE", "UART5_CTS_RTS", "UART5_DTROE", "UART6_CTS_RTS", "UART6_DTROE",
									  "CSI", "CSI2", "CSI3", "CSI4", "Remote", "SDP_1", "SDP_2", "SPI_1", "SPI_2", "SPI_3",
									  "SPI2_1", "SPI2_2", "SPI3_1", "SPI3_2", "SPI4_1", "SPI4_2", "SPI5_1", "SPI5_2", "SPI3_RDY", "SPI3_RDY2",
									  "SIF_1", "SIF1_1", "SIF2_1", "SIF2_2", "SIF2_3", "SIF3_1", "MISC", "LCD"
									 };

struct nvt_pinctrl_info info_get_id[1] = {0};
// #include <linux/of.h>
#if 0
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
#endif

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

/*
static int logo_determination(u32 *lcd_type)
{
	u32 m_logo = 0x0;
	u32 de_en = 0x0;
	struct device_node* of_node = of_find_node_by_path("/logo");

	if (of_node) {
        of_property_read_u32(of_node, "enable", &m_logo);

        of_property_read_u32(of_node, "lcd_type", lcd_type);

		of_property_read_u32(of_node, "de_en", &de_en);

		if (de_en) {
			*lcd_type = *lcd_type|PINMUX_LCD_SEL_DE_ENABLE;
		}
	}

	return m_logo;
}
*/
static void gpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char *gpio_name[] = {"C_GPIO", "P_GPIO", "D_GPIO", "L_GPIO", "S_GPIO", "DSI_GPIO", "A_GPIO"};
	char name[10];

	if (start_offset == TOP_REGCGPIO0_OFS) {
		strcpy(name, gpio_name[0]);
	} else if (start_offset == TOP_REGPGPIO0_OFS) {
		strcpy(name, gpio_name[1]);
	} else if (start_offset == TOP_REGDGPIO0_OFS) {
		strcpy(name, gpio_name[2]);
	} else if (start_offset == TOP_REGLGPIO0_OFS) {
		strcpy(name, gpio_name[3]);
	} else if (start_offset == TOP_REGSGPIO0_OFS) {
		strcpy(name, gpio_name[4]);
	} else if (start_offset == TOP_REGHSIGPIO0_OFS) {
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
/*
	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
	gpio_info_show(info, L_GPIO_NUM, TOP_REGLGPIO0_OFS);
	gpio_info_show(info, S_GPIO_NUM, TOP_REGSGPIO0_OFS);
	gpio_info_show(info, HSI_GPIO_NUM, TOP_REGHSIGPIO0_OFS);
	gpio_info_show(info, A_GPIO_NUM, TOP_REGAGPIO0_OFS);
*/
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
	if (id <= PINMUX_FUNC_ID_LCD2) {
		return disp_pinmux_config[id] & ~(PINMUX_DISPMUX_SEL_MASK | PINMUX_LCD_SEL_FEATURE_MSK);
	} else if (id <= PINMUX_FUNC_ID_HDMI) {
		return disp_pinmux_config[id] & ~PINMUX_HDMI_CFG_MASK;
	}

	return 0;
}
//EXPORT_SYMBOL(pinmux_get_dispmode);


/**
    Read pinmux data from controller base

    @param[in] info nvt_pinctrl_info
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
	// union TOP_REG7 local_top_reg7;
	union TOP_REG8 local_top_reg8;
	union TOP_REG9 local_top_reg9;
	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;
	union TOP_REG12 local_top_reg12;
	union TOP_REG13 local_top_reg13;
	union TOP_REG14 local_top_reg14;
	union TOP_REG15 local_top_reg15;
	union TOP_REG16 local_top_reg16;
	union TOP_REG17 local_top_reg17;
	union TOP_REG18 local_top_reg18;

//	union TOP_REGSGPIO0 local_top_reg_sgpio0;
//	union TOP_REGHSIGPIO0 local_top_reg_hgpio0;

	/* Enter critical section */
	loc_cpu(flags);

	//local_top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	// local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg13.reg = TOP_GETREG(info, TOP_REG13_OFS);
	local_top_reg14.reg = TOP_GETREG(info, TOP_REG14_OFS);
	local_top_reg15.reg = TOP_GETREG(info, TOP_REG15_OFS);
	local_top_reg16.reg = TOP_GETREG(info, TOP_REG16_OFS);
	local_top_reg17.reg = TOP_GETREG(info, TOP_REG17_OFS);
	local_top_reg18.reg = TOP_GETREG(info, TOP_REG18_OFS);

//	local_top_reg_sgpio0.reg = TOP_GETREG(info, TOP_REGSGPIO0_OFS);
//	local_top_reg_hgpio0.reg = TOP_GETREG(info, TOP_REGHSIGPIO0_OFS);

	/* Parsing SDIO */
	value = PIN_SDIO_CFG_NONE;

	if (local_top_reg1.bit.SDIO_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO_1;
	}

	if (local_top_reg1.bit.SDIO2_EN == MUX_1)  {
		value |= PIN_SDIO_CFG_SDIO2_1;
	}

	if (local_top_reg1.bit.SDIO3_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_1;
	}
	if (local_top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_BUS_WIDTH;
	}
	if (local_top_reg1.bit.SDIO3_DS_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_DS;
	}

	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/* Parsing NAND */
	value = PIN_NAND_CFG_NONE;

	if (local_top_reg1.bit.FSPI_EN == MUX_1) {
		value |= PIN_NAND_CFG_NAND_1;
	} else if (local_top_reg1.bit.FSPI_EN == MUX_2) {
		value |= PIN_NAND_CFG_NAND_2;
	}
	if (local_top_reg1.bit.FSPI_CS1_EN) {
		value |= PIN_NAND_CFG_NAND_CS1;
	}

	info->top_pinmux[PIN_FUNC_NAND].config = value;
	info->top_pinmux[PIN_FUNC_NAND].pin_function = PIN_FUNC_NAND;

	/* Parsing ETH */
	value = PIN_ETH_CFG_NONE;

	if (local_top_reg3.bit.ETH == MUX_1) {
		value |= PIN_ETH_CFG_ETH_RMII_1;
	} else if (local_top_reg3.bit.ETH == MUX_2) {
		value |= PIN_ETH_CFG_ETH_RMII_2;
	} else if (local_top_reg3.bit.ETH == MUX_3) {
		value |= PIN_ETH_CFG_ETH_RGMII;
	}
	if (local_top_reg3.bit.ETH_MDIO_MUX == MUX_1) {
		value |= PIN_ETH_CFG_ETH_MDIO_1;
	} else if (local_top_reg3.bit.ETH_MDIO_MUX == MUX_2) {
		value |= PIN_ETH_CFG_ETH_MDIO_2;
	}
	if (local_top_reg3.bit.ETH_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH_EXTPHYCLK;
	}
	if (local_top_reg3.bit.ETH_PTP == MUX_1) {
		value |= PIN_ETH_CFG_ETH_PTP;
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
	} else if (local_top_reg4.bit.I2C2 == MUX_2) {
		value |= PIN_I2C_CFG_I2C2_2;
	}

	if (local_top_reg4.bit.I2C3 == MUX_1) {
		value |= PIN_I2C_CFG_I2C3_1;
	} else if (local_top_reg4.bit.I2C3 == MUX_2) {
		value |= PIN_I2C_CFG_I2C3_2;
	} else if (local_top_reg4.bit.I2C3 == MUX_3) {
		value |= PIN_I2C_CFG_I2C3_3;
	}

	if (local_top_reg4.bit.I2C4 == MUX_1) {
		value |= PIN_I2C_CFG_I2C4_1;
	} else if (local_top_reg4.bit.I2C4 == MUX_2) {
		value |= PIN_I2C_CFG_I2C4_2;
	} else if (local_top_reg4.bit.I2C4 == MUX_3) {
		value |= PIN_I2C_CFG_I2C4_3;
	}

	if (local_top_reg4.bit.I2C5 == MUX_1) {
		value |= PIN_I2C_CFG_I2C5_1;
	}

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/* Parsing PWM */
	value = PIN_PWM_CFG_NONE;


	if (local_top_reg5.bit.PWM0 == MUX_1) {
		value |= PIN_PWM_CFG_PWM0_1;
	} else if (local_top_reg5.bit.PWM0 == MUX_2) {
		value |= PIN_PWM_CFG_PWM0_2;
	} else if (local_top_reg5.bit.PWM0 == MUX_3) {
		value |= PIN_PWM_CFG_PWM0_3;
	} else if (local_top_reg5.bit.PWM0 == MUX_4) {
		value |= PIN_PWM_CFG_PWM0_4;
	} else if (local_top_reg5.bit.PWM0 == MUX_5) {
		value |= PIN_PWM_CFG_PWM0_5;
	}

	if (local_top_reg5.bit.PWM1 == MUX_1) {
		value |= PIN_PWM_CFG_PWM1_1;
	} else if (local_top_reg5.bit.PWM1 == MUX_2) {
		value |= PIN_PWM_CFG_PWM1_2;
	} else if (local_top_reg5.bit.PWM1 == MUX_3) {
		value |= PIN_PWM_CFG_PWM1_3;
	} else if (local_top_reg5.bit.PWM1 == MUX_4) {
		value |= PIN_PWM_CFG_PWM1_4;
	} else if (local_top_reg5.bit.PWM1 == MUX_5) {
		value |= PIN_PWM_CFG_PWM1_5;
	}

	if (local_top_reg5.bit.PWM2 == MUX_1) {
		value |= PIN_PWM_CFG_PWM2_1;
	} else if (local_top_reg5.bit.PWM2 == MUX_2) {
		value |= PIN_PWM_CFG_PWM2_2;
	} else if (local_top_reg5.bit.PWM2 == MUX_3) {
		value |= PIN_PWM_CFG_PWM2_3;
	} else if (local_top_reg5.bit.PWM2 == MUX_4) {
		value |= PIN_PWM_CFG_PWM2_4;
	} else if (local_top_reg5.bit.PWM2 == MUX_5) {
		value |= PIN_PWM_CFG_PWM2_5;
	}

	if (local_top_reg5.bit.PWM3 == MUX_1) {
		value |= PIN_PWM_CFG_PWM3_1;
	} else if (local_top_reg5.bit.PWM3 == MUX_2) {
		value |= PIN_PWM_CFG_PWM3_2;
	} else if (local_top_reg5.bit.PWM3 == MUX_3) {
		value |= PIN_PWM_CFG_PWM3_3;
	} else if (local_top_reg5.bit.PWM3 == MUX_4) {
		value |= PIN_PWM_CFG_PWM3_4;
	} else if (local_top_reg5.bit.PWM3 == MUX_5) {
		value |= PIN_PWM_CFG_PWM3_5;
	}

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/* Parsing PWMII */
	value = PIN_PWMII_CFG_NONE;


	if (local_top_reg5.bit.PWM4 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM4_1;
	} else if (local_top_reg5.bit.PWM4 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM4_2;
	} else if (local_top_reg5.bit.PWM4 == MUX_3) {
		value |= PIN_PWMII_CFG_PWM4_3;
	} else if (local_top_reg5.bit.PWM4 == MUX_4) {
		value |= PIN_PWMII_CFG_PWM4_4;
	} else if (local_top_reg5.bit.PWM4 == MUX_5) {
		value |= PIN_PWMII_CFG_PWM4_5;
	}

	if (local_top_reg5.bit.PWM5 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM5_1;
	} else if (local_top_reg5.bit.PWM5 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM5_2;
	} else if (local_top_reg5.bit.PWM5 == MUX_3) {
		value |= PIN_PWMII_CFG_PWM5_3;
	} else if (local_top_reg5.bit.PWM5 == MUX_4) {
		value |= PIN_PWMII_CFG_PWM5_4;
	} else if (local_top_reg5.bit.PWM5 == MUX_5) {
		value |= PIN_PWMII_CFG_PWM5_5;
	}

	if (local_top_reg5.bit.PWM6 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM6_1;
	} else if (local_top_reg5.bit.PWM6 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM6_2;
	} else if (local_top_reg5.bit.PWM6 == MUX_3) {
		value |= PIN_PWMII_CFG_PWM6_3;
	} else if (local_top_reg5.bit.PWM6 == MUX_4) {
		value |= PIN_PWMII_CFG_PWM6_4;
	} else if (local_top_reg5.bit.PWM6 == MUX_5) {
		value |= PIN_PWMII_CFG_PWM6_5;
	}

	if (local_top_reg5.bit.PWM7 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM7_1;
	} else if (local_top_reg5.bit.PWM7 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM7_2;
	} else if (local_top_reg5.bit.PWM7 == MUX_3) {
		value |= PIN_PWMII_CFG_PWM7_3;
	} else if (local_top_reg5.bit.PWM7 == MUX_4) {
		value |= PIN_PWMII_CFG_PWM7_4;
	} else if (local_top_reg5.bit.PWM7 == MUX_5) {
		value |= PIN_PWMII_CFG_PWM7_5;
	}

	info->top_pinmux[PIN_FUNC_PWMII].config = value;
	info->top_pinmux[PIN_FUNC_PWMII].pin_function = PIN_FUNC_PWMII;

	/* Parsing CCNT */
	value = PIN_CCNT_CFG_NONE;

	if (local_top_reg6.bit.PWM8 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM8_1;
	} else if (local_top_reg6.bit.PWM8 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM8_2;
	} else if (local_top_reg6.bit.PWM8 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM8_4;
	} else if (local_top_reg6.bit.PWM8 == MUX_5) {
		value |= PIN_PWMIII_CFG_PWM8_5;
	}

	if (local_top_reg6.bit.PWM9 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM9_1;
	} else if (local_top_reg6.bit.PWM9 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM9_2;
	} else if (local_top_reg6.bit.PWM9 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM9_4;
	} else if (local_top_reg6.bit.PWM9 == MUX_5) {
		value |= PIN_PWMIII_CFG_PWM9_5;
	}

	if (local_top_reg6.bit.PWM10 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM10_1;
	} else if (local_top_reg6.bit.PWM10 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM10_2;
	} else if (local_top_reg6.bit.PWM10 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM10_4;
	} else if (local_top_reg6.bit.PWM10 == MUX_5) {
		value |= PIN_PWMIII_CFG_PWM10_5;
	}

	if (local_top_reg6.bit.PWM11 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM11_1;
	} else if (local_top_reg6.bit.PWM11 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM11_2;
	} else if (local_top_reg6.bit.PWM11 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM11_4;
	} else if (local_top_reg6.bit.PWM11 == MUX_5) {
		value |= PIN_PWMIII_CFG_PWM11_5;
	}

	info->top_pinmux[PIN_FUNC_PWMIII].config = value;
	info->top_pinmux[PIN_FUNC_PWMIII].pin_function = PIN_FUNC_PWMIII;

	/* Parsing CCNT */
	value = PIN_CCNT_CFG_NONE;

	if (local_top_reg6.bit.PICNT == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT_1;
	}

	if (local_top_reg6.bit.PICNT2 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT2_1;
	}

	if (local_top_reg6.bit.PICNT3 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT3_1;
	}

	info->top_pinmux[PIN_FUNC_CCNT].config = value;
	info->top_pinmux[PIN_FUNC_CCNT].pin_function = PIN_FUNC_CCNT;

	/* Parsing SENSOR */
	value = PIN_SENSOR_CFG_NONE;

	if (local_top_reg8.bit.SENSOR == MUX_1) {
		value |= PIN_SENSOR_CFG_12BITS;
	}

	info->top_pinmux[PIN_FUNC_SENSOR].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR].pin_function = PIN_FUNC_SENSOR;

	/* Parsing SENSOR2 */
	value = PIN_SENSOR2_CFG_NONE;

	if (local_top_reg8.bit.SENSOR2 == MUX_1) {
		value |= PIN_SENSOR2_CFG_12BITS;
	} else if (local_top_reg8.bit.SENSOR2 == MUX_2) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_A;
	} else if (local_top_reg8.bit.SENSOR2 == MUX_3) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_B;
	} else if (local_top_reg8.bit.SENSOR2 == MUX_4) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_AB;
	} else if (local_top_reg8.bit.SENSOR2 == MUX_5) {
		value |= PIN_SENSOR2_CFG_CCIR16BITS;
	}

	if (local_top_reg8.bit.SN2_CCIR_VSHS == MUX_1) {
		value |= PIN_SENSOR2_CFG_CCIR_VSHS;
	}

	info->top_pinmux[PIN_FUNC_SENSOR2].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR2].pin_function = PIN_FUNC_SENSOR2;

	/* Parsing SENSOR3 */
	value = PIN_SENSOR3_CFG_NONE;

	if (local_top_reg8.bit.SENSOR3 == MUX_1) {
		value |= PIN_SENSOR3_CFG_12BITS;
	} else if (local_top_reg8.bit.SENSOR3 == MUX_2) {
		value |= PIN_SENSOR3_CFG_CCIR8BITS_A;
	} else if (local_top_reg8.bit.SENSOR3 == MUX_3) {
		value |= PIN_SENSOR3_CFG_CCIR16BITS;
	}

	if (local_top_reg8.bit.SN3_CCIR_VSHS == MUX_1) {
		value |= PIN_SENSOR3_CFG_CCIR_VSHS;
	}

	info->top_pinmux[PIN_FUNC_SENSOR3].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR3].pin_function = PIN_FUNC_SENSOR3;

	/* Parsing SENSORMISC */
	value = PIN_SENSORMISC_CFG_NONE;


	if (local_top_reg9.bit.SN_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_MCLK_1;
	}
	if (local_top_reg9.bit.SN2_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_MCLK_1;
	} else if (local_top_reg9.bit.SN2_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN2_MCLK_2;
	}
	if (local_top_reg9.bit.SN3_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN3_MCLK_1;
	}
	if (local_top_reg9.bit.SN4_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN4_MCLK_1;
	} else if (local_top_reg9.bit.SN4_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN4_MCLK_2;
	}

	if (local_top_reg9.bit.SN_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_XVSXHS_1;
	}

	if (local_top_reg9.bit.SN2_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_XVSXHS_1;
	}

	if (local_top_reg9.bit.SN3_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN3_XVSXHS_1;
	}

	if (local_top_reg9.bit.SN4_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN4_XVSXHS_1;
	}

	if (local_top_reg8.bit.FLASH_TRIG_IN == MUX_1) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1;
	} else if (local_top_reg8.bit.FLASH_TRIG_IN == MUX_2) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2;
	}

	if (local_top_reg8.bit.FLASH_TRIG_OUT == MUX_1) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1;
	} else if (local_top_reg8.bit.FLASH_TRIG_OUT == MUX_2) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2;
	}

	info->top_pinmux[PIN_FUNC_SENSORMISC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORMISC].pin_function = PIN_FUNC_SENSORMISC;

	/* Parsing SENSORSYNC */
	value = PIN_SENSORSYNC_CFG_NONE;

	if (local_top_reg10.bit.SN1_MCLK_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN;
	} else if (local_top_reg10.bit.SN1_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN2;
	} else if (local_top_reg10.bit.SN1_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN3;
	} else if (local_top_reg10.bit.SN1_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN4;
	}

	if (local_top_reg10.bit.SN1_XVSHS_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN;
	} else if (local_top_reg10.bit.SN1_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN2;
	} else if (local_top_reg10.bit.SN1_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN3;
	} else if (local_top_reg10.bit.SN1_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN4;
	}

	if (local_top_reg10.bit.SN2_MCLK_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN;
	} else if (local_top_reg10.bit.SN2_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN2;
	} else if (local_top_reg10.bit.SN2_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN3;
	} else if (local_top_reg10.bit.SN2_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN4;
	}

	if (local_top_reg10.bit.SN2_XVSHS_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN;
	} else if (local_top_reg10.bit.SN2_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2;
	} else if (local_top_reg10.bit.SN2_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN3;
	} else if (local_top_reg10.bit.SN2_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN4;
	}

	if (local_top_reg10.bit.SN3_MCLK_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN;
	} else if (local_top_reg10.bit.SN3_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2;
	} else if (local_top_reg10.bit.SN3_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN3;
	} else if (local_top_reg10.bit.SN3_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN4;
	}

	if (local_top_reg10.bit.SN3_XVSHS_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN;
	} else if (local_top_reg10.bit.SN3_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2;
	} else if (local_top_reg10.bit.SN3_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN3;
	} else if (local_top_reg10.bit.SN3_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN4;
	}

	if (local_top_reg10.bit.SN4_MCLK_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN;
	} else if (local_top_reg10.bit.SN4_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2;
	} else if (local_top_reg10.bit.SN4_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3;
	} else if (local_top_reg10.bit.SN4_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN4;
	}

	if (local_top_reg10.bit.SN4_XVSHS_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN;
	} else if (local_top_reg10.bit.SN4_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2;
	} else if (local_top_reg10.bit.SN4_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3;
	} else if (local_top_reg10.bit.SN4_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN4;
	}

	info->top_pinmux[PIN_FUNC_SENSORSYNC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORSYNC].pin_function = PIN_FUNC_SENSORSYNC;

	/* Parsing AUDIO */
	value = PIN_AUDIO_CFG_NONE;

	if (local_top_reg11.bit.I2S == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_1;
	} else if (local_top_reg11.bit.I2S == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_2;
	}
	if (local_top_reg11.bit.I2S_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_1;
	} else if (local_top_reg11.bit.I2S_MCLK == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_2;
	}

	if (local_top_reg11.bit.DMIC == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_1;
	} else if (local_top_reg11.bit.DMIC == MUX_2) {
		value |= PIN_AUDIO_CFG_DMIC_2;
	} else if (local_top_reg11.bit.DMIC == MUX_3) {
		value |= PIN_AUDIO_CFG_DMIC_3;
	}

	if (local_top_reg11.bit.DMIC_DATA0 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA0;
	}
	if (local_top_reg11.bit.DMIC_DATA1 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA1;
	}

	if (local_top_reg11.bit.EXT_EAC_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_EXT_EAC_MCLK;
	}

	info->top_pinmux[PIN_FUNC_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_AUDIO].pin_function = PIN_FUNC_AUDIO;

	/* Parsing UART */
	value = PIN_UART_CFG_NONE;

	if (local_top_reg12.bit.UART == MUX_1) {
		value |= PIN_UART_CFG_UART_1;
	}

	if (local_top_reg12.bit.UART2 == MUX_1) {
		value |= PIN_UART_CFG_UART2_1;
	} else if (local_top_reg12.bit.UART2 == MUX_2) {
		value |= PIN_UART_CFG_UART2_2;
	}

	if (local_top_reg13.bit.UART2_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART2_RTSCTS;
	} else if (local_top_reg13.bit.UART2_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART2_DIROE;
	}

	if (local_top_reg12.bit.UART3 == MUX_1) {
		value |= PIN_UART_CFG_UART3_1;
	} else if (local_top_reg12.bit.UART3 == MUX_2) {
		value |= PIN_UART_CFG_UART3_2;
	} else if (local_top_reg12.bit.UART3 == MUX_3) {
		value |= PIN_UART_CFG_UART3_4;
	} else if (local_top_reg12.bit.UART3 == MUX_4) {
		value |= PIN_UART_CFG_UART3_5;
	}

	if (local_top_reg13.bit.UART3_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART3_RTSCTS;
	} else if (local_top_reg13.bit.UART3_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART3_DIROE;
	}

	if (local_top_reg12.bit.UART4 == MUX_1) {
		value |= PIN_UART_CFG_UART4_1;
	} else if (local_top_reg12.bit.UART4 == MUX_2) {
		value |= PIN_UART_CFG_UART4_2;
	}
	if (local_top_reg13.bit.UART4_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART4_RTSCTS;
	} else if (local_top_reg13.bit.UART4_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART4_DIROE;
	}

	if (local_top_reg12.bit.UART5 == MUX_1) {
		value |= PIN_UART_CFG_UART5_1;
	}
	if (local_top_reg13.bit.UART5_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART5_RTSCTS;
	} else if (local_top_reg13.bit.UART5_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART5_DIROE;
	}

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/* Parsing UARTII */
	value = PIN_UARTII_CFG_NONE;

	if (local_top_reg12.bit.UART6 == MUX_1) {
		value |= PIN_UARTII_CFG_UART6_1;
	}
	if (local_top_reg13.bit.UART6_RTSCTS == MUX_1) {
		value |= PIN_UARTII_CFG_UART6_RTSCTS;
	} else if (local_top_reg13.bit.UART6_RTSCTS == MUX_2) {
		value |= PIN_UARTII_CFG_UART6_DIROE;
	}

	info->top_pinmux[PIN_FUNC_UARTII].config = value;
	info->top_pinmux[PIN_FUNC_UARTII].pin_function = PIN_FUNC_UARTII;

	/* Parsing CSI */
	value = PIN_CSI_CFG_NONE;

	if (local_top_reg14.bit.CSI == MUX_1) {
		value |= PIN_CSI_CFG_CSI;
	}

	if (local_top_reg14.bit.CSI2 == MUX_1) {
		value |= PIN_CSI_CFG_CSI2;
	}

	if (local_top_reg14.bit.CSI3 == MUX_1) {
		value |= PIN_CSI_CFG_CSI3;
	}

	if (local_top_reg14.bit.CSI4 == MUX_1) {
		value |= PIN_CSI_CFG_CSI4;
	}

	info->top_pinmux[PIN_FUNC_CSI].config = value;
	info->top_pinmux[PIN_FUNC_CSI].pin_function = PIN_FUNC_CSI;


	/* Parsing REMOTE */
	value = PIN_REMOTE_CFG_NONE;

	if (local_top_reg15.bit.REMOTE == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_1;
	}

	if (local_top_reg15.bit.REMOTE_EXT == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_EXT_1;
	}

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/* Parsing SDP */
	value = PIN_SDP_CFG_NONE;

	if (local_top_reg15.bit.SDP == MUX_1) {
		value |= PIN_SDP_CFG_SDP_1;
	} else if (local_top_reg15.bit.SDP == MUX_2) {
		value |= PIN_SDP_CFG_SDP_2;
	}

	info->top_pinmux[PIN_FUNC_SDP].config = value;
	info->top_pinmux[PIN_FUNC_SDP].pin_function = PIN_FUNC_SDP;

	/* Parsing SPI */
	value = PIN_SPI_CFG_NONE;

	if (local_top_reg16.bit.SPI == MUX_1) {
		value |= PIN_SPI_CFG_SPI_1;
	} else if (local_top_reg16.bit.SPI == MUX_2) {
		value |= PIN_SPI_CFG_SPI_2;
	} else if (local_top_reg16.bit.SPI == MUX_3) {
		value |= PIN_SPI_CFG_SPI_3;
	}
	if (local_top_reg16.bit.SPI_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI2 == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_1;
	}
	if (local_top_reg16.bit.SPI2_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI3 == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_1;
	} else if (local_top_reg16.bit.SPI3 == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_2;
	}
	if (local_top_reg16.bit.SPI3_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI3_RDY == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_RDY_1;
	} else if (local_top_reg16.bit.SPI3_RDY == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_RDY_2;
	}

	if (local_top_reg16.bit.SPI4 == MUX_1) {
		value |= PIN_SPI_CFG_SPI4_1;
	} else if (local_top_reg16.bit.SPI4 == MUX_2) {
		value |= PIN_SPI_CFG_SPI4_2;
	}
	if (local_top_reg16.bit.SPI4_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI4_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI5 == MUX_1) {
		value |= PIN_SPI_CFG_SPI5_1;
	} else if (local_top_reg16.bit.SPI5 == MUX_2) {
		value |= PIN_SPI_CFG_SPI5_2;
	}
	if (local_top_reg16.bit.SPI5_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI5_BUS_WIDTH;
	}

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/* Parsing SIF */
	value = PIN_SIF_CFG_NONE;

	if (local_top_reg17.bit.SIF0 == MUX_1) {
		value |= PIN_SIF_CFG_SIF0_1;
	}

	if (local_top_reg17.bit.SIF1 == MUX_1) {
		value |= PIN_SIF_CFG_SIF1_1;
	}

	if (local_top_reg17.bit.SIF2 == MUX_1) {
		value |= PIN_SIF_CFG_SIF2_1;
	} else if (local_top_reg17.bit.SIF2 == MUX_2) {
		value |= PIN_SIF_CFG_SIF2_2;
	} else if (local_top_reg17.bit.SIF2 == MUX_3) {
		value |= PIN_SIF_CFG_SIF2_3;
	}

	if (local_top_reg17.bit.SIF3 == MUX_1) {
		value |= PIN_SIF_CFG_SIF3_1;
	}

	info->top_pinmux[PIN_FUNC_SIF].config = value;
	info->top_pinmux[PIN_FUNC_SIF].pin_function = PIN_FUNC_SIF;

	/* Parsing MISC */
	value = PIN_MISC_CFG_NONE;

	if (local_top_reg18.bit.RTC_CLK == MUX_1) {
		value |= PIN_MISC_CFG_RTC_CLK_1;
	}

	if (local_top_reg18.bit.SP_CLK == MUX_1) {
		value |= PIN_MISC_CFG_SP_CLK_1;
	} else if (local_top_reg18.bit.SP_CLK == MUX_2) {
		value |= PIN_MISC_CFG_SP_CLK_2;
	} else if (local_top_reg18.bit.SP_CLK == MUX_3) {
		value |= PIN_MISC_CFG_SP_CLK_3;
	}

	if (local_top_reg18.bit.SP_CLK2 == MUX_1) {
		value |= PIN_MISC_CFG_SP2_CLK_1;
	} else if (local_top_reg18.bit.SP_CLK2 == MUX_2) {
		value |= PIN_MISC_CFG_SP2_CLK_2;
	} else if (local_top_reg18.bit.SP_CLK2 == MUX_3) {
		value |= PIN_MISC_CFG_SP2_CLK_3;
	}

	if (adc_en == MUX_1) {
		value |= PIN_MISC_CFG_ADC;
	}

	if (local_top_reg2.bit.MIPI_SEL == MUX_0) {
		value |= PIN_MISC_CFG_MIPI_SEL_DSI;
	} else if (local_top_reg2.bit.MIPI_SEL == MUX_1) {
		value |= PIN_MISC_CFG_MIPI_SEL_CSI_TX;
	}

	if (local_top_reg1.bit.EJTAG_EN == MUX_1) {
		value |= PIN_MISC_CFG_CPU_ICE;
	}

	info->top_pinmux[PIN_FUNC_MISC].config = value;
	info->top_pinmux[PIN_FUNC_MISC].pin_function = PIN_FUNC_MISC;

	/* Parsing LCD */
	info->top_pinmux[PIN_FUNC_LCD].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD];
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/* Parsing TV */
	info->top_pinmux[PIN_FUNC_TV].config = disp_pinmux_config[PINMUX_FUNC_ID_TV];
	info->top_pinmux[PIN_FUNC_TV].pin_function = PIN_FUNC_TV;

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
		} else {
			value = PINMUX_LCD_SEL_CCIR601;

			if (local_top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
				value |= PINMUX_LCD_SEL_HVLD_VVLD;
			}

			if (local_top_reg2.bit.CCIR_FIELD == MUX_1) {
				value |= PINMUX_LCD_SEL_FIELD;
			}
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_3) {
		value = PINMUX_LCD_SEL_PARALLE_RGB565;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_4) {
		value = PINMUX_LCD_SEL_SERIAL_RGB_8BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_5) {
		value = PINMUX_LCD_SEL_SERIAL_RGB_6BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_6) {
		value = PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_7) {
		value = PINMUX_LCD_SEL_RGB_16BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_8) {
		value = PINMUX_LCD_SEL_PARALLE_RGB666;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_9) {
		value = PINMUX_LCD_SEL_MIPI;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_10) {
		value = PINMUX_LCD_SEL_PARALLE_RGB888;
	}

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
		} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2) {
			value = PINMUX_LCD_SEL_PARALLE_MI_16BITS;
		} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3) {
			value = PINMUX_LCD_SEL_PARALLE_MI_18BITS;
		}
	}

	if (local_top_reg2.bit.TE_SEL == MUX_1) {
		value |= PINMUX_LCD_SEL_TE_ENABLE;
	}

	if (local_top_reg2.bit.PLCD_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD].pin_function = PIN_FUNC_SEL_LCD;

	/* Leave critical section */
	unl_cpu(flags);
}

void gpio_func_keep(int start, int count, int func)
{
	int i = 0;

	for (i = start; i < start + count; i++) {
		dump_gpio_func[i] = func;
	}
}

int gpio_conflict_detect(int start, int count, int func)
{
	int i = 0;
	int confl_mod;
	int confl_flag = 0;
	for (i = start; i < start + count; i++) {
		confl_mod = dump_gpio_func[i];
		//printf("%d\r\n",confl_mod);

		if (confl_mod > 0) {
			printk("%s conflict with %s\r\n", dump_func[func - 1], dump_func[confl_mod - 1]);
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

			confl_detect += gpio_conflict_detect(CGPIO_12, 6, func_SDIO);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_12, 6, func_SDIO);
		}

		if (config & PIN_SDIO_CFG_SDIO2_1) {

			confl_detect += gpio_conflict_detect(CGPIO_18, 6, func_SDIO2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO2_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_18, 6, func_SDIO2);
		}

		if (config & PIN_SDIO_CFG_SDIO3_1) {

			confl_detect += gpio_conflict_detect(CGPIO_0, 4, func_SDIO3);
			confl_detect += gpio_conflict_detect(CGPIO_8, 2, func_SDIO3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.SDIO3_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 4, func_SDIO3);
			gpio_func_keep(CGPIO_8, 2, func_SDIO3);
			if (config & PIN_SDIO_CFG_SDIO3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_4, 4, func_SDIO3);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.SDIO3_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_4, 4, func_SDIO3);
			}

			if (config & PIN_SDIO_CFG_SDIO3_DS) {

				confl_detect += gpio_conflict_detect(CGPIO_10, 1, func_SDIO3);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg1.bit.SDIO3_DS_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_GPIO;
				gpio_func_keep(CGPIO_10, 1, func_SDIO3);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_nand(uint32_t config)
{
	if (config == PIN_NAND_CFG_NONE) {
	} else {
		if (config & PIN_NAND_CFG_NAND_1) {

			confl_detect += gpio_conflict_detect(CGPIO_0, 4, func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_8, 1, func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_11, 1, func_FSPI);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.FSPI_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 4, func_FSPI);
			gpio_func_keep(CGPIO_8, 1, func_FSPI);
			gpio_func_keep(CGPIO_11, 1, func_FSPI);
		}
		if (config & PIN_NAND_CFG_NAND_2) {

			confl_detect += gpio_conflict_detect(CGPIO_0, 9, func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_10, 2, func_FSPI);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.FSPI_EN = MUX_2;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 9, func_FSPI);
			gpio_func_keep(CGPIO_10, 2, func_FSPI);
		}

		if (config & PIN_NAND_CFG_NAND_CS1) {

			confl_detect += gpio_conflict_detect(CGPIO_9, 1, func_FSPI);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.FSPI_CS1_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_9, 1, func_FSPI);
		}
	}

	return E_OK;
}

static int pinmux_config_eth(uint32_t config)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & PIN_ETH_CFG_ETH_RMII_1) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 7, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 7, func_ETH);
			if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {

				confl_detect += gpio_conflict_detect(LGPIO_9, 1, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_EXT_PHY_CLK = MUX_1;
				top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_9, 1, func_ETH);
			}
			if (config & PIN_ETH_CFG_ETH_MDIO_1) {

				confl_detect += gpio_conflict_detect(LGPIO_7, 2, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_MDIO_MUX = MUX_1;
				top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_7, 2, func_ETH);
			}
		} else if (config & PIN_ETH_CFG_ETH_RMII_2) {

			confl_detect += gpio_conflict_detect(LGPIO_11, 3, func_ETH);
			confl_detect += gpio_conflict_detect(LGPIO_17, 4, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_2;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_11, 3, func_ETH);
			gpio_func_keep(LGPIO_17, 4, func_ETH);
			if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {

				confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}
				top_reg3.bit.ETH_EXT_PHY_CLK = MUX_1;
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_0, 1, func_ETH);
			}
			if (config & PIN_ETH_CFG_ETH_MDIO_2) {

				confl_detect += gpio_conflict_detect(LGPIO_23, 2, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_MDIO_MUX = MUX_2;
				top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_23, 2, func_ETH);
			}
		} else if (config & PIN_ETH_CFG_ETH_RGMII) {

			confl_detect += gpio_conflict_detect(LGPIO_11, 12, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_3;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_11, 12, func_ETH);
			if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {

				confl_detect += gpio_conflict_detect(DGPIO_0, 1, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_EXT_PHY_CLK = MUX_1;
				top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_0, 1, func_ETH);
			}
			if (config & PIN_ETH_CFG_ETH_MDIO_2) {

				confl_detect += gpio_conflict_detect(LGPIO_23, 2, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_MDIO_MUX = MUX_2;
				top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_23, 2, func_ETH);
			}
		} else {
			if (config & PIN_ETH_CFG_ETH_MDIO_1) {

				confl_detect += gpio_conflict_detect(LGPIO_7, 2, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_MDIO_MUX = MUX_1;
				top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_7, 2, func_ETH);
			} else if (config & PIN_ETH_CFG_ETH_MDIO_2) {

				confl_detect += gpio_conflict_detect(LGPIO_23, 2, func_ETH);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.ETH_MDIO_MUX = MUX_2;
				top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_23, 2, func_ETH);
			}
		}

		if (config & PIN_ETH_CFG_ETH_PTP) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH_PTP = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH_LED_1) {

			confl_detect += gpio_conflict_detect(DGPIO_0, 2, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH_LED = MUX_1;
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_0, 2, func_ETH);
		} else if (config & PIN_ETH_CFG_ETH_LED_2) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 2, func_ETH);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.ETH_LED = MUX_2;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 2, func_ETH);
		}
	}

	return E_OK;
}

static int pinmux_config_i2c(uint32_t config)
{
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & PIN_I2C_CFG_I2C_1) {

			confl_detect += gpio_conflict_detect(SGPIO_4, 2, func_I2C);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_1;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
			gpio_func_keep(SGPIO_4, 2, func_I2C);
		}

		if (config & PIN_I2C_CFG_I2C2_1) {

			confl_detect += gpio_conflict_detect(SGPIO_10, 2, func_I2C2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_1;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO10, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO11, PAD_NONE);
			gpio_func_keep(SGPIO_10, 2, func_I2C2_1);
		} else if (config & PIN_I2C_CFG_I2C2_2) {

			confl_detect += gpio_conflict_detect(CGPIO_18, 2, func_I2C2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_CGPIO18, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
			gpio_func_keep(CGPIO_18, 2, func_I2C2_2);
		}

		if (config & PIN_I2C_CFG_I2C3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 2, func_I2C3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
			gpio_func_keep(PGPIO_21, 2, func_I2C3_1);
		} else if (config & PIN_I2C_CFG_I2C3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 2, func_I2C3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_CGPIO12, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
			gpio_func_keep(CGPIO_12, 2, func_I2C3_2);
		} else if (config & PIN_I2C_CFG_I2C3_3) {

			confl_detect += gpio_conflict_detect(SGPIO_12, 2, func_I2C3_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
			gpio_func_keep(SGPIO_12, 2, func_I2C3_3);
		}

		if (config & PIN_I2C_CFG_I2C4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 2, func_I2C4_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
			gpio_func_keep(PGPIO_11, 2, func_I2C4_1);
		} else if (config & PIN_I2C_CFG_I2C4_2) {

			confl_detect += gpio_conflict_detect(CGPIO_6, 2, func_I2C4_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
			gpio_func_keep(CGPIO_6, 2, func_I2C4_2);
		} else if (config & PIN_I2C_CFG_I2C4_3) {

			confl_detect += gpio_conflict_detect(SGPIO_14, 2, func_I2C4_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
			gpio_func_keep(SGPIO_14, 2, func_I2C4_3);
		}

		if (config & PIN_I2C_CFG_I2C5_1) {

			confl_detect += gpio_conflict_detect(PGPIO_2, 2, func_I2C5_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.I2C5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO2, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO3, PAD_NONE);
			gpio_func_keep(PGPIO_2, 2, func_I2C5_1);
		}
	}

	return E_OK;
}

static int pinmux_config_pwm(uint32_t config)
{
	if (config == PIN_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PWM_CFG_PWM0_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 1, func_PWM_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 1, func_PWM_1);
		} else if (config & PIN_PWM_CFG_PWM0_2) {

			confl_detect += gpio_conflict_detect(CGPIO_14, 1, func_PWM_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 1, func_PWM_2);
		} else if (config & PIN_PWM_CFG_PWM0_3) {

			confl_detect += gpio_conflict_detect(SGPIO_1, 1, func_PWM_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_1, 1, func_PWM_3);
		} else if (config & PIN_PWM_CFG_PWM0_4) {

			confl_detect += gpio_conflict_detect(CGPIO_4, 1, func_PWM_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_4, 1, func_PWM_4);
		} else if (config & PIN_PWM_CFG_PWM0_5) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 1, func_PWM_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 1, func_PWM_5);
		}

		if (config & PIN_PWM_CFG_PWM1_1) {

			confl_detect += gpio_conflict_detect(PGPIO_1, 1, func_PWM1_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_1, 1, func_PWM1_1);
		} else if (config & PIN_PWM_CFG_PWM1_2) {

			confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_PWM1_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_15, 1, func_PWM1_2);
		} else if (config & PIN_PWM_CFG_PWM1_3) {

			confl_detect += gpio_conflict_detect(SGPIO_2, 1, func_PWM1_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_2, 1, func_PWM1_3);
		} else if (config & PIN_PWM_CFG_PWM1_4) {

			confl_detect += gpio_conflict_detect(CGPIO_5, 1, func_PWM1_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_5, 1, func_PWM1_4);
		} else if (config & PIN_PWM_CFG_PWM1_5) {

			confl_detect += gpio_conflict_detect(LGPIO_1, 1, func_PWM1_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_1, 1, func_PWM1_5);
		}


		if (config & PIN_PWM_CFG_PWM2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_2, 1, func_PWM2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_2, 1, func_PWM2_1);
		} else if (config & PIN_PWM_CFG_PWM2_2) {

			confl_detect += gpio_conflict_detect(CGPIO_16, 1, func_PWM2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_16, 1, func_PWM2_2);
		} else if (config & PIN_PWM_CFG_PWM2_3) {

			confl_detect += gpio_conflict_detect(SGPIO_3, 1, func_PWM2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_3, 1, func_PWM2_3);
		} else if (config & PIN_PWM_CFG_PWM2_4) {

			confl_detect += gpio_conflict_detect(CGPIO_6, 1, func_PWM2_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_6, 1, func_PWM2_4);
		} else if (config & PIN_PWM_CFG_PWM2_5) {

			confl_detect += gpio_conflict_detect(LGPIO_2, 1, func_PWM2_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_2, 1, func_PWM2_5);
		}

		if (config & PIN_PWM_CFG_PWM3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_3, 1, func_PWM3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_3, 1, func_PWM3_1);
		} else if (config & PIN_PWM_CFG_PWM3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_17, 1, func_PWM3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 1, func_PWM3_2);
		} else if (config & PIN_PWM_CFG_PWM3_3) {

			confl_detect += gpio_conflict_detect(SGPIO_4, 1, func_PWM3_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_4, 1, func_PWM3_3);
		} else if (config & PIN_PWM_CFG_PWM3_4) {

			confl_detect += gpio_conflict_detect(CGPIO_7, 1, func_PWM3_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_7, 1, func_PWM3_4);
		} else if (config & PIN_PWM_CFG_PWM3_5) {

			confl_detect += gpio_conflict_detect(LGPIO_3, 1, func_PWM3_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_3, 1, func_PWM3_5);
		}
	}

	return E_OK;
}

static int pinmux_config_pwmII(uint32_t config)
{
	if (config == PIN_PWMII_CFG_NONE) {
	} else {
		if (config & PIN_PWMII_CFG_PWM4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_4, 1, func_PWM4_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 1, func_PWM4_1);
		} else if (config & PIN_PWMII_CFG_PWM4_2) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 1, func_PWM4_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_12, 1, func_PWM4_2);
		} else if (config & PIN_PWMII_CFG_PWM4_3) {

			confl_detect += gpio_conflict_detect(SGPIO_5, 1, func_PWM4_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_5, 1, func_PWM4_3);
		} else if (config & PIN_PWMII_CFG_PWM4_4) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 1, func_PWM4_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_3, 1, func_PWM4_4);
		} else if (config & PIN_PWMII_CFG_PWM4_5) {

			confl_detect += gpio_conflict_detect(LGPIO_4, 1, func_PWM4_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_4, 1, func_PWM4_5);
		}

		if (config & PIN_PWMII_CFG_PWM5_1) {

			confl_detect += gpio_conflict_detect(PGPIO_5, 1, func_PWM5_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_5, 1, func_PWM5_1);
		} else if (config & PIN_PWMII_CFG_PWM5_2) {

			confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_PWM5_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 1, func_PWM5_2);
		} else if (config & PIN_PWMII_CFG_PWM5_3) {

			confl_detect += gpio_conflict_detect(SGPIO_6, 1, func_PWM5_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_6, 1, func_PWM5_3);
		} else if (config & PIN_PWMII_CFG_PWM5_4) {

			confl_detect += gpio_conflict_detect(DGPIO_4, 1, func_PWM5_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_4, 1, func_PWM5_4);
		} else if (config & PIN_PWMII_CFG_PWM5_5) {

			confl_detect += gpio_conflict_detect(LGPIO_5, 1, func_PWM5_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_5, 1, func_PWM5_5);
		}

		if (config & PIN_PWMII_CFG_PWM6_1) {

			confl_detect += gpio_conflict_detect(PGPIO_6, 1, func_PWM6_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_6, 1, func_PWM6_1);
		} else if (config & PIN_PWMII_CFG_PWM6_2) {

			confl_detect += gpio_conflict_detect(CGPIO_18, 1, func_PWM6_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_18, 1, func_PWM6_2);
		} else if (config & PIN_PWMII_CFG_PWM6_3) {

			confl_detect += gpio_conflict_detect(SGPIO_7, 1, func_PWM6_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_7, 1, func_PWM6_3);
		} else if (config & PIN_PWMII_CFG_PWM6_4) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_PWM6_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_PWM6_4);
		} else if (config & PIN_PWMII_CFG_PWM6_5) {

			confl_detect += gpio_conflict_detect(LGPIO_6, 1, func_PWM6_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_6, 1, func_PWM6_5);
		}

		if (config & PIN_PWMII_CFG_PWM7_1) {

			confl_detect += gpio_conflict_detect(PGPIO_7, 1, func_PWM7_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_7, 1, func_PWM7_1);
		} else if (config & PIN_PWMII_CFG_PWM7_2) {

			confl_detect += gpio_conflict_detect(CGPIO_19, 1, func_PWM7_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_19, 1, func_PWM7_2);
		} else if (config & PIN_PWMII_CFG_PWM7_3) {

			confl_detect += gpio_conflict_detect(SGPIO_8, 1, func_PWM7_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_8, 1, func_PWM7_3);
		} else if (config & PIN_PWMII_CFG_PWM7_4) {

			confl_detect += gpio_conflict_detect(DGPIO_6, 1, func_PWM7_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_6, 1, func_PWM7_4);
		} else if (config & PIN_PWMII_CFG_PWM7_5) {

			confl_detect += gpio_conflict_detect(LGPIO_7, 1, func_PWM7_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_7, 1, func_PWM7_5);
		}
	}

	return E_OK;
}

static int pinmux_config_pwmIII(uint32_t config)
{
	if (config == PIN_PWMII_CFG_NONE) {
	} else {
		if (config & PIN_PWMIII_CFG_PWM8_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_PWM8_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM8 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_PWM8_1);
		} else if (config & PIN_PWMIII_CFG_PWM8_2) {

			confl_detect += gpio_conflict_detect(CGPIO_20, 1, func_PWM8_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM8 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_20, 1, func_PWM8_2);
		} else if (config & PIN_PWMIII_CFG_PWM8_4) {

			confl_detect += gpio_conflict_detect(DGPIO_8, 1, func_PWM8_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM8 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_8, 1, func_PWM8_4);
		} else if (config & PIN_PWMIII_CFG_PWM8_5) {

			confl_detect += gpio_conflict_detect(LGPIO_8, 1, func_PWM8_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM8 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_8, 1, func_PWM8_5);
		}

		if (config & PIN_PWMIII_CFG_PWM9_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_PWM9_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM9 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 1, func_PWM9_1);
		} else if (config & PIN_PWMIII_CFG_PWM9_2) {

			confl_detect += gpio_conflict_detect(CGPIO_21, 1, func_PWM9_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM9 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_21, 1, func_PWM9_2);
		} else if (config & PIN_PWMIII_CFG_PWM9_4) {

			confl_detect += gpio_conflict_detect(DGPIO_9, 1, func_PWM9_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM9 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_9, 1, func_PWM9_4);
		} else if (config & PIN_PWMIII_CFG_PWM9_5) {

			confl_detect += gpio_conflict_detect(LGPIO_9, 1, func_PWM9_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM9 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_9, 1, func_PWM9_5);
		}

		if (config & PIN_PWMIII_CFG_PWM10_1) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_PWM10_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_10, 1, func_PWM10_1);
		} else if (config & PIN_PWMIII_CFG_PWM10_2) {

			confl_detect += gpio_conflict_detect(CGPIO_22, 1, func_PWM10_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_22, 1, func_PWM10_2);
		} else if (config & PIN_PWMIII_CFG_PWM10_4) {

			confl_detect += gpio_conflict_detect(DGPIO_10, 1, func_PWM10_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_10, 1, func_PWM10_4);
		} else if (config & PIN_PWMIII_CFG_PWM10_5) {

			confl_detect += gpio_conflict_detect(LGPIO_10, 1, func_PWM10_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_10, 1, func_PWM10_5);
		}

		if (config & PIN_PWMIII_CFG_PWM11_1) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 1, func_PWM11_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_11, 1, func_PWM11_1);
		} else if (config & PIN_PWMIII_CFG_PWM11_2) {

			confl_detect += gpio_conflict_detect(CGPIO_23, 1, func_PWM11_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_23, 1, func_PWM11_2);
		} else if (config & PIN_PWMIII_CFG_PWM11_4) {

			confl_detect += gpio_conflict_detect(DGPIO_7, 1, func_PWM11_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_7, 1, func_PWM11_4);
		} else if (config & PIN_PWMIII_CFG_PWM11_5) {

			confl_detect += gpio_conflict_detect(LGPIO_11, 1, func_PWM11_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_5;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_11, 1, func_PWM11_5);
		}
	}

	return E_OK;
}

static int pinmux_config_ccnt(uint32_t config)
{
	if (config == PIN_CCNT_CFG_NONE) {
	} else {
		if (config & PIN_CCNT_CFG_CCNT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_CCNT);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 1, func_CCNT);
		} else if (config & PIN_CCNT_CFG_CCNT_2) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_CCNT);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT = MUX_2;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 1, func_CCNT);
		}

		if (config & PIN_CCNT_CFG_CCNT2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_14, 1, func_CCNT2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_14, 1, func_CCNT2);
		} else if (config & PIN_CCNT_CFG_CCNT2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_CCNT2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 1, func_CCNT2);
		}

		if (config & PIN_CCNT_CFG_CCNT3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 1, func_CCNT3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 1, func_CCNT3);
		} else if (config & PIN_CCNT_CFG_CCNT3_2) {

			confl_detect += gpio_conflict_detect(PGPIO_20, 1, func_CCNT3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PICNT3 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_20, 1, func_CCNT3);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor(uint32_t config)
{
	if (config == PIN_SENSOR_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR_CFG_12BITS) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 12, func_SENSOR);
			confl_detect += gpio_conflict_detect(SGPIO_1, 3, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_0, 12, func_SENSOR);
			gpio_func_keep(SGPIO_1, 3, func_SENSOR);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor2(uint32_t config)
{
	if (config == PIN_SENSOR2_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR2_CFG_12BITS) {

			confl_detect += gpio_conflict_detect(HSIGPIO_12, 12, func_SENSOR2);
			confl_detect += gpio_conflict_detect(SGPIO_7, 3, func_SENSOR2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR2 = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_12, 12, func_SENSOR2);
			gpio_func_keep(SGPIO_7, 3, func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_A) {
			top_reg8.bit.SENSOR2 = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_16, 8, func_SENSOR2);
			gpio_func_keep(SGPIO_7, 1, func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_B) {

			confl_detect += gpio_conflict_detect(HSIGPIO_8, 8, func_SENSOR2);
			confl_detect += gpio_conflict_detect(SGPIO_1, 1, func_SENSOR2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR2 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_8, 8, func_SENSOR2);
			gpio_func_keep(SGPIO_1, 1, func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_AB) {

			confl_detect += gpio_conflict_detect(HSIGPIO_8, 16, func_SENSOR2);
			confl_detect += gpio_conflict_detect(SGPIO_7, 1, func_SENSOR2);
			confl_detect += gpio_conflict_detect(SGPIO_1, 1, func_SENSOR2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR2 = MUX_4;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_8, 16, func_SENSOR2);
			gpio_func_keep(SGPIO_7, 1, func_SENSOR2);
			gpio_func_keep(SGPIO_1, 1, func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR16BITS) {

			confl_detect += gpio_conflict_detect(HSIGPIO_8, 16, func_SENSOR2);
			confl_detect += gpio_conflict_detect(SGPIO_7, 1, func_SENSOR2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR2 = MUX_5;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_8, 16, func_SENSOR2);
			gpio_func_keep(SGPIO_7, 1, func_SENSOR2);
		}

		if (config & PIN_SENSOR2_CFG_CCIR_VSHS) {

			confl_detect += gpio_conflict_detect(SGPIO_8, 2, func_SENSOR2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SN2_CCIR_VSHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_8, 2, func_SENSOR2);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor3(uint32_t config)
{
	if (config == PIN_SENSOR3_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR3_CFG_12BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 13, func_SENSOR3);
			confl_detect += gpio_conflict_detect(DGPIO_11, 2, func_SENSOR3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 13, func_SENSOR3);
			gpio_func_keep(DGPIO_11, 2, func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR8BITS_A) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 9, func_SENSOR3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR3 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 9, func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR16BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 13, func_SENSOR3);
			confl_detect += gpio_conflict_detect(DGPIO_11, 4, func_SENSOR3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SENSOR3 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 13, func_SENSOR3);
			gpio_func_keep(DGPIO_11, 4, func_SENSOR3);
		}

		if (config & PIN_SENSOR3_CFG_CCIR_VSHS) {

			confl_detect += gpio_conflict_detect(DGPIO_11, 2, func_SENSOR3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.SN3_CCIR_VSHS = MUX_1;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_11, 2, func_SENSOR3);
		}
	}

	return E_OK;
}

static int pinmux_config_sensormisc(uint32_t config)
{
	if (config == PIN_SENSORMISC_CFG_NONE) {
	} else {
		/* MCLK */
		if (config & PIN_SENSORMISC_CFG_SN_MCLK_1) {

			confl_detect += gpio_conflict_detect(SGPIO_0, 1, func_SN1_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_0, 1, func_SN1_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN2_MCLK_1) {

			confl_detect += gpio_conflict_detect(SGPIO_1, 1, func_SN2_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN2_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_1, 1, func_SN2_MCLK);
		} else if (config & PIN_SENSORMISC_CFG_SN2_MCLK_2) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_SN2_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN2_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 1, func_SN2_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN3_MCLK_1) {

			confl_detect += gpio_conflict_detect(SGPIO_6, 1, func_SN3_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN3_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_6, 1, func_SN3_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN4_MCLK_1) {

			confl_detect += gpio_conflict_detect(SGPIO_7, 1, func_SN4_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN4_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_7, 1, func_SN4_MCLK);
		} else if (config & PIN_SENSORMISC_CFG_SN4_MCLK_2) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_SN4_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN4_MCLK = MUX_2;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 1, func_SN4_MCLK);
		}

		/* XVSXHS */
		if (config & PIN_SENSORMISC_CFG_SN_XVSXHS_1) {

			confl_detect += gpio_conflict_detect(SGPIO_2, 2, func_SN1_XVSXHS);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_2, 2, func_SN1_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN2_XVSXHS_1) {

			confl_detect += gpio_conflict_detect(SGPIO_8, 2, func_SN2_XVSXHS);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN2_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_8, 2, func_SN2_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN3_XVSXHS_1) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_SN3_XVSXHS);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN3_XVSXHS = MUX_1;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 2, func_SN3_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN4_XVSXHS_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 2, func_SN4_XVSXHS);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.SN4_XVSXHS = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 2, func_SN4_XVSXHS);
		}

		/* FLASH */
		if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1) {

			confl_detect += gpio_conflict_detect(SGPIO_14, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.FLASH_TRIG_IN = MUX_1;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_14, 1, func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2) {

			confl_detect += gpio_conflict_detect(SGPIO_6, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.FLASH_TRIG_IN = MUX_2;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_6, 1, func_SENSORMISC);
		}

		if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1) {

			confl_detect += gpio_conflict_detect(SGPIO_15, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.FLASH_TRIG_OUT = MUX_1;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_15, 1, func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2) {

			confl_detect += gpio_conflict_detect(SGPIO_7, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.FLASH_TRIG_OUT = MUX_2;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_7, 1, func_SENSORMISC);
		}

		/* SHUTTER */
		if (config & PIN_SENSORMISC_CFG_ME_SHUT_IN_1) {

			confl_detect += gpio_conflict_detect(SGPIO_13, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.ME_SHUT_IN = MUX_1;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_13, 1, func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_ME_SHUT_IN_2) {

			confl_detect += gpio_conflict_detect(SGPIO_8, 1, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.ME_SHUT_IN = MUX_2;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_8, 1, func_SENSORMISC);
		}

		if (config & PIN_SENSORMISC_CFG_ME_SHUT_OUT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 2, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.ME_SHUT_OUT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 2, func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_ME_SHUT_OUT_2) {

			confl_detect += gpio_conflict_detect(DGPIO_8, 2, func_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg8.bit.ME_SHUT_OUT = MUX_2;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_8, 2, func_SENSORMISC);
		}
	}

	return E_OK;
}

static int pinmux_config_sensorsync(uint32_t config)
{
	if (config == PIN_SENSORSYNC_CFG_NONE) {
	} else {
		/* MCLK source sync */
		if (config & PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN) {
			top_reg10.bit.SN1_MCLK_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN2) {
			top_reg10.bit.SN1_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN3) {
			top_reg10.bit.SN1_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN_MCLKSRC_SN4) {
			top_reg10.bit.SN1_MCLK_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN) {
			top_reg10.bit.SN2_MCLK_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN2) {
			top_reg10.bit.SN2_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN3) {
			top_reg10.bit.SN2_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN4) {
			top_reg10.bit.SN2_MCLK_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN) {
			top_reg10.bit.SN3_MCLK_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2) {
			top_reg10.bit.SN3_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN3) {
			top_reg10.bit.SN3_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN4) {
			top_reg10.bit.SN3_MCLK_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN) {
			top_reg10.bit.SN4_MCLK_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2) {
			top_reg10.bit.SN4_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3) {
			top_reg10.bit.SN4_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN4) {
			top_reg10.bit.SN4_MCLK_SRC = MUX_3;
		}

		/* XVSXHS source sync */
		if (config & PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN) {
			top_reg10.bit.SN1_XVSHS_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN2) {
			top_reg10.bit.SN1_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN3) {
			top_reg10.bit.SN1_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN_XVSXHSSRC_SN4) {
			top_reg10.bit.SN1_XVSHS_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN) {
			top_reg10.bit.SN2_XVSHS_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2) {
			top_reg10.bit.SN2_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN3) {
			top_reg10.bit.SN2_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN4) {
			top_reg10.bit.SN2_XVSHS_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN) {
			top_reg10.bit.SN3_XVSHS_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2) {
			top_reg10.bit.SN3_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN3) {
			top_reg10.bit.SN3_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN4) {
			top_reg10.bit.SN3_XVSHS_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN) {
			top_reg10.bit.SN4_XVSHS_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2) {
			top_reg10.bit.SN4_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3) {
			top_reg10.bit.SN4_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN4) {
			top_reg10.bit.SN4_XVSHS_SRC = MUX_3;
		}
	}

	return E_OK;
}

static int pinmux_config_audio(uint32_t config)
{
	if (config & PIN_AUDIO_CFG_NONE) {
	} else {
		if (config & PIN_AUDIO_CFG_I2S_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 4, func_I2S_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.I2S = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 4, func_I2S_1);
		} else if (config & PIN_AUDIO_CFG_I2S_2) {

			confl_detect += gpio_conflict_detect(CGPIO_4, 4, func_I2S_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.I2S = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_4, 4, func_I2S_2);
		}

		if (config & PIN_AUDIO_CFG_I2S_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_22, 1, func_I2S_1_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.I2S_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_22, 1, func_I2S_1_MCLK);
		} else if (config & PIN_AUDIO_CFG_I2S_MCLK_2) {

			confl_detect += gpio_conflict_detect(DGPIO_7, 1, func_I2S_2_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.I2S_MCLK = MUX_2;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_7, 1, func_I2S_2_MCLK);
		}

		if (config & PIN_AUDIO_CFG_DMIC_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.DMIC = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(PGPIO_7, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA0 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_7, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA1 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_8, 1, func_AUDIO_DMIC);
			}
		} else if (config & PIN_AUDIO_CFG_DMIC_2) {

			confl_detect += gpio_conflict_detect(DGPIO_8, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.DMIC = MUX_2;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_8, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(DGPIO_9, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA0 = MUX_1;
				top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_9, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(DGPIO_10, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA1 = MUX_1;
				top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_10, 1, func_AUDIO_DMIC);
			}
		} else if (config & PIN_AUDIO_CFG_DMIC_3) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.DMIC = MUX_3;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_12, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA0 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_13, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(CGPIO_14, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg11.bit.DMIC_DATA1 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_14, 1, func_AUDIO_DMIC);
			}
		}

		if (config & PIN_AUDIO_CFG_EXT_EAC_MCLK) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_AUDIO_EXT_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg11.bit.EXT_EAC_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_AUDIO_EXT_MCLK);
		}
	}

	return E_OK;
}

static int pinmux_config_uart(uint32_t config)
{
	printk("pinmux_config_uart = 0x%08x\r\n", config);

	if (config == PIN_UART_CFG_NONE) {
	} else {
		if (config & PIN_UART_CFG_UART_1) {

			confl_detect += gpio_conflict_detect(PGPIO_23, 2, func_UART);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART = MUX_1;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_23, 2, func_UART);
		}

		if (config & PIN_UART_CFG_UART2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 2, func_UART2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 2, func_UART2_1);

			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART2_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_19, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART2_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_19, 1, func_UART2_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART2_2) {

			confl_detect += gpio_conflict_detect(CGPIO_16, 2, func_UART2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_16, 2, func_UART2_2);
			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += gpio_conflict_detect(CGPIO_15, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART2_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_15, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART2_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_15, 1, func_UART2_DTROE);
			}
		}

		if (config & PIN_UART_CFG_UART3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_13, 2, func_UART3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_13, 2, func_UART3_1);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_15, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART3_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_15, 2, func_UART3_CTSRTS);
			} else if (config & PIN_UART_CFG_UART3_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_15, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART3_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_15, 1, func_UART3_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_22, 2, func_UART3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_22, 2, func_UART3_2);
			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += gpio_conflict_detect(CGPIO_21, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART3_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_21, 2, func_UART3_CTSRTS);
			} else if (config & PIN_UART_CFG_UART3_DIROE) {

				confl_detect += gpio_conflict_detect(CGPIO_21, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART3_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_21, 1, func_UART3_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART3_4) {

			confl_detect += gpio_conflict_detect(SGPIO_4, 1, func_UART3_4);
			confl_detect += gpio_conflict_detect(SGPIO_6, 1, func_UART3_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART3 = MUX_4;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_4, 1, func_UART3_4);
			gpio_func_keep(SGPIO_6, 1, func_UART3_4);
		} else if (config & PIN_UART_CFG_UART3_5) {

			confl_detect += gpio_conflict_detect(CGPIO_2, 2, func_UART3_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART3 = MUX_5;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_2, 2, func_UART3_5);
		}

		if (config & PIN_UART_CFG_UART4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 2, func_UART4_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 2, func_UART4_1);

			if (config & PIN_UART_CFG_UART4_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_2, 2, func_UART4_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART4_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_2, 2, func_UART4_CTSRTS);
			} else if (config & PIN_UART_CFG_UART4_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_2, 1, func_UART4_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART4_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_2, 1, func_UART4_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART4_2) {

			confl_detect += gpio_conflict_detect(CGPIO_4, 2, func_UART4_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART4 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_4, 2, func_UART4_2);
			if (config & PIN_UART_CFG_UART4_RTSCTS) {

				confl_detect += gpio_conflict_detect(CGPIO_6, 2, func_UART4_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART4_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_6, 2, func_UART4_CTSRTS);
			} else if (config & PIN_UART_CFG_UART4_DIROE) {

				confl_detect += gpio_conflict_detect(CGPIO_6, 1, func_UART4_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART4_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_6, 1, func_UART4_DTROE);
			}
		}

		if (config & PIN_UART_CFG_UART5_1) {

			confl_detect += gpio_conflict_detect(PGPIO_4, 2, func_UART5_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 2, func_UART5_1);
			if (config & PIN_UART_CFG_UART5_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_6, 2, func_UART5_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART5_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_6, 2, func_UART5_CTSRTS);
			} else if (config & PIN_UART_CFG_UART5_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_6, 1, func_UART5_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART5_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_6, 1, func_UART5_DTROE);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_uartII(uint32_t config)
{
	if (config == PIN_UARTII_CFG_NONE) {
	} else {
		if (config & PIN_UARTII_CFG_UART6_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_UART6_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg12.bit.UART6 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 2, func_UART6_1);
			if (config & PIN_UARTII_CFG_UART6_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_10, 2, func_UART6_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART6_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_10, 2, func_UART6_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART6_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_UART6_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg13.bit.UART6_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_10, 1, func_UART6_DTROE);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_CSI(uint32_t config)
{
	if (config == PIN_CSI_CFG_NONE) {
	} else {
		if (config & PIN_CSI_CFG_CSI) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 6, func_CSI);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg14.bit.CSI = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO0, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO1, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO2, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO3, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO4, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO5, PAD_NONE);
			gpio_func_keep(HSIGPIO_0, 6, func_CSI);
		}

		if (config & PIN_CSI_CFG_CSI2) {

			confl_detect += gpio_conflict_detect(HSIGPIO_6, 6, func_CSI2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg14.bit.CSI2 = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO6, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO7, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO8, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO11, PAD_NONE);
			gpio_func_keep(HSIGPIO_6, 6, func_CSI2);
		}

		if (config & PIN_CSI_CFG_CSI3) {

			confl_detect += gpio_conflict_detect(HSIGPIO_12, 6, func_CSI3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg14.bit.CSI3 = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO12, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO13, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO14, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO15, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO16, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO17, PAD_NONE);
			gpio_func_keep(HSIGPIO_12, 6, func_CSI3);
		}

		if (config & PIN_CSI_CFG_CSI4) {

			confl_detect += gpio_conflict_detect(HSIGPIO_18, 6, func_CSI4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg14.bit.CSI4 = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO18, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO19, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO20, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO21, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO22, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO23, PAD_NONE);
			gpio_func_keep(HSIGPIO_18, 6, func_CSI4);
		}
	}

	return E_OK;
}

static int pinmux_config_remote(uint32_t config)
{
	if (config == PIN_REMOTE_CFG_NONE) {
	} else {
		if (config & PIN_REMOTE_CFG_REMOTE_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.REMOTE = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 1, func_Remote);
		} else if (config & PIN_REMOTE_CFG_REMOTE_2) {

			confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.REMOTE = MUX_2;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_15, 1, func_Remote);
		} else if (config & PIN_REMOTE_CFG_REMOTE_3) {

			confl_detect += gpio_conflict_detect(DGPIO_2, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.REMOTE = MUX_3;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_2, 1, func_Remote);
		}

		if (config & PIN_REMOTE_CFG_REMOTE_EXT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.REMOTE_EXT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 1, func_Remote);
		}
	}

	return E_OK;
}

static int pinmux_config_sdp(uint32_t config)
{
	if (config == PIN_SDP_CFG_NONE) {
	} else {
		if (config & PIN_SDP_CFG_SDP_1) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 5, func_SDP_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.SDP = MUX_1;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_12, 5, func_SDP_1);
		} else if (config & PIN_SDP_CFG_SDP_2) {

			confl_detect += gpio_conflict_detect(PGPIO_4, 5, func_SDP_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg15.bit.SDP = MUX_2;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 5, func_SDP_2);
		}
	}

	return E_OK;
}

static int pinmux_config_spi(uint32_t config)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & PIN_SPI_CFG_SPI_1) {

			confl_detect += gpio_conflict_detect(CGPIO_18, 3, func_SPI_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_1;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_18, 3, func_SPI_1);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_21, 1, func_SPI_1);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_21, 1, func_SPI_1);
			}
		} else if (config & PIN_SPI_CFG_SPI_2) {

			confl_detect += gpio_conflict_detect(SGPIO_9, 3, func_SPI_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_2;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_9, 3, func_SPI_2);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(SGPIO_12, 1, func_SPI_2);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(SGPIO_12, 1, func_SPI_2);
			}
		} else if (config & PIN_SPI_CFG_SPI_3) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 3, func_SPI_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_3;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 3, func_SPI_3);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_11, 1, func_SPI_3);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_11, 1, func_SPI_3);
			}
		}

		if (config & PIN_SPI_CFG_SPI2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_13, 3, func_SPI2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_13, 3, func_SPI2_1);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_16, 1, func_SPI2_1);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_16, 1, func_SPI2_1);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 3, func_SPI3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 3, func_SPI3_1);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_20, 1, func_SPI3_1);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_20, 1, func_SPI3_1);
			}
		} else if (config & PIN_SPI_CFG_SPI3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_12, 3, func_SPI3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_12, 3, func_SPI3_2);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_SPI3_2);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_15, 1, func_SPI3_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_RDY_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_SPI3_RDY);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI3_RDY = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 1, func_SPI3_RDY);
		} else if (config & PIN_SPI_CFG_SPI3_RDY_2) {

			confl_detect += gpio_conflict_detect(CGPIO_16, 1, func_SPI3_RDY2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI3_RDY = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_16, 1, func_SPI3_RDY2);
		}

		if (config & PIN_SPI_CFG_SPI4_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 3, func_SPI4_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 3, func_SPI4_1);
			if (config & PIN_SPI_CFG_SPI4_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_3, 1, func_SPI4_1);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI4_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_3, 1, func_SPI4_1);
			}
		} else if (config & PIN_SPI_CFG_SPI4_2) {

			confl_detect += gpio_conflict_detect(CGPIO_4, 3, func_SPI4_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI4 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_4, 3, func_SPI4_2);
			if (config & PIN_SPI_CFG_SPI4_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_7, 1, func_SPI4_2);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI4_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_7, 1, func_SPI4_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI5_1) {

			confl_detect += gpio_conflict_detect(PGPIO_4, 3, func_SPI5_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 3, func_SPI5_1);
			if (config & PIN_SPI_CFG_SPI5_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_7, 1, func_SPI5_1);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI5_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_7, 1, func_SPI5_1);
			}
		} else if (config & PIN_SPI_CFG_SPI5_2) {

			confl_detect += gpio_conflict_detect(DGPIO_11, 3, func_SPI5_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg16.bit.SPI5 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_11, 3, func_SPI5_2);
			if (config & PIN_SPI_CFG_SPI5_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(DGPIO_14, 1, func_SPI5_2);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg16.bit.SPI5_BUS_WIDTH = MUX_1;
				top_reg_dgpio0.bit.DGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(DGPIO_14, 1, func_SPI5_2);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_sif(uint32_t config)
{
	if (config == PIN_SIF_CFG_NONE) {
	} else {
		if (config & PIN_SIF_CFG_SIF0_1) {

			confl_detect += gpio_conflict_detect(SGPIO_4, 3, func_SIF_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF0 = MUX_1;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_4, 3, func_SIF_1);
		}

		if (config & PIN_SIF_CFG_SIF1_1) {

			confl_detect += gpio_conflict_detect(SGPIO_9, 3, func_SIF1_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF1 = MUX_1;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(SGPIO_9, 3, func_SIF1_1);
		}

		if (config & PIN_SIF_CFG_SIF2_1) {

			confl_detect += gpio_conflict_detect(LGPIO_22, 3, func_SIF2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF2 = MUX_1;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_22, 3, func_SIF2_1);
		} else if (config & PIN_SIF_CFG_SIF2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 3, func_SIF2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 3, func_SIF2_2);
		} else if (config & PIN_SIF_CFG_SIF2_3) {

			confl_detect += gpio_conflict_detect(DGPIO_8, 3, func_SIF2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF2 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_8, 3, func_SIF2_3);
		}

		if (config & PIN_SIF_CFG_SIF3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_13, 3, func_SIF3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg17.bit.SIF3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_13, 3, func_SIF3_1);
		}
	}

	return E_OK;
}

static int pinmux_config_misc(uint32_t config)
{
	if (config == PIN_MISC_CFG_NONE) {
	} else {
		if (config & PIN_MISC_CFG_RTC_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_1, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.RTC_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_1, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_SP_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 1, func_MISC);
		} else if (config & PIN_MISC_CFG_SP_CLK_2) {

			confl_detect += gpio_conflict_detect(LGPIO_23, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK = MUX_2;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_23, 1, func_MISC);
		} else if (config & PIN_MISC_CFG_SP_CLK_3) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK = MUX_3;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_3, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_SP2_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 1, func_MISC);
		} else if (config & PIN_MISC_CFG_SP2_CLK_2) {

			confl_detect += gpio_conflict_detect(CGPIO_9, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_9, 1, func_MISC);
		} else if (config & PIN_MISC_CFG_SP2_CLK_3) {

			confl_detect += gpio_conflict_detect(DGPIO_4, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg18.bit.SP_CLK2 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_4, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_ADC) {

			confl_detect += gpio_conflict_detect(AGPIO_0, 3, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			adc_en = MUX_1;
			top_reg_agpio0.bit.AGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_2 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_AGPIO0, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_AGPIO1, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_AGPIO2, PAD_NONE);
			gpio_func_keep(AGPIO_0, 3, func_MISC);
		}

		if (config & PIN_MISC_CFG_MIPI_SEL_DSI) {

			top_reg2.bit.MIPI_SEL = MUX_0;

		} else if (config & PIN_MISC_CFG_MIPI_SEL_CSI_TX) {

			top_reg2.bit.MIPI_SEL = MUX_1;
		}

		if (config & PIN_MISC_CFG_CPU_ICE) {

			confl_detect += gpio_conflict_detect(DGPIO_2, 5, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg1.bit.EJTAG_EN = MUX_1;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_2, 5, func_MISC);
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

static int pinmux_select_primary_lcd(uint32_t config)
{
	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 9, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 9, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR656_16BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 9, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 9, func_LCD);
			gpio_func_keep(LGPIO_12, 8, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
			if (config & PINMUX_LCD_SEL_HVLD_VVLD) {

				confl_detect += gpio_conflict_detect(LGPIO_12, 2, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_HVLD_VVLD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_12, 2, func_LCD);
			}

			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(LGPIO_14, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_14, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601_16BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
			gpio_func_keep(LGPIO_12, 8, func_LCD);
			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(LGPIO_20, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_20, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_3;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
			gpio_func_keep(LGPIO_12, 8, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_8BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_4;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_2, 9, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_5;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_2, 9, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_6;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_RGB_16BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_7;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 11, func_LCD);
			gpio_func_keep(LGPIO_12, 8, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 10, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_8;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(LGPIO_0, 11, func_LCD);
			gpio_func_keep(LGPIO_12, 10, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_MIPI) {
			top_reg2.bit.LCD_TYPE = MUX_9;
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_12, 10, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_13, 4, func_LCD);
			confl_detect += gpio_conflict_detect(DGPIO_0, 2, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_10;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;

			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;

			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(LGPIO_0, 11, func_LCD);
			gpio_func_keep(LGPIO_12, 10, func_LCD);
			gpio_func_keep(PGPIO_13, 4, func_LCD);
			gpio_func_keep(DGPIO_0, 2, func_LCD);
		}

		if (config & PINMUX_LCD_SEL_TE_ENABLE) {

			confl_detect += gpio_conflict_detect(LGPIO_22, 1, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.TE_SEL = MUX_1;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_22, 1, func_LCD);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {

			confl_detect += gpio_conflict_detect(LGPIO_11, 1, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.PLCD_DE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_11, 1, func_LCD);
		}
	} else if (pinmux_type <= PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {  // mi type
		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO) {

			confl_detect += gpio_conflict_detect(LGPIO_7, 4, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_7, 4, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_11, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_11, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {

			confl_detect += gpio_conflict_detect(LGPIO_5, 2, func_LCD);
			confl_detect += gpio_conflict_detect(LGPIO_8, 3, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_5, 2, func_LCD);
			gpio_func_keep(LGPIO_8, 3, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_11, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_11, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_8BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 12, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 12, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_22, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_22, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_9BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 13, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 13, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_22, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_22, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 20, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_2;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 20, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_22, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_22, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_0, 22, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_3;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_0, 22, func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_22, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_22, 1, func_LCD);
			}
		}
	}

	return E_OK;
}
/*
ER pinmux_parsing_i2c(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		//< I2C_1   (S_GPIO[15..14])
		if (config & PIN_I2C_CFG_I2C_1) {
			if (config & PIN_I2C_CFG_I2C_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && SGPIO12 pull up => force SGPIO12 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO4 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && SGPIO13 pull up => force SGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO5 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
				}
			}
		}
		//< I2C2_1  (S_GPIO[11..10])
		//< I2C2_2  (C_GPIO[19..18])
		if (config & (PIN_I2C_CFG_I2C2_1 | PIN_I2C_CFG_I2C2_2)) {
			if (config & PIN_I2C_CFG_I2C2_1) {
				if (pad_get_pull_updown(PAD_PIN_SGPIO10, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO14 pull up => force SGPIO14 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO10, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO10 Fail, force pull none\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO10, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO11, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO15 pull up => force SGPIO15 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO11, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO11 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO11, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_CGPIO18, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO30 pull up => force PGPIO30 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO18, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO18 Fail, force pull none\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO18, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO19, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO31 pull up => force PGPIO31 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO19 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
				}
			}
		}

		//< I2C3_1  (P_GPIO[22..21])
		//< I2C3_2  (C_GPIO[13..12])
		//< I2C3_2  (S_GPIO[13..12])
		if (config & (PIN_I2C_CFG_I2C3_1 | PIN_I2C_CFG_I2C3_2 | PIN_I2C_CFG_I2C3_3)) {
			if (config & PIN_I2C_CFG_I2C3_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO21, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO28 pull up => force SGPIO28 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO21 Fail, force pull none\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO22, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && SGPIO29 pull up => force SGPIO29 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO22 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C3_2) {
				if (pad_get_pull_updown(PAD_PIN_CGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO12 pull up => force PGPIO12 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO12 Fail, force pull none\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO12, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO13, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO13 pull up => force PGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO13 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
				}
			} else {
				if (pad_get_pull_updown(PAD_PIN_SGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO12 pull up => force PGPIO12 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO12 Fail, force pull none\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO13, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO13 pull up => force PGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO13 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
				}
			}
		}

		//< I2C4_1  (P_GPIO[12..11])
		//< I2C4_2  (C_GPIO[7..6])
		//< I2C4_3  (S_GPIO[15..14])
		if (config & (PIN_I2C_CFG_I2C4_1 | PIN_I2C_CFG_I2C4_2 | PIN_I2C_CFG_I2C4_3)) {
			if (config & PIN_I2C_CFG_I2C4_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO11, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 1st FUNC_EN && SGPIO30 pull up => force SGPIO30 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO11 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 1st FUNC_EN && SGPIO31 pull up => force SGPIO31 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO12 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C4_2) {
				if (pad_get_pull_updown(PAD_PIN_CGPIO6, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 2nd FUNC_EN && PGPIO14 pull up => force PGPIO14 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO6 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO7, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 2nd FUNC_EN && PGPIO15 pull up => force PGPIO15 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO7 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
				}
				// PIN_I2C_CFG_I2C4_3
			} else {
				if (pad_get_pull_updown(PAD_PIN_SGPIO14, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 3rd FUNC_EN && DGPIO4 pull up => force DGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO14 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO15, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C4 3rd FUNC_EN && DGPIO5 pull up => force DGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO15 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
				}
			}
		}

		//PIN_I2C_CFG_I2C5_1  (P_GPIO[3..2])
		if (config & (PIN_I2C_CFG_I2C5_1)) {
			if (config & PIN_I2C_CFG_I2C5_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO2, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 1st FUNC_EN && SGPIO16 pull up => force SGPIO16 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO2, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO2 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO2, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO3, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C5 1st FUNC_EN && SGPIO17 pull up => force SGPIO17 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						//pad_set_pull_updown(PAD_PIN_PGPIO3, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO3 Fail, force pull none\r\n");
					//pad_set_pull_updown(PAD_PIN_PGPIO3, PAD_NONE);
				}
			}
		}
	}
	return E_OK;
}
*/

typedef int (*PINMUX_CONFIG_HDL)(uint32_t);
static PINMUX_CONFIG_HDL pinmux_config_hdl[] = {
	pinmux_config_sdio,
	pinmux_config_nand,
	pinmux_config_eth,
	pinmux_config_i2c,
	pinmux_config_pwm,
	pinmux_config_pwmII,
	pinmux_config_pwmIII,
	pinmux_config_ccnt,
	pinmux_config_sensor,
	pinmux_config_sensor2,
	pinmux_config_sensor3,
	pinmux_config_sensormisc,
	pinmux_config_sensorsync,
	pinmux_config_audio,
	pinmux_config_uart,
	pinmux_config_uartII,
	pinmux_config_CSI,
	pinmux_config_remote,
	pinmux_config_sdp,
	pinmux_config_spi,
	pinmux_config_sif,
	pinmux_config_misc,
	pinmux_config_lcd,
	pinmux_config_tv,
	pinmux_select_primary_lcd,
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
	u32 lcd_type = 0x0;
	// int logoboot = logo_determination(&lcd_type);
	static int boot_hint = 1;

	//pr_info("%s, %s\r\n", __func__, DRV_VERSION);

	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
	top_reg8.reg = 0;
	top_reg9.reg = 0;
	top_reg10.reg = 0;
	top_reg11.reg = 0;
	top_reg12.reg = 0;
	top_reg13.reg = 0;
	top_reg14.reg = 0;
	top_reg15.reg = 0;
	top_reg16.reg = 0;
	top_reg17.reg = 0;
	top_reg18.reg = 0;
	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_dgpio0.reg = 0xFFFFFFFF;
	top_reg_lgpio0.reg = 0xFFFFFFFF;
	top_reg_sgpio0.reg = 0xFFFFFFFF;
	top_reg_hsigpio0.reg = 0xFFFFFFFF;
	top_reg_agpio0.reg = 0xFFFFFFFF;

	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_dgpio0.reg = 0x00007F83;
	} else {
		top_reg_dgpio0.reg = 0x00007FFF;
	}

	// if (boot_hint && logoboot) {
	// 	top_reg_lgpio0.reg = TOP_GETREG(info, TOP_REGLGPIO0_OFS);
	// 	top_reg_dgpio0.reg = TOP_GETREG(info, TOP_REGDGPIO0_OFS);
	// }

	//clean dump table
	for (i = 0; i < GPIO_MAX; i++) {
		dump_gpio_func[i] = 0;
	}

	/* Enter critical section */
	loc_cpu(flags);

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
	TOP_SETREG(info, TOP_REG12_OFS, top_reg12.reg);
	TOP_SETREG(info, TOP_REG13_OFS, top_reg13.reg);
	TOP_SETREG(info, TOP_REG14_OFS, top_reg14.reg);
	TOP_SETREG(info, TOP_REG15_OFS, top_reg15.reg);
	TOP_SETREG(info, TOP_REG16_OFS, top_reg16.reg);
	TOP_SETREG(info, TOP_REG17_OFS, top_reg17.reg);
	TOP_SETREG(info, TOP_REG18_OFS, top_reg18.reg);
	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, top_reg_lgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO0_OFS, top_reg_sgpio0.reg);
	TOP_SETREG(info, TOP_REGHSIGPIO0_OFS, top_reg_hsigpio0.reg);
	TOP_SETREG(info, TOP_REGAGPIO0_OFS, top_reg_agpio0.reg);

	/* Leave critical section */
	unl_cpu(flags);

    // if (boot_hint && logoboot) {
    //     pr_info("logoboot enable, lcd_type = %08x\r\n",lcd_type);
	// 	pinmux_set_config(PINMUX_FUNC_ID_LCD, lcd_type);
	// 	boot_hint = 0;
	// }

	return E_OK;
}

void gpio_func_show(void)
{
	int i = 0;

	int gpio_count = 0;
	int func_num;
	//print C_GPIO
	for (i = CGPIO_0; i < C_GPIO_MAX; i++) {
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
	//S_GPIO
	gpio_count = 0;
	for (i = SGPIO_0; i < S_GPIO_MAX; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mS_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mS_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}


		} else {
			if (gpio_count < 10) {
				printk("S_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("S_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}
	//L_GPIO
	gpio_count = 0;
	for (i = LGPIO_0; i < L_GPIO_MAX; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mL_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mL_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}

		} else {
			if (gpio_count < 10) {
				printk("L_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("L_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}
	//A_GPIO
	gpio_count = 0;
	for (i = AGPIO_0; i < A_GPIO_MAX; i++) {
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
	//P_GPIO
	gpio_count = 0;
	for (i = PGPIO_0; i < P_GPIO_MAX; i++) {
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
	//D_GPIO
	gpio_count = 0;
	for (i = DGPIO_0; i < D_GPIO_MAX; i++) {
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
}
//EXPORT_SYMBOL(gpio_func_show);
