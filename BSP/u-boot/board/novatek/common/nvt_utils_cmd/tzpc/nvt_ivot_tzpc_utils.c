#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
#include <serial.h>
#include "nvt_ivot_tzpc_utils.h"
#include "nvt_ivot_protected.h"


#ifdef CONFIG_TARGET_NA51102_A64
#define TOP_SRAM_SD_RSA_BIT_OFS     9
#define CHIP_ID_BASE     0x2F00100F0
static TZPC_NAME tz_pc_NA51102[] = {
	{CPU_TZPC_TOP_CTRL,                 "  TOP controller"},
	{CPU_TZPC_CG_CTRL,                  "  CKG controller"},
	{CPU_TZPC_PAD_CTRL,                 "  PAD controller"},
	{CPU_TZPC_TIMER_CTRL,               "Timer controller"},
	{CPU_TZPC_WDT_CTRL,                  " WDT controller"},
	{CPU_TZPC_RTC_CTRL,                 "  RTC controller"},
	{CPU_TZPC_GPIO_CTRL,                " GPIO controller"},


	{CPU_TZPC_INTC_CTRL,                 "INTC controller"},
	{CPU_TZPC_CC_CTRL,                   "  CC controller"},
	{CPU_TZPC_HEAVY_LOAD_CTRL,    " Heavy load controller"},
	{CPU_TZPC_DRTC_CTRL,                " DRTC controller"},
	{CPU_TZPC_TZPC_CTRL,                " TZPC controller"},
	{CPU_TZPC_MAU_CTRL,                  " MAU controller"},

	{CPU_TZPC_RTC_PWBC_CTRL,             " RTC controller"},
	{CPU_TZPC_TIMER2_CTRL,             "Timer2 controller"},
	{CPU_TZPC_TIMER3_CTRL,             "Timer3 controller"},
	{CPU_TZPC_HEAVY_LOAD2_CTRL, " Heavy load 2 controller"},
	{CPU_TZPC_HEAVY_LOAD3_CTRL, " Heavy load 3 controller"},
	{CPU_TZPC_I2C6_CTRL,                " I2C6 controller"},
	{CPU_TZPC_PWM_CTRL,                  " PWM controller"},
	{CPU_TZPC_I2C_CTRL,                  " I2C controller"},
	{CPU_TZPC_SPI_CTRL,                  " SPI controller"},

	{CPU_TZPC_SIF_CTRL,                 "  SIF controller"},
	{CPU_TZPC_REMOTE_CTRL,            " Remote controller"},
	{CPU_TZPC_ADC_CTRL,                  " ADC controller"},
	{CPU_TZPC_LVDS_CTRL,                " LVDS controller"},
	{CPU_TZPC_MIPI_CSI_CTRL,            "  CSI controller"},
	{CPU_TZPC_UART_CTRL,                " UART controller"},
	{CPU_TZPC_ADC2_CTRL,                " ADC2 controller"},
	{CPU_TZPC_ETHERNET_CTRL,         "Ethernet controller"},
	{CPU_TZPC_I2C7_CTRL,                 "I2C7 controller"},
	{CPU_TZPC_I2C8_CTRL,                 "I2C8 controller"},
	{CPU_TZPC_I2C9_CTRL,                 "I2C9 controller"},
	{CPU_TZPC_I2C10_CTRL,               "I2C10 controller"},
	{CPU_TZPC_UART2_CTRL,               "UART2 controller"},
	{CPU_TZPC_UART3_CTRL,               "UART3 controller"},
	{CPU_TZPC_SPI2_CTRL,                 "SPI2 controller"},
	{CPU_TZPC_MIPI_CSI2_CTRL,            "CSI2 controller"},
	{CPU_TZPC_SPI3_CTRL,                 "SPI3 controller"},

	{CPU_TZPC_I2C2_CTRL,                 "I2C2 controller"},
	{CPU_TZPC_SPI4_CTRL,                 "SPI4 controller"},
	{CPU_TZPC_LVDS2_CTRL,               "LVDS2 controller"},

	{CPU_TZPC_UART4_CTRL,               "UART4 controller"},
	{CPU_TZPC_SDP_CTRL,                 "  SDP controller"},
	{CPU_TZPC_I2C3_CTRL,                 "I2C3 controller"},
	{CPU_TZPC_I2C4_CTRL,                 "I2C4 controller"},
	{CPU_TZPC_I2C5_CTRL,                 "I2C5 controller"},
	{CPU_TZPC_SPI5_CTRL,                 "SPI5 controller"},
	{CPU_TZPC_UART5_CTRL,               "UART5 controller"},
	{CPU_TZPC_UART6_CTRL,               "UART6 controller"},

	{CPU_TZPC_NAND_CTRL,                " NAND controller"},
	{CPU_TZPC_I2C11_CTRL,               "I2C11 controller"},
	{CPU_TZPC_SDIO_CTRL,                " SDIO controller"},

	{CPU_TZPC_ETHERNET2_CTRL,       "Ethernet2 controller"},
	{CPU_TZPC_UART7_CTRL,               "UART7 controller"},
	{CPU_TZPC_UART8_CTRL,               "UART8 controller"},
	{CPU_TZPC_UART9_CTRL,               "UART9 controller"},
	{CPU_TZPC_MIPI_CSI3_CTRL,           " CSI3 controller"},
	{CPU_TZPC_MIPI_CSI4_CTRL,           " CSI4 controller"}, //58

	{CPU_TZPC_MIPI_CSI5_CTRL,           " CSI5 controller"},
	{CPU_TZPC_LVDS3_CTRL,               "LVDS3 controller"},
	{CPU_TZPC_LVDS4_CTRL,               "LVDS4 controller"},
	{CPU_TZPC_LVDS5_CTRL,               "LVDS5 controller"},
	{CPU_TZPC_SDIO2_CTRL,               "SDIO2 controller"},
	{CPU_TZPC_SDIO3_CTRL,               "SDIO3 controller"},
	{CPU_TZPC_STBC_CKG_CTRL,        " STBC_CKG controller"},//65
	{CPU_TZPC_SATA_CTRL,                " SATA controller"},

	{CPU_TZPC_USB2_CTRL,                " USB2 controller"},
	{CPU_TZPC_USB3_CTRL,                " USB3 controller"},
	{CPU_TZPC_PMC_CTRL,                  " PMC controller"},//68
	{CPU_TZPC_SATACTRL_CTRL,             "SATA controller"},
	{CPU_TZPC_USB2PHY_CTRL,         " USB2 phy controller"},
	{CPU_TZPC_USB3PHY_CTRL,         " USB3 phy controller"},
	{CPU_TZPC_HWCP_CTRL,              "  HWCPY controller"},//73
	{CPU_TZPC_SCE_CTRL,                 "  SCE controller"},//74
	{CPU_TZPC_DAI_CTRL,                 "  DAI controller"},
	{CPU_TZPC_EAC_CTRL,                 "  EAC controller"},

	{CPU_TZPC_TSE_CTRL,                  " TSE controller"},//77
	{CPU_TZPC_EFUSE_CTRL,               "EFUSE controller"},//78
	{CPU_TZPC_HASG_CTRL,                " HASH controller"},
	{CPU_TZPC_TRNG_CTRL,                " TRNG controller"},
	{CPU_TZPC_UVCP_CTRL,                 "UVCP controller"},
	{CPU_TZPC_RSA_CTRL,                   "RSA controller"},
	{CPU_TZPC_SENPHY_CTRL,             "SENPHY controller"},
	{CPU_TZPC_SENPHY2_CTRL,           "SENPHY2 controller"},
	{CPU_TZPC_SENPHY3_CTRL,           "SENPHY3 controller"},
	{CPU_TZPC_DDR_PHY_CTRL,           "DDR phy controller"},
	{CPU_TZPC_DAI2_PHY_CTRL,             "DAT2 controller"},

	{CPU_TZPC_VCAP2_CTRL,               "VCAP2 controller"},



	{CPU_TZPC_IDE1_CTRL,                 "IDE1 controller"},
	{CPU_TZPC_MI_CTRL,                     "MI controller"},
	{CPU_TZPC_HDMI_CTRL,                 "HDMI controller"},


	{CPU_TZPC_MIPI_DSI_CTRL,              "DSI controller"},
	{CPU_TZPC_MIPI_CSI_TX_CTRL,        "CSI_TX controller"},
	{CPU_TZPC_GPENC_CTRL,               "GPENC controller"},
	{CPU_TZPC_GPENC2_CTRL,             "GPENC2 controller"},

	{CPU_TZPC_IDE2_CTRL,                 "IDE2 controller"},
	{CPU_TZPC_TRKE_CTRL,                 "TRKE controller"},

	{CPU_TZPC_JPEG_CTRL,                 "JPEG controller"},
	{CPU_TZPC_VENC_CTRL,                 "VENC controller"},
	{CPU_TZPC_VDEC_CTRL,                 "VDEC controller"},
	{CPU_TZPC_JPEG_LITE_CTRL,       "JPEG lite controller"},
	{CPU_TZPC_JPEG2_CTRL,               "JPEG2 controller"},

	{CPU_TZPC_SIE_CTRL,                   "SIE controller"},
	{CPU_TZPC_MDBC_CTRL,                 "MDBC controller"},
	{CPU_TZPC_DCE_CTRL,                   "DCE controller"},

	{CPU_TZPC_IPE_CTRL,                   "IPE controller"},
	{CPU_TZPC_IME_CTRL,                   "IME controller"},
	{CPU_TZPC_DIS_CTRL,                   "DSI controller"},
	{CPU_TZPC_NUE_CTRL,                   "NUE controller"},
	{CPU_TZPC_IFE_CTRL,                   "IFE controller"},


	{CPU_TZPC_GRAPHIC_CTRL,              "Grph controller"},
	{CPU_TZPC_ISE_CTRL,                   "ISE controller"},
	{CPU_TZPC_SIE6_CTRL,                 "SIE6 controller"},
	{CPU_TZPC_CNN_CTRL,                   "CNN controller"},
	{CPU_TZPC_TGE_CTRL,                   "TGE controller"},
	{CPU_TZPC_VPE_CTRL,                   "VPE controller"},
	{CPU_TZPC_ISE2_CTRL,                 "ISE2 controller"},

	{CPU_TZPC_DRE_CTRL,                   "DRE controller"},

	{CPU_TZPC_VPE_LITE_CTRL,         "VPE lite controller"},
	{CPU_TZPC_GRAPHIC2_CTRL,        " Graphic2 controller"},
	{CPU_TZPC_SIE2_CTRL,                 "SIE2 controller"},
	{CPU_TZPC_SIE3_CTRL,                 "SIE3 controller"},
	{CPU_TZPC_SIE4_CTRL,                 "SIE4 controller"},
	{CPU_TZPC_NUE2_CTRL,                 "NUE2 controller"},
	{CPU_TZPC_CNN2_CTRL,                 "CNN2 controller"},
	{CPU_TZPC_IVE_CTRL,                  " IVE controller"},
	{CPU_TZPC_SIE5_CTRL,                 "SIE5 controller"},
	{CPU_TZPC_SDE_CTRL,                  " SDE controller"},
	{CPU_TZPC_GRAPHIC3_CTRL,        " Graphic3 controller"},
	{CPU_TZPC_VIE_CTRL,                  " VIE controller"},
	{CPU_TZPC_VIE2_CTRL,                 "VIE2 controller"},
	{CPU_TZPC_DSP_CTRL,                   "DSP controller"},
};
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
#define TOP_SRAM_SD_RSA_BIT_OFS     9
#define CHIP_ID_BASE     0x2F00100F0
static TZPC_NAME tz_pc_NS02201[] = {
	{CPU_TZPC_TOP_CTRL,                   "TOP controller"},
	{CPU_TZPC_CG_CTRL,                    "CKG controller"},
	{CPU_TZPC_PAD_CTRL,                   "PAD controller"},
	{CPU_TZPC_GPIO_CTRL,                 "GPIO controller"},

	{CPU_TZPC_WDT_CTRL,                   "WDT controller"},    //4
	{CPU_TZPC_TIMER_CTRL,               "TIMER controller"},
	{CPU_TZPC_CC_CTRL,                     "CC controller"},
	{CPU_TZPC_PWM_CTRL,                   "PWM controller"},

	{CPU_TZPC_I2C_CTRL,                   "I2C controller"},
	{CPU_TZPC_EAC_CTRL,                   "EAC controller"},
	{CPU_TZPC_TSE_CTRL,                   "TSE controller"},
	{CPU_TZPC_N25_CTRL,                   "N25 controller"},

	{CPU_TZPC_AGE_CTRL,                   "AGE controller"},
	{CPU_TZPC_MAU_CTRL,                   "MAU controller"},
	{CPU_TZPC_MAU2_CTRL,                 "MAU2 controller"},
	{CPU_TZPC_TZPC_CTRL,                 "TZPC controller"},

	{CPU_TZPC_HEAVY_LOAD_CTRL,            "HVY controller"},    //16
	{CPU_TZPC_EFUSE_CTRL,               "eFUSE controller"},
	{CPU_TZPC_SCE_CTRL,                   "SCE controller"},
	{CPU_TZPC_HASH_CTRL,                 "HASH controller"},

	{CPU_TZPC_RSA_CTRL,                   "RSA controller"},
	{CPU_TZPC_HWCPY_CTRL,               "HWCPY controller"},
	{CPU_TZPC_XOR_CTRL,                   "XOR controller"},
	{CPU_TZPC_ISE_CTRL,                   "ISE controller"},


	{CPU_TZPC_ISE2_CTRL,                 "ISE2 controller"},
	{CPU_TZPC_DRE_CTRL,                   "DRE controller"},
	{CPU_TZPC_MIPI_CSI_CTRL,              "CSI controller"},
	{CPU_TZPC_SLVSEC_CTRL,             "SLVSEC controller"},

	{CPU_TZPC_SENPHY_CTRL,             "SENPHY controller"},
	{CPU_TZPC_SENPHY2_CTRL,           "SENPHY2 controller"},
	{CPU_TZPC_TGE_CTRL,                   "TGE controller"},
	{CPU_TZPC_SIE_CTRL,                   "SIE controller"},

	{CPU_TZPC_VIE_CTRL,                   "VIE controller"},    //32
	{CPU_TZPC_SLVSEC2_CTRL,           "SLVSEC2 controller"},
	{CPU_TZPC_SLVSECPHY_CTRL,       "SLVSECPHY controller"},
	{CPU_TZPC_LCD_CTRL,                   "LCD controller"},

	{CPU_TZPC_LCD2_CTRL,                 "LCD2 controller"},
	{CPU_TZPC_LCD3_CTRL,                 "LCD3 controller"},
	{CPU_TZPC_MI_CTRL,                     "MI controller"},
	{CPU_TZPC_HDMI_CTRL,                 "HDMI controller"},

	{CPU_TZPC_HDMI2_CTRL,               "HDMI2 controller"},
	{CPU_TZPC_ETH_CTRL,                   "ETH controller"},
	{CPU_TZPC_ETH2_CTRL,                 "ETH2 controller"},
	{CPU_TZPC_CONV_CTRL,                 "CONV controller"},

	{CPU_TZPC_CONV2_CTRL,               "CONV2 controller"},
	{CPU_TZPC_CONV3_CTRL,               "CONV3 controller"},
	{CPU_TZPC_CONV4_CTRL,               "CONV4 controller"},
	{CPU_TZPC_JOBM_CTRL,                 "JOBM controller"},

	{CPU_TZPC_UTIL_CTRL,                 "UTIL controller"},
	{CPU_TZPC_ROU_CTRL,                   "ROU controller"},
	{CPU_TZPC_CAL_CTRL,                   "CAL controller"},
	{CPU_TZPC_LSU_CTRL,                   "LSU controller"},

	{CPU_TZPC_LME_CTRL,                   "LME controller"},    //52
	{CPU_TZPC_SDE_CTRL,                   "SDE controller"},
	{CPU_TZPC_IVE_CTRL,                   "IVE controller"},
	{CPU_TZPC_MDBC_CTRL,                 "MDBC controller"},

	{CPU_TZPC_TRKE_CTRL,                 "TRKE controller"},
	{CPU_TZPC_NUE2_CTRL,                 "NUE2 controller"},
	{CPU_TZPC_MALI_CTRL,                 "MALI controller"},
	{CPU_TZPC_DSP_CTRL,                   "DSP controller"},

	{CPU_TZPC_OCEM_CTRL,                 "OCEM controller"},
	{CPU_TZPC_DSP2_CTRL,                 "DSP2 controller"},
	{CPU_TZPC_OCEM2_CTRL,               "OCEM2 controller"},
	{CPU_TZPC_VDEC_CTRL,                 "VDEC controller"},

	{CPU_TZPC_JPEG_CTRL,                 "JPEG controller"},    //64
	{CPU_TZPC_JPEG2_CTRL,               "JPEG2 controller"},
	{CPU_TZPC_JPEG_LITE_CTRL,           "JPEGL controller"},
	{CPU_TZPC_USB2_CTRL,                 "USB2 controller"},

	{CPU_TZPC_USB3_CTRL,                 "USB3 controller"},
	{CPU_TZPC_USB2_A_CTRL,             "USB2_A controller"},
	{CPU_TZPC_USB3_A_CTRL,             "USB3_A controller"},
	{CPU_TZPC_SDIO3_CTRL,               "SDIO3 controller"},

	{CPU_TZPC_NAND_CTRL,                 "NAND controller"},
	{CPU_TZPC_DSI_CTRL,                   "DSI controller"},
	{CPU_TZPC_DSI2_CTRL,                 "DSI2 controller"},
	{CPU_TZPC_CSITX_CTRL,               "CSITX controller"},

	{CPU_TZPC_CSITX2_CTRL,             "CSITX2 controller"},    //76
	{CPU_TZPC_DSIPHY_CTRL,             "DSIPHY controller"},
	{CPU_TZPC_VENC_CTRL,                "VENC controller"},
	{CPU_TZPC_VTRC_CTRL,                "VTRC controller"},

	{CPU_TZPC_TSEN_CTRL,                "TSEN controller"},
	{CPU_TZPC_VPE3_CTRL,                "VPE3 controller"},
	{CPU_TZPC_USB3_2_CTRL,            "USB3_2 controller"},
	{CPU_TZPC_SATA_CTRL,                "SATA controller"},

	{CPU_TZPC_SATA2_CTRL,              "SATA2 controller"},
	{CPU_TZPC_PCIE_CTRL,                "PCIE controller"},
	{CPU_TZPC_PCIE2_CTRL,              "PCIE2 controller"},
	{CPU_TZPC_RTC_CTRL,                  "RTC controller"},

	{CPU_TZPC_PWBC_CTRL,                "PWBC controller"},
	{CPU_TZPC_USB3_2_A_CTRL,        "USB3_2_A controller"},
	{CPU_TZPC_SATA_A_CTRL,            "SATA_A controller"},
	{CPU_TZPC_SATA2_A_CTRL,          "SATA2_A controller"},

	{CPU_TZPC_ADC_CTRL,                  "ADC controller"},
	{CPU_TZPC_PCIEPHY_L0_CTRL,     "PCIEPHYL0 controller"},
	{CPU_TZPC_PCIEPHY_L1_CTRL,     "PCIEPHYL1 controller"},
	{CPU_TZPC_IFE_CTRL,                  "IFE controller"},

	{CPU_TZPC_VPE_CTRL,                  "VPE controller"},
	{CPU_TZPC_IPE_CTRL,                  "IPE controller"},
	{CPU_TZPC_IME_CTRL,                  "IME controller"},
	{CPU_TZPC_SYS_SRAM_CTRL,                    "SYSSRAM"},     //99

	{CPU_TZPC_IFE_SRAM_CTRL,             "IFE controller"},     //100
	{CPU_TZPC_IFE2_SRAM_CTRL,           "IFE2 controller"},
	{CPU_TZPC_IFE3_SRAM_CTRL,           "IFE3 controller"},
	{CPU_TZPC_IFE4_SRAM_CTRL,           "IFE4 controller"},

	{CPU_TZPC_IFE5_SRAM_CTRL,           "IFE5 controller"},
	{CPU_TZPC_IFE6_SRAM_CTRL,           "IFE6 controller"},
	{CPU_TZPC_IFE7_SRAM_CTRL,           "IFE7 controller"},
	{CPU_TZPC_IFE8_SRAM_CTRL,           "IFE8 controller"},

	{CPU_TZPC_IFE_CTRL,                  "IFE controller"},     //108
	{CPU_TZPC_VPE2_CTRL,                "VPE2 controller"},

	{CPU_TZPC_TCM_CTRL,                  "TCM controller"},     //110
	{CPU_TZPC_TCM2_CTRL,                "TCM2 controller"},
	{CPU_TZPC_TCM3_CTRL,                "TCM3 controller"},
	{CPU_TZPC_TCM4_CTRL,                "TCM4 controller"},
	{CPU_TZPC_MALI_SRAM_CTRL,           "MALI controller"},
	{CPU_TZPC_N25_SRAM_CTRL,             "N25 controller"},
	{CPU_TZPC_PCIE_DBI1_CTRL,          "PCIE1 controller"},
	{CPU_TZPC_PCIE_DBI2_CTRL,          "PCIE2 controller"},
	{CPU_TZPC_EDAP1_CTRL,              "EDAP1 controller"},
	{CPU_TZPC_EDAP2_CTRL,              "EDAP2 controller"},
};
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
#define TOP_SRAM_SD_RSA_BIT_OFS     9
#define CHIP_ID_BASE     0x2F00100F0
static TZPC_NAME tz_pc_NS02302[] = {
	{CPU_TZPC_TOP_CTRL,                   "TOP controller"},
	{CPU_TZPC_CG_CTRL,                    "CKG controller"},
	{CPU_TZPC_PAD_CTRL,                   "PAD controller"},
	{CPU_TZPC_GPIO_CTRL,                 "GPIO controller"},

	{CPU_TZPC_TEST_CTRL,     			 "Test controller"},    //4
	{CPU_TZPC_TIMER_CTRL,               "TIMER controller"},
	{CPU_TZPC_UART_CTRL,                 "UART controller"},
    {CPU_TZPC_I2C_CTRL,     			  "I2C controller"},
	
	{CPU_TZPC_SPI_CTRL,					  "SPI controller"},
	{CPU_TZPC_PWM_CTRL,     			  "PWM controller"},
	{CPU_TZPC_EAC_CTRL,     			  "EAC controller"},
	{CPU_TZPC_EFUSE_CTRL,               "Efuse controller"},

	{CPU_TZPC_HVYLD_CTRL,   		"Heavyload controller"},
	{CPU_TZPC_MAU_CTRL,     			  "MAU controller"},
	{CPU_TZPC_IDE_CTRL,     			  "IDE controller"},
	{CPU_TZPC_MI_CTRL,      			   "MI controller"},
	
    {CPU_TZPC_ETH_CTRL,					  "ETH controller"},
	{CPU_TZPC_MCU_CTRL,					  "MCU controller"},    //16
	{CPU_TZPC_CC_CTRL,					   "CC controller"},
	{CPU_TZPC_MCU_SRAM_CTRL,	 	 "MCU SRAM controller"},
	
	{CPU_TZPC_STBC_CG_CTRL,			  "STBG CG controller"},
    {CPU_TZPC_WDT_CTRL,					  "WDT controller"},
	{CPU_TZPC_USB3_CTRL,     			 "USB3 controller"},
	{CPU_TZPC_USB3_A_CTRL,			   "USB3 A controller"},
	
	{CPU_TZPC_RTC_CTRL,					  "RTC controller"},
	{CPU_TZPC_DSI_CTRL,    				  "DSI controller"},
    {CPU_TZPC_CSI_TX_CTRL,  		   "CSI TX controller"},
    {CPU_TZPC_DSI_PHY_CTRL, 		  "DSI PHY controller"},

	{CPU_TZPC_MIPI_CSI_CTRL,              "CSI controller"},
	{CPU_TZPC_SIE_CTRL,     			  "SIE controller"},
    {CPU_TZPC_VIE_CTRL,					  "VIE controller"},
	{CPU_TZPC_TGE_CTRL,					  "TGE controller"},
	
	{CPU_TZPC_IFE_CTRL,					  "IFE controller"},
	{CPU_TZPC_NAND_CTRL,				 "NAND controller"},
	{CPU_TZPC_SD3_CTRL,					"SDIO3 controller"},
    {CPU_TZPC_DRE_CTRL,					  "DRE controller"},
	
	{CPU_TZPC_ISE_CTRL,					  "ISE controller"},    //32
	{CPU_TZPC_IPE_CTRL,      			  "IPE controller"},
	{CPU_TZPC_IME_CTRL,      			  "IME controller"},
    {CPU_TZPC_SD_CTRL,       			 "SDIO controller"},
	
	{CPU_TZPC_SD2_CTRL,      			"SDIO2 controller"},
	{CPU_TZPC_ADC_CTRL,      			  "ADC controller"},
	{CPU_TZPC_VPE_CTRL,      			  "VPE controller"},
	{CPU_TZPC_VENC_CTRL,     			 "VENC controller"},
	
	{CPU_TZPC_JM_CTRL,       			   "JM controller"},
	{CPU_TZPC_JMISP_CTRL,    			"JMISP controller"},
    {CPU_TZPC_UTL_CTRL,       			  "UTL controller"},
	{CPU_TZPC_ROU_CTRL,      			  "ROU controller"},
	
	{CPU_TZPC_CAL_CTRL,      			  "CAL controller"},
	{CPU_TZPC_LSU_CTRL,      			  "LSU controller"},
	{CPU_TZPC_PPU_CTRL,      			  "PPU controller"},
	{CPU_TZPC_CONV_CTRL,     			 "CONV controller"},
	
	{CPU_TZPC_IFE_SRAM_CTRL, 		 "IFE SRAM controller"},
    {CPU_TZPC_HWCP_CTRL,     		   "HWCOPY controller"},
	{CPU_TZPC_TSE_CTRL,					  "TSE controller"},    //52
	{CPU_TZPC_UVCP_CTRL,				 "UVCP controller"},
	
	{CPU_TZPC_GRPH_CTRL,				 "GRPH controller"},
	{CPU_TZPC_GRPH2_CTRL,				"GRPH2 controller"},
    {CPU_TZPC_GRPH3_CTRL, 				"GRPH3 controller"},
	{CPU_TZPC_JPG_CTRL,     			 "JPEG controller"},
	
	{CPU_TZPC_IVE_CTRL,     			  "IVE controller"},
	{CPU_TZPC_MDBC_CTRL,     			 "MDBC controller"},
    {CPU_TZPC_TRKE_CTRL,				 "TRKE controller"},     
	{CPU_TZPC_NUE2_CTRL,     			 "NUE2 controller"},
	
	{CPU_TZPC_TRICK_BOX_CTRL,		"Trick Box controller"},
	{CPU_TZPC_TCM_CTRL,                   "TCM controller"},     //110
	{CPU_TZPC_TCM2_CTRL,                 "TCM2 controller"},
};
#elif defined(CONFIG_TARGET_NS02301)
#define TOP_SRAM_SD_RSA_BIT_OFS     10
#define CHIP_ID_BASE     0xF00100F0
static TZPC_NAME tz_pc_NS02301[] = {
	{CPU_TZPC_DRAM_CTRL,                 "DRAM controller"},
	{CPU_TZPC_TOP_CTRL,                   "TOP controller"},
	{CPU_TZPC_CG_CTRL,                    "CKG controller"},
	{CPU_TZPC_PAD_CTRL,                   "PAD controller"},
	{CPU_TZPC_TIMER_CTRL,               "TIMER controller"},

	{CPU_TZPC_WDT_CTRL,     			  "WDT controller"},    //4
	{CPU_TZPC_GPIO_CTRL,                 "GPIO controller"},
	{CPU_TZPC_INTC_CTRL,                 "INTC controller"},
    {CPU_TZPC_DRTC_CTRL,     			 "DRTC controller"},
	
	{CPU_TZPC_HRTIMER_CTRL,			  "HRTIMER controller"},
	{CPU_TZPC_CFE_CTRL,     			  "CFE controller"},
	{CPU_TZPC_TEST_CTRL,     		     "TEST controller"},
	{CPU_TZPC_PR_8051_CTRL,           "PR-8051 controller"},

	{CPU_TZPC_PR_TOP_CTRL,   		   "PR-TOP controller"},
	{CPU_TZPC_PR_CG_CTRL,     		    "PR-CG controller"},
	{CPU_TZPC_PR_PAD_CTRL,     		   "PR-PAD controller"},
	{CPU_TZPC_PR_TIMER_CTRL,     	 "PR-TIMER controller"},
	
    {CPU_TZPC_PR_UART2_CTRL,		 "PR-UART2 controller"},
	{CPU_TZPC_PR_SPI_CTRL,			   "PR-SPI controller"},    //16
	{CPU_TZPC_PR_GPIO_CTRL,			  "PR-GPIO controller"},
	{CPU_TZPC_PR_INTC_CTRL,	 	 	  "PR-INTC controller"},
	
	{CPU_TZPC_PR_PWM_CTRL,			   "PR-PWM controller"},
    {CPU_TZPC_PR_I2C_CTRL,			   "PR-I2C controller"},
	{CPU_TZPC_PR_WEIGAND_CTRL,     "PR-WEIGAND controller"},
	{CPU_TZPC_PR_UART_CTRL,			  "PR-UART controller"},
	
	{CPU_TZPC_PR_ADC_CTRL,			   "PR-ADC controller"},
	{CPU_TZPC_PR_CC_CTRL,    		    "PR-CC controller"},
    {CPU_TZPC_PR_ISP_CTRL,  		   "PR-ISP controller"},
    {CPU_TZPC_PWM_CTRL, 		  		  "PWM controller"},

	{CPU_TZPC_I2C_CTRL,              	  "I2C controller"},
	{CPU_TZPC_SPI_CTRL,     			  "SPI controller"},
    {CPU_TZPC_SIF_CTRL,					  "SIF controller"},
	{CPU_TZPC_REMOTE_CTRL,			   "REMOTE controller"},
	
	{CPU_TZPC_THERMAL_CTRL,			  "THERMAL controller"},
	{CPU_TZPC_LVDS_CTRL,				 "LVDS controller"},
	{CPU_TZPC_MIPI_CSI_CTRL,		 "MIPI CSI controller"},
    {CPU_TZPC_UART_CTRL,			     "UART controller"},
	
	{CPU_TZPC_ETH_CTRL,					  "ETH controller"},    //32
	{CPU_TZPC_UART2_CTRL,      		    "UART2 controller"},
	{CPU_TZPC_UART3_CTRL,      		    "UART3 controller"},
    {CPU_TZPC_SPI2_CTRL,       			 "SPI2 controller"},
	
	{CPU_TZPC_SPI3_CTRL,      			 "SPI3 controller"},
	{CPU_TZPC_I2C2_CTRL,      		     "I2C2 controller"},
	{CPU_TZPC_LVDS2_CTRL,      		    "LVDS2 controller"},
	{CPU_TZPC_SDP_CTRL,     			  "SDP controller"},
	
	{CPU_TZPC_NAND_CTRL,       			 "NAND controller"},
	{CPU_TZPC_SD_CTRL,    				 "SDIO controller"},
    {CPU_TZPC_SD2_CTRL,       		    "SDIO2 controller"},
	{CPU_TZPC_SD3_CTRL,      		    "SDIO3 controller"},
	
	{CPU_TZPC_USB_PHY_CTRL,        	  "USB PHY controller"},
	{CPU_TZPC_SCE_CTRL,      			  "SCE controller"},
	{CPU_TZPC_DAI_CTRL,      			  "DAI controller"},
	{CPU_TZPC_EAC_CTRL,     			  "EAC controller"},
	
	{CPU_TZPC_TSE_CTRL, 		 		  "TSE controller"},
    {CPU_TZPC_EFUSE_CTRL,     		    "Efuse controller"},
	{CPU_TZPC_HASH_CTRL,				 "HASH controller"},    //52
	{CPU_TZPC_TRNG_CTRL,				 "TRNG controller"},
	
	{CPU_TZPC_UVCP_CTRL,				 "UVCP controller"},
	{CPU_TZPC_RSA_CTRL,					  "RSA controller"},
    {CPU_TZPC_SENPHY_CTRL, 			   "SENPHY controller"},
	{CPU_TZPC_ECDSA_CTRL,     			"ECDSA controller"},
	
	{CPU_TZPC_IME_SRAM_CTRL,     	 "IME SRAM controller"},
	{CPU_TZPC_CNN_SRAM_CTRL,     	 "CNN SRAM controller"},
	{CPU_TZPC_IPE_SRAM_CTRL,     	 "IPE SRAM controller"},
	{CPU_TZPC_IFE_SRAM_CTRL,     	 "IFE SRAM controller"},
	
	{CPU_TZPC_IDE_CTRL,					  "IDE controller"},
	{CPU_TZPC_PR_TCM_CTRL,             "PR-TCM controller"},     //110
	{CPU_TZPC_8051_TCM_CTRL,         "8051 TCM controller"},
	{CPU_TZPC_JPEG_CTRL,                 "JPEG controller"},
	{CPU_TZPC_VENC_CTRL,                 "VENC controller"},
	{CPU_TZPC_YDCC_DEC_CTRL,         "YDCC DEC controller"},
	{CPU_TZPC_SIE_CTRL,                   "SIE controller"},
	{CPU_TZPC_MDBC_CTRL,                 "MDBC controller"},
	{CPU_TZPC_DCE_CTRL,                   "DCE controller"},
	{CPU_TZPC_IPE_CTRL,                   "IPE controller"},
	{CPU_TZPC_IME_CTRL,                   "IME controller"},
	{CPU_TZPC_DIS_CTRL,                   "DIS controller"},
	{CPU_TZPC_NUE_CTRL,                   "NUE controller"},
	{CPU_TZPC_IFE_CTRL,                   "IFE controller"},
	{CPU_TZPC_GRPH_CTRL,                 "GRPH controller"},
	{CPU_TZPC_ISE_CTRL,                   "ISE controller"},
	{CPU_TZPC_CNN_CTRL,                   "CNN controller"},
	{CPU_TZPC_TGE_CTRL,                   "TGE controller"},
	{CPU_TZPC_VPE_CTRL,                   "VPE controller"},
	{CPU_TZPC_GRPH2_CTRL,               "GRPH2 controller"},
	{CPU_TZPC_SIE2_CTRL,                 "SIE2 controller"},
	{CPU_TZPC_SIE3_CTRL,                 "SIE3 controller"},
	{CPU_TZPC_NUE2_CTRL,                 "NUE2 controller"},
	{CPU_TZPC_IVE_CTRL,                   "IVE controller"},
	{CPU_TZPC_TZASC_CTRL,               "TZASC controller"},
	{CPU_TZPC_TZPC_CTRL,                 "TZPC controller"},
	{CPU_TZPC_USB_CTRL,                   "USB controller"},
};
#else
#define TOP_SRAM_SD_RSA_BIT_OFS     9
#define CHIP_ID_BASE     0xF00100F0
static TZPC_NAME tz_pc_NA51102[] = {
	{CPU_TZPC_SCE_CTRL,         "   SCE controller"},
	{CPU_TZPC_EFUSE_CTRL,       " EFUSE controller"},
	{CPU_TZPC_TRNG_CTRL,        "  TRNG controller"},
	{CPU_TZPC_RSA_CTRL,         "   RSA controller"},
	{CPU_TZPC_HASH_CTRL,        "  HASH controller"},
	{CPU_TZPC_TZPC_CTRL,        "  TZPC controller"},
};

