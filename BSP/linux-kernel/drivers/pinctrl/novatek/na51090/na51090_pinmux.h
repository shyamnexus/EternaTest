#ifndef __PINMUX_DRV_H__
#define __PINMUX_DRV_H__

#include <plat/top_reg.h>
#include <plat/hardware.h>
#include <plat/top.h>
#include <plat/nvt-gpio.h>
#include <plat/pad.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#define DRV_VERSION "1.00.17"

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

#define MAX_MODULE_NAME 9

ER pinmux_init(struct nvt_pinctrl_info *info);
ER pad_init(struct nvt_pinctrl_info *info, unsigned long nr_pad);
void gpio_init(struct nvt_gpio_info *gpio, int nr_gpio, struct nvt_pinctrl_info *info);
static inline ER power_init(struct nvt_power_info *power, int nr_power, struct nvt_pinctrl_info *info) {return 0;};
void pinmux_parsing(struct nvt_pinctrl_info *info);
ER pinmux_parsing_i2c(uint32_t config);
ER pinmux_parsing_i2c_ep(uint32_t config, u32 ep_ch);
int nvt_pinmux_proc_init(void);
void pinmux_preset(struct nvt_pinctrl_info *info);
void pad_preset(struct nvt_pinctrl_info *info);
void pinmux_gpio_parsing(struct nvt_pinctrl_info *info);
int pinmux_set_config(PINMUX_FUNC_ID id, u32 pinmux);

ER pinmux_init_ep(struct nvt_pinctrl_info *info);
ER pad_init_ep(struct nvt_pinctrl_info *info, unsigned long nr_pad);
void pinmux_parsing_ep(struct nvt_pinctrl_info *info);
int nvt_pinmux_proc_init_ep(struct nvt_pinctrl_info *info);
void pinmux_preset_ep(struct nvt_pinctrl_info *info);
int pinmux_set_config_ep(PINMUX_FUNC_ID id, u32 pinmux, int ep_ch);
unsigned long get_top_addr(int ep_ch);
unsigned long get_pad_addr(int ep_ch);

static inline int pinmux_set_host(struct nvt_pinctrl_info *info, PINMUX_FUNC_ID id, u32 pinmux) {return 0;};
#endif

