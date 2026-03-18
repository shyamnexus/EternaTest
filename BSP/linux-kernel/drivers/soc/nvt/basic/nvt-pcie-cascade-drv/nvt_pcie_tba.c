/**
    NVT PCIE cascade driver
    To handle NVT PCIE cascade driver: RC/EP connection and isr handling
    @file pcie-pcie-edma.c
    @ingroup
    @note
    Copyright Novatek Microelectronics Corp. 2021. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/soc/nvt/nvt-io.h>
#include <linux/soc/nvt/nvt-pcie-lib.h>
#include <linux/msi.h>
#include <linux/soc/nvt/nvt-pci.h>

#include "nvt_pcie_tba.h"
#include "pcie_reg.h"



struct nvt_pcie_chip *nvtpcie_tba_chip_ptr = NULL;

int nvt_pcie_tba_init(struct nvt_pcie_chip *chip_ptr,void *pcie_dbi_vbase, int pcie_id)
{

	nvtpcie_tba_chip_ptr = chip_ptr;
	spin_lock_init(&nvtpcie_tba_chip_ptr->lock_ras);

	printk("nvt_pcie_tba_init start\r\n");

	//Enable Error counter
	pcie_ras_ec_start(PCIE_EC_EN_ALLON);

	return 0;
}


void pcie_ras_tba_set(PCIE_TBA_SEL tba_sel , PCIE_TBA_TIME tba_time) {
	T_TIME_BASED_ANALYSIS_CONTROL tba_control;

	tba_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS);

	tba_control.bit.TIME_BASED_REPORT_SELECT = tba_sel;
	tba_control.bit.TIME_BASED_DURATION_SELECT = tba_time;

	PCIE_SETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS, tba_control.reg);
}

void pcie_ras_tba_start(void) {
	T_TIME_BASED_ANALYSIS_CONTROL tba_control;

	tba_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS);
	// This bit will be cleared automatically when the measurement is finished
	tba_control.bit.TIMER_START = 1;
	PCIE_SETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS, tba_control.reg);
}

void pcie_ras_tba_stop(void) {
	T_TIME_BASED_ANALYSIS_CONTROL tba_control;

	tba_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS);
	tba_control.bit.TIMER_START = 0;
	PCIE_SETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_CONTROL_OFS, tba_control.reg);
}


UINT32 pcie_ras_tba_data(void) {

	return PCIE_GETDBI(nvtpcie_tba_chip_ptr,TIME_BASED_ANALYSIS_DATA_OFS);
}



void pcie_ras_ec_set(UINT32  ec_sel) {
	T_EVENT_COUNTER_CONTROL ec_control;

	ec_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS);

	ec_control.bit.EVENT_COUNTER_EVENT_SELECT = ec_sel;

	PCIE_SETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS, ec_control.reg);
}

//[sel]
//0x1 (PER_EVENT_OFF): per event off
//0x3 (PER_EVENT_ON): per event on
//0x5 (ALL_OFF): all off
//0x7 (ALL_ON): all on
void pcie_ras_ec_start(UINT32 sel) {
	T_EVENT_COUNTER_CONTROL ec_control;

	if(sel >7)
		sel = 1;

	if((sel ==2)||(sel ==4)||(sel ==6))
		sel = 1;

	ec_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS);
	// This bit will be cleared automatically when the measurement is finished
	ec_control.bit.EVENT_COUNTER_ENABLE = sel;
	PCIE_SETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS, ec_control.reg);
}

//[sel]
//0x0 (NO_CHANGE): no change
//0x1 (PER_CLEAR): per clear
//0x2 (NO_CHANGE_2): no change
//0x3 (ALL_CLEAR): all clear
void pcie_ras_ec_stop(UINT32 sel) {
	T_EVENT_COUNTER_CONTROL ec_control;

	if(sel >3)
		sel = 0;

	ec_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS);
	ec_control.bit.EVENT_COUNTER_CLEAR = sel;
	PCIE_SETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS, ec_control.reg);
}


UINT32 pcie_ras_ec_data(void) {

	return PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_DATA_OFS);
}

u32 glb_pcie_ras_ec(u32  ec_sel) {

	T_EVENT_COUNTER_CONTROL ec_control;
	u32 val;

	spin_lock(&nvtpcie_tba_chip_ptr->lock_ras);

	ec_control.reg = PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS);

	ec_control.bit.EVENT_COUNTER_EVENT_SELECT = ec_sel;

	PCIE_SETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_CONTROL_OFS, ec_control.reg);

	val = PCIE_GETDBI(nvtpcie_tba_chip_ptr,EVENT_COUNTER_DATA_OFS);

	spin_unlock(&nvtpcie_tba_chip_ptr->lock_ras);

	return val;

}
EXPORT_SYMBOL(glb_pcie_ras_ec);