#endif
#define TERMNULL                            0           // NULL
#define BACKSPACE                           8           // back space
#define NEWLINE                             10          // new line
#define UART_ESC                            27
#define UART_ENTER                          13

#define NA51055                             0x48210000
#define NA51084                             0x50210000
#define NA51090                             0xbc210000
#define NA51102                             0x52210000
#define NS02201                             0xf2210000
#define NS02301                             0x70210000

#define EMU_COMMON_CMD_SIZE    20
static char emuCommonBuf[EMU_COMMON_CMD_SIZE];

static UINT32 chip_id = 0x0;

//#define INREG32(x)          (*((volatile UINT32*)(x)))
//#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define INREG32(x)          readl(x)
#define OUTREG32(x, y)      writel((y), (x))
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

/**
* Call with struct optee_msg_arg as argument
 *
 * Call register usage:
 * a0   SMC Function ID, OPTEE_SMC*CALL_WITH_ARG
 * a1   Upper 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a2   Lower 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a3   Cache settings, not used if physical pointer is in a predefined shared
 *      memory area else per OPTEE_SMC_SHM_*
 * a4-6 Not used
 * a7   Hypervisor Client ID register
 *
 * Normal return register usage:
 * a0   Return value, OPTEE_SMC_RETURN_*
 * a1-3 Not used
 * a4-7 Preserved
**/

