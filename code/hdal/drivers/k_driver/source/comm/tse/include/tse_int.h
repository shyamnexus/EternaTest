/*
    TS MUXER/TS DEMUXER/HWCOPY Engine Integration module internal driver header

    @file       tse_int.h
    @ingroup    mIDrvMisc_TSE
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.
*/



/*
    @addtogroup mIDrvMisc_TSE
*/
//@{

#ifndef _TSE_INT_H
#define _TSE_INT_H

#include "kwrap/nvt_type.h"
#include "comm/tse.h"
#include <nvt_api_ver.h>

#if defined(__FREERTOS)
#include "rcw_macro.h"
#else
#ifdef __KERNEL__
#include <linux/soc/nvt/rcw_macro.h>
#endif
#endif

//
//  TSE register access definition
//
#ifdef _NVT_EMULATION_
#define TSE_BUF_MAX_CNT	100//5
#else
#define TSE_BUF_MAX_CNT	6
#endif

#define TSE_STS_OPENED	0x00000001
#define TSE_STS_BUSY 	0x00000002

typedef enum {
	INT_EN_COMPLETE = (1 << 0),
	INT_EN_INPUT_END = (1 << 1),

	INT_EN_OUT0_FULL = (1 << 4),
	INT_EN_OUT1_FULL = (1 << 5),
	INT_EN_OUT2_FULL = (1 << 6),

	INT_EN_CONTY0_ERR = (1 << 8),
	INT_EN_TEI0 = (1 << 9),
	INT_EN_SYNC0_ERR = (1 << 10),
	INT_EN_ADAPFLG0_ERR = (1 << 11),

	INT_EN_CONTY1_ERR = (1 << 16),
	INT_EN_TEI1 = (1 << 17),
	INT_EN_SYNC1_ERR = (1 << 18),
	INT_EN_ADAPFLG1_ERR = (1 << 19),

	INT_EN_CONTY2_ERR = (1 << 24),
	INT_EN_TEI2 = (1 << 25),
	INT_EN_SYNC2_ERR = (1 << 26),
	INT_EN_ADAPFLG2_ERR = (1 << 27),

	ENUM_DUMMY4WORD(TSE_INT_EN)
} TSE_INT_EN;

typedef ER (*SET_FP)(TSEDATA_TYPE, TSEDATA_TYPE *);
typedef TSEDATA_TYPE (*GET_FP)(TSEDATA_TYPE);
typedef ER (*TRIG)(BOOL);
typedef void (*DONE)(void);

typedef struct {
	UINT32 id;
	TSEDATA_TYPE val;
	UINT32 max;
	SET_FP set_fp;
	GET_FP get_fp;
	CHAR *msg;
} TSE_CFG_ITEM_INFO;


typedef struct {
	TRIG trig;
	DONE done;
} TSE_MODE_INFO;

void tse_isr(void);


/************************************************/
/****   platform extern to single source drv ****/
/************************************************/
extern void tse_platform_clr_flg(void);
extern void tse_platform_set_flg(void);
extern void tse_platform_iclr_flg(void);
extern void tse_platform_iset_flg(void);
extern void tse_platform_wai_flg(void);
extern unsigned long tse_platform_loc_cpu(void);
extern void tse_platform_unl_cpu(unsigned long flag);
extern UINT32 tse_platform_wait_sem(void);
extern UINT32 tse_platform_sig_sem(void);
extern UINT32 tse_platform_attach(UINT32 clk);
extern UINT32 tse_platform_detach(void);
extern UINT32 tse_platform_disable_int(void);
extern UINT32 tse_platform_enable_int(void);
extern UINT32 tse_platform_cache_sync_dma_to_dev(TSEDATA_TYPE addr, UINT32 size);
extern UINT32 tse_platform_cache_sync_dma_from_dev(TSEDATA_TYPE addr, UINT32 size);
extern TSEDATA_TYPE tse_platform_get_reg_base(void);
extern UINT64 tse_platform_get_phy_addr(TSEDATA_TYPE addr);

#if (defined __FREERTOS)
extern void tse_platform_set_resource(void);
extern void tse_platform_release_resource(void);
#endif


#define TSE_GETREG(ofs) INW((tse_platform_get_reg_base() + ofs))
#define TSE_SETREG(ofs, value) OUTW(tse_platform_get_reg_base() + (ofs), (value))


//extern void (*TSE_SETREG)(uintptr_t offset, UINT32 value);
//extern NVT_API_CHK_DECLARE(tse);

//@}
#endif