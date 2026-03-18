/*
    Pinmux module driver.

    This file is the driver of Piumux module.

    @file       ns02301_pinmux_host.c
    @ingroup
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#include "ns02301_pinmux.h"
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

/* PR TOP */
union PR_TOP_REG0         pr_top_reg0;
union PR_TOP_REG1         pr_top_reg1;
union PR_TOP_REG3         pr_top_reg3;
union PR_TOP_REG4         pr_top_reg4;
union PR_TOP_REG5         pr_top_reg5;
union PR_TOP_REG6         pr_top_reg6;

/* GPIO */
union TOP_REGCGPIO0    top_reg_cgpio0;
union TOP_REGPGPIO0    top_reg_pgpio0;
union TOP_REGPGPIO1    top_reg_pgpio1;
union TOP_REGDGPIO0    top_reg_dgpio0;
union TOP_REGHSIGPIO0  top_reg_hsigpio0;
/* PR GPIO */
union PR_TOP_REGSGPIO0 pr_top_reg_sgpio0;
union PR_TOP_REGSGPIO1 pr_top_reg_sgpio1;
union PR_TOP_REGPGPIO0 pr_top_reg_pgpio0;
union PR_TOP_REGAGPIO0 pr_top_reg_agpio0;

uint32_t g_i2c_config;
uint32_t g_pr_i2c_config;
uint32_t g_senmisc_config;
uint32_t g_pr_senmisc_config;
int confl_detect;

// Which function(main_part) occupys pr_gpios now.
static int pr_dump_gpio_func[PR_GPIO_total] = {0};

