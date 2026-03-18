/*
    PAD controller

    Sets the PAD control of each pin.

    @file       pad.c
    @ingroup    mIDrvSys_PAD
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#include <plat/pad.h>
#include <plat/pad_reg.h>
#include "na51090_pinmux.h"

#define PAD_EP_GETREG(ofs,ch)         pad_ep_getreg(ofs, ch)
#define PAD_EP_SETREG(ofs,value,ch)   pad_ep_setreg(ofs, value, ch)

static DEFINE_SPINLOCK(pad_lock);
#define loc_cpu(lock, flags) spin_lock_irqsave(lock, flags)
#define unl_cpu(lock, flags) spin_unlock_irqrestore(lock, flags)

static uint32_t pad_ep_getreg(uint32_t offset, int ep_ch)
{
	unsigned long pad_ep_addr;
	uint32_t value;

	pad_ep_addr = get_pad_addr(ep_ch);

	if (pad_ep_addr) {
		value = readl((void __iomem *)(pad_ep_addr + offset));
	} else {
		pr_err("%s: Not Existed pad ep address\r\n", __func__);
		return E_NOEXS;
	}

	return value;
}

static void pad_ep_setreg(uint32_t offset, uint32_t value, int ep_ch)
{
	unsigned long pad_ep_addr;

	pad_ep_addr = get_pad_addr(ep_ch);

	if (pad_ep_addr) {
		writel(value, (void __iomem *)(pad_ep_addr + offset));
	} else {
		pr_err("%s: Not Existed pad ep address\r\n", __func__);
	}
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
ER pad_set_pull_updown_ep(PAD_PIN pin, PAD_PULL pulltype, int ep_ch)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;
	unsigned long flags = 0;

	if (pin > PAD_PIN_MAX) {
		pr_err("%s: Not Existed PAD_PIN\r\n", __func__);
		return E_NOEXS;
	}

	bit_offset = pin & 0x1F;
	dw_offset = (pin >> 5);

	loc_cpu(&pad_lock, flags);
	reg_data = PAD_EP_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), ep_ch);
	reg_data &= ~(0x3 << bit_offset);
	PAD_EP_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data, ep_ch);
	reg_data |= (pulltype << bit_offset);
	PAD_EP_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data, ep_ch);
	unl_cpu(&pad_lock, flags);

	return E_OK;
}
EXPORT_SYMBOL(pad_set_pull_updown_ep);

/**
    Get a pin pull up/down/keeper

    Get a pin pull up/down/keeper

    @param[in]      pin         Pin name. For example PAD_PIN_xxxx. See pad.h.
    @param[out]     pulltype    PAD_NONE or PAD_PULLUP or PAD_PULLDOWN or PAD_KEEPER

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_pull_updown_ep(PAD_PIN pin, PAD_PULL *pulltype, int ep_ch)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;

	if (pin > PAD_PIN_MAX)
		return E_NOEXS;

	bit_offset = pin & 0x1F;
	dw_offset = (pin >> 5);

	reg_data = PAD_EP_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), ep_ch);
	*pulltype = ((reg_data >> bit_offset) & 0x3);

	return E_OK;
}
EXPORT_SYMBOL(pad_get_pull_updown_ep);

/**
    Set driving/sink.

    Set driving/sink.

    @param[in] name     For example PAD_DS_xxxx. See pad.h.
    @param[in] driving  PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_drivingsink_ep(PAD_DS name, PAD_DRIVINGSINK driving, int ep_ch)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;
	unsigned long flags = 0;

	if (name > PAD_DS_MAX) {
		pr_err("%s: Not Existed PAD_DS\r\n", __func__);
		return E_NOEXS;
	}

	bit_offset = name & 0x1F;
	dw_offset = (name >> 5);

	loc_cpu(&pad_lock, flags);
	reg_data = PAD_EP_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), ep_ch);
	reg_data &= ~(0xf << bit_offset);
	PAD_EP_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data, ep_ch);
	reg_data |= (driving << bit_offset);
	PAD_EP_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data, ep_ch);
	unl_cpu(&pad_lock, flags);

	return E_OK;
}
EXPORT_SYMBOL(pad_set_drivingsink_ep);

/**
    Get driving/sink.

    Get driving/sink.

    @param[in]  name        For example PAD_DS_xxxx. See pad.h.
    @param[out] driving     PAD_DRIVINGSINK_xxMA. See pad.h.

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_get_drivingsink_ep(PAD_DS name, PAD_DRIVINGSINK *driving, int ep_ch)
{
	UINT32 dw_offset, bit_offset;
	REGVALUE reg_data;

	if (name > PAD_DS_MAX) {
		pr_err("%s: Not Existed PAD_DS\r\n", __func__);
		return E_NOEXS;
	}

	bit_offset = name & 0x1F;
	dw_offset = (name >> 5);

	reg_data = PAD_EP_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), ep_ch);
	*driving = ((reg_data >> bit_offset) & 0xf);

	return E_OK;
}
EXPORT_SYMBOL(pad_get_drivingsink_ep);

/**
    Set pad power.

    Set pad power.

    @param[in] pad_power  pointer to pad power struct

    @return
        - @b E_OK: Done with no error
        - Others: Error occured.
*/
ER pad_set_power_ep(PAD_POWER_STRUCT *pad_power, int ep_ch)
{
#if 0
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};
	union PAD_PWR3_REG pad_power3_reg = {0};
	unsigned long flags = 0;

	if (pad_power == NULL) {
		return E_PAR;
	}
	if (pad_power->pad_power_id == PAD_POWERID_MC1) {
		pr_err("%s: MC1 power id no exist\r\n", __func__);
		return E_NOEXS;
	} else if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.bit.MC_POWER0 = pad_power->pad_power;
		pad_power1_reg.bit.MC_SEL = pad_power->bias_current;
		pad_power1_reg.bit.MC_OPSEL = pad_power->opa_gain;
		pad_power1_reg.bit.MC_PULLDOWN = pad_power->pull_down;
		pad_power1_reg.bit.MC_REGULATOR_EN = pad_power->enable;

		loc_cpu(&pad_lock, flags);
		PAD_EP_SETREG(PAD_PWR1_REG_OFS, pad_power1_reg.reg, ep_ch);
		unl_cpu(&pad_lock, flags);

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

		loc_cpu(&pad_lock, flags);
		PAD_EP_SETREG(PAD_PWR2_REG_OFS, pad_power2_reg.reg, ep_ch);
		unl_cpu(&pad_lock, flags);

		return E_OK;
	} else if (pad_power->pad_power_id == PAD_POWERID_CSI) {
		if (!(pad_power->pad_vad & PAD_VAD_2P4V))
			return E_PAR;

		pad_power3_reg.bit.CSI_SEL = pad_power->bias_current;
		pad_power3_reg.bit.CSI_OPSEL = pad_power->opa_gain;
		pad_power3_reg.bit.CSI_PULLDOWN = pad_power->pull_down;
		pad_power3_reg.bit.CSI_REGULATOR_EN = pad_power->enable;
		pad_power3_reg.bit.CSI_VAD = pad_power->pad_vad & 0xFF;
		PAD_EP_SETREG(PAD_PWR3_REG_OFS, pad_power3_reg.reg, ep_ch);

		return E_OK;
	} else {
		return E_PAR;
	}
