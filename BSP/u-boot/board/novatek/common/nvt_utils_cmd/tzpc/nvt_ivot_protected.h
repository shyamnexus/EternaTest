#ifndef _DMA_PROTECTED_H_
#define _DMA_PROTECTED_H_

#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/rcw_macro.h>


#ifdef __DBG_COLOR_MONO__
#define DBG_COLOR_GRAY ""
#define DBG_COLOR_RED ""
#define DBG_COLOR_YELLOW ""
#define DBG_COLOR_CYAN ""
#define DBG_COLOR_GREEN ""
#define DBG_COLOR_MAGENTA ""
#define DBG_COLOR_BLUE ""
#define DBG_COLOR_WHITE ""
#define DBG_COLOR_HI_GRAY ""
#define DBG_COLOR_HI_RED ""
#define DBG_COLOR_HI_YELLOW ""
#define DBG_COLOR_HI_CYAN ""
#define DBG_COLOR_HI_GREEN ""
#define DBG_COLOR_HI_MAGENTA ""
#define DBG_COLOR_HI_BLUE ""
#define DBG_COLOR_HI_WHITE ""
#define DBG_COLOR_FATAL ""
#define DBG_COLOR_ERR ""
#define DBG_COLOR_WRN ""
#define DBG_COLOR_UNIT ""
#define DBG_COLOR_FUNC ""
#define DBG_COLOR_IND ""
#define DBG_COLOR_MSG ""
#define DBG_COLOR_VALUE ""
#define DBG_COLOR_USER ""
#define DBG_COLOR_END ""
#else
#define DBG_COLOR_GRAY "\033[0;30m"
#define DBG_COLOR_RED "\033[0;31m"
#define DBG_COLOR_YELLOW "\033[0;33m"
#define DBG_COLOR_CYAN "\033[0;36m"
#define DBG_COLOR_GREEN "\033[0;32m"
#define DBG_COLOR_MAGENTA "\033[0;35m"
#define DBG_COLOR_BLUE "\033[0;34m"
#define DBG_COLOR_WHITE "\033[0;37m"
#define DBG_COLOR_HI_GRAY "\033[1;30m"
#define DBG_COLOR_HI_RED "\033[1;31m"
#define DBG_COLOR_HI_YELLOW "\033[1;33m"
#define DBG_COLOR_HI_CYAN "\033[1;36m"
#define DBG_COLOR_HI_GREEN "\033[1;32m"
#define DBG_COLOR_HI_MAGENTA "\033[1;35m"
#define DBG_COLOR_HI_BLUE "\033[1;34m"
#define DBG_COLOR_HI_WHITE "\033[1;37m"
#define DBG_COLOR_FATAL "\033[1;31m"
#define DBG_COLOR_ERR "\033[1;31m"
#define DBG_COLOR_WRN "\033[1;33m"
#define DBG_COLOR_UNIT "\033[0;32m"
#define DBG_COLOR_FUNC "\033[0;36m"
#define DBG_COLOR_IND ""
#define DBG_COLOR_MSG ""
#define DBG_COLOR_VALUE ""
#define DBG_COLOR_USER ""
#define DBG_COLOR_END "\033[0m"
#endif

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif
/*
    Translate DRAM address to physical address.

    Translate DRAM address to physical address.

    @param[in] addr     DRAM address

    @return physical DRAM address
*/
#define         dma_getPhyAddr(addr)            ((((UINT32)(addr))>=0x60000000UL)?((UINT32)(addr)-0x60000000UL):(UINT32)(addr))

/*
    Translate DRAM address to non-cacheable address.

    Translate DRAM address to non-cacheable address.

    @param[in] addr     DRAM address

    @return non-cacheable DRAM address
*/
#define         dma_getNonCacheAddr(addr)       ((((UINT32)(addr))<0x60000000UL)?((UINT32)(addr)+0x60000000UL):(UINT32)(addr))

