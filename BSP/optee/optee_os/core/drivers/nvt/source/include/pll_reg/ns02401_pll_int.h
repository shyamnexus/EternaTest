
#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#ifndef CG_REG_TO_BASE_OPTEE
#define CG_REG_TO_BASE_OPTEE(reg)  ((reg - 0x70)/4 * 32)
#endif

// ref from code/hdal/drivers/k_driver/source/rtos_ns02201/pll/pll_int.h
typedef enum {
	//==========================================================================
	//[0x70]
	//==========================================================================
	AXIC_CLK = CG_REG_TO_BASE_OPTEE(0x70),                    ///< AXI clock
	///< Reserved
	WDT_CLK = CG_REG_TO_BASE_OPTEE(0x70) + 2,                 ///< WDT clock
	SYSM_CLK,                                           ///< STSM clock
	///< Reserved
	///< Reserved
	///< Reserved
	///< Reserved
	TCM_CLK = CG_REG_TO_BASE_OPTEE(0x70) + 8,                 ///< TCM clock
	TCM2_CLK,                                           ///< TCM2 clock
	///< Reserved
	///< Reserved
	MAU_CLK = CG_REG_TO_BASE_OPTEE(0x70) + 12,                ///< MAU clock
	MAU2_CLK,                                           ///< MAU2 clock
	PROT_CLK,                                           ///< PROT clock
	HVYLD_CLK,                                          ///< heavyload clock
	HVYLD2_CLK,                                         ///< heavyload2 clock
	HVYLD3_CLK,                                         ///< heavyload3 clock

	//==========================================================================
	//[0x74]
	//==========================================================================
	NAND_CLK = CG_REG_TO_BASE_OPTEE(0x74),                    ///< NAND clock
	SDIO_CLK,                                           ///< SDIO clock
	SDIO2_CLK,                                          ///< SDIO2 clock
	///< Reserved
	GPU_CLK = CG_REG_TO_BASE_OPTEE(0x74) + 4,                 ///< GPU clock
	MSIx_CLK,                                           ///< MSIx clock
	TMR_CLK,                                            ///< timer clock
	HRTMR_CLK,                                          ///< HRTMR clock
	HRTMR2_CLK,                                         ///< HRTMR2 clock
	HRTMR3_CLK,                                         ///< HRTMR3 clock
	HRTMR4_CLK,                                         ///< HRTMR4 clock
	HRTMR5_CLK,                                         ///< HRTMR5 clock
	HRTMR6_CLK,                                         ///< HRTMR6 clock
	HRTMR7_CLK,                                         ///< HRTMR7 clock
	HRTMR8_CLK,                                         ///< HRTMR8 clock
	HRTMR9_CLK,                                         ///< HRTMR9 clock
	HRTMR10_CLK,                                        ///< HRTMR10 clock
	RTC_PSS_CLK,                                        ///< RTC PSS clock
	CRYPTO_CLK,                                         ///< Crypto clock
	HASH_CLK,                                           ///< HASH clock
	RSA_CLK,                                            ///< RSA clock
	ECDSA_CLK,			                                ///< ECDSA clock
	TRNG_CLK,                                           ///< TRNG clock
	TRNG_RO_CLK,                                        ///< TRNG RO clock

	//==========================================================================
	//[0x78]
	//==========================================================================
	UART_CLK = CG_REG_TO_BASE_OPTEE(0x78),                    ///< UART clock
	UART2_CLK,                                          ///< UART2 clock
	UART3_CLK,                                          ///< UART3 clock
	UART4_CLK,                                          ///< UART4 clock
	UART5_CLK,                                          ///< UART5 clock
	I2C_CLK,                                            ///< I2C clock
	I2C2_CLK,                                           ///< I2C2 clock
	I2C3_CLK,                                           ///< I2C3 clock
	I2C4_CLK,                                           ///< I2C4 clock
	I2C5_CLK,                                           ///< I2C5 clock
	I2C6_CLK,                                           ///< I2C6 clock
	I2C7_CLK,                                           ///< I2C7 clock
	I2C8_CLK,                                           ///< I2C8 clock
	REMOTE_CLK,                                         ///< Remote clock
	SDP_CLK,                                            ///< SDP clock
	SPI_CLK,                                            ///< SPI clock
	SPI2_CLK,                                           ///< SPI2 clock
	PWM0_CLK,                                           ///< PWM0 clock
	PWM1_CLK,                                           ///< PWM1 clock
	PWM2_CLK,                                           ///< PWM2 clock
	PWM3_CLK,                                           ///< PWM3 clock
	PWM4_CLK,                                           ///< PWM4 clock
	PWM5_CLK,                                           ///< PWM5 clock
	ADC_CLK,                                            ///< ADC clock
	ADC_TSEN_CLK,                                       ///< ADC T-sensor clock
	SP_CLK,                                             ///< special clock
	SP2_CLK,                                            ///< special2 clock

	//==========================================================================
	//[0x7C]
	//==========================================================================
	USB2_CLK = CG_REG_TO_BASE_OPTEE(0x7C),                    ///< USB2 clock
	USB2_2_CLK,                                         ///< USB2_2 clock
	USB3_CLK,                                           ///< USB3 clock
	USB3_2_CLK,                                         ///< USB3_2 clock
	ETH_CLK,                                            ///< ETH clock
	ETHPHY_CLK,                                         ///< ETH EXT PHY clock
	ETH_XPCS_MAC_CLK,                                   ///< ETH XPCS MAX TX clock
	ETH_XPCS_EEE_CLK,                                   ///< ETH XPCS EEE clock
	ETH2_CLK,                                           ///< ETH2 clock
	ETHPHY2_CLK,                                        ///< ETH2 EXT PHY clock
	ETH2_XPCS_MAC_CLK,                                  ///< ETH2 XPCS MAX TX clock
	ETH2_XPCS_EEE_CLK,                                  ///< ETH2 XPCS EEE clock
	ETH_PTP_REF_CLK,                                    ///< ETH PTP clock
	ETH2_PTP_REF_CLK,                                   ///< ETH2 PTP clock
	///< Reserved
	///< Reserved
	SATA_CLK = CG_REG_TO_BASE_OPTEE(0x7C) + 16,               ///< SATA clock
	SATA2_CLK,                                          ///< SATA2 clock
	SATA3_CLK,                                          ///< SATA3 clock
	SATA4_CLK,                                          ///< SATA4 clock
	SATA5_CLK,                                          ///< SATA5 clock
	SATA6_CLK,                                          ///< SATA6 clock
	///< Reserved
	///< Reserved
	PCIE_CLK = CG_REG_TO_BASE_OPTEE(0x7C) + 24,               ///< PCIE clock
	PCIE2_CLK,                                          ///< PCIE2 clock
	PCIE3_CLK,                                          ///< PCIE3 clock
	PCIE4_CLK,                                          ///< PCIE4 clock

	//==========================================================================
	//[0x80]
	//==========================================================================
	LCD_CLK = CG_REG_TO_BASE_OPTEE(0x80),                     ///< LCD clock
	LCD2_CLK,                                           ///< LCD2 clock
	LCD3_CLK,                                           ///< LCD3 clock
	///< Reserved
	HDMI_VDO_CLK = CG_REG_TO_BASE_OPTEE(0x80) + 4,            ///< HDMI video clock
	HDMI_ADO_CLK,                                       ///< HDMI audio clock
	HDMI2_VDO_CLK,                                      ///< HDMI2 video clock
	HDMI2_ADO_CLK,                                      ///< HDMI2 audio clock
	HDMI3_VDO_CLK,                                      ///< HDMI3 video clock
	HDMI3_ADO_CLK,                                      ///< HDMI3 audio clock
	///< Reserved
	///< Reserved
	DSC_CLK = CG_REG_TO_BASE_OPTEE(0x80) + 12,                ///< DSC clock
	GPENC_CLK,                                          ///< GPENC clock
	GPENC2_CLK,                                         ///< GPENC2 clock
	GPENC3_CLK,                                         ///< GPENC3 clock
	DAI_CLK,                                            ///< DAI clock
	DAI2_CLK,                                           ///< DAI2 clock
	DAI3_CLK,                                           ///< DAI3 clock
	DAI4_CLK,                                           ///< DAI4 clock
	DAI5_CLK,                                           ///< DAI5 clock
	CEC_CLK,                                            ///< CEC clock
	CEC2_CLK,                                           ///< CEC2 clock
	CEC3_CLK,                                           ///< CEC3 clock
	GRAPH_CLK,                                          ///< Graphic clock
	GRAPH2_CLK,                                         ///< Graphic2 clock
	GRAPH3_CLK,                                         ///< Graphic3 clock
	AGE_CLK,                                            ///< AGE clock
	HWCOPY_CLK,                                         ///< Hwcopy clock
	XOR_CLK,                                            ///< XOR clock

	//==========================================================================
	//[0x84]
	//==========================================================================
	CONV_CLK = CG_REG_TO_BASE_OPTEE(0x84),                    ///< CONV clock
	LSU2_CLK,                                           ///< LSU2 clock
	PPU_CLK,                                            ///< PPU clock
	LSU_CLK,                                            ///< LSU clock
	NUE2_CLK,                                           ///< NUE2 clock
	NUE30_CLK,                                          ///< NUE30 clock
	ROU2_CLK,                                           ///< ROU2 clock
	MDBC_CLK,                                           ///< MDBC clock
	IVE_CLK,                                            ///< IVE clock
	///< Reserved
	///< Reserved
	///< Reserved
	///< Reserved
	///< Reserved
	///< Reserved
	///< Reserved
	VENC_CLK = CG_REG_TO_BASE_OPTEE(0x84) + 16,               ///< VENC clock
	///< Reserved
	AD_DEC_CLK = CG_REG_TO_BASE_OPTEE(0x84) + 18,             ///< VDEC AD/AD2 clock
	DEC_CLK,                                            ///< VDEC BIN clock
	JPEG_CLK,                                           ///< JPEG clock
	JPEG2_CLK,                                          ///< JPEG2 clock
	AD_DEC2_CLK,                                        ///< VDEC2 AD clock
	DEC2_CLK,                                           ///< VDEC2 BIN clock
	AD_DEC3_CLK,                                        ///< VDEC3 AD clock
	DEC3_CLK,                                           ///< VDEC3 BIN clock

	//==========================================================================
	//[0x88]
	//==========================================================================
	VPE_CLK = CG_REG_TO_BASE_OPTEE(0x88),                     ///< VPE clock
	VPEL_CLK,                                           ///< VPEL clock
	VPEL2_CLK,                                          ///< VPEL2 clock
	ISE_CLK,                                            ///< ISE clock
	DEI_CLK,                                            ///< DEI clock

	//0x110 start from 1280
	//(0x70 + X/32 * 4) = 0x110
	//0x110 - 0x70 = X/8
	//X=0x500(1280)
	RO_32K_DIV_TESTEN = 1280 + 16,

	//0x140 start from 1664
	//(0x70 + X/32 * 4) = 0x140
	//0x140 - 0x70 = X/8
	//X=0x680(1664)
	RO_32K_CLKEN    = 1664 + 2,
	LVD_RO_CLK12M_CLKEN,

	//0x150 start from 1792
	//(0x70 + X/32 * 4) = 0x150
	//0x150 - 0x70 = X/8
	//X=0x700(1792)

	//==========================================================================
	//[0x150]
	//==========================================================================
	NAND_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x150),
	SDIO_PROG_CLKEN,
	SDIO2_PROG_CLKEN,
	///< Reserved
	MAU_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x150) + 4,
	MAU2_PROG_CLKEN,
	PROT_PROG_CLKEN,
	HVYLD_PROG_CLKEN,
	HVYLD2_PROG_CLKEN,
	HVYLD3_PROG_CLKEN,
	CC_PROG_CLKEN,
	INTC_PROG_CLKEN,
	MSIx_PROG_CLKEN,
	DOORBELL_PROG_CLKEN,
	WDT_PROG_CLKEN,
	TIMER_PROG_CLKEN,
	HRTIMER_PROG_CLKEN,
	HRTIMER2_PROG_CLKEN,
	HRTIMER3_PROG_CLKEN,
	HRTIMER4_PROG_CLKEN,
	HRTIMER5_PROG_CLKEN,
	HRTIMER6_PROG_CLKEN,
	HRTIMER7_PROG_CLKEN,
	HRTIMER8_PROG_CLKEN,
	HRTIMER9_PROG_CLKEN,
	HRTIMER10_PROG_CLKEN,
	RTC_PROG_CLKEN,
	CRYPTO_PROG_CLKEN,
	HASH_PROG_CLKEN,
	RSA_PROG_CLKEN,
	ECDSA_PROG_CLKEN,
	TRNG_PROG_CLKEN,

	//==========================================================================
	//[0x154]
	//==========================================================================
	UART_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x154),
	UART2_PROG_CLKEN,
	UART3_PROG_CLKEN,
	UART4_PROG_CLKEN,
	UART5_PROG_CLKEN,
	I2C_PROG_CLKEN,
	I2C2_PROG_CLKEN,
	I2C3_PROG_CLKEN,
	I2C4_PROG_CLKEN,
	I2C5_PROG_CLKEN,
	I2C6_PROG_CLKEN,
	I2C7_PROG_CLKEN,
	I2C8_PROG_CLKEN,
	REMOTE_PROG_CLKEN,
	SDP_PROG_CLKEN,
	SPI_PROG_CLKEN,
	SPI2_PROG_CLKEN,
	PWM_PROG_CLKEN,
	ADC_PROG_CLKEN,
	TEMP_SENSOR_PROG_CLKEN,

	//==========================================================================
	//[0x158]
	//==========================================================================
	USB2_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x154),
	USB2_2_PROG_CLKEN,
	USB3_PROG_CLKEN,
	USB3_2_PROG_CLKEN,
	ETH_PROG_CLKEN,
	ETH2_PROG_CLKEN,
	SATA_PROG_CLKEN,
	SATA2_PROG_CLKEN,
	SATA3_PROG_CLKEN,
	SATA4_PROG_CLKEN,
	SATA5_PROG_CLKEN,
	SATA6_PROG_CLKEN,
	PCIE_PROG_CLKEN,
	PCIE2_PROG_CLKEN,
	PCIE3_PROG_CLKEN,
	PCIE4_PROG_CLKEN,

	//==========================================================================
	//[0x15C]
	//==========================================================================
	LCD_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x15C),
	LCD2_PROG_CLKEN,
	LCD3_PROG_CLKEN,
	HDMI_PROG_CLKEN,
	HDMI2_PROG_CLKEN,
	HDMI3_PROG_CLKEN,
	DSC_PROG_CLKEN,
	GPENC_PROG_CLKEN,
	GPENC2_PROG_CLKEN,
	GPENC3_PROG_CLKEN,
	DAI_PROG_CLKEN,
	DAI2_PROG_CLKEN,
	DAI3_PROG_CLKEN,
	DAI4_PROG_CLKEN,
	DAI5_PROG_CLKEN,
	GRAPHIC_PROG_CLKEN,
	GRAPHIC2_PROG_CLKEN,
	GRAPHIC3_PROG_CLKEN,
	AGE_PROG_CLKEN,
	HWCP_PROG_CLKEN,
	XOR_PROG_CLKEN,

	//==========================================================================
	//[0x160]
	//==========================================================================
	CONV_PROG_CLKEN = CG_REG_TO_BASE_OPTEE(0x160),
	PPU_PROG_CLKEN,
	LSU_PROG_CLKEN,
	NUE2_PROG_CLKEN,
	ROU_PROG_CLKEN,
	CAL_PROG_CLKEN,
	UTIL_PROG_CLKEN,
	ROU2_PROG_CLKEN,
	JM_PROG_CLKEN,
	JMISP_PROG_CLKEN,
	MDBC_PROG_CLKEN,
	IVE_PROG_CLKEN,
	VENC_PROG_CLKEN,
	AD_DEC_PROG_CLKEN,
	JPEG_PROG_CLKEN,
	JPEG2_PROG_CLKEN,
	VPE_PROG_CLKEN,
	VPEL_PROG_CLKEN,
	VPEL2_PROG_CLKEN,
	ISE_PROG_CLKEN,
	AD_DEC2_PROG_CLKEN,
	AD_DEC3_PROG_CLKEN,
	LSU2_PROG_CLKEN,
	DEI_PROG_CLKEN,

	EN_MAXNUM,
	MAX_CLK = EN_MAXNUM,
	ENUM_DUMMY4WORD(CG_EN)
} CG_EN;

