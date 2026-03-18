/*
	PAD controller

	Sets the PAD control of each pin.

	@file       pad.c
	@ingroup    mIDrvSys_PAD
	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#include <plat/pad.h>
#include "na51102_pinmux.h"

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

	/* On na51102, the pullup value of DSIGPIO0~9 is 0x3, and the reserved value is 0x2 */
	if ((pin >= PAD_PIN_DSIGPIO0) && (pin <= PAD_PIN_DSIGPIO9)) {
		if (pulltype == PAD_PULLUP) {
			pulltype = 0x3;
		} else if (pulltype == PAD_KEEPER) {
			pulltype = 0x2;
		}
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

	/* On na51102, the pullup value of DSIGPIO0~9 is 0x3, and the reserved value is 0x2 */
	if ((pin >= PAD_PIN_DSIGPIO0) && (pin <= PAD_PIN_DSIGPIO9)) {
		if (*pulltype == 0x3) {
			*pulltype = PAD_PULLUP;
		} else if (*pulltype == 0x2) {
			*pulltype = PAD_KEEPER;
		}
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
	REGVALUE pwr0_reg, pwr4_reg;
	unsigned long flags = 0;

	if (pad_power == NULL) {
		pr_err("%s: Not Existed PAD_POWER_STRUCT\r\n", __func__);
		return E_NOEXS;
	}

	if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set 0x200 BIT[1] PAD_SDIO_VREGBYPASS = 0
			loc_cpu(flags);
			pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);
			pwr0_reg &= ~(0x2);
			PAD_SETREG(PAD_PWR0_REG_OFS, pwr0_reg);
			unl_cpu(flags);

			// Delay 10 ms
			msleep(10);

			// Set 0x210 BIT[0] POC_SD_1P8V = 1
			loc_cpu(flags);
			pwr4_reg = PAD_GETREG(PAD_PWR4_REG_OFS);
			pwr4_reg |= (0x1);
			PAD_SETREG(PAD_PWR4_REG_OFS, pwr4_reg);
			unl_cpu(flags);
		} else if (pad_power->pad_power == PAD_3P3V) {
			// Set 0x210 BIT[0] POC_SD_1P8V = 0
			loc_cpu(flags);
			pwr4_reg = PAD_GETREG(PAD_PWR4_REG_OFS);
			pwr4_reg &= ~(0x1);
			PAD_SETREG(PAD_PWR4_REG_OFS, pwr4_reg);
			unl_cpu(flags);

			// Delay 10 ms
			msleep(10);

			// Set 0x200 BIT[1] PAD_SDIO_VREGBYPASS = 1
			loc_cpu(flags);
			pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);
			pwr0_reg |= (0x2);
			PAD_SETREG(PAD_PWR0_REG_OFS, pwr0_reg);
			unl_cpu(flags);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SD\r\n", __func__);
			return E_NOEXS;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set 0x200 BIT[9] PAD_SDIO2_VREGBYPASS = 0
			loc_cpu(flags);
			pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);
			pwr0_reg &= ~(0x200);
			PAD_SETREG(PAD_PWR0_REG_OFS, pwr0_reg);
			unl_cpu(flags);

			// Delay 10 ms
			msleep(10);

			// Set 0x210 BIT[1] POC_SD2_1P8V = 1
			loc_cpu(flags);
			pwr4_reg = PAD_GETREG(PAD_PWR4_REG_OFS);
			pwr4_reg |= (0x2);
			PAD_SETREG(PAD_PWR4_REG_OFS, pwr4_reg);
			unl_cpu(flags);
		} else if (pad_power->pad_power == PAD_3P3V) {
			// Set 0x210 BIT[1] POC_SD2_1P8V = 0
			loc_cpu(flags);
			pwr4_reg = PAD_GETREG(PAD_PWR4_REG_OFS);
			pwr4_reg &= ~(0x2);
			PAD_SETREG(PAD_PWR4_REG_OFS, pwr4_reg);
			unl_cpu(flags);

			// Delay 10 ms
			msleep(10);

			// Set 0x200 BIT[9] PAD_SDIO2_VREGBYPASS = 1
			loc_cpu(flags);
			pwr0_reg = PAD_GETREG(PAD_PWR0_REG_OFS);
			pwr0_reg |= (0x200);
			PAD_SETREG(PAD_PWR0_REG_OFS, pwr0_reg);
			unl_cpu(flags);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SD2\r\n", __func__);
			return E_NOEXS;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_SN) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of SN (bootstrap[13] needs to be 1 before setting)
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR1_REG_OFS, PAD_1P8V_SN_MAGIC);
			unl_cpu(flags);
		} else if (pad_power->pad_power == PAD_3P3V) {
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR1_REG_OFS, 0);
			unl_cpu(flags);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SN\r\n", __func__);
			return E_NOEXS;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_P1) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of P1 (bootstrap[14] needs to be 1 before setting)
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR2_REG_OFS, PAD_1P8V_P1_MAGIC);
			unl_cpu(flags);
		} else if (pad_power->pad_power == PAD_3P3V) {
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR2_REG_OFS, 0);
			unl_cpu(flags);
		} else {
			pr_err("%s: Not Existed PAD_POWER for P1\r\n", __func__);
			return E_NOEXS;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_SD3) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of SD3 (bootstrap[15] needs to be 1 before setting)
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR3_REG_OFS, PAD_1P8V_SD3_MAGIC);
			unl_cpu(flags);
		} else if (pad_power->pad_power == PAD_3P3V) {
			loc_cpu(flags);
			PAD_SETREG(PAD_PWR3_REG_OFS, 0);
			unl_cpu(flags);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SD3\r\n", __func__);
			return E_NOEXS;
		}
	} else {
		pr_err("%s: Not Existed PAD_POWERID\r\n", __func__);
		return E_NOEXS;
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
	REGVALUE pwr4_reg;

	if (pad_power == NULL) {
		pr_err("%s: Not Existed PAD_POWER_STRUCT\r\n", __func__);
		return;
	}

	pwr4_reg = PAD_GETREG(PAD_PWR4_REG_OFS);

	if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		if (pwr4_reg & (0x1)) {
			pad_power->pad_status = PAD_1P8V;
		} else {
			pad_power->pad_status = PAD_3P3V;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		if (pwr4_reg & (0x2)) {
			pad_power->pad_status = PAD_1P8V;
		} else {
			pad_power->pad_status = PAD_3P3V;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_SN) {
		if (pwr4_reg & (0x4)) {
			pad_power->pad_status = PAD_1P8V;
		} else {
			pad_power->pad_status = PAD_3P3V;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_P1) {
		if (pwr4_reg & (0x8)) {
			pad_power->pad_status = PAD_1P8V;
		} else {
			pad_power->pad_status = PAD_3P3V;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_SD3) {
		if (pwr4_reg & (0x10)) {
			pad_power->pad_status = PAD_1P8V;
		} else {
			pad_power->pad_status = PAD_3P3V;
		}
	} else {
		pr_err("%s: Not Existed PAD_POWERID\r\n", __func__);
	}
}
EXPORT_SYMBOL(pad_get_power);

void pad_preset(struct nvt_pinctrl_info *info)
{
	TOP_PAD_REG_BASE = (unsigned long) info->pad_base;
}

static int PAD_PIN_MAX_CNT;

int pad_suspend(struct nvt_pinctrl_info *info)
{
	PAD_DRIVINGSINK driving[1]={0};
	PAD_PULL pull[1]={0};
	unsigned long nr_pad;

	int i,j;

	PAD_DS pin_ds_end[8] = {PAD_DS_CGPIO23, PAD_DS_PGPIO37, PAD_DS_SYS_RST, PAD_DS_LGPIO30, PAD_DS_SGPIO26, PAD_DS_HSIGPIO23, PAD_DS_DSIGPIO10, PAD_DS_AGPIO3};
	PAD_DS pin_ds_base[8] = {PAD_DS_CGPIO_BASE, PAD_DS_PGPIO_BASE, PAD_DS_DGPIO_BASE, PAD_DS_LGPIO_BASE, PAD_DS_SGPIO_BASE, PAD_DS_HSIGPIO_BASE, PAD_DS_DSIGPIO_BASE, PAD_DS_AGPIO_BASE};

	PAD_DS pin_end[8] = {PAD_PIN_CGPIO23, PAD_PIN_PGPIO37, PAD_PIN_SYS_RST, PAD_PIN_LGPIO30, PAD_PIN_SGPIO26, PAD_PIN_HSIGPIO23, PAD_PIN_DSIGPIO10, PAD_PIN_AGPIO3};
	PAD_DS pin_base[8] = {PAD_CGPIO_BASE, PAD_PGPIO_BASE, PAD_DGPIO_BASE, PAD_LGPIO_BASE, PAD_SGPIO_BASE, PAD_HSIGPIO_BASE, PAD_DSIGPIO_BASE, PAD_AGPIO_BASE};


	PAD_PIN_MAX_CNT = 0;
	for(j=0; j < 8; j++){
		for(i = 0; pin_ds_base[j] + (i * 4) <= pin_ds_end[j]; i++){
			info->pad[PAD_PIN_MAX_CNT].pad_ds_pin = pin_ds_base[j] + (i * 4);
			PAD_PIN_MAX_CNT++;
		}
	}


	PAD_PIN_MAX_CNT = 0;
	for(j=0; j < 8; j++){
		for(i = 0; pin_base[j] + (i * 2) <= pin_end[j]; i++){
			info->pad[PAD_PIN_MAX_CNT].pad_gpio_pin = pin_base[j] + (i * 2);
			PAD_PIN_MAX_CNT++;
		}
	}

	for(nr_pad=0; nr_pad <PAD_PIN_MAX_CNT; nr_pad++){
		pad_get_drivingsink(info->pad[nr_pad].pad_ds_pin ,driving);
		info->pad[nr_pad].driving = *driving;
		pad_get_pull_updown(info->pad[nr_pad].pad_gpio_pin,pull);
		info->pad[nr_pad].direction = *pull;
	}
	return 0;
}

int pad_resume(struct nvt_pinctrl_info *info)
{

	pad_init(info, PAD_PIN_MAX_CNT);

	return 0;
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