#ifdef CONFIG_TARGET_NS02302_A64
typedef struct {
	UINT32 CPU: 1;   //CPU_S
	UINT32 CPU_NS: 1;
	UINT32 HVYLD: 1; 
	UINT32 reserved: 1;
	//M1
	UINT32 AGE: 1;
	UINT32 SSCA: 1;
	UINT32 OSG: 1;		
	UINT32 GPE: 1;		
	UINT32 GPE2: 1;			
	UINT32 JPG: 1;
	UINT32 SMC: 1;
	UINT32 SDIO: 1;
	UINT32 DAI3: 1;
	UINT32 DAI4: 1;
	UINT32 ETH: 1;
	UINT32 ETH2: 1;
	UINT32 USB: 1;
	UINT32 USB2: 1;
	UINT32 SATA: 1;
	UINT32 SATA2: 1;
	UINT32 LARB: 1;
	UINT32 SCE: 1;
	UINT32 HASH: 1;
	UINT32 HWCP: 1;
	UINT32 HVYLD2: 1;
	UINT32 reserved3: 1;
	UINT32 reserved4: 1;
	UINT32 reserved5: 1;

	//M2
	UINT32 CNN: 1;
	UINT32 NUE: 1;
	UINT32 NUE2: 1;
	UINT32 VPE_0: 1;
	UINT32 VPE_1: 1;
	UINT32 reserved6: 1;
	UINT32 reserved7: 1;
	UINT32 reserved8: 1;
	//M3
	UINT32 VENC_0_1: 1;
	UINT32 HVYLD3: 1;
	UINT32 reserved9: 1;
	UINT32 reserved10: 1;
	UINT32 reserved11: 1;
	UINT32 reserved12: 1;
	UINT32 reserved13: 1;
	UINT32 reserved14: 1;
	//M4
	UINT32 VDEC_0: 1;
	UINT32 VDEC_1_2_3: 1;
	UINT32 DEI: 1;
	UINT32 reserved15: 1;
	UINT32 reserved16: 1;
	UINT32 reserved17: 1;
	UINT32 reserved18: 1;
	UINT32 reserved19: 1;
	//M5
	UINT32 VCAP_0: 1;
	UINT32 VCAP_1: 1;
	UINT32 VCAP_2: 1;
	UINT32 LCD_0: 1;
	UINT32 LCD_1: 1;
	UINT32 GPD: 1;
	UINT32 DAI: 1;
	UINT32 DAI2: 1;
	UINT32 DAI5: 1;
	UINT32 reserved20: 1;
	UINT32 reserved21: 1;
	UINT32 reserved22: 1;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
        DDR_ARB_1,                           ///< DDR Arbiter
        DDR_ARB_2,                           ///< DDR Arbiter

        DDR_ARB_COUNT,                       //< Arbiter count

        ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;


typedef enum _DMA_WRITEPROT_SET {
        WPSET_0,            // Write protect function set 0
        WPSET_1,            // Write protect function set 1
        WPSET_2,            // Write protect function set 2
        WPSET_3,            // Write protect function set 3
        WPSET_4,            // Write protect function set 4
        OUT_WP,             // Out range write protect
        WPSET_COUNT,
        ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      // Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       // Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       // Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        // Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef struct _DMA_WRITEPROT_ATTR {
        DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
        DMA_WRITEPROT_LEVEL level;          // protect level
        uint64_t            starting_addr; // DDR3:must be 4 words alignment
        uint32_t            size;          // DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,
	DMA_CH_GROUP1,

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;

#define MAU_INT_ENABLE_OFS 0x190
REGDEF_BEGIN(MAU_INT_ENABLE_REG)
REGDEF_BIT(ERR_MEM_INT_EN0, 1)
REGDEF_BIT(ERR_MEM_INT_EN1, 1)
REGDEF_BIT(ERR_MEM_INT_EN2, 1)
REGDEF_BIT(ERR_MEM_INT_EN3, 1)
REGDEF_BIT(ERR_MEM_INT_EN4, 1)
REGDEF_BIT(ERR_MEM_INT_EN5, 1)
REGDEF_BIT(ERR_MEM_INT_EN6, 1)
REGDEF_BIT(ERR_MEM_INT_EN7, 1)
REGDEF_BIT(ERR_WT_INT_EN0, 1)
REGDEF_BIT(ERR_WT_INT_EN1, 1)
REGDEF_BIT(ERR_WT_INT_EN2, 1)
REGDEF_BIT(ERR_WT_INT_EN3, 1)
REGDEF_BIT(ERR_WT_INT_EN4, 1)
REGDEF_BIT(ERR_WT_INT_EN5, 1)
REGDEF_BIT(ERR_WT_INT_EN6, 1)
REGDEF_BIT(ERR_WT_INT_EN7, 1)
REGDEF_BIT(READ_TIMEOUT_INT_EN, 1)
REGDEF_BIT(MON_UPDATE_INT_EN, 1)
REGDEF_BIT(, 14)
REGDEF_END(MAU_INT_ENABLE_REG)

#define MAU_INT_STATUS_OFS 0x194
REGDEF_BEGIN(MAU_INT_STATUS_REG)
REGDEF_BIT(ERR_MEM_INT0, 1)
REGDEF_BIT(ERR_MEM_INT1, 1)
REGDEF_BIT(ERR_MEM_INT2, 1)
REGDEF_BIT(ERR_MEM_INT3, 1)
REGDEF_BIT(ERR_MEM_INT4, 1)
REGDEF_BIT(ERR_MEM_INT5, 1)
REGDEF_BIT(ERR_MEM_INT6, 1)
REGDEF_BIT(ERR_MEM_INT7, 1)
REGDEF_BIT(ERR_WT_INT0, 1)
REGDEF_BIT(ERR_WT_INT1, 1)
REGDEF_BIT(ERR_WT_INT2, 1)
REGDEF_BIT(ERR_WT_INT3, 1)
REGDEF_BIT(ERR_WT_INT4, 1)
REGDEF_BIT(ERR_WT_INT5, 1)
REGDEF_BIT(ERR_WT_INT6, 1)
REGDEF_BIT(ERR_WT_INT7, 1)
REGDEF_BIT(READ_TIMEOUT_INT, 1)
REGDEF_BIT(MON_UPDATE_INT, 1)
REGDEF_BIT(, 14)
REGDEF_END(MAU_INT_STATUS_REG)

#define MAU_PROTECT_MODE_OFS 0x48C
REGDEF_BEGIN(MAU_PROTECT_MODE_REG)
REGDEF_BIT(WP0_MODE, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(WP1_MODE, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(WP2_MODE, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(WP3_MODE, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(WP4_MODE, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(OUT_WP_MODE, 2)
REGDEF_BIT(, 10)
REGDEF_END(MAU_PROTECT_MODE_REG)

// Protect
#define MAU_IN_RANGE_PROTECT0_ENABLE_OFS0 0x440
#define MAU_IN_RANGE_PROTECT1_ENABLE_OFS0 0x448
#define MAU_IN_RANGE_PROTECT2_ENABLE_OFS0 0x450
#define MAU_IN_RANGE_PROTECT3_ENABLE_OFS0 0x458
#define MAU_IN_RANGE_PROTECT4_ENABLE_OFS0 0x460
#define MAU_OUT_RANGE_PROTECT_ENABLE_OFS0 0x468

#define MAU_IN_RANGE_PROTECT0_ENABLE_OFS1 0x444
#define MAU_IN_RANGE_PROTECT1_ENABLE_OFS1 0x44C
#define MAU_IN_RANGE_PROTECT2_ENABLE_OFS1 0x454
#define MAU_IN_RANGE_PROTECT3_ENABLE_OFS1 0x45C
#define MAU_IN_RANGE_PROTECT4_ENABLE_OFS1 0x464
#define MAU_OUT_RANGE_PROTECT_ENABLE_OFS1 0x46C

#define MAU_PROTECT0_START_ADDR 	0x490
#define MAU_PROTECT0_END_ADDR 		0x494
#define MAU_PROTECT1_START_ADDR 	0x498
#define MAU_PROTECT1_END_ADDR 		0x49C
#define MAU_PROTECT2_START_ADDR 	0x4A0
#define MAU_PROTECT2_END_ADDR 		0x4A4
#define MAU_PROTECT3_START_ADDR 	0x4A8
#define MAU_PROTECT3_END_ADDR 		0x4AC
#define MAU_PROTECT4_START_ADDR 	0x4B0
#define MAU_PROTECT4_END_ADDR 		0x4B4
#define MAU_OUT_PROTECT_START_ADDR 	0x4B8
#define MAU_OUT_PROTECT_END_ADDR 	0x4BC

#elif defined(CONFIG_TARGET_NS02301)
/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
typedef struct _DMA_CH_MSK {
	// ch 0
	UINT32 REFC: 1;                       	   //< bit0: reserved (auto refresh)
	UINT32 CPU_NS: 1;                            //< CPU_NS
	UINT32 IP_USB_ETH: 1;                      //< USB_0
	UINT32 GRA_0: 1;                           //< GRA_0 (in)
	UINT32 GRA_1: 1;                           //< GRA_1 (in)
	UINT32 GRA_2: 1;                           //< GRA_2 (in)
	UINT32 GRA_3: 1;                           //< GRA_3 (out)
	UINT32 GRA_4: 1;                           //< GRA_4 (out)
	UINT32 GRA2_0: 1;                          //< GRA2_0 (in)
	UINT32 GRA2_1: 1;                          //< GRA2_1 (in)
	UINT32 GRA2_2: 1;                          //< GRA2_2 (out)
	UINT32 JPG_0: 1;                           //< JPG IMG
	UINT32 JPG_1: 1;                           //< JPG BS
	UINT32 JPG_2: 1;                           //< JPG Enc DC out
	UINT32 JPG_3: 1;                           //< JPG Enc DC out
	UINT32 IPE_0: 1;                           //< IPE_0 (in)
	// ch 16
	UINT32 IPE_1: 1;                           //< IPE_1 (in)
	UINT32 IPE_2: 1;                           //< IPE_2 (out)
	UINT32 IPE_3: 1;                           //< IPE_3 (out)
	UINT32 IPE_4: 1;                           //< IPE_4 (out)
	UINT32 IPE_5: 1;                           //< IPE_5 (out)
	UINT32 IPE_6: 1;                           //< IPE_6 (in)
	UINT32 IPE_7: 1;                           //< IPE_7
	UINT32 IPE_8: 1;                           //< IPE_8
	UINT32 IPE_9: 1;                           //< IPE_9
	UINT32 SIE_0: 1;                           //< SIE_0 (out)
	UINT32 SIE_1: 1;                           //< SIE_1 (out)
	UINT32 SIE_2: 1;                           //< SIE_2 (in)
	UINT32 SIE_3: 1;                           //< SIE_3 (in)
	UINT32 SIE2_0: 1;                          //< SIE2_0 (out)
	UINT32 SIE2_1: 1;                          //< SIE2_1 (out)
	UINT32 SIE2_2: 1;                          //< SIE2_2 (in)
	// ch 32
	UINT32 SIE2_3: 1;                          //< SIE2_3 (in)
	UINT32 SIE3_0: 1;                          //< SIE3_0 (out)
	UINT32 SIE3_1: 1;                          //< SIE3_1 (out)
	UINT32 SIE3_2: 1;                          //< SIE3_2 (out)
	UINT32 LARB: 1;                            //< Local arbiter (SIF/BMC/I2C/UART/SPI)
	UINT32 DAI: 1;                             //< DAI
	UINT32 IFE_0: 1;                           //< IFE_0 (in)
	UINT32 IFE_1: 1;                           //< IFE_1 (in)
	UINT32 IFE_2: 1;                           //< IFE_2 (out)
	UINT32 IFE_3: 1;                           //< IFE_3 (out)
	UINT32 IFE_4: 1;                           //< IFE_4 (out)
	UINT32 IFE_5: 1;                           //< IFE_5 (out)
	UINT32 IME_0: 1;                           //< IME_0 (in)
	UINT32 IME_1: 1;                           //< IME_1 (in)
	UINT32 IME_2: 1;                           //< IME_2 (in)
	UINT32 IME_3: 1;                           //< IME_3 (out)
	// ch 48
	UINT32 IME_4: 1;                           //< IME_4 (out)
	UINT32 IME_5: 1;                           //< IME_5 (out)
	UINT32 IME_6: 1;                           //< IME_6 (out)
	UINT32 IME_7: 1;                           //< IME_7 (out)
	UINT32 IME_8: 1;                           //< IME_8 (out)
	UINT32 IME_9: 1;                           //< IME_9 (out)
	UINT32 IME_A: 1;                           //< IME_A (out)
	UINT32 IME_B: 1;                           //< IME_B (out)
	UINT32 IME_C: 1;                           //< IME_C (in)
	UINT32 IME_D: 1;                           //< IME_D (out)
	UINT32 IME_E: 1;                           //< IME_E (in)
	UINT32 IME_F: 1;                           //< IME_F (in)
	UINT32 IME_10: 1;                          //< IME_10 (in)
	UINT32 IME_11: 1;                          //< IME_11 (in)
	UINT32 IME_12: 1;                          //< IME_12 (out)
	UINT32 IME_13: 1;                          //< IME_13 (out)
	// ch 64
	UINT32 IME_14: 1;                          //< IME_14 (in)
	UINT32 IME_15: 1;                          //< IME_15 (out)
	UINT32 ISE_0: 1;                           //< ISE_0 (in)
	UINT32 ISE_1: 1;                           //< ISE_1 (out)
	UINT32 ISE_2: 1;
	UINT32 IDE_0: 1;                           //< IDE_0 (in)
	UINT32 IDE_1: 1;                           //< IDE_1 (in)
	UINT32 IDE_2: 1;                           //< IDE_2 (in/out)
	UINT32 IDE_3: 1;                           //< IDE_3 (in/out)
	UINT32 IDE_4: 1;                           //< IDE_4 (in)
	UINT32 IDE_5: 1;                           //< IDE_5 (in)
	UINT32 SDIO1: 1;                            //< SDIO
	UINT32 SDIO2: 1;                           //< SDIO2
	UINT32 SDIO3: 1;                           //< SDIO3
	UINT32 NAND: 1;                            //< NAND
	UINT32 H264_0: 1;                          //< H.264_0
	// ch 80
	UINT32 H264_1: 1;                          //< H.264_1
	UINT32 H264_3: 1;                          //< H.264_3
	UINT32 H264_4: 1;                          //< H.264_4
	UINT32 H264_5: 1;                          //< H.264_5
	UINT32 H264_6: 1;                          //< H.264_6
	UINT32 H264_8: 1;                          //< H.264_8
	UINT32 H264_9: 1;                          //< H.264_9 (COE)
	UINT32 TSM_0: 1;                           //< TSM_0
	UINT32 TSM_1: 1;                           //< TSM_1
	UINT32 CRYPTO: 1;                          //< CRYPTO (in/out)
	UINT32 HASH: 1;                            //< Hash (in/out)
	UINT32 CNN_0: 1;                           //< CNN_0 (in)
	UINT32 CNN_1: 1;                           //< CNN_1 (in)
	UINT32 CNN_2: 1;                           //< CNN_2 (in)
	UINT32 CNN_3: 1;                           //< CNN_3 (in)
	UINT32 CNN_4: 1;                           //< CNN_4 (out)
	// ch 96
	UINT32 CNN_5: 1;                            //< CNN_5 (out)
	UINT32 CNN_6: 1;                            //< CNN_6 (out)
	UINT32 CNN_7: 1;                            //< CNN_7 (out)
	UINT32 CNN_8: 1;                            //< CNN_8 (out)
	UINT32 NUE2_0: 1;                           //< NUE2_0 (in)
	UINT32 NUE2_1: 1;                           //< NUE2_1 (in)
	UINT32 NUE2_2: 1;                           //< NUE2_2 (in)
	UINT32 NUE2_3: 1;                           //< NUE2_3 (out)
	UINT32 NUE2_4: 1;							//< NUE2_4 (out)
	UINT32 NUE2_5: 1;							//< NUE2_5 (out)
	UINT32 NUE2_6: 1;							//< NUE2_5 (out)
	UINT32 MDBC_0: 1;                           //< MDBC_0 (in)
	UINT32 MDBC_1: 1;							//< MDBC_1 (in)
	UINT32 MDBC_2: 1;							//< MDBC_2 (in)
	UINT32 MDBC_3: 1;							//< MDBC_3 (in)
	UINT32 MDBC_4: 1;							//< MDBC_4 (in)
	// ch 112
	UINT32 MDBC_5: 1;							//< MDBC_5 (in)
	UINT32 MDBC_6: 1;							//< MDBC_6 (out)
	UINT32 MDBC_7: 1;							//< MDBC_7 (out)
	UINT32 HLOAD_0: 1;                          //< Heavy load ch 0
	UINT32 HLOAD_1: 1;							//< Heavy load ch 1
	UINT32 HLOAD_2: 1;							//< Heavy load ch 2
	UINT32 IVE_0: 1;
	UINT32 IVE_1: 1;
	UINT32 IVE_2: 1;
	UINT32 UVC_0: 1;
	UINT32 UVC_1: 1;
	UINT32 VPE_0: 1;
	UINT32 VPE_1: 1;
	UINT32 VPE_2: 1;
	UINT32 VPE_3: 1;
	UINT32 VPE_4: 1;
	// ch 128
	UINT32 YDCC_0: 1;
	UINT32 YDCC_1: 1;
	UINT32 YDCC_2: 1;
	UINT32 YDCC_3: 1;
	UINT32 IFE_6: 1;
	UINT32 reserved1: 27;
	// ch 160
	UINT32 CPU: 1;
	UINT32 reserved2: 31;
	//should br remove
	UINT32 IME_16: 1;
	UINT32 IME_17: 1;
	UINT32 DCE_0: 1;
	UINT32 DCE_1: 1;
	UINT32 DCE_2: 1;
	UINT32 DCE_3: 1;
	UINT32 DCE_4: 1;
	UINT32 DCE_5: 1;
	UINT32 DCE_6: 1;
	UINT32 DCE_7: 1;

} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            // Write protect function set 0
	WPSET_1,            // Write protect function set 1
	WPSET_2,            // Write protect function set 2
	WPSET_3,            // Write protect function set 3
	WPSET_4,            // Write protect function set 4
	WPSET_5,            // Write protect function set 5
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_PROT_REGION {
	DMA_PROT_RGN0,
	DMA_PROT_RGN1,
	DMA_PROT_RGN2,
	DMA_PROT_RGN3,
	DMA_PROT_RGN_TOTAL,
	ENUM_DUMMY4WORD(DMA_PROT_REGION)
} DMA_PROT_REGION;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      // Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       // Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       // Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        // Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	BOOL                en;            // enable this region
	UINT32              starting_addr; // DDR3:must be 4 words alignment
	UINT32              size;          // DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    // protect level
	DMA_PROT_MODE       protect_mode; // in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr[DMA_PROT_RGN_TOTAL];
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // represent channel 64-95
	DMA_CH_GROUP3,          // represent channel 96-127
	DMA_CH_GROUP4,          // represent channel 128-159
	DMA_CH_GROUP5,          // represent channel 160-191
	DMA_CH_GROUP6,          // add to avoid build fail ,should be delete
	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;

#define DMA_PROTECT_CTRL_REG_OFS   0x00
REGDEF_BEGIN(DMA_PROTECT_CTRL_REG)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DMA_WPWD0_EN, 1)
REGDEF_BIT(DMA_WPWD1_EN, 1)
REGDEF_BIT(DMA_WPWD2_EN, 1)
REGDEF_BIT(DMA_WPWD3_EN, 1)
REGDEF_BIT(DMA_WPWD4_EN, 1)
REGDEF_BIT(DMA_WPWD5_EN, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_WPWD0_MODE, 1)
REGDEF_BIT(DMA_WPWD1_MODE, 1)
REGDEF_BIT(DMA_WPWD2_MODE, 1)
REGDEF_BIT(DMA_WPWD3_MODE, 1)
REGDEF_BIT(DMA_WPWD4_MODE, 1)
REGDEF_BIT(DMA_WPWD5_MODE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_WPWD0_SEL, 2)
REGDEF_BIT(DMA_WPWD1_SEL, 2)
REGDEF_BIT(DMA_WPWD2_SEL, 2)
REGDEF_BIT(DMA_WPWD3_SEL, 2)
REGDEF_BIT(DMA_WPWD4_SEL, 2)
REGDEF_BIT(DMA_WPWD5_SEL, 2)
REGDEF_BIT(, 4)
REGDEF_END(DMA_PROTECT_CTRL_REG)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */

#define DMA_PROTECT_REGION_EN_REG0_OFS   0x40
REGDEF_BEGIN(DMA_PROTECT_REGION_EN_REG0)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DRAM_PROTECT0_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT1_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT2_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT3_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_END(DMA_PROTECT_REGION_EN_REG0)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */

#define DMA_PROTECT_REGION_EN_REG1_OFS   0x44
REGDEF_BEGIN(DMA_PROTECT_REGION_EN_REG1)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DRAM_PROTECT4_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT5_REGIONS_EN, 4)
REGDEF_BIT(, 20)
REGDEF_END(DMA_PROTECT_REGION_EN_REG1)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */

#define DMA_PROTECT_STARTADDR0_REG0_OFS  0x100
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG0_OFS  0x104
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG1_OFS  0x108
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG1_OFS  0x10C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG2_OFS  0x110
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG2_OFS   0x114
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG3_OFS  0x118
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG3_OFS   0x11C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG0_OFS  0x120
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG0_OFS  0x124
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG1_OFS  0x128
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG1_OFS  0x12C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG2_OFS  0x130
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG2_OFS   0x134
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG3_OFS  0x138
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG3_OFS   0x13C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG0_OFS  0x140
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG0_OFS  0x144
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG1_OFS  0x148
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG1_OFS  0x14C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG2_OFS  0x150
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG2_OFS   0x154
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG3_OFS  0x158
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG3_OFS   0x15C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG0_OFS  0x160
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG0_OFS  0x164
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG1_OFS  0x168
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG1_OFS  0x16C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG2_OFS  0x170
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG2_OFS   0x174
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG3_OFS  0x178
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG3_OFS   0x17C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG0_OFS  0x180
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG0_OFS  0x184
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG1_OFS  0x188
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG1_OFS  0x18C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG2_OFS  0x190
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG2_OFS   0x194
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG3_OFS  0x198
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG3_OFS   0x19C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG0_OFS  0x1A0
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG0_OFS  0x1A4
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG1_OFS  0x1A8
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG1_OFS  0x1AC
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG2_OFS  0x1B0
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG2_OFS   0x1B4
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG3_OFS  0x1B8
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG3_OFS   0x1BC
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

// 0x88 ~ 0x8C: Reserved

#define DMA_PROTECT_RANGE0_MSK0_REG_OFS 0x200
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK0_REG" end */

#define DMA_PROTECT_RANGE0_MSK1_REG_OFS 0x204
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK1_REG" end */

#define DMA_PROTECT_RANGE0_MSK2_REG_OFS 0x208
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK2_REG" end */

#define DMA_PROTECT_RANGE0_MSK3_REG_OFS 0x20C
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK3_REG" end */

#define DMA_PROTECT_RANGE0_MSK4_REG_OFS 0x210
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK4_REG" end */

#define DMA_PROTECT_RANGE0_MSK5_REG_OFS 0x214
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK5_REG" end */

#define DMA_PROTECT_RANGE1_MSK0_REG_OFS 0x220
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK0_REG" end */

#define DMA_PROTECT_RANGE1_MSK1_REG_OFS 0x224
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK1_REG" end */

#define DMA_PROTECT_RANGE1_MSK2_REG_OFS 0x228
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK2_REG" end */

#define DMA_PROTECT_RANGE1_MSK3_REG_OFS 0x22C
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK3_REG" end */

#define DMA_PROTECT_RANGE1_MSK4_REG_OFS 0x230
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK4_REG" end */

#define DMA_PROTECT_RANGE1_MSK5_REG_OFS 0x234
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK5_REG" end */

#define DMA_PROTECT_RANGE2_MSK0_REG_OFS 0x240
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE2_MSK1_REG_OFS 0x244
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE2_MSK2_REG_OFS 0x248
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE2_MSK3_REG_OFS 0x24C
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE2_MSK4_REG_OFS 0x250
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE2_MSK5_REG_OFS 0x254
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE3_MSK0_REG_OFS 0x260
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE3_MSK1_REG_OFS 0x264
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE3_MSK2_REG_OFS 0x268
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE3_MSK3_REG_OFS 0x26C
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE3_MSK4_REG_OFS 0x270
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE3_MSK5_REG_OFS 0x274
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE4_MSK0_REG_OFS 0x280
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE4_MSK1_REG_OFS 0x284
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE4_MSK2_REG_OFS 0x288
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE4_MSK3_REG_OFS 0x28C
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE4_MSK4_REG_OFS 0x290
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE4_MSK5_REG_OFS 0x294
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE5_MSK0_REG_OFS 0x2A0
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE5_MSK1_REG_OFS 0x2A4
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE5_MSK2_REG_OFS 0x2A8
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE5_MSK3_REG_OFS 0x2AC
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE5_MSK4_REG_OFS 0x2B0
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE5_MSK5_REG_OFS 0x2B4
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

///// PROTECT /////
#define DMA_PROTECT_INTCTRL_REG_OFS   0x64
REGDEF_BEGIN(DMA_PROTECT_INTCTRL_REG)    /* declare register cache type "DMA_PROTECT_INTCTRL_REG_OFS" begin */
REGDEF_BIT(DMA_WPWD0_INTEN0, 1)
REGDEF_BIT(DMA_WPWD0_INTEN1, 1)
REGDEF_BIT(DMA_WPWD0_INTEN2, 1)
REGDEF_BIT(DMA_WPWD0_INTEN3, 1)
REGDEF_BIT(DMA_WPWD1_INTEN0, 1)
REGDEF_BIT(DMA_WPWD1_INTEN1, 1)
REGDEF_BIT(DMA_WPWD1_INTEN2, 1)
REGDEF_BIT(DMA_WPWD1_INTEN3, 1)
REGDEF_BIT(DMA_WPWD2_INTEN0, 1)
REGDEF_BIT(DMA_WPWD2_INTEN1, 1)
REGDEF_BIT(DMA_WPWD2_INTEN2, 1)
REGDEF_BIT(DMA_WPWD2_INTEN3, 1)
REGDEF_BIT(DMA_WPWD3_INTEN0, 1)
REGDEF_BIT(DMA_WPWD3_INTEN1, 1)
REGDEF_BIT(DMA_WPWD3_INTEN2, 1)
REGDEF_BIT(DMA_WPWD3_INTEN3, 1)
REGDEF_BIT(DMA_WPWD4_INTEN0, 1)
REGDEF_BIT(DMA_WPWD4_INTEN1, 1)
REGDEF_BIT(DMA_WPWD4_INTEN2, 1)
REGDEF_BIT(DMA_WPWD4_INTEN3, 1)
REGDEF_BIT(DMA_WPWD5_INTEN0, 1)
REGDEF_BIT(DMA_WPWD5_INTEN1, 1)
REGDEF_BIT(DMA_WPWD5_INTEN2, 1)
REGDEF_BIT(DMA_WPWD5_INTEN3, 1)
REGDEF_BIT(AUTO_REFRESH_TIMEOUT_INTEN, 1)
REGDEF_BIT(DMA_OUTRANGE_INTEN, 1)
REGDEF_BIT(DMA_UPDATE_USAGE_INTEN, 1)
REGDEF_BIT(, 5)
REGDEF_END(DMA_PROTECT_INTCTRL_REG)      /* declare register cache type "DMA_PROTECT_INTCTRL_REG_OFS" end */

#define DMA_PROTECT_INTSTS_REG_OFS   0x68
REGDEF_BEGIN(DMA_PROTECT_INTSTS_REG)    /* declare register cache type "DMA_PROTECT_INTSTS_REG_OFS" begin */
REGDEF_BIT(DMA_WPWD0_STS0, 1)
REGDEF_BIT(DMA_WPWD0_STS1, 1)
REGDEF_BIT(DMA_WPWD0_STS2, 1)
REGDEF_BIT(DMA_WPWD0_STS3, 1)
REGDEF_BIT(DMA_WPWD1_STS0, 1)
REGDEF_BIT(DMA_WPWD1_STS1, 1)
REGDEF_BIT(DMA_WPWD1_STS2, 1)
REGDEF_BIT(DMA_WPWD1_STS3, 1)
REGDEF_BIT(DMA_WPWD2_STS0, 1)
REGDEF_BIT(DMA_WPWD2_STS1, 1)
REGDEF_BIT(DMA_WPWD2_STS2, 1)
REGDEF_BIT(DMA_WPWD2_STS3, 1)
REGDEF_BIT(DMA_WPWD3_STS0, 1)
REGDEF_BIT(DMA_WPWD3_STS1, 1)
REGDEF_BIT(DMA_WPWD3_STS2, 1)
REGDEF_BIT(DMA_WPWD3_STS3, 1)
REGDEF_BIT(DMA_WPWD4_STS0, 1)
REGDEF_BIT(DMA_WPWD4_STS1, 1)
REGDEF_BIT(DMA_WPWD4_STS2, 1)
REGDEF_BIT(DMA_WPWD4_STS3, 1)
REGDEF_BIT(DMA_WPWD5_STS0, 1)
REGDEF_BIT(DMA_WPWD5_STS1, 1)
REGDEF_BIT(DMA_WPWD5_STS2, 1)
REGDEF_BIT(DMA_WPWD5_STS3, 1)
REGDEF_BIT(AUTO_REFRESH_TIMEOUT_STS, 1)
REGDEF_BIT(DMA_OUTRANGE_STS, 1)
REGDEF_BIT(DMA_UPDATE_USAGE_STS, 1)
REGDEF_BIT(, 5)
REGDEF_END(DMA_PROTECT_INTSTS_REG)      /* declare register cache type "DMA_PROTECT_INTSTS_REG_OFS" end */

#define DMA_PROTECT0_CHSTS_REG_OFS   0x6C
REGDEF_BEGIN(DMA_PROTECT0_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT0_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT1_CHSTS_REG_OFS   0x70
REGDEF_BEGIN(DMA_PROTECT1_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT1_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT2_CHSTS_REG_OFS   0x74
REGDEF_BEGIN(DMA_PROTECT2_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT2_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT3_CHSTS_REG_OFS   0x78
REGDEF_BEGIN(DMA_PROTECT3_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT3_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT4_CHSTS_REG_OFS   0x7C
REGDEF_BEGIN(DMA_PROTECT4_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT4_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT5_CHSTS_REG_OFS   0x80
REGDEF_BEGIN(DMA_PROTECT5_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT5_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT0_DETECT_ADDR0_REG_OFS    0x600
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT0_DETECT_ADDR1_REG_OFS    0x604
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT0_DETECT_ADDR2_REG_OFS    0x608
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT0_DETECT_ADDR3_REG_OFS    0x60C
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT1_DETECT_ADDR0_REG_OFS    0x610
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT1_DETECT_ADDR1_REG_OFS    0x614
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT1_DETECT_ADDR2_REG_OFS    0x618
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT1_DETECT_ADDR3_REG_OFS    0x61C
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT2_DETECT_ADDR0_REG_OFS    0x620
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT2_DETECT_ADDR1_REG_OFS    0x624
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT2_DETECT_ADDR2_REG_OFS    0x628
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT2_DETECT_ADDR3_REG_OFS    0x62C
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT3_DETECT_ADDR0_REG_OFS    0x630
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT3_DETECT_ADDR1_REG_OFS    0x634
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT3_DETECT_ADDR2_REG_OFS    0x638
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT3_DETECT_ADDR3_REG_OFS    0x63C
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT4_DETECT_ADDR0_REG_OFS    0x640
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT4_DETECT_ADDR1_REG_OFS    0x644
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT4_DETECT_ADDR2_REG_OFS    0x648
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT4_DETECT_ADDR3_REG_OFS    0x64C
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT5_DETECT_ADDR0_REG_OFS    0x650
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT5_DETECT_ADDR1_REG_OFS    0x654
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT5_DETECT_ADDR2_REG_OFS    0x658
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT5_DETECT_ADDR3_REG_OFS    0x65C
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#else
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // represent channel 64-95
	DMA_CH_GROUP3,          // represent channel 96-127
	DMA_CH_GROUP4,          // represent channel 128-159
	DMA_CH_GROUP5,          // represent channel 160-191
	DMA_CH_GROUP6,          // represent channel 192-223
	DMA_CH_GROUP7,          // represent channel 224-255

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;

/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
typedef struct _DMA_CH_MSK {
	// ch 0
	UINT32 CPU_S: 1;
	UINT32 CPU_NS: 1;
	UINT32 reserved0: 29;
	UINT32 HVYLD: 1;

	// ch 1
	UINT32 SDIO: 1;
	UINT32 SDIO2: 1;
	UINT32 SDIO3: 1;
	UINT32 SMC: 1;
	UINT32 ETH: 1;
	UINT32 ETH2: 1;
	UINT32 USB2: 1;
	UINT32 USB3: 1;
	UINT32 USB3_2: 1;
	UINT32 SATA: 1;
	UINT32 SATA2: 1;
	UINT32 PCIE: 1;
	UINT32 PCIE2: 1;
	UINT32 MSI: 1;
	UINT32 MSI2: 1;
	UINT32 N25: 1;
	UINT32 LARB_1_4_SPI3: 1;
	UINT32 DAI_1_5: 1;
	UINT32 GPE_1_2: 1;
	UINT32 GRP_1_3: 1;
	UINT32 AGE: 1;
	UINT32 TSE: 1;
	UINT32 UVCP_1_2: 1;
	UINT32 XOR: 1;
	UINT32 HWCP: 1;
	UINT32 SCE: 1;
	UINT32 HASH: 1;
	UINT32 reserved1: 4;
	UINT32 HVYLD2: 1;

	// ch 2
	UINT32 VENC_0_1: 1;
	UINT32 VTRC: 1;
	UINT32 reserved2: 29;
	UINT32 HVYLD3: 1;

	// ch 3
	UINT32 IFE: 1;
	UINT32 IFE2: 1;
	UINT32 IPE: 1;
	UINT32 IPE2: 1;
	UINT32 ISE: 1;
	UINT32 ISE2: 1;
	UINT32 IME: 1;
	UINT32 IME2: 1;
	UINT32 DRE: 1;
	UINT32 reserved4: 1;
	UINT32 VDEC_0: 1;
	UINT32 VDEC_1_4: 1;
	UINT32 JPEG: 1;
	UINT32 JPEG2: 1;
	UINT32 JPEG3: 1;
	UINT32 reserved5: 1;
	UINT32 SIE_1: 1;
	UINT32 SIE2_1: 1;
	UINT32 SIE3_1: 1;
	UINT32 SIE4_1: 1;
	UINT32 SIE5_1: 1;
	UINT32 SIE6_1: 1;
	UINT32 SIE7_1: 1;
	UINT32 SIE8_1: 1;
	UINT32 SIE9_1: 1;
	UINT32 SIE10_1: 1;
	UINT32 SIE11_1: 1;
	UINT32 SIE12_1: 1;
	UINT32 reserved6: 4;


	// ch 4
	UINT32 VPE_0_1: 1;
	UINT32 VPE2_0_1: 1;
	UINT32 VPE3_0_1: 1;
	UINT32 reserved7: 29;

	// ch 5
	UINT32 LCD: 1;
	UINT32 LCD2: 1;
	UINT32 LCD3: 1;
	UINT32 CSITX: 1;
	UINT32 CSITX2: 1;
	UINT32 reserved8: 3;
	UINT32 VIE: 1;
	UINT32 VIE2: 1;
	UINT32 reserved9: 6;
	UINT32 SIE_0: 1;
	UINT32 SIE2_0: 1;
	UINT32 SIE3_0: 1;
	UINT32 SIE4_0: 1;
	UINT32 SIE5_0: 1;
	UINT32 SIE6_0: 1;
	UINT32 SIE7_0: 1;
	UINT32 SIE8_0: 1;
	UINT32 SIE9_0: 1;
	UINT32 SIE10_0: 1;
	UINT32 SIE11_0: 1;
	UINT32 SIE12_0: 1;
	UINT32 reserved10: 4;

	// ch 6
	UINT32 GPU: 1;
	UINT32 DSP: 1;
	UINT32 DSP2: 1;
	UINT32 reserved11: 13;
	UINT32 LME: 1;
	UINT32 SDE: 1;
	UINT32 IVE: 1;
	UINT32 MDBC: 1;
	UINT32 TRKE: 1;
	UINT32 NUE2: 1;
	UINT32 reserved12: 10;

	// ch 7
	UINT32 CONV: 1;
	UINT32 CONV2: 1;
	UINT32 CONV3: 1;
	UINT32 CONV4: 1;
	UINT32 reserved13: 12;
	UINT32 JM_JMISP: 1;
	UINT32 UTL: 1;
	UINT32 ROU: 1;
	UINT32 CAL: 1;
	UINT32 LSU: 1;
	UINT32 PPU: 1;
	// backward compatible
	UINT32 DCE: 1;
	UINT32 VENC_0: 1;
	UINT32 VENC_1: 1;
	UINT32 VPE_0: 1;
	UINT32 VPE_1: 1;
	UINT32 NUE: 1;
	UINT32 CNN: 1;
	UINT32 CNN2: 1;
	UINT32 DIS: 1;
	UINT32 reserved14: 1;

} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter
	DDR_ARB_2,                           ///< DDR Arbiter2

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            // Write protect function set 0
	WPSET_1,            // Write protect function set 1
	WPSET_2,            // Write protect function set 2
	WPSET_3,            // Write protect function set 3
	WPSET_4,            // Write protect function set 4
	WPSET_5,            // Write protect function set 5
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      // Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       // Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       // Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        // Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	BOOL                en;            // enable this region
	u64                 starting_addr; // DDR3:must be 4 words alignment
	u32                 size;          // DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;      // protect level
	DMA_PROT_MODE       protect_mode; // in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

/*
    @name DMA outstanding (NO USE)

    DMA outstanding setting

    @note Used in dma_set_channel_outstanding()
*/
typedef enum {
	DMA_CH_OUTSTANDING_CNN_0,           // CNN input
	DMA_CH_OUTSTANDING_CNN_1,           // CNN input
	DMA_CH_OUTSTANDING_CNN_2,           // CNN input
	DMA_CH_OUTSTANDING_CNN_3,           // CNN input
	DMA_CH_OUTSTANDING_CNN_4,           // CNN output
	DMA_CH_OUTSTANDING_CNN_5,           // CNN output

	DMA_CH_OUTSTANDING_CNN2_0,
	DMA_CH_OUTSTANDING_CNN2_1,
	DMA_CH_OUTSTANDING_CNN2_2,
	DMA_CH_OUTSTANDING_CNN2_3,
	DMA_CH_OUTSTANDING_CNN2_4,
	DMA_CH_OUTSTANDING_CNN2_5,

	DMA_CH_OUTSTANDING_NUE_0,           // NUE input
	DMA_CH_OUTSTANDING_NUE_1,           // NUE input
	DMA_CH_OUTSTANDING_NUE_2,           // NUE output

	DMA_CH_OUTSTANDING_CNN_ALL,
	DMA_CH_OUTSTANDING_CNN2_ALL,
	DMA_CH_OUTSTANDING_NUE_ALL,
	ENUM_DUMMY4WORD(DMA_CH_OUTSTANDING)
} DMA_CH_OUTSTANDING;


typedef enum _DRAM_CONSUME_BW_DEGREE {
	DRAM_CONSUME_EASY_LOADING = 0x0,
	DRAM_CONSUME_NORMAL_LOADING,
	DRAM_CONSUME_HEAVY_LOADING,

	DRAM_CONSUME_CH_DISABLE,

	ENUM_DUMMY4WORD(DRAM_CONSUME_BW_DEGREE)
} DRAM_CONSUME_BW_DEGREE;


typedef struct _DRAM_CONSUME_ATTR {
	DDR_ARB             id;       // Dram id
	DRAM_CONSUME_BW_DEGREE  load_degree;
	u64                     addr;
	u32                     size;
	BOOL                    is_start;
} DRAM_CONSUME_ATTR, *PDRAM_CONSUME_ATTR;

// protect reg
// protect reg
#define DMA_DETECT_CH0_W_REG_OFS   0x00
#define DMA_DETECT_CH1_W_REG_OFS   0x04
#define DMA_DETECT_CH2_W_REG_OFS   0x08
#define DMA_DETECT_CH3_W_REG_OFS   0x0C
#define DMA_DETECT_CH4_W_REG_OFS   0x10
#define DMA_DETECT_CH5_W_REG_OFS   0x14
#define DMA_DETECT_CH6_W_REG_OFS   0x18
#define DMA_DETECT_CH7_W_REG_OFS   0x1C

#define DMA_DETECT_CH0_R_REG_OFS   0x20
#define DMA_DETECT_CH1_R_REG_OFS   0x24
#define DMA_DETECT_CH2_R_REG_OFS   0x28
#define DMA_DETECT_CH3_R_REG_OFS   0x2C
#define DMA_DETECT_CH4_R_REG_OFS   0x30
#define DMA_DETECT_CH5_R_REG_OFS   0x34
#define DMA_DETECT_CH6_R_REG_OFS   0x38
#define DMA_DETECT_CH7_R_REG_OFS   0x3C

#define DMA_PROTECT_CH0_W_REG_OFS   0x40
#define DMA_PROTECT_CH1_W_REG_OFS   0x44
#define DMA_PROTECT_CH2_W_REG_OFS   0x48
#define DMA_PROTECT_CH3_W_REG_OFS   0x4C
#define DMA_PROTECT_CH4_W_REG_OFS   0x50
#define DMA_PROTECT_CH5_W_REG_OFS   0x54
#define DMA_PROTECT_CH6_W_REG_OFS   0x58
#define DMA_PROTECT_CH7_W_REG_OFS   0x5C

#define DMA_PROTECT_CH0_R_REG_OFS   0x60
#define DMA_PROTECT_CH1_R_REG_OFS   0x64
#define DMA_PROTECT_CH2_R_REG_OFS   0x68
#define DMA_PROTECT_CH3_R_REG_OFS   0x6C
#define DMA_PROTECT_CH4_R_REG_OFS   0x70
#define DMA_PROTECT_CH5_R_REG_OFS   0x74
#define DMA_PROTECT_CH6_R_REG_OFS   0x78
#define DMA_PROTECT_CH7_R_REG_OFS   0x7C

#define DMA_RANGE_SEL_REG_OFS       0x80
REGDEF_BEGIN(DMA_RANGE_SEL_REG)
REGDEF_BIT(range_sel_w, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(range_sel_r, 8)
REGDEF_BIT(, 8)
REGDEF_END(DMA_RANGE_SEL_REG)

#define DMA_PROTECT_START_ADDR_REG_OFS      0x88
#define DMA_PROTECT_END_ADDR_REG_OFS        0x8C
#endif

#endif /* _ASM_ARMV8_GIC_H_ */
