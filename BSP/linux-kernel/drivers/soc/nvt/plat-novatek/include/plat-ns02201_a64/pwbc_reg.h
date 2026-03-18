/*
	Header file for pwbc controller register

	This file is the header file that define register for PWBC module

	@file       pwbc_reg.h
	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PWBC_REG_H
#define _PWBC_REG_H

#include <linux/soc/nvt/rcw_macro.h>

/*
	@addtogroup mIHALRTC
*/

// RTC Power Button Control Register
#define PWBC_CTRL_REG_OFS 0x00
union PWBC_CTRL_REG {
	uint32_t reg;
	struct {
	unsigned int reset_sdt_timer:1;
	unsigned int pwr_off:1;
	unsigned int reserved0:3;
	unsigned int pwr_en2_ctrl:1;
	unsigned int pwr_en3_ctrl:1;
	unsigned int reserved1:17;
	unsigned int pwr_sw1_inten:1;
	unsigned int pwr_sw2_inten:1;
	unsigned int pwr_sw3_inten:1;
	unsigned int pwr_sw4_inten:1;
	unsigned int reserved2:4;
	} bit;
};

// RTC Power Button Control Status Register
#define PWBC_STS_REG_OFS 0x04
union PWBC_STS_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int pwronsrc_sw1:1;
	unsigned int pwronsrc_sw2:1;
	unsigned int pwronsrc_sw3:1;
	unsigned int pwronsrc_sw4:1;
	unsigned int pwron_pwren2_log:1;
	unsigned int reserved1:2;
	unsigned int pwronsrc_pwralarm:1;
	unsigned int pwr_sw1_pin:1;
	unsigned int pwr_sw2_pin:1;
	unsigned int pwr_sw3_pin:1;
	unsigned int pwr_sw4_pin:1;	
	unsigned int pwr_en2_sts:1;	
	unsigned int pwr_en3_sts:1;	
	unsigned int pwbc_cset_done:1;	
	unsigned int pwbc_ready:1;	
	unsigned int pwr_sw1_int_sts:1;
	unsigned int pwr_sw2_int_sts:1;
	unsigned int pwr_sw3_int_sts:1;
	unsigned int pwr_sw4_int_sts:1;	
	unsigned int reserved2:4;
	} bit;
};

/*PWBC Sequence Time Register*/
#define PWBC_SEQTIME_REG_OFS 0x08
union PWBC_SEQTIME_REG {
	uint32_t reg;
	struct {
	unsigned int pwr_on_seq_time1:8;
	unsigned int pwr_on_seq_time2:8;
	unsigned int pwr_off_seq_time1:8;
	unsigned int pwr_off_seq_time2:8;
	} bit;
};

/* PWBC Control Register2 */
#define PWBC_CTRL2_REG_OFS 0x10
union PWBC_CTRL2_REG {
	uint32_t reg;
	struct {
	unsigned int pwbc_cset:1;
	unsigned int pwbc_cset_inten:1;
	unsigned int pwr_en2_ctrl_sel:1;
	unsigned int pwr_en3_ctrl_sel:1;
	unsigned int seq_time1_sel:1;
	unsigned int seq_time2_sel:1;
	unsigned int seq_time3_sel:1;
	unsigned int seq_time4_sel:1;
	unsigned int pwbc_ana_sel:1;
	unsigned int reserved1:23;
	} bit;
};

/*PWBC OSC Analog Register*/
#define PWBC_OSCAN_REG_OFS 0x24
union PWBC_OSCAN_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int osc_analogcfg:8;
	unsigned int pwbc_period:8;
	unsigned int pwbc_autoread_period:8;
	} bit;
};
#endif