static int dump_gpio_func[GPIO_total] = {0};
static char *dump_func[FUNC_total] = {"FSPI", "SDIO1", "SDIO2", "SDIO3", "EJTAG", "EXTROM", "ETH", "ETH2",
										"I2C_1", "I2C_2", "I2C_3", "I2C_4", "I2C_5", "I2C2_1", "I2C2_2", "I2C2_3", "I2C2_4",
										"PWM_1", "PWM_2", "PWM_3", "PWM_4", "PWM_5",
										"PWM1_1", "PWM1_2", "PWM1_3", "PWM1_4", "PWM1_5",
										"PWM2_1", "PWM2_2", "PWM2_3", "PWM2_4", "PWM2_5",
										"PWM3_1", "PWM3_2", "PWM3_3", "PWM3_4", "PWM3_5",
										"PWM4_1", "PWM4_2", "PWM4_3", "PWM4_4", "PWM4_5",
										"PWM5_1", "PWM5_2", "PWM5_3", "PWM5_4", "PWM5_5",
										"PWM6_1", "PWM6_2", "PWM6_3", "PWM6_4", "PWM6_5",
										"PWM7_1", "PWM7_2", "PWM7_3", "PWM7_4", "PWM7_5",
										"PWM8_1", "PWM8_2", "PWM8_3", "PWM8_4",
										"PWM9_1", "PWM9_2", "PWM9_3", "PWM9_4",
										"PWM10_1", "PWM10_2", "PWM10_3", "PWM10_4",
										"PWM11_1", "PWM11_2", "PWM11_3", "PWM11_4",
										"CCNT_1", "CCNT_2", "CCNT_3", "CCNT2_1", "CCNT2_2", "CCNT2_3", "CCNT2_4", "CCNT3_1", "CCNT3_2",
										"SENSOR", "SENSOR2", "SENSORMISC", "SN1_VSHS", "SN2_VSHS", "SN1_CCIR_VSHS", "SN1_XVSXHS", "SN2_XVSXHS",
										"SN1_MCLK", "SN2_MCLK", "SN3_MCLK",
										"I2S_1", "I2S_1_MCLK", "I2S_2", "I2S_2_MCLK", "I2S_3", "I2S_3_MCLK", "AUDIO_DMIC", "AUDIO_EXT_MCLK",
										"UART", "UART2_1", "UART2_2", "UART2_3", "UART2_4", "UART3_1", "UART3_2", "UART3_3", "UART3_4", "UART3_5", "UART3_6", 
										"UART2_CTS_RTS", "UART2_DTROE", "UART3_CTS_RTS", "UART3_DTROE",
										"Remote",
										"SDP_1", "SDP_2", "SDP_3",
										"SPI_1", "SPI_2", "SPI2_1", "SPI2_2", "SPI2_3", "SPI2_4",
										"SPI3_1", "SPI3_2", "SPI3_3", "SPI3_4", "SPI3_RDY", "SPI3_RDY2", "SPI3_RDY3", "SPI3_RDY4",
										"SIF_1",
										"SIF1_1",
										"SIF2_1", "SIF2_2", "SIF2_3", "SIF2_4",
										"SIF3_1", "SIF3_2", "MISC", "LCD", "LCD2",
										"PR_I2C_1", "PR_I2C_2", "PR_UART_1", "PR_UART_2", "PR_UART2_1", "PR_UART2_2", 
										"PR_SENSOR", "PR_SENSORMISC", "PR_SPI_WG_1", "PR_SPI_WG_2", 
										"PR_PWM_1", "PR_PWM_2", "PR_PWM1_1", "PR_PWM1_2", "PR_AUDIO_DMIC",
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

#define TOP_SN_MCLK_REG_BASE 0xF001000C
#define TOP_FLASH_TRIGA_REG_BASE 0xF0010004
#define TOP_ME_SHUT_REG_BASE 0xF0010028
#define TOP_PR_SN_MCLK_REG_BASE 0xF0110008
#define TOP_PR_CFG_SRAM_REG_BASE 0xF0110008

//sensermisc_id : {0~10} = {SN_MCLK, SN2_MCLK, SN3_MCLK, SN_XVSXHS, SN2_XVSXHS, FLASH_TRIGA, ME_SHUT_IN, ME_SHUT_OUT, PR_SN_MCLK, CFG_SRAM, CFG_SRAM_SIO}
int top_senmisc_en_bit_get(int senmisc_id, unsigned long *p_addr,  unsigned int *start_bit, unsigned int *end_bit,  unsigned int *p_val)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret;

	// Update current pinmux enum, but no setting
	pinmux_config[0].pin_function = PIN_FUNC_SENSORMISC;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		pr_err("get pinmux config failed\n");


	printk("%s g_senmisc_config is 0x%x\n", __FUNCTION__, g_senmisc_config);

	ret = 1; // for default cnt is 1

	if (senmisc_id == 0) {
		printk("[enable] SN_MCLK\n");
		*p_addr = TOP_SN_MCLK_REG_BASE;
		*start_bit = 10;
		*end_bit = 11;

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN_MCLK_1:
				*p_val = 1;
				break;
			case  PIN_SENSORMISC_CFG_SN_MCLK_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting SN_MCLK\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 1) {
		printk("[enable] SN2_MCLK\n");
		*p_addr = TOP_SN_MCLK_REG_BASE;
		*start_bit = 14;
		*end_bit = 15;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN2_MCLK_1:
				*p_val = 1;
				break;
			default:
				pr_err("dtsi not setting SN2_MCLK\n");
				ret = -1;
				break;
		}

	} else if (senmisc_id == 2) {
		printk("[enable] SN3_MCLK\n");
		*p_addr = TOP_SN_MCLK_REG_BASE;
		*start_bit = 18;
		*end_bit = 18;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN3_MCLK_1:
				*p_val = 1;
				break;
			default:
				pr_err("dtsi not setting SN3_MCLK\n");
				ret = -1;
				break;
		}

	} else if (senmisc_id == 3) {
		printk("[enable] SN_XVSXHS\n");
		*p_addr = TOP_SN_MCLK_REG_BASE;
		*start_bit = 27;
		*end_bit = 28;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN_XVSXHS_1:
				*p_val = 1;
				break;
			case  PIN_SENSORMISC_CFG_SN_XVSXHS_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting SN_XVSXHS\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 4) {
		printk("[enable] SN2_XVSXHS\n");
		*p_addr = TOP_SN_MCLK_REG_BASE;
		*start_bit = 29;
		*end_bit = 30;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN2_XVSXHS_1:
				*p_val = 1;
				break;
			default:
				pr_err("dtsi not setting SN2_XVSXHS\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 5) {
		printk("[enable] FLASH_TRIGA\n");
		*p_addr = TOP_FLASH_TRIGA_REG_BASE;
		*start_bit = 4;
		*end_bit = 5;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1:
				*p_val = 1;
				break;
			case  PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting FLASH_TRIGA\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 6) {
		printk("[enable] ME_SHUT_IN\n");
		*p_addr = TOP_ME_SHUT_REG_BASE;
		*start_bit = 14;
		*end_bit = 15;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_IN_1:
				*p_val = 1;
				break;
			case  PIN_SENSORMISC_CFG_ME_SHUT_IN_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting ME_SHUT_IN\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 7) {
		printk("[enable] ME_SHUT_OUT\n");
		*p_addr = TOP_ME_SHUT_REG_BASE;
		*start_bit = 16;
		*end_bit = 17;
		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_1:
				*p_val = 1;
				break;
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting ME_SHUT_OUT\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 8) {
		printk("[enable] PR_SN_MCLK\n");
		*p_addr = TOP_PR_SN_MCLK_REG_BASE;
		*start_bit = 4;
		*end_bit = 7;
		switch(g_pr_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_1:
				*p_val = 1;
				break;
			default:
				pr_err("dtsi not setting PR_SN_MCLK\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 9) {
		printk("[enable] PR_CFG_SRAM\n");
		*p_addr = TOP_PR_CFG_SRAM_REG_BASE;
		*start_bit = 16;
		*end_bit = 17;
		switch(g_pr_senmisc_config) {
			case  PIN_PR_SENSORMISC_CFG_SRAM_1:
				*p_val = 1;
				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting PR_CFG_SRAM\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 10) {
		printk("[enable] PR_CFG_SRAM_SIO\n");
		*p_addr = TOP_PR_CFG_SRAM_REG_BASE;
		*start_bit = 18;
		*end_bit = 19;
		switch(g_pr_senmisc_config) {
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT:
				*p_val = 1;
				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT:
				*p_val = 2;
				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT:
				*p_val = 3;
				break;
			default:
				pr_err("dtsi not setting PR_CFG_SRAM_SIO\n");
				ret = -1;
				break;
		}
	} else {
		printk("error senmisc id");
		ret = -1;

	}

	return ret;
}
EXPORT_SYMBOL(top_senmisc_en_bit_get);

#define TOP_PGPIO_BASE 0xF00100A8
#define TOP_C_GPIO_BASE 0xF00100A0
#define TOP_HSI_GPIO_BASE 0xF00100D8
#define TOP_PR_SGPIO1_BASE 0xF0110028
#define TOP_PR_SGPIO2_BASE 0xF011002C
#define TOP_PR_PGPIO_BASE 0xF0110030

//sensermisc_id : {0~10} = {SN_MCLK, SN2_MCLK, SN3_MCLK, SN_XVSXHS, SN2_XVSXHS, FLASH_TRIGA, ME_SHUT_IN, ME_SHUT_OUT, PR_SN_MCLK, CFG_SRAM, CFG_SRAM_SIO}
int top_senmisc_func_bit_get(int senmisc_id, unsigned long *p_addr,  unsigned int *start_bit, unsigned int *end_bit,  unsigned int *p_val)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret;

	// Update current pinmux enum, but no setting
	pinmux_config[0].pin_function = PIN_FUNC_SENSORMISC;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		pr_err("get pinmux config failed\n");


	printk("%s g_senmisc_config is 0x%x\n", __FUNCTION__, g_senmisc_config);
	printk("%s g_pr_senmisc_config is 0x%x\n", __FUNCTION__, g_pr_senmisc_config);

	ret = 1; // for default cnt is 1

	if (senmisc_id == 0) {
		printk("[enable] SN_MCLK\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN_MCLK_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 0;
				*end_bit = 1;
				*p_val = 0x2;
				break;
			case  PIN_SENSORMISC_CFG_SN_MCLK_2:
				*p_addr = TOP_HSI_GPIO_BASE;
				*start_bit = 11;
				*end_bit = 11;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting SN_MCLK\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 1) {
		printk("[enable] SN2_MCLK\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN2_MCLK_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 2;
				*end_bit = 3;
				*p_val = 0x2;
				break;
			default:
				pr_err("dtsi not setting SN2_MCLK\n");
				ret = -1;
				break;
		}

	} else if (senmisc_id == 2) {
		printk("[enable] SN3_MCLK\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN3_MCLK_1:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 12;
				*end_bit = 12;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting SN3_MCLK\n");
				ret = -1;
				break;
		}

	} else if (senmisc_id == 3) {
		printk("[enable] SN_XVSXHS\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN_XVSXHS_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 4;
				*end_bit = 7;
				*p_val = 0xA;
				break;
			case  PIN_SENSORMISC_CFG_SN_XVSXHS_2:
				*p_addr = TOP_HSI_GPIO_BASE;
				*start_bit = 11;
				*end_bit = 11;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting SN_XVSXHS\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 4) {
		printk("[enable] SN2_XVSXHS\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_SN2_XVSXHS_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 8;
				*end_bit = 11;
				*p_val = 0xA;
				break;
			default:
				pr_err("dtsi not setting SN2_XVSXHS\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 5) {
		printk("[enable] FLASH_TRIGA\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1:
				*p_addr = TOP_C_GPIO_BASE;
				*start_bit = 21;
				*end_bit = 22;
				*p_val = 0;
				break;
			case  PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 11;
				*end_bit = 12;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting FLASH_TRIGA\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 6) {
		printk("[enable] ME_SHUT_IN\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_IN_1:
				*p_addr = TOP_C_GPIO_BASE;
				*start_bit = 20;
				*end_bit = 20;
				*p_val = 0;
				break;
			case  PIN_SENSORMISC_CFG_ME_SHUT_IN_2:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 4;
				*end_bit = 4;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting ME_SHUT_IN\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 7) {
		printk("[enable] ME_SHUT_OUT\n");

		switch(g_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_1:
				*p_addr = TOP_C_GPIO_BASE;
				*start_bit = 23;
				*end_bit = 24;
				*p_val = 0;
				break;
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_2:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 5;
				*end_bit = 6;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting ME_SHUT_OUT\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 8) {
		printk("[enable] PR_SN_MCLK\n");

		switch(g_pr_senmisc_config) {
			case  PIN_SENSORMISC_CFG_ME_SHUT_OUT_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 0;
				*end_bit = 1;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting PR_SN_MCLK\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 9) {
		printk("[enable] PR_CFG_SRAM\n");

		switch(g_pr_senmisc_config) {
			case  PIN_PR_SENSORMISC_CFG_SRAM_1:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 16;
				*end_bit = 19;
				*p_val = 0;
				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_2:
				p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 0;
				*end_bit = 3;
				*p_val = 0;
				break;
			default:
				pr_err("dtsi not setting PR_CFG_SRAM\n");
				ret = -1;
				break;
		}
	} else if (senmisc_id == 10) {
		printk("[enable] PR_CFG_SRAM_SIO\n");

		switch(g_pr_senmisc_config) {
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 20;
				*end_bit = 21;
				*p_val = 0;

				*(p_addr + 1) = TOP_PR_PGPIO_BASE;
				*(start_bit + 1) = 4;
				*(end_bit + 1) = 5;
				*(p_val + 1) = 0;

				ret = 2;

				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 20;
				*end_bit = 23;
				*p_val = 0;

				*(p_addr + 1) = TOP_PR_PGPIO_BASE;
				*(start_bit + 1) = 4;
				*(end_bit + 1) = 7;
				*(p_val + 1) = 0;

				ret = 2;
				break;
			case  PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 20;
				*end_bit = 27;
				*p_val = 0;

				*(p_addr + 1) = TOP_PR_PGPIO_BASE;
				*(start_bit + 1) = 4;
				*(end_bit + 1) = 11;
				*(p_val + 1) = 0;

				ret = 2;
				break;
			default:
				pr_err("dtsi not setting PR_CFG_SRAM_SIO\n");
				ret = -1;
				break;
		}
	} else {
		printk("error senmisc id");
		ret = -1;

	}

	return ret;
}
EXPORT_SYMBOL(top_senmisc_func_bit_get);

#define TOP_I2C_REG_BASE 0xF0010010
#define TOP_I2C2_REG_BASE 0xF0010010
#define TOP_PRI2C_REG_BASE 0xF0110000
int top_i2c_en_bit_get(int i2c_id, unsigned long *p_addr,  unsigned int *start_bit, unsigned int *end_bit,  unsigned int *p_val)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret;

	// Update current pinmux enum, but no setting
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		pr_err("get pinmux config failed\n");


	printk("%s g_i2c_config is 0x%x\n", __FUNCTION__, g_i2c_config);
	printk("%s g_pr_i2c_config is 0x%x\n", __FUNCTION__, g_pr_i2c_config);

	ret = 1; // for default cnt is 1
	//if (strcmp(name, "i2c")) {
	if (i2c_id == 0) {
		printk("[enable] i2c\n");
		*p_addr = TOP_I2C_REG_BASE;
		*start_bit = 0;
		*end_bit = 2;

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C_1:
				*p_val = 1;
				break;
			case  PIN_I2C_CFG_I2C_2:
				*p_val = 2;
				break;
			case  PIN_I2C_CFG_I2C_3:
				*p_val = 3;
				break;
			case  PIN_I2C_CFG_I2C_4:
				*p_val = 4;
				break;
			case  PIN_I2C_CFG_I2C_5:
				*p_val = 5;
				break;
			default:
				pr_err("dtsi not setting i2c\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 1) {
		printk("[enable] i2c2\n");
		*p_addr = TOP_I2C2_REG_BASE;
		*start_bit = 3;
		*end_bit = 5;

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C2_1:
				*p_val = 1;
				break;
			case  PIN_I2C_CFG_I2C2_2:
				*p_val = 2;
				break;
			case  PIN_I2C_CFG_I2C2_3:
				*p_val = 3;
				break;
			case  PIN_I2C_CFG_I2C2_4:
				*p_val = 4;
				break;
			default:
				pr_err("dtsi not setting i2c2\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 2) {
		printk("[enable] pri2c\n");
		*p_addr = TOP_PRI2C_REG_BASE;
		*start_bit = 0;
		*end_bit = 3;

		switch(g_pr_i2c_config) {
			case  PIN_PR_I2C_CFG_I2C_1:
				*p_val = 1;
				break;
			case  PIN_PR_I2C_CFG_I2C_2:
				*p_val = 2;
				break;
			default:
				pr_err("dtsi not setting pri2c\n");
				ret = -1;
				break;
		}

	} else {
		printk("error i2c id");
		ret = -1;
	}

	return ret;
}
EXPORT_SYMBOL(top_i2c_en_bit_get);

int top_i2c_func_bit_get(int i2c_id, unsigned long *p_addr,  unsigned int *start_bit, unsigned int *end_bit,  unsigned int *p_val)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret;

	// Update current pinmux enum, but no setting
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		pr_err("get pinmux config failed\n");

	printk("%s g_i2c_config is 0x%x\n", __FUNCTION__, g_i2c_config);
        printk("%s g_pr_i2c_config is 0x%x\n", __FUNCTION__, g_pr_i2c_config);

	ret = 1; // for default cnt is 1
	if (i2c_id == 0) {
		printk("[FUNC] i2c\n");

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C_1:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 21;
				*end_bit = 22;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_2:
				*p_addr = TOP_C_GPIO_BASE;
				*start_bit = 13;
				*end_bit = 14;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_3:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 9;
				*end_bit = 10;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_4:
				*p_addr = TOP_PGPIO_BASE;
				*start_bit = 11;
				*end_bit = 12;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_5:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 30;
				*end_bit = 31;
				*p_val = 0xA;

				*(p_addr + 1) = TOP_PR_SGPIO2_BASE;
				*(start_bit + 1) = 0;
				*(end_bit + 1) = 1;
				*(p_val + 1) = 0xA;

				ret = 2;
				break;

			default:
				pr_err("[FUNC]dtsi not setting i2c\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 1) {
		printk("[FUNC] i2c2\n");

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C2_1:
				*p_addr = TOP_C_GPIO_BASE;
				*start_bit = 19;
				*end_bit = 20;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C2_2:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 12;
				*end_bit = 15;
				*p_val = 0xA;
				break;
			case  PIN_I2C_CFG_I2C2_3:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 8;
				*end_bit = 11;
				*p_val = 0xA;
				break;
			case  PIN_I2C_CFG_I2C2_4:
				*p_addr = TOP_HSI_GPIO_BASE;
				*start_bit = 9;
				*end_bit = 10;
				*p_val = 0;
				break;
			default:
				pr_err("[FUNC]dtsi not setting i2c2\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 2) {
		printk("[FUNC] pri2c\n");

		switch(g_pr_i2c_config) {
			case  PIN_PR_I2C_CFG_I2C_1:
				*p_addr = TOP_PR_SGPIO1_BASE;
				*start_bit = 14;
				*end_bit = 17;
				*p_val = 0;
				break;
			case  PIN_PR_I2C_CFG_I2C_2:
				*p_addr = TOP_PR_PGPIO_BASE;
				*start_bit = 8;
				*end_bit = 11;
				*p_val = 0;
				break;
			default:
				pr_err("[FUNC]dtsi not setting pri2c\n");
				ret = -1;
				break;
		}

	} else {
		printk("[FUNC]error i2c id");
		ret = -1;
	}

	return ret;
}
EXPORT_SYMBOL(top_i2c_func_bit_get);

#define PAD_PGPIO_BASE 0xF0030008
#define PAD_C_GPIO_BASE 0xF0030000
#define PAD_HSI_GPIO_BASE 0xF003001C
#define PAD_PR_SGPIO_BASE 0xF0130008
#define PAD_PR_PGPIO_BASE 0xF0130000

int top_i2c_pad_bit_get(int i2c_id, unsigned long *p_addr,  unsigned int *start_bit, unsigned int *end_bit,  unsigned int *p_val)
{
	PIN_GROUP_CONFIG pinmux_config[1];
	int ret;

	// Update current pinmux enum, but no setting
	pinmux_config[0].pin_function = PIN_FUNC_I2C;
	ret = nvt_pinmux_capture(pinmux_config, 1);
	if (ret)
		pr_err("get pinmux config failed\n");

	printk("%s g_i2c_config is 0x%x\n", __FUNCTION__, g_i2c_config);
	printk("%s g_pr_i2c_config is 0x%x\n", __FUNCTION__, g_pr_i2c_config);

	ret = 1; // for default cnt is 1
	if (i2c_id == 0) {
		printk("[PAD] i2c\n");

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C_1:
				*p_addr = PAD_PGPIO_BASE + 0x8;
				*start_bit = 10;
				*end_bit = 13;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_2:
				*p_addr = PAD_C_GPIO_BASE + 0x0;
				*start_bit = 26;
				*end_bit = 29;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_3:
				*p_addr = PAD_PGPIO_BASE + 0x0;
				*start_bit = 18;
				*end_bit = 21;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_4:
				*p_addr = TOP_PGPIO_BASE + 0x0;
				*start_bit = 24;
				*end_bit = 27;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C_5:
				*p_addr = PAD_PR_SGPIO_BASE + 0x0;
				*start_bit = 30;
				*end_bit = 31;
				*p_val = 0;

				*(p_addr + 1) = PAD_PR_SGPIO_BASE + 0x4;
				*(start_bit + 1) = 0;
				*(end_bit + 1) = 1;
				*(p_val + 1) = 0;

				ret = 2;
				break;

			default:
				pr_err("[PAD]dtsi not setting i2c\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 1) {
		printk("[PAD] i2c2\n");

		switch(g_i2c_config) {
			case  PIN_I2C_CFG_I2C2_1:
				*p_addr = PAD_C_GPIO_BASE + 0x4;
				*start_bit = 6;
				*end_bit = 9;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C2_2:
				*p_addr = PAD_PR_PGPIO_BASE;
				*start_bit = 12;
				*end_bit = 15;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C2_3:
				*p_addr = PAD_PR_SGPIO_BASE;
				*start_bit = 8;
				*end_bit = 11;
				*p_val = 0;
				break;
			case  PIN_I2C_CFG_I2C2_4:
				*p_addr = PAD_HSI_GPIO_BASE;
				*start_bit = 18;
				*end_bit = 21;
				*p_val = 0;
				break;
			default:
				pr_err("[PAD]dtsi not setting i2c2\n");
				ret = -1;
				break;
		}

	} else if (i2c_id == 2) {
		printk("[PAD] pri2c\n");

		switch(g_pr_i2c_config) {
			case  PIN_PR_I2C_CFG_I2C_1:
				*p_addr = PAD_PR_SGPIO_BASE;
				*start_bit = 14;
				*end_bit = 17;
				*p_val = 0;
				break;
			case  PIN_PR_I2C_CFG_I2C_2:
				*p_addr = PAD_PR_PGPIO_BASE;
				*start_bit = 8;
				*end_bit = 11;
				*p_val = 0;
				break;
			default:
				pr_err("[PAD]dtsi not setting pri2c\n");
				ret = -1;
				break;
		}

	} else {
		printk("[PAD]error i2c id");
		ret = -1;
	}

	return ret;
}
EXPORT_SYMBOL(top_i2c_pad_bit_get);

static void gpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char *gpio_name[] = {"C_GPIO", "P_GPIO", "HSI_GPIO", "D_GPIO"};
	char name[10];

	if (start_offset == TOP_REGCGPIO0_OFS) {
		strcpy(name, gpio_name[0]);
	} else if (start_offset == TOP_REGPGPIO0_OFS) {
		strcpy(name, gpio_name[1]);
	} else if (start_offset == TOP_REGHSIGPIO0_OFS) {
		strcpy(name, gpio_name[2]);
	} else if (start_offset == TOP_REGDGPIO0_OFS) {
		strcpy(name, gpio_name[3]);
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
	pr_info("================ Main Part Dump ================\n");
	pr_info("\n[PIN]       [NO]      [STATUS]\n");

	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, HSI_GPIO_NUM, TOP_REGHSIGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
}

/* For pre-roll part */
#define PR_P_GPIO_NUM 14
#define PR_S_GPIO_NUM 17
#define PR_A_GPIO_NUM 4
#define PR_MUX_BIT 2
#define PR_MUX_MASK 3
static void prgpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char *gpio_name[] = {"PR_P_GPIO", "PR_S_GPIO", "PR_A_GPIO"};
	char name[10];

	if (start_offset == PR_TOP_REGPGPIO0_OFS) {
		strcpy(name, gpio_name[0]);
	} else if (start_offset == PR_TOP_REGSGPIO0_OFS) {
		strcpy(name, gpio_name[1]);
	} else if (start_offset == PR_TOP_REGAGPIO0_OFS) {
		strcpy(name, gpio_name[2]);
	}

	if (gpio_number > 0x10) {
		reg_value = PR_TOP_GETREG(info, start_offset);

		for (i = 0; i < 0x10; i++) {
			if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x0) {
				pr_info("%-12s%-4d      PR_FUNCTION\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x1) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x2) {
				pr_info("%-12s%-4d      MAIN_FUNCTION\n", name, i);
			} else {
				pr_info("invalid\n");
			}
		}

		reg_value = PR_TOP_GETREG(info, start_offset + 0x4);

		for (j = 0; j < (gpio_number - 0x10); j++) {
			if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x0) {
				pr_info("%-12s%-4d      PR_FUNCTION\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x1) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x2) {
				pr_info("%-12s%-4d      MAIN_FUNCTION\n", name, i);
			} else {
				pr_info("invalid\n");
			}
			i++;
		}
	} else {
		reg_value = PR_TOP_GETREG(info, start_offset);
		for (i = 0; i < gpio_number; i++) {
			if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x0) {
				pr_info("%-12s%-4d      PR_FUNCTION\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x1) {
				pr_info("%-12s%-4d      GPIO\n", name, i);
			} else if (((reg_value >> (i * PR_MUX_BIT)) & PR_MUX_MASK) == 0x2) {
				pr_info("%-12s%-4d      MAIN_FUNCTION\n", name, i);
			} else {
				pr_info("invalid\n");
			}
		}
	}
}

void pinmux_prgpio_parsing(struct nvt_pinctrl_info *info)
{
        pr_info("================ Pre_Roll Part Dump ================\n");
        pr_info("\n[PIN]       [NO]      [STATUS]\n");
        prgpio_info_show(info, PR_P_GPIO_NUM, PR_TOP_REGPGPIO0_OFS);
        prgpio_info_show(info, PR_S_GPIO_NUM, PR_TOP_REGSGPIO0_OFS);
        prgpio_info_show(info, PR_A_GPIO_NUM, PR_TOP_REGAGPIO0_OFS);

}

static uint32_t disp_pinmux_config[] = {
	PINMUX_DISPMUX_SEL_NONE,            // LCD
	PINMUX_DISPMUX_SEL_NONE,            // LCD2
	PINMUX_TV_HDMI_CFG_NORMAL,          // TV
	PINMUX_TV_HDMI_CFG_NORMAL           // HDMI
};

static uint32_t adc_en = MUX_0;

//static uint32_t lcd2_rgb_6bit_flag = 0;    // workaround flag

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
//	union TOP_REG8 local_top_reg8; //SYSTEM top
	union TOP_REG9 local_top_reg9;
	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;

	/*Pre-roll part*/
	union PR_TOP_REG0 local_pr_top_reg0;
	union PR_TOP_REG1 local_pr_top_reg1;
	union PR_TOP_REG3 local_pr_top_reg3;
	//union PR_TOP_REG4 local_pr_top_reg4;//PR_SYSTEM top
	union PR_TOP_REG5 local_pr_top_reg5;
	union PR_TOP_REG6 local_pr_top_reg6;

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
//	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS); //SYSTEM top
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);

	local_pr_top_reg0.reg = PR_TOP_GETREG(info, PR_TOP_REG0_OFS);
	local_pr_top_reg1.reg = PR_TOP_GETREG(info, PR_TOP_REG1_OFS);
	local_pr_top_reg3.reg = PR_TOP_GETREG(info, PR_TOP_REG3_OFS);
	local_pr_top_reg5.reg = PR_TOP_GETREG(info, PR_TOP_REG5_OFS);
	local_pr_top_reg6.reg = PR_TOP_GETREG(info, PR_TOP_REG6_OFS);

	/* Parsing SDIO */
	value = PIN_SDIO_CFG_NONE;

	if (local_top_reg1.bit.SDIO_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO_1;
	}

	if (local_top_reg1.bit.SDIO2_EN == MUX_1)  {
		value |= PIN_SDIO_CFG_SDIO2_1;
	} else if (local_top_reg1.bit.SDIO2_EN == MUX_2) {
		value |= PIN_SDIO_CFG_SDIO2_2;
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

	if (local_top_reg9.bit.ETH == MUX_1) {
		value |= PIN_ETH_CFG_ETH_RMII_1;
	}

	if (local_top_reg9.bit.ETH_MDIO_MUX == MUX_1) {
		value |= PIN_ETH_CFG_ETH_MDIO_1;
	}

	if (local_top_reg9.bit.ETH_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH_EXTPHYCLK;
	}

   if (local_top_reg9.bit.ETH_LED == MUX_1) {
        value |= PIN_ETH_CFG_ETH_LED_1;
    } else if (local_top_reg9.bit.ETH_LED == MUX_2) {
        value |= PIN_ETH_CFG_ETH_LED_2;
    }

	info->top_pinmux[PIN_FUNC_ETH].config = value;
	info->top_pinmux[PIN_FUNC_ETH].pin_function = PIN_FUNC_ETH;

	/* Parsing I2C */
	value = PIN_I2C_CFG_NONE;

	if (local_top_reg4.bit.I2C == MUX_1) {
		value |= PIN_I2C_CFG_I2C_1;
	} else  if (local_top_reg4.bit.I2C == MUX_2) {
		value |= PIN_I2C_CFG_I2C_2;
	} else  if (local_top_reg4.bit.I2C == MUX_3) {
		value |= PIN_I2C_CFG_I2C_3;
	} else  if (local_top_reg4.bit.I2C == MUX_4) {
		value |= PIN_I2C_CFG_I2C_4;
	} else  if (local_top_reg4.bit.I2C == MUX_5) {
		value |= PIN_I2C_CFG_I2C_5;
	}

	if (local_top_reg4.bit.I2C2 == MUX_1) {
		value |= PIN_I2C_CFG_I2C2_1;
	} else if (local_top_reg4.bit.I2C2 == MUX_2) {
		value |= PIN_I2C_CFG_I2C2_2;
	} else if (local_top_reg4.bit.I2C2 == MUX_3) {
		value |= PIN_I2C_CFG_I2C2_3;
	} else if (local_top_reg4.bit.I2C2 == MUX_4) {
		value |= PIN_I2C_CFG_I2C2_4;
	}


	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;
	g_i2c_config = value;

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

	/* Parsing PWMIII */
	//0x14
	value = PIN_PWMIII_CFG_NONE;
	if (local_top_reg5.bit.PWM8 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM8_1;
	} else if (local_top_reg5.bit.PWM8 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM8_2;
	} else if (local_top_reg5.bit.PWM8 == MUX_3) {
		value |= PIN_PWMIII_CFG_PWM8_3;
	} else if (local_top_reg5.bit.PWM8 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM8_4;
	}

	if (local_top_reg5.bit.PWM9 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM9_1;
	} else if (local_top_reg5.bit.PWM9 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM9_2;
	} else if (local_top_reg5.bit.PWM9 == MUX_3) {
		value |= PIN_PWMIII_CFG_PWM9_3;
	} else if (local_top_reg5.bit.PWM9 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM9_4;
	}

	//0x18
	if (local_top_reg6.bit.PWM10 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM10_1;
	} else if (local_top_reg6.bit.PWM10 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM10_2;
	} else if (local_top_reg6.bit.PWM10 == MUX_3) {
		value |= PIN_PWMIII_CFG_PWM10_3;
	} else if (local_top_reg6.bit.PWM10 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM10_4;
	}

	if (local_top_reg6.bit.PWM11 == MUX_1) {
		value |= PIN_PWMIII_CFG_PWM11_1;
	} else if (local_top_reg6.bit.PWM11 == MUX_2) {
		value |= PIN_PWMIII_CFG_PWM11_2;
	} else if (local_top_reg6.bit.PWM11 == MUX_3) {
		value |= PIN_PWMIII_CFG_PWM11_3;
	} else if (local_top_reg6.bit.PWM11 == MUX_4) {
		value |= PIN_PWMIII_CFG_PWM11_4;
	}

	info->top_pinmux[PIN_FUNC_PWMIII].config = value;
	info->top_pinmux[PIN_FUNC_PWMIII].pin_function = PIN_FUNC_PWMIII;

	/* Parsing CCNT */
	value = PIN_CCNT_CFG_NONE;

	if (local_top_reg10.bit.PICNT == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT_1;
	} else if (local_top_reg10.bit.PICNT == MUX_2) {
		value |= PIN_CCNT_CFG_CCNT_2;
	} else if (local_top_reg10.bit.PICNT == MUX_3) {
		value |= PIN_CCNT_CFG_CCNT_3;
	}

	if (local_top_reg10.bit.PICNT2 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT2_1;
	} else if (local_top_reg10.bit.PICNT2 == MUX_2) {
		value |= PIN_CCNT_CFG_CCNT2_2;
	} else if (local_top_reg10.bit.PICNT2 == MUX_3) {
		value |= PIN_CCNT_CFG_CCNT2_3;
	} else if (local_top_reg10.bit.PICNT2 == MUX_4) {
		value |= PIN_CCNT_CFG_CCNT2_4;
	}

	if (local_top_reg10.bit.PICNT3 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT3_1;
	} else if (local_top_reg10.bit.PICNT3 == MUX_2) {
		value |= PIN_CCNT_CFG_CCNT3_2;
	}

	info->top_pinmux[PIN_FUNC_CCNT].config = value;
	info->top_pinmux[PIN_FUNC_CCNT].pin_function = PIN_FUNC_CCNT;

	/* Parsing SENSOR1 */
	value = PIN_SENSOR_CFG_NONE;

	if (local_top_reg3.bit.SENSOR == MUX_1) {
		value |= PIN_SENSOR_CFG_12BITS;
	} else if (local_top_reg3.bit.SENSOR == MUX_2) {
		value |= PIN_SENSOR_CFG_12BITS_2;
	} else if (local_top_reg3.bit.SENSOR == MUX_3) {
		value |= PIN_SENSOR_CFG_CCIR8BITS;
	} else if (local_top_reg3.bit.SENSOR == MUX_4) {
		value |= PIN_SENSOR_CFG_CCIR16BITS;
	}

	if (local_top_reg3.bit.SN_CCIR_VSHS == MUX_0) {
		value |= PIN_SENSOR_CFG_CCIR_VSHS;
	}

	if (local_top_reg3.bit.SN_VSHS == MUX_1) {
		value |= PIN_SENSOR_CFG_SN_VSHS;
	}

	info->top_pinmux[PIN_FUNC_SENSOR].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR].pin_function = PIN_FUNC_SENSOR;

	/* Parsing SENSOR2 */
	value = PIN_SENSOR2_CFG_NONE;

	if (local_top_reg3.bit.SENSOR2 == MUX_1) {
		value |= PIN_SENSOR2_CFG_12BITS;
	} else if (local_top_reg3.bit.SENSOR2 == MUX_2) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS;
	}

	if (local_top_reg3.bit.SN2_VSHS == MUX_1) {
		value |= PIN_SENSOR2_CFG_SN2_VSHS;
	}

	info->top_pinmux[PIN_FUNC_SENSOR2].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR2].pin_function = PIN_FUNC_SENSOR2;

	/* Parsing SENSORMISC */
	value = PIN_SENSORMISC_CFG_NONE;

	if (local_top_reg3.bit.SN_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_MCLK_1;
	} else if (local_top_reg3.bit.SN_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN_MCLK_2;
	}

	if (local_top_reg3.bit.SN2_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_MCLK_1;
	}
	if (local_top_reg3.bit.SN3_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN3_MCLK_1;
	}

	if (local_top_reg3.bit.SN_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_XVSXHS_1;
	} else if (local_top_reg3.bit.SN_XVSXHS == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN_XVSXHS_2;
	}

	if (local_top_reg3.bit.SN2_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_XVSXHS_1;
	}

	if (local_top_reg1.bit.FLASH_TRIGA == MUX_1) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1;
	} else if (local_top_reg1.bit.FLASH_TRIGA == MUX_2) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2;
	}

	if (local_top_reg10.bit.ME_SHUT_IN == MUX_1) {
		value |= PIN_SENSORMISC_CFG_ME_SHUT_IN_1;
	} else if (local_top_reg10.bit.ME_SHUT_IN == MUX_2) {
		value |= PIN_SENSORMISC_CFG_ME_SHUT_IN_2;
	}

	if (local_top_reg10.bit.ME_SHUT_OUT == MUX_1) {
		value |= PIN_SENSORMISC_CFG_ME_SHUT_OUT_1;
	} else if (local_top_reg10.bit.ME_SHUT_OUT == MUX_2) {
		value |= PIN_SENSORMISC_CFG_ME_SHUT_OUT_2;
	}

	info->top_pinmux[PIN_FUNC_SENSORMISC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORMISC].pin_function = PIN_FUNC_SENSORMISC;

	g_senmisc_config = value;

	/* Parsing SENSORSYNC */
	value = PIN_SENSORSYNC_CFG_NONE;

	if (local_top_reg3.bit.SN2_MCLK_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK2;
	} else if (local_top_reg3.bit.SN2_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK;
	}

	if (local_top_reg3.bit.SN2_XVSHS_SRC == MUX_0) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2;
	} else if (local_top_reg3.bit.SN2_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN1;
	}

	info->top_pinmux[PIN_FUNC_SENSORSYNC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORSYNC].pin_function = PIN_FUNC_SENSORSYNC;

	/* Parsing AUDIO */
	value = PIN_AUDIO_CFG_NONE;

	if (local_top_reg9.bit.I2S == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_1;
	} else if (local_top_reg9.bit.I2S == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_2;
	} else if (local_top_reg9.bit.I2S == MUX_3) {
		value |= PIN_AUDIO_CFG_I2S_3;
	}
	if (local_top_reg9.bit.I2S_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_1;
	} else if (local_top_reg9.bit.I2S_MCLK == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_2;
	} else if (local_top_reg9.bit.I2S_MCLK == MUX_3) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_3;
	}

	if (local_top_reg9.bit.DMIC == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_1;
	} else if (local_top_reg9.bit.DMIC == MUX_2) {
		value |= PIN_AUDIO_CFG_DMIC_2;
	} else if (local_top_reg9.bit.DMIC == MUX_3) {
		value |= PIN_AUDIO_CFG_DMIC_3;
	} else if (local_top_reg9.bit.DMIC == MUX_4) {
		value |= PIN_AUDIO_CFG_DMIC_4;
	} else if (local_top_reg9.bit.DMIC == MUX_5) {
		value |= PIN_AUDIO_CFG_PR_DMIC_1;
	}

	if (local_top_reg9.bit.DMIC_DATA0 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA0;
	}
	if (local_top_reg9.bit.DMIC_DATA1 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA1;
	}

	if (local_top_reg9.bit.EXT_EAC_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_EXT_EAC_MCLK;
	}

	info->top_pinmux[PIN_FUNC_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_AUDIO].pin_function = PIN_FUNC_AUDIO;

	/* Parsing UART */
	value = PIN_UART_CFG_NONE;

	if (local_top_reg4.bit.UART == MUX_1) {
		value |= PIN_UART_CFG_UART_1;
	}

	if (local_top_reg4.bit.UART2 == MUX_1) {
		value |= PIN_UART_CFG_UART2_1;
	} else if (local_top_reg4.bit.UART2 == MUX_2) {
		value |= PIN_UART_CFG_UART2_2;
	} else if (local_top_reg4.bit.UART2 == MUX_3) {
		value |= PIN_UART_CFG_UART2_3;
	} else if (local_top_reg4.bit.UART2 == MUX_4) {
		value |= PIN_UART_CFG_UART2_4;
	}

	if (local_top_reg4.bit.UART2_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART2_RTSCTS;
	} else if (local_top_reg4.bit.UART2_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART2_DIROE;
	}

	if (local_top_reg4.bit.UART3 == MUX_1) {
		value |= PIN_UART_CFG_UART3_1;
	} else if (local_top_reg4.bit.UART3 == MUX_2) {
		value |= PIN_UART_CFG_UART3_2;
	} else if (local_top_reg4.bit.UART3 == MUX_3) {
		value |= PIN_UART_CFG_UART3_3;
	} else if (local_top_reg4.bit.UART3 == MUX_4) {
		value |= PIN_UART_CFG_UART3_4;
	} else if (local_top_reg4.bit.UART3 == MUX_5) {
		value |= PIN_UART_CFG_UART3_5;
	} else if (local_top_reg4.bit.UART3 == MUX_6) {
		value |= PIN_UART_CFG_UART3_6;
	}

	if (local_top_reg4.bit.UART3_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART3_RTSCTS;
	} else if (local_top_reg4.bit.UART3_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART3_DIROE;
	}

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/* Parsing CSI */
	value = PIN_CSI_CFG_NONE;

	if (local_top_reg3.bit.SENSOR == MUX_5) {
		value |= PIN_CSI_CFG_CSI_1C4D;
	}

	if (local_top_reg3.bit.SENSOR == MUX_6) {
		value |= PIN_CSI_CFG_CSICSI2_1C2D;
	}

	if (local_top_reg3.bit.SENSOR == MUX_7) {
		value |= PIN_CSI_CFG_CSI_1C2D;
	}

	if (local_top_reg3.bit.SENSOR == MUX_8) {
		value |= PIN_CSI_CFG_CSI2_1C2D;
	}

	info->top_pinmux[PIN_FUNC_CSI].config = value;
	info->top_pinmux[PIN_FUNC_CSI].pin_function = PIN_FUNC_CSI;

	/* Parsing REMOTE */
	value = PIN_REMOTE_CFG_NONE;

	if (local_top_reg10.bit.REMOTE == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_1;
	} else if (local_top_reg10.bit.REMOTE == MUX_2) {
		value |= PIN_REMOTE_CFG_REMOTE_2;
	}

	if (local_top_reg10.bit.REMOTE_EXT == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_EXT_1;
	} else if (local_top_reg10.bit.REMOTE_EXT == MUX_2) {
		value |= PIN_REMOTE_CFG_REMOTE_EXT_2;
	}

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/* Parsing SDP */
	value = PIN_SDP_CFG_NONE;

	if (local_top_reg7.bit.SDP == MUX_1) {
		value |= PIN_SDP_CFG_SDP_1;
	} else if (local_top_reg7.bit.SDP == MUX_2) {
		value |= PIN_SDP_CFG_SDP_2;
	} else if (local_top_reg7.bit.SDP == MUX_3) {
		value |= PIN_SDP_CFG_SDP_3;
	}

	info->top_pinmux[PIN_FUNC_SDP].config = value;
	info->top_pinmux[PIN_FUNC_SDP].pin_function = PIN_FUNC_SDP;

	/* Parsing SPI */
	value = PIN_SPI_CFG_NONE;

	if (local_top_reg7.bit.SPI == MUX_1) {
		value |= PIN_SPI_CFG_SPI_1;
	} else if (local_top_reg7.bit.SPI == MUX_2) {
		value |= PIN_SPI_CFG_SPI_2;
	}

	if (local_top_reg7.bit.SPI_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI_BUS_WIDTH;
	}

	if (local_top_reg7.bit.SPI2 == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_1;
	} else if (local_top_reg7.bit.SPI2 == MUX_2) {
		value |= PIN_SPI_CFG_SPI2_2;
	} else if (local_top_reg7.bit.SPI2 == MUX_3) {
		value |= PIN_SPI_CFG_SPI2_3;
	} else if (local_top_reg7.bit.SPI2 == MUX_4) {
		value |= PIN_SPI_CFG_SPI2_4;
	}

	if (local_top_reg7.bit.SPI2_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_BUS_WIDTH;
	}

	if (local_top_reg7.bit.SPI3 == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_1;
	} else if (local_top_reg7.bit.SPI3 == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_2;
	} else if (local_top_reg7.bit.SPI3 == MUX_3) {
		value |= PIN_SPI_CFG_SPI3_3;
	} else if (local_top_reg7.bit.SPI3 == MUX_4) {
		value |= PIN_SPI_CFG_SPI3_4;
	}

	if (local_top_reg7.bit.SPI3_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_BUS_WIDTH;
	}

	if (local_top_reg7.bit.SPI3_RDY == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_RDY_1;
	} else if (local_top_reg7.bit.SPI3_RDY == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_RDY_2;
	} else if (local_top_reg7.bit.SPI3_RDY == MUX_3) {
		value |= PIN_SPI_CFG_SPI3_RDY_3;
	} else if (local_top_reg7.bit.SPI3_RDY == MUX_4) {
		value |= PIN_SPI_CFG_SPI3_RDY_4;
	}

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/* Parsing SIF */
	value = PIN_SIF_CFG_NONE;

	if (local_top_reg7.bit.SIF0 == MUX_1) {
		value |= PIN_SIF_CFG_SIF0_1;
	}

	if (local_top_reg7.bit.SIF1 == MUX_1) {
		value |= PIN_SIF_CFG_SIF1_1;
	}

	if (local_top_reg7.bit.SIF2 == MUX_1) {
		value |= PIN_SIF_CFG_SIF2_1;
	} else if (local_top_reg7.bit.SIF2 == MUX_2) {
		value |= PIN_SIF_CFG_SIF2_2;
	} else if (local_top_reg7.bit.SIF2 == MUX_3) {
		value |= PIN_SIF_CFG_SIF2_3;
	} else if (local_top_reg7.bit.SIF2 == MUX_4) {
		value |= PIN_SIF_CFG_SIF2_4;
	}

	if (local_top_reg7.bit.SIF3 == MUX_1) {
		value |= PIN_SIF_CFG_SIF3_1;
	} else if (local_top_reg7.bit.SIF3 == MUX_2) {
		value |= PIN_SIF_CFG_SIF3_2;
	}

	info->top_pinmux[PIN_FUNC_SIF].config = value;
	info->top_pinmux[PIN_FUNC_SIF].pin_function = PIN_FUNC_SIF;

	/* Parsing MISC */
	value = PIN_MISC_CFG_NONE;

	if (local_top_reg10.bit.RTC_CLK == MUX_1) {
		value |= PIN_MISC_CFG_RTC_CLK_1;
	}

	if (local_top_reg3.bit.SP_CLK == MUX_1) {
		value |= PIN_MISC_CFG_SP_CLK_1;
	} else if (local_top_reg3.bit.SP_CLK == MUX_2) {
		value |= PIN_MISC_CFG_SP_CLK_2;
	} else if (local_top_reg3.bit.SP_CLK == MUX_3) {
		value |= PIN_MISC_CFG_SP_CLK_3;
	} else if (local_top_reg3.bit.SP_CLK == MUX_4) {
		value |= PIN_MISC_CFG_SP_CLK_4;
	} else if (local_top_reg3.bit.SP_CLK == MUX_5) {
		value |= PIN_MISC_CFG_SP_CLK_5;
	}

	if (local_top_reg3.bit.SP_CLK2 == MUX_1) {
		value |= PIN_MISC_CFG_SP2_CLK_1;
	} else if (local_top_reg3.bit.SP_CLK2 == MUX_2) {
		value |= PIN_MISC_CFG_SP2_CLK_2;
	} else if (local_top_reg3.bit.SP_CLK2 == MUX_3) {
		value |= PIN_MISC_CFG_SP2_CLK_3;
	} else if (local_top_reg3.bit.SP_CLK2 == MUX_4) {
		value |= PIN_MISC_CFG_SP2_CLK_4;
	} 

	info->top_pinmux[PIN_FUNC_MISC].config = value;
	info->top_pinmux[PIN_FUNC_MISC].pin_function = PIN_FUNC_MISC;

	/* Parsing LCD */
	info->top_pinmux[PIN_FUNC_LCD].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD];
	//info->top_pinmux[PIN_FUNC_LCD].config = 0;
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/* Parsing TV */
	//info->top_pinmux[PIN_FUNC_TV].config = 0;
	info->top_pinmux[PIN_FUNC_TV].config = disp_pinmux_config[PINMUX_FUNC_ID_TV];
	info->top_pinmux[PIN_FUNC_TV].pin_function = PIN_FUNC_TV;

	/* Parsing SEL_LCD */
#if 0
	value = PINMUX_LCD_SEL_GPIO;
	info->top_pinmux[PIN_FUNC_SEL_LCD].config = value;
        info->top_pinmux[PIN_FUNC_SEL_LCD].pin_function = PIN_FUNC_SEL_LCD;
#else
	value = PINMUX_LCD_SEL_GPIO;

	/* LCD mode select */
	if (local_top_reg2.bit.LCD_TYPE == MUX_1) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR656_16BITS;
		} else {
			value = PINMUX_LCD_SEL_CCIR656;
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_2) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR601_16BITS;

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
	}

	if (local_top_reg2.bit.PLCD_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD].pin_function = PIN_FUNC_SEL_LCD;
#endif

	/* Pre-roll part parsing*/
	/* Parsing PR-I2C */
	value = PIN_PR_I2C_CFG_NONE;

	if (local_pr_top_reg0.bit.PR_I2C == MUX_1) {
		value |= PIN_PR_I2C_CFG_I2C_1;
	} else if (local_pr_top_reg0.bit.PR_I2C == MUX_2) {
		value |= PIN_PR_I2C_CFG_I2C_2;
	}

	info->top_pinmux[PIN_FUNC_PR_I2C].config = value;
	info->top_pinmux[PIN_FUNC_PR_I2C].pin_function = PIN_FUNC_PR_I2C;

	g_pr_i2c_config = value;

	/* Parsing PR-UART */
	value = PIN_PR_UART_CFG_NONE;

	if (local_pr_top_reg1.bit.PR_UART == MUX_1) {
		value |= PIN_PR_UART_CFG_UART_1;
	} else if (local_pr_top_reg1.bit.PR_UART == MUX_2) {
		value |= PIN_PR_UART_CFG_UART_2;
	}

	if (local_pr_top_reg1.bit.PR_UART2 == MUX_1) {
		value |= PIN_PR_UART_CFG_UART2_1;
	} else if (local_pr_top_reg1.bit.PR_UART2 == MUX_2) {
		value |= PIN_PR_UART_CFG_UART2_2;
	}

	info->top_pinmux[PIN_FUNC_PR_UART].config = value;
	info->top_pinmux[PIN_FUNC_PR_UART].pin_function = PIN_FUNC_PR_UART;

	/* Parsing PR-SENSOR */
	value = PIN_PR_SENSOR_CFG_NONE;

	if (local_pr_top_reg3.bit.PR_SENSOR == MUX_1) {
		value |= PIN_PR_SENSOR_CFG_SVT_2BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_2) {
		value |= PIN_PR_SENSOR_CFG_SVT_4BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_3) {
		value |= PIN_PR_SENSOR_CFG_DVP;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_4) {
		value |= PIN_PR_SENSOR_CFG_AO_1BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_5) {
		value |= PIN_PR_SENSOR_CFG_AO_2BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_6) {
		value |= PIN_PR_SENSOR_CFG_AO_4BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_7) {
		value |= PIN_PR_SENSOR_CFG_SPI_PIXART_2BIT;
	} else if (local_pr_top_reg3.bit.PR_SENSOR == MUX_8) {
		value |= PIN_PR_SENSOR_CFG_SPI_PIXART_4BIT;
	}

	info->top_pinmux[PIN_FUNC_PR_SENSOR].config = value;
	info->top_pinmux[PIN_FUNC_PR_SENSOR].pin_function = PIN_FUNC_PR_SENSOR;

	/* Parsing PR-SENSORMISC */
	value = PIN_PR_SENSORMISC_CFG_NONE;

	if (local_pr_top_reg3.bit.PR_SN_MCLK == MUX_1) {
		value |= PIN_PR_SENSORMISC_CFG_SN_MCLK_1;
	}

	if (local_pr_top_reg3.bit.PR_SRAM == MUX_1) {
		value |= PIN_PR_SENSORMISC_CFG_SRAM_1;
	} else if (local_pr_top_reg3.bit.PR_SRAM == MUX_2) {
		value |= PIN_PR_SENSORMISC_CFG_SRAM_2;
	}

	if (local_pr_top_reg3.bit.PR_SRAM_SIO == MUX_1) {
		value |= PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT;
	} else if (local_pr_top_reg3.bit.PR_SRAM_SIO == MUX_2) {
		value |= PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT;
	} else if (local_pr_top_reg3.bit.PR_SRAM_SIO == MUX_3) {
		value |= PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT;
	}

	info->top_pinmux[PIN_FUNC_PR_SENSORMISC].config = value;
	info->top_pinmux[PIN_FUNC_PR_SENSORMISC].pin_function = PIN_FUNC_PR_SENSORMISC;

	g_pr_senmisc_config = value;

	/* Parsing PR-SPI_WG */
	value = PIN_PR_SPI_CFG_NONE;

	if (local_pr_top_reg5.bit.PR_SPI_WG == MUX_1) {
		value |= PIN_PR_SPI_CFG_SPI_WG_1;
	} else if (local_pr_top_reg5.bit.PR_SPI_WG == MUX_2) {
		value |= PIN_PR_SPI_CFG_SPI_WG_2;
	}

	info->top_pinmux[PIN_FUNC_PR_SPI].config = value;
	info->top_pinmux[PIN_FUNC_PR_SPI].pin_function = PIN_FUNC_PR_SPI;

	/* Parsing PR-PWM */
	value = PIN_PR_PWM_CFG_NONE;

	if (local_pr_top_reg6.bit.PR_PWM0 == MUX_1) {
		value |= PIN_PR_PWM_CFG_PWM0_1;
	} else if (local_pr_top_reg6.bit.PR_PWM0 == MUX_2) {
		value |= PIN_PR_PWM_CFG_PWM0_2;
	}

	if (local_pr_top_reg6.bit.PR_PWM1 == MUX_1) {
		value |= PIN_PR_PWM_CFG_PWM1_1;
	} else if (local_pr_top_reg6.bit.PR_PWM1 == MUX_2) {
		value |= PIN_PR_PWM_CFG_PWM1_2;
	}

	info->top_pinmux[PIN_FUNC_PR_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PR_PWM].pin_function = PIN_FUNC_PR_PWM;

	/* Parsing PR-AUDIO */
	value = PIN_PR_AUDIO_CFG_NONE;

	if (local_pr_top_reg5.bit.PR_DMIC == MUX_1) {
		value |= PIN_PR_AUDIO_CFG_DMIC_1;
	}

	info->top_pinmux[PIN_FUNC_PR_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_PR_AUDIO].pin_function = PIN_FUNC_PR_AUDIO;

	/* Leave critical section */
	unl_cpu(flags);

}

/* Function conflict handle : MAIN PART pad */
void gpio_func_keep(int start, int count, int func)
{
	int i = 0;

	for (i = start; i < start + count; i++) {
		dump_gpio_func[i] = func;
	}
}

int gpio_conflict_detect(int start,int count,int func)
{
	int i=0;
    int confl_mod;
    int confl_flag=0;
    for(i = start; i < start + count; i++)
    {
        confl_mod = dump_gpio_func[i];
        //printf("%d\r\n",confl_mod);

        if(confl_mod >0 )
        {
            pr_err("%s conflict with %s\r\n", dump_func[func - 1], dump_func[confl_mod - 1]);
            confl_flag++;
            break;
        }
    }
    return confl_flag;

}


/* Function conflict handle : Pre-roll part pad */
void pr_gpio_func_keep(int start, int count, int func)
{
        int i = 0;

        for (i = start; i < start + count; i++) {
                pr_dump_gpio_func[i] = func;
        }
}

int pr_gpio_conflict_detect(int start,int count,int func)
{
        int i=0;
    int confl_mod;
    int confl_flag=0;
    for(i = start; i < start + count; i++)
    {
        confl_mod = pr_dump_gpio_func[i];
        //printf("%d\r\n",confl_mod);

        if(confl_mod >0 )
        {
            pr_err("%s conflict with %s\r\n", dump_func[func - 1], dump_func[confl_mod - 1]); //main part function
            confl_flag++;
            break;
        }
    }
    return confl_flag;

}

/*--------------------------------------------------------*/
/*          PINMUX Interface Functions(Pre-Roll)          */
/*--------------------------------------------------------*/
static int pinmux_config_pr_i2c(uint32_t config)
{
	g_pr_i2c_config = config;
	//printk("g_pr_i2c_config is 0x%x\n", g_pr_i2c_config);

	if (config == PIN_PR_I2C_CFG_NONE) {
	} else {
		if (config & PIN_PR_I2C_CFG_I2C_1) {

			confl_detect += pr_gpio_conflict_detect(SGPIO_7 , 2 , func_PR_I2C_1);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			pr_top_reg0.bit.PR_I2C = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC_PR;
			pad_set_pull_updown(PAD_PIN_SGPIO7, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO8, PAD_NONE);
			pr_gpio_func_keep(SGPIO_7, 2, func_PR_I2C_1);

		} else if (config & PIN_PR_I2C_CFG_I2C_2) {

			confl_detect += pr_gpio_conflict_detect(PGPIO_29 , 2 , func_PR_I2C_2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			pr_top_reg0.bit.PR_I2C = MUX_2;
			pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC_PR;
			pad_set_pull_updown(PAD_PIN_PGPIO29, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO30, PAD_NONE);
			pr_gpio_func_keep(PGPIO_29, 2, func_PR_I2C_2);
		}
	}

	return E_OK;
}

static int pinmux_config_pr_uart(uint32_t config)
{
	//printk("pinmux_config_pr_uart = 0x%08x\r\n", config);

	if (config == PIN_PR_UART_CFG_NONE) {
	} else {
		//UART_1
		if (config & PIN_PR_UART_CFG_UART_1) {

			confl_detect += pr_gpio_conflict_detect(SGPIO_2, 2, func_PR_UART_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg1.bit.PR_UART = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_2, 2, func_PR_UART_1);

		} else if (PIN_PR_UART_CFG_UART_2) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_15, 2, func_PR_UART_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg1.bit.PR_UART = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_15, 2, func_PR_UART_2);
		}

		//UART2_1
		if (config & PIN_PR_UART_CFG_UART2_1) {

			confl_detect += pr_gpio_conflict_detect(PGPIO_31 , 2 , func_PR_UART2_1);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			pr_top_reg1.bit.PR_UART2 = MUX_1;
			pr_top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_pgpio0.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_31, 2, func_PR_UART2_1);

		//UART2_2
		} else if (config & PIN_PR_UART_CFG_UART2_2) {

			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 2, func_PR_UART2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg1.bit.PR_UART2 = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_9, 2, func_PR_UART2_2);
		}


	}

	return E_OK;
}

