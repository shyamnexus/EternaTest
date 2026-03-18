/*
	PAD controller

	Sets the PAD control of each pin.

	@file       ns02301_pad_host.c
	@ingroup
	@note

	Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#include <plat/pad.h>
#include "ns02301_pinmux.h"

static unsigned long TOP_PAD_REG_BASE;
#define PAD_GETREG(ofs)         readl((void __iomem *)(TOP_PAD_REG_BASE + ofs))
#define PAD_SETREG(ofs,value)   writel(value, (void __iomem *)(TOP_PAD_REG_BASE + ofs))

static unsigned long TOP_PRPAD_REG_BASE;
#define PR_PAD_GETREG(ofs)         readl((void __iomem *)(TOP_PRPAD_REG_BASE + ofs))
#define PR_PAD_SETREG(ofs,value)   writel(value, (void __iomem *)(TOP_PRPAD_REG_BASE + ofs))

static DEFINE_SPINLOCK(pad_lock);
#define loc_cpu(flags) spin_lock_irqsave(&pad_lock, flags)
#define unl_cpu(flags) spin_unlock_irqrestore(&pad_lock, flags)

/**
 * PR PART PAD check
 */
#define PR_PREFIX_CHK 0x400
static bool is_pr_pad(PAD_PIN pad_no)
{
	if (pad_no >= PR_PREFIX_CHK)
		return 1;
	else
		return 0;
}

/**
    Set a pin pull up/down/keeper.

    Set a pin pull up/down/keeper.

    @param[in]  pin Pin name. For example PAD_PIN_xxxx. See pad.h.
    @param[in]  pulltype PAD_NONE or PAD_PULLUP or PAD_PULLDOWN or PAD_KEEPER

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_pull_updown(PAD_PIN pin, PAD_PULL pulltype)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;
	unsigned long flags = 0;

	if (is_pr_pad(pin)) { //pre-roll pad
		if (pin > PR_PAD_PIN_MAX) {
			pr_err("%s: Not Existed PR_PAD_PIN %d\r\n", __func__, pin);
			return E_NOEXS;
		}

		#if 0 //TODO
		/* On ns02301, the pullup value of AGPIO is 0x3, and the reserved value is 0x2 */
		if (pin == PAD_PIN_AGPIO0) {
			if (pulltype == PAD_PULLUP) {
				pulltype = 0x3;
			} else if (pulltype == PAD_KEEPER) {
				pulltype = 0x2;
			}
		}
		#endif
		pin = pin - PR_PREFIX_CHK;

		bit_offset = pin & 0x1F;
		dw_offset = (pin >> 5);

		loc_cpu(flags);
		reg_data = PR_PAD_GETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2));
		reg_data &= ~(0x3 << bit_offset);
		PR_PAD_SETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		reg_data |= (pulltype << bit_offset);
		PR_PAD_SETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		unl_cpu(flags);
	} else {  // main part pad
		if (pin > PAD_PIN_MAX) {
			pr_err("%s: Not Existed PAD_PIN %d\r\n", __func__, pin);
			return E_NOEXS;
		}

		bit_offset = pin & 0x1F;
		dw_offset = (pin >> 5);

		loc_cpu(flags);
		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
		reg_data &= ~(0x3 << bit_offset);
		PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		reg_data |= (pulltype << bit_offset);
		PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		unl_cpu(flags);
	}
	return E_OK;
}
EXPORT_SYMBOL(pad_set_pull_updown);

/**
    Get a pin pull up/down/keeper

    Get a pin pull up/down/keeper

    @param[in]      pin         Pin name. For example PAD_PIN_xxxx. See pad.h.
    @param[out]     pulltype    PAD_NONE or PAD_PULLUP or PAD_PULLDOWN or PAD_KEEPER

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_pull_updown(PAD_PIN pin, PAD_PULL *pulltype)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;

	if (is_pr_pad(pin)) { //pre-roll pad
		if (pin > PR_PAD_PIN_MAX) {
			pr_err("%s: Not Existed PR_PAD_PIN %d\r\n", __func__, pin);
			return E_NOEXS;
		}

		pin = pin - PR_PREFIX_CHK;

		bit_offset = pin & 0x1F;
		dw_offset = (pin >> 5);

		reg_data = PR_PAD_GETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2));
		*pulltype = ((reg_data >> bit_offset) & 0x3);

		#if 0 //TODO
		/* On ns02301, the pullup value of AGPIO0 is 0x3, and the reserved value is 0x2 */
		if (pin == PAD_PIN_AGPIO0) {
			if (*pulltype == 0x3) {
				*pulltype = PAD_PULLUP;
			} else if (*pulltype == 0x2) {
				*pulltype = PAD_KEEPER;
			}
		}
		#endif
	} else { //main part pad

		if (pin > PAD_PIN_MAX) {
			pr_err("%s: Not Existed PAD_PIN %d\r\n", __func__, pin);
			return E_NOEXS;
		}

		bit_offset = pin & 0x1F;
		dw_offset = (pin >> 5);

		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
		*pulltype = ((reg_data >> bit_offset) & 0x3);
	}
	return E_OK;
}
EXPORT_SYMBOL(pad_get_pull_updown);

