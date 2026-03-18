/*

    @file       dai_int.h
    @ingroup    mIDrvAud_DAI

    @brief      DAI internal header file, This file is the header file that define register for DAI module

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

*/


#ifndef _DAI_INT_H
#define _DAI_INT_H

#ifdef __KERNEL__
#include <rcw_macro.h>
#include "kwrap/type.h"
#include <asm/io.h>
#include <linux/soc/nvt/fmem.h>
#include "kdrv_audioio_drv.h"

#define dma_get_noncache_addr(parm) parm
#define dma_get_phy_addr(parm) (parm)
#elif defined(__FREERTOS)
#include <rcw_macro.h>
#include "kwrap/type.h"
#include "plat/interrupt.h" 
#define dma_get_noncache_addr(parm) parm
#define dma_get_phy_addr(parm) dma_getPhyAddr(parm)
#endif

#include "dai.h"
#include "dai_platform.h"
#include <nvt_api_ver.h>

#include <io_address.h>
//#include <sys.h>

/*
    @addtogroup mIDrvAud_DAI
*/
//@{

#define DAI_DBG_MSG                      DISABLE
#define DAI2_DBG_MSG                     DISABLE
#define DAI3_DBG_MSG                     DISABLE
#define DAI4_DBG_MSG                     DISABLE
#define DAI5_DBG_MSG                     DISABLE


#ifdef __KERNEL__
extern void __iomem *dai_reg_base[PLATFORM_MAX_CHIP_CNT][MODULE_REG_NUM];


#define DAI_SETREG(id,ofs, value)       writel((value), (dai_reg_base[id][0]+(ofs)))
#define DAI_GETREG(id,ofs)              readl(dai_reg_base[id][0]+(ofs))

#else


#define INT_ID_DAI_EP 0
#define INT_ID_DAI2_EP 0
#define INT_ID_DAI3_EP 0
#define INT_ID_DAI4_EP 0
#define INT_ID_DAI5_EP 0  // rtos ep is useless
#if defined(_PCIE_EMU_ON_)
#define DAI_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_DAI_REG_BASE)+(ofs))           
#define DAI_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI_REG_BASE)+(ofs), (value))    
#define DAI2_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_DAI2_REG_BASE)+(ofs))           
#define DAI2_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI2_REG_BASE)+(ofs), (value))    
#define DAI3_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_DAI3_REG_BASE)+(ofs))           
#define DAI3_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI3_REG_BASE)+(ofs), (value))    
#define DAI4_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_DAI4_REG_BASE)+(ofs))           
#define DAI4_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI4_REG_BASE)+(ofs), (value))    
#define DAI5_GETREG(ofs)          INW(APB_LOCAL2PCIE(IOADDR_DAI5_REG_BASE)+(ofs))           
#define DAI5_SETREG(ofs,value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI5_REG_BASE)+(ofs), (value)) 
#else
#if 1
    #define DAI_SETREG(id,ofs, value)       OUTW((UINT32 *)(IOADDR_DAI_REG_BASE+(ofs)), (value))
    #define DAI_GETREG(id,ofs)              INW((UINT32 *)(IOADDR_DAI_REG_BASE+(ofs)))
    #define DAI2_SETREG(id,ofs, value)       OUTW((UINT32 *)(IOADDR_DAI2_REG_BASE+(ofs)), (value))
    #define DAI2_GETREG(id,ofs)              INW((UINT32 *)(IOADDR_DAI2_REG_BASE+(ofs)))
    #define DAI3_SETREG(id,ofs, value)       OUTW((UINT32 *)(IOADDR_DAI3_REG_BASE+(ofs)), (value))
    #define DAI3_GETREG(id,ofs)              INW((UINT32 *)(IOADDR_DAI3_REG_BASE+(ofs)))
    #define DAI4_SETREG(id,ofs, value)       OUTW((UINT32 *)(IOADDR_DAI4_REG_BASE+(ofs)), (value))
    #define DAI4_GETREG(id,ofs)              INW((UINT32 *)(IOADDR_DAI4_REG_BASE+(ofs)))
    #define DAI5_SETREG(id,ofs, value)       OUTW((UINT32 *)(IOADDR_DAI5_REG_BASE+(ofs)), (value))
    #define DAI5_GETREG(id,ofs)              INW((UINT32 *)(IOADDR_DAI5_REG_BASE+(ofs)))