static int pinmux_config_pr_sensor(uint32_t config)
{
	if (config == PIN_PR_SENSOR_CFG_NONE) {
	} else {
		if (config & PIN_PR_SENSOR_CFG_SVT_2BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_2, 2, func_PR_SENSOR);
			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 3, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_2, 2, func_PR_SENSOR);
			pr_gpio_func_keep(SGPIO_10, 3, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_SVT_4BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_2, 2, func_PR_SENSOR);
			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 5, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_2, 2, func_PR_SENSOR);
			pr_gpio_func_keep(SGPIO_10, 5, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_DVP) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_1, 3, func_PR_SENSOR);
			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 8, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_1, 3, func_PR_SENSOR);
			pr_gpio_func_keep(SGPIO_9, 8, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_AO_1BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 2, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_4;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_10, 2, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_AO_2BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 3, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_10, 3, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_AO_4BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 5, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_6;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_10, 5, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_SPI_PIXART_2BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 4, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_7;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_9, 4, func_PR_SENSOR);

		} else if (config & PIN_PR_SENSOR_CFG_SPI_PIXART_4BIT) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 6, func_PR_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SENSOR = MUX_8;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_9, 6, func_PR_SENSOR);
		}

	}

	return E_OK;
}


static int pinmux_config_pr_sensormisc(uint32_t config)
{
	g_pr_senmisc_config = config;

	if (config == PIN_PR_SENSORMISC_CFG_NONE) {
	} else {
		if (config & PIN_PR_SENSORMISC_CFG_SN_MCLK_1) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_0, 1, func_PR_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SN_MCLK = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_0, 1, func_PR_SENSORMISC);
		}

		if (config & PIN_PR_SENSORMISC_CFG_SRAM_1) {
			confl_detect += pr_gpio_conflict_detect(PGPIO_33, 2, func_PR_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SRAM = MUX_1;
			pr_top_reg_pgpio0.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_pgpio0.bit.PGPIO_34 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_33, 2, func_PR_SENSORMISC);

			if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_35, 1, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_1;
				pr_top_reg_pgpio0.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_35, 1, func_PR_SENSORMISC);

			} else if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_35, 2, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_2;
				pr_top_reg_pgpio0.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_35, 2, func_PR_SENSORMISC);

			} else if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_35, 4, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_3;
				pr_top_reg_pgpio0.bit.PGPIO_35 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_38 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_35, 4, func_PR_SENSORMISC);
			}

		} else if (config & PIN_PR_SENSORMISC_CFG_SRAM_2) {
			confl_detect += pr_gpio_conflict_detect(PGPIO_25, 2, func_PR_SENSORMISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg3.bit.PR_SRAM = MUX_2;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_25, 2, func_PR_SENSORMISC);

			if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_1BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_27, 1, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_1;
				pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_27, 1, func_PR_SENSORMISC);

			} else if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_2BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_27, 2, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_2;
				pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_27, 2, func_PR_SENSORMISC);

			} else if (config & PIN_PR_SENSORMISC_CFG_SRAM_SIO_4BIT) {
				confl_detect += pr_gpio_conflict_detect(PGPIO_27, 4, func_PR_SENSORMISC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				pr_top_reg3.bit.PR_SRAM_SIO = MUX_3;
				pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_PR;
				pr_top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC_PR;
				pr_gpio_func_keep(PGPIO_27, 4, func_PR_SENSORMISC);
			}

		}

	}

	return E_OK;
}

