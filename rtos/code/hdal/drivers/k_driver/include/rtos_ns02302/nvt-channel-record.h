/**
    SRAM Control header file
    This file will handle core communications.
    @file       nvt-sramctl.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef _NA51102_NVT_CHANNEL_RECORD_H
#define _NA51102_NVT_CHANNEL_RECORD_H

//#define ENUM_DUMMY4WORD(m)
#include <kwrap/nvt_type.h>


/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {
	/*0x1000*/
	CHREC_CPU 		= 0x00,
	CHREC_HVYLD 	= 0x01,
	CHREC_HVYLD2	=0x02,
	CHREC_PERI		=0x03,
	CHREC_PG8		=0x04,
	CHREC_PGEDAP	=0x05,
	CHREC_NULL_6	=0x06,
	CHREC_NULL_7	=0x07,
	CHREC_MAU_0 	=0x08,
	CHREC_MAU_1 	=0x09,
	CHREC_MAU_2		=0x0A,
	CHREC_MAU_3		=0x0B,
	CHREC_MAU_4		=0x0C,
	CHREC_MAU_5		=0x0D,
	CHREC_MAU_6		=0x0E,
	CHREC_MAU_7		=0x0F,

	CHREC_ETH		=0x10,
	CHREC_ETH2I		=0x11,
	CHREC_HWCP		=0x12,
	CHREC_LARB		=0x13,
	CHREC_LARB2		=0x14,
	CHREC_LARB3		=0x15,
	CHREC_DAI		=0x16,
	CHREC_GPENC		=0x17,
	CHREC_TSE		=0x18,
	CHREC_HASH		=0x19,
	CHREC_SCE		=0x1A,
	CHREC_UVCP		=0x1B,
	CHREC_UVCP2		=0x1C,
	CHREC_DAI2		=0x1D,
	CHREC_GPENC2	=0x1E,
	CHREC_SDIO		=0x1F,

	CHREC_SDIO2		=0x20,
	CHREC_SDIO3		=0x21,
	CHREC_SMC		=0x22,
	CHREC_IFE		=0x23,
	CHREC_IDE		=0x24,
	CHREC_IDE2		=0x25,
	CHREC_CSITX		=0x26,
	CHREC_NULL_27	=0x27,
	CHREC_NULL_28	=0x28,
	CHREC_NULL_29	=0x29,
	CHREC_NULL_2A	=0x2A,
	CHREC_NULL_2B	=0x2B,
	CHREC_NULL_2C	=0x2C,
	CHREC_NULL_2D	=0x2D,
	CHREC_NULL_2E	=0x2E,
	CHREC_NULL_2F	=0x2F,

	CHREC_VPE_0		=0x30,
	CHREC_VPE_1		=0x31,
	CHREC_DIS		=0x32,
	CHREC_DRE		=0x33,
	CHREC_SDE		=0x34,
	CHREC_SIE_1		=0x35,
	CHREC_SIE2_1	=0x36,
	CHREC_SIE3_1	=0x37,
	CHREC_SIE4_1	=0x38,
	CHREC_SIE5_1	=0x39,
	CHREC_SIE6_1	=0x3A,
	CHREC_SIE_0		=0x3B,
	CHREC_SIE2_0	=0x3C,
	CHREC_SIE3_0	=0x3D,
	CHREC_SIE4_0	=0x3E,
	CHREC_SIE5_0	=0x3F,
	CHREC_SIE6_0	=0x40,
	CHREC_VIE		=0x41,
	CHREC_VIE2		=0x42,

	CHREC_NULL_43	=0x43,
	CHREC_NULL_44	=0x44,
	CHREC_NULL_45	=0x45,
	CHREC_NULL_46	=0x46,
	CHREC_NULL_47	=0x47,
	CHREC_CNN		=0x48,
	CHREC_CNN2		=0x49,
	CHREC_DCE		=0x4A,
	CHREC_NUE		=0x4B,
	CHREC_NUE2		=0x4C,
	CHREC_NULL_4D	=0x4D,
	CHREC_NULL_4E	=0x4E,
	CHREC_NULL_4F	=0x4F,
	CHREC_IPE		=0x50,
	CHREC_IME		=0x51,

	CHREC_NULL_52	=0x52,
	CHREC_NULL_53	=0x53,
	CHREC_NULL_54	=0x54,
	CHREC_NULL_55	=0x55,
	CHREC_NULL_56	=0x56,
	CHREC_NULL_57	=0x57,
	CHREC_NULL_58	=0x58,
	CHREC_NULL_59	=0x59,
	CHREC_NULL_5A	=0x5A,
	CHREC_NULL_5B	=0x5B,
	CHREC_NULL_5C	=0x5C,
	CHREC_NULL_5D	=0x5D,
	CHREC_NULL_5E	=0x5E,
	CHREC_NULL_5F	=0x5F,

	CHREC_JPEG		=0x60,
	CHREC_JPEG2		=0x61,
	CHREC_VENC_0	=0x62,
	CHREC_VENC_1	=0x63,
	CHREC_HVYLD3	=0x64,
	CHREC_NULL_65	=0x65,
	CHREC_NULL_66	=0x66,
	CHREC_NULL_67	=0x67,
	CHREC_NULL_68	=0x68,
	CHREC_NULL_69	=0x69,
	CHREC_NULL_6A	=0x6A,
	CHREC_NULL_6B	=0x6B,
	CHREC_NULL_6C	=0x6C,
	CHREC_NULL_6D	=0x6D,
	CHREC_NULL_6E	=0x6E,
	CHREC_NULL_6F	=0x6F,

	CHREC_EDP		=0x70,
	CHREC_EPP		=0x71,
	CHREC_IOP		=0x72,
	CHREC_NULL_73	=0x73,
	CHREC_NULL_74	=0x74,
	CHREC_NULL_75	=0x75,
	CHREC_NULL_76	=0x76,
	CHREC_NULL_77	=0x77,
	CHREC_NULL_78	=0x78,
	CHREC_NULL_79	=0x79,
	CHREC_NULL_7A	=0x7A,
	CHREC_NULL_7B	=0x7B,
	CHREC_NULL_7C	=0x7C,
	CHREC_NULL_7D	=0x7D,
	CHREC_NULL_7E	=0x7E,
	CHREC_NULL_7F	=0x7F,


	CHREC_USB		=0x80,
	CHREC_USB3		=0x81,
	CHREC_SATA		=0x82,
	CHREC_NULL_83	=0x83,
	CHREC_NULL_84	=0x84,
	CHREC_NULL_85	=0x85,
	CHREC_NULL_86	=0x86,
	CHREC_NULL_87	=0x87,
	CHREC_NULL_88	=0x88,
	CHREC_NULL_89	=0x89,
	CHREC_NULL_8A	=0x8A,
	CHREC_NULL_8B	=0x8B,
	CHREC_NULL_8C	=0x8C,
	CHREC_NULL_8D	=0x8D,
	CHREC_NULL_8E	=0x8E,
	CHREC_NULL_8F	=0x8F,

	CHREC_ISE		=0x90,
	CHREC_IVE		=0x91,
	CHREC_MDBC		=0x92,
	CHREC_TRKE		=0x93,
	CHREC_GRAPHIC	=0x94,
	CHREC_GRAPHIC2	=0x95,
	CHREC_GRAPHIC3	=0x96,
	CHREC_VPEL_0	=0x97,
	CHREC_VPEL_1	=0x98,
	CHREC_DEC_0		=0x99,
	CHREC_DEC_1		=0x9A,
	CHREC_DEC_2		=0x9B,
	CHREC_DEC_3		=0x9C,
	CHREC_JPEGL		=0x9D,
	CHREC_NUM,

	//ENUM_DUMMY4WORD(SRAM_SD)
} CHREC_SEL;


