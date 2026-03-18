#ifndef __IDE_PLATFORM_H_
#define __IDE_PLATFORM_H__

#if (defined __UITRON || defined __ECOS)
#include <string.h>
#include "DrvCommon.h"
#include "interrupt.h"
#include "Utility.h"
#include "dma.h"
#include "SxCmd.h"
#include "top.h"
#include "nvtDrvProtected.h"
#include "kernel.h"
#include "DxSys.h"
#include "pll.h"
#include "pll_protected.h"
#include "Perf.h"
#include "ide.h"
#elif defined __FREERTOS
#include <string.h>
#include <stdlib.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>
#include <kwrap/task.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "ide.h"
#include "ide_dbg.h"
#include "comm/timer.h"
#include "kdrv_videoout/kdrv_vdoout.h"
#include "top.h"
#include "dma_protected.h"
#include "nvt-sramctl.h"
#define _EMULATION_             (0)	// no emu macro in linux
// manually defined here
#define FLGPTN_IDE              FLGPTN_BIT(0)
#define FLGPTN_IDE_2            FLGPTN_BIT(1)
#define FLGPTN_IDE_3            FLGPTN_BIT(2)
#define FLGPTN_IDE_4            FLGPTN_BIT(3)
#define FLGPTN_IDE_5            FLGPTN_BIT(4)
#define FLGPTN_IDE_DMA_DONE     FLGPTN_BIT(5)

#define FLGPTN_IDE2              FLGPTN_BIT(0)
#define FLGPTN_IDE2_2            FLGPTN_BIT(1)
#define FLGPTN_IDE2_3            FLGPTN_BIT(2)
#define FLGPTN_IDE2_4            FLGPTN_BIT(3)
#define FLGPTN_IDE2_5            FLGPTN_BIT(4)
#define FLGPTN_IDE2_DMA_DONE     FLGPTN_BIT(5)


//extern UINT32 IOADDR_IDE2_REG_BASE;
#else
//#include <rcw_macro.h>
#include <common.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/rcw_macro_bit.h>
//#include <linux/spinlock.h>
//#include "kwrap/type.h"//a header for basic variable type
//#include "kwrap/flag.h"
//#include "kwrap/semaphore.h"
//#include "kwrap/task.h"
//#include <linux/soc/nvt/fmem.h>
//#include <nvt-sramctl.h>
#include <linux/delay.h>
//#include <linux/clk.h>
//#include <top.h>
//#include "ide_drv.h"
//#include "ide_dbg.h"
#include <asm/arch/nvt_ide.h>
//#include "comm/drvdump.h"
//#include "kdrv_videoout/kdrv_vdoout.h"
#include <asm/arch/kdrv_vdoout.h>
typedef unsigned int        	FLGPTN;                     ///< Flag patterns
typedef unsigned int        	*PFLGPTN;                   ///< Flag patterns (Pointer)
#define FLGPTN_BIT(n)       	((FLGPTN)(1 << (n)))        ///< Bit of flag pattern
typedef unsigned long           ULONG;

#define _EMULATION_             (0)	// no emu macro in linux
// manually defined here
#define FLGPTN_IDE              FLGPTN_BIT(0)
#define FLGPTN_IDE_2            FLGPTN_BIT(1)
#define FLGPTN_IDE_3            FLGPTN_BIT(2)
#define FLGPTN_IDE_4            FLGPTN_BIT(3)
#define FLGPTN_IDE_5            FLGPTN_BIT(4)
#define FLGPTN_IDE_DMA_DONE     FLGPTN_BIT(5)

#define FLGPTN_IDE2              FLGPTN_BIT(0)
#define FLGPTN_IDE2_2            FLGPTN_BIT(1)
#define FLGPTN_IDE2_3            FLGPTN_BIT(2)
#define FLGPTN_IDE2_4            FLGPTN_BIT(3)
#define FLGPTN_IDE2_5            FLGPTN_BIT(4)
#define FLGPTN_IDE2_DMA_DONE     FLGPTN_BIT(5)
#define dma_getNonCacheAddr(addr) addr


#endif