static int atoi(const char *str)
{
	return (int)simple_strtoul(str, '\0', 10);
}

/**
    Get a string from UART

    Get a string from UART.
    This api would be returned only if the ENTER key is pressed or the length of the buffer is reached in normal operation.
    Besides, the uart_abortGetChar() can also abort this api.

    @param[out] pcString        The incoming string.
    @param[in,out] pcBufferLen  For input, it's the maximum length of the buffer. And it will also return length of the input string.

    @return
     - @b E_PAR: Parameter error. pcString or pcBufferLen is NULL or buffer length is 0.
     - @b E_OK:  Get string successfully
     - @b E_OBJ: Input string longer than buffer
*/
INT32 uart_get_string(char *pcString, UINT32 *pcBufferLen)
{
	UINT8   uiBuferLen;
	int     ch;

	// check parameters
	if (pcString == NULL ||         // Parameter is NULL
		pcBufferLen == NULL ||      // Parameter is NULL
		*pcBufferLen == 0) {        // Buffer length is 0
		return -1;
	}

	uiBuferLen = *pcBufferLen;

	*pcBufferLen = 0;

	while (*pcBufferLen < uiBuferLen) {
		ch = serial_getc();
		*pcString = ch;
		if (*pcString == BACKSPACE) {
			if (*pcBufferLen) {
				(*pcBufferLen)--;
				pcString--;
			}
		} else if ((*pcString == NEWLINE) || (*pcString == UART_ENTER)) {

			*pcString = TERMNULL;
			return 0;
		} else {
			(*pcBufferLen)++;
			pcString++;
		}
	}
	return -1;
}