typedef enum {
	PLL_ID_0        = 0,        ///< PLL0  (for AXI0)
	PLL_ID_1        = 1,        ///< PLL1  (for internal 480 MHz)
	PLL_ID_2        = 2,        ///< PLL2  (for REF_500)
	PLL_ID_3        = 3,        ///< PLL3  (for DRAM)
	PLL_ID_4        = 4,        ///< PLL4  (for )
	PLL_ID_5        = 5,        ///< PLL5  (for GPU)
	PLL_ID_6        = 6,        ///< PLL6  (for SDC)
	PLL_ID_7        = 7,        ///< PLL7  (for ETH)
	PLL_ID_8        = 8,        ///< PLL8  (for LCD)
	PLL_ID_9        = 9,        ///< PLL9  (for LCD2)
	PLL_ID_10       = 10,       ///< PLL10 (for LCD3)
	PLL_ID_11       = 11,       ///< PLL11 (for LCD_MCLK)
	PLL_ID_12       = 12,       ///< PLL12 (for AUDIO)
	PLL_ID_13       = 13,       ///< PLL13 (for REF_400)
	PLL_ID_14       = 14,       ///< PLL14 (for USB3_PHY)
	PLL_ID_15       = 15,       ///< PLL15 (for HDMI2P1 (phy ref))
	PLL_ID_16       = 16,       ///< PLL16 (for CPU (CA55))
	PLL_ID_17       = 17,       ///< PLL17 (for REF_600)
	PLL_ID_18       = 18,       ///< PLL18 (for ETH_SERDES)
	PLL_ID_19       = 19,       ///< PLL19 (for AXI1)
	PLL_ID_20       = 20,       ///< PLL20 (for )
	PLL_ID_21       = 21,       ///< PLL21 (for CPU2 (CA76))
	PLL_ID_22       = 22,       ///< PLL22 (for )
	PLL_ID_23       = 23,       ///< PLL23 (for VDEC3 (for AV1))
	PLL_ID_24       = 24,       ///< PLL24 (for PCIE_PHY)
	PLL_ID_25       = 25,       ///< PLL25 (for SATA_PHY_SSC)
	PLL_ID_26       = 26,       ///< PLL26 (for IPP)
	PLL_ID_27       = 27,       ///< PLL27 (for VDEC2 (for BIN))
	PLL_ID_28       = 28,       ///< PLL28 (for CNN)
	PLL_ID_29       = 29,       ///< PLL29 (for CNN2)
	PLL_ID_30       = 30,       ///< PLL30 (for VENC)
	PLL_ID_31       = 31,       ///< PLL31 (for VDEC (for AD))
	PLL_ID_32       = 32,       ///< PLL32 (for DRAM2)

	PLL_ID_FIXED320 = 33,       ///< Fixed 320MHz PLL

	PLL_ID_MAX,
	ENUM_DUMMY4WORD(PLL_ID)
} PLL_ID;

// MASK define in pll_protected