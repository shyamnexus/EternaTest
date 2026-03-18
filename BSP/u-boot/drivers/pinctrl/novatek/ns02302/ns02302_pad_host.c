/*
	PAD controller

	Sets the PAD control of each pin.

	@file       pad.c
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#include <plat/pad.h>
#include <plat/pad_reg.h>
#include "ns02302_pinmux.h"

static unsigned long TOP_PAD_REG_BASE;
#define PAD_GETREG(ofs)         readl((void __iomem *)(TOP_PAD_REG_BASE + ofs))
#define PAD_SETREG(ofs,value)   writel(value, (void __iomem *)(TOP_PAD_REG_BASE + ofs))

static DEFINE_SPINLOCK(pad_lock);
#define loc_cpu(flags) spin_lock_irqsave(&pad_lock, flags)
#define unl_cpu(flags) spin_unlock_irqrestore(&pad_lock, flags)

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

	if (pin > PAD_PIN_MAX) {
		pr_err("%s: Not Existed PAD_PIN %d\r\n", __func__, pin);
		return E_NOEXS;
	}

	bit_offset = pin & 0x1F;
	dw_offset = (pin >> 5);

	reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
	*pulltype = ((reg_data >> bit_offset) & 0x3);

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

	if (name > PAD_DS_MAX) {
		pr_err("%s: Not Existed PAD_DS %d\r\n", __func__, name);
		return E_NOEXS;
	}

	bit_offset = name & 0x1F;
	dw_offset = (name >> 5);

	loc_cpu(flags);
	reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
	reg_data &= ~(0xf << bit_offset);
	PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
	reg_data |= (driving << bit_offset);
	PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
	unl_cpu(flags);

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

	if (name > PAD_DS_MAX) {
		pr_err("%s: Not Existed PAD_DS %d\r\n", __func__, name);
		return E_NOEXS;
	}

	bit_offset = name & 0x1F;
	dw_offset = (name >> 5);

	reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
	*driving = ((reg_data >> bit_offset) & 0xf);

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
ER pad_set_power(PAD_POWER_STRUCT *pad_power)
{
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};
	union PAD_PWR3_REG pad_power3_reg = {0};
	unsigned long flags = 0;

	if (pad_power == NULL) {
		pr_err("%s: Not Existed PAD_POWER_STRUCT\r\n", __func__);
		return E_NOEXS;
	}

	if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		pr_err("MC1 power id no exist\r\n");
		return E_NOEXS;
	} else if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.bit.MC_POWER0 = pad_power->pad_power;
		pad_power1_reg.bit.MC_SEL = pad_power->bias_current;
		pad_power1_reg.bit.MC_OPSEL = pad_power->opa_gain;
		pad_power1_reg.bit.MC_PULLDOWN = pad_power->pull_down;
		pad_power1_reg.bit.MC_REGULATOR_EN = pad_power->enable;

		loc_cpu(flags);
		PAD_SETREG(PAD_PWR1_REG_OFS, pad_power1_reg.reg);
		unl_cpu(flags);

		return E_OK;
	} else if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		if (pad_power->pad_vad & PAD_VAD_2P4V)
			return E_PAR;

		pad_power2_reg.bit.ADC_POWER0 = pad_power->pad_power;
		pad_power2_reg.bit.ADC_SEL = pad_power->bias_current;
		pad_power2_reg.bit.ADC_OPSEL = pad_power->opa_gain;
		pad_power2_reg.bit.ADC_PULLDOWN = pad_power->pull_down;
		pad_power2_reg.bit.ADC_REGULATOR_EN = pad_power->enable;
		pad_power2_reg.bit.ADC_VAD = pad_power->pad_vad;

		loc_cpu(flags);
		PAD_SETREG(PAD_PWR2_REG_OFS, pad_power2_reg.reg);
		unl_cpu(flags);

		return E_OK;
	} else if (pad_power->pad_power_id == PAD_POWERID_CSI) {
		if (!(pad_power->pad_vad & PAD_VAD_2P4V))
			return E_PAR;

		pad_power3_reg.bit.CSI_SEL = pad_power->bias_current;
		pad_power3_reg.bit.CSI_OPSEL = pad_power->opa_gain;
		pad_power3_reg.bit.CSI_PULLDOWN = pad_power->pull_down;
		pad_power3_reg.bit.CSI_REGULATOR_EN = pad_power->enable;
		pad_power3_reg.bit.CSI_VAD = pad_power->pad_vad & 0xFF;

		loc_cpu(flags);
		PAD_SETREG(PAD_PWR3_REG_OFS, pad_power3_reg.reg);
		unl_cpu(flags);

		return E_OK;
	} else {
		return E_PAR;
	}

	return E_OK;
}
EXPORT_SYMBOL(pad_set_power);

/**
    Get pad power.

    get pad power.

    @param[in] pad_power  pointer to pad power struct
*/
void pad_get_power(PAD_POWER_STRUCT *pad_power)
{
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};
	union PAD_PWR3_REG pad_power3_reg = {0};

	if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.reg = PAD_GETREG(PAD_PWR1_REG_OFS);

		pad_power->pad_power = pad_power1_reg.bit.MC_POWER0;
		pad_power->bias_current = pad_power1_reg.bit.MC_SEL;
		pad_power->opa_gain = pad_power1_reg.bit.MC_OPSEL;
		pad_power->pull_down = pad_power1_reg.bit.MC_PULLDOWN;
		pad_power->enable = pad_power1_reg.bit.MC_REGULATOR_EN;
	} else if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		pad_power2_reg.reg = PAD_GETREG(PAD_PWR2_REG_OFS);

		pad_power->pad_power = pad_power2_reg.bit.ADC_POWER0;
		pad_power->bias_current = pad_power2_reg.bit.ADC_SEL;
		pad_power->opa_gain = pad_power2_reg.bit.ADC_OPSEL;
		pad_power->pull_down = pad_power2_reg.bit.ADC_PULLDOWN;
		pad_power->enable = pad_power2_reg.bit.ADC_REGULATOR_EN;
		pad_power->pad_vad = pad_power2_reg.bit.ADC_VAD;
	} else if (pad_power->pad_power_id == PAD_POWERID_CSI) {
		pad_power3_reg.reg = PAD_GETREG(PAD_PWR3_REG_OFS);

		pad_power->bias_current = pad_power3_reg.bit.CSI_SEL;
		pad_power->opa_gain = pad_power3_reg.bit.CSI_OPSEL;
		pad_power->pull_down = pad_power3_reg.bit.CSI_PULLDOWN;
		pad_power->enable = pad_power3_reg.bit.CSI_REGULATOR_EN;
		pad_power->pad_vad = pad_power3_reg.bit.CSI_VAD;
	}
}
EXPORT_SYMBOL(pad_get_power);

void pad_preset(struct nvt_pinctrl_info *info)
{
	TOP_PAD_REG_BASE = (unsigned long) info->pad_base;
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