static UINT32 emu_get_number(UINT32 uiStart, UINT32 uiEnd)
{
	UINT32  cLen;
	UINT32  result;

	while (1) {
		if (uiEnd < 10) {
			cLen = 1;
		} else if (uiEnd < 100) {
			cLen = 2;
		} else if (uiEnd < 1000) {
			cLen = 3;
		} else if (uiEnd < 10000) {
			cLen = 4;
		} else if (uiEnd < 100000) {
			cLen = 5;
		} else if (uiEnd < 1000000) {
			cLen = 6;
		} else {
			cLen = 7;
		}
		memset((void *)emuCommonBuf, 0x0, EMU_COMMON_CMD_SIZE);
		printf(">> Input > %d ~ %d\r\n", uiStart, uiEnd);
		uart_get_string(emuCommonBuf, &cLen);
		result = atoi(emuCommonBuf);

		if ((result <= uiEnd) && (result >= uiStart)) {
			return result;
		}
	}
}

void debug_dump_addr(uintptr_t addr, UINT32 size)
{
	UINT32 i;
	uintptr_t j;

	for (j = addr; j < (addr + size); j += 0x10) {
		printf("0x%lx:", j);
		for (i = 0; i < 4; i++) {
			printf("%08x", readl(j + 4 * i));
			printf("  ");
		}
		printf("\r\n");
	}
}

