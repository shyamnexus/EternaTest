#ifndef __PINMUX_DRV_H__
#define __PINMUX_DRV_H__

#include <plat/top_reg.h>
#include <plat/hardware.h>
#include <plat/top.h>
#include <plat/nvt-gpio.h>
#include <plat/pad.h>
#include <linux/spinlock.h>
#include <linux/slab.h>


#define DRV_VERSION "1.00.08"



typedef enum
{
	PCTRL_X_CPU_NTRST,
	PCTRL_X_CPU_TDI,
	PCTRL_X_CPU_TMS,
	PCTRL_X_CPU_TCK,
	PCTRL_X_CPU_TDO,
	PCTRL_X_SPI_SCK,
	PCTRL_X_SPI_CSN0,
	PCTRL_X_SPI_TX,

	PCTRL_X_SPI_RX,
	PCTRL_X_SPI_WPN,
	PCTRL_X_SPI_HOLDN,
	PCTRL_X_SPI_CSN1,
	PCTRL_X_SPI_RST,
	PCTRL_X_UART0_SOUT,
	PCTRL_X_UART0_SIN,
	PCTRL_X_UART0_RTS,

	PCTRL_X_UART0_CTS,
	PCTRL_X_UART1_SOUT,
	PCTRL_X_UART1_SIN,
	PCTRL_X_UART2_SOUT,
	PCTRL_X_UART2_SIN,
	PCTRL_X_I2C0_SCL,
	PCTRL_X_I2C0_SDA,
	PCTRL_X_I2C1_SCL,

	PCTRL_X_I2C1_SDA,
	PCTRL_X_I2C2_SCL,
	PCTRL_X_I2C2_SDA,
	PCTRL_X_I2S0_SCLK,
	PCTRL_X_I2S0_FS,
	PCTRL_X_I2S0_RXD,
	PCTRL_X_I2S0_TXD,
	PCTRL_X_I2S1_SCLK,

	PCTRL_X_I2S1_FS,
	PCTRL_X_I2S1_RXD,
	PCTRL_X_I2S1_TXD,
	PCTRL_X_EXT0_CLK,
	PCTRL_X_EXT1_CLK,
	PCTRL_X_RESET_OUT,
	PCTRL_X_PWM0_OUT,
	PCTRL_X_PWM1_OUT,

	PCTRL_X_GPIO_0 = 64,
	PCTRL_X_GPIO_1,
	PCTRL_X_GPIO_2,
	PCTRL_X_GPIO_3,
	PCTRL_X_GPIO_4,
	PCTRL_X_GPIO_5,
	PCTRL_X_GPIO_6,
	PCTRL_X_GPIO_7,

	PCTRL_X_GPIO_8,
	PCTRL_X_GPIO_9,


	PCTRL_X_CAP0_CLK	= 96,
	PCTRL_X_CAP0_DATA0,
	PCTRL_X_CAP0_DATA1,
	PCTRL_X_CAP0_DATA2,
	PCTRL_X_CAP0_DATA3,
	PCTRL_X_CAP0_DATA4,
	PCTRL_X_CAP0_DATA5,
	PCTRL_X_CAP0_DATA6,

	PCTRL_X_CAP0_DATA7,
	PCTRL_X_CAP1_CLK,
	PCTRL_X_CAP1_DATA0,
	PCTRL_X_CAP1_DATA1,
	PCTRL_X_CAP1_DATA2,
	PCTRL_X_CAP1_DATA3,
	PCTRL_X_CAP1_DATA4,
	PCTRL_X_CAP1_DATA5,

	PCTRL_X_CAP1_DATA6,
	PCTRL_X_CAP1_DATA7,
	PCTRL_X_CAP2_CLK,
	PCTRL_X_CAP2_DATA0,
	PCTRL_X_CAP2_DATA1,
	PCTRL_X_CAP2_DATA2,
	PCTRL_X_CAP2_DATA3,
	PCTRL_X_CAP2_DATA4,

	PCTRL_X_CAP2_DATA5,
	PCTRL_X_CAP2_DATA6,
	PCTRL_X_CAP2_DATA7,
	PCTRL_X_CAP3_CLK,
	PCTRL_X_CAP3_DATA0,
	PCTRL_X_CAP3_DATA1,
	PCTRL_X_CAP3_DATA2,
	PCTRL_X_CAP3_DATA3,

	PCTRL_X_CAP3_DATA4,
	PCTRL_X_CAP3_DATA5,
	PCTRL_X_CAP3_DATA6,
	PCTRL_X_CAP3_DATA7,

	PCTRL_X_EPHY0_REFCLK = 160,
	PCTRL_X_EPHY0_RESET,
	PCTRL_X_RGMII0_RX_CLK,
	PCTRL_X_RGMII0_RX_CTL,
	PCTRL_X_RGMII0_RXD0,
	PCTRL_X_RGMII0_RXD1,
	PCTRL_X_RGMII0_RXD2,
	PCTRL_X_RGMII0_RXD3,

	PCTRL_X_RGMII0_TX_CLK,
	PCTRL_X_RGMII0_TX_CTL,
	PCTRL_X_RGMII0_TXD0,
	PCTRL_X_RGMII0_TXD1,
	PCTRL_X_RGMII0_TXD2,
	PCTRL_X_RGMII0_TXD3,
	PCTRL_X_RGMII0_MDC,
	PCTRL_X_RGMII0_MDIO,

	PCTRL_X_VGA_HS = 192,
	PCTRL_X_VGA_VS,
	PCTRL_X_HDMI0_I2C_SCL,
	PCTRL_X_HDMI0_I2C_SDA,
	PCTRL_X_HDMI0_HPD,


	_PCTRL_VCAP_VI0_MUX = 320,
	_PCTRL_VCAP_VI1_MUX,
	_PCTRL_VCAP_VI2_MUX,
	_PCTRL_VCAP_VI3_MUX,

	_PCTRL_ETH_PHY_SEL = 336,

	PCTRL_X_MAX_PIN,
	PCTRL_X_DUMMY_PIN = 63,// this must be assign to non existed pinmux location.
	ENUM_DUMMY4WORD(PCTRL)
} PCTRL;