/*
extern IDEDATA_TYPE IOADDR_LCD_REG_BASE;
extern IDEDATA_TYPE IOADDR_LCD2_REG_BASE;
extern IDEDATA_TYPE IOADDR_LCD3_REG_BASE;
*/

/*
    @name   IDE clock divider

    IDE clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD_CLKDIV).
*/
//@{
#define PLL_LCD_CLKDIV(x)           ((x) << (PLL_CLKSEL_LCD_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD_CLKDIV)
//@}

/*
    @name   IDE Output Interface clock divider

    IDE Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_LCD_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD_OUTIF_CLKDIV)
//@}

/*
    @name   IDE2 clock divider

    IDE2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD_CLKDIV).
*/
//@{
#define PLL_LCD2_CLKDIV(x)           ((x) << (PLL_CLKSEL_LCD2_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD2_CLKDIV)
//@}

/*
    @name   IDE2 Output Interface clock divider

    IDE2 Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD2_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE2_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_LCD2_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD2_OUTIF_CLKDIV)
//@}

/*
    @name   IDE lite clock divider

    IDE2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD_CLKDIV).
*/
//@{
#define PLL_LCD_LITE_CLKDIV(x)           ((x) << (PLL_CLKSEL_LCD_LITE_CLKDIV - PLL_CLKSEL_RLITE_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD2_CLKDIV)
//@}