#else
	return E_NOSPT;
#endif
}
EXPORT_SYMBOL(pad_set_power_ep);

/**
    Get pad power.

    get pad power.

    @param[in] pad_power  pointer to pad power struct
*/
void pad_get_power_ep(PAD_POWER_STRUCT *pad_power, int ep_ch)
{
#if 0
	union PAD_PWR1_REG pad_power1_reg = {0};
	union PAD_PWR2_REG pad_power2_reg = {0};
	union PAD_PWR3_REG pad_power3_reg = {0};

	if (pad_power->pad_power_id == PAD_POWERID_MC0) {
		pad_power1_reg.reg = PAD_EP_GETREG(PAD_PWR1_REG_OFS, ep_ch);

		pad_power->pad_power = pad_power1_reg.bit.MC_POWER0;
		pad_power->bias_current = pad_power1_reg.bit.MC_SEL;
		pad_power->opa_gain = pad_power1_reg.bit.MC_OPSEL;
		pad_power->pull_down = pad_power1_reg.bit.MC_PULLDOWN;
		pad_power->enable = pad_power1_reg.bit.MC_REGULATOR_EN;
	} else if (pad_power->pad_power_id == PAD_POWERID_ADC) {
		pad_power2_reg.reg = PAD_EP_GETREG(PAD_PWR2_REG_OFS, ep_ch);

		pad_power->pad_power = pad_power2_reg.bit.ADC_POWER0;
		pad_power->bias_current = pad_power2_reg.bit.ADC_SEL;
		pad_power->opa_gain = pad_power2_reg.bit.ADC_OPSEL;
		pad_power->pull_down = pad_power2_reg.bit.ADC_PULLDOWN;
		pad_power->enable = pad_power2_reg.bit.ADC_REGULATOR_EN;
		pad_power->pad_vad = pad_power2_reg.bit.ADC_VAD;
	} else if (pad_power->pad_power_id == PAD_POWERID_CSI) {
		pad_power3_reg.reg = PAD_EP_GETREG(PAD_PWR3_REG_OFS, ep_ch);

		pad_power->bias_current = pad_power3_reg.bit.CSI_SEL;
		pad_power->opa_gain = pad_power3_reg.bit.CSI_OPSEL;
		pad_power->pull_down = pad_power3_reg.bit.CSI_PULLDOWN;
		pad_power->enable = pad_power3_reg.bit.CSI_REGULATOR_EN;
		pad_power->pad_vad = pad_power3_reg.bit.CSI_VAD;
	}
#endif
}
EXPORT_SYMBOL(pad_get_power_ep);