/*
  PR-SPI Wiegand
*/
static int pinmux_config_pr_spi(uint32_t config)
{
	if (config == PIN_PR_SPI_CFG_NONE) {
	} else {
		if (config & PIN_PR_SPI_CFG_SPI_WG_1) {
			confl_detect += pr_gpio_conflict_detect(PGPIO_25, 2, func_PR_SPI_WG_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg5.bit.PR_SPI_WG = MUX_1;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_25, 2, func_PR_SPI_WG_1);

		} else if (config & PIN_PR_SPI_CFG_SPI_WG_2) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_11, 2, func_PR_SPI_WG_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg5.bit.PR_SPI_WG = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_11, 2, func_PR_SPI_WG_2);

		}
	}
	return E_OK;
}


static int pinmux_config_pr_pwm(uint32_t config)
{
	if (config == PIN_PR_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PR_PWM_CFG_PWM0_1) {
			confl_detect += pr_gpio_conflict_detect(PGPIO_27, 1, func_PR_PWM_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg6.bit.PR_PWM0 = MUX_1;
			pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_27, 1, func_PR_PWM_1);

		} else if (config & PIN_PR_PWM_CFG_PWM0_2) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_13, 1, func_PR_PWM_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg6.bit.PR_PWM0 = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_13, 1, func_PR_PWM_2);

		}

		if (config & PIN_PR_PWM_CFG_PWM1_1) {
			confl_detect += pr_gpio_conflict_detect(PGPIO_28, 1, func_PR_PWM1_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg6.bit.PR_PWM1 = MUX_1;
			pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(PGPIO_28, 1, func_PR_PWM1_1);

		} else if (config & PIN_PR_PWM_CFG_PWM1_2) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_14, 1, func_PR_PWM1_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg6.bit.PR_PWM1 = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_14, 1, func_PR_PWM1_2);

		}
	}

	return E_OK;
}