#else
    #if 0 // PCIE
    #define DAI_GETREG(id,ofs)            INW(APB_LOCAL2PCIE(IOADDR_DAI_REG_BASE)+(ofs))           
    #define DAI_SETREG(id,ofs, value)     OUTW(APB_LOCAL2PCIE(IOADDR_DAI_REG_BASE)+(ofs), (value))    
    #define DAI2_GETREG(id,ofs)           INW(APB_LOCAL2PCIE(IOADDR_DAI2_REG_BASE)+(ofs))           
    #define DAI2_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI2_REG_BASE)+(ofs), (value))    
    #define DAI3_GETREG(id,ofs)           INW(APB_LOCAL2PCIE(IOADDR_DAI3_REG_BASE)+(ofs))           
    #define DAI3_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI3_REG_BASE)+(ofs), (value))    
    #define DAI4_GETREG(id,ofs)           INW(APB_LOCAL2PCIE(IOADDR_DAI4_REG_BASE)+(ofs))           
    #define DAI4_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI4_REG_BASE)+(ofs), (value))    
    #define DAI5_GETREG(id,ofs)           INW(APB_LOCAL2PCIE(IOADDR_DAI5_REG_BASE)+(ofs))           
    #define DAI5_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE(IOADDR_DAI5_REG_BASE)+(ofs), (value)) 
    #else //PCIE2
    #define DAI_GETREG(id,ofs)            INW(APB_LOCAL2PCIE2(IOADDR_DAI_REG_BASE)+(ofs))           
    #define DAI_SETREG(id,ofs, value)     OUTW(APB_LOCAL2PCIE2(IOADDR_DAI_REG_BASE)+(ofs), (value))    
    #define DAI2_GETREG(id,ofs)           INW(APB_LOCAL2PCIE2(IOADDR_DAI2_REG_BASE)+(ofs))           
    #define DAI2_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE2(IOADDR_DAI2_REG_BASE)+(ofs), (value))    
    #define DAI3_GETREG(id,ofs)           INW(APB_LOCAL2PCIE2(IOADDR_DAI3_REG_BASE)+(ofs))           
    #define DAI3_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE2(IOADDR_DAI3_REG_BASE)+(ofs), (value))    
    #define DAI4_GETREG(id,ofs)           INW(APB_LOCAL2PCIE2(IOADDR_DAI4_REG_BASE)+(ofs))           
    #define DAI4_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE2(IOADDR_DAI4_REG_BASE)+(ofs), (value))    
    #define DAI5_GETREG(id,ofs)           INW(APB_LOCAL2PCIE2(IOADDR_DAI5_REG_BASE)+(ofs))           
    #define DAI5_SETREG(id,ofs, value)    OUTW(APB_LOCAL2PCIE2(IOADDR_DAI5_REG_BASE)+(ofs), (value)) 
    #endif
#endif

#endif
#endif

//
//  DAI register access definition
//
#define DAI_REG_ADDR(ofs)       (IOADDR_DAI_REG_BASE+(ofs))
//#define DAI2_REG_ADDR(ofs)      (IOADDR_DAI2_REG_BASE+(ofs))


/* I2S order used vecotr*/
extern const UINT32	 DAI_I2S_DataOrder_2[2][2];
extern const UINT32	 DAI_I2S_DataOrder_4[2][4];
extern const UINT32	 DAI_I2S_DataOrder_6[2][6];
extern const UINT32	 DAI_I2S_DataOrder_8[2][8];
extern const UINT32	 DAI_I2S_DataOrder_10[2][10];
extern const UINT32	 DAI_I2S_DataOrder_16[2][16];
extern const UINT32	 DAI_I2S_DataOrder_20[2][20];

/**
    DAI channel ID

*/
typedef enum {
        DAI_ID_1,                          ///< DAI_ID Controller
        DAI_ID_2,                          ///< DAI_ID Controller 2
		DAI_ID_3,                          ///< DAI_ID Controller 3
		DAI_ID_4,                          ///< DAI_ID Controller 4
		DAI_ID_5,                          ///< DAI_ID Controller 5
        
        ENUM_DUMMY4WORD(DAI_ID)
} DAI_ID;


#define DAI_I2S_CLK_OFS_MAX    			0x13E


/*
    DAI I2S Frame clock ratio

    @note For dai_set_i2s_config(DAI_I2SCONFIG_ID_CLKRATIO)
*/
typedef enum {
	DAI_I2SFRAMECLKR_32BIT,     ///< FrameSync = 32 BitClk. NT96680 valid.
	DAI_I2SFRAMECLKR_48BIT,     ///< FrameSync = 48 BitClk
	DAI_I2SFRAMECLKR_64BIT,     ///< FrameSync = 64 BitClk. NT96680 valid.
	DAI_I2SFRAMECLKR_96BIT,     ///< FrameSync = 96 BitClk
	DAI_I2SFRAMECLKR_128BIT,    ///< FrameSync = 128 BitClk. NT96680 valid.
	DAI_I2SFRAMECLKR_192BIT,    ///< FrameSync = 192 BitClk
	DAI_I2SFRAMECLKR_256BIT,    ///< FrameSync = 256 BitClk. NT96680 valid.

	ENUM_DUMMY4WORD(DAI_I2SFRAMECLKR)
} DAI_I2SFRAMECLKR;


//
// Internal function prototype
//
extern DAI_INTERRUPT    dai_wait_interrupt(int id, DAI_INTERRUPT wait_flag);
// 530
/*
extern void             dai_select_pinmux(BOOL enable);
extern void             dai_select_mclk_pinmux(BOOL enable);

extern void             dai_debug(BOOL enable);
extern void             dai_debug_eac(BOOL enable, BOOL mode_ad);
extern void             dai_set_debug_dma_para(UINT32 buf_addr, UINT32 buf_size);
extern BOOL             dai_get_debug_status(void);
extern void             dai_clr_debug_status(void);
*/

extern ER               dai_lock(int id);
extern ER               dai_unlock(int id);
extern UINT64 			dai_platform_va2pa(uintptr_t addr);
extern UINT64 			dai_platform_pcie_addr(int loc_chipid, int tar_ddrid, uintptr_t addr);

#ifdef __KERNEL__
extern void dai_tasklet(int id);
extern void _dai_enableclk_platform(int id);
extern void _dai_disableclk_platform(int id);
extern void _dai_setclkrate_platform(int id, unsigned long clkrate);

#endif

//@}

#endif
