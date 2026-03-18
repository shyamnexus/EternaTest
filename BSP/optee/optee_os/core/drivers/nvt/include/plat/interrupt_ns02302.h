/**
    Header file for Interrupt module

    This file is the header file that define the API for Interrupt module.

    @file       Interrupt.h
    @ingroup    mIDrvSys_Interrupt
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
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
	INT_ID_TIMER = INT_GIC_SPI_START_ID,    // 0
	INT_ID_SIE,                             // 1
	INT_ID_SIE2,                            // 2
	INT_ID_SIE3,                            // 3

	INT_ID_DRTC,                            // 4
	INT_ID_IPE,                             // 5
	INT_ID_IME,                             // 6
	INT_ID_DCE,                             // 7

	INT_ID_IFE,                             // 8
	INT_ID_VPEL,                            // 9
	INT_ID_DIS,                             //10
	INT_ID_CNN,                             //11

	INT_ID_SDP,                             //12
	INT_ID_NUE2,                            //13
	INT_ID_NUE,                             //14
	INT_ID_DAI,                             //15

	INT_ID_HEVC,                            //16
	INT_ID_JPEG,                            //17
	INT_ID_GRAPHIC,                         //18
	INT_ID_GRAPHIC2,                        //19

	INT_ID_RSA,                             //20
	INT_ID_ISE,                             //21
	INT_ID_TGE,                             //22
	INT_ID_TSE,                             //23

	INT_ID_GPIO,                            //24
	INT_ID_REMOTE,                          //25
	INT_ID_PWM,                             //26
	INT_ID_USB3,                            //27

	INT_ID_HASH,                            //28
	INT_ID_NAND,                            //29
	INT_ID_SDIO,                            //30
	INT_ID_SDIO2,                           //31

	INT_ID_SDIO3,                           //32
	INT_ID_DMA,                             //33
	INT_ID_ETHERNET,                        //34
	INT_ID_SPI,                             //35

	INT_ID_SPI2,                            //36
	INT_ID_SPI3,                            //37
	INT_ID_CRYPTO,                          //38
	INT_ID_TZPC,                            //39

	INT_ID_SIF,                             //40
	INT_ID_I2C,                             //41
	INT_ID_I2C2,                            //42
	INT_ID_UART,                            //43

	INT_ID_UART2,                           //44
	INT_ID_UART3,                           //45
	INT_ID_MDBC,                            //46
	INT_ID_ADC,                             //47

	INT_ID_IDE,                             //48
	INT_ID_CSI_TX,                          //49
	INT_ID_DSI,                             //50
	INT_ID_TRKE,                            //51

	INT_ID_CC,                              //52
	INT_ID_IVE,                             //53 (o)
	INT_ID_LVDS,                            //54
	INT_ID_LVDS2,                           //55

	INT_ID_RTC,                             //56
	INT_ID_WDT,                             //57
	INT_ID_CG,                              //58
	INT_ID_SIE4,                            //59

	INT_ID_I2C3,                            //60
	INT_ID_SIE5,                            //61
	INT_ID_VPE,                             //62
	INT_ID_UART4,                           //63
	INT_ID_UART5,                           //64
	INT_ID_UART6,                           //65
	INT_ID_SPI4,                            //66
	INT_ID_SPI5,                            //67
	INT_ID_I2C4,                            //68
	INT_ID_I2C5,                            //69
	INT_ID_TIMER2,                          //70
	INT_ID_GPIO2,                           //71
	INT_ID_SIF2,                            //72
	INT_ID_PWM2,                            //73
	INT_ID_SDE,                             //74
	INT_ID_TIMER64_1,                       //75
	INT_ID_TIMER64_2,                       //76
	INT_ID_CC2,                             //77
	INT_ID_CC3,                             //78
	INT_ID_CC4,                             //79
	INT_ID_DRE,                             //80
	INT_ID_JPEG2,                           //81
	INT_ID_DSP,                             //82
	INT_ID_I2C6,                            //83
	INT_ID_I2C7,                            //84
	INT_ID_I2C8,                            //85
	INT_ID_I2C9,                            //86
	INT_ID_I2C10,                           //87
	INT_ID_I2C11,                           //88
	INT_ID_IDE2,                            //89
	INT_ID_UART7,                           //90
	INT_ID_USB,                             //91
	INT_ID_HDMI,                            //92
	INT_ID_MI,                              //92
	INT_ID_HVTLOAD,                         //94
	INT_ID_PWBC,                            //95
	INT_ID_UART8,                           //96
	INT_ID_UART9,                           //97
	INT_ID_LVDS3,                           //98
	INT_ID_LVDS4,                           //99
	INT_ID_LVDS5,                           //100
	INT_ID_UVCP,                            //101
	INT_ID_DAI2,                            //102
	INT_ID_CNN2,                          //103
	INT_ID_GPENC,                           //104
	INT_ID_GPENC2,                          //105
	INT_ID_SATA,                            //106
	INT_ID_ISE2,                            //107
	INT_ID_SIE6,                            //108
	INT_ID_VIE,                             //109
	INT_ID_VIE2,                            //110
	INT_ID_UVCP2,                           //111
	INT_ID_ETHERNET2,                       //112
	INT_ID_GRAPHIC3,                        //113
	INT_ID_ETHERNET2_RX0,                   //114
	INT_ID_GPIO3,                           //115
	INT_ID_GPIO4,                           //116
	INT_ID_GPIO5,                           //117
	INT_ID_VTR,                             //118
	INT_ID_ETHERNET_RX0,                    //119
	INT_ID_ETHERNET_RX1,                    //120
	INT_ID_ETHERNET_RX2,                    //121
	INT_ID_ETHERNET_RX3,                    //122
	INT_ID_ETHERNET_TX0,                    //123
	INT_ID_ETHERNET_TX1,                    //124
	INT_ID_ETHERNET_TX2,                    //125
	INT_ID_ETHERNET_TX3,                    //126
	INT_ID_HWCP,                            //127
	INT_ID_ETHERNET2_RX1,           //128
	INT_ID_ETHERNET2_RX2,           //129
	INT_ID_ETHERNET2_RX3,           //130
	INT_ID_ETHERNET2_TX0,           //131
	INT_ID_ETHERNET2_TX1,           //132
	INT_ID_ETHERNET2_TX2,           //133
	INT_ID_ETHERNET2_TX3,           //134
	INT_ID_SIF3,                    //135
	INT_ID_VDEC,                    //136
	INT_ID_PMC,                     //137
	INT_ID_JPEG_LITE,               //138


	INT_ID_CNT,
	INT_ID_MAX = INT_ID_CNT - INT_GIC_SPI_START_ID, //128
	INT_ID_DUMMY_DGPIO,

	INT_ID_WFI = 229,
	INT_ID_GIC_TOTAL = 256,
	ENUM_DUMMY4WORD(INT_ID)
} INT_ID;

typedef union {
	UINT64 reg[4];
	struct {
		UINT32  bInt_ID_PPI: 32;
		//INT0
		UINT32  bInt_ID_TIMER: 1;
		UINT32  bInt_ID_SIE: 1;
		UINT32  bInt_ID_SIE2: 1;
		UINT32  bInt_ID_SIE3: 1;

		UINT32  bInt_ID_DRTC: 1;
		UINT32  bInt_ID_IPE: 1;
		UINT32  bInt_ID_IME: 1;
		UINT32  bInt_ID_DCE: 1;

		UINT32  bInt_ID_IFE: 1;
		UINT32  bInt_ID_VPEL: 1;
		UINT32  bInt_ID_DIS: 1;
		UINT32  bInt_ID_CNN: 1;

		UINT32  bInt_ID_SDP: 1;
		UINT32  bInt_ID_NUE2: 1;
		UINT32  bInt_ID_NUE: 1;
		UINT32  bInt_ID_DAI: 1;


		//INT16
		UINT32  bInt_ID_HEVC: 1;
		UINT32  bInt_ID_JPEG: 1;
		UINT32  bInt_ID_GRAPHIC: 1;
		UINT32  bInt_ID_GRAPHIC2: 1;

		UINT32  bInt_ID_RSA: 1;
		UINT32  bInt_ID_ISE: 1;
		UINT32  bInt_ID_TGE: 1;
		UINT32  bInt_ID_TSMUX: 1;

		UINT32  bInt_ID_GPIO: 1;
		UINT32  bInt_ID_REMOTE: 1;
		UINT32  bInt_ID_PWM: 1;
		UINT32  bInt_ID_USB3: 1;

		UINT32  bInt_ID_HASH: 1;
		UINT32  bInt_ID_NAND: 1;
		UINT32  bInt_ID_SDIO: 1;
		UINT32  bInt_ID_SDIO2: 1;

		//INT32
		UINT32  bInt_ID_SDIO3: 1;
		UINT32  bInt_ID_DMA: 1;
		UINT32  bInt_ID_ETHERNET: 1;
		UINT32  bInt_ID_SPI: 1;

		UINT32  bInt_ID_SPI2: 1;
		UINT32  bInt_ID_SPI3: 1;
		UINT32  bInt_ID_SCE: 1;
		UINT32  bInt_ID_TZPC: 1;

		UINT32  bInt_ID_SIF: 1;
		UINT32  bInt_ID_I2C: 1;
		UINT32  bInt_ID_I2C2: 1;
		UINT32  bInt_ID_UART: 1;

		UINT32  bInt_ID_UART2: 1;
		UINT32  bInt_ID_UART3: 1;
		UINT32  bInt_ID_MDBC: 1;
		UINT32  bInt_ID_ADC: 1;

		//INT48
		UINT32  bInt_ID_IDE: 1;
		UINT32  bInt_ID_CNN2: 1;
		UINT32  bInt_ID_DSI: 1;
		UINT32  bInt_ID_TRKE: 1;

		//INT52
		UINT32  bInt_ID_CC: 1;
		UINT32  bInt_ID_IVE: 1;
		UINT32  bInt_ID_LVDS: 1;        // LVDS& HiSPI& CSI
		UINT32  bInt_ID_LVDS2: 1;       //LVDS2&HiSPI2&CSI2

		//INT56
		UINT32  bInt_ID_RTC: 1;
		UINT32  bInt_ID_WDT: 1;
		UINT32  bInt_ID_CG: 1;
		UINT32  bInt_ID_SIE4: 1;        //@NT98528

		//INT60
		UINT32  bInt_ID_I2C3: 1;
		UINT32  bInt_ID_SIE5: 1;        //@NT98528
		UINT32  bInt_ID_VPE: 1;         //@NT98528
		UINT32  bInt_ID_UART4: 1;       //@NT98528

		//INT64
		UINT32  bInt_ID_UART5: 1;       //@NT98528
		UINT32  bInt_ID_UART6: 1;       //@NT98528
		UINT32  bInt_ID_SPI4: 1;        //@NT98528
		UINT32  bInt_ID_SPI5: 1;        //@NT98528

		//INT68
		UINT32  bInt_ID_I2C4: 1;
		UINT32  bInt_ID_I2C5: 1;
		UINT32  bInt_ID_TIMER2: 1;
		UINT32  bInt_ID_GPIO2: 1;

		//INT72
		UINT32  bInt_ID_SIF2: 1;
		UINT32  bInt_ID_PWM2: 1;
		UINT32  bInt_ID_SDE: 1;
		UINT32  bInt_ID_TIMER64_1: 1;

		//INT76
		UINT32  bInt_ID_TIMER64_2: 1;
		UINT32  bInt_ID_CC2: 1;
		UINT32  bInt_ID_CC3: 1;
		UINT32  bInt_ID_CC4: 1;

		//INT80
		UINT32  bInt_ID_DRE: 1;
		UINT32  bInt_ID_JPEG2: 1;
		UINT32  bInt_ID_DSP: 1;
		UINT32  bInt_ID_I2C6: 1;

		//INT84
		UINT32  bInt_ID_I2C7: 1;
		UINT32  bInt_ID_I2C8: 1;
		UINT32  bInt_ID_I2C9: 1;
		UINT32  bInt_ID_I2C10: 1;

		//INT88
		UINT32  bInt_ID_I2C11: 1;
		UINT32  bInt_ID_IDE2: 1;
		UINT32  bInt_ID_UART7: 1;
		UINT32  bInt_ID_USB: 1;

		//INT92
		UINT32  bInt_ID_HDMI: 1;
		UINT32  bInt_ID_MI: 1;
		UINT32  bInt_ID_HVTLOAD: 1;
		UINT32  bInt_ID_PWBC: 1;

		//INT96
		UINT32  bInt_ID_UART8: 1;
		UINT32  bInt_ID_UART9: 1;
		UINT32  bInt_ID_CSI3: 1;
		UINT32  bInt_ID_CSI4: 1;

		//INT100
		UINT32  bInt_ID_CSI5: 1;
		UINT32  bInt_ID_UVCP: 1;
		UINT32  bInt_ID_DAI2: 1;
		UINT32  bInt_ID_CSI_TX: 1;

		//INT104
		UINT32  bInt_ID_GPENC: 1;
		UINT32  bInt_ID_GPENC2: 1;
		UINT32  bInt_ID_SATA: 1;
		UINT32  bInt_ID_ISE2: 1;

		//INT108
		UINT32  bInt_ID_SIE6: 1;
		UINT32  bInt_ID_VIE: 1;
		UINT32  bInt_ID_VIE2: 1;
		UINT32  bInt_ID_UVCP2: 1;

		//INT112
		UINT32  bInt_ID_ETHERNET2: 1;
		UINT32  bInt_ID_GRAPHIC3: 1;
		UINT32  bInt_ID_ETHERNET2_RX0: 1;
		UINT32  bInt_ID_GPIO3: 1;

		//INT116
		UINT32  bInt_ID_GPIO4: 1;
		UINT32  bInt_ID_GPIO5: 1;
		UINT32  bInt_ID_VTR: 1;
		UINT32  bInt_ID_ETHERNET_RX0: 1;

		//INT120
		UINT32  bInt_ID_ETHERNET_RX1: 1;
		UINT32  bInt_ID_ETHERNET_RX2: 1;
		UINT32  bInt_ID_ETHERNET_RX3: 1;
		UINT32  bInt_ID_ETHERNET_TX0: 1;

		//INT124
		UINT32  bInt_ID_ETHERNET_TX1: 1;
		UINT32  bInt_ID_ETHERNET_TX2: 1;
		UINT32  bInt_ID_ETHERNET_TX3: 1;
		UINT32  bInt_ID_HWCP: 1;

		//INT128
		UINT32  bInt_ID_ETHERNET2_RX1: 1;
		UINT32  bInt_ID_ETHERNET2_RX2: 1;
		UINT32  bInt_ID_ETHERNET2_RX3: 1;
		UINT32  bInt_ID_ETHERNET2_TX0: 1;

		//INT132
		UINT32  bInt_ID_ETHERNET2_TX1: 1;
		UINT32  bInt_ID_ETHERNET2_TX2: 1;
		UINT32  bInt_ID_ETHERNET2_TX3: 1;
		UINT32  bInt_ID_SIF3: 1;


	} Bit;
} INT_GIC_ID_ENABLE, *PINT_GIC_ID_ENABLE;

typedef struct {
	INT_GIC_ID_ENABLE  int_id_enable;
} INT_INTC_ENABLE, *PINT_INTC_ENABLE;

#define IRQF_TRIGGER_NONE       0x00000000
#define IRQF_TRIGGER_RISING     0x00000001
#define IRQF_TRIGGER_FALLING    0x00000002
#define IRQF_TRIGGER_HIGH       0x00000004
#define IRQF_TRIGGER_LOW        0x00000008


typedef enum {
	IRQF_BH_PRI_LOW     =   0x00000100,
	IRQF_BH_PRI_MIDDLE  =   0x00000200,
	IRQF_BH_PRI_HIGH    =   0x00000400,

	IRQF_BH_PRI_DEFAULT =   IRQF_BH_PRI_MIDDLE,
	ENUM_DUMMY4WORD(IRQF_BH)
} IRQF_BH;



enum irqreturn {
	IRQ_NONE            = (0 << 0),
	IRQ_HANDLED         = (1 << 0),
	IRQ_WAKE_THREAD     = (1 << 1),
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
