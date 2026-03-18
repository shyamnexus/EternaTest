/**
    Header file for Interrupt module

    This file is the header file that define the API for Interrupt module.

    @file       Interrupt.h
    @ingroup    mIDrvSys_Interrupt
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2024.  All rights reserved.
*/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <kwrap/nvt_type.h>

/**
    Interrupt module ID

    Interrupt module ID for int_getIRQId() and int_getDummyId().
*/
#define INT_GIC_SPI_START_ID    32

typedef enum {
	INT_ID_TIMER = INT_GIC_SPI_START_ID,
	INT_ID_SIE,
	INT_ID_SIE2,
	INT_ID_SIE3,

	INT_ID_DRTC,
	INT_ID_IPE,
	INT_ID_IME,
	INT_ID_DCE,

	INT_ID_IFE,
	INT_ID_IFE2,
	INT_ID_DIS,
	INT_ID_FDE,

	INT_ID_SDP,             //SDP
	INT_ID_RHE,
	INT_ID_DRE,
	INT_ID_DAI,             //15

	INT_ID_H26X,
	INT_ID_JPEG,
	INT_ID_GRAPHIC,
	INT_ID_GRAPHIC2,

	INT_ID_RSA,
	INT_ID_ISE,
	INT_ID_TGE,
	INT_ID_TSE,

	INT_ID_GPIO,
	INT_ID_REMOTE,
	INT_ID_PWM,
	INT_ID_USB,

	INT_ID_HASH,
	INT_ID_NAND,
	INT_ID_SDIO,
	INT_ID_SDIO2,           //31

	INT_ID_SDIO3,
	INT_ID_DMA,
	INT_ID_ETHERNET,
	INT_ID_SPI,

	INT_ID_SPI2,
	INT_ID_SPI3,
	INT_ID_CRYPTO,
	INT_ID_ETH_REV_MII,     //Ethernet_RevMII

	INT_ID_SIF,
	INT_ID_I2C,
	INT_ID_I2C2,
	INT_ID_UART,            //43

	INT_ID_UART2,
	INT_ID_UART3,
	INT_ID_UART4,
	INT_ID_ADC,

	INT_ID_IDE,
	INT_ID_IDE2,
	INT_ID_DSI,
	INT_ID_DMA2,
	INT_ID_AFFINE,          //51

	INT_ID_HDMI,
//  INT_ID_VX1,             //Removed @ NT96680
	INT_ID_LVDS,
	INT_ID_LVDS2,           //55

	INT_ID_RTC,
	INT_ID_WDT,
	INT_ID_CG,
	INT_ID_CC,              //59

	INT_ID_I2C3,            //60
//  New add @ NT96680
	INT_ID_SLVS_EC,
	INT_ID_LVDS3,           //share with HiSPi3/CSI3
	INT_ID_LVDS4,           //share with HiSPi4/CSI4

	INT_ID_LVDS5,           //share with HiSPi5/CSI5
	INT_ID_LVDS6,           //share with HiSPi6/CSI6
	INT_ID_LVDS7,           //share with HiSPi7/CSI7
	INT_ID_LVDS8,           //share with HiSPi8/CSI8

	INT_ID_SIE5,
	INT_ID_SIE6,
	INT_ID_SIE7,
	INT_ID_SIE8,

	INT_ID_IVE,
	INT_ID_SVM,
	INT_ID_SDE,
	INT_ID_CNN,             //75

	INT_ID_DSP,
	INT_ID_DSP2,
	INT_ID_CANBUS,
	//79

	INT_ID_COPY,
	INT_ID_ROTATE,
	INT_ID_I2C4,
	INT_ID_I2C5,            //83
	INT_ID_ISE2,
	INT_ID_TIMER2,
	INT_ID_GPIO2,           //87

	INT_ID_PWM2,
	INT_ID_CC2,             //89
	INT_ID_SIF2,
	INT_ID_DSP_2,
	INT_ID_DSP_2_2,
	INT_ID_DUMMY_DGPIO,
	INT_ID_CNT,
	INT_ID_MAX = INT_ID_CNT - INT_GIC_SPI_START_ID, //94


	INT_ID_WFI = 229,
	INT_ID_GIC_TOTAL = 256,
	ENUM_DUMMY4WORD(INT_ID)
} INT_ID;

#define IRQF_TRIGGER_NONE		0x00000000
#define IRQF_TRIGGER_RISING		0x00000001
#define IRQF_TRIGGER_FALLING	0x00000002
#define IRQF_TRIGGER_HIGH		0x00000004
#define IRQF_TRIGGER_LOW		0x00000008


typedef enum {
	IRQF_BH_PRI_LOW		=	0x00000100,
	IRQF_BH_PRI_MIDDLE	=	0x00000200,
	IRQF_BH_PRI_HIGH	=	0x00000400,

	IRQF_BH_PRI_DEFAULT	=	IRQF_BH_PRI_MIDDLE,
	ENUM_DUMMY4WORD(IRQF_BH)
}IRQF_BH;



enum irqreturn {
	IRQ_NONE			= (0 << 0),
	IRQ_HANDLED			= (1 << 0),
	IRQ_WAKE_THREAD		= (1 << 1),
};

typedef enum irqreturn irqreturn_t;
typedef irqreturn_t (* irq_handler_t)(int irq, void *dev);
typedef irqreturn_t (* irq_bh_handler_t)(int irq, unsigned long event, void *data);


extern void	irq_init(void);
extern int	request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev);
extern void	free_irq(unsigned int irq, void *dev);

extern int	request_irq_bh(unsigned int irq, irq_bh_handler_t bh_handler, IRQF_BH flags);
extern void	free_irq_bh(unsigned int irq, void *dev);
extern int	kick_bh(unsigned int irq, unsigned long event, void *data);

//@}

#endif