ER pad_set_delaychain_ep(PAD_DLY_STRUCT *del_sel, int ep_ch)
{
	union PAD_LCDDLY_REG reg_data;
	unsigned long flags = 0;

	loc_cpu(&pad_lock, flags);
	reg_data.reg = PAD_EP_GETREG(PAD_LCDDLY_REG_OFS, ep_ch);
	reg_data.bit.lcd_dly_vo_clk = del_sel->vo_clk_dly;
	reg_data.bit.lcd_inv_vo_clk = del_sel->vo_clk_inv;
	reg_data.bit.lcd_dly_vga_hs = del_sel->vga_hs_dly;
	reg_data.bit.lcd_inv_vga_hs = del_sel->vga_hs_inv;
	reg_data.bit.lcd_dly_vga_vs = del_sel->vga_vs_dly;
	reg_data.bit.lcd_inv_vga_hs = del_sel->vga_hs_inv;
	PAD_EP_SETREG(PAD_LCDDLY_REG_OFS, reg_data.reg, ep_ch);
	unl_cpu(&pad_lock, flags);

	return E_OK;
}
EXPORT_SYMBOL(pad_set_delaychain_ep);

void pad_get_delaychain_ep(PAD_DLY_STRUCT *del_sel, int ep_ch)
{
	union PAD_LCDDLY_REG reg_data;

	reg_data.reg = PAD_EP_GETREG(PAD_LCDDLY_REG_OFS, ep_ch);
	del_sel->vo_clk_dly = reg_data.bit.lcd_dly_vo_clk;
	del_sel->vo_clk_inv = reg_data.bit.lcd_inv_vo_clk;
	del_sel->vga_hs_dly = reg_data.bit.lcd_dly_vga_hs;
	del_sel->vga_hs_inv = reg_data.bit.lcd_inv_vga_hs;
	del_sel->vga_vs_dly = reg_data.bit.lcd_dly_vga_vs;
	del_sel->vga_hs_inv = reg_data.bit.lcd_inv_vga_hs;
}
EXPORT_SYMBOL(pad_get_delaychain_ep);

ER pad_init_ep(struct nvt_pinctrl_info *info, unsigned long nr_pad)
{
	int cnt, ret = 0;

	for (cnt = 0; cnt < nr_pad; cnt++) {
		ret = pad_set_pull_updown_ep(info->pad[cnt].pad_gpio_pin, info->pad[cnt].direction, info->ep_ch);
		if (ret) {
			pr_err("%s: pad pin 0x%lx pull failed at ep%d!\n", __func__, info->pad[cnt].pad_gpio_pin, info->ep_ch);
			return ret;
		}

		ret = pad_set_drivingsink_ep(info->pad[cnt].pad_ds_pin, info->pad[cnt].driving, info->ep_ch);
		if (ret) {
			pr_err("%s: pad pin 0x%lx driving failed at ep%d!\n", __func__, info->pad[cnt].pad_ds_pin, info->ep_ch);
			return ret;
		}
	}

	return E_OK;
}
EXPORT_SYMBOL(pad_init_ep);