typedef enum {
	PINMUX_FUNC_ID_LCD2,
	PINMUX_FUNC_ID_COUNT,                           //< Total function count
} PINMUX_FUNC_ID;

#define TOP_SETREG(info, ofs,value)	   OUTW(info->top_base +(ofs),(value))
#define TOP_GETREG(info, ofs)		   INW(info->top_base+(ofs))

#define TGE_REG_ADDR(ofs)       (NVT_TGE_BASE_VIRT+(ofs))
#define TGE_GETREG(ofs)         INW(TGE_REG_ADDR(ofs))
#define TGE_SETREG(ofs,value)   OUTW(TGE_REG_ADDR(ofs), (value))
#define TGE_CONTROL_OFS 0x0

#define MAX_PAD_NUM  236

#define GPIO_SETREG(info, ofs,value)	   OUTW(info->gpio_base +(ofs),(value))
#define GPIO_GETREG(info, ofs)		   INW(info->gpio_base+(ofs))

struct nvt_pad_info {
	unsigned long pad_ds_pin;
	unsigned long driving;
	unsigned long pad_gpio_pin;
	unsigned long direction;
};

struct nvt_gpio_info {
	unsigned long gpio_pin;
	unsigned long direction;
	unsigned long level;
};

struct nvt_power_info {
	unsigned long pad_power_id;
	unsigned long pad_power;
};

struct nvt_pinctrl_info {
	void __iomem *top_base;
	void __iomem *pad_base;
	void __iomem *gpio_base;
	PIN_GROUP_CONFIG top_pinmux[PIN_FUNC_MAX];
	struct nvt_pad_info pad[MAX_PAD_NUM];
	u32 ep_ch;
};

typedef enum
{
	CGPIO_0,
	CGPIO_1,
	CGPIO_2,
	CGPIO_3,
	CGPIO_4,
	CGPIO_5,
	CGPIO_6,
	CGPIO_7,
	C_GPIO_all,
	JGPIO_0,
	JGPIO_1,
	JGPIO_2,
	JGPIO_3,
	JGPIO_4,
	J_GPIO_all,
	PGPIO_0,
	PGPIO_1,
	PGPIO_2,
	PGPIO_3,
	PGPIO_4,
	PGPIO_5,
	PGPIO_6,
	PGPIO_7,
	PGPIO_8,
	PGPIO_9,
	PGPIO_10,
	PGPIO_11,
	PGPIO_12,
	PGPIO_13,
	PGPIO_14,
	PGPIO_15,
	PGPIO_16,
	PGPIO_17,
	PGPIO_18,
	PGPIO_19,
	PGPIO_20,
	PGPIO_21,
	PGPIO_22,
	PGPIO_23,
	PGPIO_24,
	PGPIO_25,
	PGPIO_26,
	PGPIO_27,
	PGPIO_28,
	PGPIO_29,
	PGPIO_30,
	PGPIO_31,
	P_GPIO_all,
	EGPIO_0,
	EGPIO_1,
	EGPIO_2,
	EGPIO_3,
	EGPIO_4,
	EGPIO_5,
	EGPIO_6,
	EGPIO_7,
	EGPIO_8,
	EGPIO_9,
	EGPIO_10,
	EGPIO_11,
	EGPIO_12,
	EGPIO_13,
	EGPIO_14,
	EGPIO_15,
	E_GPIO_all,
	DGPIO_0,
	DGPIO_1,
	DGPIO_2,
	DGPIO_3,
	DGPIO_4,
	DGPIO_5,
	DGPIO_6,
	DGPIO_7,
	DGPIO_8,
	DGPIO_9,
	D_GPIO_all,
	SGPIO_0,
	SGPIO_1,
	SGPIO_2,
	SGPIO_3,
	SGPIO_4,
	SGPIO_5,
	SGPIO_6,
	SGPIO_7,
	SGPIO_8,
	SGPIO_9,
	SGPIO_10,
	SGPIO_11,
	SGPIO_12,
	SGPIO_13,
	SGPIO_14,
	SGPIO_15,
	SGPIO_16,
	SGPIO_17,
	SGPIO_18,
	SGPIO_19,
	SGPIO_20,
	SGPIO_21,
	SGPIO_22,
	SGPIO_23,
	SGPIO_24,
	SGPIO_25,
	SGPIO_26,
	SGPIO_27,
	SGPIO_28,
	SGPIO_29,
	SGPIO_30,
	SGPIO_31,
	SGPIO_32,
	SGPIO_33,
	SGPIO_34,
	SGPIO_35,
	S_GPIO_all,
	GPIO_total,



	ENUM_DUMMY4WORD(GPIO_ALL)
} GPIO_ALL;

