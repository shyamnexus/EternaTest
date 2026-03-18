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
	INT_ID_VPELITE,
	INT_ID_DMACP,
	INT_ID_VPE,

	INT_ID_VCAP,
	INT_ID_VCAP2,
	INT_ID_DEI,
	INT_ID_VENC,
	INT_ID_VENC2,
	INT_ID_VDEC,
	INT_ID_OSG,
	INT_ID_ETHERNET2,
	INT_ID_ETHERNET,
	INT_ID_SATA0,
	INT_ID_SATA1,
	INT_ID_DAI,
	INT_ID_AGE,
	INT_ID_JPEG,
	INT_ID_LCD,
	INT_ID_LCD2,
	INT_ID_LCD3,
	INT_ID_DEI2,
	INT_ID_GPENC,
	INT_ID_GPENC2,
	INT_ID_GPIO,
	INT_ID_REMOTE,
	INT_ID_PWM,
	INT_ID_USB,
	INT_ID_USB3,
	INT_ID_NAND,
	INT_ID_SDIO,
	INT_ID_PCIE,    //31

	INT_ID_ETHERNET_TX,
	INT_ID_MAU,
	INT_ID_DAI2,
	INT_ID_DAI3,
	INT_ID_DAI4,
	INT_ID_DAI5,
	INT_ID_UART4,
	INT_ID_ETHERNET_RX,
	INT_ID_ETHERNET2_RX,
	INT_ID_I2C,
	INT_ID_I2C2,
	INT_ID_UART,
	INT_ID_UART2,
	INT_ID_UART3,
	INT_ID_HASH,    //46
	INT_ID_UART5,
	INT_ID_RSA,     //48
	INT_ID_ETHERNET2_TX,
	INT_ID_I2C4,
	INT_ID_TVE100,
	INT_ID_HDMI,
	INT_ID_MAU2,
	INT_ID_CSI,
	INT_ID_CSI2,
	INT_ID_RTC,
	INT_ID_WDT,
	INT_ID_CG,
	INT_ID_CSI3,
	INT_ID_I2C3,
	INT_ID_SDIO2,
	INT_ID_GPIO2,
	INT_ID_GPIO3,   //63

	INT_ID_CSI4,
	INT_ID_CSI5,
	INT_ID_CSI6,
	INT_ID_CSI7,
	INT_ID_CSI8,
	INT_ID_SATA3,
	INT_ID_HWCP,
	INT_ID_IVE,
	INT_ID_NUE,
	INT_ID_NUE2,
	INT_ID_SSCA,
	INT_ID_CNN,
	INT_ID_CNN2,
	INT_ID_CNN3,
	INT_ID_USB2,
	INT_ID_CRYPTO,  //79
	INT_ID_GPIO4,
	INT_ID_CC,
	INT_ID_CC1,
	INT_ID_CC2,
	INT_ID_CC3,

	INT_ID_RSV0,
	INT_ID_RSV1,
	INT_ID_RSV2,
	INT_ID_RSV3,
	INT_ID_RSV4,
	INT_ID_RSV5,
	INT_ID_RSV6,
	INT_ID_RSV7,
	INT_ID_RSV8,
	INT_ID_RSV9,
	INT_ID_SUDO_INT_MSIx,	//95

	INT_ID_MSIx_0,
	INT_ID_MSIx_1,
	INT_ID_MSIx_2,
	INT_ID_MSIx_3,

	INT_ID_MSIx_4,
	INT_ID_MSIx_5,
	INT_ID_MSIx_6,
	INT_ID_MSIx_7,

	INT_ID_MSIx_8,
	INT_ID_MSIx_9,
	INT_ID_MSIx_10,
	INT_ID_MSIx_11,

	INT_ID_MSIx_12,
	INT_ID_MSIx_13,
	INT_ID_MSIx_14,
	INT_ID_MSIx_15,

	INT_ID_MSIx_16,
	INT_ID_MSIx_17,
	INT_ID_MSIx_18,
	INT_ID_MSIx_19,

	INT_ID_MSIx_20,
	INT_ID_MSIx_21,
	INT_ID_MSIx_22,
	INT_ID_MSIx_23,

	INT_ID_MSIx_24,
	INT_ID_MSIx_25,
	INT_ID_MSIx_26,
	INT_ID_MSIx_27,

	INT_ID_MSIx_28,
	INT_ID_MSIx_29,
	INT_ID_MSIx_30,
	INT_ID_MSIx_31,
	//
	//
	//

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