static int pinmux_config_pr_audio(uint32_t config)
{
	if (config == PIN_PR_AUDIO_CFG_NONE) {
	} else {
		if (config & PIN_PR_AUDIO_CFG_DMIC_1) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_5, 2, func_PR_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			pr_top_reg5.bit.PR_DMIC = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_PR;
			pr_top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC_PR;
			pr_gpio_func_keep(SGPIO_5, 2, func_PR_AUDIO_DMIC);

		}
	}

	return E_OK;
}
/*----------------------------------------------*/
/*          PINMUX Interface Functions          */
/*----------------------------------------------*/
static int pinmux_config_sdio(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_SDIO_1) {

			confl_detect += gpio_conflict_detect(CGPIO_13 , 6 , func_SDIO);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 6, func_SDIO);
		}

		if (config & PIN_SDIO_CFG_SDIO2_1) {
			confl_detect += gpio_conflict_detect(CGPIO_19 , 6 , func_SDIO2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.SDIO2_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_19, 6, func_SDIO2);
		} else if (config & PIN_SDIO_CFG_SDIO2_2) {
			confl_detect += gpio_conflict_detect(PGPIO_2 , 6 , func_SDIO2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.SDIO2_EN = MUX_2;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_2, 6, func_SDIO2);
		}

		if (config & PIN_SDIO_CFG_SDIO3_1) {
			confl_detect += gpio_conflict_detect(CGPIO_0 , 4 , func_SDIO3);
			confl_detect += gpio_conflict_detect(CGPIO_10 , 2 , func_SDIO3);
			if(confl_detect > 0)
			{
				return E_PAR;
			}


			top_reg1.bit.SDIO3_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 4, func_SDIO3);
			gpio_func_keep(CGPIO_10, 2, func_SDIO3);

			if (config & PIN_SDIO_CFG_SDIO3_BUS_WIDTH) {
				confl_detect += gpio_conflict_detect(CGPIO_4 , 4 , func_SDIO3);
				if(confl_detect > 0)
				{
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
				confl_detect += gpio_conflict_detect(CGPIO_8 , 1 , func_SDIO3);
				if(confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg1.bit.SDIO3_DS_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC; //for 567 DS function.
				gpio_func_keep(CGPIO_8, 1, func_SDIO3);
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

			confl_detect += gpio_conflict_detect(CGPIO_0 , 4 , func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_10 , 1 , func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_12 , 1 , func_FSPI);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.FSPI_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 4, func_FSPI);
			gpio_func_keep(CGPIO_10, 1, func_FSPI);
			gpio_func_keep(CGPIO_12, 1, func_FSPI);

		} else if (config & PIN_NAND_CFG_NAND_2) {
			
			confl_detect += gpio_conflict_detect(CGPIO_0, 9, func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_10, 1, func_FSPI);
			confl_detect += gpio_conflict_detect(CGPIO_12, 1, func_FSPI);
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
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_0, 9, func_FSPI);
			gpio_func_keep(CGPIO_10, 1, func_FSPI);
			gpio_func_keep(CGPIO_12, 1, func_FSPI);
		}

		if (config & PIN_NAND_CFG_NAND_CS1) {

			confl_detect += gpio_conflict_detect(CGPIO_9 , 1 , func_FSPI);
			if(confl_detect > 0)
			{
					return E_PAR;
			}

			top_reg1.bit.FSPI_CS1_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_9, 1, func_FSPI);
		}

		if (config & PIN_NAND_CFG_NAND_TRIGA_1) {

			confl_detect += gpio_conflict_detect(CGPIO_21, 2, func_FSPI);
			if (confl_detect > 0) {
					return E_PAR;
			}

			top_reg1.bit.FLASH_TRIGA = MUX_1;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(CGPIO_21, 1, func_FSPI);
			gpio_func_keep(CGPIO_22, 1, func_FSPI);

		} else if (config & PIN_NAND_CFG_NAND_TRIGA_2) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 2, func_FSPI);
			if (confl_detect > 0) {
					return E_PAR;
			}

			top_reg1.bit.FLASH_TRIGA = MUX_2;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(PGPIO_11, 2, func_FSPI);
		}
	}

	return E_OK;
}

static int pinmux_config_eth(uint32_t config)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & PIN_ETH_CFG_ETH_RMII_1) {

			confl_detect += gpio_conflict_detect(PGPIO_0 , 6 , func_ETH);
			confl_detect += gpio_conflict_detect(PGPIO_8 , 1 , func_ETH);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg9.bit.ETH = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 6, func_ETH);
			gpio_func_keep(PGPIO_8, 1, func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {
			top_reg9.bit.ETH_EXT_PHY_CLK = MUX_1;

			confl_detect += gpio_conflict_detect(PGPIO_7 , 1 , func_ETH);
			if(confl_detect > 0)
			{
					return E_PAR;
			}

			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_7, 1, func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH_MDIO_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 2, func_ETH);
			if (confl_detect > 0) {
						return E_PAR;
			}

			top_reg9.bit.ETH_MDIO_MUX = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 2, func_ETH);
		}


		if (config & PIN_ETH_CFG_ETH_LED_1) {

			confl_detect += gpio_conflict_detect(DGPIO_0, 2, func_ETH);
			if (confl_detect > 0) {
					return E_PAR;
			}

			top_reg9.bit.ETH_LED = MUX_1;
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_0, 2, func_ETH);
		} else if (config & PIN_ETH_CFG_ETH_LED_2) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 2, func_ETH);
			if (confl_detect > 0) {
						return E_PAR;
			}

			top_reg9.bit.ETH_LED = MUX_2;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 2, func_ETH);
		}

	}

	return E_OK;
}

static int pinmux_config_i2c(uint32_t config)
{
	g_i2c_config = config;

	//printk("g_i2c_config is 0x%x\n", g_i2c_config);

	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & PIN_I2C_CFG_I2C_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21 , 2 , func_I2C_1);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
			gpio_func_keep(PGPIO_21, 2, func_I2C_1);
		} else if (config & PIN_I2C_CFG_I2C_2) {

			confl_detect += gpio_conflict_detect(CGPIO_13 , 2 , func_I2C_2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_2;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_CGPIO14, PAD_NONE);
			gpio_func_keep(CGPIO_13, 2, func_I2C_2);
		} else if (config & PIN_I2C_CFG_I2C_3) {

			confl_detect += gpio_conflict_detect(PGPIO_9 , 2 , func_I2C_3);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_3;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
			gpio_func_keep(PGPIO_9, 2, func_I2C_3);
		} else if (config & PIN_I2C_CFG_I2C_4) {

			confl_detect += gpio_conflict_detect(PGPIO_11 , 2 , func_I2C_4);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_4;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
			gpio_func_keep(PGPIO_11, 2, func_I2C_4);
		} else if (config & PIN_I2C_CFG_I2C_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_15 , 2 , func_I2C_5);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio1.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_MAIN;
			pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
			pr_gpio_func_keep(SGPIO_15, 2, func_I2C_5);
		}

		if (config & PIN_I2C_CFG_I2C2_1) {

			confl_detect += gpio_conflict_detect(CGPIO_19 , 2 , func_I2C2_1);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_1;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_CGPIO20, PAD_NONE);
			gpio_func_keep(CGPIO_19, 2, func_I2C2_1);
		} else if (config & PIN_I2C_CFG_I2C2_2) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_31 , 2 , func_I2C2_2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_2;
			pr_top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC_MAIN;
			pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_PGPIO32, PAD_NONE);
			pr_gpio_func_keep(PGPIO_31, 2, func_I2C2_2);

		} else if (config & PIN_I2C_CFG_I2C2_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_4 , 2 , func_I2C2_3);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
			pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
			pr_gpio_func_keep(SGPIO_4, 2, func_I2C2_3);

		} else if (config & PIN_I2C_CFG_I2C2_4) {

			confl_detect += gpio_conflict_detect(HSIGPIO_9 , 2 , func_I2C2_4);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_4;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
			gpio_func_keep(HSIGPIO_9, 2, func_I2C2_4);
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

			confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_PWM_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_15, 1, func_PWM_2);
		} else if (config & PIN_PWM_CFG_PWM0_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_1, 1, func_PWM_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_1, 1, func_PWM_3);
		} else if (config & PIN_PWM_CFG_PWM0_4) {

			confl_detect += gpio_conflict_detect(CGPIO_4, 1, func_PWM_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_4, 1, func_PWM_4);
		} else if (config & PIN_PWM_CFG_PWM0_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 1, func_PWM_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM0 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_9, 1, func_PWM_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_16, 1, func_PWM1_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_16, 1, func_PWM1_2);
		} else if (config & PIN_PWM_CFG_PWM1_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_2, 1, func_PWM1_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_2, 1, func_PWM1_3);
		} else if (config & PIN_PWM_CFG_PWM1_4) {

			confl_detect += gpio_conflict_detect(CGPIO_5, 1, func_PWM1_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_5, 1, func_PWM1_4);
		} else if (config & PIN_PWM_CFG_PWM1_5) {

			confl_detect += pr_gpio_conflict_detect(SGPIO_10, 1, func_PWM1_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM1 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_10, 1, func_PWM1_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_17, 1, func_PWM2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 1, func_PWM2_2);
		} else if (config & PIN_PWM_CFG_PWM2_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_3, 1, func_PWM2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_3, 1, func_PWM2_3);
		} else if (config & PIN_PWM_CFG_PWM2_4) {

			confl_detect += gpio_conflict_detect(CGPIO_6, 1, func_PWM2_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_6, 1, func_PWM2_4);
		} else if (config & PIN_PWM_CFG_PWM2_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_11, 1, func_PWM2_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM2 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_11, 1, func_PWM2_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_18, 1, func_PWM3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_18, 1, func_PWM3_2);
		} else if (config & PIN_PWM_CFG_PWM3_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_4, 1, func_PWM3_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_4, 1, func_PWM3_3);
		} else if (config & PIN_PWM_CFG_PWM3_4) {

			confl_detect += gpio_conflict_detect(CGPIO_7, 1, func_PWM3_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_4;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_7, 1, func_PWM3_4);
		} else if (config & PIN_PWM_CFG_PWM3_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_12, 1, func_PWM3_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM3 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_12, 1, func_PWM3_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_PWM4_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 1, func_PWM4_2);
		} else if (config & PIN_PWMII_CFG_PWM4_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_5, 1, func_PWM4_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_5, 1, func_PWM4_3);
		} else if (config & PIN_PWMII_CFG_PWM4_4) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 1, func_PWM4_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_3, 1, func_PWM4_4);
		} else if (config & PIN_PWMII_CFG_PWM4_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_13, 1, func_PWM4_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM4 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_13, 1, func_PWM4_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_14, 1, func_PWM5_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 1, func_PWM5_2);
		} else if (config & PIN_PWMII_CFG_PWM5_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_6, 1, func_PWM5_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_6, 1, func_PWM5_3);
		} else if (config & PIN_PWMII_CFG_PWM5_4) {

			confl_detect += gpio_conflict_detect(DGPIO_4, 1, func_PWM5_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_4, 1, func_PWM5_4);
		} else if (config & PIN_PWMII_CFG_PWM5_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_14, 1, func_PWM5_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM5 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_14, 1, func_PWM5_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_19, 1, func_PWM6_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_19, 1, func_PWM6_2);
		} else if (config & PIN_PWMII_CFG_PWM6_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_7, 1, func_PWM6_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_7, 1, func_PWM6_3);
		} else if (config & PIN_PWMII_CFG_PWM6_4) {

			confl_detect += gpio_conflict_detect(DGPIO_5, 1, func_PWM6_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_5, 1, func_PWM6_4);
		} else if (config & PIN_PWMII_CFG_PWM6_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_15, 1, func_PWM6_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM6 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_15, 1, func_PWM6_5);
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

			confl_detect += gpio_conflict_detect(CGPIO_20, 1, func_PWM7_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_20, 1, func_PWM7_2);
		} else if (config & PIN_PWMII_CFG_PWM7_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_8, 1, func_PWM7_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_8, 1, func_PWM7_3);
		} else if (config & PIN_PWMII_CFG_PWM7_4) {

			confl_detect += gpio_conflict_detect(DGPIO_6, 1, func_PWM7_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_4;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_6, 1, func_PWM7_4);
		} else if (config & PIN_PWMII_CFG_PWM7_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_16, 1, func_PWM7_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM7 = MUX_5;
			pr_top_reg_sgpio1.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_16, 1, func_PWM7_5);
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

			top_reg5.bit.PWM8 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_PWM8_1);

		} else if (config & PIN_PWMIII_CFG_PWM8_2) {

			confl_detect += gpio_conflict_detect(CGPIO_21, 1, func_PWM8_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM8 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_21, 1, func_PWM8_2);

		} else if (config & PIN_PWMIII_CFG_PWM8_3) {

			confl_detect += gpio_conflict_detect(HSIGPIO_6, 1, func_PWM8_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM8 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_6, 1, func_PWM8_3);

		} else if (config & PIN_PWMIII_CFG_PWM8_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_25, 1, func_PWM8_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM8 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_25, 1, func_PWM8_4);
		}

		if (config & PIN_PWMIII_CFG_PWM9_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_PWM9_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM9 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 1, func_PWM9_1);

		} else if (config & PIN_PWMIII_CFG_PWM9_2) {

			confl_detect += gpio_conflict_detect(CGPIO_22, 1, func_PWM9_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM9 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_22, 1, func_PWM9_2);

		} else if (config & PIN_PWMIII_CFG_PWM9_3) {

			confl_detect += gpio_conflict_detect(HSIGPIO_7, 1, func_PWM9_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM9 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_7, 1, func_PWM9_3);

		} else if (config & PIN_PWMIII_CFG_PWM9_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_26, 1, func_PWM9_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg5.bit.PWM9 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_26, 1, func_PWM9_4);
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

			confl_detect += gpio_conflict_detect(CGPIO_23, 1, func_PWM10_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_23, 1, func_PWM10_2);

		} else if (config & PIN_PWMIII_CFG_PWM10_3) {

			confl_detect += gpio_conflict_detect(HSIGPIO_8, 1, func_PWM10_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_8, 1, func_PWM10_3);

		} else if (config & PIN_PWMIII_CFG_PWM10_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_27, 1, func_PWM10_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM10 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_27, 1, func_PWM10_4);
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

			confl_detect += gpio_conflict_detect(CGPIO_24, 1, func_PWM11_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_24, 1, func_PWM11_2);

		} else if (config & PIN_PWMIII_CFG_PWM11_3) {

			confl_detect += gpio_conflict_detect(HSIGPIO_9, 1, func_PWM11_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_9, 1, func_PWM11_3);

		} else if (config & PIN_PWMIII_CFG_PWM11_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_28, 1, func_PWM11_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg6.bit.PWM11 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_28, 1, func_PWM11_4);
		}
	}

	return E_OK;
}

