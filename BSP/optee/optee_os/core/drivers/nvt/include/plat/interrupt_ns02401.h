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
	INT_ID_TIMER = INT_GIC_SPI_START_ID,    // 0
	INT_ID_SIE,                             // 1
	INT_ID_SIE2,                            // 2
	INT_ID_SIE3,                            // 3

	INT_ID_DRTC,                            // 4
	INT_ID_IPE,                             // 5
	INT_ID_IME,                             // 6
	INT_ID_SLVS_EC,                         // 7

	INT_ID_IFE,                             // 8
	INT_ID_VPE2,                            // 9
	INT_ID_LME,                             //10
	INT_ID_NUE2,                            //11

	INT_ID_SDP,                             //12
	INT_ID_CONV,                            //13
	INT_ID_CONV2,                           //14
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
	INT_ID_CONV3,                           //49
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
	INT_ID_CSI_TX,                          //103
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
	INT_ID_LCD_LITE,                        //117
	INT_ID_VTR,                             //118
	INT_ID_ETHERNET_RX0,                    //119
	INT_ID_ETHERNET_RX1,                    //120
	INT_ID_ETHERNET_RX2,                    //121
	INT_ID_ETHERNET_RX3,                    //122
	INT_ID_ETHERNET_TX0,                    //123
	INT_ID_ETHERNET_TX1,                    //124
	INT_ID_ETHERNET_TX2,                    //125
	INT_ID_ETHERNET_TX3,                    //126
	INT_ID_XOR,				//127
	INT_ID_ETHERNET2_RX1,           	//128
	INT_ID_ETHERNET2_RX2,           	//129
	INT_ID_ETHERNET2_RX3,           	//130
	INT_ID_ETHERNET2_TX0,           	//131
	INT_ID_ETHERNET2_TX1,           	//132
	INT_ID_ETHERNET2_TX2,           	//133
	INT_ID_ETHERNET2_TX3,           	//134
	INT_ID_SIF3,                    	//135
	INT_ID_VDEC,                    	//136
	INT_ID_PMC,                     	//137
	INT_ID_JPEG_LITE,               	//138
	INT_ID_AGE,				//139
	INT_ID_SIF4,				//140
	INT_ID_DAI3,				//141
	INT_ID_SIE7,				//142
	INT_ID_SIE8,				//143
	INT_ID_SIE9,				//144
	INT_ID_SIE10,				//145
	INT_ID_SIE11,				//146
	INT_ID_SIE12,				//147
	INT_ID_SIE13,				//148
	INT_ID_SIE14,				//149
	INT_ID_SIE15,				//150
	INT_ID_SIE16,				//151
	INT_ID_VIE3,				//152
	INT_ID_VIE4,				//153
	INT_ID_VPE3,				//154
	INT_ID_IME2,				//155
	INT_ID_IPE2,				//156
	INT_ID_IFE2,				//157
	INT_ID_DSI2,				//158
	INT_ID_DSP2,				//159
	INT_ID_TIMER64_3,                       //160
	INT_ID_TIMER64_4,                       //161
	INT_ID_TIMER64_5,                       //162
	INT_ID_DMA2,                       	//163
	INT_ID_DOORBELL,                       	//164
	INT_ID_LVDS6,				//165
	INT_ID_LVDS7,				//166
	INT_ID_LVDS8,				//167
	INT_ID_MALI_EVENT,			//168
	INT_ID_MALI_JOB,			//169
	INT_ID_MALI_MMU,			//170
	INT_ID_MALI_GPU,			//171
	INT_ID_I2C12,				//172
	INT_ID_I2C13,				//173
	INT_ID_I2C14,				//174
	INT_ID_I2C15,				//175
	INT_ID_I2C16,				//176
	INT_ID_I2C17,				//177
	INT_ID_I2C18,				//178
	INT_ID_I2C19,				//179
	INT_ID_I2C20,				//180
	INT_ID_SATA2,				//181
	INT_ID_SATA3,				//182
	INT_ID_CONV4,				//183
	INT_ID_UTIL,				//184
	INT_ID_ROU,				//185
	INT_ID_LSU,				//186
	INT_ID_CAL,				//187
	INT_ID_JOBM,				//188
	INT_ID_PWM3,				//189
	INT_ID_PWM4,				//190
	INT_ID_USB3_2,				//191
	INT_ID_DAI4,				//192
	INT_ID_TIMER64_6,                       //193
	INT_ID_MCU_ERR,                       	//194
	INT_ID_SLVS_EC2,			//195
	INT_ID_HWCP,				//196
	INT_ID_PCIE,				//197
	INT_ID_PCIE2,				//198

	INT_ID_MSIx_0 = INT_GIC_SPI_START_ID + 224,	//224
	INT_ID_MSIx_1,					//225
	INT_ID_MSIx_2,					//226
	INT_ID_MSIx_3,					//227
	INT_ID_MSIx_4,					//228
	INT_ID_MSIx_5,					//229
	INT_ID_MSIx_6,					//230
	INT_ID_MSIx_7,					//231
	INT_ID_MSIx_8,					//232
	INT_ID_MSIx_9,					//233
	INT_ID_MSIx_10,					//234
	INT_ID_MSIx_11,					//235
	INT_ID_MSIx_12,					//236
	INT_ID_MSIx_13,					//237
	INT_ID_MSIx_14,					//238
	INT_ID_MSIx_15,					//239

	INT_ID_MSIx2_0,					//240
	INT_ID_MSIx2_1,					//241
	INT_ID_MSIx2_2,					//242
	INT_ID_MSIx2_3,					//243
	INT_ID_MSIx2_4,					//244
	INT_ID_MSIx2_5,					//245
	INT_ID_MSIx2_6,					//246
	INT_ID_MSIx2_7,					//247
	INT_ID_MSIx2_8,					//248
	INT_ID_MSIx2_9,					//249
	INT_ID_MSIx2_10,				//250
	INT_ID_MSIx2_11,				//251
	INT_ID_MSIx2_12,				//252
	INT_ID_MSIx2_13,				//253
	INT_ID_MSIx2_14,				//254
	INT_ID_MSIx2_15,				//255


	INT_ID_CNT,
	INT_ID_MAX = INT_ID_CNT - INT_GIC_SPI_START_ID, //224
	INT_ID_DUMMY_DGPIO,

	INT_ID_WFI = INT_GIC_SPI_START_ID + 261,
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