/*
    SRAM ShutDown ID

    This is for nvt_disable_sram_shutdown() and nvt_enable_sram_shutdown().
*/
typedef enum {
	CHREC_OSEL_0_31 = 0x0,
	CHREC_OSEL_32_34,
}CHREC_OSEL;


// 0x1094 Channel record reg
#define TOP_CHANNEL_STATUS_REG_OFS			(0x1000 + 0x190)
union TOP_CHANNEL_STATUS_REG0 {
	UINT32 reg;
	struct {
	unsigned int RCNT:8;              	 	// Record channel select
	unsigned int RCNT_M:8;             		// Record channel output select
	unsigned int WCNT:8;
	unsigned int WCNT_M:8;
	} bit;
};

union TOP_CHANNEL_STATUS_REG1 {
	UINT32 reg;
	struct {
	unsigned int RCH_EN:1;              	// Record channel select
	unsigned int WCH_EN:1;             		// Record channel output select
	unsigned int RCH_IDLE:1;
	unsigned int WCH_IDLE:1;
	unsigned int reserved1:28;
	} bit;
};
// 0x1094 Channel record reg
#define TOP_CHANNEL_RECORD_REG_OFS			(0x1000 + 0x194)
union TOP_CHANNEL_RECORD_REG {
	UINT32 reg;
	struct {
	unsigned int CHREC_SEL:8;               // Record channel select
	unsigned int CHREC_OSEL:1;             	// Record channel output select
	unsigned int reserved1:23;
	} bit;
};

typedef struct {
	CHREC_SEL       chrec_sel;
	CHAR           *name;
} REC_CH_INFO, *PREC_CH_INFO;

extern void nvt_channel_record_output_sel(CHREC_SEL sel, CHREC_OSEL osel);
extern void nvt_channel_record_show_result(void);

#endif /* NVT_SRAMCTL_H */