static int pinmux_config_ccnt(uint32_t config)
{
	if (config == PIN_CCNT_CFG_NONE) {
	} else {
		if (config & PIN_CCNT_CFG_CCNT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_CCNT_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 1, func_CCNT_1);

		} else if (config & PIN_CCNT_CFG_CCNT_2) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_CCNT_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT = MUX_2;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 1, func_CCNT_2);

		} else if (config & PIN_CCNT_CFG_CCNT_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_29, 1, func_CCNT_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT = MUX_2;
			pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_29, 1, func_CCNT_3);
		}

		if (config & PIN_CCNT_CFG_CCNT2_1) {

			confl_detect += gpio_conflict_detect(CGPIO_23, 1, func_CCNT2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT2 = MUX_1;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_23, 1, func_CCNT2_1);

		} else if (config & PIN_CCNT_CFG_CCNT2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_CCNT2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 1, func_CCNT2_2);

		} else if (config & PIN_CCNT_CFG_CCNT2_3) {

			confl_detect += gpio_conflict_detect(CGPIO_21, 1, func_CCNT2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT2 = MUX_3;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_21, 1, func_CCNT2_3);

		} else if (config & PIN_CCNT_CFG_CCNT2_4) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 1, func_CCNT2_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT2 = MUX_4;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_11, 1, func_CCNT2_4);
		}

		if (config & PIN_CCNT_CFG_CCNT3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 1, func_CCNT3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 1, func_CCNT3_1);

		} else if (config & PIN_CCNT_CFG_CCNT3_2) {

			confl_detect += gpio_conflict_detect(PGPIO_20, 1, func_CCNT3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.PICNT3 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_20, 1, func_CCNT3_2);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor(uint32_t config)
{
	if (config == PIN_SENSOR_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR_CFG_12BITS) { //PR_FIXG

			confl_detect += gpio_conflict_detect(PGPIO_0, 4, func_SENSOR);
			confl_detect += gpio_conflict_detect(HSIGPIO_2, 9, func_SENSOR);
			confl_detect += pr_gpio_conflict_detect(SGPIO_2, 2, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;

			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;

			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_MAIN;

			gpio_func_keep(PGPIO_0, 4, func_SENSOR);
			gpio_func_keep(HSIGPIO_2, 9, func_SENSOR);
			pr_gpio_func_keep(SGPIO_2, 2, func_SENSOR);
		} else if (config & PIN_SENSOR_CFG_12BITS_2) {
			//not support
			return E_PAR;
		} else if (config & PIN_SENSOR_CFG_CCIR8BITS) {

			confl_detect += gpio_conflict_detect(HSIGPIO_2, 9, func_SENSOR);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_3;

			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_2, 9, func_SENSOR);

			if (config & PIN_SENSOR_CFG_CCIR_VSHS) {

				confl_detect += gpio_conflict_detect(HSIGPIO_0, 2, func_SN1_CCIR_VSHS);
				confl_detect += gpio_conflict_detect(HSIGPIO_10, 1, func_SN1_CCIR_VSHS);

				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.SN_CCIR_VSHS = MUX_0;

				top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(HSIGPIO_0, 2, func_SN1_CCIR_VSHS);
				gpio_func_keep(HSIGPIO_10, 1, func_SN1_CCIR_VSHS);
			}

		} else if (config & PIN_SENSOR_CFG_CCIR16BITS) {
			confl_detect += gpio_conflict_detect(HSIGPIO_2, 9, func_SENSOR);
			confl_detect += gpio_conflict_detect(PGPIO_0, 8, func_SENSOR);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_4;

			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;


			gpio_func_keep(HSIGPIO_2, 9, func_SENSOR);
			gpio_func_keep(PGPIO_0, 8, func_SENSOR);

			if (config & PIN_SENSOR_CFG_CCIR_VSHS) {

				confl_detect += gpio_conflict_detect(HSIGPIO_0, 2, func_SN1_CCIR_VSHS);
				confl_detect += gpio_conflict_detect(HSIGPIO_10, 1, func_SN1_CCIR_VSHS);

				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.SN_CCIR_VSHS = MUX_0;

				top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(HSIGPIO_0, 2, func_SN1_CCIR_VSHS);
				gpio_func_keep(HSIGPIO_10, 1, func_SN1_CCIR_VSHS);
			}
		}

		if (config & PIN_SENSOR_CFG_SN_VSHS) {
				confl_detect += gpio_conflict_detect(HSIGPIO_0, 2, func_SN1_VSHS);
				confl_detect += gpio_conflict_detect(HSIGPIO_10, 1, func_SN1_VSHS);
			
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg3.bit.SN_VSHS = MUX_1;

				top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
				top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(HSIGPIO_0, 2, func_SN1_VSHS);
				gpio_func_keep(HSIGPIO_10, 1, func_SN1_VSHS);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor2(uint32_t config)
{
	if (config == PIN_SENSOR2_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR2_CFG_12BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_SENSOR2);
			confl_detect += gpio_conflict_detect(HSIGPIO_6, 4, func_SENSOR2);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR2 = MUX_1;
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

			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(PGPIO_0, 11, func_SENSOR2);
			gpio_func_keep(HSIGPIO_6, 4, func_SENSOR2);

		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS) {
			confl_detect += gpio_conflict_detect(PGPIO_8, 3, func_SENSOR2);
			confl_detect += gpio_conflict_detect(HSIGPIO_2, 8, func_SENSOR2);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 3, func_SENSOR2);
			gpio_func_keep(HSIGPIO_2, 8, func_SENSOR2);
		}

		if (config & PIN_SENSOR2_CFG_SN2_VSHS) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 3, func_SN2_VSHS);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SN2_VSHS = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 3, func_SN2_VSHS);
		}
	}

	return E_OK;
}

static int pinmux_config_sensormisc(uint32_t config)
{
	g_senmisc_config = config;

	if (config == PIN_SENSORMISC_CFG_NONE) {
	} else {
		/* MCLK */
		if (config & PIN_SENSORMISC_CFG_SN_MCLK_1) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_0 , 1 , func_SN1_MCLK);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN_MCLK = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_0, 1, func_SN1_MCLK);

		} else if (config & PIN_SENSORMISC_CFG_SN_MCLK_2) {

			confl_detect += gpio_conflict_detect(HSIGPIO_11 , 1 , func_SN1_MCLK);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN_MCLK = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_11, 1, func_SN1_MCLK);

		}

		if (config & PIN_SENSORMISC_CFG_SN2_MCLK_1) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_1 , 1 , func_SN2_MCLK);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN2_MCLK = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_1, 1, func_SN2_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN3_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_12 , 1 , func_SN3_MCLK);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN3_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 1, func_SN3_MCLK);
		}

		/* XVSXHS */
		if (config & PIN_SENSORMISC_CFG_SN_XVSXHS_1) { //PR_FIXG
			confl_detect += pr_gpio_conflict_detect(SGPIO_2 , 2 , func_SN1_XVSXHS);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN_XVSXHS = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_2, 2, func_SN1_XVSXHS);

		} else if (config & PIN_SENSORMISC_CFG_SN_XVSXHS_2) {
			confl_detect += gpio_conflict_detect(HSIGPIO_7 , 2 , func_SN1_XVSXHS);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN_XVSXHS = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(HSIGPIO_7 , 2, func_SN1_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN2_XVSXHS_1) { //PR_FIXG
			confl_detect += pr_gpio_conflict_detect(SGPIO_4 , 2 , func_SN2_XVSXHS);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg3.bit.SN2_XVSXHS = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_4, 2, func_SN2_XVSXHS);
		}
		/* FLASH_TRIGA */
		if (config & PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_1) {
			confl_detect += gpio_conflict_detect(CGPIO_21 , 2 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.FLASH_TRIGA = MUX_1;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_21, 2, func_SENSORMISC);

		} else if (config & PIN_SENSORMISC_CFG_FLASH_TRIGA_IN_2) {

			confl_detect += gpio_conflict_detect(PGPIO_11 , 2 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg1.bit.FLASH_TRIGA = MUX_2;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_11, 2, func_SENSORMISC);

		}

		/* ME_SHUT */
		if (config & PIN_SENSORMISC_CFG_ME_SHUT_IN_1) {

			confl_detect += gpio_conflict_detect(CGPIO_20 , 1 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg10.bit.ME_SHUT_IN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_20, 1, func_SENSORMISC);

		} else if (config & PIN_SENSORMISC_CFG_ME_SHUT_IN_2) {

			confl_detect += gpio_conflict_detect(PGPIO_4 , 1 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg10.bit.ME_SHUT_IN = MUX_2;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 1, func_SENSORMISC);

		}

		if (config & PIN_SENSORMISC_CFG_ME_SHUT_OUT_1) {

			confl_detect += gpio_conflict_detect(CGPIO_23 , 2 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg10.bit.ME_SHUT_OUT = MUX_1;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_23, 2, func_SENSORMISC);

		} else if (config & PIN_SENSORMISC_CFG_ME_SHUT_OUT_2) {

			confl_detect += gpio_conflict_detect(PGPIO_5 , 2 , func_SENSORMISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg10.bit.ME_SHUT_OUT = MUX_2;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_5, 2, func_SENSORMISC);

		}
	}

	return E_OK;
}

static int pinmux_config_sensorsync(uint32_t config)
{
	if (config == PIN_SENSORSYNC_CFG_NONE) {
	} else {
		/* MCLK source sync */
		if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK2) {
			top_reg3.bit.SN2_MCLK_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SIEMCLK) {
			top_reg3.bit.SN2_MCLK_SRC = MUX_1;
		}

		/* XVSXHS source sync */
		if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN2) {
			top_reg3.bit.SN2_XVSHS_SRC = MUX_0;
		} else if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN1) {
			top_reg3.bit.SN2_XVSHS_SRC = MUX_1;
		}
	}

	return E_OK;
}

static int pinmux_config_audio(uint32_t config)
{
	if (config & PIN_AUDIO_CFG_NONE) {
	} else {
		/* I2S */
		if (config & PIN_AUDIO_CFG_I2S_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9, 4, func_I2S_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 4, func_I2S_1);

		} else if (config & PIN_AUDIO_CFG_I2S_2) {

			confl_detect += gpio_conflict_detect(CGPIO_14, 4, func_I2S_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S = MUX_2;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_14, 4, func_I2S_2);

		} else if (config & PIN_AUDIO_CFG_I2S_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_26, 4, func_I2S_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S = MUX_3;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_26, 4, func_I2S_3);
		}

		/* I2S_MCLK */
		if (config & PIN_AUDIO_CFG_I2S_MCLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_I2S_1_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_I2S_1_MCLK);

		} else if (config & PIN_AUDIO_CFG_I2S_MCLK_2) {

			confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_I2S_2_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S_MCLK = MUX_2;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 1, func_I2S_2_MCLK);

		} else if (config & PIN_AUDIO_CFG_I2S_MCLK_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_25, 1, func_I2S_3_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.I2S_MCLK = MUX_3;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_25, 1, func_I2S_3_MCLK);

		}

		/* DMIC_1 */
		if (config & PIN_AUDIO_CFG_DMIC_1) {

			confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.DMIC = MUX_1;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(PGPIO_17, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA0 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_17, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA1 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_18, 1, func_AUDIO_DMIC);
			}
		/* DMIC_2 */
		} else if (config & PIN_AUDIO_CFG_DMIC_2) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.DMIC = MUX_2;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_10, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(PGPIO_9, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA0 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_9, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(PGPIO_8, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA1 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_8, 1, func_AUDIO_DMIC);
			}
		/* DMIC_3 */
		} else if (config & PIN_AUDIO_CFG_DMIC_3) {

			confl_detect += gpio_conflict_detect(CGPIO_13, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.DMIC = MUX_3;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(CGPIO_14, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA0 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_14, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(CGPIO_15, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA1 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_15, 1, func_AUDIO_DMIC);
			}
		//DMIC_4
		} else if (config & PIN_AUDIO_CFG_DMIC_4) {

			confl_detect += gpio_conflict_detect(CGPIO_22, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.DMIC = MUX_4;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_22, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += gpio_conflict_detect(CGPIO_23, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA0 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_23, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += gpio_conflict_detect(CGPIO_24, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA1 = MUX_1;
				top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_24, 1, func_AUDIO_DMIC);
			}
		//PR_DMIC
		} else if (config & PIN_AUDIO_CFG_PR_DMIC_1) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_6, 1, func_AUDIO_DMIC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.DMIC = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_6, 1, func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {

				confl_detect += pr_gpio_conflict_detect(SGPIO_5, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA0 = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_5, 1, func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {

				confl_detect += pr_gpio_conflict_detect(SGPIO_4, 1, func_AUDIO_DMIC);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg9.bit.DMIC_DATA1 = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_4, 1, func_AUDIO_DMIC);
			}
		}

		if (config & PIN_AUDIO_CFG_EXT_EAC_MCLK) {

			confl_detect += gpio_conflict_detect(CGPIO_19, 1, func_AUDIO_EXT_MCLK);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg9.bit.EXT_EAC_MCLK = MUX_1;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_19, 1, func_AUDIO_EXT_MCLK);
		}
	}

	return E_OK;
}

static int pinmux_config_uart(uint32_t config)
{
	//printk("pinmux_config_uart = 0x%08x\r\n", config);

	if (config == PIN_UART_CFG_NONE) {
	} else {
		//UART_1
		if (config & PIN_UART_CFG_UART_1) {

			confl_detect += gpio_conflict_detect(PGPIO_23, 2, func_UART);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART = MUX_1;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_23, 2, func_UART);
		}

		//UART2_1
		if (config & PIN_UART_CFG_UART2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 2, func_UART2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 2, func_UART2_1);

			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_19, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_19, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_19, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_19, 1, func_UART2_DTROE);
			}
		//UART2_2
		} else if (config & PIN_UART_CFG_UART2_2) {

			confl_detect += gpio_conflict_detect(CGPIO_17, 2, func_UART2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 2, func_UART2_2);
			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += gpio_conflict_detect(CGPIO_15, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_15, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += gpio_conflict_detect(CGPIO_16, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_16, 1, func_UART2_DTROE);
			}
		//UART2_3
		} else if (config & PIN_UART_CFG_UART2_3) {
			confl_detect += gpio_conflict_detect(PGPIO_0, 2, func_UART2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART2 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 2, func_UART2_3);

			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_2, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_2, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_2, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_2, 1, func_UART2_DTROE);
			}
		//UART2_4
		} else if (config & PIN_UART_CFG_UART2_4) {
			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 2, func_UART2_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART2 = MUX_4;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_9, 2, func_UART2_4);

			if (config & PIN_UART_CFG_UART2_RTSCTS) {

				confl_detect += pr_gpio_conflict_detect(SGPIO_11, 2, func_UART2_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_11, 2, func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {

				confl_detect += pr_gpio_conflict_detect(SGPIO_11, 1, func_UART2_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART2_RTSCTS = MUX_2;
				pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_11, 1, func_UART2_DTROE);
			}
		}

		//UART3_1
		if (config & PIN_UART_CFG_UART3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_8, 2, func_UART3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 2, func_UART3_1);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_10, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_10, 2, func_UART3_CTSRTS);

			} else if (config & PIN_UART_CFG_UART3_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_10, 1, func_UART3_DTROE);
			}
		//UART3_2
		} else if (config & PIN_UART_CFG_UART3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_23, 2, func_UART3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_23, 2, func_UART3_2);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += gpio_conflict_detect(CGPIO_21, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_21, 2, func_UART3_CTSRTS);

			} else if (config & PIN_UART_CFG_UART3_DIROE) {

				confl_detect += gpio_conflict_detect(CGPIO_22, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_22, 1, func_UART3_DTROE);
			}
		//UART3_3
		} else if (config & PIN_UART_CFG_UART3_3) {
			confl_detect += gpio_conflict_detect(PGPIO_4, 2, func_UART3_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 2, func_UART3_3);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += gpio_conflict_detect(PGPIO_6, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_6, 2, func_UART3_CTSRTS);

			} else if (config & PIN_UART_CFG_UART3_DIROE) {

				confl_detect += gpio_conflict_detect(PGPIO_6, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_6, 1, func_UART3_DTROE);
			}
		//UART3_4
		} else if (config & PIN_UART_CFG_UART3_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_0, 2, func_UART3_4);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_4;
			pr_top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_0, 2, func_UART3_4);
		//UART3_5
		} else if (config & PIN_UART_CFG_UART3_5) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_4, 2, func_UART3_5);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_5;
			pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_4, 2, func_UART3_5);

		//UART3_6
		} else if (config & PIN_UART_CFG_UART3_6) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_13, 2, func_UART3_6);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg4.bit.UART3 = MUX_6;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_13, 2, func_UART3_6);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {

				confl_detect += pr_gpio_conflict_detect(SGPIO_15, 2, func_UART3_CTSRTS);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_top_reg_sgpio1.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_15, 2, func_UART3_CTSRTS);

			} else if (config & PIN_UART_CFG_UART3_DIROE) { //PR_FIXG

				confl_detect += pr_gpio_conflict_detect(SGPIO_15, 1, func_UART3_DTROE);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg4.bit.UART3_RTSCTS = MUX_2;
				pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_15, 1, func_UART3_DTROE);
			}
		}

	}

	return E_OK;
}