typedef enum
{
	func_FSPI=0x1,
	func_SDIO_1,
	func_SDIO_2,
	func_SPI,
	func_SPI2_1,
	func_SPI2_2,
	func_EJTAG,
	func_EXTROM,
	func_LCD310_1,
	func_LCD310_2,
	func_LCD310_RGB888,
	func_LCD210_1,
	func_LCD210_2,
	func_LCD310_DE,
	func_ETH,
	func_ETH2,
	func_I2C_1,
	func_I2C_2,
	func_I2C2_1,
	func_I2C2_2,
	func_I2C2_3,
	func_I2C2_4,
	func_I2C2_5,
	func_I2C3_1,
	func_I2C3_2,
	func_I2C_HDMI,
	func_EXT_CLK,
	func_EXT2_CLK,
	func_EXT3_CLK,
	func_EXT4_CLK,
	func_IRDA,
	func_RTC_CAL,
	func_TVDAC_TEST_CLK_IN,
	func_PWM,
	func_PWM2,
	func_PWM3,
	func_UART,
	func_UART2_1,
	func_UART2_2,
	func_UART2_3,
	func_UART2_4,
	func_UART3,
	func_UART4_1,
	func_UART4_2,
	func_UART4_3,
	func_UART4_4,
	func_UART4_5,
	func_UART4_6,
	func_UART_CTS_RTS,
	func_UART2_CTS_RTS,
	func_UART3_RTS,
	func_CAP,
	func_CAP1,
	func_CAP2,
	func_CAP3,
	func_CAP_CLK1,
	func_CAP_CLK2,
	func_CAP1_CLK1,
	func_CAP1_CLK2,
	func_CAP2_CLK1,
	func_CAP2_CLK2,
	func_CAP3_CLK1,
	func_CAP3_CLK2,
	func_CAP3_CLK3,
	func_I2S,
	func_I2S_MCLK,
	func_I2S_TX,
	func_I2S_RX,
	func_I2S2,
	func_I2S2_MCLK,
	func_I2S2_TX,
	func_I2S2_RX,
	func_I2S3_1,
	func_I2S3_1_MCLK,
	func_I2S3_1_TX,
	func_I2S3_1_RX,
	func_I2S3_2,
	func_I2S3_2_MCLK,
	func_I2S3_2_TX,
	func_I2S3_2_RX,
	func_I2S3_3,
	func_I2S3_3_MCLK,
	func_I2S3_3_TX,
	func_I2S3_3_RX,
	func_I2S4_1,
	func_I2S4_1_MCLK,
	func_I2S4_1_TX,
	func_I2S4_1_RX,
	func_I2S4_2,
	func_I2S4_2_MCLK,
	func_I2S4_2_TX,
	func_I2S4_2_RX,
	func_misc,
	FUNC_total,



	ENUM_DUMMY4WORD(FUNC_ALL)
} FUNC_ALL;

#define MAX_MODULE_NAME 9

ER pinmux_init(struct nvt_pinctrl_info *info);
ER pad_init(struct nvt_pinctrl_info *info, unsigned long nr_pad);
void gpio_init(struct nvt_gpio_info *gpio, int nr_gpio, struct nvt_pinctrl_info *info);
static inline ER power_init(struct nvt_power_info *power, int nr_power, struct nvt_pinctrl_info *info) {return 0;};
void pinmux_parsing(struct nvt_pinctrl_info *info);
int nvt_pinmux_proc_init(void);
void pinmux_preset(struct nvt_pinctrl_info *info);
void pad_preset(struct nvt_pinctrl_info *info);
void pinmux_gpio_parsing(struct nvt_pinctrl_info *info);
void gpio_func_show(void);
int pinmux_set_config(PINMUX_FUNC_ID id, u32 pinmux);

ER pinmux_init_ep(struct nvt_pinctrl_info *info);
ER pad_init_ep(struct nvt_pinctrl_info *info, unsigned long nr_pad);
void pinmux_parsing_ep(struct nvt_pinctrl_info *info);
int nvt_pinmux_proc_init_ep(int ep_ch);
void pinmux_preset_ep(struct nvt_pinctrl_info *info);
int pinmux_set_config_ep(PINMUX_FUNC_ID id, u32 pinmux, int ep_ch);
unsigned long get_top_addr(int ep_ch);
unsigned long get_pad_addr(int ep_ch);

static inline int pinmux_set_host(struct nvt_pinctrl_info *info, PINMUX_FUNC_ID id, u32 pinmux) {return 0;};
#endif