/**
    Set driving/sink.

    Set driving/sink.

    @param[in] name     For example PAD_DS_xxxx. See pad.h.
    @param[in] driving  PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_drivingsink(PAD_DS name, PAD_DRIVINGSINK driving)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;
	unsigned long flags = 0;

	if (is_pr_pad(name)) { //pre-roll pad
		if (name > PR_PAD_DS_MAX) {
			pr_err("%s: Not Existed PR_PAD_DS %d\r\n", __func__, name);
			return E_NOEXS;
		}

		name = name - PR_PREFIX_CHK;

		bit_offset = name & 0x1F;
		dw_offset = (name >> 5);

		loc_cpu(flags);
		reg_data = PR_PAD_GETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2));

		if ((dw_offset << 2) == PR_PAD_2L_DS_SGPIO_BASE) {
			/* 2 level case */
			reg_data &= ~(0x1 << bit_offset);

		} else {
			/* 4 level case */
			reg_data &= ~(0x3 << bit_offset);
		}

		PR_PAD_SETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		reg_data |= (driving << bit_offset);
		PR_PAD_SETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		unl_cpu(flags);

	} else { //main part pad
		if (name > PAD_DS_MAX) {
			pr_err("%s: Not Existed PAD_DS %d\r\n", __func__, name);
			return E_NOEXS;
		}

		bit_offset = name & 0x1F;
		dw_offset = (name >> 5);

		loc_cpu(flags);
		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));

		if ((dw_offset << 2) == PAD_8L_DS_CGPIO_BASE) {
			/* 8 level case */
			reg_data &= ~(0xf << bit_offset);

		} else if ((dw_offset << 2) == PAD_2L_DS_HSIGPIO_BASE) {
			/* 2 level case */
			reg_data &= ~(0x1 << bit_offset);

		} else {
			/* 4 level case */
			reg_data &= ~(0x3 << bit_offset);
		}

		PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		reg_data |= (driving << bit_offset);
		PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
		unl_cpu(flags);
	}
	return E_OK;
}
EXPORT_SYMBOL(pad_set_drivingsink);

/**
    Get driving/sink.

    Get driving/sink.

    @param[in]  name        For example PAD_DS_xxxx. See pad.h.
    @param[out] driving     PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_drivingsink(PAD_DS name, PAD_DRIVINGSINK *driving)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;

	if (is_pr_pad(name)) { //pre-roll pad
		if (name > PR_PAD_DS_MAX) {
			pr_err("%s: Not Existed PR_PAD_DS %d\r\n", __func__, name);
			return E_NOEXS;
		}

		name = name - PR_PREFIX_CHK;

		bit_offset = name & 0x1F;
		dw_offset = (name >> 5);

		reg_data = PR_PAD_GETREG(PR_PAD_PUPD0_REG_OFS + (dw_offset << 2));

		if ((dw_offset << 2) == PR_PAD_2L_DS_SGPIO_BASE) {
			/* 2 level case */
			*driving = ((reg_data >> bit_offset) & 0x1);

		} else {
			/* 4 level case */
			*driving = ((reg_data >> bit_offset) & 0x3);

		}

	} else { //main part pad
		if (name > PAD_DS_MAX) {
			pr_err("%s: Not Existed PAD_DS %d\r\n", __func__, name);
			return E_NOEXS;
		}

		bit_offset = name & 0x1F;
		dw_offset = (name >> 5);

		reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));

		if ((dw_offset << 2) == PAD_8L_DS_CGPIO_BASE) {
			/* 8 level case */
			*driving = ((reg_data >> bit_offset) & 0xf);

		} else if ((dw_offset << 2) == PAD_2L_DS_HSIGPIO_BASE) {
			/* 2 level case */
			*driving = ((reg_data >> bit_offset) & 0x1);

		} else {
			/* 4 level case */
			*driving = ((reg_data >> bit_offset) & 0x3);
		}
	}
	return E_OK;
}
EXPORT_SYMBOL(pad_get_drivingsink);