static int pinmux_config_csi(uint32_t config)
{
	if (config == PIN_CSI_CFG_NONE) {
	} else {
		if (config & PIN_CSI_CFG_CSI_1C4D) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 10, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_5;

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

			pad_set_pull_updown(PAD_PIN_HSIGPIO0, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO1, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO2, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO3, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO4, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO5, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO7, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO8, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);

			gpio_func_keep(HSIGPIO_0, 10, func_SENSOR);

		} else if (config & PIN_CSI_CFG_CSICSI2_1C2D) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 12, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_6;

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

			pad_set_pull_updown(PAD_PIN_HSIGPIO0, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO1, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO2, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO3, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO4, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO5, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO7, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO8, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO11, PAD_NONE);

			gpio_func_keep(HSIGPIO_0, 12, func_SENSOR);
		} else if (config & PIN_CSI_CFG_CSI_1C2D) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 6, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_7;

			top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;

			pad_set_pull_updown(PAD_PIN_HSIGPIO0, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO1, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO2, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO3, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO4, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO5, PAD_NONE);

			gpio_func_keep(HSIGPIO_0, 6, func_SENSOR);
		} else if (config & PIN_CSI_CFG_CSI2_1C2D) {

			confl_detect += gpio_conflict_detect(HSIGPIO_0, 6, func_SENSOR);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SENSOR = MUX_8;

			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;

			pad_set_pull_updown(PAD_PIN_HSIGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO7, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO8, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO11, PAD_NONE);

			gpio_func_keep(HSIGPIO_0, 6, func_SENSOR);
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

			top_reg10.bit.REMOTE = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 1, func_Remote);
		} else if (config & PIN_REMOTE_CFG_REMOTE_2) {

			confl_detect += gpio_conflict_detect(CGPIO_16, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.REMOTE = MUX_2;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_16, 1, func_Remote);
		}

		if (config & PIN_REMOTE_CFG_REMOTE_EXT_1) {

			confl_detect += gpio_conflict_detect(PGPIO_18, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.REMOTE_EXT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_18, 1, func_Remote);

		} else if (config & PIN_REMOTE_CFG_REMOTE_EXT_2) {
			confl_detect += gpio_conflict_detect(CGPIO_17, 1, func_Remote);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg10.bit.REMOTE_EXT = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 1, func_Remote);
		}
	}

	return E_OK;
}

static int pinmux_config_sdp(uint32_t config)
{
	if (config == PIN_SDP_CFG_NONE) {
	} else {
		if (config & PIN_SDP_CFG_SDP_1) {	

			confl_detect += gpio_conflict_detect(CGPIO_13 , 5 , func_SDP_1);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SDP = MUX_1;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 5, func_SDP_1);
		} else if (config & PIN_SDP_CFG_SDP_2) {

			confl_detect += gpio_conflict_detect(PGPIO_4 , 5 , func_SDP_2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SDP = MUX_2;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 5, func_SDP_2);
		} else if (config & PIN_SDP_CFG_SDP_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_28 , 5 , func_SDP_3);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SDP = MUX_3;
			pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_28, 5, func_SDP_3);
		}
	}

	return E_OK;
}

static int pinmux_config_spi(uint32_t config)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & PIN_SPI_CFG_SPI_1) {

			confl_detect += gpio_conflict_detect(CGPIO_19 , 3 , func_SPI_1);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI = MUX_1;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_19, 3, func_SPI_1);

			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_22 , 1 , func_SPI_1);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_22, 1, func_SPI_1);
			}
		} else if (config & PIN_SPI_CFG_SPI_2) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_6 , 3 , func_SPI_2);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_6, 3, func_SPI_2);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {
				confl_detect += pr_gpio_conflict_detect(SGPIO_5 , 1 , func_SPI_2);
				if (confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg7.bit.SPI_BUS_WIDTH = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_5, 1, func_SPI_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_9 , 3 , func_SPI2_1);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_9, 3, func_SPI2_1);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_12 , 1 , func_SPI2_1);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_12, 1, func_SPI2_1);
			}
		} else if (config & PIN_SPI_CFG_SPI2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_0 , 3 , func_SPI2_2);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 3, func_SPI2_2);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_3 , 1 , func_SPI2_2);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_3, 1, func_SPI2_2);
			}
		} else if (config & PIN_SPI_CFG_SPI2_3) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_9 , 3 , func_SPI2_3);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI2 = MUX_3;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_9, 3, func_SPI2_3);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {
				confl_detect += pr_gpio_conflict_detect(SGPIO_12 , 1 , func_SPI2_3);
				if (confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg7.bit.SPI2_BUS_WIDTH = MUX_1;
				pr_top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_12, 1, func_SPI2_3);
			}
		} else if (config & PIN_SPI_CFG_SPI2_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_13 , 3 , func_SPI2_4);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI2 = MUX_4;
			pr_top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_13, 3, func_SPI2_4);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {
				confl_detect += pr_gpio_conflict_detect(SGPIO_16 , 1 , func_SPI2_4);
				if (confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg7.bit.SPI2_BUS_WIDTH = MUX_1;
				pr_top_reg_sgpio1.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(SGPIO_16, 1, func_SPI2_4);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_17 , 3 , func_SPI3_1);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 3, func_SPI3_1);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_20 , 1 , func_SPI3_1);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_20, 1, func_SPI3_1);
			}
		} else if (config & PIN_SPI_CFG_SPI3_2) {

			confl_detect += gpio_conflict_detect(CGPIO_13 , 3 , func_SPI3_2);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_13, 3, func_SPI3_2);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(CGPIO_16 , 1 , func_SPI3_2);
				if (confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg7.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(CGPIO_16, 1, func_SPI3_2);
			}
		} else if (config & PIN_SPI_CFG_SPI3_3) {
			confl_detect += gpio_conflict_detect(PGPIO_4 , 3 , func_SPI3_3);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_4, 3, func_SPI3_3);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += gpio_conflict_detect(PGPIO_7 , 1 , func_SPI3_3);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_7, 1, func_SPI3_3);
			}
		} else if (config & PIN_SPI_CFG_SPI3_4) { //PR_FIXG
			confl_detect += pr_gpio_conflict_detect(PGPIO_25 , 3 , func_SPI3_4);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_25, 3, func_SPI3_4);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {

				confl_detect += pr_gpio_conflict_detect(PGPIO_28 , 1 , func_SPI3_4);
				if (confl_detect > 0)
				{
					return E_PAR;
				}

				top_reg7.bit.SPI3_BUS_WIDTH = MUX_1;
				pr_top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC_MAIN;
				pr_gpio_func_keep(PGPIO_28, 1, func_SPI3_4);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_RDY_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21 , 1 , func_SPI3_RDY);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3_RDY = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_SPI3_RDY);

		} else if (config & PIN_SPI_CFG_SPI3_RDY_2) {

			confl_detect += gpio_conflict_detect(CGPIO_17 , 1 , func_SPI3_RDY2);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3_RDY = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_17, 1, func_SPI3_RDY2);
		} else if (config & PIN_SPI_CFG_SPI3_RDY_3) {

			confl_detect += gpio_conflict_detect(PGPIO_8 , 1 , func_SPI3_RDY3);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3_RDY = MUX_3;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_8, 1, func_SPI3_RDY3);

		} else if (config & PIN_SPI_CFG_SPI3_RDY_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_29 , 1 , func_SPI3_RDY4);
			if (confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg7.bit.SPI3_RDY = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_29, 1, func_SPI3_RDY4);

		}
	}

	return E_OK;
}

static int pinmux_config_sif(uint32_t config)
{
	if (config == PIN_SIF_CFG_NONE) {
	} else {
		if (config & PIN_SIF_CFG_SIF0_1) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_6, 3, func_SIF_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF0 = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_6, 3, func_SIF_1);
		}

		if (config & PIN_SIF_CFG_SIF1_1) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(SGPIO_3, 3, func_SIF1_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF1 = MUX_1;
			pr_top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_3, 3, func_SIF1_1);
		}

		if (config & PIN_SIF_CFG_SIF2_1) {

			confl_detect += gpio_conflict_detect(PGPIO_10, 3, func_SIF2_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_10, 3, func_SIF2_1);
		} else if (config & PIN_SIF_CFG_SIF2_2) {

			confl_detect += gpio_conflict_detect(PGPIO_17, 3, func_SIF2_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF2 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_17, 3, func_SIF2_2);
		} else if (config & PIN_SIF_CFG_SIF2_3) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 3, func_SIF2_3);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF2 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_3, 3, func_SIF2_3);
		} else if (config & PIN_SIF_CFG_SIF2_4) { //PR_FIXG

			confl_detect += pr_gpio_conflict_detect(PGPIO_25, 3, func_SIF2_4);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF2 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_25, 3, func_SIF2_4);
		}

		if (config & PIN_SIF_CFG_SIF3_1) {

			confl_detect += gpio_conflict_detect(PGPIO_7, 3, func_SIF3_1);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_7, 3, func_SIF3_1);
		} else if (config & PIN_SIF_CFG_SIF3_2) { //PR_FIXG
			confl_detect += pr_gpio_conflict_detect(SGPIO_9, 3, func_SIF3_2);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg7.bit.SIF3 = MUX_2;
			pr_top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(SGPIO_9, 3, func_SIF3_2);
		}
	}

	return E_OK;
}

