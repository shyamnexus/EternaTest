/*
    Protected header file of NT96510's DMA driver.

    The header file for protected APIs of NT96510's DMA driver.

    @file       dma_protected.h
    @ingroup    mIDrvDMA_DMA
    @note       For Novatek internal reference, don't export to agent or customer.

    Copyright   Novatek Microelectronics Corp. 2017.  All rights reserved.
*/

#ifndef _DMA_PROTECTED_H
#define _DMA_PROTECTED_H

#include "plat/dma.h"
#include "comm/ddr_arb.h"

/*
    Translate DRAM address to physical address.

    Translate DRAM address to physical address.

    @param[in] addr     DRAM address

    @return physical DRAM address
*/
#define         dma_getPhyAddr(addr)            ((((ULONG)(addr))>=0x60000000UL)?((ULONG)(addr)-0x60000000UL):(ULONG)(addr))

/*
    Translate DRAM address to non-cacheable address.

    Translate DRAM address to non-cacheable address.

    @param[in] addr     DRAM address

    @return non-cacheable DRAM address
*/
#define         dma_getNonCacheAddr(addr)       ((((ULONG)(addr))<0x60000000UL)?((ULONG)(addr)+0x60000000UL):(ULONG)(addr))

/*
    Flush read (Device to CPU) cache with checking address is cacheable or not.

    In DMA operation, if buffer is cacheable, we have to flush read buffer before
    DMA operation to make sure CPU will read correct data.

    @param[in] uiAddr   DRAM address
    @param[in] uiSize   DRAM size
*/
#define dma_flushReadCache(uiAddr, uiSize) \
	{ \
		if (dma_isCacheAddr((uiAddr)) == TRUE) \
		{ \
			dma_flushReadCacheWithoutCheck((uiAddr), (uiSize)); \
		} \
	}

/*
    Flush read (Device to CPU) cache with checking address is cacheable or not.

    This API is required to be invoked after DMA complete.

    @param[in] uiAddr   DRAM address
    @param[in] uiSize   DRAM size
*/
#define dma_flushReadCacheDmaEnd(uiAddr, uiSize) \
	{ \
		if (dma_isCacheAddr((uiAddr)) == TRUE) \
		{ \
			dma_flushReadCacheDmaEndWithoutCheck((uiAddr), (uiSize)); \
		} \
	}


/*
    Flush write (CPU to Device) cache with checking address is cacheable or not.

    In DMA operation, if buffer is cacheable, we have to flush write buffer before
    DMA operation to make sure DMA will send correct data.

    @param[in] uiAddr   DRAM address
    @param[in] uiSize   DRAM size
*/
#if defined(__aarch64__)
#define dma_flushWriteCache(uiAddr, uiSize) \
    { \
		if (dma_isCacheAddr((uiAddr)) == TRUE) \
		{ \
			dma_flushWriteCacheWithoutCheck((uiAddr), (uiSize)); \
		} \
		else \
		{ \
			__asm__ __volatile__("dsb sy\n\t");\
		}\
		(*((volatile UINT32*)(0x60000000)));\
	}
#else
#define dma_flushWriteCache(uiAddr, uiSize) \
	{ \
		if (dma_isCacheAddr((uiAddr)) == TRUE) \
		{ \
			dma_flushWriteCacheWithoutCheck((uiAddr), (uiSize)); \
		} \
		else \
		{ \
			__asm__ __volatile__("dsb\n\t");\
		}\
		(*((volatile UINT32*)(0x60000000)));\
	}
#endif

/*
    Flush read (Device to CPU) cache where lineoffset not equal to width

    Once if case like these two condition as show below, need call
    dma_flushReadCacheWidthNEQLineOffset() instead of dma_flushReadCache()

    Example:
    @code
    {
        case(1): image lineoffset != width
         image width
         <-------------->
        start
         +--------------+----+
         |--------------|xxxx|
         |--------------|xxxx|
         |--------------|xxxx|
         |--------------|xxxx|
         +--------------+----+<--end
         <------------------->
         image line offset

         case(2):
            (a) CPU put data into _addr
            (b) Engine read from _addr and process
            (c) Engine write(output) data back to _addr
            (d) CPU read back from _addr
    }
    @endcode

    @param[in] uiAddr   DRAM address
    @param[in] uiSize   DRAM size
*/
#define dma_flushReadCacheWidthNEQLineOffset(uiAddr, uiSize) \
	{ \
		if (dma_isCacheAddr((uiAddr)) == TRUE) \
		{ \
			dma_flushReadCacheWidthNEQLineOffsetWithoutCheck((uiAddr), (uiSize)); \
		} \
	}


/*

    Flush read (Device to CPU) cache without checking buffer is cacheable or not


    In DMA operation, if buffer is cacheable, we have to flush read buffer before
    DMA operation to make sure CPU will read correct data.



    @param[in] uiStartAddr  Buffer starting address
    @param[in] uiLength     Buffer length
    @return Use clean and invalidate data cache all or not (cpu_cleanInvalidateDCacheAll)
        - @b TRUE:   Use cpu_cleanInvalidateDCacheAll
        - @b FALSE:  Not use cpu_cleanInvalidateDCacheAll
*/
extern BOOL             dma_flushReadCacheWithoutCheck(UINT32 uiStartAddr, UINT32 uiLength);

/*

    Flush read (Device to CPU) cache without checking buffer is cacheable or not


    In DMA operation, if buffer is cacheable, we also have to flush read buffer after
    DMA operation to make sure CPU will read correct data.



    @param[in] uiStartAddr  Buffer starting address
    @param[in] uiLength     Buffer length
    @return void
*/
extern void 			dma_flushReadCacheDmaEndWithoutCheck(UINT32 uiStartAddr, UINT32 uiLength);


/*
    Flush write (CPU to Device) cache without checking buffer is cacheable or not

    In DMA operation, if buffer is cacheable, we have to flush write buffer before
    DMA operation to make sure DMA will send correct data.

    @note   Depend on performance measurement
        clean data cache 016K = 00015 us
        clean data cache 032K = 00020 us
        clean data cache 064K = 00031 us
        clean data cache 128K = 00056 us
        clean data cache 256K = 00102 us
        clean data cache 512K = 00198 us

        fatch 16K data cache  = 00024 us

        So, once if (uiEndAddr - uiStartAddr) > 32K
        calling cpu_cleanInvalidateDCacheAll

    @param[in] uiStartAddr  Buffer starting address
    @param[in] uiLength     Buffer length
    @return Use clean and invalidate data cache all or not (cpu_cleanInvalidateDCacheAll)
        - @b TRUE:   Use cpu_cleanInvalidateDCacheAll
        - @b FALSE:  Not use cpu_cleanInvalidateDCacheAll
*/
extern BOOL             dma_flushWriteCacheWithoutCheck(UINT32 uiStartAddr, UINT32 uiLength);

/*
    Flush read (Device to CPU) cache where lineoffset not equal to width
    Flush read (Device to CPU) cache where lineoffset not equal to width

    @param[in] uiStartAddr  Buffer starting address
    @param[in] uiLength     Buffer length
    @return Use clean and invalidate data cache all or not (cpu_cleanInvalidateDCacheAll)
        - @b TRUE:   Use cpu_cleanInvalidateDCacheAll
        - @b FALSE:  Not use cpu_cleanInvalidateDCacheAll

*/
extern BOOL             dma_flushReadCacheWidthNEQLineOffsetWithoutCheck(UINT32 uiStartAddr, UINT32 uiLength);

//@}

#endif