static int do_tzpc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	UINT32  selectionBit = 0;
	UINT32  element;
	//UINT32  address, address_ofs, address_bit, tzpc_address;
	UINT32  address_ofs, address_bit;
#if defined(CONFIG_TARGET_NS02301)
	UINT32  address, tzpc_address;
	UINT32  trng_address;
#else
	UINT64  address, tzpc_address;
	UINT64  trng_address;
#endif
	UINT32  reg;
	UINT32  index;

	chip_id = INREG32(CHIP_ID_BASE);

	printf("do_tzpc argc = %d\r\n", argc);
	if (argc < 2) {
		printf("argc %d error\n", argc);
		return -1;
	}


	printf("chip id = 0x%08x\r\n", chip_id);

	if (strncmp(argv[1], "pct", 3) == 0) {
#ifdef CONFIG_TARGET_NA51102_A64
		element = sizeof(tz_pc_NA51102) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		element = sizeof(tz_pc_NS02201) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
		element = sizeof(tz_pc_NS02302) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02301)
		element = sizeof(tz_pc_NS02301) / sizeof(TZPC_NAME);
#endif
		printf("=>Select test module 0 ~ %d\r\n", element);
		selectionBit = emu_get_number(0, element - 1);
#ifdef CONFIG_TARGET_NA51102_A64
		address = tz_pc_NA51102[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		address = tz_pc_NS02201[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
		address = tz_pc_NS02302[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02301)
		address = tz_pc_NS02301[selectionBit].base_address;
#endif
		if (address >= 0x100) {
#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
			tzpc_address = 0x2F0220024;
			if (address == CPU_TZPC_PCIE_DBI2_CTRL) {
				address_bit = 6;
			} else if (address == CPU_TZPC_EDAP1_CTRL) {
				address_bit = 7;
			} else if (address == CPU_TZPC_EDAP2_CTRL) {
				address_bit = 7;
			} else {
				address_bit = address - CPU_TZPC_SPECIAL_START;
			}

			if (address == CPU_TZPC_TCM_CTRL) {
				address = 0x2F1000000;
			} else if (address == CPU_TZPC_TCM2_CTRL) {
				address = 0x2F1100000;
			} else if (address == CPU_TZPC_TCM3_CTRL) {
				address = 0x2F1200000;
			} else if (address == CPU_TZPC_TCM4_CTRL) {
				address = 0x2F1300000;
			} else if (address == CPU_TZPC_MALI_SRAM_CTRL) {
				address = 0x2F1400000;
			} else if (address == CPU_TZPC_N25_SRAM_CTRL) {
				address = 0x2F1800000;
			} else if (address == CPU_TZPC_PCIE_DBI1_CTRL) {
				address = 0x2F2000000;
			} else if (address == CPU_TZPC_PCIE_DBI2_CTRL) {
				address = 0x2F2400000;
			} else if (address == CPU_TZPC_EDAP1_CTRL) {
				address = 0x2F3000000;
			} else if (address == CPU_TZPC_EDAP2_CTRL) {
				address = 0x2F3800000;
			}
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
			tzpc_address = 0x2F0166024;

			address_bit = address - CPU_TZPC_SPECIAL_START;
	
			if (address == CPU_TZPC_TCM_CTRL) {
				address = 0x2F1000000;
			} else if (address == CPU_TZPC_TCM2_CTRL) {
				address = 0x2F1100000;
			}
#elif defined(CONFIG_TARGET_NS02301)
			tzpc_address = 0xF0FF0020;
			if (address == CPU_TZPC_USB_CTRL) {
				address = (0xF0000000 | (address << 16));
				address_bit = 2;
			} else {
				printf(("error\r\n"));
			}
#endif
		} else {
			address_ofs = address;
			address_bit = address_ofs % 32;
			address_ofs = (address_ofs / 32) * 4;
#ifdef CONFIG_TARGET_NA51102_A64
			tzpc_address = 0x2F00C0000 + address_ofs;
			address = (0x2F0000000 | (address << 16));
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
			tzpc_address = 0x2F0220000 + address_ofs;
			address = (0x2F0000000 | (address << 16));
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
			tzpc_address = 0x2F0166000 + address_ofs;
			address = (0x2F0000000 | (address << 16));
#elif defined(CONFIG_TARGET_NS02301)
			tzpc_address = 0xF0FF0000 + address_ofs;
			address = (0xF0000000 | (address << 16));
#endif
		}
//		if (chip_id == NA51055) {
//			printf(("TZPC test base address = 0x%08x -> %s\r\n", address, tz_pc_NA51055[selectionBit].module_name));
//		} else {
//			printf(("TZPC test base address = 0x%08x -> %s\r\n", address, tz_pc_NA51084[selectionBit].module_name));
//		}
#ifdef CONFIG_TARGET_NA51102_A64
		printf("[51102][%s]\r\n", tz_pc_NA51102[selectionBit].module_name);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		printf("[02201][%s]\r\n", tz_pc_NS02201[selectionBit].module_name);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
		printf("[02302][%s]\r\n", tz_pc_NS02302[selectionBit].module_name);
#elif defined(CONFIG_TARGET_NS02301)
		printf("[02301][%s]\r\n", tz_pc_NS02301[selectionBit].module_name);
#endif
		printf(" -> TZPC base address = 0x%08lx bit[%d]\r\n", tzpc_address, address_bit);

#if defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		if (address < 0x2F1400000) {
			if (address == 0xf0700000) {
				debug_dump_addr(address, 128);
			} else if (address == 0xff600000) {
				debug_dump_addr(address, 0x70);
			} else if ((address >= 0x2f0b00000) && (address <= 0x2f0b50000))  {
				printf("skip read 0x%lx\r\n", address);
			} else if ((address >= 0x2f0800000) && (address <= 0x2f0ba0000))  {
				printf("skip read 0x%lx\r\n", address);
			} else if ((address == 0x2f0C30000) || (address == 0x2f0E20000))  {
				printf("skip read 0x%lx\r\n", address);
			} else {
				debug_dump_addr(address, 256);
			}
		} else {
			printf("skip read 0x%lx\r\n", address);
		}
#endif
#if defined(CONFIG_TARGET_NS02301)
		if (address == 0xFF600000) {
			*(volatile UINT32 *)tzpc_address |= (1 << address_bit);
		} else if (address == 0xF0700000) {
			*(volatile uintptr_t *)tzpc_address |= (0x7 << address_bit);
		} else if (address == 0xF0730000) {
			*(volatile uintptr_t *)tzpc_address |= (0x3 << address_bit);
		} else if (address == 0xF0760000) {
			*(volatile uintptr_t *)tzpc_address |= (0x3 << address_bit);
		} else if (address == 0xF07C0000) {
			*(volatile uintptr_t *)tzpc_address |= (0x3 << address_bit);
		} else if (address == 0xF0900000) {
			*(volatile uintptr_t *)tzpc_address |= (0xfff << address_bit);
		} else {
			*(volatile uintptr_t *)tzpc_address |= (1 << address_bit);
		}
#endif
		printf("switch to secure mode allow only\r\n");
#if defined(CONFIG_TARGET_NS02301)
		if (address == 0xff600000) {
			debug_dump_addr(address, 0x70);
		} else {
			debug_dump_addr(address, 256);
		}
#else		
		debug_dump_addr(address, 256);
#endif
		printf("=============================================\r\n");
		debug_dump_addr(tzpc_address, 256);


	} else if (strncmp(argv[1], "slave_clk", 9) == 0) {

		if (chip_id == NS02301) {
			
		
			address_ofs = 0x24;
			tzpc_address = 0xF0FF0000 + address_ofs;
			address_bit = 0;

			printf(" -> TZPC base address = 0x%08x bit[%02d]\r\n", (int)tzpc_address, address_bit);

			reg = *(volatile UINT32 *)0xF0020080;
			printf("   RSTN ori [0xF0020080] crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

			reg = *(volatile UINT32 *)0xF0020084;
			printf("   RSTN ori [0xF0020084]    TMR->18[%d], TOP->27[%d],efuse->28[%d]\r\n", ((reg >> 18) & 0x1), ((reg >> 27) & 0x1), ((reg >> 28) & 0x1));

			reg = *(volatile UINT32 *)0xF0020088;
			printf("   RSTN ori [0xF0020088]   DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));


			reg = *(volatile UINT32 *)0xF002001C;
			printf(" clksel ori [0xF002001C] Crypto->[21..20][0x%02x], RSA->[23..22][0x%02x]\r\n", ((reg >> 20) & 0x3), ((reg >> 22) & 0x3));

			reg = *(volatile UINT32 *)0xF0020024;
			printf(" clksel ori [0xF0020024]   HASH->[17..16][0x%02x],TRNG->[18][%d], TRNG_RO->[19][%d], DRTC->[20][%d]\r\n", ((reg >> 16) & 0x3), ((reg >> 18) & 0x1), ((reg >> 19) & 0x1), ((reg >> 20) & 0x1));

			reg = *(volatile UINT32 *)0xF0020070;
			printf(" clk_en ori [0xF0020070] Crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

			*(volatile UINT32 *)0xF0020074 |= (1 << 18);
			reg = *(volatile UINT32 *)0xF0020074;
			printf(" clk_en ori [0xF0020074] TMR->18[%d]\r\n", ((reg >> 18) & 0x1));

			*(volatile UINT32 *)0xF0020078 |= (1 << 22);
			reg = *(volatile UINT32 *)0xF0020078;
			printf(" clk_en ori [0xF0020078]   DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));

			printf(" -> TZPC base address = 0x%08x bit[%02d] => Enable CKG slave \r\n", (int)(tzpc_address), address_bit);

			*(volatile UINT32 *)(tzpc_address) |= 0x1;

			*(volatile UINT32 *)0xF0020080 &= ~(1 << 23);
			reg = *(volatile UINT32 *)0xF0020080;
			printf("   RSTN clr [0xF0020080] crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

			*(volatile UINT32 *)0xF0020084 &= ~((1 << 16) | (1 << 17) | (1 << 18) | (1 << 27) | (1 << 28));
			reg = *(volatile UINT32 *)0xF0020084;
			printf("   RSTN clr [0xF0020084]   TMR->18[%d],TOP->27[%d],efuse->28[%d]\r\n", ((reg >> 18) & 0x1), ((reg >> 27) & 0x1), ((reg >> 28) & 0x1));

			*(volatile UINT32 *)0xF0020088 &= ~((1 << 22) | (1 << 25) | (1 << 26) | (1 << 27));
			reg = *(volatile UINT32 *)0xF0020088;
			printf("   RSTN clr [0xF0020088]  DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));

			*(volatile UINT32 *)0xF002001C |= ((0x3 << 20) | (0x3 << 22));
			reg = *(volatile UINT32 *)0xF002001C;
			printf(" clksel set [0xF002001C] Crypto->[21..20][0x%02x], RSA->[23..22][0x%02x]\r\n", ((reg >> 20) & 0x3), ((reg >> 22) & 0x3));

			*(volatile UINT32 *)0xF0020024 |= ((0x3 << 16) | (0x1 << 18) | (0x1 << 19) | (0x1 << 20));
			reg = *(volatile UINT32 *)0xF0020024;
			printf(" clksel set [0xF0020024]   HASH->[17..16][0x%02x],TRNG->[18][%d], TRNG_RO->[19][%d], DRTC->[20][%d]\r\n", ((reg >> 16) & 0x3), ((reg >> 18) & 0x1), ((reg >> 19) & 0x1), ((reg >> 20) & 0x1));


			*(volatile UINT32 *)0xF0020070 |= ((0x1 << 23));
			reg = *(volatile UINT32 *)0xF0020070;
			printf(" clk_en set [0xF0020070] Crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

			*(volatile UINT32 *)0xF0020074 &= ~(0x1 << 18);
			reg = *(volatile UINT32 *)0xF0020074;
			printf(" clk_en clr [0xF0020074] TMR->18[%d]\r\n", ((reg >> 18) & 0x1));

			*(volatile UINT32 *)0xF0020078 &= ~((0x1 << 22) | (0x1 << 25) | (0x1 << 26) | (0x1 << 27));
			reg = *(volatile UINT32 *)0xF0020078;
			printf(" clk_en clr [0xF0020078] DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));
		} else {
			printf("not support\r\n");
		}

	} else if (strncmp(argv[1], "slave_trng", 10) == 0) {
		if (chip_id == NS02301) {
			address_ofs = 0x00;
			tzpc_address = IOADDR_TRNG_REG_BASE + address_ofs;
			address_bit = 1;

			*(volatile UINT32 *)0xF0020078 |= (1 << 25);
			reg = *(volatile UINT32 *)0xF0020078;
			
			printf(" -> TRNG base address = 0x%08lx bit[%02d]\r\n", (int)tzpc_address, address_bit);
			reg = INREG32(tzpc_address);
			printf(" ctrl reg [0x%08x] = 0x%08lx\r\n", (int)tzpc_address, (int)reg);

			SETREG32(IOADDR_TZPC_REG_BASE + 0x24, 0x2);
			OUTREG32(tzpc_address, 0x0);
			reg = INREG32(tzpc_address);
			printf(" ctrl reg [0x%08x] = 0x%08lx\r\n", (int)tzpc_address, (int)reg);
		} else {
			printf("not support\r\n");
		}

	} else if (strncmp(argv[1], "slave_top", 9) == 0) {
		if (chip_id == NS02301) {
			address_ofs = 0x24;
			tzpc_address = IOADDR_TZPC_REG_BASE + address_ofs;
			address_bit = 2;

			printf(" -> TZPC base address = 0x%08lx bit[%02d]\r\n", (int)tzpc_address, address_bit);

			SETREG32(IOADDR_TOP_REG_BASE + 0x1000, (1 << TOP_SRAM_SD_RSA_BIT_OFS));
			reg = INREG32(IOADDR_TOP_REG_BASE + 0x1000);
			printf(" RSA SRAM ori [0x%08lx] = bit%d[%d]\r\n", (int)reg, TOP_SRAM_SD_RSA_BIT_OFS, (int)((reg >> TOP_SRAM_SD_RSA_BIT_OFS) & 0x1));

			SETREG32(IOADDR_TZPC_REG_BASE + 0x24, 0x4);
			CLRREG32(IOADDR_TOP_REG_BASE + 0x1000, (1 << TOP_SRAM_SD_RSA_BIT_OFS));

			reg = INREG32(IOADDR_TOP_REG_BASE + 0x1000);
			printf(" RSA SRAM clr [0x%08lx] = bit%d[%d]\r\n", (int)reg, TOP_SRAM_SD_RSA_BIT_OFS, (int)((reg >> TOP_SRAM_SD_RSA_BIT_OFS) & 0x1));
		} else {
			printf("not support\r\n");
		}

	} else if (strncmp(argv[1], "tzpc_dump", 9) == 0) {
#ifdef CONFIG_TARGET_NA51102_A64
		element = sizeof(tz_pc_NA51102) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		element = sizeof(tz_pc_NS02201) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
		element = sizeof(tz_pc_NS02302) / sizeof(TZPC_NAME);
#elif defined(CONFIG_TARGET_NS02301)
		element = sizeof(tz_pc_NS02301) / sizeof(TZPC_NAME);
#endif
		for (index = 0; index < element; index ++) {
#ifdef CONFIG_TARGET_NA51102_A64
		address = tz_pc_NA51102[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
		address = tz_pc_NS02201[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
		address = tz_pc_NS02302[selectionBit].base_address;
#elif defined(CONFIG_TARGET_NS02301)
		address = tz_pc_NS02301[selectionBit].base_address;
#endif
			address_ofs = address;
			address_bit = address_ofs % 32;
			address_ofs = (address_ofs / 32) * 4;
			tzpc_address = IOADDR_TOP_REG_BASE + address_ofs;
			address = (IOADDR_GLOBAL_BASE | (address << 16));
#ifdef CONFIG_TARGET_NA51102_A64
			printf("TZPC[0x%08lx]bit[%2d] -> [0x%08x] => %s\r\n", tzpc_address, address_bit, address, tz_pc_NA51102[index].module_name);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
			printf("TZPC[0x%08lx]bit[%2d] -> [0x%08x] => %s\r\n", tzpc_address, address_bit, address, tz_pc_NS02201[index].module_name);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
			printf("TZPC[0x%08lx]bit[%2d] -> [0x%08x] => %s\r\n", tzpc_address, address_bit, address, tz_pc_NS02302[index].module_name);
#elif defined(CONFIG_TARGET_NS02301)
			printf("TZPC[0x%08lx]bit[%2d] -> [0x%08x] => %s\r\n", tzpc_address, address_bit, address, tz_pc_NS02301[index].module_name);
#endif	
			
			if ((INREG32(tzpc_address) & (1 << address_bit)) == (1 << address_bit)) {
#ifdef CONFIG_TARGET_NA51102_A64
				printf("[SEC only]=>[%s]\r\n", tz_pc_NA51102[index].module_name);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
				printf("[SEC only]=>[%s]\r\n", tz_pc_NS02201[index].module_name);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
				printf("[SEC only]=>[%s]\r\n", tz_pc_NS02302[index].module_name);
#elif defined(CONFIG_TARGET_NS02301)
				printf("[SEC only]=>[%s]\r\n", tz_pc_NS02301[index].module_name);
#endif	
			} else {
#ifdef CONFIG_TARGET_NA51102_A64
				printf("[BOTH]=>[%s]\r\n", tz_pc_NA51102[index].module_name);
#elif defined(CONFIG_TARGET_NS02201) || defined(CONFIG_TARGET_NS02201_A64)
				printf("[BOTH]=>[%s]\r\n", tz_pc_NS02201[index].module_name);
#elif defined(CONFIG_TARGET_NS02302) || defined(CONFIG_TARGET_NS02302_A64)
				printf("[BOTH]=>[%s]\r\n", tz_pc_NS02302[index].module_name);
#elif defined(CONFIG_TARGET_NS02301)
				printf("[BOTH]=>[%s]\r\n", tz_pc_NS02301[index].module_name);
#endif
			}
		}
	}
#if 0
	else if (strncmp(argv[1], "mem_prot_in_1", 13) == 0) {
		DMA_WRITEPROT_ATTR  ProtectAttr = {0};
		BOOL                bResult;
		UINT32              uiSet;
		UINT32              i;
		UINT32              protect_mode;
		UINT32              uiTestSize;
		UINT32              uiTestAddrStart;


		uiTestSize = 0x4000;

		uiTestAddrStart = (UINT32)malloc(0x4000);

		if (uiTestAddrStart % 16) {
			uiTestAddrStart = (uiTestAddrStart + 15) & 0xFFFFFFF0;
		}


		printf(" Memory address = 0x%08x size = 0x4000\r\n", (int)uiTestAddrStart);

		for (uiSet = 0; uiSet < 6; uiSet++) {
			for (protect_mode = 0; protect_mode < 4; protect_mode++) {
				printf("====================set[%d] mode[%d]====================\r\n", uiSet, protect_mode);
				ProtectAttr.chEnMask.bCPU_NS = TRUE;
				ProtectAttr.uiProtectlel = protect_mode;
				ProtectAttr.uiStartingAddr = uiTestAddrStart;
				ProtectAttr.uiSize = uiTestSize * 2;

				//fLib_PutSerialStr("1. CPU write protect test start, write original pattern, test set %d, test mode %d\r\n", uiSet, protect_mode);

				printf("address[0x%08x] size[0x%08x]\r\n", (int)uiTestAddrStart, (int)uiTestSize);

				memset((void *)uiTestAddrStart, 0x96, uiTestSize);
				memset((void *)uiTestAddrStart + uiTestSize, 0xff, uiTestSize);

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(uiTestSize, ARCH_DMA_MINALIGN));

				if (uiSet == DMA_WPSET_0) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet0*/);
				} else if (uiSet == DMA_WPSET_1) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet1*/);
				} else if (uiSet == DMA_WPSET_2) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet2*/);
				} else if (uiSet == DMA_WPSET_3) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet3*/);
				} else if (uiSet == DMA_WPSET_4) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet4*/);
				} else {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet5*/);
				}


				dma_enableWPFunc(uiSet);

				//pinmux_select_debugport(PINMUX_DEBUGPORT_DDR);
				//0x0 : Write protection only
				//0x1 : Write detection only
				//0x2 : Read protection only
				//0x3 : Read & write protection


				//   CPU read               CPU read write
				if ((protect_mode == 2) || (protect_mode == 3)) {
					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x96969696) {
							printf("cpu read fail = 0x%08x\r\n", *(volatile UINT32 *)(uiTestAddrStart + i));
						} else {
							printf("cpu read = 0x%08x\r\n", *(volatile UINT32 *)(uiTestAddrStart + i));
							if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x55aa55aa) {
								printf(" --> corrected");
							} else {
								printf(" -->     error");
							}
							printf("\r\n");
						}
					}
				}

				for (i = 0; i < ARCH_DMA_MINALIGN; i += 4) {
					*(volatile UINT32 *)(uiTestAddrStart + i) = 0x69696969;
				}

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(ARCH_DMA_MINALIGN, ARCH_DMA_MINALIGN));

				//cpu_cleanInvalidateDCacheBlock(uiTestAddrStart, uiTestAddrStart + 8);

				if ((protect_mode == 0) || (protect_mode == 3)) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x69696969) {
							bResult = FALSE;
							printf("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						printf("CPU write protect test success\r\n");
					}
				} else if (protect_mode == 1) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) != 0x69696969) {
							bResult = FALSE;
							printf("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						printf("CPU write protect test success\r\n");
					}
				}

				dma_disableWPFunc(uiSet);
			}
		}
	}
