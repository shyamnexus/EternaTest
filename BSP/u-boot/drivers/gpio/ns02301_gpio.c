/*
 *  gpio/ns02301_gpio.c
 *
 *  Author:	Howard Chang
 *  Created:	Dec 2, 2020
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

#define GPIO_VERSION "1.0.00"

struct nvt_gpio_bank {
	phys_addr_t base;
};

#define GPIO_GETREG(ofs)	         INW(IOADDR_GPIO_REG_BASE+(ofs))
#define GPIO_SETREG(ofs, value)	     OUTW(IOADDR_GPIO_REG_BASE+(ofs),(value))
#define PRGPIO_GETREG(ofs)	         INW(IOADDR_PRGPIO_REG_BASE+(ofs))
#define PRGPIO_SETREG(ofs, value)	 OUTW(IOADDR_PRGPIO_REG_BASE+(ofs),(value))

#define NVT_GPIO_STG_DATA_0		(0)
#define NVT_GPIO_STG_DIR_0		(0x10)
#define NVT_GPIO_STG_SET_0		(0x20)
#define NVT_GPIO_STG_CLR_0		(0x30)

#define NVT_PRGPIO_STG_DATA_0   (0)
#define NVT_PRGPIO_STG_DIR_0	(0x0C)
#define NVT_PRGPIO_STG_SET_0    (0x18)
#define NVT_PRGPIO_STG_CLR_0    (0x24)

static int gpio_validation(unsigned pin)
{
	if (pin >= NVT_GPIO_BASE_NUM) {
		//pre-roll gpio check
		if ((pin > P_GPIO(38) && pin < S_GPIO(0)) || \
		(pin > S_GPIO(16) && pin < A_GPIO(0)) || \
		(pin > A_GPIO(3))) {
			printf("The pre-roll gpio number is out of range: pin = %d\n", pin);
			return E_NOSPT;
		} else {
			return 0;
		}
	} else {
		//main part gpio check
		if (pin == 0x39) {
			//pgpio39 case
			return 0;
		} else {
			if ((pin < C_GPIO(0)) || \
			(pin > C_GPIO(24) && pin < P_GPIO(0))  || \
			(pin > P_GPIO(24) && pin < HSI_GPIO(0))  || \
			(pin > HSI_GPIO(11) && pin < D_GPIO(0))  || \
			(pin > D_GPIO(7))) {
				printf("The main gpio number is out of range: pin = %d\n", pin);
				return E_NOSPT;
			} else {
				return 0;
			}
		}
	}
}

static int nvt_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	unsigned long reg_data;
	unsigned long ofs;
	BOOL          pre_roll = FALSE;

	if (gpio_validation(offset) < 0) {
		return E_NOSPT;
	}
		
	if (offset >= NVT_GPIO_BASE_NUM) {
		offset -= NVT_GPIO_BASE_NUM;
		pre_roll = TRUE;
	}
	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);

	if (pre_roll) {
		reg_data = PRGPIO_GETREG(NVT_PRGPIO_STG_DIR_0 + ofs);
	} else {
		reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);
	}

	reg_data &= ~(1 << offset);   /*input*/

	if (pre_roll) {
		PRGPIO_SETREG(NVT_PRGPIO_STG_DIR_0 + ofs, reg_data);
	} else {
		GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);
	}

	return 0;
}

static int nvt_gpio_direction_output(struct udevice *dev, unsigned offset, int value)
{
	unsigned long reg_data, tmp;
	unsigned long ofs;
	BOOL          pre_roll = FALSE;

	if (gpio_validation(offset) < 0) {
		return E_NOSPT;
	}
		
	if (offset >= NVT_GPIO_BASE_NUM) {
		offset -= NVT_GPIO_BASE_NUM;
		pre_roll = TRUE;
	}
	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);

	if (pre_roll) {
		reg_data = PRGPIO_GETREG(NVT_PRGPIO_STG_DIR_0 + ofs);
	} else {
		reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);
	}

	reg_data |= (1 << offset);    /*output*/

	if (pre_roll) {
		PRGPIO_SETREG(NVT_PRGPIO_STG_DIR_0 + ofs, reg_data);
		if (value) {
			PRGPIO_SETREG(NVT_PRGPIO_STG_SET_0 + ofs, tmp);
		} else {
			PRGPIO_SETREG(NVT_PRGPIO_STG_CLR_0 + ofs, tmp);
		}
	} else {
		GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);
		if (value) {
			GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
		} else {
			GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);
		}
	}

	return 0;
}