static int pinmux_config_misc(uint32_t config)
{
	if (config == PIN_MISC_CFG_NONE) {
	} else {
		if (config & PIN_MISC_CFG_RTC_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_19 , 1 , func_MISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg10.bit.RTC_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_19, 1, func_MISC);
		}

		/* SP_CLK */
		if (config & PIN_MISC_CFG_SP_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_20, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_20, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP_CLK_2) {

			confl_detect += gpio_conflict_detect(CGPIO_24, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK = MUX_2;
			top_reg_cgpio0.bit.CGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_24, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP_CLK_3) {

			confl_detect += gpio_conflict_detect(DGPIO_3, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK = MUX_3;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_3, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP_CLK_4) {
			confl_detect += gpio_conflict_detect(CGPIO_22, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK = MUX_4;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_22, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP_CLK_5) {
			confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK = MUX_5;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_12, 1, func_MISC);
		}

		if (config & PIN_MISC_CFG_SP2_CLK_1) {

			confl_detect += gpio_conflict_detect(PGPIO_21, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_21, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP2_CLK_2) {

			confl_detect += gpio_conflict_detect(CGPIO_11, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK2 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(CGPIO_11, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP2_CLK_3) {

			confl_detect += gpio_conflict_detect(DGPIO_4, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK2 = MUX_3;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_4, 1, func_MISC);

		} else if (config & PIN_MISC_CFG_SP2_CLK_4) { //PR_FIXG
			confl_detect += pr_gpio_conflict_detect(PGPIO_30, 1, func_MISC);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg3.bit.SP_CLK2 = MUX_4;
			pr_top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC_MAIN;
			pr_gpio_func_keep(PGPIO_30, 1, func_MISC);

		}

		if (config & PIN_MISC_CFG_CPU_ICE) {
			confl_detect += gpio_conflict_detect(DGPIO_2 , 5 , func_MISC);
			if(confl_detect > 0)
			{
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

		/*
		if (config & PIN_MISC_CFG_MIPI_MODE_SEL_1C4D) {

			confl_detect += gpio_conflict_detect(DSIGPIO_0 , 10 , func_MISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg2.bit.MIPI_MODE_SEL = MUX_0;
			top_reg_dsigpio0.bit.DSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DSIGPIO_0, 10, func_MISC);

			if (config & PIN_MISC_CFG_MIPI_CK0_SEL_DSI) {

				top_reg2.bit.MIPI_CK0_SEL = MUX_0;
			} else if (config & PIN_MISC_CFG_MIPI_CK0_SEL_CSI_TX) {

				top_reg2.bit.MIPI_CK0_SEL = MUX_1;
			}
		} else if (config & PIN_MISC_CFG_MIPI_MODE_SEL_1C2D) {

			confl_detect += gpio_conflict_detect(DSIGPIO_0 , 12 , func_MISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg2.bit.MIPI_MODE_SEL = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DSIGPIO_0, 12, func_MISC);

			if (config & PIN_MISC_CFG_MIPI_CK0_SEL_DSI) {

				top_reg2.bit.MIPI_CK0_SEL = MUX_0;
			} else if (config & PIN_MISC_CFG_MIPI_CK0_SEL_CSI_TX) {

				top_reg2.bit.MIPI_CK0_SEL = MUX_1;
			}

			if (config & PIN_MISC_CFG_MIPI_CK1_SEL_DSI) {

				top_reg2.bit.MIPI_CK1_SEL = MUX_0;
			} else if (config & PIN_MISC_CFG_MIPI_CK1_SEL_CSI_TX) {

				top_reg2.bit.MIPI_CK1_SEL = MUX_1;
			}
		}

		if (config & PIN_MISC_CFG_CPU_ICE) {
			confl_detect += gpio_conflict_detect(DGPIO_10 , 5 , func_MISC);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg1.bit.EJTAG_EN = MUX_1;
			top_reg_dgpio0.bit.DGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DGPIO_10, 5, func_MISC);
		}
		*/
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

#if 0
static int pinmux_config_lcd2(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD2] = config;

	return E_OK;
}

static int pinmux_config_lcd3(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD3] = config;

	return E_OK;
}
#endif

static int pinmux_config_tv(uint32_t config)
{
#if 0
	uint32_t tmp;

	tmp = config & PINMUX_TV_HDMI_CFG_MASK;
	if ((tmp != PINMUX_TV_HDMI_CFG_NORMAL) && (tmp != PINMUX_TV_HDMI_CFG_PINMUX_ON)) {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_TV] = config;
#endif
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

#if 1
static int pinmux_select_primary_lcd(uint32_t config)
{
	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 8, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 8, func_LCD);
			gpio_func_keep(PGPIO_10, 1, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR656_16BITS) {
			confl_detect += gpio_conflict_detect(PGPIO_0, 8, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_10, 1, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_13, 8, func_LCD);

			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;

			gpio_func_keep(PGPIO_0, 8, func_LCD);
			gpio_func_keep(PGPIO_10, 1, func_LCD);
			gpio_func_keep(PGPIO_13, 8, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
			if (config & PINMUX_LCD_SEL_HVLD_VVLD) {

				confl_detect += gpio_conflict_detect(PGPIO_13, 2, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_HVLD_VVLD = MUX_1;
				top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_13, 2, func_LCD);
			}

			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_12, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601_16BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_13, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
			gpio_func_keep(PGPIO_13, 8, func_LCD);

			if (config & PINMUX_LCD_SEL_FIELD) {

				confl_detect += gpio_conflict_detect(PGPIO_12, 1, func_LCD);
				if (confl_detect > 0) {
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(PGPIO_12, 1, func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_13, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_3;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
			gpio_func_keep(PGPIO_13, 8, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_8BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_4;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_2, 9, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_5;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_2, 9, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_6;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_RGB_16BITS) {

			confl_detect += gpio_conflict_detect(PGPIO_0, 11, func_LCD);
			confl_detect += gpio_conflict_detect(PGPIO_13, 8, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_7;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_0, 11, func_LCD);
			gpio_func_keep(PGPIO_13, 8, func_LCD);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {

			confl_detect += gpio_conflict_detect(PGPIO_11, 1, func_LCD);
			if (confl_detect > 0) {
				return E_PAR;
			}

			top_reg2.bit.PLCD_DE = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(PGPIO_11, 1, func_LCD);
		}
	}

	return E_OK;
}
#if 0
static int pinmux_select_secondary_lcd(uint32_t config)
{

	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		/*if (pinmux_type != PINMUX_LCD_SEL_MIPI) {
		    if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
		        top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with LCD_TYPE\r\n");
		        return E_PAR;
		    }
		    if (top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with CCIR_DATA_WIDTH\r\n");
		        return E_PAR;
		    }
		    if (top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with CCIR_HVLD_VVLD\r\n");
		        return E_PAR;
		    }
		    if (top_reg12.bit.I2S == MUX_1) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with I2S_1\r\n");
		        return E_PAR;
		    }
		    if (top_reg2.bit.MEMIF_TYPE == MUX_2 &&
		        top_reg2.bit.MEMIF_SEL == MUX_0 &&
		        (top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
		        return E_PAR;
		    }
		    if (top_reg16.bit.SPI5 == MUX_2) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with SPI5_2\r\n");
		        return E_PAR;
		    }
		    if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with ETH\r\n");
		        return E_PAR;
		    }
		    if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
		        pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with ETH2\r\n");
		        return E_PAR;
		    }
		}*/

		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {

			confl_detect += gpio_conflict_detect(LGPIO_13 , 9 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.LCD2_TYPE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_13, 9, func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {
			/*if (top_reg17.bit.SIF0 == MUX_2) {
			    pr_err("(LCD2_TYPE = 0x2) conflict with SIF0_2\r\n");
			    return E_PAR;
			}*/

			confl_detect += gpio_conflict_detect(LGPIO_13 , 11 , func_LCD2);
			confl_detect += gpio_conflict_detect(LGPIO_25 , 1 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}

			top_reg2.bit.LCD2_TYPE = MUX_2;
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
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_13, 11, func_LCD2);
			gpio_func_keep(LGPIO_25, 1, func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_8BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_13 , 11 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.LCD2_TYPE = MUX_4;
			lcd2_rgb_6bit_flag = 0;
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
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_13, 11, func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_15 , 9 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.LCD2_TYPE = MUX_5;
			lcd2_rgb_6bit_flag = 1;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_15, 9, func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS) {

			confl_detect += gpio_conflict_detect(LGPIO_13 , 11 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.LCD2_TYPE = MUX_6;
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
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_13, 11, func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_MIPI) {
			top_reg2.bit.LCD_TYPE = MUX_9;
		}

		if (config & PINMUX_LCD_SEL_TE_ENABLE) {

			confl_detect += gpio_conflict_detect(DSIGPIO_11 , 1 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.TE_SEL = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DSIGPIO_11, 1, func_LCD2);
		}

		if (config & PINMUX_LCD_SEL_TE2_ENABLE) {

			confl_detect += gpio_conflict_detect(DSIGPIO_13 , 1 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.TE2_SEL = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(DSIGPIO_13, 1, func_LCD);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {

			confl_detect += gpio_conflict_detect(LGPIO_24 , 1 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.PLCD2_DE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_24, 1, func_LCD2);
		}
	} else if (pinmux_type <= PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {  // mi type
		/*if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
		    top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
		    pr_err("(MEMIF_SEL = 1) conflict with LCD_TYPE\r\n");
		    return E_PAR;
		}
		if (top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
		    pr_err("(MEMIF_SEL = 1) conflict with CCIR_DATA_WIDTH\r\n");
		    return E_PAR;
		}
		if (top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
		    pr_err("(MEMIF_SEL = 1) conflict with CCIR_HVLD_VVLD\r\n");
		    return E_PAR;
		}
		if (top_reg2.bit.CCIR_FIELD == MUX_1) {
		    pr_err("(MEMIF_SEL = 1) conflict with CCIR_FIELD\r\n");
		    return E_PAR;
		}
		if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
		    pr_err("(MEMIF_SEL = 1) conflict with ETH2\r\n");
		    return E_PAR;
		}*/

		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO) {

			confl_detect += gpio_conflict_detect(LGPIO_14 , 4 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_14, 4, func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_18 , 1 , func_LCD2);
				if(confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_18, 1, func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {

			confl_detect += gpio_conflict_detect(LGPIO_12 , 2 , func_LCD2);
			confl_detect += gpio_conflict_detect(LGPIO_15 , 3 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_12, 2, func_LCD2);
			gpio_func_keep(LGPIO_15, 3, func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_18 , 1 , func_LCD2);
				if(confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_18, 1, func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_8BITS) {
			/*if (top_reg12.bit.I2S2 == MUX_1) {
			    pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 0) conflict with I2S2_1\r\n");
			    return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
			    pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 0) conflict with ETH\r\n");
			    return E_PAR;
			}*/

			confl_detect += gpio_conflict_detect(LGPIO_12 , 8 , func_LCD2);
			confl_detect += gpio_conflict_detect(LGPIO_21 , 4 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_12, 8, func_LCD2);
			gpio_func_keep(LGPIO_21, 4, func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {

				confl_detect += gpio_conflict_detect(LGPIO_25 , 1 , func_LCD2);
				if(confl_detect > 0)
				{
					return E_PAR;
				}
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_25, 1, func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_9BITS) {
			/*if (top_reg12.bit.I2S2 == MUX_1) {
			    pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 1) conflict with I2S2_1\r\n");
			    return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
			    pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 1) conflict with ETH\r\n");
			    return E_PAR;
			}*/

			confl_detect += gpio_conflict_detect(LGPIO_12 , 13 , func_LCD2);
			if(confl_detect > 0)
			{
				return E_PAR;
			}
			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			gpio_func_keep(LGPIO_12, 13, func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
				gpio_func_keep(LGPIO_25, 1, func_LCD2);
			}
		}
	}

	return E_OK;
}
#endif

#endif //LCD_MARK_FIRST
ER pinmux_parsing_i2c(uint32_t config)
{
	PAD_PULL pad_pull;
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		//< I2C_1   (P_GPIO[21..22])
		//< I2C_2   (C_GPIO[13..14])
		//< I2C_3   (P_GPIO[9..10])
		//< I2C_4   (P_GPIO[11..12])
		//< I2C_5   (S_GPIO[15..16])
		if (config & (PIN_I2C_CFG_I2C_1 | PIN_I2C_CFG_I2C_2 | PIN_I2C_CFG_I2C_3 | PIN_I2C_CFG_I2C_4 | PIN_I2C_CFG_I2C_5)) {
			if (config & PIN_I2C_CFG_I2C_1) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO21, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && PGPIO21 pull up => force PGPIO21 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO21 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO22, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && PGPIO22 pull up => force PGPIO22 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO22 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C_2) {
				if (pad_get_pull_updown(PAD_PIN_CGPIO13, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 2ND FUNC_EN && CGPIO13 pull up => force CGPIO13 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO13 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO13, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO14, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 2ND FUNC_EN && CGPIO14 pull up => force CGPIO14 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO14, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO14 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_CGPIO14, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C_3) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO9, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && PGPIO9 pull up => force PGPIO9 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO9 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO9, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO10, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && PGPIO10 pull up => force PGPIO10 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO10 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO10, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C_4) {
				if (pad_get_pull_updown(PAD_PIN_PGPIO11, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && PGPIO11 pull up => force PGPIO11 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO11 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO11, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO12, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && PGPIO12 pull up => force PGPIO12 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO12 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C_5) { //PR_FIXG
				if (pad_get_pull_updown(PAD_PIN_SGPIO15, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("###I2C 1st FUNC_EN && SGPIO15 pull up => force SGPIO15 pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO15 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO16, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C 1st FUNC_EN && SGPIO16 pull up => force SGPIO16 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO16 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
				}
			}
		}
		//< I2C2_1  (C_GPIO[19..20])
		//< I2C2_2  (P_GPIO[31..32])
		//< I2C2_3  (S_GPIO[4..5])
		//< I2C2_4  (HSI_GPIO[9..10])
		if (config & (PIN_I2C_CFG_I2C2_1 | PIN_I2C_CFG_I2C2_2 | PIN_I2C_CFG_I2C2_3 | PIN_I2C_CFG_I2C2_4)) {
			if (config & PIN_I2C_CFG_I2C2_1) {
				if (pad_get_pull_updown(PAD_PIN_CGPIO19, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && CGPIO19 pull up => force CGPIO20 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO19 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_CGPIO19, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_CGPIO20, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 1st FUNC_EN && CGPIO20 pull up => force CGPIO20 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_CGPIO20, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_CGPIO20 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_CGPIO20, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C2_2) { //PR_FIXG
				if (pad_get_pull_updown(PAD_PIN_PGPIO31, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO31 pull up => force PGPIO31 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO31 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_PGPIO32, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && PGPIO32 pull up => force PGPIO32 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_PGPIO32, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_PGPIO32 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_PGPIO32, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C2_3) { //PR_FIXG
				if (pad_get_pull_updown(PAD_PIN_SGPIO4, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && SGPIO4 pull up => force SGPIO4 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO4 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_SGPIO4, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_SGPIO5, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && SGPIO5 pull up => force SGPIO5 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_SGPIO5 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_SGPIO5, PAD_NONE);
				}
			} else if (config & PIN_I2C_CFG_I2C2_4) {
				if (pad_get_pull_updown(PAD_PIN_HSIGPIO9, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && HSIGPIO9 pull up => force HSIGPIO9 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_HSIGPIO9 Fail, force pull none\n");
					pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
				}

				if (pad_get_pull_updown(PAD_PIN_HSIGPIO10, &pad_pull) == E_OK) {
					if (pad_pull != PAD_NONE) {
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pr_err("!!!I2C2 2nd FUNC_EN && HSIGPIO10 pull up => force HSIGPIO10 = pull none!!!\n");
						pr_err("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
					}
				} else {
					pr_err("Get PAD_PIN_HSIGPIO10 Fail, force pull none\r\n");
					pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
				}
			}
		}
	}
	return E_OK;
}

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
	pinmux_config_sensormisc,
	pinmux_config_sensorsync,
	pinmux_config_audio,
	pinmux_config_uart,
	pinmux_config_csi,
	pinmux_config_remote,
	pinmux_config_sdp,
	pinmux_config_spi,
	pinmux_config_sif,
	pinmux_config_misc,
	pinmux_config_lcd,
	//pinmux_config_lcd2,
	//pinmux_config_lcd3,
	pinmux_config_tv,
	//pinmux_config_lcd_pinout_sel,
	pinmux_select_primary_lcd,
	//pinmux_select_secondary_lcd,
	pinmux_config_pr_i2c,
	pinmux_config_pr_uart,
	pinmux_config_pr_sensor,
	pinmux_config_pr_sensormisc,
	pinmux_config_pr_spi,
	pinmux_config_pr_pwm,
	pinmux_config_pr_audio,
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
	int logoboot = logo_determination(&lcd_type);
	static int boot_hint = 1;

	//pr_info("%s, %s\r\n", __func__, DRV_VERSION);

	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);

#if 0
	/* Disable MIPI TX bypass mode when DSI_PROT_EN BST[7] is 1 */
	if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
		void __iomem *dsiphy_reg_addr = NULL;
		uint32_t val;

		#if 0 //no DSI
		dsiphy_reg_addr = ioremap(NVT_DSIPHY_BASE_PHYS, 0x1000);
		if (dsiphy_reg_addr) {
			val = readl(dsiphy_reg_addr + 0x0c);
			val |= (0x1 << 1);
			writel(val, dsiphy_reg_addr + 0x0c);
			iounmap(dsiphy_reg_addr);
		} else {
			pr_err("invalid dsi_reg_addr\n");
			return -ENOMEM;
		}
		#endif
	}
#endif
	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0;
	top_reg2.reg = 0;
	top_reg3.reg = 0;
	top_reg4.reg = 0;
	top_reg5.reg = 0;
	top_reg6.reg = 0;
	top_reg7.reg = 0;
	top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	top_reg9.reg = 0;
	top_reg10.reg = 0;
	top_reg11.reg = 0;

	pr_top_reg0.reg = 0;
	pr_top_reg1.reg = 0;
	pr_top_reg3.reg = 0;
	pr_top_reg5.reg = 0;
	pr_top_reg6.reg = 0;

	top_reg_cgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio1.reg = 0xFFFFFFFF;
	top_reg_dgpio0.reg = 0xFFFFFFFF;
	top_reg_hsigpio0.reg = 0xFFFFFFFF;
	/* PR_GPIO */
	pr_top_reg_sgpio0.reg = 0x55555555;
	pr_top_reg_sgpio1.reg = 0x1;
	pr_top_reg_pgpio0.reg = 0x5555555;
	pr_top_reg_agpio0.reg = 0x55;

	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_dgpio0.reg = 0x83;
	} else {
		top_reg_dgpio0.reg = 0xFF;
	}
	
	if (boot_hint && logoboot) {
		top_reg_pgpio0.reg = TOP_GETREG(info, TOP_REGPGPIO0_OFS);
	}
	
	//clean dump table
	for (i = 0; i < GPIO_total; i++) {
		dump_gpio_func[i] = 0;
	}

	for (i = 0; i < PR_GPIO_total; i++) {
		pr_dump_gpio_func[i] = 0;
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

	/* TOP */
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
	/* PR_TOP */
	PR_TOP_SETREG(info, PR_TOP_REG0_OFS, pr_top_reg0.reg);
	PR_TOP_SETREG(info, PR_TOP_REG1_OFS, pr_top_reg1.reg);
	PR_TOP_SETREG(info, PR_TOP_REG3_OFS, pr_top_reg3.reg);
	PR_TOP_SETREG(info, PR_TOP_REG5_OFS, pr_top_reg5.reg);
	PR_TOP_SETREG(info, PR_TOP_REG6_OFS, pr_top_reg6.reg);
	/* GPIO */
	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGHSIGPIO0_OFS, top_reg_hsigpio0.reg);
	/* PR_GPIO */
	PR_TOP_SETREG(info, PR_TOP_REGSGPIO0_OFS, pr_top_reg_sgpio0.reg);
	PR_TOP_SETREG(info, PR_TOP_REGSGPIO1_OFS, pr_top_reg_sgpio1.reg);
	PR_TOP_SETREG(info, PR_TOP_REGPGPIO0_OFS, pr_top_reg_pgpio0.reg);
	PR_TOP_SETREG(info, PR_TOP_REGAGPIO0_OFS, pr_top_reg_agpio0.reg);
	/* Leave critical section */
	unl_cpu(flags);
	
    if (boot_hint && logoboot) {
        pr_info("logoboot enable, lcd_type = %08x\r\n",lcd_type);
		//pinmux_set_config(PINMUX_FUNC_ID_LCD, lcd_type);
		boot_hint = 0;
	}	

	return E_OK;
}

void gpio_func_show(void)
{
	int i = 0;

	int func_num;
	//print C_GPIO
	int gpio_count = 0; // CGPIO_0 ~ CGPIO_24
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
	//PR S_GPIO
	gpio_count = 0; // SGPIO_0 ~ S_GPIO16
	for (i = SGPIO_0; i < PR_S_GPIO_all; i++) {
		func_num = pr_dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mPR_S_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mPR_S_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}


		} else {
			if (gpio_count < 10) {
				printk("PR_S_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("PR_S_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	//PR A_GPIO
	gpio_count = 0; // AGPIO_0 ~ AGPIO_3
	for (i = AGPIO_0; i < PR_A_GPIO_all; i++) {
		func_num = pr_dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mPR_A_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mPR_A_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}

		} else {
			if (gpio_count < 10) {
				printk("PR_A_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("PR_A_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}
	//P_GPIO
	gpio_count = 0; // PGPIO_0 ~ PGPIO_24; P_GPIO39
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
		if (gpio_count == 25 ) {
			gpio_count = 39; // enum "GPIO_ALL" index jump
		}
	}

	//PR P_GPIO
	gpio_count = 25; // PGPIO_25 ~ PGPIO_38
	for (i = PGPIO_25; i < PR_P_GPIO_all; i++) {
		func_num = pr_dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mPR_P_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mPR_P_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("PR_P_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("PR_P_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

	//D_GPIO
	gpio_count = 0; //DGPIO_0 ~ DGPIO_7
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

	//HSI_GPIO
	gpio_count = 0; //HSIGPIO_0 ~ HSIGPIO_11
	for (i = HSIGPIO_0; i < HSI_GPIO_all; i++) {
		func_num = dump_gpio_func[i];
		if (func_num) {

			if (gpio_count < 10) {
				printk("\033[0;32mHSI_GPIO%d---------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			} else {
				printk("\033[0;32mHSI_GPIO%d--------------------%s\033[0m\n", gpio_count, dump_func[func_num - 1]);
			}
		} else {
			if (gpio_count < 10) {
				printk("HSI_GPIO%d---------------------GPIO\n", gpio_count);
			} else {
				printk("HSI_GPIO%d--------------------GPIO\n", gpio_count);
			}
		}
		gpio_count++;
	}

}
EXPORT_SYMBOL(gpio_func_show);