/*
    @name   IDE lite Output Interface clock divider

    IDE2 Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_LCD2_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_LITE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_LCD_LITE_OUTIF_CLKDIV - PLL_CLKSEL_RLITE_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_LCD2_OUTIF_CLKDIV)
//@}


#define LCD_SETREG(ofs, value)   OUTW(IOADDR_LCD_REG_BASE + (ofs), value)
#define LCD_GETREG(ofs)         INW(IOADDR_LCD_REG_BASE + (ofs))
#define LCD2_SETREG(ofs, value)  OUTW(IOADDR_LCD2_REG_BASE + (ofs), value)
#define LCD2_GETREG(ofs)        INW(IOADDR_LCD2_REG_BASE + (ofs))
#define LCD3_SETREG(ofs, value)  OUTW(IOADDR_LCD3_REG_BASE + (ofs), value)
#define LCD3_GETREG(ofs)        INW(IOADDR_LCD3_REG_BASE + (ofs))

//#define TV_SETREG(ofs, value)    OUTW(IOADDR_TV_REG_BASE + (ofs), value)
//#define TV_GETREG(ofs)          INW(IOADDR_TV_REG_BASE + (ofs))

//extern ER ide_platform_flg_clear(IDE_ID id, FLGPTN flg);
//extern ER ide_platform_flg_set(IDE_ID id, FLGPTN flg);
extern ER ide_platform_flg_wait(IDE_ID id, FLGPTN flg);
//extern ER ide_platform_sem_set(IDE_ID id);
//extern ER ide_platform_sem_wait(IDE_ID id);
extern ULONG ide_platform_spin_lock(IDE_ID id);
extern void ide_platform_spin_unlock(IDE_ID id, ULONG flag);
//extern void ide_platform_sram_enable(IDE_ID id);
//extern void ide_platform_sram_disable(IDE_ID id);
//extern void ide_platform_int_enable(IDE_ID id);
//extern void ide_platform_int_disable(IDE_ID id);
extern void ide_platform_delay_ms(UINT32 ms);
extern void ide_platform_delay_us(UINT32 us);
extern IDEDATA_TYPE ide_platform_va2pa(IDE_ID id, IDEDATA_TYPE addr);
extern UINT32 factor_caculate(UINT16 x, UINT16 y, BOOL h);
//extern PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id);
#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
//extern void ide_platform_create_resource(IDE_ID id);
//extern void ide_platform_release_resource(IDE_ID id);

#else
//extern int DBG_ERR(const char *fmt, ...);
//extern int DBG_WRN(const char *fmt, ...);
//extern int DBG_IND(const char *fmt, ...);

//extern void ide2_isr(void);
//extern void ide_platform_create_resource(MODULE_INFO *pmodule_info);
//extern void ide_platform_release_resource(void);
//extern PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id);
#endif
extern void ide_isr(void);
extern void ide2_isr(void);
extern void ide3_isr(void);
extern void idec_parsing_interrupt(IDE_ID id);

#endif
extern void idec_isr_bottom(IDE_ID id, UINT32 events);
extern BOOL ide_platform_list_empty(IDE_ID id);
extern ER ide_platform_add_list(IDE_ID id, KDRV_CALLBACK_FUNC *p_callback);
extern struct _IDE_REQ_LIST_NODE* ide_platform_get_head(IDE_ID id);
extern ER ide_platform_del_list(IDE_ID id);

extern void ide_platform_set_ist_event(IDE_ID id);
extern int  ide_platform_ist(IDE_ID id, UINT32 event);


#define PLL_CR0_OFS                      0x20
#define PLL_CR1_OFS                      0x24
#define PLL_CR2_OFS                      0x28



#define PLL690_PLL00_BASE_REG_OFS        0x4400
#define PLL690_PLL02_BASE_REG_OFS        0x4480
#define PLL690_PLL03_BASE_REG_OFS        0x44C0
#define PLL690_PLL04_BASE_REG_OFS        0x4500
#define PLL690_PLL05_BASE_REG_OFS        0x4540
#define PLL690_PLL06_BASE_REG_OFS        0x4580
#define PLL690_PLL07_BASE_REG_OFS        0x45C0
#define PLL690_PLL08_BASE_REG_OFS        0x4600
#define PLL690_PLL09_BASE_REG_OFS        0x4640
#define PLL690_PLL10_BASE_REG_OFS        0x4680
#define PLL690_PLL11_BASE_REG_OFS        0x46C0
#define PLL690_PLL12_BASE_REG_OFS        0x4700
#define PLL690_PLL13_BASE_REG_OFS        0x4740
#define PLL690_PLL14_BASE_REG_OFS        0x4780
#define PLL690_PLL15_BASE_REG_OFS        0x47C0
#define PLL690_PLL16_BASE_REG_OFS        0x4800
#define PLL690_PLL17_BASE_REG_OFS        0x4840
#define PLL690_PLL18_BASE_REG_OFS        0x4880
#define PLL690_PLL19_BASE_REG_OFS        0x48C0
#define PLL690_PLL20_BASE_REG_OFS        0x4900
#define PLL690_PLL21_BASE_REG_OFS        0x4940
#define PLL690_PLL22_BASE_REG_OFS        0x4980
#define PLL690_PLL23_BASE_REG_OFS        0x49C0
#define PLL690_PLL24_BASE_REG_OFS        0x4A00


#define PLL690_PLL0_CR0_REG_OFS         (PLL690_PLL00_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL2_CR0_REG_OFS         (PLL690_PLL02_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL3_CR0_REG_OFS         (PLL690_PLL03_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL4_CR0_REG_OFS         (PLL690_PLL04_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL5_CR0_REG_OFS         (PLL690_PLL05_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL6_CR0_REG_OFS         (PLL690_PLL06_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL7_CR0_REG_OFS         (PLL690_PLL07_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL8_CR0_REG_OFS         (PLL690_PLL08_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL9_CR0_REG_OFS         (PLL690_PLL09_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL10_CR0_REG_OFS        (PLL690_PLL10_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL11_CR0_REG_OFS        (PLL690_PLL11_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL12_CR0_REG_OFS        (PLL690_PLL12_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL13_CR0_REG_OFS        (PLL690_PLL13_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL14_CR0_REG_OFS        (PLL690_PLL14_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL15_CR0_REG_OFS        (PLL690_PLL15_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL16_CR0_REG_OFS        (PLL690_PLL16_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL17_CR0_REG_OFS        (PLL690_PLL17_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL18_CR0_REG_OFS        (PLL690_PLL18_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL19_CR0_REG_OFS        (PLL690_PLL19_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL20_CR0_REG_OFS        (PLL690_PLL20_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL21_CR0_REG_OFS        (PLL690_PLL21_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL22_CR0_REG_OFS        (PLL690_PLL22_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL23_CR0_REG_OFS        (PLL690_PLL23_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL690_PLL24_CR0_REG_OFS        (PLL690_PLL24_BASE_REG_OFS+PLL_CR0_OFS)


#endif