static int nvt_gpio_get_value(struct udevice *dev, unsigned offset)
{
	unsigned long tmp;
	unsigned long ofs;
	BOOL          pre_roll = FALSE;

	if (gpio_validation(offset) < 0) {
		return E_NOSPT;
	}
		
	if (offset >= NVT_GPIO_BASE_NUM) {
		offset -= NVT_GPIO_BASE_NUM;
		pre_roll = TRUE;
	}
	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);

	if (pre_roll) {
		return (PRGPIO_GETREG(NVT_PRGPIO_STG_DATA_0 + ofs) & tmp) != 0;
	} else {
		return (GPIO_GETREG(NVT_GPIO_STG_DATA_0 + ofs) & tmp) != 0;
	}
}

static int nvt_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	unsigned long tmp;
	unsigned long ofs;
	BOOL          pre_roll = FALSE;

	if (gpio_validation(offset) < 0) {
		return E_NOSPT;
	}
		
	if (offset >= NVT_GPIO_BASE_NUM) {
		offset -= NVT_GPIO_BASE_NUM;
		pre_roll = TRUE;
	}
	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);
	tmp = (1 << offset);

	if (pre_roll) {
		if (value) {
			PRGPIO_SETREG(NVT_PRGPIO_STG_SET_0 + ofs, tmp);
		} else {
			PRGPIO_SETREG(NVT_PRGPIO_STG_CLR_0 + ofs, tmp);
		}
	} else {
		if (value) {
			GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
		} else {
			GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);
		}
	}

	return 0;
}

static int nvt_gpio_get_function(struct udevice *dev, unsigned offset)
{
	unsigned long reg_data;
	unsigned long ofs;
	BOOL          pre_roll = FALSE;

	if (gpio_validation(offset) < 0) {
		return E_NOSPT;
	}

	if (offset >= NVT_GPIO_BASE_NUM) {
		offset -= NVT_GPIO_BASE_NUM;
		pre_roll = TRUE;
	}
	ofs = (offset >> 5) << 2;
	offset &= (32 - 1);

	if (pre_roll) {
		reg_data = PRGPIO_GETREG(NVT_PRGPIO_STG_DIR_0 + ofs);
	} else {
		reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);
	}
	
	if (reg_data & (1 << offset)) {
		return GPIOF_OUTPUT;
	}
	
	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_nvt_ops = {
	.direction_input	= nvt_gpio_direction_input,
	.direction_output	= nvt_gpio_direction_output,
	.get_value		    = nvt_gpio_get_value,
	.set_value		    = nvt_gpio_set_value,
	.get_function		= nvt_gpio_get_function,
};

static int nvt_gpio_probe(struct udevice *dev)
{
	printf("GPIO version: %s\n", GPIO_VERSION);
	return 0;
}

static int nvt_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 256;
	uc_priv->bank_name = "nvt-gpio";

	return 0;
}

static const struct udevice_id nvt_gpio_ids[] = {
	{ .compatible = "nvt,nvt_gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_msm) = {
	.name	= "gpio_nvt",
	.id	= UCLASS_GPIO,
	.of_match = nvt_gpio_ids,
	.of_to_plat = nvt_gpio_ofdata_to_platdata,
	.probe	= nvt_gpio_probe,
	.ops	= &gpio_nvt_ops,
	.priv_auto = sizeof(struct nvt_gpio_bank),
};