#endif
	else if (strncmp(argv[1], "apb_tout", 8) == 0) {
		if (chip_id == NA51055) {
			printf("=>NT98528 support only\r\n");
		} else if (chip_id == NS02301) {
			printf("=>Enable / disable APB timeout enabled\r\n");
			reg = *(UINT32 *)0xF0FF0020;
			*(UINT32 *)0xF0FF0020 = (reg | (1 << 31));
			debug_dump_addr(0xF0FF0000, 128);
		} else {
			printf("not support\r\n");
		}
	} else if (strncmp(argv[1], "nv_counter", 10) == 0) {
		if (chip_id == NA51055) {
			printf("=>NT98525 NV counter over flow @ 49\r\n");
			int i;
			printf("=>NT98528 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT32 *)0xF0FF0030 = 1;
				reg = *(UINT32 *)0xF0FF0030;
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		} else if (chip_id == NS02301) {
			int i;
			printf("=>NT98567 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT32 *)0xF0FF0030 = 1;
				reg = *(UINT32 *)0xF0FF0030;
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		} else if (chip_id == NA51102) {
			int i;
			printf("=>NT98530 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT64 *)0x2F00C0030 = 1;
				reg = *(UINT64 *)0x2F00C0030;
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		} else if (chip_id == NS02201) {
			int i;
			printf("=>NT98690 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT64 *)0x2F0220030 = 1;
				reg = readl(0x2F0220030);
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		} else {
			int i;
			printf("=>NT98538 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT64 *)0x2F0166030 = 1;
				reg = readl(0x2F0166030);
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		}
	}
	return 0;
}


U_BOOT_CMD(nvt_tzpc, 3, 0, do_tzpc,
		   "tzpc emulation cmd:",
		   "[Option] \n"
		   "                 [pct] : primary protected control test\n"
		   "            [apb_tout] : enable APB bus timeout\n"
		   "          [nv_counter] : nv_counter\n"
		   "           [tzpc_dump] : dump register info\n"
		   "           [slave_clk] : slave  ckg protected control test\n"
		   "          [slave_trng] : slave trng protected control test\n"
		   "           [slave_top] : slave  top protected control test\n"
		   "       [mem_prot_in_1] : mem protect in @ dram1 @ non secure world\n"
		   "       [mem_prot_in_2] : mem protect in @ dram2 @ non secure world\n"
		   "      [mem_prot_out_1] : mem protect out @ dram1 @ non secure world\n"
		   "      [mem_prot_out_1] : mem protect out @ dram2 @ non secure world\n"
		  );