/**
    Set pad power.

    Set pad power.

    @param[in] pad_power  pointer to pad power struct

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
#define ADC_POWER0_BIT 0
#define ADC_SEL_BIT 1
#define ADC_OPSEL_BIT 2
#define ADC_PULLDOWN_BIT 3
#define ADC_REGULATOR_EN_BIT 4
#define ADC_VAD_BIT 5
ER pad_set_power(PAD_POWER_STRUCT *pad_power)
{
	REGVALUE pwr0_reg;
	unsigned long flags = 0;

	if (pad_power == NULL) {
		pr_err("%s: Not Existed PAD_POWER_STRUCT\r\n", __func__);
		return E_NOEXS;
	}

	pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);

	if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		if (pad_power->pad_vad & PAD_VAD_2P4V)
			return E_PAR;

		//ADC_POWER0
		pwr0_reg &= ~(0x1 << ADC_POWER0_BIT);
		pwr0_reg |= (pad_power->pad_power << ADC_POWER0_BIT);
		//ADC_SEL
		pwr0_reg &= ~(0x1 << ADC_SEL_BIT);
		pwr0_reg |= (pad_power->bias_current << ADC_SEL_BIT);

		//ADC_OPSEL
		pwr0_reg &= ~(0x1 << ADC_OPSEL_BIT);
		pwr0_reg |= (pad_power->opa_gain << ADC_OPSEL_BIT);

		//ADC_PULLDOWN
		pwr0_reg &= ~(0x1 << ADC_PULLDOWN_BIT);
		pwr0_reg |= (pad_power->pull_down << ADC_PULLDOWN_BIT);

		//ADC_REGULATOR_EN
		pwr0_reg &= ~(0x1 << ADC_REGULATOR_EN_BIT);
		pwr0_reg |= (pad_power->enable << ADC_REGULATOR_EN_BIT);

		//ADC_VAD
		pwr0_reg &= ~(0x3 << ADC_VAD_BIT);
		pwr0_reg |= (pad_power->pad_vad << ADC_VAD_BIT);

		loc_cpu(flags);
		PAD_SETREG(PAD_PWR0_REG_OFS, pwr0_reg);
		unl_cpu(flags);

		return E_OK;
	} else {
		pr_err("%s: Not Existed PAD_POWERID\r\n", __func__);
		return E_NOEXS;
	}
}
EXPORT_SYMBOL(pad_set_power);

/**
    Get pad power.

    get pad power.

    @param[in] pad_power  pointer to pad power struct
*/
void pad_get_power(PAD_POWER_STRUCT *pad_power)
{
	REGVALUE pwr0_reg;

	pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);

	if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);

		pad_power->pad_power = (pwr0_reg >> ADC_POWER0_BIT) & 0x1;
		pad_power->bias_current = (pwr0_reg >> ADC_SEL_BIT) & 0x1;
		pad_power->opa_gain = (pwr0_reg >> ADC_OPSEL_BIT) & 0x1;
		pad_power->pull_down =  (pwr0_reg >> ADC_PULLDOWN_BIT) & 0x1;
		pad_power->enable = (pwr0_reg >> ADC_REGULATOR_EN_BIT) & 0x1;
		pad_power->pad_vad = (pwr0_reg >> ADC_VAD_BIT) & 0x3;;
	}
}
EXPORT_SYMBOL(pad_get_power);

void pad_preset(struct nvt_pinctrl_info *info)
{
	TOP_PAD_REG_BASE = (unsigned long) info->pad_base;
	TOP_PRPAD_REG_BASE = (unsigned long) info->pr_pad_base;
}

ER pad_init(struct nvt_pinctrl_info *info, unsigned long nr_pad)
{
	unsigned long cnt;
	int ret = 0;

	for (cnt = 0; cnt < nr_pad; cnt++) {
		ret = pad_set_pull_updown(info->pad[cnt].pad_gpio_pin, info->pad[cnt].direction);
		if (ret) {
			pr_err("%s: pad pin 0x%lx pull failed!\n", __func__, info->pad[cnt].pad_gpio_pin);
			return ret;
		}

		ret = pad_set_drivingsink(info->pad[cnt].pad_ds_pin, info->pad[cnt].driving);
		if (ret) {
			pr_err("%s: pad pin 0x%lx driving failed!\n", __func__, info->pad[cnt].pad_ds_pin);
			return ret;
		}
	}

	return E_OK;
}

ER power_init(struct nvt_power_info *power, int nr_power, struct nvt_pinctrl_info *info)
{
	PAD_POWER_STRUCT pad_power;
	int cnt;
	int ret = 0;

	for (cnt = 0; cnt < nr_power; cnt++) {
		pad_power.pad_power_id = power[cnt].pad_power_id;
		pad_power.pad_power = power[cnt].pad_power;

		ret = pad_set_power(&pad_power);
		if (ret) {
			pr_err("%s: pad_power_id(%lx) set pad_power(%lx) failed!\n", __func__, power[cnt].pad_power_id, power[cnt].pad_power);
			return ret;
		}
	}

	return E_OK;
}
